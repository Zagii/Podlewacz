#ifndef CWIFI_h
#define CWIFI_h

#include <ESP8266WiFi.h>
#include <ESP8266WiFiMulti.h>
#include <PubSubClient.h>
#include <NTPClient.h>
#include <WiFiUdp.h>

//#include "Defy.h"


#define DEBUG   //If you comment this line, the DPRINT & DPRINTLN lines are defined as blank.
#ifdef DEBUG    //Macros are usually in all capital letters.
  #define DPRINT(...)    Serial.print(__VA_ARGS__)     //DPRINT is a macro, debug print
  #define DPRINTLN(...)  Serial.println(__VA_ARGS__)   //DPRINTLN is a macro, debug print with new line
#else
  #define DPRINT(...)     //now defines a blank line
  #define DPRINTLN(...)   //now defines a blank line
#endif

#define CONN_STAT_NO 0
#define CONN_STAT_WIFI_CONNECTING 1
#define CONN_STAT_WIFI_OK 2
#define CONN_STAT_WIFIMQTT_CONNECTING 3
#define CONN_STAT_WIFIMQTT_OK 4

#define MAX_TOPIC_LENGHT 50
#define MAX_MSG_LENGHT 20



class CWifi
{

//// wifi udp  ntp
WiFiUDP ntpUDP;
// By default 'time.nist.gov' is used with 60 seconds update interval and
// no offset
NTPClient *timeClient;
// You can specify the time server pool and the offset, (in seconds)
// additionaly you can specify the update interval (in milliseconds).
// NTPClient timeClient(ntpUDP, "europe.pool.ntp.org", 3600, 60000);

//////////////////////////////
const char* nodeMCUid="Podlewacz";
const char* outTopic="Podlewacz/OUT";
const char* inTopic="Podlewacz/IN";
const char* debugTopic="DebugTopic/Podlewacz";
const char* mqtt_server ="broker.hivemq.com"; //"m23.cloudmqtt.com";
const char* mqtt_user="";//"aigejtoh";
const char* mqtt_pass="";//"ZFlzjMm4T-XH";
const uint16_t mqtt_port=1883;
  
  int conStat=CONN_STAT_NO;
  unsigned long lastMQTTReconnectAttempt = 0;
  unsigned long lastWIFIReconnectAttempt = 0;
  
  unsigned long loopMillis=0;
  unsigned long WDmillis=0;

  ESP8266WiFiMulti wifiMulti;
  WiFiClient espClient;
  PubSubClient client;
  
  public:
  CWifi(){};
  void begin();
  void loop();
  PubSubClient *getMQTTClient(){return &client;};
  bool wifiConnected();
  bool getWifiStatusString(char *b);
  bool reconnectMQTT();
  void RSpisz(const char* topic,char* msg);
  char * TimeToString(unsigned long t);
  void setWDmillis(unsigned long m){WDmillis=m;};
  unsigned long getWDmillis(){return WDmillis;};
  int getConStat(){return conStat;};
  const char * getOutTopic(){return outTopic;};
  const char * getInTopic(){return inTopic;};
 String getTimeString(){return timeClient->getFormattedTime();};
// uint8_t getTimeDay(){return timeClient->getFormattedTime();};
 void wifiScanNetworks();
  void wifiReconnect();
  
};

#endif
