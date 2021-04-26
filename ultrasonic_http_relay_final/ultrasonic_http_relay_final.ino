
#include <Arduino.h>
#include <WiFi.h>
#include <ESPAsyncWebServer.h>
#include "AsyncJson.h"
#include "ArduinoJson.h"

#include "NewPing.h"      // include NewPing library thư viên để đo khoảng cách 

//khai báo giá trị 2 chân trig và echo của Utra sonic
int trigPin = 4;      // trigger pin
int echoPin = 0;      // echo pin
long duration; // variable for the duration of sound wave travel
int distance; // variable for the distance measurement


int relay_pin = 2;  //khai bao chân relay

NewPing sonar(trigPin, echoPin);


AsyncWebServer server(80);
const char *ssid = "UIT Public";
const char *password = "";

WiFiServer server1(80);    // Server will be at port 80

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
  
  WiFi.mode(WIFI_STA);
  WiFi.begin(ssid, password);
  while (WiFi.waitForConnectResult() != WL_CONNECTED)
  {
    Serial.printf("WiFi Failed!\n");
  }
  Serial.print("IP Address:");
  Serial.println(WiFi.localIP());

  
server.on("/", HTTP_GET, [](AsyncWebServerRequest *request) {
    request->send(200, "application/json", "{\"message\": \"Welcome\" }");
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
  server1.begin();
}
void loop()
{

   WiFiClient client = server1.available();     //Checking if any client request is available or not
  if (client)
  {
    boolean currentLineIsBlank = true;
    String buffer = "";  
    while (client.connected())
    {
      if (client.available())                    // if there is some client data available
      {
        char c = client.read(); 
        buffer+=c;                              // read a byte
        if (c == '\n' && currentLineIsBlank)    // check for newline character, 
        {
          client.println("HTTP/1.1 200 OK");
          client.println("Content-Type: text/html");
          client.println();    
          client.print("<HTML><title>ESP32</title>");
          client.print("<body><h1>ESP32 Standalone Relay Control </h1>");
          client.print("<p>Relay Control</p>");
          client.print("<a href=\"/?relayon\"\"><button>ON</button></a>");
          client.print("<a href=\"/?relayoff\"\"><button>OFF</button></a>");
          client.print("</body></HTML>");
          break;        // break out of the while loop:
        }
        if (c == '\n') { 
          currentLineIsBlank = true;
          buffer="";       
        } 
        else 
          if (c == '\r') {     
          if(buffer.indexOf("GET /?relayon")>=0)
            digitalWrite(relay_pin, LOW);
          if(buffer.indexOf("GET /?relayoff")>=0)
            digitalWrite(relay_pin, HIGH);   
        }
        else {
          currentLineIsBlank = false;
        }  
      }
    }
    client.stop();
  }
}
