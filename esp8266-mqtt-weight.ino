#include <FS.h> 
#include<ESP8266WiFi.h>

#include <DNSServer.h>
#include <ESP8266WebServer.h>
#include <WiFiManager.h>         //https://github.com/tzapu/WiFiManager

#include <PubSubClient.h>
#include "HX711.h"

#include<ArduinoJson.h>  //ArduinoJson6

#include <ESP8266mDNS.h>
#include <WiFiUdp.h>
#include <ArduinoOTA.h>

const char* ApName = "Esp-Mqtt-Weight-2";
const char* topics = "/weight/2";
byte point = 2;
String json ="";
IPAddress server(192, 168, 77, 9);
const int LOADCELL_DOUT_PIN = D1;
const int LOADCELL_SCK_PIN = D2;
long  weight = 0;
HX711 scale;
WiFiClient wclient;
PubSubClient client(wclient, server);
//DynamicJsonDocument doc(1024);
StaticJsonDocument<256> doc;

void setup() {
  Serial.begin(115200);
  
  delay(10);
  scale.begin(LOADCELL_DOUT_PIN, LOADCELL_SCK_PIN);
  delay(10);
  
  WiFiManager wifimanager;
  wifimanager.setBreakAfterConfig(true);
  if (!wifimanager.autoConnect(ApName)) {
    Serial.println("failed to connect, we should reset as see if it connects");
    delay(3000);
    ESP.reset();
    delay(5000);
  }


   ArduinoOTA.onStart([]() {
    String type;
    if (ArduinoOTA.getCommand() == U_FLASH) {
      type = "sketch";
    } else { // U_SPIFFS
      type = "filesystem";
    }

    // NOTE: if updating SPIFFS this would be the place to unmount SPIFFS using SPIFFS.end()
    Serial.println("Start updating " + type);
  });
  ArduinoOTA.onEnd([]() {
    Serial.println("\nEnd");
  });
  ArduinoOTA.onProgress([](unsigned int progress, unsigned int total) {
    Serial.printf("Progress: %u%%\r", (progress / (total / 100)));
  });
  ArduinoOTA.onError([](ota_error_t error) {
    Serial.printf("Error[%u]: ", error);
    if (error == OTA_AUTH_ERROR) {
      Serial.println("Auth Failed");
    } else if (error == OTA_BEGIN_ERROR) {
      Serial.println("Begin Failed");
    } else if (error == OTA_CONNECT_ERROR) {
      Serial.println("Connect Failed");
    } else if (error == OTA_RECEIVE_ERROR) {
      Serial.println("Receive Failed");
    } else if (error == OTA_END_ERROR) {
      Serial.println("End Failed");
    }
  });
  ArduinoOTA.begin();
  
  Serial.println("Connected!");
  Serial.print("local ip:");
  Serial.println(WiFi.localIP());
  
  

}

void loop() {
  if(WiFi.status()!=WL_CONNECTED){
     delay(3000);
     ESP.reset();
     delay(5000);
  }
  if(WiFi.status()==WL_CONNECTED){
    if(!client.connected()){
      if(client.connect(ApName)){
        Serial.println("MQTT conected!");
      }
    }
    if(client.connected()){
      client.loop();
    }
   
  }

//  DynamicJsonDocument doc(1024);
  doc["position"] = point;
  doc["weight"] = readweight();
  serializeJson(doc,json);
  client.publish(topics,json);
  Serial.println(json);
  json = "";
  
  ArduinoOTA.handle();
  
}

long readweight(){
      if (scale.is_ready()) {
      weight  = scale.read();
      Serial.print("HX711 reading: ");
      Serial.println(weight);
    } else {
      weight = 0;
      Serial.println("HX711 not found.");
    }
  
    delay(100);
    return weight;
  }
