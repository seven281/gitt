#include "TaskServiceConfig.h"
#include "TSC_Inc.h"
#include "ActionImplementInc.h"
#include "TestProc_CanConfig.h"
#include "Can1DataType.h"
//任务控制块,8字节对齐
#pragma pack(8)
//任务堆栈
CPU_STK stackBufferTaskServiceRankPathWay[STK_SIZE_TASK_SERVICE_RANK_PATH_WAY];
#pragma pack()

OS_TCB tcbTaskServiceRankPathWay;

//串口指令处理
static void TaskServiceCommandProc_RankPathWay_UartRS1(SYSTEM_CMD_UART_RS1* uartCommand);

//CAN口指令处理
static void TaskServiceCommandProc_RankPathWay_Can1(SYSTEM_CMD_CAN1* canCommand);

//网口指令处理
static void TaskServiceCommandProc_RankPathWay_NetW5500(SYSTEM_CMD_NET_W5500* netCommand);

//自身指令处理
static void TaskServiceCommandProc_RankPathWay_Self(SYSTEM_CMD_SELF* selfCommand);

//任务函数
void TaskServiceFuncRankPathWay(void *p_arg)
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
                TaskServiceCommandProc_RankPathWay_UartRS1(uartRS1CommandPtr);
                //处置完成,执行后续操作
                UserMemFree(SRAM_CCM,uartRS1CommandPtr);
            }
            else if (SYSTEM_CMD_SRC_SELF == taskSystemRecvCommandPtr->commandSrc)
            {
                //自身指令源
                SYSTEM_CMD_SELF* selfCommandPtr =  (SYSTEM_CMD_SELF*)(taskSystemRecvCommandPtr->systemCommandDataPtr);
                //对接收到的数据进行处理
                TaskServiceCommandProc_RankPathWay_Self(selfCommandPtr);
                //处置完成,执行后续操作
                UserMemFree(SRAM_CCM,selfCommandPtr);
            }
            else if(SYSTEM_CMD_SRC_NET_W5500 == taskSystemRecvCommandPtr->commandSrc)
            {
                //网口指令源
                SYSTEM_CMD_NET_W5500* netW5500CommandPtr =  (SYSTEM_CMD_NET_W5500*)(taskSystemRecvCommandPtr->systemCommandDataPtr);
                //对接收到的数据进行处理
                TaskServiceCommandProc_RankPathWay_NetW5500(netW5500CommandPtr);
                //处置完成,执行后续操作
                UserMemFree(SRAM_CCM,netW5500CommandPtr);
            }
            else if(SYSTEM_CMD_SRC_CAN1 == taskSystemRecvCommandPtr->commandSrc)
            {
                //CAN1口指令源
                SYSTEM_CMD_CAN1* can1CommandPtr =  (SYSTEM_CMD_CAN1*)(taskSystemRecvCommandPtr->systemCommandDataPtr);
                //对接收到的数据进行处理
                TaskServiceCommandProc_RankPathWay_Can1(can1CommandPtr);
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
//试管架传送带 复位
static LH_ERR TaskServiceCommandProc_RankPathWay_UartRS1_Reset(SYSTEM_CMD_UART_RS1* uartCommandPtr,uint16_t* resultData);
//试管架传送带 移动
static LH_ERR TaskServiceCommandProc_RankPathWay_UartRS1_Move(SYSTEM_CMD_UART_RS1* uartCommandPtr,uint16_t* resultData);
//试管架传送带 回退
static LH_ERR TaskServiceCommandProc_RankPathWay_UartRS1_Back(SYSTEM_CMD_UART_RS1* uartCommandPtr,uint16_t* resultData);

//定义函数数组,用于串口处理线程回调
static const TaskServiceProcPtrWithUartCommand taskServiceProcRankPathWayPtrWithUartCommandArray[] = {
	TaskServiceCommandProc_RankPathWay_UartRS1_Reset,
	TaskServiceCommandProc_RankPathWay_UartRS1_Move,
	TaskServiceCommandProc_RankPathWay_UartRS1_Back,
};
#define RANK_PATH_WAY_FUNC_UART_COUNT_MAX      (sizeof(taskServiceProcRankPathWayPtrWithUartCommandArray)/sizeof(taskServiceProcRankPathWayPtrWithUartCommandArray[0]))


//串口指令的第一级处理
static void TaskServiceCommandProc_RankPathWay_UartRS1(SYSTEM_CMD_UART_RS1* uartCommand)
{
    LH_ERR errorCode = LH_ERR_NONE;
	//发送ACK,创建一条新数据包
	SystemUartRS_ModbusCreatePackage(uartCommand->uartRecvCommandArray,uartCommand->srcDeviceAddr,uartCommand->frameNo,
						uartCommand->funcCode,uartCommand->regAddr,uartCommand->regValue);
	BoardRS_SendBuffer(ACTION_DEBUG_PORT,uartCommand->uartRecvCommandArray,12);

    //调用指令
	uint8_t actionCode= ((uartCommand->regAddr)>>8);
    uint16_t resultData = 0x0000;
	if(actionCode > RANK_PATH_WAY_FUNC_UART_COUNT_MAX)
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
		errorCode = taskServiceProcRankPathWayPtrWithUartCommandArray[actionCode-1](uartCommand,&resultData);
	}
	SystemUartRS_ModbusCreatePackage(uartCommand->uartRecvCommandArray, uartCommand->srcDeviceAddr, uartCommand->frameNo,
							0x01, (uint16_t)errorCode, resultData);
	//发送一个填充好的数据包
	BoardRS_SendBuffer(ACTION_DEBUG_PORT,uartCommand->uartRecvCommandArray,12);
}

