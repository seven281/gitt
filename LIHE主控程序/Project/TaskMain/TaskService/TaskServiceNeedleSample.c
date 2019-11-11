#include "TaskServiceConfig.h"
#include "TSC_Inc.h"
#include "ActionImplementInc.h"
#include "TestProc_canCommand.h"
#include "Can1DataType.h"
//任务控制块,8字节对齐
#pragma pack(8)
//任务堆栈
CPU_STK stackBufferTaskServiceNeedleSample[STK_SIZE_TASK_SERVICE_NEEDLE_SAMPLE];
#pragma pack()

OS_TCB tcbTaskServiceNeedleSample;

//串口指令处理
static void TaskServiceCommandProc_NeedleSample_UartRS1(SYSTEM_CMD_UART_RS1* uartCommand);

//CAN口指令处理
static void TaskServiceCommandProc_NeedleSample_Can1(SYSTEM_CMD_CAN1* canCommand);

//网口指令处理
static void TaskServiceCommandProc_NeedleSample_NetW5500(SYSTEM_CMD_NET_W5500* netCommand);

//自身指令处理
static void TaskServiceCommandProc_NeedleSample_Self(SYSTEM_CMD_SELF* selfCommand);

//任务函数
void TaskServiceFuncNeedleSample(void *p_arg)
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
                TaskServiceCommandProc_NeedleSample_UartRS1(uartRS1CommandPtr);
                //处置完成,执行后续操作
                UserMemFree(SRAM_CCM,uartRS1CommandPtr);
            }
            else if (SYSTEM_CMD_SRC_SELF == taskSystemRecvCommandPtr->commandSrc)
            {
                //自身指令源
                SYSTEM_CMD_SELF* selfCommandPtr =  (SYSTEM_CMD_SELF*)(taskSystemRecvCommandPtr->systemCommandDataPtr);
                //对接收到的数据进行处理
                TaskServiceCommandProc_NeedleSample_Self(selfCommandPtr);
                //处置完成,执行后续操作
                UserMemFree(SRAM_CCM,selfCommandPtr);
            }
            else if(SYSTEM_CMD_SRC_NET_W5500 == taskSystemRecvCommandPtr->commandSrc)
            {
                //网口指令源
                SYSTEM_CMD_NET_W5500* netW5500CommandPtr =  (SYSTEM_CMD_NET_W5500*)(taskSystemRecvCommandPtr->systemCommandDataPtr);
                //对接收到的数据进行处理
                TaskServiceCommandProc_NeedleSample_NetW5500(netW5500CommandPtr);
                //处置完成,执行后续操作
                UserMemFree(SRAM_CCM,netW5500CommandPtr);
            }
            else if(SYSTEM_CMD_SRC_CAN1 == taskSystemRecvCommandPtr->commandSrc)
            {
                //CAN1口指令源
                SYSTEM_CMD_CAN1* can1CommandPtr =  (SYSTEM_CMD_CAN1*)(taskSystemRecvCommandPtr->systemCommandDataPtr);
                //对接收到的数据进行处理
                TaskServiceCommandProc_NeedleSample_Can1(can1CommandPtr);
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
//样本针 升降复位
static LH_ERR TaskServiceCommandProc_NeedleSample_UartRS1_ResetUpdown(SYSTEM_CMD_UART_RS1* uartCommandPtr,uint16_t* resultData);
//样本针 旋转复位
static LH_ERR TaskServiceCommandProc_NeedleSample_UartRS1_ResetRotate(SYSTEM_CMD_UART_RS1* uartCommandPtr,uint16_t* resultData);
//样本针 模块复位
static LH_ERR TaskServiceCommandProc_NeedleSample_UartRS1_ResetAll(SYSTEM_CMD_UART_RS1* uartCommandPtr,uint16_t* resultData);
//样本针 泵复位
static LH_ERR TaskServiceCommandProc_NeedleSample_UartRS1_ResetPump(SYSTEM_CMD_UART_RS1* uartCommandPtr,uint16_t* resultData);
//样本针 旋转到清洗位
static LH_ERR TaskServiceCommandProc_NeedleSample_UartRS1_RotateClean(SYSTEM_CMD_UART_RS1* uartCommandPtr,uint16_t* resultData);
//样本针 旋转到强洗位
static LH_ERR TaskServiceCommandProc_NeedleSample_UartRS1_RotateForceClean(SYSTEM_CMD_UART_RS1* uartCommandPtr,uint16_t* resultData);
//样本针 旋转到试管架1
static LH_ERR TaskServiceCommandProc_NeedleSample_UartRS1_RotateAbsorbRank1(SYSTEM_CMD_UART_RS1* uartCommandPtr,uint16_t* resultData);
//样本针 旋转到试管架2
static LH_ERR TaskServiceCommandProc_NeedleSample_UartRS1_RotateAbsorbRank2(SYSTEM_CMD_UART_RS1* uartCommandPtr,uint16_t* resultData);
//样本针 旋转到反应盘排液
static LH_ERR TaskServiceCommandProc_NeedleSample_UartRS1_RotateInjectReaction(SYSTEM_CMD_UART_RS1* uartCommandPtr,uint16_t* resultData);
//样本针 旋转到反应盘吸液
static LH_ERR TaskServiceCommandProc_NeedleSample_UartRS1_RotateAbsorbReaction(SYSTEM_CMD_UART_RS1* uartCommandPtr,uint16_t* resultData);
//样本针 吸液
static LH_ERR TaskServiceCommandProc_NeedleSample_UartRS1_Absorb(SYSTEM_CMD_UART_RS1* uartCommandPtr,uint16_t* resultData);
//样本针 排液
static LH_ERR TaskServiceCommandProc_NeedleSample_UartRS1_Inject(SYSTEM_CMD_UART_RS1* uartCommandPtr,uint16_t* resultData);
//样本针 清洗
static LH_ERR TaskServiceCommandProc_NeedleSample_UartRS1_Clean(SYSTEM_CMD_UART_RS1* uartCommandPtr,uint16_t* resultData);
//样本针 强洗
static LH_ERR TaskServiceCommandProc_NeedleSample_UartRS1_ForceClean(SYSTEM_CMD_UART_RS1* uartCommandPtr,uint16_t* resultData);
//样本针 灌注
static LH_ERR TaskServiceCommandProc_NeedleSample_UartRS1_Prime(SYSTEM_CMD_UART_RS1* uartCommandPtr,uint16_t* resultData);
//样本针 强灌注
static LH_ERR TaskServiceCommandProc_NeedleSample_UartRS1_ForcePrime(SYSTEM_CMD_UART_RS1* uartCommandPtr,uint16_t* resultData);

//定义函数数组,用于串口处理线程回调
static const TaskServiceProcPtrWithUartCommand taskServiceProcNeedleSamplePtrWithUartCommandArray[] = {
	TaskServiceCommandProc_NeedleSample_UartRS1_ResetUpdown,
	TaskServiceCommandProc_NeedleSample_UartRS1_ResetRotate,
	TaskServiceCommandProc_NeedleSample_UartRS1_ResetAll,
	TaskServiceCommandProc_NeedleSample_UartRS1_ResetPump,
	TaskServiceCommandProc_NeedleSample_UartRS1_RotateClean,
	TaskServiceCommandProc_NeedleSample_UartRS1_RotateForceClean,
	TaskServiceCommandProc_NeedleSample_UartRS1_RotateAbsorbRank1,
	TaskServiceCommandProc_NeedleSample_UartRS1_RotateAbsorbRank2,
	TaskServiceCommandProc_NeedleSample_UartRS1_RotateInjectReaction,
	TaskServiceCommandProc_NeedleSample_UartRS1_RotateAbsorbReaction,
	TaskServiceCommandProc_NeedleSample_UartRS1_Absorb,
	TaskServiceCommandProc_NeedleSample_UartRS1_Inject,
	TaskServiceCommandProc_NeedleSample_UartRS1_Clean,
	TaskServiceCommandProc_NeedleSample_UartRS1_ForceClean,
	TaskServiceCommandProc_NeedleSample_UartRS1_Prime,
	TaskServiceCommandProc_NeedleSample_UartRS1_ForcePrime,
};

//回调函数数量
#define NEEDLE_SAMPLE_FUNC_UART_COUNT_MAX      (sizeof(taskServiceProcNeedleSamplePtrWithUartCommandArray)/sizeof(taskServiceProcNeedleSamplePtrWithUartCommandArray[0]))


//串口指令的第一级处理
static void TaskServiceCommandProc_NeedleSample_UartRS1(SYSTEM_CMD_UART_RS1* uartCommand)
{
    LH_ERR errorCode = LH_ERR_NONE;
	//发送ACK,创建一条新数据包
	SystemUartRS_ModbusCreatePackage(uartCommand->uartRecvCommandArray,uartCommand->srcDeviceAddr,uartCommand->frameNo,
						uartCommand->funcCode,uartCommand->regAddr,uartCommand->regValue);
	BoardRS_SendBuffer(ACTION_DEBUG_PORT,uartCommand->uartRecvCommandArray,12);

    //调用指令
	uint8_t actionCode= ((uartCommand->regAddr)>>8);
    uint16_t resultData = 0x0000;
	if(actionCode > NEEDLE_SAMPLE_FUNC_UART_COUNT_MAX)
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
		errorCode = taskServiceProcNeedleSamplePtrWithUartCommandArray[actionCode-1](uartCommand,&resultData);
	}
	SystemUartRS_ModbusCreatePackage(uartCommand->uartRecvCommandArray, uartCommand->srcDeviceAddr, uartCommand->frameNo,
							0x01, (uint16_t)errorCode, resultData);
	//发送一个填充好的数据包
	BoardRS_SendBuffer(ACTION_DEBUG_PORT,uartCommand->uartRecvCommandArray,12);
}

