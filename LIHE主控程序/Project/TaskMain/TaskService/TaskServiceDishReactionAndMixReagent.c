#include "TaskServiceConfig.h"
#include "TSC_Inc.h"
#include "ActionImplementInc.h"
#include "TestProc_CanConfig.h"
#include "Can1DataType.h"
//任务控制块,8字节对齐
#pragma pack(8)
//任务堆栈
CPU_STK stackBufferTaskServiceDishReactionMixReagent[STK_SIZE_TASK_SERVICE_DISH_REACTION_MIX_REAGENT];
#pragma pack()

OS_TCB tcbTaskServiceDishReactionMixReagent;

//串口指令处理
static void TaskServiceCommandProc_DishReactionMixReagent_UartRS1(SYSTEM_CMD_UART_RS1* uartCommand);

//CAN口指令处理
static void TaskServiceCommandProc_DishReactionMixReagent_Can1(SYSTEM_CMD_CAN1* canCommand);

//网口指令处理
static void TaskServiceCommandProc_DishReactionMixReagent_NetW5500(SYSTEM_CMD_NET_W5500* netCommand);

//自身指令处理
static void TaskServiceCommandProc_DishReactionMixReagent_Self(SYSTEM_CMD_SELF* selfCommand);

//任务函数
void TaskServiceFuncDishReactionMixReagent(void *p_arg)
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
                TaskServiceCommandProc_DishReactionMixReagent_UartRS1(uartRS1CommandPtr);
                //处置完成,执行后续操作
                UserMemFree(SRAM_CCM,uartRS1CommandPtr);
            }
            else if (SYSTEM_CMD_SRC_SELF == taskSystemRecvCommandPtr->commandSrc)
            {
                //自身指令源
                SYSTEM_CMD_SELF* selfCommandPtr =  (SYSTEM_CMD_SELF*)(taskSystemRecvCommandPtr->systemCommandDataPtr);
                //对接收到的数据进行处理
                TaskServiceCommandProc_DishReactionMixReagent_Self(selfCommandPtr);
                //处置完成,执行后续操作
                UserMemFree(SRAM_CCM,selfCommandPtr);
            }
            else if(SYSTEM_CMD_SRC_NET_W5500 == taskSystemRecvCommandPtr->commandSrc)
            {
                //网口指令源
                SYSTEM_CMD_NET_W5500* netW5500CommandPtr =  (SYSTEM_CMD_NET_W5500*)(taskSystemRecvCommandPtr->systemCommandDataPtr);
                //对接收到的数据进行处理
                TaskServiceCommandProc_DishReactionMixReagent_NetW5500(netW5500CommandPtr);
                //处置完成,执行后续操作
                UserMemFree(SRAM_CCM,netW5500CommandPtr);
            }
            else if(SYSTEM_CMD_SRC_CAN1 == taskSystemRecvCommandPtr->commandSrc)
            {
                //CAN1口指令源
                SYSTEM_CMD_CAN1* can1CommandPtr =  (SYSTEM_CMD_CAN1*)(taskSystemRecvCommandPtr->systemCommandDataPtr);
                //对接收到的数据进行处理
                TaskServiceCommandProc_DishReactionMixReagent_Can1(can1CommandPtr);
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
//反应盘指令处理

//反应盘初始化
static LH_ERR TaskServiceCommandProc_DishReactionMixReagent_UartRS1_DishReactionReset(SYSTEM_CMD_UART_RS1* uartCommandPtr,uint16_t* resultData);
//反应盘移动到指定杯位
static LH_ERR TaskServiceCommandProc_DishReactionMixReagent_UartRS1_DishReactionMoveSpecialCup(SYSTEM_CMD_UART_RS1* uartCommandPtr,uint16_t* resultData);
//反应盘移动指定个杯位
static LH_ERR TaskServiceCommandProc_DishReactionMixReagent_UartRS1_DishReactionMoveAnyCup(SYSTEM_CMD_UART_RS1* uartCommandPtr,uint16_t* resultData);

//定义函数数组,用于串口处理线程回调
static const TaskServiceProcPtrWithUartCommand taskServiceProcPtrWithDishReactionUartCommandArray[] = {
	TaskServiceCommandProc_DishReactionMixReagent_UartRS1_DishReactionReset,
	TaskServiceCommandProc_DishReactionMixReagent_UartRS1_DishReactionMoveSpecialCup,
	TaskServiceCommandProc_DishReactionMixReagent_UartRS1_DishReactionMoveAnyCup,
};
//反应盘回调函数数量
#define DISH_REACTION_FUNC_UART_COUNT_MAX      (sizeof(taskServiceProcPtrWithDishReactionUartCommandArray)/sizeof(taskServiceProcPtrWithDishReactionUartCommandArray[0]))

//试剂混匀

//试剂混匀初始化
static LH_ERR TaskServiceCommandProc_DishReactionMixReagent_UartRS1_MixReagentReset(SYSTEM_CMD_UART_RS1* uartCommandPtr,uint16_t* resultData);
//试剂混匀上升
static LH_ERR TaskServiceCommandProc_DishReactionMixReagent_UartRS1_MixReagentUp(SYSTEM_CMD_UART_RS1* uartCommandPtr,uint16_t* resultData);
//试剂混匀下降
static LH_ERR TaskServiceCommandProc_DishReactionMixReagent_UartRS1_MixReagentDown(SYSTEM_CMD_UART_RS1* uartCommandPtr,uint16_t* resultData);
//试剂混匀混匀指定时间
static LH_ERR TaskServiceCommandProc_DishReactionMixReagent_UartRS1_MixReagentStartMix(SYSTEM_CMD_UART_RS1* uartCommandPtr,uint16_t* resultData);

//定义函数数组,用于串口处理线程回调
static const TaskServiceProcPtrWithUartCommand taskServiceProcMixReagentPtrWithUartCommandArray[] = {
	TaskServiceCommandProc_DishReactionMixReagent_UartRS1_MixReagentReset,
	TaskServiceCommandProc_DishReactionMixReagent_UartRS1_MixReagentUp,
	TaskServiceCommandProc_DishReactionMixReagent_UartRS1_MixReagentDown,
	TaskServiceCommandProc_DishReactionMixReagent_UartRS1_MixReagentStartMix,
};
//试剂混匀回调函数数量
#define MIX_REAGENT_FUNC_UART_COUNT_MAX      (sizeof(taskServiceProcMixReagentPtrWithUartCommandArray)/sizeof(taskServiceProcMixReagentPtrWithUartCommandArray[0]))

//反应盘与试剂摇匀任务的串口处理
static void TaskServiceCommandProc_DishReactionMixReagent_UartRS1(SYSTEM_CMD_UART_RS1* uartCommand)
{
    LH_ERR errorCode = LH_ERR_NONE;
	//发送ACK,创建一条新数据包
	SystemUartRS_ModbusCreatePackage(uartCommand->uartRecvCommandArray,uartCommand->srcDeviceAddr,uartCommand->frameNo,
						uartCommand->funcCode,uartCommand->regAddr,uartCommand->regValue);
	BoardRS_SendBuffer(ACTION_DEBUG_PORT,uartCommand->uartRecvCommandArray,12);

    //调用指令
	uint8_t actionCode= ((uartCommand->regAddr)>>8);
    uint16_t resultData = 0x0000;

    //指令处理
    if(((uartCommand->regAddr)&0x00ff) == 0x0D)
    {
        //反应盘
        if(actionCode > DISH_REACTION_FUNC_UART_COUNT_MAX)
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
            errorCode = taskServiceProcPtrWithDishReactionUartCommandArray[actionCode-1](uartCommand,&resultData);
        }
    }
    else if(((uartCommand->regAddr)&0x00ff) == 0x0B)
    {
        //试剂混匀
         if(actionCode > MIX_REAGENT_FUNC_UART_COUNT_MAX)
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
            errorCode = taskServiceProcMixReagentPtrWithUartCommandArray[actionCode-1](uartCommand,&resultData);
        }
    }

	//结果返回
	SystemUartRS_ModbusCreatePackage(uartCommand->uartRecvCommandArray, uartCommand->srcDeviceAddr, uartCommand->frameNo,
							0x01, (uint16_t)errorCode, resultData);
	//发送一个填充好的数据包
	BoardRS_SendBuffer(ACTION_DEBUG_PORT,uartCommand->uartRecvCommandArray,12);
}

