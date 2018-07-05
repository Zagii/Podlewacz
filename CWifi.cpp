#include "CWifi.h"

void CWifi::wifiReconnect()
{

  //return;
 // DPRINTLN("  Debug CWifi::wifiReconnect start"); 
//  WiFi.disconnect();
  delay(1000);
 // WiFi.begin("DOrangeFreeDom", "KZagaw01_ruter_key");
 // delay(1000);
 // DPRINTLN("  Debug CWifi::wifiReconnect end"); 
}
void CWifi::setNTP(String host,unsigned long offset)
{
  ntp_server=host;
  timeClient->end();
  delete timeClient;
  ntp_offset=offset;
  timeClient=new NTPClient(ntpUDP, ntp_server.c_str(), offset*3600, 60000);
  timeClient->begin();
}

void CWifi::zmianaAP(String jsonString)
{
  DynamicJsonBuffer jsonBuffer;
  JsonObject& js= jsonBuffer.parse(jsonString);
  if (!js.success()) 
  {
    DPRINTLN("Blad parsowania zmianyAP");
    return;
  }
  if(js.containsKey("ssid"))
  {
    String s=js["ssid"];
    if(js.containsKey("pwd"))
    {
      String p=js["pwd"];
      zmianaAP(s,p);
    }
  }
}
void CWifi::zmianaAP(String ssid,String pwd)
{
  wifi_ssid=ssid;
  wifi_pwd=pwd;
  if(wifiMulti) delete wifiMulti;
  wifiMulti=new ESP8266WiFiMulti();
  wifiMulti->addAP(ssid.c_str(),pwd.c_str());
  delay(10);
  
}

void CWifi::begin()
{
  DPRINTLN("Debug CWifi::begin start"); 
 
 mqtt_server="broker.hivemq.com";



  WiFi.hostname(WiFiHostname);
  WiFi.mode(WIFI_AP_STA);
//  WiFi.mode(WIFI_STA);

  WiFi.softAPConfig(apIP, apIP, IPAddress(255, 255, 255, 0));
  WiFi.softAP(APssid, APpassword); //|| reset();             // Start the access point
  DPRINTLN("AP uruchomiony");
  
  wifiReconnect();
  wifiMulti=new ESP8266WiFiMulti();
  wifiMulti->addAP("DOrangeFreeDom", "KZagaw01_ruter_key");
  wifiMulti->addAP("open.t-mobile.pl");
  wifiMulti->addAP("InstalujWirusa", "BlaBlaBla123");

  clientMqtt.setClient(espClient);
  clientMqtt.setServer(mqtt_server.c_str(), mqtt_port);
 // clientMqtt.setCallback(callback);
  timeClient=new NTPClient(ntpUDP, "europe.pool.ntp.org", 2*3600, 60000);// new NTPClient(ntpUDP);
  timeClient->begin();
  
  DPRINTLN("Debug CWifi::begin end"); 
}

