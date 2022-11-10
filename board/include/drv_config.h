#ifndef  __DRV_CONFIG_H_
#define  __DRV_CONFIG_H_

#include <stm32l1xx.h>
#include "stm32l1xx_hal_conf.h"
#include <stdio.h>
#include <stdint.h>
#include <stdbool.h>
#include <string.h>


#define rt_hw_interrupt_disable()  __set_PRIMASK(1)
#define rt_hw_interrupt_enable()   __set_PRIMASK(0)


#define rt_enter_critical()  __set_PRIMASK(1)
#define rt_exit_critical()   __set_PRIMASK(0)

#define rt_malloc  pvPortMalloc
#define rt_free    vPortFree


#define  RT_NAME_MAX  8

#define BSP_USING_UART1
#define BSP_USING_I2C1

struct rt_list_node
{
    struct rt_list_node *next;                          /**< point to next node. */
    struct rt_list_node *prev;                          /**< point to prev node. */
};
typedef struct rt_list_node rt_list_t;                  /**< Type for lists. */

struct rt_slist_node
{
    struct rt_slist_node *next;                         /**< point to next node. */
};
typedef struct rt_slist_node rt_slist_t;                /**< Type for single list. */


enum rt_object_class_type
{
    RT_Object_Class_Null   = 0,                         /**< The object is not used. */
    RT_Object_Class_Thread,                             /**< The object is a thread. */
    RT_Object_Class_Semaphore,                          /**< The object is a semaphore. */
    RT_Object_Class_Mutex,                              /**< The object is a mutex. */
    RT_Object_Class_Event,                              /**< The object is a event. */
    RT_Object_Class_MailBox,                            /**< The object is a mail box. */
    RT_Object_Class_MessageQueue,                       /**< The object is a message queue. */
    RT_Object_Class_MemHeap,                            /**< The object is a memory heap */
    RT_Object_Class_MemPool,                            /**< The object is a memory pool. */
    RT_Object_Class_Device,                             /**< The object is a device */
    RT_Object_Class_Timer,                              /**< The object is a timer. */
    RT_Object_Class_Module,                             /**< The object is a module. */
    RT_Object_Class_Unknown,                            /**< The object is unknown. */
    RT_Object_Class_Static = 0x80                       /**< The object is a static object. */
};

/**
 * The information of the kernel object
 */
struct rt_object_information
{
    enum rt_object_class_type type;                     /**< object class type */
    rt_list_t                 object_list;              /**< object list */
    uint32_t                  object_size;              /**< object size */
};


enum rt_device_class_type
{
    RT_Device_Class_Char = 0,                           /**< character device */
    RT_Device_Class_Block,                              /**< block device */
    RT_Device_Class_NetIf,                              /**< net interface */
    RT_Device_Class_MTD,                                /**< memory device */
    RT_Device_Class_CAN,                                /**< CAN device */
    RT_Device_Class_RTC,                                /**< RTC device */
    RT_Device_Class_Sound,                              /**< Sound device */
    RT_Device_Class_Graphic,                            /**< Graphic device */
    RT_Device_Class_I2CBUS,                             /**< I2C bus device */
    RT_Device_Class_USBDevice,                          /**< USB slave device */
    RT_Device_Class_USBHost,                            /**< USB host bus */
    RT_Device_Class_SPIBUS,                             /**< SPI bus device */
    RT_Device_Class_SPIDevice,                          /**< SPI device */
    RT_Device_Class_SDIO,                               /**< SDIO bus device */
    RT_Device_Class_PM,                                 /**< PM pseudo device */
    RT_Device_Class_Pipe,                               /**< Pipe device */
    RT_Device_Class_Portal,                             /**< Portal device */
    RT_Device_Class_Timer,                              /**< Timer device */
    RT_Device_Class_Miscellaneous,                      /**< Miscellaneous device */
    RT_Device_Class_Sensor,                             /**< Sensor device */
    RT_Device_Class_Touch,                              /**< Touch device */
    RT_Device_Class_Unknown                             /**< unknown device */
};



