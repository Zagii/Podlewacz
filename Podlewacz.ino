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
#include "Defy.h"
#include "CKomora.h"
#include "CWiatrak.h"

const char* nodeMCUid="Reku";
const char* outTopic="Reku/OUT";
const char* inTopic="Reku/IN";
const char* debugTopic="DebugTopic/Reku";
const char* mqtt_server ="broker.hivemq.com"; //"m23.cloudmqtt.com";
const char* mqtt_user="";//"aigejtoh";
const char* mqtt_pass="";//"ZFlzjMm4T-XH";
const uint16_t mqtt_port=1883;


#define CONN_STAT_NO 0
#define CONN_STAT_WIFI_CONNECTING 1
#define CONN_STAT_WIFI_OK 2
#define CONN_STAT_WIFIMQTT_CONNECTING 3
#define CONN_STAT_WIFIMQTT_OK 4

int conStat=CONN_STAT_NO;
unsigned long sLEDmillis=0;

char trybPracy=T_OFF;
char trybPracyPop=T_OFF;
unsigned long kominekMillis=0;
uint8_t publicID=0;
unsigned long publicMillis=0;
unsigned long WDmillis=0;

void(* resetFunc) (void) = 0; //declare reset function @ address 0

CKomora komory[KOMORA_SZT];
CWiatrak wiatraki[WIATRAKI_SZT]=
{
  CWiatrak(PIN_WIATRAK_CZERPNIA,PIN_TACHO_WIATRAK_CZERPNIA),
  CWiatrak(PIN_WIATRAK_WYWIEW,PIN_TACHO_WIATRAK_WYWIEW)
};

#define MAX_ROZKAZOW 10
uint8_t ile_w_kolejce=0;
uint16_t kolejkaRozkazow[MAX_ROZKAZOW][2];


ESP8266WiFiMulti wifiMulti;
WiFiClient espClient;
PubSubClient client(espClient);


unsigned long lastMQTTReconnectAttempt = 0;
unsigned long lastWIFIReconnectAttempt = 0;

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
 parsujIdodajDoKolejki(topic,p);
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

