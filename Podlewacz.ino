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

#include <pcf8574_esp.h>
#include <ArduinoJson.h>
#include <Time.h>
#include <TimeLib.h>
#include "Defy.h"
#include "CWifi.h"
#include "Config.h"
#include "CWebSerwer.h"



////////////pcf
PCF857x pcf8574(0b00111000, &Wire);
////////////////

CWifi wifi;
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

byte stanSekcji=0;
bool czekaNaPublikacjeStanuMQTT=false;
bool czekaNaPublikacjeStanuWS=false;
bool czekaNaPublikacjeStanuHW=false;

uint8_t publicID=0;
unsigned long publicMillis=0;


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
    DPRINT("wse if type==TEXT: ");DPRINTLN(p);
    
    char t[MAX_TOPIC_LENGHT];
    StaticJsonBuffer<200> jsonBuffer;
    JsonObject& root = jsonBuffer.parseObject(p);
    if (!root.success()) 
    {
      Serial.println("parseObject() failed");
      Serial.println(p);
      free(p);
      DPRINTLN("return");
      return;
    }

    char* topic="";
    char* msg="";
    for (auto kv : root) {
       topic=(char*)kv.key;
       msg=(char*)kv.value.as<char*>();
       DPRINT(topic);DPRINT("=");DPRINTLN(msg);
       
    
    //const char* topic = root["topic"];
    //const char* msg= root["msg"];
    // if(topic){ DPRINT("topic: ");DPRINTLN(topic);
     
    // if(msg) {DPRINT("msg: ");DPRINTLN(msg);
   //   if(strstr(topic,"SEKCJA")||strstr(topic,"TRYB"))//sekcja
    //  {
     //   strcpy(t,topic);
       // parsujRozkaz(t,(char*)msg);
        parsujRozkaz(topic,msg);
     // }else DPRINTLN("nieznany rozkaz");
    }
    free(p);
  }
}

/////////////////////////SETUP///////////////////////////
void setup()
{
 
  Serial.begin(115200);
   
  DPRINTLN("");
  DPRINTLN("Setup Serial");
  pinMode(LED, OUTPUT);     // Initialize the BUILTIN_LED pin as an output
  digitalWrite(LED,ON);
 
  //Setup PCF8574
  Wire.pins(PIN_SDA, PIN_SCL);//SDA - D1, SCL - D2
  Wire.begin();
  
  pinMode(PIN_WILGOC, INPUT_PULLUP); //czujnik wilgoci

  pcf8574.begin( 0x00 ); //8 pin output
  pcf8574.resetInterruptPin();
  wylaczWszystko();
  //

//////////////// wifi i mqtt init ///////////
  wifi.begin();
  mqtt=wifi.getMQTTClient();
  mqtt->setCallback(callback);
///////////// koniec wifi i mqtt init /////////

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
web.begin();
WebSocketsServer * webSocket=web.getWebSocket();
webSocket->onEvent(wse);
}

void wylaczWszystko()
{
  stanSekcji=0;
  czekaNaPublikacjeStanuMQTT=true;
}
void zmienStanSekcjiAll(uint8_t stan)
{
  if(stanSekcji==stan) return;
   stanSekcji=stan;
   czekaNaPublikacjeStanuMQTT=true;
   czekaNaPublikacjeStanuWS=true;
   czekaNaPublikacjeStanuHW=true;
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
  //web.zmienStanSekcji(stanSekcji);
  czekaNaPublikacjeStanuMQTT=true;
  czekaNaPublikacjeStanuWS=true;
  czekaNaPublikacjeStanuHW=true;
}
void publikujStanSekcjiMQTT()
{
   if(wifi.getConStat()!=CONN_STAT_WIFIMQTT_OK)return;
   
   byte b = pcf8574.read8();
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
    //////////////////////// komendy ktore maja jsona jako msg /////////////////////////
    StaticJsonBuffer<200> jsonBuffer;
    JsonObject& json = jsonBuffer.parseObject(msg);

    if (!json.success()) {
       DPRINTLN("Blad parsowania json !!!!");
      return;
    }
    ind=strstr(topic,"NTP");
    if(ind!=NULL)
    {
      const char* host=json["host"];
      unsigned long offset=json["offset"];
      wifi.setNTP(host,offset);
      // zapisz do pliku
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
        // zapisz do pliku
      }else
      {/// utworzyc AP
        
      }
    }
    ind=strstr(topic,"Mqtt");
    if(ind!=NULL)
    {     
      const char* host=json["host"];
      uint16_t port=json["port"];
      const char* user=json["user"];
      const char* pwd=json["pwd"];
      wifi.setupMqtt(json["host"],json["port"],json["user"],json["pwd"]);
      // zapisz do pliku
    }
    ind=strstr(topic,"LBL");
    if(ind!=NULL)
    {
      
    }
    ind=strstr(topic,"GET");
    if(ind!=NULL)
    {
      
    }
    ind=strstr(topic,"PROG");
    if(ind!=NULL)
    {
      
    }
 }

unsigned long d=0;


void loop()
{
   if(millis()-czasLokalnyMillis>1000)
  {
    czasLokalnyMillis=millis();
    czasLokalny++;
  }
 delay(5);
  wifi.loop();
 delay(5);
 /*
  *  1 styczen 2010 to
  *  Epoch timestamp: 1262304000
  *  Timestamp in milliseconds: 1262304000000
  */
  if(wifi.getEpochTime()<1262304000)// czyli brak polaczenia z NTP
  {
    web.loop(czasLokalny, stanSekcji,"Duchnice",20.1f,1023.34f,0,conf.getTryb());
  }else{
    web.loop(wifi.getEpochTime(), stanSekcji,"Duchnice",20.1f,1023.34f,0,conf.getTryb());
  }
 delay(5);

 
 
   ///// LED status blink
   d=millis()-sLEDmillis;
   if(d>3000)// max 3 sek
   {
     sLEDmillis=millis();
     DPRINT( "[");DPRINT(wifi.getTimeString());DPRINT("] ");DPRINTLN(wifi.getEpochTime());
     if(conf.getTryb()==TRYB_AUTO) // test czy programator każe wlączyć
     {
      uint8_t sekcjaProg=conf.wlaczoneSekcje(wifi.getEpochTime());
      //  Serial.println(sekcjaProg,BIN);
      zmienStanSekcjiAll(sekcjaProg);
     }
   
   }
   /////////////////// obsluga hardware //////////////////////
    if(czekaNaPublikacjeStanuHW)
    {
        pcf8574.write8(stanSekcji);
        czekaNaPublikacjeStanuHW=false;
    }
    /////////// publikowanie ///////////////
    if(czekaNaPublikacjeStanuMQTT)
    {
        publikujStanSekcjiMQTT();  // na podstawie pcf8574
        czekaNaPublikacjeStanuMQTT=false;   
    }
    if(czekaNaPublikacjeStanuWS)
    {
        web.publikujStanSekcji(stanSekcji);
        czekaNaPublikacjeStanuWS=false;     
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

  for(int x=0;x<tBuf.length();x++)
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
  
  bool decPt = false;
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
   int j=0;
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
