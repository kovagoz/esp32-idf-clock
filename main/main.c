#include <stdio.h>
#include <time.h>
#include <freertos/FreeRTOS.h>
#include <freertos/portmacro.h>
#include <freertos/task.h>
#include <esp_err.h>

#include "clock/display.h"
#include "clock/ntp.h"
#include "clock/timer.h"
#include "clock/wifi.h"

// Short messages to display on the 7-segment display
#define MSG_BOOT 0x7c,0x5c,0x5c,0x78
#define MSG_SYNC 0x6d,0x66,0x54,0x39
#define MSG_FAIL 0x71,0x77,0x06,0x38
#define MSG_FINE 0x71,0x06,0x54,0x79

#define WITH_TWO_BUFFERS 2
#define CLOCK_DISPLAY_BUFFER 0
#define SECOND 1000000
#define TEN_SECONDS 10000000

/* static void display_write_celsius(float temperature) */
/* { */
/* 	temp_buffer[0] = symbols[2]; */
/* 	temp_buffer[1] = symbols[1]; */
/* 	temp_buffer[2] = 0b01100011; */
/* 	temp_buffer[3] = symbols[3]; */
/* } */

static void update_time()
{
	time_t systime = time(NULL);
	struct tm *now = localtime(&systime);

	display_write_time(CLOCK_DISPLAY_BUFFER, now->tm_hour, now->tm_min);
}

/* static void update_temperature() */
/* { */
/* 	display_write_celsius(21.3); */
/* } */

void app_main()
{
	timezone_set(CLOCK_TZ_EUROPE_BUDAPEST);

	display_init(WITH_TWO_BUFFERS);

	DISPLAY_FLASH(MSG_BOOT);

	if (wifi_connect() == ESP_OK) {
		DISPLAY_FLASH(MSG_SYNC);

		ntp_start();
		ntp_wait_for_sync();

		call_every(SECOND, &update_time);
		/* call_every(TEN_SECONDS, &update_temperature); */
	} else {
		DISPLAY_FLASH(MSG_FAIL);
	}
}
