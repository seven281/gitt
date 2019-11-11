#include "TaskServiceConfig.h"
#include "TSC_Inc.h"
#include "ActionImplementInc.h"
#include "TestProc_CanConfig.h"
#include "Can1DataType.h"
//任务控制块,8字节对齐
#pragma pack(8)
//任务堆栈
CPU_STK stackBufferTaskServiceHandCup[STK_SIZE_TASK_SERVICE_HAND_CUP];
#pragma pack()

OS_TCB tcbTaskServiceHandCup;

//串口指令处理
static void TaskServiceCommandProc_HandCup_UartRS1(SYSTEM_CMD_UART_RS1* uartCommand);

//CAN口指令处理
static void TaskServiceCommandProc_HandCup_Can1(SYSTEM_CMD_CAN1* canCommand);

//网口指令处理
static void TaskServiceCommandProc_HandCup_NetW5500(SYSTEM_CMD_NET_W5500* netCommand);

//自身指令处理
static void TaskServiceCommandProc_HandCup_Self(SYSTEM_CMD_SELF* selfCommand);

//任务函数
void TaskServiceFuncHandCup(void *p_arg)
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
                TaskServiceCommandProc_HandCup_UartRS1(uartRS1CommandPtr);
                //处置完成,执行后续操作
                UserMemFree(SRAM_CCM,uartRS1CommandPtr);
            }
            else if (SYSTEM_CMD_SRC_SELF == taskSystemRecvCommandPtr->commandSrc)
            {
                //自身指令源
                SYSTEM_CMD_SELF* selfCommandPtr =  (SYSTEM_CMD_SELF*)(taskSystemRecvCommandPtr->systemCommandDataPtr);
                //对接收到的数据进行处理
                TaskServiceCommandProc_HandCup_Self(selfCommandPtr);
                //处置完成,执行后续操作
                UserMemFree(SRAM_CCM,selfCommandPtr);
            }
            else if(SYSTEM_CMD_SRC_NET_W5500 == taskSystemRecvCommandPtr->commandSrc)
            {
                //网口指令源
                SYSTEM_CMD_NET_W5500* netW5500CommandPtr =  (SYSTEM_CMD_NET_W5500*)(taskSystemRecvCommandPtr->systemCommandDataPtr);
                //对接收到的数据进行处理
                TaskServiceCommandProc_HandCup_NetW5500(netW5500CommandPtr);
                //处置完成,执行后续操作
                UserMemFree(SRAM_CCM,netW5500CommandPtr);
            }
            else if(SYSTEM_CMD_SRC_CAN1 == taskSystemRecvCommandPtr->commandSrc)
            {
                //CAN1口指令源
                SYSTEM_CMD_CAN1* can1CommandPtr =  (SYSTEM_CMD_CAN1*)(taskSystemRecvCommandPtr->systemCommandDataPtr);
                //对接收到的数据进行处理
                TaskServiceCommandProc_HandCup_Can1(can1CommandPtr);
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
//机械手升降复位
static LH_ERR TaskServiceCommandProc_HandCup_UartRS1_UpDownReset(SYSTEM_CMD_UART_RS1* uartCommandPtr,uint16_t* resultData);
//机械手左右复位
static LH_ERR TaskServiceCommandProc_HandCup_UartRS1_LeftRightReset(SYSTEM_CMD_UART_RS1* uartCommandPtr,uint16_t* resultData);
//机械手前后复位
static LH_ERR TaskServiceCommandProc_HandCup_UartRS1_FrontBackReset(SYSTEM_CMD_UART_RS1* uartCommandPtr,uint16_t* resultData);
//机械手平面复位
static LH_ERR TaskServiceCommandProc_HandCup_UartRS1_ResetWithOutUpDown(SYSTEM_CMD_UART_RS1* uartCommandPtr,uint16_t* resultData);
//机械手整体复位
static LH_ERR TaskServiceCommandProc_HandCup_UartRS1_ResetAll(SYSTEM_CMD_UART_RS1* uartCommandPtr,uint16_t* resultData);
//机械手移动到反应盘
static LH_ERR TaskServiceCommandProc_HandCup_UartRS1_Move2DishReaction(SYSTEM_CMD_UART_RS1* uartCommandPtr,uint16_t* resultData);
//机械手移动到新杯区
static LH_ERR TaskServiceCommandProc_HandCup_UartRS1_Move2NewCup(SYSTEM_CMD_UART_RS1* uartCommandPtr,uint16_t* resultData);
//机械手移动到垃圾桶
static LH_ERR TaskServiceCommandProc_HandCup_UartRS1_Move2Garbage(SYSTEM_CMD_UART_RS1* uartCommandPtr,uint16_t* resultData);
//机械手移动到清洗盘
static LH_ERR TaskServiceCommandProc_HandCup_UartRS1_Move2DishWash(SYSTEM_CMD_UART_RS1* uartCommandPtr,uint16_t* resultData);
//机械手移动到测量室
static LH_ERR TaskServiceCommandProc_HandCup_UartRS1_Move2MeasureRoom(SYSTEM_CMD_UART_RS1* uartCommandPtr,uint16_t* resultData);
//机械手往反应盘放杯
static LH_ERR TaskServiceCommandProc_HandCup_UartRS1_PutCup2DishReaction(SYSTEM_CMD_UART_RS1* uartCommandPtr,uint16_t* resultData);
//机械手往垃圾桶放杯
static LH_ERR TaskServiceCommandProc_HandCup_UartRS1_PutCup2Garbage(SYSTEM_CMD_UART_RS1* uartCommandPtr,uint16_t* resultData);
//机械手从反应盘取杯
static LH_ERR TaskServiceCommandProc_HandCup_UartRS1_CatchCupFromDishReaction(SYSTEM_CMD_UART_RS1* uartCommandPtr,uint16_t* resultData);
//机械手从新杯栈取杯
static LH_ERR TaskServiceCommandProc_HandCup_UartRS1_CatchCupFromNewStack(SYSTEM_CMD_UART_RS1* uartCommandPtr,uint16_t* resultData);
//机械手往清洗盘放杯
static LH_ERR TaskServiceCommandProc_HandCup_UartRS1_PutCup2DishWash(SYSTEM_CMD_UART_RS1* uartCommandPtr,uint16_t* resultData);
//机械手从清洗盘取杯
static LH_ERR TaskServiceCommandProc_HandCup_UartRS1_CatchCupFromDishWash(SYSTEM_CMD_UART_RS1* uartCommandPtr,uint16_t* resultData);
//机械手往测量室放杯
static LH_ERR TaskServiceCommandProc_HandCup_UartRS1_PutCup2MeasureRoom(SYSTEM_CMD_UART_RS1* uartCommandPtr,uint16_t* resultData);
//机械手从测量室抓杯
static LH_ERR TaskServiceCommandProc_HandCup_UartRS1_CatchCupFromMeasureRoom(SYSTEM_CMD_UART_RS1* uartCommandPtr,uint16_t* resultData);

//定义函数数组,用于串口处理线程回调
static const TaskServiceProcPtrWithUartCommand taskServiceProcPtrWithHandCupUartCommandArray[] = {
	TaskServiceCommandProc_HandCup_UartRS1_UpDownReset,
	TaskServiceCommandProc_HandCup_UartRS1_LeftRightReset,
	TaskServiceCommandProc_HandCup_UartRS1_FrontBackReset,
    TaskServiceCommandProc_HandCup_UartRS1_ResetWithOutUpDown,
    TaskServiceCommandProc_HandCup_UartRS1_ResetAll,
    TaskServiceCommandProc_HandCup_UartRS1_Move2DishReaction,
    TaskServiceCommandProc_HandCup_UartRS1_Move2NewCup,
    TaskServiceCommandProc_HandCup_UartRS1_Move2Garbage,
    TaskServiceCommandProc_HandCup_UartRS1_Move2DishWash,
    TaskServiceCommandProc_HandCup_UartRS1_Move2MeasureRoom,
    TaskServiceCommandProc_HandCup_UartRS1_PutCup2DishReaction,
    TaskServiceCommandProc_HandCup_UartRS1_PutCup2Garbage,
    TaskServiceCommandProc_HandCup_UartRS1_CatchCupFromDishReaction,
    TaskServiceCommandProc_HandCup_UartRS1_CatchCupFromNewStack,
    TaskServiceCommandProc_HandCup_UartRS1_PutCup2DishWash,
    TaskServiceCommandProc_HandCup_UartRS1_CatchCupFromDishWash,
    TaskServiceCommandProc_HandCup_UartRS1_PutCup2MeasureRoom,
    TaskServiceCommandProc_HandCup_UartRS1_CatchCupFromMeasureRoom,
};
//清洗盘回调函数数量
#define HAND_CUP_FUNC_UART_COUNT_MAX      (sizeof(taskServiceProcPtrWithHandCupUartCommandArray)/sizeof(taskServiceProcPtrWithHandCupUartCommandArray[0]))

//串口指令的第一级处理
static void TaskServiceCommandProc_HandCup_UartRS1(SYSTEM_CMD_UART_RS1* uartCommand)
{
    LH_ERR errorCode = LH_ERR_NONE;
	//发送ACK,创建一条新数据包
	SystemUartRS_ModbusCreatePackage(uartCommand->uartRecvCommandArray,uartCommand->srcDeviceAddr,uartCommand->frameNo,
						uartCommand->funcCode,uartCommand->regAddr,uartCommand->regValue);
	BoardRS_SendBuffer(ACTION_DEBUG_PORT,uartCommand->uartRecvCommandArray,12);

    //调用指令
	uint8_t actionCode= ((uartCommand->regAddr)>>8);
    uint16_t resultData = 0x0000;
	if(actionCode > HAND_CUP_FUNC_UART_COUNT_MAX)
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
		errorCode = taskServiceProcPtrWithHandCupUartCommandArray[actionCode-1](uartCommand,&resultData);
	}
	SystemUartRS_ModbusCreatePackage(uartCommand->uartRecvCommandArray, uartCommand->srcDeviceAddr, uartCommand->frameNo,
							0x01, (uint16_t)errorCode, resultData);
	//发送一个填充好的数据包
	BoardRS_SendBuffer(ACTION_DEBUG_PORT,uartCommand->uartRecvCommandArray,12);
}

