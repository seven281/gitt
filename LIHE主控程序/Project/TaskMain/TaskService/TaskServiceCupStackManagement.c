#include "TaskServiceConfig.h"
#include "TSC_Inc.h"
#include "ActionImplementInc.h"
#include "TestProc_CanConfig.h"
#include "Can1DataType.h"
//任务控制块,8字节对齐
#pragma pack(8)
//任务堆栈
CPU_STK stackBufferTaskServiceCupStack[STK_SIZE_TASK_SERVICE_CUP_STACK];
#pragma pack()

OS_TCB tcbTaskServiceCupStackManagement;

//串口指令处理
static void TaskServiceCommandProc_CupStackManagement_UartRS1(SYSTEM_CMD_UART_RS1* uartCommand);

//CAN口指令处理
static void TaskServiceCommandProc_CupStackManagement_Can1(SYSTEM_CMD_CAN1* canCommand);

//网口指令处理
static void TaskServiceCommandProc_CupStackManagement_NetW5500(SYSTEM_CMD_NET_W5500* netCommand);

//自身指令处理
static void TaskServiceCommandProc_CupStackManagement_Self(SYSTEM_CMD_SELF* selfCommand);

//任务函数
void TaskServiceFuncCupStackManagement(void *p_arg)
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
                TaskServiceCommandProc_CupStackManagement_UartRS1(uartRS1CommandPtr);
                //处置完成,执行后续操作
                UserMemFree(SRAM_CCM,uartRS1CommandPtr);
            }
            else if (SYSTEM_CMD_SRC_SELF == taskSystemRecvCommandPtr->commandSrc)
            {
                //自身指令源
                SYSTEM_CMD_SELF* selfCommandPtr =  (SYSTEM_CMD_SELF*)(taskSystemRecvCommandPtr->systemCommandDataPtr);
                //对接收到的数据进行处理
                TaskServiceCommandProc_CupStackManagement_Self(selfCommandPtr);
                //处置完成,执行后续操作
                UserMemFree(SRAM_CCM,selfCommandPtr);
            }
            else if(SYSTEM_CMD_SRC_NET_W5500 == taskSystemRecvCommandPtr->commandSrc)
            {
                //网口指令源
                SYSTEM_CMD_NET_W5500* netW5500CommandPtr =  (SYSTEM_CMD_NET_W5500*)(taskSystemRecvCommandPtr->systemCommandDataPtr);
                //对接收到的数据进行处理
                TaskServiceCommandProc_CupStackManagement_NetW5500(netW5500CommandPtr);
                //处置完成,执行后续操作
                UserMemFree(SRAM_CCM,netW5500CommandPtr);
            }
            else if(SYSTEM_CMD_SRC_CAN1 == taskSystemRecvCommandPtr->commandSrc)
            {
                //CAN1口指令源
                SYSTEM_CMD_CAN1* can1CommandPtr =  (SYSTEM_CMD_CAN1*)(taskSystemRecvCommandPtr->systemCommandDataPtr);
                //对接收到的数据进行处理
                TaskServiceCommandProc_CupStackManagement_Can1(can1CommandPtr);
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
//新杯栈初始化
static LH_ERR TaskServiceCommandProc_CupStackManagement_UartRS1_NewStackInit(SYSTEM_CMD_UART_RS1* uartCommandPtr,uint16_t* resultData);
//空杯栈初始化
static LH_ERR TaskServiceCommandProc_CupStackManagement_UartRS1_EmptyStackInit(SYSTEM_CMD_UART_RS1* uartCommandPtr,uint16_t* resultData);
//进样推手初始化
static LH_ERR TaskServiceCommandProc_CupStackManagement_UartRS1_PushInit(SYSTEM_CMD_UART_RS1* uartCommandPtr,uint16_t* resultData);
//夹手初始化
static LH_ERR TaskServiceCommandProc_CupStackManagement_UartRS1_ClampInit(SYSTEM_CMD_UART_RS1* uartCommandPtr,uint16_t* resultData);
//四大机构全部初始化
static LH_ERR TaskServiceCommandProc_CupStackManagement_UartRS1_InitAll(SYSTEM_CMD_UART_RS1* uartCommandPtr,uint16_t* resultData);
//夹手打开
static LH_ERR TaskServiceCommandProc_CupStackManagement_UartRS1_ClampOpen(SYSTEM_CMD_UART_RS1* uartCommandPtr,uint16_t* resultData);
//夹手关闭
static LH_ERR TaskServiceCommandProc_CupStackManagement_UartRS1_ClampClose(SYSTEM_CMD_UART_RS1* uartCommandPtr,uint16_t* resultData);
//新杯栈上传一个新盘
static LH_ERR TaskServiceCommandProc_CupStackManagement_UartRS1_NewStackUpload(SYSTEM_CMD_UART_RS1* uartCommandPtr,uint16_t* resultData);
//推手推进一次
static LH_ERR TaskServiceCommandProc_CupStackManagement_UartRS1_PushOnce(SYSTEM_CMD_UART_RS1* uartCommandPtr,uint16_t* resultData);
//空杯栈上升承载新盘
static LH_ERR TaskServiceCommandProc_CupStackManagement_UartRS1_EmptyStackAscend(SYSTEM_CMD_UART_RS1* uartCommandPtr,uint16_t* resultData);
//垃圾桶锁定
static LH_ERR TaskServiceCommandProc_CupStackManagement_UartRS1_GarbageLock(SYSTEM_CMD_UART_RS1* uartCommandPtr,uint16_t* resultData);
//指定垃圾桶解锁
static LH_ERR TaskServiceCommandProc_CupStackManagement_UartRS1_GarbageUnlock(SYSTEM_CMD_UART_RS1* uartCommandPtr,uint16_t* resultData);

//定义函数数组,用于串口处理线程回调
static const TaskServiceProcPtrWithUartCommand taskServiceProcPtrWithUartCommandArray[] = {
	TaskServiceCommandProc_CupStackManagement_UartRS1_NewStackInit,
	TaskServiceCommandProc_CupStackManagement_UartRS1_EmptyStackInit,
	TaskServiceCommandProc_CupStackManagement_UartRS1_PushInit,
	TaskServiceCommandProc_CupStackManagement_UartRS1_ClampInit,
	TaskServiceCommandProc_CupStackManagement_UartRS1_InitAll,
	TaskServiceCommandProc_CupStackManagement_UartRS1_ClampOpen,
	TaskServiceCommandProc_CupStackManagement_UartRS1_ClampClose,
	TaskServiceCommandProc_CupStackManagement_UartRS1_NewStackUpload,
	TaskServiceCommandProc_CupStackManagement_UartRS1_PushOnce,
	TaskServiceCommandProc_CupStackManagement_UartRS1_EmptyStackAscend,
	TaskServiceCommandProc_CupStackManagement_UartRS1_GarbageLock,
	TaskServiceCommandProc_CupStackManagement_UartRS1_GarbageUnlock,
};

#define CUP_STACK_MANAGE_FUNC_UART_COUNT_MAX      (sizeof(taskServiceProcPtrWithUartCommandArray)/sizeof(taskServiceProcPtrWithUartCommandArray[0]))

//系统串口指令处理
static void TaskServiceCommandProc_CupStackManagement_UartRS1(SYSTEM_CMD_UART_RS1* uartCommand)
{
    LH_ERR errorCode = LH_ERR_NONE;
	//发送ACK,创建一条新数据包
	SystemUartRS_ModbusCreatePackage(uartCommand->uartRecvCommandArray,uartCommand->srcDeviceAddr,uartCommand->frameNo,
						uartCommand->funcCode,uartCommand->regAddr,uartCommand->regValue);
	BoardRS_SendBuffer(ACTION_DEBUG_PORT,uartCommand->uartRecvCommandArray,12);

    //调用指令
	uint8_t actionCode= ((uartCommand->regAddr)>>8);
    uint16_t resultData = 0x0000;
	if(actionCode > CUP_STACK_MANAGE_FUNC_UART_COUNT_MAX)
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
		errorCode = taskServiceProcPtrWithUartCommandArray[actionCode-1](uartCommand,&resultData);
	}
	SystemUartRS_ModbusCreatePackage(uartCommand->uartRecvCommandArray, uartCommand->srcDeviceAddr, uartCommand->frameNo,
							0x01, (uint16_t)errorCode, resultData);
	//发送一个填充好的数据包
	BoardRS_SendBuffer(ACTION_DEBUG_PORT,uartCommand->uartRecvCommandArray,12);
}

