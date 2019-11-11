#include "MCU_Uart3.h"
#include "ADT_CircleByteQueue.h"
#include "MCU_Port.h"


//串口中断接收数据处理函数
static MCU_UartRecvIntProcFunc uart3RecvFuncPtr = NULL;

//串口互斥信号量
static OS_MUTEX mutexMCU_Uart3;

//循环队列
static ADT_CIRCLE_BYTE_QUEUE circleByteQueueUart3Send = {0};

//用于循环队列存储数据的数组
static uint8_t bufferWithUart3Send[LENGTH_UART3_BUFFER_WITH_SEND] = {0};

//用于DMA发送的数据块
static uint8_t blockBufferWithUart3DMA_Send[LENGTH_UART3_BUFFER_WITH_BLOCK] = {0};

//用于printf的缓冲区
static uint8_t blockBufferWithUart3Printf[LENGTH_UART3_BUFFER_WITH_FORMAT] = {0};

//串口中断发送正在运行标志
static uint8_t flagUart3DMA = 0;

//串口初始化
void MCU_Uart3Init(uint32_t baud, MCU_UART_LENGTH length, MCU_UART_STOPBIT stopBit,
                      MCU_UART_CHECK_MODE checkMode, MCU_UART_HARD_CONTROL hardWareControl, MCU_UartRecvIntProcFunc rxCallBack)
{
    //DMA模式驱动,不需要发送完成中断
    USART_InitTypeDef USART_InitStructure;
    NVIC_InitTypeDef NVIC_InitStructure;
    DMA_InitTypeDef DMA_InitStructure;
    OS_ERR err;

    ADT_CircleByteQueueInit(&circleByteQueueUart3Send, bufferWithUart3Send, LENGTH_UART3_BUFFER_WITH_SEND);

    //使能USART3时钟
    RCC_APB1PeriphClockCmd(RCC_APB1Periph_USART3,ENABLE);
    //DMA1时钟使能
    RCC_AHB1PeriphClockCmd(RCC_AHB1Periph_DMA1, ENABLE);

    //复位串口和DMA模块
    RCC_APB1PeriphResetCmd(RCC_APB1Periph_USART3,ENABLE);
    RCC_AHB1PeriphResetCmd(RCC_AHB1Periph_DMA1,ENABLE);

    //取消复位
    RCC_APB1PeriphResetCmd(RCC_APB1Periph_USART3,DISABLE);
    RCC_AHB1PeriphResetCmd(RCC_AHB1Periph_DMA1,DISABLE);


    //复位串口3
    USART_DeInit(USART3);  

    //通道初始化
    DMA_DeInit(DMA1_Stream3);

    //等待DMA可配置
    while (DMA_GetCmdStatus(DMA1_Stream3) != DISABLE)
    {
    }

    //USART3端口配置
    MCU_PortInit(MCU_PIN_B_10, GPIO_Mode_AF, GPIO_OType_PP, GPIO_PuPd_UP, GPIO_Speed_100MHz);
    MCU_PortInit(MCU_PIN_B_11, GPIO_Mode_AF, GPIO_OType_PP, GPIO_PuPd_UP, GPIO_Speed_100MHz);

    //GPIOB10复用为USART3	
    GPIO_PinAFConfig(GPIOB,GPIO_PinSource10,GPIO_AF_USART3); 
    //GPIOB11复用为USART3
	GPIO_PinAFConfig(GPIOB,GPIO_PinSource11,GPIO_AF_USART3); 

    USART_InitStructure.USART_BaudRate = baud;
    USART_InitStructure.USART_WordLength = length;
    USART_InitStructure.USART_StopBits = stopBit;
    USART_InitStructure.USART_Parity = checkMode;
    USART_InitStructure.USART_HardwareFlowControl = hardWareControl;
    //发送接收同时打开
    USART_InitStructure.USART_Mode = USART_Mode_Rx | USART_Mode_Tx;
    USART_Init(USART3, &USART_InitStructure);
    //使能串口3
    USART_Cmd(USART3, ENABLE);

    //STM32在使能串口的时候会发送一帧空闲帧作为起始,所以要等待第一帧发送完成
    USART_ClearFlag(USART3, USART_FLAG_TC);

    while (USART_GetFlagStatus(USART3, USART_FLAG_TC) == RESET)
    {
        asm("nop"); //等待空闲帧发送完成后  再清零发送标志
    }
    USART_ClearFlag(USART3, USART_FLAG_TC);

    //开启相关中断
    USART_ITConfig(USART3, USART_IT_RXNE, ENABLE);

    //Usart3 NVIC 配置
    NVIC_InitStructure.NVIC_IRQChannel = USART3_IRQn;
    NVIC_InitStructure.NVIC_IRQChannelPreemptionPriority = MCU_UART3_PREE_PRI;
    NVIC_InitStructure.NVIC_IRQChannelSubPriority = MCU_UART3_SUB_PRI;
    NVIC_InitStructure.NVIC_IRQChannelCmd = ENABLE;
    NVIC_Init(&NVIC_InitStructure);

    /* 配置 DMA Stream */
    //通道选择
    DMA_InitStructure.DMA_Channel = DMA_Channel_4;
    //DMA外设地址
    DMA_InitStructure.DMA_PeripheralBaseAddr = (uint32_t)&USART3->DR;
    //DMA 存储器0地址
    DMA_InitStructure.DMA_Memory0BaseAddr = (uint32_t)(blockBufferWithUart3DMA_Send);
    //存储器到外设模式
    DMA_InitStructure.DMA_DIR = DMA_DIR_MemoryToPeripheral;
    //数据传输量
    DMA_InitStructure.DMA_BufferSize = LENGTH_UART3_BUFFER_WITH_BLOCK;
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
    DMA_Init(DMA1_Stream3, &DMA_InitStructure);

    //清除DMA中断
    DMA_ClearFlag(DMA1_Stream3, DMA_FLAG_TCIF4);

    NVIC_InitStructure.NVIC_IRQChannel = DMA1_Stream3_IRQn;
    NVIC_InitStructure.NVIC_IRQChannelPreemptionPriority = MCU_DMA1_STREAM3_PREE_PRI;
    NVIC_InitStructure.NVIC_IRQChannelSubPriority = MCU_DMA1_STREAM3_SUB_PRI;
    NVIC_InitStructure.NVIC_IRQChannelCmd = ENABLE;
    NVIC_Init(&NVIC_InitStructure);
    //打开DMA中断
    DMA_ITConfig(DMA1_Stream3, DMA_IT_TC, ENABLE);

    //创建一个互斥信号量,用于DMA串口发送
	OSMutexCreate((OS_MUTEX*	)&mutexMCU_Uart3,
				  (CPU_CHAR*	)"mutexMCU_Uart3",
                  (OS_ERR*		)&err);	

    uart3RecvFuncPtr = rxCallBack;
    flagUart3DMA = 0;
}