//试管架传送带 复位
static LH_ERR TaskServiceCommandProc_RankPathWay_UartRS1_Reset(SYSTEM_CMD_UART_RS1* uartCommandPtr,uint16_t* resultData)
{
    LH_ERR errorCode;
	uint8_t currentTubeIndex;
	errorCode = ActionRankPathWay_Init(&currentTubeIndex);
	return errorCode;
}

//试管架传送带 移动
static LH_ERR TaskServiceCommandProc_RankPathWay_UartRS1_Move(SYSTEM_CMD_UART_RS1* uartCommandPtr,uint16_t* resultData)
{
    uint8_t paramLow = (uint8_t)(uartCommandPtr->regValue);
	LH_ERR errorCode;

	uint8_t currentTubeIndex;

	errorCode = ActionRankPathWay_Move2SpecialTube(paramLow,&currentTubeIndex);
	return errorCode;
}

//试管架传送带 回退
static LH_ERR TaskServiceCommandProc_RankPathWay_UartRS1_Back(SYSTEM_CMD_UART_RS1* uartCommandPtr,uint16_t* resultData)
{
    LH_ERR errorCode;
	uint8_t currentTubeIndex;

	errorCode = ActionRankPathWay_BackOneRack(&currentTubeIndex);
	return errorCode;
}


/********************************************************************************************************************/

/*****************************************************CAN指令处理*****************************************************/
//试管架传送带 复位
static LH_ERR TaskServiceCommandProc_RankPathWay_Can1_Reset(SYSTEM_CMD_CAN1* canCommand)
{
    LH_ERR errorCode;
	uint8_t currentTubeIndex,count;
	errorCode = ActionRankPathWay_Init(&currentTubeIndex);
	count = Arraypack_Common(canCommand,0,errorCode);
	CanSendDataToCell(TESTPROC_CELL_PLATFORM_NMBR,BOARD_ID_TRACK_COMM,0x0018,canCommand->can1RecvDataBuffer,count);
	return errorCode;
}
//试管架传送带 移动
static LH_ERR TaskServiceCommandProc_RankPathWay_Can1_Move(SYSTEM_CMD_CAN1* canCommand)
{
    uint8_t paramLow = canCommand->can1RecvDataBuffer[0];
	LH_ERR errorCode;
	uint8_t currentTubeIndex,count;
	errorCode = ActionRankPathWay_Move2SpecialTube(paramLow,&currentTubeIndex);
	count = Arraypack_Common(canCommand,paramLow,errorCode);
	CanSendDataToCell(TESTPROC_CELL_PLATFORM_NMBR,BOARD_ID_TRACK_COMM,0x0018,canCommand->can1RecvDataBuffer,count);
	return errorCode;
}
//试管架传送带 回退
static LH_ERR TaskServiceCommandProc_RankPathWay_Can1_Back(SYSTEM_CMD_CAN1* canCommand)
{
    LH_ERR errorCode;
	uint8_t currentTubeIndex,count;
	errorCode = ActionRankPathWay_BackOneRack(&currentTubeIndex);
	count = Arraypack_Common(canCommand,0,errorCode);
	CanSendDataToCell(TESTPROC_CELL_PLATFORM_NMBR,BOARD_ID_TRACK_COMM,0x0018,canCommand->can1RecvDataBuffer,count);
	return errorCode;
}


