#include "TaskServiceConfig.h"
#include "TSC_Inc.h"
#include "ActionImplementInc.h"
#include "TestProc_CanConfig.h"
#include "Can1DataType.h"
//任务控制块,8字节对齐
#pragma pack(8)
//任务堆栈
CPU_STK stackBufferTaskServiceNeedleWash[STK_SIZE_TASK_SERVICE_NEEDLE_WASH];
CLEAN_NEEDLE_CMD_DATA cleandata;
#pragma pack()

#define OBTAIN(a)     (a)?1:0

OS_TCB tcbTaskServiceNeedleWash;

//串口指令处理
static void TaskServiceCommandProc_NeedleWash_UartRS1(SYSTEM_CMD_UART_RS1* uartCommand);

//CAN口指令处理
static void TaskServiceCommandProc_NeedleWash_Can1(SYSTEM_CMD_CAN1* canCommand);

//网口指令处理
static void TaskServiceCommandProc_NeedleWash_NetW5500(SYSTEM_CMD_NET_W5500* netCommand);

//自身指令处理
static void TaskServiceCommandProc_NeedleWash_Self(SYSTEM_CMD_SELF* selfCommand);

//任务函数
void TaskServiceFuncNeedleWash(void *p_arg)
{
    p_arg = p_arg;
    //任务接收指令
    SYSTEM_CMD *taskSystemRecvCommandPtr = NULL;
    //任务接收数据长度
    OS_MSG_SIZE size;
    //操作系统错误代码
    OS_ERR err;
    //开始主线程
    while(1)
    {
        //任务从队列中读取指令
        taskSystemRecvCommandPtr = OSTaskQPend((OS_TICK)0,
                                               (OS_OPT)OS_OPT_PEND_BLOCKING,
                                               (OS_MSG_SIZE *)&size,
                                               (CPU_TS *)0,
                                               (OS_ERR *)&err);
        if ((taskSystemRecvCommandPtr != NULL) && (err == OS_ERR_NONE))
        {
            //判定指令源
            if (SYSTEM_CMD_SRC_UART_RS1 == taskSystemRecvCommandPtr->commandSrc)
            {
                //串口指令源
                SYSTEM_CMD_UART_RS1* uartRS1CommandPtr = (SYSTEM_CMD_UART_RS1*)(taskSystemRecvCommandPtr->systemCommandDataPtr);
                //对接收到的数据进行数据处理
                TaskServiceCommandProc_NeedleWash_UartRS1(uartRS1CommandPtr);
                //处置完成,执行后续操作
                UserMemFree(SRAM_CCM,uartRS1CommandPtr);
            }
            else if (SYSTEM_CMD_SRC_SELF == taskSystemRecvCommandPtr->commandSrc)
            {
                //自身指令源
                SYSTEM_CMD_SELF* selfCommandPtr =  (SYSTEM_CMD_SELF*)(taskSystemRecvCommandPtr->systemCommandDataPtr);
                //对接收到的数据进行处理
                TaskServiceCommandProc_NeedleWash_Self(selfCommandPtr);
                //处置完成,执行后续操作
                UserMemFree(SRAM_CCM,selfCommandPtr);
            }
            else if(SYSTEM_CMD_SRC_NET_W5500 == taskSystemRecvCommandPtr->commandSrc)
            {
                //网口指令源
                SYSTEM_CMD_NET_W5500* netW5500CommandPtr =  (SYSTEM_CMD_NET_W5500*)(taskSystemRecvCommandPtr->systemCommandDataPtr);
                //对接收到的数据进行处理
                TaskServiceCommandProc_NeedleWash_NetW5500(netW5500CommandPtr);
                //处置完成,执行后续操作
                UserMemFree(SRAM_CCM,netW5500CommandPtr);
            }
            else if(SYSTEM_CMD_SRC_CAN1 == taskSystemRecvCommandPtr->commandSrc)
            {
                //CAN1口指令源
                SYSTEM_CMD_CAN1* can1CommandPtr =  (SYSTEM_CMD_CAN1*)(taskSystemRecvCommandPtr->systemCommandDataPtr);
                //对接收到的数据进行处理
                TaskServiceCommandProc_NeedleWash_Can1(can1CommandPtr);
                //处置完成,执行后续操作
                UserMemFree(SRAM_CCM,can1CommandPtr);
            }
            else
            {
                //不支持的指令源,串口报警
                TaskServiceSerialErrMsgShow(LH_ERR_MAIN_CMD_SRC_UNSUPPORT);
            }
            //处理完成,释放指令内存
            taskSystemRecvCommandPtr->systemCommandDataPtr = NULL;
            UserMemFree(SRAM_CCM,taskSystemRecvCommandPtr);
            taskSystemRecvCommandPtr = NULL;
        }
    }
}