//反应盘初始化
static LH_ERR TaskServiceCommandProc_DishReactionMixReagent_UartRS1_DishReactionReset(SYSTEM_CMD_UART_RS1* uartCommandPtr,uint16_t* resultData)
{
    LH_ERR errorCode;
	ACTION_SENSOR_STATUS dishReactionlightSensorCupHasExist;
	uint16_t currentReactionHoleIndex;
	//调用API复位
	errorCode = ActionDishReaction_Reset(&currentReactionHoleIndex,&dishReactionlightSensorCupHasExist);
	if(errorCode == LH_ERR_NONE)
	{
		//返回杯状态
		SystemUartRS_ModbusCreatePackage(uartCommandPtr->uartRecvCommandArray,uartCommandPtr->srcDeviceAddr,uartCommandPtr->frameNo,
						0x01,0x07,(((uint16_t)dishReactionlightSensorCupHasExist)<<8)|currentReactionHoleIndex);
		//发送一个填充好的数据包
		BoardRS_SendBuffer(ACTION_DEBUG_PORT,uartCommandPtr->uartRecvCommandArray,12);
	}
	return errorCode;
}

//反应盘移动到指定杯位
static LH_ERR TaskServiceCommandProc_DishReactionMixReagent_UartRS1_DishReactionMoveSpecialCup(SYSTEM_CMD_UART_RS1* uartCommandPtr,uint16_t* resultData)
{
    LH_ERR errorCode;
	ACTION_SENSOR_STATUS dishReactionlightSensorCupHasExist;
	uint16_t currentReactionHoleIndex;

	uint8_t paramLow = (uint8_t)(uartCommandPtr->regValue);
	//调用API运转
	errorCode = ActionDishReaction_Move2SpecialCup(paramLow,&currentReactionHoleIndex,&dishReactionlightSensorCupHasExist);
	if(errorCode == LH_ERR_NONE)
	{
		//返回杯状态
		SystemUartRS_ModbusCreatePackage(uartCommandPtr->uartRecvCommandArray,uartCommandPtr->srcDeviceAddr,uartCommandPtr->frameNo,
						0x01,0x07,(((uint16_t)dishReactionlightSensorCupHasExist)<<8)|currentReactionHoleIndex);
		//发送一个填充好的数据包
		BoardRS_SendBuffer(ACTION_DEBUG_PORT,uartCommandPtr->uartRecvCommandArray,12);
	}
	return errorCode;
}

