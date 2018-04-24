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

  //SPIFFS.begin();    

  server.onNotFound([this]() {                              // If the client requests any URI
    if (!handleFileRead(server.uri()))                  // send it if it exists
      server.send(404, "text/plain", "404: Not Found"); // otherwise, respond with a 404 (Not Found) error
  });

  /*  // handle index
    server.on("/", [this]() {
        // send index.html
        server.send(200, "text/html", "<html><head><script>var connection = new WebSocket('ws://'+location.hostname+':81/', ['arduino']);connection.onopen = function () {  connection.send('Connect ' + new Date()); }; connection.onerror = function (error) {    console.log('WebSocket Error ', error);};connection.onmessage = function (e) {  console.log('Server: ', e.data);};function sendRGB() {  var r = parseInt(document.getElementById('r').value).toString(16);  var g = parseInt(document.getElementById('g').value).toString(16);  var b = parseInt(document.getElementById('b').value).toString(16);  if(r.length < 2) { r = '0' + r; }   if(g.length < 2) { g = '0' + g; }   if(b.length < 2) { b = '0' + b; }   var rgb = '#'+r+g+b;    console.log('RGB: ' + rgb); connection.send(rgb); }</script></head><body>LED Control:<br/><br/>R: <input id=\"r\" type=\"range\" min=\"0\" max=\"255\" step=\"1\" oninput=\"sendRGB();\" /><br/>G: <input id=\"g\" type=\"range\" min=\"0\" max=\"255\" step=\"1\" oninput=\"sendRGB();\" /><br/>B: <input id=\"b\" type=\"range\" min=\"0\" max=\"255\" step=\"1\" oninput=\"sendRGB();\" /><br/></body></html>");
    });
*/
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

void CWebSerwer::zmienStanSekcji(uint8_t stan)
{
  if(stan==stanSekcji)return;
  stanSekcji=stan;
  char w[3]="s";
  w[1]=(char)stan;
  w[2]='\0';
  webSocket->broadcastTXT(w);
}

bool CWebSerwer::handleFileRead(String path){  // send the right file to the client (if it exists)
  Serial.println("handleFileRead: " + path);
  if(path.endsWith("/")) path += "index.html";           // If a folder is requested, send the index file
  String adr="";
  if(path=="ws.js")
  {
    adr="var wsSerw=\""+WiFi.localIP().toString()+"\"";
  }
  String contentType = getContentType(path);             // Get the MIME type
  String pathWithGz = path + ".gz";
  if(SPIFFS.exists(pathWithGz) || SPIFFS.exists(path)){  // If the file exists, either as a compressed archive, or normal
    if(SPIFFS.exists(pathWithGz))                          // If there's a compressed version available
      path += ".gz";                                         // Use the compressed version
    File file = SPIFFS.open(path, "r");                    // Open the file
    if(adr!="")
    {
       size_t sent = server.streamFile(adr, contentType);  
    }
    
      size_t sent = server.streamFile(file, contentType);    // Send it to the client
    
    file.close();                                          // Close the file again
    Serial.println(String("\tSent file: ") + path);
    return true;
  }
  Serial.println(String("\tFile Not Found: ") + path);
  return false;                                          // If the file doesn't exist, return false
}

String CWebSerwer::getContentType(String filename){
  if(server.hasArg("download")) return "application/octet-stream";
  else if(filename.endsWith(".htm")) return "text/html";
  else if(filename.endsWith(".html")) return "text/html";
  else if(filename.endsWith(".css")) return "text/css";
  else if(filename.endsWith(".js")) return "application/javascript";
  else if(filename.endsWith(".png")) return "image/png";
  else if(filename.endsWith(".gif")) return "image/gif";
  else if(filename.endsWith(".jpg")) return "image/jpeg";
  else if(filename.endsWith(".ico")) return "image/x-icon";
  else if(filename.endsWith(".xml")) return "text/xml";
  else if(filename.endsWith(".pdf")) return "application/x-pdf";
  else if(filename.endsWith(".zip")) return "application/x-zip";
  else if(filename.endsWith(".gz")) return "application/x-gzip";
  return "text/plain";
}

void CWebSerwer::loop(unsigned long t_s, char* geo, double temp,double p, bool deszcz,bool trybAuto)
{
   webSocket->loop();
   server.handleClient();
   if(ostatnioWyslanyCzas_s!=t_s)
   {
    ostatnioWyslanyCzas_s=t_s;
    char w[15]="t";
 
    sprintf(w, "t%03ldd%02ldh%02dm",hour(t_s), minute(t_s), second(t_s));
    
    webSocket->broadcastTXT(w);
   }
   if(strcmp(geo,geoLok)!=0)
   {
     /*Temperatura;
    Cisnienie;
    czujnikDeszczu;
    TrybAuto;
    geoLok[30];*/
   }
   
}

