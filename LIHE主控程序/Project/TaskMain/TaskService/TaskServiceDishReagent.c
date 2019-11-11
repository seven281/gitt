#include "TaskServiceConfig.h"
#include "TSC_Inc.h"
#include "ActionImplementInc.h"
#include "TestProc_CanConfig.h"
#include "Can1DataType.h"

//任务控制块,8字节对齐
#pragma pack(8)
//任务堆栈
CPU_STK stackBufferTaskServiceDishReagent[STK_SIZE_TASK_SERVICE_DISH_REAGENT];
#pragma pack()

OS_TCB tcbTaskServiceDishReagent;

//串口指令处理
static void TaskServiceCommandProc_DishReagent_UartRS1(SYSTEM_CMD_UART_RS1* uartCommand);

//CAN口指令处理
static void TaskServiceCommandProc_DishReagent_Can1(SYSTEM_CMD_CAN1* canCommand);

//网口指令处理
static void TaskServiceCommandProc_DishReagent_NetW5500(SYSTEM_CMD_NET_W5500* netCommand);

//自身指令处理
static void TaskServiceCommandProc_DishReagent_Self(SYSTEM_CMD_SELF* selfCommand);

//任务函数
void TaskServiceFuncDishReagent(void *p_arg)
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
                TaskServiceCommandProc_DishReagent_UartRS1(uartRS1CommandPtr);
                //处置完成,执行后续操作
                UserMemFree(SRAM_CCM,uartRS1CommandPtr);
            }
            else if (SYSTEM_CMD_SRC_SELF == taskSystemRecvCommandPtr->commandSrc)
            {
                //自身指令源
                SYSTEM_CMD_SELF* selfCommandPtr =  (SYSTEM_CMD_SELF*)(taskSystemRecvCommandPtr->systemCommandDataPtr);
                //对接收到的数据进行处理
                TaskServiceCommandProc_DishReagent_Self(selfCommandPtr);
                //处置完成,执行后续操作
                UserMemFree(SRAM_CCM,selfCommandPtr);
            }
            else if(SYSTEM_CMD_SRC_NET_W5500 == taskSystemRecvCommandPtr->commandSrc)
            {
                //网口指令源
                SYSTEM_CMD_NET_W5500* netW5500CommandPtr =  (SYSTEM_CMD_NET_W5500*)(taskSystemRecvCommandPtr->systemCommandDataPtr);
                //对接收到的数据进行处理
                TaskServiceCommandProc_DishReagent_NetW5500(netW5500CommandPtr);
                //处置完成,执行后续操作
                UserMemFree(SRAM_CCM,netW5500CommandPtr);
            }
            else if(SYSTEM_CMD_SRC_CAN1 == taskSystemRecvCommandPtr->commandSrc)
            {
                //CAN1口指令源
                SYSTEM_CMD_CAN1* can1CommandPtr =  (SYSTEM_CMD_CAN1*)(taskSystemRecvCommandPtr->systemCommandDataPtr);
                //对接收到的数据进行处理
                TaskServiceCommandProc_DishReagent_Can1(can1CommandPtr);
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
//试剂盘复位
static LH_ERR TaskServiceCommandProc_DishReagent_UartRS1_Reset(SYSTEM_CMD_UART_RS1* uartCommandPtr,uint16_t* resultData);
//试剂盘移动到指定杯位
static LH_ERR TaskServiceCommandProc_DishReagent_UartRS1_Move2Special(SYSTEM_CMD_UART_RS1* uartCommandPtr,uint16_t* resultData);
//试剂盘移动指定个杯位
static LH_ERR TaskServiceCommandProc_DishReagent_UartRS1_MoveAny(SYSTEM_CMD_UART_RS1* uartCommandPtr,uint16_t* resultData);

//定义函数数组,用于串口处理线程回调
static const TaskServiceProcPtrWithUartCommand taskServiceProcPtrWithDishReagentUartCommandArray[] = {
	TaskServiceCommandProc_DishReagent_UartRS1_Reset,
	TaskServiceCommandProc_DishReagent_UartRS1_Move2Special,
	TaskServiceCommandProc_DishReagent_UartRS1_MoveAny,
};
//是极寒回调函数数量
#define DISH_REAGENT_FUNC_UART_COUNT_MAX      (sizeof(taskServiceProcPtrWithDishReagentUartCommandArray)/sizeof(taskServiceProcPtrWithDishReagentUartCommandArray[0]))


static void TaskServiceCommandProc_DishReagent_UartRS1(SYSTEM_CMD_UART_RS1* uartCommand)
{
    LH_ERR errorCode = LH_ERR_NONE;
	//发送ACK,创建一条新数据包
	SystemUartRS_ModbusCreatePackage(uartCommand->uartRecvCommandArray,uartCommand->srcDeviceAddr,uartCommand->frameNo,
						uartCommand->funcCode,uartCommand->regAddr,uartCommand->regValue);
	BoardRS_SendBuffer(ACTION_DEBUG_PORT,uartCommand->uartRecvCommandArray,12);

    //调用指令
	uint8_t actionCode= ((uartCommand->regAddr)>>8);
    uint16_t resultData = 0x0000;
	if(actionCode > DISH_REAGENT_FUNC_UART_COUNT_MAX)
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
		errorCode = taskServiceProcPtrWithDishReagentUartCommandArray[actionCode-1](uartCommand,&resultData);
	}
	SystemUartRS_ModbusCreatePackage(uartCommand->uartRecvCommandArray, uartCommand->srcDeviceAddr, uartCommand->frameNo,
							0x01, (uint16_t)errorCode, resultData);
	//发送一个填充好的数据包
	BoardRS_SendBuffer(ACTION_DEBUG_PORT,uartCommand->uartRecvCommandArray,12);
}