//新杯栈初始化
static LH_ERR TaskServiceCommandProc_CupStackManagement_UartRS1_NewStackInit(SYSTEM_CMD_UART_RS1* uartCommandPtr,uint16_t* resultData)
{
    LH_ERR errCode;
	ACTION_SENSOR_STATUS newStackTopSensorStatus;
	uint8_t newStackCount;
	errCode = ActionCupStackManagement_NewStackInit(&newStackTopSensorStatus,&newStackCount);
	if(errCode == LH_ERR_NONE)
	{
		//上传传感器讯息
		SystemUartRS_ModbusCreatePackage(uartCommandPtr->uartRecvCommandArray,uartCommandPtr->srcDeviceAddr,uartCommandPtr->frameNo,
                        0X01,((NEW_STACK_TOP_SENSOR)<<8)|(0X06),(((NEW_STACK_TOP_SENSOR)&0xff00))+newStackTopSensorStatus);
		//发送一个填充好的数据包
		BoardRS_SendBuffer(ACTION_DEBUG_PORT,uartCommandPtr->uartRecvCommandArray,12);

		//上传新杯盘数量讯息
		SystemUartRS_ModbusCreatePackage(uartCommandPtr->uartRecvCommandArray,uartCommandPtr->srcDeviceAddr,uartCommandPtr->frameNo,
                        0X01,0x000D,newStackCount);
		//发送一个填充好的数据包
		BoardRS_SendBuffer(ACTION_DEBUG_PORT,uartCommandPtr->uartRecvCommandArray,12);

	}
	return errCode;
}

//空杯栈初始化
static LH_ERR TaskServiceCommandProc_CupStackManagement_UartRS1_EmptyStackInit(SYSTEM_CMD_UART_RS1* uartCommandPtr,uint16_t* resultData)
{
    LH_ERR errCode;
	ACTION_SENSOR_STATUS emptyStackPushOnPosStatus;
	uint8_t emptyStackDiskCount;
	errCode = ActionCupStackManagement_EmptytackInit(&emptyStackPushOnPosStatus,&emptyStackDiskCount);
	if(errCode == LH_ERR_NONE)
	{
		//返回空杯栈数量
		SystemUartRS_ModbusCreatePackage(uartCommandPtr->uartRecvCommandArray,uartCommandPtr->srcDeviceAddr,uartCommandPtr->frameNo,
					0X01,0X010D,emptyStackDiskCount);
		//发送一个填充好的数据包
		BoardRS_SendBuffer(ACTION_DEBUG_PORT,uartCommandPtr->uartRecvCommandArray,12);
		//返回待测区盘子
		SystemUartRS_ModbusCreatePackage(uartCommandPtr->uartRecvCommandArray,uartCommandPtr->srcDeviceAddr,uartCommandPtr->frameNo,
						0X01,((NEW_STACK_PUSH_IN_POS_SENSOR)<<8)|(0X06),(((NEW_STACK_PUSH_IN_POS_SENSOR)&0xff00))+emptyStackPushOnPosStatus);
		//发送一个填充好的数据包
		BoardRS_SendBuffer(ACTION_DEBUG_PORT,uartCommandPtr->uartRecvCommandArray,12);
	}
	return errCode;
}

