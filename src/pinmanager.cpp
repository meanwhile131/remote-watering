#include <Arduino.h>
#include <ESP32Servo.h>
#include <ArduinoJson.h>
#include <pinmanager.h>
#include <comms.h>
#include <Preferences.h>
#include <nvs_flash.h>

bool lastButtonState[8];
bool on[10];
Servo water;
Preferences savedPinState;

void runPins(void *)
{
	nvs_flash_init();
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
	water.attach(13);
	setPinState(9, savedPinState.getBool("water"));
	printf("Pin init done");
	for (;;)
	{
		buttonPressHandler(26, 7);
		buttonPressHandler(27, 6);
		buttonPressHandler(32, 5);
		buttonPressHandler(33, 4);
		buttonPressHandler(34, 3);
		buttonPressHandler(35, 2);
		buttonPressHandler(36, 1);
		buttonPressHandler(39, 0);
		delay(100);
	}
}

void setPinState(int pin, bool state)
{
	printf("Setting pin state...\n");
	on[pin] = state;
	if (pin < 8)
	{
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
		digitalWrite(4, on[0] || on[1] || on[2] || on[3] || on[4] || on[5] || on[6] || on[7]);
		if (state)
		{
			xTaskCreate(turnOffAfterTime, "pinTurnOff", ESP_TASK_MAIN_STACK, (void *)pin, tskIDLE_PRIORITY + 1, NULL);
		}
	}
	else if (pin == 8)
	{
		savedPinState.putBool("auto", state);
	}
	else if (pin == 9)
	{
		savedPinState.putBool("water", state);
		xTaskCreate(setWaterState, "setWaterState", ESP_TASK_MAIN_STACK, (void *)state, tskIDLE_PRIORITY + 1, NULL);
	}
	JsonDocument message;
	message[String(pin)] = state;
	textAll(message);
}

void turnOffAfterTime(void *pin)
{
	delay((int)pin != 1 ? WATER_TIME : WATER_TIME_8);
	setPinState((int)pin, 0);
	vTaskDelete(NULL);
}

void setWaterState(void *state)
{
	on[9] = (bool)state;
	water.write((bool)state ? 180 : 55);
	delay(3000);
	water.release();
	vTaskDelete(NULL);
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