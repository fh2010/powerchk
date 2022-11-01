#include "drv_uart.h"
#include "FreeRTOS.h"

#ifdef BSP_SERIAL_USING_DMA
    static void uart_dma_config(struct rt_serial_device *serial, rt_ubase_t flag);
#endif

enum
{
#ifdef BSP_USING_UART1
    UART1_INDEX,
#endif
#ifdef BSP_USING_UART2
    UART2_INDEX,
#endif
#ifdef BSP_USING_UART3
    UART3_INDEX,
#endif
};

static struct drv_uart_config uart_config[] =
{
#ifdef BSP_USING_UART1
    UART1_CONFIG,
#endif
#ifdef BSP_USING_UART2
    UART2_CONFIG,
#endif
#ifdef BSP_USING_UART3
    UART3_CONFIG,
#endif
};
static struct drv_uart uart_obj[sizeof(uart_config) / sizeof(uart_config[0])] = {0};

static uint32_t drv_uart_configure(struct rt_serial_device *serial, struct serial_configure *cfg)
{
    struct drv_uart *uart;
    assert_param(serial != NULL);
    assert_param(cfg != NULL);

    uart = rt_container_of(serial, struct drv_uart, serial);

    uart->handle.Instance          = uart->config->Instance;
    uart->handle.Init.BaudRate     = cfg->baud_rate;
    uart->handle.Init.HwFlowCtl    = UART_HWCONTROL_NONE;
    uart->handle.Init.Mode         = UART_MODE_TX_RX;
    uart->handle.Init.OverSampling = UART_OVERSAMPLING_16;
    switch (cfg->data_bits)
    {
    case DATA_BITS_8:
        uart->handle.Init.WordLength = UART_WORDLENGTH_8B;
        break;
    case DATA_BITS_9:
        uart->handle.Init.WordLength = UART_WORDLENGTH_9B;
        break;
    default:
        uart->handle.Init.WordLength = UART_WORDLENGTH_8B;
        break;
    }
    switch (cfg->stop_bits)
    {
    case STOP_BITS_1:
        uart->handle.Init.StopBits   = UART_STOPBITS_1;
        break;
    case STOP_BITS_2:
        uart->handle.Init.StopBits   = UART_STOPBITS_2;
        break;
    default:
        uart->handle.Init.StopBits   = UART_STOPBITS_1;
        break;
    }
    switch (cfg->parity)
    {
    case PARITY_NONE:
        uart->handle.Init.Parity     = UART_PARITY_NONE;
        break;
    case PARITY_ODD:
        uart->handle.Init.Parity     = UART_PARITY_ODD;
        break;
    case PARITY_EVEN:
        uart->handle.Init.Parity     = UART_PARITY_EVEN;
        break;
    default:
        uart->handle.Init.Parity     = UART_PARITY_NONE;
        break;
    }

    if (HAL_UART_Init(&uart->handle) != HAL_OK)
    {
        return DRV_ERROR_INTERNAL;
    }

    return DRV_SUCCESS;
}

static uint32_t drv_uart_control(struct rt_serial_device *serial, int cmd, void *arg)
{
    struct drv_uart *uart;
#ifdef BSP_SERIAL_USING_DMA
    rt_ubase_t ctrl_arg = (rt_ubase_t)arg;
#endif

    assert_param(serial != NULL);
    uart = rt_container_of(serial, struct drv_uart, serial);

    switch (cmd)
    {
    /* disable interrupt */
    case RT_DEVICE_CTRL_CLR_INT:
        /* disable rx irq */
        NVIC_DisableIRQ(uart->config->irq_type);
        /* disable interrupt */
        __HAL_UART_DISABLE_IT(&(uart->handle), UART_IT_RXNE);
        break;
    /* enable interrupt */
    case RT_DEVICE_CTRL_SET_INT:
        /* enable rx irq */
        NVIC_EnableIRQ(uart->config->irq_type);
        /* enable interrupt */
        __HAL_UART_ENABLE_IT(&(uart->handle), UART_IT_RXNE);
        break;

#ifdef BSP_SERIAL_USING_DMA
    case RT_DEVICE_CTRL_CONFIG:
        stm32_dma_config(serial, ctrl_arg);
        break;
#endif
    }
    return DRV_SUCCESS;
}

static int drv_uart_putc(struct rt_serial_device *serial, char c)
{
    struct drv_uart *uart;
    assert_param(serial != NULL);

    uart = rt_container_of(serial, struct drv_uart, serial);
    __HAL_UART_CLEAR_FLAG(&(uart->handle), UART_FLAG_TC);
    uart->handle.Instance->DR = c;
    while (__HAL_UART_GET_FLAG(&(uart->handle), UART_FLAG_TC) == RESET);
    return 1;
}

static int drv_uart_getc(struct rt_serial_device *serial)
{
    int ch;
    struct drv_uart *uart;
    assert_param(serial != NULL);
    uart = rt_container_of(serial, struct drv_uart, serial);

    ch = -1;
    if (__HAL_UART_GET_FLAG(&(uart->handle), UART_FLAG_RXNE) != RESET)
    {
      ch = uart->handle.Instance->DR & 0xff;
    }
    return ch;
}

static uint32_t drv_uart_dma_transmit(struct rt_serial_device *serial, uint8_t *buf, uint32_t size, int direction)
{
    struct drv_uart *uart;
    assert_param(serial != NULL);
    uart = rt_container_of(serial, struct drv_uart, serial);
    
    if (size == 0)
    {
        return 0;
    }
    
    if (DRV_SERIAL_DMA_TX == direction)
    {
        if (HAL_UART_Transmit_DMA(&uart->handle, buf, size) == HAL_OK)
        {
            return size;
        }
        else
        {
            return 0;
        }
    }
    return 0;
}

static const struct rt_uart_ops drv_uart_ops =
{
    .configure = drv_uart_configure,
    .control = drv_uart_control,
    .putc = drv_uart_putc,
    .getc = drv_uart_getc,
    .dma_transmit = drv_uart_dma_transmit
};

/**
 * Uart common interrupt process. This need add to uart ISR.
 *
 * @param serial serial device
 */
