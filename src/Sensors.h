#pragma once

#include <functional>
#include "Arduino.h"
#include <list>
#include <numeric>

typedef struct SensorDescriptor {
    int pin_;
    std::function<int(int inputPin)> readFunction_;
    String topic_;
    String description_;
} SensorDescriptor;

class Sensor {
    private:
        SensorDescriptor sensorDescriptor_;

    public:
        Sensor(SensorDescriptor sensorDescriptor) {
            sensorDescriptor_ = sensorDescriptor;
        }

        int getAvgValue() {
            long start = millis();
            long counter = 0;

            long val;
            while(millis() - start <= 1000) {
                val += sensorDescriptor_.readFunction_(sensorDescriptor_.pin_);
                counter++;
            }

            return (int) (val/counter);
        }

        SensorDescriptor getDescriptor() {
            return sensorDescriptor_;
        }
};