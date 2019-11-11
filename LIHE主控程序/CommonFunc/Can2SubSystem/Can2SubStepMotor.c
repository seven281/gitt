#include "Can2SubStepMotor.h"

//CAN2模块计算指定电机的板卡号和通道号
static LH_ERR Can2SubSM_CalcBoardChannel(CAN2_SUB_SM_INDEX stepMotorIndex,CAN2_SUB_BOARD* boardIndex,CAN2_CHANNEL* channelIndex)
{
    //超过最大值
    if(stepMotorIndex > CAN2_SUB_SM_BOARD10_SM12)
    {
        return LH_ERR_SM_INDEX;
    }
    if(stepMotorIndex <= CAN2_SUB_SM_BOARD1_SM12)
    {
        *boardIndex = CAN2_SUB_BOARD1;
        *channelIndex = (CAN2_CHANNEL)(CAN2_CHANNEL_SM1 + (stepMotorIndex - CAN2_SUB_SM_BOARD1_SM1));
    }
    else if(stepMotorIndex <= CAN2_SUB_SM_BOARD2_SM12)
    {
        *boardIndex = CAN2_SUB_BOARD2;
        *channelIndex = (CAN2_CHANNEL)(CAN2_CHANNEL_SM1 + (stepMotorIndex - CAN2_SUB_SM_BOARD2_SM1));
    }
    else if(stepMotorIndex <= CAN2_SUB_SM_BOARD3_SM12)
    {
        *boardIndex = CAN2_SUB_BOARD3;
        *channelIndex = (CAN2_CHANNEL)(CAN2_CHANNEL_SM1 + (stepMotorIndex - CAN2_SUB_SM_BOARD3_SM1));
    }
    else if(stepMotorIndex <= CAN2_SUB_SM_BOARD4_SM12)
    {
        *boardIndex = CAN2_SUB_BOARD4;
        *channelIndex = (CAN2_CHANNEL)(CAN2_CHANNEL_SM1 + (stepMotorIndex - CAN2_SUB_SM_BOARD4_SM1));
    }
    else if(stepMotorIndex <= CAN2_SUB_SM_BOARD5_SM12)
    {
        *boardIndex = CAN2_SUB_BOARD5;
        *channelIndex = (CAN2_CHANNEL)(CAN2_CHANNEL_SM1 + (stepMotorIndex - CAN2_SUB_SM_BOARD5_SM1));
    }
    else if(stepMotorIndex <= CAN2_SUB_SM_BOARD6_SM12)
    {
        *boardIndex = CAN2_SUB_BOARD6;
        *channelIndex = (CAN2_CHANNEL)(CAN2_CHANNEL_SM1 + (stepMotorIndex - CAN2_SUB_SM_BOARD6_SM1));
    }
    else if(stepMotorIndex <= CAN2_SUB_SM_BOARD7_SM12)
    {
        *boardIndex = CAN2_SUB_BOARD7;
        *channelIndex = (CAN2_CHANNEL)(CAN2_CHANNEL_SM1 + (stepMotorIndex - CAN2_SUB_SM_BOARD7_SM1));
    }
    else if(stepMotorIndex <= CAN2_SUB_SM_BOARD8_SM12)
    {
        *boardIndex = CAN2_SUB_BOARD8;
        *channelIndex = (CAN2_CHANNEL)(CAN2_CHANNEL_SM1 + (stepMotorIndex - CAN2_SUB_SM_BOARD8_SM1));
    }
    else if(stepMotorIndex <= CAN2_SUB_SM_BOARD9_SM12)
    {
        *boardIndex = CAN2_SUB_BOARD9;
        *channelIndex = (CAN2_CHANNEL)(CAN2_CHANNEL_SM1 + (stepMotorIndex - CAN2_SUB_SM_BOARD9_SM1));
    }
    else if(stepMotorIndex <= CAN2_SUB_SM_BOARD10_SM12)
    {
        *boardIndex = CAN2_SUB_BOARD10;
        *channelIndex = (CAN2_CHANNEL)(CAN2_CHANNEL_SM1 + (stepMotorIndex - CAN2_SUB_SM_BOARD10_SM1));
    }
    return LH_ERR_NONE;
}