bool CWifi::getWifiStatusString(char *b) 
{ 
  if(wifiConnected())
  {
    IPAddress ip=WiFi.localIP();
    sprintf(b,"WiFi connected: %s ,%d.%d.%d.%d\n", WiFi.SSID().c_str(), ip[0],ip[1],ip[2],ip[3]);
    return true;
  }else
  {
    sprintf(b,"Wifi Connection Error. status= %d",wifiMulti->run());
 // sprintf(b,"Wifi Connection Error. status= %d",WiFi.status());
    return false; 
  }
}

 void CWifi::wifiScanNetworks()
{
  DPRINT("scaning.. ");
  int n = WiFi.scanNetworks(false,false);
  if(n>=0) DPRINT(".. done ");
  if (n == 0)
    DPRINTLN(" no networks found");
  else
  {
    DPRINT(n);
    DPRINTLN(" networks found");
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

bool CWifi::wifiConnected()
{
 if (wifiMulti->run()==WL_CONNECTED)return true; else return false;
}
void CWifi::setupMqtt(String mqttJsonStr)
{
  DynamicJsonBuffer jsonBuffer;
  JsonObject& js= jsonBuffer.parse(mqttJsonStr);
  if (!js.success()) 
  {
    DPRINTLN("Blad parsowania zmianyMQTT");
    return;
  }
  if(js.containsKey("host")&&js.containsKey("port"))
  {
    String host=js["host"];
    uint16_t port=js["port"];
  
    String usr="";
    if(js.containsKey("user"))usr=String((const char*)js["user"]);
    String pwd="";
    if(js.containsKey("pwd"))pwd=String((const char*)js["pwd"]);
    
    setupMqtt(host,port,usr,pwd);
    
  }else
  setupMqtt(MQTT_def_Host,MQTT_def_port,"","");
}
void CWifi::setupMqtt(String host, uint16_t port,String usr,String pwd)
{
  DPRINT("setupMqtt host:");DPRINT(host);DPRINT("; port:");DPRINT(port);DPRINT("; user:");DPRINT(usr);DPRINT(", pass:");DPRINTLN(pwd);
 // clientMqtt.disconnect();
  mqtt_server=host; 
  mqtt_port=port; 
  mqtt_user= usr;
  mqtt_pass=pwd;
  
  clientMqtt.setServer(mqtt_server.c_str(), mqtt_port);
  delay(30);
  reconnectMQTT(); 
}

bool CWifi::reconnectMQTT()
{
  DPRINT("reconnectMQTT()");DPRINTLN(clientMqtt.state());
  bool cliConStat=false;
  DPRINT(" host:");DPRINT(mqtt_server);DPRINT(" port:");DPRINT(mqtt_port);DPRINT(" user:");DPRINT(mqtt_user);DPRINT(", pass:");DPRINTLN(mqtt_pass);
  if(mqtt_user.length()>0)
  {
    if(mqtt_pass.length()>0)  
      {cliConStat=clientMqtt.connect(nodeMCUid,mqtt_user.c_str(),mqtt_pass.c_str());DPRINTLN("z userem i haslem");}
    else
     { cliConStat=clientMqtt.connect(nodeMCUid,mqtt_user.c_str(),NULL);DPRINTLN("bez hasla");}
  }else
  {
    cliConStat=clientMqtt.connect(nodeMCUid);
    DPRINTLN("bez usera");
  }
  delay(200);
  DPRINT("cliConState=");DPRINTLN(clientMqtt.state());
  if(cliConStat) 
  {
    char s[MAX_TOPIC_LENGHT];
    strcpy(s,inTopic);
    strcat(s,"\/#");  
    clientMqtt.subscribe(s);
    const char *t="reconnectMQTT, subskrybcja do: ";
    char b[MAX_TOPIC_LENGHT+strlen(t)];
    sprintf(b,"%s%s",t,s);
    RSpisz(debugTopic,b);
   
  }
  return clientMqtt.connected();
}
void CWifi::RSpiszStr(String topic,String msg,bool cisza)
{
  if(!cisza)
  {
    DPRINT(">>>>Debug String RSpisz, topic=");  DPRINT(topic); DPRINT(", msg=");  DPRINTLN(msg);
  }
  RSpisz((const char*)topic.c_str(),(char*)msg.c_str(),cisza);
}
void CWifi::RSpisz(const char* topic,char* msg,bool cisza)
{
   if(!cisza)
   {
   DPRINT(">>Debug RSpisz, topic=");  DPRINT(topic); DPRINT(", msg=");  DPRINTLN(msg);
 //  DPRINT(", wynik=");
   }
   if(clientMqtt.connected())//conStat==CONN_STAT_WIFIMQTT_OK)
   {
	    bool publ=clientMqtt.publish(topic,msg);
      if(!cisza){
        DPRINT("wyslano MQTT");
        DPRINT( "[");DPRINT(timeClient->getFormattedTime());DPRINT("# ");DPRINT(publ);DPRINT("] ");DPRINTLN(timeClient->getEpochTime());
        }
   }else
   {
	   if(!cisza)
	   {
      DPRINTLN(" nie wysylam, brak polaczenia");
	    DPRINTLN(clientMqtt.state());
      } 
   }
}


// t is time in seconds = millis()/1000;
char *  CWifi::TimeToString(unsigned long t)
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


char bb[70];
void CWifi::loop()
{
   loopMillis=millis();  
   if(loopMillis%5000==0)
   {
      if(!getWifiStatusString(bb))
      {
         Serial.println(bb);
         wifiScanNetworks();
         DPRINT("Free heap: ");DPRINTLN(ESP.getFreeHeap());
       }
           
    }
 
  if(!wifiConnected())
 {
    wifiReconnect();
 }else
 {
 
     if (!clientMqtt.connected()) 
    {
      conStat=CONN_STAT_WIFIMQTT_CONNECTING;
      if (millis() - lastMQTTReconnectAttempt > 5000)
      {
        lastMQTTReconnectAttempt = millis();
        if(reconnectMQTT())
        {
          delay(100);
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
             DPRINT("Err MQTTstat= ");DPRINTLN(clientMqtt.state());
             DPRINT("WIFI ip= ");DPRINTLN(WiFi.localIP());
        }
      }
    } else
    {
          clientMqtt.loop();     
          delay(5);
          timeClient->update();
         
           if(loopMillis%600000==0) //10 min wysyłaj pingi watchdoga cyklicznie 
           {
            char m[MAX_MSG_LENGHT];
            sprintf(m,"%ld",loopMillis);
            char m2[MAX_TOPIC_LENGHT];
            sprintf(m2,"%s/watchdog",inTopic);
            RSpisz(m2,m);
          }
          if(loopMillis%600000==0)//10 min sprawdza czy wrocil ping watchdoga
          {
            String str="czas od restartu= "+(String) TimeToString(loopMillis/1000);
            DPRINTLN(str);
            DPRINT("Watchdog czas ");
            DPRINTLN(loopMillis-WDmillis);
            if(loopMillis-WDmillis>600000)
            {
                RSpisz(debugTopic,"Watchdog restart");
                delay(3000);
                //ESP.restart();
              //  ESP.reset();  // hard reset
              //  resetFunc();
            }
          }         
    }
 } 
}
