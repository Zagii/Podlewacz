/*
 PubSubClient -mqtt


 piny bez ryzyka
 D1, D2, D5, D6, D7
 
 flashowanie
 TX(D10),RX(D8)

 status boot
 D8, D3, D4
*/
#include <ESP8266WiFi.h>
#include <ESP8266WiFiMulti.h>
#include <PubSubClient.h>
#include <WebSocketsServer.h>
#include <ESP8266WebServer.h>
#include <ESP8266mDNS.h>
#include <Adafruit_PWMServoDriver.h>
//#include <pcf8574_esp.h>
#include <ArduinoJson.h>
#include <Time.h>
#include <ArduinoTrace.h>
//#include <TimeLib.h>
#include "Defy.h"
#include "CWifiManager.h" 
#include "Config.h"
#include "CWebSerwer.h"


bool isNumber(char * tmp);
void parsujRozkaz(char *topic, char *msg);
void wylaczWszystko();
////////////pcf
//PCF857x pcf8574(0b00111000, &Wire);
////////////////
/////// pca9685
Adafruit_PWMServoDriver hardwareOut = Adafruit_PWMServoDriver(0x40);
/////////

CWifiManager wifi;
PubSubClient *mqtt;

CConfig conf;

CWebSerwer web;

char tmpTopic[MAX_TOPIC_LENGHT];
char tmpMsg[MAX_MSG_LENGHT];

////////////// sprawdzic ntp
////////// https://github.com/arduino-libraries/NTPClient

unsigned long czasLokalnyMillis=0;
unsigned long czasLokalny=0;

unsigned long sLEDmillis=0;

uint8_t stanSekcji=0;
bool czekaNaPublikacjeStanuMQTT=false;
bool czekaNaPublikacjeStanuWS=false;
bool czekaNaPublikacjeStanuHW=false;

bool czekaNaPublikacjeLBL=false;
bool czekaNaPublikacjePROG=false;
bool czekaNaPublikacjeKONF=false;
bool czekaNaPublikacjeSTAT=false;

uint8_t ID=0;
unsigned long Millis=0;

/////////////// czujnik wilgoci ///////////////////////
int stanCzujnikaWilgoci;             // the current reading from the input pin
int lastButtonState = LOW;   // the previous reading from the input pin
unsigned long lastDebounceTime = 0;  // the last time the output pin was toggled
unsigned long debounceDelay = 50;    // the debounce time; increase if the output flickers
////////////////////// czujnik wilgoci koniec //////////

void(* resetFunc) (void) = 0; //declare reset function @ address 0



void callback(char* topic, byte* payload, unsigned int length) 
{
  char* p = (char*)malloc(length+1);
  memcpy(p,payload,length);
  p[length]='\0';
  if(strstr(topic,"watchdog"))
  {
    DPRINT("Watchdog msg=");
    DPRINT(p);
    DPRINT(" teraz=");
   
    if(isNumber(p))
      wifi.setWDmillis(strtoul (p, NULL, 0));
    DPRINTLN(wifi.getWDmillis());
    

  }
 if(topic[strlen(topic)-1]=='/')
    {topic[strlen(topic)-1] = '\0';}
 DPRINT("Debug: callback topic=");
 DPRINT(topic);
 DPRINT(" msg=");
 DPRINTLN(p);
 parsujRozkaz(topic,p);
  free(p);
}

////////////// obsluga websocket