//系统内部电机复位
//修正步数 超时时间
LH_ERR Can2SubSM_ResetWhileAck(SM_RESET_CMD* resetCmdPtr)
{
    CAN2_SEND_CMD can2SendCmd;
    LH_ERR errCode = LH_ERR_NONE;
    //计算板卡号码和单元号码
    errCode = Can2SubSM_CalcBoardChannel(resetCmdPtr->stepMotorIndex,&(can2SendCmd.targetBoardID),
                                &(can2SendCmd.channelNo));
    if(errCode != LH_ERR_NONE)
    {
        return errCode;
    }
    //申请内存
    can2SendCmd.paramBuffer = NULL;
    can2SendCmd.paramLength = 8;
    do
    {
        can2SendCmd.paramBuffer = UserMemMalloc(SRAM_IN,sizeof(can2SendCmd.paramLength));
        if(can2SendCmd.paramBuffer == NULL)
        {
            CoreDelayMinTick();
        }
    }while(can2SendCmd.paramBuffer == NULL);
    //内存数据拷贝
    Can2ProcUtilConvertInt32ToArray(can2SendCmd.paramBuffer,0,resetCmdPtr->correctionPosition);
    Can2ProcUtilConvertUint32ToArray(can2SendCmd.paramBuffer,4,resetCmdPtr->timeOutMs);
    //指令码设定
    can2SendCmd.commandCode = SM_CMD_RESET;
    can2SendCmd.timeoutMs = resetCmdPtr->timeOutMs;
    //数据发送
    errCode = Can2SendCommandWhileAck(&can2SendCmd);
    //释放内存
    UserMemFree(SRAM_IN,can2SendCmd.paramBuffer);
    return errCode;
}

//系统内部电机执行指定步数
LH_ERR Can2SubSM_RunSpecialStepsWhileAck(SM_RUN_STEP_CMD* runStepsCmdPtr)
{
    CAN2_SEND_CMD can2SendCmd;
    LH_ERR errCode = LH_ERR_NONE;
    //计算板卡号码和单元号码
    errCode = Can2SubSM_CalcBoardChannel(runStepsCmdPtr->stepMotorIndex,&(can2SendCmd.targetBoardID),
                                &(can2SendCmd.channelNo));
    if(errCode != LH_ERR_NONE)
    {
        return errCode;
    }
    //申请内存
    can2SendCmd.paramBuffer = NULL;
    can2SendCmd.paramLength = 13;
    do
    {
        can2SendCmd.paramBuffer = UserMemMalloc(SRAM_IN,sizeof(can2SendCmd.paramLength));
        if(can2SendCmd.paramBuffer == NULL)
        {
            CoreDelayMinTick();
        }
    }while(can2SendCmd.paramBuffer == NULL);
    //内存数据拷贝
    Can2ProcUtilConvertInt32ToArray(can2SendCmd.paramBuffer,0,runStepsCmdPtr->steps);
    can2SendCmd.paramBuffer[4] = runStepsCmdPtr->curveSelect;
    can2SendCmd.paramBuffer[5] = runStepsCmdPtr->speedRatio;
    can2SendCmd.paramBuffer[6] = runStepsCmdPtr->speedMode;
    can2SendCmd.paramBuffer[7] = runStepsCmdPtr->utilStop1Enable;
    can2SendCmd.paramBuffer[8] = runStepsCmdPtr->utilStop2Enable;
    Can2ProcUtilConvertUint32ToArray(can2SendCmd.paramBuffer,9,runStepsCmdPtr->timeOutMs);
    //指令码设定
    can2SendCmd.commandCode = SM_CMD_RUN_STEPS;
    can2SendCmd.timeoutMs = runStepsCmdPtr->timeOutMs;
    //数据发送
    errCode = Can2SendCommandWhileAck(&can2SendCmd);
    //释放内存
    UserMemFree(SRAM_IN,can2SendCmd.paramBuffer);
    return errCode;
}

//系统内部电机运行到指定坐标
LH_ERR Can2SubSM_RunToCoordinateWhileAck(SM_RUN_COORDINATE_CMD* runCoordinateCmdPtr)
{
    CAN2_SEND_CMD can2SendCmd;
    LH_ERR errCode = LH_ERR_NONE;
    //计算板卡号码和单元号码
    errCode = Can2SubSM_CalcBoardChannel(runCoordinateCmdPtr->stepMotorIndex,&(can2SendCmd.targetBoardID),
                                &(can2SendCmd.channelNo));
    if(errCode != LH_ERR_NONE)
    {
        return errCode;
    }
    //申请内存
    can2SendCmd.paramBuffer = NULL;
    can2SendCmd.paramLength = 13;
    do
    {
        can2SendCmd.paramBuffer = UserMemMalloc(SRAM_IN,sizeof(can2SendCmd.paramLength));
        if(can2SendCmd.paramBuffer == NULL)
        {
            CoreDelayMinTick();
        }
    }while(can2SendCmd.paramBuffer == NULL);
    //内存数据拷贝
    Can2ProcUtilConvertInt32ToArray(can2SendCmd.paramBuffer,0,runCoordinateCmdPtr->targetCoordinate);
    can2SendCmd.paramBuffer[4] = runCoordinateCmdPtr->curveSelect;
    can2SendCmd.paramBuffer[5] = runCoordinateCmdPtr->speedRatio;
    can2SendCmd.paramBuffer[6] = runCoordinateCmdPtr->speedMode;
    can2SendCmd.paramBuffer[7] = runCoordinateCmdPtr->utilStop1Enable;
    can2SendCmd.paramBuffer[8] = runCoordinateCmdPtr->utilStop2Enable;
    Can2ProcUtilConvertUint32ToArray(can2SendCmd.paramBuffer,9,runCoordinateCmdPtr->timeOutMs);
    //指令码设定
    can2SendCmd.commandCode = SM_CMD_RUN_CORDINATE;
    can2SendCmd.timeoutMs = runCoordinateCmdPtr->timeOutMs;
    //数据发送
    errCode = Can2SendCommandWhileAck(&can2SendCmd);
    //释放内存
    UserMemFree(SRAM_IN,can2SendCmd.paramBuffer);
    return errCode;
}