//样本针 升降复位
static LH_ERR TaskServiceCommandProc_NeedleSample_UartRS1_ResetUpdown(SYSTEM_CMD_UART_RS1* uartCommandPtr,uint16_t* resultData)
{
    LH_ERR errorCode;
	TSC_NEEDLE_SAMPLE_POS currentNeedleRotatePos;

	errorCode = ActionNeedleSample_Init(&currentNeedleRotatePos);
	return errorCode;
}

//样本针 旋转复位
static LH_ERR TaskServiceCommandProc_NeedleSample_UartRS1_ResetRotate(SYSTEM_CMD_UART_RS1* uartCommandPtr,uint16_t* resultData)
{
    LH_ERR errorCode;
	TSC_NEEDLE_SAMPLE_POS currentNeedleRotatePos;
    
	errorCode = ActionNeedleSample_Init(&currentNeedleRotatePos);
	return errorCode;
}

//样本针 模块复位
static LH_ERR TaskServiceCommandProc_NeedleSample_UartRS1_ResetAll(SYSTEM_CMD_UART_RS1* uartCommandPtr,uint16_t* resultData)
{
    LH_ERR errorCode;
	TSC_NEEDLE_SAMPLE_POS currentNeedleRotatePos;
    
	errorCode = ActionNeedleSample_Init(&currentNeedleRotatePos);
	return errorCode;
}

//样本针 泵复位
static LH_ERR TaskServiceCommandProc_NeedleSample_UartRS1_ResetPump(SYSTEM_CMD_UART_RS1* uartCommandPtr,uint16_t* resultData)
{
    LH_ERR errorCode;
	TSC_NEEDLE_SAMPLE_POS currentNeedleRotatePos;
    
	errorCode = ActionNeedleSample_Init(&currentNeedleRotatePos);
	return errorCode;
}

//样本针 旋转到清洗位
static LH_ERR TaskServiceCommandProc_NeedleSample_UartRS1_RotateClean(SYSTEM_CMD_UART_RS1* uartCommandPtr,uint16_t* resultData)
{
    LH_ERR errorCode;
	TSC_NEEDLE_SAMPLE_POS currentNeedleRotatePos;
	errorCode = ActionNeedleSample_RotateSpecialPos(TSC_NEEDLE_SAMPLE_POS_CLEAN,&currentNeedleRotatePos);
	return errorCode;
}

//样本针 旋转到强洗位
static LH_ERR TaskServiceCommandProc_NeedleSample_UartRS1_RotateForceClean(SYSTEM_CMD_UART_RS1* uartCommandPtr,uint16_t* resultData)
{
    LH_ERR errorCode;
	TSC_NEEDLE_SAMPLE_POS currentNeedleRotatePos;
	errorCode = ActionNeedleSample_RotateSpecialPos(TSC_NEEDLE_SAMPLE_POS_FORCE_CLEAN,&currentNeedleRotatePos);
	return errorCode;
}

