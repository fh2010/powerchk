#include "drv_config.h"
#include "drv_server.h"


#define device_init     (dev->ops->init)
#define device_open     (dev->ops->open)
#define device_close    (dev->ops->close)
#define device_read     (dev->ops->read)
#define device_write    (dev->ops->write)
#define device_control  (dev->ops->control)


/**
 * This function registers a device driver with specified name.
 *
 * @param dev the pointer of device driver structure
 * @param name the device driver's name
 * @param flags the capabilities flag of device
 *
 * @return the error code, DRV_SUCCESS on initialization successfully.
 */
uint32_t rt_device_register(rt_device_t dev,
                            const char *name,
                            uint16_t flags)
{
    if (dev == NULL)
        return DRV_ERROR_INVALID_PARAM;

    if (rt_device_find(name) != NULL)
        return DRV_ERROR_INVALID_PARAM;

    rt_object_init(&(dev->parent), RT_Object_Class_Device, name);
    dev->flag = flags;
    dev->ref_count = 0;
    dev->open_flag = 0;
    return DRV_SUCCESS;
}


/**
 * This function removes a previously registered device driver
 *
 * @param dev the pointer of device driver structure
 *
 * @return the error code, DRV_SUCCESS on successfully.
 */
uint32_t rt_device_unregister(rt_device_t dev)
{
    assert_param(dev != NULL);
    assert_param(rt_object_get_type(&dev->parent) == RT_Object_Class_Device);
    assert_param(rt_object_is_systemobject(&dev->parent));

    rt_object_detach(&(dev->parent));

    return DRV_SUCCESS;
}

/**
 * This function initializes all registered device driver
 *
 * @return the error code, DRV_SUCCESS on successfully.
 *
 * @deprecated since 1.2.x, this function is not needed because the initialization
 *             of a device is performed when application opens it.
 */
uint32_t rt_device_init_all(void)
{
    return DRV_SUCCESS;
}

/**
 * This function finds a device driver by specified name.
 *
 * @param name the device driver's name
 *
 * @return the registered device driver on successful, or NULL on failure.
 */
rt_device_t rt_device_find(const char *name)
{
    struct rt_object *object;
    struct rt_list_node *node;
    struct rt_object_information *information;

    /* enter critical */
    rt_enter_critical();

    /* try to find device object */
    information = rt_object_get_information(RT_Object_Class_Device);
    assert_param(information != NULL);
    for (node  = information->object_list.next;
         node != &(information->object_list);
         node  = node->next)
    {
        object = rt_list_entry(node, struct rt_object, list);
        if (strncmp(object->name, name, RT_NAME_MAX) == 0)
        {
            /* leave critical */
            rt_exit_critical();

            return (rt_device_t)object;
        }
    }

    /* leave critical */
    rt_exit_critical();

    /* not found */
    return NULL;
}

#ifdef RT_USING_HEAP
/**
 * This function creates a device object with user data size.
 *
 * @param type, the kind type of this device object.
 * @param attach_size, the size of user data.
 *
 * @return the allocated device object, or NULL when failed.
 */
rt_device_t rt_device_create(int type, int attach_size)
{
    int size;
    rt_device_t device;

    size = RT_ALIGN(sizeof(struct rt_device), RT_ALIGN_SIZE);
    attach_size = RT_ALIGN(attach_size, RT_ALIGN_SIZE);
    /* use the total size */
    size += attach_size;

    device = (rt_device_t)rt_malloc(size);
    if (device)
    {
        rt_memset(device, 0x0, sizeof(struct rt_device));
        device->type = (enum rt_device_class_type)type;
    }

    return device;
}


/**
 * This function destroy the specific device object.
 *
 * @param dev, the specific device object.
 */
void rt_device_destroy(rt_device_t dev)
{
    assert_param(dev != NULL);
    assert_param(rt_object_get_type(&dev->parent) == RT_Object_Class_Device);
    assert_param(rt_object_is_systemobject(&dev->parent) == RT_FALSE);

    rt_object_detach(&(dev->parent));

    /* release this device object */
    rt_free(dev);
}

#endif

/**
 * This function will initialize the specified device
 *
 * @param dev the pointer of device driver structure
 *
 * @return the result
 */
uint32_t rt_device_init(rt_device_t dev)
{
    uint32_t result = DRV_SUCCESS;

    assert_param(dev != NULL);

    /* get device_init handler */
    if (device_init != NULL)
    {
        if (!(dev->flag & RT_DEVICE_FLAG_ACTIVATED))
        {
            result = device_init(dev);
            if (result != DRV_SUCCESS)
            {

            }
            else
            {
                dev->flag |= RT_DEVICE_FLAG_ACTIVATED;
            }
        }
    }

    return result;
}

/**
 * This function will open a device
 *
 * @param dev the pointer of device driver structure
 * @param oflag the flags for device open
 *
 * @return the result
 */
