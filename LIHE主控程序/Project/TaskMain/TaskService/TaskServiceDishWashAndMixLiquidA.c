//#include "TaskServiceConfig.h"
#include "TSC_Inc.h"
#include "ActionImplementInc.h"
#include "Can1DataType.h"
//任务控制块,8字节对齐
#pragma pack(8)
//任务堆栈
CPU_STK stackBufferTaskServiceDishWashMixLiquidA[STK_SIZE_TASK_SERVICE_DISH_WASH_MIX_LIQUID_A];
#pragma pack()

OS_TCB tcbTaskServiceDishWashMixLiquidA;

//串口指令处理
static void TaskServiceCommandProc_DishWashMixLiquidA_UartRS1(SYSTEM_CMD_UART_RS1* uartCommand);

//CAN口指令处理
static void TaskServiceCommandProc_DishWashMixLiquidA_Can1(SYSTEM_CMD_CAN1* canCommand);

//网口指令处理
static void TaskServiceCommandProc_DishWashMixLiquidA_NetW5500(SYSTEM_CMD_NET_W5500* netCommand);

//自身指令处理
static void TaskServiceCommandProc_DishWashMixLiquidA_Self(SYSTEM_CMD_SELF* selfCommand);

//任务函数
void TaskServiceFuncDishWashMixLiquidA(void *p_arg)
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
                TaskServiceCommandProc_DishWashMixLiquidA_UartRS1(uartRS1CommandPtr);
                //处置完成,执行后续操作
                UserMemFree(SRAM_CCM,uartRS1CommandPtr);
            }
            else if (SYSTEM_CMD_SRC_SELF == taskSystemRecvCommandPtr->commandSrc)
            {
                //自身指令源
                SYSTEM_CMD_SELF* selfCommandPtr =  (SYSTEM_CMD_SELF*)(taskSystemRecvCommandPtr->systemCommandDataPtr);
                //对接收到的数据进行处理
                TaskServiceCommandProc_DishWashMixLiquidA_Self(selfCommandPtr);
                //处置完成,执行后续操作
                UserMemFree(SRAM_CCM,selfCommandPtr);
            }
            else if(SYSTEM_CMD_SRC_NET_W5500 == taskSystemRecvCommandPtr->commandSrc)
            {
                //网口指令源
                SYSTEM_CMD_NET_W5500* netW5500CommandPtr =  (SYSTEM_CMD_NET_W5500*)(taskSystemRecvCommandPtr->systemCommandDataPtr);
                //对接收到的数据进行处理
                TaskServiceCommandProc_DishWashMixLiquidA_NetW5500(netW5500CommandPtr);
                //处置完成,执行后续操作
                UserMemFree(SRAM_CCM,netW5500CommandPtr);
            }
            else if(SYSTEM_CMD_SRC_CAN1 == taskSystemRecvCommandPtr->commandSrc)
            {
                //CAN1口指令源
                SYSTEM_CMD_CAN1* can1CommandPtr =  (SYSTEM_CMD_CAN1*)(taskSystemRecvCommandPtr->systemCommandDataPtr);
                //对接收到的数据进行处理
                TaskServiceCommandProc_DishWashMixLiquidA_Can1(can1CommandPtr);
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

//清洗盘指令处理

//清洗盘初始化
static LH_ERR TaskServiceCommandProc_DishWashMixLiquidA_UartRS1_DishWashReset(SYSTEM_CMD_UART_RS1* uartCommandPtr,uint16_t* resultData);
//清洗盘移动到指定杯位
static LH_ERR TaskServiceCommandProc_DishWashMixLiquidA_UartRS1DishWashMoveSpecialCup(SYSTEM_CMD_UART_RS1* uartCommandPtr,uint16_t* resultData);
//清洗盘移动指定个杯位
static LH_ERR TaskServiceCommandProc_DishWashMixLiquidA_UartRS1_DishWashMoveAnyCup(SYSTEM_CMD_UART_RS1* uartCommandPtr,uint16_t* resultData);

//定义函数数组,用于串口处理线程回调
static const TaskServiceProcPtrWithUartCommand taskServiceProcPtrWithDishWashUartCommandArray[] = {
	TaskServiceCommandProc_DishWashMixLiquidA_UartRS1_DishWashReset,
	TaskServiceCommandProc_DishWashMixLiquidA_UartRS1DishWashMoveSpecialCup,
	TaskServiceCommandProc_DishWashMixLiquidA_UartRS1_DishWashMoveAnyCup,
};
//清洗盘回调函数数量
#define DISH_WASH_FUNC_UART_COUNT_MAX      (sizeof(taskServiceProcPtrWithDishWashUartCommandArray)/sizeof(taskServiceProcPtrWithDishWashUartCommandArray[0]))


//A液混匀指令处理

//A液摇匀复位
static LH_ERR TaskServiceCommandProc_DishWashMixLiquidA_UartRS1_MixLiquidAReset(SYSTEM_CMD_UART_RS1* uartCommandPtr,uint16_t* resultData);
//A液摇匀上升
static LH_ERR TaskServiceCommandProc_DishWashMixLiquidA_UartRS1_MixLiquidAUp(SYSTEM_CMD_UART_RS1* uartCommandPtr,uint16_t* resultData);
//A液摇匀下降
static LH_ERR TaskServiceCommandProc_DishWashMixLiquidA_UartRS1_MixLiquidADown(SYSTEM_CMD_UART_RS1* uartCommandPtr,uint16_t* resultData);
//A液摇匀指定时间
static LH_ERR TaskServiceCommandProc_DishWashMixLiquidA_UartRS1_MixLiquidARotateAnyTimes(SYSTEM_CMD_UART_RS1* uartCommandPtr,uint16_t* resultData);

//定义函数数组,用于串口处理线程回调
static const TaskServiceProcPtrWithUartCommand taskServiceProcPtrWithMixLiquidAUartCommandArray[] = {
	TaskServiceCommandProc_DishWashMixLiquidA_UartRS1_MixLiquidAReset,
	TaskServiceCommandProc_DishWashMixLiquidA_UartRS1_MixLiquidAUp,
	TaskServiceCommandProc_DishWashMixLiquidA_UartRS1_MixLiquidADown,
    TaskServiceCommandProc_DishWashMixLiquidA_UartRS1_MixLiquidARotateAnyTimes,
};
//清洗盘回调函数数量
#define MIX_LIQUID_A_FUNC_UART_COUNT_MAX      (sizeof(taskServiceProcPtrWithMixLiquidAUartCommandArray)/sizeof(taskServiceProcPtrWithMixLiquidAUartCommandArray[0]))


//串口指令初步处理
static void TaskServiceCommandProc_DishWashMixLiquidA_UartRS1(SYSTEM_CMD_UART_RS1* uartCommand)
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
    if(((uartCommand->regAddr)&0x00ff) == 0x01)
    {
        //清洗盘
        if(actionCode > DISH_WASH_FUNC_UART_COUNT_MAX)
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
            errorCode = taskServiceProcPtrWithDishWashUartCommandArray[actionCode-1](uartCommand,&resultData);
        }
    }
    else if(((uartCommand->regAddr)&0x00ff) == 0x04)
    {
        //A液混匀
         if(actionCode > MIX_LIQUID_A_FUNC_UART_COUNT_MAX)
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
            errorCode = taskServiceProcPtrWithMixLiquidAUartCommandArray[actionCode-1](uartCommand,&resultData);
        }
    }

	//结果返回
	SystemUartRS_ModbusCreatePackage(uartCommand->uartRecvCommandArray, uartCommand->srcDeviceAddr, uartCommand->frameNo,
							0x01, (uint16_t)errorCode, resultData);
	//发送一个填充好的数据包
	BoardRS_SendBuffer(ACTION_DEBUG_PORT,uartCommand->uartRecvCommandArray,12);
}


