#pragma once

#include <AsyncTCP.h>
#include <ESPAsyncWebServer.h>
#include <WiFiManager.h>
#include <string>
#include <memory>

class WiFiRestAPI {
    public:
        static void registerHandlers(std::shared_ptr<AsyncWebServer> server, std::shared_ptr<WiFiManager> manager) {
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

                    if(manager->sta(ssid, password)) {
                        request->send(200, "application/json", "message:\"Wi-Fi successfully connected!\"");
                    } else {
                        request->send(503, "application/json", "message:\"Wi-Fi connection failed!\"");
                    }
                });

            server->on(
                "/wifi",
                HTTP_GET,
                [&](AsyncWebServerRequest *request) {
                    String joinedList;
                    for (WiFiNetwork network : manager->scanNetworks()) {
                        joinedList+=network.toString() + "\n";
                    }

                    request->send(200, "application/json", "message:\"" + joinedList + "\"");
                });

            server->on(
                "/wifi",
                HTTP_DELETE,
                [&](AsyncWebServerRequest *request) {
                    if(request->hasHeader("ssid")) {
                        String ssid = request->getHeader("ssid")->value();
                        if(manager->clearKey(ssid)) {
                            request->send(200, "application/json", "message:\"Key " + ssid + " cleared successfully!\"");
                        } else {
                            request->send(503, "application/json", "message:\"Key " + ssid + " not found!\"");
                        }
                    } else {
                        if(manager->clearKeys()) {
                            request->send(200, "application/json", "message:\"Keys cleared successfully!\"");
                        } else {
                            request->send(500);
                        }
                    }
                });
        }
};