struct rt_object
{
    char       name[8];                       /**< name of kernel object */
    uint8_t type;                                    /**< type of kernel object */
    uint8_t flag;                                    /**< flag of kernel object */
    rt_list_t  list;                                    /**< list node of kernel object */
};
typedef struct rt_object *rt_object_t;                  /**< Type for kernel objects. */


typedef struct rt_device *rt_device_t;

struct rt_device_ops
{
    /* common device interface */
    uint32_t  (*init)   (rt_device_t dev);
    uint32_t  (*open)   (rt_device_t dev, uint16_t oflag);
    uint32_t  (*close)  (rt_device_t dev);
    uint32_t  (*read)   (rt_device_t dev, uint32_t pos, void *buffer, uint32_t size);
    uint32_t  (*write)  (rt_device_t dev, uint32_t pos, const void *buffer, uint32_t size);
    uint32_t  (*control)(rt_device_t dev, int cmd, void *args);
};


struct rt_device
{
    struct rt_object          parent;                   /**< inherit from rt_object */

    enum rt_device_class_type type;                     /**< device type */
    uint16_t               flag;                     /**< device flag */
    uint16_t               open_flag;                /**< device open flag */

    uint8_t                ref_count;                /**< reference count */
    uint8_t                device_id;                /**< 0 - 255 */

    /* device call back */
    uint32_t (*rx_indicate)(rt_device_t dev, uint32_t size);
    uint32_t (*tx_complete)(rt_device_t dev, void *buffer);

    const struct rt_device_ops *ops;
    void                     *user_data;                /**< device private data */
};









#define RT_DEVICE_CTRL_CONFIG           0x03            /**< configure device */

#define RT_DEVICE_CTRL_SET_INT          0x10            /**< set interrupt */
#define RT_DEVICE_CTRL_CLR_INT          0x11            /**< clear interrupt */
#define RT_DEVICE_CTRL_GET_INT          0x12            /**< get interrupt status */

#define RT_DEVICE_FLAG_DEACTIVATE       0x000           /**< device is not not initialized */

#define RT_DEVICE_FLAG_RDONLY           0x001           /**< read only */
#define RT_DEVICE_FLAG_WRONLY           0x002           /**< write only */
#define RT_DEVICE_FLAG_RDWR             0x003           /**< read and write */

#define RT_DEVICE_FLAG_REMOVABLE        0x004           /**< removable device */
#define RT_DEVICE_FLAG_STANDALONE       0x008           /**< standalone device */
#define RT_DEVICE_FLAG_ACTIVATED        0x010           /**< device is activated */
#define RT_DEVICE_FLAG_SUSPENDED        0x020           /**< device is suspended */
#define RT_DEVICE_FLAG_STREAM           0x040           /**< stream mode */

#define RT_DEVICE_FLAG_INT_RX           0x100           /**< INT mode on Rx */
#define RT_DEVICE_FLAG_DMA_RX           0x200           /**< DMA mode on Rx */
#define RT_DEVICE_FLAG_INT_TX           0x400           /**< INT mode on Tx */
#define RT_DEVICE_FLAG_DMA_TX           0x800           /**< DMA mode on Tx */

#define RT_DEVICE_OFLAG_CLOSE           0x000           /**< device is closed */
#define RT_DEVICE_OFLAG_RDONLY          0x001           /**< read only access */
#define RT_DEVICE_OFLAG_WRONLY          0x002           /**< write only access */
#define RT_DEVICE_OFLAG_RDWR            0x003           /**< read and write */
#define RT_DEVICE_OFLAG_OPEN            0x008           /**< device is opened */
#define RT_DEVICE_OFLAG_MASK            0xf0f           /**< mask of open flag */


/** @defgroup DRV_ERRORS_BASE Error Codes Base number definitions
 * @{ */
#define DRV_ERROR_BASE_NUM      (0x0)       ///< Global error base
#define DRV_ERROR_SDM_BASE_NUM  (0x1000)    ///< SDM error base
#define DRV_ERROR_SOC_BASE_NUM  (0x2000)    ///< SoC error base
#define DRV_ERROR_STK_BASE_NUM  (0x3000)    ///< STK error base
/** @} */