static void uart_isr(struct rt_serial_device *serial)
{
    struct drv_uart *uart;
#ifdef BSP_SERIAL_USING_DMA
    rt_size_t recv_total_index, recv_len;
    rt_base_t level;
#endif

    assert_param(serial != NULL);
    uart = rt_container_of(serial, struct drv_uart, serial);

    /* UART in mode Receiver -------------------------------------------------*/
    if ((__HAL_UART_GET_FLAG(&(uart->handle), UART_FLAG_RXNE) != RESET) &&
            (__HAL_UART_GET_IT_SOURCE(&(uart->handle), UART_IT_RXNE) != RESET))
    {
        rt_hw_serial_isr(serial, DRV_SERIAL_EVENT_RX_IND);
    }
#ifdef BSP_SERIAL_USING_DMA
    else if ((uart->uart_dma_flag) && (__HAL_UART_GET_FLAG(&(uart->handle), UART_FLAG_IDLE) != RESET)
             && (__HAL_UART_GET_IT_SOURCE(&(uart->handle), UART_IT_IDLE) != RESET))
    {
        level = rt_hw_interrupt_disable();
        recv_total_index = serial->config.bufsz - __HAL_DMA_GET_COUNTER(&(uart->dma_rx.handle));
        recv_len = recv_total_index - uart->dma_rx.last_index;
        uart->dma_rx.last_index = recv_total_index;
        rt_hw_interrupt_enable(level);

        if (recv_len)
        {
            rt_hw_serial_isr(serial, RT_SERIAL_EVENT_RX_DMADONE | (recv_len << 8));
        }
        __HAL_UART_CLEAR_IDLEFLAG(&uart->handle);
    }
    else if (__HAL_UART_GET_FLAG(&(uart->handle), UART_FLAG_TC) != RESET)
    {
        if ((serial->parent.open_flag & RT_DEVICE_FLAG_DMA_TX) != 0)
        {
            HAL_UART_IRQHandler(&(uart->handle));
        }
        else
        {
            UART_INSTANCE_CLEAR_FUNCTION(&(uart->handle), UART_FLAG_TC);
        }
    }
#endif
    else
    {
        if (__HAL_UART_GET_FLAG(&(uart->handle), UART_FLAG_ORE) != RESET)
        {
            __HAL_UART_CLEAR_OREFLAG(&uart->handle);
        }
        if (__HAL_UART_GET_FLAG(&(uart->handle), UART_FLAG_NE) != RESET)
        {
            __HAL_UART_CLEAR_NEFLAG(&uart->handle);
        }
        if (__HAL_UART_GET_FLAG(&(uart->handle), UART_FLAG_FE) != RESET)
        {
            __HAL_UART_CLEAR_FEFLAG(&uart->handle);
        }
        if (__HAL_UART_GET_FLAG(&(uart->handle), UART_FLAG_PE) != RESET)
        {
            __HAL_UART_CLEAR_PEFLAG(&uart->handle);
        }
		
        if (__HAL_UART_GET_FLAG(&(uart->handle), UART_FLAG_LBD) != RESET)
        {
            __HAL_UART_CLEAR_FLAG(&(uart->handle), UART_FLAG_LBD);
        }

        if (__HAL_UART_GET_FLAG(&(uart->handle), UART_FLAG_CTS) != RESET)
        {
            __HAL_UART_CLEAR_FLAG(&(uart->handle), UART_FLAG_CTS);
        }
        if (__HAL_UART_GET_FLAG(&(uart->handle), UART_FLAG_TXE) != RESET)
        {
            __HAL_UART_CLEAR_FLAG(&(uart->handle), UART_FLAG_TXE);
        }
        if (__HAL_UART_GET_FLAG(&(uart->handle), UART_FLAG_TC) != RESET)
        {
            __HAL_UART_CLEAR_FLAG(&(uart->handle), UART_FLAG_TC);
        }
        if (__HAL_UART_GET_FLAG(&(uart->handle), UART_FLAG_RXNE) != RESET)
        {
            __HAL_UART_CLEAR_FLAG(&(uart->handle), UART_FLAG_RXNE);
        }
    }
}

#ifdef BSP_SERIAL_USING_DMA
static void dma_isr(struct rt_serial_device *serial)
{
    struct stm32_uart *uart;
    rt_size_t recv_total_index, recv_len;
    rt_base_t level;

    assert_param(serial != NULL);
    uart = rt_container_of(serial, struct stm32_uart, serial);

    if ((__HAL_DMA_GET_IT_SOURCE(&(uart->dma_rx.handle), DMA_IT_TC) != RESET) ||
            (__HAL_DMA_GET_IT_SOURCE(&(uart->dma_rx.handle), DMA_IT_HT) != RESET))
    {
        level = rt_hw_interrupt_disable();
        recv_total_index = serial->config.bufsz - __HAL_DMA_GET_COUNTER(&(uart->dma_rx.handle));
        if (recv_total_index == 0)
        {
            recv_len = serial->config.bufsz - uart->dma_rx.last_index;
        }
        else
        {
            recv_len = recv_total_index - uart->dma_rx.last_index;
        }
        uart->dma_rx.last_index = recv_total_index;
        rt_hw_interrupt_enable(level);

        if (recv_len)
        {
            rt_hw_serial_isr(serial, RT_SERIAL_EVENT_RX_DMADONE | (recv_len << 8));
        }
    }
}
#endif

#if defined(BSP_USING_UART1)
void USART1_IRQHandler(void)
{
    /* enter interrupt */
    //rt_interrupt_enter();

    uart_isr(&(uart_obj[UART1_INDEX].serial));

    /* leave interrupt */
    //rt_interrupt_leave();
}
#if defined(RT_SERIAL_USING_DMA) && defined(BSP_UART1_RX_USING_DMA)
void UART1_DMA_RX_IRQHandler(void)
{
    /* enter interrupt */
    rt_interrupt_enter();

    HAL_DMA_IRQHandler(&uart_obj[UART1_INDEX].dma_rx.handle);

    /* leave interrupt */
    rt_interrupt_leave();
}
#endif /* defined(RT_SERIAL_USING_DMA) && defined(BSP_UART1_RX_USING_DMA) */
#if defined(RT_SERIAL_USING_DMA) && defined(BSP_UART1_TX_USING_DMA)
void UART1_DMA_TX_IRQHandler(void)
{
    /* enter interrupt */
    rt_interrupt_enter();

    HAL_DMA_IRQHandler(&uart_obj[UART1_INDEX].dma_tx.handle);

    /* leave interrupt */
    rt_interrupt_leave();
}
#endif /* defined(RT_SERIAL_USING_DMA) && defined(BSP_UART1_TX_USING_DMA) */
#endif /* BSP_USING_UART1 */

#if defined(BSP_USING_UART2)
void USART2_IRQHandler(void)
{
    /* enter interrupt */
    rt_interrupt_enter();

    uart_isr(&(uart_obj[UART2_INDEX].serial));

    /* leave interrupt */
    rt_interrupt_leave();
}
#if defined(RT_SERIAL_USING_DMA) && defined(BSP_UART2_RX_USING_DMA)
void UART2_DMA_RX_IRQHandler(void)
{
    /* enter interrupt */
    rt_interrupt_enter();

    HAL_DMA_IRQHandler(&uart_obj[UART2_INDEX].dma_rx.handle);

    /* leave interrupt */
    rt_interrupt_leave();
}
#endif /* defined(RT_SERIAL_USING_DMA) && defined(BSP_UART2_RX_USING_DMA) */
#if defined(RT_SERIAL_USING_DMA) && defined(BSP_UART2_TX_USING_DMA)
void UART2_DMA_TX_IRQHandler(void)
{
    /* enter interrupt */
    rt_interrupt_enter();

    HAL_DMA_IRQHandler(&uart_obj[UART2_INDEX].dma_tx.handle);

    /* leave interrupt */
    rt_interrupt_leave();
}
#endif /* defined(RT_SERIAL_USING_DMA) && defined(BSP_UART2_TX_USING_DMA) */
#endif /* BSP_USING_UART2 */

