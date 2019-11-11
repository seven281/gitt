#include "Can2SubSerial.h"

//写指令初始化
void Can2SubSerialWriteCmdDataStructInit(CAN2_SERIAL_WRITE_CMD* writeCmdPtr)
{
    writeCmdPtr->serialIndex = CAN2_SUB_BOARD1_SERIAL;
    writeCmdPtr->bufferLength = 0;
    writeCmdPtr->bufferPtr = NULL;
}

//写并延时读指令初始化
void Can2SubSerialWriteDelayReadCmdDataStructInit(CAN2_SERIAL_WRITE_DELAY_READ_CMD* writeDelayReadCmdPtr)
{
    writeDelayReadCmdPtr->serialIndex = CAN2_SUB_BOARD1_SERIAL;
    writeDelayReadCmdPtr->bufferLength = 0;
    writeDelayReadCmdPtr->bufferPtr = NULL;
    writeDelayReadCmdPtr->waitDelayTimeMs = 1000;
    writeDelayReadCmdPtr->bufferReceiveLength = 0;
    writeDelayReadCmdPtr->bufferReceivePtr = NULL;
}

//读指令初始化
void Can2SubSerialReadCmdDataStructInit(CAN2_SERIAL_READ_CMD* readCmdPtr)
{
    readCmdPtr->serialIndex = CAN2_SUB_BOARD1_SERIAL;
    readCmdPtr->bufferReceiveLength = 0;
    readCmdPtr->bufferReceivePtr = NULL;
}

//通过序号计算板号和通道号码
static LH_ERR Can2SubSerialCalcBoardChannel(CAN2_SUB_SERIAL serialIndex,CAN2_SUB_BOARD* boardIndex,
                                                CAN2_CHANNEL* channelIndex)
{
    if(serialIndex > CAN2_SUB_BOARD10_SERIAL)
    {
        return LH_ERR_LOW_SERIAL_INDEX;
    }
    *channelIndex = CAN2_CHANNEL_SERIAL1;
    if(serialIndex == CAN2_SUB_BOARD1_SERIAL)
    {
        *boardIndex = CAN2_SUB_BOARD1;
    }
    else if(serialIndex == CAN2_SUB_BOARD2_SERIAL)
    {
        *boardIndex = CAN2_SUB_BOARD2;
    }
    else if(serialIndex == CAN2_SUB_BOARD3_SERIAL)
    {
        *boardIndex = CAN2_SUB_BOARD3;
    }
    else if(serialIndex == CAN2_SUB_BOARD4_SERIAL)
    {
        *boardIndex = CAN2_SUB_BOARD4;
    }
    else if(serialIndex == CAN2_SUB_BOARD5_SERIAL)
    {
        *boardIndex = CAN2_SUB_BOARD5;
    }
    else if(serialIndex == CAN2_SUB_BOARD6_SERIAL)
    {
        *boardIndex = CAN2_SUB_BOARD6;
    }
    else if(serialIndex == CAN2_SUB_BOARD7_SERIAL)
    {
        *boardIndex = CAN2_SUB_BOARD7;
    }
    else if(serialIndex == CAN2_SUB_BOARD8_SERIAL)
    {
        *boardIndex = CAN2_SUB_BOARD8;
    }
    else if(serialIndex == CAN2_SUB_BOARD9_SERIAL)
    {
        *boardIndex = CAN2_SUB_BOARD9;
    }
    else if(serialIndex == CAN2_SUB_BOARD10_SERIAL)
    {
        *boardIndex = CAN2_SUB_BOARD10;
    }
    return LH_ERR_NONE;
}