//反应盘移动指定个杯位
static LH_ERR TaskServiceCommandProc_DishReactionMixReagent_UartRS1_DishReactionMoveAnyCup(SYSTEM_CMD_UART_RS1* uartCommandPtr,uint16_t* resultData)
{
    LH_ERR errorCode;
	ACTION_SENSOR_STATUS dishReactionlightSensorCupHasExist;
	uint16_t currentReactionHoleIndex;

	uint8_t paramLow = (uint8_t)(uartCommandPtr->regValue);
	//调用API运转
	errorCode = ActionDishReaction_MoveSpecialCups(paramLow,&currentReactionHoleIndex,&dishReactionlightSensorCupHasExist);
	if(errorCode == LH_ERR_NONE)
	{
		//返回杯状态
		SystemUartRS_ModbusCreatePackage(uartCommandPtr->uartRecvCommandArray,uartCommandPtr->srcDeviceAddr,uartCommandPtr->frameNo,
						0x01,0x07,(((uint16_t)dishReactionlightSensorCupHasExist)<<8)|currentReactionHoleIndex);
		//发送一个填充好的数据包
		BoardRS_SendBuffer(ACTION_DEBUG_PORT,uartCommandPtr->uartRecvCommandArray,12);
	}
	return errorCode;
}




//试剂混匀初始化
static LH_ERR TaskServiceCommandProc_DishReactionMixReagent_UartRS1_MixReagentReset(SYSTEM_CMD_UART_RS1* uartCommandPtr,uint16_t* resultData)
{
    LH_ERR errorCode;
	errorCode = ActionMixReagent_Reset();
	return errorCode;
}

//试剂混匀上升
static LH_ERR TaskServiceCommandProc_DishReactionMixReagent_UartRS1_MixReagentUp(SYSTEM_CMD_UART_RS1* uartCommandPtr,uint16_t* resultData)
{
    LH_ERR errorCode;
	errorCode = ActionMixReagent_Up();
	return errorCode;
}