#if defined(BSP_USING_UART3)
void USART3_IRQHandler(void)
{
    /* enter interrupt */
    rt_interrupt_enter();

    uart_isr(&(uart_obj[UART3_INDEX].serial));

    /* leave interrupt */
    rt_interrupt_leave();
}
#if defined(RT_SERIAL_USING_DMA) && defined(BSP_UART3_RX_USING_DMA)
void UART3_DMA_RX_IRQHandler(void)
{
    /* enter interrupt */
    rt_interrupt_enter();

    HAL_DMA_IRQHandler(&uart_obj[UART3_INDEX].dma_rx.handle);

    /* leave interrupt */
    rt_interrupt_leave();
}
#endif /* defined(BSP_UART_USING_DMA_RX) && defined(BSP_UART3_RX_USING_DMA) */
#if defined(RT_SERIAL_USING_DMA) && defined(BSP_UART3_TX_USING_DMA)
void UART3_DMA_TX_IRQHandler(void)
{
    /* enter interrupt */
    rt_interrupt_enter();

    HAL_DMA_IRQHandler(&uart_obj[UART3_INDEX].dma_tx.handle);

    /* leave interrupt */
    rt_interrupt_leave();
}
#endif /* defined(BSP_UART_USING_DMA_TX) && defined(BSP_UART3_TX_USING_DMA) */
#endif /* BSP_USING_UART3*/


#ifdef BSP_SERIAL_USING_DMA
static void drv_uart_dma_config(struct rt_serial_device *serial, rt_ubase_t flag)
{
    struct rt_serial_rx_fifo *rx_fifo;
    DMA_HandleTypeDef *DMA_Handle;
    struct dma_config *dma_config;
    struct stm32_uart *uart;
    
    assert_param(serial != NULL);
    uart = rt_container_of(serial, struct stm32_uart, serial);

    if (RT_DEVICE_FLAG_DMA_RX == flag)
    {
        DMA_Handle = &uart->dma_rx.handle;
        dma_config = uart->config->dma_rx;
    }
    else if (RT_DEVICE_FLAG_DMA_TX == flag)
    {
        DMA_Handle = &uart->dma_tx.handle;
        dma_config = uart->config->dma_tx;
    }
    LOG_D("%s dma config start", uart->config->name);

    {
        rt_uint32_t tmpreg = 0x00U;
#if defined(SOC_SERIES_STM32F1) || defined(SOC_SERIES_STM32F0) || defined(SOC_SERIES_STM32G0) \
    || defined(SOC_SERIES_STM32L0)
        /* enable DMA clock && Delay after an RCC peripheral clock enabling*/
        SET_BIT(RCC->AHBENR, dma_config->dma_rcc);
        tmpreg = READ_BIT(RCC->AHBENR, dma_config->dma_rcc);
#elif defined(SOC_SERIES_STM32F4) || defined(SOC_SERIES_STM32F7) || defined(SOC_SERIES_STM32L4) \
    || defined(SOC_SERIES_STM32G4)
        /* enable DMA clock && Delay after an RCC peripheral clock enabling*/
        SET_BIT(RCC->AHB1ENR, dma_config->dma_rcc);
        tmpreg = READ_BIT(RCC->AHB1ENR, dma_config->dma_rcc);

#if (defined(SOC_SERIES_STM32L4) || defined(SOC_SERIES_STM32G4)) && defined(DMAMUX1)
        /* enable DMAMUX clock for L4+ and G4 */
        __HAL_RCC_DMAMUX1_CLK_ENABLE();
#endif

#endif
        UNUSED(tmpreg);   /* To avoid compiler warnings */
    }

    if (RT_DEVICE_FLAG_DMA_RX == flag)
    {
        __HAL_LINKDMA(&(uart->handle), hdmarx, uart->dma_rx.handle);
    }
    else if (RT_DEVICE_FLAG_DMA_TX == flag)
    {
        __HAL_LINKDMA(&(uart->handle), hdmatx, uart->dma_tx.handle);
    }

#if defined(SOC_SERIES_STM32F1) || defined(SOC_SERIES_STM32F0) || defined(SOC_SERIES_STM32L0)
    DMA_Handle->Instance                 = dma_config->Instance;
#elif defined(SOC_SERIES_STM32F4) || defined(SOC_SERIES_STM32F7)
    DMA_Handle->Instance                 = dma_config->Instance;
    DMA_Handle->Init.Channel             = dma_config->channel;
#elif defined(SOC_SERIES_STM32L4) || defined(SOC_SERIES_STM32G0) || defined(SOC_SERIES_STM32G4)
    DMA_Handle->Instance                 = dma_config->Instance;
    DMA_Handle->Init.Request             = dma_config->request;
#endif
    DMA_Handle->Init.PeriphInc           = DMA_PINC_DISABLE;
    DMA_Handle->Init.MemInc              = DMA_MINC_ENABLE;
    DMA_Handle->Init.PeriphDataAlignment = DMA_PDATAALIGN_BYTE;
    DMA_Handle->Init.MemDataAlignment    = DMA_MDATAALIGN_BYTE;
    
    if (RT_DEVICE_FLAG_DMA_RX == flag)
    {
        DMA_Handle->Init.Direction           = DMA_PERIPH_TO_MEMORY;
        DMA_Handle->Init.Mode                = DMA_CIRCULAR;
    }
    else if (RT_DEVICE_FLAG_DMA_TX == flag)
    {
        DMA_Handle->Init.Direction           = DMA_MEMORY_TO_PERIPH;
        DMA_Handle->Init.Mode                = DMA_NORMAL;
    }
    
    DMA_Handle->Init.Priority            = DMA_PRIORITY_MEDIUM;
#if defined(SOC_SERIES_STM32F4) || defined(SOC_SERIES_STM32F7)
    DMA_Handle->Init.FIFOMode            = DMA_FIFOMODE_DISABLE;
#endif
    if (HAL_DMA_DeInit(DMA_Handle) != HAL_OK)
    {
        assert_param(0);
    }

    if (HAL_DMA_Init(DMA_Handle) != HAL_OK)
    {
        assert_param(0);
    }

    /* enable interrupt */
    if (flag == RT_DEVICE_FLAG_DMA_RX)
    {
        rx_fifo = (struct rt_serial_rx_fifo *)serial->serial_rx;
        /* Start DMA transfer */
        if (HAL_UART_Receive_DMA(&(uart->handle), rx_fifo->buffer, serial->config.bufsz) != HAL_OK)
        {
            /* Transfer error in reception process */
            assert_param(0);
        }
        CLEAR_BIT(uart->handle.Instance->CR3, USART_CR3_EIE);
        __HAL_UART_ENABLE_IT(&(uart->handle), UART_IT_IDLE);
    }
 
    /* enable irq */
    HAL_NVIC_SetPriority(dma_config->dma_irq, 0, 0);
    HAL_NVIC_EnableIRQ(dma_config->dma_irq);

    HAL_NVIC_SetPriority(uart->config->irq_type, 1, 0);
    HAL_NVIC_EnableIRQ(uart->config->irq_type);

    LOG_D("%s dma %s instance: %x", uart->config->name, flag == RT_DEVICE_FLAG_DMA_RX ? "RX" : "TX", DMA_Handle->Instance);
    LOG_D("%s dma config done", uart->config->name);
}

