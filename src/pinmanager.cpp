#include <iot_servo.h>
#include <ArduinoJson.h>
#include <pinmanager.h>
#include <comms.h>
#include <esp_task.h>
#include <nvs_flash.h>
#include <esp_log.h>

static const char *TAG = "Pin manager";

bool lastButtonState[8];
bool on[10];
TaskHandle_t turnOffTaskHandles[8];
servo_config_t servo_cfg = {
	.max_angle = 180,
	.min_width_us = 544,
	.max_width_us = 2400,
	.freq = 50,
	.timer_number = LEDC_TIMER_0,
	.channels = {
		.servo_pin = {
			GPIO_NUM_13,
		},
		.ch = {
			LEDC_CHANNEL_0,
		},
	},
	.channel_number = 1,
};
nvs_handle_t nvs;

void runPins(void *)
{
	ESP_LOGI(TAG, "Opening NVS namespace...");
	ESP_ERROR_CHECK(nvs_open("state", NVS_READWRITE, &nvs));
	ESP_LOGI(TAG, "Setting up pins...");
	gpio_config_t gpio_conf = {};

	gpio_conf.mode = GPIO_MODE_OUTPUT;
	gpio_conf.pin_bit_mask = 1ULL << 16 | 1ULL << 17 | 1ULL << 18 | 1ULL << 19 | 1ULL << 21 | 1ULL << 22 | 1ULL << 23 | 1ULL << 25 | 1ULL << 4;
	ESP_ERROR_CHECK(gpio_config(&gpio_conf));

	gpio_conf.mode = GPIO_MODE_INPUT;
	gpio_conf.pin_bit_mask = 1ULL << 26 | 1ULL << 27 | 1ULL << 32 | 1ULL << 33 | 1ULL << 34 | 1ULL << 35 | 1ULL << 36 | 1ULL << 39;
	gpio_conf.pull_down_en = GPIO_PULLDOWN_ENABLE;
	ESP_ERROR_CHECK(gpio_config(&gpio_conf));

	ESP_LOGI(TAG, "Reading pin states from NVS...");
	uint8_t autoState, waterState;
	nvs_get_u8(nvs, "auto", &autoState);
	setPinState(8, autoState);
	nvs_get_u8(nvs, "water", &waterState);
	setPinState(9, waterState);

	ESP_LOGI(TAG, "Init done! Starting button handler!");

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
		nvs_set_u8(nvs, "auto", state);
		nvs_commit(nvs);
	}
	else if (pin == 9)
	{
		nvs_set_u8(nvs, "water", state);
		nvs_commit(nvs);
		ESP_ERROR_CHECK_WITHOUT_ABORT(iot_servo_init(LEDC_LOW_SPEED_MODE, &servo_cfg));
		ESP_ERROR_CHECK_WITHOUT_ABORT(iot_servo_write_angle(LEDC_LOW_SPEED_MODE, LEDC_CHANNEL_0, state ? 180 : 40));
		xTaskCreate(releaseWaterServo, "releaseServo", ESP_TASK_MAIN_STACK, NULL, tskIDLE_PRIORITY + 1, NULL);
	}
	JsonDocument message;
	message[std::to_string(pin)] = state;
	textAll(message);
}

void turnOffAfterTime(void *pin)
{
	vTaskDelay(((int)pin != 1 ? WATER_TIME : WATER_TIME_LONG) / portTICK_PERIOD_MS);
	turnOffTaskHandles[(int)pin] = NULL;
	setPinState((int)pin, 0);
	vTaskDelete(NULL);
}

void releaseWaterServo(void *)
{
	vTaskDelay(3000 / portTICK_PERIOD_MS);
	ESP_ERROR_CHECK_WITHOUT_ABORT(iot_servo_deinit(LEDC_LOW_SPEED_MODE));
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