//下位机串口写入
LH_ERR Can2SubSerialWriteBuffer(CAN2_SERIAL_WRITE_CMD* writeCmdPtr)
{
    CAN2_SEND_CMD can2SendCmd;
    CAN2_READ_DATA can2ReadData;
    LH_ERR errCode = LH_ERR_NONE;
    //计算板卡号码和单元号码
    errCode = Can2SubSerialCalcBoardChannel(writeCmdPtr->serialIndex,&(can2SendCmd.targetBoardID),
                                &(can2SendCmd.channelNo));
    if(errCode != LH_ERR_NONE)
    {
        return errCode;
    }
    //检查数据长度
    if(writeCmdPtr->bufferLength == 0)
    {
        return LH_ERR_LOW_SERIAL_SEND_LENGTH_ZERO;
    }
    //检查数据指针
    if(writeCmdPtr->bufferPtr == NULL)
    {
        return LH_ERR_LOW_SERIAL_SEND_PTR_NULL;
    }
    //申请内存
    can2SendCmd.paramBuffer = NULL;
    can2SendCmd.paramLength = 2+writeCmdPtr->bufferLength;
    do
    {
        can2SendCmd.paramBuffer = UserMemMalloc(SRAM_IN,sizeof(can2SendCmd.paramLength));
        if(can2SendCmd.paramBuffer == NULL)
        {
            CoreDelayMinTick();
        }
    }while(can2SendCmd.paramBuffer == NULL);
    //内存数据拷贝
    Can2ProcUtilConvertUint16ToArray(can2SendCmd.paramBuffer,0,writeCmdPtr->bufferLength);
    //拷贝数据
    UserMemCopy(can2SendCmd.paramBuffer+2,writeCmdPtr->bufferPtr,writeCmdPtr->bufferLength);
    //指令码设定
    can2SendCmd.commandCode = CAN2_SERIAL_CMD_WRITE;
    can2SendCmd.timeoutMs = 100;
    //数据发送
    errCode = Can2SendCommandWhileReturn(&can2SendCmd,&can2ReadData);
    //释放发送内存
    UserMemFree(SRAM_IN,can2SendCmd.paramBuffer);
    //不接收数据,所以把接收内存也清除
    if(can2ReadData.recvDataPtr != NULL)
    {
        //释放内存
        UserMemFree(SRAM_IN,can2ReadData.recvDataPtr);
    }
    return errCode;
}

//下位机串口写入并读取
LH_ERR Can2SubSerialWriteDelayReadBuffer(CAN2_SERIAL_WRITE_DELAY_READ_CMD* writeDelayReadCmdPtr)
{
    CAN2_SEND_CMD can2SendCmd;
    CAN2_READ_DATA can2ReadData;
    LH_ERR errCode = LH_ERR_NONE;
    //计算板卡号码和单元号码
    errCode = Can2SubSerialCalcBoardChannel(writeDelayReadCmdPtr->serialIndex,&(can2SendCmd.targetBoardID),
                                &(can2SendCmd.channelNo));
    if(errCode != LH_ERR_NONE)
    {
        return errCode;
    }
    //检查数据长度
    if(writeDelayReadCmdPtr->bufferLength == 0)
    {
        return LH_ERR_LOW_SERIAL_SEND_LENGTH_ZERO;
    }
    //检查数据指针
    if(writeDelayReadCmdPtr->bufferPtr == NULL)
    {
        return LH_ERR_LOW_SERIAL_SEND_PTR_NULL;
    }
    //申请内存
    can2SendCmd.paramBuffer = NULL;
    can2SendCmd.paramLength = 6 + writeDelayReadCmdPtr->bufferLength;
    do
    {
        can2SendCmd.paramBuffer = UserMemMalloc(SRAM_IN,sizeof(can2SendCmd.paramLength));
        if(can2SendCmd.paramBuffer == NULL)
        {
            CoreDelayMinTick();
        }
    }while(can2SendCmd.paramBuffer == NULL);
    //内存数据拷贝,写入长度
    Can2ProcUtilConvertUint16ToArray(can2SendCmd.paramBuffer,0,writeDelayReadCmdPtr->bufferLength);
    //返回延时
    Can2ProcUtilConvertUint32ToArray(can2SendCmd.paramBuffer,2,writeDelayReadCmdPtr->waitDelayTimeMs);
    //拷贝数据
    UserMemCopy(can2SendCmd.paramBuffer+6,writeDelayReadCmdPtr->bufferPtr,writeDelayReadCmdPtr->bufferLength);

    can2SendCmd.commandCode = CAN2_SERIAL_CMD_WRITE_READ;
    can2SendCmd.timeoutMs = writeDelayReadCmdPtr->waitDelayTimeMs + 1000;

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
        return errCode;
    }

    //数据为空
    if(can2ReadData.recvDataPtr == NULL)
    {
        UserMemFree(SRAM_IN,can2SendCmd.paramBuffer);
        return LH_ERR_LOW_SERIAL_DATA_NULL;
    }
    //数据长度不足
    if(can2ReadData.recvDatLength < 3)
    {
        //释放发送内存
        UserMemFree(SRAM_IN,can2SendCmd.paramBuffer);
        //释放接收内存
        UserMemFree(SRAM_IN,can2ReadData.recvDataPtr);
        //释放内存
        return LH_ERR_LOW_SERIAL_DATA_LENGTH;
    }
    //读取数据
    if(CAN2_SUB_DATA_SERIAL_RS1 != Can2ProcUtilConvertArrayToUint16(can2ReadData.recvDataPtr,0))
    {
        //释放发送内存
        UserMemFree(SRAM_IN,can2SendCmd.paramBuffer);
        //释放接收内存
        UserMemFree(SRAM_IN,can2ReadData.recvDataPtr);
        //数据标志位错误
        return LH_ERR_LOW_SERIAL_DATA_FLAG;
    }
    //申请内存并拷贝数据
    writeDelayReadCmdPtr->bufferReceivePtr = NULL;
    do
    {
        writeDelayReadCmdPtr->bufferReceivePtr = UserMemMalloc(SRAM_IN,can2ReadData.recvDatLength -2);
        if(writeDelayReadCmdPtr->bufferReceivePtr == NULL)
        {
            CoreDelayMinTick();
        }
    }while(writeDelayReadCmdPtr->bufferReceivePtr == NULL);
    //拷贝数据
    UserMemCopy(writeDelayReadCmdPtr->bufferReceivePtr,can2ReadData.recvDataPtr+2,can2ReadData.recvDatLength -2);
    writeDelayReadCmdPtr->bufferReceiveLength = can2ReadData.recvDatLength - 2;
    //释放发送内存
    UserMemFree(SRAM_IN,can2SendCmd.paramBuffer);
    //释放接收内存
    UserMemFree(SRAM_IN,can2ReadData.recvDataPtr);
    return errCode;
}

