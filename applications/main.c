/*
 * Copyright (c) 2006-2021, RT-Thread Development Team
 *
 * SPDX-License-Identifier: Apache-2.0
 *
 * Change Logs:
 * Date           Author       Notes
 * 2018-11-06     SummerGift   first version
 * 2023-12-03     Meco Man     support nano version
 */

#include <board.h>
#include <rtthread.h>
#ifndef RT_USING_NANO
#include <rtdevice.h>
#endif /* RT_USING_NANO */

/* include private header file */
#include "alex.h"
#include "led.h"
#include "alarm_sample.h"
#include "oled_sample.h"
#include "aht10_sample.h"
#include "ap3216c_sample.h"

/* Set log level */
#define DBG_TAG "main"
#define DBG_LVL DBG_LOG
#include <rtdbg.h>

/*
 * Private variable
*/

/* Event control block */
rt_event_t rtc_event = RT_NULL;

/* pointer to a semaphore */
rt_sem_t rtc_sem = RT_NULL;
rt_sem_t aht10_sem = RT_NULL;
rt_sem_t ap3216c_sem = RT_NULL;

/* Message Queuing control block */
struct rt_messagequeue mq;

/* The pool of memory used in the message queue to place messages */
rt_uint8_t msg_pool[1024];

int main(void)
{
    rt_err_t result;

    /* Create event */
    rtc_event = rt_event_create("rtc_event", RT_IPC_FLAG_PRIO);

    /* Create a dynamic semaphore with an initial value of 0 */
    rtc_sem = rt_sem_create("rtc_sem", 0, RT_IPC_FLAG_PRIO);
    aht10_sem = rt_sem_create("aht10_sem", 0, RT_IPC_FLAG_PRIO);
    ap3216c_sem = rt_sem_create("ap3216c_sem", 0, RT_IPC_FLAG_PRIO);

    /* Initialize the message queue */
    result = rt_mq_init(&mq,
                        "mqt",
                        &msg_pool[0],           /* The mempool points to msg_pool */
                        sizeof(MsgData_t),      /* The size of each message is 1 byte */
                        sizeof(msg_pool),       /* The size of the mempool is the size of msg_pool */
                        RT_IPC_FLAG_PRIO);      /* Messages are distributed according to the priority size */

    if (result != RT_EOK)
    {
        LOG_E("init message queue failed.");
    }

    /* Start the led thread, priority 25 */
    led_sample_start();
    /* Start the oled thread, priority 23 */
    oled_sample_start();
    /* Start the alarm thread, priority 18 */
    alarm_sample_start();
    /* Start the aht10 thread, priority 20 */
    aht10_sample_start();
    /* Start the ap3216c thread, priority 20 */
    ap3216c_sample_start();

    return RT_EOK;
}