//进样推手初始化
static LH_ERR TaskServiceCommandProc_CupStackManagement_UartRS1_PushInit(SYSTEM_CMD_UART_RS1* uartCommandPtr,uint16_t* resultData)
{
    LH_ERR errCode;
	errCode = ActionCupStackManagement_PushInit();
	return errCode;
}

//夹手初始化
static LH_ERR TaskServiceCommandProc_CupStackManagement_UartRS1_ClampInit(SYSTEM_CMD_UART_RS1* uartCommandPtr,uint16_t* resultData)
{
    LH_ERR errCode;
	errCode = ActionCupStackManagement_ClampInit();
	return errCode;
}

//四大机构全部初始化
static LH_ERR TaskServiceCommandProc_CupStackManagement_UartRS1_InitAll(SYSTEM_CMD_UART_RS1* uartCommandPtr,uint16_t* resultData)
{
    LH_ERR errCode;

	ACTION_SENSOR_STATUS newStackTopSensorStatus;
	uint8_t newStackCount;

	ACTION_SENSOR_STATUS emptyStackPushOnPosStatus;
	uint8_t emptyStackDiskCount;

	//夹紧初始化
	errCode = ActionCupStackManagement_ClampInit();
	if(errCode != LH_ERR_NONE)
	{
		return errCode;
	}

	//推杆初始化
	errCode = ActionCupStackManagement_PushInit();
	if(errCode != LH_ERR_NONE)
	{
		return errCode;
	}

	//空杯栈初始化
	errCode = ActionCupStackManagement_EmptytackInit(&emptyStackPushOnPosStatus,&emptyStackDiskCount);
	if(errCode == LH_ERR_NONE)
	{
		//返回空杯栈数量
		SystemUartRS_ModbusCreatePackage(uartCommandPtr->uartRecvCommandArray,uartCommandPtr->srcDeviceAddr,uartCommandPtr->frameNo,
					0X01,0X010D,emptyStackDiskCount);
		//发送一个填充好的数据包
		BoardRS_SendBuffer(ACTION_DEBUG_PORT,uartCommandPtr->uartRecvCommandArray,12);
		//返回待测区盘子
		SystemUartRS_ModbusCreatePackage(uartCommandPtr->uartRecvCommandArray,uartCommandPtr->srcDeviceAddr,uartCommandPtr->frameNo,
						0X01,((NEW_STACK_PUSH_IN_POS_SENSOR)<<8)|(0X06),(((NEW_STACK_PUSH_IN_POS_SENSOR)&0xff00))+emptyStackPushOnPosStatus);
		//发送一个填充好的数据包
		BoardRS_SendBuffer(ACTION_DEBUG_PORT,uartCommandPtr->uartRecvCommandArray,12);
	}
	else
	{
		return errCode;
	}

	//新杯栈初始化
	errCode = ActionCupStackManagement_NewStackInit(&newStackTopSensorStatus,&newStackCount);
	if(errCode == LH_ERR_NONE)
	{
		//上传传感器讯息
		SystemUartRS_ModbusCreatePackage(uartCommandPtr->uartRecvCommandArray,uartCommandPtr->srcDeviceAddr,uartCommandPtr->frameNo,
                        0X01,((NEW_STACK_TOP_SENSOR)<<8)|(0X06),(((NEW_STACK_TOP_SENSOR)&0xff00))+newStackTopSensorStatus);
		//发送一个填充好的数据包
		BoardRS_SendBuffer(ACTION_DEBUG_PORT,uartCommandPtr->uartRecvCommandArray,12);

		//上传新杯盘数量讯息
		SystemUartRS_ModbusCreatePackage(uartCommandPtr->uartRecvCommandArray,uartCommandPtr->srcDeviceAddr,uartCommandPtr->frameNo,
                        0X01,0x000D,newStackCount);
		//发送一个填充好的数据包
		BoardRS_SendBuffer(ACTION_DEBUG_PORT,uartCommandPtr->uartRecvCommandArray,12);

	}
	else
	{
		return errCode;
	}
	return errCode;
}

//夹手打开
static LH_ERR TaskServiceCommandProc_CupStackManagement_UartRS1_ClampOpen(SYSTEM_CMD_UART_RS1* uartCommandPtr,uint16_t* resultData)
{
    LH_ERR errCode;
	errCode = ActionCupStackManagement_ClampOpen();
	return errCode;
}

//夹手关闭
static LH_ERR TaskServiceCommandProc_CupStackManagement_UartRS1_ClampClose(SYSTEM_CMD_UART_RS1* uartCommandPtr,uint16_t* resultData)
{
    LH_ERR errCode;
	errCode = ActionCupStackManagement_ClampClose();
	return errCode;
}

//新杯栈上传一个新盘
static LH_ERR TaskServiceCommandProc_CupStackManagement_UartRS1_NewStackUpload(SYSTEM_CMD_UART_RS1* uartCommandPtr,uint16_t* resultData)
{
    LH_ERR errCode;

	ACTION_SENSOR_STATUS newStackTopSensorStatus;
	uint8_t newStackCount;

	errCode = ActionCupStackManagement_NewStackUpload(&newStackTopSensorStatus,&newStackCount);

	if(errCode == LH_ERR_NONE)
	{
		//上传传感器讯息
		SystemUartRS_ModbusCreatePackage(uartCommandPtr->uartRecvCommandArray,uartCommandPtr->srcDeviceAddr,uartCommandPtr->frameNo,
                        0X01,((NEW_STACK_TOP_SENSOR)<<8)|(0X06),(((NEW_STACK_TOP_SENSOR)&0xff00))+newStackTopSensorStatus);
		//发送一个填充好的数据包
		BoardRS_SendBuffer(ACTION_DEBUG_PORT,uartCommandPtr->uartRecvCommandArray,12);

		//上传新杯盘数量讯息
		SystemUartRS_ModbusCreatePackage(uartCommandPtr->uartRecvCommandArray,uartCommandPtr->srcDeviceAddr,uartCommandPtr->frameNo,
                        0X01,0x000D,newStackCount);
		//发送一个填充好的数据包
		BoardRS_SendBuffer(ACTION_DEBUG_PORT,uartCommandPtr->uartRecvCommandArray,12);

	}
	return errCode;
}