/*****************************************************串口指令处理*****************************************************/
//清洗针 复位
static LH_ERR TaskServiceCommandProc_NeedleWash_UartRS1_Reset(SYSTEM_CMD_UART_RS1* uartCommandPtr,uint16_t* resultData);
//清洗针 灌注
static LH_ERR TaskServiceCommandProc_NeedleWash_UartRS1_Prime(SYSTEM_CMD_UART_RS1* uartCommandPtr,uint16_t* resultData);
//清洗针 泵初始化
static LH_ERR TaskServiceCommandProc_NeedleWash_UartRS1_PumpInit(SYSTEM_CMD_UART_RS1* uartCommandPtr,uint16_t* resultData);
//清洗针 清洗分离一次
static LH_ERR TaskServiceCommandProc_NeedleWash_UartRS1_Clean(SYSTEM_CMD_UART_RS1* uartCommandPtr,uint16_t* resultData);

//定义函数数组,用于串口处理线程回调
static const TaskServiceProcPtrWithUartCommand taskServiceProcNeedleWashPtrWithUartCommandArray[] = {
	TaskServiceCommandProc_NeedleWash_UartRS1_Reset,
	TaskServiceCommandProc_NeedleWash_UartRS1_Prime,
	TaskServiceCommandProc_NeedleWash_UartRS1_PumpInit,
	TaskServiceCommandProc_NeedleWash_UartRS1_Clean,
};
//清洗针指令回调函数数量
#define NEEDLE_WASH_FUNC_UART_COUNT_MAX      (sizeof(taskServiceProcNeedleWashPtrWithUartCommandArray)/sizeof(taskServiceProcNeedleWashPtrWithUartCommandArray[0]))


//串口指令的第一级处理
static void TaskServiceCommandProc_NeedleWash_UartRS1(SYSTEM_CMD_UART_RS1* uartCommand)
{
    LH_ERR errorCode = LH_ERR_NONE;
	//发送ACK,创建一条新数据包
	SystemUartRS_ModbusCreatePackage(uartCommand->uartRecvCommandArray,uartCommand->srcDeviceAddr,uartCommand->frameNo,
						uartCommand->funcCode,uartCommand->regAddr,uartCommand->regValue);
	BoardRS_SendBuffer(ACTION_DEBUG_PORT,uartCommand->uartRecvCommandArray,12);

    //调用指令
	uint8_t actionCode= ((uartCommand->regAddr)>>8);
    uint16_t resultData = 0x0000;
	if(actionCode > NEEDLE_WASH_FUNC_UART_COUNT_MAX)
	{
		//返回程序不支持
		SystemUartRS_ModbusCreatePackage(uartCommand->uartRecvCommandArray, uartCommand->srcDeviceAddr, uartCommand->frameNo,
							0x01, 0x0100, 0x00);
		//发送一个填充好的数据包
		BoardRS_SendBuffer(ACTION_DEBUG_PORT,uartCommand->uartRecvCommandArray,12);
	}
	else
	{
		//转入子程序处理
		errorCode = taskServiceProcNeedleWashPtrWithUartCommandArray[actionCode-1](uartCommand,&resultData);
	}
	SystemUartRS_ModbusCreatePackage(uartCommand->uartRecvCommandArray, uartCommand->srcDeviceAddr, uartCommand->frameNo,
							0x01, (uint16_t)errorCode, resultData);
	//发送一个填充好的数据包
	BoardRS_SendBuffer(ACTION_DEBUG_PORT,uartCommand->uartRecvCommandArray,12);
}

//清洗针 复位
static LH_ERR TaskServiceCommandProc_NeedleWash_UartRS1_Reset(SYSTEM_CMD_UART_RS1* uartCommandPtr,uint16_t* resultData)
{
    //针与泵同时复位
	LH_ERR errorCode;

	errorCode = ActionNeedleWash_Init();
	return errorCode;
}

