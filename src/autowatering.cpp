#include <Arduino.h>
#include <time.h>
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

void autoWatering(void *)
{
    for (;;)
    {
        if (on[8])
        {
            struct tm timeInfo;
            getLocalTime(&timeInfo);
            waterIfNeeded(timeInfo, 23, 0, 0);
            waterIfNeeded(timeInfo, 23, 2, 1);
            waterIfNeeded(timeInfo, 23, 4, 2);
            waterIfNeeded(timeInfo, 23, 6, 3);
            waterIfNeeded(timeInfo, 23, 8, 4);
            waterIfNeeded(timeInfo, 23, 10, 5);
            waterIfNeeded(timeInfo, 23, 12, 6);
            waterIfNeeded(timeInfo, 23, 14, 7);
        }
        delay(1000);
    }
}

void waterIfNeeded(tm timeInfo, int tm_hour, int tm_min, int pin)
{
    if (timeInfo.tm_hour == tm_hour && timeInfo.tm_min == tm_min && !on[pin])
        setPinState(pin, 1);
}