//样本针 旋转到试管架1
static LH_ERR TaskServiceCommandProc_NeedleSample_UartRS1_RotateAbsorbRank1(SYSTEM_CMD_UART_RS1* uartCommandPtr,uint16_t* resultData)
{
    LH_ERR errorCode;
	TSC_NEEDLE_SAMPLE_POS currentNeedleRotatePos;
	errorCode = ActionNeedleSample_RotateSpecialPos(TSC_NEEDLE_SAMPLE_POS_RACK,&currentNeedleRotatePos);
	return errorCode;
}

//样本针 旋转到试管架2
static LH_ERR TaskServiceCommandProc_NeedleSample_UartRS1_RotateAbsorbRank2(SYSTEM_CMD_UART_RS1* uartCommandPtr,uint16_t* resultData)
{
    LH_ERR errorCode;
	TSC_NEEDLE_SAMPLE_POS currentNeedleRotatePos;
	errorCode = ActionNeedleSample_RotateSpecialPos(TSC_NEEDLE_SAMPLE_POS_RACK,&currentNeedleRotatePos);
	return errorCode;
}

//样本针 旋转到反应盘排液
static LH_ERR TaskServiceCommandProc_NeedleSample_UartRS1_RotateInjectReaction(SYSTEM_CMD_UART_RS1* uartCommandPtr,uint16_t* resultData)
{
    LH_ERR errorCode;
	TSC_NEEDLE_SAMPLE_POS currentNeedleRotatePos;
	errorCode = ActionNeedleSample_RotateSpecialPos(TSC_NEEDLE_SAMPLE_POS_REACTION_INJECT,&currentNeedleRotatePos);
	return errorCode;
}

//样本针 旋转到反应盘吸液
static LH_ERR TaskServiceCommandProc_NeedleSample_UartRS1_RotateAbsorbReaction(SYSTEM_CMD_UART_RS1* uartCommandPtr,uint16_t* resultData)
{
    LH_ERR errorCode;
	TSC_NEEDLE_SAMPLE_POS currentNeedleRotatePos;
	errorCode = ActionNeedleSample_RotateSpecialPos(TSC_NEEDLE_SAMPLE_POS_REACTION_ABSORB,&currentNeedleRotatePos);
	return errorCode;
}

//样本针 吸液
static LH_ERR TaskServiceCommandProc_NeedleSample_UartRS1_Absorb(SYSTEM_CMD_UART_RS1* uartCommandPtr,uint16_t* resultData)
{
    LH_ERR errorCode;
	TSC_NEEDLE_SAMPLE_POS currentNeedleRotatePos;
	uint8_t paramLow = (uint8_t)(uartCommandPtr->regValue);
	static uint8_t flag=1;//测试用
	SAMPLE_STATUS use;
	if(flag == 1)//测试用
	{
		use = SAMPLE_NEW;
		flag = 0;
	}
	else//测试用
	{
		use = SAMPLE_USED;
	}
	
	errorCode = ActionNeedleSample_AbsorbLiquid(paramLow,&currentNeedleRotatePos,use);
	return errorCode;
}

//样本针 排液
static LH_ERR TaskServiceCommandProc_NeedleSample_UartRS1_Inject(SYSTEM_CMD_UART_RS1* uartCommandPtr,uint16_t* resultData)
{
    LH_ERR errorCode;
	TSC_NEEDLE_SAMPLE_POS currentNeedleRotatePos;
	uint8_t paramLow = (uint8_t)(uartCommandPtr->regValue);

	errorCode = ActionNeedleSample_InjectLiquid(paramLow,&currentNeedleRotatePos);

	return errorCode;
}

//样本针 清洗
static LH_ERR TaskServiceCommandProc_NeedleSample_UartRS1_Clean(SYSTEM_CMD_UART_RS1* uartCommandPtr,uint16_t* resultData)
{
    LH_ERR errorCode;
	TSC_NEEDLE_SAMPLE_POS currentNeedleRotatePos;

	uint8_t paramLow = (uint8_t)(uartCommandPtr->regValue);
	uint8_t paramHigh = (uint8_t)((uartCommandPtr->regValue)>>8);
	TSC_NEEDLE_SAMPLE_CLEAN_OPT opt = TSC_NEEDLE_SAMPLE_CLEAN_OPT_ALL;
	if(paramHigh == 0)
	{
		paramHigh = 0x03;
	}
	if(paramHigh&0x03)
	{
		opt = TSC_NEEDLE_SAMPLE_CLEAN_OPT_ALL;
	}
	else if(paramHigh & 0x01)
	{
		opt = TSC_NEEDLE_SAMPLE_CLEAN_OPT_INNER;
	}
	else//0x02
	{
		opt = TSC_NEEDLE_SAMPLE_CLEAN_OPT_OUT;
	}
	
	if(paramLow == 0)
	{
        //清洗完成旋转到试管架上方
		errorCode = ActionNeedleSample_Clean(opt,TSC_NEEDLE_SAMPLE_POS_RACK,&currentNeedleRotatePos);
	}
	else
	{
        //清洗完成旋转到反应盘上方
		errorCode = ActionNeedleSample_Clean(opt,TSC_NEEDLE_SAMPLE_POS_REACTION_ABSORB,&currentNeedleRotatePos);
	}
	return errorCode;
}

//样本针 强洗
static LH_ERR TaskServiceCommandProc_NeedleSample_UartRS1_ForceClean(SYSTEM_CMD_UART_RS1* uartCommandPtr,uint16_t* resultData)
{
    LH_ERR errorCode;
	TSC_NEEDLE_SAMPLE_POS currentNeedleRotatePos;

	errorCode = ActionNeedleSample_ForceClean(&currentNeedleRotatePos);
	return errorCode;
}

