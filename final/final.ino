
#include <Arduino.h>
#include <WiFi.h>
#include <ESPAsyncWebServer.h>
#include "AsyncJson.h"
#include "ArduinoJson.h"

#include "NewPing.h"      // include NewPing library thư viên để đo khoảng cách 

//khai báo giá trị 2 chân trig và echo của Utra sonic
int trigPin = 4;      // trigger pin
int echoPin = 5;      // echo pin
long duration; // variable for the duration of sound wave travel
int distance; // variable for the distance measurement


int relay_pin = 2;  //khai bao chân relay

NewPing sonar(trigPin, echoPin);


//AsyncWebServer server(80);
//const char *ssid = "UIT Public";
//const char *password = "";

// Set to true to define Relay as Normally Open (NO)
#define RELAY_NO    true

// Set number of relays
#define NUM_RELAYS  1

// Assign each GPIO to a relay
//int relayGPIOs[NUM_RELAYS] = {2, 26, 27, 25, 33};
int relayGPIOs[NUM_RELAYS] = {2};

// Replace with your network credentials
const char* ssid = "UIT Public";
const char* password = "";

const char* PARAM_INPUT_1 = "relay";  
const char* PARAM_INPUT_2 = "state";

// Create AsyncWebServer object on port 80
AsyncWebServer server(80);


String relayState(int numRelay){
  if(RELAY_NO){
    if(digitalRead(relayGPIOs[numRelay-1])){
      return "";
    }
    else {
      return "checked";
    }
  }
  else {
    if(digitalRead(relayGPIOs[numRelay-1])){
      return "checked";
    }
    else {
      return "";
    }
  }
  return "";
}

