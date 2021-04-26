#include "DHT.h"
#include <PubSubClient.h>
#include <WiFi.h>
 
#define WIFI_AP "UIT Public"
#define WIFI_PASSWORD ""
 
#define TOKEN "dht22_token_mydevice"
 
#define DHTPIN 4
#define DHTTYPE DHT11
 
char thingsboardServer[] = "demo.thingsboard.io";
 
WiFiClient wifiClient;
 
 
DHT dht(DHTPIN, DHTTYPE);
 
PubSubClient client(wifiClient);
 
int status = WL_IDLE_STATUS;
unsigned long lastSend;
 
void setup(){
  Serial.begin(115200);
  dht.begin();
  delay(10);
  InitWiFi();
  client.setServer(thingsboardServer,1883);
  lastSend = 0;
}
 
void loop(){
  if(!client.connected()){
    reconnect();
  }
  if(millis() - lastSend > 1000){
    getAndSendTemperatureAndHumidityData();
    lastSend = millis();
  }
  client.loop();
}
void getAndSendTemperatureAndHumidityData(){
  Serial.println("Collecting temperature data.");
  float h = dht.readHumidity();
  float t = dht.readTemperature();
//  if (isnan(h) || isnan(t)) {
//    Serial.println("Failed to read from DHT sensor!");
//    return;
//  }
  Serial.print("Humidity: ");
  Serial.print(h);
  Serial.print(" %\t");
  Serial.print("Temperature: ");
  Serial.print(t);
  Serial.print(" *C ");
  
  String temperature = String(t);
  String humidity = String(h);
 
  Serial.print( "Sending temperature and humidity : [" );
  Serial.print( temperature ); Serial.print( "," );
  Serial.print( humidity );
  Serial.print( "]->" );
 
  String payload = "{";
  payload += "\"temperature\":"; payload += temperature; payload += ",";
  payload += "\"humidity\":"; payload += humidity;
  payload += "}";
 
  char telemetry[100];
  payload.toCharArray( telemetry, 100 );
  client.publish( "v1/devices/me/telemetry", telemetry );
  Serial.println( telemetry);
  Serial.println(lastSend);
}
void InitWiFi(){
  Serial.println("Connecting to AP ...");
  WiFi.begin(WIFI_AP, WIFI_PASSWORD);
  while (WiFi.status() != WL_CONNECTED){
    delay(500);
    Serial.print(".");
  }
  Serial.println("Connected to AP");
}
void reconnect(){
  while (!client.connected()){
    status = WiFi.status();
    if(status != WL_CONNECTED){
      WiFi.begin(WIFI_AP, WIFI_PASSWORD);
      while(WiFi.status() != WL_CONNECTED){
        delay(500);
        Serial.print(".");
      }
      Serial.println("Connected to AP");
    }
    Serial.print("Connecting to ThingsBoard node ...");
    if ( client.connect("ESP8266 Device", TOKEN, NULL)){
      Serial.println( "[DONE]" );
    }
    else {
      Serial.print("[FAILED] [ rc = " );
      Serial.print(client.state() );
      Serial.println( " : retrying in 5 seconds]");
      delay( 5000 );
    }
  }
}
