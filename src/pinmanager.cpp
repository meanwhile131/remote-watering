#include <Arduino.h>
#include <ESP32Servo.h>
#include <EEPROM.h>
#include <ArduinoJson.h>
#include <pinmanager.h>
#include <comms.h>

// #define FANTIMER 5000
#define MINUTE 60 * 1000
#define WATER_TIME 1 * MINUTE
#define WATER_TIME_8 10 * MINUTE

#ifdef FANTIMER
unsigned long fanOffTime;
bool fanTurnOff;
#endif
bool lastButtonState[8];
unsigned long turnedOnTime[8];
bool on[10];
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
	else if (pin == 8)
	{
		EEPROM.writeBool(0, state);
		EEPROM.commit();
	}
	else if (pin == 9)
	{
		water.attach(13);
		water.write(state ? 180 : 55);
		EEPROM.writeBool(1, state);
		EEPROM.commit();
	}
	if (on[0] || on[1] || on[2] || on[3] || on[4] || on[5] || on[6] || on[7])
	{
#ifdef FANTIMER
		fanTurnOff = false;
#endif
		digitalWrite(4, 1);
	}
	else
	{
#ifdef FANTIMER
		fanTurnOff = true;
		fanOffTime = millis();
#else
		digitalWrite(4, 0);
#endif
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
	EEPROM.begin(2);
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
	setPinState(9, EEPROM.readBool(1));
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
#ifdef FANTIMER
	if (fanTurnOff && millis() - fanOffTime > FANTIMER)
	{
		fanTurnOff = false;
		digitalWrite(4, 0);
	}
#endif
}