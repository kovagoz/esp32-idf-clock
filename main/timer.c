#include <esp_timer.h>

void call_every(uint32_t interval, void *callback)
{
	const esp_timer_create_args_t periodic_timer_args = {
		.callback = callback,
	};

	esp_timer_handle_t periodic_timer;

	esp_timer_create(&periodic_timer_args, &periodic_timer);
	esp_timer_start_periodic(periodic_timer, interval);
}
