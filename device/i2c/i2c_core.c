/*
 * Copyright (c) 2006-2022, RT-Thread Development Team
 *
 * SPDX-License-Identifier: Apache-2.0
 *
 * Change Logs:
 * Date           Author        Notes
 * 2012-04-25     weety         first version
 * 2021-04-20     RiceChen      added support for bus control api
 */

#include "dev_console.h"
#include "i2c_dev.h"

uint32_t rt_i2c_bus_device_register(struct rt_i2c_bus_device *bus,
                                    const char               *bus_name)
{
    uint32_t res = DRV_SUCCESS;

    rt_mutex_init(&bus->lock, "i2c_bus_lock", RT_IPC_FLAG_PRIO);

    if (bus->timeout == 0) bus->timeout = RT_TICK_PER_SECOND;

    res = rt_i2c_bus_device_device_init(bus, bus_name);

    return res;
}

struct rt_i2c_bus_device *rt_i2c_bus_device_find(const char *bus_name)
{
    struct rt_i2c_bus_device *bus;
    rt_device_t dev = rt_device_find(bus_name);
    if (dev == NULL || dev->type != RT_Device_Class_I2CBUS)
    {
        return NULL;
    }

    bus = (struct rt_i2c_bus_device *)dev->user_data;

    return bus;
}

uint32_t rt_i2c_transfer(struct rt_i2c_bus_device *bus,
                          struct rt_i2c_msg         msgs[],
                          uint32_t               num)
{
    uint32_t ret;

    if (bus->ops->master_xfer)
    {

        rt_mutex_take(&bus->lock, RT_WAITING_FOREVER);
        ret = bus->ops->master_xfer(bus, msgs, num);
        rt_mutex_release(&bus->lock);

        return ret;
    }
    else
    {
        return 0;
    }
}

uint32_t rt_i2c_control(struct rt_i2c_bus_device *bus,
                        uint32_t               cmd,
                        uint32_t               arg)
{
    uint32_t ret;

    if(bus->ops->i2c_bus_control)
    {
        ret = bus->ops->i2c_bus_control(bus, cmd, arg);

        return ret;
    }
    else
    {
        return 0;
    }
}

uint32_t rt_i2c_master_send(struct rt_i2c_bus_device *bus,
                             uint16_t               addr,
                             uint16_t               flags,
                             const uint8_t         *buf,
                             uint32_t               count)
{
    uint32_t ret;
    struct rt_i2c_msg msg;

    msg.addr  = addr;
    msg.flags = flags;
    msg.len   = count;
    msg.buf   = (uint8_t *)buf;

    ret = rt_i2c_transfer(bus, &msg, 1);

    return (ret > 0) ? count : ret;
}

uint32_t rt_i2c_master_recv(struct rt_i2c_bus_device *bus,
                             uint16_t               addr,
                             uint16_t               flags,
                             uint8_t               *buf,
                             uint32_t               count)
{
    uint32_t ret;
    struct rt_i2c_msg msg;
    assert_param(bus != NULL);

    msg.addr   = addr;
    msg.flags  = flags | RT_I2C_RD;
    msg.len    = count;
    msg.buf    = buf;

    ret = rt_i2c_transfer(bus, &msg, 1);

    return (ret > 0) ? count : ret;
}

int rt_i2c_core_init(void)
{
    return 0;
}
