#include "TaskServiceConfig.h"
#include "TSC_Inc.h"
#include "ActionImplementInc.h"
#include "TestProc_CanConfig.h"
#include "Can1DataType.h"


//任务控制块,8字节对齐
#pragma pack(8)
//任务堆栈
CPU_STK stackBufferTaskServiceMeasureRoomAndLiquidB[STK_SIZE_TASK_SERVICE_MEASURE_ROOM_AND_LIQUID_B];
#pragma pack()

OS_TCB tcbTaskServiceMeasureRoomAndLiquidB;

//串口指令处理
static void TaskServiceCommandProc_MeasureRoomAndLiquidB_UartRS1(SYSTEM_CMD_UART_RS1* uartCommand);

//CAN口指令处理
static void TaskServiceCommandProc_MeasureRoomAndLiquidB_Can1(SYSTEM_CMD_CAN1* canCommand);

//网口指令处理
static void TaskServiceCommandProc_MeasureRoomAndLiquidB_NetW5500(SYSTEM_CMD_NET_W5500* netCommand);

//自身指令处理
static void TaskServiceCommandProc_MeasureRoomAndLiquidB_Self(SYSTEM_CMD_SELF* selfCommand);


//任务函数
void TaskServiceFuncMeasureRoomAndLiquidB(void *p_arg)
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
                TaskServiceCommandProc_MeasureRoomAndLiquidB_UartRS1(uartRS1CommandPtr);
                //处置完成,执行后续操作
                UserMemFree(SRAM_CCM,uartRS1CommandPtr);
            }
            else if (SYSTEM_CMD_SRC_SELF == taskSystemRecvCommandPtr->commandSrc)
            {
                //自身指令源
                SYSTEM_CMD_SELF* selfCommandPtr =  (SYSTEM_CMD_SELF*)(taskSystemRecvCommandPtr->systemCommandDataPtr);
                //对接收到的数据进行处理
                TaskServiceCommandProc_MeasureRoomAndLiquidB_Self(selfCommandPtr);
                //处置完成,执行后续操作
                UserMemFree(SRAM_CCM,selfCommandPtr);
            }
            else if(SYSTEM_CMD_SRC_NET_W5500 == taskSystemRecvCommandPtr->commandSrc)
            {
                //网口指令源
                SYSTEM_CMD_NET_W5500* netW5500CommandPtr =  (SYSTEM_CMD_NET_W5500*)(taskSystemRecvCommandPtr->systemCommandDataPtr);
                //对接收到的数据进行处理
                TaskServiceCommandProc_MeasureRoomAndLiquidB_NetW5500(netW5500CommandPtr);
                //处置完成,执行后续操作
                UserMemFree(SRAM_CCM,netW5500CommandPtr);
            }
            else if(SYSTEM_CMD_SRC_CAN1 == taskSystemRecvCommandPtr->commandSrc)
            {
                //CAN1口指令源
                SYSTEM_CMD_CAN1* can1CommandPtr =  (SYSTEM_CMD_CAN1*)(taskSystemRecvCommandPtr->systemCommandDataPtr);
                //对接收到的数据进行处理
                TaskServiceCommandProc_MeasureRoomAndLiquidB_Can1(can1CommandPtr);
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
//测量上门函数申明

//上门初始化
static LH_ERR TaskServiceCommandProc_MeasuringRoomUpDoor_UartRS1_Init(SYSTEM_CMD_UART_RS1* uartCommandPtr,uint16_t* resultData);
//上门打开
static LH_ERR TaskServiceCommandProc_MeasuringRoomUpDoor_UartRS1_Open(SYSTEM_CMD_UART_RS1* uartCommandPtr,uint16_t* resultData);
//上门关闭
static LH_ERR TaskServiceCommandProc_MeasuringRoomUpDoor_UartRS1_Close(SYSTEM_CMD_UART_RS1* uartCommandPtr,uint16_t* resultData);
//上门打开到灌注位
static LH_ERR TaskServiceCommandProc_MeasuringRoomUpDoor_UartRS1_OpenLiquid(SYSTEM_CMD_UART_RS1* uartCommandPtr,uint16_t* resultData);
//注B液复位
static LH_ERR TaskServiceCommandProc_LiquidB_UartRS1_PumpInit(SYSTEM_CMD_UART_RS1* uartCommandPtr,uint16_t* resultData);
//B液灌注
static LH_ERR TaskServiceCommandProc_LiquidB_UartRS1_Prime(SYSTEM_CMD_UART_RS1* uartCommandPtr,uint16_t* resultData);
//B液注液
static LH_ERR TaskServiceCommandProc_LiquidB_UartRS1_Inject(SYSTEM_CMD_UART_RS1* uartCommandPtr,uint16_t* resultData);
//上门半开
static LH_ERR TaskServiceCommandProc_MeasuringRoomUpDoor_UartRS1_OpenHalf(SYSTEM_CMD_UART_RS1* uartCommandPtr,uint16_t* resultData);

//定义函数数组,用于串口处理线程回调
static const TaskServiceProcPtrWithUartCommand taskServiceProcPtrWithMeasuringRoomUpDoorUartCommandArray[] = {
	TaskServiceCommandProc_MeasuringRoomUpDoor_UartRS1_Init,
	TaskServiceCommandProc_MeasuringRoomUpDoor_UartRS1_Open,
	TaskServiceCommandProc_MeasuringRoomUpDoor_UartRS1_Close,
	TaskServiceCommandProc_MeasuringRoomUpDoor_UartRS1_OpenLiquid,
	TaskServiceCommandProc_LiquidB_UartRS1_PumpInit,
	TaskServiceCommandProc_LiquidB_UartRS1_Prime,
	TaskServiceCommandProc_LiquidB_UartRS1_Inject,
	TaskServiceCommandProc_MeasuringRoomUpDoor_UartRS1_OpenHalf,
};
//上门回调函数数量
#define MEASURE_ROOM_UP_DOOR_FUNC_UART_COUNT_MAX      (sizeof(taskServiceProcPtrWithMeasuringRoomUpDoorUartCommandArray)/sizeof(taskServiceProcPtrWithMeasuringRoomUpDoorUartCommandArray[0]))


//测量窗函数申明

//测量窗初始化
static LH_ERR TaskServiceCommandProc_MeasuringWindow_UartRS1_Init(SYSTEM_CMD_UART_RS1* uartCommandPtr,uint16_t* resultData);
//测量窗打开
static LH_ERR TaskServiceCommandProc_MeasuringWindow_UartRS1_Open(SYSTEM_CMD_UART_RS1* uartCommandPtr,uint16_t* resultData);
//测量窗关闭
static LH_ERR TaskServiceCommandProc_MeasuringWindow_UartRS1_Close(SYSTEM_CMD_UART_RS1* uartCommandPtr,uint16_t* resultData);
//测量窗暗计数
static LH_ERR TaskServiceCommandProc_MeasuringWindow_UartRS1_ReadValueDark(SYSTEM_CMD_UART_RS1* uartCommandPtr,uint16_t* resultData);
//测量窗读本底
static LH_ERR TaskServiceCommandProc_MeasuringWindow_UartRS1_ReadValueBackGround(SYSTEM_CMD_UART_RS1* uartCommandPtr,uint16_t* resultData);
//测量窗读值
static LH_ERR TaskServiceCommandProc_MeasuringWindow_UartRS1_ReadValue(SYSTEM_CMD_UART_RS1* uartCommandPtr,uint16_t* resultData);

//定义函数数组,用于串口处理线程回调
static const TaskServiceProcPtrWithUartCommand taskServiceProcPtrWithMeasuringWindowUartCommandArray[] = {
	TaskServiceCommandProc_MeasuringWindow_UartRS1_Init,
	TaskServiceCommandProc_MeasuringWindow_UartRS1_Open,
	TaskServiceCommandProc_MeasuringWindow_UartRS1_Close,
	TaskServiceCommandProc_MeasuringWindow_UartRS1_ReadValueDark,
	TaskServiceCommandProc_MeasuringWindow_UartRS1_ReadValueBackGround,
    TaskServiceCommandProc_MeasuringWindow_UartRS1_ReadValue,
};
//测量窗回调函数数量
#define MEASURE_WINDOW_FUNC_UART_COUNT_MAX      (sizeof(taskServiceProcPtrWithMeasuringWindowUartCommandArray)/sizeof(taskServiceProcPtrWithMeasuringWindowUartCommandArray[0]))



static void TaskServiceCommandProc_MeasureRoomAndLiquidB_UartRS1(SYSTEM_CMD_UART_RS1* uartCommand)
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
    if(((uartCommand->regAddr)&0x00ff) == 0x06)
    {
        //测量上门
        if(actionCode > MEASURE_ROOM_UP_DOOR_FUNC_UART_COUNT_MAX)
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
            errorCode = taskServiceProcPtrWithMeasuringRoomUpDoorUartCommandArray[actionCode-1](uartCommand,&resultData);
        }
    }
    else if(((uartCommand->regAddr)&0x00ff) == 0x07)
    {
        //测量窗
         if(actionCode > MEASURE_WINDOW_FUNC_UART_COUNT_MAX)
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
            errorCode = taskServiceProcPtrWithMeasuringWindowUartCommandArray[actionCode-1](uartCommand,&resultData);
        }
    }

	//结果返回
	SystemUartRS_ModbusCreatePackage(uartCommand->uartRecvCommandArray, uartCommand->srcDeviceAddr, uartCommand->frameNo,
							0x01, (uint16_t)errorCode, resultData);
	//发送一个填充好的数据包
	BoardRS_SendBuffer(ACTION_DEBUG_PORT,uartCommand->uartRecvCommandArray,12);
}

