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
#include <drv_lcd.h>
#include "ssd1306.h"
#include "alex.h"
#include "type_to_ascii.h"

#define DBG_TAG "lcd"
#define DBG_LVL DBG_INFO
#include <rtdbg.h>

#define THREAD_PRIORITY         23
#define THREAD_STACK_SIZE       1024
#define THREAD_TIMESLICE        20

static rt_thread_t tid1 = RT_NULL;

static time_t now;
static char humidity_menu[]       = "Hum :   . %";
static char temperature_menu[]    = "Temp:   .  ";
static char proximity_menu[]      = "Ps  :      ";
static char brightness_menu[]     = "Lux :    . ";
static char sec_clear[]           = " ";
#define CLEAR_POSITION              16

static void display_menu(void)
{
    lcd_clear(BLACK);
    lcd_set_color(BLACK, WHITE);
    /* get time */
    now = time(RT_NULL);
    lcd_show_string(24, 0, 24, ctime(&now)+8);
    lcd_show_string(24+12*CLEAR_POSITION, 0, 24, sec_clear);
    lcd_show_string(54, 62, 24, humidity_menu);
    lcd_show_string(54, 92, 24, temperature_menu);
    lcd_show_string(54, 122, 24, proximity_menu);
    lcd_show_string(54, 152, 24, brightness_menu);
}

/* The entry function of the thread */
static void lcd_entry(void *parameter)
{
    rt_ssize_t recv_size;
    rt_uint32_t recv;
    MsgData_t msg;
    Aht10Data_t *ath10_data_ptr;
    AP3216CData_t *ap3216c_data_ptr;
    static float humidity = 0.0;
    static float temperature = 0.0;
    static rt_uint16_t ps_data = 0;
    static float brightness = 0.0;
    unsigned char size;

    /* Menu init */
    display_menu();

    while (1)
    {
        /* Receive messages from the message queue into MSG */
        recv_size = rt_mq_recv(&mq, (void*)&msg, sizeof(MsgData_t), RT_WAITING_FOREVER);
        if (recv_size > 0)
        {
            switch ( msg.from )
            {
            case AHT10:
                /* The message is successfully received and the corresponding data processing is carried out */
                ath10_data_ptr = msg.data_ptr;
                humidity = ath10_data_ptr->humidity;
                temperature = ath10_data_ptr->temperature;

                /* Convert data into characters */
                int_to_ascii( (int)humidity, &humidity_menu[6], &size, 2, ' ' );
                int_to_ascii( (int)(humidity * 10) % 10, &humidity_menu[9], &size, 1, ' ' );
                int_to_ascii( (int)temperature, &temperature_menu[6], &size, 2, ' ' );
                int_to_ascii( (int)(temperature * 10) % 10, &temperature_menu[9], &size, 1, ' ' );

                rt_sem_release(aht10_sem);

                break;
            case AP3216C:
                /* The message is successfully received and the corresponding data processing is carried out */
                ap3216c_data_ptr = msg.data_ptr;
                ps_data = ap3216c_data_ptr->ps_data;
                brightness = ap3216c_data_ptr->brightness;

                /* Convert data into characters */
                int_to_ascii( (int)ps_data, &proximity_menu[6], &size, 4, ' ' );
                int_to_ascii( (int)brightness, &brightness_menu[5], &size, 4, ' ' );
                int_to_ascii( (int)(brightness * 10) % 10, &brightness_menu[10], &size, 1, ' ' );

                rt_sem_release(ap3216c_sem);

                break;
            default:
                break;
            }

            /* Forever wait, either Event 3 or 5 can trigger, event flag cleared after receiving it */
            if (rt_event_recv(rtc_event, (EVENT_FLAG3 | EVENT_FLAG5),
                              RT_EVENT_FLAG_AND | RT_EVENT_FLAG_CLEAR,
                              0, &recv) == RT_EOK);
            {
                /* get time */
                now = time(RT_NULL);
                LOG_D("%s", ctime(&now));
                lcd_show_string(24, 0, 24, ctime(&now)+8);
                lcd_show_string(24+12*CLEAR_POSITION, 0, 24, sec_clear);
                lcd_show_string(54, 62, 24, humidity_menu);
                lcd_show_string(54, 92, 24, temperature_menu);
                lcd_show_string(54, 122, 24, proximity_menu);
                lcd_show_string(54, 152, 24, brightness_menu);
            }

        } else {
            LOG_E("recv a message Fail.");
        }
    }
}

/* Example of a thread */
int lcd_sample_start(void)
{
    /* Create a thread with the name "oled" and the entry "oled_entry" */
    tid1 = rt_thread_create("lcd",
                            lcd_entry, RT_NULL,
                            THREAD_STACK_SIZE,
                            THREAD_PRIORITY, THREAD_TIMESLICE);

    /* If you get a thread control block, start the thread */
    if (tid1 != RT_NULL)
        rt_thread_startup(tid1);

    return RT_EOK;
}
