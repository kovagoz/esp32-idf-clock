#include <stdio.h>
#include <freertos/FreeRTOS.h>
#include <freertos/projdefs.h>
#include <sdkconfig.h>
#include <tm1637.h>

static tm1637_led_t *display;

void display_init()
{
	display = tm1637_init(CONFIG_CLOCK_DISPLAY_CLK_PIN, CONFIG_CLOCK_DISPLAY_DIO_PIN);
	tm1637_set_brightness(display, CONFIG_CLOCK_DISPLAY_BRIGHTNESS);
}

void display_write(uint8_t s0, uint8_t s1, uint8_t s2, uint8_t s3)
{
	tm1637_set_segment_raw(display, 0, s0);
	tm1637_set_segment_raw(display, 1, s1);
	tm1637_set_segment_raw(display, 2, s2);
	tm1637_set_segment_raw(display, 3, s3);
}

void display_write_time(int hours, int minutes)
{
	tm1637_set_segment_number(display, 0, hours / 10, pdFALSE);
	tm1637_set_segment_number(display, 1, hours % 10, pdTRUE);
	tm1637_set_segment_number(display, 2, minutes / 10, pdFALSE);
	tm1637_set_segment_number(display, 3, minutes % 10, pdFALSE);
}
