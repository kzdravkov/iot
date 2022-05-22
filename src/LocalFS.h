#pragma once

#include <Arduino.h>
#include "FS.h"
#include <LITTLEFS.h>

class LocalFS {
    public:
        static bool writeFile(const String path, const String message) {
            Serial.printf("Writing file: %s\n", path.c_str());

            File file = LITTLEFS.open(path, LITTLEFS.exists(path) ? FILE_APPEND : FILE_WRITE);
            if(file) {
                file.print(message);
                Serial.printf("Writing file succeeded: %s\n", path.c_str());
                // file.close();

                return true;
            } else {
                Serial.printf("File write failed %s\n", path.c_str());
                return false;
            }
        }

        static File readFile(const char * path) {
            Serial.printf("Reading file: %s\n", path);

            File file = LITTLEFS.open(path);
            if(!file || file.isDirectory()){
                Serial.println("- failed to open file for reading");
                return File();
            }

            return file;
        }

        static boolean deleteFile(const char * path){
            Serial.printf("Deleting file: %s\n", path);
            
            bool result;
            if((result = LITTLEFS.remove(path))) {
                Serial.println("- file deleted");
            } else {
                Serial.println("- delete failed");
            }

            return result;
        }
};