//试剂盘复位
static LH_ERR TaskServiceCommandProc_DishReagent_UartRS1_Reset(SYSTEM_CMD_UART_RS1* uartCommandPtr,uint16_t* resultData)
{
    LH_ERR errorCode;
	uint16_t currentHoleIndex;
	DISH_REAGENT_OFFSET currentOffset;

	errorCode = ActionDishReagent_Reset(&currentHoleIndex,&currentOffset);

	return errorCode;
}

//试剂盘移动到指定杯位
static LH_ERR TaskServiceCommandProc_DishReagent_UartRS1_Move2Special(SYSTEM_CMD_UART_RS1* uartCommandPtr,uint16_t* resultData)
{
    LH_ERR errorCode;
	uint16_t currentHoleIndex;
	DISH_REAGENT_OFFSET currentOffset;

	uint8_t paramLow = (uint8_t)(uartCommandPtr->regValue);
	uint8_t paramHigh = (uint8_t)((uartCommandPtr->regValue)>>8);

	//再根据高位决定偏移
    if((paramHigh&0x40) == 0x40)
    {
        //扫码位
        errorCode = ActionDishReagent_Move2SpecialHoleWithOffset(&currentHoleIndex,&currentOffset,paramLow,
																			DISH_REAGENT_OFFSET_SCAN);
    }
	else if((paramHigh&0xf0) != 0)
	{
		//相对于试剂针
        if((paramHigh&0x0f) == 0x01)
        {
            //R1
            errorCode = ActionDishReagent_Move2SpecialHoleWithOffset(&currentHoleIndex,&currentOffset,paramLow,
																			DISH_REAGENT_OFFSET_R1);
        }
        else if((paramHigh&0x0f) == 0x02)
        {
            //R2
            errorCode = ActionDishReagent_Move2SpecialHoleWithOffset(&currentHoleIndex,&currentOffset,paramLow,
																			DISH_REAGENT_OFFSET_R2);
        }
        else if((paramHigh&0x0f) == 0x03)
        {
            //磁珠
            errorCode = ActionDishReagent_Move2SpecialHoleWithOffset(&currentHoleIndex,&currentOffset,paramLow,
																			DISH_REAGENT_OFFSET_BEAD);
        }
        else
        {
            errorCode = ActionDishReagent_Move2SpecialHoleWithOffset(&currentHoleIndex,&currentOffset,paramLow,
																			DISH_REAGENT_OFFSET_NONE);
        }
	}
	return errorCode;
}

