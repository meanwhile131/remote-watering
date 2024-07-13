#include <Arduino.h>
#include <pinmanager.h>
#include <autowatering.h>
#include <esp_sntp.h>

static const char *TAG = "Autowatering";

void runAutoWatering(void *)
{
    ESP_LOGI(TAG, "Setting up NTP...");
    setenv("TZ", "UTC-4", 1);
    tzset();
    esp_sntp_setoperatingmode(ESP_SNTP_OPMODE_POLL);
    esp_sntp_setservername(0, "pool.ntp.org");
    esp_sntp_init();
    ESP_LOGI(TAG, "NTP setup done! Starting loop...");
    for (;;)
    {
        if (on[8])
        {
            time_t now;
            struct tm timeinfo;
            time(&now);
            localtime_r(&now, &timeinfo);
            // if (timeinfo.tm_hour == 23 && timeinfo.tm_min == 0)
            // {
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
            // }
        }
        vTaskDelay(1000 / portTICK_PERIOD_MS);
    }
}