//推手推进一次
static LH_ERR TaskServiceCommandProc_CupStackManagement_UartRS1_PushOnce(SYSTEM_CMD_UART_RS1* uartCommandPtr,uint16_t* resultData)
{
    LH_ERR errCode;
	ACTION_SENSOR_STATUS emptyStackPushOnPosStatus;
	errCode = ActionCupStackManagement_PushOneDisk(&emptyStackPushOnPosStatus);

	if(errCode == LH_ERR_NONE)
	{
		//返回待测区盘子
		SystemUartRS_ModbusCreatePackage(uartCommandPtr->uartRecvCommandArray,uartCommandPtr->srcDeviceAddr,uartCommandPtr->frameNo,
						0X01,((NEW_STACK_PUSH_IN_POS_SENSOR)<<8)|(0X06),(((NEW_STACK_PUSH_IN_POS_SENSOR)&0xff00))+emptyStackPushOnPosStatus);
		//发送一个填充好的数据包
		BoardRS_SendBuffer(ACTION_DEBUG_PORT,uartCommandPtr->uartRecvCommandArray,12);
	}
	return errCode;
}

//空杯栈上升承载新盘
static LH_ERR TaskServiceCommandProc_CupStackManagement_UartRS1_EmptyStackAscend(SYSTEM_CMD_UART_RS1* uartCommandPtr,uint16_t* resultData)
{
    LH_ERR errCode;
	ACTION_SENSOR_STATUS emptyStackPushOnPosStatus;
	uint8_t emptyStackDiskCount;

	errCode = ActionCupStackManagement_EmptyStackAscend(&emptyStackPushOnPosStatus,&emptyStackDiskCount);

	if(errCode == LH_ERR_NONE)
	{
		//返回空杯栈数量
		SystemUartRS_ModbusCreatePackage(uartCommandPtr->uartRecvCommandArray,uartCommandPtr->srcDeviceAddr,uartCommandPtr->frameNo,
					0X01,0X010D,emptyStackDiskCount);
		//发送一个填充好的数据包
		BoardRS_SendBuffer(ACTION_DEBUG_PORT,uartCommandPtr->uartRecvCommandArray,12);
        
		//返回待测区盘子
		SystemUartRS_ModbusCreatePackage(uartCommandPtr->uartRecvCommandArray,uartCommandPtr->srcDeviceAddr,uartCommandPtr->frameNo,
						0X01,((NEW_STACK_PUSH_IN_POS_SENSOR)<<8)|(0X06),(((NEW_STACK_PUSH_IN_POS_SENSOR)&0xff00))+emptyStackPushOnPosStatus);
		//发送一个填充好的数据包
		BoardRS_SendBuffer(ACTION_DEBUG_PORT,uartCommandPtr->uartRecvCommandArray,12);
	}
	return errCode;
}

//垃圾桶锁定
static LH_ERR TaskServiceCommandProc_CupStackManagement_UartRS1_GarbageLock(SYSTEM_CMD_UART_RS1* uartCommandPtr,uint16_t* resultData)
{
    uint8_t paramLow = (uint8_t)(uartCommandPtr->regValue);
	LH_ERR errCode;
	if((paramLow&0x01))
	{
		errCode = ActionCupStackManagement_GarbageUnlock(TSC_STACK_MANAGE_GARBAGE_NO1);
	}
	else
	{
		errCode = ActionCupStackManagement_GarbageUnlock(TSC_STACK_MANAGE_GARBAGE_NO2);
	}
	return errCode;
}

//指定垃圾桶解锁
static LH_ERR TaskServiceCommandProc_CupStackManagement_UartRS1_GarbageUnlock(SYSTEM_CMD_UART_RS1* uartCommandPtr,uint16_t* resultData)
{
    uint8_t paramLow = (uint8_t)(uartCommandPtr->regValue);
	LH_ERR errCode;
	if((paramLow&0x01))
	{
		errCode = ActionCupStackManagement_GarbageUnlock(TSC_STACK_MANAGE_GARBAGE_NO1);
	}
	else
	{
		errCode = ActionCupStackManagement_GarbageUnlock(TSC_STACK_MANAGE_GARBAGE_NO2);
	}
	return errCode;
}


/********************************************************************************************************************/

