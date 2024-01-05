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
#include "aht10.h"
#include "alex.h"

#define DBG_TAG "aht10"
#define DBG_LVL DBG_LOG
#include <rtdbg.h>

#define THREAD_PRIORITY         20
#define THREAD_STACK_SIZE       1024
#define THREAD_TIMESLICE        5

static rt_thread_t tid1 = RT_NULL;

#define AHT10_I2C_BUS_NAME      "i2c4"  /* The name of the I2C bus device to which the sensor is connected */
#define AHT10_DATA_NAX          10  /* The maximum number of data that can be read from the array */

/* The entry function of the thread */
static void aht10_entry(void *parameter)
{
    rt_uint32_t recv;

    rt_uint8_t cnt = 0;
    float humidity = 0.0;
    float temperature = 0.0;

    static Aht10Data_t sensor_data[AHT10_DATA_NAX];
    MsgData_t msg = {
            .from       = AHT10,
            .to         = OLED,
            .data_ptr   = RT_NULL,
            .data_size  = sizeof(sensor_data)
    };

    aht10_init(AHT10_I2C_BUS_NAME);

    while (1)
    {
        if ( cnt == AHT10_DATA_NAX )
            cnt = 0;

        /* Forever wait, either Event 1 can trigger, event flag cleared after receiving it */
        if (rt_event_recv(rtc_event, EVENT_FLAG1,
                          RT_EVENT_FLAG_OR | RT_EVENT_FLAG_CLEAR,
                          RT_WAITING_FOREVER, &recv) == RT_EOK)
        {
            /* Read temperature and humidity data */
            read_temp_humi(&temperature, &humidity);

            LOG_D("read aht10 sensor humidity   : %d.%d %%", (int)humidity, (int)(humidity * 10) % 10);
            LOG_D("read aht10 sensor temperature: %d.%d ", (int)temperature, (int)(temperature * 10) % 10);

            sensor_data[cnt].humidity = humidity;
            sensor_data[cnt].temperature = temperature;
            msg.data_ptr = (void *)(&sensor_data[cnt]);

            /* This message pointer is sent to the mq message queue */
            if (rt_mq_send(&mq, (void*)&msg, sizeof(MsgData_t)) != RT_EOK)
            {
                LOG_E("rt_mq_send ERR.");
                rt_thread_mdelay(500);
            }

            if ( rt_sem_take(aht10_sem, RT_WAITING_FOREVER) == RT_EOK )
            {
                /* Send event */
                rt_event_send(rtc_event, EVENT_FLAG3);
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
int aht10_sample_start(void)
{
    /* Create a thread with the name "aht10" and the entry "aht10_entry" */
    tid1 = rt_thread_create("aht10",
                            aht10_entry, RT_NULL,
                            THREAD_STACK_SIZE,
                            THREAD_PRIORITY, THREAD_TIMESLICE);

    /* If you get a thread control block, start the thread */
    if (tid1 != RT_NULL)
        rt_thread_startup(tid1);

    return RT_EOK;
}
