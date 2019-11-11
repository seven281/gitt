#include "TaskServiceConfig.h"
#include "TSC_Inc.h"
#include "ActionImplementInc.h"
#include "TestProc_CanConfig.h"
#include "Can1DataType.h"

//任务控制块,8字节对齐
#pragma pack(8)
//任务堆栈
CPU_STK stackBufferTaskServiceNeedleReagentBead[STK_SIZE_TASK_SERVICE_NEEDLE_REAGENT_BEAD];
#pragma pack()

OS_TCB tcbTaskServiceNeedleReagentBead;


//串口指令处理
static void TaskServiceCommandProc_NeedleReagentBead_UartRS1(SYSTEM_CMD_UART_RS1* uartCommand);

//CAN口指令处理
static void TaskServiceCommandProc_NeedleReagentBead_Can1(SYSTEM_CMD_CAN1* canCommand);

//网口指令处理
static void TaskServiceCommandProc_NeedleReagentBead_NetW5500(SYSTEM_CMD_NET_W5500* netCommand);

//自身指令处理
static void TaskServiceCommandProc_NeedleReagentBead_Self(SYSTEM_CMD_SELF* selfCommand);

//任务函数
void TaskServiceFuncNeedleReagentBead(void *p_arg)
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
                TaskServiceCommandProc_NeedleReagentBead_UartRS1(uartRS1CommandPtr);
                //处置完成,执行后续操作
                UserMemFree(SRAM_CCM,uartRS1CommandPtr);
            }
            else if (SYSTEM_CMD_SRC_SELF == taskSystemRecvCommandPtr->commandSrc)
            {
                //自身指令源
                SYSTEM_CMD_SELF* selfCommandPtr =  (SYSTEM_CMD_SELF*)(taskSystemRecvCommandPtr->systemCommandDataPtr);
                //对接收到的数据进行处理
                TaskServiceCommandProc_NeedleReagentBead_Self(selfCommandPtr);
                //处置完成,执行后续操作
                UserMemFree(SRAM_CCM,selfCommandPtr);
            }
            else if(SYSTEM_CMD_SRC_NET_W5500 == taskSystemRecvCommandPtr->commandSrc)
            {
                //网口指令源
                SYSTEM_CMD_NET_W5500* netW5500CommandPtr =  (SYSTEM_CMD_NET_W5500*)(taskSystemRecvCommandPtr->systemCommandDataPtr);
                //对接收到的数据进行处理
                TaskServiceCommandProc_NeedleReagentBead_NetW5500(netW5500CommandPtr);
                //处置完成,执行后续操作
                UserMemFree(SRAM_CCM,netW5500CommandPtr);
            }
            else if(SYSTEM_CMD_SRC_CAN1 == taskSystemRecvCommandPtr->commandSrc)
            {
                //CAN1口指令源
                SYSTEM_CMD_CAN1* can1CommandPtr =  (SYSTEM_CMD_CAN1*)(taskSystemRecvCommandPtr->systemCommandDataPtr);
                //对接收到的数据进行处理
                TaskServiceCommandProc_NeedleReagentBead_Can1(can1CommandPtr);
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
//试剂磁珠针 升降复位
static LH_ERR TaskServiceCommandProc_NeedleReagentBead_UartRS1_ResetUpdown(SYSTEM_CMD_UART_RS1* uartCommandPtr,uint16_t* resultData);
//试剂磁珠针 旋转复位
static LH_ERR TaskServiceCommandProc_NeedleReagentBead_UartRS1_ResetRotate(SYSTEM_CMD_UART_RS1* uartCommandPtr,uint16_t* resultData);
//试剂磁珠针 模块复位
static LH_ERR TaskServiceCommandProc_NeedleReagentBead_UartRS1_ResetAll(SYSTEM_CMD_UART_RS1* uartCommandPtr,uint16_t* resultData);
//试剂磁珠针 泵复位
static LH_ERR TaskServiceCommandProc_NeedleReagentBead_UartRS1_ResetPump(SYSTEM_CMD_UART_RS1* uartCommandPtr,uint16_t* resultData);
//试剂磁珠针 旋转到清洗并清洗
static LH_ERR TaskServiceCommandProc_NeedleReagentBead_UartRS1_RotateCleanAndClean(SYSTEM_CMD_UART_RS1* uartCommandPtr,uint16_t* resultData);
//试剂磁珠针 旋转到R1
static LH_ERR TaskServiceCommandProc_NeedleReagentBead_UartRS1_RotateR1(SYSTEM_CMD_UART_RS1* uartCommandPtr,uint16_t* resultData);
//试剂磁珠针 旋转到R2
static LH_ERR TaskServiceCommandProc_NeedleReagentBead_UartRS1_RotateR2(SYSTEM_CMD_UART_RS1* uartCommandPtr,uint16_t* resultData);
//试剂磁珠针 旋转到磁珠位
static LH_ERR TaskServiceCommandProc_NeedleReagentBead_UartRS1_RotateBead(SYSTEM_CMD_UART_RS1* uartCommandPtr,uint16_t* resultData);
//试剂磁珠针 旋转到反应盘
static LH_ERR TaskServiceCommandProc_NeedleReagentBead_UartRS1_RotateReaction(SYSTEM_CMD_UART_RS1* uartCommandPtr,uint16_t* resultData);
//试剂磁珠针 吸液
static LH_ERR TaskServiceCommandProc_NeedleReagentBead_UartRS1_Absorb(SYSTEM_CMD_UART_RS1* uartCommandPtr,uint16_t* resultData);
//试剂磁珠针 排液
static LH_ERR TaskServiceCommandProc_NeedleReagentBead_UartRS1_Inject(SYSTEM_CMD_UART_RS1* uartCommandPtr,uint16_t* resultData);
//试剂磁珠针  清洗
static LH_ERR TaskServiceCommandProc_NeedleReagentBead_UartRS1_Clean(SYSTEM_CMD_UART_RS1* uartCommandPtr,uint16_t* resultData);
//试剂磁珠针 灌注
static LH_ERR TaskServiceCommandProc_NeedleReagentBead_UartRS1_Prime(SYSTEM_CMD_UART_RS1* uartCommandPtr,uint16_t* resultData);
//试剂磁珠针 旋转到清洗
static LH_ERR TaskServiceCommandProc_NeedleReagentBead_UartRS1_RotateClean(SYSTEM_CMD_UART_RS1* uartCommandPtr,uint16_t* resultData);


//定义函数数组,用于串口处理线程回调
static const TaskServiceProcPtrWithUartCommand taskServiceProcNeedleReagentBeadPtrWithUartCommandArray[] = {
	TaskServiceCommandProc_NeedleReagentBead_UartRS1_ResetUpdown,
	TaskServiceCommandProc_NeedleReagentBead_UartRS1_ResetRotate,
	TaskServiceCommandProc_NeedleReagentBead_UartRS1_ResetAll,
	TaskServiceCommandProc_NeedleReagentBead_UartRS1_ResetPump,
	TaskServiceCommandProc_NeedleReagentBead_UartRS1_RotateCleanAndClean,
	TaskServiceCommandProc_NeedleReagentBead_UartRS1_RotateR1,
	TaskServiceCommandProc_NeedleReagentBead_UartRS1_RotateR2,
	TaskServiceCommandProc_NeedleReagentBead_UartRS1_RotateBead,
	TaskServiceCommandProc_NeedleReagentBead_UartRS1_RotateReaction,
	TaskServiceCommandProc_NeedleReagentBead_UartRS1_Absorb,
	TaskServiceCommandProc_NeedleReagentBead_UartRS1_Inject,
	TaskServiceCommandProc_NeedleReagentBead_UartRS1_Clean,
	TaskServiceCommandProc_NeedleReagentBead_UartRS1_Prime,
	TaskServiceCommandProc_NeedleReagentBead_UartRS1_RotateClean,
};
//回调函数数量
#define NEEDLE_REAGENT_BEAD_FUNC_UART_COUNT_MAX      (sizeof(taskServiceProcNeedleReagentBeadPtrWithUartCommandArray)/sizeof(taskServiceProcNeedleReagentBeadPtrWithUartCommandArray[0]))

//串口指令的第一级处理
static void TaskServiceCommandProc_NeedleReagentBead_UartRS1(SYSTEM_CMD_UART_RS1* uartCommand)
{
    LH_ERR errorCode = LH_ERR_NONE;
	//发送ACK,创建一条新数据包
	SystemUartRS_ModbusCreatePackage(uartCommand->uartRecvCommandArray,uartCommand->srcDeviceAddr,uartCommand->frameNo,
						uartCommand->funcCode,uartCommand->regAddr,uartCommand->regValue);
	BoardRS_SendBuffer(ACTION_DEBUG_PORT,uartCommand->uartRecvCommandArray,12);

    //调用指令
	uint8_t actionCode= ((uartCommand->regAddr)>>8);
    uint16_t resultData = 0x0000;
	if(actionCode > NEEDLE_REAGENT_BEAD_FUNC_UART_COUNT_MAX)
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
		errorCode = taskServiceProcNeedleReagentBeadPtrWithUartCommandArray[actionCode-1](uartCommand,&resultData);
	}
	SystemUartRS_ModbusCreatePackage(uartCommand->uartRecvCommandArray, uartCommand->srcDeviceAddr, uartCommand->frameNo,
							0x01, (uint16_t)errorCode, resultData);
	//发送一个填充好的数据包
	BoardRS_SendBuffer(ACTION_DEBUG_PORT,uartCommand->uartRecvCommandArray,12);
}

