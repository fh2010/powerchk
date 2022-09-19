#include "dev_console.h"

static rt_device_t _console_device = NULL;

rt_device_t dev_console_set_device(void)
{
    rt_device_t new_device, old_device;

    /* save old device */
    old_device = _console_device;

    /* find new console device */
    new_device = rt_device_find("uart1");
    if (new_device != NULL)
    {
        if (_console_device != NULL)
        {
            /* close old console device */
            rt_device_close(_console_device);
        }

        /* set new console device */
        rt_device_open(new_device, RT_DEVICE_OFLAG_RDWR | RT_DEVICE_FLAG_STREAM);
        _console_device = new_device;
    }
    return old_device;
}