/*****************************************************CAN指令处理*****************************************************/
//反应杯装载新杯栈初始化 
static LH_ERR TaskServiceCommandProc_CupStackManagement_Can1_NewStackInit(SYSTEM_CMD_CAN1* canCommand)
{
	LH_ERR errCode;
	ACTION_SENSOR_STATUS newStackTopSensorStatus;
	uint8_t newStackCount,count;
	errCode = ActionCupStackManagement_NewStackInit(&newStackTopSensorStatus,&newStackCount);

	count = Arraypack_Common(canCommand,0,errCode);

	CanSendDataToCell(TESTPROC_CELL_PLATFORM_NMBR,BOARD_ID_TRACK_COMM,0x0018,canCommand->can1RecvDataBuffer,count);
	return errCode;
}
//空杯栈初始化
static LH_ERR TaskServiceCommandProc_CupStackManagement_Can1_EmptyStackInit(SYSTEM_CMD_CAN1* canCommand)
{
    LH_ERR errCode;
	ACTION_SENSOR_STATUS emptyStackPushOnPosStatus;
	uint8_t emptyStackDiskCount,count;
	errCode = ActionCupStackManagement_EmptytackInit(&emptyStackPushOnPosStatus,&emptyStackDiskCount);
	
	count = Arraypack_Common(canCommand,0,errCode);

    CanSendDataToCell(TESTPROC_CELL_PLATFORM_NMBR,BOARD_ID_TRACK_COMM,0x0018,canCommand->can1RecvDataBuffer,count);
	return errCode;
}
//进样推手初始化
static LH_ERR TaskServiceCommandProc_CupStackManagement_Can1_PushInit(SYSTEM_CMD_CAN1* canCommand)
{
    LH_ERR errCode;
	uint8_t count;
	errCode = ActionCupStackManagement_PushInit();
	count = Arraypack_Common(canCommand,0,errCode);
	CanSendDataToCell(TESTPROC_CELL_PLATFORM_NMBR,BOARD_ID_TRACK_COMM,0x0018,canCommand->can1RecvDataBuffer,count);
	return errCode;
}
//夹手初始化
static LH_ERR TaskServiceCommandProc_CupStackManagement_Can1_ClampInit(SYSTEM_CMD_CAN1* canCommand)
{
    LH_ERR errCode;
	uint8_t count;
	errCode = ActionCupStackManagement_ClampInit();
	count = Arraypack_Common(canCommand,0,errCode);
	CanSendDataToCell(TESTPROC_CELL_PLATFORM_NMBR,BOARD_ID_TRACK_COMM,0x0018,canCommand->can1RecvDataBuffer,count);
	return errCode;
}
//四大机构全部初始化
static LH_ERR TaskServiceCommandProc_CupStackManagement_Can1_InitAll(SYSTEM_CMD_CAN1* canCommand)
{
    LH_ERR errCode;

	uint8_t count;

	ACTION_SENSOR_STATUS newStackTopSensorStatus;
	uint8_t newStackCount;

	ACTION_SENSOR_STATUS emptyStackPushOnPosStatus;
	uint8_t emptyStackDiskCount;

	//夹紧初始化
	errCode = ActionCupStackManagement_ClampInit();
	
	//推杆初始化
	errCode = ActionCupStackManagement_PushInit();
	
	//空杯栈初始化
	errCode = ActionCupStackManagement_EmptytackInit(&emptyStackPushOnPosStatus,&emptyStackDiskCount);

	//新杯栈初始化
	errCode = ActionCupStackManagement_NewStackInit(&newStackTopSensorStatus,&newStackCount);
	
	count = Arraypack_Common(canCommand,0,errCode);
	CanSendDataToCell(TESTPROC_CELL_PLATFORM_NMBR,BOARD_ID_TRACK_COMM,0x0018,canCommand->can1RecvDataBuffer,count);
	return errCode;
}

//夹手打开
static LH_ERR TaskServiceCommandProc_CupStackManagement_Can1_ClampOpen(SYSTEM_CMD_CAN1* canCommand)
{
    LH_ERR errCode;
	uint8_t count;
	errCode = ActionCupStackManagement_ClampOpen();
	count = Arraypack_Common(canCommand,0,errCode);
	CanSendDataToCell(TESTPROC_CELL_PLATFORM_NMBR,BOARD_ID_TRACK_COMM,0x0018,canCommand->can1RecvDataBuffer,count);
	return errCode;
}
//夹手关闭
static LH_ERR TaskServiceCommandProc_CupStackManagement_Can1_ClampClose(SYSTEM_CMD_CAN1* canCommand)
{
    LH_ERR errCode;
	uint8_t count;
	errCode = ActionCupStackManagement_ClampClose();
	count = Arraypack_Common(canCommand,0,errCode);
	CanSendDataToCell(TESTPROC_CELL_PLATFORM_NMBR,BOARD_ID_TRACK_COMM,0x0018,canCommand->can1RecvDataBuffer,count);
	return errCode;
}
//新杯栈上传一个新盘
static LH_ERR TaskServiceCommandProc_CupStackManagement_Can1_NewStackUpload(SYSTEM_CMD_CAN1* canCommand)
{
    LH_ERR errCode;

	ACTION_SENSOR_STATUS newStackTopSensorStatus;
	uint8_t newStackCount,count;

	errCode = ActionCupStackManagement_NewStackUpload(&newStackTopSensorStatus,&newStackCount);
	count = Arraypack_Common(canCommand,0,errCode);
	CanSendDataToCell(TESTPROC_CELL_PLATFORM_NMBR,BOARD_ID_TRACK_COMM,0x0018,canCommand->can1RecvDataBuffer,count);
	
	return errCode;
}
//推手推进一次
static LH_ERR TaskServiceCommandProc_CupStackManagement_Can1_PushOnce(SYSTEM_CMD_CAN1* canCommand)
{
    LH_ERR errCode;
	uint8_t count;
	ACTION_SENSOR_STATUS emptyStackPushOnPosStatus;
	errCode = ActionCupStackManagement_PushOneDisk(&emptyStackPushOnPosStatus);
	count = Arraypack_Common(canCommand,0,errCode);
	CanSendDataToCell(TESTPROC_CELL_PLATFORM_NMBR,BOARD_ID_TRACK_COMM,0x0018,canCommand->can1RecvDataBuffer,count);
	
	return errCode;
}
//空杯栈上升承载新盘
static LH_ERR TaskServiceCommandProc_CupStackManagement_Can1_EmptyStackAscend(SYSTEM_CMD_CAN1* canCommand)
{
    LH_ERR errCode;
	ACTION_SENSOR_STATUS emptyStackPushOnPosStatus;
	uint8_t emptyStackDiskCount,count;

	errCode = ActionCupStackManagement_EmptyStackAscend(&emptyStackPushOnPosStatus,&emptyStackDiskCount);
	count = Arraypack_Common(canCommand,0,errCode);
	CanSendDataToCell(TESTPROC_CELL_PLATFORM_NMBR,BOARD_ID_TRACK_COMM,0x0018,canCommand->can1RecvDataBuffer,count);
	return errCode;
}