//测量上门函数实现
//上门初始化
static LH_ERR TaskServiceCommandProc_MeasuringRoomUpDoor_UartRS1_Init(SYSTEM_CMD_UART_RS1* uartCommandPtr,uint16_t* resultData)
{
    LH_ERR errorCode;
	errorCode = ActionMeasureRoom_Init();
	//执行完成
	return errorCode;
}

//上门打开
static LH_ERR TaskServiceCommandProc_MeasuringRoomUpDoor_UartRS1_Open(SYSTEM_CMD_UART_RS1* uartCommandPtr,uint16_t* resultData)
{
    LH_ERR errorCode;
	errorCode = ActionMeasureRoom_UpDoorOpenFull();
	//执行完成
	return errorCode;
}

//上门关闭
static LH_ERR TaskServiceCommandProc_MeasuringRoomUpDoor_UartRS1_Close(SYSTEM_CMD_UART_RS1* uartCommandPtr,uint16_t* resultData)
{
    LH_ERR errorCode;
	errorCode = ActionMeasureRoom_UpDoorClose();
	//执行完成
	return errorCode;
}

//上门打开到灌注位
static LH_ERR TaskServiceCommandProc_MeasuringRoomUpDoor_UartRS1_OpenLiquid(SYSTEM_CMD_UART_RS1* uartCommandPtr,uint16_t* resultData)
{
    LH_ERR errorCode;
	errorCode = ActionMeasureRoom_UpDoorOpenPrime();
	//执行完成
	return errorCode;
}