//试剂磁珠针 升降复位
static LH_ERR TaskServiceCommandProc_NeedleReagentBead_UartRS1_ResetUpdown(SYSTEM_CMD_UART_RS1* uartCommandPtr,uint16_t* resultData)
{
    LH_ERR errorCode;
	TSC_NEEDLE_REAGENT_BEAD_POS rotatePos;

	errorCode = ActionNeedleReagentBead_Init(&rotatePos);
	return errorCode;
}

//试剂磁珠针 旋转复位
static LH_ERR TaskServiceCommandProc_NeedleReagentBead_UartRS1_ResetRotate(SYSTEM_CMD_UART_RS1* uartCommandPtr,uint16_t* resultData)
{
    LH_ERR errorCode;
	TSC_NEEDLE_REAGENT_BEAD_POS rotatePos;

	errorCode = ActionNeedleReagentBead_Init(&rotatePos);
	return errorCode;
}

//试剂磁珠针 模块复位
static LH_ERR TaskServiceCommandProc_NeedleReagentBead_UartRS1_ResetAll(SYSTEM_CMD_UART_RS1* uartCommandPtr,uint16_t* resultData)
{
    LH_ERR errorCode;
	TSC_NEEDLE_REAGENT_BEAD_POS rotatePos;

	errorCode = ActionNeedleReagentBead_Init(&rotatePos);
	return errorCode;
}

//试剂磁珠针 泵复位
static LH_ERR TaskServiceCommandProc_NeedleReagentBead_UartRS1_ResetPump(SYSTEM_CMD_UART_RS1* uartCommandPtr,uint16_t* resultData)
{
    LH_ERR errorCode;
	TSC_NEEDLE_REAGENT_BEAD_POS rotatePos;

	errorCode = ActionNeedleReagentBead_Init(&rotatePos);
	return errorCode;
}