//清洗盘初始化
static LH_ERR TaskServiceCommandProc_DishWashMixLiquidA_UartRS1_DishWashReset(SYSTEM_CMD_UART_RS1* uartCommandPtr,uint16_t* resultData)
{
    LH_ERR errorCode;
	ACTION_SENSOR_STATUS dishWashlightSensorCupHasExist;
	uint16_t currentDishWashHoleIndex;
	//调用API复位
	errorCode = ActionDishWash_Reset(&currentDishWashHoleIndex,&dishWashlightSensorCupHasExist);
	if(errorCode == LH_ERR_NONE)
	{
		//返回杯状态
		SystemUartRS_ModbusCreatePackage(uartCommandPtr->uartRecvCommandArray,uartCommandPtr->srcDeviceAddr,uartCommandPtr->frameNo,
						0x01,0x0107,(((uint16_t)dishWashlightSensorCupHasExist)<<8)|currentDishWashHoleIndex);
		//发送一个填充好的数据包
		BoardRS_SendBuffer(ACTION_DEBUG_PORT,uartCommandPtr->uartRecvCommandArray,12);
	}
	return errorCode;
}

//清洗盘移动到指定杯位
static LH_ERR TaskServiceCommandProc_DishWashMixLiquidA_UartRS1DishWashMoveSpecialCup(SYSTEM_CMD_UART_RS1* uartCommandPtr,uint16_t* resultData)
{
    LH_ERR errorCode;
	ACTION_SENSOR_STATUS dishWashlightSensorCupHasExist;
	uint16_t currentDishWashHoleIndex;
	uint8_t paramLow = (uint8_t)(uartCommandPtr->regValue);

	errorCode = ActionDishWash_Move2SpecialCup(paramLow,&currentDishWashHoleIndex,&dishWashlightSensorCupHasExist);
	if(errorCode == LH_ERR_NONE)
	{
		//返回杯状态
		SystemUartRS_ModbusCreatePackage(uartCommandPtr->uartRecvCommandArray,uartCommandPtr->srcDeviceAddr,uartCommandPtr->frameNo,
						0x01,0x0107,(((uint16_t)dishWashlightSensorCupHasExist)<<8)|currentDishWashHoleIndex);
		//发送一个填充好的数据包
		BoardRS_SendBuffer(ACTION_DEBUG_PORT,uartCommandPtr->uartRecvCommandArray,12);
	}

	return errorCode;
}

