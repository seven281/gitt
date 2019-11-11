#include "MCU_Uart6.h"
#include "ADT_CircleByteQueue.h"
#include "MCU_Port.h"


//串口中断接收数据处理函数
static MCU_UartRecvIntProcFunc uart6RecvFuncPtr = NULL;

//串口互斥信号量
static OS_MUTEX mutexMCU_Uart6;

//循环队列
static ADT_CIRCLE_BYTE_QUEUE circleByteQueueUart6Send = {0};

//用于循环队列存储数据的数组
static uint8_t bufferWithUart6Send[LENGTH_UART6_BUFFER_WITH_SEND] = {0};

//用于printf的缓冲区
static uint8_t blockBufferWithUart6Printf[LENGTH_UART6_BUFFER_WITH_FORMAT] = {0};

//串口中断发送正在运行标志
static uint8_t flagUart6TC = 0;

//串口初始化
void MCU_Uart6Init(uint32_t baud, MCU_UART_LENGTH length, MCU_UART_STOPBIT stopBit,
                      MCU_UART_CHECK_MODE checkMode, MCU_UART_HARD_CONTROL hardWareControl, MCU_UartRecvIntProcFunc rxCallBack)
{
    USART_InitTypeDef USART_InitStructure;
    NVIC_InitTypeDef NVIC_InitStructure;
    OS_ERR err;

    ADT_CircleByteQueueInit(&circleByteQueueUart6Send, bufferWithUart6Send, LENGTH_UART6_BUFFER_WITH_SEND);

    //使能USART时钟
    RCC_APB2PeriphClockCmd(RCC_APB2Periph_USART6,ENABLE);

     //复位串口
    RCC_APB2PeriphResetCmd(RCC_APB2Periph_USART6,ENABLE);

    //取消复位
    RCC_APB2PeriphResetCmd(RCC_APB2Periph_USART6,DISABLE);

    //复位串口
    USART_DeInit(USART6);  

    //USART端口配置
    MCU_PortInit(MCU_PIN_C_6, GPIO_Mode_AF, GPIO_OType_PP, GPIO_PuPd_UP, GPIO_Speed_100MHz);
    MCU_PortInit(MCU_PIN_C_7, GPIO_Mode_AF, GPIO_OType_PP, GPIO_PuPd_UP, GPIO_Speed_100MHz);

    //UART2复用
    GPIO_PinAFConfig(GPIOC,GPIO_PinSource6,GPIO_AF_USART6); 
	GPIO_PinAFConfig(GPIOC,GPIO_PinSource7,GPIO_AF_USART6); 

    USART_InitStructure.USART_BaudRate = baud;
    USART_InitStructure.USART_WordLength = length;
    USART_InitStructure.USART_StopBits = stopBit;
    USART_InitStructure.USART_Parity = checkMode;
    USART_InitStructure.USART_HardwareFlowControl = hardWareControl;
    //发送接收同时打开
    USART_InitStructure.USART_Mode = USART_Mode_Rx | USART_Mode_Tx;
    USART_Init(USART6, &USART_InitStructure);
    //使能串口
    USART_Cmd(USART6, ENABLE);

    //STM32在使能串口的时候会发送一帧空闲帧作为起始,所以要等待第一帧发送完成
    USART_ClearFlag(USART6, USART_FLAG_TC);

    while (USART_GetFlagStatus(USART6, USART_FLAG_TC) == RESET)
    {
        asm("nop"); //等待空闲帧发送完成后  再清零发送标志
    }
    USART_ClearFlag(USART6, USART_FLAG_TC);

    //开启相关中断
    USART_ITConfig(USART6, USART_IT_RXNE, ENABLE);
    USART_ITConfig(USART6, USART_IT_TC, ENABLE);

    //Usart NVIC 配置
    NVIC_InitStructure.NVIC_IRQChannel = USART6_IRQn;
    NVIC_InitStructure.NVIC_IRQChannelPreemptionPriority = MCU_UART6_PREE_PRI;
    NVIC_InitStructure.NVIC_IRQChannelSubPriority = MCU_UART6_SUB_PRI;
    NVIC_InitStructure.NVIC_IRQChannelCmd = ENABLE;
    NVIC_Init(&NVIC_InitStructure);

    //创建一个互斥信号量,用于DMA串口发送
	OSMutexCreate((OS_MUTEX*	)&mutexMCU_Uart6,
				  (CPU_CHAR*	)"mutexMCU_Uart6",
                  (OS_ERR*		)&err);	

    uart6RecvFuncPtr = rxCallBack;
    flagUart6TC = 0;
}

