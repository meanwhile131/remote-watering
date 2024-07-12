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
TaskHandle_t turnOffTaskHandles[8];
Servo water;
Preferences savedPinState;

void runPins(void *)
{
	ESP_LOGI(TAG, "Setting up flash memory...");
	savedPinState.begin("state");
	ESP_LOGI(TAG, "Setting up pins...");
	gpio_config_t gpio_conf = {};

	gpio_conf.mode = GPIO_MODE_OUTPUT;
	gpio_conf.pin_bit_mask = 1ULL << 16 | 1ULL << 17 | 1ULL << 18 | 1ULL << 19 | 1ULL << 21 | 1ULL << 22 | 1ULL << 23 | 1ULL << 25 | 1ULL << 4;
	ESP_ERROR_CHECK(gpio_config(&gpio_conf));

	gpio_conf.mode = GPIO_MODE_INPUT;
	gpio_conf.pin_bit_mask = 1ULL << 26 | 1ULL << 27 | 1ULL << 32 | 1ULL << 33 | 1ULL << 34 | 1ULL << 35 | 1ULL << 36 | 1ULL << 39;
	gpio_conf.pull_down_en = GPIO_PULLDOWN_ENABLE;
	ESP_ERROR_CHECK(gpio_config(&gpio_conf));

	setPinState(8, savedPinState.getBool("auto"));
	water.attach(13);
	setPinState(9, savedPinState.getBool("water"));
	ESP_LOGI(TAG, "Pin init done! Starting button handler!");
	for (;;)
	{
		buttonPressHandler(GPIO_NUM_26, 7);
		buttonPressHandler(GPIO_NUM_27, 6);
		buttonPressHandler(GPIO_NUM_32, 5);
		buttonPressHandler(GPIO_NUM_33, 4);
		buttonPressHandler(GPIO_NUM_34, 3);
		buttonPressHandler(GPIO_NUM_35, 2);
		buttonPressHandler(GPIO_NUM_36, 1);
		buttonPressHandler(GPIO_NUM_39, 0);
		vTaskDelay(100 / portTICK_PERIOD_MS);
	}
}

void setPinState(int pin, bool state)
{
	ESP_LOGI(TAG, "Setting state of pin %i to %d", pin, state);
	on[pin] = state;
	if (pin < 8)
	{
		gpio_set_level(pin == 0	  ? GPIO_NUM_17
					   : pin == 1 ? GPIO_NUM_25
					   : pin == 2 ? GPIO_NUM_19
					   : pin == 3 ? GPIO_NUM_21
					   : pin == 4 ? GPIO_NUM_22
					   : pin == 5 ? GPIO_NUM_23
					   : pin == 6 ? GPIO_NUM_18
					   : pin == 7 ? GPIO_NUM_16
								  : GPIO_NUM_NC,
					   on[pin]);
		gpio_set_level(GPIO_NUM_4, on[0] || on[1] || on[2] || on[3] || on[4] || on[5] || on[6] || on[7]);
		if (state)
		{
			xTaskCreate(turnOffAfterTime, "pinTurnOff", ESP_TASK_MAIN_STACK, (void *)pin, tskIDLE_PRIORITY + 1, &turnOffTaskHandles[pin]);
		}
		else if (turnOffTaskHandles[pin] != NULL)
		{
			vTaskDelete(turnOffTaskHandles[pin]);
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
	vTaskDelay(((int)pin != 1 ? WATER_TIME : WATER_TIME_LONG) / portTICK_PERIOD_MS);
	setPinState((int)pin, 0);
	vTaskDelete(NULL);
}

void setWaterState(void *state)
{
	on[9] = (bool)state;
	water.write((bool)state ? 180 : 40);
	vTaskDelay(3000 / portTICK_PERIOD_MS);
	water.release();
	vTaskDelete(NULL);
}

void buttonPressHandler(gpio_num_t pin, int button)
{
	bool state = gpio_get_level(pin);
	if (!state)
		lastButtonState[button] = false;
	else if (!lastButtonState[button])
	{
		lastButtonState[button] = true;
		setPinState(button, !on[button]);
	}
}