//清洗盘移动指定个杯位
static LH_ERR TaskServiceCommandProc_DishWashMixLiquidA_UartRS1_DishWashMoveAnyCup(SYSTEM_CMD_UART_RS1* uartCommandPtr,uint16_t* resultData)
{
    LH_ERR errorCode;
	ACTION_SENSOR_STATUS dishWashlightSensorCupHasExist;
	uint16_t currentDishWashHoleIndex;
	uint8_t paramLow = (uint8_t)(uartCommandPtr->regValue);

	errorCode = ActionDishWash_MoveSpecialCups(paramLow,&currentDishWashHoleIndex,&dishWashlightSensorCupHasExist);
	if(errorCode == LH_ERR_NONE)
	{
		//返回杯状态
		SystemUartRS_ModbusCreatePackage(uartCommandPtr->uartRecvCommandArray,uartCommandPtr->srcDeviceAddr,uartCommandPtr->frameNo,
						0x01,0x0107,(((uint16_t)dishWashlightSensorCupHasExist)<<8)|currentDishWashHoleIndex);
		//发送一个填充好的数据包
		BoardRS_SendBuffer(ACTION_DEBUG_PORT,uartCommandPtr->uartRecvCommandArray,12);
	}
	return errorCode;
}

//A液摇匀复位
static LH_ERR TaskServiceCommandProc_DishWashMixLiquidA_UartRS1_MixLiquidAReset(SYSTEM_CMD_UART_RS1* uartCommandPtr,uint16_t* resultData)
{
    LH_ERR errorCode;
	errorCode = ActionMixLiquidA_Reset();
	return errorCode;
}

