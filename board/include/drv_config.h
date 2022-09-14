#ifndef  __DRV_CONFIG_H_
#define  __DRV_CONFIG_H_

#include <stm32l1xx.h>
#include <stdio.h>
#include <stdint.h>
#include <stdbool.h>

#define BSP_USING_UART1
















#define rt_container_of(ptr, type, member) \
    ((type *)((char *)(ptr) - (unsigned long)(&((type *)0)->member)))


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









#endif

