#include "MCU_Can.h"
#include "MCU_Port.h"
#include "SystemConfig.h"
#include "TestProc_boardID.h"
#include "Can2ProcConfig.h"

#define CELL_BOARD_ID                          (BOARD_ID_CELL_MAIN)  // 主控板板号
#define CELL_BOARD_COMM_ID                     (BOARD_ID_BOARDCAST)  // 公共板号

//CAN模块初始化配置信息
typedef struct MCU_CAN_BPS_SETTING
{
    uint8_t tqWithSJW;
    uint8_t tqWithBS1;
    uint8_t tqWithBS2;
    uint16_t clkDiv;
} MCU_CAN_BPS_SETTING;

//CAN总线在不同的速率下的配置
//tsjw:重新同步跳跃时间单元.范围:CAN_SJW_1tq~ CAN_SJW_4tq
//tbs2:时间段2的时间单元.   范围:CAN_BS2_1tq~CAN_BS2_8tq;
//tbs1:时间段1的时间单元.   范围:CAN_BS1_1tq ~CAN_BS1_16tq
//brp :波特率分频器.范围:1~1024; tq=(brp)*tpclk1
//波特率=Fpclk1/((tbs1+1+tbs2+1+1)*brp);
//Fpclk1的时钟在初始化的时候设置为42M,如果设置CAN1_Mode_Init(CAN_SJW_1tq,CAN_BS2_6tq,CAN_BS1_7tq,6,CAN_Mode_LoopBack);
//则波特率为:42M/((6+7+1)*6)=500Kbps

//根据CIA官方建议,(SJW+BS1)/(SJW+BS1+BS2)
// 速度 > 800K以上时候,采样点 75%  
// 速度 > 500K,采样点 80%
// 速度 <= 500K,采样点 87.5%
const MCU_CAN_BPS_SETTING MCU_CAN_BaudArray[] = {
    {CAN_SJW_1tq, CAN_BS1_11tq, CAN_BS2_2tq, 3},  //1000K 85%
    {CAN_SJW_1tq, CAN_BS1_11tq, CAN_BS2_2tq, 6},  //500K  85%
    {CAN_SJW_1tq, CAN_BS1_11tq, CAN_BS2_2tq, 12}, //250K  85%
    {CAN_SJW_1tq, CAN_BS1_11tq, CAN_BS2_2tq, 24}, //125K  85%
};

//CAN1
static MCU_CAN_RecvIntProcFuncPtr can1RecvProc = NULL;
//CAN1接收缓冲区
static MCU_CAN_RECEIVE_BUFFER can1ReceiveBuffer;
//互斥信号量,外界对于CAN总线调用的独占
static OS_MUTEX mutexMCU_CAN1;
//定义一个信号量,用于保证发送数据无延时
static OS_SEM semMCU_CAN1;		


//CAN2
static MCU_CAN_RecvIntProcFuncPtr can2RecvProc = NULL;
//CAN2接收缓冲区
static MCU_CAN_RECEIVE_BUFFER can2ReceiveBuffer;
//互斥信号量,外界对于CAN总线调用的独占
static OS_MUTEX mutexMCU_CAN2;
//定义一个信号量,用于保证发送数据无延时
static OS_SEM semMCU_CAN2;	

