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



unsigned long sLEDmillis=0;

byte stanSekcji=0;
bool czekaNaPublikacjeStanu=false;

uint8_t publicID=0;
unsigned long publicMillis=0;


void(* resetFunc) (void) = 0; //declare reset function @ address 0






void callback(char* topic, byte* payload, unsigned int length) 
{
  char* p = (char*)malloc(length);
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

//delay(1000);
/*DPRINTLN("Programy");
Program pp;
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
conf.setProg(pp,1, 1, 1970, 15, 43,50,3600,1,7);
conf.addProg(pp);
conf.setProg(pp,1, 1, 1970, 15, 55,50,15*60,1,5);
conf.addProg(pp);
conf.publishAllProg();
*/
//conf.saveConfig();
conf.publishAllProg();
web.begin();
WebSocketsServer * webSocket=web.getWebSocket();
webSocket->onEvent(wse);
}

void wylaczWszystko()
{
  stanSekcji=0;
  czekaNaPublikacjeStanu=true;
}
void zmienStanSekcji(uint8_t stan)
{
  if(stanSekcji==stan) return;
   stanSekcji=stan;
   czekaNaPublikacjeStanu=true;
}
void zmienStanSekcji(uint8_t sekcjanr,uint8_t stan)
{
  if((stanSekcji & (1<<sekcjanr))==stan) return;
   
  if(stan==ON)
  {
    stanSekcji |= 1<<sekcjanr;
  }else
  {
    stanSekcji &= ~(1<<sekcjanr);
  }
  czekaNaPublikacjeStanu=true;
  
}
void publikujStanSekcji()
{
   if(wifi.getConStat()!=CONN_STAT_WIFIMQTT_OK)return;
   
   byte b = pcf8574.read8();
   for(int i=SEKCJA_MIN;i<=SEKCJA_MAX;i++)
   {
      if(b&(1<<i))
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
  czekaNaPublikacjeStanu=false;
}

 void parsujRozkaz(char *topic, char *msg)
 {
   char *ind=NULL;
   ///////////////// SEKCJA  //////////////////////////
   ind=strstr(topic,"SEKCJA");
   if(ind!=NULL)
   {
     ind+=strlen("SEKCJA");

     if(isIntChars(ind))
     {
        if(isIntChars(msg))
        {
          if(msg[0]=='0')
          {
            zmienStanSekcji(atoi(ind),OFF);
          }else
          {
            zmienStanSekcji(atoi(ind),ON);
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

 }

unsigned long d=0;


void loop()
{
  wifi.loop();
  web.loop();
   ///// LED status blink
   d=millis()-sLEDmillis;
   if(d>3000)// max 3 sek
   {
     sLEDmillis=millis();
     //DPRINT( "[");DPRINT(wifi.getTimeString());DPRINT("] ");DPRINTLN(wifi.getEpochTime());
     uint8_t sekcjaProg=conf.wlaczoneSekcje(wifi.getEpochTime());
     Serial.println(sekcjaProg,BIN);
     zmienStanSekcji(sekcjaProg);
   
   }
   /////////////////// obsluga hardware //////////////////////
  if(czekaNaPublikacjeStanu)
  {
    DPRINT("######### ZMIANA STANU WYJSC ############ ");DPRINTLN(stanSekcji,BIN);
      pcf8574.write8(stanSekcji);
      publikujStanSekcji();
         
  }
   /////////////////// obsluga hardware //////////////////////
   
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
