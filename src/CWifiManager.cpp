#include "CWifiManager.h"

void CWifiManager::wifiReconnect()
{

  //return;
  DPRINTLN("  Debug CWifiManager::wifiReconnect start"); 
//  WiFi.disconnect();
  delay(1000);
 // WiFi.begin("DOrangeFreeDom", "KZagaw01_ruter_key");
 // delay(1000);
  DPRINTLN("  Debug CWifiManager::wifiReconnect end"); 
}
int CWifiManager::setNTP(const char* host,unsigned long offset)
{
  if(strlen(host)>=MAX_URL_LENGTH)return -1;
  strcpy(ntp_server,host);
  if(timeClient)
  {
    timeClient->end();
    delete timeClient;
  }
  ntp_offset=offset;
  timeClient=new NTPClient(ntpUDP, ntp_server, offset*3600, 60000);
  timeClient->begin();
  return 0;
}

int CWifiManager::zmianaAP(const char* jsonString)
{
  DynamicJsonDocument doc(2048);
  DeserializationError error= deserializeJson(doc,jsonString);

  if (error) 
    {
      Serial.print(F("Blad parsowania zmianyAP "));
      Serial.println(jsonString);
      Serial.print(doc.memoryUsage());Serial.print(F(" bytes. "));
      Serial.println(error.c_str());
      DPRINTLN("return");
      return -11;
    }
  JsonObject js = doc.as<JsonObject>();
  if(js.containsKey("ssid"))
  {
    const char* s=js["ssid"];
    if(js.containsKey("pwd"))
    {
        const char *p=js["pwd"];
      return zmianaAP(s,p);
    }
    return zmianaAP(s,"");  
  }
  return -12;
}
int CWifiManager::zmianaAP(const char* ssid,const char* pwd)
{
  if(strlen(ssid)>=MAX_URL_LENGTH)return -1;
  if(strlen(pwd)>=MAX_URL_LENGTH)return -2;
  strcpy(wifi_ssid,ssid);
  strcpy(wifi_pwd,pwd);
  //if(wifiMulti) delete wifiMulti;
  //wifiMulti=new ESP8266WiFiMulti();
  //wifiMulti->addAP(wifi_ssid,wifi_pwd);
  delay(10);
  return 0;
}

void CWifiManager::begin()
{
  DPRINTLN("Debug CWifiManager::begin start"); 
 

 resetWifiConfig = false;
 if (drd.detectDoubleReset()) {
    Serial.println("Double Reset Detected");
    digitalWrite(LED, LOW);   // Turn the LED on (Note that LOW is the voltage level
    resetWifiConfig = true;
    drd.stop();
  } else {
    Serial.println("No Double Reset Detected");
    //digitalWrite(LED_BUILTIN, HIGH);
  }


  WiFi.mode(WIFI_STA);
  WiFi.begin(wifi_ssid,wifi_pwd);
  // Wait for connection
    int timeout = 0;
    while ((WiFi.status() != WL_CONNECTED) && (timeout < 100)) { //connect or 10s
      Serial.print(".");
      delay(100);
      digitalToggle(LED);
      timeout++;
    }
  if (WiFi.status() != WL_CONNECTED)  changed_data = true;
  drd.stop(); // Wifi is going to be connected, so double reset click is timeout... max 10s


  //wifiReconnect();
 // wifiMulti=new ESP8266WiFiMulti();
  //wifiMulti->addAP("DOrangeFreeDom", "KZagaw01_ruter_key");
  //wifiMulti->addAP("open1.t-mobile.pl");
 // wifiMulti->addAP("InstalujWirusa", "BlaBlaBla123");
 //wifiManager.setBreakAfterConfig(true);
// wifiManager.resetSettings();
 char ssid[20]; 
WiFiManager wifiManager;

sprintf(ssid,"AquaTouch_%d",ESP.getChipId());
 
if (resetWifiConfig)
    wifiManager.startConfigPortal("ssid");
else
{

 if (!wifiManager.autoConnect(ssid/*, "password"*/)) {
    Serial.println("failed to connect, we should reset as see if it connects");
    delay(3000);
    ESP.reset();
    delay(5000);
  }
}
WiFi.setAutoReconnect(true);
WiFi.hostname(ssid);

if(strcmp(wifi_ssid ,WiFi.SSID())!=0 || strcmp(wifi_pwd,WiFi.psk().c_str())!=0)
{
  strncpy(new_data.WIFI_SSID, WiFi.SSID().c_str(), 20);
  strncpy(new_data.WIFI_PASS, WiFi.psk().c_str(), 20);
////////////todo zapis konfigu do pliku wifi.json///////////////////////////////////
}


client.setClient(espClient);
client.setServer(mqtt_server, mqtt_port);

  //setNTP("europe.pool.ntp.org",2*3600);
  timeClient=new NTPClient(ntpUDP, "europe.pool.ntp.org", 2*3600, 60000);// new NTPClient(ntpUDP);
  timeClient->begin();
  
  DPRINTLN("Debug CWifiManager::begin end"); 
}