//同时初始化CAN1与CAN2,CAN2与下位机通讯,带有主板ID
void MCU_CAN_Init(MCU_CAN_BAUD baud1, MCU_CAN_BAUD baud2, 
                        MCU_CAN_RecvIntProcFuncPtr callBackCan1, 
                        MCU_CAN_RecvIntProcFuncPtr callBackCan2)
{
    OS_ERR err;
    //CAN初始化配置
	CAN_InitTypeDef        CAN_InitStructure;
	//过滤器配置
	CAN_FilterInitTypeDef  CAN_FilterInitStructure;
	//中断配置
	NVIC_InitTypeDef  NVIC_InitStructure;

    const uint8_t u8CellNumber = SystemConfig_GetCellNumber();

    //初始化接收缓存器
    for(uint16_t index = 0; index < MCU_CAN_RECEIVE_BUFFER_LENGTH; index++)
    {
        can1ReceiveBuffer.receiveBufferArray[index].waitProcessFlag = 0;
        can2ReceiveBuffer.receiveBufferArray[index].waitProcessFlag = 0;
    }
    //接收序号变成0
    can1ReceiveBuffer.currentReceiveIndex = 0;
    can2ReceiveBuffer.currentReceiveIndex = 0;

    //创建一个互斥信号量,用于can总线发送
	OSMutexCreate((OS_MUTEX*	)&mutexMCU_CAN1,
				  (CPU_CHAR*	)"mutexMCU_CAN1",
                  (OS_ERR*		)&err);
    OSMutexCreate((OS_MUTEX*	)&mutexMCU_CAN2,
				  (CPU_CHAR*	)"mutexMCU_CAN2",
                  (OS_ERR*		)&err);

    //创建一个信号量
	OSSemCreate ((OS_SEM*	)&semMCU_CAN1,
                 (CPU_CHAR*	)"semMCU_CAN1",
                 (OS_SEM_CTR)3,		
                 (OS_ERR*	)&err);
    OSSemCreate ((OS_SEM*	)&semMCU_CAN2,
                 (CPU_CHAR*	)"semMCU_CAN2",
                 (OS_SEM_CTR)3,		
                 (OS_ERR*	)&err);

    //使能CAN1 2 时钟
	RCC_APB1PeriphClockCmd(RCC_APB1Periph_CAN1, ENABLE);
    RCC_APB1PeriphClockCmd(RCC_APB1Periph_CAN2, ENABLE);

    //复位CAN1 2 然后启动CAN1 2
    RCC_APB1PeriphResetCmd(RCC_APB1Periph_CAN1,ENABLE);
    RCC_APB1PeriphResetCmd(RCC_APB1Periph_CAN2,ENABLE);
    //结束复位
    RCC_APB1PeriphResetCmd(RCC_APB1Periph_CAN1,DISABLE);
    RCC_APB1PeriphResetCmd(RCC_APB1Periph_CAN2,DISABLE);

    //RX CAN1
    MCU_PortInit(MCU_PIN_A_11, GPIO_Mode_AF, GPIO_OType_PP, GPIO_PuPd_UP, GPIO_Speed_100MHz);
    //TX
    MCU_PortInit(MCU_PIN_A_12, GPIO_Mode_AF, GPIO_OType_PP, GPIO_PuPd_UP, GPIO_Speed_100MHz);
    //电压监控
    MCU_PortInit(MCU_PIN_B_7, GPIO_Mode_IN, GPIO_OType_PP, GPIO_PuPd_UP, GPIO_Speed_100MHz);

    //引脚复用映射配置 GPIOA11复用为CAN1
    GPIO_PinAFConfig(GPIOA, GPIO_PinSource11, GPIO_AF_CAN1);
    //GPIOA12复用为CAN1
    GPIO_PinAFConfig(GPIOA, GPIO_PinSource12, GPIO_AF_CAN1);

    //RX CAN2
    MCU_PortInit(MCU_PIN_B_5, GPIO_Mode_AF, GPIO_OType_PP, GPIO_PuPd_UP, GPIO_Speed_100MHz);
    //TX
    MCU_PortInit(MCU_PIN_B_6, GPIO_Mode_AF, GPIO_OType_PP, GPIO_PuPd_UP, GPIO_Speed_100MHz);
    //电压监控
    MCU_PortInit(MCU_PIN_B_8, GPIO_Mode_IN, GPIO_OType_PP, GPIO_PuPd_UP, GPIO_Speed_100MHz);

    //复用CAN2
    GPIO_PinAFConfig(GPIOB, GPIO_PinSource5, GPIO_AF_CAN2);
    GPIO_PinAFConfig(GPIOB, GPIO_PinSource6, GPIO_AF_CAN2);

    //CAN单元设置 非时间触发通信模式
    CAN_InitStructure.CAN_TTCM = DISABLE;
    //软件自动离线管理
    CAN_InitStructure.CAN_ABOM = ENABLE;
    //睡眠模式通过软件唤醒(清除CAN->MCR的SLEEP位)
    CAN_InitStructure.CAN_AWUM = DISABLE;
    //报文自动传送
    CAN_InitStructure.CAN_NART = DISABLE;
    //报文锁定
	CAN_InitStructure.CAN_RFLM=ENABLE;
    //优先级由写入次序决定
    CAN_InitStructure.CAN_TXFP = ENABLE;
    //模式设置 正常收发
    CAN_InitStructure.CAN_Mode = CAN_Mode_Normal;

    //重新同步跳跃宽度(Tsjw)为tsjw+1个时间单位 CAN_SJW_1tq~CAN_SJW_4tq
    CAN_InitStructure.CAN_SJW = MCU_CAN_BaudArray[baud1].tqWithSJW;
    //Tbs1范围CAN_BS1_1tq ~CAN_BS1_16tq
    CAN_InitStructure.CAN_BS1 = MCU_CAN_BaudArray[baud1].tqWithBS1;
    //Tbs2范围CAN_BS2_1tq ~	CAN_BS2_8tq
    CAN_InitStructure.CAN_BS2 = MCU_CAN_BaudArray[baud1].tqWithBS2;
    //分频系数(Fdiv)为brp+1
    CAN_InitStructure.CAN_Prescaler = MCU_CAN_BaudArray[baud1].clkDiv;

    // 初始化CAN1
    CAN_Init(CAN1, &CAN_InitStructure);

    //初始化CAN2
    CAN_InitStructure.CAN_Mode = CAN_Mode_Normal;
    //重新同步跳跃宽度(Tsjw)为tsjw+1个时间单位 CAN_SJW_1tq~CAN_SJW_4tq
    CAN_InitStructure.CAN_SJW = MCU_CAN_BaudArray[baud2].tqWithSJW;
    //Tbs1范围CAN_BS1_1tq ~CAN_BS1_16tq
    CAN_InitStructure.CAN_BS1 = MCU_CAN_BaudArray[baud2].tqWithBS1;
    //Tbs2范围CAN_BS2_1tq ~	CAN_BS2_8tq
    CAN_InitStructure.CAN_BS2 = MCU_CAN_BaudArray[baud2].tqWithBS2;
    //分频系数(Fdiv)为brp+1
    CAN_InitStructure.CAN_Prescaler = MCU_CAN_BaudArray[baud2].clkDiv;
    // 初始化CAN2
    CAN_Init(CAN2, &CAN_InitStructure);

    /***********************CAN1中位机过滤器配置**************************/
    //配置过滤器 过滤器0 过滤master flag
    CAN_FilterInitStructure.CAN_FilterNumber = 0;
    //指定掩码过滤
    CAN_FilterInitStructure.CAN_FilterMode = CAN_FilterMode_IdMask;
    //32位
    CAN_FilterInitStructure.CAN_FilterScale = CAN_FilterScale_32bit;
    //32位ID 高16位
    CAN_FilterInitStructure.CAN_FilterIdHigh = (((uint32_t)((u8CellNumber<<23)|(CELL_BOARD_ID<<11))<<3)&0xFFFF0000)>>16;
    //低16位
    CAN_FilterInitStructure.CAN_FilterIdLow = (((uint32_t)((u8CellNumber<<23)|(CELL_BOARD_ID<<11))<<3)|CAN_ID_EXT|CAN_RTR_DATA)&0xFFFF;
    //32位MASK 高16位
    CAN_FilterInitStructure.CAN_FilterMaskIdHigh = (((uint32_t)((0x07<<23)|(0x3F<<11))<<3)&0xFFFF0000)>>16;
    //低16位
    CAN_FilterInitStructure.CAN_FilterMaskIdLow = (((uint32_t)((0x07<<23)|(0x3F<<11))<<3)|CAN_ID_EXT|CAN_RTR_DATA)&0xFFFF;
    //过滤器0关联到FIFO0
    CAN_FilterInitStructure.CAN_FilterFIFOAssignment = CAN_Filter_FIFO0;
    //激活过滤器0
    CAN_FilterInitStructure.CAN_FilterActivation = ENABLE;
    //过滤器初始化
    CAN_FilterInit(&CAN_FilterInitStructure);

    //配置过滤器 过滤器0 过滤master flag
    CAN_FilterInitStructure.CAN_FilterNumber = 1;
    //指定掩码过滤
    CAN_FilterInitStructure.CAN_FilterMode = CAN_FilterMode_IdMask;
    //32位
    CAN_FilterInitStructure.CAN_FilterScale = CAN_FilterScale_32bit;
    //32位ID 高16位
    CAN_FilterInitStructure.CAN_FilterIdHigh = (((uint32_t)((u8CellNumber<<23)|(CELL_BOARD_COMM_ID<<11))<<3)&0xFFFF0000)>>16;
    //低16位
    CAN_FilterInitStructure.CAN_FilterIdLow = (((uint32_t)((u8CellNumber<<23)|(CELL_BOARD_COMM_ID<<11))<<3)|CAN_ID_EXT|CAN_RTR_DATA)&0xFFFF;
    //32位MASK 高16位
    CAN_FilterInitStructure.CAN_FilterMaskIdHigh = (((uint32_t)((0x07<<23)|(0x3F<<11))<<3)&0xFFFF0000)>>16;
    //低16位
    CAN_FilterInitStructure.CAN_FilterMaskIdLow = (((uint32_t)((0x07<<23)|(0x3F<<11))<<3)|CAN_ID_EXT|CAN_RTR_DATA)&0xFFFF;
    //过滤器0关联到FIFO0
    CAN_FilterInitStructure.CAN_FilterFIFOAssignment = CAN_Filter_FIFO0;
    //激活过滤器0
    CAN_FilterInitStructure.CAN_FilterActivation = ENABLE;
    //过滤器初始化
    CAN_FilterInit(&CAN_FilterInitStructure);

    //FIFO0消息挂号中断允许.满中断允许 溢出中断允许 CAN1
    CAN_ITConfig(CAN1, CAN_IT_FMP0|CAN_IT_FF0|CAN_IT_FOV0, ENABLE);
    //CAN中断发送完成中断允许
    CAN_ITConfig(CAN1,CAN_IT_TME,ENABLE);

    //CAN2的过滤器    
    CAN_FilterInitStructure.CAN_FilterNumber = 14;
    //指定掩码过滤
    CAN_FilterInitStructure.CAN_FilterMode = CAN_FilterMode_IdMask;
    //32位
    CAN_FilterInitStructure.CAN_FilterScale = CAN_FilterScale_32bit;
    //32位ID 高16位
    CAN_FilterInitStructure.CAN_FilterIdHigh = 0x0000;
    //低16位
    CAN_FilterInitStructure.CAN_FilterIdLow = ((DEVICE_MAIN_BOARD_ID<<3)|CAN_ID_EXT|CAN_RTR_DATA)&0xFFFF;
    //32位MASK 高16位
    CAN_FilterInitStructure.CAN_FilterMaskIdHigh = 0x0000;
    //低16位
    CAN_FilterInitStructure.CAN_FilterMaskIdLow = (0x003F<<3)|(0x0006);
    //过滤器0关联到FIFO0
    CAN_FilterInitStructure.CAN_FilterFIFOAssignment = CAN_Filter_FIFO0;
    //激活过滤器0
    CAN_FilterInitStructure.CAN_FilterActivation = ENABLE;
    //过滤器初始化
    CAN_FilterInit(&CAN_FilterInitStructure);

    //FIFO0消息挂号中断允许.满中断允许 溢出中断允许  CAN2
    CAN_ITConfig(CAN2, CAN_IT_FMP0|CAN_IT_FF0|CAN_IT_FOV0, ENABLE);
    //CAN中断发送完成中断允许
    CAN_ITConfig(CAN2,CAN_IT_TME,ENABLE);

    //can1接收中断,初始化的时候不开接收中断,直到系统正常初始化完成才开接收中断
	NVIC_InitStructure.NVIC_IRQChannel = CAN1_RX0_IRQn;
	// 主优先级
	NVIC_InitStructure.NVIC_IRQChannelPreemptionPriority = CAN1_RX_INT_PREE_PRI;
	// 次优先级
	NVIC_InitStructure.NVIC_IRQChannelSubPriority = CAN1_RX_INT_SUB_PRI;
	NVIC_InitStructure.NVIC_IRQChannelCmd = ENABLE;
	NVIC_Init(&NVIC_InitStructure);

    //can1发送中断,初始化的时候不开接收中断,直到系统正常初始化完成才开接收中断
	NVIC_InitStructure.NVIC_IRQChannel = CAN1_TX_IRQn;
	// 主优先级
	NVIC_InitStructure.NVIC_IRQChannelPreemptionPriority = CAN1_TX_INT_PREE_PRI;
	// 次优先级
	NVIC_InitStructure.NVIC_IRQChannelSubPriority = CAN1_TX_INT_SUB_PRI;
	NVIC_InitStructure.NVIC_IRQChannelCmd = ENABLE;
	NVIC_Init(&NVIC_InitStructure);

    //can2接收中断,初始化的时候不开接收中断,直到系统正常初始化完成才开接收中断
	NVIC_InitStructure.NVIC_IRQChannel = CAN2_RX0_IRQn;
	// 主优先级
	NVIC_InitStructure.NVIC_IRQChannelPreemptionPriority = CAN2_RX_INT_PREE_PRI;
	// 次优先级
	NVIC_InitStructure.NVIC_IRQChannelSubPriority = CAN2_RX_INT_SUB_PRI;
	NVIC_InitStructure.NVIC_IRQChannelCmd = ENABLE;
	NVIC_Init(&NVIC_InitStructure);

    //can2发送中断,初始化的时候不开接收中断,直到系统正常初始化完成才开接收中断
	NVIC_InitStructure.NVIC_IRQChannel = CAN2_TX_IRQn;
	// 主优先级
	NVIC_InitStructure.NVIC_IRQChannelPreemptionPriority = CAN2_TX_INT_PREE_PRI;
	// 次优先级
	NVIC_InitStructure.NVIC_IRQChannelSubPriority = CAN2_TX_INT_SUB_PRI;
	NVIC_InitStructure.NVIC_IRQChannelCmd = ENABLE;
	NVIC_Init(&NVIC_InitStructure);
    
    //关联接收处理函数
    can1RecvProc = callBackCan1;
    can2RecvProc = callBackCan2;
}

