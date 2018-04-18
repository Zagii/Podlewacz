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

#include <pcf8574_esp.h>
#include <ArduinoJson.h>
#include <Time.h>
#include <TimeLib.h>
#include "Defy.h"
#include "CWifi.h"
#include "Config.h"




////////////pcf
PCF857x pcf8574(0b00111000, &Wire);
////////////////

CWifi wifi;
PubSubClient *mqtt;

CConfig conf;

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

delay(1000);
DPRINTLN("Programy");
Program pp;
conf.setProg(pp,16, 4, 18, 17, 23,0,600,11,1);
conf.addProg(pp);
conf.setProg(pp,16, 5, 18, 17, 33,10,500,22,2);
conf.addProg(pp);
conf.setProg(pp,16, 6, 18, 17, 43,20,400,33,3);
conf.addProg(pp);
conf.setProg(pp,16, 7, 18, 17, 53,30,300,44,4);
conf.addProg(pp);

conf.publishAllProg();

conf.delProg(2);
conf.setProg(pp,16, 8, 18, 17, 63,10,410,55,5);
conf.addProg(pp);
conf.publishAllProg();
}

void wylaczWszystko()
{
  stanSekcji=0;
  czekaNaPublikacjeStanu=true;
}
void zmienStanSekcji(uint8_t sekcjanr,uint8_t stan)
{
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
   ///// LED status blink
   d=millis()-sLEDmillis;
   if(d>3000)// max 3 sek
   {
     sLEDmillis=millis();
     DPRINT( "[");DPRINT(wifi.getTimeString());DPRINTLN("]");
   
   }
   /////////////////// obsluga hardware //////////////////////
  if(czekaNaPublikacjeStanu)
  {
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
