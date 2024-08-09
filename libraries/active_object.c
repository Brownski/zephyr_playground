#include <stdio.h>
#include <zephyr/kernel.h>
#include <active_object.h>

static void zephyrAO_threadFunction(void *p1, void *p2, void *p3)
{
    zephyr_ao *me = (zephyr_ao *)p1;
    ARG_UNUSED(p2);
    ARG_UNUSED(p3);

    k_msgq_get(me->ao_msg_queue, &me->data, K_FOREVER);

    while (1)
    {
        me->handler(me, 1);
    }
}

void zephyrAO_constructor(zephyr_ao *me, dispatch_handler handler)
{
    me->handler = handler;
}

void zephyrAO_start(zephyr_ao *me, char *msg_q_buf, k_thread_stack_t *active_thread_stack)
{
    k_msgq_init(me->ao_msg_queue, msg_q_buf, sizeof(struct data_item_type), 10);
    me->ao_thread = k_thread_create(me->ao_thread, active_thread_stack,
                                    K_THREAD_STACK_SIZEOF(active_thread_stack),
                                    zephyrAO_threadFunction, me, NULL, NULL,
                                    BASIC_PRIORITY, 0, K_FOREVER);
}