//试剂混匀下降
static LH_ERR TaskServiceCommandProc_DishReactionMixReagent_UartRS1_MixReagentDown(SYSTEM_CMD_UART_RS1* uartCommandPtr,uint16_t* resultData)
{
    LH_ERR errorCode;
	errorCode = ActionMixReagent_Down();
	return errorCode;
}

//试剂混匀混匀指定时间
static LH_ERR TaskServiceCommandProc_DishReactionMixReagent_UartRS1_MixReagentStartMix(SYSTEM_CMD_UART_RS1* uartCommandPtr,uint16_t* resultData)
{
    uint8_t paramLow = (uint8_t)(uartCommandPtr->regValue);
	LH_ERR errorCode;
    uint16_t rotateTime = paramLow * 50;
	errorCode = ActionMixReagent_RotateAnyTimes(rotateTime);
	return errorCode;
}




/********************************************************************************************************************/

/*****************************************************CAN指令处理*****************************************************/
//试剂混匀初始化
static LH_ERR TaskServiceCommandProc_DishReactionMixReagent_Can1_MixReagentReset(SYSTEM_CMD_CAN1* canCommand)
{
    LH_ERR errorCode;
	uint8_t count;
	errorCode = ActionMixReagent_Reset();
	count = Arraypack_Common(canCommand,0,errorCode);
	CanSendDataToCell(TESTPROC_CELL_PLATFORM_NMBR,BOARD_ID_TRACK_COMM,0x0018,canCommand->can1RecvDataBuffer,count);
	return errorCode;
}
//试剂混匀上升
static LH_ERR TaskServiceCommandProc_DishReactionMixReagent_Can1_MixReagentUp(SYSTEM_CMD_CAN1* canCommand)
{
    LH_ERR errorCode;
	uint8_t count;
	errorCode = ActionMixReagent_Up();
	count = Arraypack_Common(canCommand,0,errorCode);
	CanSendDataToCell(TESTPROC_CELL_PLATFORM_NMBR,BOARD_ID_TRACK_COMM,0x0018,canCommand->can1RecvDataBuffer,count);
	return errorCode;
}
//试剂混匀下降
static LH_ERR TaskServiceCommandProc_DishReactionMixReagent_Can1_MixReagentDown(SYSTEM_CMD_CAN1* canCommand)
{
    LH_ERR errorCode;
	uint8_t count;
	errorCode = ActionMixReagent_Down();
	count = Arraypack_Common(canCommand,0,errorCode);
	CanSendDataToCell(TESTPROC_CELL_PLATFORM_NMBR,BOARD_ID_TRACK_COMM,0x0018,canCommand->can1RecvDataBuffer,count);
	return errorCode;
}

