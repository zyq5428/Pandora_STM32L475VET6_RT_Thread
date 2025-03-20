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
    lv_obj_t * btn = timer->user_data;
    if (led_code)
    {
        lv_obj_set_style_bg_color(btn, lv_palette_main(LV_PALETTE_GREEN), LV_PART_MAIN);
        lv_label_set_text(lv_obj_get_child(btn, 0), "ON");
    } else {
        lv_obj_set_style_bg_color(btn, lv_palette_main(LV_PALETTE_GREY), LV_PART_MAIN);
        lv_label_set_text(lv_obj_get_child(btn, 0), "OFF");
    }
}

static void lv_example(void)
{
    int temperature = 25;
    int humidity = 65;
    int brightness = 1000;
    int proximity = 88;

    lv_obj_t * btn = lv_btn_create(lv_scr_act());
    lv_obj_add_event_cb(btn, event_handler, LV_EVENT_ALL, NULL);
    lv_obj_align(btn, LV_ALIGN_CENTER, 0, -80);

    lv_obj_t * led_label = lv_label_create(btn);
    lv_label_set_text(led_label, "LED");
    lv_obj_center(led_label);

    lv_obj_t * temperature_label = lv_label_create(lv_scr_act());
    lv_obj_align(temperature_label, LV_ALIGN_CENTER, 0, -40);
    lv_label_set_text_fmt(temperature_label, "Temperature: %d°C", temperature);

    lv_obj_t * humidity_label = lv_label_create(lv_scr_act());
    lv_obj_align(humidity_label, LV_ALIGN_CENTER, 0, 0);
    lv_label_set_text_fmt(humidity_label, "Humidity: %d%%", humidity);

    lv_obj_t * brightness_label = lv_label_create(lv_scr_act());
    lv_obj_align(brightness_label, LV_ALIGN_CENTER, 0, 40);
    lv_label_set_text_fmt(brightness_label, "Brightness: %d Lux", brightness);

    lv_obj_t * proximity_label = lv_label_create(lv_scr_act());
    lv_obj_align(proximity_label, LV_ALIGN_CENTER, 0, 80);
    lv_label_set_text_fmt(proximity_label, "Proximity: %d mm", proximity);

    lv_timer_create(set_status, 100, btn);
}

void lv_user_gui_init(void)
{
    /* display demo; you may replace with your LVGL application at here */
//    extern void lv_demo_calendar(void);
//    lv_demo_calendar();
     lv_example();
}

/* The entry function of the thread */
static void lvgl_entry(void *parameter)
{
    rt_uint32_t recv;

    LOG_D("lvgl thread running");

    while (1)
    {
        /* Forever wait, either Event 1 can trigger, event flag cleared after receiving it */
        if (rt_event_recv(lvgl_event, EVENT_FLAG1 | EVENT_FLAG3,
                          RT_EVENT_FLAG_OR | RT_EVENT_FLAG_CLEAR,
                          RT_WAITING_FOREVER, &recv) == RT_EOK)
        {
            if (recv & EVENT_FLAG1)
            {
                led_code = 0;
                LOG_D("Received EVENT_FLAG1, led_code set to 0");
            }
            else if (recv & EVENT_FLAG3)
            {
                led_code = 1;
                LOG_D("Received EVENT_FLAG3, led_code set to 1");
            }
            else
            {
                LOG_W("Received unexpected event flags: 0x%x", recv);
            }
        }
        else
        {
            LOG_E("Event receive failed");
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