//机械手升降复位
static LH_ERR TaskServiceCommandProc_HandCup_UartRS1_UpDownReset(SYSTEM_CMD_UART_RS1* uartCommandPtr,uint16_t* resultData)
{
    LH_ERR errorCode;
	errorCode = ActionHand_CupUpDownReset();
	return errorCode;
}

//机械手左右复位
static LH_ERR TaskServiceCommandProc_HandCup_UartRS1_LeftRightReset(SYSTEM_CMD_UART_RS1* uartCommandPtr,uint16_t* resultData)
{
    LH_ERR errorCode;
	errorCode = ActionHand_CupLeftRightReset();
	return errorCode;
}

//机械手前后复位
static LH_ERR TaskServiceCommandProc_HandCup_UartRS1_FrontBackReset(SYSTEM_CMD_UART_RS1* uartCommandPtr,uint16_t* resultData)
{
    LH_ERR errorCode;
	errorCode = ActionHand_CupFrontBackReset();
	return errorCode;
}

//机械手平面复位
static LH_ERR TaskServiceCommandProc_HandCup_UartRS1_ResetWithOutUpDown(SYSTEM_CMD_UART_RS1* uartCommandPtr,uint16_t* resultData)
{
    LH_ERR errorCode;
	errorCode = ActionHand_FlatReset();
	return errorCode;
}

//机械手整体复位
static LH_ERR TaskServiceCommandProc_HandCup_UartRS1_ResetAll(SYSTEM_CMD_UART_RS1* uartCommandPtr,uint16_t* resultData)
{
    LH_ERR errorCode;
	errorCode = ActionHand_SolidReset();
	return errorCode;
}

//机械手移动到反应盘
static LH_ERR TaskServiceCommandProc_HandCup_UartRS1_Move2DishReaction(SYSTEM_CMD_UART_RS1* uartCommandPtr,uint16_t* resultData)
{
    LH_ERR errorCode;
	errorCode = ActionHand_Move2DiskReaction();
	return errorCode;
}

//机械手移动到新杯区
static LH_ERR TaskServiceCommandProc_HandCup_UartRS1_Move2NewCup(SYSTEM_CMD_UART_RS1* uartCommandPtr,uint16_t* resultData)
{
    LH_ERR errorCode;
	uint8_t paramLow = (uint8_t)(uartCommandPtr->regValue);
	uint8_t paramHigh = (uint8_t)((uartCommandPtr->regValue)>>8);

	errorCode = ActionHand_Move2NewCupRegion(paramLow,paramHigh);

	return errorCode;
}