//系统内部电机回零
LH_ERR Can2SubSM_ReturnZeroWhileAck(SM_RETURN_ZERO_CMD* returnZeroCmdPtr)
{
    CAN2_SEND_CMD can2SendCmd;
    LH_ERR errCode = LH_ERR_NONE;
    //计算板卡号码和单元号码
    errCode = Can2SubSM_CalcBoardChannel(returnZeroCmdPtr->stepMotorIndex,&(can2SendCmd.targetBoardID),
                                &(can2SendCmd.channelNo));
    if(errCode != LH_ERR_NONE)
    {
        return errCode;
    }
    //申请内存
    can2SendCmd.paramBuffer = NULL;
    can2SendCmd.paramLength = 13;
    do
    {
        can2SendCmd.paramBuffer = UserMemMalloc(SRAM_IN,sizeof(can2SendCmd.paramLength));
        if(can2SendCmd.paramBuffer == NULL)
        {
            CoreDelayMinTick();
        }
    }while(can2SendCmd.paramBuffer == NULL);
    //内存数据拷贝
    Can2ProcUtilConvertInt32ToArray(can2SendCmd.paramBuffer,0,returnZeroCmdPtr->correctionPosition);
    can2SendCmd.paramBuffer[4] = returnZeroCmdPtr->curveSelect;
    can2SendCmd.paramBuffer[5] = returnZeroCmdPtr->speedRatio;
    can2SendCmd.paramBuffer[6] = returnZeroCmdPtr->speedMode;
    can2SendCmd.paramBuffer[7] = returnZeroCmdPtr->utilStop1Enable;
    can2SendCmd.paramBuffer[8] = returnZeroCmdPtr->utilStop2Enable;
    Can2ProcUtilConvertUint32ToArray(can2SendCmd.paramBuffer,9,returnZeroCmdPtr->timeOutMs);
    //指令码设定
    can2SendCmd.commandCode = SM_CMD_RETURN_ZERO;
    can2SendCmd.timeoutMs = returnZeroCmdPtr->timeOutMs;
    //数据发送
    errCode = Can2SendCommandWhileAck(&can2SendCmd);
    //释放内存
    UserMemFree(SRAM_IN,can2SendCmd.paramBuffer);
    return errCode;
}

//系统内部电机电机持续运转
LH_ERR Can2SubSM_RunAlwaysWhileAck(SM_RUN_ALWAYS_CMD* runAlwaysCmdPtr)
{
    CAN2_SEND_CMD can2SendCmd;
    LH_ERR errCode = LH_ERR_NONE;
    //计算板卡号码和单元号码
    errCode = Can2SubSM_CalcBoardChannel(runAlwaysCmdPtr->stepMotorIndex,&(can2SendCmd.targetBoardID),
                                &(can2SendCmd.channelNo));
    if(errCode != LH_ERR_NONE)
    {
        return errCode;
    }
    //申请内存
    can2SendCmd.paramBuffer = NULL;
    can2SendCmd.paramLength = 3;
    do
    {
        can2SendCmd.paramBuffer = UserMemMalloc(SRAM_IN,sizeof(can2SendCmd.paramLength));
        if(can2SendCmd.paramBuffer == NULL)
        {
            CoreDelayMinTick();
        }
    }while(can2SendCmd.paramBuffer == NULL);
    //内存数据拷贝
    can2SendCmd.paramBuffer[0] = runAlwaysCmdPtr->curveSelect;
    can2SendCmd.paramBuffer[1] = runAlwaysCmdPtr->speedRatio;
    can2SendCmd.paramBuffer[2] = runAlwaysCmdPtr->speedMode;
    //指令码设定
    can2SendCmd.commandCode = SM_CMD_RUN_ALWAYS;
    can2SendCmd.timeoutMs = 5000;
    //数据发送
    errCode = Can2SendCommandWhileAck(&can2SendCmd);
    //释放内存
    UserMemFree(SRAM_IN,can2SendCmd.paramBuffer);
    return errCode;
}