//试剂磁珠针 旋转到清洗并清洗
static LH_ERR TaskServiceCommandProc_NeedleReagentBead_UartRS1_RotateCleanAndClean(SYSTEM_CMD_UART_RS1* uartCommandPtr,uint16_t* resultData)
{
    LH_ERR errorCode;
	TSC_NEEDLE_REAGENT_BEAD_POS rotatePos;

//	uint8_t paramLow = (uint8_t)(uartCommandPtr->uartRecvCommandArray[7]);
	uint8_t paramHigh = (uint8_t)(uartCommandPtr->uartRecvCommandArray[8]);
	TSC_NEEDLE_REAGENT_BEAD_CLEAN_OPT opt = TSC_NEEDLE_REAGENT_BEAD_CLEAN_OPT_IN_OUT;

    if(paramHigh == 0x01)
	{
		opt = TSC_NEEDLE_REAGENT_BEAD_CLEAN_OPT_IN;
	}
	else if(paramHigh == 0x02)//0x02
	{
		opt = TSC_NEEDLE_REAGENT_BEAD_CLEAN_OPT_OUT;
	}
    else
    {
        opt = TSC_NEEDLE_REAGENT_BEAD_CLEAN_OPT_IN_OUT;
    }


	errorCode = ActionNeedleReagentBead_Clean(opt,&rotatePos);
	return errorCode;
}

//试剂磁珠针 旋转到R1
static LH_ERR TaskServiceCommandProc_NeedleReagentBead_UartRS1_RotateR1(SYSTEM_CMD_UART_RS1* uartCommandPtr,uint16_t* resultData)
{
    LH_ERR errorCode;
	TSC_NEEDLE_REAGENT_BEAD_POS rotatePos;

	errorCode = ActionNeedleReagentBead_RotateSpecialPos(TSC_NEEDLE_REAGENT_BEAD_POS_R1,&rotatePos);
	return errorCode;
}

//试剂磁珠针 旋转到R2
static LH_ERR TaskServiceCommandProc_NeedleReagentBead_UartRS1_RotateR2(SYSTEM_CMD_UART_RS1* uartCommandPtr,uint16_t* resultData)
{
    LH_ERR errorCode;
	TSC_NEEDLE_REAGENT_BEAD_POS rotatePos;

	errorCode = ActionNeedleReagentBead_RotateSpecialPos(TSC_NEEDLE_REAGENT_BEAD_POS_R2,&rotatePos);
	return errorCode;
}

//试剂磁珠针 旋转到磁珠位
static LH_ERR TaskServiceCommandProc_NeedleReagentBead_UartRS1_RotateBead(SYSTEM_CMD_UART_RS1* uartCommandPtr,uint16_t* resultData)
{
    LH_ERR errorCode;
	TSC_NEEDLE_REAGENT_BEAD_POS rotatePos;

	errorCode = ActionNeedleReagentBead_RotateSpecialPos(TSC_NEEDLE_REAGENT_BEAD_POS_BEAD,&rotatePos);
	return errorCode;
}

//试剂磁珠针 旋转到反应盘
static LH_ERR TaskServiceCommandProc_NeedleReagentBead_UartRS1_RotateReaction(SYSTEM_CMD_UART_RS1* uartCommandPtr,uint16_t* resultData)
{
    LH_ERR errorCode;
	TSC_NEEDLE_REAGENT_BEAD_POS rotatePos;

	errorCode = ActionNeedleReagentBead_RotateSpecialPos(TSC_NEEDLE_REAGENT_BEAD_POS_REACTION,&rotatePos);
	return errorCode;
}
//试剂磁珠针 吸液
static LH_ERR TaskServiceCommandProc_NeedleReagentBead_UartRS1_Absorb(SYSTEM_CMD_UART_RS1* uartCommandPtr,uint16_t* resultData)
{
    LH_ERR errorCode;
	TSC_NEEDLE_REAGENT_BEAD_POS rotatePos;
	uint8_t paramLow = (uint8_t)(uartCommandPtr->regValue);
	static ONE_OF_CUPSTATUS oneofcups;//测试用
	oneofcups.cups = 1;
	oneofcups.cupholder = 1;
	errorCode = ActionNeedleReagentBead_AbsorbAnyLiquid(paramLow,&rotatePos,&oneofcups);

	return errorCode;
}

//试剂磁珠针 排液
static LH_ERR TaskServiceCommandProc_NeedleReagentBead_UartRS1_Inject(SYSTEM_CMD_UART_RS1* uartCommandPtr,uint16_t* resultData)
{
    LH_ERR errorCode;
	TSC_NEEDLE_REAGENT_BEAD_POS rotatePos;
	uint8_t paramLow = (uint8_t)(uartCommandPtr->regValue);

	errorCode = ActionNeedleReagentBead_InjectAnyLiquid(paramLow,&rotatePos);

	return errorCode;
}

//试剂磁珠针  清洗
static LH_ERR TaskServiceCommandProc_NeedleReagentBead_UartRS1_Clean(SYSTEM_CMD_UART_RS1* uartCommandPtr,uint16_t* resultData)
{
    LH_ERR errorCode;
	TSC_NEEDLE_REAGENT_BEAD_POS rotatePos;

	//	uint8_t paramLow = (uint8_t)(uartCommandPtr->uartRecvCommandArray[7]);
	uint8_t paramHigh = (uint8_t)(uartCommandPtr->uartRecvCommandArray[8]);
	TSC_NEEDLE_REAGENT_BEAD_CLEAN_OPT opt = TSC_NEEDLE_REAGENT_BEAD_CLEAN_OPT_IN_OUT;

    if(paramHigh == 0x01)
	{
		opt = TSC_NEEDLE_REAGENT_BEAD_CLEAN_OPT_IN;
	}
	else if(paramHigh == 0x02)//0x02
	{
		opt = TSC_NEEDLE_REAGENT_BEAD_CLEAN_OPT_OUT;
	}
    else
    {
        opt = TSC_NEEDLE_REAGENT_BEAD_CLEAN_OPT_IN_OUT;
    }

	errorCode = ActionNeedleReagentBead_Clean(opt,&rotatePos);

	return errorCode;
}

