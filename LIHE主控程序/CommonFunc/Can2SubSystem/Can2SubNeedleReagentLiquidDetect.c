#include "Can2SubNeedleReagentLiquidDetect.h"

//检测探液板是否存在
LH_ERR Can2SubNeedleReagentLiquidDetectCheckExist(void)
{
    CAN2_SEND_CMD can2SendCmd;
    CAN2_READ_DATA can2ReadData;
    LH_ERR errCode = LH_ERR_NONE;

    can2SendCmd.targetBoardID = NEEDLE_REAGENT_BOARD_ID;
    can2SendCmd.channelNo = CAN2_CHANNEL_SERIAL1;

    //申请内存
    can2SendCmd.paramBuffer = NULL;
    can2SendCmd.paramLength = 0;

    //指令码设定
    can2SendCmd.commandCode = CAN2_SUB_NEEDLE_LIQUID_DETECT_CMD_CHECK_EXIST;
    can2SendCmd.timeoutMs = 60000;

    //数据发送
    errCode = Can2SendCommandWhileReturn(&can2SendCmd,&can2ReadData);

    //发送完成,释放发送内存
    if(can2SendCmd.paramBuffer != NULL)
    {
        UserMemFree(SRAM_IN,can2SendCmd.paramBuffer);
    }

    //不接收数据,所以把接收内存也清除
    if(can2ReadData.recvDataPtr != NULL)
    {
        //释放内存
        UserMemFree(SRAM_IN,can2ReadData.recvDataPtr);
    }

    return errCode;
}

//读取探液板检测阈值
LH_ERR Can2SubNeedleReagentLiquidDetectReadThresholdValue(uint16_t* paramReadPtr)
{
    CAN2_SEND_CMD can2SendCmd;
    CAN2_READ_DATA can2ReadData;
    LH_ERR errCode = LH_ERR_NONE;

    can2SendCmd.targetBoardID = NEEDLE_REAGENT_BOARD_ID;
    can2SendCmd.channelNo = CAN2_CHANNEL_SERIAL1;

    //申请内存
    can2SendCmd.paramBuffer = NULL;
    can2SendCmd.paramLength = 0;

    //指令码设定
    can2SendCmd.commandCode = CAN2_SUB_NEEDLE_LIQUID_DETECT_CMD_READ_THRESHOLD_VALUE;
    can2SendCmd.timeoutMs = 60000;

    //数据发送
    errCode = Can2SendCommandWhileReturn(&can2SendCmd,&can2ReadData);

    //发送完成,释放发送内存
    if(can2SendCmd.paramBuffer != NULL)
    {
        UserMemFree(SRAM_IN,can2SendCmd.paramBuffer);
    }

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
        return LH_ERR_LOW_SERIAL_DATA_NULL;
    }

    //接收到的数据不需要
    if(can2ReadData.recvDatLength != 4)//2 + 2
    {
        //释放内存
        UserMemFree(SRAM_IN,can2ReadData.recvDataPtr);
        return LH_ERR_LOW_SERIAL_DATA_LENGTH;
    }

    //读取数据
    if(CAN2_SUB_DATA_SERIAL_RS1 != Can2ProcUtilConvertArrayToUint16(can2ReadData.recvDataPtr,0))
    {
        //数据标志位错误
        UserMemFree(SRAM_IN,can2ReadData.recvDataPtr);
        return LH_ERR_LOW_SERIAL_DATA_FLAG;
    }

    //读取结果
    *paramReadPtr = Can2ProcUtilConvertArrayToUint16(can2ReadData.recvDataPtr,2);

    //释放内存
    UserMemFree(SRAM_IN,can2ReadData.recvDataPtr);
    return errCode;
}