//系统内部电机急停
LH_ERR Can2SubSM_StopImmediatelyWhileAck(CAN2_SUB_SM_INDEX motorIndex)
{
    CAN2_SEND_CMD can2SendCmd;
    LH_ERR errCode = LH_ERR_NONE;
    //计算板卡号码和单元号码
    errCode = Can2SubSM_CalcBoardChannel(motorIndex,&(can2SendCmd.targetBoardID),
                                &(can2SendCmd.channelNo));
    if(errCode != LH_ERR_NONE)
    {
        return errCode;
    }
    //申请内存
    can2SendCmd.paramBuffer = NULL;
    can2SendCmd.paramLength = 0;
    //指令码设定
    can2SendCmd.commandCode = SM_CMD_STOP_IMME;
    can2SendCmd.timeoutMs = 1000;
    //数据发送
    errCode = Can2SendCommandWhileAck(&can2SendCmd);
    return errCode;
}

//系统内部电机减速停止
LH_ERR Can2SubSM_StopGraduallyWhileAck(CAN2_SUB_SM_INDEX motorIndex)
{
    CAN2_SEND_CMD can2SendCmd;
    LH_ERR errCode = LH_ERR_NONE;
    //计算板卡号码和单元号码
    errCode = Can2SubSM_CalcBoardChannel(motorIndex,&(can2SendCmd.targetBoardID),
                                &(can2SendCmd.channelNo));
    if(errCode != LH_ERR_NONE)
    {
        return errCode;
    }
    //不需要申请内存
    can2SendCmd.paramBuffer = NULL;
    can2SendCmd.paramLength = 0;
    //指令码设定
    can2SendCmd.commandCode = SM_CMD_STOP_DACC;
    can2SendCmd.timeoutMs = 5000;
    //数据发送
    errCode = Can2SendCommandWhileAck(&can2SendCmd);
    return errCode;
}


/**********************************************************发送电机指令并等待电机执行完成***************************************************/
//系统内部电机复位
//修正步数 超时时间
LH_ERR Can2SubSM_ResetWhileReturn(SM_RESET_CMD* resetCmdPtr)
{
    CAN2_SEND_CMD can2SendCmd;
    CAN2_READ_DATA can2ReadData;
    LH_ERR errCode = LH_ERR_NONE;
    //初始化接收器
    can2ReadData.recvDataPtr = NULL;
    can2ReadData.recvDatLength = 0;
    //计算板卡号码和单元号码
    errCode = Can2SubSM_CalcBoardChannel(resetCmdPtr->stepMotorIndex,&(can2SendCmd.targetBoardID),
                                &(can2SendCmd.channelNo));
    if(errCode != LH_ERR_NONE)
    {
        return errCode;
    }
    //申请内存
    can2SendCmd.paramBuffer = NULL;
    can2SendCmd.paramLength = 8;
    do
    {
        can2SendCmd.paramBuffer = UserMemMalloc(SRAM_IN,sizeof(can2SendCmd.paramLength));
        if(can2SendCmd.paramBuffer == NULL)
        {
            CoreDelayMinTick();
        }
    }while(can2SendCmd.paramBuffer == NULL);
    //内存数据拷贝
    Can2ProcUtilConvertInt32ToArray(can2SendCmd.paramBuffer,0,resetCmdPtr->correctionPosition);
    Can2ProcUtilConvertUint32ToArray(can2SendCmd.paramBuffer,4,resetCmdPtr->timeOutMs);
    //指令码设定
    can2SendCmd.commandCode = SM_CMD_RESET;
    can2SendCmd.timeoutMs = resetCmdPtr->timeOutMs;
    //数据发送
    errCode = Can2SendCommandWhileReturn(&can2SendCmd,&can2ReadData);
    //释放内存
    UserMemFree(SRAM_IN,can2SendCmd.paramBuffer);
    //接收到的数据不需要
    if(can2ReadData.recvDataPtr != NULL)
    {
        //释放内存
        UserMemFree(SRAM_IN,can2ReadData.recvDataPtr);
    }
    return errCode;
}

