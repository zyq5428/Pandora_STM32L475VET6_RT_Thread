/*
 * Copyright (c) 2006-2021, RT-Thread Development Team
 *
 * SPDX-License-Identifier: Apache-2.0
 *
 * Change Logs:
 * Date           Author        Notes
 * 2021-10-17     Meco Man      first version
 * 2022-05-10     Meco Man      improve rt-thread initialization process
 */
#include <lvgl.h>
#include <board.h>
#include <rtthread.h>
#ifndef RT_USING_NANO
#include <rtdevice.h>
#endif /* RT_USING_NANO */

/* include private header file */
#include "alex.h"

#define DBG_TAG "lvgl_sample"
#define DBG_LVL DBG_INFO
#include <rtdbg.h>

#define THREAD_PRIORITY         23
#define THREAD_STACK_SIZE       1024
#define THREAD_TIMESLICE        5

static rt_thread_t tid1 = RT_NULL;

/* led status code */
static rt_uint8_t led_code = 0;
static float humidity = 65.0;
static float temperature = 27.0;
static rt_uint16_t ps_data = 100;
static float brightness = 555.0;
static time_t now;
static char *ptime;

static void update_time(void)
{
    /* get time */
    now = time(RT_NULL);
    ptime = ctime(&now);
    LOG_D("%s", p);
}

static void event_handler(lv_event_t * e)
{
    lv_event_code_t code = lv_event_get_code(e);

    if(code == LV_EVENT_CLICKED) {
        LV_LOG_USER("Clicked");
    }
}

static void set_status(lv_timer_t * timer)
{
    LV_UNUSED(timer);
//    lv_obj_t * btn = timer->user_data;
//    if (led_code)
//    {
//        lv_obj_set_style_bg_color(btn, lv_palette_main(LV_PALETTE_GREEN), LV_PART_MAIN);
//        lv_label_set_text(lv_obj_get_child(btn, 0), "ON");
//    } else {
//        lv_obj_set_style_bg_color(btn, lv_palette_main(LV_PALETTE_GREY), LV_PART_MAIN);
//        lv_label_set_text(lv_obj_get_child(btn, 0), "OFF");
//    }
    lv_obj_t * label = timer->user_data;
    lv_label_set_text_fmt(label,
            "%s\n"
            "Temperature: %d°C\n\n"
            "Humidity: %d%%\n\n"
            "Brightness: %d Lux\n\n"
            "Proximity: %d mm",
            ptime,
            (rt_uint16_t)temperature,
            (rt_uint16_t)humidity,
            (rt_uint16_t)brightness,
            ps_data);
}

static void lv_example(void)
{
    lv_obj_t * btn = lv_btn_create(lv_scr_act());
    lv_obj_add_event_cb(btn, event_handler, LV_EVENT_ALL, NULL);
    lv_obj_align(btn, LV_ALIGN_CENTER, 0, -80);

    lv_obj_t * led_label = lv_label_create(btn);
    lv_label_set_text(led_label, "LED");
    lv_obj_center(led_label);

    update_time();

    lv_obj_t * main_label = lv_label_create(lv_scr_act());
    lv_label_set_text_fmt(main_label,
            "%s\n"
            "Temperature: %d°C\n\n"
            "Humidity: %d%%\n\n"
            "Brightness: %d Lux\n\n"
            "Proximity: %d mm",
            ptime,
            (rt_uint16_t)temperature,
            (rt_uint16_t)humidity,
            (rt_uint16_t)brightness,
            ps_data);
    lv_obj_align(main_label, LV_ALIGN_CENTER, 0, 0);

    lv_timer_create(set_status, 100, main_label);
}

void lv_user_gui_init(void)
{
    /* display demo; you may replace with your LVGL application at here */
//    extern void lv_example_menu_5(void);
//    lv_example_menu_5();
    lv_example();
}

/* The entry function of the thread */
static void lvgl_entry(void *parameter)
{
    rt_ssize_t recv_size;
    rt_uint32_t recv;
    MsgData_t msg;
    Aht10Data_t *ath10_data_ptr;
    AP3216CData_t *ap3216c_data_ptr;

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

                rt_sem_release(aht10_sem);

                break;
            case AP3216C:
                /* The message is successfully received and the corresponding data processing is carried out */
                ap3216c_data_ptr = msg.data_ptr;
                ps_data = ap3216c_data_ptr->ps_data;
                brightness = ap3216c_data_ptr->brightness;

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
                update_time();
            }

        } else {
            LOG_E("recv a message Fail.");
        }
    }
}

/* Example of a thread */
int lvgl_sample_start(void)
{
    /* Create a thread with the name "led" and the entry "led_entry" */
    tid1 = rt_thread_create("lvgl_sample",
                            lvgl_entry,
                            RT_NULL,
                            THREAD_STACK_SIZE,
                            THREAD_PRIORITY,
                            THREAD_TIMESLICE);

    /* If you get a thread control block, start the thread */
    if (tid1 != RT_NULL)
        rt_thread_startup(tid1);

    return RT_EOK;
}
