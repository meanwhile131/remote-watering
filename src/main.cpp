#include <pinmanager.h>
#include <comms.h>
#include <autowatering.h>
#include <esp_wifi.h>
#include <esp_netif.h>
#include <Print.h>

static void event_handler(void *arg, esp_event_base_t event_base,
                          int32_t event_id, void *event_data)
{
    if (event_base == WIFI_EVENT && (event_id == WIFI_EVENT_STA_START || event_id == WIFI_EVENT_STA_DISCONNECTED))
    {
        esp_wifi_connect();
    }
}

extern "C" void app_main()
{

    xTaskCreate(runPins, "Pin manager", ESP_TASK_MAIN_STACK, NULL, 1, NULL);
    esp_netif_init();
    esp_event_loop_create_default();
    esp_netif_create_default_wifi_sta();
    wifi_init_config_t init_wifi_config = WIFI_INIT_CONFIG_DEFAULT();
    esp_wifi_init(&init_wifi_config);
    esp_wifi_set_storage(WIFI_STORAGE_RAM);
    wifi_config_t wifi_config = {
        .sta = {
            {.ssid = "H1"},
            {.password = "qazwsxedc"},
        },
    };
    esp_event_handler_instance_register(WIFI_EVENT,
                                        ESP_EVENT_ANY_ID,
                                        &event_handler,
                                        NULL,
                                        NULL);
    esp_wifi_set_config(WIFI_IF_STA, &wifi_config);
    esp_wifi_start();
    esp_wifi_connect();
    xTaskCreate(runAutoWatering, "Autowatering", ESP_TASK_MAIN_STACK, NULL, tskIDLE_PRIORITY, NULL);
    runComms();
}