//样本针 灌注
static LH_ERR TaskServiceCommandProc_NeedleSample_UartRS1_Prime(SYSTEM_CMD_UART_RS1* uartCommandPtr,uint16_t* resultData)
{
    LH_ERR errorCode;
	TSC_NEEDLE_SAMPLE_POS currentNeedleRotatePos;

	//	uint8_t paramLow = (uint8_t)(uartCommandPtr->uartRecvCommandArray[7]);
	uint8_t paramHigh = (uint8_t)(uartCommandPtr->uartRecvCommandArray[8]);
	TSC_NEEDLE_SAMPLE_PRIME_OPT opt = TSC_NEEDLE_SAMPLE_PRIME_OPT_ALL;

	if(paramHigh == 0x01)
	{
		opt = TSC_NEEDLE_SAMPLE_PRIME_OPT_INNER;
	}
	else if(paramHigh == 0x02)
	{
		opt = TSC_NEEDLE_SAMPLE_PRIME_OPT_OUT;
	}
    else
    {
        opt = TSC_NEEDLE_SAMPLE_PRIME_OPT_ALL;
    }

	errorCode = ActionNeedleSample_Prime(opt,&currentNeedleRotatePos);
	return errorCode;
}

//样本针 强灌注
static LH_ERR TaskServiceCommandProc_NeedleSample_UartRS1_ForcePrime(SYSTEM_CMD_UART_RS1* uartCommandPtr,uint16_t* resultData)
{
    LH_ERR errorCode;
	TSC_NEEDLE_SAMPLE_POS currentNeedleRotatePos;

	errorCode = ActionNeedleSample_ForcePrime(&currentNeedleRotatePos);
	return errorCode;
}



/********************************************************************************************************************/

/*****************************************************CAN指令处理*****************************************************/
//样本针垂直初始化
static LH_ERR TaskServiceCommandProc_NeedleSample_Can1_ResetUpdown(SYSTEM_CMD_CAN1* canCommand)
{
	LH_ERR errorCode;
	TSC_NEEDLE_SAMPLE_POS currentNeedleRotatePos;
	uint8_t count;
	errorCode = ActionNeedleSample_Init(&currentNeedleRotatePos);
	count = Arraypack_Common(canCommand,0,errorCode);
	CanSendDataToCell(TESTPROC_CELL_PLATFORM_NMBR,BOARD_ID_TRACK_COMM,0x0018,canCommand->can1RecvDataBuffer,count);
	return errorCode;
}
//样本针 旋转复位
static LH_ERR TaskServiceCommandProc_NeedleSample_Can1_ResetRotate(SYSTEM_CMD_CAN1* canCommand)
{
    LH_ERR errorCode;
	TSC_NEEDLE_SAMPLE_POS currentNeedleRotatePos;
	uint8_t count;
	
	errorCode = ActionNeedleSample_Init(&currentNeedleRotatePos);
    count = Arraypack_Common(canCommand,0,errorCode);

	CanSendDataToCell(TESTPROC_CELL_PLATFORM_NMBR,BOARD_ID_TRACK_COMM,0x0018,canCommand->can1RecvDataBuffer,count);
	return errorCode;
}
//样本针 模块复位
static LH_ERR TaskServiceCommandProc_NeedleSample_Can1_ResetAll(SYSTEM_CMD_CAN1* canCommand)
{
    LH_ERR errorCode;
	TSC_NEEDLE_SAMPLE_POS currentNeedleRotatePos;
    uint8_t count;
	errorCode = ActionNeedleSample_Init(&currentNeedleRotatePos);
	count = Arraypack_Common(canCommand,0,errorCode);
	CanSendDataToCell(TESTPROC_CELL_PLATFORM_NMBR,BOARD_ID_TRACK_COMM,0x0018,canCommand->can1RecvDataBuffer,count);
	return errorCode;
}
//样本针 泵复位
static LH_ERR TaskServiceCommandProc_NeedleSample_Can1_ResetPump(SYSTEM_CMD_CAN1* canCommand)
{
    LH_ERR errorCode;
	TSC_NEEDLE_SAMPLE_POS currentNeedleRotatePos;
	uint8_t count;
	errorCode = ActionNeedleSample_Init(&currentNeedleRotatePos);
    count = Arraypack_Common(canCommand,0,errorCode);
	CanSendDataToCell(TESTPROC_CELL_PLATFORM_NMBR,BOARD_ID_TRACK_COMM,0x0018,canCommand->can1RecvDataBuffer,count);
	return errorCode;
}
//样本针 旋转到清洗位
static LH_ERR TaskServiceCommandProc_NeedleSample_Can1_RotateClean(SYSTEM_CMD_CAN1* canCommand)
{
    LH_ERR errorCode;
	TSC_NEEDLE_SAMPLE_POS currentNeedleRotatePos;
	uint8_t count;
	errorCode = ActionNeedleSample_RotateSpecialPos(TSC_NEEDLE_SAMPLE_POS_CLEAN,&currentNeedleRotatePos);
	count = Arraypack_Common(canCommand,0,errorCode);
	CanSendDataToCell(TESTPROC_CELL_PLATFORM_NMBR,BOARD_ID_TRACK_COMM,0x0018,canCommand->can1RecvDataBuffer,count);
	return errorCode;
}
//样本针 旋转到强洗位
static LH_ERR TaskServiceCommandProc_NeedleSample_Can1_RotateForceClean(SYSTEM_CMD_CAN1* canCommand)
{
    LH_ERR errorCode;
	TSC_NEEDLE_SAMPLE_POS currentNeedleRotatePos;
	uint8_t count;
	errorCode = ActionNeedleSample_RotateSpecialPos(TSC_NEEDLE_SAMPLE_POS_FORCE_CLEAN,&currentNeedleRotatePos);
	count = Arraypack_Common(canCommand,0,errorCode);
	CanSendDataToCell(TESTPROC_CELL_PLATFORM_NMBR,BOARD_ID_TRACK_COMM,0x0018,canCommand->can1RecvDataBuffer,count);
	return errorCode;
}
//样本针 旋转到样本架位
static LH_ERR TaskServiceCommandProc_NeedleSample_Can1_RotateAbsorbRank2(SYSTEM_CMD_CAN1* canCommand)
{
    LH_ERR errorCode;
	TSC_NEEDLE_SAMPLE_POS currentNeedleRotatePos;
	uint8_t count;
	errorCode = ActionNeedleSample_RotateSpecialPos(TSC_NEEDLE_SAMPLE_POS_RACK,&currentNeedleRotatePos);
	count = Arraypack_Common(canCommand,0,errorCode);
	CanSendDataToCell(TESTPROC_CELL_PLATFORM_NMBR,BOARD_ID_TRACK_COMM,0x0018,canCommand->can1RecvDataBuffer,count);
	return errorCode;
}
//样本针 旋转到反应盘注液位
static LH_ERR TaskServiceCommandProc_NeedleSample_Can1_RotateInjectReaction(SYSTEM_CMD_CAN1* canCommand)
{
    LH_ERR errorCode;
	TSC_NEEDLE_SAMPLE_POS currentNeedleRotatePos;
	uint8_t count;
	errorCode = ActionNeedleSample_RotateSpecialPos(TSC_NEEDLE_SAMPLE_POS_REACTION_INJECT,&currentNeedleRotatePos);
	count = Arraypack_Common(canCommand,0,errorCode);
	CanSendDataToCell(TESTPROC_CELL_PLATFORM_NMBR,BOARD_ID_TRACK_COMM,0x0018,canCommand->can1RecvDataBuffer,count);
	return errorCode;
}
//样本针 旋转到反应盘吸液位
static LH_ERR TaskServiceCommandProc_NeedleSample_Can1_RotateAbsorbReaction(SYSTEM_CMD_CAN1* canCommand)
{
    LH_ERR errorCode;
	TSC_NEEDLE_SAMPLE_POS currentNeedleRotatePos;
	uint8_t count;
	errorCode = ActionNeedleSample_RotateSpecialPos(TSC_NEEDLE_SAMPLE_POS_REACTION_ABSORB,&currentNeedleRotatePos);
	count = Arraypack_Common(canCommand,0,errorCode);
	CanSendDataToCell(TESTPROC_CELL_PLATFORM_NMBR,BOARD_ID_TRACK_COMM,0x0018,canCommand->can1RecvDataBuffer,count);
	return errorCode;
}
//样本针 吸液
static LH_ERR TaskServiceCommandProc_NeedleSample_Can1_Absorb(SYSTEM_CMD_CAN1* canCommand)
{
    LH_ERR errorCode;
	TSC_NEEDLE_SAMPLE_POS currentNeedleRotatePos;
	uint8_t count;
	uint16_t paramLow;
	uint32_t yogi;
	
	yogi = *(uint32_t *)(&canCommand->can1RecvDataBuffer[0]);
	paramLow = (uint16_t)yogi;
	SAMPLE_STATUS use = SAMPLE_NEW;
	errorCode = ActionNeedleSample_AbsorbLiquid(paramLow,&currentNeedleRotatePos,use);
	count = Arraypack_Common(canCommand,paramLow,errorCode);
	CanSendDataToCell(TESTPROC_CELL_PLATFORM_NMBR,BOARD_ID_TRACK_COMM,0x0018,canCommand->can1RecvDataBuffer,count);
	return errorCode;
}