//垃圾桶锁定
static LH_ERR TaskServiceCommandProc_CupStackManagement_Can1_GarbageLock(SYSTEM_CMD_CAN1* canCommand)
{
    uint8_t paramLow = canCommand->can1RecvDataBuffer[3];
	uint8_t count;
	LH_ERR errCode;
	if((paramLow&0x01))
	{
		errCode = ActionCupStackManagement_GarbageUnlock(TSC_STACK_MANAGE_GARBAGE_NO1);
	}
	else
	{
		errCode = ActionCupStackManagement_GarbageUnlock(TSC_STACK_MANAGE_GARBAGE_NO2);
	}
	count = Arraypack_Common(canCommand,paramLow,errCode);
	CanSendDataToCell(TESTPROC_CELL_PLATFORM_NMBR,BOARD_ID_TRACK_COMM,0x0018,canCommand->can1RecvDataBuffer,count);
	return errCode;
}

//指定垃圾桶解锁
static LH_ERR TaskServiceCommandProc_CupStackManagement_Can1_GarbageUnlock(SYSTEM_CMD_CAN1* canCommand)
{
    uint8_t paramLow = canCommand->can1RecvDataBuffer[3];
	uint8_t count;
	LH_ERR errCode;
	if((paramLow&0x01))
	{
		errCode = ActionCupStackManagement_GarbageUnlock(TSC_STACK_MANAGE_GARBAGE_NO1);
	}
	else
	{
		errCode = ActionCupStackManagement_GarbageUnlock(TSC_STACK_MANAGE_GARBAGE_NO2);
	}
	count = Arraypack_Common(canCommand,paramLow,errCode);
	CanSendDataToCell(TESTPROC_CELL_PLATFORM_NMBR,BOARD_ID_TRACK_COMM,0x0018,canCommand->can1RecvDataBuffer,count);
	return errCode;
}
//调试指令处理
const APP_TASK_CAN1_PROC_UNIT appTaskCupStackCan1ProcArray[] = {
	{CAN1_COMMAND_FULL_CUP_STACK_RESET          ,        TaskServiceCommandProc_CupStackManagement_Can1_NewStackInit},//反应杯装载新杯栈初始化 
    {CAN1_COMMAND_EMPTY_CUP_STACK_RESET         ,        TaskServiceCommandProc_CupStackManagement_Can1_EmptyStackInit},//反应杯装载空架栈初始化 
    {CAN1_COMMAND_PUSH_HAND_RESET              ,         TaskServiceCommandProc_CupStackManagement_Can1_PushInit},//反应杯装载推手初始化  
    {CAN1_COMMAND_GRIPPER_RESET                 ,        TaskServiceCommandProc_CupStackManagement_Can1_ClampInit},//反应杯装载夹持机构初始化
    {CAN1_COMMAND_STACK_GRIPPER_PUSHHAND_RESET  ,        TaskServiceCommandProc_CupStackManagement_Can1_InitAll},//反应杯装载初始化全部  
    {CAN1_COMMAND_GRIPPER_OPEN                  ,        TaskServiceCommandProc_CupStackManagement_Can1_ClampOpen},//反应杯装载夹手打开   
    {CAN1_COMMAND_GRIPPER_CLOSE                 ,        TaskServiceCommandProc_CupStackManagement_Can1_ClampClose},//反应杯装载夹手夹紧   
    {CAN1_COMMAND_FAR_PUSH_HAND_NEW_CUP         ,        TaskServiceCommandProc_CupStackManagement_Can1_NewStackUpload},//反应杯装载长传一个新杯 
    {CAN1_COMMAND_PUSHONCE_NEW_CUP              ,        TaskServiceCommandProc_CupStackManagement_Can1_PushOnce},//反应杯装载推一次新杯  
    {CAN1_COMMAND_UNDERTAKE_NEW_CUP             ,        TaskServiceCommandProc_CupStackManagement_Can1_EmptyStackAscend},//反应杯装载承接一次空杯 
    {CAN1_COMMAND_TRASH_LOCK                    ,        TaskServiceCommandProc_CupStackManagement_Can1_GarbageLock},//反应杯装载垃圾桶锁定  
    {CAN1_COMMAND_TRASH_UNLOCK                  ,        TaskServiceCommandProc_CupStackManagement_Can1_GarbageUnlock},//反应杯装载垃圾桶解锁  
};

#define COUNT_CUPSTACKMANAGEMENT     (sizeof(appTaskCupStackCan1ProcArray)/sizeof(appTaskCupStackCan1ProcArray[0]))

static void TaskServiceCommandProc_CupStackManagement_Can1(SYSTEM_CMD_CAN1* canCommand)
{
	uint16_t indexUtil = 0;
    for(indexUtil = 0; indexUtil < COUNT_CUPSTACKMANAGEMENT; indexUtil++)
    {
        //指令匹配
        if(canCommand->command == appTaskCupStackCan1ProcArray[indexUtil].command)
        {
            //调用处理函数
            if(appTaskCupStackCan1ProcArray[indexUtil].can1CommandProcFunc != NULL)
            {
                appTaskCupStackCan1ProcArray[indexUtil].can1CommandProcFunc(canCommand);
                break;
            }
        }
    }
}

/********************************************************************************************************************/

/*******************************************************网口指令处理**************************************************/

static void TaskServiceCommandProc_CupStackManagement_NetW5500(SYSTEM_CMD_NET_W5500* netCommand)
{

}

