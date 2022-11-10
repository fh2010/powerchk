/*
 * Copyright (c) 2006-2021, RT-Thread Development Team
 *
 * SPDX-License-Identifier: Apache-2.0
 *
 * Change Logs:
 * Date           Author        Notes
 * 2012-04-25     weety         first version
 * 2021-04-20     RiceChen      added support for bus control api
 */

#ifndef __I2C_H__
#define __I2C_H__

#include <stdint.h>
#include "drv_config.h"

#ifdef __cplusplus
extern "C" {
#endif

#define RT_I2C_WR                0x0000
#define RT_I2C_RD               (1u << 0)
#define RT_I2C_ADDR_10BIT       (1u << 2)  /* this is a ten bit chip address */
#define RT_I2C_NO_START         (1u << 4)
#define RT_I2C_IGNORE_NACK      (1u << 5)
#define RT_I2C_NO_READ_ACK      (1u << 6)  /* when I2C reading, we do not ACK */
#define RT_I2C_NO_STOP          (1u << 7)

struct rt_i2c_msg
{
    uint16_t addr;
    uint16_t flags;
    uint16_t len;
    uint8_t  *buf;
};

struct rt_i2c_bus_device;

struct rt_i2c_bus_device_ops
{
    uint32_t (*master_xfer)(struct rt_i2c_bus_device *bus,
                             struct rt_i2c_msg msgs[],
                             uint32_t num);
    uint32_t (*slave_xfer)(struct rt_i2c_bus_device *bus,
                            struct rt_i2c_msg msgs[],
                            uint32_t num);
    uint32_t (*i2c_bus_control)(struct rt_i2c_bus_device *bus,
                                uint32_t,
                                uint32_t);
};

/*for i2c bus driver*/
struct rt_i2c_bus_device
{
    struct rt_device parent;
    const struct rt_i2c_bus_device_ops *ops;
    uint16_t  flags;
    struct rt_mutex lock;
    uint32_t  timeout;
    uint32_t  retries;
    void *priv;
};

struct rt_i2c_client
{
    struct rt_i2c_bus_device       *bus;
    uint16_t                    client_addr;
};

uint32_t rt_i2c_bus_device_register(struct rt_i2c_bus_device *bus,
                                    const char               *bus_name);
struct rt_i2c_bus_device *rt_i2c_bus_device_find(const char *bus_name);
uint32_t rt_i2c_transfer(struct rt_i2c_bus_device *bus,
                          struct rt_i2c_msg         msgs[],
                          uint32_t               num);
uint32_t rt_i2c_control(struct rt_i2c_bus_device *bus,
                        uint32_t               cmd,
                        uint32_t               arg);
uint32_t rt_i2c_master_send(struct rt_i2c_bus_device *bus,
                             uint16_t               addr,
                             uint16_t               flags,
                             const uint8_t         *buf,
                             uint32_t               count);
uint32_t rt_i2c_master_recv(struct rt_i2c_bus_device *bus,
                             uint16_t               addr,
                             uint16_t               flags,
                             uint8_t               *buf,
                             uint32_t               count);

__inline uint32_t rt_i2c_bus_lock(struct rt_i2c_bus_device *bus, uint32_t timeout)
{
    return rt_mutex_take(&bus->lock, timeout);
}

__inline uint32_t rt_i2c_bus_unlock(struct rt_i2c_bus_device *bus)
{
    return rt_mutex_release(&bus->lock);
}

int rt_i2c_core_init(void);

#ifdef __cplusplus
}
#endif

#endif
