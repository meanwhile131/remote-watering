#include <Arduino.h>
#include <ESP32Servo.h>
#include <EEPROM.h>
#include <ArduinoJson.h>
#include <pinmanager.h>

bool lastButtonState[8];
unsigned long turnedOnTime[8];
unsigned long fanOffTime;
bool fanTurnOff;
bool on[9];

Servo water;

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
	else
	{
		water.attach(13);
		water.write(state ? 180 : 55);
		EEPROM.writeBool(0, state);
		EEPROM.commit();
	}
	if (on[0] || on[1] || on[2] || on[3] || on[4] || on[5] || on[6] || on[7])
	{
		fanTurnOff = false;
		digitalWrite(4, 1);
	}
	else
	{
		fanTurnOff = true;
		fanOffTime = millis();
	}
	JsonDocument message;
	message[String(pin)] = state;
	if (pin == 8)
	{
		delay(3000);
		water.detach();
	}
}

void initPins()
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
	setPinState(8, EEPROM.readBool(0));
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
	for (size_t i = 0; i < 8; i++)
	{
		if (millis() - turnedOnTime[i] > (i < 7 ? 120000 : 600000) && on[i])
		{
			setPinState(i, 0);
		}
	}
	if (fanTurnOff && millis() - fanOffTime > 3000)
	{
		fanTurnOff = false;
		digitalWrite(4, 0);
	}
}