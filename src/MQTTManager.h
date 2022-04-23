#pragma once

#include <AsyncMqttClient.hpp>
#include <stdlib.h>
#include <memory>

class MQTTManager {
    public:
        std::shared_ptr<AsyncMqttClient> client;

        MQTTManager(String hostname, int port, String topic) {
            client = std::shared_ptr<AsyncMqttClient>(new AsyncMqttClient);
            client->onConnect([&](bool sessionPresent) {
                client->subscribe(topic.c_str(), 2);
            });
        }
};