/**
  * @brief  UART error callbacks
  * @param  huart: UART handle
  * @note   This example shows a simple way to report transfer error, and you can
  *         add your own implementation.
  * @retval None
  */
void HAL_UART_ErrorCallback(UART_HandleTypeDef *huart)
{
    assert_param(huart != NULL);
    struct stm32_uart *uart = (struct stm32_uart *)huart;
    LOG_D("%s: %s %d\n", __FUNCTION__, uart->config->name, huart->ErrorCode);
    UNUSED(uart);
}

/**
  * @brief  Rx Transfer completed callback
  * @param  huart: UART handle
  * @note   This example shows a simple way to report end of DMA Rx transfer, and
  *         you can add your own implementation.
  * @retval None
  */
void HAL_UART_RxCpltCallback(UART_HandleTypeDef *huart)
{
    struct stm32_uart *uart;
    assert_param(huart != NULL);
    uart = (struct stm32_uart *)huart;
    dma_isr(&uart->serial);
}

/**
  * @brief  Rx Half transfer completed callback
  * @param  huart: UART handle
  * @note   This example shows a simple way to report end of DMA Rx Half transfer, 
  *         and you can add your own implementation.
  * @retval None
  */
void HAL_UART_RxHalfCpltCallback(UART_HandleTypeDef *huart)
{
    struct stm32_uart *uart;
    assert_param(huart != NULL);
    uart = (struct stm32_uart *)huart;
    dma_isr(&uart->serial);
}
void HAL_UART_TxCpltCallback(UART_HandleTypeDef *huart)
{
    struct stm32_uart *uart;
    assert_param(huart != NULL);
    uart = (struct stm32_uart *)huart;
    rt_hw_serial_isr(&uart->serial, RT_SERIAL_EVENT_TX_DMADONE);
}
#endif  /* RT_SERIAL_USING_DMA */

static void drv_uart_get_dma_config(void)
{
#ifdef BSP_USING_UART1
    uart_obj[UART1_INDEX].uart_dma_flag = 0;
#ifdef BSP_UART1_RX_USING_DMA
    uart_obj[UART1_INDEX].uart_dma_flag |= RT_DEVICE_FLAG_DMA_RX;
    static struct dma_config uart1_dma_rx = UART1_DMA_RX_CONFIG;
    uart_config[UART1_INDEX].dma_rx = &uart1_dma_rx;
#endif
#ifdef BSP_UART1_TX_USING_DMA
    uart_obj[UART1_INDEX].uart_dma_flag |= RT_DEVICE_FLAG_DMA_TX;
    static struct dma_config uart1_dma_tx = UART1_DMA_TX_CONFIG;
    uart_config[UART1_INDEX].dma_tx = &uart1_dma_tx;
#endif
#endif

#ifdef BSP_USING_UART2
    uart_obj[UART2_INDEX].uart_dma_flag = 0;
#ifdef BSP_UART2_RX_USING_DMA
    uart_obj[UART2_INDEX].uart_dma_flag |= RT_DEVICE_FLAG_DMA_RX;
    static struct dma_config uart2_dma_rx = UART2_DMA_RX_CONFIG;
    uart_config[UART2_INDEX].dma_rx = &uart2_dma_rx;
#endif
#ifdef BSP_UART2_TX_USING_DMA
    uart_obj[UART2_INDEX].uart_dma_flag |= RT_DEVICE_FLAG_DMA_TX;
    static struct dma_config uart2_dma_tx = UART2_DMA_TX_CONFIG;
    uart_config[UART2_INDEX].dma_tx = &uart2_dma_tx;
#endif
#endif

#ifdef BSP_USING_UART3
    uart_obj[UART3_INDEX].uart_dma_flag = 0;
#ifdef BSP_UART3_RX_USING_DMA
    uart_obj[UART3_INDEX].uart_dma_flag |= RT_DEVICE_FLAG_DMA_RX;
    static struct dma_config uart3_dma_rx = UART3_DMA_RX_CONFIG;
    uart_config[UART3_INDEX].dma_rx = &uart3_dma_rx;
#endif
#ifdef BSP_UART3_TX_USING_DMA
    uart_obj[UART3_INDEX].uart_dma_flag |= RT_DEVICE_FLAG_DMA_TX;
    static struct dma_config uart3_dma_tx = UART3_DMA_TX_CONFIG;
    uart_config[UART3_INDEX].dma_tx = &uart3_dma_tx;
#endif
#endif

}


static __inline int _serial_poll_rx(struct rt_serial_device *serial, uint8_t *data, int length)
{
    int ch;
    int size;

    assert_param(serial != NULL);
    size = length;

    while (length)
    {
        ch = serial->ops->getc(serial);
        if (ch == -1) break;

        *data = ch;
        data ++; length --;

        if (ch == '\n') break;
    }

    return size - length;
}

static __inline int _serial_poll_tx(struct rt_serial_device *serial, const uint8_t *data, int length)
{
    int size;
    assert_param(serial != NULL);

    size = length;
    while (length)
    {
        /*
         * to be polite with serial console add a line feed
         * to the carriage return character
         */
        if (*data == '\n' && (serial->parent.open_flag & RT_DEVICE_FLAG_STREAM))
        {
            serial->ops->putc(serial, '\r');
        }

        serial->ops->putc(serial, *data);

        ++ data;
        -- length;
    }

    return size - length;
}

/*
 * Serial interrupt routines
 */
static __inline int _serial_int_rx(struct rt_serial_device *serial, uint8_t *data, int length)
{
    int size;
    struct rt_serial_rx_fifo* rx_fifo;

    assert_param(serial != NULL);
    size = length;

    rx_fifo = (struct rt_serial_rx_fifo*) serial->serial_rx;
    assert_param(rx_fifo != NULL);

    /* read from software FIFO */
    while (length)
    {
        int ch;

        /* disable interrupt */
        rt_hw_interrupt_disable();

        /* there's no data: */
        if ((rx_fifo->get_index == rx_fifo->put_index) && (rx_fifo->is_full == false))
        {
            /* no data, enable interrupt and break out */
            rt_hw_interrupt_enable();
            break;
        }

        /* otherwise there's the data: */
        ch = rx_fifo->buffer[rx_fifo->get_index];
        rx_fifo->get_index += 1;
        if (rx_fifo->get_index >= serial->config.bufsz) rx_fifo->get_index = 0;

        if (rx_fifo->is_full)
        {
            rx_fifo->is_full = false;
        }

        /* enable interrupt */
        rt_hw_interrupt_enable();

        *data = ch & 0xff;
        data ++; length --;
    }

    return size - length;
}

