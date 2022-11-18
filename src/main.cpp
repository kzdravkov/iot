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
String DEVICE_NAME = "ESP32-1";
boolean QUEUE_MESSAGES = true;


PubSubClientWrapper* pubSubClient;
std::shared_ptr<AsyncWebServer> webServer;
WiFiManager* wifiManager;

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

    LITTLEFS.begin(true);

    esp_wifi_set_ps(WIFI_PS_NONE);
}

void setup() {
    initESP();
    
    webServer = std::shared_ptr<AsyncWebServer>(new AsyncWebServer(WEB_SERVER_PORT));

    registerWebServer();

    wifiManager = new WiFiManager();

    pubSubClient = new PubSubClientWrapper(
        "ec9402f7d1fb4d5b9087f63a81c0aaff.s1.eu.hivemq.cloud",
        8883,
        "kzdravkov",
        "qfn2Veqm",
        DEVICE_NAME,
        QUEUE_MESSAGES);
}

void loop() {
    while(WiFi.status() != WL_CONNECTED && !wifiManager->autoConnect()) {
        vTaskDelay(5000);
    }

    pubSubClient->loop();
}