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
            /* Authmode threshold resets to WPA2 as default if password matches WPA2 standards (password len => 8).
             * If you want to connect the device to deprecated WEP/WPA networks, Please set the threshold value
             * to WIFI_AUTH_WEP/WIFI_AUTH_WPA_PSK and set the password with length and format matching to
             * WIFI_AUTH_WEP/WIFI_AUTH_WPA_PSK standards.
             */
            // .threshold.authmode = WIFI_AUTH_WPA_WPA2_PSK,
            // .sae_pwe_h2e = WPA3_SAE_PWE_HUNT_AND_PECK,
            // .sae_h2e_identifier = "",
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