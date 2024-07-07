#include <Arduino.h>
#include <pinmanager.h>
#include <comms.h>
#include <autowatering.h>
#include <esp_wifi.h>

void WiFiDisconnected(WiFiEvent_t event, WiFiEventInfo_t info);

void app_main()
{
    initPins();
    WiFi.setHostname("plant-watering");
    WiFi.begin("H1", "qazwsxedc");
    wifi_init_config_t wifi_config = WIFI_INIT_CONFIG_DEFAULT();
    WiFi.onEvent(WiFiDisconnected, WiFiEvent_t::ARDUINO_EVENT_WIFI_STA_DISCONNECTED);
    Serial.begin(115200);
    initAutoWatering();
    initComms();
}

void loop()
{
    handleComms();
    handlePins();
}

void WiFiDisconnected(WiFiEvent_t event, WiFiEventInfo_t info)
{
    Serial.print("WiFi lost connection: ");
    Serial.println(info.wifi_sta_disconnected.reason);
    WiFi.reconnect();
}