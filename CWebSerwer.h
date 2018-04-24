#ifndef CWEBSERWER_H
#define CWEBSERWER_H

#include <WebSocketsServer.h>
#include <ESP8266WebServer.h>
#include <ESP8266mDNS.h>
#include <Time.h>
#include <TimeLib.h>
#include "FS.h"

#define USE_SERIAL Serial


class CWebSerwer
{
  
    ESP8266WebServer server;
    WebSocketsServer *webSocket;

    uint8_t stanSekcji;

    unsigned long ostatnioWyslanyCzas_s;
    char geoLok[30];
    double Temperatura;
    double Cisnienie;
    bool czujnikDeszczu;
    bool TrybAuto;
    
    public: 
    CWebSerwer(){};
     void webSocketEvent(uint8_t num, WStype_t type, uint8_t * payload, size_t length);
     void begin();
     void loop(unsigned long t_s, char* geo, double temp,double p, bool deszcz,bool trybAuto);
     WebSocketsServer *getWebSocket(){return webSocket;};
     void zmienStanSekcji(uint8_t stan);
     String getContentType(String filename);
     bool handleFileRead(String path);
  
};
#endif

