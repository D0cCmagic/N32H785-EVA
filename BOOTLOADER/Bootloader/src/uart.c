
#include "uart.h"
#include "main.h"
#include "misc.h"
#include "delay.h"
#include "xmodem.h"

extern uint8_t received_packet_data[X_PACKET_1024_SIZE];

///**
//*\*\name    fputc.
//*\*\fun     retarget the C library printf function to the USART
//*\*\param   ch
//*\*\param   f
//*\*\return  none
//**/
int fputc(int ch, FILE* f)
{
    /* Loop until the end of transmission */
    while (USART_GetFlagStatus(USART1, USART_FLAG_TXC) == RESET)
    {
    }
    USART_SendData(USART1, (uint8_t)ch);

    return (ch);
}

static ErrorStatus UART_WaitFlagTimeout(USART_Module* USARTx, uint32_t Flag, uint32_t Timeout)
{
    uint32_t time_cnt = 0;
    
    while(USART_GetFlagStatus(USARTx, Flag) == RESET)
    {
        time_cnt++;
        systick_delay_us(100);

        if(time_cnt  > Timeout)
        {
            return  ERROR;
        }
    }
    return SUCCESS;
}


void uart_rcc_config(void)
{
    /* Initialize system clock */
    RCC_SetSysClkToMode0();

    /* Enable GPIO clock */
    RCC_EnableAHB5PeriphClk1(RCC_AHB5_PERIPHEN_M7_GPIOA, ENABLE); //GPIOA~GPIOH

    RCC_EnableAHB5PeriphClk2(RCC_AHB5_PERIPHEN_M7_AFIO, ENABLE);
    /* Enable USART Clock */
    RCC_EnableAPB1PeriphClk3(RCC_APB1_PERIPHEN_M7_USART1, ENABLE);
}


void uart_gpio_config(void)
{
    GPIO_InitType GPIO_InitStructure = {0};
    
    GPIO_InitStructure.Pin            = GPIO_PIN_9;   //usart1 tx pin
    GPIO_InitStructure.GPIO_Mode      = GPIO_MODE_AF_PP;
    GPIO_InitStructure.GPIO_Pull      = GPIO_NO_PULL;
    GPIO_InitStructure.GPIO_Alternate = GPIO_AF7;
    GPIO_InitPeripheral(GPIOA, &GPIO_InitStructure );


    GPIO_InitStructure.Pin            = GPIO_PIN_10;  //usart1 rx pin
    GPIO_InitStructure.GPIO_Mode      = GPIO_MODE_AF_PP;
    GPIO_InitStructure.GPIO_Pull      = GPIO_NO_PULL;
    GPIO_InitStructure.GPIO_Alternate = GPIO_AF5;
    GPIO_InitPeripheral(GPIOA, &GPIO_InitStructure );

}



void uart_dma_init(void)
{
    NVIC_InitType NVIC_InitStructure = {0};
    
    /* Enable peripheral clocks */
    
    /* Enable DMA clock */
    RCC_EnableAHB1PeriphClk3(RCC_AHB1_PERIPHEN_M7_DMA1, ENABLE);
    /* Enable DMAMUX clock */
    RCC_EnableAHB1PeriphClk1(RCC_AHB1_PERIPHEN_M7_DMAMUX1, ENABLE);

    DMA_ChInitType DMA_InitStructure = {0};
    DMA_ChStatusType  RetStatus;

   /* USARTz RX DMA1 Channel (triggered by USARTz Rx event) Config */
    DMA_ChannelStructInit(&DMA_InitStructure);
    DMA_InitStructure.IntEn              = 0x1U;
    DMA_InitStructure.SrcAddr            = (uint32_t)&(USART1->DAT);
    DMA_InitStructure.DstAddr            = (uint32_t)received_packet_data;
    DMA_InitStructure.SrcTfrWidth        = DMA_CH_TRANSFER_WIDTH_8;
    DMA_InitStructure.DstTfrWidth        = DMA_CH_TRANSFER_WIDTH_8;
    DMA_InitStructure.DstAddrCountMode   = DMA_CH_ADDRESS_COUNT_MODE_INCREMENT; 
    DMA_InitStructure.SrcAddrCountMode   = DMA_CH_ADDRESS_COUNT_MODE_NO_CHANGE;	
    DMA_InitStructure.TfrTypeFlowCtrl    = DMA_CH_TRANSFER_FLOW_P2M_DMA;

    DMA_InitStructure.DstBurstLen        = DMA_CH_BURST_LENGTH_1;
    DMA_InitStructure.SrcBurstLen        = DMA_CH_BURST_LENGTH_1;
    DMA_InitStructure.SrcGatherEn        = 0x0U;
    DMA_InitStructure.DstScatterEn       = 0x0U;
    DMA_InitStructure.BlkTfrSize         = X_PACKET_1024_SIZE;	
    DMA_InitStructure.pLinkListItem      = NULL;
    DMA_InitStructure.SrcGatherInterval  = 0x0U;
    DMA_InitStructure.SrcGatherCount     = 0x0U;
    DMA_InitStructure.DstScatterInterval = 0x0U;
    DMA_InitStructure.DstScatterCount    = 0x0U;
    DMA_InitStructure.TfrType            = DMA_CH_TRANSFER_TYPE_SINGLE_BLOCK;
    DMA_InitStructure.ChannelPriority    = DMA_CH_PRIORITY_7;
    DMA_InitStructure.SrcHandshaking	 = DMA_CH_SRC_HANDSHAKING_HARDWARE;
    DMA_InitStructure.SrcHsInterface     = DMA_CH_HARDWARE_HANDSHAKING_IF_0;
    DMA_InitStructure.DstHandshaking     = DMA_CH_DST_HANDSHAKING_HARDWARE;
    DMA_InitStructure.DstHsInterface     = DMA_CH_HARDWARE_HANDSHAKING_IF_1;
    
    /* DMA controller must be enabled before initializing the channel */
    DMA_ControllerCmd(DMA1, ENABLE);
    
    /* Initialize the specified DMA channel */
    RetStatus = DMA_ChannelInit(DMA1, &DMA_InitStructure, DMA_CHANNEL_0);
    
     /* Whether the specified channel was successfully initialized */
    if (RetStatus == DMA_CH_STS_OK)
    {
        /* Enable Source transaction complete interrupt event */
        DMA_ChannelEventCmd(DMA1, DMA_CHANNEL_0, DMA_CH_EVENT_SRC_TRANSACTION_COMPLETE, ENABLE);

        /* Configure DMAMUX */
        DMAMUX_SetRequestID(DMAMUX1_ID, DMAMUX_CHANNEL_0, DMAMUX1_REQUEST_USART1_RX);

        /* Enable the specified DMA channel */
        DMA_ChannelCmd(DMA1, DMA_CHANNEL_0, ENABLE);
    }
    
    /* Enable DMA channel IRQ */
    NVIC_InitStructure.NVIC_IRQChannel                   = DMA1_Channel0_IRQn; 
    NVIC_InitStructure.NVIC_IRQChannelPreemptionPriority = 0;
    NVIC_InitStructure.NVIC_IRQChannelSubPriority        = 0;
    NVIC_InitStructure.NVIC_IRQChannelCmd                = ENABLE;
    NVIC_Init(&NVIC_InitStructure);
}


