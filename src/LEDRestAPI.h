#pragma once

#include <AsyncTCP.h>
#include <ESPAsyncWebServer.h>
#include <memory>

class LEDRestAPI {
    public:
        static const uint8_t LED_BUILTIN = 2;
        static void registerHandlers(std::shared_ptr<AsyncWebServer> server) {
            pinMode(LED_BUILTIN, OUTPUT);

            server -> on("/", HTTP_GET, [&](AsyncWebServerRequest *request) { 
                log_i("Hello World");
                request-> send(200, "text/html", "Hello");
            });

            server -> on("/ledOn", HTTP_GET, [&](AsyncWebServerRequest *request) { 
                log_i("Led on");
                digitalWrite(LED_BUILTIN, 1);
                request-> send(200, "text/json", "mesage:{LED turned ON}");
            });

            server -> on("/ledOff", HTTP_GET, [&](AsyncWebServerRequest *request) {
                log_i("Led off");
                digitalWrite(LED_BUILTIN, 0);
                request-> send(200, "text/json", "mesage:{LED turned OFF}"); 
            });
        }
};