//机械手移动到垃圾桶
static LH_ERR TaskServiceCommandProc_HandCup_UartRS1_Move2Garbage(SYSTEM_CMD_UART_RS1* uartCommandPtr,uint16_t* resultData)
{
    LH_ERR errorCode;
	uint8_t paramLow = (uint8_t)(uartCommandPtr->regValue);

	ACTION_PARAM_STACK_MANAGE_GARBAGE_NO garbageNo;
	if(paramLow == 0)
    {
        garbageNo = TSC_STACK_MANAGE_GARBAGE_NO1;
    }
    else
    {
        garbageNo = TSC_STACK_MANAGE_GARBAGE_NO2;
    }

	errorCode = ActionHand_Move2Garbage(garbageNo);

	return errorCode;
}

//机械手移动到清洗盘
static LH_ERR TaskServiceCommandProc_HandCup_UartRS1_Move2DishWash(SYSTEM_CMD_UART_RS1* uartCommandPtr,uint16_t* resultData)
{
    LH_ERR errorCode;
	errorCode = ActionHand_Move2DiskWash();
	return errorCode;
}

//机械手移动到测量室
static LH_ERR TaskServiceCommandProc_HandCup_UartRS1_Move2MeasureRoom(SYSTEM_CMD_UART_RS1* uartCommandPtr,uint16_t* resultData)
{
    LH_ERR errorCode;
	errorCode = ActionHand_Move2MeasureRoom();
	return errorCode;
}

//机械手往反应盘放杯
static LH_ERR TaskServiceCommandProc_HandCup_UartRS1_PutCup2DishReaction(SYSTEM_CMD_UART_RS1* uartCommandPtr,uint16_t* resultData)
{
    LH_ERR errorCode;
	ACTION_SENSOR_STATUS diskReactionCupHasExist;

	errorCode = ActionHand_PutCup2DiskReaction(&diskReactionCupHasExist);
	if(errorCode == LH_ERR_NONE)
	{
		//返回杯状态
		SystemUartRS_ModbusCreatePackage(uartCommandPtr->uartRecvCommandArray,uartCommandPtr->srcDeviceAddr,uartCommandPtr->frameNo,
						0x01,0x07,(((uint16_t)diskReactionCupHasExist)<<8)|1);
		//发送一个填充好的数据包
		BoardRS_SendBuffer(ACTION_DEBUG_PORT,uartCommandPtr->uartRecvCommandArray,12);
	}

	return errorCode;
}

//机械手往垃圾桶放杯
static LH_ERR TaskServiceCommandProc_HandCup_UartRS1_PutCup2Garbage(SYSTEM_CMD_UART_RS1* uartCommandPtr,uint16_t* resultData)
{
    LH_ERR errorCode;
	uint8_t paramLow = (uint8_t)(uartCommandPtr->regValue);

	ACTION_PARAM_STACK_MANAGE_GARBAGE_NO garbageNo;

	if(paramLow == 0)
    {
        garbageNo = TSC_STACK_MANAGE_GARBAGE_NO1;
    }
    else
    {
        garbageNo = TSC_STACK_MANAGE_GARBAGE_NO2;
    }

	errorCode = ActionHand_PutCup2Garbage(garbageNo);

	return errorCode;
}

//机械手从反应盘取杯
static LH_ERR TaskServiceCommandProc_HandCup_UartRS1_CatchCupFromDishReaction(SYSTEM_CMD_UART_RS1* uartCommandPtr,uint16_t* resultData)
{
    LH_ERR errorCode;
	ACTION_SENSOR_STATUS diskReactionCupHasExist;

	errorCode = ActionHand_CatchCupFromDiskReaction(&diskReactionCupHasExist);

	if(errorCode == LH_ERR_NONE)
	{
		//返回杯状态
		SystemUartRS_ModbusCreatePackage(uartCommandPtr->uartRecvCommandArray,uartCommandPtr->srcDeviceAddr,uartCommandPtr->frameNo,
						0x01,0x07,(((uint16_t)diskReactionCupHasExist)<<8)|1);
		//发送一个填充好的数据包
		BoardRS_SendBuffer(ACTION_DEBUG_PORT,uartCommandPtr->uartRecvCommandArray,12);
	}

	return errorCode;
}

//机械手从新杯栈取杯
static LH_ERR TaskServiceCommandProc_HandCup_UartRS1_CatchCupFromNewStack(SYSTEM_CMD_UART_RS1* uartCommandPtr,uint16_t* resultData)
{
    LH_ERR errorCode;
	errorCode = ActionHand_CatchFromNewCupRegion();
	return errorCode;
}

//机械手往清洗盘放杯
static LH_ERR TaskServiceCommandProc_HandCup_UartRS1_PutCup2DishWash(SYSTEM_CMD_UART_RS1* uartCommandPtr,uint16_t* resultData)
{
    LH_ERR errorCode;

	ACTION_SENSOR_STATUS diskWashCupHasExist;

	errorCode = ActionHand_PutCup2DiskWash(&diskWashCupHasExist);

	if(errorCode == LH_ERR_NONE)
	{
		//返回杯状态
		SystemUartRS_ModbusCreatePackage(uartCommandPtr->uartRecvCommandArray,uartCommandPtr->srcDeviceAddr,uartCommandPtr->frameNo,
						0x01,0x0107,(((uint16_t)diskWashCupHasExist)<<8)|1);
		//发送一个填充好的数据包
		BoardRS_SendBuffer(ACTION_DEBUG_PORT,uartCommandPtr->uartRecvCommandArray,12);
	}

	return errorCode;
}

//机械手从清洗盘取杯
static LH_ERR TaskServiceCommandProc_HandCup_UartRS1_CatchCupFromDishWash(SYSTEM_CMD_UART_RS1* uartCommandPtr,uint16_t* resultData)
{
    LH_ERR errorCode;

	ACTION_SENSOR_STATUS diskWashCupHasExist;

	errorCode = ActionHand_CatchFromDiskWash(&diskWashCupHasExist);

	if(errorCode == LH_ERR_NONE)
	{
		//返回杯状态
		SystemUartRS_ModbusCreatePackage(uartCommandPtr->uartRecvCommandArray,uartCommandPtr->srcDeviceAddr,uartCommandPtr->frameNo,
						0x01,0x0107,(((uint16_t)diskWashCupHasExist)<<8)|1);
		//发送一个填充好的数据包
		BoardRS_SendBuffer(ACTION_DEBUG_PORT,uartCommandPtr->uartRecvCommandArray,12);
	}

	return errorCode;
}

//机械手往测量室放杯
static LH_ERR TaskServiceCommandProc_HandCup_UartRS1_PutCup2MeasureRoom(SYSTEM_CMD_UART_RS1* uartCommandPtr,uint16_t* resultData)
{
    LH_ERR errorCode;

	errorCode = ActionHand_PutCup2MeasureRoom();

	return errorCode;
}

