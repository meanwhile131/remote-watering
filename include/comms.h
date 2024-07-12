#include <ArduinoJson.h>
#include <esp_http_server.h>

void runComms();
esp_err_t websocket_handler(httpd_req_t *req);
esp_err_t ota_handler(httpd_req_t *req);
void textAll(JsonDocument message);
