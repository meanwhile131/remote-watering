#include <Arduino.h>
#include <ESP32Servo.h>
#include <ArduinoJson.h>
#include <pinmanager.h>
#include <comms.h>
#include <Preferences.h>
#include <esp_task.h>
#include <nvs_flash.h>

static const char *TAG = "Pin manager";

bool lastButtonState[8];
bool on[10];
Servo water;
Preferences savedPinState;

void runPins(void *)
{
	ESP_LOGI(TAG, "Setting up flash memory...");
	savedPinState.begin("state");
	ESP_LOGI(TAG, "Setting up pins...");
	gpio_set_direction(16, GPIO_MODE_OUTPUT);
	gpio_set_direction(17, GPIO_MODE_OUTPUT);
	gpio_set_direction(18, GPIO_MODE_OUTPUT);
	gpio_set_direction(19, GPIO_MODE_OUTPUT);
	gpio_set_direction(21, GPIO_MODE_OUTPUT);
	gpio_set_direction(22, GPIO_MODE_OUTPUT);
	gpio_set_direction(23, GPIO_MODE_OUTPUT);
	gpio_set_direction(25, GPIO_MODE_OUTPUT);
	gpio_set_direction(4, GPIO_MODE_OUTPUT);

	gpio_set_direction(26, GPIO_MODE_INPUT);
	gpio_set_pull_mode(26, GPIO_PULLDOWN_ENABLE);
	gpio_set_direction(27, GPIO_MODE_INPUT);
	gpio_set_pull_mode(27, GPIO_PULLDOWN_ENABLE);
	gpio_set_direction(32, GPIO_MODE_INPUT);
	gpio_set_pull_mode(32, GPIO_PULLDOWN_ENABLE);
	gpio_set_direction(33, GPIO_MODE_INPUT);
	gpio_set_pull_mode(33, GPIO_PULLDOWN_ENABLE);

	gpio_set_direction(34, GPIO_MODE_INPUT);
	gpio_set_direction(35, GPIO_MODE_INPUT);
	gpio_set_direction(36, GPIO_MODE_INPUT);
	gpio_set_direction(39, GPIO_MODE_INPUT);

	setPinState(8, savedPinState.getBool("auto"));
	// water.attach(13);

	setPinState(9, savedPinState.getBool("water"));
	ESP_LOGI(TAG, "Pin init done! Starting button handler!");
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
	ESP_LOGI(TAG, "Setting state of pin %i to %d", pin, state);
	on[pin] = state;
	if (pin < 8)
	{
		gpio_set_level(pin == 0	  ? 17
					   : pin == 1 ? 25
					   : pin == 2 ? 19
					   : pin == 3 ? 21
					   : pin == 4 ? 22
					   : pin == 5 ? 23
					   : pin == 6 ? 18
					   : pin == 7 ? 16
								  : -1,
					   on[pin]);
		gpio_set_level(4, on[0] || on[1] || on[2] || on[3] || on[4] || on[5] || on[6] || on[7]);
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
	delay((int)pin != 1 ? WATER_TIME : WATER_TIME_LONG);
	if (on[pin])
		setPinState((int)pin, 0);
	vTaskDelete(NULL);
}

void setWaterState(void *state)
{
	on[9] = (bool)state;
	water.write((bool)state ? 180 : 40);
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