//清洗针 灌注
static LH_ERR TaskServiceCommandProc_NeedleWash_UartRS1_Prime(SYSTEM_CMD_UART_RS1* uartCommandPtr,uint16_t* resultData)
{
    LH_ERR errorCode;

	errorCode = ActionNeedleWash_Prime();
	return errorCode;
}

//清洗针 泵初始化
static LH_ERR TaskServiceCommandProc_NeedleWash_UartRS1_PumpInit(SYSTEM_CMD_UART_RS1* uartCommandPtr,uint16_t* resultData)
{
    LH_ERR errorCode;

	errorCode = ActionNeedleWash_Init();
	return errorCode;
}

//清洗针 清洗分离一次
static LH_ERR TaskServiceCommandProc_NeedleWash_UartRS1_Clean(SYSTEM_CMD_UART_RS1* uartCommandPtr,uint16_t* resultData)
{
    LH_ERR errorCode;
//	CLEAN_NEEDLE_CMD_DATA paramLow = (CLEAN_NEEDLE_CMD_DATA)(uartCommandPtr->regValue);

//	errorCode = ActionNeedleWash_Clean(paramLow);
	return errorCode;
}



/********************************************************************************************************************/

/*****************************************************CAN指令处理*****************************************************/
//清洗针 复位
static LH_ERR TaskServiceCommandProc_NeedleWash_Can1_Reset(SYSTEM_CMD_CAN1* canCommand)
{
    //针与泵同时复位
	LH_ERR errorCode;
	uint8_t count;
	errorCode = ActionNeedleWash_Init();
	count = Arraypack_Common(canCommand,0,errorCode);
	CanSendDataToCell(TESTPROC_CELL_PLATFORM_NMBR,BOARD_ID_TRACK_COMM,0x0018,canCommand->can1RecvDataBuffer,count);
	return errorCode;
}
//清洗针 灌注
static LH_ERR TaskServiceCommandProc_NeedleWash_Can1_Prime(SYSTEM_CMD_CAN1* canCommand)
{
    LH_ERR errorCode;
	uint8_t count;
	errorCode = ActionNeedleWash_Prime();
	count = Arraypack_Common(canCommand,0,errorCode);
	CanSendDataToCell(TESTPROC_CELL_PLATFORM_NMBR,BOARD_ID_TRACK_COMM,0x0018,canCommand->can1RecvDataBuffer,count);
	return errorCode;
}
//清洗针 清洗分离一次
static LH_ERR TaskServiceCommandProc_NeedleWash_Can1_Clean(SYSTEM_CMD_CAN1* canCommand)
{
    LH_ERR errorCode;
	uint8_t count;

	
	cleandata.needle.able =OBTAIN(canCommand->can1RecvDataBuffer[0]>>4);//1号针
	cleandata.needle.index =(CLEAN_NEEDLE_INDEX)(canCommand->can1RecvDataBuffer[0]&0x0f);
	cleandata.needle1.able=OBTAIN(canCommand->can1RecvDataBuffer[1]>>4);//2号针
	cleandata.needle1.index=(CLEAN_NEEDLE_INDEX)(canCommand->can1RecvDataBuffer[1]&0x0f);
	cleandata.needle2.able=OBTAIN(canCommand->can1RecvDataBuffer[2]>>4);//3号针
	cleandata.needle2.index=(CLEAN_NEEDLE_INDEX)(canCommand->can1RecvDataBuffer[2]&0x0f);

	errorCode = ActionNeedleWash_Clean(cleandata);
	count = Arraypack_Common(canCommand,0,errorCode);
	CanSendDataToCell(TESTPROC_CELL_PLATFORM_NMBR,BOARD_ID_TRACK_COMM,0x0018,canCommand->can1RecvDataBuffer,count);
	return errorCode;
}
static const APP_TASK_CAN1_PROC_UNIT taskServiceProcPtrWithNeedleWashCan1CommandArray[] = {
    {CAN1_COMMAND_NEEDLEWASH_PUMP_RESET    ,        TaskServiceCommandProc_NeedleWash_Can1_Reset},//清洗针与泵复位
    {CAN1_COMMAND_NEEDLEWASH_PRIME         ,        TaskServiceCommandProc_NeedleWash_Can1_Prime},//清洗针灌注
    {CAN1_COMMAND_NEEDLEWASH_CLEAN         ,        TaskServiceCommandProc_NeedleWash_Can1_Clean},//清洗针磁珠清洗并返回
};
#define NEEDLE_WASH_FUNC_CAN_COUNT_MAX      (sizeof(taskServiceProcPtrWithNeedleWashCan1CommandArray)/sizeof(taskServiceProcPtrWithNeedleWashCan1CommandArray[0]))

