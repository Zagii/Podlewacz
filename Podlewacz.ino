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
//#include <NTPClient.h>
#include <WiFiUdp.h>

#include "Defy.h"

//// wifi udp  ntp
WiFiUDP ntpUDP;
// By default 'time.nist.gov' is used with 60 seconds update interval and
// no offset
NTPClient timeClient(ntpUDP);
// You can specify the time server pool and the offset, (in seconds)
// additionaly you can specify the update interval (in milliseconds).
// NTPClient timeClient(ntpUDP, "europe.pool.ntp.org", 3600, 60000);

//////////////////////////////

////////////pcf
PCF857x pcf8574(0b00111000, &Wire);
////////////////

const char* nodeMCUid="Podlewacz";
const char* outTopic="Podlewacz/OUT";
const char* inTopic="Podlewacz/IN";
const char* debugTopic="DebugTopic/Podlewacz";
const char* mqtt_server ="broker.hivemq.com"; //"m23.cloudmqtt.com";
const char* mqtt_user="";//"aigejtoh";
const char* mqtt_pass="";//"ZFlzjMm4T-XH";
const uint16_t mqtt_port=1883;


////////////// sprawdzic ntp
////////// https://github.com/arduino-libraries/NTPClient

#define CONN_STAT_NO 0
#define CONN_STAT_WIFI_CONNECTING 1
#define CONN_STAT_WIFI_OK 2
#define CONN_STAT_WIFIMQTT_CONNECTING 3
#define CONN_STAT_WIFIMQTT_OK 4

int conStat=CONN_STAT_NO;
unsigned long sLEDmillis=0;

byte stanSekcji=0;
bool czekaNaPublikacjeStanu=false;

uint8_t publicID=0;
unsigned long publicMillis=0;
unsigned long WDmillis=0;

void(* resetFunc) (void) = 0; //declare reset function @ address 0

// Event Handler when an IP address has been assigned
// Once connected to WiFi, start the NTP Client
/*void onSTAGotIP(WiFiEventStationModeGotIP event) {
  Serial.printf("Got IP: %s\n", event.ip.toString().c_str());
  NTP.init((char *)"pool.ntp.org", UTC0900);
  NTP.setPollingInterval(60); // Poll every minute
}*/
// Event Handler when WiFi is disconnected
/*void onSTADisconnected(WiFiEventStationModeDisconnected event) {
  Serial.printf("WiFi connection (%s) dropped.\n", event.ssid.c_str());
  Serial.printf("Reason: %d\n", event.reason);
}*/

ESP8266WiFiMulti wifiMulti;
WiFiClient espClient;
PubSubClient client(espClient);


unsigned long lastMQTTReconnectAttempt = 0;
unsigned long lastWIFIReconnectAttempt = 0;



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
      WDmillis=strtoul (p, NULL, 0);
    DPRINTLN(WDmillis);
    

  }

 DPRINT("Debug: callback topic=");
 DPRINT(topic);
 DPRINT(" msg=");
 DPRINTLN(p);
 parsujRozkaz(topic,p);
  free(p);
}


void RSpisz(const char* topic,char* msg)
{
   DPRINT("Debug RSpisz, topic=");  DPRINT(topic); DPRINT(", msg=");  DPRINT(msg);
   DPRINT(", wynik=");
   if(conStat==CONN_STAT_WIFIMQTT_OK)
   {
	DPRINTLN(client.publish(topic,msg));
   }else
   {
	   DPRINTLN("nie wysylam, brak polaczenia");
   }
}




bool setup_wifi() 
{ 
  RSpisz(debugTopic,"Restart WiFi ");
  WiFi.mode(WIFI_STA);
  if(wifiMulti.run() == WL_CONNECTED)
  {
    IPAddress ip=WiFi.localIP();
    char b[100];
    sprintf(b,"WiFi connected: %s ,%d.%d.%d.%d\n", WiFi.SSID().c_str(), ip[0],ip[1],ip[2],ip[3]);
    RSpisz(debugTopic,b);
    
    return true;
  }else
  {
    RSpisz(debugTopic,"Wifi Connection Error."); 
    return false;
  }
}


boolean reconnectMQTT()
{
  if (client.connect(nodeMCUid,mqtt_user,mqtt_pass)) 
  {
    char s[MAX_TOPIC_LENGHT];
    strcpy(s,inTopic);
    strcat(s,"\/#");  
    client.subscribe(s);
    const char *t="reconnectMQTT, subskrybcja do: ";
    char b[MAX_TOPIC_LENGHT+strlen(t)];
    sprintf(b,"%s%s",t,s);
    RSpisz(debugTopic,b);
   
  }
  return client.connected();
}


/////////////////////////SETUP///////////////////////////
void setup()
{
 //static WiFiEventHandler gotIpEventHandler, disconnectedEventHandler;
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

  wifiMulti.addAP("DOrangeFreeDom", "KZagaw01_ruter_key");
  wifiMulti.addAP("open.t-mobile.pl", "");
  wifiMulti.addAP("InstalujWirusa", "BlaBlaBla123");
  
  client.setServer(mqtt_server, mqtt_port);
  client.setCallback(callback);
  delay(2000);
  setup_wifi();
  delay(2000);
  timeClient.begin();
  /*NTP.onSyncEvent([](NTPSyncEvent_t ntpEvent) {
    switch (ntpEvent) {
    case NTP_EVENT_INIT:
      break;
    case NTP_EVENT_STOP:
      break;
    case NTP_EVENT_NO_RESPONSE:
      Serial.printf("NTP server not reachable.\n");
      break;
    case NTP_EVENT_SYNCHRONIZED:
      Serial.printf("Got NTP time: %s\n", NTP.getTimeDate(NTP.getLastSync()));
      break;
    }
  });

  gotIpEventHandler = WiFi.onStationModeGotIP(onSTAGotIP);
  disconnectedEventHandler = WiFi.onStationModeDisconnected(onSTADisconnected);
  */
}




