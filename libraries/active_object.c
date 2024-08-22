#include <stdio.h>
#include <zephyr/kernel.h>
#include <active_object.h>

#define STACKSIZE 1024
K_THREAD_STACK_DEFINE(stack, STACKSIZE);

static void zephyrAO_threadFunction(void *p1, void *p2, void *p3)
{
    zephyr_ao *me = (zephyr_ao *)p1;
    ARG_UNUSED(p2);
    ARG_UNUSED(p3);

    Event event;

    while (1)
    {
        k_msgq_get(&me->ao_msg_queue, &event, K_FOREVER);
        me->handler(me, event.signal);
    }
}

void zephyrAO_constructor(zephyr_ao *me, dispatch_handler handler)
{
    me->handler = handler;
}

void zephyrAO_start(zephyr_ao *me, char *msg_q_buf, k_thread_stack_t *active_thread_stack)
{
    k_msgq_init(&me->ao_msg_queue, msg_q_buf, sizeof(Event), 10);
    k_thread_create(&me->ao_thread, stack,
                    K_THREAD_STACK_SIZEOF(stack),
                    zephyrAO_threadFunction, me, NULL, NULL,
                    BASIC_PRIORITY, 0, K_FOREVER);
    k_thread_start(&me->ao_thread);
}

void zephyrAO_post(zephyr_ao *me, Event *data)
{
    k_msgq_put(&me->ao_msg_queue, data, K_NO_WAIT);
}
