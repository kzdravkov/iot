#pragma once

#include <memory>
#include <list>
#include <map>
#include <PubSubClient.h>
#include <Arduino.h>
#include <WiFiManager.h>
#include <WiFi.h>
#include <TAs.h>
#include <functional>
#include <Sensors.h>
#include <tuple>

void callback(char* topic, byte* payload, unsigned int len) {
    String msg;

    for (int i = 0; i < len; i++) {
        msg += (char)payload[i];
    }

    log_i("read topic %s, message %s", topic, msg);

    GLOBAL_HANDLERS[topic](msg);
}

class PubSubClientWrapper {
    static PubSubClientWrapper instance;

    std::shared_ptr<PubSubClient> _pubSubClient;

    std::list<TimerHandle_t> sensorTimers;

    String _username;
    String _password;
    String _deviceName;
    boolean _queueMessages;

    long lastSensorPush = 0;

    public:
        PubSubClientWrapper(
                char* hostname,
                int port,
                String username,
                String password,
                String deviceName,
                boolean queueMessages) {
            _username = username;
            _password = password;
            _deviceName = deviceName;
            _queueMessages = queueMessages;

            WiFiClient* wifiClient = new WiFiClient();
            SSLClient* sslClient = new SSLClient(*wifiClient, TAs, (size_t)TAs_NUM, 34);

            _pubSubClient = std::shared_ptr<PubSubClient>(new PubSubClient(*sslClient));
            _pubSubClient->setServer(hostname, port);
            _pubSubClient->setBufferSize(8192);
            _pubSubClient->setCallback(callback);
        }

        void loop() {
                if (!_pubSubClient->connected()) {
                    log_i("MQTT Trying to connect");
                    if(_pubSubClient->connect(
                        _deviceName.c_str(),
                        _username.c_str(),
                        _password.c_str(),
                        NULL, /*willTopic*/
                        0, /*willQos*/
                        false, /*willRetain*/
                        NULL, /*willMessage*/
                        !_queueMessages /*cleanSession*/)) {
                            log_i("MQTT Connected!");
                            for(auto handler : GLOBAL_HANDLERS) {
                                log_i("Subscribing %s", handler.first.c_str());
                                _pubSubClient->subscribe(handler.first.c_str(), 0);
                            }
                    }
                } else {
                    // if(millis() - lastSensorPush > 1000) {
                    //     for(auto sensor : GLOBAL_SENSORS)
                    //         MQTTUtils::publish(_pubSubClient, sensor.getAvgValue(), sensor.topic, 0);

                    //     lastSensorPush = millis();
                    // }

                    _pubSubClient->loop();
                }

        }
};