//机械手从测量室抓杯
static LH_ERR TaskServiceCommandProc_HandCup_UartRS1_CatchCupFromMeasureRoom(SYSTEM_CMD_UART_RS1* uartCommandPtr,uint16_t* resultData)
{
    LH_ERR errorCode;

	errorCode = ActionHand_CatchCupFromMeasureRoom();
    
	return errorCode;
}




/********************************************************************************************************************/

/*****************************************************CAN指令处理*****************************************************/

//机械手升降复位
static LH_ERR TaskServiceCommandProc_HandCup_Can1_UpDownReset(SYSTEM_CMD_CAN1* canCommand)
{
    LH_ERR errorCode;
	uint8_t count;
	errorCode = ActionHand_CupUpDownReset();
	count = Arraypack_Common(canCommand,0,errorCode);
	CanSendDataToCell(TESTPROC_CELL_PLATFORM_NMBR,BOARD_ID_TRACK_COMM,0x0018,canCommand->can1RecvDataBuffer,count);
	return errorCode;
}
//机械手左右复位
static LH_ERR TaskServiceCommandProc_HandCup_Can1_LeftRightReset(SYSTEM_CMD_CAN1* canCommand)
{
    LH_ERR errorCode;
	uint8_t count;
	errorCode = ActionHand_CupLeftRightReset();
	count = Arraypack_Common(canCommand,0,errorCode);
	CanSendDataToCell(TESTPROC_CELL_PLATFORM_NMBR,BOARD_ID_TRACK_COMM,0x0018,canCommand->can1RecvDataBuffer,count);
	return errorCode;
}
//机械手前后复位
static LH_ERR TaskServiceCommandProc_HandCup_Can1_FrontBackReset(SYSTEM_CMD_CAN1* canCommand)
{
    LH_ERR errorCode;
	uint8_t count;
	errorCode = ActionHand_CupFrontBackReset();
	count = Arraypack_Common(canCommand,0,errorCode);
	CanSendDataToCell(TESTPROC_CELL_PLATFORM_NMBR,BOARD_ID_TRACK_COMM,0x0018,canCommand->can1RecvDataBuffer,count);
	return errorCode;
}
//机械手平面复位
static LH_ERR TaskServiceCommandProc_HandCup_Can1_ResetWithOutUpDown(SYSTEM_CMD_CAN1* canCommand)
{
    LH_ERR errorCode;
	uint8_t count;
	errorCode = ActionHand_FlatReset();
	count = Arraypack_Common(canCommand,0,errorCode);
	CanSendDataToCell(TESTPROC_CELL_PLATFORM_NMBR,BOARD_ID_TRACK_COMM,0x0018,canCommand->can1RecvDataBuffer,count);
	return errorCode;
}
//机械手整体复位
static LH_ERR TaskServiceCommandProc_HandCup_Can1_ResetAll(SYSTEM_CMD_CAN1* canCommand)
{
    LH_ERR errorCode;
	uint8_t count;
	errorCode = ActionHand_SolidReset();
	count = Arraypack_Common(canCommand,0,errorCode);
	CanSendDataToCell(TESTPROC_CELL_PLATFORM_NMBR,BOARD_ID_TRACK_COMM,0x0018,canCommand->can1RecvDataBuffer,count);
	return errorCode;
}
//机械手移动到反应盘
static LH_ERR TaskServiceCommandProc_HandCup_Can1_Move2DishReaction(SYSTEM_CMD_CAN1* canCommand)
{
    LH_ERR errorCode;
	uint8_t count;
	errorCode = ActionHand_Move2DiskReaction();
	count = Arraypack_Common(canCommand,0,errorCode);
	CanSendDataToCell(TESTPROC_CELL_PLATFORM_NMBR,BOARD_ID_TRACK_COMM,0x0018,canCommand->can1RecvDataBuffer,count);
	return errorCode;
}

