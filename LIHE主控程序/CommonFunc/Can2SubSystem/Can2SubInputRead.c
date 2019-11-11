#include "Can2SubInputRead.h"

LH_ERR Can2SubPortInCalcBoardChannel(CAN2_SUB_INPUT* input,CAN2_SUB_BOARD* boardIndex,CAN2_CHANNEL* channelIndex)
{
    *channelIndex = CAN2_CHANNEL_INPUT;
    //超过最大值
    if(input->inputPinIndex > CAN2_SUB_IN_PIN_BOARD10_MCU_YM_ARM2_PB5)
    {
        return LH_ERR_LOW_INPIN_INDEX;
    }
    if(input->inputPinIndex <= CAN2_SUB_IN_PIN_BOARD1_MCU_YM_ARM2_PB5)
    {
        input->inputPinIndex = (CAN2_SUB_IN_PIN_INDEX)(input->inputPinIndex - CAN2_SUB_IN_PIN_BOARD1_CPLD_NOP1);
        *boardIndex = CAN2_SUB_BOARD1;
        return LH_ERR_NONE;
    }
    else if(input->inputPinIndex <= CAN2_SUB_IN_PIN_BOARD2_MCU_YM_ARM2_PB5)
    {
        input->inputPinIndex = (CAN2_SUB_IN_PIN_INDEX)(input->inputPinIndex - CAN2_SUB_IN_PIN_BOARD2_CPLD_NOP1);
        *boardIndex = CAN2_SUB_BOARD2;
        return LH_ERR_NONE;
    }
    else if(input->inputPinIndex <= CAN2_SUB_IN_PIN_BOARD3_MCU_YM_ARM2_PB5)
    {
        input->inputPinIndex = (CAN2_SUB_IN_PIN_INDEX)(input->inputPinIndex - CAN2_SUB_IN_PIN_BOARD3_CPLD_NOP1);
        *boardIndex = CAN2_SUB_BOARD3;
        return LH_ERR_NONE;
    }
    else if(input->inputPinIndex <= CAN2_SUB_IN_PIN_BOARD4_MCU_YM_ARM2_PB5)
    {
        input->inputPinIndex = (CAN2_SUB_IN_PIN_INDEX)(input->inputPinIndex - CAN2_SUB_IN_PIN_BOARD4_CPLD_NOP1);
        *boardIndex = CAN2_SUB_BOARD4;
        return LH_ERR_NONE;
    }
    else if(input->inputPinIndex <= CAN2_SUB_IN_PIN_BOARD5_MCU_YM_ARM2_PB5)
    {
        input->inputPinIndex = (CAN2_SUB_IN_PIN_INDEX)(input->inputPinIndex - CAN2_SUB_IN_PIN_BOARD5_CPLD_NOP1);
        *boardIndex = CAN2_SUB_BOARD5;
        return LH_ERR_NONE;
    }
    else if(input->inputPinIndex <= CAN2_SUB_IN_PIN_BOARD6_MCU_YM_ARM2_PB5)
    {
        input->inputPinIndex = (CAN2_SUB_IN_PIN_INDEX)(input->inputPinIndex - CAN2_SUB_IN_PIN_BOARD6_CPLD_NOP1);
        *boardIndex = CAN2_SUB_BOARD6;
        return LH_ERR_NONE;
    }
    else if(input->inputPinIndex <= CAN2_SUB_IN_PIN_BOARD7_MCU_YM_ARM2_PB5)
    {
        input->inputPinIndex = (CAN2_SUB_IN_PIN_INDEX)(input->inputPinIndex - CAN2_SUB_IN_PIN_BOARD7_CPLD_NOP1);
        *boardIndex = CAN2_SUB_BOARD7;
        return LH_ERR_NONE;
    }
    else if(input->inputPinIndex <= CAN2_SUB_IN_PIN_BOARD8_MCU_YM_ARM2_PB5)
    {
        input->inputPinIndex = (CAN2_SUB_IN_PIN_INDEX)(input->inputPinIndex - CAN2_SUB_IN_PIN_BOARD8_CPLD_NOP1);
        *boardIndex = CAN2_SUB_BOARD8;
        return LH_ERR_NONE;
    }
    else if(input->inputPinIndex <= CAN2_SUB_IN_PIN_BOARD9_MCU_YM_ARM2_PB5)
    {
        input->inputPinIndex = (CAN2_SUB_IN_PIN_INDEX)(input->inputPinIndex - CAN2_SUB_IN_PIN_BOARD9_CPLD_NOP1);
        *boardIndex = CAN2_SUB_BOARD9;
        return LH_ERR_NONE;
    }
    else if(input->inputPinIndex <= CAN2_SUB_IN_PIN_BOARD10_MCU_YM_ARM2_PB5)
    {
        input->inputPinIndex = (CAN2_SUB_IN_PIN_INDEX)(input->inputPinIndex - CAN2_SUB_IN_PIN_BOARD10_CPLD_NOP1);
        *boardIndex = CAN2_SUB_BOARD10;
        return LH_ERR_NONE;
    }
    return LH_ERR_NONE;
}