//A液摇匀上升
static LH_ERR TaskServiceCommandProc_DishWashMixLiquidA_UartRS1_MixLiquidAUp(SYSTEM_CMD_UART_RS1* uartCommandPtr,uint16_t* resultData)
{
    LH_ERR errorCode;
	errorCode = ActionMixLiquidA_Up();
	return errorCode;
}

//A液摇匀下降
static LH_ERR TaskServiceCommandProc_DishWashMixLiquidA_UartRS1_MixLiquidADown(SYSTEM_CMD_UART_RS1* uartCommandPtr,uint16_t* resultData)
{
    LH_ERR errorCode;
	errorCode = ActionMixLiquidA_Down();
	return errorCode;
}

//A液摇匀指定时间
static LH_ERR TaskServiceCommandProc_DishWashMixLiquidA_UartRS1_MixLiquidARotateAnyTimes(SYSTEM_CMD_UART_RS1* uartCommandPtr,uint16_t* resultData)
{
    LH_ERR errorCode;
	uint8_t paramLow = (uint8_t)(uartCommandPtr->regValue);
	//升降上升
	errorCode = ActionMixLiquidA_Up();
	if(errorCode != LH_ERR_NONE)
	{
		return errorCode;
	}
	//混匀开始
	errorCode = ActionMixLiquidA_RotateAnyTimes(paramLow*50);
	if(errorCode != LH_ERR_NONE)
	{
		return errorCode;
	}
	//升降下降
	errorCode = ActionMixLiquidA_Down();
	if(errorCode != LH_ERR_NONE)
	{
		return errorCode;
	}
	//返回错误代码
	return errorCode;
}







/********************************************************************************************************************/

/*****************************************************CAN指令处理*****************************************************/

//清洗盘复位
static LH_ERR TaskServiceCommandProc_DishWashMixLiquidA_Can1_DishWashReset(SYSTEM_CMD_CAN1* canCommand)
{
	LH_ERR errorCode;
	ACTION_SENSOR_STATUS dishWashlightSensorCupHasExist;
	uint16_t currentDishWashHoleIndex;
	uint16_t count;
	//调用API复位
	errorCode = ActionDishWash_Reset(&currentDishWashHoleIndex,&dishWashlightSensorCupHasExist);

    count = Arraypack_Common(canCommand,0,errorCode);
	CanSendDataToCell(TESTPROC_CELL_PLATFORM_NMBR,BOARD_ID_TRACK_COMM,0x0018,canCommand->can1RecvDataBuffer,count);
    return errorCode;
}
//清洗盘旋转指定杯位
static LH_ERR TaskServiceCommandProc_DishWashMixLiquidA_Can1_DishWashMoveAnyCup(SYSTEM_CMD_CAN1* canCommand)
{
	LH_ERR errorCode;
	ACTION_SENSOR_STATUS dishWashlightSensorCupHasExist;
	uint16_t currentDishWashHoleIndex;
	uint8_t paramLow = canCommand->can1RecvDataBuffer[0];
    uint8_t count;

	errorCode = ActionDishWash_MoveSpecialCups(paramLow,&currentDishWashHoleIndex,&dishWashlightSensorCupHasExist);

	count = Arraypack_Common(canCommand,paramLow,errorCode);
	
	CanSendDataToCell(TESTPROC_CELL_PLATFORM_NMBR,BOARD_ID_TRACK_COMM,0x0018,canCommand->can1RecvDataBuffer,count);
    return errorCode;
}
//清洗盘移动到指定杯位
static LH_ERR TaskServiceCommandProc_DishWashMixLiquidA_Can1_DishWashMoveSpecialCup(SYSTEM_CMD_CAN1* canCommand)
{
    LH_ERR errorCode;
	ACTION_SENSOR_STATUS dishWashlightSensorCupHasExist;
	uint16_t currentDishWashHoleIndex;
	uint8_t count;
	uint8_t paramLow = canCommand->can1RecvDataBuffer[0];

	errorCode = ActionDishWash_Move2SpecialCup(paramLow,&currentDishWashHoleIndex,&dishWashlightSensorCupHasExist);
	
	count = Arraypack_Common(canCommand,paramLow,errorCode);

	CanSendDataToCell(TESTPROC_CELL_PLATFORM_NMBR,BOARD_ID_TRACK_COMM,0x0018,canCommand->can1RecvDataBuffer,count);
	return errorCode;
}