//注B液复位
static LH_ERR TaskServiceCommandProc_LiquidB_UartRS1_PumpInit(SYSTEM_CMD_UART_RS1* uartCommandPtr,uint16_t* resultData)
{
    LH_ERR errorCode;
	errorCode = ActionMeasureRoom_Init();
	//执行完成
	return errorCode;
}

//B液灌注
static LH_ERR TaskServiceCommandProc_LiquidB_UartRS1_Prime(SYSTEM_CMD_UART_RS1* uartCommandPtr,uint16_t* resultData)
{
    LH_ERR errorCode;
	errorCode = ActionMeasureRoom_PrimeOnce();
	//执行完成
	return errorCode;
}

//B液注液
static LH_ERR TaskServiceCommandProc_LiquidB_UartRS1_Inject(SYSTEM_CMD_UART_RS1* uartCommandPtr,uint16_t* resultData)
{
    LH_ERR errorCode;
	errorCode = ActionMeasureRoom_InjectOnce();
	//执行完成
	return errorCode;
}

//上门半开
static LH_ERR TaskServiceCommandProc_MeasuringRoomUpDoor_UartRS1_OpenHalf(SYSTEM_CMD_UART_RS1* uartCommandPtr,uint16_t* resultData)
{
    LH_ERR errorCode;
	errorCode = ActionMeasureRoom_UpDoorOpenHalf();
	//执行完成
	return errorCode;
}


//测量窗函数实现
//测量窗初始化
static LH_ERR TaskServiceCommandProc_MeasuringWindow_UartRS1_Init(SYSTEM_CMD_UART_RS1* uartCommandPtr,uint16_t* resultData)
{
    return LH_ERR_NONE;
}

//测量窗打开
static LH_ERR TaskServiceCommandProc_MeasuringWindow_UartRS1_Open(SYSTEM_CMD_UART_RS1* uartCommandPtr,uint16_t* resultData)
{
    return LH_ERR_NONE;
}

//测量窗关闭
static LH_ERR TaskServiceCommandProc_MeasuringWindow_UartRS1_Close(SYSTEM_CMD_UART_RS1* uartCommandPtr,uint16_t* resultData)
{
    return LH_ERR_NONE;
}

//测量暗计数
static LH_ERR TaskServiceCommandProc_MeasuringWindow_UartRS1_ReadValueDark(SYSTEM_CMD_UART_RS1* uartCommandPtr,uint16_t* resultData)
{
    LH_ERR errorCode;
	uint32_t recvLightSensorValue = 0;

    //测值
	errorCode = ActionMeasureRoom_ReadValueDark(actionParam->moduleMeasureRoomAndLiquidBParam.timeMeasureBaseMs,
												&recvLightSensorValue);
	if(errorCode == LH_ERR_NONE)
	{
		//解析接收数据,并发送给上位机 
		SystemUartRS_ModbusCreatePackage(uartCommandPtr->uartRecvCommandArray,uartCommandPtr->srcDeviceAddr,uartCommandPtr->frameNo,
							0x11,(uint16_t)(recvLightSensorValue),(uint16_t)(recvLightSensorValue>>16));
		//发送一个填充好的数据包
		BoardRS_SendBuffer(ACTION_DEBUG_PORT,uartCommandPtr->uartRecvCommandArray,12);
	}
    return errorCode;
}

//测量读本底
static LH_ERR TaskServiceCommandProc_MeasuringWindow_UartRS1_ReadValueBackGround(SYSTEM_CMD_UART_RS1* uartCommandPtr,uint16_t* resultData)
{
    LH_ERR errorCode;
	uint32_t recvLightSensorValue = 0;

    //测值
	errorCode = ActionMeasureRoom_ReadValueBackgroud(actionParam->moduleMeasureRoomAndLiquidBParam.timeMeasureBaseMs,
												&recvLightSensorValue);
	if(errorCode == LH_ERR_NONE)
	{
		//解析接收数据,并发送给上位机 
		SystemUartRS_ModbusCreatePackage(uartCommandPtr->uartRecvCommandArray,uartCommandPtr->srcDeviceAddr,uartCommandPtr->frameNo,
							0x11,(uint16_t)(recvLightSensorValue),(uint16_t)(recvLightSensorValue>>16));
		//发送一个填充好的数据包
		BoardRS_SendBuffer(ACTION_DEBUG_PORT,uartCommandPtr->uartRecvCommandArray,12);
	}
    return errorCode;
}