//样本针 排液
static LH_ERR TaskServiceCommandProc_NeedleSample_Can1_Inject(SYSTEM_CMD_CAN1* canCommand)
{
    LH_ERR errorCode;
	TSC_NEEDLE_SAMPLE_POS currentNeedleRotatePos;
	uint32_t yogi;
	uint16_t paramLow,count;
	yogi = *(uint32_t *)(&canCommand->can1RecvDataBuffer[0]);
	paramLow = (uint16_t)yogi;

	errorCode = ActionNeedleSample_InjectLiquid(paramLow,&currentNeedleRotatePos);
	count = Arraypack_Common(canCommand,paramLow,errorCode);
	CanSendDataToCell(TESTPROC_CELL_PLATFORM_NMBR,BOARD_ID_TRACK_COMM,0x0018,canCommand->can1RecvDataBuffer,count);
	return errorCode;
}

//样本针 清洗
static LH_ERR TaskServiceCommandProc_NeedleSample_Can1_Clean(SYSTEM_CMD_CAN1* canCommand)
{
    LH_ERR errorCode;
	TSC_NEEDLE_SAMPLE_POS currentNeedleRotatePos;

	uint16_t paramLow =  (canCommand->can1RecvDataBuffer[1]<<8)|canCommand->can1RecvDataBuffer[0];
	uint16_t paramHigh = (canCommand->can1RecvDataBuffer[3]<<8)|canCommand->can1RecvDataBuffer[2];
	uint8_t count;
	TSC_NEEDLE_SAMPLE_CLEAN_OPT opt = TSC_NEEDLE_SAMPLE_CLEAN_OPT_ALL;
	if(paramHigh == 0)
	{
		paramHigh = 0x03;
	}
	if(paramHigh&0x03)
	{
		opt = TSC_NEEDLE_SAMPLE_CLEAN_OPT_ALL;
	}
	else if(paramHigh & 0x01)
	{
		opt = TSC_NEEDLE_SAMPLE_CLEAN_OPT_INNER;
	}
	else//0x02
	{
		opt = TSC_NEEDLE_SAMPLE_CLEAN_OPT_OUT;
	}
	
	if(paramLow == 0)
	{
        //清洗完成旋转到试管架上方--->清洗位
		errorCode = ActionNeedleSample_Clean(opt,TSC_NEEDLE_SAMPLE_POS_CLEAN,&currentNeedleRotatePos);
	}
	else
	{
        //清洗完成旋转到反应盘上方---->清洗位
		errorCode = ActionNeedleSample_Clean(opt,TSC_NEEDLE_SAMPLE_POS_CLEAN,&currentNeedleRotatePos);
	}
	count = Arraypack_Common(canCommand,paramLow,errorCode);
	CanSendDataToCell(TESTPROC_CELL_PLATFORM_NMBR,BOARD_ID_TRACK_COMM,0x0018,canCommand->can1RecvDataBuffer,count);
	return errorCode;
}
//样本针 强洗
static LH_ERR TaskServiceCommandProc_NeedleSample_Can1_ForceClean(SYSTEM_CMD_CAN1* canCommand)
{
    LH_ERR errorCode;
	TSC_NEEDLE_SAMPLE_POS currentNeedleRotatePos;
	uint8_t count;

	errorCode = ActionNeedleSample_ForceClean(&currentNeedleRotatePos);
	count = Arraypack_Common(canCommand,0,errorCode);
	CanSendDataToCell(TESTPROC_CELL_PLATFORM_NMBR,BOARD_ID_TRACK_COMM,0x0018,canCommand->can1RecvDataBuffer,count);
	return errorCode;
}