void wse(uint8_t num, WStype_t type, uint8_t * payload, size_t length)
{
  web.webSocketEvent(num,type,payload,length);
  if(type==WStype_TEXT)
  {
    char* p = (char*)malloc(length+1);
    memcpy(p,payload,length);
    p[length]='\0';
    DPRINT("webSocket TEXT: ");DPRINTLN(p);

    DynamicJsonDocument doc(200);
    DeserializationError error= deserializeJson(doc,p);

    if (error) 
    {
      Serial.print(F("deserializeJson failed: "));
      Serial.println(p);
      Serial.print(doc.memoryUsage());Serial.print(F(" bytes. "));
      Serial.println(error.c_str());
      Serial.println(p);
      free(p);
      DPRINTLN("return");
      return;
    }

    char topic[200]="";
    char msg[200]="";
    JsonObject root = doc.as<JsonObject>();
    
    for (JsonPair p : root) {
       strcpy(topic,p.key().c_str());
        DPRINT(topic);DPRINTLN("root[topic]=");//Serial.println(String(root[topic]));
     /*   kv.value.prettyPrintTo(Serial); Serial.println(" # ");kv.value.printTo(Serial);*/
       if(root[topic].is<const char*>())
       {
          DPRINTLN("msg char*");
          strcpy(msg,(char*)p.value().as<char*>());
       }else 
       { 
        if(p.value().is<JsonObject>())
        {
            DPRINTLN(" p obj ");
           // root[topic].printTo(msg);
           String s=p.value().as<String>();
            strcpy(msg,s.c_str());
            DPRINT("msg JSON: ");DPRINTLN(msg);
        }else if(p.value().is<JsonArray>())
         {
           DPRINTLN(" p array ");
         }else  if(p.value().is<unsigned int>())
         {
          DPRINTLN(" p uint ");
          itoa ((uint8_t)p.value().as<unsigned int>(), msg, 10);
          }else {   DPRINTLN(" p undef... ");   }
       }
       DPRINT(topic);DPRINT("=");DPRINTLN(msg);
       parsujRozkaz(topic,msg);
     }
    free(p);
  }
}

/////////////////////////SETUP///////////////////////////
void setup()
{
 
  Serial.begin(115200);
   
  DPRINTLN("AquaTouch Init");
  DPRINTLN("Setup Serial");
  pinMode(LED, OUTPUT);     // Initialize the BUILTIN_LED pin as an output
  digitalWrite(LED,ON);
  delay(500); // delay na zabezpieczajacy seriala
  //Setup PCF8574
  //Wire.pins(PIN_SDA, PIN_SCL);//SDA - D1, SCL - D2
  //Wire.begin();
  hardwareOut.setPWMFreq(60); 
  hardwareOut.begin();
  hardwareOut.setOutputMode(true);
 /*  for(uint8_t i=0;i<8;i++)
        {
         if(i<4)
         hardwareOut.setPin(i,0);
         else
         hardwareOut.setPin(i,4095);
        }*/
  pinMode(PIN_WILGOC, INPUT_PULLUP); //czujnik wilgoci

  //pcf8574.begin( 0x00 ); //8 pin output
 // pcf8574.resetInterruptPin();
  wylaczWszystko();
  //


conf.begin();
conf.setTryb(TRYB_AUTO);
//delay(1000);
DPRINTLN("Programy");
/*Program pp;
conf.setProg(pp,1, 1, 1970, 7, 0,0,8*60,1,1); 
conf.addProg(pp);
conf.setProg(pp,1, 1, 1970, 7, 8,10,8*60,1,2);
conf.addProg(pp);
conf.setProg(pp,1, 1, 1970, 7, 16,20,8*60,1,3);
conf.addProg(pp);
conf.setProg(pp,1, 1, 1970, 7, 24,30,8*60,1,4);
conf.addProg(pp);
conf.setProg(pp,1, 1, 1970, 7, 32,40,8*60,1,6);
conf.addProg(pp);
conf.setProg(pp,1, 1, 1970, 7, 40,50,5*60,1,5);
conf.addProg(pp);
conf.setProg(pp,1, 1, 1970, 19, 0,0,8*60,1,1); 
conf.addProg(pp);
conf.setProg(pp,1, 1, 1970, 19, 8,10,8*60,1,2);
conf.addProg(pp);
conf.setProg(pp,1, 1, 1970, 19, 16,20,8*60,1,3);
conf.addProg(pp);
conf.setProg(pp,1, 1, 1970, 19, 24,30,8*60,1,4);
conf.addProg(pp);
conf.setProg(pp,1, 1, 1970, 19, 32,40,8*60,1,6);
conf.addProg(pp);
conf.setProg(pp,1, 1, 1970, 19, 40,50,5*60,1,5);
conf.addProg(pp);
conf.publishAllProg();

conf.saveConfig();
*/
conf.publishAllProg();

//////////////// odczyt WiFi
String wifiJson=conf.loadJsonStr(PLIK_WIFI);
DPRINT("Konfig wifi:");DPRINTLN(wifiJson);
wifi.zmianaAP(wifiJson.c_str());
String mqttJson=conf.loadJsonStr(PLIK_MQTT);
DPRINT("Konfig mqtt:");DPRINTLN(mqttJson);
DPRINTLN("TRACE 1");
wifi.setupMqtt(mqttJson.c_str());
//String NTPJson=conf.loadJsonStr(PLIK_NTP);  TODO: odczyt NTP z konfiguracji przy inicjalizacji
//wifi.setNTP()
//////////////// wifi i mqtt init ///////////
DPRINTLN("TRACE 2");
  wifi.begin();
  DPRINTLN("TRACE 3");
  mqtt=wifi.getMQTTClient();
  mqtt->setCallback(callback);
///////////// koniec wifi i mqtt init /////////

//////////////// wifi i mqtt init ///////////
  wifi.begin();
  mqtt=wifi.getMQTTClient();
  mqtt->setCallback(callback);
///////////// koniec wifi i mqtt init /////////

web.begin();
WebSocketsServer * webSocket=web.getWebSocket();
webSocket->onEvent(wse);
}


