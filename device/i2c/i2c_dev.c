/*
 * Copyright (c) 2006-2021, RT-Thread Development Team
 *
 * SPDX-License-Identifier: Apache-2.0
 *
 * Change Logs:
 * Date           Author        Notes
 * 2012-04-25     weety         first version
 * 2014-08-03     bernard       fix some compiling warning
 * 2021-04-20     RiceChen      added support for bus clock control
 */

#include "dev_console.h"
#include "i2c_dev.h"

static uint32_t i2c_bus_device_read(rt_device_t dev,
                                     uint32_t    pos,
                                     void       *buffer,
                                     uint32_t   count)
{
    uint16_t addr;
    uint16_t flags;
    struct rt_i2c_bus_device *bus = (struct rt_i2c_bus_device *)dev->user_data;

    assert_param(bus != NULL);
    assert_param(buffer != NULL);

    addr = pos & 0xffff;
    flags = (pos >> 16) & 0xffff;

    return rt_i2c_master_recv(bus, addr, flags, (uint8_t *)buffer, count);
}

static uint32_t i2c_bus_device_write(rt_device_t dev,
                                      uint32_t    pos,
                                      const void *buffer,
                                      uint32_t   count)
{
    uint16_t addr;
    uint16_t flags;
    struct rt_i2c_bus_device *bus = (struct rt_i2c_bus_device *)dev->user_data;

    assert_param(bus != NULL);
    assert_param(buffer != NULL);

    addr = pos & 0xffff;
    flags = (pos >> 16) & 0xffff;

    return rt_i2c_master_send(bus, addr, flags, (const uint8_t *)buffer, count);
}

static uint32_t i2c_bus_device_control(rt_device_t dev,
                                       int         cmd,
                                       void       *args)
{
    uint32_t ret;
    struct rt_i2c_priv_data *priv_data;
    struct rt_i2c_bus_device *bus = (struct rt_i2c_bus_device *)dev->user_data;
    uint32_t bus_clock;

    assert_param(bus != NULL);

    switch (cmd)
    {
    /* set 10-bit addr mode */
    case RT_I2C_DEV_CTRL_10BIT:
        bus->flags |= RT_I2C_ADDR_10BIT;
        break;
    case RT_I2C_DEV_CTRL_TIMEOUT:
        bus->timeout = *(uint32_t *)args;
        break;
    case RT_I2C_DEV_CTRL_RW:
        priv_data = (struct rt_i2c_priv_data *)args;
        ret = rt_i2c_transfer(bus, priv_data->msgs, priv_data->number);
        if (ret < 0)
        {
            return DRV_ERROR_BUSY;
        }
        break;
    case RT_I2C_DEV_CTRL_CLK:
        bus_clock = *(uint32_t *)args;
        ret = rt_i2c_control(bus, cmd, bus_clock);
        if (ret < 0)
        {
            return DRV_ERROR_BUSY;
        }
        break;
    default:
        break;
    }

    return DRV_SUCCESS;
}

#ifdef RT_USING_DEVICE_OPS
const static struct rt_device_ops i2c_ops =
{
    NULL,
    NULL,
    NULL,
    i2c_bus_device_read,
    i2c_bus_device_write,
    i2c_bus_device_control
};
#endif

uint32_t rt_i2c_bus_device_device_init(struct rt_i2c_bus_device *bus,
                                       const char               *name)
{
    struct rt_device *device;
    assert_param(bus != NULL);

    device = &bus->parent;

    device->user_data = bus;

    /* set device type */
    device->type    = RT_Device_Class_I2CBUS;
    /* initialize device interface */
#ifdef RT_USING_DEVICE_OPS
    device->ops     = &i2c_ops;
#else
    device->init    = NULL;
    device->open    = NULL;
    device->close   = NULL;
    device->read    = i2c_bus_device_read;
    device->write   = i2c_bus_device_write;
    device->control = i2c_bus_device_control;
#endif

    /* register to device manager */
    rt_device_register(device, name, RT_DEVICE_FLAG_RDWR);

    return DRV_SUCCESS;
}
