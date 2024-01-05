#include <rtthread.h>
#include <rtdevice.h>

#define DBG_TAG "AHT10"
#define DBG_LVL DBG_LOG
#include <rtdbg.h>

#define AHT10_ADDR                  0x38    /* Slave address */
#define AHT10_CALIBRATION_CMD       0xE1    /* Calibration command */
#define AHT10_NORMAL_CMD            0xA8    /* Normal command */
#define AHT10_GET_DATA              0xAC    /* GET_Data command */

static struct rt_i2c_bus_device *i2c_bus = RT_NULL;     /* I2C bus device handle */
static rt_bool_t initialized = RT_FALSE;                /* Sensor initialization status */

/* Write sensor registers */
static rt_err_t write_reg(struct rt_i2c_bus_device *bus, rt_uint8_t reg, rt_uint8_t *data)
{
    rt_uint8_t buf[3];
    struct rt_i2c_msg msgs;

    buf[0] = reg;
    buf[1] = data[0];
    buf[2] = data[1];

    msgs.addr = AHT10_ADDR;
    msgs.flags = RT_I2C_WR;
    msgs.buf = buf;
    msgs.len = 3;

    /* Call the I2C device interface to transmit data */
    if (rt_i2c_transfer(bus, &msgs, 1) == 1)
    {
        return RT_EOK;
    }
    else
    {
        return -RT_ERROR;
    }
}

/* Read sensor registers */
static rt_err_t read_regs(struct rt_i2c_bus_device *bus, rt_uint8_t len, rt_uint8_t *buf)
{
    struct rt_i2c_msg msgs;

    msgs.addr = AHT10_ADDR;
    msgs.flags = RT_I2C_RD;
    msgs.buf = buf;
    msgs.len = len;

    /* Call the I2C device interface to transmit data */
    if (rt_i2c_transfer(bus, &msgs, 1) == 1)
    {
        return RT_EOK;
    }
    else
    {
        return -RT_ERROR;
    }
}

void read_temp_humi(float *cur_temp, float *cur_humi)
{
    rt_uint8_t temp[6];

    write_reg(i2c_bus, AHT10_GET_DATA, 0);      /* Send cmd */
    rt_thread_mdelay(400);
    read_regs(i2c_bus, 6, temp);                /* Get sensor data */

    /* Humidity data conversion */
    *cur_humi = (temp[1] << 12 | temp[2] << 4 | (temp[3] & 0xf0) >> 4) * 100.0 / (1 << 20);
    /* Temperature data conversion */
    *cur_temp = ((temp[3] & 0xf) << 16 | temp[4] << 8 | temp[5]) * 200.0 / (1 << 20) - 50;
}

void aht10_init(const char *name)
{
    rt_uint8_t temp[2] = {0, 0};

    /* Find the I2C bus device and get the I2C bus device handle */
    i2c_bus = (struct rt_i2c_bus_device *)rt_device_find(name);

    if (i2c_bus == RT_NULL)
    {
        LOG_D("can't find %s device!", name);
    }
    else
    {
        write_reg(i2c_bus, AHT10_NORMAL_CMD, temp);
        rt_thread_mdelay(400);

        temp[0] = 0x08;
        temp[1] = 0x00;
        write_reg(i2c_bus, AHT10_CALIBRATION_CMD, temp);
        rt_thread_mdelay(400);
        initialized = RT_TRUE;
    }
}