//测量读值
static LH_ERR TaskServiceCommandProc_MeasuringWindow_UartRS1_ReadValue(SYSTEM_CMD_UART_RS1* uartCommandPtr,uint16_t* resultData)
{
    LH_ERR errorCode;
	uint32_t recvLightSensorValue = 0;

	errorCode = ActionMeasureRoom_ReadValueWithInject(actionParam->moduleMeasureRoomAndLiquidBParam.timeMeasureNormalMs,
												&recvLightSensorValue);
	if(errorCode == LH_ERR_NONE)
	{
		//解析接收数据,并发送给上位机 
		SystemUartRS_ModbusCreatePackage(uartCommandPtr->uartRecvCommandArray,uartCommandPtr->srcDeviceAddr,uartCommandPtr->frameNo,
							0x11,(uint16_t)(recvLightSensorValue),(uint16_t)(recvLightSensorValue>>16));
		//发送一个填充好的数据包
		BoardRS_SendBuffer(ACTION_DEBUG_PORT,uartCommandPtr->uartRecvCommandArray,12);
	}
    return errorCode;
}


/********************************************************************************************************************/

/*****************************************************CAN指令处理*****************************************************/
//测量上门函数实现
//上门初始化
static LH_ERR TaskServiceCommandProc_MeasuringRoomUpDoor_Can1_Init(SYSTEM_CMD_CAN1* canCommand)
{
    LH_ERR errorCode;
	uint8_t count;
	errorCode = ActionMeasureRoom_Init();
	count = Arraypack_Common(canCommand,0,errorCode);
	CanSendDataToCell(TESTPROC_CELL_PLATFORM_NMBR,BOARD_ID_TRACK_COMM,0x0018,canCommand->can1RecvDataBuffer,count);
	return errorCode;
}
//上门打开
static LH_ERR TaskServiceCommandProc_MeasuringRoomUpDoor_Can1_Open(SYSTEM_CMD_CAN1* canCommand)
{
    LH_ERR errorCode;
	uint8_t count;
	errorCode = ActionMeasureRoom_UpDoorOpenFull();
	count = Arraypack_Common(canCommand,0,errorCode);
	CanSendDataToCell(TESTPROC_CELL_PLATFORM_NMBR,BOARD_ID_TRACK_COMM,0x0018,canCommand->can1RecvDataBuffer,count);
	return errorCode;
}
//上门关闭
static LH_ERR TaskServiceCommandProc_MeasuringRoomUpDoor_Can1_Close(SYSTEM_CMD_CAN1* canCommand)
{
    LH_ERR errorCode;
	uint8_t count;
	errorCode = ActionMeasureRoom_UpDoorClose();
	count = Arraypack_Common(canCommand,0,errorCode);
	CanSendDataToCell(TESTPROC_CELL_PLATFORM_NMBR,BOARD_ID_TRACK_COMM,0x0018,canCommand->can1RecvDataBuffer,count);
	return errorCode;
}
//上门打开到灌注位
static LH_ERR TaskServiceCommandProc_MeasuringRoomUpDoor_Can1_OpenLiquid(SYSTEM_CMD_CAN1* canCommand)
{
    LH_ERR errorCode;
	uint8_t count;
	errorCode = ActionMeasureRoom_UpDoorOpenPrime();
	count = Arraypack_Common(canCommand,0,errorCode);
	CanSendDataToCell(TESTPROC_CELL_PLATFORM_NMBR,BOARD_ID_TRACK_COMM,0x0018,canCommand->can1RecvDataBuffer,count);
	return errorCode;
}
//注B液复位
static LH_ERR TaskServiceCommandProc_LiquidB_Can1_PumpInit(SYSTEM_CMD_CAN1* canCommand)
{
    LH_ERR errorCode;
	uint8_t count;
	errorCode = ActionMeasureRoom_Init();
	count = Arraypack_Common(canCommand,0,errorCode);
	CanSendDataToCell(TESTPROC_CELL_PLATFORM_NMBR,BOARD_ID_TRACK_COMM,0x0018,canCommand->can1RecvDataBuffer,count);
	return errorCode;
}
//B液灌注
static LH_ERR TaskServiceCommandProc_LiquidB_Can1_Prime(SYSTEM_CMD_CAN1* canCommand)
{
    LH_ERR errorCode;
	uint8_t count;
	errorCode = ActionMeasureRoom_PrimeOnce();
	count = Arraypack_Common(canCommand,0,errorCode);
	CanSendDataToCell(TESTPROC_CELL_PLATFORM_NMBR,BOARD_ID_TRACK_COMM,0x0018,canCommand->can1RecvDataBuffer,count);
	return errorCode;
}
//B液注液
static LH_ERR TaskServiceCommandProc_LiquidB_Can1_Inject(SYSTEM_CMD_CAN1* canCommand)
{
    LH_ERR errorCode;
	uint8_t count;
	errorCode = ActionMeasureRoom_InjectOnce();
	count = Arraypack_Common(canCommand,0,errorCode);
	CanSendDataToCell(TESTPROC_CELL_PLATFORM_NMBR,BOARD_ID_TRACK_COMM,0x0018,canCommand->can1RecvDataBuffer,count);
	return errorCode;
}
//上门半开
static LH_ERR TaskServiceCommandProc_MeasuringRoomUpDoor_Can1_OpenHalf(SYSTEM_CMD_CAN1* canCommand)
{
    LH_ERR errorCode;
	uint8_t count;
	errorCode = ActionMeasureRoom_UpDoorOpenHalf();
	count = Arraypack_Common(canCommand,0,errorCode);
	CanSendDataToCell(TESTPROC_CELL_PLATFORM_NMBR,BOARD_ID_TRACK_COMM,0x0018,canCommand->can1RecvDataBuffer,count);
	return errorCode;
}
//测量窗函数实现
//测量窗初始化
static LH_ERR TaskServiceCommandProc_MeasuringWindow_Can1_Init(SYSTEM_CMD_CAN1* canCommand)
{
	uint8_t count;
	LH_ERR errorCode = LH_ERR_NONE;
	errorCode = ActionMeasureRoom_WindowReset();
	count = Arraypack_Common(canCommand,0,errorCode);
	CanSendDataToCell(TESTPROC_CELL_PLATFORM_NMBR,BOARD_ID_TRACK_COMM,0x0018,canCommand->can1RecvDataBuffer,count);
    return errorCode;
}

