/*
 * Blink
 * Turns on an LED on for one second,
 * then off for one second, repeatedly.
 */

#include <Arduino.h>
#include <WiFiManager.h>
#include <WebServerREST.h>
#include <LEDRestAPI.h>
#include <WiFiRestAPI.h>
#include <esp_task_wdt.h>
#include <AsyncMqttClient.hpp>
#include <list>
#include <SensorDescriptors.h>

WebServer_* webServer;
std::shared_ptr<WiFiManager> wifiManager;
AsyncMqttClient mqttClient;

Sensor moistureSensor(SensorDescriptor {34, readAnalogLocal, "sensor/analog/34", "Moisture Sensor"});

void setup() {
    pinMode(2, OUTPUT);

    Serial.begin(115200);
    Serial.setDebugOutput(true);

    LITTLEFS.begin();    

    wifiManager = std::shared_ptr<WiFiManager>(new WiFiManager(10000));
    wifiManager -> autoConnect();
    
    webServer = new WebServer_(8080);

    LEDRestAPI::registerHandlers(webServer->getServer());
    WiFiRestAPI::registerHandlers(webServer->getServer(), wifiManager);

    mqttClient.setServer("localhost", 3030);
    mqttClient.connect();
}

void loop() {
    mqttClient.publish(moistureSensor.getAvgValue());
}

int readAnalogLocal(int pin) {
  return analogRead(pin);
}
