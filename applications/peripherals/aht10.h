/*
 * Copyright (c) 2006-2021, RT-Thread Development Team
 *
 * SPDX-License-Identifier: Apache-2.0
 *
 * Change Logs:
 * Date           Author       Notes
 * 2023-03-25     59427       the first version
 */
#ifndef APPLICATIONS_PERIPHERALS_AHT10_H_
#define APPLICATIONS_PERIPHERALS_AHT10_H_

extern void aht10_init(const char *name);
extern void read_temp_humi(float *cur_temp, float *cur_humi);

#endif /* APPLICATIONS_PERIPHERALS_AHT10_H_ */