/********************************************************************************************************************/

/*******************************************************自身指令处理**************************************************/
//杯栈管理全部初始化
static LH_ERR TaskServiceCommandProc_CupStackManagement_Self_StackManageInit(SYSTEM_CMD_SELF* selfCommandPtr);
//上传一个新盘
static LH_ERR TaskServiceCommandProc_CupStackManagement_Self_UpLoadNewDisk(SYSTEM_CMD_SELF* selfCommandPtr);
//推一个新盘到测试位
static LH_ERR TaskServiceCommandProc_CupStackManagement_Self_PushNewDisk(SYSTEM_CMD_SELF* selfCommandPtr);
//承载一个空杯盘
static LH_ERR TaskServiceCommandProc_CupStackManagement_Self_AscendEmptyDisk(SYSTEM_CMD_SELF* selfCommandPtr);
//垃圾桶锁定
static LH_ERR TaskServiceCommandProc_CupStackManagement_Self_GarbageLock(SYSTEM_CMD_SELF* selfCommandPtr);
//垃圾桶解锁
static LH_ERR TaskServiceCommandProc_CupStackManagement_Self_GarbageUnlock(SYSTEM_CMD_SELF* selfCommandPtr);
//垃圾桶解锁
static LH_ERR TaskServiceCommandProc_CupStackManagement_Self_DoorControl(SYSTEM_CMD_SELF* selfCommandPtr);

//自身指令回调函数数组
static const TaskServicePtrWithSelfCommand TaskProcWithSelfCupStackManagementCommandArray[] = {
	TaskServiceCommandProc_CupStackManagement_Self_StackManageInit,
	TaskServiceCommandProc_CupStackManagement_Self_UpLoadNewDisk,
	TaskServiceCommandProc_CupStackManagement_Self_PushNewDisk,
	TaskServiceCommandProc_CupStackManagement_Self_AscendEmptyDisk,
	TaskServiceCommandProc_CupStackManagement_Self_GarbageLock,
	TaskServiceCommandProc_CupStackManagement_Self_GarbageUnlock,
	TaskServiceCommandProc_CupStackManagement_Self_DoorControl,
};

//支持指令的个数
#define TASK_COMMAND_PROC_SELF_COUNT	(sizeof(TaskProcWithSelfCupStackManagementCommandArray)/sizeof(TaskProcWithSelfCupStackManagementCommandArray[0]))

//用于调试指令的头调试字符串
static const uint8_t TaskSelfCommandDebugStartMsgHeadString[] = "$$Action 1 Start : ";
static const uint8_t TaskSelfCommandDebugEndMsgHeadString[] = "$$Action 1 End ";

