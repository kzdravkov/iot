#include <Arduino.h>
#include <WiFiManager.h>
#include <WiFi.h>
#include <LEDRestAPI.h>
#include <WiFiRestAPI.h>
#include <esp_task_wdt.h>
#include <ESPAsyncWebServer.h>
#include <Sensors.h>
#include <list>
#include <cstdio>
#include <functional>
#include <esp_wifi.h>
#include <MQTTUtils.h>
#include <PubSubClient.h>
#include <SSLClient.h>
#include <PubSubClientWrapper.h>
#include <tuple>

int WEB_SERVER_PORT = 8080;

PubSubClientWrapper* pubSubClient = new PubSubClientWrapper(
    "ec9402f7d1fb4d5b9087f63a81c0aaff.s1.eu.hivemq.cloud",
    8883,
    "kzdravkov",
    "qfn2Veqm",
    "ESP32-1");

std::shared_ptr<AsyncWebServer> webServer = std::shared_ptr<AsyncWebServer>(new AsyncWebServer(WEB_SERVER_PORT));
WiFiManager* wifiManager = new WiFiManager();

void registerWebServer() {
    LEDRestAPI::registerHandlers(webServer);
    WiFiRestAPI::registerHandlers(webServer, wifiManager);

    WiFi.onEvent(
        [&](WiFiEvent_t event, WiFiEventInfo_t info) {
            webServer->begin();
            log_i("WebServer listening on %s:%d", WiFi.softAPIP().toString().c_str(), WEB_SERVER_PORT);
        },
        ARDUINO_EVENT_WIFI_AP_START);
}

void initESP() {
    esp_task_wdt_init(30, false);

    pinMode(2, OUTPUT);

    Serial.begin(115200);
    Serial.setDebugOutput(true);

    LITTLEFS.begin();

    esp_wifi_set_ps(WIFI_PS_NONE);
}

void reconnect (void* arg) {
    // WiFiManager* wifiManager = ((WiFiManager*) arg);
    // while(!wifiManager->autoConnect()) {
    //     vTaskDelay(500);
    // }

    // vTaskDelay(1000);
}

void setup() {
    initESP();

    registerWebServer();

    TaskHandle_t wifiReconnectHandle;

    xTaskCreate(
        reconnect,
        "WiFi-Reconnect",
        8192,
        &wifiManager,
        tskIDLE_PRIORITY,
        &wifiReconnectHandle);

    // wifiManager->setAutoConnectTaskHandle(wifiReconnectHandle);

    // vTaskStartScheduler();
}

void loop() {
    // pubSubClient.loop();
    // vTaskDelay(1000);
}