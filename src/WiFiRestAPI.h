#pragma once

#include <AsyncTCP.h>
#include <ESPAsyncWebServer.h>
#include <WiFiManager.h>
#include <string>

class WiFiRestAPI {
    public:
        static void registerHandlers(AsyncWebServer* server, std::shared_ptr<WiFiManager> manager) {
            server->on(
                "/wifi",
                HTTP_POST,
                [&](AsyncWebServerRequest *request) {
                    String ssid;
                    String password;

                    if(request->hasHeader("ssid")) {
                        ssid = request->getHeader("ssid")->value();
                    } else {
                        request->send(301, "test/json", "message:\"Missing ssid\"");
                        return;
                    }

                    if(request->hasHeader("password")) {
                        password = request->getHeader("password")->value();
                    }

                    request->send(200, "application/json", "message:\"Wi-Fi successfully configured\"");
                    manager->sta(ssid, password);
                });

            server->on(
                "/wifi",
                HTTP_GET,
                [&](AsyncWebServerRequest *request) {
                    String joinedList;
                    for (WiFiNetwork network : manager->scanNetworks()) {
                        joinedList+=network.toString() + "\n";
                    }

                    request->send(200, "appclication/json", "message:\"" + joinedList + "\"");
                });

            server->on(
                "/wifi",
                HTTP_DELETE,
                [&](AsyncWebServerRequest *request) {
                    if(manager->clearKeys()) {
                        request->send(200, "application/json", "message:\"Keys cleared successfully\"");
                    } else {
                        request->send(500);
                    }
                });
        }
};