/*
 * Copyright (c) 2006-2021, RT-Thread Development Team
 *
 * SPDX-License-Identifier: Apache-2.0
 *
 * Change Logs:
 * Date           Author       Notes
 * 2023-02-19     59427       the first version
 */

#include <rtthread.h>
#include <drv_gpio.h>
#ifndef RT_USING_NANO
#include <rtdevice.h>
#endif /* RT_USING_NANO */

#define DBG_TAG "led"
#define DBG_LVL DBG_INFO
#include <rtdbg.h>

#define THREAD_PRIORITY         25
#define THREAD_STACK_SIZE       256
#define THREAD_TIMESLICE        5

static rt_thread_t tid1 = RT_NULL;

#define LED_R_PIN       GET_PIN(E, 7)  /* PE7 */
#define LED_G_PIN       GET_PIN(E, 8)  /* PE8 */
#define LED_B_PIN       GET_PIN(E, 9)  /* PE9 */

/* The entry function of the thread */
static void led_entry(void *parameter)
{
    /* The LED pins are in output mode */
    rt_pin_mode(LED_R_PIN, PIN_MODE_OUTPUT);
    rt_pin_mode(LED_G_PIN, PIN_MODE_OUTPUT);
    rt_pin_mode(LED_B_PIN, PIN_MODE_OUTPUT);
    /* The LED pin is powered on and off */
    rt_pin_write(LED_R_PIN, PIN_HIGH);
    rt_pin_write(LED_G_PIN, PIN_HIGH);
    rt_pin_write(LED_B_PIN, PIN_HIGH);

    while (1)
    {
        /* RGB LED use a common anode method, so the low level is on and the high level is off */

        /* Set the level low */
        rt_pin_write(LED_G_PIN, PIN_LOW);
        LOG_D("Green led On");
        rt_thread_mdelay(1000);

        /* Set the level high */
        rt_pin_write(LED_G_PIN, PIN_HIGH);
        LOG_D("Green led Off");
        rt_thread_mdelay(1000);
    }
}

/* Example of a thread */
int led_sample_start(void)
{
    /* Create a thread with the name "led" and the entry "led_entry" */
    tid1 = rt_thread_create("led",
                            led_entry, RT_NULL,
                            THREAD_STACK_SIZE,
                            THREAD_PRIORITY, THREAD_TIMESLICE);

    /* If you get a thread control block, start the thread */
    if (tid1 != RT_NULL)
        rt_thread_startup(tid1);

    return RT_EOK;
}


