#ifndef CWIFI_h
#define CWIFI_h

#include <ESP8266WiFi.h>
#include <ESP8266WiFiMulti.h>
#include <PubSubClient.h>
#include <NTPClient.h>
#include <WiFiUdp.h>

//#include "Defy.h"

/*
typedef enum {
    WL_NO_SHIELD        = 255,   // for compatibility with WiFi Shield library
    WL_IDLE_STATUS      = 0,
    WL_NO_SSID_AVAIL    = 1,
    WL_SCAN_COMPLETED   = 2,
    WL_CONNECTED        = 3,
    WL_CONNECT_FAILED   = 4,
    WL_CONNECTION_LOST  = 5,
    WL_DISCONNECTED     = 6
} wl_status_t;

*/


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

char ntp_server[50];
unsigned long ntp_offset;

char mqtt_server[50];// ="broker.hivemq.com"; //"m23.cloudmqtt.com";
char mqtt_user[20];//"aigejtoh";
char mqtt_pass[20];//"ZFlzjMm4T-XH";
uint16_t mqtt_port=1883;

char wifi_ssid[50];
char wifi_pwd[30];
char wifi_ip[20];
char wifi_tryb[5];
  
  int conStat=CONN_STAT_NO;
  unsigned long lastMQTTReconnectAttempt = 0;
  unsigned long lastWIFIReconnectAttempt = 0;
  
  unsigned long loopMillis=0;
  unsigned long WDmillis=0;

  ESP8266WiFiMulti *wifiMulti;
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
 unsigned long getEpochTime(){return timeClient->getEpochTime();};
 void wifiScanNetworks();
  void wifiReconnect();
  void setNTP(const char* host,unsigned long offset);
  String getNTPjsonStr(){ return  String("{\"host\":\""+String(ntp_server)+"\",\"offset\":"+String(ntp_offset)+"}");};
  String getMQTTjsonStr(){ return String("{\"host\":\""+String(mqtt_server)+"\",\"port\":"+String(mqtt_port)+",\"user\":\""+String(mqtt_user)+"\",\"pwd\":\""+String(mqtt_pass)+"\"}");};
  String getWifijsonStr(){ return String("{\"ssid\":\""+String(wifi_ssid)+"\",\"pwd\":"+String(wifi_pwd)+",\"ip\":\""+String(wifi_ip)+"\",\"tryb\":\""+String(wifi_tryb)+"\"}");};
  void zmianaAP(const char* ssid,const char* pwd);
  void setupMqtt(const char* host, uint16_t port, const char * usr, const char* pwd);
};

#endif