//试剂磁珠针 灌注
static LH_ERR TaskServiceCommandProc_NeedleReagentBead_UartRS1_Prime(SYSTEM_CMD_UART_RS1* uartCommandPtr,uint16_t* resultData)
{
    LH_ERR errorCode;
	TSC_NEEDLE_REAGENT_BEAD_POS rotatePos;

	uint8_t paramLow = (uint8_t)(uartCommandPtr->regValue);
	uint8_t paramHigh = (uint8_t)((uartCommandPtr->regValue)>>8);
	TSC_NEEDLE_REAGENT_PRIME_OPT opt = TSC_NEEDLE_REAGENT_PRIME_OPT_ALL;

    if(paramHigh == 0x01)
	{
		opt = TSC_NEEDLE_REAGENT_PRIME_OPT_INNER;
	}
	else if(paramHigh == 0x02)//0x02
	{
		opt = TSC_NEEDLE_REAGENT_PRIME_OPT_OUT;
	}
    else
    {
        opt = TSC_NEEDLE_REAGENT_PRIME_OPT_ALL;
    }
	errorCode = ActionNeedleReagentBead_Prime(opt,&rotatePos);

	return errorCode;
}

//试剂磁珠针 旋转到清洗
static LH_ERR TaskServiceCommandProc_NeedleReagentBead_UartRS1_RotateClean(SYSTEM_CMD_UART_RS1* uartCommandPtr,uint16_t* resultData)
{
    LH_ERR errorCode;
	TSC_NEEDLE_REAGENT_BEAD_POS rotatePos;

	errorCode = ActionNeedleReagentBead_RotateSpecialPos(TSC_NEEDLE_REAGENT_BEAD_POS_CLEAN,&rotatePos);

	return errorCode;
}



/********************************************************************************************************************/