//CAN1总线传输讯息
void MCU_CAN1_TransMessage(CanTxMsg* txMsg)
{
    OS_ERR err;
    if(OSRunning)
    {
        //先申请互斥信号量,申请不到,任务挂起等待OS_OPT_PEND_BLOCKING
		OSMutexPend (&mutexMCU_CAN1,0,OS_OPT_PEND_BLOCKING,0,&err);
        //请求信号量
        OSSemPend(&semMCU_CAN1,0,OS_OPT_PEND_BLOCKING,0,&err); 	
    }
    //中间进行操作
    //发送这一帧,因为是有信号量调度的,所以不用担心发不出去
	CAN_Transmit(CAN1, txMsg);
    if(OSRunning)
    {
        //释放互斥信号量
		OSMutexPost(&mutexMCU_CAN1,OS_OPT_POST_NO_SCHED,&err);
    }
}

//CAN2总线传输讯息
void MCU_CAN2_TransMessage(CanTxMsg* txMsg)
{
    OS_ERR err;
    if(OSRunning)
    {
        //先申请互斥信号量,申请不到,任务挂起等待OS_OPT_PEND_BLOCKING
		OSMutexPend (&mutexMCU_CAN2,0,OS_OPT_PEND_BLOCKING,0,&err);
        //请求信号量
        OSSemPend(&semMCU_CAN2,0,OS_OPT_PEND_BLOCKING,0,&err); 	
    }
    //中间进行操作
    //发送这一帧,因为是有信号量调度的,所以不用担心发不出去
	CAN_Transmit(CAN2, txMsg);
    if(OSRunning)
    {
        //释放互斥信号量
		OSMutexPost(&mutexMCU_CAN2,OS_OPT_POST_NO_SCHED,&err);
    }
}

