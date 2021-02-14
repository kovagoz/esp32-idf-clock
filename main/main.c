#include <stdio.h>
#include <time.h>
#include <freertos/FreeRTOS.h>
#include <freertos/portmacro.h>
#include <freertos/projdefs.h>
#include <freertos/task.h>
#include <esp_err.h>
#include <esp_log.h>

#include "clock/button.h"
#include "clock/dht.h"
#include "clock/display.h"
#include "clock/ntp.h"
#include "clock/timer.h"
#include "clock/wifi.h"

// Short messages for 7-segment display
#define MSG_BOOT 0x7c,0x5c,0x5c,0x78
#define MSG_SYNC 0x6d,0x66,0x54,0x39
#define MSG_FAIL 0x71,0x77,0x06,0x38
#define MSG_FINE 0x71,0x06,0x54,0x79

// Write data to the active buffer and display it immediately.
#define DISPLAY_FLASH(msg) {                 \
		buffer_t __data = { msg };           \
		display_write(0, __data);            \
		display_select(0);                   \
	}

#define WITH_THREE_BUFFERS 3

#define DISPLAY_BUFFER_CLOCK 0
#define DISPLAY_BUFFER_TEMP 1
#define DISPLAY_BUFFER_HUMIDITY 2

#define SECOND 1000000
#define TEN_SECONDS 10000000

#define BUTTON_PIN CONFIG_CLOCK_BUTTON_PIN

static const char *TAG = "clock";

TaskHandle_t task = NULL;

static void update_time()
{
	time_t systime = time(NULL);
	struct tm *now = localtime(&systime);

	display_write_time(DISPLAY_BUFFER_CLOCK, now->tm_hour, now->tm_min);
}

static void update_temperature()
{
	float temperature = 0;
    float humidity    = 0;

	if (dht_read(&humidity, &temperature) == ESP_OK) {
		display_write_celsius(DISPLAY_BUFFER_TEMP, temperature);
		display_write_percent(DISPLAY_BUFFER_HUMIDITY, humidity);
	} else {
		ESP_LOGE(TAG, "Could not read data from DHT22");
	}
}

static void IRAM_ATTR button_handler(void *arg)
{
	uint8_t pin = (uint8_t) arg;

	if (pin == BUTTON_PIN) {
		vTaskNotifyGiveFromISR(task, NULL);
	}
}

static void switch_display_task(void *pvParameters)
{
	for (;;) {
		ulTaskNotifyTake(pdTRUE, portMAX_DELAY);
		ESP_LOGI(TAG, "Buffer switch request received");
		display_select_next();
	}
}

void app_main()
{
	ESP_LOGI(TAG, "Setting timezone");
	timezone_set(CLOCK_TZ_EUROPE_BUDAPEST);

	ESP_LOGI(TAG, "Initialize display");
	ESP_ERROR_CHECK(display_init(WITH_THREE_BUFFERS));

	DISPLAY_FLASH(MSG_BOOT);

	if (wifi_connect() == ESP_OK) {
		ESP_LOGI(TAG, "WiFi connected");
		DISPLAY_FLASH(MSG_SYNC);

		ESP_LOGI(TAG, "Starting NTP sync");
		ntp_start();
		ntp_wait_for_sync();

		ESP_LOGI(TAG, "Starting timers");
		call_every(SECOND, &update_time);
		call_every(TEN_SECONDS, &update_temperature);

		ESP_LOGI(TAG, "Start display switcher task");
		xTaskCreate(switch_display_task, "", 2048, NULL, 10, &task);

		configASSERT(task);

		ESP_LOGI(TAG, "Initialize button");
		ESP_ERROR_CHECK(button_init(BUTTON_PIN, &button_handler));
	} else {
		ESP_LOGE(TAG, "WiFi connection failed");
		DISPLAY_FLASH(MSG_FAIL);
	}
}
