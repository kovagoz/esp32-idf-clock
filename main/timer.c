#include <esp_err.h>
#include <esp_timer.h>

void call_every(uint32_t interval, esp_timer_cb_t callback)
{
	(*callback)(NULL);

	const esp_timer_create_args_t periodic_timer_args = {
		.callback = callback,
	};

	esp_timer_handle_t periodic_timer;

	ESP_ERROR_CHECK(esp_timer_create(&periodic_timer_args, &periodic_timer));
	ESP_ERROR_CHECK(esp_timer_start_periodic(periodic_timer, interval));
}
