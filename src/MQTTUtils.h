#pragma once

#include <functional>
#include <list>
#include <numeric>
#include <AsyncMqttClient.hpp>

#include <PubSubClient.h>
#include <SSLClient.h>

class MQTTUtils {
    public:
        static void publish(std::shared_ptr<AsyncMqttClient> mqttClient, long val, String topic, int qos) {
            char payload[sizeof(val) + sizeof(char)];
            std::sprintf(payload, "%d", val);

            log_i("publish %d in %s", val, topic.c_str());

            mqttClient->publish(
                topic.c_str(),
                qos, /*qos*/
                false, /*retain*/
                payload);
        }

        static void onMessage(std::shared_ptr<AsyncMqttClient> mqttClient, String topic, int qos, std::function<void(String msg)> action) {
            mqttClient->subscribe(topic.c_str(), qos);
            mqttClient->onMessage([action](
                char* topic,
                char* payload,
                AsyncMqttClientMessageProperties properties,
                size_t len,
                size_t index,
                size_t total) {
                    String msg;

                    for (int i = 0; i < len; i++) {
                        msg += (char)payload[i];
                    }

                    log_i("read topic %s, message %s", topic, msg);

                    action(msg);
                });

        }
};