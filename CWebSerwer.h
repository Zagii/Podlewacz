#ifndef CWEBSERWER_H
#define CWEBSERWER_H

#include <WebSocketsServer.h>
#include <ESP8266WebServer.h>
#include <ESP8266mDNS.h>

#define USE_SERIAL Serial


class CWebSerwer
{
  
    ESP8266WebServer server;
    WebSocketsServer *webSocket;
    
    public: 
    CWebSerwer(){};
     void webSocketEvent(uint8_t num, WStype_t type, uint8_t * payload, size_t length);
     void begin();
     void loop();
     WebSocketsServer *getWebSocket(){return webSocket;};
};
#endif

