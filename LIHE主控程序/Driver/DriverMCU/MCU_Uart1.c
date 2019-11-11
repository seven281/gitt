#include "MCU_Uart1.h"
#include "ADT_CircleByteQueue.h"
#include "MCU_Port.h"

//串口中断接收数据处理函数
static MCU_UartRecvIntProcFunc uart1RecvFuncPtr = NULL;

//串口互斥信号量
static OS_MUTEX mutexMCU_Uart1;

//循环队列
static ADT_CIRCLE_BYTE_QUEUE circleByteQueueUart1Send = {0};

//用于循环队列存储数据的数组
static uint8_t bufferWithUart1Send[LENGTH_UART1_BUFFER_WITH_SEND] = {0};

//用于DMA发送的数据块
static uint8_t blockBufferWithUart1DMA_Send[LENGTH_UART1_BUFFER_WITH_BLOCK] = {0};

//用于printf的缓冲区
static uint8_t blockBufferWithUart1Printf[LENGTH_UART1_BUFFER_WITH_FORMAT] = {0};

//串口DMA中断发送正在运行标志
static uint8_t flagUart1DMA = 0;


//串口初始化
void MCU_Uart1Init(uint32_t baud, MCU_UART_LENGTH length, MCU_UART_STOPBIT stopBit,
                      MCU_UART_CHECK_MODE checkMode, MCU_UART_HARD_CONTROL hardWareControl, MCU_UartRecvIntProcFunc rxCallBack)
{
    //DMA模式驱动,不需要发送完成中断
    USART_InitTypeDef USART_InitStructure;
    NVIC_InitTypeDef NVIC_InitStructure;
    DMA_InitTypeDef DMA_InitStructure;
    OS_ERR err;

    ADT_CircleByteQueueInit(&circleByteQueueUart1Send, bufferWithUart1Send, LENGTH_UART1_BUFFER_WITH_SEND);

    //使能USART1时钟
    RCC_APB2PeriphClockCmd(RCC_APB2Periph_USART1, ENABLE);
    //DMA2时钟使能
    RCC_AHB1PeriphClockCmd(RCC_AHB1Periph_DMA2, ENABLE);

    //复位串口和DMA模块
    RCC_APB2PeriphResetCmd(RCC_APB2Periph_USART1,ENABLE);
    RCC_AHB1PeriphResetCmd(RCC_AHB1Periph_DMA2,ENABLE);

    //取消复位
    RCC_APB2PeriphResetCmd(RCC_APB2Periph_USART1,DISABLE);
    RCC_AHB1PeriphResetCmd(RCC_AHB1Periph_DMA2,DISABLE);

    //通道初始化
    DMA_DeInit(DMA2_Stream7);
    //等待DMA可配置
    while (DMA_GetCmdStatus(DMA2_Stream7) != DISABLE)
    {
    }

    //USART1端口配置
    MCU_PortInit(MCU_PIN_A_9, GPIO_Mode_AF, GPIO_OType_PP, GPIO_PuPd_UP, GPIO_Speed_100MHz);
    MCU_PortInit(MCU_PIN_A_10, GPIO_Mode_AF, GPIO_OType_PP, GPIO_PuPd_UP, GPIO_Speed_100MHz);

    //串口1对应引脚复用映射,先初始化IO口,然后进行管脚映射
    //GPIOA9复用为USART1
    GPIO_PinAFConfig(GPIOA, GPIO_PinSource9, GPIO_AF_USART1);
    //GPIOA10复用为USART1
    GPIO_PinAFConfig(GPIOA, GPIO_PinSource10, GPIO_AF_USART1);

    //USART1 初始化设置
    USART_InitStructure.USART_BaudRate = baud;
    USART_InitStructure.USART_WordLength = length;
    USART_InitStructure.USART_StopBits = stopBit;
    USART_InitStructure.USART_Parity = checkMode;
    USART_InitStructure.USART_HardwareFlowControl = hardWareControl;
    USART_InitStructure.USART_Mode = USART_Mode_Rx | USART_Mode_Tx;
    USART_Init(USART1, &USART_InitStructure);
    //使能串口1
    USART_Cmd(USART1, ENABLE);

    //STM32在使能串口的时候会发送一帧空闲帧作为起始,所以要等待第一帧发送完成
    USART_ClearFlag(USART1, USART_FLAG_TC);

    while (USART_GetFlagStatus(USART1, USART_FLAG_TC) == RESET)
    {
        asm("nop"); //等待空闲帧发送完成后  再清零发送标志
    }
    USART_ClearFlag(USART1, USART_FLAG_TC);

    //开启相关中断
    USART_ITConfig(USART1, USART_IT_RXNE, ENABLE);

    //Usart1 NVIC 配置
    NVIC_InitStructure.NVIC_IRQChannel = USART1_IRQn;
    NVIC_InitStructure.NVIC_IRQChannelPreemptionPriority = MCU_UART1_DMA_PREE_PRI;
    NVIC_InitStructure.NVIC_IRQChannelSubPriority = MCU_UART1_DMA_SUB_PRI;
    NVIC_InitStructure.NVIC_IRQChannelCmd = ENABLE;
    NVIC_Init(&NVIC_InitStructure);

    /* 配置 DMA Stream */
    //通道选择
    DMA_InitStructure.DMA_Channel = DMA_Channel_4;
    //DMA外设地址
    DMA_InitStructure.DMA_PeripheralBaseAddr = (uint32_t)&USART1->DR;
    //DMA 存储器0地址
    DMA_InitStructure.DMA_Memory0BaseAddr = (uint32_t)(blockBufferWithUart1DMA_Send);
    //存储器到外设模式
    DMA_InitStructure.DMA_DIR = DMA_DIR_MemoryToPeripheral;
    //数据传输量
    DMA_InitStructure.DMA_BufferSize = LENGTH_UART1_BUFFER_WITH_BLOCK;
    //外设非增量模式
    DMA_InitStructure.DMA_PeripheralInc = DMA_PeripheralInc_Disable;
    //存储器增量模式
    DMA_InitStructure.DMA_MemoryInc = DMA_MemoryInc_Enable;
    //外设数据长度:8位
    DMA_InitStructure.DMA_PeripheralDataSize = DMA_PeripheralDataSize_Byte;
    //存储器数据长度:8位
    DMA_InitStructure.DMA_MemoryDataSize = DMA_MemoryDataSize_Byte;
    // 使用普通模式
    DMA_InitStructure.DMA_Mode = DMA_Mode_Normal;
    //中等优先级
    DMA_InitStructure.DMA_Priority = DMA_Priority_Medium;
    DMA_InitStructure.DMA_FIFOMode = DMA_FIFOMode_Disable;
    DMA_InitStructure.DMA_FIFOThreshold = DMA_FIFOThreshold_Full;
    //存储器突发单次传输
    DMA_InitStructure.DMA_MemoryBurst = DMA_MemoryBurst_Single;
    //外设突发单次传输
    DMA_InitStructure.DMA_PeripheralBurst = DMA_PeripheralBurst_Single;
    //初始化DMA Stream
    DMA_Init(DMA2_Stream7, &DMA_InitStructure);

    //清除DMA中断
    DMA_ClearFlag(DMA2_Stream7, DMA_FLAG_TCIF4);

    //DMA发送中断,不使用操作系统API,单纯发送数据
    NVIC_InitStructure.NVIC_IRQChannel = DMA2_Stream7_IRQn;
    NVIC_InitStructure.NVIC_IRQChannelPreemptionPriority = MCU_DMA2_STREAM7_PREE_PRI;
    NVIC_InitStructure.NVIC_IRQChannelSubPriority = MCU_DMA2_STREAM7_SUB_PRI;
    NVIC_InitStructure.NVIC_IRQChannelCmd = ENABLE;
    NVIC_Init(&NVIC_InitStructure);
    //打开DMA中断
    DMA_ITConfig(DMA2_Stream7, DMA_IT_TC, ENABLE);

    //创建一个互斥信号量,用于DMA串口发送
	OSMutexCreate((OS_MUTEX*	)&mutexMCU_Uart1,
				  (CPU_CHAR*	)"mutexMCU_Uart1",
                  (OS_ERR*		)&err);	

    uart1RecvFuncPtr = rxCallBack;
    flagUart1DMA = 0;
}