static void TaskServiceCommandProc_NeedleWash_Can1(SYSTEM_CMD_CAN1* canCommand)
{
	uint16_t indexUtil = 0;
    for(indexUtil = 0; indexUtil < NEEDLE_WASH_FUNC_CAN_COUNT_MAX; indexUtil++)
    {
        //指令匹配
        if(canCommand->command == taskServiceProcPtrWithNeedleWashCan1CommandArray[indexUtil].command)
        {
            //调用处理函数
            if(taskServiceProcPtrWithNeedleWashCan1CommandArray[indexUtil].can1CommandProcFunc != NULL)
            {
                taskServiceProcPtrWithNeedleWashCan1CommandArray[indexUtil].can1CommandProcFunc(canCommand);
                break;
            }
        }
    }
}

/********************************************************************************************************************/

/*******************************************************网口指令处理**************************************************/

static void TaskServiceCommandProc_NeedleWash_NetW5500(SYSTEM_CMD_NET_W5500* netCommand)
{
	
}

/********************************************************************************************************************/

/*******************************************************自身指令处理**************************************************/
//清洗针初始化
static LH_ERR TaskServiceCommandProc_NeedleWash_Self_Init(SYSTEM_CMD_SELF* selfCommandPtr);
//清洗针灌注一次
static LH_ERR TaskServiceCommandProc_NeedleWash_Self_PrimeOnce(SYSTEM_CMD_SELF* selfCommandPtr);
//清洗针进行一次清洗分离
static LH_ERR TaskServiceCommandProc_NeedleWash_Self_CleanOnce(SYSTEM_CMD_SELF* selfCommandPtr);

//自身指令回调函数数组
static const TaskServicePtrWithSelfCommand TaskProcWithNeedleWashSelfCommandArray[] = {
	TaskServiceCommandProc_NeedleWash_Self_Init,
	TaskServiceCommandProc_NeedleWash_Self_PrimeOnce,
	TaskServiceCommandProc_NeedleWash_Self_CleanOnce,
};

//支持指令的个数
#define TASK_COMMAND_PROC_SELF_COUNT	(sizeof(TaskProcWithNeedleWashSelfCommandArray)/sizeof(TaskProcWithNeedleWashSelfCommandArray[0]))

//用于调试指令的头调试字符串
static const uint8_t TaskSelfCommandDebugStartMsgHeadString[] = "$$Action 11 Start : ";
static const uint8_t TaskSelfCommandDebugEndMsgHeadString[] = "$$Action 11 End ";


static void TaskServiceCommandProc_NeedleWash_Self(SYSTEM_CMD_SELF* selfCommand)
{
    OS_ERR err;
	LH_ERR actionResult;
	//指令过程与结果对象
	TSC_CMD_RESULT_DATA_NEEDLE_WASH* cmdResultPtr = (TSC_CMD_RESULT_DATA_NEEDLE_WASH*)selfCommand->commandDataSpecialChannel;
	
	//进入响应流程,申请互斥信号量
	OSMutexPend (cmdResultPtr->dataMutexPtr,0,OS_OPT_PEND_BLOCKING,0,&err);
	//设置状态,开始处理,等待返回
	cmdResultPtr->status = TSC_CMD_STATE_WAIT_RETURN;

	//检查序号,不能大于最大支持
	if(selfCommand->commandIndex >= TASK_COMMAND_PROC_SELF_COUNT)
	{
		TaskServiceSerialDebugMsgShowStart((uint8_t*)TaskSelfCommandDebugStartMsgHeadString,(uint8_t*)"Unsupport",
				selfCommand->commandParam1,selfCommand->commandParam2,selfCommand->commandParam3,
				selfCommand->commandParam4,selfCommand->commandParam5
				);
		//不支持的指令
		actionResult = LH_ERR_ACTION_CHANNEL_CMD_NOT_SUPPORT;
	}
	else
	{
		//回调函数
		actionResult = TaskProcWithNeedleWashSelfCommandArray[selfCommand->commandIndex](selfCommand);
	}
	
	//打印结束处理信息
	TaskServiceSerialDebugMsgShowEnd((uint8_t*)TaskSelfCommandDebugEndMsgHeadString,actionResult);

	//处理完成,设置状态
	cmdResultPtr->status = TSC_CMD_STATE_COMPLETE;
	cmdResultPtr->resultValue = actionResult;
	//释放信号量
	OSMutexPost(cmdResultPtr->dataMutexPtr,OS_OPT_POST_NONE,&err);
    return;
}