//样本针 灌注
static LH_ERR TaskServiceCommandProc_NeedleSample_Can1_Prime(SYSTEM_CMD_CAN1* canCommand)
{
    LH_ERR errorCode;
	TSC_NEEDLE_SAMPLE_POS currentNeedleRotatePos;

	uint8_t paramHigh = canCommand->can1RecvDataBuffer[3];
	uint8_t count;
	TSC_NEEDLE_SAMPLE_PRIME_OPT opt = TSC_NEEDLE_SAMPLE_PRIME_OPT_ALL;

	if(paramHigh == 0x01)
	{
		opt = TSC_NEEDLE_SAMPLE_PRIME_OPT_INNER;
	}
	else if(paramHigh == 0x02)
	{
		opt = TSC_NEEDLE_SAMPLE_PRIME_OPT_OUT;
	}
    else
    {
        opt = TSC_NEEDLE_SAMPLE_PRIME_OPT_ALL;
    }

	errorCode = ActionNeedleSample_Prime(opt,&currentNeedleRotatePos);
	count = Arraypack_Common(canCommand,paramHigh,errorCode);
	CanSendDataToCell(TESTPROC_CELL_PLATFORM_NMBR,BOARD_ID_TRACK_COMM,0x0018,canCommand->can1RecvDataBuffer,count);
	return errorCode;
}
//样本针 强灌注
static LH_ERR TaskServiceCommandProc_NeedleSample_Can1_ForcePrime(SYSTEM_CMD_CAN1* canCommand)
{
    LH_ERR errorCode;
	TSC_NEEDLE_SAMPLE_POS currentNeedleRotatePos;
	uint8_t count;
	errorCode = ActionNeedleSample_ForcePrime(&currentNeedleRotatePos);
	count = Arraypack_Common(canCommand,0,errorCode);
	CanSendDataToCell(TESTPROC_CELL_PLATFORM_NMBR,BOARD_ID_TRACK_COMM,0x0018,canCommand->can1RecvDataBuffer,count);
	return errorCode;
}
//调试指令处理
const APP_TASK_CAN1_PROC_UNIT appTaskNeedleSampleCan1ProcArray[] = {

    {CAN1_COMMAND_NEEDLE_SAMPLE_VERTICAL_RESET                          ,        TaskServiceCommandProc_NeedleSample_Can1_ResetUpdown},//样本针垂直初始化ruan
    {CAN1_COMMAND_NEEDLE_SAMPLE_LEVEL_RESET                             ,        TaskServiceCommandProc_NeedleSample_Can1_ResetRotate},//样本针水平初始化
    {CAN1_COMMAND_NEEDLE_SAMPLE_VERTICAL_LEVEL_INIT                     ,        TaskServiceCommandProc_NeedleSample_Can1_ResetAll},//垂直水平初始化
    {CAN1_COMMAND_NEEDLE_SAMPLE_PUMP_RESET                              ,        TaskServiceCommandProc_NeedleSample_Can1_ResetPump},//泵初始化
    {CAN1_COMMAND_NEEDLE_SAMPLE_WASH_POS                                ,        TaskServiceCommandProc_NeedleSample_Can1_RotateClean},//样本针转到清洗位
    {CAN1_COMMAND_NEEDLE_SAMPLE_FORCE_WASH_POS                          ,        TaskServiceCommandProc_NeedleSample_Can1_RotateForceClean},//样本针转到强洗位
    {CAN1_COMMAND_NEEDLE_SAMPLE_SAMPLING_POS                            ,        TaskServiceCommandProc_NeedleSample_Can1_RotateAbsorbRank2},//样本针转到吸样本位
    {CAN1_COMMAND_NEEDLE_SAMPLE_SAMPLE_FRAME                            ,        TaskServiceCommandProc_NeedleSample_Can1_RotateAbsorbRank2},//样本针转到样本架
    {CAN1_COMMAND_NEEDLE_SAMPLE_PUSH_POS                                ,        TaskServiceCommandProc_NeedleSample_Can1_RotateInjectReaction},//转到反应盘注液位上
    {CAN1_COMMAND_NEEDLE_SAMPLE_DILUTION_POS                            ,        TaskServiceCommandProc_NeedleSample_Can1_RotateAbsorbReaction},//样本针转到反应盘稀释样本位上
    {CAN1_COMMAND_NEEDLE_SAMPLE_ABSORB                                  ,        TaskServiceCommandProc_NeedleSample_Can1_Absorb},//样本针吸样本
    {CAN1_COMMAND_NEEDLE_SAMPLE_ROW                                     ,        TaskServiceCommandProc_NeedleSample_Can1_Inject},//样本针排样本
    {CAN1_COMMAND_NEEDLE_SAMPLE_WASH                                    ,        TaskServiceCommandProc_NeedleSample_Can1_Clean},//样本针清洗
    {CAN1_COMMAND_NEEDLE_SAMPLE_FORCEWASH                               ,        TaskServiceCommandProc_NeedleSample_Can1_ForceClean},//样本针强清洗
    {CAN1_COMMAND_NEEDLE_SAMPLE_PRIME                                   ,        TaskServiceCommandProc_NeedleSample_Can1_Prime},//样本针灌注
    {CAN1_COMMAND_NEEDLE_SAMPLE_FORCEPRIME                              ,        TaskServiceCommandProc_NeedleSample_Can1_ForcePrime},//样本针强灌注
};

#define COUNT_NEEDLE_SAMPLE     (sizeof(appTaskNeedleSampleCan1ProcArray)/sizeof(appTaskNeedleSampleCan1ProcArray[0]))

static void TaskServiceCommandProc_NeedleSample_Can1(SYSTEM_CMD_CAN1* canCommand)
{
	uint16_t indexUtil = 0;
    for(indexUtil = 0; indexUtil < COUNT_NEEDLE_SAMPLE; indexUtil++)
    {
        //指令匹配
        if(canCommand->command == appTaskNeedleSampleCan1ProcArray[indexUtil].command)
        {
            //调用处理函数
            if(appTaskNeedleSampleCan1ProcArray[indexUtil].can1CommandProcFunc != NULL)
            {
                appTaskNeedleSampleCan1ProcArray[indexUtil].can1CommandProcFunc(canCommand);
                break;
            }
        }
    }
}

/********************************************************************************************************************/

/*******************************************************网口指令处理**************************************************/

static void TaskServiceCommandProc_NeedleSample_NetW5500(SYSTEM_CMD_NET_W5500* netCommand)
{

}

/********************************************************************************************************************/