//系统内部电机执行执行步数
LH_ERR Can2SubSM_RunSpecialStepsWhileReturn(SM_RUN_STEP_CMD* runStepsCmdPtr)
{
    CAN2_SEND_CMD can2SendCmd;
    CAN2_READ_DATA can2ReadData;
    LH_ERR errCode = LH_ERR_NONE;
    //初始化接收器
    can2ReadData.recvDataPtr = NULL;
    can2ReadData.recvDatLength = 0;
    //计算板卡号码和单元号码
    errCode = Can2SubSM_CalcBoardChannel(runStepsCmdPtr->stepMotorIndex,&(can2SendCmd.targetBoardID),
                                &(can2SendCmd.channelNo));
    if(errCode != LH_ERR_NONE)
    {
        return errCode;
    }
    //申请内存
    can2SendCmd.paramBuffer = NULL;
    can2SendCmd.paramLength = 13;
    do
    {
        can2SendCmd.paramBuffer = UserMemMalloc(SRAM_IN,sizeof(can2SendCmd.paramLength));
        if(can2SendCmd.paramBuffer == NULL)
        {
            CoreDelayMinTick();
        }
    }while(can2SendCmd.paramBuffer == NULL);
    //内存数据拷贝
    Can2ProcUtilConvertInt32ToArray(can2SendCmd.paramBuffer,0,runStepsCmdPtr->steps);
    can2SendCmd.paramBuffer[4] = runStepsCmdPtr->curveSelect;
    can2SendCmd.paramBuffer[5] = runStepsCmdPtr->speedRatio;
    can2SendCmd.paramBuffer[6] = runStepsCmdPtr->speedMode;
    can2SendCmd.paramBuffer[7] = runStepsCmdPtr->utilStop1Enable;
    can2SendCmd.paramBuffer[8] = runStepsCmdPtr->utilStop2Enable;
    Can2ProcUtilConvertUint32ToArray(can2SendCmd.paramBuffer,9,runStepsCmdPtr->timeOutMs);
    //指令码设定
    can2SendCmd.commandCode = SM_CMD_RUN_STEPS;
    can2SendCmd.timeoutMs = runStepsCmdPtr->timeOutMs;
    //数据发送
    errCode = Can2SendCommandWhileReturn(&can2SendCmd,&can2ReadData);
    //释放内存
    UserMemFree(SRAM_IN,can2SendCmd.paramBuffer);
    //接收到的数据不需要
    if(can2ReadData.recvDataPtr != NULL)
    {
        //释放内存
        UserMemFree(SRAM_IN,can2ReadData.recvDataPtr);
    }
    return errCode;
}

//系统内部电机运行到指定坐标
LH_ERR Can2SubSM_RunToCoordinateWhileReturn(SM_RUN_COORDINATE_CMD* runCoordinateCmdPtr)
{
    CAN2_SEND_CMD can2SendCmd;
    CAN2_READ_DATA can2ReadData;
    LH_ERR errCode = LH_ERR_NONE;
    //初始化接收器
    can2ReadData.recvDataPtr = NULL;
    can2ReadData.recvDatLength = 0;
    //计算板卡号码和单元号码
    errCode = Can2SubSM_CalcBoardChannel(runCoordinateCmdPtr->stepMotorIndex,&(can2SendCmd.targetBoardID),
                                &(can2SendCmd.channelNo));
    if(errCode != LH_ERR_NONE)
    {
        return errCode;
    }
    //申请内存
    can2SendCmd.paramBuffer = NULL;
    can2SendCmd.paramLength = 13;
    do
    {
        can2SendCmd.paramBuffer = UserMemMalloc(SRAM_IN,sizeof(can2SendCmd.paramLength));
        if(can2SendCmd.paramBuffer == NULL)
        {
            CoreDelayMinTick();
        }
    }while(can2SendCmd.paramBuffer == NULL);
    //内存数据拷贝
    Can2ProcUtilConvertInt32ToArray(can2SendCmd.paramBuffer,0,runCoordinateCmdPtr->targetCoordinate);
    can2SendCmd.paramBuffer[4] = runCoordinateCmdPtr->curveSelect;
    can2SendCmd.paramBuffer[5] = runCoordinateCmdPtr->speedRatio;
    can2SendCmd.paramBuffer[6] = runCoordinateCmdPtr->speedMode;
    can2SendCmd.paramBuffer[7] = runCoordinateCmdPtr->utilStop1Enable;
    can2SendCmd.paramBuffer[8] = runCoordinateCmdPtr->utilStop2Enable;
    Can2ProcUtilConvertUint32ToArray(can2SendCmd.paramBuffer,9,runCoordinateCmdPtr->timeOutMs);
    //指令码设定
    can2SendCmd.commandCode = SM_CMD_RUN_CORDINATE;
    can2SendCmd.timeoutMs = runCoordinateCmdPtr->timeOutMs;
    //数据发送
    errCode = Can2SendCommandWhileReturn(&can2SendCmd,&can2ReadData);
    //释放内存
    UserMemFree(SRAM_IN,can2SendCmd.paramBuffer);
    //接收到的数据不需要
    if(can2ReadData.recvDataPtr != NULL)
    {
        //释放内存
        UserMemFree(SRAM_IN,can2ReadData.recvDataPtr);
    }
    return errCode;
}