//读取探液板阈值检测次数
LH_ERR Can2SubNeedleReagentLiquidDetectReadThresholdCount(uint16_t* paramReadPtr)
{
    CAN2_SEND_CMD can2SendCmd;
    CAN2_READ_DATA can2ReadData;
    LH_ERR errCode = LH_ERR_NONE;

    can2SendCmd.targetBoardID = NEEDLE_REAGENT_BOARD_ID;
    can2SendCmd.channelNo = CAN2_CHANNEL_SERIAL1;

    //申请内存
    can2SendCmd.paramBuffer = NULL;
    can2SendCmd.paramLength = 0;

    //指令码设定
    can2SendCmd.commandCode = CAN2_SUB_NEEDLE_LIQUID_DETECT_CMD_READ_THRESHOLD_COUNT;
    can2SendCmd.timeoutMs = 60000;

    //数据发送
    errCode = Can2SendCommandWhileReturn(&can2SendCmd,&can2ReadData);

    //发送完成,释放发送内存
    if(can2SendCmd.paramBuffer != NULL)
    {
        UserMemFree(SRAM_IN,can2SendCmd.paramBuffer);
    }

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
        return LH_ERR_LOW_SERIAL_DATA_NULL;
    }

    //接收到的数据不需要
    if(can2ReadData.recvDatLength != 4)//2 + 2
    {
        //释放内存
        UserMemFree(SRAM_IN,can2ReadData.recvDataPtr);
        return LH_ERR_LOW_SERIAL_DATA_LENGTH;
    }

    //读取数据
    if(CAN2_SUB_DATA_SERIAL_RS1 != Can2ProcUtilConvertArrayToUint16(can2ReadData.recvDataPtr,0))
    {
        //数据标志位错误
        UserMemFree(SRAM_IN,can2ReadData.recvDataPtr);
        return LH_ERR_LOW_SERIAL_DATA_FLAG;
    }

    //读取结果
    *paramReadPtr = Can2ProcUtilConvertArrayToUint16(can2ReadData.recvDataPtr,2);

    //释放内存
    UserMemFree(SRAM_IN,can2ReadData.recvDataPtr);
    return errCode;
}

//读取探液板斜率检测值
LH_ERR Can2SubNeedleReagentLiquidDetectReadSlopeValue(uint16_t* paramReadPtr)
{
    CAN2_SEND_CMD can2SendCmd;
    CAN2_READ_DATA can2ReadData;
    LH_ERR errCode = LH_ERR_NONE;

    can2SendCmd.targetBoardID = NEEDLE_REAGENT_BOARD_ID;
    can2SendCmd.channelNo = CAN2_CHANNEL_SERIAL1;

    //申请内存
    can2SendCmd.paramBuffer = NULL;
    can2SendCmd.paramLength = 0;

    //指令码设定
    can2SendCmd.commandCode = CAN2_SUB_NEEDLE_LIQUID_DETECT_CMD_READ_CLOPE_VALUE;
    can2SendCmd.timeoutMs = 60000;

    //数据发送
    errCode = Can2SendCommandWhileReturn(&can2SendCmd,&can2ReadData);

    //发送完成,释放发送内存
    if(can2SendCmd.paramBuffer != NULL)
    {
        UserMemFree(SRAM_IN,can2SendCmd.paramBuffer);
    }

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
        return LH_ERR_LOW_SERIAL_DATA_NULL;
    }

    //接收到的数据不需要
    if(can2ReadData.recvDatLength != 4)//2 + 2
    {
        //释放内存
        UserMemFree(SRAM_IN,can2ReadData.recvDataPtr);
        return LH_ERR_LOW_SERIAL_DATA_LENGTH;
    }

    //读取数据
    if(CAN2_SUB_DATA_SERIAL_RS1 != Can2ProcUtilConvertArrayToUint16(can2ReadData.recvDataPtr,0))
    {
        //数据标志位错误
        UserMemFree(SRAM_IN,can2ReadData.recvDataPtr);
        return LH_ERR_LOW_SERIAL_DATA_FLAG;
    }

    //读取结果
    *paramReadPtr = Can2ProcUtilConvertArrayToUint16(can2ReadData.recvDataPtr,2);

    //释放内存
    UserMemFree(SRAM_IN,can2ReadData.recvDataPtr);
    return errCode;
}