static __inline int _serial_int_tx(struct rt_serial_device *serial, const uint8_t *data, int length)
{
    int size;
    struct rt_serial_tx_fifo *tx;

    assert_param(serial != NULL);

    size = length;
    tx = (struct rt_serial_tx_fifo*) serial->serial_tx;
    assert_param(tx != NULL);

    while (length)
    {
        if (serial->ops->putc(serial, *(char*)data) == -1)
        {
            //rt_completion_wait(&(tx->completion), RT_WAITING_FOREVER);
            continue;
        }

        data ++; length --;
    }

    return size - length;
}

static void _serial_check_buffer_size(void)
{
    static bool already_output = false;
		
    if (already_output == false)
    {
        already_output = true;
    }
}	

#if defined(RT_USING_POSIX) || defined(RT_SERIAL_USING_DMA)
static rt_size_t _serial_fifo_calc_recved_len(struct rt_serial_device *serial)
{
    struct rt_serial_rx_fifo *rx_fifo = (struct rt_serial_rx_fifo *) serial->serial_rx;

    assert_param(rx_fifo != NULL);

    if (rx_fifo->put_index == rx_fifo->get_index)
    {
        return (rx_fifo->is_full == false ? 0 : serial->config.bufsz);
    }
    else
    {
        if (rx_fifo->put_index > rx_fifo->get_index)
        {
            return rx_fifo->put_index - rx_fifo->get_index;
        }
        else
        {
            return serial->config.bufsz - (rx_fifo->get_index - rx_fifo->put_index);
        }
    }
}
#endif /* RT_USING_POSIX || RT_SERIAL_USING_DMA */

#ifdef RT_SERIAL_USING_DMA
/**
 * Calculate DMA received data length.
 *
 * @param serial serial device
 *
 * @return length
 */
static rt_size_t rt_dma_calc_recved_len(struct rt_serial_device *serial)
{
    return _serial_fifo_calc_recved_len(serial);
}

/**
 * Read data finish by DMA mode then update the get index for receive fifo.
 *
 * @param serial serial device
 * @param len get data length for this operate
 */
static void rt_dma_recv_update_get_index(struct rt_serial_device *serial, rt_size_t len)
{
    struct rt_serial_rx_fifo *rx_fifo = (struct rt_serial_rx_fifo *) serial->serial_rx;

    assert_param(rx_fifo != NULL);
    assert_param(len <= rt_dma_calc_recved_len(serial));

    if (rx_fifo->is_full && len != 0) rx_fifo->is_full = false;

    rx_fifo->get_index += len;
    if (rx_fifo->get_index >= serial->config.bufsz)
    {
        rx_fifo->get_index %= serial->config.bufsz;
    }
}

/**
 * DMA received finish then update put index for receive fifo.
 *
 * @param serial serial device
 * @param len received length for this transmit
 */
static void rt_dma_recv_update_put_index(struct rt_serial_device *serial, rt_size_t len)
{
    struct rt_serial_rx_fifo *rx_fifo = (struct rt_serial_rx_fifo *)serial->serial_rx;

    assert_param(rx_fifo != NULL);

    if (rx_fifo->get_index <= rx_fifo->put_index)
    {
        rx_fifo->put_index += len;
        /* beyond the fifo end */
        if (rx_fifo->put_index >= serial->config.bufsz)
        {
            rx_fifo->put_index %= serial->config.bufsz;
            /* force overwrite get index */
            if (rx_fifo->put_index >= rx_fifo->get_index)
            {
                rx_fifo->is_full = RT_TRUE;
            }
        }
    }
    else
    {
        rx_fifo->put_index += len;
        if (rx_fifo->put_index >= rx_fifo->get_index)
        {
            /* beyond the fifo end */
            if (rx_fifo->put_index >= serial->config.bufsz)
            {
                rx_fifo->put_index %= serial->config.bufsz;
            }
            /* force overwrite get index */
            rx_fifo->is_full = RT_TRUE;
        }
    }
    
    if(rx_fifo->is_full == RT_TRUE)
    {
        _serial_check_buffer_size();
        rx_fifo->get_index = rx_fifo->put_index;
    }
}

/*
 * Serial DMA routines
 */
rt_inline int _serial_dma_rx(struct rt_serial_device *serial, uint8_t *data, int length)
{
    rt_base_t level;

    assert_param((serial != NULL) && (data != NULL));

    level = rt_hw_interrupt_disable();

    if (serial->config.bufsz == 0)
    {
        int result = DRV_SUCCESS;
        struct rt_serial_rx_dma *rx_dma;

        rx_dma = (struct rt_serial_rx_dma*)serial->serial_rx;
        assert_param(rx_dma != NULL);

        if (rx_dma->activated != RT_TRUE)
        {
            rx_dma->activated = RT_TRUE;
            assert_param(serial->ops->dma_transmit != NULL);
            serial->ops->dma_transmit(serial, data, length, RT_SERIAL_DMA_RX);
        }
        else result = -RT_EBUSY;
        rt_hw_interrupt_enable(level);

        if (result == DRV_SUCCESS) return length;

        rt_set_errno(result);
        return 0;
    }
    else
    {
        struct rt_serial_rx_fifo *rx_fifo = (struct rt_serial_rx_fifo *) serial->serial_rx;
        rt_size_t recv_len = 0, fifo_recved_len = rt_dma_calc_recved_len(serial);

        assert_param(rx_fifo != NULL);

        if (length < (int)fifo_recved_len)
            recv_len = length;
        else
            recv_len = fifo_recved_len;

        if (rx_fifo->get_index + recv_len < serial->config.bufsz)
            rt_memcpy(data, rx_fifo->buffer + rx_fifo->get_index, recv_len);
        else
        {
            rt_memcpy(data, rx_fifo->buffer + rx_fifo->get_index,
                    serial->config.bufsz - rx_fifo->get_index);
            rt_memcpy(data + serial->config.bufsz - rx_fifo->get_index, rx_fifo->buffer,
                    recv_len + rx_fifo->get_index - serial->config.bufsz);
        }
        rt_dma_recv_update_get_index(serial, recv_len);
        rt_hw_interrupt_enable(level);
        return recv_len;
    }
}

rt_inline int _serial_dma_tx(struct rt_serial_device *serial, const uint8_t *data, int length)
{
    rt_base_t level;
    rt_err_t result;
    struct rt_serial_tx_dma *tx_dma;

    tx_dma = (struct rt_serial_tx_dma*)(serial->serial_tx);

    result = rt_data_queue_push(&(tx_dma->data_queue), data, length, RT_WAITING_FOREVER);
    if (result == DRV_SUCCESS)
    {
        level = rt_hw_interrupt_disable();
        if (tx_dma->activated != RT_TRUE)
        {
            tx_dma->activated = RT_TRUE;
            rt_hw_interrupt_enable(level);

            /* make a DMA transfer */
            serial->ops->dma_transmit(serial, (uint8_t *)data, length, RT_SERIAL_DMA_TX);
        }
        else
        {
            rt_hw_interrupt_enable(level);
        }

        return length;
    }
    else
    {
        rt_set_errno(result);
        return 0;
    }
}
#endif /* RT_SERIAL_USING_DMA */

/* RT-Thread Device Interface */
/*
 * This function initializes serial device.
 */