//试剂盘移动指定个杯位
static LH_ERR TaskServiceCommandProc_DishReagent_UartRS1_MoveAny(SYSTEM_CMD_UART_RS1* uartCommandPtr,uint16_t* resultData)
{
    LH_ERR errorCode;
	uint8_t paramLow = (uint8_t)(uartCommandPtr->regValue);
	uint16_t currentHoleIndex;
	DISH_REAGENT_OFFSET currentOffset;

	errorCode = ActionDishReagent_MoveAnyHoleStep(&currentHoleIndex,&currentOffset,paramLow);

	return errorCode;
}



/********************************************************************************************************************/

/*****************************************************CAN指令处理*****************************************************/

//试剂盘复位
static LH_ERR TaskServiceCommandProc_DishReagent_Can1_Reset(SYSTEM_CMD_CAN1* canCommand)
{
    LH_ERR errorCode;
	uint16_t currentHoleIndex,count;
	DISH_REAGENT_OFFSET currentOffset;
	errorCode = ActionDishReagent_Reset(&currentHoleIndex,&currentOffset);
	count = Arraypack_Common(canCommand,0,errorCode);
	CanSendDataToCell(TESTPROC_CELL_PLATFORM_NMBR,BOARD_ID_TRACK_COMM,0x0018,canCommand->can1RecvDataBuffer,count);
	return errorCode;
}
//试剂盘移动到指定杯位
static LH_ERR TaskServiceCommandProc_DishReagent_Can1_Move2Special(SYSTEM_CMD_CAN1* canCommand)
{
    LH_ERR errorCode;
	uint16_t currentHoleIndex,count;
	DISH_REAGENT_OFFSET currentOffset;

	uint8_t paramLow = canCommand->can1RecvDataBuffer[0];
	uint8_t paramHigh = canCommand->can1RecvDataBuffer[2];

    if(paramLow == 0x11)//外圈r1
        errorCode = ActionDishReagent_Move2SpecialHoleWithOffset(&currentHoleIndex,&currentOffset,paramHigh,DISH_REAGENT_OFFSET_R1);
    else if(paramLow == 0x12)//中圈r2
        errorCode = ActionDishReagent_Move2SpecialHoleWithOffset(&currentHoleIndex,&currentOffset,paramHigh,DISH_REAGENT_OFFSET_R2);
    else if(paramLow == 0x23)//内圈 磁珠
        errorCode = ActionDishReagent_Move2SpecialHoleWithOffset(&currentHoleIndex,&currentOffset,paramHigh,DISH_REAGENT_OFFSET_BEAD);
    else if(paramLow == 0x40)//扫码位
         errorCode = ActionDishReagent_Move2SpecialHoleWithOffset(&currentHoleIndex,&currentOffset,paramHigh,DISH_REAGENT_OFFSET_SCAN);
    else
        errorCode = ActionDishReagent_Move2SpecialHoleWithOffset(&currentHoleIndex,&currentOffset,paramHigh,DISH_REAGENT_OFFSET_NONE);
    
	count = Arraypack_Common(canCommand,0,errorCode);
	CanSendDataToCell(TESTPROC_CELL_PLATFORM_NMBR,BOARD_ID_TRACK_COMM,0x0018,canCommand->can1RecvDataBuffer,count);
	return errorCode;
}
//试剂盘移动指定个杯位
static LH_ERR TaskServiceCommandProc_DishReagent_Can1_MoveAny(SYSTEM_CMD_CAN1* canCommand)
{
    LH_ERR errorCode;
	uint8_t paramLow = canCommand->can1RecvDataBuffer[0];
	uint16_t currentHoleIndex,count;
	DISH_REAGENT_OFFSET currentOffset;

	errorCode = ActionDishReagent_MoveAnyHoleStep(&currentHoleIndex,&currentOffset,paramLow);
	count = Arraypack_Common(canCommand,0,errorCode);
	CanSendDataToCell(TESTPROC_CELL_PLATFORM_NMBR,BOARD_ID_TRACK_COMM,0x0018,canCommand->can1RecvDataBuffer,count);
	return errorCode;
}

