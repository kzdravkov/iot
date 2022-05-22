#pragma once

#include <functional>
#include "Arduino.h"
#include <list>
#include <numeric>
#include <AsyncMqttClient.hpp>
#include <MQTTUtils.h>


typedef struct Sensor {
    int pin;
    std::function<long(int inputPin)> readFunction;
    String topic;
    String description;

    long getAvgValue() {
        long start = millis();
        long counter = 0;

        long val;
        while(millis() - start <= 50) {
            val += readFunction(pin);
            counter++;
        }

        return (long) (val/counter);
    }

    void publish(std::shared_ptr<AsyncMqttClient> mqttClient) {
        long val = getAvgValue();
        MQTTUtils::publish(mqttClient, val, topic, 0);
    }
} Sensor;