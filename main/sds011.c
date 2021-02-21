#include <driver/uart.h>

#include "sds011.h"
#include "sds011_consts.h"
#include "sds011_structs.h"

#define SDS011_ON_DURATION CONFIG_CLOCK_SDS011_RUN_INTERVAL * 1000

static const struct sds011_tx_packet sds011_tx_sleep_packet = {
	.head               = SDS011_PACKET_HEAD,
	.command            = SDS011_CMD_TX,
	.sub_command        = SDS011_TX_CMD_SLEEP_MODE,
	.payload_sleep_mode = {
		.method = SDS011_METHOD_SET,
		.mode   = SDS011_SLEEP_MODE_ENABLED
	},
	.device_id = SDS011_DEVICE_ID_ALL,
	.tail      = SDS011_PACKET_TAIL
};

static const struct sds011_tx_packet sds011_tx_wakeup_packet = {
	.head               = SDS011_PACKET_HEAD,
	.command            = SDS011_CMD_TX,
	.sub_command        = SDS011_TX_CMD_SLEEP_MODE,
	.payload_sleep_mode = {
		.method = SDS011_METHOD_SET,
		.mode   = SDS011_SLEEP_MODE_DISABLED
	},
	.device_id = SDS011_DEVICE_ID_ALL,
	.tail      = SDS011_PACKET_TAIL
};

void sds011_init()
{
	sds011_begin(
		CONFIG_CLOCK_SDS011_UART,
		CONFIG_CLOCK_SDS011_TX_PIN,
		CONFIG_CLOCK_SDS011_RX_PIN
	);
}

void sds011_read(float *pm25, float *pm10)
{
	struct sds011_rx_packet rx_packet;

	sds011_send_cmd_to_queue(&sds011_tx_wakeup_packet, 0);

	vTaskDelay(pdMS_TO_TICKS(SDS011_ON_DURATION));

	if (sds011_recv_data_from_queue(&rx_packet, 0) == SDS011_OK) {
		*pm25 = ((rx_packet.payload_query_data.pm2_5_high << 8) |
		   rx_packet.payload_query_data.pm2_5_low) /
		  10.0;

		*pm10 = ((rx_packet.payload_query_data.pm10_high << 8) |
		  rx_packet.payload_query_data.pm10_low) /
		 10.0;
	}

	sds011_send_cmd_to_queue(&sds011_tx_sleep_packet, 0);
}