const char index_html[] PROGMEM = R"rawliteral(
<!DOCTYPE HTML><html>
<head>
  <meta name="viewport" content="width=device-width, initial-scale=1">
  <style>
    html {font-family: Arial; display: inline-block; text-align: center;}
    h2 {font-size: 3.0rem;}
    p {font-size: 3.0rem;}
    body {max-width: 600px; margin:0px auto; padding-bottom: 25px;}
    .switch {position: relative; display: inline-block; width: 120px; height: 68px} 
    .switch input {display: none}
    .slider {position: absolute; top: 0; left: 0; right: 0; bottom: 0; background-color: #ccc; border-radius: 34px}
    .slider:before {position: absolute; content: ""; height: 52px; width: 52px; left: 8px; bottom: 8px; background-color: #fff; -webkit-transition: .4s; transition: .4s; border-radius: 68px}
    input:checked+.slider {background-color: #2196F3}
    input:checked+.slider:before {-webkit-transform: translateX(52px); -ms-transform: translateX(52px); transform: translateX(52px)}
  </style>
</head>
<body>
  <h2>ESP Web Server</h2>
  %BUTTONPLACEHOLDER%
<script>function toggleCheckbox(element) {
  var xhr = new XMLHttpRequest();
  if(element.checked){ xhr.open("GET", "/update?relay="+element.id+"&state=1", true); }
  else { xhr.open("GET", "/update?relay="+element.id+"&state=0", true); }
  xhr.send();
}</script>
</body>
</html>
)rawliteral";

// Replaces placeholder with button section in your web page
String processor(const String& var){
  //Serial.println(var);
  if(var == "BUTTONPLACEHOLDER"){
    String buttons ="";
    for(int i=1; i<=NUM_RELAYS; i++){
      String relayStateValue = relayState(i);
      buttons+= "<h4>Relay #" + String(i) + " - GPIO " + relayGPIOs[i-1] + "</h4><label class=\"switch\"><input type=\"checkbox\" onchange=\"toggleCheckbox(this)\" id=\"" + String(i) + "\" "+ relayStateValue +"><span class=\"slider\"></span></label>";
    }
    return buttons;
  }
  return String();
}


//
//WiFiServer server1(80);    // Server will be at port 80

void notFound(AsyncWebServerRequest *request)
{
  request->send(404, "application/json", "{\"message\":\"Not found\"}");
}
void setup()
{
  //biến khoảng cách
  float distance = sonar.ping_cm();

  
  Serial.begin(115200);

   pinMode (relay_pin, OUTPUT);

    // Set all relays to off when the program starts - if set to Normally Open (NO), the relay is off when you set the relay to HIGH
  for(int i=1; i<=NUM_RELAYS; i++){
    pinMode(relayGPIOs[i-1], OUTPUT);
    if(RELAY_NO){
      digitalWrite(relayGPIOs[i-1], HIGH);
    }
    else{
      digitalWrite(relayGPIOs[i-1], LOW);
    }
  }
  
  WiFi.mode(WIFI_STA);
  WiFi.begin(ssid, password);
  while (WiFi.waitForConnectResult() != WL_CONNECTED)
  {
    Serial.printf("WiFi Failed!\n");
  }
  Serial.print("IP Address:");
  Serial.println(WiFi.localIP());

  
server.on("/", HTTP_GET, [](AsyncWebServerRequest *request) {
     request->send_P(200, "text/html", index_html, processor);
  });
  server.on("/get-message", HTTP_GET, [](AsyncWebServerRequest *request) {
    StaticJsonDocument<100> data;
    if (request->hasParam("message"))
    {
      data["message"] = request->getParam("message")->value();
    }
    else
    {
//      data["message"] = "abc";
     data["message"] =  sonar.ping_cm();
    }
    String response;
    serializeJson(data, response);
    request->send(200, "application/json", response);
  });

  
  // Send a GET request to <ESP_IP>/update?relay=<inputMessage>&state=<inputMessage2>
  server.on("/update", HTTP_GET, [] (AsyncWebServerRequest *request) {
    String inputMessage;
    String inputParam;
    String inputMessage2;
    String inputParam2;
    // GET input1 value on <ESP_IP>/update?relay=<inputMessage>
    if (request->hasParam(PARAM_INPUT_1) & request->hasParam(PARAM_INPUT_2)) {
      inputMessage = request->getParam(PARAM_INPUT_1)->value();
      inputParam = PARAM_INPUT_1;
      inputMessage2 = request->getParam(PARAM_INPUT_2)->value();
      inputParam2 = PARAM_INPUT_2;
      if(RELAY_NO){
        Serial.print("NO ");
        digitalWrite(relayGPIOs[inputMessage.toInt()-1], !inputMessage2.toInt());
      }
      else{
        Serial.print("NC ");
        digitalWrite(relayGPIOs[inputMessage.toInt()-1], inputMessage2.toInt());
      }
    }
    else {
      inputMessage = "No message sent";
      inputParam = "none";
    }
    Serial.println(inputMessage + inputMessage2);
    request->send(200, "text/plain", "OK");
  });
  
  AsyncCallbackJsonWebHandler *handler = new AsyncCallbackJsonWebHandler("/post-message", [](AsyncWebServerRequest *request, JsonVariant &json) {
    StaticJsonDocument<200> data;
    if (json.is<JsonArray>())
    {
      data = json.as<JsonArray>();
    }
    else if (json.is<JsonObject>())
    {
      data = json.as<JsonObject>();
    }
    String response;
    serializeJson(data, response);
    request->send(200, "application/json", response);
    Serial.println(response);
  });
  server.addHandler(handler);
  server.onNotFound(notFound);
  server.begin();
//  server1.begin();
}
void loop()
{
//
//   WiFiClient client = server1.available();     //Checking if any client request is available or not
//  if (client)
//  {
//    boolean currentLineIsBlank = true;
//    String buffer = "";  
//    while (client.connected())
//    {
//      if (client.available())                    // if there is some client data available
//      {
//        char c = client.read(); 
//        buffer+=c;                              // read a byte
//        if (c == '\n' && currentLineIsBlank)    // check for newline character, 
//        {
//          client.println("HTTP/1.1 200 OK");
//          client.println("Content-Type: text/html");
//          client.println();    
//          client.print("<HTML><title>ESP32</title>");
//          client.print("<body><h1>ESP32 Standalone Relay Control </h1>");
//          client.print("<p>Relay Control</p>");
//          client.print("<a href=\"/?relayon\"\"><button>ON</button></a>");
//          client.print("<a href=\"/?relayoff\"\"><button>OFF</button></a>");
//          client.print("</body></HTML>");
//          break;        // break out of the while loop:
//        }
//        if (c == '\n') { 
//          currentLineIsBlank = true;
//          buffer="";       
//        } 
//        else 
//          if (c == '\r') {     
//          if(buffer.indexOf("GET /?relayon")>=0)
//            digitalWrite(relay_pin, LOW);
//          if(buffer.indexOf("GET /?relayoff")>=0)
//            digitalWrite(relay_pin, HIGH);   
//        }
//        else {
//          currentLineIsBlank = false;
//        }  
//      }
//    }
//    client.stop();
//  }
}