//测量窗打开
static LH_ERR TaskServiceCommandProc_MeasuringWindow_Can1_Open(SYSTEM_CMD_CAN1* canCommand)
{
	uint8_t count;
	LH_ERR errorCode = LH_ERR_NONE;

	errorCode = ActionMeasureRoom_WindowOpen();
	count = Arraypack_Common(canCommand,0,errorCode);
	CanSendDataToCell(TESTPROC_CELL_PLATFORM_NMBR,BOARD_ID_TRACK_COMM,0x0018,canCommand->can1RecvDataBuffer,count);
    return errorCode;
}
//测量窗关闭
static LH_ERR TaskServiceCommandProc_MeasuringWindow_Can1_Close(SYSTEM_CMD_CAN1* canCommand)
{
    uint8_t count;
	LH_ERR errorCode = LH_ERR_NONE;
	errorCode = ActionMeasureRoom_WindowClose();
	count = Arraypack_Common(canCommand,0,errorCode);
	CanSendDataToCell(TESTPROC_CELL_PLATFORM_NMBR,BOARD_ID_TRACK_COMM,0x0018,canCommand->can1RecvDataBuffer,count);
    return errorCode;
}
//测量窗暗计数
static LH_ERR TaskServiceCommandProc_MeasuringWindow_Can1_ReadValueDark(SYSTEM_CMD_CAN1* canCommand)
{
    LH_ERR errorCode;
	uint32_t recvLightSensorValue = 0;
	uint8_t count;
    //测值
	errorCode = ActionMeasureRoom_ReadValue(actionParam->moduleMeasureRoomAndLiquidBParam.timeMeasureBaseMs,MEASURE_FLAG_DARK,&recvLightSensorValue);
//	errorCode = ActionMeasureRoom_ReadValueDark(actionParam->moduleMeasureRoomAndLiquidBParam.timeMeasureBaseMs,
//												&recvLightSensorValue);
	count = Arraypack_Common(canCommand,recvLightSensorValue,errorCode);
	CanSendDataToCell(TESTPROC_CELL_PLATFORM_NMBR,BOARD_ID_TRACK_COMM,0x0018,canCommand->can1RecvDataBuffer,count);
    return errorCode;
}
//测量窗读本底
static LH_ERR TaskServiceCommandProc_MeasuringWindow_Can1_ReadValueBackGround(SYSTEM_CMD_CAN1* canCommand)
{
    LH_ERR errorCode;
	uint32_t recvLightSensorValue = 0;
	uint8_t count;
    //测值
	errorCode = ActionMeasureRoom_ReadValue(actionParam->moduleMeasureRoomAndLiquidBParam.timeMeasureBaseMs,MEASURE_FLAG_BACK,&recvLightSensorValue);
//	errorCode = ActionMeasureRoom_ReadValueBackgroud(actionParam->moduleMeasureRoomAndLiquidBParam.timeMeasureBaseMs,
//												&recvLightSensorValue);
	count = Arraypack_Common(canCommand,recvLightSensorValue,errorCode);
	CanSendDataToCell(TESTPROC_CELL_PLATFORM_NMBR,BOARD_ID_TRACK_COMM,0x0018,canCommand->can1RecvDataBuffer,count);
    return errorCode;
}
//测量窗读值
static LH_ERR TaskServiceCommandProc_MeasuringWindow_Can1_ReadValue(SYSTEM_CMD_CAN1* canCommand)
{
    LH_ERR errorCode;
	uint32_t recvLightSensorValue = 0;
	uint8_t count;
	errorCode = ActionMeasureRoom_ReadValue(actionParam->moduleMeasureRoomAndLiquidBParam.timeMeasureBaseMs,MEASURE_FLAG_READ,&recvLightSensorValue);
//	errorCode = ActionMeasureRoom_ReadValueWithInject(actionParam->moduleMeasureRoomAndLiquidBParam.timeMeasureNormalMs,
//												&recvLightSensorValue);
	count = Arraypack_Common(canCommand,recvLightSensorValue,errorCode);
	CanSendDataToCell(TESTPROC_CELL_PLATFORM_NMBR,BOARD_ID_TRACK_COMM,0x0018,canCommand->can1RecvDataBuffer,count);
    return errorCode;
}