//系统内部电机回零
LH_ERR Can2SubSM_ReturnZeroWhileReturn(SM_RETURN_ZERO_CMD* returnZeroCmdPtr)
{
    CAN2_SEND_CMD can2SendCmd;
    CAN2_READ_DATA can2ReadData;
    LH_ERR errCode = LH_ERR_NONE;
    //初始化接收器
    can2ReadData.recvDataPtr = NULL;
    can2ReadData.recvDatLength = 0;
    //计算板卡号码和单元号码
    errCode = Can2SubSM_CalcBoardChannel(returnZeroCmdPtr->stepMotorIndex,&(can2SendCmd.targetBoardID),
                                &(can2SendCmd.channelNo));
    if(errCode != LH_ERR_NONE)
    {
        return errCode;
    }
    //申请内存
    can2SendCmd.paramBuffer = NULL;
    can2SendCmd.paramLength = 13;
    do
    {
        can2SendCmd.paramBuffer = UserMemMalloc(SRAM_IN,sizeof(can2SendCmd.paramLength));
        if(can2SendCmd.paramBuffer == NULL)
        {
            CoreDelayMinTick();
        }
    }while(can2SendCmd.paramBuffer == NULL);
    //内存数据拷贝
    Can2ProcUtilConvertInt32ToArray(can2SendCmd.paramBuffer,0,returnZeroCmdPtr->correctionPosition);
    can2SendCmd.paramBuffer[4] = returnZeroCmdPtr->curveSelect;
    can2SendCmd.paramBuffer[5] = returnZeroCmdPtr->speedRatio;
    can2SendCmd.paramBuffer[6] = returnZeroCmdPtr->speedMode;
    can2SendCmd.paramBuffer[7] = returnZeroCmdPtr->utilStop1Enable;
    can2SendCmd.paramBuffer[8] = returnZeroCmdPtr->utilStop2Enable;
    Can2ProcUtilConvertUint32ToArray(can2SendCmd.paramBuffer,9,returnZeroCmdPtr->timeOutMs);
    //指令码设定
    can2SendCmd.commandCode = SM_CMD_RETURN_ZERO;
    can2SendCmd.timeoutMs = returnZeroCmdPtr->timeOutMs;
    //数据发送
    errCode = Can2SendCommandWhileReturn(&can2SendCmd,&can2ReadData);
    //释放内存
    UserMemFree(SRAM_IN,can2SendCmd.paramBuffer);
    //接收到的数据不需要
    if(can2ReadData.recvDataPtr != NULL)
    {
        //释放内存
        UserMemFree(SRAM_IN,can2ReadData.recvDataPtr);
    }
    return errCode;
}

//系统内部电机电机持续运转
LH_ERR Can2SubSM_RunAlwaysWhileReturn(SM_RUN_ALWAYS_CMD* runAlwaysCmdPtr)
{
    CAN2_SEND_CMD can2SendCmd;
    CAN2_READ_DATA can2ReadData;
    LH_ERR errCode = LH_ERR_NONE;
    //初始化接收器
    can2ReadData.recvDataPtr = NULL;
    can2ReadData.recvDatLength = 0;
    //计算板卡号码和单元号码
    errCode = Can2SubSM_CalcBoardChannel(runAlwaysCmdPtr->stepMotorIndex,&(can2SendCmd.targetBoardID),
                                &(can2SendCmd.channelNo));
    if(errCode != LH_ERR_NONE)
    {
        return errCode;
    }
    //申请内存
    can2SendCmd.paramBuffer = NULL;
    can2SendCmd.paramLength = 3;
    do
    {
        can2SendCmd.paramBuffer = UserMemMalloc(SRAM_IN,sizeof(can2SendCmd.paramLength));
        if(can2SendCmd.paramBuffer == NULL)
        {
            CoreDelayMinTick();
        }
    }while(can2SendCmd.paramBuffer == NULL);
    //内存数据拷贝
    can2SendCmd.paramBuffer[0] = runAlwaysCmdPtr->curveSelect;
    can2SendCmd.paramBuffer[1] = runAlwaysCmdPtr->speedRatio;
    can2SendCmd.paramBuffer[2] = runAlwaysCmdPtr->speedMode;
    //指令码设定
    can2SendCmd.commandCode = SM_CMD_RUN_ALWAYS;
    can2SendCmd.timeoutMs = 500000;
    //数据发送
    errCode = Can2SendCommandWhileReturn(&can2SendCmd,&can2ReadData);
    //释放内存
    UserMemFree(SRAM_IN,can2SendCmd.paramBuffer);
    //接收到的数据不需要
    if(can2ReadData.recvDataPtr != NULL)
    {
        //释放内存
        UserMemFree(SRAM_IN,can2ReadData.recvDataPtr);
    }
    return errCode;
}

