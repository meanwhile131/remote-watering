#include <Arduino.h>
#include <WiFi.h>
#include <pinmanager.h>
#include <comms.h>
#include <autowatering.h>

void WiFiDisconnected(WiFiEvent_t event, WiFiEventInfo_t info);

void setup()
{
    initPins();
    WiFi.setHostname("plant-watering");
    WiFi.begin("H1", "qazwsxedc");
    WiFi.onEvent(WiFiDisconnected, WiFiEvent_t::ARDUINO_EVENT_WIFI_STA_DISCONNECTED);
    Serial.begin(115200);
    initAutoWatering();
    initComms();
}

void loop()
{
    handleComms();
    handlePins();
    handeAutoWatering();
    delay(100);
}

void WiFiDisconnected(WiFiEvent_t event, WiFiEventInfo_t info)
{
    Serial.print("WiFi lost connection: ");
    Serial.println(info.wifi_sta_disconnected.reason);
    WiFi.reconnect();
}