/*******************************************************自身指令处理**************************************************/
//样本针 初始化
static LH_ERR TaskServiceCommandProc_NeedleSample_Self_Init(SYSTEM_CMD_SELF* selfCommandPtr);
//样本针 旋转到指定位置
static LH_ERR TaskServiceCommandProc_NeedleSample_Self_RotateSpecialPosition(SYSTEM_CMD_SELF* selfCommandPtr);
//样本针 吸液一次
static LH_ERR TaskServiceCommandProc_NeedleSample_Self_AbsorbOnce(SYSTEM_CMD_SELF* selfCommandPtr);
//样本针 注液一次
static LH_ERR TaskServiceCommandProc_NeedleSample_Self_InjectOnce(SYSTEM_CMD_SELF* selfCommandPtr);
//样本针 清洗
static LH_ERR TaskServiceCommandProc_NeedleSample_Self_Clean(SYSTEM_CMD_SELF* selfCommandPtr);
//样本针 强力清洗
static LH_ERR TaskServiceCommandProc_NeedleSample_Self_ForceClean(SYSTEM_CMD_SELF* selfCommandPtr);
//样本针 灌注
static LH_ERR TaskServiceCommandProc_NeedleSample_Self_Prime(SYSTEM_CMD_SELF* selfCommandPtr);
//样本针 强力灌注
static LH_ERR TaskServiceCommandProc_NeedleSample_Self_ForcePrime(SYSTEM_CMD_SELF* selfCommandPtr);

//自身指令回调函数数组
static const TaskServicePtrWithSelfCommand TaskProcWithNeedleSampleSelfCommandArray[] = {
	TaskServiceCommandProc_NeedleSample_Self_Init,
	TaskServiceCommandProc_NeedleSample_Self_RotateSpecialPosition,
	TaskServiceCommandProc_NeedleSample_Self_AbsorbOnce,
	TaskServiceCommandProc_NeedleSample_Self_InjectOnce,
	TaskServiceCommandProc_NeedleSample_Self_Clean,
	TaskServiceCommandProc_NeedleSample_Self_ForceClean,
	TaskServiceCommandProc_NeedleSample_Self_Prime,
	TaskServiceCommandProc_NeedleSample_Self_ForcePrime,
};

//支持指令的个数
#define TASK_COMMAND_PROC_SELF_COUNT	(sizeof(TaskProcWithNeedleSampleSelfCommandArray)/sizeof(TaskProcWithNeedleSampleSelfCommandArray[0]))

//用于调试指令的头调试字符串
static const uint8_t TaskSelfCommandDebugStartMsgHeadString[] = "$$Action 10 Start : ";
static const uint8_t TaskSelfCommandDebugEndMsgHeadString[] = "$$Action 10 End ";