//读取探液板斜率检测次数
LH_ERR Can2SubNeedleReagentLiquidDetectSlopeCount(uint16_t* paramReadPtr)
{
    CAN2_SEND_CMD can2SendCmd;
    CAN2_READ_DATA can2ReadData;
    LH_ERR errCode = LH_ERR_NONE;

    can2SendCmd.targetBoardID = NEEDLE_REAGENT_BOARD_ID;
    can2SendCmd.channelNo = CAN2_CHANNEL_SERIAL1;

    //申请内存
    can2SendCmd.paramBuffer = NULL;
    can2SendCmd.paramLength = 0;

    //指令码设定
    can2SendCmd.commandCode = CAN2_SUB_NEEDLE_LIQUID_DETECT_CMD_READ_SLOPE_COUNT;
    can2SendCmd.timeoutMs = 60000;

    //数据发送
    errCode = Can2SendCommandWhileReturn(&can2SendCmd,&can2ReadData);

    //发送完成,释放发送内存
    if(can2SendCmd.paramBuffer != NULL)
    {
        UserMemFree(SRAM_IN,can2SendCmd.paramBuffer);
    }

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
        return LH_ERR_LOW_SERIAL_DATA_NULL;
    }

    //接收到的数据不需要
    if(can2ReadData.recvDatLength != 4)//2 + 2
    {
        //释放内存
        UserMemFree(SRAM_IN,can2ReadData.recvDataPtr);
        return LH_ERR_LOW_SERIAL_DATA_LENGTH;
    }

    //读取数据
    if(CAN2_SUB_DATA_SERIAL_RS1 != Can2ProcUtilConvertArrayToUint16(can2ReadData.recvDataPtr,0))
    {
        //数据标志位错误
        UserMemFree(SRAM_IN,can2ReadData.recvDataPtr);
        return LH_ERR_LOW_SERIAL_DATA_FLAG;
    }

    //读取结果
    *paramReadPtr = Can2ProcUtilConvertArrayToUint16(can2ReadData.recvDataPtr,2);

    //释放内存
    UserMemFree(SRAM_IN,can2ReadData.recvDataPtr);
    return errCode;
}

//读取探液板信号保持时间
LH_ERR Can2SubNeedleReagentLiquidDetectSignalHoldTimeMs(uint16_t* paramReadPtr)
{
    CAN2_SEND_CMD can2SendCmd;
    CAN2_READ_DATA can2ReadData;
    LH_ERR errCode = LH_ERR_NONE;

    can2SendCmd.targetBoardID = NEEDLE_REAGENT_BOARD_ID;
    can2SendCmd.channelNo = CAN2_CHANNEL_SERIAL1;

    //申请内存
    can2SendCmd.paramBuffer = NULL;
    can2SendCmd.paramLength = 0;

    //指令码设定
    can2SendCmd.commandCode = CAN2_SUB_NEEDLE_LIQUID_DETECT_CMD_READ_SIGNAL_HOLD;
    can2SendCmd.timeoutMs = 60000;

    //数据发送
    errCode = Can2SendCommandWhileReturn(&can2SendCmd,&can2ReadData);

    //发送完成,释放发送内存
    if(can2SendCmd.paramBuffer != NULL)
    {
        UserMemFree(SRAM_IN,can2SendCmd.paramBuffer);
    }

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
        return LH_ERR_LOW_SERIAL_DATA_NULL;
    }

    //接收到的数据不需要
    if(can2ReadData.recvDatLength != 4)//2 + 2
    {
        //释放内存
        UserMemFree(SRAM_IN,can2ReadData.recvDataPtr);
        return LH_ERR_LOW_SERIAL_DATA_LENGTH;
    }

    //读取数据
    if(CAN2_SUB_DATA_SERIAL_RS1 != Can2ProcUtilConvertArrayToUint16(can2ReadData.recvDataPtr,0))
    {
        //数据标志位错误
        UserMemFree(SRAM_IN,can2ReadData.recvDataPtr);
        return LH_ERR_LOW_SERIAL_DATA_FLAG;
    }

    //读取结果
    *paramReadPtr = Can2ProcUtilConvertArrayToUint16(can2ReadData.recvDataPtr,2);

    //释放内存
    UserMemFree(SRAM_IN,can2ReadData.recvDataPtr);
    return errCode;
}