//机械手移动到新杯区
static LH_ERR TaskServiceCommandProc_HandCup_Can1_Move2NewCup(SYSTEM_CMD_CAN1* canCommand)
{
    LH_ERR errorCode;
	uint8_t count;
	uint16_t paramLow = (canCommand->can1RecvDataBuffer[1]<<8)|canCommand->can1RecvDataBuffer[0];
	uint16_t paramHigh = (canCommand->can1RecvDataBuffer[3]<<8)|canCommand->can1RecvDataBuffer[2];
	uint32_t param;
	param = (paramHigh<<16)|paramLow;
	errorCode = ActionHand_Move2NewCupRegion(paramLow,paramHigh);
	count = Arraypack_Common(canCommand,param,errorCode);
	CanSendDataToCell(TESTPROC_CELL_PLATFORM_NMBR,BOARD_ID_TRACK_COMM,0x0018,canCommand->can1RecvDataBuffer,count);
	return errorCode;
}
//机械手移动到垃圾桶
static LH_ERR TaskServiceCommandProc_HandCup_Can1_Move2Garbage(SYSTEM_CMD_CAN1* canCommand)
{
    LH_ERR errorCode;
	uint8_t paramLow = canCommand->can1RecvDataBuffer[0];
	uint8_t count;
	ACTION_PARAM_STACK_MANAGE_GARBAGE_NO garbageNo;
	if(paramLow == 0)
    {
        garbageNo = TSC_STACK_MANAGE_GARBAGE_NO1;
    }
    else
    {
        garbageNo = TSC_STACK_MANAGE_GARBAGE_NO2;
    }

	errorCode = ActionHand_Move2Garbage(garbageNo);
	count = Arraypack_Common(canCommand,paramLow,errorCode);
	CanSendDataToCell(TESTPROC_CELL_PLATFORM_NMBR,BOARD_ID_TRACK_COMM,0x0018,canCommand->can1RecvDataBuffer,count);
	return errorCode;
}
//机械手移动到清洗盘
static LH_ERR TaskServiceCommandProc_HandCup_Can1_Move2DishWash(SYSTEM_CMD_CAN1* canCommand)
{
    LH_ERR errorCode;
	uint8_t count;
	errorCode = ActionHand_Move2DiskWash();
	count = Arraypack_Common(canCommand,0,errorCode);
	CanSendDataToCell(TESTPROC_CELL_PLATFORM_NMBR,BOARD_ID_TRACK_COMM,0x0018,canCommand->can1RecvDataBuffer,count);
	return errorCode;
}
//机械手移动到测量室
static LH_ERR TaskServiceCommandProc_HandCup_Can1_Move2MeasureRoom(SYSTEM_CMD_CAN1* canCommand)
{
    LH_ERR errorCode;
	uint8_t count;
	errorCode = ActionHand_Move2MeasureRoom();
	count = Arraypack_Common(canCommand,0,errorCode);
	CanSendDataToCell(TESTPROC_CELL_PLATFORM_NMBR,BOARD_ID_TRACK_COMM,0x0018,canCommand->can1RecvDataBuffer,count);
	return errorCode;
}
//机械手往反应盘放杯
static LH_ERR TaskServiceCommandProc_HandCup_Can1_PutCup2DishReaction(SYSTEM_CMD_CAN1* canCommand)
{
    LH_ERR errorCode;
	ACTION_SENSOR_STATUS diskReactionCupHasExist;
	uint8_t count;
	errorCode = ActionHand_PutCup2DiskReaction(&diskReactionCupHasExist);
	count = Arraypack_Common(canCommand,0,errorCode);
	CanSendDataToCell(TESTPROC_CELL_PLATFORM_NMBR,BOARD_ID_TRACK_COMM,0x0018,canCommand->can1RecvDataBuffer,count);
	return errorCode;
}
//机械手往垃圾桶放杯
static LH_ERR TaskServiceCommandProc_HandCup_Can1_PutCup2Garbage(SYSTEM_CMD_CAN1* canCommand)
{
    LH_ERR errorCode;
	uint8_t paramLow = canCommand->can1RecvDataBuffer[0];
	uint8_t count;
	ACTION_PARAM_STACK_MANAGE_GARBAGE_NO garbageNo;

	if(paramLow == 0)
    {
        garbageNo = TSC_STACK_MANAGE_GARBAGE_NO1;
    }
    else
    {
        garbageNo = TSC_STACK_MANAGE_GARBAGE_NO2;
    }

	errorCode = ActionHand_PutCup2Garbage(garbageNo);
	count = Arraypack_Common(canCommand,paramLow,errorCode);
	CanSendDataToCell(TESTPROC_CELL_PLATFORM_NMBR,BOARD_ID_TRACK_COMM,0x0018,canCommand->can1RecvDataBuffer,count);
	return errorCode;
}
//机械手从反应盘取杯
static LH_ERR TaskServiceCommandProc_HandCup_Can1_CatchCupFromDishReaction(SYSTEM_CMD_CAN1* canCommand)
{
    LH_ERR errorCode;
	ACTION_SENSOR_STATUS diskReactionCupHasExist;
	uint8_t count;
	errorCode = ActionHand_CatchCupFromDiskReaction(&diskReactionCupHasExist);

	count = Arraypack_Common(canCommand,0,errorCode);
	CanSendDataToCell(TESTPROC_CELL_PLATFORM_NMBR,BOARD_ID_TRACK_COMM,0x0018,canCommand->can1RecvDataBuffer,count);	

	return errorCode;
}
//机械手从新杯栈取杯
static LH_ERR TaskServiceCommandProc_HandCup_Can1_CatchCupFromNewStack(SYSTEM_CMD_CAN1* canCommand)
{
    LH_ERR errorCode;
	uint8_t count;
	errorCode = ActionHand_CatchFromNewCupRegion();
	count = Arraypack_Common(canCommand,0,errorCode);
	CanSendDataToCell(TESTPROC_CELL_PLATFORM_NMBR,BOARD_ID_TRACK_COMM,0x0018,canCommand->can1RecvDataBuffer,count);	
	return errorCode;
}
//机械手往清洗盘放杯
static LH_ERR TaskServiceCommandProc_HandCup_Can1_PutCup2DishWash(SYSTEM_CMD_CAN1* canCommand)
{
    LH_ERR errorCode;

	ACTION_SENSOR_STATUS diskWashCupHasExist;
	uint8_t count;
	errorCode = ActionHand_PutCup2DiskWash(&diskWashCupHasExist);
	count = Arraypack_Common(canCommand,0,errorCode);
	CanSendDataToCell(TESTPROC_CELL_PLATFORM_NMBR,BOARD_ID_TRACK_COMM,0x0018,canCommand->can1RecvDataBuffer,count);
	return errorCode;
}
//机械手从清洗盘取杯
static LH_ERR TaskServiceCommandProc_HandCup_Can1_CatchCupFromDishWash(SYSTEM_CMD_CAN1* canCommand)
{
    LH_ERR errorCode;

	ACTION_SENSOR_STATUS diskWashCupHasExist;
	uint8_t count;
	errorCode = ActionHand_CatchFromDiskWash(&diskWashCupHasExist);
	count = Arraypack_Common(canCommand,0,errorCode);
	CanSendDataToCell(TESTPROC_CELL_PLATFORM_NMBR,BOARD_ID_TRACK_COMM,0x0018,canCommand->can1RecvDataBuffer,count);
	return errorCode;
}
//机械手往测量室放杯
static LH_ERR TaskServiceCommandProc_HandCup_Can1_PutCup2MeasureRoom(SYSTEM_CMD_CAN1* canCommand)
{
    LH_ERR errorCode;
	uint8_t count;
	errorCode = ActionHand_PutCup2MeasureRoom();
	count = Arraypack_Common(canCommand,0,errorCode);
	CanSendDataToCell(TESTPROC_CELL_PLATFORM_NMBR,BOARD_ID_TRACK_COMM,0x0018,canCommand->can1RecvDataBuffer,count);
	return errorCode;
}
//机械手从测量室抓杯
static LH_ERR TaskServiceCommandProc_HandCup_Can1_CatchCupFromMeasureRoom(SYSTEM_CMD_CAN1* canCommand)
{
    LH_ERR errorCode;
	uint8_t count;
	errorCode = ActionHand_CatchCupFromMeasureRoom();
    count = Arraypack_Common(canCommand,0,errorCode);
	CanSendDataToCell(TESTPROC_CELL_PLATFORM_NMBR,BOARD_ID_TRACK_COMM,0x0018,canCommand->can1RecvDataBuffer,count);
	return errorCode;
}