static const APP_TASK_CAN1_PROC_UNIT taskServiceProcPtrWithDishReagentCan1CommandArray[] = {
    {CAN1_COMMAND_CUP_REAGENT_RESET                         ,        TaskServiceCommandProc_DishReagent_Can1_Reset},//试剂盘初始化
    {CAN1_COMMAND_CUP_REAGENT_ROTATE_TO_SPECIFIED_PS        ,        TaskServiceCommandProc_DishReagent_Can1_Move2Special},//试剂盘旋转到指定位置
    {CAN1_COMMAND_CUP_REAGENT_ROTATE_N_CUP_POS              ,        TaskServiceCommandProc_DishReagent_Can1_MoveAny},//试剂盘旋转N个杯位
};
#define DISH_REAGENT_FUNC_CAN_COUNT_MAX      (sizeof(taskServiceProcPtrWithDishReagentCan1CommandArray)/sizeof(taskServiceProcPtrWithDishReagentCan1CommandArray[0]))

static void TaskServiceCommandProc_DishReagent_Can1(SYSTEM_CMD_CAN1* canCommand)
{
	uint16_t indexUtil = 0;
    for(indexUtil = 0; indexUtil < DISH_REAGENT_FUNC_CAN_COUNT_MAX; indexUtil++)
    {
        //指令匹配
        if(canCommand->command == taskServiceProcPtrWithDishReagentCan1CommandArray[indexUtil].command)
        {
            //调用处理函数
            if(taskServiceProcPtrWithDishReagentCan1CommandArray[indexUtil].can1CommandProcFunc != NULL)
            {
                taskServiceProcPtrWithDishReagentCan1CommandArray[indexUtil].can1CommandProcFunc(canCommand);
                break;
            }
        }
    }
}

/********************************************************************************************************************/

/*******************************************************网口指令处理**************************************************/

static void TaskServiceCommandProc_DishReagent_NetW5500(SYSTEM_CMD_NET_W5500* netCommand)
{

}

/********************************************************************************************************************/

/*******************************************************自身指令处理**************************************************/
//试剂盘初始化
static LH_ERR TaskServiceCommandProc_DishReagent_Self_Init(SYSTEM_CMD_SELF* selfCommandPtr);
//试剂盘移动到指定孔位,带偏移
static LH_ERR TaskServiceCommandProc_DishReagent_Self_Move2SpecialHoleWithOffset(SYSTEM_CMD_SELF* selfCommandPtr);
//试剂盘移动指定个孔位
static LH_ERR TaskServiceCommandProc_DishReagent_Self_MoveSpecialHoleStep(SYSTEM_CMD_SELF* selfCommandPtr);

//自身指令回调函数数组
static const TaskServicePtrWithSelfCommand TaskProcWithSelfDishReagentCommandArray[] = {
	TaskServiceCommandProc_DishReagent_Self_Init,
	TaskServiceCommandProc_DishReagent_Self_Move2SpecialHoleWithOffset,
	TaskServiceCommandProc_DishReagent_Self_MoveSpecialHoleStep,
};

//支持指令的个数
#define TASK_COMMAND_PROC_SELF_COUNT	(sizeof(TaskProcWithSelfDishReagentCommandArray)/sizeof(TaskProcWithSelfDishReagentCommandArray[0]))

//用于调试指令的头调试字符串
static const uint8_t TaskSelfCommandDebugStartMsgHeadString[] = "$$Action 3 Start : ";
static const uint8_t TaskSelfCommandDebugEndMsgHeadString[] = "$$Action 3 End ";