//WiFiConnected
//true - connected
//false -not connected
bool WiFiConnected() 
{
  return (WiFi.status() == WL_CONNECTED);
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

void isrIN()
{
  wiatraki[WIATRAK_IN].obslugaTachoISR();
}
void isrOUT()
{
  wiatraki[WIATRAK_OUT].obslugaTachoISR();
}
/////////////////////////SETUP///////////////////////////
void setup()
{
 
  Serial.begin(115200);
   
  DPRINTLN("");
  DPRINTLN("Setup Serial");
  pinMode(LED, OUTPUT);     // Initialize the BUILTIN_LED pin as an output
  digitalWrite(LED,ON);
  for(uint8_t i=0;i<KOMORA_SZT;i++)
   {
    komory[i]=CKomora();
     komory[i].begin(i);
     delay(1000); //po to by kazda komora mierzyla w innym momencie
   }
   wiatraki[WIATRAK_IN].begin();
   wiatraki[WIATRAK_OUT].begin();
  
   attachInterrupt(digitalPinToInterrupt( wiatraki[WIATRAK_IN].dajISR()), isrIN, RISING );
   attachInterrupt(digitalPinToInterrupt( wiatraki[WIATRAK_OUT].dajISR()), isrOUT, RISING );
  

  wifiMulti.addAP("DOrangeFreeDom", "KZagaw01_ruter_key");
  wifiMulti.addAP("open.t-mobile.pl", "");
  wifiMulti.addAP("instalujWirusa", "blablabla123");
  
  client.setServer(mqtt_server, mqtt_port);
  client.setCallback(callback);
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

void loop()
{

  if(!WiFiConnected())
  { 
    conStat=CONN_STAT_WIFI_CONNECTING;
    if (millis() - lastWIFIReconnectAttempt > 5000)
    {
      if(setup_wifi())
      {
        RSpisz(debugTopic,"WiFi=ok");
        conStat=CONN_STAT_WIFI_OK;
      }
      else
      {
        RSpisz(debugTopic,"WiFi=Err");
      
      }
      lastWIFIReconnectAttempt = millis();
    }
  }else
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
  }
   
     
          ///// LED status blink
          unsigned long d=millis()-sLEDmillis;
          if(millis()%600000==0) //10 min
          {
            char m[MAX_MSG_LENGHT];
            sprintf(m,"%ld",millis());
            char m2[MAX_TOPIC_LENGHT];
            sprintf(m2,"%s/watchdog",inTopic);
            RSpisz(m2,m);
          }
          if(d>3000)// max 3 sek
          {
           
           sLEDmillis=millis();
         
 //char mst[50];
  //   sprintf(mst,"nodeMCU millis od restartu %lu ms.",sLEDmillis);
    // serial.printRS(RS_DEBUG_INFO,"Z nodeMCU",mst);
   //  Serial.println(mst);
          }
           
          if(millis()%600000==0)//10 min
          {
            unsigned long mmm=millis();
   
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
            for(uint8_t i=0;i<KOMORA_SZT;i++)
            {
              komory[i].loop();
            }
            wiatraki[WIATRAK_IN].loop();
            wiatraki[WIATRAK_OUT].loop();
          ///////////////     obsługa kolejki     //////////////////////
          
          if(ile_w_kolejce>0)
            {
              
                DPRINT("Jest w kolejce ");  DPRINT(ile_w_kolejce); DPRINT(" ");
                DPRINT(kolejkaRozkazow[0][0]); DPRINT(", "); DPRINTLN(kolejkaRozkazow[0][1]);
                realizujRozkaz(kolejkaRozkazow[0][0],kolejkaRozkazow[0][1]);
              //usuniecie z kolejki pierwszego elementu
              for(int i=0;i<ile_w_kolejce-1;i++)
              {
               kolejkaRozkazow[i][0]=kolejkaRozkazow[i+1][0];
               kolejkaRozkazow[i][1]=kolejkaRozkazow[i+1][1];
                
              }
              ile_w_kolejce--;
            }
          ///////////////////// wyznacz obroty wiatraka ////////////
            // gdy manual to tylko sprawdz czy nie zamarza
            //jesli temp wyrzutni < 2C ustaw rozmrazanie
            if((trybPracy!=T_OFF)&&(komory[KOMORA_WYRZUTNIA].dajTemp()<1.0f))
            {
               setTrybPracy(T_ROZMRAZANIE_WIATRAKI);
            }
            switch(trybPracy)
            {
              case T_OFF:
                   wiatraki[WIATRAK_IN].ustawPredkosc(0);
                   wiatraki[WIATRAK_OUT].ustawPredkosc(0);
                 break;
              case T_MANUAL:
                break;
              case T_AUTO:
                //pora roku czy chlodzic czy ziebic
                //temp zewn? czy z temp wewn da sie oszacować ile os
                //liczba osob = czy pusto/domownicy/impreza
                //pora dnia
                  wiatraki[WIATRAK_IN].ustawPredkosc(15);
                  wiatraki[WIATRAK_OUT].ustawPredkosc(15);
                break;
              case T_KOMINEK:
              //todo czas 5min?
                if(millis()-kominekMillis>300000)//5min
                {
                    setTrybPracy(trybPracyPop);
                }
                  wiatraki[WIATRAK_IN].ustawPredkosc(50);
                  wiatraki[WIATRAK_OUT].ustawPredkosc(15);
                
                break;
              case T_ROZMRAZANIE_WIATRAKI:
                if(komory[KOMORA_WYRZUTNIA].dajTemp()>3.0f)
                {
                  setTrybPracy(T_AUTO);
                } 
                wiatraki[WIATRAK_IN].ustawPredkosc(15);
                wiatraki[WIATRAK_OUT].ustawPredkosc(30);
                break;
              case T_ROZMRAZANIE_GGWC:
                break;
            
            }
            
            // gdy auto to ???
          /////////////////// publikuj stan do mqtt ////////////////
           if(millis()-publicMillis>1000)
           { 
            
            publicMillis=millis();
            publicID++;
           if(publicID>=KOMORA_SZT+WIATRAKI_SZT)
           {
            publicID=0;
           }
            char tmpTopic[MAX_TOPIC_LENGHT];
            char tmpMsg[MAX_TOPIC_LENGHT];
            
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

void setTrybPracy(char t)
{
  trybPracyPop=trybPracy;
  trybPracy=t;
  if(trybPracy==T_KOMINEK)
  {
    kominekMillis=millis();
  }
  
  char topic[MAX_TOPIC_LENGHT];
  strcpy(topic,outTopic);
  strcat(topic,"/TrybPracy");
  char msg[2];//[MAX_MSG_LENGHT];
  msg[0]=t;msg[1]='\0';
  RSpisz(topic,msg);
}

void dodajDoKolejki(uint16_t paramName,uint16_t paramValue)
{
  DPRINT("Dodaj do kolejki"); DPRINT(paramName);DPRINT(", ");DPRINTLN(paramValue);
  
  if(ile_w_kolejce>=MAX_ROZKAZOW) return;
  kolejkaRozkazow[ile_w_kolejce][0]=paramName;
  kolejkaRozkazow[ile_w_kolejce][1]=paramValue;
  ile_w_kolejce++;
}
void parsujIdodajDoKolejki(char* topic,char * msg)
{
   if(strstr(topic,"WiatrakN")>0)
   {
     if(isIntChars(msg))
     {
        dodajDoKolejki(R_PWM_NAWIEW,atoi(msg));       
     }else
     {
         DPRINT("ERR msg WiatrakN nie int, linia:");DPRINTLN(__LINE__);
     }
     return;
    }
    if(strstr(topic,"WiatrakW")>0)
    {
      if(isIntChars(msg))
      {
        dodajDoKolejki(R_PWM_WYWIEW,atoi(msg));
      }else
      {
        DPRINT("ERR msg WiatrakW nie int, linia:");DPRINTLN(__LINE__);
      }
     return; 
    }
    if(strlen(topic)==strlen(inTopic)+2)  //Reku/X
    {
       if(isIntChars(msg))
      {
        dodajDoKolejki(topic[strlen(topic)-1],atoi(msg));
      }else
      {
        DPRINT("ERR msg ");DPRINT(msg);DPRINT(" nie int, linia:");DPRINTLN(__LINE__);
      }
      
     return; 
    }
}
void realizujRozkaz(uint16_t paramName,uint16_t paramValue) 
{
  switch(paramName)
  {
    case R_PWM_NAWIEW:
      setTrybPracy(T_MANUAL);
      wiatraki[WIATRAK_IN].ustawPredkosc(paramValue);
    break;
    case R_PWM_WYWIEW:
      setTrybPracy(T_MANUAL);
      wiatraki[WIATRAK_OUT].ustawPredkosc(paramValue);
    break;
    case R_ROZMRAZANIE_WIATRAKI:
      setTrybPracy(T_ROZMRAZANIE_WIATRAKI) ;
    break;
    case R_ROZMRAZANIE_GGWC:
      setTrybPracy(T_ROZMRAZANIE_GGWC);
    break;
    case R_KOMINEK:
      setTrybPracy(T_KOMINEK);   
    break;
    case R_AUTO:
      setTrybPracy(T_AUTO);
    break;
    case R_OFF:
      setTrybPracy(T_OFF);
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