void uart_init(void)
{
    USART_InitType USART_InitStructure = {0}; 
    NVIC_InitType NVIC_InitStructure  = {0};
    uart_rcc_config();
    uart_gpio_config();

    USART_StructInit(&USART_InitStructure);
    USART_InitStructure.BaudRate   = 115200;
    USART_InitStructure.WordLength = USART_WL_8B;
    USART_InitStructure.StopBits   = USART_STPB_1;
    USART_InitStructure.Parity     = USART_PE_NO ;
    USART_InitStructure.HardwareFlowControl = USART_HFCTRL_NONE;
    USART_InitStructure.Mode = USART_MODE_RX | USART_MODE_TX;
    USART_Init( USART1, &USART_InitStructure );

    uart_dma_init();
    
    /*enable USART1 DMA */
    USART_EnableDMA(USART1, USART_DMAREQ_RX | USART_DMAREQ_TX, ENABLE);
    
    /*enable USART1 IDLE Interrupt*/
    USART_ConfigInt(USART1, USART_INT_IDLEF, ENABLE);

    NVIC_PriorityGroupConfig(NVIC_PriorityGroup_2);
    /* Enable the USART1 Interrupt */
    NVIC_InitStructure.NVIC_IRQChannel             = USART1_IRQn;
    NVIC_InitStructure.NVIC_IRQChannelSubPriority  = 1;
    NVIC_InitStructure.NVIC_IRQChannelCmd          = ENABLE;
    NVIC_Init(&NVIC_InitStructure);
    
    /* Enable the USART1 */
    USART_Enable(USART1, ENABLE );
}

/**
 * @brief   Transmits a string to UART.
 * @param   *data: Array of the data.
 * @return  status: Report about the success of the transmission.
 */
uart_status uart_transmit_str(uint8_t *data)
{
    uart_status status = UART_ERROR;
    uint16_t length = 0u;
    
    /* Calculate the length. */
    while ('\0' != data[length])
    {
        length++;
    }
  
    while (length > 0U)
    {
        length--;
        
        if (UART_WaitFlagTimeout(USART1, USART_FLAG_TXDE, 100) != SUCCESS)
        {
           USART_ClrFlag(USART1, USART_FLAG_TXDE);
           return UART_ERROR;
        }
        USART_SendData(USART1, *data++);
    }

    if (UART_WaitFlagTimeout(USART1, USART_FLAG_TXC, 100) != SUCCESS)
    {
        USART_ClrFlag(USART1, USART_FLAG_TXC);
        return UART_ERROR;
    }

    
    return status;
}


/**
 * @brief   Transmits a single char to UART.
 * @param   *data: The char.
 * @return  status: Report about the success of the transmission.
 */
uart_status uart_transmit_ch(uint8_t data)
{
    uart_status status = UART_ERROR;
    uint16_t TxCnt = 1;

    while (TxCnt > 0U)
    {
        TxCnt--;
        
        if (UART_WaitFlagTimeout(USART1, USART_FLAG_TXDE, 100) != SUCCESS)
        {
           USART_ClrFlag(USART1, USART_FLAG_TXDE);
           return UART_ERROR;
        }
        USART_SendData(USART1, data);
    }
    
    if (UART_WaitFlagTimeout(USART1, USART_FLAG_TXC, 100) != SUCCESS)
    {
        USART_ClrFlag(USART1, USART_FLAG_TXC);
        return UART_ERROR;
    }

    return status;
}











