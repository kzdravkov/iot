#include <AsyncTCP.h>
#include <ESPAsyncWebServer.h>
#include <stdlib.h>

const u_long TIMEOUT_MS = 2000;
const int8_t LED_BUILTIN = 2;

class WebServer_ {
    private:
        AsyncWebServer* _server;

    public:
        WebServer_(int port) {
            _server = new AsyncWebServer(port);

            _server -> begin();

            Serial.println("WebServer listening on port ");
            Serial.println(port);
        }

        AsyncWebServer* getServer() {
            return _server;
        }
};