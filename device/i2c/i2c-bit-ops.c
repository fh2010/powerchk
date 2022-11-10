/*
 * Copyright (c) 2006-2022, RT-Thread Development Team
 *
 * SPDX-License-Identifier: Apache-2.0
 *
 * Change Logs:
 * Date           Author        Notes
 * 2012-04-25     weety         first version
 */

#include "dev_console.h"
#include "i2c-bit-ops.h"
#include "i2c.h"

#define SET_SDA(ops, val)   ops->set_sda(ops->data, val)
#define SET_SCL(ops, val)   ops->set_scl(ops->data, val)
#define GET_SDA(ops)        ops->get_sda(ops->data)
#define GET_SCL(ops)        ops->get_scl(ops->data)

__inline void i2c_delay(struct rt_i2c_bit_ops *ops)
{
    ops->udelay((ops->delay_us + 1) >> 1);
}

__inline void i2c_delay2(struct rt_i2c_bit_ops *ops)
{
    ops->udelay(ops->delay_us);
}

#define SDA_L(ops)          SET_SDA(ops, 0)
#define SDA_H(ops)          SET_SDA(ops, 1)
#define SCL_L(ops)          SET_SCL(ops, 0)

/**
 * release scl line, and wait scl line to high.
 */
static uint32_t SCL_H(struct rt_i2c_bit_ops *ops)
{
    uint32_t start;

    SET_SCL(ops, 1);

    if (!ops->get_scl)
        goto done;

    start = rt_tick_get();
    while (!GET_SCL(ops))
    {
        if ((rt_tick_get() - start) > ops->timeout)
            return -RT_ETIMEOUT;
        rt_thread_delay((ops->timeout + 1) >> 1);
    }

done:
    i2c_delay(ops);

    return DRV_SUCCESS;
}

static void i2c_start(struct rt_i2c_bit_ops *ops)
{
    SDA_L(ops);
    i2c_delay(ops);
    SCL_L(ops);
}

static void i2c_restart(struct rt_i2c_bit_ops *ops)
{
    SDA_H(ops);
    SCL_H(ops);
    i2c_delay(ops);
    SDA_L(ops);
    i2c_delay(ops);
    SCL_L(ops);
}

static void i2c_stop(struct rt_i2c_bit_ops *ops)
{
    SDA_L(ops);
    i2c_delay(ops);
    SCL_H(ops);
    i2c_delay(ops);
    SDA_H(ops);
    i2c_delay2(ops);
}

__inline bool i2c_waitack(struct rt_i2c_bit_ops *ops)
{
    bool ack;

    SDA_H(ops);
    i2c_delay(ops);

    if (SCL_H(ops) < 0)
    {
        return DRV_ERROR_TIMEOUT;
    }

    ack = !GET_SDA(ops);    /* ACK : SDA pin is pulled low */
    SCL_L(ops);

    return ack;
}

static int32_t i2c_writeb(struct rt_i2c_bus_device *bus, uint8_t data)
{
    int32_t i;
    uint8_t bit;

    struct rt_i2c_bit_ops *ops = (struct rt_i2c_bit_ops *)bus->priv;

    for (i = 7; i >= 0; i--)
    {
        SCL_L(ops);
        bit = (data >> i) & 1;
        SET_SDA(ops, bit);
        i2c_delay(ops);
        if (SCL_H(ops) < 0)
        {
            return DRV_ERROR_TIMEOUT;
        }
    }
    SCL_L(ops);
    i2c_delay(ops);

    return i2c_waitack(ops);
}

static int32_t i2c_readb(struct rt_i2c_bus_device *bus)
{
    uint8_t i;
    uint8_t data = 0;
    struct rt_i2c_bit_ops *ops = (struct rt_i2c_bit_ops *)bus->priv;

    SDA_H(ops);
    i2c_delay(ops);
    for (i = 0; i < 8; i++)
    {
        data <<= 1;

        if (SCL_H(ops) < 0)
        {
            return DRV_ERROR_TIMEOUT;
        }

        if (GET_SDA(ops))
            data |= 1;
        SCL_L(ops);
        i2c_delay2(ops);
    }

    return data;
}

static uint32_t i2c_send_bytes(struct rt_i2c_bus_device *bus,
                                struct rt_i2c_msg        *msg)
{
    int32_t ret;
    uint32_t bytes = 0;
    const uint8_t *ptr = msg->buf;
    int32_t count = msg->len;
    uint16_t ignore_nack = msg->flags & RT_I2C_IGNORE_NACK;

    while (count > 0)
    {
        ret = i2c_writeb(bus, *ptr);

        if ((ret > 0) || (ignore_nack && (ret == 0)))
        {
            count --;
            ptr ++;
            bytes ++;
        }
        else if (ret == 0)
        {

            return 0;
        }
        else
        {
            return ret;
        }
    }

    return bytes;
}

static uint32_t i2c_send_ack_or_nack(struct rt_i2c_bus_device *bus, int ack)
{
    struct rt_i2c_bit_ops *ops = (struct rt_i2c_bit_ops *)bus->priv;

    if (ack)
        SET_SDA(ops, 0);
    i2c_delay(ops);
    if (SCL_H(ops) < 0)
    {
        return DRV_ERROR_TIMEOUT;
    }
    SCL_L(ops);

    return DRV_SUCCESS;
}

