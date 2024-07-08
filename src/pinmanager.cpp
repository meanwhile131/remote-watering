#include <Arduino.h>
#include <ESP32Servo.h>
#include <ArduinoJson.h>
#include <pinmanager.h>
#include <comms.h>
#include <Preferences.h>

#define MINUTE 60 * 1000
#define WATER_TIME 2 * MINUTE
#define WATER_TIME_8 10 * MINUTE

bool lastButtonState[8];
unsigned long turnedOnTime[8];
bool on[10];
Servo water;
Preferences savedPinState;

void setPinState(int pin, bool state)
{
	on[pin] = state;
	if (pin < 8)
	{
		if (state)
		{
			turnedOnTime[pin] = millis();
		}
		digitalWrite(pin == 0	? 17
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
	else if (pin == 8)
	{
		savedPinState.putBool("auto", state);
	}
	else if (pin == 9)
	{
		savedPinState.putBool("water", state);
		water.attach(13);
		water.write(state ? 180 : 55);
	}
	if (on[0] || on[1] || on[2] || on[3] || on[4] || on[5] || on[6] || on[7])
	{
		digitalWrite(4, 1);
	}
	else
	{
		digitalWrite(4, 0);
	}
	JsonDocument message;
	message[String(pin)] = state;
	textAll(message);
	if (pin == 9)
	{
		delay(3000);
		water.detach();
	}
}

void initPins()
{
	savedPinState.begin("state");
	pinMode(16, OUTPUT);
	pinMode(17, OUTPUT);
	pinMode(18, OUTPUT);
	pinMode(19, OUTPUT);
	pinMode(21, OUTPUT);
	pinMode(22, OUTPUT);
	pinMode(23, OUTPUT);
	pinMode(25, OUTPUT);
	pinMode(26, INPUT_PULLDOWN);
	pinMode(27, INPUT_PULLDOWN);
	pinMode(32, INPUT_PULLDOWN);
	pinMode(33, INPUT_PULLDOWN);
	pinMode(34, INPUT);
	pinMode(35, INPUT);
	pinMode(36, INPUT);
	pinMode(39, INPUT);
	pinMode(4, OUTPUT);
	setPinState(8, savedPinState.getBool("auto"));
	setPinState(9, savedPinState.getBool("water"));
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

void handlePins()
{
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
		if (millis() - turnedOnTime[i] > (i < 7 ? WATER_TIME : WATER_TIME_8) && on[i])
		{
			setPinState(i, 0);
		}
	}
}