#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <freertos/FreeRTOS.h>
#include <freertos/event_groups.h>
#include <freertos/portmacro.h>
#include <freertos/projdefs.h>
#include <freertos/task.h>
#include <esp_log.h>
#include <esp_err.h>
#include <sdkconfig.h>
#include <tm1637.h>

#include "clock/display.h"

#define VALIDATE_BUFFER_ID(x) if (x >= bufcount) {     \
		ESP_LOGE(TAG, "Invalid buffer ID");            \
		return ESP_FAIL;                               \
	}

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
static const char *TAG = "display";

static tm1637_led_t *display;

static buffer_t *buffers;
static uint8_t bufcount = 0;
static uint8_t active_buffer = 0;

static EventGroupHandle_t display_events;

esp_err_t display_write(uint8_t buf_id, buffer_t data)
{
	VALIDATE_BUFFER_ID(buf_id);

	memcpy(buffers[buf_id], data, sizeof(buffer_t));

	xEventGroupSetBits(display_events, 1 << buf_id);

	return ESP_OK;
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

esp_err_t display_write_celsius(uint8_t buf_id, float temperature)
{
	// Convert to decicelsius :)
	uint16_t dc = (int) (temperature * 10);

	buffer_t data;
	data[0] = symbols[dc / 100];
	data[1] = symbols[dc % 100 / 10];
	data[2] = 0b01100011; // degree sign
	data[3] = symbols[dc % 10];

	return display_write(buf_id, data);
}

esp_err_t display_write_percent(uint8_t buf_id, uint8_t percent)
{
	buffer_t data;

	if (percent > 99) {
		// Write "HI" for high
		data[0] = 0b01110110;
		data[1] = symbols[1];
	} else {
		data[0] = symbols[percent / 10];
		data[1] = symbols[percent % 10];
	}

	// Display the % sign (kind of)
	data[2] = 0b01100011; // upper "o"
	data[3] = 0b01011100; // lower "o"

	return display_write(buf_id, data);
}

esp_err_t display_select(uint8_t buf_id)
{
	VALIDATE_BUFFER_ID(buf_id);

	// Do not refresh the display unnecessarily
	if (buf_id != active_buffer) {
		active_buffer = buf_id;
		xEventGroupSetBits(display_events, 1 << buf_id);
	}

	return ESP_OK;
}

void display_select_next()
{
	uint8_t nextbuf = active_buffer + 1;

	if (nextbuf == bufcount) {
		nextbuf = 0;
	}

	display_select(nextbuf);
}

static void display_update_task(void *pvParameters)
{
	for (;;) {
		EventBits_t bits = xEventGroupWaitBits(
			display_events,
			255, // FIXME set it to it's highest possible value
			pdTRUE, // xClearOnExit
			pdFALSE, // xWaitForAllBits
			portMAX_DELAY
		);

		// Make a copy of the ID of active buffer so it can't be changed
		// during refreshing the display.
		uint8_t buf_id = active_buffer;

		if (bits & (1 << buf_id)) {
			ESP_LOGI(TAG, "Updating display");
			tm1637_set_segment_raw(display, 0, buffers[buf_id][0]);
			tm1637_set_segment_raw(display, 1, buffers[buf_id][1]);
			tm1637_set_segment_raw(display, 2, buffers[buf_id][2]);
			tm1637_set_segment_raw(display, 3, buffers[buf_id][3]);
		}
	}
}

/**
 * @param buffer_count Number of display buffers to allocate. Each buffer
 * acts as a virtual display and you can switch between them by display_select()
 * and display_select_next() functions.
 */
esp_err_t display_init(uint8_t buffer_count)
{
	if (buffer_count < 1) {
		ESP_LOGE(TAG, "At least 1 buffer is required");
		return ESP_FAIL;
	}

	// Initialize the LED display hardware.
	display = tm1637_init(CONFIG_CLOCK_DISPLAY_CLK_PIN, CONFIG_CLOCK_DISPLAY_DIO_PIN);
	tm1637_set_brightness(display, CONFIG_CLOCK_DISPLAY_BRIGHTNESS);

	// Allocate memory for display buffers.
	buffers = calloc(buffer_count, sizeof(buffer_t));

	if (buffers == NULL) {
		ESP_LOGE(TAG, "Cannot allocate enough memory for display buffers");
		return ESP_FAIL;
	}

	bufcount = buffer_count;

	// Tasks can request display refresh throw this event group.
	display_events = xEventGroupCreate();

	// Start the display updater task which listens the event group above.
	xTaskCreate(display_update_task, "", 2048, NULL, 10, NULL);

	return ESP_OK;
}
