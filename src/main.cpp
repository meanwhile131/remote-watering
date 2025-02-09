#include <pinmanager.h>
#include <comms.h>
#include <autowatering.h>
#include <esp_wifi.h>
#include <esp_netif.h>
#include <nvs_flash.h>
#include <esp_log.h>

static const char *TAG = "Main";

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
    ESP_LOGI(TAG, "Initializing NVS...");
    esp_err_t err = nvs_flash_init();
    if (err == ESP_ERR_NVS_NO_FREE_PAGES || err == ESP_ERR_NVS_NEW_VERSION_FOUND)
    {
        ESP_ERROR_CHECK(nvs_flash_erase());
        err = nvs_flash_init();
    }
    ESP_ERROR_CHECK(err);
    ESP_LOGI(TAG, "Starting pin manager...");
    xTaskCreate(runPins, "Pin manager", 4096, NULL, tskIDLE_PRIORITY + 1, NULL);
    ESP_LOGI(TAG, "Setting up WiFi...");
    esp_wifi_set_storage(WIFI_STORAGE_RAM);
    ESP_ERROR_CHECK(esp_netif_init());
    ESP_ERROR_CHECK(esp_event_loop_create_default());
    esp_netif_create_default_wifi_sta();
    wifi_init_config_t init_wifi_config = WIFI_INIT_CONFIG_DEFAULT();
    ESP_ERROR_CHECK(esp_wifi_init(&init_wifi_config));
    wifi_config_t wifi_config = {};
    strlcpy((char *)wifi_config.sta.ssid, "H", 32);
    strlcpy((char *)wifi_config.sta.password, "qazwsxedc", 64);
    ESP_ERROR_CHECK(esp_event_handler_instance_register(WIFI_EVENT,
                                                        ESP_EVENT_ANY_ID,
                                                        &event_handler,
                                                        NULL,
                                                        NULL));
    ESP_ERROR_CHECK(esp_wifi_set_config(WIFI_IF_STA, &wifi_config));
    ESP_ERROR_CHECK(esp_wifi_start());
    ESP_ERROR_CHECK_WITHOUT_ABORT(esp_wifi_connect());
    ESP_LOGI(TAG, "Starting autowatering...");
    xTaskCreate(runAutoWatering, "Autowatering", 2048, NULL, tskIDLE_PRIORITY + 1, NULL);
    ESP_LOGI(TAG, "Starting comms...");
    runComms();
}