static void TaskServiceCommandProc_NeedleSample_Self(SYSTEM_CMD_SELF* selfCommand)
{
	OS_ERR err;
	LH_ERR actionResult;
	//指令过程与结果对象
	TSC_CMD_RESULT_DATA_NEEDLE_SAMPLE* cmdResultPtr = (TSC_CMD_RESULT_DATA_NEEDLE_SAMPLE*)selfCommand->commandDataSpecialChannel;
	
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
		actionResult = TaskProcWithNeedleSampleSelfCommandArray[selfCommand->commandIndex](selfCommand);
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


//样本针 初始化
static LH_ERR TaskServiceCommandProc_NeedleSample_Self_Init(SYSTEM_CMD_SELF* selfCommandPtr)
{
	LH_ERR errCode;
	//打印开始处理信息
	TaskServiceSerialDebugMsgShowStart((uint8_t*)TaskSelfCommandDebugStartMsgHeadString,
										(uint8_t*)"NeedleSampleInit",
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
	
	TSC_NEEDLE_SAMPLE_POS currentNeedleRotatePos;

	TSC_CMD_RESULT_DATA_NEEDLE_SAMPLE* cmdResultPtr = 
				(TSC_CMD_RESULT_DATA_NEEDLE_SAMPLE*)selfCommandPtr->commandDataSpecialChannel;
	//处理
	errCode = ActionNeedleSample_Init(&currentNeedleRotatePos);
	if(errCode == LH_ERR_NONE)
	{
		cmdResultPtr->currentNeedlePosition = currentNeedleRotatePos;
	}
#endif

	return errCode;
}

//样本针 旋转到指定位置
static LH_ERR TaskServiceCommandProc_NeedleSample_Self_RotateSpecialPosition(SYSTEM_CMD_SELF* selfCommandPtr)
{
	LH_ERR errCode;
	//打印开始处理信息
	TaskServiceSerialDebugMsgShowStart((uint8_t*)TaskSelfCommandDebugStartMsgHeadString,
										(uint8_t*)"NeedleSampleRotateSpecialPosition",
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
	
	TSC_NEEDLE_SAMPLE_POS currentNeedleRotatePos;

	TSC_CMD_RESULT_DATA_NEEDLE_SAMPLE* cmdResultPtr = 
				(TSC_CMD_RESULT_DATA_NEEDLE_SAMPLE*)selfCommandPtr->commandDataSpecialChannel;

	TSC_NEEDLE_SAMPLE_POS targetPos = (TSC_NEEDLE_SAMPLE_POS)(selfCommandPtr->commandParam1);
	//处理
	errCode = ActionNeedleSample_RotateSpecialPos(targetPos,&currentNeedleRotatePos);
	if(errCode == LH_ERR_NONE)
	{
		cmdResultPtr->currentNeedlePosition = currentNeedleRotatePos;
	}
#endif

	return errCode;
}

//样本针 吸液一次
static LH_ERR TaskServiceCommandProc_NeedleSample_Self_AbsorbOnce(SYSTEM_CMD_SELF* selfCommandPtr)
{
	LH_ERR errCode;
	//打印开始处理信息
	TaskServiceSerialDebugMsgShowStart((uint8_t*)TaskSelfCommandDebugStartMsgHeadString,
										(uint8_t*)"NeedleSampleAbsorbOnce",
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
	
	TSC_NEEDLE_SAMPLE_POS currentNeedleRotatePos;

	TSC_CMD_RESULT_DATA_NEEDLE_SAMPLE* cmdResultPtr = 
				(TSC_CMD_RESULT_DATA_NEEDLE_SAMPLE*)selfCommandPtr->commandDataSpecialChannel;

	uint16_t liquidAbsorbUL = selfCommandPtr->commandParam1;
	SAMPLE_STATUS use = (SAMPLE_STATUS)selfCommandPtr->commandParam2;
	//处理
	errCode = ActionNeedleSample_AbsorbLiquid(liquidAbsorbUL,&currentNeedleRotatePos,use);
	if(errCode == LH_ERR_NONE)
	{
		cmdResultPtr->currentNeedlePosition = currentNeedleRotatePos;
	}
#endif

	return errCode;
}

//样本针 注液一次
static LH_ERR TaskServiceCommandProc_NeedleSample_Self_InjectOnce(SYSTEM_CMD_SELF* selfCommandPtr)
{
	LH_ERR errCode;
	//打印开始处理信息
	TaskServiceSerialDebugMsgShowStart((uint8_t*)TaskSelfCommandDebugStartMsgHeadString,
										(uint8_t*)"NeedleSampleInjectOnce",
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
	
	TSC_NEEDLE_SAMPLE_POS currentNeedleRotatePos;

	TSC_CMD_RESULT_DATA_NEEDLE_SAMPLE* cmdResultPtr = 
				(TSC_CMD_RESULT_DATA_NEEDLE_SAMPLE*)selfCommandPtr->commandDataSpecialChannel;
				
	uint16_t liquidInjectUL = selfCommandPtr->commandParam1;
	//处理
	errCode = ActionNeedleSample_InjectLiquid(liquidInjectUL,&currentNeedleRotatePos);
	if(errCode == LH_ERR_NONE)
	{
		cmdResultPtr->currentNeedlePosition = currentNeedleRotatePos;
	}
#endif

	return errCode;
}

//样本针 清洗
static LH_ERR TaskServiceCommandProc_NeedleSample_Self_Clean(SYSTEM_CMD_SELF* selfCommandPtr)
{
	LH_ERR errCode;
	//打印开始处理信息
	TaskServiceSerialDebugMsgShowStart((uint8_t*)TaskSelfCommandDebugStartMsgHeadString,
										(uint8_t*)"NeedleSampleClean",
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
	
	TSC_NEEDLE_SAMPLE_POS currentNeedleRotatePos;

	TSC_CMD_RESULT_DATA_NEEDLE_SAMPLE* cmdResultPtr = 
				(TSC_CMD_RESULT_DATA_NEEDLE_SAMPLE*)selfCommandPtr->commandDataSpecialChannel;

	//结束之后的转换方向
	TSC_NEEDLE_SAMPLE_POS overTargetPos = (TSC_NEEDLE_SAMPLE_POS)(selfCommandPtr->commandParam1);
	TSC_NEEDLE_SAMPLE_CLEAN_OPT opt = (TSC_NEEDLE_SAMPLE_CLEAN_OPT)(selfCommandPtr->commandParam2);
	//处理
	errCode = ActionNeedleSample_Clean(opt,overTargetPos,&currentNeedleRotatePos);
	if(errCode == LH_ERR_NONE)
	{
		cmdResultPtr->currentNeedlePosition = currentNeedleRotatePos;
	}
#endif

	return errCode;
}

//样本针 强力清洗
static LH_ERR TaskServiceCommandProc_NeedleSample_Self_ForceClean(SYSTEM_CMD_SELF* selfCommandPtr)
{
	LH_ERR errCode;
	//打印开始处理信息
	TaskServiceSerialDebugMsgShowStart((uint8_t*)TaskSelfCommandDebugStartMsgHeadString,
										(uint8_t*)"NeedleSampleForceClean",
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
	
	TSC_NEEDLE_SAMPLE_POS currentNeedleRotatePos;

	TSC_CMD_RESULT_DATA_NEEDLE_SAMPLE* cmdResultPtr = 
				(TSC_CMD_RESULT_DATA_NEEDLE_SAMPLE*)selfCommandPtr->commandDataSpecialChannel;
	//处理
	errCode = ActionNeedleSample_ForceClean(&currentNeedleRotatePos);

	if(errCode == LH_ERR_NONE)
	{
		cmdResultPtr->currentNeedlePosition = currentNeedleRotatePos;
	}
#endif

	return errCode;
}

//样本针 灌注
static LH_ERR TaskServiceCommandProc_NeedleSample_Self_Prime(SYSTEM_CMD_SELF* selfCommandPtr)
{
	LH_ERR errCode;
	//打印开始处理信息
	TaskServiceSerialDebugMsgShowStart((uint8_t*)TaskSelfCommandDebugStartMsgHeadString,
										(uint8_t*)"NeedleSamplePrime",
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
	
	TSC_NEEDLE_SAMPLE_POS currentNeedleRotatePos;

	TSC_CMD_RESULT_DATA_NEEDLE_SAMPLE* cmdResultPtr = 
				(TSC_CMD_RESULT_DATA_NEEDLE_SAMPLE*)selfCommandPtr->commandDataSpecialChannel;

	TSC_NEEDLE_SAMPLE_PRIME_OPT opt = (TSC_NEEDLE_SAMPLE_PRIME_OPT)(selfCommandPtr->commandParam1);
	//处理
	errCode = ActionNeedleSample_Prime(opt,&currentNeedleRotatePos);
	if(errCode == LH_ERR_NONE)
	{
		cmdResultPtr->currentNeedlePosition = currentNeedleRotatePos;
	}
#endif

	return errCode;
}

//样本针 强力灌注
static LH_ERR TaskServiceCommandProc_NeedleSample_Self_ForcePrime(SYSTEM_CMD_SELF* selfCommandPtr)
{
	LH_ERR errCode;
	//打印开始处理信息
	TaskServiceSerialDebugMsgShowStart((uint8_t*)TaskSelfCommandDebugStartMsgHeadString,
										(uint8_t*)"NeedleSampleForcePrime",
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
	
	TSC_NEEDLE_SAMPLE_POS currentNeedleRotatePos;

	TSC_CMD_RESULT_DATA_NEEDLE_SAMPLE* cmdResultPtr = 
				(TSC_CMD_RESULT_DATA_NEEDLE_SAMPLE*)selfCommandPtr->commandDataSpecialChannel;

	//处理
	errCode = ActionNeedleSample_ForcePrime(&currentNeedleRotatePos);
	if(errCode == LH_ERR_NONE)
	{
		cmdResultPtr->currentNeedlePosition = currentNeedleRotatePos;
	}
#endif

	return errCode;
}


/********************************************************************************************************************/




