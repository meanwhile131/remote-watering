#include <Arduino.h>
#include <WiFi.h>
#include <AsyncTCP.h>
#include <ArduinoOTA.h>
#include <ArduinoJson.h>
#include <AsyncWebSocket.h>
#include <ESP32Servo.h>
#include <time.h>
#include <EEPROM.h>

AsyncWebServer server(80);
AsyncWebSocket ws("/ws");
Servo water;
bool on[9];
// bool fanTurnOff;
unsigned long turnedOnTime[8];
// unsigned long fanOffTime;

bool lastButtonState[8];
void WiFiDisconnected(WiFiEvent_t event, WiFiEventInfo_t info);
void onEvent(AsyncWebSocket *server, AsyncWebSocketClient *client, AwsEventType type, void *arg, uint8_t *data, size_t len);
void buttonPressHandler(int pin, int button);
void setPinState(int pin, bool state);
void autoWatering(void *);

void setup()
{
    EEPROM.begin(1);
    pinMode(16, OUTPUT);
    pinMode(17, OUTPUT);
    pinMode(18, OUTPUT);
    pinMode(19, OUTPUT);
    pinMode(21, OUTPUT);
    pinMode(22, OUTPUT);
    pinMode(23, OUTPUT);
    pinMode(25, OUTPUT);
    pinMode(LED_BUILTIN, OUTPUT);
    pinMode(26, INPUT_PULLDOWN);
    pinMode(27, INPUT_PULLDOWN);
    pinMode(32, INPUT_PULLDOWN);
    pinMode(33, INPUT_PULLDOWN);
    pinMode(34, INPUT);
    pinMode(35, INPUT);
    pinMode(36, INPUT);
    pinMode(39, INPUT);
    pinMode(4, OUTPUT);
    WiFi.setHostname("plant-watering");
    WiFi.begin("H1", "qazwsxedc");
    WiFi.onEvent(WiFiDisconnected, WiFiEvent_t::ARDUINO_EVENT_WIFI_STA_DISCONNECTED);
    Serial.begin(115200);
    ws.onEvent(onEvent);
    server.addHandler(&ws);
    server.begin();
    configTime(14400, 0, "pool.ntp.org");

    ArduinoOTA.begin();
    // xTaskCreate(autoWatering, "Autowatering", 2048, NULL, 1, NULL);
    setPinState(8, EEPROM.readBool(0));
}

void loop()
{
    ArduinoOTA.handle();
    buttonPressHandler(26, 7);
    buttonPressHandler(27, 6);
    buttonPressHandler(32, 5);
    buttonPressHandler(33, 4);
    buttonPressHandler(34, 3);
    buttonPressHandler(35, 2);
    buttonPressHandler(36, 1);
    buttonPressHandler(39, 0);
    for (size_t i = 0; i < 8; i++)
    {
        if (millis() - turnedOnTime[i] > 65000 && on[i])
        {
            setPinState(i, 0);
        }
    }
    // if (fanTurnOff && millis() - fanOffTime > 3000)
    // {
    //     fanTurnOff = false;
    //     digitalWrite(4, 0);
    // }
}

void WiFiDisconnected(WiFiEvent_t event, WiFiEventInfo_t info)
{
    Serial.print("WiFi lost connection: ");
    Serial.println(info.wifi_sta_disconnected.reason);
    WiFi.reconnect();
}

void autoWatering(void *)
{
    for (;;)
    {
        struct tm timeInfo;
        getLocalTime(&timeInfo);
        if (timeInfo.tm_hour == 13 && timeInfo.tm_min == 6 && !on[2])
        {
            setPinState(2, 1);
        }
        yield();
    }
}

void buttonPressHandler(int pin, int button)
{
    if (digitalRead(pin) && !lastButtonState[button])
    {
        lastButtonState[button] = true;
        setPinState(button, !on[button]);
    }
    if (!digitalRead(pin))
        lastButtonState[button] = false;
}

void setPinState(int pin, bool state)
{
    on[pin] = state;
    if (pin < 8)
    {
        if (state)
        {
            turnedOnTime[pin] = millis();
        }
        digitalWrite(pin == 0   ? 17
                     : pin == 1 ? 25
                     : pin == 2 ? 19
                     : pin == 3 ? 21
                     : pin == 4 ? 22
                     : pin == 5 ? 23
                     : pin == 6 ? 18
                     : pin == 7 ? 16
                                : -1,
                     on[pin]);
    }
    else
    {
        water.attach(13);
        water.write(state ? 180 : 55);
        EEPROM.writeBool(0, state);
        EEPROM.commit();
    }
    if (on[0] || on[1] || on[2] || on[3] || on[4] || on[5] || on[6] || on[7])
    {
        // fanTurnOff = false;
        digitalWrite(4, 1);
    }
    else
    {
        // fanTurnOff = true;
        // fanOffTime = millis();
        digitalWrite(4, 0);
    }
    JsonDocument message;
    message[String(pin)] = state;
    String msg;
    serializeJson(message, msg);
    ws.textAll(msg);
    if (pin == 8)
    {
        delay(3000);
        water.detach();
    }
}

void onEvent(AsyncWebSocket *server, AsyncWebSocketClient *client, AwsEventType type, void *arg, uint8_t *m, size_t len)
{
    if (type == WS_EVT_CONNECT)
    {

        Serial.println("Client connected!");
        JsonDocument message;
        for (size_t i = 0; i < 9; i++)
        {
            message[String(i)] = on[i];
        }
        String msg;
        serializeJson(message, msg);
        client->text(msg);
    }
    else if (type == WS_EVT_DISCONNECT)
    {
        Serial.println("Client disconnected!");
    }
    else if (type == WS_EVT_DATA)
    {
        JsonDocument message;
        deserializeJson(message, (char *)m);
        for (size_t i = 0; i < 9; i++)
        {
            if (message.containsKey(String(i)))
                setPinState(i, message[String(i)]);
        }
    }
}
