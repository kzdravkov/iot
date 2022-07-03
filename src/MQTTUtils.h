#pragma once

#include <functional>
#include <memory>
#include <list>
#include <map>
#include <numeric>
#include <PubSubClient.h>
#include <SSLClient.h>
#include <string>

class MQTTUtils {
    public:

        static std::map<String, std::function<void(char*, uint8_t*, unsigned int)>> mqttHandlers;

        static void publish(std::shared_ptr<PubSubClient> pubSubClient, long val, String topic, int qos) {
            const char* payload = std::to_string(val).c_str();

            log_i("publish %s in %s", payload, topic.c_str());

            pubSubClient->publish(topic.c_str(), payload, false);
        }

        static void registerMQTTHandler() {

        }

        // static void onMessage(std::shared_ptr<PubSubClient> pubSubClient, char* topic, byte* payload, unsigned int length) {
        //     pubSubClient->setCallback
        //     mqttClient->subscribe(topic.c_str(), qos);
        //     mqttClient->onMessage([action](
        //         char* topic,
        //         char* payload,
        //         AsyncMqttClientMessageProperties properties,
        //         size_t len,
        //         size_t index,
        //         size_t total) {
        //             String msg;

        //             for (int i = 0; i < len; i++) {
        //                 msg += (char)payload[i];
        //             }

        //             log_i("read topic %s, message %s", topic, msg);

        //             action(msg);
        //         });

        // }
};