bool CWifiManager::getWifiStatusString(char *b) 
{ 
  if(wifiConnected())
  {
    IPAddress ip=WiFi.localIP();
    sprintf(b,"WiFi connected: %s ,%d.%d.%d.%d\n", WiFi.SSID().c_str(), ip[0],ip[1],ip[2],ip[3]);
    return true;
  }else
  {
   
    //sprintf(b,"Wifi Connection Error. status= %d",wifiMulti->run());
  sprintf(b,"Wifi Connection Error. status= %d",WiFi.status());
    return false; 
  }
}

 void CWifiManager::wifiScanNetworks()
{
  DPRINT(F("scaning.. "));
  int n = WiFi.scanNetworks(false,false);
  if(n>=0) DPRINT(F(".. done "));
  if (n == 0)
    DPRINTLN(F(" no networks found"));
  else
  {
    DPRINT(n);
    DPRINTLN(F(" networks found"));
    for (int i = 0; i < n; ++i)
    {
      // Print SSID and RSSI for each network found
     DPRINT(i + 1);
     DPRINT(": ");
     DPRINT(WiFi.SSID(i));
     DPRINT(" (");
     DPRINT(WiFi.RSSI(i));
     DPRINT(")");
     DPRINTLN((WiFi.encryptionType(i) == ENC_TYPE_NONE)?" ":"*");
      delay(10);
    }
    DPRINTLN("");
  }
  
}

bool CWifiManager::wifiConnected()
{
 // return wifiMulti.run() == WL_CONNECTED;
// if (WiFi.status()==WL_CONNECTED)return true; else return false;
 if (WiFi.status()/*wifiMulti->run()*/==wl_status_t::WL_CONNECTED)return true; else return false;
}
int CWifiManager::setupMqtt(const char* mqttJsonStr)
{
  DynamicJsonDocument doc(2048);
  DeserializationError error= deserializeJson(doc,mqttJsonStr);
 if (error) 
  {
    DPRINTLN("Blad parsowania zmianyMQTT");
    Serial.println(mqttJsonStr);
    Serial.print(doc.memoryUsage());Serial.print(F(" bytes. "));
    Serial.println(error.c_str());
    return -11;
  }
  JsonObject js = doc.as<JsonObject>();
  if(js.containsKey("host")&&js.containsKey("port"))
  {
    const char* ho=js["host"];
    uint16_t po=js["port"];
  
    const char* ur=js["user"];
    const char* ha=js["pwd"];
    if(ha&&ur)
      setupMqtt(ho,po,ur,ha);
     else if(ur)setupMqtt(ho,po,ur,"");
      else setupMqtt(ho,po,"","");
  }
  return 0;
}
int CWifiManager::setupMqtt(const char* host, uint16_t port,const char* usr,const char* pwd)
{
  if(strlen(host)>=MAX_URL_LENGTH)return -1;
  if(strlen(usr)>=MAX_URL_LENGTH)return -2;
  if(strlen(pwd)>=MAX_URL_LENGTH)return -3;

  strcpy(mqtt_server,host); 
  mqtt_port=port; 
  strcpy(mqtt_user, usr);
  strcpy(mqtt_pass, pwd);

  return 0;
}

