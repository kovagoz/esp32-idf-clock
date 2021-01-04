#include <stdio.h>
#include "freertos/FreeRTOS.h"
#include "freertos/FreeRTOSConfig.h"
#include "freertos/portmacro.h"
#include "freertos/task.h"
#include "freertos/semphr.h"
#include "driver/gpio.h"
#include "sdkconfig.h"
#include "tm1637.h"

#define ESP_INTR_FLAG_DEFAULT 0

#define GPIO_INPUT_PIN     23
#define GPIO_INPUT_PIN_SEL (1ULL<<GPIO_INPUT_PIN)

const gpio_num_t LED_CLK = CONFIG_TM1637_CLK_PIN;
const gpio_num_t LED_DTA = CONFIG_TM1637_DIO_PIN;

tm1637_led_t *led;
volatile int counter = 0;
SemaphoreHandle_t xSemaphore = NULL;

static void update_display(void *arg)
{
	for (;;) {
		tm1637_set_number(led, counter++);
		xSemaphoreTake(xSemaphore, portMAX_DELAY);
	}
}

static void IRAM_ATTR button_isr_handler(void *arg)
{
	xSemaphoreGiveFromISR(xSemaphore, pdFALSE);
}

void app_main() {
	xSemaphore = xSemaphoreCreateBinary();

	// Initialize the LED display
	led = tm1637_init(LED_CLK, LED_DTA);
	tm1637_set_brightness(led, 1);
	tm1637_set_number(led, counter);

	xTaskCreate(update_display, "Update LED display", 2048, NULL, 10, NULL);

	// Configure push button
	gpio_config_t io_conf;
	io_conf.pin_bit_mask = GPIO_INPUT_PIN_SEL;
	io_conf.mode         = GPIO_MODE_INPUT;
	io_conf.intr_type    = GPIO_PIN_INTR_NEGEDGE;
	io_conf.pull_up_en   = GPIO_PULLUP_ENABLE;
	io_conf.pull_down_en = GPIO_PULLDOWN_DISABLE;
	gpio_config(&io_conf);

	// Install the interrupt handler
	gpio_install_isr_service(ESP_INTR_FLAG_DEFAULT);
	gpio_isr_handler_add(GPIO_INPUT_PIN, button_isr_handler, NULL);
}
