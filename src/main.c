/*
 * Copyright (c) 2016 Intel Corporation
 *
 * SPDX-License-Identifier: Apache-2.0
 */

#include <zephyr/kernel.h>
#include <zephyr/drivers/gpio.h>

#include <nrf_802154.h>

typedef uint8_t byte;

/* 1000 msec = 1 sec */
#define SLEEP_TIME_MS   1000

/* The devicetree node identifier for the "led0" alias. */
#define LED0_NODE DT_ALIAS(led0)

/*
 * A build error on this line means your board is unsupported.
 * See the sample documentation for information on how to fix this.
 */
static const struct gpio_dt_spec led = GPIO_DT_SPEC_GET(LED0_NODE, gpios);
static const struct gpio_dt_spec led_r = GPIO_DT_SPEC_GET(DT_ALIAS(led1_red), gpios);
static const struct gpio_dt_spec led_b = GPIO_DT_SPEC_GET(DT_ALIAS(led1_blue), gpios);

void blink(){
	gpio_pin_set_dt(&led, true);
	k_msleep(1000);
	gpio_pin_set_dt(&led, false);
	k_msleep(100);
}

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

	ret = gpio_pin_configure_dt(&led, GPIO_OUTPUT_ACTIVE);
	if (ret < 0) {
		return 0;
	}

	gpio_pin_configure_dt(&led_r, GPIO_OUTPUT_INACTIVE);
	gpio_pin_configure_dt(&led_b, GPIO_OUTPUT_INACTIVE);
	gpio_pin_set_dt(&led_b, false);

	byte data_to_transmit[] = 
		"\x0f\x61\x88\x00\xE7\x01\xFF\xFF\x96\xF0\x44\x05\x00\xB7\x00\x00";
	//     ^^ packet length (with FCS)
	//                                                             ^^  ^^  space for FCS (packet crc), computed by tx function	

	blink();
	nrf_802154_init();
	blink();
	nrf_802154_channel_set(12);
	blink();

	int n = 0;

	while (1) {
		set_pos(data_to_transmit, n / 100 % 2 == 0 ? 0 : 1000);
		ret = nrf_802154_transmit_raw(data_to_transmit, NULL);
		gpio_pin_set_dt(&led_b, ret);
		
		k_msleep(20);
		++n;
		++data_to_transmit[3];
	}
	return 0;
}
