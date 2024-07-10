#include <Arduino.h>
#include <AsyncWebSocket.h>
#include <ArduinoJson.h>
#include <AsyncTCP.h>
#include <pinmanager.h>
#include <comms.h>
#include <ESPAsyncWebServer.h>

static const char *TAG = "Comms";

AsyncWebServer server(80);
AsyncWebSocket ws("/ws");

void runComms()
{
	ESP_LOGI(TAG, "Initializing web server...");
	ws.onEvent(onEvent);
	server.addHandler(&ws);
	server.begin();
	ESP_LOGI(TAG, "Web server init done!");
}

void onEvent(AsyncWebSocket *server, AsyncWebSocketClient *client, AwsEventType type, void *arg, uint8_t *m, size_t len)
{
	if (type == WS_EVT_CONNECT)
	{

		ESP_LOGI(TAG, "Client connected!");
		JsonDocument message;
		for (size_t i = 0; i < 10; i++)
		{
			message[String(i)] = on[i];
		}
		String msg;
		serializeJson(message, msg);
		client->text(msg);
	}
	else if (type == WS_EVT_DISCONNECT)
	{
		ESP_LOGI(TAG, "Client disconnected!");
	}
	else if (type == WS_EVT_DATA)
	{
		JsonDocument message;
		deserializeJson(message, (char *)m);
		for (size_t i = 0; i < 10; i++)
		{
			if (message.containsKey(String(i)))
				setPinState(i, message[String(i)]);
		}
	}
}

void textAll(JsonDocument message)
{
	String msg;
	serializeJson(message, msg);
	ESP_LOGI(TAG, "Sending %s...\n", msg);
	ws.textAll(msg);
}