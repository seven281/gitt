#include "Can2SubPhotonCount.h"

//光子测量单次测量开始
LH_ERR Can2SubPhotonCountSingleMeasureStart(uint16_t nms)
{
    CAN2_SEND_CMD can2SendCmd;
    CAN2_READ_DATA can2ReadData;
    LH_ERR errCode = LH_ERR_NONE;

    can2SendCmd.targetBoardID = CAN2_SUB_BOARD3;
    can2SendCmd.channelNo = CAN2_CHANNEL_SERIAL1;//

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
    Can2ProcUtilConvertUint16ToArray(can2SendCmd.paramBuffer,0,nms);

    //指令码设定
    can2SendCmd.commandCode = 0x00000003;
    can2SendCmd.timeoutMs = 1000;

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

//获取光子单次测量的测量结果
LH_ERR Can2SubPhotonCountGetSingleMeasureResult(uint32_t* result,uint32_t timeOut)
{
    CAN2_SEND_CMD can2SendCmd;
    CAN2_READ_DATA can2ReadData;
    LH_ERR errCode = LH_ERR_NONE;

    can2SendCmd.targetBoardID = CAN2_SUB_BOARD3;
    can2SendCmd.channelNo = CAN2_CHANNEL_SERIAL1;

    //申请内存
    can2SendCmd.paramBuffer = NULL;
    can2SendCmd.paramLength = 0;
    //指令码设定
    can2SendCmd.commandCode = 0x00000004;
    can2SendCmd.timeoutMs = timeOut;
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
    //接收到的数据不需要
    if(can2ReadData.recvDatLength != 6)//2 + 4 
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
    *result = Can2ProcUtilConvertArrayToUint32(can2ReadData.recvDataPtr,2);
    
    //释放内存
    UserMemFree(SRAM_IN,can2ReadData.recvDataPtr);
    return errCode;
}

//光子测量多次测量开始
LH_ERR Can2SubPhotonCountMultiMeasureStartWhileAck(uint16_t nms, uint16_t count)
{
    CAN2_SEND_CMD can2SendCmd;
    LH_ERR errCode = LH_ERR_NONE;

    can2SendCmd.targetBoardID = CAN2_SUB_BOARD3;
    can2SendCmd.channelNo = CAN2_CHANNEL_SERIAL1;

    //申请内存
    can2SendCmd.paramBuffer = NULL;
    can2SendCmd.paramLength = 4;
    do
    {
        can2SendCmd.paramBuffer = UserMemMalloc(SRAM_IN,4);
        if(can2SendCmd.paramBuffer == NULL)
        {
            CoreDelayMinTick();
        }
    }while(can2SendCmd.paramBuffer == NULL);

    //时间写入
    Can2ProcUtilConvertUint16ToArray(can2SendCmd.paramBuffer,0,nms);
    Can2ProcUtilConvertUint16ToArray(can2SendCmd.paramBuffer,2,count);

    //指令码设定
    can2SendCmd.commandCode = 0x00000005;
    can2SendCmd.timeoutMs = 60000;

    //数据发送
    errCode = Can2SendCommandWhileAck(&can2SendCmd);
     //释放发送内存
    UserMemFree(SRAM_IN,can2SendCmd.paramBuffer);

    return errCode;
}

//光子测量多次测量开始并等待结束
LH_ERR Can2SubPhotonCountMultiMeasureStarWhileReturn(uint16_t nms, uint16_t count)
{
    CAN2_SEND_CMD can2SendCmd;
    CAN2_READ_DATA can2ReadData;
    LH_ERR errCode = LH_ERR_NONE;

    can2SendCmd.targetBoardID = CAN2_SUB_BOARD3;
    can2SendCmd.channelNo = CAN2_CHANNEL_SERIAL1;

    //申请内存
    can2SendCmd.paramBuffer = NULL;
    can2SendCmd.paramLength = 4;
    do
    {
        can2SendCmd.paramBuffer = UserMemMalloc(SRAM_IN,4);
        if(can2SendCmd.paramBuffer == NULL)
        {
            CoreDelayMinTick();
        }
    }while(can2SendCmd.paramBuffer == NULL);

    //时间写入
    Can2ProcUtilConvertUint16ToArray(can2SendCmd.paramBuffer,0,nms);
    Can2ProcUtilConvertUint16ToArray(can2SendCmd.paramBuffer,2,count);

    //指令码设定
    can2SendCmd.commandCode = 0x00000005;
    can2SendCmd.timeoutMs = 60000;

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

//等待光子测量结束
LH_ERR Can2SubPhotonCountWaitLastReturn(uint32_t timeOutMs)
{
    CAN2_READ_DATA can2ReadData;
    LH_ERR errCode = LH_ERR_NONE;

    can2ReadData.recvDataPtr = NULL;
    can2ReadData.recvDatLength = 0;
    errCode = Can2SendCommandWaitReturn(CAN2_SUB_BOARD2,CAN2_CHANNEL_SERIAL1,timeOutMs,&can2ReadData);
    //接收到的数据不需要
    if(can2ReadData.recvDataPtr != NULL)
    {
        //释放内存
        UserMemFree(SRAM_IN,can2ReadData.recvDataPtr);
    }

    return errCode;
}

//光子测量获取多次测量结果
LH_ERR Can2SubPhotonCountGetMultiMeasureResult(uint16_t exceptCount,uint16_t* resultCount,uint32_t** resultArrayPtr)
{
    CAN2_SEND_CMD can2SendCmd;
    CAN2_READ_DATA can2ReadData;
    LH_ERR errCode = LH_ERR_NONE;

    can2SendCmd.targetBoardID = CAN2_SUB_BOARD3;
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

    //读取数量写入
    Can2ProcUtilConvertUint16ToArray(can2SendCmd.paramBuffer,0,exceptCount);

    //指令码设定
    can2SendCmd.commandCode = 0x00000006;
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
    //接收到的数据不需要
    if(can2ReadData.recvDatLength < 4)
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

    //读取结果,也就是结果个数
    *resultCount = Can2ProcUtilConvertArrayToUint16(can2ReadData.recvDataPtr,2);
    //根据结果个数申请内存
    do
    {
        *resultArrayPtr = UserMemMalloc(SRAM_IN,(4*(*resultCount)));
        if(*resultArrayPtr == NULL)
        {
            CoreDelayMinTick();
        }
    }while(*resultArrayPtr == NULL);
    //将缓存数据拷贝出来
    for(uint16_t index = 0; index < (*resultCount);index++)
    {
        (*resultArrayPtr)[index] = Can2ProcUtilConvertArrayToUint32(can2ReadData.recvDataPtr,4+(index*4));
    }

    //释放内存
    UserMemFree(SRAM_IN,can2ReadData.recvDataPtr);
    return errCode;
}
