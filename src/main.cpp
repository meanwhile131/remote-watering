#include <Arduino.h>
#include <WiFi.h>
#include <AsyncTCP.h>
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
    // xTaskCreate(autoWatering, "Autowatering", 2048, NULL, 1, NULL);
}

void loop()
{
    ArduinoOTA.handle();
    buttonPressHandler(26, 7);
    buttonPressHandler(27, 6);
    buttonPressHandler(32, 5);
    buttonPressHandler(33, 4);
    buttonPressHandler(34, 3);
    buttonPressHandler(35, 2);
    buttonPressHandler(36, 1);
    buttonPressHandler(39, 0);
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
        if (timeInfo.tm_hour == 13 && timeInfo.tm_min == 6 && !on[2])
        {
            setPinState(2, 1);
        }
        yield();
    }
}