static const APP_TASK_CAN1_PROC_UNIT taskServiceProcPtrWithMeasureRoomCan1CommandArray[] = {
    {CAN1_COMMAND_MEASUREROOM_UP_RESET          ,        TaskServiceCommandProc_MeasuringRoomUpDoor_Can1_Init},//测量室上门初始化
    {CAN1_COMMAND_MEASUREROOM_UP_FULL_OPEN      ,        TaskServiceCommandProc_MeasuringRoomUpDoor_Can1_Open},//测量室上门全开
    {CAN1_COMMAND_MEASUREROOM_UP_HALF_OPEN      ,        TaskServiceCommandProc_MeasuringRoomUpDoor_Can1_OpenHalf},//测量室上门半开
    {CAN1_COMMAND_MEASUREROOM_UP_PRIME_OPEN     ,        TaskServiceCommandProc_MeasuringRoomUpDoor_Can1_OpenLiquid},//测量室上门开到灌注位 
    {CAN1_COMMAND_MEASUREROOM_UP_CLOSE          ,        TaskServiceCommandProc_MeasuringRoomUpDoor_Can1_Close},//测量室上门关闭
    {CAN1_COMMAND_LIQUID_B_RESET                ,        TaskServiceCommandProc_LiquidB_Can1_PumpInit},//B液泵复位
    {CAN1_COMMAND_LIQUID_B_PRIME                ,        TaskServiceCommandProc_LiquidB_Can1_Prime},//B液灌注
    {CAN1_COMMAND_LIQUID_B_INJECT               ,        TaskServiceCommandProc_LiquidB_Can1_Inject},//B液注液
    {CAN1_COMMAND_MEASUREROOM_SIDE_RESET        ,        TaskServiceCommandProc_MeasuringWindow_Can1_Init},//测量室侧窗初始化
    {CAN1_COMMAND_MEASUREROOM_SIDE_OPEN         ,        TaskServiceCommandProc_MeasuringWindow_Can1_Open},//测量室侧窗打开
    {CAN1_COMMAND_MEASUREROOM_SIDE_CLOSE        ,        TaskServiceCommandProc_MeasuringWindow_Can1_Close},//测量室侧窗关闭
    {CAN1_COMMAND_DARK_COUNT                    ,        TaskServiceCommandProc_MeasuringWindow_Can1_ReadValueDark},//暗计数读数
    {CAN1_COMMAND_BASE_COUNT                    ,        TaskServiceCommandProc_MeasuringWindow_Can1_ReadValueBackGround},//本底读数
    {CAN1_COMMAND_METER_COUNT                   ,        TaskServiceCommandProc_MeasuringWindow_Can1_ReadValue},//测光值读数
};

#define MEASURE_ROOM_FUNC_CAN_COUNT_MAX      (sizeof(taskServiceProcPtrWithMeasureRoomCan1CommandArray)/sizeof(taskServiceProcPtrWithMeasureRoomCan1CommandArray[0]))

static void TaskServiceCommandProc_MeasureRoomAndLiquidB_Can1(SYSTEM_CMD_CAN1* canCommand)
{
	uint16_t indexUtil = 0;
    for(indexUtil = 0; indexUtil < MEASURE_ROOM_FUNC_CAN_COUNT_MAX; indexUtil++)
    {
        //指令匹配
        if(canCommand->command == taskServiceProcPtrWithMeasureRoomCan1CommandArray[indexUtil].command)
        {
            //调用处理函数
            if(taskServiceProcPtrWithMeasureRoomCan1CommandArray[indexUtil].can1CommandProcFunc != NULL)
            {
                taskServiceProcPtrWithMeasureRoomCan1CommandArray[indexUtil].can1CommandProcFunc(canCommand);
                break;
            }
        }
    }
}

/********************************************************************************************************************/

/*******************************************************网口指令处理**************************************************/

static void TaskServiceCommandProc_MeasureRoomAndLiquidB_NetW5500(SYSTEM_CMD_NET_W5500* netCommand)
{

}

/********************************************************************************************************************/

/*******************************************************自身指令处理**************************************************/
//测量室初始化
static LH_ERR TaskServiceCommandProc_MeasureRoomAndLiquidB_Self_Init(SYSTEM_CMD_SELF* selfCommandPtr);
//测量室上门全开
static LH_ERR TaskServiceCommandProc_MeasureRoomAndLiquidB_Self_UpDoorOpenFull(SYSTEM_CMD_SELF* selfCommandPtr);
//测量室上门半开
static LH_ERR TaskServiceCommandProc_MeasureRoomAndLiquidB_Self_UpDoorOpenHalf(SYSTEM_CMD_SELF* selfCommandPtr);
//测量室上门关闭
static LH_ERR TaskServiceCommandProc_MeasureRoomAndLiquidB_Self_UpDoorClose(SYSTEM_CMD_SELF* selfCommandPtr);
//测量室B液灌注
static LH_ERR TaskServiceCommandProc_MeasureRoomAndLiquidB_Self_PrimeOnce(SYSTEM_CMD_SELF* selfCommandPtr);
//测量室B液注液
static LH_ERR TaskServiceCommandProc_MeasureRoomAndLiquidB_Self_InjectOnce(SYSTEM_CMD_SELF* selfCommandPtr);
//测量模块读值,带注液
static LH_ERR TaskServiceCommandProc_MeasureRoomAndLiquidB_Self_ReadValueWithInject(SYSTEM_CMD_SELF* selfCommandPtr);
//测量模块读值,不带注液
static LH_ERR TaskServiceCommandProc_MeasureRoomAndLiquidB_Self_ReadValueWithOutInject(SYSTEM_CMD_SELF* selfCommandPtr);
//关闭测量窗
static LH_ERR TaskServiceCommandProc_MeasureRoomAndLiquidB_WindowsClose(SYSTEM_CMD_SELF* selfCommandPtr);
//测量模块读值
static LH_ERR TaskServiceCommandProc_MeasureRoomAndLiquidB_Self_ReadValue(SYSTEM_CMD_SELF* selfCommandPtr);
//自身指令回调函数数组
static const TaskServicePtrWithSelfCommand TaskProcWithMeasureRoomAndLiquidBSelfCommandArray[] = {
	TaskServiceCommandProc_MeasureRoomAndLiquidB_Self_Init,
	TaskServiceCommandProc_MeasureRoomAndLiquidB_Self_UpDoorOpenFull,
	TaskServiceCommandProc_MeasureRoomAndLiquidB_Self_UpDoorOpenHalf,
	TaskServiceCommandProc_MeasureRoomAndLiquidB_Self_UpDoorClose,
	TaskServiceCommandProc_MeasureRoomAndLiquidB_Self_PrimeOnce,
	TaskServiceCommandProc_MeasureRoomAndLiquidB_Self_InjectOnce,
	TaskServiceCommandProc_MeasureRoomAndLiquidB_Self_ReadValueWithInject,
	TaskServiceCommandProc_MeasureRoomAndLiquidB_Self_ReadValueWithOutInject,
	TaskServiceCommandProc_MeasureRoomAndLiquidB_WindowsClose,
	TaskServiceCommandProc_MeasureRoomAndLiquidB_Self_ReadValue,
};