//输入IO口的数据结构初始化
void Can2SubPortInDataStructInit(CAN2_SUB_INPUT* input)
{
    input->inputPinIndex = CAN2_SUB_IN_PIN_BOARD1_CPLD_NOP1;
    input->pinValue = Bit_RESET;
}

//读取系统内部某个IO状态
LH_ERR Can2SubPortInReadState(CAN2_SUB_INPUT* input)
{
    CAN2_SEND_CMD can2SendCmd;
    CAN2_READ_DATA can2ReadData;
    LH_ERR errCode = LH_ERR_NONE;
    //初始化接收器
    can2ReadData.recvDataPtr = NULL;
    can2ReadData.recvDatLength = 0;
    CAN2_SUB_IN_PIN_INDEX pinIndexLocal = input->inputPinIndex;
    //计算板卡号码和单元号码
    errCode = Can2SubPortInCalcBoardChannel(input,&(can2SendCmd.targetBoardID),&(can2SendCmd.channelNo));
    if(errCode != LH_ERR_NONE)
    {
        return errCode;
    }
    //申请内存
    can2SendCmd.paramBuffer = NULL;
    can2SendCmd.paramLength = 1;
    do
    {
        can2SendCmd.paramBuffer = UserMemMalloc(SRAM_IN,1);
        if(can2SendCmd.paramBuffer == NULL)
        {
            CoreDelayMinTick();
        }
    }while(can2SendCmd.paramBuffer == NULL);

    //指令码设定
    can2SendCmd.commandCode = CAN2_SUB_INPUT_CMD_READ_SINGLE;
    can2SendCmd.timeoutMs = 200;
    can2SendCmd.paramBuffer[0] = input->inputPinIndex;

    //数据发送
    errCode = Can2SendCommandWhileReturn(&can2SendCmd,&can2ReadData);
    //首先看指令是否出错
    if(errCode != LH_ERR_NONE)
    {
        //释放发送内存
        UserMemFree(SRAM_IN,can2SendCmd.paramBuffer);
        //错误产生
        if(can2ReadData.recvDataPtr != NULL)
        {
            //释放内存
            UserMemFree(SRAM_IN,can2ReadData.recvDataPtr);
        }
        input->inputPinIndex = pinIndexLocal;
        return errCode;
    }
    //数据为空
    if(can2ReadData.recvDataPtr == NULL)
    {
        UserMemFree(SRAM_IN,can2SendCmd.paramBuffer);
        input->inputPinIndex = pinIndexLocal;
        return LH_ERR_LOW_INPIN_DATA_NULL;
    }
    //接收到的数据不需要
    if(can2ReadData.recvDatLength != 4)
    {
        //释放发送内存
        UserMemFree(SRAM_IN,can2SendCmd.paramBuffer);
        //释放接收内存
        UserMemFree(SRAM_IN,can2ReadData.recvDataPtr);
        input->inputPinIndex = pinIndexLocal;
        //释放内存
        return LH_ERR_LOW_INPIN_DATA_LENGTH;
    }
    //读取数据
    if(CAN2_SUB_DATA_IO_IN != Can2ProcUtilConvertArrayToUint16(can2ReadData.recvDataPtr,0))
    {
        //释放发送内存
        UserMemFree(SRAM_IN,can2SendCmd.paramBuffer);
        //释放接收内存
        UserMemFree(SRAM_IN,can2ReadData.recvDataPtr);
        input->inputPinIndex = pinIndexLocal;
        //数据标志位错误
        return LH_ERR_LOW_INPIN_DATA_FLAG;
    }
    if(can2ReadData.recvDataPtr[2] != input->inputPinIndex)
    {
        //释放发送内存
        UserMemFree(SRAM_IN,can2SendCmd.paramBuffer);
        //释放接收内存
        UserMemFree(SRAM_IN,can2ReadData.recvDataPtr);
        input->inputPinIndex = pinIndexLocal;
        //数据标志位错误
        return LH_ERR_LOW_INPIN_DATA_INDEX;
    }
    //读取坐标
    input->pinValue = (BitAction)can2ReadData.recvDataPtr[3];
    //释放发送内存
    UserMemFree(SRAM_IN,can2SendCmd.paramBuffer);
    //释放接收内存
    UserMemFree(SRAM_IN,can2ReadData.recvDataPtr);
    input->inputPinIndex = pinIndexLocal;
    return errCode;
}

//指定板卡上所有步进电机急停
LH_ERR Can2SubPortInAllMotorEmergency(CAN2_SUB_BOARD board)
{
    CAN2_SEND_CMD can2SendCmd;
    CAN2_READ_DATA can2ReadData;
    LH_ERR errCode = LH_ERR_NONE;
    //初始化接收器
    can2ReadData.recvDataPtr = NULL;
    can2ReadData.recvDatLength = 0;
    can2SendCmd.targetBoardID = board;
    can2SendCmd.channelNo = CAN2_CHANNEL_INPUT;
    //不需要申请内存
    can2SendCmd.paramBuffer = NULL;
    can2SendCmd.paramLength = 0;
    //指令码设定
    can2SendCmd.commandCode = CAN2_SUB_INPUT_CMD_ALL_MOTOR_EMERGENCY;
    can2SendCmd.timeoutMs = 100;
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