static uint32_t rt_serial_init(struct rt_device *dev)
{
    uint32_t result = DRV_SUCCESS;
    struct rt_serial_device *serial;

    assert_param(dev != NULL);
    serial = (struct rt_serial_device *)dev;

    /* initialize rx/tx */
    serial->serial_rx = NULL;
    serial->serial_tx = NULL;

    /* apply configuration */
    if (serial->ops->configure)
        result = serial->ops->configure(serial, &serial->config);

    return result;
}

static uint32_t rt_serial_open(struct rt_device *dev, uint16_t oflag)
{
    uint16_t stream_flag = 0;
    struct rt_serial_device *serial;

    assert_param(dev != NULL);
    serial = (struct rt_serial_device *)dev;

    /* check device flag with the open flag */
    if ((oflag & RT_DEVICE_FLAG_DMA_RX) && !(dev->flag & RT_DEVICE_FLAG_DMA_RX))
        return DRV_ERROR_NOT_FOUND;
    if ((oflag & RT_DEVICE_FLAG_DMA_TX) && !(dev->flag & RT_DEVICE_FLAG_DMA_TX))
        return DRV_ERROR_NOT_FOUND;
    if ((oflag & RT_DEVICE_FLAG_INT_RX) && !(dev->flag & RT_DEVICE_FLAG_INT_RX))
        return DRV_ERROR_NOT_FOUND;
    if ((oflag & RT_DEVICE_FLAG_INT_TX) && !(dev->flag & RT_DEVICE_FLAG_INT_TX))
        return DRV_ERROR_NOT_FOUND;

    /* keep steam flag */
    if ((oflag & RT_DEVICE_FLAG_STREAM) || (dev->open_flag & RT_DEVICE_FLAG_STREAM))
        stream_flag = RT_DEVICE_FLAG_STREAM;

    /* get open flags */
    dev->open_flag = oflag & 0xff;

    /* initialize the Rx/Tx structure according to open flag */
    if (serial->serial_rx == NULL)
    { 
        if (oflag & RT_DEVICE_FLAG_INT_RX)
        {
            struct rt_serial_rx_fifo* rx_fifo;

            rx_fifo = (struct rt_serial_rx_fifo*) rt_malloc (sizeof(struct rt_serial_rx_fifo) +
                serial->config.bufsz);
            assert_param(rx_fifo != NULL);
            rx_fifo->buffer = (uint8_t*) (rx_fifo + 1);
            memset(rx_fifo->buffer, 0, serial->config.bufsz);
            rx_fifo->put_index = 0;
            rx_fifo->get_index = 0;
            rx_fifo->is_full = false;

            serial->serial_rx = rx_fifo;
            dev->open_flag |= RT_DEVICE_FLAG_INT_RX;
            /* configure low level device */
            serial->ops->control(serial, RT_DEVICE_CTRL_SET_INT, (void *)RT_DEVICE_FLAG_INT_RX);
        }
#ifdef RT_SERIAL_USING_DMA        
        else if (oflag & RT_DEVICE_FLAG_DMA_RX)
        {
            if (serial->config.bufsz == 0) {
                struct rt_serial_rx_dma* rx_dma;

                rx_dma = (struct rt_serial_rx_dma*) rt_malloc (sizeof(struct rt_serial_rx_dma));
                assert_param(rx_dma != NULL);
                rx_dma->activated = false;

                serial->serial_rx = rx_dma;
            } else {
                struct rt_serial_rx_fifo* rx_fifo;

                rx_fifo = (struct rt_serial_rx_fifo*) rt_malloc (sizeof(struct rt_serial_rx_fifo) +
                    serial->config.bufsz);
                assert_param(rx_fifo != NULL);
                rx_fifo->buffer = (uint8_t*) (rx_fifo + 1);
                rt_memset(rx_fifo->buffer, 0, serial->config.bufsz);
                rx_fifo->put_index = 0;
                rx_fifo->get_index = 0;
                rx_fifo->is_full = false;
                serial->serial_rx = rx_fifo;
                /* configure fifo address and length to low level device */
                serial->ops->control(serial, RT_DEVICE_CTRL_CONFIG, (void *) RT_DEVICE_FLAG_DMA_RX);
            }
            dev->open_flag |= RT_DEVICE_FLAG_DMA_RX;
        }
#endif /* RT_SERIAL_USING_DMA */
        else
        {
            serial->serial_rx = NULL;
        }
    }
    else
    {
        if (oflag & RT_DEVICE_FLAG_INT_RX)
            dev->open_flag |= RT_DEVICE_FLAG_INT_RX;
#ifdef RT_SERIAL_USING_DMA
        else if (oflag & RT_DEVICE_FLAG_DMA_RX)
            dev->open_flag |= RT_DEVICE_FLAG_DMA_RX;
#endif /* RT_SERIAL_USING_DMA */  
    }

    if (serial->serial_tx == NULL)
    {
        if (oflag & RT_DEVICE_FLAG_INT_TX)
        {
            struct rt_serial_tx_fifo *tx_fifo;

            tx_fifo = (struct rt_serial_tx_fifo*) rt_malloc(sizeof(struct rt_serial_tx_fifo));
            assert_param(tx_fifo != NULL);

            //rt_completion_init(&(tx_fifo->completion));
            serial->serial_tx = tx_fifo;

            dev->open_flag |= RT_DEVICE_FLAG_INT_TX;
            /* configure low level device */
            serial->ops->control(serial, RT_DEVICE_CTRL_SET_INT, (void *)RT_DEVICE_FLAG_INT_TX);
        }
#ifdef RT_SERIAL_USING_DMA
        else if (oflag & RT_DEVICE_FLAG_DMA_TX)
        {
            struct rt_serial_tx_dma* tx_dma;

            tx_dma = (struct rt_serial_tx_dma*) rt_malloc (sizeof(struct rt_serial_tx_dma));
            assert_param(tx_dma != NULL);
            tx_dma->activated = false;

            rt_data_queue_init(&(tx_dma->data_queue), 8, 4, NULL);
            serial->serial_tx = tx_dma;

            dev->open_flag |= RT_DEVICE_FLAG_DMA_TX;
            /* configure low level device */
            serial->ops->control(serial, RT_DEVICE_CTRL_CONFIG, (void *)RT_DEVICE_FLAG_DMA_TX);
        }
#endif /* RT_SERIAL_USING_DMA */
        else
        {
            serial->serial_tx = NULL;
        }
    }
    else
    {
        if (oflag & RT_DEVICE_FLAG_INT_TX)
            dev->open_flag |= RT_DEVICE_FLAG_INT_TX;
#ifdef RT_SERIAL_USING_DMA
        else if (oflag & RT_DEVICE_FLAG_DMA_TX)
            dev->open_flag |= RT_DEVICE_FLAG_DMA_TX;
#endif /* RT_SERIAL_USING_DMA */    
    }

    /* set stream flag */
    dev->open_flag |= stream_flag;

    return DRV_SUCCESS;
}