//读取探液板电子电阻值
LH_ERR Can2SubNeedleReagentLiquidDetectResValue(uint16_t* paramReadPtr)
{
    CAN2_SEND_CMD can2SendCmd;
    CAN2_READ_DATA can2ReadData;
    LH_ERR errCode = LH_ERR_NONE;

    can2SendCmd.targetBoardID = NEEDLE_REAGENT_BOARD_ID;
    can2SendCmd.channelNo = CAN2_CHANNEL_SERIAL1;

    //申请内存
    can2SendCmd.paramBuffer = NULL;
    can2SendCmd.paramLength = 0;

    //指令码设定
    can2SendCmd.commandCode = CAN2_SUB_NEEDLE_LIQUID_DETECT_CMD_READ_RES;
    can2SendCmd.timeoutMs = 60000;

    //数据发送
    errCode = Can2SendCommandWhileReturn(&can2SendCmd,&can2ReadData);

    //发送完成,释放发送内存
    if(can2SendCmd.paramBuffer != NULL)
    {
        UserMemFree(SRAM_IN,can2SendCmd.paramBuffer);
    }

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
        return LH_ERR_LOW_SERIAL_DATA_NULL;
    }

    //接收到的数据不需要
    if(can2ReadData.recvDatLength != 4)//2 + 2
    {
        //释放内存
        UserMemFree(SRAM_IN,can2ReadData.recvDataPtr);
        return LH_ERR_LOW_SERIAL_DATA_LENGTH;
    }

    //读取数据
    if(CAN2_SUB_DATA_SERIAL_RS1 != Can2ProcUtilConvertArrayToUint16(can2ReadData.recvDataPtr,0))
    {
        //数据标志位错误
        UserMemFree(SRAM_IN,can2ReadData.recvDataPtr);
        return LH_ERR_LOW_SERIAL_DATA_FLAG;
    }

    //读取结果
    *paramReadPtr = Can2ProcUtilConvertArrayToUint16(can2ReadData.recvDataPtr,2);

    //释放内存
    UserMemFree(SRAM_IN,can2ReadData.recvDataPtr);
    return errCode;
}

//写入电子电阻值
LH_ERR Can2SubNeedleReagentLiquidDetectWriteResValue(uint16_t paramWrite)
{
    CAN2_SEND_CMD can2SendCmd;
    CAN2_READ_DATA can2ReadData;
    LH_ERR errCode = LH_ERR_NONE;

    can2SendCmd.targetBoardID = NEEDLE_REAGENT_BOARD_ID;
    can2SendCmd.channelNo = CAN2_CHANNEL_SERIAL1;

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
    //时间写入
    Can2ProcUtilConvertUint16ToArray(can2SendCmd.paramBuffer,0,paramWrite);

    //指令码设定
    can2SendCmd.commandCode = CAN2_SUB_NEEDLE_LIQUID_DETECT_CMD_WRITE_RES;
    can2SendCmd.timeoutMs = 60000;

    //数据发送
    errCode = Can2SendCommandWhileReturn(&can2SendCmd,&can2ReadData);

    //发送完成,释放发送内存
    if(can2SendCmd.paramBuffer != NULL)
    {
        UserMemFree(SRAM_IN,can2SendCmd.paramBuffer);
    }

    //不接收数据,所以把接收内存也清除
    if(can2ReadData.recvDataPtr != NULL)
    {
        //释放内存
        UserMemFree(SRAM_IN,can2ReadData.recvDataPtr);
    }

    return errCode;
}

