/*
 * Copyright (c) 2024 Straw Lab
 * SPDX-License-Identifier: Apache-2.0
 */

#include <zephyr/kernel.h>
#include <zephyr/drivers/gpio.h>

#include <nrf_802154.h>

typedef uint8_t byte;

static const struct gpio_dt_spec led = GPIO_DT_SPEC_GET(DT_ALIAS(led0), gpios);
static const struct gpio_dt_spec led_r = GPIO_DT_SPEC_GET(DT_ALIAS(led1_red), gpios);
static const struct gpio_dt_spec led_b = GPIO_DT_SPEC_GET(DT_ALIAS(led1_blue), gpios);

void set_pos(byte* data_to_transmit, int val){
	data_to_transmit[10] = 0x40 | (byte)((val >> 8) & 0x0F);
	data_to_transmit[11] = (byte)(val & 0xFF);
	data_to_transmit[13] = 0 - data_to_transmit[10] - data_to_transmit[11] - data_to_transmit[12];
}

int main(void)
{
	int ret;

	if (!(gpio_is_ready_dt(&led) && gpio_is_ready_dt(&led_r) && gpio_is_ready_dt(&led_b))) {
		return 0;
	}

	gpio_pin_configure_dt(&led, GPIO_OUTPUT_INACTIVE);
	gpio_pin_configure_dt(&led_r, GPIO_OUTPUT_INACTIVE);

	byte data_to_transmit[] = 
		"\x0f\x61\x88\x00\xE7\x01\xFF\xFF\x96\xF0\x44\x05\x00\xB7\x00\x00";
	//     ^^ packet length (with FCS)
	//                                                             ^^  ^^  space for FCS (packet crc, computed by nrf_802154_transmit_raw)	

	nrf_802154_init();
	nrf_802154_channel_set(12);

	int n = 0;

	while (1) {
		bool toggle = ((n / 200) % 2 == 0);
		set_pos(data_to_transmit,  toggle ? n % 2 : 1000 + (n % 2)); //(n%2) is adding some noise to the data, that keeps the motor at full speed
		gpio_pin_set_dt(&led_r, toggle); //toggle red together with position
		nrf_802154_transmit_raw(data_to_transmit, NULL);
		
		k_msleep(10); //ideally, we should also ensure that tx has finished before editing the buffer... skipping that for simplicity
		++n;
		++data_to_transmit[3]; //increment packet sequence number

		gpio_pin_set_dt(&led,true); //indicate that everything is running
	}
	return 0;
}
