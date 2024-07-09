#include <Arduino.h>
#include <AsyncWebSocket.h>
#include <ArduinoJson.h>
#include <AsyncTCP.h>
#include <pinmanager.h>
#include <comms.h>
#include <ESPAsyncWebServer.h>

AsyncWebServer server(80);
AsyncWebSocket ws("/ws");

void runComms()
{
	ws.onEvent(onEvent);
	server.addHandler(&ws);
	server.begin();
}

void onEvent(AsyncWebSocket *server, AsyncWebSocketClient *client, AwsEventType type, void *arg, uint8_t *m, size_t len)
{
	printf("Comms event happened\n");
	if (type == WS_EVT_CONNECT)
	{

		printf("Client connected!\n");
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
		printf("Client disconnected!n");
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
	printf("Sending stuff...\n");
	String msg;
	serializeJson(message, msg);
	ws.textAll(msg);
}