//定义函数数组,用于Can处理线程回调
static const APP_TASK_CAN1_PROC_UNIT taskServiceProcPtrWithHandCupCan1CommandArray[] = {
    {CAN1_COMMAND_MANIPULATOR_VERTICAL_RESET          ,        TaskServiceCommandProc_HandCup_Can1_UpDownReset},//垂直复位  
    {CAN1_COMMAND_MANIPULATOR_LEVEL_ABOUT_RESET       ,        TaskServiceCommandProc_HandCup_Can1_LeftRightReset},//水平左右复位
    {CAN1_COMMAND_MANIPULATOR_LEVEL_AROUND_RESET      ,        TaskServiceCommandProc_HandCup_Can1_FrontBackReset},//水平前后复位
    {CAN1_COMMAND_MANIPULATOR_LEVEL_RESET             ,        TaskServiceCommandProc_HandCup_Can1_ResetWithOutUpDown},//水平复位  
    {CAN1_COMMAND_MANIPULATOR_VERTICAL_LEVEL          ,        TaskServiceCommandProc_HandCup_Can1_ResetAll},//垂直水平复位
    {CAN1_COMMAND_MANIPULATOR_MOVE_REACTION           ,        TaskServiceCommandProc_HandCup_Can1_Move2DishReaction},//移到反应盘上方
    {CAN1_COMMAND_MANIPULATOR_NEW_CUP                 ,        TaskServiceCommandProc_HandCup_Can1_Move2NewCup},//移到新杯装载上方
    {CAN1_COMMAND_MANIPULATOR_MOVE_TRASH              ,        TaskServiceCommandProc_HandCup_Can1_Move2Garbage},//移到垃圾桶上方
    {CAN1_COMMAND_MANIPULATOR_CLEAN_TRAY              ,        TaskServiceCommandProc_HandCup_Can1_Move2DishWash},//移到清洗盘上方
    {CAN1_COMMAND_MANIPULATOR_MEASURE_ROOM            ,        TaskServiceCommandProc_HandCup_Can1_Move2MeasureRoom},//移到测量室上方
    {CAN1_COMMAND_MANIPULATOR_REACTION_PUTCUP         ,        TaskServiceCommandProc_HandCup_Can1_PutCup2DishReaction},//向反应盘放杯
    {CAN1_COMMAND_MANIPULATOR_TRASH_PUTCUP            ,        TaskServiceCommandProc_HandCup_Can1_PutCup2Garbage},//向垃圾箱放杯
    {CAN1_COMMAND_MANIPULATOR_REATION_CATCHCUP        ,        TaskServiceCommandProc_HandCup_Can1_CatchCupFromDishReaction},//从反应盘取杯 
    {CAN1_COMMAND_MANIPULATOR_NEW_CATCHCUP            ,        TaskServiceCommandProc_HandCup_Can1_CatchCupFromNewStack},//从新杯区取杯
    {CAN1_COMMAND_MANIPULATOR_CLEAN_PUTCUP            ,        TaskServiceCommandProc_HandCup_Can1_PutCup2DishWash},//向清洗盘放杯
    {CAN1_COMMAND_MANIPULATOR_CLEAN_CATCHCUP          ,        TaskServiceCommandProc_HandCup_Can1_CatchCupFromDishWash},//从清洗盘取杯
    {CAN1_COMMAND_MANIPULATOR_MEASURE_PUTCUP          ,        TaskServiceCommandProc_HandCup_Can1_PutCup2MeasureRoom},//向测量室放杯
    {CAN1_COMMAND_MANIPULATOR_MEASURE_CATCHCUP        ,        TaskServiceCommandProc_HandCup_Can1_CatchCupFromMeasureRoom},//从测量室取杯
};
//清洗盘回调函数数量
#define HAND_CUP_FUNC_CAN_COUNT_MAX      (sizeof(taskServiceProcPtrWithHandCupCan1CommandArray)/sizeof(taskServiceProcPtrWithHandCupCan1CommandArray[0]))

static void TaskServiceCommandProc_HandCup_Can1(SYSTEM_CMD_CAN1* canCommand)
{
	uint16_t indexUtil = 0;
    for(indexUtil = 0; indexUtil < HAND_CUP_FUNC_CAN_COUNT_MAX; indexUtil++)
    {
        //指令匹配
        if(canCommand->command == taskServiceProcPtrWithHandCupCan1CommandArray[indexUtil].command)
        {
            //调用处理函数
            if(taskServiceProcPtrWithHandCupCan1CommandArray[indexUtil].can1CommandProcFunc != NULL)
            {
                taskServiceProcPtrWithHandCupCan1CommandArray[indexUtil].can1CommandProcFunc(canCommand);
                break;
            }
        }
    }
}

/********************************************************************************************************************/

/*******************************************************网口指令处理**************************************************/

static void TaskServiceCommandProc_HandCup_NetW5500(SYSTEM_CMD_NET_W5500* netCommand)
{

}

/********************************************************************************************************************/

/*******************************************************自身指令处理**************************************************/
//抓杯机械手初始化
static LH_ERR TaskServiceCommandProc_HandCup_Self_Init(SYSTEM_CMD_SELF* selfCommandPtr);
//抓杯机械手移动到反应盘
static LH_ERR TaskServiceCommandProc_HandCup_Self_Move2DishReaction(SYSTEM_CMD_SELF* selfCommandPtr);
//抓杯机械手移动到新杯栈
static LH_ERR TaskServiceCommandProc_HandCup_Self_Move2NewDisk(SYSTEM_CMD_SELF* selfCommandPtr);
//抓杯机械手移动到垃圾桶
static LH_ERR TaskServiceCommandProc_HandCup_Self_Move2Garbage(SYSTEM_CMD_SELF* selfCommandPtr);
//抓杯机械手移动到清洗盘
static LH_ERR TaskServiceCommandProc_HandCup_Self_Move2DishWash(SYSTEM_CMD_SELF* selfCommandPtr);
//抓杯机械手移动到测量室
static LH_ERR TaskServiceCommandProc_HandCup_Self_Move2MeasureRoom(SYSTEM_CMD_SELF* selfCommandPtr);
//抓杯机械手往反应盘放杯
static LH_ERR TaskServiceCommandProc_HandCup_Self_PutCup2DishReaction(SYSTEM_CMD_SELF* selfCommandPtr);
//抓杯机械手往垃圾桶放杯
static LH_ERR TaskServiceCommandProc_HandCup_Self_PutCup2Garbage(SYSTEM_CMD_SELF* selfCommandPtr);
//抓杯机械手从反应盘抓杯
static LH_ERR TaskServiceCommandProc_HandCup_Self_CatchCupFromDishReaction(SYSTEM_CMD_SELF* selfCommandPtr);
//抓杯机械手从新杯栈抓杯
static LH_ERR TaskServiceCommandProc_HandCup_Self_CatchCupFromNewDisk(SYSTEM_CMD_SELF* selfCommandPtr);
//抓杯机械手往清洗盘放杯
static LH_ERR TaskServiceCommandProc_HandCup_Self_PutCup2DishWash(SYSTEM_CMD_SELF* selfCommandPtr);
//抓杯机械手从清洗盘抓杯
static LH_ERR TaskServiceCommandProc_HandCup_Self_CatchCupFromDishWash(SYSTEM_CMD_SELF* selfCommandPtr);
//抓杯机械手往测量室放杯
static LH_ERR TaskServiceCommandProc_HandCup_Self_PutCup2MeasureRoom(SYSTEM_CMD_SELF* selfCommandPtr);
//抓杯机械手从测量室抓杯
static LH_ERR TaskServiceCommandProc_HandCup_Self_CatchFromMeasureRoom(SYSTEM_CMD_SELF* selfCommandPtr);