/*****************************************************CAN指令处理*****************************************************/
//试剂磁珠针 升降复位
static LH_ERR TaskServiceCommandProc_NeedleReagentBead_Can1_ResetUpdown(SYSTEM_CMD_CAN1* canCommand)
{
    LH_ERR errorCode;
	TSC_NEEDLE_REAGENT_BEAD_POS rotatePos;
	uint8_t count;
	errorCode = ActionNeedleReagentBead_Init(&rotatePos);
	count = Arraypack_Common(canCommand,0,errorCode);
	CanSendDataToCell(TESTPROC_CELL_PLATFORM_NMBR,BOARD_ID_TRACK_COMM,0x0018,canCommand->can1RecvDataBuffer,count);
	return errorCode;
}
//试剂磁珠针 旋转复位
static LH_ERR TaskServiceCommandProc_NeedleReagentBead_Can1_ResetRotate(SYSTEM_CMD_CAN1* canCommand)
{
    LH_ERR errorCode;
	TSC_NEEDLE_REAGENT_BEAD_POS rotatePos;
	uint8_t count;
	errorCode = ActionNeedleReagentBead_Init(&rotatePos);
	count = Arraypack_Common(canCommand,0,errorCode);
	CanSendDataToCell(TESTPROC_CELL_PLATFORM_NMBR,BOARD_ID_TRACK_COMM,0x0018,canCommand->can1RecvDataBuffer,count);
	return errorCode;
}
//试剂磁珠针 模块复位
static LH_ERR TaskServiceCommandProc_NeedleReagentBead_Can1_ResetAll(SYSTEM_CMD_CAN1* canCommand)
{
    LH_ERR errorCode;
	TSC_NEEDLE_REAGENT_BEAD_POS rotatePos;
	uint8_t count;
	errorCode = ActionNeedleReagentBead_Init(&rotatePos);
	count = Arraypack_Common(canCommand,0,errorCode);
	CanSendDataToCell(TESTPROC_CELL_PLATFORM_NMBR,BOARD_ID_TRACK_COMM,0x0018,canCommand->can1RecvDataBuffer,count);
	return errorCode;
}
//试剂磁珠针 泵复位
static LH_ERR TaskServiceCommandProc_NeedleReagentBead_Can1_ResetPump(SYSTEM_CMD_CAN1* canCommand)
{
    LH_ERR errorCode;
	TSC_NEEDLE_REAGENT_BEAD_POS rotatePos;
	uint8_t count;
	errorCode = ActionNeedleReagentBead_Init(&rotatePos);
	count = Arraypack_Common(canCommand,0,errorCode);
	CanSendDataToCell(TESTPROC_CELL_PLATFORM_NMBR,BOARD_ID_TRACK_COMM,0x0018,canCommand->can1RecvDataBuffer,count);
	return errorCode;
}
//试剂磁珠针 旋转到清洗位
static LH_ERR TaskServiceCommandProc_NeedleReagentBead_Can1_RotateClean(SYSTEM_CMD_CAN1* canCommand)
{
    LH_ERR errorCode;
	TSC_NEEDLE_REAGENT_BEAD_POS rotatePos;
	uint8_t count;
	errorCode = ActionNeedleReagentBead_RotateSpecialPos(TSC_NEEDLE_REAGENT_BEAD_POS_CLEAN,&rotatePos);
	count = Arraypack_Common(canCommand,0,errorCode);
	CanSendDataToCell(TESTPROC_CELL_PLATFORM_NMBR,BOARD_ID_TRACK_COMM,0x0018,canCommand->can1RecvDataBuffer,count);
	return errorCode;
}
//试剂磁珠针 旋转到R1
static LH_ERR TaskServiceCommandProc_NeedleReagentBead_Can1_RotateR1(SYSTEM_CMD_CAN1* canCommand)
{
    LH_ERR errorCode;
	TSC_NEEDLE_REAGENT_BEAD_POS rotatePos;
	uint8_t count;
	errorCode = ActionNeedleReagentBead_RotateSpecialPos(TSC_NEEDLE_REAGENT_BEAD_POS_R1,&rotatePos);
	count = Arraypack_Common(canCommand,0,errorCode);
	CanSendDataToCell(TESTPROC_CELL_PLATFORM_NMBR,BOARD_ID_TRACK_COMM,0x0018,canCommand->can1RecvDataBuffer,count);
	return errorCode;
}
//试剂磁珠针 旋转到R2
static LH_ERR TaskServiceCommandProc_NeedleReagentBead_Can1_RotateR2(SYSTEM_CMD_CAN1* canCommand)
{
    LH_ERR errorCode;
	TSC_NEEDLE_REAGENT_BEAD_POS rotatePos;
	uint8_t count;
	errorCode = ActionNeedleReagentBead_RotateSpecialPos(TSC_NEEDLE_REAGENT_BEAD_POS_R2,&rotatePos);
	count = Arraypack_Common(canCommand,0,errorCode);
	CanSendDataToCell(TESTPROC_CELL_PLATFORM_NMBR,BOARD_ID_TRACK_COMM,0x0018,canCommand->can1RecvDataBuffer,count);
	return errorCode;
}
//试剂磁珠针 旋转到磁珠位
static LH_ERR TaskServiceCommandProc_NeedleReagentBead_Can1_RotateBead(SYSTEM_CMD_CAN1* canCommand)
{
    LH_ERR errorCode;
	TSC_NEEDLE_REAGENT_BEAD_POS rotatePos;
	uint8_t count;
	errorCode = ActionNeedleReagentBead_RotateSpecialPos(TSC_NEEDLE_REAGENT_BEAD_POS_BEAD,&rotatePos);
	count = Arraypack_Common(canCommand,0,errorCode);
	CanSendDataToCell(TESTPROC_CELL_PLATFORM_NMBR,BOARD_ID_TRACK_COMM,0x0018,canCommand->can1RecvDataBuffer,count);
	return errorCode;
}
//试剂磁珠针 旋转到反应盘
static LH_ERR TaskServiceCommandProc_NeedleReagentBead_Can1_RotateReaction(SYSTEM_CMD_CAN1* canCommand)
{
    LH_ERR errorCode;
	TSC_NEEDLE_REAGENT_BEAD_POS rotatePos;
	uint8_t count;
	errorCode = ActionNeedleReagentBead_RotateSpecialPos(TSC_NEEDLE_REAGENT_BEAD_POS_REACTION,&rotatePos);
	count = Arraypack_Common(canCommand,0,errorCode);
	CanSendDataToCell(TESTPROC_CELL_PLATFORM_NMBR,BOARD_ID_TRACK_COMM,0x0018,canCommand->can1RecvDataBuffer,count);
	return errorCode;
}
//试剂磁珠针 吸液
static LH_ERR TaskServiceCommandProc_NeedleReagentBead_Can1_Absorb(SYSTEM_CMD_CAN1* canCommand)
{
    LH_ERR errorCode;
	ONE_OF_CUPSTATUS oneofcups;
	TSC_NEEDLE_REAGENT_BEAD_POS rotatePos;
	uint8_t paramLow = canCommand->can1RecvDataBuffer[0];
	uint8_t count;
	errorCode = ActionNeedleReagentBead_AbsorbAnyLiquid(paramLow,&rotatePos,&oneofcups);
	count = Arraypack_Common(canCommand,paramLow,errorCode);
	CanSendDataToCell(TESTPROC_CELL_PLATFORM_NMBR,BOARD_ID_TRACK_COMM,0x0018,canCommand->can1RecvDataBuffer,count);
	return errorCode;
}
//试剂磁珠针 排液
static LH_ERR TaskServiceCommandProc_NeedleReagentBead_Can1_Inject(SYSTEM_CMD_CAN1* canCommand)
{
    LH_ERR errorCode;
	TSC_NEEDLE_REAGENT_BEAD_POS rotatePos;
	uint8_t paramLow = canCommand->can1RecvDataBuffer[0];
	uint8_t count;
	errorCode = ActionNeedleReagentBead_InjectAnyLiquid(paramLow,&rotatePos);
	count = Arraypack_Common(canCommand,paramLow,errorCode);
	CanSendDataToCell(TESTPROC_CELL_PLATFORM_NMBR,BOARD_ID_TRACK_COMM,0x0018,canCommand->can1RecvDataBuffer,count);
	return errorCode;
}
//试剂磁珠针  清洗
static LH_ERR TaskServiceCommandProc_NeedleReagentBead_Can1_Clean(SYSTEM_CMD_CAN1* canCommand)
{
    LH_ERR errorCode;
	TSC_NEEDLE_REAGENT_BEAD_POS rotatePos;
	uint8_t count;
	uint8_t paramHigh = canCommand->can1RecvDataBuffer[0];
	TSC_NEEDLE_REAGENT_BEAD_CLEAN_OPT opt = TSC_NEEDLE_REAGENT_BEAD_CLEAN_OPT_IN_OUT;

    if(paramHigh == 0x01)
	{
		opt = TSC_NEEDLE_REAGENT_BEAD_CLEAN_OPT_IN;
	}
	else if(paramHigh == 0x02)//0x02
	{
		opt = TSC_NEEDLE_REAGENT_BEAD_CLEAN_OPT_OUT;
	}
    else
    {
        opt = TSC_NEEDLE_REAGENT_BEAD_CLEAN_OPT_IN_OUT;
    }

	errorCode = ActionNeedleReagentBead_Clean(opt,&rotatePos);
	count = Arraypack_Common(canCommand,paramHigh,errorCode);
	CanSendDataToCell(TESTPROC_CELL_PLATFORM_NMBR,BOARD_ID_TRACK_COMM,0x0018,canCommand->can1RecvDataBuffer,count);
	return errorCode;
}
//试剂磁珠针 灌注
static LH_ERR TaskServiceCommandProc_NeedleReagentBead_Can1_Prime(SYSTEM_CMD_CAN1* canCommand)
{
    LH_ERR errorCode;
	TSC_NEEDLE_REAGENT_BEAD_POS rotatePos;
	uint8_t paramHigh = canCommand->can1RecvDataBuffer[0];
	uint8_t count;
	TSC_NEEDLE_REAGENT_PRIME_OPT opt = TSC_NEEDLE_REAGENT_PRIME_OPT_ALL;

    if(paramHigh == 0x01)
	{
		opt = TSC_NEEDLE_REAGENT_PRIME_OPT_INNER;
	}
	else if(paramHigh == 0x02)//0x02
	{
		opt = TSC_NEEDLE_REAGENT_PRIME_OPT_OUT;
	}
    else
    {
        opt = TSC_NEEDLE_REAGENT_PRIME_OPT_ALL;
    }
	errorCode = ActionNeedleReagentBead_Prime(opt,&rotatePos);
	count = Arraypack_Common(canCommand,paramHigh,errorCode);
	CanSendDataToCell(TESTPROC_CELL_PLATFORM_NMBR,BOARD_ID_TRACK_COMM,0x0018,canCommand->can1RecvDataBuffer,count);
	return errorCode;
}

