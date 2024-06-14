#include <Arduino.h>
#include <WiFi.h>
#include <AsyncTCP.h>
#include <ArduinoOTA.h>
#include <time.h>
#include <pinmanager.h>
#include <comms.h>

void buttonPressHandler(int pin, int button);
void setPinState(int pin, bool state);
void autoWatering(void *);

void setup()
{

    initPins();
    WiFi.setHostname("plant-watering");
    WiFi.begin("H1", "qazwsxedc");
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