//下位机串口读取
LH_ERR Can2SubSerialReadBuffer(CAN2_SERIAL_READ_CMD* readCmdPtr)
{
    CAN2_SEND_CMD can2SendCmd;
    CAN2_READ_DATA can2ReadData;
    LH_ERR errCode = LH_ERR_NONE;
    //计算板卡号码和单元号码
    errCode = Can2SubSerialCalcBoardChannel(readCmdPtr->serialIndex,&(can2SendCmd.targetBoardID),
                                &(can2SendCmd.channelNo));
    if(errCode != LH_ERR_NONE)
    {
        return errCode;
    }
    //申请内存
    can2SendCmd.paramBuffer = NULL;
    can2SendCmd.paramLength = 0;
    can2SendCmd.commandCode = CAN2_SERIAL_CMD_READ;
    can2SendCmd.timeoutMs = 1000;

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
        return LH_ERR_LOW_SERIAL_DATA_NULL;
    }
    //数据长度不足
    if(can2ReadData.recvDatLength < 3)
    {
        //释放接收内存
        UserMemFree(SRAM_IN,can2ReadData.recvDataPtr);
        //释放内存
        return LH_ERR_LOW_SERIAL_DATA_LENGTH;
    }
    //读取数据
    if(CAN2_SUB_DATA_SERIAL_RS1 != Can2ProcUtilConvertArrayToUint16(can2ReadData.recvDataPtr,0))
    {
        //释放接收内存
        UserMemFree(SRAM_IN,can2ReadData.recvDataPtr);
        //数据标志位错误
        return LH_ERR_LOW_SERIAL_DATA_FLAG;
    }
    //申请内存并拷贝数据
    readCmdPtr->bufferReceivePtr = NULL;
    do
    {
        readCmdPtr->bufferReceivePtr = UserMemMalloc(SRAM_IN,can2ReadData.recvDatLength -2);
        if(readCmdPtr->bufferReceivePtr == NULL)
        {
            CoreDelayMinTick();
        }
    }while(readCmdPtr->bufferReceivePtr == NULL);
    //拷贝数据
    UserMemCopy(readCmdPtr->bufferReceivePtr,can2ReadData.recvDataPtr+2,can2ReadData.recvDatLength -2);
    readCmdPtr->bufferReceiveLength = can2ReadData.recvDatLength - 2;
    //释放接收内存
    UserMemFree(SRAM_IN,can2ReadData.recvDataPtr);
    return errCode;
}