static const APP_TASK_CAN1_PROC_UNIT taskServiceProcPtrWithNeedleReagentBeadCan1CommandArray[] = {
    {CAN1_COMMAND_NEEDLE_REAGENT_VERTICAL_RESET         ,        TaskServiceCommandProc_NeedleReagentBead_Can1_ResetUpdown},//试剂针垂直初始化
    {CAN1_COMMAND_NEEDLE_REAGENT_LEVEL_RESET            ,        TaskServiceCommandProc_NeedleReagentBead_Can1_ResetRotate},//试剂针水平初始化
    {CAN1_COMMAND_NEEDLE_REAGENT_RESET                  ,        TaskServiceCommandProc_NeedleReagentBead_Can1_ResetAll},//试剂针垂直水平初始化
    {CAN1_COMMAND_NEEDLE_REAGENT_PUMP_RESET             ,        TaskServiceCommandProc_NeedleReagentBead_Can1_ResetPump},//试剂针泵初始化
    {CAN1_COMMAND_NEEDLE_REAGENT_ROTATE_WASH            ,        TaskServiceCommandProc_NeedleReagentBead_Can1_RotateClean},//转到清洗位
    {CAN1_COMMAND_NEEDLE_REAGENT_ROTATE_OUTSIDE         ,        TaskServiceCommandProc_NeedleReagentBead_Can1_RotateR1},//转到试剂盘外圈
    {CAN1_COMMAND_NEEDLE_REAGENT_ROTATE_MIDDLE          ,        TaskServiceCommandProc_NeedleReagentBead_Can1_RotateR2},//转到试剂盘中圈
    {CAN1_COMMAND_NEEDLE_REAGENT_ROTATE_INSIDE          ,        TaskServiceCommandProc_NeedleReagentBead_Can1_RotateBead},//转到试剂盘内圈
    {CAN1_COMMAND_NEEDLE_REAGENT_ROTATE_REACTION        ,        TaskServiceCommandProc_NeedleReagentBead_Can1_RotateReaction},//转到反应盘
    {CAN1_COMMAND_NEEDLE_REAGENT_ABSORBENT              ,        TaskServiceCommandProc_NeedleReagentBead_Can1_Absorb},//吸试剂
    {CAN1_COMMAND_NEEDLE_REAGENT_EXCRETION              ,        TaskServiceCommandProc_NeedleReagentBead_Can1_Inject},//排试剂
    {CAN1_COMMAND_NEEDLE_REAGENT_WASH                   ,        TaskServiceCommandProc_NeedleReagentBead_Can1_Clean},//清洗
    {CAN1_COMMAND_NEEDLE_REAGENT_PRIME                  ,        TaskServiceCommandProc_NeedleReagentBead_Can1_Prime},//灌注
};
#define NEEDLE_REAGENT_BEAD_FUNC_CAN_COUNT_MAX      (sizeof(taskServiceProcPtrWithNeedleReagentBeadCan1CommandArray)/sizeof(taskServiceProcPtrWithNeedleReagentBeadCan1CommandArray[0]))

static void TaskServiceCommandProc_NeedleReagentBead_Can1(SYSTEM_CMD_CAN1* canCommand)
{
	uint16_t indexUtil = 0;
    for(indexUtil = 0; indexUtil < NEEDLE_REAGENT_BEAD_FUNC_CAN_COUNT_MAX; indexUtil++)
    {
        //指令匹配
        if(canCommand->command == taskServiceProcPtrWithNeedleReagentBeadCan1CommandArray[indexUtil].command)
        {
            //调用处理函数
            if(taskServiceProcPtrWithNeedleReagentBeadCan1CommandArray[indexUtil].can1CommandProcFunc != NULL)
            {
                taskServiceProcPtrWithNeedleReagentBeadCan1CommandArray[indexUtil].can1CommandProcFunc(canCommand);
                break;
            }
        }
    }
}

