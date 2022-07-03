#pragma once

#include <AsyncTCP.h>
#include <ESPAsyncWebServer.h>
#include <memory>
#include <AsyncMqttClient.hpp>
#include <MQTTUtils.h>

static const String LED_TOPIC = "control/led";
static const uint8_t LED_BUILTIN = 2;

class LEDRestAPI {
    public:
        static void registerHandlers(std::shared_ptr<AsyncWebServer> server) {
            server -> on("/ledOn", HTTP_GET, [&](AsyncWebServerRequest *request) {
                led(1);
                request-> send(200, "text/json", "mesage:{LED turned ON}");
            });

            server -> on("/ledOff", HTTP_GET, [&](AsyncWebServerRequest *request) {
                led(0);
                request-> send(200, "text/json", "mesage:{LED turned OFF}"); 
            });
        }

        static void registerMQTT(std::shared_ptr<AsyncMqttClient> mqttClient) {
            // MQTTUtils::onMessage(
            //     mqttClient,
            //     LED_TOPIC,
            //     2,
            //     [](String msg) {
            //         int valNow = digitalRead(LED_BUILTIN);
            //         led(valNow > 0 ? 0 : 1);
            //     }
            // );
        }

        static void led(boolean on) {
            if(on) {
                log_i("Led on");
                digitalWrite(LED_BUILTIN, 1);
            } else {
                log_i("Led off");
                digitalWrite(LED_BUILTIN, 0);
            }
        }
};