static void TaskServiceCommandProc_CupStackManagement_Self(SYSTEM_CMD_SELF* selfCommand)
{
	OS_ERR err;
	LH_ERR actionResult;
	//指令过程与结果对象
	TSC_CMD_RESULT_DATA_STACK_MANAGE* cmdResultPtr = (TSC_CMD_RESULT_DATA_STACK_MANAGE*)selfCommand->commandDataSpecialChannel;
	
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
		actionResult = TaskProcWithSelfCupStackManagementCommandArray[selfCommand->commandIndex](selfCommand);
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

//杯栈管理全部初始化
static LH_ERR TaskServiceCommandProc_CupStackManagement_Self_StackManageInit(SYSTEM_CMD_SELF* selfCommandPtr)
{
	LH_ERR errCode;
	//打印开始处理信息
	TaskServiceSerialDebugMsgShowStart((uint8_t*)TaskSelfCommandDebugStartMsgHeadString,
										(uint8_t*)"StackManageInit",
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
	ACTION_SENSOR_STATUS newStackTopSensorStatus;
	uint8_t newStackCount;
	ACTION_SENSOR_STATUS emptyStackPushOnPosStatus;
	uint8_t emptyStackDiskCount;

	//指令过程与结果对象
	TSC_CMD_RESULT_DATA_STACK_MANAGE* cmdResultPtr = 
			(TSC_CMD_RESULT_DATA_STACK_MANAGE*)selfCommandPtr->commandDataSpecialChannel;

	errCode = ActionCupStackManagement_ClampInit();
	if(errCode != LH_ERR_NONE)
	{
		return errCode;
	}

	//推杆初始化
	errCode = ActionCupStackManagement_PushInit();
	if(errCode != LH_ERR_NONE)
	{
		return errCode;
	}

	//空杯栈初始化
	errCode = ActionCupStackManagement_EmptytackInit(&emptyStackPushOnPosStatus,&emptyStackDiskCount);
	//更新信息
	if(errCode == LH_ERR_NONE)
	{
		cmdResultPtr->platformDiskWaitTestExist = emptyStackPushOnPosStatus;
		cmdResultPtr->emptyDiskCount = emptyStackDiskCount;
	}
	else
	{
		return errCode;
	}

	//新杯栈初始化
	errCode = ActionCupStackManagement_NewStackInit(&newStackTopSensorStatus,&newStackCount);
	//更新信息
	if(errCode == LH_ERR_NONE)
	{
		cmdResultPtr->newDiskCount = newStackCount;
		cmdResultPtr->platformNewDiskExist = newStackTopSensorStatus;
	}
	else
	{
		return errCode;
	}
#endif

	return errCode;
}

//上传一个新盘
static LH_ERR TaskServiceCommandProc_CupStackManagement_Self_UpLoadNewDisk(SYSTEM_CMD_SELF* selfCommandPtr)
{
	LH_ERR errCode;
	//打印开始处理信息
	TaskServiceSerialDebugMsgShowStart((uint8_t*)TaskSelfCommandDebugStartMsgHeadString,
										(uint8_t*)"UpLoadNewDisk",
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
	ACTION_SENSOR_STATUS newStackTopSensorStatus;
	uint8_t newStackCount;

	//指令过程与结果对象
	TSC_CMD_RESULT_DATA_STACK_MANAGE* cmdResultPtr = 
			(TSC_CMD_RESULT_DATA_STACK_MANAGE*)selfCommandPtr->commandDataSpecialChannel;

	//处理
	errCode = ActionCupStackManagement_NewStackUpload(&newStackTopSensorStatus,&newStackCount);
	//更新数据
	if(errCode == LH_ERR_NONE)
	{
		cmdResultPtr->newDiskCount = newStackCount;
		cmdResultPtr->platformNewDiskExist = newStackTopSensorStatus;
	}

#endif

	return errCode;
}

//推一个新盘到测试位
static LH_ERR TaskServiceCommandProc_CupStackManagement_Self_PushNewDisk(SYSTEM_CMD_SELF* selfCommandPtr)
{
	LH_ERR errCode;
	//打印开始处理信息
	TaskServiceSerialDebugMsgShowStart((uint8_t*)TaskSelfCommandDebugStartMsgHeadString,
										(uint8_t*)"PushNewDisk",
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
	ACTION_SENSOR_STATUS emptyStackPushOnPosStatus;

	//指令过程与结果对象
	TSC_CMD_RESULT_DATA_STACK_MANAGE* cmdResultPtr = 
			(TSC_CMD_RESULT_DATA_STACK_MANAGE*)selfCommandPtr->commandDataSpecialChannel;
	//处理
	errCode = ActionCupStackManagement_PushOneDisk(&emptyStackPushOnPosStatus);
	//更新数据
	if(errCode == LH_ERR_NONE)
	{
		cmdResultPtr->platformDiskWaitTestExist = emptyStackPushOnPosStatus;
	}
#endif

	return errCode;
}

//承载一个空杯盘
static LH_ERR TaskServiceCommandProc_CupStackManagement_Self_AscendEmptyDisk(SYSTEM_CMD_SELF* selfCommandPtr)
{
	LH_ERR errCode;
	//打印开始处理信息
	TaskServiceSerialDebugMsgShowStart((uint8_t*)TaskSelfCommandDebugStartMsgHeadString,
										(uint8_t*)"AscendEmptyDisk",
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
	ACTION_SENSOR_STATUS emptyStackPushOnPosStatus;
	uint8_t emptyStackDiskCount;

	//指令过程与结果对象
	TSC_CMD_RESULT_DATA_STACK_MANAGE* cmdResultPtr = 
			(TSC_CMD_RESULT_DATA_STACK_MANAGE*)selfCommandPtr->commandDataSpecialChannel;

	//处理
	errCode = ActionCupStackManagement_EmptyStackAscend(&emptyStackPushOnPosStatus,&emptyStackDiskCount);

	//信息更新
	if(errCode == LH_ERR_NONE)
	{
		cmdResultPtr->platformDiskWaitTestExist = emptyStackPushOnPosStatus;
		cmdResultPtr->emptyDiskCount = emptyStackDiskCount;
	}
#endif

	return errCode;
}

//垃圾桶锁定
static LH_ERR TaskServiceCommandProc_CupStackManagement_Self_GarbageLock(SYSTEM_CMD_SELF* selfCommandPtr)
{
	LH_ERR errCode;
	//打印开始处理信息
	TaskServiceSerialDebugMsgShowStart((uint8_t*)TaskSelfCommandDebugStartMsgHeadString,
										(uint8_t*)"GarbageLock",
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

	//指令过程与结果对象
	TSC_CMD_RESULT_DATA_STACK_MANAGE* cmdResultPtr = 
			(TSC_CMD_RESULT_DATA_STACK_MANAGE*)selfCommandPtr->commandDataSpecialChannel;
    
	//获取垃圾桶号码
	ACTION_PARAM_STACK_MANAGE_GARBAGE_NO garbageNo = (ACTION_PARAM_STACK_MANAGE_GARBAGE_NO)selfCommandPtr->commandParam1;

	errCode = ActionCupStackManagement_GarbageLock(garbageNo);

#endif

	return errCode;
}

//垃圾桶解锁
static LH_ERR TaskServiceCommandProc_CupStackManagement_Self_GarbageUnlock(SYSTEM_CMD_SELF* selfCommandPtr)
{
	LH_ERR errCode;
	//打印开始处理信息
	TaskServiceSerialDebugMsgShowStart((uint8_t*)TaskSelfCommandDebugStartMsgHeadString,
										(uint8_t*)"GarbageUnlock",
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
	//指令过程与结果对象
	TSC_CMD_RESULT_DATA_STACK_MANAGE* cmdResultPtr = 
			(TSC_CMD_RESULT_DATA_STACK_MANAGE*)selfCommandPtr->commandDataSpecialChannel;
    
	//获取垃圾桶号码
	ACTION_PARAM_STACK_MANAGE_GARBAGE_NO garbageNo = (ACTION_PARAM_STACK_MANAGE_GARBAGE_NO)selfCommandPtr->commandParam1;

	errCode = ActionCupStackManagement_GarbageUnlock(garbageNo);

#endif

	return errCode;
}
//新杯门开关
static LH_ERR TaskServiceCommandProc_CupStackManagement_Self_DoorControl(SYSTEM_CMD_SELF* selfCommandPtr)
{
	LH_ERR errCode;
	//打印开始处理信息
	TaskServiceSerialDebugMsgShowStart((uint8_t*)TaskSelfCommandDebugStartMsgHeadString,
										(uint8_t*)"DoorControl",
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

	//获取开关命令
	MACHINE_DOOR_CMD doorcmd = (MACHINE_DOOR_CMD)selfCommandPtr->commandParam1;

	errCode = ActionCupStackManagement_DoorControl(doorcmd);

#endif

	return errCode;
}
/********************************************************************************************************************/