static void TaskServiceCommandProc_DishReagent_Self(SYSTEM_CMD_SELF* selfCommand)
{
    OS_ERR err;
	LH_ERR actionResult;
	//指令过程与结果对象
	TSC_CMD_RESULT_DATA_DISH_REAGENT* cmdResultPtr = (TSC_CMD_RESULT_DATA_DISH_REAGENT*)selfCommand->commandDataSpecialChannel;
	
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
		actionResult = TaskProcWithSelfDishReagentCommandArray[selfCommand->commandIndex](selfCommand);
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

//试剂盘初始化
static LH_ERR TaskServiceCommandProc_DishReagent_Self_Init(SYSTEM_CMD_SELF* selfCommandPtr)
{
    LH_ERR errCode;
	//打印开始处理信息
	TaskServiceSerialDebugMsgShowStart((uint8_t*)TaskSelfCommandDebugStartMsgHeadString,
										(uint8_t*)"DishReagentInit",
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
	uint16_t currentHoleIndex;
	DISH_REAGENT_OFFSET currentOffset;

	//指令过程与结果对象
	TSC_CMD_RESULT_DATA_DISH_REAGENT* cmdResultPtr = 
				(TSC_CMD_RESULT_DATA_DISH_REAGENT*)selfCommandPtr->commandDataSpecialChannel;
	//处理
	errCode = ActionDishReagent_Reset(&currentHoleIndex,&currentOffset);
	//数据更新
	if(errCode == LH_ERR_NONE)
	{
		cmdResultPtr->offsetFlag = currentOffset;
		cmdResultPtr->currentHoleIndex = currentHoleIndex;
	}
#endif

	return errCode;
}

//试剂盘移动到指定孔位,带偏移
static LH_ERR TaskServiceCommandProc_DishReagent_Self_Move2SpecialHoleWithOffset(SYSTEM_CMD_SELF* selfCommandPtr)
{
    LH_ERR errCode;
	//打印开始处理信息
	TaskServiceSerialDebugMsgShowStart((uint8_t*)TaskSelfCommandDebugStartMsgHeadString,
										(uint8_t*)"DishReagentMove2SpecialHoleWithOffset",
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
	uint16_t currentHoleIndex;
	DISH_REAGENT_OFFSET currentOffset;

	//指令过程与结果对象
	TSC_CMD_RESULT_DATA_DISH_REAGENT* cmdResultPtr = 
				(TSC_CMD_RESULT_DATA_DISH_REAGENT*)selfCommandPtr->commandDataSpecialChannel;
	
	//参数转换
	uint16_t targetHoleIndex = selfCommandPtr->commandParam1;
	DISH_REAGENT_OFFSET offsetFlag = (DISH_REAGENT_OFFSET)selfCommandPtr->commandParam2;

	//处理
	errCode = ActionDishReagent_Move2SpecialHoleWithOffset(&currentHoleIndex,&currentOffset,targetHoleIndex,
						offsetFlag);
	//数据更新
	if(errCode == LH_ERR_NONE)
	{
		cmdResultPtr->offsetFlag = currentOffset;
		cmdResultPtr->currentHoleIndex = currentHoleIndex;
	}
#endif

	return errCode;
}

//试剂盘移动指定个孔位
static LH_ERR TaskServiceCommandProc_DishReagent_Self_MoveSpecialHoleStep(SYSTEM_CMD_SELF* selfCommandPtr)
{
    LH_ERR errCode;
	//打印开始处理信息
	TaskServiceSerialDebugMsgShowStart((uint8_t*)TaskSelfCommandDebugStartMsgHeadString,
										(uint8_t*)"DishReagentMoveSpecialHoleStep",
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
	uint16_t currentHoleIndex;
	DISH_REAGENT_OFFSET currentOffset;

	//指令过程与结果对象
	TSC_CMD_RESULT_DATA_DISH_REAGENT* cmdResultPtr = 
				(TSC_CMD_RESULT_DATA_DISH_REAGENT*)selfCommandPtr->commandDataSpecialChannel;

	//参数转换
	uint16_t targetHoleOffset = selfCommandPtr->commandParam1;
	//处理
	errCode = ActionDishReagent_MoveAnyHoleStep(&currentHoleIndex,&currentOffset,targetHoleOffset);
	//数据更新
	if(errCode == LH_ERR_NONE)
	{
		cmdResultPtr->offsetFlag = currentOffset;
		cmdResultPtr->currentHoleIndex = currentHoleIndex;
	}
#endif

	return errCode;
}


/********************************************************************************************************************/


