#pragma once

#include <Arduino.h>

class StringUtils {
    public:
        static String fromIntArray(int len, int payload[]) {
            String msg;

            for (int i = 0; i < len; i++)
                msg += (char)payload[i];

            return msg;
        }
};