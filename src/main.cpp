/*
 * Blink
 * Turns on an LED on for one second,
 * then off for one second, repeatedly.
 */

#include <Arduino.h>
#include <WiFiManager.h>
#include <LEDRestAPI.h>
#include <WiFiRestAPI.h>
#include <esp_task_wdt.h>
#include <AsyncMqttClient.hpp>
#include <ESPAsyncWebServer.h>
#include <Sensors.h>
#include <list>
#include <cstdio>

std::shared_ptr<AsyncWebServer> webServer;
std::shared_ptr<WiFiManager> wifiManager;
AsyncMqttClient mqttClient;

std::list<Sensor> sensors;

void setup() {
    sensors.push_back(
      Sensor(
        SensorDescriptor {34, [&](int pin) { return analogRead(pin); }, "sensor/analog/moisture", "Moisture Sensor"}));

    Serial.begin(115200);
    Serial.setDebugOutput(true);

    LITTLEFS.begin();    

    wifiManager = std::shared_ptr<WiFiManager>(new WiFiManager(10000));
    wifiManager -> autoConnect();

    webServer = std::shared_ptr<AsyncWebServer>(new AsyncWebServer(8080));
    webServer -> begin();

    LEDRestAPI::registerHandlers(webServer);
    WiFiRestAPI::registerHandlers(webServer, wifiManager);

    mqttClient.setServer("localhost", 3030);
    mqttClient.connect();
}

void loop() {
    for (auto sensor : sensors) {
        int val = sensor.getAvgValue(10);
        char payload[sizeof(val) + sizeof(char)];
        std::sprintf(payload, "%d", val);

        mqttClient.publish(
            sensor.getDescriptor().topic_.c_str(),
            0, /*qos*/
            false, /*retain*/
            payload);
    }
}