//系统内部电机急停
LH_ERR Can2SubSM_StopImmediatelyWhileReturn(CAN2_SUB_SM_INDEX motorIndex)
{
    CAN2_SEND_CMD can2SendCmd;
    CAN2_READ_DATA can2ReadData;
    LH_ERR errCode = LH_ERR_NONE;
    //初始化接收器
    can2ReadData.recvDataPtr = NULL;
    can2ReadData.recvDatLength = 0;
    //计算板卡号码和单元号码
    errCode = Can2SubSM_CalcBoardChannel(motorIndex,&(can2SendCmd.targetBoardID),
                                &(can2SendCmd.channelNo));
    if(errCode != LH_ERR_NONE)
    {
        return errCode;
    }
    //申请内存
    can2SendCmd.paramBuffer = NULL;
    can2SendCmd.paramLength = 0;
    //指令码设定
    can2SendCmd.commandCode = SM_CMD_STOP_IMME;
    can2SendCmd.timeoutMs = 1000;
    //数据发送
    errCode = Can2SendCommandWhileReturn(&can2SendCmd,&can2ReadData);
    //接收到的数据不需要
    if(can2ReadData.recvDataPtr != NULL)
    {
        //释放内存
        UserMemFree(SRAM_IN,can2ReadData.recvDataPtr);
    }
    return errCode;
}

//系统内部电机减速停止
LH_ERR Can2SubSM_StopGraduallyWhileReturn(CAN2_SUB_SM_INDEX motorIndex)
{
    CAN2_SEND_CMD can2SendCmd;
    CAN2_READ_DATA can2ReadData;
    LH_ERR errCode = LH_ERR_NONE;
    //初始化接收器
    can2ReadData.recvDataPtr = NULL;
    can2ReadData.recvDatLength = 0;
    //计算板卡号码和单元号码
    errCode = Can2SubSM_CalcBoardChannel(motorIndex,&(can2SendCmd.targetBoardID),
                                &(can2SendCmd.channelNo));
    if(errCode != LH_ERR_NONE)
    {
        return errCode;
    }
    //不需要申请内存
    can2SendCmd.paramBuffer = NULL;
    can2SendCmd.paramLength = 0;
    //指令码设定
    can2SendCmd.commandCode = SM_CMD_STOP_DACC;
    can2SendCmd.timeoutMs = 5000;
    //数据发送
    errCode = Can2SendCommandWhileReturn(&can2SendCmd,&can2ReadData);
    //接收到的数据不需要
    if(can2ReadData.recvDataPtr != NULL)
    {
        //释放内存
        UserMemFree(SRAM_IN,can2ReadData.recvDataPtr);
    }
    return errCode;
}

//系统读取指定电机当前位置
LH_ERR Can2SubSM_ReadPositionWhileReturn(CAN2_SUB_SM_INDEX motorIndex,int32_t* motorPos)
{
    CAN2_SEND_CMD can2SendCmd;
    CAN2_READ_DATA can2ReadData;
    LH_ERR errCode = LH_ERR_NONE;
    //初始化接收器
    can2ReadData.recvDataPtr = NULL;
    can2ReadData.recvDatLength = 0;
    //计算板卡号码和单元号码
    errCode = Can2SubSM_CalcBoardChannel(motorIndex,&(can2SendCmd.targetBoardID),
                                &(can2SendCmd.channelNo));
    if(errCode != LH_ERR_NONE)
    {
        return errCode;
    }
    //不需要申请内存
    can2SendCmd.paramBuffer = NULL;
    can2SendCmd.paramLength = 0;
    //指令码设定
    can2SendCmd.commandCode = SM_CMD_READ_POS;
    can2SendCmd.timeoutMs = 200;
    //数据发送
    errCode = Can2SendCommandWhileReturn(&can2SendCmd,&can2ReadData);
    //首先看指令是否出错
    if(errCode != LH_ERR_NONE)
    {
        //错误产生
        if(can2ReadData.recvDataPtr != NULL)
        {
            //释放内存
            UserMemFree(SRAM_IN,can2ReadData.recvDataPtr);
        }
        return errCode;
    }
    //数据为空
    if(can2ReadData.recvDataPtr == NULL)
    {
        return LH_ERR_SM_DATA_NULL;
    }
    //接收到的数据不需要
    if(can2ReadData.recvDatLength != 10)
    {
        //释放内存
        UserMemFree(SRAM_IN,can2ReadData.recvDataPtr);
        return LH_ERR_SM_DATA_LENGTH;
    }
    //读取数据
    if(CAN2_SUB_DATA_MOTOR != Can2ProcUtilConvertArrayToUint16(can2ReadData.recvDataPtr,0))
    {
        //数据标志位错误
        UserMemFree(SRAM_IN,can2ReadData.recvDataPtr);
        return LH_ERR_SM_DATA_FLAG;
    }
    //读取坐标
    *motorPos = Can2ProcUtilConvertArrayToInt32(can2ReadData.recvDataPtr,2);
    //释放内存
    UserMemFree(SRAM_IN,can2ReadData.recvDataPtr);
    return errCode;
}

