#include "CWebSerwer.h"



void CWebSerwer::begin()
{
//  server= new ESP8266WebServer(80);
  webSocket = new WebSocketsServer(81);
   webSocket->begin();
   //webSocket->onEvent(webSocketEvent);

    if(MDNS.begin("podlewacz")) {
        USE_SERIAL.println("MDNS responder started");
    }

    // handle index
    server.on("/", [this]() {
        // send index.html
        server.send(200, "text/html", "<html><head><script>var connection = new WebSocket('ws://'+location.hostname+':81/', ['arduino']);connection.onopen = function () {  connection.send('Connect ' + new Date()); }; connection.onerror = function (error) {    console.log('WebSocket Error ', error);};connection.onmessage = function (e) {  console.log('Server: ', e.data);};function sendRGB() {  var r = parseInt(document.getElementById('r').value).toString(16);  var g = parseInt(document.getElementById('g').value).toString(16);  var b = parseInt(document.getElementById('b').value).toString(16);  if(r.length < 2) { r = '0' + r; }   if(g.length < 2) { g = '0' + g; }   if(b.length < 2) { b = '0' + b; }   var rgb = '#'+r+g+b;    console.log('RGB: ' + rgb); connection.send(rgb); }</script></head><body>LED Control:<br/><br/>R: <input id=\"r\" type=\"range\" min=\"0\" max=\"255\" step=\"1\" oninput=\"sendRGB();\" /><br/>G: <input id=\"g\" type=\"range\" min=\"0\" max=\"255\" step=\"1\" oninput=\"sendRGB();\" /><br/>B: <input id=\"b\" type=\"range\" min=\"0\" max=\"255\" step=\"1\" oninput=\"sendRGB();\" /><br/></body></html>");
    });

    server.begin();

    // Add service to MDNS
    MDNS.addService("http", "tcp", 80);
    MDNS.addService("ws", "tcp", 81);

   // digitalWrite(LED_RED, 0);
   // digitalWrite(LED_GREEN, 0);
   // digitalWrite(LED_BLUE, 0);
}

void CWebSerwer::webSocketEvent(uint8_t num, WStype_t type, uint8_t * payload, size_t length)
{

    switch(type) {
        case WStype_DISCONNECTED:
            USE_SERIAL.printf("[%u] Disconnected!\n", num);
            break;
        case WStype_CONNECTED: {
            IPAddress ip = webSocket->remoteIP(num);
            USE_SERIAL.printf("[%u] Connected from %d.%d.%d.%d url: %s\n", num, ip[0], ip[1], ip[2], ip[3], payload);

            // send message to client
            webSocket->sendTXT(num, "Connected");
        }
            break;
        case WStype_TEXT:
            USE_SERIAL.printf("[%u] get Text: %s\n", num, payload);

            if(payload[0] == '#') {
                // we get RGB data

                // decode rgb data
                uint32_t rgb = (uint32_t) strtol((const char *) &payload[1], NULL, 16);

           //     analogWrite(LED_RED,    ((rgb >> 16) & 0xFF));
           //     analogWrite(LED_GREEN,  ((rgb >> 8) & 0xFF));
             //   analogWrite(LED_BLUE,   ((rgb >> 0) & 0xFF));
            }

            break;
    }

}

void CWebSerwer::loop()
{
   webSocket->loop();
   server.handleClient();
}

