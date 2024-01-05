/*
 * Copyright (c) 2006-2021, RT-Thread Development Team
 *
 * SPDX-License-Identifier: Apache-2.0
 *
 * Change Logs:
 * Date           Author       Notes
 * 2023-03-25     59427       the first version
 */
#include <rtthread.h>
#include <rtdevice.h>
#include "ap3216c.h"
#include "alex.h"

#define DBG_TAG "ap3216c"
#define DBG_LVL DBG_LOG
#include <rtdbg.h>

#define THREAD_PRIORITY         20
#define THREAD_STACK_SIZE       1024
#define THREAD_TIMESLICE        5

static rt_thread_t tid1 = RT_NULL;

#define AP3216C_I2C_BUS_NAME    "i2c3"  /* The name of the I2C bus device to which the sensor is connected */
#define AP3216C_DATA_NAX        10  /* The maximum number of data that can be read from the array */

/* The entry function of the thread */
static void ap3216c_entry(void *parameter)
{
    rt_uint32_t recv;
    static ap3216c_device_t dev = RT_NULL;

    rt_uint8_t cnt = 0;

    rt_uint16_t ps_data = 0;
    float brightness = 0.0;

    static AP3216CData_t sensor_data[AP3216C_DATA_NAX];
    MsgData_t msg = {
            .from       = AP3216C,
            .to         = OLED,
            .data_ptr   = RT_NULL,
            .data_size  = sizeof(sensor_data)
    };

    dev = ap3216c_init(AP3216C_I2C_BUS_NAME);

    while (1)
    {
        if ( cnt == AP3216C_DATA_NAX )
            cnt = 0;

        /* Forever wait, either Event 1 can trigger, event flag cleared after receiving it */
        if (rt_event_recv(rtc_event, EVENT_FLAG1,
                          RT_EVENT_FLAG_OR | RT_EVENT_FLAG_CLEAR,
                          RT_WAITING_FOREVER, &recv) == RT_EOK)
        {
            /* Read the sensor */
            ps_data = ap3216c_read_ps_data(dev);
            if (ps_data == 0)
            {
                LOG_D("object is not proximity of sensor.");
            }
            else
            {
                LOG_D("ap3216c read current ps data      : %d", ps_data);
            }

            brightness = ap3216c_read_ambient_light(dev);
            LOG_D("ap3216c measure current brightness: %d.%d(lux) ",
                    (int)brightness, ((int)(10 * brightness) % 10));

            sensor_data[cnt].ps_data = ps_data;
            sensor_data[cnt].brightness = brightness;
            msg.data_ptr = (void *)(&sensor_data[cnt]);

            /* This message pointer is sent to the mq message queue */
            if (rt_mq_send(&mq, (void*)&msg, sizeof(MsgData_t)) != RT_EOK)
            {
                LOG_E("rt_mq_send ERR.");
                rt_thread_mdelay(500);
            }

            if ( rt_sem_take(ap3216c_sem, RT_WAITING_FOREVER) == RT_EOK )
            {
                /* Send event */
                rt_event_send(rtc_event, EVENT_FLAG5);
            }

            cnt++;
        }
        else
        {
            LOG_E("take a event failed.");
        }
    }
}

/* Example of a thread */
int ap3216c_sample_start(void)
{
    /* Create a thread with the name "ap3216c" and the entry "ap3216c_entry" */
    tid1 = rt_thread_create("ap3216c",
                            ap3216c_entry, RT_NULL,
                            THREAD_STACK_SIZE,
                            THREAD_PRIORITY, THREAD_TIMESLICE);

    /* If you get a thread control block, start the thread */
    if (tid1 != RT_NULL)
        rt_thread_startup(tid1);

    return RT_EOK;
}