//获取电压检测器状态
BitAction MCU_CAN1_GetSensorStatus(void)
{
    return MCU_PortReadSingle(MCU_PIN_B_7);
}

//获取电压检测器状态
BitAction MCU_CAN2_GetSensorStatus(void)
{
    return MCU_PortReadSingle(MCU_PIN_B_8);
}

//CAN1发送中断处理

//CAN1中断代码,在中断中检测当前信号量的数量,并决定是否进行发送
void CAN1_TX_IRQHandler()
{
    OS_ERR err;
    //此时要禁止操作系统调度打断中断
	CPU_SR_ALLOC();

	//进入临界段
    CPU_CRITICAL_ENTER();
	/* Tell uC/OS-III that we are starting an ISR           */
    OSIntEnter();   
	//退出临界段                                            
    CPU_CRITICAL_EXIT();

    if(SET == CAN_GetFlagStatus(CAN1,CAN_FLAG_RQCP0))
    {
        CAN_ClearFlag(CAN1,CAN_FLAG_RQCP0);
    }

    if(SET == CAN_GetFlagStatus(CAN1,CAN_FLAG_RQCP1))
    {
        CAN_ClearFlag(CAN1,CAN_FLAG_RQCP1);
    }

    if(SET == CAN_GetFlagStatus(CAN1,CAN_FLAG_RQCP2))
    {
        CAN_ClearFlag(CAN1,CAN_FLAG_RQCP2);
    }

    //发送信号量
    OSSemPost(&semMCU_CAN1,OS_OPT_POST_FIFO,&err);				
            
    //退出中断,启用调度
	OSIntExit();
}

