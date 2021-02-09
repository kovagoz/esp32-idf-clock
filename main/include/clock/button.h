#ifndef CLOCK_BUTTON_H
#define CLOCK_BUTTON_H

#include <esp_err.h>

esp_err_t button_init(uint8_t pin, void *callback);

#endif
