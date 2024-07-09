#include <Arduino.h>
#include <pinmanager.h>
#include <autowatering.h>

void runAutoWatering(void *)
{
    configTime(14400, 0, "pool.ntp.org");
    for (;;)
    {
        if (on[8])
        {
            struct tm timeInfo;
            getLocalTime(&timeInfo);
            if (timeInfo.tm_hour == 23 && timeInfo.tm_min == 0)
            {
                setPinState(0, 1);
                delay(WATER_TIME);
                if (!on[8])
                    continue;
                setPinState(2, 1);
                delay(WATER_TIME);
                if (!on[8])
                    continue;
                setPinState(3, 1);
                delay(WATER_TIME);
                if (!on[8])
                    continue;
                setPinState(4, 1);
                delay(WATER_TIME);
                if (!on[8])
                    continue;
                setPinState(5, 1);
                delay(WATER_TIME);
                if (!on[8])
                    continue;
                setPinState(6, 1);
                delay(WATER_TIME);
                if (!on[8])
                    continue;
                setPinState(1, 1);
            }
        }
        delay(1000);
    }
}