//A液摇匀复位
static LH_ERR TaskServiceCommandProc_DishWashMixLiquidA_Can1_MixLiquidAReset(SYSTEM_CMD_CAN1* canCommand)
{
    LH_ERR errorCode;
	uint8_t count;
	errorCode = ActionMixLiquidA_Reset();
	count = Arraypack_Common(canCommand,0,errorCode);
	CanSendDataToCell(TESTPROC_CELL_PLATFORM_NMBR,BOARD_ID_TRACK_COMM,0x0018,canCommand->can1RecvDataBuffer,count);
	return errorCode;
}
//A液摇匀上升
static LH_ERR TaskServiceCommandProc_DishWashMixLiquidA_Can1_MixLiquidAUp(SYSTEM_CMD_CAN1* canCommand)
{
    LH_ERR errorCode;
	uint8_t count;
	errorCode = ActionMixLiquidA_Up();
	count = Arraypack_Common(canCommand,0,errorCode);
	CanSendDataToCell(TESTPROC_CELL_PLATFORM_NMBR,BOARD_ID_TRACK_COMM,0x0018,canCommand->can1RecvDataBuffer,count);
	return errorCode;
}
//A液摇匀下降
static LH_ERR TaskServiceCommandProc_DishWashMixLiquidA_Can1_MixLiquidADown(SYSTEM_CMD_CAN1* canCommand)
{
    LH_ERR errorCode;
	uint8_t count;
	errorCode = ActionMixLiquidA_Down();
	count = Arraypack_Common(canCommand,0,errorCode);
	CanSendDataToCell(TESTPROC_CELL_PLATFORM_NMBR,BOARD_ID_TRACK_COMM,0x0018,canCommand->can1RecvDataBuffer,count);
	return errorCode;
}

//A液摇匀指定时间
static LH_ERR TaskServiceCommandProc_DishWashMixLiquidA_Can1_MixLiquidARotateAnyTimes(SYSTEM_CMD_CAN1* canCommand)
{
    LH_ERR errorCode;
	uint16_t paramLow = canCommand->can1RecvDataBuffer[0]|(canCommand->can1RecvDataBuffer[1]<<8);
	uint8_t count;
	//升降上升
	errorCode = ActionMixLiquidA_Up();
	if(errorCode != LH_ERR_NONE)
	{
		count = Arraypack_Common(canCommand,0,errorCode);
		CanSendDataToCell(TESTPROC_CELL_PLATFORM_NMBR,BOARD_ID_TRACK_COMM,0x0018,canCommand->can1RecvDataBuffer,count);
		return errorCode;
	}
	//混匀开始
	errorCode = ActionMixLiquidA_RotateAnyTimes(paramLow);
	if(errorCode != LH_ERR_NONE)
	{
		count = Arraypack_Common(canCommand,0,errorCode);
		CanSendDataToCell(TESTPROC_CELL_PLATFORM_NMBR,BOARD_ID_TRACK_COMM,0x0018,canCommand->can1RecvDataBuffer,count);
		return errorCode;
	}
	//升降下降
	errorCode = ActionMixLiquidA_Down();
	if(errorCode != LH_ERR_NONE)
	{
		count = Arraypack_Common(canCommand,0,errorCode);
		CanSendDataToCell(TESTPROC_CELL_PLATFORM_NMBR,BOARD_ID_TRACK_COMM,0x0018,canCommand->can1RecvDataBuffer,count);
		return errorCode;
	}
	//返回错误代码
	count = Arraypack_Common(canCommand,paramLow,errorCode);
	CanSendDataToCell(TESTPROC_CELL_PLATFORM_NMBR,BOARD_ID_TRACK_COMM,0x0018,canCommand->can1RecvDataBuffer,count);
	return errorCode;
}