void zmienStanSekcjiAll(uint8_t stan)
{
  if(stanSekcji==stan) return;
   stanSekcji=stan;
   czekaNaPublikacjeStanuHW=true;
}
void wylaczWszystko()
{
  zmienStanSekcjiAll(0);
}
void zmienStanSekcji(uint8_t sekcjanr,uint8_t stan)
{
  DPRINT("zmienStanSekcji nr=");DPRINT(sekcjanr);DPRINT(", stan=");DPRINT(stan);DPRINT(", stanSekcji=");DPRINTLN(stanSekcji);
  uint8_t x=bitRead(stanSekcji,sekcjanr);
  if(x==stan)return;
  if(stan==1)
  {
    DPRINT("ON");
    bitSet(stanSekcji,sekcjanr);
  }else
  {
    DPRINT("OFF");
    bitClear(stanSekcji,sekcjanr);
  }
   DPRINT("zmienStanSekcji koniec nr=");DPRINT(sekcjanr);DPRINT(", stan=");DPRINT(stan);DPRINT(", stanSekcji=");DPRINTLN(stanSekcji);
   czekaNaPublikacjeStanuHW=true;
}
void publikujStanSekcjiMQTT()
{
   if(wifi.getConStat()!=CONN_STAT_WIFIMQTT_OK)return;
   
   uint8_t b = stanSekcji;//pcf8574.read8();
   DPRINT("publikujStanSekcjiMQTT ");DPRINT(b);DPRINT(", ");DPRINTLN(stanSekcji);
   for(int i=SEKCJA_MIN;i<=SEKCJA_MAX;i++)
   {
      //if(b&(1<<i))
      if(bitRead(b,i))
      {
          sprintf(tmpTopic,"%s/SEKCJA/%d/",wifi.getOutTopic(),i);
          strcpy(tmpMsg,"1");
      }else
      {
          sprintf(tmpTopic,"%s/SEKCJA/%d/",wifi.getOutTopic(),i);
          strcpy(tmpMsg,"0");
      }
      wifi.RSpisz(tmpTopic,tmpMsg);
   }
   DPRINT("######### ZMIANA STANU WYJSC ############ ");DPRINTLN(stanSekcji,BIN);
}

 void parsujRozkaz(char *topic, char *msg)
 {
  DPRINT("parsujRozkaz topic=");DPRINT(topic);DPRINT(", msg=");DPRINTLN(msg);
   char *ind=NULL;
   ///////////////// SEKCJA  //////////////////////////
   ind=strstr(topic,"SEKCJA/");
   if(ind!=NULL)
   {
    DPRINTLN(ind);
     ind+=strlen("SEKCJA/"); //bo jeszcze nr sekcji
    DPRINTLN(ind);
     if(isIntChars(ind))
     {
        if(isIntChars(msg))
        {
          if(msg[0]=='0')
          {
            zmienStanSekcji(atoi(ind),0);
          }else
          {
            zmienStanSekcji(atoi(ind),1);
          }
        }else
        {
          DPRINT("ERR dla topic SEKCJAx msg ");DPRINT(msg);DPRINT(" nie int, linia:");DPRINTLN(__LINE__);
        }
     }else
     {
         DPRINT("ERR topic ");DPRINT(topic);DPRINT(" nie int, linia:");DPRINTLN(__LINE__);
     }
     return;
    }
    //////////////////////////////////////
    ind=strstr(topic,"TRYB");
    if(ind!=NULL)
    {
       if(msg[0]=='a')
       {
          conf.setTryb(TRYB_AUTO);  
       }
       if(msg[0]=='m')
       {
            conf.setTryb(TRYB_MANUAL);
       }

        return;
    }
    ind=strstr(topic,"CZAS");
    if(ind!=NULL)
    {
      czasLokalny=atoi(msg);
      return;
    }
    ind=strstr(topic,"DEL_PROG");
    if(ind!=NULL)
    {
      uint16_t delID=atoi(msg);
      conf.delProg(delID);
      conf.saveProgs();  
      czekaNaPublikacjePROG=true;
    }
    ind=strstr(topic,"GET");
    if(ind!=NULL)
    {
       ind=strstr(msg,"SLBL");
       if(ind!=NULL)
       {
          czekaNaPublikacjeLBL=true;
       }
       ind=strstr(msg,"PROG");
       if(ind!=NULL)
       {
        czekaNaPublikacjePROG=true;
       }
       ind=strstr(msg,"KONF");
       if(ind!=NULL)
       {
        czekaNaPublikacjeKONF=true;
       }
       ind=strstr(msg,"STAT");
       if(ind!=NULL)
       {
        czekaNaPublikacjeSTAT=true;
       }
      
       return;
    }
    //////////////////////// komendy ktore maja jsona jako msg /////////////////////////
    DynamicJsonDocument doc(200);
    DeserializationError error= deserializeJson(doc,msg);

    if (error) 
    {
      Serial.print(F("Blad parsowania json !!!!"));
      Serial.println(msg);
       Serial.print(doc.memoryUsage());Serial.print(F(" bytes. "));
      Serial.println(error.c_str());
      Serial.println(msg);
      DPRINTLN("return");
      return;
    }
    JsonObject json = doc.as<JsonObject>();
    String jsonStr;
    serializeJson(json,jsonStr); 
   
    DPRINTLN("Parsowanie zagniezdzonego jsona");
  
    ind=strstr(topic,"NTP");
    if(ind!=NULL)
    {
      const char* host=json["host"];
      unsigned long offset=json["offset"];
      wifi.setNTP(host,offset);
      // zapisz do pliku
      conf.saveConfigStr(PLIK_NTP,jsonStr.c_str());
    }
    
    ind=strstr(topic,"Wifi");
    if(ind!=NULL)
    {
     const char* tryb=json["tryb"];
      const char* ssid=json["ssid"];
       const char* pass=json["pass"];
      if(strcmp(tryb,"STA")==0)
      {
        wifi.zmianaAP(ssid,pass);
        conf.saveConfigStr(PLIK_WIFI,jsonStr.c_str());
        // zapisz do pliku
      }else
      {/// utworzyc AP
        
      }
      czekaNaPublikacjeKONF=true;
    }
    ind=strstr(topic,"Mqtt");
    if(ind!=NULL)
    {     
     const char* host=json["host"];
      uint16_t port=json["port"];
       String user="";
       String pwd="";
      if(json.containsKey("user"))
      {
       user=String((const char*)json["user"]);
      }
      if(json.containsKey("pwd"))
      {
        pwd=String((const char*)json["pwd"]);
      }
      wifi.setupMqtt(host,port,user.c_str(),pwd.c_str());
      // zapisz do pliku
      conf.saveConfigStr(PLIK_MQTT,jsonStr.c_str());
      czekaNaPublikacjeKONF=true;
    }
    ind=strstr(topic,"LBL");
    if(ind!=NULL)
    {
      uint8_t id=json["id"];
     
      String lbl=json["lbl"];
      conf.setSekcjaLbl(id,lbl);
      String str="{\"LBL\":[";
      for(int i=0;i<8;i++)
      {
        str+="{\"id\":"+String(i)+",\"lbl\":\""+conf.getSekcjaLbl(i)+"\"}";
        if(i<7)str+=",";
      }
      str+="]}";
      conf.saveConfigStr(PLIK_LBL,str.c_str());
      czekaNaPublikacjeLBL=true;
    }
    
    ind=strstr(topic,"PROG");
    if(ind!=NULL)
    {
       if(json.containsKey("id"))
      {
        uint16_t i=json["id"];
        Program a;
        conf.getProg(a,i);
        if(json.containsKey("ms")&&json.containsKey("dzienTyg"))
        {
          /////////// tu przeba przeliczyc
          //a.dataOdKiedy=json["dt"];
          a.dzienTyg=json["dzienTyg"];
          a.godzinaStartu=json["ms"];
        }
        if(json.containsKey("tStr"))a.tStr=String(json["tStr"].as<char*>());
        if(json.containsKey("okresS"))a.czas_trwania_s=json["okresS"];
        if(json.containsKey("sekcja"))a.sekcja=json["sekcja"];
        if(json.containsKey("coIle"))a.co_ile_dni=json["coIle"];
        if(json.containsKey("aktywny"))a.aktywny=json["aktywny"];
        ///////////////
        //set progr tab
        conf.changeProg(a,i);
        conf.saveProgs();
        czekaNaPublikacjePROG=true;
      }else
      {
        DPRINTLN("Nowy program");
        if(json.containsKey("dzienTyg")&&json.containsKey("ms")&&json.containsKey("okresS")&&json.containsKey("sekcja")&&json.containsKey("coIle")&&json.containsKey("aktywny"))
        {
          Program a;
          conf.setProg(a,json["dzienTyg"],json["tStr"],json["ms"],json["okresS"],json["coIle"],json["sekcja"],json["aktywny"]);
          conf.addProg(a);
          conf.saveProgs();
          czekaNaPublikacjePROG=true;
        }else
          {DPRINTLN("Za mało parametrów by dodać program.");}
      }
    }
 }

