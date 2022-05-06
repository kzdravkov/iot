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
#include <esp_task_wdt.h>
#include <list>
#include <cstdio>

std::shared_ptr<AsyncWebServer> webServer;
std::shared_ptr<WiFiManager> wifiManager;
std::shared_ptr<AsyncMqttClient> mqttClient;

void setup() {
    esp_task_wdt_init(30, false);

    Serial.begin(115200);
    Serial.setDebugOutput(true);

    LITTLEFS.begin();

    wifiManager = std::shared_ptr<WiFiManager>(new WiFiManager(10000));
    wifiManager -> autoConnect();

    webServer = std::shared_ptr<AsyncWebServer>(new AsyncWebServer(8080));
    webServer -> begin();

    LEDRestAPI::registerHandlers(webServer);
    WiFiRestAPI::registerHandlers(webServer, wifiManager);

    std::list<Sensor> sensors;
    sensors.push_back(
        Sensor(
            SensorDescriptor {34, [](int pin) { return analogRead(pin); }, "sensor/analog/moisture", "Moisture Sensor"}));

    sensors.push_back(
        Sensor(
            SensorDescriptor {2, [](int pin) { pinMode(pin, OUTPUT); return digitalRead(pin); }, "sensor/digital/led", "LED"}));

    mqttClient = std::shared_ptr<AsyncMqttClient>(new AsyncMqttClient());

    mqttClient->onConnect([sensors] (bool session) {
        log_i("connected");

        while(true) {
            for (auto sensor : sensors) {
                int val = sensor.getAvgValue();
                char payload[sizeof(val) + sizeof(char)];
                std::sprintf(payload, "%d", val);

                log_i("publish %d in %s", val, sensor.getDescriptor().topic_);

                mqttClient->publish(
                    sensor.getDescriptor().topic_.c_str(),
                    0, /*qos*/
                    false, /*retain*/
                    payload);
            }

            vTaskDelay(1000);
        }
    });

    mqttClient->setServer(IPAddress(192, 168, 1, 6), 3031);
    mqttClient->onDisconnect([] (AsyncMqttClientDisconnectReason reason) { log_i("%s, reason"); });
    mqttClient->connect();
}

void loop() {
}