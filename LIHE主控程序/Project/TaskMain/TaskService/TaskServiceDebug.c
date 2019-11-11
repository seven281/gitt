#include "TaskServiceConfig.h"
#include "TSC_Inc.h"
#include "ActionImplementInc.h"
#include "Can1ProcConfig.h"
#include "TestProc_CanConfig.h"
#include "SystemConfig.h"
//任务控制块,8字节对齐
#pragma pack(8)
//任务堆栈
CPU_STK stackBufferTaskServiceDebug[STK_SIZE_TASK_SERVICE_DEBUG];
#pragma pack()

OS_TCB tcbTaskServiceDebug;


//串口指令处理
static void TaskServiceCommandProc_Debug_UartRS1(SYSTEM_CMD_UART_RS1* uartCommand);

//CAN口指令处理
static void TaskServiceCommandProc_Debug_Can1(SYSTEM_CMD_CAN1* canCommand);

//网口指令处理
static void TaskServiceCommandProc_Debug_NetW5500(SYSTEM_CMD_NET_W5500* netCommand);

//自身指令处理
static void TaskServiceCommandProc_Debug_Self(SYSTEM_CMD_SELF* selfCommand);


//任务函数
void TaskServiceFuncDebug(void *p_arg)
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
                TaskServiceCommandProc_Debug_UartRS1(uartRS1CommandPtr);
                //处置完成,执行后续操作
                UserMemFree(SRAM_CCM,uartRS1CommandPtr);
            }
            else if (SYSTEM_CMD_SRC_SELF == taskSystemRecvCommandPtr->commandSrc)
            {
                //自身指令源
                SYSTEM_CMD_SELF* selfCommandPtr =  (SYSTEM_CMD_SELF*)(taskSystemRecvCommandPtr->systemCommandDataPtr);
                //对接收到的数据进行处理
                TaskServiceCommandProc_Debug_Self(selfCommandPtr);
                //处置完成,执行后续操作
                UserMemFree(SRAM_CCM,selfCommandPtr);
            }
            else if(SYSTEM_CMD_SRC_NET_W5500 == taskSystemRecvCommandPtr->commandSrc)
            {
                //网口指令源
                SYSTEM_CMD_NET_W5500* netW5500CommandPtr =  (SYSTEM_CMD_NET_W5500*)(taskSystemRecvCommandPtr->systemCommandDataPtr);
                //对接收到的数据进行处理
                TaskServiceCommandProc_Debug_NetW5500(netW5500CommandPtr);
                //处置完成,执行后续操作
                UserMemFree(SRAM_CCM,netW5500CommandPtr);
            }
            else if(SYSTEM_CMD_SRC_CAN1 == taskSystemRecvCommandPtr->commandSrc)
            {
                //CAN1口指令源
                SYSTEM_CMD_CAN1* can1CommandPtr =  (SYSTEM_CMD_CAN1*)(taskSystemRecvCommandPtr->systemCommandDataPtr);
                //对接收到的数据进行处理
                TaskServiceCommandProc_Debug_Can1(can1CommandPtr);
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

//读取IO口状态
static void TaskServiceCommandProc_Debug_UartRS1_ReadInput(SYSTEM_CMD_UART_RS1* uartCommand);
//写入IO口状态
static void TaskServiceCommandProc_Debug_UartRS1_WriteOutput(SYSTEM_CMD_UART_RS1* uartCommand);
//电机复位
static void TaskServiceCommandProc_Debug_UartRS1_StepMotorReset(SYSTEM_CMD_UART_RS1* uartCommand);
//电机走位
static void TaskServiceCommandProc_Debug_UartRS1_StepMotorRunStep(SYSTEM_CMD_UART_RS1* uartCommand);
//读参数
static void TaskServiceCommandProc_Debug_UartRS1_ReadParam(SYSTEM_CMD_UART_RS1* uartCommand);
//写参数
static void TaskServiceCommandProc_Debug_UartRS1_WriteParam(SYSTEM_CMD_UART_RS1* uartCommand);

//串口指令处理
static void TaskServiceCommandProc_Debug_UartRS1(SYSTEM_CMD_UART_RS1* uartCommand)
{
    switch(uartCommand->funcCode)
    {
        case 0x04:
            TaskServiceCommandProc_Debug_UartRS1_ReadInput(uartCommand);
            break;
        case 0x55:
            TaskServiceCommandProc_Debug_UartRS1_WriteOutput(uartCommand);
            break;
        case 0x09:
            TaskServiceCommandProc_Debug_UartRS1_StepMotorReset(uartCommand);
            break;
        case 0x0A:
            TaskServiceCommandProc_Debug_UartRS1_StepMotorRunStep(uartCommand);
            break;
        case 0x03:
            TaskServiceCommandProc_Debug_UartRS1_ReadParam(uartCommand);
            break;
        case 0x06:
            TaskServiceCommandProc_Debug_UartRS1_WriteParam(uartCommand);
            break;
    }
}

//读取IO口状态
static void TaskServiceCommandProc_Debug_UartRS1_ReadInput(SYSTEM_CMD_UART_RS1* uartCommand)
{
    BitAction bitValue;
	LH_ERR errorCode;
    //读取输入端口讯息
    errorCode = SystemInputRead((SYSTEM_INPUT_INDEX)(uartCommand->regAddr),&bitValue);
    if(errorCode == LH_ERR_NONE)
    {
        //清空接收缓存
        UserMemSet(uartCommand->uartRecvCommandArray,0,12);
        //创建一帧数据包
        SystemUartRS_ModbusCreatePackage(uartCommand->uartRecvCommandArray,uartCommand->srcDeviceAddr,uartCommand->frameNo,0x01,
        (((uartCommand->regAddr)<<8)|(0X06)),(((uartCommand->regAddr)&0xff00))+bitValue);
        BoardRS_SendBuffer(ACTION_DEBUG_PORT,uartCommand->uartRecvCommandArray,12);
    }
    else
    {
        //读取失败
        return;
    }
}

//写入IO口状态
static void TaskServiceCommandProc_Debug_UartRS1_WriteOutput(SYSTEM_CMD_UART_RS1* uartCommand)
{
    BitAction bitValue;
	LH_ERR errorCode;
    if(uartCommand->regValue != 0)
    {
        bitValue = Bit_SET;
    }
    else
    {
        bitValue = Bit_RESET;
    }
    //写入IO口状态
    errorCode = SystemOutputWrite((SYSTEM_OUTPUT_INDEX)(uartCommand->regAddr),bitValue);
    if(errorCode == LH_ERR_NONE)
    {
        UserMemSet(uartCommand->uartRecvCommandArray,0,12);
        SystemUartRS_ModbusCreatePackage(uartCommand->uartRecvCommandArray,uartCommand->srcDeviceAddr,uartCommand->frameNo,0x01,
            0X0000,0X0000);
        BoardRS_SendBuffer(ACTION_DEBUG_PORT,uartCommand->uartRecvCommandArray,12);
    }
    else
    {
        //写入失败
        UserMemSet(uartCommand->uartRecvCommandArray,0,12);
        SystemUartRS_ModbusCreatePackage(uartCommand->uartRecvCommandArray,uartCommand->srcDeviceAddr,uartCommand->frameNo,0x01,
            0x0100,0X0000);
        BoardRS_SendBuffer(ACTION_DEBUG_PORT,uartCommand->uartRecvCommandArray,12);
    }
}

//电机复位
static void TaskServiceCommandProc_Debug_UartRS1_StepMotorReset(SYSTEM_CMD_UART_RS1* uartCommand)
{
    LH_ERR errorCode;
    //电机复位结构体
    SM_RESET_CMD resetCommand;
    //结构体初始化
    Can2SubSM_ResetCmdDataStructInit(&resetCommand);
    //指定电机复位
    uint8_t motorIndex = (uint8_t)(uartCommand->regAddr)&0x7f;
    //发送应答
    //创建一条新数据包
    SystemUartRS_ModbusCreatePackage(uartCommand->uartRecvCommandArray,uartCommand->srcDeviceAddr,uartCommand->frameNo,
                        uartCommand->funcCode,uartCommand->regAddr,uartCommand->regValue);
    //发送一个填充好的数据包
    BoardRS_SendBuffer(ACTION_DEBUG_PORT,uartCommand->uartRecvCommandArray,12);
    
    //调用电机复位指令
    resetCommand.stepMotorIndex = (CAN2_SUB_SM_INDEX)motorIndex;
    errorCode = Can2SubSM_ResetWhileReturn(&resetCommand);
    if(errorCode == LH_ERR_NONE)
    {
        SystemUartRS_ModbusCreatePackage(uartCommand->uartRecvCommandArray,uartCommand->srcDeviceAddr,uartCommand->frameNo,
                    0x01,0X0000,0x0000);
    }
    else
    {
        SystemUartRS_ModbusCreatePackage(uartCommand->uartRecvCommandArray,uartCommand->srcDeviceAddr,uartCommand->frameNo,
                    0x01,0X0300,0x0000);
    }
    BoardRS_SendBuffer(ACTION_DEBUG_PORT,uartCommand->uartRecvCommandArray,12);
}

//电机走位
static void TaskServiceCommandProc_Debug_UartRS1_StepMotorRunStep(SYSTEM_CMD_UART_RS1* uartCommand)
{
    LH_ERR errorCode;
    SM_RUN_STEP_CMD runStepCommand;
    //指定电机走位
    uint8_t dir = (uint8_t)(((uartCommand->regAddr)&0x0080)>>7);
    uint8_t motorIndex = (uint8_t)(uartCommand->regAddr)&0x7f;
    //走位结构体初始化
    Can2SubSM_RunStepsCmdDataStructInit(&runStepCommand);
    //步数计算
    if(dir > 0)
    {
        runStepCommand.steps = (int32_t)(uartCommand->regValue);
    }
    else
    {
        runStepCommand.steps = (0 - (int32_t)(uartCommand->regValue));
    }
    //电机序号
    runStepCommand.stepMotorIndex = (CAN2_SUB_SM_INDEX)(motorIndex);

    //发送应答,创建一条新数据包
    SystemUartRS_ModbusCreatePackage(uartCommand->uartRecvCommandArray,uartCommand->srcDeviceAddr,uartCommand->frameNo,
                        uartCommand->funcCode,uartCommand->regAddr,uartCommand->regValue);
    //发送一个填充好的数据包
    BoardRS_SendBuffer(ACTION_DEBUG_PORT,uartCommand->uartRecvCommandArray,12);
    
    //调用走位指令
    errorCode = Can2SubSM_RunSpecialStepsWhileReturn(&runStepCommand);

    //发送结果包
    if(errorCode == LH_ERR_NONE)
    {
        SystemUartRS_ModbusCreatePackage(uartCommand->uartRecvCommandArray,uartCommand->srcDeviceAddr,uartCommand->frameNo,
                    0x01,0X0000,0x0000);
    }
    else
    {
        SystemUartRS_ModbusCreatePackage(uartCommand->uartRecvCommandArray,uartCommand->srcDeviceAddr,uartCommand->frameNo,
                    0x01,0X0300,0x0000);
    }
    BoardRS_SendBuffer(ACTION_DEBUG_PORT,uartCommand->uartRecvCommandArray,12);
}

//读参数
static void TaskServiceCommandProc_Debug_UartRS1_ReadParam(SYSTEM_CMD_UART_RS1* uartCommand)
{
    //读参数
    //参数主序号
    uint8_t mainParamIndex = (uint8_t)((uartCommand->regAddr)>>8);
    //参数次序号
    uint8_t subParamIndex = (uint8_t)(uartCommand->regAddr);
    //参数辅助计算
    int32_t paramValueUtil = 0;
    int16_t paramResultConvertUtil = 0;
    //最终参数
    uint16_t paramResult;

    //读取参数把并转换出结果
    SystemReadActionParamWithIndex(mainParamIndex,subParamIndex,&paramValueUtil);
    paramResultConvertUtil = (int16_t)paramValueUtil;
    paramResult = *((uint16_t*)(&paramResultConvertUtil));

    //先返回ACK
    SystemUartRS_ModbusCreatePackage(uartCommand->uartRecvCommandArray,uartCommand->srcDeviceAddr,uartCommand->frameNo,uartCommand->funcCode,
                            uartCommand->regAddr,paramResult);
    //发送一个填充好的数据包
    BoardRS_SendBuffer(ACTION_DEBUG_PORT,uartCommand->uartRecvCommandArray,12);

    //将数据返回
    SystemUartRS_ModbusCreatePackage(uartCommand->uartRecvCommandArray,uartCommand->srcDeviceAddr,uartCommand->frameNo,0x01,
                            (((uartCommand->regAddr)<<8)|0x05),paramResult);
    //发送一个填充好的数据包
    BoardRS_SendBuffer(ACTION_DEBUG_PORT,uartCommand->uartRecvCommandArray,12);
    
    //将结果返回
    SystemUartRS_ModbusCreatePackage(uartCommand->uartRecvCommandArray,uartCommand->srcDeviceAddr,uartCommand->frameNo,0x01,
                            0x0000,0x0000);
    //发送一个填充好的数据包
    BoardRS_SendBuffer(ACTION_DEBUG_PORT,uartCommand->uartRecvCommandArray,12);
}

//写参数
static void TaskServiceCommandProc_Debug_UartRS1_WriteParam(SYSTEM_CMD_UART_RS1* uartCommand)
{
    //写参数
    //参数的序号和次一级序号
    uint8_t mainParamIndex = (uint8_t)((uartCommand->regAddr)>>8);
    uint8_t subParamIndex = (uint8_t)(uartCommand->regAddr);
    //参数的值
    int32_t paramValue = (int32_t)(*((int16_t*)(&(uartCommand->regValue))));

    //写入参数
    SystemWriteActionParamWithIndex(mainParamIndex,subParamIndex,paramValue);

    //发送ACK
    SystemUartRS_ModbusCreatePackage(uartCommand->uartRecvCommandArray,uartCommand->srcDeviceAddr,uartCommand->frameNo,uartCommand->funcCode,
                            uartCommand->regAddr,0x0000);
    //发送一个填充好的数据包
    BoardRS_SendBuffer(ACTION_DEBUG_PORT,uartCommand->uartRecvCommandArray,12);

    //将结果返回
    SystemUartRS_ModbusCreatePackage(uartCommand->uartRecvCommandArray,uartCommand->srcDeviceAddr,uartCommand->frameNo,0x01,
                            0x0000,0x0000);
    //发送一个填充好的数据包
    BoardRS_SendBuffer(ACTION_DEBUG_PORT,uartCommand->uartRecvCommandArray,12);
}



/************************************************************************************/

/*****************************************************CAN指令处理*****************************************************/
//写参数
static void TaskServiceCommandProc_Debug_Can1_WriteParam(SYSTEM_CMD_CAN1* canCommand);
//读参数
static void TaskServiceCommandProc_Debug_Can1_ReadParam(SYSTEM_CMD_CAN1* canCommand);
//步进电机运行指定步数
static void TaskServiceCommandProc_Debug_Can1_StepMotorRunStep(SYSTEM_CMD_CAN1* canCommand);
//步进电机复位
static void TaskServiceCommandProc_Debug_Can1_StepMotorReset(SYSTEM_CMD_CAN1* canCommand);
//写端口
static void TaskServiceCommandProc_Debug_Can1_WriteOutput(SYSTEM_CMD_CAN1* canCommand);
//读端口
static void TaskServiceCommandProc_Debug_Can1_ReadInput(SYSTEM_CMD_CAN1* canCommand);

static void TaskServiceCommandProc_Debug_Can1(SYSTEM_CMD_CAN1* canCommand)
{
    switch(canCommand->command)
    {
        case CAN1_COMMAND_IO_READ:
            TaskServiceCommandProc_Debug_Can1_ReadInput(canCommand);
            break;
        case CAN1_COMMAND_IO_WRITE:
            TaskServiceCommandProc_Debug_Can1_WriteOutput(canCommand);
            break;
        case CAN1_COMMAND_SM_RESET:
            TaskServiceCommandProc_Debug_Can1_StepMotorReset(canCommand);
            break;
        case CAN1_COMMAND_SM_RUN:
            TaskServiceCommandProc_Debug_Can1_StepMotorRunStep(canCommand);
            break;
        case CAN1_COMMAND_PARAM_READ:
            TaskServiceCommandProc_Debug_Can1_ReadParam(canCommand);
            break;
        case CAN1_COMMAND_PARAM_WRITE:
            TaskServiceCommandProc_Debug_Can1_WriteParam(canCommand);
            break;
    }
}

//读取IO口状态
static void TaskServiceCommandProc_Debug_Can1_ReadInput(SYSTEM_CMD_CAN1* canCommand)
{
    BitAction bitValue;
	LH_ERR errorCode;
    uint8_t cellnum;
    SYSTEM_INPUT_INDEX PortID = (SYSTEM_INPUT_INDEX)(canCommand->can1RecvDataBuffer[0]|(canCommand->can1RecvDataBuffer[1]<<8));

    //读取输入端口讯息
    errorCode = SystemInputRead(PortID,&bitValue);
    cellnum = SystemConfig_GetCellNumber();
	canCommand->can1RecvDataBuffer[0] = cellnum+1;//canCommand->targetModuleNo;
    canCommand->can1RecvDataBuffer[1] = 0x01;//canCommand->targetBoardAddr;
    canCommand->can1RecvDataBuffer[2] = canCommand->command&0xff;
    canCommand->can1RecvDataBuffer[3] = (canCommand->command>>8)&0xff;
    canCommand->can1RecvDataBuffer[4] = (PortID)&0xff;
    canCommand->can1RecvDataBuffer[5] = (PortID>>8)&0xff;
    canCommand->can1RecvDataBuffer[6] = bitValue;
    canCommand->can1RecvDataBuffer[7] = (errorCode)&0xff;
    canCommand->can1RecvDataBuffer[8] = (errorCode>>8)&0xff;
    canCommand->can1RecvDataBuffer[9] = (errorCode>>16)&0xff;
    canCommand->can1RecvDataBuffer[10] = (errorCode>>24)&0xff;
	CanSendDataToCell(TESTPROC_CELL_PLATFORM_NMBR,BOARD_ID_TRACK_COMM,0x0018,canCommand->can1RecvDataBuffer,11);
}
//写入IO口状态
static void TaskServiceCommandProc_Debug_Can1_WriteOutput(SYSTEM_CMD_CAN1* canCommand)
{
    BitAction bitValue;
	LH_ERR errorCode;
 //   uint8_t cmd = canCommand->command;
    uint16_t regaddr= canCommand->can1RecvDataBuffer[0]|(canCommand->can1RecvDataBuffer[1]<<8);
    char regvalue = canCommand->can1RecvDataBuffer[2];
    if(regvalue != 0)
    {
        bitValue = Bit_SET;
    }
    else
    {
        bitValue = Bit_RESET;
    }
    uint8_t cellnum = SystemConfig_GetCellNumber();
    //写入IO口状态
    errorCode = SystemOutputWrite((SYSTEM_OUTPUT_INDEX)regaddr,bitValue);
    canCommand->can1RecvDataBuffer[0] = cellnum+1;//canCommand->targetModuleNo;
    canCommand->can1RecvDataBuffer[1] = 0x01;//canCommand->targetBoardAddr;
    canCommand->can1RecvDataBuffer[2] = canCommand->command&0xff;
    canCommand->can1RecvDataBuffer[3] = (canCommand->command>>8)&0xff;
    canCommand->can1RecvDataBuffer[4] = (regaddr)&0xff;
    canCommand->can1RecvDataBuffer[5] = (regaddr>>8)&0xff;
    canCommand->can1RecvDataBuffer[6] = bitValue;
    canCommand->can1RecvDataBuffer[7] = (errorCode)&0xff;
    canCommand->can1RecvDataBuffer[8] = (errorCode>>8)&0xff;
    canCommand->can1RecvDataBuffer[9] = (errorCode>>16)&0xff;
    canCommand->can1RecvDataBuffer[10] = (errorCode>>24)&0xff;
	CanSendDataToCell(TESTPROC_CELL_PLATFORM_NMBR,BOARD_ID_TRACK_COMM,0x0018,canCommand->can1RecvDataBuffer,11);
}

//电机复位
static void TaskServiceCommandProc_Debug_Can1_StepMotorReset(SYSTEM_CMD_CAN1* canCommand)
{
    LH_ERR errorCode;
    //电机复位结构体
    SM_RESET_CMD resetCommand;
 //   uint8_t cmd = canCommand->command;
    //结构体初始化
    Can2SubSM_ResetCmdDataStructInit(&resetCommand);
    //指定电机复位
    uint8_t motorIndex = canCommand->can1RecvDataBuffer[0];
    
    //调用电机复位指令
    resetCommand.stepMotorIndex = (CAN2_SUB_SM_INDEX)motorIndex;
    errorCode = Can2SubSM_ResetWhileReturn(&resetCommand);
    uint8_t cellnum = SystemConfig_GetCellNumber();
    canCommand->can1RecvDataBuffer[0] = cellnum+1;//canCommand->targetModuleNo;
    canCommand->can1RecvDataBuffer[1] = 0x01;//canCommand->targetBoardAddr;
    canCommand->can1RecvDataBuffer[2] = canCommand->command&0xff;
    canCommand->can1RecvDataBuffer[3] = (canCommand->command>>8)&0xff;
    canCommand->can1RecvDataBuffer[4] = resetCommand.stepMotorIndex;
    canCommand->can1RecvDataBuffer[5] = (errorCode)&0xff;
    canCommand->can1RecvDataBuffer[6] = (errorCode>>8)&0xff;
    canCommand->can1RecvDataBuffer[7] = (errorCode>>16)&0xff;
    canCommand->can1RecvDataBuffer[8] = (errorCode>>24)&0xff;
	CanSendDataToCell(TESTPROC_CELL_PLATFORM_NMBR,BOARD_ID_TRACK_COMM,0x0018,canCommand->can1RecvDataBuffer,9);
}
//电机走位
static void TaskServiceCommandProc_Debug_Can1_StepMotorRunStep(SYSTEM_CMD_CAN1* canCommand)
{
    LH_ERR errorCode;
    SM_RUN_STEP_CMD runStepCommand;
    //uint8_t cmd = canCommand->command;
    //指定电机走位
    uint8_t motorIndex = canCommand->can1RecvDataBuffer[0];
    uint8_t dir = canCommand->can1RecvDataBuffer[1];
    int32_t movesteps =  (canCommand->can1RecvDataBuffer[2])| 
                         (canCommand->can1RecvDataBuffer[3]<<8)| 
                         (canCommand->can1RecvDataBuffer[4]<<16)| 
                         (canCommand->can1RecvDataBuffer[5]<<24);
    //走位结构体初始化
    Can2SubSM_RunStepsCmdDataStructInit(&runStepCommand);
    //步数计算
    if(dir > 0)
    {
        runStepCommand.steps = (int32_t)movesteps;
    }
    else
    {
        runStepCommand.steps = 0 - (int32_t)movesteps;
    }
    //电机序号
    runStepCommand.stepMotorIndex = (CAN2_SUB_SM_INDEX)(motorIndex);
 
    //调用走位指令
    errorCode = Can2SubSM_RunSpecialStepsWhileReturn(&runStepCommand);
    uint8_t cellnum = SystemConfig_GetCellNumber();
    //发送结果包
    canCommand->can1RecvDataBuffer[0] = cellnum+1;//canCommand->targetModuleNo;
    canCommand->can1RecvDataBuffer[1] = 0x01;//canCommand->targetBoardAddr;
    canCommand->can1RecvDataBuffer[2] = canCommand->command&0xff;
    canCommand->can1RecvDataBuffer[3] = (canCommand->command>>8)&0xff;
    canCommand->can1RecvDataBuffer[4] = runStepCommand.stepMotorIndex;
    canCommand->can1RecvDataBuffer[5] = dir;
    canCommand->can1RecvDataBuffer[6] = (movesteps)&0xff;
    canCommand->can1RecvDataBuffer[7] = (movesteps>>8)&0xff;
    canCommand->can1RecvDataBuffer[8] = (movesteps>>16)&0xff;
    canCommand->can1RecvDataBuffer[9] = (movesteps>>24)&0xff;
    canCommand->can1RecvDataBuffer[10] = (errorCode)&0xff;
    canCommand->can1RecvDataBuffer[11] = (errorCode>>8)&0xff;
    canCommand->can1RecvDataBuffer[12] = (errorCode>>16)&0xff;
    canCommand->can1RecvDataBuffer[13] = (errorCode>>24)&0xff;
	CanSendDataToCell(TESTPROC_CELL_PLATFORM_NMBR,BOARD_ID_TRACK_COMM,0x0018,canCommand->can1RecvDataBuffer,14);
}

//读参数
static void TaskServiceCommandProc_Debug_Can1_ReadParam(SYSTEM_CMD_CAN1* canCommand)
{
    //读参数
//    uint8_t cmd = canCommand->command;
    //参数主序号
    uint8_t mainParamIndex = canCommand->can1RecvDataBuffer[0];
    //参数次序号
    uint8_t subParamIndex = canCommand->can1RecvDataBuffer[1];
    //参数辅助计算
    int32_t paramValueUtil = 0;
 //   int16_t paramResultConvertUtil = 0;
    //最终参数
    int32_t paramResult;

    //读取参数把并转换出结果
    SystemReadActionParamWithIndex(mainParamIndex,subParamIndex,&paramValueUtil);
    uint8_t cellnum = SystemConfig_GetCellNumber();
    paramResult = paramValueUtil;
    canCommand->can1RecvDataBuffer[0] = cellnum+1;//canCommand->targetModuleNo;
    canCommand->can1RecvDataBuffer[1] = 0x01;//canCommand->targetBoardAddr;
    canCommand->can1RecvDataBuffer[2] = canCommand->command&0xff;
    canCommand->can1RecvDataBuffer[3] = (canCommand->command>>8)&0xff;
    canCommand->can1RecvDataBuffer[4] = subParamIndex;
    canCommand->can1RecvDataBuffer[5] = mainParamIndex;
    canCommand->can1RecvDataBuffer[6] = (paramResult)&0xff;
    canCommand->can1RecvDataBuffer[7] = (paramResult>>8)&0xff;
    canCommand->can1RecvDataBuffer[8] = (paramResult>>16)&0xff;
    canCommand->can1RecvDataBuffer[9] = (paramResult>>24)&0xff;
    canCommand->can1RecvDataBuffer[10] = 0;
    canCommand->can1RecvDataBuffer[11] = 0;
    canCommand->can1RecvDataBuffer[12] = 0;
    canCommand->can1RecvDataBuffer[13] = 0;
    CanSendDataToCell(TESTPROC_CELL_PLATFORM_NMBR,BOARD_ID_TRACK_COMM,0x0018,canCommand->can1RecvDataBuffer,14);
}
//写参数
static void TaskServiceCommandProc_Debug_Can1_WriteParam(SYSTEM_CMD_CAN1* canCommand)
{
    //写参数
//    uint8_t cmd = canCommand->command;
    //参数的序号和次一级序号
    uint8_t mainParamIndex = canCommand->can1RecvDataBuffer[0];
    uint8_t subParamIndex = canCommand->can1RecvDataBuffer[1];
    //参数的值
    int32_t paramValue = canCommand->can1RecvDataBuffer[2] |
                         canCommand->can1RecvDataBuffer[3]<<8 |
                         canCommand->can1RecvDataBuffer[4]<<16  |
                         canCommand->can1RecvDataBuffer[5]<<24;
    
    //写入参数
    SystemWriteActionParamWithIndex(mainParamIndex,subParamIndex,paramValue);
    uint8_t cellnum = SystemConfig_GetCellNumber();
    canCommand->can1RecvDataBuffer[0] = cellnum+1;//canCommand->targetModuleNo;
    canCommand->can1RecvDataBuffer[1] = 0x01;//canCommand->targetBoardAddr;
    canCommand->can1RecvDataBuffer[2] = canCommand->command&0xff;
    canCommand->can1RecvDataBuffer[3] = (canCommand->command>>8)&0xff;
    canCommand->can1RecvDataBuffer[4] = subParamIndex;
    canCommand->can1RecvDataBuffer[5] = mainParamIndex;
    canCommand->can1RecvDataBuffer[6] = (paramValue)&0xff;;
    canCommand->can1RecvDataBuffer[7] = (paramValue>>8)&0xff;;
    canCommand->can1RecvDataBuffer[8] = (paramValue>>16)&0xff;
    canCommand->can1RecvDataBuffer[9] = (paramValue>>24)&0xff;
    canCommand->can1RecvDataBuffer[10] = 0;
    canCommand->can1RecvDataBuffer[11] = 0;
    canCommand->can1RecvDataBuffer[12] = 0;
    canCommand->can1RecvDataBuffer[13] = 0;
    CanSendDataToCell(TESTPROC_CELL_PLATFORM_NMBR,BOARD_ID_TRACK_COMM,0x0018,canCommand->can1RecvDataBuffer,14);
}
/************************************************************************************/




/*******************************************************网口指令处理**************************************************/

static void TaskServiceCommandProc_Debug_NetW5500(SYSTEM_CMD_NET_W5500* netCommand)
{

}

/************************************************************************************/

/*******************************************************自身指令处理**************************************************/

static void TaskServiceCommandProc_Debug_Self(SYSTEM_CMD_SELF* selfCommand)
{

}

/************************************************************************************/





