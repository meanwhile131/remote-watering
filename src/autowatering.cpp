#include <Arduino.h>
#include <pinmanager.h>
#include <autowatering.h>

static const char *TAG = "Autowatering";

void runAutoWatering(void *)
{
    ESP_LOGI(TAG, "Setting up NTP...");
    configTime(14400, 0, "pool.ntp.org");
    ESP_LOGI(TAG, "NTP setup done! Starting loop...");
    for (;;)
    {
        if (on[8])
        {
            struct tm timeInfo;
            getLocalTime(&timeInfo);
            if (timeInfo.tm_hour == 23 && timeInfo.tm_min == 0)
            {
                setPinState(0, 1);
                vTaskDelay(WATER_TIME / portTICK_PERIOD_MS);
                if (!on[8])
                    continue;
                setPinState(2, 1);
                vTaskDelay(WATER_TIME / portTICK_PERIOD_MS);
                if (!on[8])
                    continue;
                setPinState(3, 1);
                vTaskDelay(WATER_TIME / portTICK_PERIOD_MS);
                if (!on[8])
                    continue;
                setPinState(4, 1);
                vTaskDelay(WATER_TIME / portTICK_PERIOD_MS);
                if (!on[8])
                    continue;
                setPinState(5, 1);
                vTaskDelay(WATER_TIME / portTICK_PERIOD_MS);
                if (!on[8])
                    continue;
                setPinState(6, 1);
                vTaskDelay(WATER_TIME / portTICK_PERIOD_MS);
                if (!on[8])
                    continue;
                setPinState(1, 1);
                vTaskDelay(WATER_TIME_LONG / portTICK_PERIOD_MS);
            }
        }
        vTaskDelay(1000 / portTICK_PERIOD_MS);
    }
}