static uint32_t rt_serial_close(struct rt_device *dev)
{
    struct rt_serial_device *serial;

    assert_param(dev != NULL);
    serial = (struct rt_serial_device *)dev;

    /* this device has more reference count */
    if (dev->ref_count > 1) return DRV_SUCCESS;

    if (dev->open_flag & RT_DEVICE_FLAG_INT_RX)
    {
        struct rt_serial_rx_fifo* rx_fifo;

        rx_fifo = (struct rt_serial_rx_fifo*)serial->serial_rx;
        assert_param(rx_fifo != NULL);

        rt_free(rx_fifo);
        serial->serial_rx = NULL;
        dev->open_flag &= ~RT_DEVICE_FLAG_INT_RX;
        /* configure low level device */
        serial->ops->control(serial, RT_DEVICE_CTRL_CLR_INT, (void*)RT_DEVICE_FLAG_INT_RX);
    }
#ifdef RT_SERIAL_USING_DMA
    else if (dev->open_flag & RT_DEVICE_FLAG_DMA_RX)
    {
        if (serial->config.bufsz == 0) {
            struct rt_serial_rx_dma* rx_dma;

            rx_dma = (struct rt_serial_rx_dma*)serial->serial_rx;
            assert_param(rx_dma != NULL);

            rt_free(rx_dma);
        } else {
            struct rt_serial_rx_fifo* rx_fifo;

            rx_fifo = (struct rt_serial_rx_fifo*)serial->serial_rx;
            assert_param(rx_fifo != NULL);

            rt_free(rx_fifo);
        }
        /* configure low level device */
        serial->ops->control(serial, RT_DEVICE_CTRL_CLR_INT, (void *) RT_DEVICE_FLAG_DMA_RX);
        serial->serial_rx = NULL;
        dev->open_flag &= ~RT_DEVICE_FLAG_DMA_RX;
    }
#endif /* RT_SERIAL_USING_DMA */
    
    if (dev->open_flag & RT_DEVICE_FLAG_INT_TX)
    {
        struct rt_serial_tx_fifo* tx_fifo;

        tx_fifo = (struct rt_serial_tx_fifo*)serial->serial_tx;
        assert_param(tx_fifo != NULL);

        rt_free(tx_fifo);
        serial->serial_tx = NULL;
        dev->open_flag &= ~RT_DEVICE_FLAG_INT_TX;
        /* configure low level device */
        serial->ops->control(serial, RT_DEVICE_CTRL_CLR_INT, (void*)RT_DEVICE_FLAG_INT_TX);
    }
#ifdef RT_SERIAL_USING_DMA
    else if (dev->open_flag & RT_DEVICE_FLAG_DMA_TX)
    {
        struct rt_serial_tx_dma* tx_dma;

        tx_dma = (struct rt_serial_tx_dma*)serial->serial_tx;
        assert_param(tx_dma != NULL);

        rt_free(tx_dma);
        serial->serial_tx = NULL;
        dev->open_flag &= ~RT_DEVICE_FLAG_DMA_TX;
    }
#endif /* RT_SERIAL_USING_DMA */
    return DRV_SUCCESS;
}

static uint32_t rt_serial_read(struct rt_device *dev,
                                uint32_t          pos,
                                void             *buffer,
                                uint32_t         size)
{
    struct rt_serial_device *serial;

    assert_param(dev != NULL);
    if (size == 0) return 0;

    serial = (struct rt_serial_device *)dev;

    if (dev->open_flag & RT_DEVICE_FLAG_INT_RX)
    {
        return _serial_int_rx(serial, (uint8_t *)buffer, size);
    }
#ifdef RT_SERIAL_USING_DMA
    else if (dev->open_flag & RT_DEVICE_FLAG_DMA_RX)
    {
        return _serial_dma_rx(serial, (uint8_t *)buffer, size);
    }
#endif /* RT_SERIAL_USING_DMA */    

    return _serial_poll_rx(serial, (uint8_t *)buffer, size);
}

static uint32_t rt_serial_write(struct rt_device *dev,
                                 uint32_t          pos,
                                 const void       *buffer,
                                 uint32_t         size)
{
    struct rt_serial_device *serial;

    assert_param(dev != NULL);
    if (size == 0) return 0;

    serial = (struct rt_serial_device *)dev;

    if (dev->open_flag & RT_DEVICE_FLAG_INT_TX)
    {
        return _serial_int_tx(serial, (const uint8_t *)buffer, size);
    }
#ifdef RT_SERIAL_USING_DMA    
    else if (dev->open_flag & RT_DEVICE_FLAG_DMA_TX)
    {
        return _serial_dma_tx(serial, (const uint8_t *)buffer, size);
    }
#endif /* RT_SERIAL_USING_DMA */
    else
    {
        return _serial_poll_tx(serial, (const uint8_t *)buffer, size);
    }
}

#ifdef RT_USING_POSIX_TERMIOS
struct speed_baudrate_item
{
    speed_t speed;
    int baudrate;
};

const static struct speed_baudrate_item _tbl[] =
{
    {B2400, BAUD_RATE_2400},
    {B4800, BAUD_RATE_4800},
    {B9600, BAUD_RATE_9600},
    {B19200, BAUD_RATE_19200},
    {B38400, BAUD_RATE_38400},
    {B57600, BAUD_RATE_57600},
    {B115200, BAUD_RATE_115200},
    {B230400, BAUD_RATE_230400},
    {B460800, BAUD_RATE_460800},
    {B921600, BAUD_RATE_921600},
    {B2000000, BAUD_RATE_2000000},
    {B3000000, BAUD_RATE_3000000},
};

static speed_t _get_speed(int baudrate)
{
    int index;

    for (index = 0; index < sizeof(_tbl)/sizeof(_tbl[0]); index ++)
    {
        if (_tbl[index].baudrate == baudrate)
            return _tbl[index].speed;
    }

    return B0;
}

static int _get_baudrate(speed_t speed)
{
    int index;

    for (index = 0; index < sizeof(_tbl)/sizeof(_tbl[0]); index ++)
    {
        if (_tbl[index].speed == speed)
            return _tbl[index].baudrate;
    }

    return 0;
}

static void _tc_flush(struct rt_serial_device *serial, int queue)
{
    rt_base_t level;
    int ch = -1;
    struct rt_serial_rx_fifo *rx_fifo = NULL;
    struct rt_device *device = NULL;

    assert_param(serial != NULL);

    device = &(serial->parent);
    rx_fifo = (struct rt_serial_rx_fifo *) serial->serial_rx;

    switch(queue)
    {
        case TCIFLUSH:
        case TCIOFLUSH:

            assert_param(rx_fifo != NULL);

            if((device->open_flag & RT_DEVICE_FLAG_INT_RX) || (device->open_flag & RT_DEVICE_FLAG_DMA_RX))
            {
                assert_param(NULL != rx_fifo);
                level = rt_hw_interrupt_disable();
                rt_memset(rx_fifo->buffer, 0, serial->config.bufsz);
                rx_fifo->put_index = 0;
                rx_fifo->get_index = 0;
                rx_fifo->is_full = false;
                rt_hw_interrupt_enable(level);
            }
            else
            {
                while (1)
                {
                    ch = serial->ops->getc(serial);
                    if (ch == -1) break;
                }
            }

            break;

         case TCOFLUSH:
            break;
    }

}