//调试指令处理
const APP_TASK_CAN1_PROC_UNIT appTaskDishwashLiqidACan1ProcArray[] = {

	{CAN1_COMMAND_WASHDISH_RESET                     ,     TaskServiceCommandProc_DishWashMixLiquidA_Can1_DishWashReset},//清洗盘复位
    {CAN1_COMMAND_WASHDISH_MOVE_SPECIALCUP           ,     TaskServiceCommandProc_DishWashMixLiquidA_Can1_DishWashMoveAnyCup},//清洗盘旋转N个杯位
    {CAN1_COMMAND_WASHDISH_MOVE_TO_SPECIALCUP        ,     TaskServiceCommandProc_DishWashMixLiquidA_Can1_DishWashMoveSpecialCup},//清洗盘旋转到指定杯位
	{CAN1_COMMAND_LIQUID_A_MIX_RESET                 ,        TaskServiceCommandProc_DishWashMixLiquidA_Can1_MixLiquidAReset},//A液混匀初始化
    {CAN1_COMMAND_LIQUID_A_MIX_UP                    ,        TaskServiceCommandProc_DishWashMixLiquidA_Can1_MixLiquidAUp},//A液上升
    {CAN1_COMMAND_LIQUID_A_MIX_DOWN                  ,        TaskServiceCommandProc_DishWashMixLiquidA_Can1_MixLiquidADown},//A液下降
    {CAN1_COMMAND_LIQUID_A_MIX_TIME                  ,        TaskServiceCommandProc_DishWashMixLiquidA_Can1_MixLiquidARotateAnyTimes},//A液混匀指定时间
//    {CAN1_COMMAND_LIQUID_A_MIX_AUTO                  ,        &tcbTaskServiceDishWashMixLiquidA},//A液自动混匀
};

//CAN指令支持的数量
#define COUNT_DISHWASH_LIQUID_A     (sizeof(appTaskDishwashLiqidACan1ProcArray)/sizeof(appTaskDishwashLiqidACan1ProcArray[0]))

static void TaskServiceCommandProc_DishWashMixLiquidA_Can1(SYSTEM_CMD_CAN1* canCommand)
{
	uint16_t indexUtil = 0;
    for(indexUtil = 0; indexUtil < COUNT_DISHWASH_LIQUID_A; indexUtil++)
    {
        //指令匹配
        if(canCommand->command == appTaskDishwashLiqidACan1ProcArray[indexUtil].command)
        {
            //调用处理函数
            if(appTaskDishwashLiqidACan1ProcArray[indexUtil].can1CommandProcFunc != NULL)
            {
                appTaskDishwashLiqidACan1ProcArray[indexUtil].can1CommandProcFunc(canCommand);
                break;
            }
        }
    }
}

/********************************************************************************************************************/

/*******************************************************网口指令处理**************************************************/

static void TaskServiceCommandProc_DishWashMixLiquidA_NetW5500(SYSTEM_CMD_NET_W5500* netCommand)
{

}

/********************************************************************************************************************/

/*******************************************************自身指令处理**************************************************/
//清洗盘初始化
static LH_ERR TaskServiceCommandProc_DishWash_Self_Init(SYSTEM_CMD_SELF* selfCommandPtr);
//清洗盘转动指定孔数
static LH_ERR TaskServiceCommandProc_DishWash_Self_MoveAnyHole(SYSTEM_CMD_SELF* selfCommandPtr);
//清洗盘移动到指定孔位
static LH_ERR TaskServiceCommandProc_DishWash_Self_Move2SpecialHole(SYSTEM_CMD_SELF* selfCommandPtr);
//A液混匀初始化
static LH_ERR TaskServiceCommandProc_LiquidAMix_Self_Init(SYSTEM_CMD_SELF* selfCommandPtr);
//A液混匀混匀指定时间
static LH_ERR TaskServiceCommandProc_LiquidAMix_Self_RotateAnyTimes(SYSTEM_CMD_SELF* selfCommandPtr);