void createStatusJson(char* bufJson)
{
 //// przygotowanie ogólnego statusu
    DynamicJsonDocument root(1024);
    // Serial.print(F(", jsonDocument: ")); Serial.print(system_get_free_heap_size());

   //JsonObject root = doc.as<JsonObject>();
   //  Serial.print(F(", parse: ")); Serial.print(system_get_free_heap_size());

     /*
    *  1 styczen 2010 to
    *  Epoch timestamp: 1262304000
    *  Timestamp in milliseconds: 1262304000000
    */
    if(wifi.getEpochTime()<1262304000)// czyli brak polaczenia z NTP
    {
       root["CZAS"]=czasLokalny; 
    }else
    {
     root["CZAS"]= wifi.getEpochTime();
    }
    root["SEKCJE"]=stanSekcji;
    root["TRYB"]=String(conf.getTryb());
    root["GEO"]="Duchnice";
    root["TEMP"]=20.1f;
    root["CISN"]=1023.34f;
    root["DESZCZ"]=stanCzujnikaWilgoci;
    char t[20];
    root["SYSTIME"]=wifi.TimeToString(t,millis()/1000);
    root["Mem"]=ESP.getFreeHeap();
    String tmpStr;
    
    serializeJson(root,tmpStr);
   // serializeJson(root,Serial);
    strcpy(bufJson,tmpStr.c_str()); 
   
}

