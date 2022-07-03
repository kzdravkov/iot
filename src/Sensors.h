#pragma once

#include <functional>
#include <list>
#include <numeric>
#include <MQTTUtils.h>
#include <LEDRestAPI.h>


typedef struct Sensor {
    int pin;
    std::function<long(int inputPin)> readFunction;
    String topic;

    long getAvgValue() {
        long start = millis();
        long counter = 0;

        long val = 0;
        while(millis() - start <= 50) {
            val += readFunction(pin);
            counter++;
        }

        return (long) (val/counter);
    }

    void publish(std::shared_ptr<PubSubClient> pubSubClient) {
        long val = getAvgValue();
        MQTTUtils::publish(pubSubClient, val, topic, 0);
    }
} Sensor;

std::list<Sensor> GLOBAL_SENSORS = {
    // Sensor{ 34, [](int pin) { return analogRead(pin); }, "sensor/analog/moisture"}
    // {Sensor{ 2, [](int pin) { return digitalRead(pin); }, "sensor/digital/led", "LED" },
    Sensor{ 4, [](int pin) { return touchRead(pin); }, "sensor/digital/4"}
};

std::map<String, std::function<void(String)>> GLOBAL_HANDLERS = {
    {"control/led", [&](String message) {
        if(std::stoi(message.c_str()))
            LEDRestAPI::led(1);
        else
            LEDRestAPI::led(0);
    }}
};