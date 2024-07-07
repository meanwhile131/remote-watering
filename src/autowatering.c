#include <Arduino.h>
#include "time.h"
#include <pinmanager.h>
#include <autowatering.h>

#define ENABLE

void initAutoWatering()
{
#ifdef ENABLE
    configTime(14400, 0, "pool.ntp.org");
    xTaskCreate(autoWatering, "Autowatering", 2048, NULL, 1, NULL);
#endif
}

void autoWatering(void *param)
{
    for (;;)
    {
        if (on[8])
        {
            struct tm timeInfo;
            getLocalTime(&timeInfo);
            waterIfNeeded(timeInfo, 10, 35, 0);
            waterIfNeeded(timeInfo, 10, 37, 1);
            waterIfNeeded(timeInfo, 10, 39, 2);
            waterIfNeeded(timeInfo, 10, 41, 3);
            waterIfNeeded(timeInfo, 10, 43, 4);
            waterIfNeeded(timeInfo, 10, 45, 5);
            waterIfNeeded(timeInfo, 10, 47, 6);
            waterIfNeeded(timeInfo, 10, 49, 7);
        }
        delay(1000);
    }
}

void waterIfNeeded(tm timeInfo, int tm_hour, int tm_min, int pin)
{
    if (timeInfo.tm_hour == tm_hour && timeInfo.tm_min == tm_min && !on[pin])
        setPinState(pin, 1);
}