unsigned long d=0;
char tmpTxt[300];
String millisTimeStr;
void loop()
{

  /////////////// czujnik wilgoci //////////////////
   int reading = digitalRead(PIN_WILGOC);
    if (reading != lastButtonState) {
    lastDebounceTime = millis();
  }

  if ((millis() - lastDebounceTime) > debounceDelay) {
    if (reading != stanCzujnikaWilgoci) {
      stanCzujnikaWilgoci = reading;
    }
  }
  lastButtonState = reading;
/////////////// czujnik wilgoci koniec //////////////////   
 
   if(millis()-czasLokalnyMillis>1000)
  {
    czasLokalnyMillis=millis();
    czasLokalny++;
  //  millisTimeStr=String(wifi.TimeToString(millis()/1000));

    createStatusJson(tmpTxt);
    
    char tmpTopic[MAX_TOPIC_LENGHT];
    sprintf(tmpTopic,"%s/INFO/",wifi.getOutTopic());
   /// Serial.println(tmpTxt);
    wifi.RSpisz(String(tmpTopic),String(tmpTxt),true);


  ////////////
  }
 yield();
  wifi.loop();
 yield();

    web.loop(czasLokalny, String(tmpTxt));
  
 delay(5);

 
 
   ///// LED status blink
   d=millis()-sLEDmillis;
   if(d>3000)// max 3 sek
   {
     sLEDmillis=millis();
     wifi.getWifiStatusString(tmpTxt);
     
    // DPRINT( "[");DPRINT(wifi.getTimeString());DPRINT("] ");DPRINTLN(wifi.getEpochTime());
     if(conf.getTryb()==TRYB_AUTO) // test czy programator każe wlączyć
     {
      uint8_t sekcjaProg=conf.wlaczoneSekcje(wifi.getEpochTime());
      //Serial.print(F("Tryb: "));
      //Serial.println(sekcjaProg,BIN);
      zmienStanSekcjiAll(sekcjaProg);
     }
    if(stanCzujnikaWilgoci==LOW);
    {
      //DPRINTLN("WYLACZANIE Z POWODU DESZCZU !!!!");
   //   zmienStanSekcjiAll(0);
    }
   }
   /////////////////// obsluga hardware //////////////////////
    if(czekaNaPublikacjeStanuHW)
    {
      
        //pcf8574.write8(~stanSekcji);
        for(uint8_t i=0;i<8;i++)
        {
          uint8_t x=bitRead(stanSekcji,i);
          if(x==1)
            hardwareOut.setPWM(i,4096,0);
          else hardwareOut.setPWM(i,0,4096);
        }
        czekaNaPublikacjeStanuMQTT=true;
        czekaNaPublikacjeStanuWS=true;
        czekaNaPublikacjeStanuHW=false;
        delay(5);
    }
    /////////// publikowanie ///////////////
    if(czekaNaPublikacjeStanuMQTT)
    {
        publikujStanSekcjiMQTT();  // na podstawie pcf8574
        czekaNaPublikacjeStanuMQTT=false;   
        delay(5);
    }
    if(czekaNaPublikacjeStanuWS)
    {
        web.publikujStanSekcji(stanSekcji);
        czekaNaPublikacjeStanuWS=false;     
        delay(5);
    }
    
   if(czekaNaPublikacjeLBL)
   {
      String str;
      for(int i=1;i<7;i++)
      {
        str="{\"id\":"+String(i)+",\"lbl\":\""+conf.getSekcjaLbl(i)+"\"}";
        char tmpTopic[MAX_TOPIC_LENGHT];
        sprintf(tmpTopic,"%s/LBL/",wifi.getOutTopic());
        wifi.RSpisz((const char*)tmpTopic,(char*)str.c_str());
        String js=String("{\"LBL\":")+str+"}";
        DPRINTLN(js);
         web.sendWebSocket(js.c_str());
      }
      delay(10);      
      czekaNaPublikacjeLBL=false;
   }
   if(czekaNaPublikacjePROG)
   {
    DPRINTLN("PublikacjaPROG");
     char tmpTopic[MAX_TOPIC_LENGHT];
     sprintf(tmpTopic,"%s/INIT_PROGS/",wifi.getOutTopic());
      DPRINTLN(tmpTopic);
      String jjs=String(conf.getProgIle());
      DPRINTLN(jjs);
     wifi.RSpisz(String(tmpTopic),jjs);
     jjs=String("{\"INIT_PROGS\":")+String(conf.getProgIle())+"}";
     DPRINTLN(jjs);
     web.sendWebSocket(jjs.c_str());
      delay(5);
     sprintf(tmpTopic,"%s/PROG/",wifi.getOutTopic());
    for(uint16_t i=0;i<conf.getProgIle();i++)
    {
      String str=conf.publishTabProgJsonStr(i);
      wifi.RSpisz((const char*)tmpTopic,(char*)str.c_str());
      String js=String("{\"PROG\":")+str+"}";
      DPRINTLN(js);
      web.sendWebSocket(js.c_str());
      delay(1);
    }
    czekaNaPublikacjePROG=false;
    delay(10);
   }
   if(czekaNaPublikacjeKONF)
   {
    //ntp
    char konf_json[30];
    char konf_msg[MAX_MSG_LENGHT];
    wifi.getNTPjsonStr(konf_json);
    char tmpTopic[MAX_TOPIC_LENGHT];
    sprintf(tmpTopic,"%s/NTP/",wifi.getOutTopic());
    wifi.RSpisz((const char*) tmpTopic,konf_json);
    sprintf(konf_msg,"{\"NTP\":%s}",konf_json);
    web.sendWebSocket(konf_msg);
    //Wifi
    wifi.getWifijsonStr(konf_json); 
    sprintf(tmpTopic,"%s/Wifi/",wifi.getOutTopic());
    wifi.RSpisz((const char*) tmpTopic,konf_json);
    sprintf(konf_msg,"{\"Wifi\":%s}",konf_json);
    web.sendWebSocket(konf_msg);
    //Mqtt
    wifi.getMQTTjsonStr(konf_json); 
    sprintf(tmpTopic,"%s/Mqtt/",wifi.getOutTopic());
    wifi.RSpisz((const char*) tmpTopic,konf_json);
    sprintf(konf_msg,"{\"Mqtt\":%s}",konf_json);
    web.sendWebSocket(konf_msg);

    czekaNaPublikacjeKONF=false;
    delay(10);
   }
   if(czekaNaPublikacjeSTAT)
   {//tryb
    //sekcja
    //geo,temp,czas,cisn,deszcz
    czekaNaPublikacjeSTAT=false;
    delay(5);
   }
   
  ///////////////////// status LED /////////////////////////
            switch(wifi.getConStat())
            {
              case CONN_STAT_NO: ///----------__------------__  <-- ten stan praktycznie nie występuje
                if(d<2800)digitalWrite(LED,ON); else digitalWrite(LED,OFF);
                    break;
              case CONN_STAT_WIFI_CONNECTING: // ------_-_---------_-_----
                if(d>=0&&d<1000)digitalWrite(LED,ON);
                if(d>=1000&&d<1300)digitalWrite(LED,OFF);
                if(d>=1300&&d<1600)digitalWrite(LED,ON);
                if(d>=1600&&d<1900)digitalWrite(LED,OFF);
                if(d>=1900)digitalWrite(LED,ON);
                    break;
              case CONN_STAT_WIFI_OK: // ---___---___---___ <-- ten stan praktycznie nie występuje
                if(d>=0&&d<700)digitalWrite(LED,ON);
                if(d>=700&&d<1400)digitalWrite(LED,OFF);
                if(d>=1400&&d<2100)digitalWrite(LED,ON);
                if(d>=2100)digitalWrite(LED,OFF);
                    break;
              case CONN_STAT_WIFIMQTT_CONNECTING:// ---___---___---___
                  digitalWrite(LED, !digitalRead(LED));
                    break;
              case CONN_STAT_WIFIMQTT_OK: // --___________--_________
               if(d<300)digitalWrite(LED,ON); else digitalWrite(LED,OFF);
                    break;
              }
}