// t is time in seconds = millis()/1000;
char * TimeToString(unsigned long t)
{
 static char str[12];
 long d =t/(3600*24);
 t=t%(3600*24);
 long h = t / 3600;
 t = t % 3600;
 int m = t / 60;
 int s = t % 60;
 sprintf(str, "%03ldd%02ldh%02dm",d, h, m);
 return str;
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
   if(conStat!=CONN_STAT_WIFIMQTT_OK)return;
   
   byte b = pcf8574.read8();
   for(int i=SEKCJA_MIN;i<=SEKCJA_MAX;i++)
   {
      if(b&(1<<i))
      {
          sprintf(tmpTopic,"%s/SEKCJA%d/%d",outTopic,i,1);
      }else
      {
          sprintf(tmpTopic,"%s/SEKCJA%d/%d",outTopic,i,1);
      }
      RSpisz(tmpTopic,tmpMsg);
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
unsigned long mmm=0;
char tmpTopic[MAX_TOPIC_LENGHT];
char tmpMsg[MAX_TOPIC_LENGHT];
void loop()
{

     if (!client.connected()) 
    {
     conStat=CONN_STAT_WIFIMQTT_CONNECTING;
      if (millis() - lastMQTTReconnectAttempt > 5000)
      {
        lastMQTTReconnectAttempt = millis();
        if (reconnectMQTT())
        {
           RSpisz(debugTopic,"MQTT=ok");
           conStat=CONN_STAT_WIFIMQTT_OK;
          lastMQTTReconnectAttempt = 0;
         char t[100];
         sprintf(t,"Polaczono SSID=%s, IP=%d.%d.%d.%d",WiFi.SSID().c_str(),WiFi.localIP()[0],WiFi.localIP()[1],WiFi.localIP()[2],WiFi.localIP()[3]);
         RSpisz(debugTopic,t);
        }
        else
        {
             RSpisz(debugTopic,"MQTT=Err");
             DPRINT("Err MQTTstat= ");DPRINTLN(client.state());
             DPRINT("WIFI ip= ");DPRINTLN(WiFi.localIP());
        }
      }
    } else
    {
          client.loop();                
    }
    

    if(millis()%600000==0) //10 min
     {
            char m[MAX_MSG_LENGHT];
            sprintf(m,"%ld",millis());
            char m2[MAX_TOPIC_LENGHT];
            sprintf(m2,"%s/watchdog",inTopic);
            RSpisz(m2,m);
   }
   timeClient.update();

  
   ///// LED status blink
   d=millis()-sLEDmillis;
   if(d>3000)// max 3 sek
   {
     Serial.println(timeClient.getFormattedTime());
      /*    Serial.printf("Current time: %s - First synchronized at: %s.\n",
                  NTP.getTimeDate(now()), NTP.getTimeDate(NTP.getFirstSync()));  
           sLEDmillis=millis();
        */ 
 //char mst[50];
  //   sprintf(mst,"nodeMCU millis od restartu %lu ms.",sLEDmillis);
    // serial.printRS(RS_DEBUG_INFO,"Z nodeMCU",mst);
   //  Serial.println(mst);
   }
  if(millis()%600000==0)//10 min
  {
           mmm=millis();
   
            String str="czas od restartu= "+(String) TimeToString(mmm/1000);
   
            DPRINTLN(str);
            DPRINT("Watchdog czas ");
            DPRINTLN(mmm-WDmillis);
            if(mmm-WDmillis>600000)
            {
             
              RSpisz(debugTopic,"Watchdog restart");
              delay(3000);
              //ESP.restart();
			         ESP.reset();  // hard reset
			         resetFunc();
            }
    }

          /////////////////// obsluga hardware //////////////////////
        if(czekaNaPublikacjeStanu)
        {
          pcf8574.write8(stanSekcji);
          publikujStanSekcji();
         
        }
          ///////////////    ////////////////     //////////////////////
   
          /////////////////// publikuj stan do mqtt ////////////////
           if(millis()-publicMillis>1000)
           { 
            
            publicMillis=millis();
            publicID++;
            if(publicID>=KOMORA_SZT+WIATRAKI_SZT)
            {
              publicID=0;
            }

            
            if(publicID<KOMORA_SZT)  //publikacja stanu komor
            {
              
              sprintf(tmpTopic,"%s/KOM%d/Term%s",outTopic,publicID,komory[publicID].getTempAddress());
              dtostrf(komory[publicID].dajTemp(), 5, 2, tmpMsg);
              RSpisz(tmpTopic,tmpMsg);
            }else //publikacja wiatrakow
            {
              sprintf(tmpTopic,"%s/Wiatrak%d",outTopic,publicID-KOMORA_SZT);
              sprintf(tmpMsg,"%lu",wiatraki[publicID-KOMORA_SZT].dajOstPredkosc());
              RSpisz(tmpTopic,tmpMsg);
            }
           }
          ///////////////////// status LED /////////////////////////
            switch(conStat)
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