#ifndef ACTIVE_OBJECT_H
#define ACTIVE_OBJECT_H

#include <zephyr/kernel.h>

#define BASIC_PRIORITY 7

#define AO_TRANSITION(ao, state)    ao.handler = &ao##_##state

typedef enum
{
    AO_INIT = 0,
    BUTTON1_PRESSED,
    BUTTON2_PRESSED,
    BUTTON3_PRESSED,
    SPI_PACKET_RXD,
    /*BLE Specific Signals start */
    BLE_ADV_PACKET_RECEIVED,
    BLE_NOTIFY_RECEIVED,
    BLE_WRITE_RECEIVED
} Signal;

typedef struct
{
    Signal signal;
} Event;

typedef struct zephyr_ao_t zephyr_ao;

typedef void (*dispatch_handler)(zephyr_ao const *me, const Event event);

void zephyrAO_constructor(zephyr_ao *, dispatch_handler);
void zephyrAO_start(zephyr_ao *, char *, k_thread_stack_t *);
void zephyrAO_post(zephyr_ao *me, Event *data);

struct zephyr_ao_t
{
    struct k_msgq ao_msg_queue;
    struct k_thread ao_thread;
    dispatch_handler handler;
};

#endif