//串口发送数组
void MCU_Uart3SendBuffer(uint8_t* bufferStartPtr,uint16_t sendLength)
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
        OSMutexPend(&mutexMCU_Uart3, 0, OS_OPT_PEND_BLOCKING, 0, &err);
    }
    do
    {
        realWriteDataCount = 0;
        ADT_CircleByteQueueWriteBuffer(&circleByteQueueUart3Send,(bufferStartPtr+currentBufferHadSendLength),currentBufferWaitSendLength,
                                        &realWriteDataCount);
        currentBufferHadSendLength += realWriteDataCount;
        currentBufferWaitSendLength -= realWriteDataCount;
        if(flagUart3DMA == 0)
        {
            ADT_CircleByteQueueReadBuffer(&circleByteQueueUart3Send,blockBufferWithUart3DMA_Send,LENGTH_UART3_BUFFER_WITH_BLOCK,&realWriteDataCount);
            if(realWriteDataCount > 0)
            {
                //数据传输量
                DMA_SetCurrDataCounter(DMA1_Stream3, realWriteDataCount);
                //使能串口1的DMA发送
                USART_DMACmd(USART3, USART_DMAReq_Tx, ENABLE);
                //开启DMA传输
                DMA_Cmd(DMA1_Stream3, ENABLE);
                flagUart3DMA = 1;
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
        OSMutexPost(&mutexMCU_Uart3, OS_OPT_POST_NO_SCHED, &err);
    }
}

//串口发送字符串
void MCU_Uart3SendString(uint8_t* stringStartPtr)
{
    int stringLength = strlen((char const *)stringStartPtr);
    if (stringLength <= 0)
    {
        return;
    }
    MCU_Uart3SendBuffer(stringStartPtr, (uint16_t)stringLength);
}

//串口发送字符串,带格式化
int MCU_Uart3Printf(const char *format, ...)
{
    //转换
    va_list arg;
    int rv;
    OS_ERR err;
    if (OSRunning)
    {
        //请求互斥信号量,申请不到,任务挂起等待OS_OPT_PEND_BLOCKING
        OSMutexPend(&mutexMCU_Uart3, 0, OS_OPT_PEND_BLOCKING, 0, &err);
    }
    va_start(arg, format);
    rv = vsnprintf((char *)blockBufferWithUart3Printf, LENGTH_UART3_BUFFER_WITH_FORMAT, format, arg);
    va_end(arg);
    if ((rv > 0) && (rv <= LENGTH_UART3_BUFFER_WITH_FORMAT))
    {
        MCU_Uart3SendBuffer(blockBufferWithUart3Printf, (uint16_t)rv);
    }
    else
    {
        //转换失败
    }
    if (OSRunning)
    {
        //释放互斥信号量,禁止在释放信号量的时候出发任务调度
        OSMutexPost(&mutexMCU_Uart3, OS_OPT_POST_NO_SCHED, &err);
    }
    return rv;
}

void USART3_IRQHandler(void)
{
    uint8_t readDat;
    //中断内核感知,需要
    CPU_SR_ALLOC();
    CPU_CRITICAL_ENTER();
    //此时要禁止操作系统调度打断中断
    OSIntEnter();
    //关闭了就要打开
    CPU_CRITICAL_EXIT();

    //获取当前是否存在接收中断
    if (USART_GetITStatus(USART3, USART_IT_RXNE) != RESET)
    {
        readDat = USART_ReceiveData(USART3);
        if (uart3RecvFuncPtr != NULL)
        {
            uart3RecvFuncPtr(readDat);
        }
    }

    //退出中断,启用调度
    OSIntExit();
}  


//DMA发送中断
void DMA1_Stream3_IRQHandler()
{
    //DMA传输完成中断4
    if (DMA_GetITStatus(DMA1_Stream3, DMA_IT_TCIF3) != RESET)
    {
        uint16_t transPackLength = 0;
        //清除中断
        DMA_ClearITPendingBit(DMA1_Stream3, DMA_IT_TCIF3);
        ADT_CircleByteQueueReadBuffer(&circleByteQueueUart3Send,blockBufferWithUart3DMA_Send,LENGTH_UART3_BUFFER_WITH_BLOCK,&transPackLength);
        //缓冲区里面有数据
        if(transPackLength > 0)
        {
            //数据传输量
            DMA_SetCurrDataCounter(DMA1_Stream3, transPackLength);
            //使能串口1的DMA发送
            USART_DMACmd(USART3, USART_DMAReq_Tx, ENABLE);
            //开启DMA传输
            DMA_Cmd(DMA1_Stream3, ENABLE);
        }
        else
        {
            //关闭DMA传输
            DMA_Cmd(DMA1_Stream3, DISABLE);
            //设置DMA中断结束
            flagUart3DMA = 0;
        }
    }
}
