/////////////////////////////////////////////////////////// pomocnicze funkcje /////////////////////////////


bool isFloatString(String tString) {
  String tBuf;
  bool decPt = false;
 
  if(tString.charAt(0) == '+' || tString.charAt(0) == '-') tBuf = &tString[1];
  else tBuf = tString; 

  for(uint16_t x=0;x<tBuf.length();x++)
  {
    if(tBuf.charAt(x) == '.') {
      if(decPt) return false;
      else decPt = true; 
    }   
    else if(tBuf.charAt(x) < '0' || tBuf.charAt(x) > '9') return false;
  }
  return true;
}


bool isFloatChars(char * ctab) {
  
  boolean decPt = false;
 uint8_t startInd=0;
  if(ctab[0] == '+' || ctab[0] == '-') startInd=1;

  for(uint8_t x=startInd;x<strlen(ctab);x++)
  {
    if(ctab[x] == '.')// ||ctab[x] == ',') 
    {
      if(decPt) return false;
      else decPt = true; 
    }   
    else if(!isDigit(ctab[x])) return false;
  }
  return true;
}
  
  bool isIntChars(char * ctab) {
  
  //bool decPt = false;
  uint8_t startInd=0;
  if(ctab[0] == '+' || ctab[0] == '-') startInd=1;

  for(uint8_t x=startInd;x<strlen(ctab);x++)
  {
   if(!isDigit(ctab[x])) return false;
  }
  return true;
}
bool isNumber(char * tmp)
{
   uint16_t j=0;
   while(j<strlen(tmp))
  {
    if(tmp[j] > '9' || tmp[j] < '0')
    {
      return false;
    }     
    j++;
  }
 return true; 
}
void PrintMEM()
{ 
  char m[30];
  sprintf(m,"Free Heap: %d bytes.",ESP.getFreeHeap());
  Serial.println(m);
}