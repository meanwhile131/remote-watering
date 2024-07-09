#include <AsyncWebSocket.h>
#include <ArduinoJson.h>

void runComms(void *);
void onEvent(AsyncWebSocket *server, AsyncWebSocketClient *client, AwsEventType type, void *arg, uint8_t *data, size_t len);
void textAll(JsonDocument message);