bool CWifiManager::reconnectMQTT()
{
  client.disconnect();
  client.setServer(mqtt_server, mqtt_port);
  if (client.connect(nodeMCUid,mqtt_user,mqtt_pass)) 
  {
    char s[MAX_TOPIC_LENGHT];
    strcpy(s,inTopic);
    strcat(s,"/#");  
    client.subscribe(s);
    const char *t="reconnectMQTT, subskrybcja do: ";
    char b[MAX_TOPIC_LENGHT+strlen(t)];
    sprintf(b,"%s%s",t,s);
    RSpisz(debugTopic,b);
   
  }
  return client.connected();
}
void CWifiManager::RSpisz(String topic,String msg,bool cisza)
{
  if(!cisza){
    DPRINT("Debug String RSpisz, topic=");  DPRINT(topic); DPRINT(", msg=");  DPRINT(msg);
  }
  RSpisz((const char*)topic.c_str(),(const char*)msg.c_str(),cisza);
}
void CWifiManager::RSpisz(const char* topic,const char* msg,bool cisza)
{
   if(!cisza){
   DPRINT("Debug RSpisz, topic=");  DPRINT(topic); DPRINT(", msg=");  DPRINTLN(msg);
 //  DPRINT(", wynik=");
   }
   if(conStat==CONN_STAT_WIFIMQTT_OK)
   {
	    client.publish(topic,msg);
      if(!cisza){
        DPRINT( "[");DPRINT(timeClient->getFormattedTime());DPRINT("] ");DPRINTLN(timeClient->getEpochTime());
        }
   }else
   {
	   if(!cisza){DPRINTLN(" nie wysylam, brak polaczenia");}
   }
}


// t is time in seconds = millis()/1000;
char *  CWifiManager::TimeToString(char* str,unsigned long t)
{
// static char str[12];
 long d =t/(3600*24);
 t=t%(3600*24);
 long h = t / 3600;
 t = t % 3600;
 int m = t / 60;
 //int s = t % 60;
 sprintf(str, "%03ldd%02ldh%02dm",d, h, m);
 return str;
}


void CWifiManager::loop()
{
   loopMillis=millis();  
   if(loopMillis%5000==0)
   {
      if(!getWifiStatusString(buforChar))
      {
         Serial.println(buforChar);
         wifiScanNetworks();
         DPRINT("Free heap: ");DPRINTLN(ESP.getFreeHeap());
       }
           
    }
 
  if(!wifiConnected())
 {
    wifiReconnect();
 }else
 {
 
    if (!client.connected()) 
    {
      PrintMEM();

     conStat=CONN_STAT_WIFIMQTT_CONNECTING;
      if (millis() - lastMQTTReconnectAttempt > 5000)
      {
        PrintMEM();
        lastMQTTReconnectAttempt = millis();
        if (reconnectMQTT())
        { 
          PrintMEM();
           RSpisz(debugTopic,"MQTT=ok");
           conStat=CONN_STAT_WIFIMQTT_OK;
          lastMQTTReconnectAttempt = 0;
         
         sprintf(buforChar,"Polaczono SSID=%s, IP=%d.%d.%d.%d",WiFi.SSID().c_str(),WiFi.localIP()[0],WiFi.localIP()[1],WiFi.localIP()[2],WiFi.localIP()[3]);
         RSpisz(debugTopic,buforChar);
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
          yield();
          timeClient->update();
         
           if(loopMillis%600000==0) //10 min wysyłaj pingi watchdoga cyklicznie 
           {
            char m[MAX_MSG_LENGHT];
            sprintf(m,"%ld",loopMillis);
            sprintf(buforChar,"%s/watchdog",inTopic);
            RSpisz(buforChar,m);
          }
          if(loopMillis%600000==0)//10 min sprawdza czy wrocil ping watchdoga
          {
            char t[20];
            sprintf(buforChar,"czas od restartu= %s", TimeToString(t,loopMillis/1000));
            DPRINTLN(buforChar);
            PrintMEM();
            DPRINT(F("Watchdog czas "));
            DPRINTLN(loopMillis-WDmillis);
            if(loopMillis-WDmillis>600000)
            {
                RSpisz(debugTopic,F("Watchdog restart"));
                delay(3000);
                //ESP.restart();
              //  ESP.reset();  // hard reset
              //  resetFunc();
            }
          }         
    }
 } 
}