#endif

static uint32_t rt_serial_control(struct rt_device *dev,
                                  int              cmd,
                                  void             *args)
{
    uint32_t ret = DRV_SUCCESS;
    struct rt_serial_device *serial;

    assert_param(dev != NULL);
    serial = (struct rt_serial_device *)dev;

    switch (cmd)
    {
        case RT_DEVICE_CTRL_CONFIG:
            if (args)
            {
                struct serial_configure *pconfig = (struct serial_configure *) args;
                if (pconfig->bufsz != serial->config.bufsz && serial->parent.ref_count)
                {
                    /*can not change buffer size*/
                    return DRV_ERROR_BUSY;
                }
                /* set serial configure */
                serial->config = *pconfig;
                if (serial->parent.ref_count)
                {
                    /* serial device has been opened, to configure it */
                    serial->ops->configure(serial, (struct serial_configure *) args);
                }
            }

            break;
        default :
            /* control device */
            ret = serial->ops->control(serial, cmd, args);
            break;
    }

    return ret;
}

const static struct rt_device_ops serial_ops = 
{
    rt_serial_init,
    rt_serial_open,
    rt_serial_close,
    rt_serial_read,
    rt_serial_write,
    rt_serial_control
};


/*
 * serial register
 */
uint32_t rt_hw_serial_register(struct rt_serial_device *serial,
                               const char              *name,
                               uint32_t              flag,
                               void                    *data)
{
    uint32_t ret;
    struct rt_device *device;
    assert_param(serial != NULL);

    device = &(serial->parent);

    device->type        = RT_Device_Class_Char;
    device->rx_indicate = NULL;
    device->tx_complete = NULL;

    device->ops         = &serial_ops;

    device->user_data   = data;

    /* register a character device */
    ret = rt_device_register(device, name, flag);

    return ret;
}

/* ISR for serial interrupt */
void rt_hw_serial_isr(struct rt_serial_device *serial, int event)
{
    switch (event & 0xff)
    {
        case DRV_SERIAL_EVENT_RX_IND:
        {
            int ch = -1;
            struct rt_serial_rx_fifo* rx_fifo;

            /* interrupt mode receive */
            rx_fifo = (struct rt_serial_rx_fifo*)serial->serial_rx;
            assert_param(rx_fifo != NULL);

            while (1)
            {
                ch = serial->ops->getc(serial);
                if (ch == -1) break;


                /* disable interrupt */
                rt_hw_interrupt_disable();

                rx_fifo->buffer[rx_fifo->put_index] = ch;
                rx_fifo->put_index += 1;
                if (rx_fifo->put_index >= serial->config.bufsz) rx_fifo->put_index = 0;

                /* if the next position is read index, discard this 'read char' */
                if (rx_fifo->put_index == rx_fifo->get_index)
                {
                    rx_fifo->get_index += 1;
                    rx_fifo->is_full = true;
                    if (rx_fifo->get_index >= serial->config.bufsz) rx_fifo->get_index = 0;

                    _serial_check_buffer_size();
                }

                /* enable interrupt */
                rt_hw_interrupt_enable();
            }

            /* invoke callback */
            if (serial->parent.rx_indicate != NULL)
            {
                uint32_t rx_length;

                /* get rx length */
                rt_hw_interrupt_disable();
                rx_length = (rx_fifo->put_index >= rx_fifo->get_index)? (rx_fifo->put_index - rx_fifo->get_index):
                    (serial->config.bufsz - (rx_fifo->get_index - rx_fifo->put_index));
                rt_hw_interrupt_enable();

                if (rx_length)
                {
                    serial->parent.rx_indicate(&serial->parent, rx_length);
                }
            }
            break;
        }
        case DRV_SERIAL_EVENT_TX_DONE:
        {
            struct rt_serial_tx_fifo* tx_fifo;

            tx_fifo = (struct rt_serial_tx_fifo*)serial->serial_tx;
            //rt_completion_done(&(tx_fifo->completion));
            break;
        }
#ifdef RT_SERIAL_USING_DMA
        case RT_SERIAL_EVENT_TX_DMADONE:
        {
            const void *data_ptr;
            rt_size_t data_size;
            const void *last_data_ptr;
            struct rt_serial_tx_dma *tx_dma;

            tx_dma = (struct rt_serial_tx_dma*) serial->serial_tx;

            rt_data_queue_pop(&(tx_dma->data_queue), &last_data_ptr, &data_size, 0);
            if (rt_data_queue_peak(&(tx_dma->data_queue), &data_ptr, &data_size) == DRV_SUCCESS)
            {
                /* transmit next data node */
                tx_dma->activated = RT_TRUE;
                serial->ops->dma_transmit(serial, (uint8_t *)data_ptr, data_size, RT_SERIAL_DMA_TX);
            }
            else
            {
                tx_dma->activated = false;
            }

            /* invoke callback */
            if (serial->parent.tx_complete != NULL)
            {
                serial->parent.tx_complete(&serial->parent, (void*)last_data_ptr);
            }
            break;
        }
        case DRV_SERIAL_EVENT_RX_DMADONE:
        {
            int length;
            rt_base_t level;

            /* get DMA rx length */
            length = (event & (~0xff)) >> 8;

            if (serial->config.bufsz == 0)
            {
                struct rt_serial_rx_dma* rx_dma;

                rx_dma = (struct rt_serial_rx_dma*) serial->serial_rx;
                assert_param(rx_dma != NULL);

                assert_param(serial->parent.rx_indicate != NULL);
                serial->parent.rx_indicate(&(serial->parent), length);
                rx_dma->activated = false;
            }
            else
            {
                /* disable interrupt */
                level = rt_hw_interrupt_disable();
                /* update fifo put index */
                rt_dma_recv_update_put_index(serial, length);
                /* calculate received total length */
                length = rt_dma_calc_recved_len(serial);
                /* enable interrupt */
                rt_hw_interrupt_enable(level);
                /* invoke callback */
                if (serial->parent.rx_indicate != NULL)
                {
                    serial->parent.rx_indicate(&(serial->parent), length);
                }
            }
            break;
        }
#endif /* RT_SERIAL_USING_DMA */
    }
}


int drv_usart_init(void)
{
    uint32_t obj_num = sizeof(uart_obj) / sizeof(struct drv_uart);
    struct serial_configure config = DRV_SERIAL_CONFIG_DEFAULT;
    uint32_t result = 0;

    drv_uart_get_dma_config();

    for (uint8_t i = 0; i < obj_num; i++)
    {
        uart_obj[i].config = &uart_config[i];
        uart_obj[i].serial.ops    = &drv_uart_ops;
        uart_obj[i].serial.config = config;
        /* register UART device */
        result = rt_hw_serial_register(&uart_obj[i].serial, uart_obj[i].config->name,
                                       RT_DEVICE_FLAG_RDWR
                                       | RT_DEVICE_FLAG_INT_RX
                                       | RT_DEVICE_FLAG_INT_TX
                                       | uart_obj[i].uart_dma_flag
                                       , NULL);
        assert_param(result == DRV_SUCCESS);
    }

    return result;
}


