/*
 * Copyright (c) 2016 Intel Corporation
 *
 * SPDX-License-Identifier: Apache-2.0
 */

#include <stdio.h>
#include <zephyr/kernel.h>
#include <zephyr/drivers/gpio.h>

struct data_item_type
{
	uint32_t field1;
	uint32_t field2;
	uint32_t field3;
};

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

/* scheduling priority used by each thread */
#define PRIORITY 7

K_THREAD_STACK_DEFINE(activeA_stack_area, STACKSIZE);
static struct k_thread activeA_data;

K_THREAD_STACK_DEFINE(activeB_stack_area, STACKSIZE);
static struct k_thread activeB_data;
/*
 * A build error on this line means your board is unsupported.
 * See the sample documentation for information on how to fix this.
 */
static const struct gpio_dt_spec led0 = GPIO_DT_SPEC_GET(LED0_NODE, gpios);
static const struct gpio_dt_spec led1 = GPIO_DT_SPEC_GET(LED1_NODE, gpios);
static const struct gpio_dt_spec led2 = GPIO_DT_SPEC_GET(LED2_NODE, gpios);
static const struct gpio_dt_spec button = GPIO_DT_SPEC_GET(BUTTON_NODE, gpios);

static struct gpio_callback trigger_pin_callback_data;

static void trigger_pin_callback_handler(const struct device *port, struct gpio_callback *cb, gpio_port_pins_t pins)
{

	struct data_item_type data;

	data.field1 = 0;
	data.field2 = 1;
	data.field3 = 1;

	/* send data to consumers */
	while (k_msgq_put(&my_msgq, &data, K_NO_WAIT) != 0)
	{
		/* message queue is full: purge old data & try again */
		k_msgq_purge(&my_msgq);
	}
}

static void setup_gpio(void)
{
	uint8_t ret = 0;
	bool led_state = true;

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
}

void activeA_Thread(void *dummy1, void *dummy2, void *dummy3)
{
	ARG_UNUSED(dummy1);
	ARG_UNUSED(dummy2);
	ARG_UNUSED(dummy3);

	struct data_item_type data;

    while (1) {
        /* get a data item */
        k_msgq_get(&my_msgq, &data, K_FOREVER);
		
		if(data.field1 > 0)
			gpio_pin_toggle_dt(&led0);
		
		if(data.field2 > 0)
			gpio_pin_toggle_dt(&led1);
		
		if(data.field3 > 0)
			gpio_pin_toggle_dt(&led2);

    }
}

void activeB_Thread(void *dummy1, void *dummy2, void *dummy3)
{
	ARG_UNUSED(dummy1);
	ARG_UNUSED(dummy2);
	ARG_UNUSED(dummy3);


	while (1)
	{
		/* create data item to send (e.g. measurement, timestamp, ...) */
		k_msleep(SLEEP_TIME_MS);

		/* data item was successfully added to message queue */
	}
}

int main(void)
{
	setup_gpio();

	k_msgq_init(&my_msgq, my_msgq_buffer, sizeof(struct data_item_type), 10);

	k_thread_create(&activeA_data, activeA_stack_area,
					K_THREAD_STACK_SIZEOF(activeA_stack_area),
					activeA_Thread, NULL, NULL, NULL,
					PRIORITY, 0, K_FOREVER);
	k_thread_start(&activeA_data);

	k_thread_create(&activeB_data, activeB_stack_area,
					K_THREAD_STACK_SIZEOF(activeB_stack_area),
					activeB_Thread, NULL, NULL, NULL,
					PRIORITY, 0, K_FOREVER);
	k_thread_start(&activeB_data);

	return 0;
}

/*uint8_t ret = 0;
	bool led_state = true;

	if (!gpio_is_ready_dt(&led))
	{
		return 0;
	}

	ret = gpio_pin_configure_dt(&led, GPIO_OUTPUT_ACTIVE);
	if (ret < 0)
	{
		return 0;
	}

	while (1)
	{
		ret = gpio_pin_toggle_dt(&led);
		if (ret < 0)
		{
			continue;
		}

		led_state = !led_state;
		printf("LED state: %s\n", led_state ? "ON" : "OFF");
		k_msleep(SLEEP_TIME_MS);
	}*/