#include "Can2SubOutputWrite.h"

static LH_ERR Can2SubOutputWriteCalcPinIndex(CAN2_SUB_OUT_WRITE* outPin,CAN2_SUB_BOARD* boardIndex,CAN2_CHANNEL* channelIndex)
{
    *channelIndex = CAN2_CHANNEL_OUTPUT;
    //超过最大值
    if(outPin->outputPinIndex > CAN2_SUB_OUT_PIN_BOARD10_MCU_YM_ARM4_PG15)
    {
        return LH_ERR_LOW_OUTPIN_INDEX;
    }
    if(outPin->outputPinIndex <= CAN2_SUB_OUT_PIN_BOARD1_MCU_YM_ARM4_PG15)
    {
        outPin->outputPinIndex = (CAN2_SUB_IO_OUTPUT)(outPin->outputPinIndex - CAN2_SUB_OUT_PIN_BOARD1_CPLD_CDC1);
        *boardIndex = CAN2_SUB_BOARD1;
        return LH_ERR_NONE;
    }
    else if(outPin->outputPinIndex <= CAN2_SUB_OUT_PIN_BOARD2_MCU_YM_ARM4_PG15)
    {
        outPin->outputPinIndex = (CAN2_SUB_IO_OUTPUT)(outPin->outputPinIndex - CAN2_SUB_OUT_PIN_BOARD2_CPLD_CDC1);
        *boardIndex = CAN2_SUB_BOARD2;
        return LH_ERR_NONE;
    }
    else if(outPin->outputPinIndex <= CAN2_SUB_OUT_PIN_BOARD3_MCU_YM_ARM4_PG15)
    {
        outPin->outputPinIndex = (CAN2_SUB_IO_OUTPUT)(outPin->outputPinIndex - CAN2_SUB_OUT_PIN_BOARD3_CPLD_CDC1);
        *boardIndex = CAN2_SUB_BOARD3;
        return LH_ERR_NONE;
    }
    else if(outPin->outputPinIndex <= CAN2_SUB_OUT_PIN_BOARD4_MCU_YM_ARM4_PG15)
    {
        outPin->outputPinIndex = (CAN2_SUB_IO_OUTPUT)(outPin->outputPinIndex - CAN2_SUB_OUT_PIN_BOARD4_CPLD_CDC1);
        *boardIndex = CAN2_SUB_BOARD4;
        return LH_ERR_NONE;
    }
    else if(outPin->outputPinIndex <= CAN2_SUB_OUT_PIN_BOARD5_MCU_YM_ARM4_PG15)
    {
        outPin->outputPinIndex = (CAN2_SUB_IO_OUTPUT)(outPin->outputPinIndex - CAN2_SUB_OUT_PIN_BOARD5_CPLD_CDC1);
        *boardIndex = CAN2_SUB_BOARD5;
        return LH_ERR_NONE;
    }
    else if(outPin->outputPinIndex <= CAN2_SUB_OUT_PIN_BOARD6_MCU_YM_ARM4_PG15)
    {
        outPin->outputPinIndex = (CAN2_SUB_IO_OUTPUT)(outPin->outputPinIndex - CAN2_SUB_OUT_PIN_BOARD6_CPLD_CDC1);
        *boardIndex = CAN2_SUB_BOARD6;
        return LH_ERR_NONE;
    }
    else if(outPin->outputPinIndex <= CAN2_SUB_OUT_PIN_BOARD7_MCU_YM_ARM4_PG15)
    {
        outPin->outputPinIndex = (CAN2_SUB_IO_OUTPUT)(outPin->outputPinIndex - CAN2_SUB_OUT_PIN_BOARD7_CPLD_CDC1);
        *boardIndex = CAN2_SUB_BOARD7;
        return LH_ERR_NONE;
    }
    else if(outPin->outputPinIndex <= CAN2_SUB_OUT_PIN_BOARD8_MCU_YM_ARM4_PG15)
    {
        outPin->outputPinIndex = (CAN2_SUB_IO_OUTPUT)(outPin->outputPinIndex - CAN2_SUB_OUT_PIN_BOARD8_CPLD_CDC1);
        *boardIndex = CAN2_SUB_BOARD8;
        return LH_ERR_NONE;
    }
    else if(outPin->outputPinIndex <= CAN2_SUB_OUT_PIN_BOARD9_MCU_YM_ARM4_PG15)
    {
        outPin->outputPinIndex = (CAN2_SUB_IO_OUTPUT)(outPin->outputPinIndex - CAN2_SUB_OUT_PIN_BOARD9_CPLD_CDC1);
        *boardIndex = CAN2_SUB_BOARD9;
        return LH_ERR_NONE;
    }
    else if(outPin->outputPinIndex <= CAN2_SUB_OUT_PIN_BOARD10_MCU_YM_ARM4_PG15)
    {
        outPin->outputPinIndex = (CAN2_SUB_IO_OUTPUT)(outPin->outputPinIndex - CAN2_SUB_OUT_PIN_BOARD10_CPLD_CDC1);
        *boardIndex = CAN2_SUB_BOARD10;
        return LH_ERR_NONE;
    }
    return LH_ERR_NONE;
}

//指定的IO写入结构体初始化
void Can2SubOutWriteDataStructInit(CAN2_SUB_OUT_WRITE* writePin)
{
    writePin->outputPinIndex = CAN2_SUB_OUT_PIN_BOARD1_CPLD_CDC1;
    writePin->setState = Bit_SET;
}

//写入系统内部某个IO状态
LH_ERR Can2SubOutWriteState(CAN2_SUB_OUT_WRITE* writePin)
{
    CAN2_SEND_CMD can2SendCmd;
    CAN2_READ_DATA can2ReadData;
    LH_ERR errCode = LH_ERR_NONE;
    CAN2_SUB_IO_OUTPUT outPinIndexLocal = writePin->outputPinIndex;
    //初始化接收器
    can2ReadData.recvDataPtr = NULL;
    can2ReadData.recvDatLength = 0;
    //计算板卡号码和单元号码
    errCode = Can2SubOutputWriteCalcPinIndex(writePin,&(can2SendCmd.targetBoardID),
                                &(can2SendCmd.channelNo));
    if(errCode != LH_ERR_NONE)
    {
        return errCode;
    }
    //申请内存
    can2SendCmd.paramBuffer = NULL;
    can2SendCmd.paramLength = 2;
    do
    {
        can2SendCmd.paramBuffer = UserMemMalloc(SRAM_IN,2);
        if(can2SendCmd.paramBuffer == NULL)
        {
            CoreDelayMinTick();
        }
    }while(can2SendCmd.paramBuffer == NULL);
    //指令码设定
    can2SendCmd.commandCode = CAN2_CMD_OUTPUT_WRITE_SINGLE;
    can2SendCmd.timeoutMs = 200;
    //写入参数
    can2SendCmd.paramBuffer[0] = writePin->outputPinIndex;
    can2SendCmd.paramBuffer[1] = writePin->setState;
    //数据发送
    errCode = Can2SendCommandWhileReturn(&can2SendCmd,&can2ReadData);
    //释放参数内存
    UserMemFree(SRAM_IN,can2SendCmd.paramBuffer);
    //接收到的数据不需要
    if(can2ReadData.recvDataPtr != NULL)
    {
        //释放内存
        UserMemFree(SRAM_IN,can2ReadData.recvDataPtr);
    }
    writePin->outputPinIndex = outPinIndexLocal;
    return errCode;
}


