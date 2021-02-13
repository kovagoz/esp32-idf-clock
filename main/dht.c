#include <esp_err.h>
#include <sdkconfig.h>

#include "dht.h"

static const dht_sensor_type_t dht_sensor_type = DHT_TYPE_AM2301;
static const gpio_num_t dht_gpio = CONFIG_CLOCK_DHT_PIN;

esp_err_t dht_read(float *humidity, float *temperature)
{
	return dht_read_float_data(dht_sensor_type, dht_gpio, humidity, temperature);
}