//执行探液板校准
LH_ERR Can2SubNeedleReagentLiquidDetectAdjust(void)
{
    CAN2_SEND_CMD can2SendCmd;
    CAN2_READ_DATA can2ReadData;
    LH_ERR errCode = LH_ERR_NONE;

    can2SendCmd.targetBoardID = NEEDLE_REAGENT_BOARD_ID;
    can2SendCmd.channelNo = CAN2_CHANNEL_SERIAL1;

    //申请内存
    can2SendCmd.paramBuffer = NULL;
    can2SendCmd.paramLength = 0;

    //指令码设定
    can2SendCmd.commandCode = CAN2_SUB_NEEDLE_LIQUID_DETECT_CMD_ADJUST;
    can2SendCmd.timeoutMs = 60000;

    //数据发送
    errCode = Can2SendCommandWhileReturn(&can2SendCmd,&can2ReadData);

    //发送完成,释放发送内存
    if(can2SendCmd.paramBuffer != NULL)
    {
        UserMemFree(SRAM_IN,can2SendCmd.paramBuffer);
    }

    //不接收数据,所以把接收内存也清除
    if(can2ReadData.recvDataPtr != NULL)
    {
        //释放内存
        UserMemFree(SRAM_IN,can2ReadData.recvDataPtr);
    }

    return errCode;
}

//打开探液功能
LH_ERR Can2SubNeedleReagentLiquidDetectEnableFunc(void)
{
    CAN2_SEND_CMD can2SendCmd;
    CAN2_READ_DATA can2ReadData;
    LH_ERR errCode = LH_ERR_NONE;

    can2SendCmd.targetBoardID = NEEDLE_REAGENT_BOARD_ID;
    can2SendCmd.channelNo = CAN2_CHANNEL_SERIAL1;

    //申请内存
    can2SendCmd.paramBuffer = NULL;
    can2SendCmd.paramLength = 0;

    //指令码设定
    can2SendCmd.commandCode = CAN2_SUB_NEEDLE_LIQUID_DETECT_CMD_ENABLE_FUNC;
    can2SendCmd.timeoutMs = 60000;

    //数据发送
    errCode = Can2SendCommandWhileReturn(&can2SendCmd,&can2ReadData);

    //发送完成,释放发送内存
    if(can2SendCmd.paramBuffer != NULL)
    {
        UserMemFree(SRAM_IN,can2SendCmd.paramBuffer);
    }

    //不接收数据,所以把接收内存也清除
    if(can2ReadData.recvDataPtr != NULL)
    {
        //释放内存
        UserMemFree(SRAM_IN,can2ReadData.recvDataPtr);
    }

    return errCode;
}

//关闭探液功能
LH_ERR Can2SubNeedleReagentLiquidDetectDisableFunc(void)
{
    CAN2_SEND_CMD can2SendCmd;
    CAN2_READ_DATA can2ReadData;
    LH_ERR errCode = LH_ERR_NONE;

    can2SendCmd.targetBoardID = NEEDLE_REAGENT_BOARD_ID;
    can2SendCmd.channelNo = CAN2_CHANNEL_SERIAL1;

    //申请内存
    can2SendCmd.paramBuffer = NULL;
    can2SendCmd.paramLength = 0;

    //指令码设定
    can2SendCmd.commandCode = CAN2_SUB_NEEDLE_LIQUID_DETECT_CMD_DISABLE_FUNC;
    can2SendCmd.timeoutMs = 60000;

    //数据发送
    errCode = Can2SendCommandWhileReturn(&can2SendCmd,&can2ReadData);

    //发送完成,释放发送内存
    if(can2SendCmd.paramBuffer != NULL)
    {
        UserMemFree(SRAM_IN,can2SendCmd.paramBuffer);
    }
    
    //不接收数据,所以把接收内存也清除
    if(can2ReadData.recvDataPtr != NULL)
    {
        //释放内存
        UserMemFree(SRAM_IN,can2ReadData.recvDataPtr);
    }

    return errCode;
}