//试剂混匀混匀指定时间
static LH_ERR TaskServiceCommandProc_DishReactionMixReagent_Can1_MixReagentStartMix(SYSTEM_CMD_CAN1* canCommand)
{
    uint16_t paramLow = canCommand->can1RecvDataBuffer[0]|(canCommand->can1RecvDataBuffer[1]<<8);
	LH_ERR errorCode;
	uint8_t count;
    uint16_t rotateTime = paramLow;
	errorCode = ActionMixReagent_RotateAnyTimes(rotateTime);
	count = Arraypack_Common(canCommand,0,errorCode);
	CanSendDataToCell(TESTPROC_CELL_PLATFORM_NMBR,BOARD_ID_TRACK_COMM,0x0018,canCommand->can1RecvDataBuffer,count);
	return errorCode;
}
//反应盘初始化
static LH_ERR TaskServiceCommandProc_DishReactionMixReagent_Can1_DishReactionReset(SYSTEM_CMD_CAN1* canCommand)
{
    LH_ERR errorCode;
	ACTION_SENSOR_STATUS dishReactionlightSensorCupHasExist;
	uint16_t currentReactionHoleIndex,count;
	//调用API复位
	errorCode = ActionDishReaction_Reset(&currentReactionHoleIndex,&dishReactionlightSensorCupHasExist);
	count = Arraypack_Common(canCommand,0,errorCode);
	CanSendDataToCell(TESTPROC_CELL_PLATFORM_NMBR,BOARD_ID_TRACK_COMM,0x0018,canCommand->can1RecvDataBuffer,count);
	return errorCode;
}
//反应盘移动到指定杯位
static LH_ERR TaskServiceCommandProc_DishReactionMixReagent_Can1_DishReactionMoveSpecialCup(SYSTEM_CMD_CAN1* canCommand)
{
    LH_ERR errorCode;
	ACTION_SENSOR_STATUS dishReactionlightSensorCupHasExist;
	uint16_t currentReactionHoleIndex,count;

	uint8_t paramLow =canCommand->can1RecvDataBuffer[0];
	//调用API运转
	errorCode = ActionDishReaction_Move2SpecialCup(paramLow,&currentReactionHoleIndex,&dishReactionlightSensorCupHasExist);
	count = Arraypack_Common(canCommand,paramLow,errorCode);
	CanSendDataToCell(TESTPROC_CELL_PLATFORM_NMBR,BOARD_ID_TRACK_COMM,0x0018,canCommand->can1RecvDataBuffer,count);
	return errorCode;
}
//反应盘移动指定个杯位
static LH_ERR TaskServiceCommandProc_DishReactionMixReagent_Can1_DishReactionMoveAnyCup(SYSTEM_CMD_CAN1* canCommand)
{
    LH_ERR errorCode;
	ACTION_SENSOR_STATUS dishReactionlightSensorCupHasExist;
	uint16_t currentReactionHoleIndex,count;

	uint8_t paramLow = canCommand->can1RecvDataBuffer[0];
	//调用API运转
	errorCode = ActionDishReaction_MoveSpecialCups(paramLow,&currentReactionHoleIndex,&dishReactionlightSensorCupHasExist);
	count = Arraypack_Common(canCommand,paramLow,errorCode);
	CanSendDataToCell(TESTPROC_CELL_PLATFORM_NMBR,BOARD_ID_TRACK_COMM,0x0018,canCommand->can1RecvDataBuffer,count);
	return errorCode;
}

static const APP_TASK_CAN1_PROC_UNIT taskServiceProcPtrWithMixReagentCan1CommandArray[] = {
    {CAN1_COMMAND_REAGENT_MIX_RESET          ,        TaskServiceCommandProc_DishReactionMixReagent_Can1_MixReagentReset},//试剂混匀初始化
    {CAN1_COMMAND_REAGENT_MIX_UP             ,        TaskServiceCommandProc_DishReactionMixReagent_Can1_MixReagentUp},//试剂混匀上升
    {CAN1_COMMAND_REAGENT_MIX_DOWN           ,        TaskServiceCommandProc_DishReactionMixReagent_Can1_MixReagentDown},//试剂混匀下降
    {CAN1_COMMAND_REAGENT_MIX                ,        TaskServiceCommandProc_DishReactionMixReagent_Can1_MixReagentStartMix},//试剂混匀
	{CAN1_COMMAND_REACTION_DISH_RESET                  ,       TaskServiceCommandProc_DishReactionMixReagent_Can1_DishReactionReset},//反应盘初始化
    {CAN1_COMMAND_REACTION_DISH_ROTATE_TO_SPECIALPOS   ,        TaskServiceCommandProc_DishReactionMixReagent_Can1_DishReactionMoveSpecialCup},//反应盘旋转到指定杯位
    {CAN1_COMMAND_REACTION_DISH_ROTAT_SPECIALPOS       ,        TaskServiceCommandProc_DishReactionMixReagent_Can1_DishReactionMoveAnyCup},//反应盘旋转N个杯位
};
#define MIX_REAGENT_FUNC_CAN_COUNT_MAX      (sizeof(taskServiceProcPtrWithMixReagentCan1CommandArray)/sizeof(taskServiceProcPtrWithMixReagentCan1CommandArray[0]))

static void TaskServiceCommandProc_DishReactionMixReagent_Can1(SYSTEM_CMD_CAN1* canCommand)
{
	uint16_t indexUtil = 0;
    for(indexUtil = 0; indexUtil < MIX_REAGENT_FUNC_CAN_COUNT_MAX; indexUtil++)
    {
        //指令匹配
        if(canCommand->command == taskServiceProcPtrWithMixReagentCan1CommandArray[indexUtil].command)
        {
            //调用处理函数
            if(taskServiceProcPtrWithMixReagentCan1CommandArray[indexUtil].can1CommandProcFunc != NULL)
            {
                taskServiceProcPtrWithMixReagentCan1CommandArray[indexUtil].can1CommandProcFunc(canCommand);
                break;
            }
        }
    }
}

