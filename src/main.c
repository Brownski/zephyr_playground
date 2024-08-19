/*
 * Copyright (c) 2016 Intel Corporation
 *
 * SPDX-License-Identifier: Apache-2.0
 */

#include <stdio.h>
#include <zephyr/kernel.h>
#include <zephyr/drivers/gpio.h>
#include <zephyr/logging/log.h>

#include <zephyr/bluetooth/bluetooth.h>
#include <zephyr/bluetooth/hci.h>
#include <zephyr/bluetooth/conn.h>
#include <zephyr/bluetooth/uuid.h>
#include <zephyr/bluetooth/gatt.h>

#include <active_object.h>

LOG_MODULE_REGISTER(app, LOG_LEVEL_DBG);

zephyr_ao buttonAO;
static void buttonAO_handler(zephyr_ao const *me, const uint8_t signal);

char my_msgq_buffer[10 * sizeof(struct data_item_type)];
struct k_msgq my_msgq;

/* 1000 msec = 1 sec */
#define SLEEP_TIME_MS 1000

/* The devicetree node identifier for the "led0" alias. */
#define LED0_NODE DT_ALIAS(led0)

#define LED1_NODE DT_ALIAS(led1)

#define LED2_NODE DT_ALIAS(led2)

#define BUTTON_NODE DT_ALIAS(btn0)

/* size of stack area used by each thread */
#define STACKSIZE 1024
K_THREAD_STACK_DEFINE(buttonAO_stack, STACKSIZE);

/* scheduling priority used by each thread */
#define PRIORITY 7

#define BUTTON_PRESSED 1

/*
 * A build error on this line means your board is unsupported.
 * See the sample documentation for information on how to fix this.
 */
static const struct gpio_dt_spec led0 = GPIO_DT_SPEC_GET(LED0_NODE, gpios);
static const struct gpio_dt_spec led1 = GPIO_DT_SPEC_GET(LED1_NODE, gpios);
static const struct gpio_dt_spec led2 = GPIO_DT_SPEC_GET(LED2_NODE, gpios);
static const struct gpio_dt_spec button = GPIO_DT_SPEC_GET(BUTTON_NODE, gpios);

static struct gpio_callback trigger_pin_callback_data;

static void buttonAO_handler(zephyr_ao const *me, const uint8_t signal)
{

	switch (signal)
	{
	case BUTTON_PRESSED:
		gpio_pin_toggle_dt(&led0);
		//gpio_pin_toggle_dt(&led1);
		//gpio_pin_toggle_dt(&led2);
		break;

	default:
		break;
	}
}

static void trigger_pin_callback_handler(const struct device *port, struct gpio_callback *cb, gpio_port_pins_t pins)
{
	zephyrAO_post(&buttonAO);
}

static int setup_gpio(void)
{
	uint8_t ret = 0;

	if (!gpio_is_ready_dt(&led0))
	{
		return 0;
	}

	ret = gpio_pin_configure_dt(&led0, GPIO_OUTPUT_ACTIVE);
	if (ret < 0)
	{
		return 0;
	}

	if (!gpio_is_ready_dt(&led1))
	{
		return 0;
	}

	ret = gpio_pin_configure_dt(&led1, GPIO_OUTPUT_ACTIVE);
	if (ret < 0)
	{
		return 0;
	}

	if (!gpio_is_ready_dt(&led2))
	{
		return 0;
	}

	ret = gpio_pin_configure_dt(&led2, GPIO_OUTPUT_ACTIVE);
	if (ret < 0)
	{
		return 0;
	}

	gpio_pin_configure_dt(&button, GPIO_INPUT);
	gpio_pin_interrupt_configure_dt(&button, GPIO_INT_EDGE_FALLING);
	gpio_init_callback(&trigger_pin_callback_data, trigger_pin_callback_handler, BIT(button.pin));
	gpio_add_callback(button.port, &trigger_pin_callback_data);

	return 1;
}

int main(void)
{
	int res = 0;

	res = setup_gpio();

	LOG_INF("Successfully initialised the GPIO!");

	zephyrAO_constructor(&buttonAO, &buttonAO_handler);

	zephyrAO_start(&buttonAO, my_msgq_buffer, buttonAO_stack);

	return 0;
}