static const APP_TASK_CAN1_PROC_UNIT taskServiceProcPtrWithRankPathwayCan1CommandArray[] = {
    {CAN1_COMMAND_SAMPLE_CONVEYOR_RESET              ,        TaskServiceCommandProc_RankPathWay_Can1_Reset},//加样传送带初始化
    {CAN1_COMMAND_SAMPLE_CONVEYOR_MOVE_TO_SAMPLE     ,        TaskServiceCommandProc_RankPathWay_Can1_Move},//移到吸样位
    {CAN1_COMMAND_SAMPLE_CONVEYOR_RETURN_SAMPLE      ,        TaskServiceCommandProc_RankPathWay_Can1_Back},//退回样本架
};
#define RANK_PATH_WAY_FUNC_CAN_COUNT_MAX      (sizeof(taskServiceProcPtrWithRankPathwayCan1CommandArray)/sizeof(taskServiceProcPtrWithRankPathwayCan1CommandArray[0]))

static void TaskServiceCommandProc_RankPathWay_Can1(SYSTEM_CMD_CAN1* canCommand)
{
	uint16_t indexUtil = 0;
    for(indexUtil = 0; indexUtil < RANK_PATH_WAY_FUNC_CAN_COUNT_MAX; indexUtil++)
    {
        //指令匹配
        if(canCommand->command == taskServiceProcPtrWithRankPathwayCan1CommandArray[indexUtil].command)
        {
            //调用处理函数
            if(taskServiceProcPtrWithRankPathwayCan1CommandArray[indexUtil].can1CommandProcFunc != NULL)
            {
                taskServiceProcPtrWithRankPathwayCan1CommandArray[indexUtil].can1CommandProcFunc(canCommand);
                break;
            }
        }
    }
}

/********************************************************************************************************************/

/*******************************************************网口指令处理**************************************************/

static void TaskServiceCommandProc_RankPathWay_NetW5500(SYSTEM_CMD_NET_W5500* netCommand)
{

}

/********************************************************************************************************************/

/*******************************************************自身指令处理**************************************************/
//试管架传送带初始化
static LH_ERR TaskServiceCommandProc_RankPathWay_Self_Init(SYSTEM_CMD_SELF* selfCommandPtr);
//试管架传送带移动到指定试管
static LH_ERR TaskServiceCommandProc_RankPathWay_Self_Move2SpecialTube(SYSTEM_CMD_SELF* selfCommandPtr);
//试管架传送带回退一个试管架
static LH_ERR TaskServiceCommandProc_RankPathWay_Self_BackOneRack(SYSTEM_CMD_SELF* selfCommandPtr);

//自身指令回调函数数组
static const TaskServicePtrWithSelfCommand TaskProcWithRankPathWaySelfCommandArray[] = {
	TaskServiceCommandProc_RankPathWay_Self_Init,
	TaskServiceCommandProc_RankPathWay_Self_Move2SpecialTube,
	TaskServiceCommandProc_RankPathWay_Self_BackOneRack,
};

//支持指令的个数
#define TASK_COMMAND_PROC_SELF_COUNT	(sizeof(TaskProcWithRankPathWaySelfCommandArray)/sizeof(TaskProcWithRankPathWaySelfCommandArray[0]))