//串口发送数组
void MCU_Uart6SendBuffer(uint8_t* bufferStartPtr,uint16_t sendLength)
{
    uint16_t currentBufferHadSendLength = 0;
    uint16_t currentBufferWaitSendLength = sendLength;
    uint16_t realWriteDataCount = 0;
    OS_ERR err;
    uint8_t maybeNeedSendByte = 0;
    if(sendLength == 0)
    {
        return;
    }
    if (OSRunning)
    {
        //请求互斥信号量,申请不到,任务挂起等待OS_OPT_PEND_BLOCKING
        OSMutexPend(&mutexMCU_Uart6, 0, OS_OPT_PEND_BLOCKING, 0, &err);
    }
    do
    {
        realWriteDataCount = 0;
        ADT_CircleByteQueueWriteBuffer(&circleByteQueueUart6Send,(bufferStartPtr+currentBufferHadSendLength),currentBufferWaitSendLength,
                                        &realWriteDataCount);
        currentBufferHadSendLength += realWriteDataCount;
        currentBufferWaitSendLength -= realWriteDataCount;
        if(flagUart6TC == 0)
        {
            ADT_CircleByteQueueReadBuffer(&circleByteQueueUart6Send,&maybeNeedSendByte,1,&realWriteDataCount);
            if(realWriteDataCount > 0)
            {
                USART_SendData(USART6,maybeNeedSendByte);
                flagUart6TC = 1;
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
        OSMutexPost(&mutexMCU_Uart6, OS_OPT_POST_NO_SCHED, &err);
    }
}

//串口发送字符串
void MCU_Uart6SendString(uint8_t* stringStartPtr)
{
    int stringLength = strlen((char const *)stringStartPtr);
    if (stringLength <= 0)
    {
        return;
    }
    MCU_Uart6SendBuffer(stringStartPtr, (uint16_t)stringLength);
}

//串口发送字符串,带格式化
int MCU_Uart6Printf(const char *format, ...)
{
    //转换
    va_list arg;
    int rv;
    OS_ERR err;
    if (OSRunning)
    {
        //请求互斥信号量,申请不到,任务挂起等待OS_OPT_PEND_BLOCKING
        OSMutexPend(&mutexMCU_Uart6, 0, OS_OPT_PEND_BLOCKING, 0, &err);
    }
    va_start(arg, format);
    rv = vsnprintf((char *)blockBufferWithUart6Printf, LENGTH_UART6_BUFFER_WITH_FORMAT, format, arg);
    va_end(arg);
    if ((rv > 0) && (rv <= LENGTH_UART6_BUFFER_WITH_FORMAT))
    {
        MCU_Uart6SendBuffer(blockBufferWithUart6Printf, (uint16_t)rv);
    }
    else
    {
        //转换失败
    }
    if (OSRunning)
    {
        //释放互斥信号量,禁止在释放信号量的时候出发任务调度
        OSMutexPost(&mutexMCU_Uart6, OS_OPT_POST_NO_SCHED, &err);
    }
    return rv;
}


void USART6_IRQHandler(void)
{
    uint8_t intData;
    uint8_t readDat;
    uint16_t realReadDataCount;

    //中断内核感知,需要
    CPU_SR_ALLOC();
    CPU_CRITICAL_ENTER();
    //此时要禁止操作系统调度打断中断
    OSIntEnter();
    //关闭了就要打开
    CPU_CRITICAL_EXIT();

    //获取当前是否存在接收中断
    if (USART_GetITStatus(USART6, USART_IT_RXNE) != RESET)
    {
        intData = USART_ReceiveData(USART6);
        if (uart6RecvFuncPtr != NULL)
        {
            uart6RecvFuncPtr(intData);
        }
    }
    //发送完成中断
    if(USART_GetITStatus(USART6, USART_IT_TC) != RESET)
    {
        USART_ClearFlag(USART6, USART_FLAG_TC);
        ADT_CircleByteQueueReadBuffer(&circleByteQueueUart6Send,&readDat,1,&realReadDataCount);
        if(realReadDataCount > 0)
        {
            USART_SendData(USART6,readDat);
        }
        else
        {
            flagUart6TC = 0;
        }
    }

    //退出中断,启用调度
    OSIntExit();
}  
