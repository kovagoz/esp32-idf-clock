#include <driver/gpio.h>
#include <esp_err.h>

#define ESP_INTR_FLAG_DEFAULT 0

uint8_t gpio_isr_installed = 0;

esp_err_t button_init(uint8_t pin, void *callback)
{
	esp_err_t retval;

	if (gpio_isr_installed == 0) {
		esp_err_t retval = gpio_install_isr_service(ESP_INTR_FLAG_DEFAULT);

		if (retval != ESP_OK) {
			return retval;
		}

		gpio_isr_installed = 1;
	}

	gpio_config_t io_conf = {
		.pin_bit_mask = 1ULL << pin,
		.mode         = GPIO_MODE_INPUT,
		.intr_type    = GPIO_PIN_INTR_NEGEDGE,
		.pull_up_en   = GPIO_PULLUP_ENABLE,
		.pull_down_en = GPIO_PULLDOWN_DISABLE,
	};

	retval = gpio_config(&io_conf);

	if (retval != ESP_OK) {
		return retval;
	}

	return gpio_isr_handler_add(pin, callback, (void *) pin);
}
