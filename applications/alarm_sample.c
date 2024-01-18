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
#include <rtdevice.h>
#include "alex.h"

#define DBG_TAG "alarm"
#define DBG_LVL DBG_LOG
#include <rtdbg.h>

#define THREAD_PRIORITY         18
#define THREAD_STACK_SIZE       1024
#define THREAD_TIMESLICE        5

#define RTC_NAME       "rtc"

static rt_thread_t tid1 = RT_NULL;

#define MINUTE_TIMESLICE        30

void user_alarm_callback(rt_alarm_t alarm, time_t timestamp)
{
//    rt_event_send(rtc_event, EVENT_FLAG1);
    static u_int8_t sec = 0;

    if ( sec < (u_int8_t)(60 / MINUTE_TIMESLICE) )
        sec++;
    else {
        /* Send event */
        rt_event_send(rtc_event, EVENT_FLAG1);
        sec = 0;
    }
}

/* The entry function of the thread */
static void alarm_entry(void *parameter)
{
    rt_err_t ret = RT_EOK;

    /* RTC related parameters */
    time_t now;
    rt_device_t device = RT_NULL;

    /* Alarm related parameters */
    struct rt_alarm_setup setup;
    struct rt_alarm * alarm = RT_NULL;
    struct tm p_tm;

    /* Find device */
    device = rt_device_find(RTC_NAME);
    if (!device)
    {
        LOG_E("find %s failed!", RTC_NAME);
        return;
    }
    /* init RTC device */
    if(rt_device_open(device, 0) != RT_EOK)
    {
        LOG_E("open %s failed!", RTC_NAME);
        return;
    }
    /* set date */
    ret = set_date(2024, 1, 1);
    if (ret != RT_EOK)
    {
        LOG_E("set RTC date failed");
        return;
    }
    /* set time */
    ret = set_time(0, 0, 0);
    if (ret != RT_EOK)
    {
        LOG_E("set RTC time failed");
        return;
    }

    /* Get the current timestamp and set the next 2 seconds as the alarm time */
    now = time(RT_NULL) + 5;
    gmtime_r(&now,&p_tm);
    LOG_D("set Alarm:%s", ctime(&now));

    setup.flag = RT_ALARM_SECOND;
//    setup.flag = RT_ALARM_MINUTE;
    setup.wktime.tm_year = p_tm.tm_year;
    setup.wktime.tm_mon = p_tm.tm_mon;
    setup.wktime.tm_mday = p_tm.tm_mday;
    setup.wktime.tm_wday = p_tm.tm_wday;
    setup.wktime.tm_hour = p_tm.tm_hour;
    setup.wktime.tm_min = p_tm.tm_min;
    setup.wktime.tm_sec = p_tm.tm_sec;

    alarm = rt_alarm_create(user_alarm_callback, &setup);
    if(RT_NULL != alarm)
    {
        rt_alarm_start(alarm);
    }
}

/* Example of a thread */
int alarm_sample_start(void)
{
    /* Create a thread with the name "alarm" and the entry "alarm_entry" */
    tid1 = rt_thread_create("alarm",
                            alarm_entry, RT_NULL,
                            THREAD_STACK_SIZE,
                            THREAD_PRIORITY, THREAD_TIMESLICE);

    /* If you get a thread control block, start the thread */
    if (tid1 != RT_NULL)
        rt_thread_startup(tid1);

    return RT_EOK;
}
