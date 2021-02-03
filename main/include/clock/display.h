#ifndef CLOCK_DISPLAY_H
#define CLOCK_DISPLAY_H

// Write data to the active buffer and display it immediately.
#define DISPLAY_FLASH(msg) {               \
		buffer_t __data = { msg };         \
		display_flash(__data);             \
	}

typedef uint8_t buffer_t[4];

#include <esp_err.h>

esp_err_t display_init(uint8_t buffer_count);
esp_err_t display_write(uint8_t buf_id, buffer_t data);
esp_err_t display_write_time(uint8_t id, uint8_t hour, uint8_t minute);
esp_err_t display_select(uint8_t buf_id);

// Helper functions
void display_flash(buffer_t data);
uint8_t display_select_next();

#endif