#define DRV_SUCCESS                           (DRV_ERROR_BASE_NUM + 0)  ///< Successful command
#define DRV_ERROR_SVC_HANDLER_MISSING         (DRV_ERROR_BASE_NUM + 1)  ///< SVC handler is missing
#define DRV_ERROR_SOFTDEVICE_NOT_ENABLED      (DRV_ERROR_BASE_NUM + 2)  ///< SoftDevice has not been enabled
#define DRV_ERROR_INTERNAL                    (DRV_ERROR_BASE_NUM + 3)  ///< Internal Error
#define DRV_ERROR_NO_MEM                      (DRV_ERROR_BASE_NUM + 4)  ///< No Memory for operation
#define DRV_ERROR_NOT_FOUND                   (DRV_ERROR_BASE_NUM + 5)  ///< Not found
#define DRV_ERROR_NOT_SUPPORTED               (DRV_ERROR_BASE_NUM + 6)  ///< Not supported
#define DRV_ERROR_INVALID_PARAM               (DRV_ERROR_BASE_NUM + 7)  ///< Invalid Parameter
#define DRV_ERROR_INVALID_STATE               (DRV_ERROR_BASE_NUM + 8)  ///< Invalid state, operation disallowed in this state
#define DRV_ERROR_INVALID_LENGTH              (DRV_ERROR_BASE_NUM + 9)  ///< Invalid Length
#define DRV_ERROR_INVALID_FLAGS               (DRV_ERROR_BASE_NUM + 10) ///< Invalid Flags
#define DRV_ERROR_INVALID_DATA                (DRV_ERROR_BASE_NUM + 11) ///< Invalid Data
#define DRV_ERROR_DATA_SIZE                   (DRV_ERROR_BASE_NUM + 12) ///< Data size exceeds limit
#define DRV_ERROR_TIMEOUT                     (DRV_ERROR_BASE_NUM + 13) ///< Operation timed out
#define DRV_ERROR_NULL                        (DRV_ERROR_BASE_NUM + 14) ///< Null Pointer
#define DRV_ERROR_FORBIDDEN                   (DRV_ERROR_BASE_NUM + 15) ///< Forbidden Operation
#define DRV_ERROR_INVALID_ADDR                (DRV_ERROR_BASE_NUM + 16) ///< Bad Memory Address
#define DRV_ERROR_BUSY                        (DRV_ERROR_BASE_NUM + 17) ///< Busy




void rt_system_object_init(void);
struct rt_object_information *
rt_object_get_information(enum rt_object_class_type type);
void rt_object_init(struct rt_object         *object,
                    enum rt_object_class_type type,
                    const char               *name);
void rt_object_detach(rt_object_t object);
rt_object_t rt_object_allocate(enum rt_object_class_type type,
                               const char               *name);
void rt_object_delete(rt_object_t object);
bool rt_object_is_systemobject(rt_object_t object);
uint8_t rt_object_get_type(rt_object_t object);
rt_object_t rt_object_find(const char *name, uint8_t type);

rt_device_t rt_device_find(const char *name);

uint32_t rt_device_register(rt_device_t dev,
                            const char *name,
                            uint16_t flags);
uint32_t rt_device_unregister(rt_device_t dev);

rt_device_t rt_device_create(int type, int attach_size);
void rt_device_destroy(rt_device_t device);

uint32_t rt_device_init_all(void);

uint32_t
rt_device_set_rx_indicate(rt_device_t dev,
                          uint32_t (*rx_ind)(rt_device_t dev, uint32_t size));
uint32_t
rt_device_set_tx_complete(rt_device_t dev,
                          uint32_t (*tx_done)(rt_device_t dev, void *buffer));

uint32_t  rt_device_init (rt_device_t dev);
uint32_t  rt_device_open (rt_device_t dev, uint16_t oflag);
uint32_t  rt_device_close(rt_device_t dev);
uint32_t rt_device_read (rt_device_t dev,
                          uint32_t    pos,
                          void       *buffer,
                          uint32_t   size);
uint32_t rt_device_write(rt_device_t dev,
                          uint32_t    pos,
                          const void *buffer,
                          uint32_t   size);
uint32_t  rt_device_control(rt_device_t dev, int cmd, void *arg);



#endif

