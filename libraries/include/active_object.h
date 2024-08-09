#ifndef ACTIVE_OBJECT_H
#define ACTIVE_OBJECT_H

#include <zephyr/kernel.h>

#define BASIC_PRIORITY 7

struct data_item_type
{
    uint32_t field1;
    uint32_t field2;
    uint32_t field3;
};

typedef struct zephyr_ao_t zephyr_ao;

typedef void (*dispatch_handler)(zephyr_ao const *me, const uint8_t signal);

void zephyrAO_constructor(zephyr_ao *, dispatch_handler);
void zephyrAO_start(zephyr_ao *, char *, k_thread_stack_t *);

struct zephyr_ao_t
{
    struct k_msgq *ao_msg_queue;
    struct k_thread *ao_thread;
    dispatch_handler handler;
    struct data_item_type data;
};

#endif