static uint32_t i2c_recv_bytes(struct rt_i2c_bus_device *bus,
                                struct rt_i2c_msg        *msg)
{
    int32_t val;
    int32_t bytes = 0;   /* actual bytes */
    uint8_t *ptr = msg->buf;
    int32_t count = msg->len;
    const uint32_t flags = msg->flags;

    while (count > 0)
    {
        val = i2c_readb(bus);
        if (val >= 0)
        {
            *ptr = val;
            bytes ++;
        }
        else
        {
            break;
        }

        ptr ++;
        count --;

        if (!(flags & RT_I2C_NO_READ_ACK))
        {
            val = i2c_send_ack_or_nack(bus, count);
            if (val < 0)
                return val;
        }
    }

    return bytes;
}

static int32_t i2c_send_address(struct rt_i2c_bus_device *bus,
                                   uint8_t                addr,
                                   int32_t                retries)
{
    struct rt_i2c_bit_ops *ops = (struct rt_i2c_bit_ops *)bus->priv;
    int32_t i;
    uint32_t ret = 0;

    for (i = 0; i <= retries; i++)
    {
        ret = i2c_writeb(bus, addr);
        if (ret == 1 || i == retries)
            break;
        i2c_stop(ops);
        i2c_delay2(ops);
        i2c_start(ops);
    }

    return ret;
}

static uint32_t i2c_bit_send_address(struct rt_i2c_bus_device *bus,
                                     struct rt_i2c_msg        *msg)
{
    uint16_t flags = msg->flags;
    uint16_t ignore_nack = msg->flags & RT_I2C_IGNORE_NACK;
    struct rt_i2c_bit_ops *ops = (struct rt_i2c_bit_ops *)bus->priv;

    uint8_t addr1, addr2;
    int32_t retries;
    uint32_t ret;

    retries = ignore_nack ? 0 : bus->retries;

    if (flags & RT_I2C_ADDR_10BIT)
    {
        addr1 = 0xf0 | ((msg->addr >> 7) & 0x06);
        addr2 = msg->addr & 0xff;
        ret = i2c_send_address(bus, addr1, retries);
        if ((ret != 1) && !ignore_nack)
        {
            return DRV_ERROR_BUSY;
        }

        ret = i2c_writeb(bus, addr2);
        if ((ret != 1) && !ignore_nack)
        {
            return DRV_ERROR_BUSY;
        }
        if (flags & RT_I2C_RD)
        {
            i2c_restart(ops);
            addr1 |= 0x01;
            ret = i2c_send_address(bus, addr1, retries);
            if ((ret != 1) && !ignore_nack)
            {
                return DRV_ERROR_BUSY;
            }
        }
    }
    else
    {
        /* 7-bit addr */
        addr1 = msg->addr << 1;
        if (flags & RT_I2C_RD)
            addr1 |= 1;
        ret = i2c_send_address(bus, addr1, retries);
        if ((ret != 1) && !ignore_nack)
            return DRV_ERROR_BUSY;
    }

    return DRV_SUCCESS;
}

static uint32_t i2c_bit_xfer(struct rt_i2c_bus_device *bus,
                              struct rt_i2c_msg         msgs[],
                              uint32_t               num)
{
    struct rt_i2c_msg *msg;
    struct rt_i2c_bit_ops *ops = (struct rt_i2c_bit_ops *)bus->priv;
    int32_t ret;
    uint32_t i;
    uint16_t ignore_nack;

    if (num == 0) return 0;

    for (i = 0; i < num; i++)
    {
        msg = &msgs[i];
        ignore_nack = msg->flags & RT_I2C_IGNORE_NACK;
        if (!(msg->flags & RT_I2C_NO_START))
        {
            if (i)
            {
                i2c_restart(ops);
            }
            else
            {
                i2c_start(ops);
            }
            ret = i2c_bit_send_address(bus, msg);
            if ((ret != DRV_SUCCESS) && !ignore_nack)
            {
                goto out;
            }
        }
        if (msg->flags & RT_I2C_RD)
        {
            ret = i2c_recv_bytes(bus, msg);
            if (ret >= 1)
            {

            }
            if (ret < msg->len)
            {
                if (ret >= 0)
                    ret = DRV_ERROR_BUSY;
                goto out;
            }
        }
        else
        {
            ret = i2c_send_bytes(bus, msg);
            if (ret >= 1)
            {

            }
            if (ret < msg->len)
            {
                if (ret >= 0)
                    ret = DRV_ERROR_INTERNAL;
                goto out;
            }
        }
    }
    ret = i;

out:
    if (!(msg->flags & RT_I2C_NO_STOP))
    {
        i2c_stop(ops);
    }

    return ret;
}

static const struct rt_i2c_bus_device_ops i2c_bit_bus_ops =
{
    i2c_bit_xfer,
    NULL,
    NULL
};

uint32_t rt_i2c_bit_add_bus(struct rt_i2c_bus_device *bus,
                            const char               *bus_name)
{
    bus->ops = &i2c_bit_bus_ops;

    return rt_i2c_bus_device_register(bus, bus_name);
}
