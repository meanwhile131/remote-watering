#include <Arduino.h>
#include <pinmanager.h>
#include <comms.h>
#include <autowatering.h>
#include <esp_wifi.h>

void WiFiDisconnected(WiFiEvent_t event, WiFiEventInfo_t info);

extern "C" void app_main()
{
    Serial.begin(115200);
    Serial.println("serial");
    initPins();
    Serial.println("pins");
    wifi_init_config_t init_wifi_config = WIFI_INIT_CONFIG_DEFAULT();
    esp_wifi_init(&init_wifi_config);
    wifi_config_t wifi_config = {
        .sta = {
            {.ssid = "H1"},
            {.password = "qazwsxedc"},
        },
    };
    ESP_ERROR_CHECK(esp_wifi_set_mode(WIFI_MODE_STA));
    ESP_ERROR_CHECK(esp_wifi_set_config(WIFI_IF_STA, &wifi_config));
    ESP_ERROR_CHECK(esp_wifi_start());
    ESP_ERROR_CHECK(esp_wifi_connect());

    Serial.println("wifi");
    // WiFi.onEvent(WiFiDisconnected, WiFiEvent_t::ARDUINO_EVENT_WIFI_STA_DISCONNECTED);
    Serial.println("wifi2");

    initAutoWatering();
    Serial.println("auto");
    initComms();
    Serial.println("comms");
    for (;;)
    {
        handleComms();
        handlePins();
        delay(50);
    }
}

void WiFiDisconnected(WiFiEvent_t event, WiFiEventInfo_t info)
{
    Serial.print("WiFi lost connection: ");
    Serial.println(info.wifi_sta_disconnected.reason);
    WiFi.reconnect();
}