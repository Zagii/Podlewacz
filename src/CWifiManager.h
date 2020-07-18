#ifndef CWIFI_h
#define CWIFI_h

#include <ArduinoJson.h>
#include <ESP8266WiFi.h>
//#include <ESP8266WiFiMulti.h>
#include <WiFiManager.h>
#include <PubSubClient.h>
#include <NTPClient.h>
#include <WiFiUdp.h>
//#include <ArduinoTrace.h>
#include "Defy.h"

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
#define MAX_URL_LENGTH 30



class CWifiManager
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

char ntp_server[MAX_URL_LENGTH];
unsigned long ntp_offset;

char mqtt_server[MAX_URL_LENGTH];// ="broker.hivemq.com"; //"m23.cloudmqtt.com";
char mqtt_user[MAX_URL_LENGTH];//"aigejtoh";
char mqtt_pass[MAX_URL_LENGTH];//"ZFlzjMm4T-XH";
uint16_t mqtt_port=1883;

char wifi_ssid[MAX_URL_LENGTH];
char wifi_pwd[MAX_URL_LENGTH];
char wifi_ip[MAX_URL_LENGTH];
char wifi_tryb[5];
char buforChar[3*MAX_URL_LENGTH];
  
  int conStat=CONN_STAT_NO;
  unsigned long lastMQTTReconnectAttempt = 0;
  unsigned long lastWIFIReconnectAttempt = 0;
  
  unsigned long loopMillis=0;
  unsigned long WDmillis=0;

 // ESP8266WiFiMulti *wifiMulti;
  WiFiManager wifiManager;
  WiFiClient espClient;
  PubSubClient client;
  
  public:
  CWifiManager(){
    strcpy(mqtt_server,"broker.hivemq.com");
    strcpy(mqtt_user,"");
    strcpy(mqtt_pass,"");
    strcpy(wifi_ssid,"");
    strcpy(wifi_pwd,"");
    strcpy(wifi_ip,"");
  };
  void begin();
  void loop();
  PubSubClient *getMQTTClient(){return &client;};
  bool wifiConnected();
  bool getWifiStatusString(char *b);
  bool reconnectMQTT();
  void RSpisz(const char* topic,const char* msg,bool cisza=false);
  void RSpisz(String topic,String msg,bool cisza=false);
  char * TimeToString(char* str, unsigned long t);
  void setWDmillis(unsigned long m){WDmillis=m;};
  unsigned long getWDmillis(){return WDmillis;};
  int getConStat(){return conStat;};
  const char * getOutTopic(){return outTopic;};
  const char * getInTopic(){return inTopic;};
 char* getTimeString(char* b){ strcpy(b,timeClient->getFormattedTime().c_str()); return b;};
 unsigned long getEpochTime(){return timeClient->getEpochTime();};
 void wifiScanNetworks();
  void wifiReconnect();
  int setNTP(const char* host,unsigned long offset);
  char* getNTPjsonStr(char* b){ 
    sprintf(b,"{\"host\":\"%s\",\"offset\":%lu}",ntp_server,ntp_offset);
   // strcpy(b,"{\"host\":\"");strcat(b,ntp_server);strcat(b,"\",\"offset\":");
    //strcat(b,ntp_offset);strcat(b,"}");
    return  b;};
  char* getMQTTjsonStr(char* b){ 
    sprintf(b,"{\"host\":\"%s\",\"port\":%d,\"user\":\"%s\",\"pwd\":\"%s\"}",
          mqtt_server,mqtt_port,mqtt_user,mqtt_pass );
    return b;
   // return String("{\"host\":\""+mqtt_server+"\",\"port\":"+String(mqtt_port)+",\"user\":\""+mqtt_user+"\",\"pwd\":\""+mqtt_pass+"\"}");
   };
  char* getWifijsonStr(char* b){ 
    sprintf(b,"{\"ssid\":\"%s\",\"pwd\":\"%s\",\"ip\":\"%s\",\"tryb\":\"%s\"}",
        wifi_ssid,wifi_pwd,wifi_ip,wifi_tryb);
    return b;
   // return String("{\"ssid\":\""+wifi_ssid+"\",\"pwd\":\""+wifi_pwd+"\",\"ip\":\""+wifi_ip+"\",\"tryb\":\""+wifi_tryb+"\"}");
    };
  int zmianaAP(const char* ssid,const char* pwd);
  int zmianaAP(const char* jsonString);
  int setupMqtt(const char* mqttJsonStr);
  int setupMqtt(const char* host, uint16_t port, const char* usr,const char* pwd);
};

#endif