//自身指令回调函数数组
static const TaskServicePtrWithSelfCommand TaskProcWithHandCupSelfCommandArray[] = {
	TaskServiceCommandProc_HandCup_Self_Init,
	TaskServiceCommandProc_HandCup_Self_Move2DishReaction,
	TaskServiceCommandProc_HandCup_Self_Move2NewDisk,
	TaskServiceCommandProc_HandCup_Self_Move2Garbage,
	TaskServiceCommandProc_HandCup_Self_Move2DishWash,
	TaskServiceCommandProc_HandCup_Self_Move2MeasureRoom,
	TaskServiceCommandProc_HandCup_Self_PutCup2DishReaction,
	TaskServiceCommandProc_HandCup_Self_PutCup2Garbage,
	TaskServiceCommandProc_HandCup_Self_CatchCupFromDishReaction,
	TaskServiceCommandProc_HandCup_Self_CatchCupFromNewDisk,
	TaskServiceCommandProc_HandCup_Self_PutCup2DishWash,
	TaskServiceCommandProc_HandCup_Self_CatchCupFromDishWash,
	TaskServiceCommandProc_HandCup_Self_PutCup2MeasureRoom,
	TaskServiceCommandProc_HandCup_Self_CatchFromMeasureRoom,
};

//支持指令的个数
#define TASK_COMMAND_PROC_SELF_COUNT	(sizeof(TaskProcWithHandCupSelfCommandArray)/sizeof(TaskProcWithHandCupSelfCommandArray[0]))

//用于调试指令的头调试字符串
static const uint8_t TaskSelfCommandDebugStartMsgHeadString[] = "$$Action 5 Start : ";
static const uint8_t TaskSelfCommandDebugEndMsgHeadString[] = "$$Action 5 End ";

