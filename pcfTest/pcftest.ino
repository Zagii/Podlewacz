#include <ESP8266WiFi.h>
#include <WiFiClient.h>
#include <ESP8266WebServer.h>
#include <pcf8574_esp.h>
#include <Wire.h>


PCF857x pcf8574(0b00111000, &Wire);
#define PIN_INT D5
#define PIN_SDA D1
#define PIN_SCL D2
//bool pcf_int = false;
byte sw_state;

//------------------------------------------
//SETUP LED
long time_led;
#define LED_DELAY 1000
#define LED_PIN D0

//------------------------------------------
//WIFI i server HTTP
ESP8266WebServer server(80);
#define WIFI_SSID "DOrangeFreeDom"
#define WIFI_PASSWORD "KZagaw01_ruter_key"
#define HOST_NAME "onoff"



void setup() {
  pinMode(LED_PIN, OUTPUT);
  
  //Setup UART
  Serial.begin(115200);
  Serial.println("");
  Serial.println("START");

  //Setup WIFI 
  WiFi.begin(WIFI_SSID, WIFI_PASSWORD);
  WiFi.hostname( HOST_NAME );
  Serial.print("Connecting to WIFI");

  //Wait for WIFI connection
  while( WiFi.status() != WL_CONNECTED ) {
    delay(500);
    Serial.print(".");
  }
  Serial.println("");
  Serial.print("Connected to ");
  Serial.println( WIFI_SSID );
  Serial.print("IP address: ");
  Serial.println(WiFi.localIP());

  //Setup HTTP server
  server.on("/", handleRoot);
  server.on("/sw", handleSW);
  server.on("/state", handleState);
  server.onNotFound(handleNotFound);
  server.begin();
  Serial.println("HTTP server started");

  //Setup PCF8574
  Wire.pins(PIN_SDA, PIN_SCL);//SDA - D1, SCL - D2
  Wire.begin();
  
  pinMode(PIN_INT, INPUT_PULLUP);

  pcf8574.begin( 0x00 ); //0xF0 4 pin input, 4 pin output
  pcf8574.resetInterruptPin();
}

void loop() {
  long time_loop = millis();

  server.handleClient();

  //BEGIN: PCF8574 INT
  if( digitalRead(PIN_INT)==LOW ){
    delay(50);
    byte b = pcf8574.read8();
    Serial.println( "INT: " + String(b));

    byte keys = ((~b) >> 4) & 0x0F;
    
    if( CheckKey(keys, 0) ){
      Serial.println( "KEY 0");
      SW_toggle(0);
    }

    if( CheckKey(keys, 1) ){
      Serial.println( "KEY 1");
      SW_toggle(1);
    }

    if( CheckKey(keys, 2) ){
      Serial.println( "KEY 2");
      SW_toggle(2);
    }

    if( CheckKey(keys, 3) ){
      Serial.println( "KEY 3");
      SW_toggle(3);
    }
  }
  //END: PCF8574 INT
  
  if( time_loop - time_led > LED_DELAY ){
    digitalWrite( LED_PIN, !digitalRead( LED_PIN) );
    time_led = time_loop;
  }

}

bool CheckKey(byte key, byte num){ //0, 1, 2, 3
  return key & (1 << num);
}

bool SW_state(byte num){
  return (sw_state >> num) & 1;
}

void SW_all_on(){
  pcf8574.write8( 0xFF );
  sw_state =0; //0x0F;
}

void SW_all_off(){
  pcf8574.write8( 0xF0 | 0 );
  sw_state = 0x00;
}

void SW_on(byte num){
  byte b = pcf8574.read8();
  b = b | (/* 0xF0 | */(1 << num) );
  pcf8574.write8( b );
  sw_state = /*0x0F &*/ b;
}

void SW_off(byte num){
  byte b = pcf8574.read8();
  b = b & (~(1 << num));
  pcf8574.write8(/* 0xF0 |*/ b );
  sw_state = /*0x0F &*/ b;
}

bool SW_toggle(byte num){
  byte b = pcf8574.read8();
  byte n = (~b) & (1 << num);
  b = b & (~(1 << num));
  b = b | n;
//  b = 0xF0 | b;
  pcf8574.write8( b );
  sw_state =/* 0x0F &*/ b;
  
  return (b >> num) & 1;
}

void SW(byte num, bool sw){
  if( sw ){
    SW_on(num);
  }else{
    SW_off(num);
  }
}

String HtmlGetState(){
  String html = "";
  for(byte i=0;i<8;i++){
    if( SW_state(i) ){
      html += "<a href='sw?sw=off&num=" + String(i) + "' style='display: inline-block; width: 100px; background-color: green; text-align: center; margin-bottom: 15px;'>"+i+" = OFF</a>\r\n<br>";
    }else{
      html += "<a href='sw?sw=on&num=" + String(i) + "' style='display: inline-block; width: 100px; background-color: red; text-align: center; margin-bottom: 15px;'>"+i+" = ON</a>\r\n<br>";
    }
  }

  return html;
}

void handleRoot(){
  String html = "<!DOCTYPE html>\r\n";
  html += "<html>\r\n";
  html += "<head>\r\n";
  html += "<meta charset='UTF-8'>\r\n";
  html += "<meta name='viewport' content='width=device-width, minimum-scale=1.0, maximum-scale=1.0' />";
  html += "<title>Podlewacz test</title>\r\n";

  //BEGIN: AJAX ------------------------------------------
  html += "<script src='https://ajax.googleapis.com/ajax/libs/jquery/3.1.1/jquery.min.js'></script>\r\n";
  html += "<script>\r\n";
  html += "function ajax_read(){\r\n";
  html += " $.ajax({\r\n";
  html += "    url: 'state', \r\n";
  html += "    success: function(result){\r\n";
  html += "      $('#res').html(result);\r\n";
  html += "    }\r\n";
  html += "  }\r\n";
  html += " );\r\n";
  html += "}\r\n";

  html += "function check() {\r\n";
  html += " ajax_read();\r\n";
  html += " setTimeout(function(){ check(); }, 1000);\r\n";
  html += "}\r\n";
  html += "check();\r\n";
  
  html += "</script>\r\n";
  //END: AJAX ------------------------------------------
  
  html += "</head>\r\n";
  html += "<body>\r\n";

//  html += "ESP8266 - ";
//  html += HOST_NAME;
 // html += "\r\n";

  html += "<br>Lista sekcji<br><br><br>";

  html += "<div id='res'>";
  html += HtmlGetState();
  html += "</div>\r\n";
  html += "<br>\r\n";
  
  html += "</body>\r\n";
  html += "</html>\r\n";

  
  server.send(200, "text/html", html);
}

void handleSW(){
  byte num = server.arg("num").toInt();
  if (server.arg("sw")== "on"){
    SW(num, HIGH);
  }else{
    SW(num, LOW);
  }

  String ip = WiFi.localIP().toString();
  server.sendHeader("Location", String("http://") + ip, true);
  server.send ( 302, "text/plain", "");
  server.client().stop();
}

void handleState(){
  String message = HtmlGetState();
  server.send(200, "text/html", message );
}

void handleNotFound(){
  String message = "File Not Found\n\n";
  message += "URI: ";
  message += server.uri();
  message += "\nMethod: ";
  message += (server.method() == HTTP_GET) ? "GET" : "POST";
  message += "\nArguments: ";
  message += server.args();
  message += "\n";
  for (uint8_t i=0; i<server.args(); i++){
    message += " " + server.argName(i) + ": " + server.arg(i) + "\n";
  }
  server.send(404, "text/plain", message);
}
