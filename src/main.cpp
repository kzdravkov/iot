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
#include <esp_wifi.h>
#include <MQTTUtils.h>
#include <PubSubClient.h>
#include <SSLClient.h>
#include <TAs.h>

std::shared_ptr<AsyncWebServer> webServer;
std::shared_ptr<WiFiManager> wifiManager;
std::shared_ptr<AsyncMqttClient> mqttClient;
std::shared_ptr<PubSubClient> pubSubClient;

std::map<String, Sensor> sensors = {
    {"Moisture", Sensor{ 34, [](int pin) { return analogRead(pin); }, "sensor/analog/moisture", "Moisture Sensor" }},
    {"LED", Sensor{ 2, [](int pin) { return digitalRead(pin); }, "sensor/digital/led", "LED" }},
    {"Touch", Sensor{ 4, [](int pin) { return touchRead(pin); }, "sensor/digital/4", "LED" }},
};

std::list<TimerHandle_t> sensorTimers;

void registerWebServer() {
    int port = 8080;
    webServer = std::shared_ptr<AsyncWebServer>(new AsyncWebServer(port));

    LEDRestAPI::registerHandlers(webServer);
    WiFiRestAPI::registerHandlers(webServer, wifiManager);

    WiFi.onEvent(
        [&](WiFiEvent_t event, WiFiEventInfo_t info) {
            webServer->begin();
            log_i("WebServer listening on port %d", port);
        },
        ARDUINO_EVENT_WIFI_AP_START);

    WiFi.onEvent(
        [&](WiFiEvent_t event, WiFiEventInfo_t info) {
            webServer->end();
            log_i("Closing webserver");
        },
        ARDUINO_EVENT_WIFI_STA_DISCONNECTED);
}

void registerMQTTManager() {
    WiFiClient wifiClient;
    SSLClient sslClient(wifiClient, TAs, (size_t)TAs_NUM, 34);
    pubSubClient = std::shared_ptr<PubSubClient>(new PubSubClient(sslClient));
    pubSubClient->setServer("ec9402f7d1fb4d5b9087f63a81c0aaff.s1.eu.hivemq.cloud", 8883);
    pubSubClient->connect("ESP32-1", "kzdravkov", "qfn2Veqm");

    // mqttClient->setServer(IPAddress(192, 168, 1, 103), 1883);

    sensorTimers = {
        // xTimerCreate(
        //     sensors["Moisture"].description.c_str(),
        //     1000,
        //     pdTRUE,
        //     0,
        //     [](TimerHandle_t xTimer) { sensors["Moisture"].publish(mqttClient); }),
        // xTimerCreate(
        //     sensors["LED"].description.c_str(),
        //     1000,
        //     pdTRUE,
        //     0,
        //     [](TimerHandle_t xTimer) { sensors["LED"].publish(mqttClient); }),
        // xTimerCreate(
        //     sensors["Touch"].description.c_str(),
        //     1000,
        //     pdTRUE,
        //     0,
        //     [](TimerHandle_t xTimer) { sensors["Touch"].publish(mqttClient); }),
        // xTimerCreate(
        //     "Touch Control",
        //     1000,
        //     pdTRUE,
        //     0,
        //     [](TimerHandle_t xTimer) {
        //         log_i("avg value %d", sensors["Touch"].getAvgValue());
        //         if(sensors["Touch"].getAvgValue() < 50)
        //             MQTTUtils::publish(mqttClient, 1, LED_TOPIC, 2);
        //     }
        // )
    };

    mqttClient->onConnect([] (bool session) {
        log_i("MQTT connected");

        LEDRestAPI::registerMQTT(mqttClient);

        for(TimerHandle_t handle : sensorTimers)
            xTimerStart(handle, 0);
    });

    mqttClient->onDisconnect([] (AsyncMqttClientDisconnectReason reason) {
        log_i("MQTT Disconnected! Reason: %d", reason);

        for(TimerHandle_t handle : sensorTimers)
            xTimerStop(handle, 0);

        vTaskDelay(1000);
        mqttClient->connect();
    });

    WiFi.onEvent(
        [&](WiFiEvent_t event, WiFiEventInfo_t info) {
            mqttClient->connect();
        },
        ARDUINO_EVENT_WIFI_STA_GOT_IP);

    WiFi.onEvent(
        [&](WiFiEvent_t event, WiFiEventInfo_t info) {
            mqttClient->disconnect(true);
        },
        ARDUINO_EVENT_WIFI_STA_DISCONNECTED);
}

void initESP() {
    esp_task_wdt_init(30, false);

    pinMode(2, OUTPUT);

    Serial.begin(115200);
    Serial.setDebugOutput(true);

    LITTLEFS.begin();

    esp_wifi_set_ps(WIFI_PS_NONE);
}

void setup() {
    initESP();

    registerWebServer();
    registerMQTTManager();

    wifiManager = std::shared_ptr<WiFiManager>(new WiFiManager());
    wifiManager -> autoConnect();
}

void loop() {
}