static void TaskServiceCommandProc_HandCup_Self(SYSTEM_CMD_SELF* selfCommand)
{
    OS_ERR err;
	LH_ERR actionResult;
	//指令过程与结果对象
	TSC_CMD_RESULT_DATA_HAND_CUP* cmdResultPtr = (TSC_CMD_RESULT_DATA_HAND_CUP*)selfCommand->commandDataSpecialChannel;
	
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
		actionResult = TaskProcWithHandCupSelfCommandArray[selfCommand->commandIndex](selfCommand);
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

//抓杯机械手初始化
static LH_ERR TaskServiceCommandProc_HandCup_Self_Init(SYSTEM_CMD_SELF* selfCommandPtr)
{
    LH_ERR errCode;
	//打印开始处理信息
	TaskServiceSerialDebugMsgShowStart((uint8_t*)TaskSelfCommandDebugStartMsgHeadString,
										(uint8_t*)"HandCupInit",
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
	errCode = ActionHand_SolidReset();
#endif

	return errCode;
}

//抓杯机械手移动到反应盘
static LH_ERR TaskServiceCommandProc_HandCup_Self_Move2DishReaction(SYSTEM_CMD_SELF* selfCommandPtr)
{
    LH_ERR errCode;
	//打印开始处理信息
	TaskServiceSerialDebugMsgShowStart((uint8_t*)TaskSelfCommandDebugStartMsgHeadString,
										(uint8_t*)"HandCupMove2DishReaction",
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
	errCode = ActionHand_Move2DiskReaction();
#endif

	return errCode;
}

//抓杯机械手移动到新杯栈
static LH_ERR TaskServiceCommandProc_HandCup_Self_Move2NewDisk(SYSTEM_CMD_SELF* selfCommandPtr)
{
    LH_ERR errCode;
	//打印开始处理信息
	TaskServiceSerialDebugMsgShowStart((uint8_t*)TaskSelfCommandDebugStartMsgHeadString,
										(uint8_t*)"HandCupMove2NewDisk",
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
	uint16_t row = selfCommandPtr->commandParam1;
	uint16_t col = selfCommandPtr->commandParam2;

	//处理
	errCode = ActionHand_Move2NewCupRegion(row,col);
#endif

	return errCode;
}

//抓杯机械手移动到垃圾桶
static LH_ERR TaskServiceCommandProc_HandCup_Self_Move2Garbage(SYSTEM_CMD_SELF* selfCommandPtr)
{
    LH_ERR errCode;
	//打印开始处理信息
	TaskServiceSerialDebugMsgShowStart((uint8_t*)TaskSelfCommandDebugStartMsgHeadString,
										(uint8_t*)"HandCupMove2Garbage",
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
	ACTION_PARAM_STACK_MANAGE_GARBAGE_NO garbageNo = (ACTION_PARAM_STACK_MANAGE_GARBAGE_NO)(selfCommandPtr->commandParam1);
	//处理
	errCode = ActionHand_Move2Garbage(garbageNo);
#endif

	return errCode;
}

//抓杯机械手移动到清洗盘
static LH_ERR TaskServiceCommandProc_HandCup_Self_Move2DishWash(SYSTEM_CMD_SELF* selfCommandPtr)
{
    LH_ERR errCode;
	//打印开始处理信息
	TaskServiceSerialDebugMsgShowStart((uint8_t*)TaskSelfCommandDebugStartMsgHeadString,
										(uint8_t*)"HandCupMove2DishWash",
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
	errCode = ActionHand_Move2DiskWash();
#endif

	return errCode;
}

//抓杯机械手移动到测量室
static LH_ERR TaskServiceCommandProc_HandCup_Self_Move2MeasureRoom(SYSTEM_CMD_SELF* selfCommandPtr)
{
    LH_ERR errCode;
	//打印开始处理信息
	TaskServiceSerialDebugMsgShowStart((uint8_t*)TaskSelfCommandDebugStartMsgHeadString,
										(uint8_t*)"HandCupMove2MeasureRoom",
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
	errCode = ActionHand_Move2MeasureRoom();
#endif

	return errCode;
}

//抓杯机械手往反应盘放杯
static LH_ERR TaskServiceCommandProc_HandCup_Self_PutCup2DishReaction(SYSTEM_CMD_SELF* selfCommandPtr)
{
    LH_ERR errCode;
	//打印开始处理信息
	TaskServiceSerialDebugMsgShowStart((uint8_t*)TaskSelfCommandDebugStartMsgHeadString,
										(uint8_t*)"HandCupPutCup2DishReaction",
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
	
	ACTION_SENSOR_STATUS diskReactionCupHasExist;

	TSC_CMD_RESULT_DATA_HAND_CUP* cmdResultPtr = 
					(TSC_CMD_RESULT_DATA_HAND_CUP*)selfCommandPtr->commandDataSpecialChannel;

	//处理
	errCode = ActionHand_PutCup2DiskReaction(&diskReactionCupHasExist);
	if(errCode == LH_ERR_NONE)
	{
		cmdResultPtr->dishReactionHasCup = diskReactionCupHasExist;
	}
#endif

	return errCode;
}

//抓杯机械手往垃圾桶放杯
static LH_ERR TaskServiceCommandProc_HandCup_Self_PutCup2Garbage(SYSTEM_CMD_SELF* selfCommandPtr)
{
    LH_ERR errCode;
	//打印开始处理信息
	TaskServiceSerialDebugMsgShowStart((uint8_t*)TaskSelfCommandDebugStartMsgHeadString,
										(uint8_t*)"HandCupPutCup2Garbage",
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
	ACTION_PARAM_STACK_MANAGE_GARBAGE_NO garbageNo = (ACTION_PARAM_STACK_MANAGE_GARBAGE_NO)(selfCommandPtr->commandParam1);
	//处理
	errCode = ActionHand_PutCup2Garbage(garbageNo);

#endif

	return errCode;
}

//抓杯机械手从反应盘抓杯
static LH_ERR TaskServiceCommandProc_HandCup_Self_CatchCupFromDishReaction(SYSTEM_CMD_SELF* selfCommandPtr)
{
    LH_ERR errCode;
	//打印开始处理信息
	TaskServiceSerialDebugMsgShowStart((uint8_t*)TaskSelfCommandDebugStartMsgHeadString,
										(uint8_t*)"HandCupCatchCupFromDishReaction",
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
	
	ACTION_SENSOR_STATUS diskReactionCupHasExist;

	TSC_CMD_RESULT_DATA_HAND_CUP* cmdResultPtr = 
					(TSC_CMD_RESULT_DATA_HAND_CUP*)selfCommandPtr->commandDataSpecialChannel;

	//处理
	errCode = ActionHand_CatchCupFromDiskReaction(&diskReactionCupHasExist);
	if(errCode == LH_ERR_NONE)
	{
		cmdResultPtr->dishReactionHasCup = diskReactionCupHasExist;
	}
#endif

	return errCode;
}

//抓杯机械手从新杯栈抓杯
static LH_ERR TaskServiceCommandProc_HandCup_Self_CatchCupFromNewDisk(SYSTEM_CMD_SELF* selfCommandPtr)
{
    LH_ERR errCode;
	//打印开始处理信息
	TaskServiceSerialDebugMsgShowStart((uint8_t*)TaskSelfCommandDebugStartMsgHeadString,
										(uint8_t*)"HandCupCatchCupFromNewDisk",
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
	errCode = ActionHand_CatchFromNewCupRegion();
#endif

	return errCode;
}

//抓杯机械手往清洗盘放杯
static LH_ERR TaskServiceCommandProc_HandCup_Self_PutCup2DishWash(SYSTEM_CMD_SELF* selfCommandPtr)
{
    LH_ERR errCode;
	//打印开始处理信息
	TaskServiceSerialDebugMsgShowStart((uint8_t*)TaskSelfCommandDebugStartMsgHeadString,
										(uint8_t*)"HandCupPutCup2DishWash",
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
	ACTION_SENSOR_STATUS diskWashCupHasExist;

	TSC_CMD_RESULT_DATA_HAND_CUP* cmdResultPtr = 
					(TSC_CMD_RESULT_DATA_HAND_CUP*)selfCommandPtr->commandDataSpecialChannel;

	//处理
	errCode = ActionHand_PutCup2DiskWash(&diskWashCupHasExist);

	if(errCode == LH_ERR_NONE)
	{
		cmdResultPtr->dishWashHasCup = diskWashCupHasExist;
	}
#endif

	return errCode;
}

//抓杯机械手从清洗盘抓杯
static LH_ERR TaskServiceCommandProc_HandCup_Self_CatchCupFromDishWash(SYSTEM_CMD_SELF* selfCommandPtr)
{
    LH_ERR errCode;
	//打印开始处理信息
	TaskServiceSerialDebugMsgShowStart((uint8_t*)TaskSelfCommandDebugStartMsgHeadString,
										(uint8_t*)"HandCupCatchCupFromDishWash",
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
	ACTION_SENSOR_STATUS diskWashCupHasExist;
	
	TSC_CMD_RESULT_DATA_HAND_CUP* cmdResultPtr = 
					(TSC_CMD_RESULT_DATA_HAND_CUP*)selfCommandPtr->commandDataSpecialChannel;

	//处理
	errCode = ActionHand_CatchFromDiskWash(&diskWashCupHasExist);

	if(errCode == LH_ERR_NONE)
	{
		cmdResultPtr->dishWashHasCup = diskWashCupHasExist;
	}
#endif

	return errCode;
}

//抓杯机械手往测量室放杯
static LH_ERR TaskServiceCommandProc_HandCup_Self_PutCup2MeasureRoom(SYSTEM_CMD_SELF* selfCommandPtr)
{
    LH_ERR errCode;
	//打印开始处理信息
	TaskServiceSerialDebugMsgShowStart((uint8_t*)TaskSelfCommandDebugStartMsgHeadString,
										(uint8_t*)"HandCupPutCup2MeasureRoom",
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
	errCode = ActionHand_PutCup2MeasureRoom();
#endif

	return errCode;
}

//抓杯机械手从测量室抓杯
static LH_ERR TaskServiceCommandProc_HandCup_Self_CatchFromMeasureRoom(SYSTEM_CMD_SELF* selfCommandPtr)
{
    LH_ERR errCode;
	//打印开始处理信息
	TaskServiceSerialDebugMsgShowStart((uint8_t*)TaskSelfCommandDebugStartMsgHeadString,
										(uint8_t*)"HandCupCatchFromMeasureRoom",
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
	errCode = ActionHand_CatchCupFromMeasureRoom();
#endif

	return errCode;
}


/********************************************************************************************************************/