//支持指令的个数
#define TASK_COMMAND_PROC_SELF_COUNT	(sizeof(TaskProcWithMeasureRoomAndLiquidBSelfCommandArray)/sizeof(TaskProcWithMeasureRoomAndLiquidBSelfCommandArray[0]))

//用于调试指令的头调试字符串
static const uint8_t TaskSelfCommandDebugStartMsgHeadString[] = "$$Action 7 Start : ";
static const uint8_t TaskSelfCommandDebugEndMsgHeadString[] = "$$Action 7 End ";


static void TaskServiceCommandProc_MeasureRoomAndLiquidB_Self(SYSTEM_CMD_SELF* selfCommand)
{
    OS_ERR err;
	LH_ERR actionResult;
	//指令过程与结果对象
	TSC_CMD_RESULT_DATA_MEASURE_ROOM_LIQUID_B* cmdResultPtr = (TSC_CMD_RESULT_DATA_MEASURE_ROOM_LIQUID_B*)selfCommand->commandDataSpecialChannel;
	
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
		actionResult = TaskProcWithMeasureRoomAndLiquidBSelfCommandArray[selfCommand->commandIndex](selfCommand);
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

//测量室初始化
static LH_ERR TaskServiceCommandProc_MeasureRoomAndLiquidB_Self_Init(SYSTEM_CMD_SELF* selfCommandPtr)
{
    LH_ERR errCode;
	//打印开始处理信息
	TaskServiceSerialDebugMsgShowStart((uint8_t*)TaskSelfCommandDebugStartMsgHeadString,
										(uint8_t*)"MeasureRoomAndLiquidBInit",
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
	errCode = ActionMeasureRoom_Init();
#endif

	return errCode;
}

//测量室上门全开
static LH_ERR TaskServiceCommandProc_MeasureRoomAndLiquidB_Self_UpDoorOpenFull(SYSTEM_CMD_SELF* selfCommandPtr)
{
    LH_ERR errCode;
	//打印开始处理信息
	TaskServiceSerialDebugMsgShowStart((uint8_t*)TaskSelfCommandDebugStartMsgHeadString,
										(uint8_t*)"MeasureRoomAndLiquidBUpDoorOpenFull",
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
	errCode = ActionMeasureRoom_UpDoorOpenFull();
#endif

	return errCode;
}

//测量室上门半开
static LH_ERR TaskServiceCommandProc_MeasureRoomAndLiquidB_Self_UpDoorOpenHalf(SYSTEM_CMD_SELF* selfCommandPtr)
{
    LH_ERR errCode;
	//打印开始处理信息
	TaskServiceSerialDebugMsgShowStart((uint8_t*)TaskSelfCommandDebugStartMsgHeadString,
										(uint8_t*)"MeasureRoomAndLiquidBUpDoorOpenHalf",
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
	errCode = ActionMeasureRoom_UpDoorOpenHalf();
#endif

	return errCode;
}

//测量室上门关闭
static LH_ERR TaskServiceCommandProc_MeasureRoomAndLiquidB_Self_UpDoorClose(SYSTEM_CMD_SELF* selfCommandPtr)
{
    LH_ERR errCode;
	//打印开始处理信息
	TaskServiceSerialDebugMsgShowStart((uint8_t*)TaskSelfCommandDebugStartMsgHeadString,
										(uint8_t*)"MeasureRoomAndLiquidBUpDoorClose",
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
	errCode = ActionMeasureRoom_UpDoorClose();
#endif

	return errCode;
}

//测量室B液灌注
static LH_ERR TaskServiceCommandProc_MeasureRoomAndLiquidB_Self_PrimeOnce(SYSTEM_CMD_SELF* selfCommandPtr)
{
    LH_ERR errCode;
	//打印开始处理信息
	TaskServiceSerialDebugMsgShowStart((uint8_t*)TaskSelfCommandDebugStartMsgHeadString,
										(uint8_t*)"MeasureRoomAndLiquidBPrimeOnce",
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
	errCode = ActionMeasureRoom_PrimeOnce();
#endif

	return errCode;
}

//测量室B液注液
static LH_ERR TaskServiceCommandProc_MeasureRoomAndLiquidB_Self_InjectOnce(SYSTEM_CMD_SELF* selfCommandPtr)
{
    LH_ERR errCode;
	//打印开始处理信息
	TaskServiceSerialDebugMsgShowStart((uint8_t*)TaskSelfCommandDebugStartMsgHeadString,
										(uint8_t*)"MeasureRoomAndLiquidBInjectOnce",
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
	errCode = ActionMeasureRoom_InjectOnce();
#endif

	return errCode;
}

//测量模块读值,带注液
static LH_ERR TaskServiceCommandProc_MeasureRoomAndLiquidB_Self_ReadValueWithInject(SYSTEM_CMD_SELF* selfCommandPtr)
{
    LH_ERR errCode;
	//打印开始处理信息
	TaskServiceSerialDebugMsgShowStart((uint8_t*)TaskSelfCommandDebugStartMsgHeadString,
										(uint8_t*)"MeasureRoomAndLiquidBReadValueWithInject",
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
	
	uint32_t resultMeasureValue = 0;

	//指令过程与结果对象
	TSC_CMD_RESULT_DATA_MEASURE_ROOM_LIQUID_B* cmdResultPtr = 
				(TSC_CMD_RESULT_DATA_MEASURE_ROOM_LIQUID_B*)selfCommandPtr->commandDataSpecialChannel;

	uint16_t measureTimeMs = selfCommandPtr->commandParam1;

	//处理
	errCode = ActionMeasureRoom_ReadValueWithInject(measureTimeMs,&resultMeasureValue);
	if(errCode == LH_ERR_NONE)
	{
		cmdResultPtr->lastLightSensorReadValue = resultMeasureValue;
	}
#endif

	return errCode;
}

//测量模块读值,不带注液
static LH_ERR TaskServiceCommandProc_MeasureRoomAndLiquidB_Self_ReadValueWithOutInject(SYSTEM_CMD_SELF* selfCommandPtr)
{
    LH_ERR errCode;
	//打印开始处理信息
	TaskServiceSerialDebugMsgShowStart((uint8_t*)TaskSelfCommandDebugStartMsgHeadString,
										(uint8_t*)"MeasureRoomAndLiquidBReadValueWithOutInject",
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
	
	uint32_t resultMeasureValue = 0;

	//指令过程与结果对象
	TSC_CMD_RESULT_DATA_MEASURE_ROOM_LIQUID_B* cmdResultPtr = 
				(TSC_CMD_RESULT_DATA_MEASURE_ROOM_LIQUID_B*)selfCommandPtr->commandDataSpecialChannel;

	uint16_t measureTimeMs = selfCommandPtr->commandParam1;

	//处理
	errCode = ActionMeasureRoom_ReadValueDark(measureTimeMs,&resultMeasureValue);
	if(errCode == LH_ERR_NONE)
	{
		cmdResultPtr->lastLightSensorReadDark = resultMeasureValue;
	}
#endif

	return errCode;
}
//关侧门窗口
static LH_ERR TaskServiceCommandProc_MeasureRoomAndLiquidB_WindowsClose(SYSTEM_CMD_SELF* selfCommandPtr)
{
    LH_ERR errCode;
	//打印开始处理信息
	TaskServiceSerialDebugMsgShowStart((uint8_t*)TaskSelfCommandDebugStartMsgHeadString,
										(uint8_t*)"MeasureRoomAndLiquidBWindowClose",
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
	errCode = ActionMeasureRoom_WindowClose();
#endif

	return errCode;
}
//读取测量值
static LH_ERR TaskServiceCommandProc_MeasureRoomAndLiquidB_Self_ReadValue(SYSTEM_CMD_SELF* selfCommandPtr)
{
    LH_ERR errCode;
	//打印开始处理信息
	TaskServiceSerialDebugMsgShowStart((uint8_t*)TaskSelfCommandDebugStartMsgHeadString,
										(uint8_t*)"MeasureRoomAndLiquidBReadValue",
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
	
	uint32_t resultMeasureValue = 0;

	//指令过程与结果对象
	TSC_CMD_RESULT_DATA_MEASURE_ROOM_LIQUID_B* cmdResultPtr = 
				(TSC_CMD_RESULT_DATA_MEASURE_ROOM_LIQUID_B*)selfCommandPtr->commandDataSpecialChannel;

	uint16_t measureTimeMs = selfCommandPtr->commandParam1;
	MEASURE_VALUE_FLAG measureflag = (MEASURE_VALUE_FLAG)selfCommandPtr->commandParam2;
	//处理
	errCode = ActionMeasureRoom_ReadValue(measureTimeMs,measureflag,&resultMeasureValue);
	if(errCode == LH_ERR_NONE)
	{
		if(measureflag == MEASURE_FLAG_DARK)
			cmdResultPtr->lastLightSensorReadDark = resultMeasureValue;
		else if(measureflag == MEASURE_FLAG_BACK)
			cmdResultPtr->lastLightSensorReadBackGround = resultMeasureValue;
		else if(measureflag == MEASURE_FLAG_READ)
			cmdResultPtr->lastLightSensorReadValue = resultMeasureValue;
	}
#endif

	return errCode;
}
/********************************************************************************************************************/