//清洗针初始化
static LH_ERR TaskServiceCommandProc_NeedleWash_Self_Init(SYSTEM_CMD_SELF* selfCommandPtr)
{
    LH_ERR errCode;
	//打印开始处理信息
	TaskServiceSerialDebugMsgShowStart((uint8_t*)TaskSelfCommandDebugStartMsgHeadString,
										(uint8_t*)"NeedleWashInit",
										selfCommandPtr->commandParam1,
										selfCommandPtr->commandParam2,
										selfCommandPtr->commandParam3,
										selfCommandPtr->commandParam4,
										selfCommandPtr->commandParam5);

	//程序的运行时处理
#if(TASK_SERVICE_WITH_REAL_MACHINE == 0)
	//延时流程响应
	CoreDelayMs(TASK_SERVICE_DELAY_TIME_MS);
	errCode = LH_ERR_NONE;
#else
	//处理
	errCode = ActionNeedleWash_Init();
#endif

	return errCode;
}
//清洗针灌注一次
static LH_ERR TaskServiceCommandProc_NeedleWash_Self_PrimeOnce(SYSTEM_CMD_SELF* selfCommandPtr)
{
    LH_ERR errCode;
	//打印开始处理信息
	TaskServiceSerialDebugMsgShowStart((uint8_t*)TaskSelfCommandDebugStartMsgHeadString,
										(uint8_t*)"NeedleWashPrimeOnce",
										selfCommandPtr->commandParam1,
										selfCommandPtr->commandParam2,
										selfCommandPtr->commandParam3,
										selfCommandPtr->commandParam4,
										selfCommandPtr->commandParam5);

	//程序的运行时处理
#if(TASK_SERVICE_WITH_REAL_MACHINE == 0)
	//延时流程响应
	CoreDelayMs(TASK_SERVICE_DELAY_TIME_MS);
	errCode = LH_ERR_NONE;
#else
	//处理
	errCode = ActionNeedleWash_Prime();
#endif

	return errCode;
}

//清洗针进行一次清洗分离
static LH_ERR TaskServiceCommandProc_NeedleWash_Self_CleanOnce(SYSTEM_CMD_SELF* selfCommandPtr)
{
    LH_ERR errCode;
	//打印开始处理信息
	TaskServiceSerialDebugMsgShowStart((uint8_t*)TaskSelfCommandDebugStartMsgHeadString,
										(uint8_t*)"NeedleWashCleanOnce",
										selfCommandPtr->commandParam1,
										selfCommandPtr->commandParam2,
										selfCommandPtr->commandParam3,
										selfCommandPtr->commandParam4,
										selfCommandPtr->commandParam5);

	//程序的运行时处理
#if(TASK_SERVICE_WITH_REAL_MACHINE == 0)
	//延时流程响应
	CoreDelayMs(TASK_SERVICE_DELAY_TIME_MS);
	errCode = LH_ERR_NONE;
#else
	
	CLEAN_NEEDLE_CMD_DATA washFlag;
	TSC_NEEDLE_WASH_SELECT_FLAG select_flag = (TSC_NEEDLE_WASH_SELECT_FLAG)selfCommandPtr->commandParam1;
	CLEAN_NEEDLE_INDEX needle_index  = (CLEAN_NEEDLE_INDEX)selfCommandPtr->commandParam2;

	if(select_flag&0x01)washFlag.needle.able = ENABLE;
	else washFlag.needle.able = DISABLE;

	if(select_flag&0x02)washFlag.needle1.able = ENABLE;
	else washFlag.needle1.able = DISABLE;

	if(select_flag&0x04)washFlag.needle2.able = ENABLE;
	else washFlag.needle2.able = DISABLE;

	washFlag.needle.index = needle_index;
	washFlag.needle1.index = needle_index;
	washFlag.needle2.index = needle_index;
	//处理
	errCode = ActionNeedleWash_Clean(washFlag);
#endif

	return errCode;
}

/********************************************************************************************************************/





