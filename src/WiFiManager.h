#pragma once

#include <Arduino.h>
#include <Wifi.h>
#include <LITTLEFS.h>

#include <stdlib.h>
#include <list>
#include <map>

#include <LocalFS.h>

struct WiFiNetwork {
    String SSID;
    int RSSI;
    wifi_auth_mode_t auth;

    String toString() {
        return SSID + " " + RSSI + " " + auth;
    }
};

class WiFiManager {
    private:
        TimerHandle_t reconnectTimer;

        boolean connect() {
            int timeout = 10000;
            unsigned long startTime = millis();

            /* Wait until WiFi connection but do not exceed MAX_CONNECT_TIME */
            while (WiFi.status() != WL_CONNECTED && (millis() - startTime) < timeout) {
                vTaskDelay(100);
            }

            if(WiFi.status() == WL_CONNECTED) {
                log_i("Connection Successful! Your device IP address is %s", WiFi.localIP().toString());

                return true;
            } else {
                log_i("Connection FAILED!: %s", WiFi.status());

                return false;
            }
        }

        void disconnect() {
            /* Clear previous modes. */
            WiFi.softAPdisconnect();
            WiFi.disconnect();
        }

        std::map<String, String> readKeysFile() {
            File file = LocalFS::readFile("/wifi.txt");
            std::map<String, String> keys;

            while(file.available()) {
                String SSID = file.readStringUntil(';');
                String password = file.readStringUntil('\n');
                keys.insert(std::make_pair(SSID, password));
            }

            file.close();

            return keys;
        }

        /* Scan available networks and sort them in order to their signal strength. */
        String getStrongestNetwork(boolean open) {
            std::list<WiFiNetwork> networks = scanNetworks();
            for(WiFiNetwork network : networks) {
                if(open && network.auth != 0)
                    continue;
                else if (!open && network.auth == 0)
                    continue;
                else
                    return network.SSID;
            }

            return "";
        }

    public:
        WiFiManager(int MAX_CONNECT_TIME) {
            // reconnectTimer = xTimerCreate(
            //     "wifiReconnectTimer",
            //     pdMS_TO_TICKS(2000),
            //     pdFALSE,
            //     (void*)0,
            //     reinterpret_cast<TimerCallbackFunction_t>(autoConnect));
        }

        ~WiFiManager() {
            disconnect();
        }

        void autoConnect() {
            auto keys = readKeysFile();

            for (WiFiNetwork network : scanNetworks()) {
                if(network.auth != 0) {
                    auto key = keys.find(network.SSID);
                    if (key != keys.end()) {
                        if (sta(key->first, key->second))
                            return;
                    }
                }
            }

            softAP();
        }

        bool clearKeys() {
            return LocalFS::deleteFile("/wifi.txt");
        }

        String connectToStrongestOpen() {
            String SSID = getStrongestNetwork(true/*open*/);
            if(SSID = "") {
                Serial.println("No open networks discovered");
            } else {
                sta(SSID, "");
            }

            return SSID;
        }

        std::list<WiFiNetwork> scanNetworks() {
            Serial.print("Scan networks\n");

            int n = WiFi.scanNetworks();

            int indices[n];
            for (int i = 0; i < n; i++) {
                indices[i] = i;
            }

            for (int i = 0; i < n; i++)
                for (int j = i + 1; j < n; j++)
                    if (WiFi.RSSI(indices[j]) > WiFi.RSSI(indices[i]))
                        std::swap(indices[i], indices[j]);

            std::list<WiFiNetwork> networks;

            for (int i = 0; i < n; ++i) {
                WiFiNetwork network;

                network.SSID = WiFi.SSID(indices[i]);
                network.RSSI = WiFi.RSSI(indices[i]);
                network.auth = WiFi.encryptionType(indices[i]);

                networks.push_back(network);
            }

            for(WiFiNetwork network : networks) {
                Serial.println(network.toString());
            }

            return networks;
        }

        bool sta(String SSID, String password) {
            WiFi.mode(WIFI_STA);

            Serial.println("Connecting to " + SSID);

            if(password == emptyString) {
                WiFi.begin(SSID.c_str());
            } else {
                WiFi.begin(SSID.c_str(), password.c_str());
            }

            bool connected = connect();
            if(connected) {
                auto keys = readKeysFile();

                if(!keys.count(SSID)) {
                    String s = SSID + ";" + password + "\n";
                    LocalFS::writeFile("/wifi.txt", s);
                }
            }

            return connected;
        }

        void softAP() {
            disconnect();

            const char* ssid = "ESP32-Access-Point";
            const char* password = "123456789";

            if(WiFi.softAP(ssid, password)) {
                Serial.println("AP IP address: " + WiFi.softAPIP().toString());
            }
        }
};