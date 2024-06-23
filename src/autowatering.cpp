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
            Serial.println("1");
        }
        delay(100);
    }
}