//CAN2发送中断处理
void CAN2_TX_IRQHandler()
{
    OS_ERR err;
    //此时要禁止操作系统调度打断中断
	CPU_SR_ALLOC();

	//进入临界段
    CPU_CRITICAL_ENTER();
	/* Tell uC/OS-III that we are starting an ISR           */
    OSIntEnter();   
	//退出临界段                                            
    CPU_CRITICAL_EXIT();

    if(SET == CAN_GetFlagStatus(CAN2,CAN_FLAG_RQCP0))
    {
        CAN_ClearFlag(CAN2,CAN_FLAG_RQCP0);
    }

    if(SET == CAN_GetFlagStatus(CAN2,CAN_FLAG_RQCP1))
    {
        CAN_ClearFlag(CAN2,CAN_FLAG_RQCP1);
    }

    if(SET == CAN_GetFlagStatus(CAN2,CAN_FLAG_RQCP2))
    {
        CAN_ClearFlag(CAN2,CAN_FLAG_RQCP2);
    }

    //发送信号量
    OSSemPost(&semMCU_CAN2,OS_OPT_POST_FIFO,&err);				
            
    //退出中断,启用调度
	OSIntExit();
}

//灯光显示通讯状态
#include "BoardBeep.h"
#include "BoardLed.h"