//自身指令回调函数数组
static const TaskServicePtrWithSelfCommand TaskProcWithDishWashLiquidAMixSelfCommandArray[] = {
	TaskServiceCommandProc_DishWash_Self_Init,
	TaskServiceCommandProc_DishWash_Self_MoveAnyHole,
	TaskServiceCommandProc_DishWash_Self_Move2SpecialHole,
	TaskServiceCommandProc_LiquidAMix_Self_Init,
	TaskServiceCommandProc_LiquidAMix_Self_RotateAnyTimes,
};

//支持指令的个数
#define TASK_COMMAND_PROC_SELF_COUNT	(sizeof(TaskProcWithDishWashLiquidAMixSelfCommandArray)/sizeof(TaskProcWithDishWashLiquidAMixSelfCommandArray[0]))

//用于调试指令的头调试字符串
static const uint8_t TaskSelfCommandDebugStartMsgHeadString[] = "$$Action 4 Start : ";
static const uint8_t TaskSelfCommandDebugEndMsgHeadString[] = "$$Action 4 End ";

static void TaskServiceCommandProc_DishWashMixLiquidA_Self(SYSTEM_CMD_SELF* selfCommand)
{
    OS_ERR err;
	LH_ERR actionResult;
	//指令过程与结果对象
	TSC_CMD_RESULT_DATA_DISH_WASH_LIQUID_A_MIX* cmdResultPtr = (TSC_CMD_RESULT_DATA_DISH_WASH_LIQUID_A_MIX*)selfCommand->commandDataSpecialChannel;
	
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
		actionResult = TaskProcWithDishWashLiquidAMixSelfCommandArray[selfCommand->commandIndex](selfCommand);
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

//清洗盘初始化
static LH_ERR TaskServiceCommandProc_DishWash_Self_Init(SYSTEM_CMD_SELF* selfCommandPtr)
{
    LH_ERR errCode;
	//打印开始处理信息
	TaskServiceSerialDebugMsgShowStart((uint8_t*)TaskSelfCommandDebugStartMsgHeadString,
										(uint8_t*)"DishWashInit",
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
	ACTION_SENSOR_STATUS dishWashlightSensorCupHasExist;
	uint16_t currentDishWashHoleIndex;

	//指令过程与结果对象
	TSC_CMD_RESULT_DATA_DISH_WASH_LIQUID_A_MIX* cmdResultPtr = 
				(TSC_CMD_RESULT_DATA_DISH_WASH_LIQUID_A_MIX*)selfCommandPtr->commandDataSpecialChannel;

	//处理,调用API复位
	errCode = ActionDishWash_Reset(&currentDishWashHoleIndex,&dishWashlightSensorCupHasExist);
	if(errCode == LH_ERR_NONE)
	{
		cmdResultPtr->currentHoleHasCup = dishWashlightSensorCupHasExist;
		cmdResultPtr->currentHoleIndex = currentDishWashHoleIndex;
	}
#endif

	return errCode;
}

//清洗盘转动指定孔数
static LH_ERR TaskServiceCommandProc_DishWash_Self_MoveAnyHole(SYSTEM_CMD_SELF* selfCommandPtr)
{
    LH_ERR errCode;
	//打印开始处理信息
	TaskServiceSerialDebugMsgShowStart((uint8_t*)TaskSelfCommandDebugStartMsgHeadString,
										(uint8_t*)"DishWashMoveAnyHole",
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
	ACTION_SENSOR_STATUS dishWashlightSensorCupHasExist;
	uint16_t currentDishWashHoleIndex;

	//指令过程与结果对象
	TSC_CMD_RESULT_DATA_DISH_WASH_LIQUID_A_MIX* cmdResultPtr = 
				(TSC_CMD_RESULT_DATA_DISH_WASH_LIQUID_A_MIX*)selfCommandPtr->commandDataSpecialChannel;
	//参数获取
	uint16_t holeOffset = selfCommandPtr->commandParam1;

	//处理
	errCode = ActionDishWash_MoveSpecialCups(holeOffset,&currentDishWashHoleIndex,&dishWashlightSensorCupHasExist);
	if(errCode == LH_ERR_NONE)
	{
		cmdResultPtr->currentHoleHasCup = dishWashlightSensorCupHasExist;
		cmdResultPtr->currentHoleIndex = currentDishWashHoleIndex;
	}
#endif

	return errCode;
}

//清洗盘移动到指定孔位
static LH_ERR TaskServiceCommandProc_DishWash_Self_Move2SpecialHole(SYSTEM_CMD_SELF* selfCommandPtr)
{
    LH_ERR errCode;
	//打印开始处理信息
	TaskServiceSerialDebugMsgShowStart((uint8_t*)TaskSelfCommandDebugStartMsgHeadString,
										(uint8_t*)"DishWashMove2SpecialHole",
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
	ACTION_SENSOR_STATUS dishWashlightSensorCupHasExist;
	uint16_t currentDishWashHoleIndex;

	//指令过程与结果对象
	TSC_CMD_RESULT_DATA_DISH_WASH_LIQUID_A_MIX* cmdResultPtr = 
				(TSC_CMD_RESULT_DATA_DISH_WASH_LIQUID_A_MIX*)selfCommandPtr->commandDataSpecialChannel;
	//参数获取
	uint16_t targetHoleIndex = selfCommandPtr->commandParam1;

	//处理
	errCode = ActionDishWash_Move2SpecialCup(targetHoleIndex,&currentDishWashHoleIndex,&dishWashlightSensorCupHasExist);
	if(errCode == LH_ERR_NONE)
	{
		cmdResultPtr->currentHoleHasCup = dishWashlightSensorCupHasExist;
		cmdResultPtr->currentHoleIndex = currentDishWashHoleIndex;
	}
#endif

	return errCode;
}

//A液混匀初始化
static LH_ERR TaskServiceCommandProc_LiquidAMix_Self_Init(SYSTEM_CMD_SELF* selfCommandPtr)
{
    LH_ERR errCode;
	//打印开始处理信息
	TaskServiceSerialDebugMsgShowStart((uint8_t*)TaskSelfCommandDebugStartMsgHeadString,
										(uint8_t*)"LiquidAMixInit",
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
	errCode = ActionMixLiquidA_Reset();
#endif

	return errCode;
}

//A液混匀混匀指定时间
static LH_ERR TaskServiceCommandProc_LiquidAMix_Self_RotateAnyTimes(SYSTEM_CMD_SELF* selfCommandPtr)
{
    LH_ERR errCode;
	//打印开始处理信息
	TaskServiceSerialDebugMsgShowStart((uint8_t*)TaskSelfCommandDebugStartMsgHeadString,
										(uint8_t*)"LiquidAMixRotateAnyTimes",
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
	//参数获取
	uint16_t rotateTimeMs = selfCommandPtr->commandParam1;
	//处理
	errCode = ActionMixLiquidA_Up();
	if(errCode != LH_ERR_NONE)
	{
		return errCode;
	}
	errCode = ActionMixLiquidA_RotateAnyTimes(rotateTimeMs);
	if(errCode != LH_ERR_NONE)
	{
		return errCode;
	}
	errCode = ActionMixLiquidA_Down();
	if(errCode != LH_ERR_NONE)
	{
		return errCode;
	}
#endif

	return errCode;
}



/********************************************************************************************************************/