//串口发送数组
void MCU_Uart1SendBuffer(uint8_t* bufferStartPtr,uint16_t sendLength)
{
    uint16_t currentBufferHadSendLength = 0;
    uint16_t currentBufferWaitSendLength = sendLength;
    uint16_t realWriteDataCount = 0;
    OS_ERR err;
    if(sendLength == 0)
    {
        return;
    }
    if (OSRunning)
    {
        //请求互斥信号量,申请不到,任务挂起等待OS_OPT_PEND_BLOCKING
        OSMutexPend(&mutexMCU_Uart1, 0, OS_OPT_PEND_BLOCKING, 0, &err);
    }
    do
    {
        realWriteDataCount = 0;
        ADT_CircleByteQueueWriteBuffer(&circleByteQueueUart1Send,(bufferStartPtr+currentBufferHadSendLength),currentBufferWaitSendLength,
                                        &realWriteDataCount);
        currentBufferHadSendLength += realWriteDataCount;
        currentBufferWaitSendLength -= realWriteDataCount;
        if(flagUart1DMA == 0)
        {
            ADT_CircleByteQueueReadBuffer(&circleByteQueueUart1Send,blockBufferWithUart1DMA_Send,LENGTH_UART1_BUFFER_WITH_BLOCK,&realWriteDataCount);
            if(realWriteDataCount > 0)
            {
                //数据传输量
                DMA_SetCurrDataCounter(DMA2_Stream7, realWriteDataCount);
                //使能串口1的DMA发送
                USART_DMACmd(USART1, USART_DMAReq_Tx, ENABLE);
                //开启DMA传输
                DMA_Cmd(DMA2_Stream7, ENABLE);
                flagUart1DMA = 1;
            }
        }
        if(currentBufferHadSendLength != sendLength)       
        {
            CoreDelayMinTick();
        }                       
    }while(currentBufferHadSendLength != sendLength);
    if (OSRunning)
    {
        //释放互斥信号量,禁止在释放信号量的时候出发任务调度
        OSMutexPost(&mutexMCU_Uart1, OS_OPT_POST_NO_SCHED, &err);
    }
}