/********************************************************************************************************************/

/*******************************************************网口指令处理**************************************************/

static void TaskServiceCommandProc_DishReactionMixReagent_NetW5500(SYSTEM_CMD_NET_W5500* netCommand)
{

}

/********************************************************************************************************************/

/*******************************************************自身指令处理**************************************************/
//反应盘初始化
static LH_ERR TaskServiceCommandProc_DishReaction_Self_Init(SYSTEM_CMD_SELF* selfCommandPtr);
//反应盘移动到指定杯位
static LH_ERR TaskServiceCommandProc_DishReaction_Self_Move2SpecialHole(SYSTEM_CMD_SELF* selfCommandPtr);
//反应盘移动指定个杯位
static LH_ERR TaskServiceCommandProc_DishReaction_Self_MoveAnyHoleStep(SYSTEM_CMD_SELF* selfCommandPtr);
//试剂混匀初始化
static LH_ERR TaskServiceCommandProc_ReagentMix_Self_Init(SYSTEM_CMD_SELF* selfCommandPtr);
//试剂混匀运行指定时间
static LH_ERR TaskServiceCommandProc_ReagentMix_Self_RotateAnyTimes(SYSTEM_CMD_SELF* selfCommandPtr);


//自身指令回调函数数组
static const TaskServicePtrWithSelfCommand TaskProcWithSelfDishReactionMixReagentCommandArray[] = {
    TaskServiceCommandProc_DishReaction_Self_Init,
    TaskServiceCommandProc_DishReaction_Self_Move2SpecialHole,
    TaskServiceCommandProc_DishReaction_Self_MoveAnyHoleStep,
    TaskServiceCommandProc_ReagentMix_Self_Init,
    TaskServiceCommandProc_ReagentMix_Self_RotateAnyTimes,
};

//支持指令的个数
#define TASK_COMMAND_PROC_SELF_COUNT	(sizeof(TaskProcWithSelfDishReactionMixReagentCommandArray)/sizeof(TaskProcWithSelfDishReactionMixReagentCommandArray[0]))

//用于调试指令的头调试字符串
static const uint8_t TaskSelfCommandDebugStartMsgHeadString[] = "$$Action 2 Start : ";
static const uint8_t TaskSelfCommandDebugEndMsgHeadString[] = "$$Action 2 End ";



