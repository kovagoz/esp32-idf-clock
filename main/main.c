#include <stdio.h>
#include <time.h>
#include <freertos/FreeRTOS.h>
#include <freertos/portmacro.h>
#include <freertos/task.h>
#include <esp_err.h>
#include <esp_timer.h>

#include "clock/display.h"
#include "clock/ntp.h"
#include "clock/timer.h"
#include "clock/wifi.h"

int hours   = -1;
int minutes = -1;

TaskHandle_t display_task;

static void update_time()
{
	time_t systime = time(NULL);
	struct tm *now = localtime(&systime);

	// If time has changed
	if (now->tm_hour != hours || now->tm_min != minutes) {
		hours   = now->tm_hour;
		minutes = now->tm_min;

		// Tell the task below it's time to update the display
		xTaskNotifyGive(display_task);
	}
}

static void update_display(void *pvParameters)
{
	for (;;) {
		// Waiting for notification
		ulTaskNotifyTake(pdTRUE, portMAX_DELAY);
		display_write_time(hours, minutes);
	}
}

void app_main()
{
	timezone_set(CLOCK_TZ_EUROPE_BUDAPEST);

	display_init();
	display_write(DISPLAY_MSG_BOOT);

	if (wifi_connect() == ESP_OK) {
		display_write(DISPLAY_MSG_SYNC);

		ntp_start();
		ntp_wait_for_sync();

		xTaskCreate(update_display, "", 2048, NULL, 10, &display_task);

		call_every_second(&update_time);
	} else {
		display_write(DISPLAY_MSG_FAIL);
	}
}
