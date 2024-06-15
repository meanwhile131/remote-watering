#include <Arduino.h>
#include <WiFi.h>
#include <ArduinoOTA.h>
#include <time.h>
#include <pinmanager.h>
#include <comms.h>

void WiFiDisconnected(WiFiEvent_t event, WiFiEventInfo_t info);
void autoWatering(void *);

void setup()
{
    initPins();
    WiFi.setHostname("plant-watering");
    WiFi.begin("H1", "qazwsxedc");
    WiFi.onEvent(WiFiDisconnected, WiFiEvent_t::ARDUINO_EVENT_WIFI_STA_DISCONNECTED);
    Serial.begin(115200);
    configTime(14400, 0, "pool.ntp.org");
    ArduinoOTA.begin();
    initComms();
    xTaskCreate(autoWatering, "Autowatering", 2048, NULL, 1, NULL);
}

void loop()
{
    ArduinoOTA.handle();
    handlePins();
}

void WiFiDisconnected(WiFiEvent_t event, WiFiEventInfo_t info)
{
    Serial.print("WiFi lost connection: ");
    Serial.println(info.wifi_sta_disconnected.reason);
    WiFi.reconnect();
}

void autoWatering(void *)
{
    for (;;)
    {
        struct tm timeInfo;
        getLocalTime(&timeInfo);
        if (timeInfo.tm_hour == 23 && timeInfo.tm_min == 0 && !on[0] && !on[1] && !on[2])
        {
            setPinState(0, 1);
            setPinState(1, 1);
            setPinState(2, 1);
        }
        if (timeInfo.tm_hour == 23 && timeInfo.tm_min == 2 && !on[3] && !on[4] && !on[5] && !on[6])
        {
            setPinState(3, 1);
            setPinState(4, 1);
            setPinState(5, 1);
            setPinState(6, 1);
        }
        if (timeInfo.tm_hour == 23 && timeInfo.tm_min == 4 && !on[7])
        {
            setPinState(7, 1);
        }
        delay(100);
    }
}