/*********************************************************等待对应的电机动作执行完成**************************************************************/
LH_ERR Can2SubSM_WaitLastReturn(CAN2_SUB_SM_INDEX motorIndex,uint32_t timeOutMs)
{
    CAN2_READ_DATA can2ReadData;
    CAN2_SUB_BOARD boardID;
    CAN2_CHANNEL channelNo;
    LH_ERR errCode = LH_ERR_NONE;
    //计算板卡号码和单元号码
    errCode = Can2SubSM_CalcBoardChannel(motorIndex,&boardID,&channelNo);
    if(errCode != LH_ERR_NONE)
    {
        return errCode;
    }
    can2ReadData.recvDataPtr = NULL;
    can2ReadData.recvDatLength = 0;
    errCode = Can2SendCommandWaitReturn(boardID,channelNo,timeOutMs,&can2ReadData);
    //接收到的数据不需要
    if(can2ReadData.recvDataPtr != NULL)
    {
        //释放内存
        UserMemFree(SRAM_IN,can2ReadData.recvDataPtr);
    }
    return errCode;
}


//复位指令的参数的初始化
void Can2SubSM_ResetCmdDataStructInit(SM_RESET_CMD* cmdPtr)
{
    cmdPtr->stepMotorIndex = CAN2_SUB_SM_BOARD1_SM1;
    cmdPtr->correctionPosition = 0;
    cmdPtr->timeOutMs = 60000;
}

//走步数指令的参数的初始化
void Can2SubSM_RunStepsCmdDataStructInit(SM_RUN_STEP_CMD* cmdPtr)
{
    cmdPtr->stepMotorIndex = CAN2_SUB_SM_BOARD1_SM1;
    cmdPtr->steps = 0;
    cmdPtr->curveSelect = SM_CURVE_0;
    cmdPtr->speedRatio = 100;
    cmdPtr->speedMode = SPEED_MODE_UP_START;
    cmdPtr->utilStop1Enable = DISABLE;
    cmdPtr->utilStop2Enable = DISABLE;
    cmdPtr->timeOutMs = 60000;
}

//走坐标指令的参数的初始化
void Can2SubSM_RunCoordinateDataStructInit(SM_RUN_COORDINATE_CMD* cmdPtr)
{
    cmdPtr->stepMotorIndex = CAN2_SUB_SM_BOARD1_SM1;
    cmdPtr->targetCoordinate = 0;
    cmdPtr->curveSelect = SM_CURVE_0;
    cmdPtr->speedRatio = 100;
    cmdPtr->speedMode = SPEED_MODE_UP_START;
    cmdPtr->utilStop1Enable = DISABLE;
    cmdPtr->utilStop2Enable = DISABLE;
    cmdPtr->timeOutMs = 60000;
}

//回零指令的参数的初始化
void Can2SubSM_ReturnZeroDataStructInit(SM_RETURN_ZERO_CMD* cmdPtr)
{
    cmdPtr->stepMotorIndex = CAN2_SUB_SM_BOARD1_SM1;
    cmdPtr->correctionPosition = 0;
    cmdPtr->curveSelect = SM_CURVE_0;
    cmdPtr->speedRatio = 100;
    cmdPtr->speedMode = SPEED_MODE_UP_START;
    cmdPtr->utilStop1Enable = DISABLE;
    cmdPtr->utilStop2Enable = DISABLE;
    cmdPtr->timeOutMs = 60000;
}

//持续运转指令的参数的初始化
void Can2SubSM_RunAlwaysDataStructInit(SM_RUN_ALWAYS_CMD* cmdPtr)
{
    cmdPtr->stepMotorIndex = CAN2_SUB_SM_BOARD1_SM1;
    cmdPtr->curveSelect = SM_CURVE_0;
    cmdPtr->speedRatio = 100;
    cmdPtr->speedMode = SPEED_MODE_UP_START;
}