//数据满的时候的应急处理
static CanRxMsg rxMsgCan1WhenFull;
static CanRxMsg rxMsgCan2WhenFull;

static void Can1ReceiveReadOnce(void)
{
    //读第一次
    if(0 != CAN_MessagePending(CAN1,CAN_FIFO0))
    {
        //首先检查要读取的数据,是否上一次的接收数据处理完成,如果没有完成,那么本次数据就不能处理,应急读一下,然后抛弃
        if(1 == (can1ReceiveBuffer.receiveBufferArray[can1ReceiveBuffer.currentReceiveIndex].waitProcessFlag))
        {
            //数据满了
            CAN_Receive(CAN1, CAN_FIFO0, &rxMsgCan1WhenFull);
            //蜂鸣器叫
            BoardBeepSetState(BEEP_ON);
            //黄灯亮
            BoardLedWrite(BOARD_LED_YELLOW,BOARD_LED_LIGHT);
        }
        else
        {
            //没满,正常读取
            CAN_Receive(CAN1, CAN_FIFO0, &(can1ReceiveBuffer.receiveBufferArray[can1ReceiveBuffer.currentReceiveIndex].rxMsg));
            //把这一帧数据传递到中断回调函数处理
            if(can1RecvProc != NULL)
            {
                //读取完成,设置变量
                can1ReceiveBuffer.receiveBufferArray[can1ReceiveBuffer.currentReceiveIndex].waitProcessFlag = 1;
                can1RecvProc(&(can1ReceiveBuffer.receiveBufferArray[can1ReceiveBuffer.currentReceiveIndex]));
                //处理完成,对当前队列索引进行处理
                (can1ReceiveBuffer.currentReceiveIndex)++;
                //如果到头了,就要成环
                if(can1ReceiveBuffer.currentReceiveIndex >= MCU_CAN_RECEIVE_BUFFER_LENGTH)
                {
                    can1ReceiveBuffer.currentReceiveIndex = 0;
                }
            }
            
        }
    }
}

