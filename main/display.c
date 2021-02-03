#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <freertos/FreeRTOS.h>
#include <freertos/event_groups.h>
#include <freertos/projdefs.h>
#include <freertos/task.h>
#include <esp_log.h>
#include <esp_err.h>
#include <sdkconfig.h>
#include <tm1637.h>

#include "clock/display.h"

static const int8_t symbols[] = {
	0x3f, // 0b00111111,    // 0
	0x06, // 0b00000110,    // 1
	0x5b, // 0b01011011,    // 2
	0x4f, // 0b01001111,    // 3
	0x66, // 0b01100110,    // 4
	0x6d, // 0b01101101,    // 5
	0x7d, // 0b01111101,    // 6
	0x07, // 0b00000111,    // 7
	0x7f, // 0b01111111,    // 8
	0x6f, // 0b01101111,    // 9
};

// Log tag
static const char* TAG = "display";

static tm1637_led_t *display;

static buffer_t *buffers;
static uint8_t bufcount = 0;
static uint8_t active_buffer = 0;

static EventGroupHandle_t display_events;

esp_err_t display_write(uint8_t buf_id, buffer_t data)
{
	if (buf_id >= bufcount) {
		ESP_LOGE(TAG, "Invalid buffer ID");
		return ESP_FAIL;
	}

	memcpy(buffers[buf_id], data, sizeof(buffer_t));

	xEventGroupSetBits(display_events, (1 << buf_id));

	return ESP_OK;
}

void display_flash(buffer_t data)
{
	display_write(active_buffer, data);
}

esp_err_t display_write_time(uint8_t buf_id, uint8_t hour, uint8_t minute)
{
	buffer_t data;
	data[0] = symbols[hour / 10];
	data[1] = symbols[hour % 10];
	data[2] = symbols[minute / 10];
	data[3] = symbols[minute % 10];

	data[1] |= 0x80; // Turn on the colon

	return display_write(buf_id, data);
}

esp_err_t display_select(uint8_t buf_id)
{
	if (buf_id >= bufcount) {
		ESP_LOGE(TAG, "Invalid buffer ID");
		return ESP_FAIL;
	}

	active_buffer = buf_id;

	return ESP_OK;
}

uint8_t display_select_next()
{
	if (active_buffer == bufcount - 1) {
		active_buffer = 0;
	} else {
		active_buffer++;
	}

	return active_buffer;
}

static void display_update_task(void *pvParameters)
{
	for (;;) {
		xEventGroupWaitBits(
			display_events,
			1 << active_buffer,
			pdTRUE, // xClearOnExit
			pdFALSE, // xWaitForAllBits
			portMAX_DELAY // xTicksToWait
		);

		tm1637_set_segment_raw(display, 0, buffers[active_buffer][0]);
		tm1637_set_segment_raw(display, 1, buffers[active_buffer][1]);
		tm1637_set_segment_raw(display, 2, buffers[active_buffer][2]);
		tm1637_set_segment_raw(display, 3, buffers[active_buffer][3]);
	}
}

esp_err_t display_init(uint8_t buffer_count)
{
	if (buffer_count < 1) {
		ESP_LOGE(TAG, "At least 1 buffer is required");
		return ESP_FAIL;
	}

	display = tm1637_init(CONFIG_CLOCK_DISPLAY_CLK_PIN, CONFIG_CLOCK_DISPLAY_DIO_PIN);
	tm1637_set_brightness(display, CONFIG_CLOCK_DISPLAY_BRIGHTNESS);

	display_events = xEventGroupCreate();

	buffers = calloc(buffer_count, sizeof(buffer_t));

	if (buffers == NULL) {
		ESP_LOGE(TAG, "Cannot allocate enough memory for display buffers");
		return ESP_FAIL;
	}

	bufcount = buffer_count;

	xTaskCreate(display_update_task, "", 2048, NULL, 10, NULL);

	return ESP_OK;
}