//串口发送字符串
void MCU_Uart1SendString(uint8_t* stringStartPtr)
{
    int stringLength = strlen((char const *)stringStartPtr);
    if (stringLength <= 0)
    {
        return;
    }
    MCU_Uart1SendBuffer(stringStartPtr, (uint16_t)stringLength);
}

//串口发送字符串,带格式化
int MCU_Uart1Printf(const char *format, ...)
{
    //转换
    va_list arg;
    int rv;
    OS_ERR err;
    if (OSRunning)
    {
        //请求互斥信号量,申请不到,任务挂起等待OS_OPT_PEND_BLOCKING
        OSMutexPend(&mutexMCU_Uart1, 0, OS_OPT_PEND_BLOCKING, 0, &err);
    }
    va_start(arg, format);
    rv = vsnprintf((char *)blockBufferWithUart1Printf, LENGTH_UART1_BUFFER_WITH_FORMAT, format, arg);
    va_end(arg);
    if ((rv > 0) && (rv <= LENGTH_UART1_BUFFER_WITH_FORMAT))
    {
        MCU_Uart1SendBuffer(blockBufferWithUart1Printf, (uint16_t)rv);
    }
    else
    {
        //转换失败
    }
    if (OSRunning)
    {
        //释放互斥信号量,禁止在释放信号量的时候出发任务调度
        OSMutexPost(&mutexMCU_Uart1, OS_OPT_POST_NO_SCHED, &err);
    }
    return rv;
}

//串口接收中断
void USART1_IRQHandler(void) //串口1中断服务程序
{
    uint8_t intData;
    
    //中断内核感知,需要
    CPU_SR_ALLOC();
    CPU_CRITICAL_ENTER();
    //此时要禁止操作系统调度打断中断
    OSIntEnter();
    //关闭了就要打开
    CPU_CRITICAL_EXIT();

    //获取当前是否存在接收中断
    if (USART_GetITStatus(USART1, USART_IT_RXNE) != RESET)
    {
        intData = USART_ReceiveData(USART1);
        if (uart1RecvFuncPtr != NULL)
        {
            uart1RecvFuncPtr(intData);
        }
    }
    //退出中断,启用调度
    OSIntExit();
}



//DMA发送中断
void DMA2_Stream7_IRQHandler()
{
    //DMA传输完成中断4
    if (DMA_GetITStatus(DMA2_Stream7, DMA_IT_TCIF7) != RESET)
    {
        uint16_t transPackLength = 0;
        //清除中断
        DMA_ClearITPendingBit(DMA2_Stream7, DMA_IT_TCIF7);
        ADT_CircleByteQueueReadBuffer(&circleByteQueueUart1Send,blockBufferWithUart1DMA_Send,LENGTH_UART1_BUFFER_WITH_BLOCK,&transPackLength);
        //缓冲区里面有数据
        if(transPackLength > 0)
        {
            //数据传输量
            DMA_SetCurrDataCounter(DMA2_Stream7, transPackLength);
            //使能串口1的DMA发送
            USART_DMACmd(USART1, USART_DMAReq_Tx, ENABLE);
            //开启DMA传输
            DMA_Cmd(DMA2_Stream7, ENABLE);
        }
        else
        {
            //关闭DMA传输
            DMA_Cmd(DMA2_Stream7, DISABLE);
            //设置DMA中断结束
            flagUart1DMA = 0;
        }
    }
}



