static void Can2ReceiveReadOnce(void)
{
    //读第一次
    if(0 != CAN_MessagePending(CAN2,CAN_FIFO0))
    {
        //首先检查要读取的数据,是否上一次的接收数据处理完成,如果没有完成,那么本次数据就不能处理,应急读一下,然后抛弃
        if(1 == (can2ReceiveBuffer.receiveBufferArray[can2ReceiveBuffer.currentReceiveIndex].waitProcessFlag))
        {
            //数据满了
            CAN_Receive(CAN2, CAN_FIFO0, &rxMsgCan2WhenFull);
            //蜂鸣器叫
            BoardBeepSetState(BEEP_ON);
            //红灯亮
            BoardLedWrite(BOARD_LED_RED,BOARD_LED_LIGHT);
        }
        else
        {
            //没满,正常读取
            CAN_Receive(CAN2, CAN_FIFO0, &(can2ReceiveBuffer.receiveBufferArray[can2ReceiveBuffer.currentReceiveIndex].rxMsg));
            //把这一帧数据传递到中断回调函数处理
            if(can2RecvProc != NULL)
            {
                //读取完成,设置变量
                can2ReceiveBuffer.receiveBufferArray[can2ReceiveBuffer.currentReceiveIndex].waitProcessFlag = 1;
                can2RecvProc(&(can2ReceiveBuffer.receiveBufferArray[can2ReceiveBuffer.currentReceiveIndex]));
                //处理完成,对当前队列索引进行处理
                (can2ReceiveBuffer.currentReceiveIndex)++;
                //如果到头了,就要成环
                if(can2ReceiveBuffer.currentReceiveIndex >= MCU_CAN_RECEIVE_BUFFER_LENGTH)
                {
                    can2ReceiveBuffer.currentReceiveIndex = 0;
                }
            }
            
        }
    }
}

//CAN1中断服务函数
void CAN1_RX0_IRQHandler(void)
{
    //此时要禁止操作系统调度打断中断
	CPU_SR_ALLOC();

	//进入临界段
    CPU_CRITICAL_ENTER();
	/* Tell uC/OS-III that we are starting an ISR           */
    OSIntEnter();   
	//退出临界段                                            
    CPU_CRITICAL_EXIT();

    //清除满中断
    if(SET == CAN_GetITStatus(CAN1,CAN_IT_FF0))
    {
        //满中断
        CAN_ClearITPendingBit(CAN1,CAN_IT_FF0);
        //黄灯亮
        BoardLedWrite(BOARD_LED_YELLOW,BOARD_LED_LIGHT);
    }

    //溢出中断
    if(SET == CAN_GetITStatus(CAN1,CAN_IT_FOV0))
    {
        //溢出中断
        CAN_ClearITPendingBit(CAN1,CAN_IT_FOV0);
        //黄灯亮
        BoardLedWrite(BOARD_LED_YELLOW,BOARD_LED_LIGHT);
    }
    //因为FIFO有三级,所以读三次,一次性解决
    Can1ReceiveReadOnce();
    Can1ReceiveReadOnce();
    Can1ReceiveReadOnce();
    //退出中断,启用调度
	OSIntExit();
}

//CAN2中断服务函数
void CAN2_RX0_IRQHandler(void)
{
    //此时要禁止操作系统调度打断中断
	CPU_SR_ALLOC();

	//进入临界段
    CPU_CRITICAL_ENTER();
	/* Tell uC/OS-III that we are starting an ISR           */
    OSIntEnter();   
	//退出临界段                                            
    CPU_CRITICAL_EXIT();

    //清除满中断
    if(SET == CAN_GetITStatus(CAN2,CAN_IT_FF0))
    {
        //满中断
        CAN_ClearITPendingBit(CAN2,CAN_IT_FF0);
        //黄灯亮
        BoardLedWrite(BOARD_LED_YELLOW,BOARD_LED_LIGHT);
    }

    //溢出中断
    if(SET == CAN_GetITStatus(CAN2,CAN_IT_FOV0))
    {
        //溢出中断
        CAN_ClearITPendingBit(CAN2,CAN_IT_FOV0);
        //黄灯亮
        BoardLedWrite(BOARD_LED_YELLOW,BOARD_LED_LIGHT);
    }
    //因为FIFO有三级,所以读三次,一次性解决
    Can2ReceiveReadOnce();
    Can2ReceiveReadOnce();
    Can2ReceiveReadOnce();
    //退出中断,启用调度
	OSIntExit();
}