/********************************************************************************************************************/

/*******************************************************网口指令处理**************************************************/

static void TaskServiceCommandProc_NeedleReagentBead_NetW5500(SYSTEM_CMD_NET_W5500* netCommand)
{

}

/********************************************************************************************************************/

/*******************************************************自身指令处理**************************************************/
//磁珠试剂针初始化
static LH_ERR TaskServiceCommandProc_NeedleReagentBead_Self_Init(SYSTEM_CMD_SELF* selfCommandPtr);
//磁珠试剂针 旋转到指定位置
static LH_ERR TaskServiceCommandProc_NeedleReagentBead_Self_RotateSpecialPosition(SYSTEM_CMD_SELF* selfCommandPtr);
//磁珠试剂针 吸液
static LH_ERR TaskServiceCommandProc_NeedleReagentBead_Self_Absorb(SYSTEM_CMD_SELF* selfCommandPtr);
//磁珠试剂针 排液
static LH_ERR TaskServiceCommandProc_NeedleReagentBead_Self_Inject(SYSTEM_CMD_SELF* selfCommandPtr);
//磁珠试剂针 清洗
static LH_ERR TaskServiceCommandProc_NeedleReagentBead_Self_Clean(SYSTEM_CMD_SELF* selfCommandPtr);
//磁珠试剂针 灌注
static LH_ERR TaskServiceCommandProc_NeedleReagentBead_Self_Prime(SYSTEM_CMD_SELF* selfCommandPtr);

//自身指令回调函数数组
static const TaskServicePtrWithSelfCommand TaskProcWithNeedleReagentBeadSelfCommandArray[] = {
	TaskServiceCommandProc_NeedleReagentBead_Self_Init,
	TaskServiceCommandProc_NeedleReagentBead_Self_RotateSpecialPosition,
	TaskServiceCommandProc_NeedleReagentBead_Self_Absorb,
	TaskServiceCommandProc_NeedleReagentBead_Self_Inject,
	TaskServiceCommandProc_NeedleReagentBead_Self_Clean,
	TaskServiceCommandProc_NeedleReagentBead_Self_Prime,
};

//支持指令的个数
#define TASK_COMMAND_PROC_SELF_COUNT	(sizeof(TaskProcWithNeedleReagentBeadSelfCommandArray)/sizeof(TaskProcWithNeedleReagentBeadSelfCommandArray[0]))

//用于调试指令的头调试字符串
static const uint8_t TaskSelfCommandDebugStartMsgHeadString[] = "$$Action 9 Start : ";
static const uint8_t TaskSelfCommandDebugEndMsgHeadString[] = "$$Action 9 End ";