uint32_t rt_device_open(rt_device_t dev, uint16_t oflag)
{
    uint32_t result = DRV_SUCCESS;

    assert_param(dev != NULL);
    assert_param(rt_object_get_type(&dev->parent) == RT_Object_Class_Device);

    /* if device is not initialized, initialize it. */
    if (!(dev->flag & RT_DEVICE_FLAG_ACTIVATED))
    {
        if (device_init != NULL)
        {
            result = device_init(dev);
            if (result != DRV_SUCCESS)
            {
                return result;
            }
        }

        dev->flag |= RT_DEVICE_FLAG_ACTIVATED;
    }

    /* device is a stand alone device and opened */
    if ((dev->flag & RT_DEVICE_FLAG_STANDALONE) &&
        (dev->open_flag & RT_DEVICE_OFLAG_OPEN))
    {
        return DRV_ERROR_BUSY;
    }

    /* call device_open interface */
    if (device_open != NULL)
    {
        result = device_open(dev, oflag);
    }
    else
    {
        /* set open flag */
        dev->open_flag = (oflag & RT_DEVICE_OFLAG_MASK);
    }

    /* set open flag */
    if (result == DRV_SUCCESS || result == DRV_ERROR_INVALID_PARAM)
    {
        dev->open_flag |= RT_DEVICE_OFLAG_OPEN;

        dev->ref_count++;
        /* don't let bad things happen silently. If you are bitten by this assert,
         * please set the ref_count to a bigger type. */
        assert_param(dev->ref_count != 0);
    }

    return result;
}

/**
 * This function will close a device
 *
 * @param dev the pointer of device driver structure
 *
 * @return the result
 */
uint32_t rt_device_close(rt_device_t dev)
{
    uint32_t result = DRV_SUCCESS;

    assert_param(dev != NULL);
    assert_param(rt_object_get_type(&dev->parent) == RT_Object_Class_Device);

    if (dev->ref_count == 0)
        return DRV_ERROR_INVALID_PARAM;

    dev->ref_count--;

    if (dev->ref_count != 0)
        return DRV_SUCCESS;

    /* call device_close interface */
    if (device_close != NULL)
    {
        result = device_close(dev);
    }

    /* set open flag */
    if (result == DRV_SUCCESS || result == DRV_ERROR_INVALID_PARAM)
        dev->open_flag = RT_DEVICE_OFLAG_CLOSE;

    return result;
}

/**
 * This function will read some data from a device.
 *
 * @param dev the pointer of device driver structure
 * @param pos the position of reading
 * @param buffer the data buffer to save read data
 * @param size the size of buffer
 *
 * @return the actually read size on successful, otherwise negative returned.
 *
 * @note since 0.4.0, the unit of size/pos is a block for block device.
 */
uint32_t rt_device_read(rt_device_t dev,
                         uint32_t    pos,
                         void       *buffer,
                         uint32_t   size)
{
    assert_param(dev != NULL);
    assert_param(rt_object_get_type(&dev->parent) == RT_Object_Class_Device);

    if (dev->ref_count == 0)
    {
        return 0;
    }

    /* call device_read interface */
    if (device_read != NULL)
    {
        return device_read(dev, pos, buffer, size);
    }

    return 0;
}

/**
 * This function will write some data to a device.
 *
 * @param dev the pointer of device driver structure
 * @param pos the position of written
 * @param buffer the data buffer to be written to device
 * @param size the size of buffer
 *
 * @return the actually written size on successful, otherwise negative returned.
 *
 * @note since 0.4.0, the unit of size/pos is a block for block device.
 */
uint32_t rt_device_write(rt_device_t dev,
                          uint32_t    pos,
                          const void *buffer,
                          uint32_t   size)
{
    assert_param(dev != NULL);
    assert_param(rt_object_get_type(&dev->parent) == RT_Object_Class_Device);

    if (dev->ref_count == 0)
    {
        return 0;
    }

    /* call device_write interface */
    if (device_write != NULL)
    {
        return device_write(dev, pos, buffer, size);
    }
    return 0;
}

/**
 * This function will perform a variety of control functions on devices.
 *
 * @param dev the pointer of device driver structure
 * @param cmd the command sent to device
 * @param arg the argument of command
 *
 * @return the result
 */
uint32_t rt_device_control(rt_device_t dev, int cmd, void *arg)
{
    assert_param(dev != NULL);
    assert_param(rt_object_get_type(&dev->parent) == RT_Object_Class_Device);

    /* call device_write interface */
    if (device_control != NULL)
    {
        return device_control(dev, cmd, arg);
    }

    return DRV_ERROR_NOT_FOUND;
}

/**
 * This function will set the reception indication callback function. This callback function
 * is invoked when this device receives data.
 *
 * @param dev the pointer of device driver structure
 * @param rx_ind the indication callback function
 *
 * @return DRV_SUCCESS
 */
uint32_t rt_device_set_rx_indicate(rt_device_t dev,
                          uint32_t (*rx_ind)(rt_device_t dev, uint32_t size))
{
    assert_param(dev != NULL);
    assert_param(rt_object_get_type(&dev->parent) == RT_Object_Class_Device);

    dev->rx_indicate = rx_ind;

    return DRV_SUCCESS;
}

/**
 * This function will set the indication callback function when device has
 * written data to physical hardware.
 *
 * @param dev the pointer of device driver structure
 * @param tx_done the indication callback function
 *
 * @return DRV_SUCCESS
 */

uint32_t rt_device_set_tx_complete(rt_device_t dev,
                          uint32_t (*tx_done)(rt_device_t dev, void *buffer))
{
    assert_param(dev != NULL);
    assert_param(rt_object_get_type(&dev->parent) == RT_Object_Class_Device);

    dev->tx_complete = tx_done;

    return DRV_SUCCESS;
}

