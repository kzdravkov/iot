#pragma once

#include <Arduino.h>
#include <Wifi.h>

#include <stdlib.h>
#include <list>
#include <map>
#include <iostream>

#include <LocalFS.h>
#include <LEDRestAPI.h>
#include <LITTLEFS.h>
#include <StringUtils.h>

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
        volatile boolean isConnecting;

        boolean waitForConnection();
        void disconnect();
        std::map<String, String> readKeysFile();
        String getStrongestNetwork(boolean open);
    public:
        WiFiManager();
        boolean autoConnect();
        bool clearKeys();
        bool clearKey(String SSID);
        std::list<WiFiNetwork> scanNetworks();
        String connectToStrongestOpen();
        boolean sta(String SSID, String password);
        void staAsync(String SSID, String password);
};

WiFiManager::WiFiManager() {
    WiFi.onEvent(
        [this](WiFiEvent_t event, WiFiEventInfo_t info) {
            if(event == ARDUINO_EVENT_WIFI_STA_DISCONNECTED) {
                LEDRestAPI::led(0);

                if(info.wifi_sta_disconnected.reason == 23 ||     // 802_1X_AUTH_FAILED
                    info.wifi_sta_disconnected.reason == 202) {   // AUTH_FAIL
                    clearKey(
                        StringUtils::fromIntArray(
                            info.wifi_sta_disconnected.ssid_len,
                            (int*) info.wifi_sta_disconnected.ssid));
                }

                autoConnect();
            } else if(event == ARDUINO_EVENT_WIFI_STA_CONNECTED) {
                LEDRestAPI::led(1);
            }
        });
}

boolean WiFiManager::waitForConnection() {
    int timeout = 10000;
    unsigned long startTime = millis();

    /* Wait until WiFi connection but do not exceed MAX_CONNECT_TIME */
    while (WiFi.status() != WL_CONNECTED && (millis() - startTime) < timeout) {
        vTaskDelay(500);
    }

    return WiFi.status() == WL_CONNECTED;
}

void WiFiManager::disconnect() {
    /* Clear previous modes. */
    WiFi.softAPdisconnect();
    WiFi.disconnect();
}

std::map<String, String> WiFiManager::readKeysFile() {
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
String WiFiManager::getStrongestNetwork(boolean open) {
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

boolean WiFiManager::autoConnect() {
    WiFi.mode(WIFI_AP_STA);
    WiFi.softAP("ESP32-Access-Point", "123456789");

    auto keys = readKeysFile();
    if(keys.size() == 0)
        return false;

    while(true) {
        for (WiFiNetwork network : scanNetworks()) {
            if(network.auth != 0) {
                auto key = keys.find(network.SSID);
                if (key != keys.end()) {
                    return sta(key->first, key->second);
                }
            }
        }

        vTaskDelay(1000);
    }
}

bool WiFiManager::clearKeys() {
    return LocalFS::deleteFile("/wifi.txt");
}

boolean WiFiManager::clearKey(String SSID) {
    auto keys = readKeysFile();

    if((keys.find(SSID)) == keys.end())
        return false;

    String s;
    for(auto key : keys)
        if(key.first != SSID)
            s = s + key.first + ";" + key.second + "\n";

    clearKeys();

    LocalFS::writeFile("/wifi.txt", s);

    return true;
}

String WiFiManager::connectToStrongestOpen() {
    String SSID = getStrongestNetwork(true/*open*/);
    if(SSID = "") {
        Serial.println("No open networks discovered");
    } else {
        staAsync(SSID, "");
    }

    return SSID;
}

std::list<WiFiNetwork> WiFiManager::scanNetworks() {
    int n = WiFi.scanNetworks();

    if(n == WIFI_SCAN_RUNNING || n == WIFI_SCAN_FAILED) {
        vTaskDelay(1000);
        scanNetworks();
    }

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

boolean WiFiManager::sta(String SSID, String password) {
    if(password == emptyString) {
        WiFi.begin(SSID.c_str());
    } else {
        WiFi.begin(SSID.c_str(), password.c_str());
    }

    boolean success = waitForConnection();
    if(success) {
        auto keys = readKeysFile();

        if(!keys.count(SSID)) {
            String s = SSID + ";" + password + "\n";
            LocalFS::writeFile("/wifi.txt", s);
        } 
    }

    return success;
}