//用于调试指令的头调试字符串
static const uint8_t TaskSelfCommandDebugStartMsgHeadString[] = "$$Action 13 Start : ";
static const uint8_t TaskSelfCommandDebugEndMsgHeadString[] = "$$Action 13 End ";

static void TaskServiceCommandProc_RankPathWay_Self(SYSTEM_CMD_SELF* selfCommand)
{
    OS_ERR err;
	LH_ERR actionResult;
	//指令过程与结果对象
	TSC_CMD_RESULT_DATA_RACK_PATH_WAY* cmdResultPtr = (TSC_CMD_RESULT_DATA_RACK_PATH_WAY*)selfCommand->commandDataSpecialChannel;
	
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
		actionResult = TaskProcWithRankPathWaySelfCommandArray[selfCommand->commandIndex](selfCommand);
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

//试管架传送带初始化
static LH_ERR TaskServiceCommandProc_RankPathWay_Self_Init(SYSTEM_CMD_SELF* selfCommandPtr)
{
    LH_ERR errCode;
	//打印开始处理信息
	TaskServiceSerialDebugMsgShowStart((uint8_t*)TaskSelfCommandDebugStartMsgHeadString,
										(uint8_t*)"RankPathWayInit",
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
	
	uint8_t currentTubeIndex;

	//指令过程与结果对象
	TSC_CMD_RESULT_DATA_RACK_PATH_WAY* cmdResultPtr = 
				(TSC_CMD_RESULT_DATA_RACK_PATH_WAY*)selfCommandPtr->commandDataSpecialChannel;
	//处理
	errCode = ActionRankPathWay_Init(&currentTubeIndex);
	if(errCode == LH_ERR_NONE)
	{
		cmdResultPtr->currentTubeIndex = currentTubeIndex;
	}
#endif

	return errCode;
}

//试管架传送带移动到指定试管
static LH_ERR TaskServiceCommandProc_RankPathWay_Self_Move2SpecialTube(SYSTEM_CMD_SELF* selfCommandPtr)
{
    LH_ERR errCode;
	//打印开始处理信息
	TaskServiceSerialDebugMsgShowStart((uint8_t*)TaskSelfCommandDebugStartMsgHeadString,
										(uint8_t*)"RankPathWayMove2SpecialTube",
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
	
	uint8_t currentTubeIndex;

	uint16_t targetTubeIndex = selfCommandPtr->commandParam1;
	//指令过程与结果对象
	TSC_CMD_RESULT_DATA_RACK_PATH_WAY* cmdResultPtr = 
				(TSC_CMD_RESULT_DATA_RACK_PATH_WAY*)selfCommandPtr->commandDataSpecialChannel;

	//处理
	errCode = ActionRankPathWay_Move2SpecialTube(targetTubeIndex,&currentTubeIndex);
	if(errCode == LH_ERR_NONE)
	{
		cmdResultPtr->currentTubeIndex = currentTubeIndex;
	}
#endif

	return errCode;
}

//试管架传送带回退一个试管架
static LH_ERR TaskServiceCommandProc_RankPathWay_Self_BackOneRack(SYSTEM_CMD_SELF* selfCommandPtr)
{
    LH_ERR errCode;
	//打印开始处理信息
	TaskServiceSerialDebugMsgShowStart((uint8_t*)TaskSelfCommandDebugStartMsgHeadString,
										(uint8_t*)"RankPathWayBackOneRack",
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
	
	uint8_t currentTubeIndex;

	//指令过程与结果对象
	TSC_CMD_RESULT_DATA_RACK_PATH_WAY* cmdResultPtr = 
				(TSC_CMD_RESULT_DATA_RACK_PATH_WAY*)selfCommandPtr->commandDataSpecialChannel;
	//处理
	errCode = ActionRankPathWay_BackOneRack(&currentTubeIndex);
	if(errCode == LH_ERR_NONE)
	{
		cmdResultPtr->currentTubeIndex = currentTubeIndex;
	}
#endif

	return errCode;
}


/********************************************************************************************************************/