static void TaskServiceCommandProc_DishReactionMixReagent_Self(SYSTEM_CMD_SELF* selfCommand)
{
    OS_ERR err;
	LH_ERR actionResult;
	//指令过程与结果对象
	TSC_CMD_RESULT_DATA_DISH_REACTION_REAGENT_MIX* cmdResultPtr = 
                    (TSC_CMD_RESULT_DATA_DISH_REACTION_REAGENT_MIX*)selfCommand->commandDataSpecialChannel;
	
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
		actionResult = TaskProcWithSelfDishReactionMixReagentCommandArray[selfCommand->commandIndex](selfCommand);
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

//反应盘初始化
static LH_ERR TaskServiceCommandProc_DishReaction_Self_Init(SYSTEM_CMD_SELF* selfCommandPtr)
{
    LH_ERR errCode;
	//打印开始处理信息
	TaskServiceSerialDebugMsgShowStart((uint8_t*)TaskSelfCommandDebugStartMsgHeadString,
										(uint8_t*)"DishReactionInit",
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
	ACTION_SENSOR_STATUS dishReactionlightSensorCupHasExist;
	uint16_t currentReactionHoleIndex;

	//指令过程与结果对象
	TSC_CMD_RESULT_DATA_DISH_REACTION_REAGENT_MIX* cmdResultPtr = 
				(TSC_CMD_RESULT_DATA_DISH_REACTION_REAGENT_MIX*)selfCommandPtr->commandDataSpecialChannel;

	//处理
	errCode = ActionDishReaction_Reset(&currentReactionHoleIndex,&dishReactionlightSensorCupHasExist);
	if(errCode == LH_ERR_NONE)
	{
		cmdResultPtr->currentHoleHasCup = dishReactionlightSensorCupHasExist;
		cmdResultPtr->currentReactionHoleIndex = currentReactionHoleIndex;
	}
#endif

	return errCode;
}

//反应盘移动到指定杯位
static LH_ERR TaskServiceCommandProc_DishReaction_Self_Move2SpecialHole(SYSTEM_CMD_SELF* selfCommandPtr)
{
    LH_ERR errCode;
	//打印开始处理信息
	TaskServiceSerialDebugMsgShowStart((uint8_t*)TaskSelfCommandDebugStartMsgHeadString,
										(uint8_t*)"DishReactionInit",
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
	ACTION_SENSOR_STATUS dishReactionlightSensorCupHasExist;
	uint16_t currentReactionHoleIndex;

	//指令过程与结果对象
	TSC_CMD_RESULT_DATA_DISH_REACTION_REAGENT_MIX* cmdResultPtr = 
				(TSC_CMD_RESULT_DATA_DISH_REACTION_REAGENT_MIX*)selfCommandPtr->commandDataSpecialChannel;
	
	uint16_t targetHoleIndex = selfCommandPtr->commandParam1;

	//处理
	errCode = ActionDishReaction_Move2SpecialCup(targetHoleIndex,&currentReactionHoleIndex,&dishReactionlightSensorCupHasExist);

	if(errCode == LH_ERR_NONE)
	{
		cmdResultPtr->currentHoleHasCup = dishReactionlightSensorCupHasExist;
		cmdResultPtr->currentReactionHoleIndex = currentReactionHoleIndex;
	}
#endif

	return errCode;
}

//反应盘移动指定个杯位
static LH_ERR TaskServiceCommandProc_DishReaction_Self_MoveAnyHoleStep(SYSTEM_CMD_SELF* selfCommandPtr)
{
    LH_ERR errCode;
	//打印开始处理信息
	TaskServiceSerialDebugMsgShowStart((uint8_t*)TaskSelfCommandDebugStartMsgHeadString,
										(uint8_t*)"DishReactionMoveAnyHoleStep",
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
	ACTION_SENSOR_STATUS dishReactionlightSensorCupHasExist;
	uint16_t currentReactionHoleIndex;

	//指令过程与结果对象
	TSC_CMD_RESULT_DATA_DISH_REACTION_REAGENT_MIX* cmdResultPtr = 
				(TSC_CMD_RESULT_DATA_DISH_REACTION_REAGENT_MIX*)selfCommandPtr->commandDataSpecialChannel;

	uint16_t targetHoleOffset = selfCommandPtr->commandParam1;

	//处理
	errCode = ActionDishReaction_MoveSpecialCups(targetHoleOffset,&currentReactionHoleIndex,&dishReactionlightSensorCupHasExist);

	if(errCode == LH_ERR_NONE)
	{
		cmdResultPtr->currentHoleHasCup = dishReactionlightSensorCupHasExist;
		cmdResultPtr->currentReactionHoleIndex = currentReactionHoleIndex;
	}
#endif

	return errCode;
}

//试剂混匀初始化
static LH_ERR TaskServiceCommandProc_ReagentMix_Self_Init(SYSTEM_CMD_SELF* selfCommandPtr)
{
    LH_ERR errCode;
	//打印开始处理信息
	TaskServiceSerialDebugMsgShowStart((uint8_t*)TaskSelfCommandDebugStartMsgHeadString,
										(uint8_t*)"ReagentMixInit",
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
	errCode = ActionMixReagent_Reset();
#endif

	return errCode;
}

//试剂混匀运行指定时间
static LH_ERR TaskServiceCommandProc_ReagentMix_Self_RotateAnyTimes(SYSTEM_CMD_SELF* selfCommandPtr)
{
    LH_ERR errCode;
	//打印开始处理信息
	TaskServiceSerialDebugMsgShowStart((uint8_t*)TaskSelfCommandDebugStartMsgHeadString,
										(uint8_t*)"ReagentMixRotateAnyTimes",
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
	uint16_t rotateTimes = selfCommandPtr->commandParam1;

	//处理
	errCode = ActionMixReagent_Up();
	if(errCode != LH_ERR_NONE)
	{
		//打印结束处理信息
		return errCode;
	}
	errCode = ActionMixReagent_RotateAnyTimes(rotateTimes);
	if(errCode != LH_ERR_NONE)
	{
		//打印结束处理信息
		return errCode;
	}
	errCode = ActionMixReagent_Down();
	if(errCode != LH_ERR_NONE)
	{
		return errCode;
	}
#endif

	return errCode;
}



/********************************************************************************************************************/