static void TaskServiceCommandProc_NeedleReagentBead_Self(SYSTEM_CMD_SELF* selfCommand)
{
    OS_ERR err;
	LH_ERR actionResult;
	//指令过程与结果对象
	TSC_CMD_RESULT_DATA_NEEDLE_REAGENT_BEAD* cmdResultPtr = (TSC_CMD_RESULT_DATA_NEEDLE_REAGENT_BEAD*)selfCommand->commandDataSpecialChannel;
	
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
		actionResult = TaskProcWithNeedleReagentBeadSelfCommandArray[selfCommand->commandIndex](selfCommand);
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

//磁珠试剂针初始化
static LH_ERR TaskServiceCommandProc_NeedleReagentBead_Self_Init(SYSTEM_CMD_SELF* selfCommandPtr)
{
    LH_ERR errCode;
	//打印开始处理信息
	TaskServiceSerialDebugMsgShowStart((uint8_t*)TaskSelfCommandDebugStartMsgHeadString,
										(uint8_t*)"NeedleReagentBeadInit",
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
	
	TSC_NEEDLE_REAGENT_BEAD_POS rotatePos;

	TSC_CMD_RESULT_DATA_NEEDLE_REAGENT_BEAD* cmdResultPtr = 
			(TSC_CMD_RESULT_DATA_NEEDLE_REAGENT_BEAD*)selfCommandPtr->commandDataSpecialChannel;

	//处理
	errCode = ActionNeedleReagentBead_Init(&rotatePos);
	if(errCode == LH_ERR_NONE)
	{
		cmdResultPtr->currentNeedlePosition = rotatePos;
	}
#endif

	return errCode;
}

//磁珠试剂针 旋转到指定位置
static LH_ERR TaskServiceCommandProc_NeedleReagentBead_Self_RotateSpecialPosition(SYSTEM_CMD_SELF* selfCommandPtr)
{
    LH_ERR errCode;
	//打印开始处理信息
	TaskServiceSerialDebugMsgShowStart((uint8_t*)TaskSelfCommandDebugStartMsgHeadString,
										(uint8_t*)"NeedleReagentBeadRotateSpecialPosition",
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
	
	TSC_NEEDLE_REAGENT_BEAD_POS rotatePos;

	TSC_CMD_RESULT_DATA_NEEDLE_REAGENT_BEAD* cmdResultPtr = 
			(TSC_CMD_RESULT_DATA_NEEDLE_REAGENT_BEAD*)selfCommandPtr->commandDataSpecialChannel;

	TSC_NEEDLE_REAGENT_BEAD_POS targetPos = (TSC_NEEDLE_REAGENT_BEAD_POS)(selfCommandPtr->commandParam1);

	//处理
	errCode = ActionNeedleReagentBead_RotateSpecialPos(targetPos,&rotatePos);

	if(errCode == LH_ERR_NONE)
	{
		cmdResultPtr->currentNeedlePosition = rotatePos;
	}
#endif

	return errCode;
}

//磁珠试剂针 吸液
static LH_ERR TaskServiceCommandProc_NeedleReagentBead_Self_Absorb(SYSTEM_CMD_SELF* selfCommandPtr)
{
    LH_ERR errCode;
	//打印开始处理信息
	TaskServiceSerialDebugMsgShowStart((uint8_t*)TaskSelfCommandDebugStartMsgHeadString,
										(uint8_t*)"NeedleReagentBeadAbsorb",
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
	
	TSC_NEEDLE_REAGENT_BEAD_POS rotatePos;

	TSC_CMD_RESULT_DATA_NEEDLE_REAGENT_BEAD* cmdResultPtr = 
			(TSC_CMD_RESULT_DATA_NEEDLE_REAGENT_BEAD*)selfCommandPtr->commandDataSpecialChannel;

	uint16_t liquidUl = selfCommandPtr->commandParam1;
	ONE_OF_CUPSTATUS oneofcups;
    oneofcups.cupholder = selfCommandPtr->commandParam2;//试管架号
    oneofcups.cups = selfCommandPtr->commandParam3;//试管架杯号
	errCode = ActionNeedleReagentBead_AbsorbAnyLiquid(liquidUl,&rotatePos,&oneofcups);

	if(errCode == LH_ERR_NONE)
	{
		cmdResultPtr->currentNeedlePosition = rotatePos;
	}
#endif

	return errCode;
}

//磁珠试剂针 排液
static LH_ERR TaskServiceCommandProc_NeedleReagentBead_Self_Inject(SYSTEM_CMD_SELF* selfCommandPtr)
{
    LH_ERR errCode;
	//打印开始处理信息
	TaskServiceSerialDebugMsgShowStart((uint8_t*)TaskSelfCommandDebugStartMsgHeadString,
										(uint8_t*)"NeedleReagentBeadInject",
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
	
	TSC_NEEDLE_REAGENT_BEAD_POS rotatePos;

	TSC_CMD_RESULT_DATA_NEEDLE_REAGENT_BEAD* cmdResultPtr = 
			(TSC_CMD_RESULT_DATA_NEEDLE_REAGENT_BEAD*)selfCommandPtr->commandDataSpecialChannel;

	uint16_t liquidUl = selfCommandPtr->commandParam1;

	//处理
	errCode = ActionNeedleReagentBead_InjectAnyLiquid(liquidUl,&rotatePos);

	if(errCode == LH_ERR_NONE)
	{
		cmdResultPtr->currentNeedlePosition = rotatePos;
	}
#endif

	return errCode;
}

//磁珠试剂针 清洗
static LH_ERR TaskServiceCommandProc_NeedleReagentBead_Self_Clean(SYSTEM_CMD_SELF* selfCommandPtr)
{
    LH_ERR errCode;
	//打印开始处理信息
	TaskServiceSerialDebugMsgShowStart((uint8_t*)TaskSelfCommandDebugStartMsgHeadString,
										(uint8_t*)"NeedleReagentBeadClean",
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
	
	TSC_NEEDLE_REAGENT_BEAD_POS rotatePos;

	TSC_CMD_RESULT_DATA_NEEDLE_REAGENT_BEAD* cmdResultPtr = 
			(TSC_CMD_RESULT_DATA_NEEDLE_REAGENT_BEAD*)selfCommandPtr->commandDataSpecialChannel;
	
	TSC_NEEDLE_REAGENT_BEAD_POS targetPos = (TSC_NEEDLE_REAGENT_BEAD_POS)(selfCommandPtr->commandParam1);
    TSC_NEEDLE_REAGENT_BEAD_CLEAN_OPT opt      = (TSC_NEEDLE_REAGENT_BEAD_CLEAN_OPT)(selfCommandPtr->commandParam2);  
	//先清洗
	errCode = ActionNeedleReagentBead_Clean(opt,&rotatePos);
	if(errCode != LH_ERR_NONE)
	{
		return errCode;
	}
	//后旋转
	errCode = ActionNeedleReagentBead_RotateSpecialPos(targetPos,&rotatePos);
	if(errCode == LH_ERR_NONE)
	{
		cmdResultPtr->currentNeedlePosition = rotatePos;
	}
#endif

	return errCode;
}

//磁珠试剂针 灌注
static LH_ERR TaskServiceCommandProc_NeedleReagentBead_Self_Prime(SYSTEM_CMD_SELF* selfCommandPtr)
{
    LH_ERR errCode;
	//打印开始处理信息
	TaskServiceSerialDebugMsgShowStart((uint8_t*)TaskSelfCommandDebugStartMsgHeadString,
										(uint8_t*)"NeedleReagentBeadPrime",
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
	
	TSC_NEEDLE_REAGENT_BEAD_POS rotatePos;

	TSC_CMD_RESULT_DATA_NEEDLE_REAGENT_BEAD* cmdResultPtr = 
			(TSC_CMD_RESULT_DATA_NEEDLE_REAGENT_BEAD*)selfCommandPtr->commandDataSpecialChannel;
	TSC_NEEDLE_REAGENT_PRIME_OPT opt = (TSC_NEEDLE_REAGENT_PRIME_OPT)selfCommandPtr->commandParam1;
	//处理
	errCode = ActionNeedleReagentBead_Prime(opt,&rotatePos);
	if(errCode == LH_ERR_NONE)
	{
        cmdResultPtr->currentNeedlePosition = rotatePos;
	}
#endif

	return errCode;
}



/********************************************************************************************************************/






