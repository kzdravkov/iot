#pragma once

#include <functional>
#include "Arduino.h"
#include <list>

typedef struct SensorDescriptor {
    int pin_;
    std::function<int(int inputPin)> readFunction_;
    String topic_;
    String description_;
} SensorDescriptor;

class Sensor {
    std::list<int> values;

    private:
        SensorDescriptor sensorDescriptor_;

    public:
        Sensor(SensorDescriptor sensorDescriptor) {
            sensorDescriptor_ = sensorDescriptor;
        }

        int getAvgValue(int samples) {
            if (values.size() <= samples) {
                values.push_back(sensorDescriptor_.readFunction_(sensorDescriptor_.pin_));

                delay(100);
                return getAvgValue(samples);
            } else
                return std::accumulate(values.begin(), values.end(), 0.0) / values.size();
        }

        SensorDescriptor getDescriptor() {
            return sensorDescriptor_;
        }
};