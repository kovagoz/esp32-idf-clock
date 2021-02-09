#ifndef CLOCK_DISPLAY_H
#define CLOCK_DISPLAY_H

typedef uint8_t buffer_t[4];

#include <esp_err.h>

esp_err_t display_init(uint8_t buffer_count);
esp_err_t display_write(uint8_t buf_id, buffer_t data);
esp_err_t display_write_time(uint8_t buf_id, uint8_t hour, uint8_t minute);
esp_err_t display_write_celsius(uint8_t buf_id, float temperature);
esp_err_t display_select(uint8_t buf_id);

void display_select_next();

#endif
