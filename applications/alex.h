/*
 * Copyright (c) 2006-2021, RT-Thread Development Team
 *
 * SPDX-License-Identifier: Apache-2.0
 *
 * Change Logs:
 * Date           Author       Notes
 * 2023-03-25     59427       the first version
 */
#ifndef APPLICATIONS_ALEX_H_
#define APPLICATIONS_ALEX_H_

typedef struct __MsgData_t{
    rt_uint8_t from;    /* Sender */
    rt_uint8_t to;    /* Recipients */
    void *data_ptr;    /* Pointers to message data structures */
    rt_uint32_t data_size;   /* Data size*/
} MsgData_t;
typedef struct __MsgData_t *MsgData_ptr;

typedef struct {
    float humidity;    /* Humidity value */
    float temperature;    /* Temperature value */
} Aht10Data_t;

typedef struct {
    rt_uint16_t ps_data;    /* Proximity value */
    float brightness;    /* Brightness value */
} AP3216CData_t;

typedef enum {
    LED,
    OLED,
    ALARM,
    AHT10,
    AP3216C,
} ADDR_ListTypeDef;


#define EVENT_FLAG1 (1 << 1)
#define EVENT_FLAG3 (1 << 3)
#define EVENT_FLAG5 (1 << 5)

/* Event control block */
extern rt_event_t rtc_event;
extern rt_event_t lvgl_event;

/* pointer to a semaphore */
extern rt_sem_t rtc_sem;
extern rt_sem_t aht10_sem;
extern rt_sem_t ap3216c_sem;

/* Message Queuing control block */
extern struct rt_messagequeue mq;

#endif /* APPLICATIONS_ALEX_H_ */
