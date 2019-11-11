#include "Can2ProcMain.h"
#include "MCU_Can.h"

//整个CAN2的缓存
static CAN2_RECV_CACHE can2RecvCache;

//数据长度缓存
static uint8_t currentCan2RecvDataLength;

//数据缓存
static uint8_t currentCan2RecvDataBuffer[8] = {0};

//临时解析ID
static CAN2_RECV_ID_PROC_RESULT can2ID_AnalysisResult;

//CAN2接收协议初始化
void Can2ProcDataInit(void)
{
    OS_ERR err;
    int32_t boardIndex = 0;
    int32_t channelIndex = 0;
    CAN2_RECV_CHANNEL_UNIT* channelUnitPtr = NULL;
    //初始化通道信息
    for(boardIndex = 0; boardIndex < SYSTEM_CAN2_BOARD_MAX; boardIndex++)
    {
        for(channelIndex = 0; channelIndex < SYSTEM_CAN2_BOARD_CHANNEL_MAX; channelIndex++)
        {
            //获取通道状态指针
            channelUnitPtr = &(can2RecvCache.boardCacheArray[boardIndex].channelCacheArray[channelIndex]);
            //初始化互斥信号量
            OSMutexCreate((OS_MUTEX*	)&(channelUnitPtr->mutexCan2Receive),
                        (CPU_CHAR*	)"mutexCan2Receive",
                        (OS_ERR*		)&err);	
            //初始化通道状态
            channelUnitPtr->channelState = CAN2_RECV_CMD_STATE_IDLE;
            //指针帧号
            channelUnitPtr->cmdFrameCode = 1;
            //指令时间
            channelUnitPtr->commandSendOutStartMs = 0;
            //指令发送重发计数
            channelUnitPtr->commandSendRetryCount = 0;
            //ACK代码
            channelUnitPtr->ackCode = 0;
            //结果代码
            channelUnitPtr->errCode = 0;
            //数据接收缓存区
            channelUnitPtr->dataRecvUnitPtr = NULL;
        }
    }
}

//将接受的数据解析称为协议码
static void Can2RecvAnalsisID(uint32_t canID,CAN2_RECV_ID_PROC_RESULT* protocolID)
{
    protocolID->targetBoardAddr = (uint8_t)(canID & 0x003f);
    protocolID->senderBoardAddr = (uint8_t)((canID >> 6) & 0x003f);
    protocolID->channelNumber = (uint8_t)((canID >> 12) & 0x007f);
    protocolID->packType = (CAN2_PACK_TYPE)((canID >> 19) & 0x0007);
    protocolID->packLenFlag = (CAN2_PACK_LEN_FLAG)((canID >> 22) & 0x0001);
    protocolID->singleCode = (CAN2_SINGLE_CODE)((canID >> 23) & 0x0003);
    protocolID->frameCode = (uint8_t)((canID >> 25) & 0x000f);
}

//ACK包处理
static void Can2RecvProcWithAckPack(CAN2_RECV_CHANNEL_UNIT* channelUnit)
{
    uint32_t ackCode = 0;
    //查看状态
    if(channelUnit->channelState == CAN2_RECV_CMD_STATE_WAIT_ACK)
    {
        //在等待ACK环境下,如果ACK代码为0,等待返回,否则,ACK错误
        ackCode = Can2ProcUtilConvertArrayToUint32(currentCan2RecvDataBuffer,0);
        //保存ACK代码
        channelUnit->ackCode = ackCode;
        if(ackCode == 0)
        {
            channelUnit->channelState = CAN2_RECV_CMD_STATE_WAIT_RETURN;
        }
        else
        {
            channelUnit->channelState = CAN2_RECV_CMD_STATE_ERR_ACK;
        }
    }
    else
    {
        //其他状态下,不管
        return;
    }
}

//结果包处理
static void Can2RecvProcWithResultPack(CAN2_RECV_CHANNEL_UNIT* channelUnit)
{
    uint32_t errCode;
    if(channelUnit->channelState == CAN2_RECV_CMD_STATE_WAIT_RETURN)
    {
        errCode = Can2ProcUtilConvertArrayToUint32(currentCan2RecvDataBuffer,0);
        //保存ACK代码
        channelUnit->errCode = errCode;
        //指令结束
        channelUnit->channelState = CAN2_RECV_CMD_STATE_COMPLETE;
    }
    else
    {
        //其他状态下,不管
    }
}

//数据短包
static void Can2RecvProcWithDataPackShort(CAN2_RECV_CHANNEL_UNIT* channelUnit)
{
    if(channelUnit->dataRecvUnitPtr->currentDataState == CAN2_RECV_DATA_STATE_INIT)
    {
        UserMemCopy(channelUnit->dataRecvUnitPtr->dataBuffer,currentCan2RecvDataBuffer,
                        currentCan2RecvDataLength);
        channelUnit->dataRecvUnitPtr->exceptRecvPackCount = 1;
        channelUnit->dataRecvUnitPtr->exceptRecvDataLength = currentCan2RecvDataLength;
        channelUnit->dataRecvUnitPtr->currentRecvPackCount = 1;
        channelUnit->dataRecvUnitPtr->currentRecvDataLength = currentCan2RecvDataLength;
        channelUnit->dataRecvUnitPtr->currentDataState = CAN2_RECV_DATA_STATE_END;
    }
    else
    {
        return;
    }
}

//数据长包起始帧
static void Can2RecvProcWithDataPackLongStart(CAN2_RECV_CHANNEL_UNIT* channelUnit)
{
    uint16_t exceptLength = Can2ProcUtilConvertArrayToUint16(currentCan2RecvDataBuffer,0);
    uint8_t exceptPack = currentCan2RecvDataBuffer[2];
    if(channelUnit->dataRecvUnitPtr->currentDataState == CAN2_RECV_DATA_STATE_INIT)
    {
        channelUnit->dataRecvUnitPtr->exceptRecvPackCount = exceptPack;
        channelUnit->dataRecvUnitPtr->exceptRecvDataLength = exceptLength;
        channelUnit->dataRecvUnitPtr->currentRecvPackCount = 0;
        channelUnit->dataRecvUnitPtr->currentRecvDataLength = 0;
        //转换到接收到起始数据包
        channelUnit->dataRecvUnitPtr->currentDataState = CAN2_RECV_DATA_STATE_START;
    }
    else
    {
        return;
    }
}

//数据长包过程帧
static void Can2RecvProcWithDataPackLongData(CAN2_RECV_CHANNEL_UNIT* channelUnit)
{
    //接收起始包或者接收数据包状态
    if((channelUnit->dataRecvUnitPtr->currentDataState == CAN2_RECV_DATA_STATE_START)||
        (channelUnit->dataRecvUnitPtr->currentDataState == CAN2_RECV_DATA_STATE_DATA))
    {
        if(currentCan2RecvDataBuffer[0] > channelUnit->dataRecvUnitPtr->exceptRecvPackCount)
        {
            //数据包溢出
            channelUnit->dataRecvUnitPtr->currentDataState = CAN2_RECV_DATA_STATE_ERR_PACK_OVERFLOW;
        }
        //当前数据包和接收数据包匹配
        if(currentCan2RecvDataBuffer[0] == channelUnit->dataRecvUnitPtr->currentRecvPackCount)
        {
            //数据拷贝
            UserMemCopy(channelUnit->dataRecvUnitPtr->dataBuffer+channelUnit->dataRecvUnitPtr->currentRecvDataLength,
                        currentCan2RecvDataBuffer+1,
                        currentCan2RecvDataLength-1);
            //计数值增加
            channelUnit->dataRecvUnitPtr->currentRecvPackCount++;
            channelUnit->dataRecvUnitPtr->currentRecvDataLength += (currentCan2RecvDataLength-1);
            //转换成等待数据
            if((channelUnit->dataRecvUnitPtr->currentRecvPackCount == channelUnit->dataRecvUnitPtr->exceptRecvPackCount)&&
                (channelUnit->dataRecvUnitPtr->currentRecvDataLength == channelUnit->dataRecvUnitPtr->exceptRecvDataLength))
            {
                channelUnit->dataRecvUnitPtr->currentDataState = CAN2_RECV_DATA_STATE_WAIT_END;
            }
        }
        else if(currentCan2RecvDataBuffer[0] > channelUnit->dataRecvUnitPtr->currentRecvPackCount)
        {
            //数据丢帧
            channelUnit->dataRecvUnitPtr->currentDataState = CAN2_RECV_DATA_STATE_ERR_PACK_LOSS;
        }
        
    }
    else
    {
        return;
    }
    
}

//数据长包结果帧
static void Can2RecvProcWithDataPackLongEnd(CAN2_RECV_CHANNEL_UNIT* channelUnit)
{
    if(channelUnit->dataRecvUnitPtr->currentDataState == CAN2_RECV_DATA_STATE_WAIT_END)
    {
        //等待结果的时候收到结果包
        channelUnit->dataRecvUnitPtr->currentDataState = CAN2_RECV_DATA_STATE_END;
    }
    else
    {
        channelUnit->dataRecvUnitPtr->currentDataState = CAN2_RECV_DATA_STATE_ERR_PACK_LOSS;
    }
}

//CAN2接收处理一个数据单元
void Can2RecvProc(MCU_CAN_RECEIVE_BUFFER_UNIT* rxUnit)
{
    //ID解析
    Can2RecvAnalsisID(rxUnit->rxMsg.ExtId,&can2ID_AnalysisResult);
    //然后对数据进行拷贝
    currentCan2RecvDataLength = rxUnit->rxMsg.DLC;
    for(uint8_t i = 0; i < currentCan2RecvDataLength; i++)
    {
        currentCan2RecvDataBuffer[i] = rxUnit->rxMsg.Data[i];
    }
    //处理完成,清除等待处理标志,给中断快速处理的机会
    rxUnit->waitProcessFlag = 0;
    //查看板号和通道号是否支持
    if(can2ID_AnalysisResult.targetBoardAddr != DEVICE_MAIN_BOARD_ID)
    {
        return;
    }
    //通道号超出
    if(can2ID_AnalysisResult.channelNumber >= SYSTEM_CAN2_BOARD_CHANNEL_MAX)
    {
        return;
    }
    //板号超出,因为板号从1开始,所以是大于不是大于等于
    if(can2ID_AnalysisResult.senderBoardAddr > SYSTEM_CAN2_BOARD_MAX)
    {
        return;
    }
    //通道缓存指针,记住,板号从1开始,与缓存数组错了一位
    CAN2_RECV_CHANNEL_UNIT* channelUnit = 
        &(can2RecvCache.boardCacheArray[can2ID_AnalysisResult.senderBoardAddr -1]
        .channelCacheArray[can2ID_AnalysisResult.channelNumber]);
    //分为不同的状态处理
    if(can2ID_AnalysisResult.packType == CAN2_PACK_ACK)
    {
        //ACK包
        Can2RecvProcWithAckPack(channelUnit);
    }
    else if(can2ID_AnalysisResult.packType == CAN2_PACK_RESULT)
    {
        //Result包
        Can2RecvProcWithResultPack(channelUnit);
    }
    else if(can2ID_AnalysisResult.packType == CAN2_PACK_DATA)
    {
        if(channelUnit->dataRecvUnitPtr == NULL)
        {
            //接收缓存数据指针为空
            return;
        }
        if(can2ID_AnalysisResult.singleCode == CAN2_SINGLE_CODE_SHORT)
        {
            //短包数据包
            Can2RecvProcWithDataPackShort(channelUnit);
        }
        else if(can2ID_AnalysisResult.singleCode == CAN2_SINGLE_CODE_LONG_START)
        {
            //长包起始帧
            Can2RecvProcWithDataPackLongStart(channelUnit);
        }
        else if(can2ID_AnalysisResult.singleCode == CAN2_SINGLE_CODE_LONG_DATA)
        {
            //长包数据帧
            Can2RecvProcWithDataPackLongData(channelUnit);
        }
        else if(can2ID_AnalysisResult.singleCode == CAN2_SINGLE_CODE_LONG_END)
        {
            //长包结果帧
            Can2RecvProcWithDataPackLongEnd(channelUnit);
        }
    }
    else if(can2ID_AnalysisResult.packType == CAN2_PACK_AUTO_UPDATE)
    {
        //主动上报
        //主动上报先不管
    }
    else if(can2ID_AnalysisResult.packType == CAN2_PACK_CMD)
    {
        //指令帧
        //指令帧不管
    }
}


//通过板号和通道号获取一个通道缓存指针
static CAN2_RECV_CHANNEL_UNIT* Can2ProcUtilGetChannelCacheUnitPtr(uint8_t targetBoardAddr,uint8_t channelNo,LH_ERR* errCode)
{
    OS_ERR err;
    if(targetBoardAddr > SYSTEM_CAN2_BOARD_MAX)
    {
        *errCode = LH_ERR_CAN2_BOARD;
        return NULL;
    }
    if(channelNo >= SYSTEM_CAN2_BOARD_CHANNEL_MAX)
    {
        *errCode = LH_ERR_CAN2_CHANNEL;
        return NULL;
    }
    //找到通道缓存
    CAN2_RECV_CHANNEL_UNIT* channelUniPtr = &(can2RecvCache.boardCacheArray[targetBoardAddr -1].channelCacheArray[channelNo]);
    //申请互斥信号量,防止多任务抢占
    OSMutexPend (&(channelUniPtr->mutexCan2Receive),60000,OS_OPT_PEND_BLOCKING,0,&err);
    //无错误
    if(err == OS_ERR_NONE)
    {
        *errCode = LH_ERR_NONE;
        return channelUniPtr;
    }
    else if(err == OS_ERR_MUTEX_OWNER)
    {
        *errCode = LH_ERR_CAN2_MUTEX_OWNER;
        return channelUniPtr;
    }
    else
    {
        //获取不到互斥信号量
        *errCode = LH_ERR_CAN2_MUTEX_BUSY;
        return NULL;
    }
}

//检查指令结构体
static LH_ERR Can2CheckCommandStruct(CAN2_SEND_CMD* sendCommand)
{
    if(sendCommand == NULL)
    {
        return LH_ERR_CAN2_CMD_NULL;
    }
    if(sendCommand->timeoutMs == 0)
    {
        return LH_ERR_CAN2_CMD_TIMEOUT_ZERO;
    }
    if (sendCommand->paramLength >= SYSTEM_CAN2_SINGLE_RECV_DATA_MAX)
    {
        return LH_ERR_CAN2_CMD_LENGTH_MAX;
    }
    return LH_ERR_NONE;
}

//数据缓存初始化
static void Can2ResetDataChannel(CAN2_RECV_DATA_UNIT* dataUnitPtr)
{
    dataUnitPtr->currentDataState = CAN2_RECV_DATA_STATE_INIT;
    dataUnitPtr->exceptRecvPackCount = 0;
    dataUnitPtr->exceptRecvDataLength = 0;
    dataUnitPtr->currentRecvPackCount = 0;
    dataUnitPtr->currentRecvDataLength = 0;
    UserMemSet(dataUnitPtr->dataBuffer,0,SYSTEM_CAN2_SINGLE_RECV_DATA_MAX);
}

//指令缓存初始化
static void Can2ResetCommandChannel(CAN2_RECV_CHANNEL_UNIT* channelUnitPtr)
{
    //状态
    channelUnitPtr->channelState = CAN2_RECV_CMD_STATE_IDLE;
    //帧号
    channelUnitPtr->cmdFrameCode++;
    //帧号1-15
    if(channelUnitPtr->cmdFrameCode > 15)
    {
        channelUnitPtr->cmdFrameCode = 1;
    }
    channelUnitPtr->commandSendOutStartMs = 0;
    channelUnitPtr->commandSendRetryCount = 0;
    channelUnitPtr->ackCode = 0;
    channelUnitPtr->errCode = 0;
    //如果有没释放的内存,先释放
    if(channelUnitPtr->dataRecvUnitPtr != NULL)
    {
        UserMemFree(SRAM_IN,channelUnitPtr->dataRecvUnitPtr);
    }
    //然后再申请内存
    channelUnitPtr->dataRecvUnitPtr = NULL;
    do
    {
        channelUnitPtr->dataRecvUnitPtr = UserMemMalloc(SRAM_IN,sizeof(CAN2_RECV_DATA_UNIT));
        if(channelUnitPtr->dataRecvUnitPtr == NULL)
        {
            CoreDelayMinTick();
        }
    }while(channelUnitPtr->dataRecvUnitPtr == NULL);
    //初始化数据缓存
    Can2ResetDataChannel(channelUnitPtr->dataRecvUnitPtr);
}


//指令缓存为ACK重发的初始化
void Can2ResetCommandChannelWithAckRetry(CAN2_RECV_CHANNEL_UNIT* channelUnitPtr)
{
    //状态
    channelUnitPtr->channelState = CAN2_RECV_CMD_STATE_IDLE;
    //帧号
    channelUnitPtr->cmdFrameCode++;
    //帧号1-15
    if(channelUnitPtr->cmdFrameCode > 15)
    {
        channelUnitPtr->cmdFrameCode = 1;
    }
    channelUnitPtr->commandSendOutStartMs = 0;
    channelUnitPtr->ackCode = 0;
    channelUnitPtr->errCode = 0;
    //检查是否有接收缓存,没有的话,申请一下
    if(channelUnitPtr->dataRecvUnitPtr == NULL)
    {
        do
        {
            channelUnitPtr->dataRecvUnitPtr = UserMemMalloc(SRAM_IN,sizeof(CAN2_RECV_DATA_UNIT));
            if(channelUnitPtr->dataRecvUnitPtr == NULL)
            {
                CoreDelayMinTick();
            }
        }while(channelUnitPtr->dataRecvUnitPtr == NULL);
    }
    //初始化数据缓存
    Can2ResetDataChannel(channelUnitPtr->dataRecvUnitPtr);
}

//短包数据发送
static void Can2SendCommandWithShortPack(CAN2_SEND_CMD* sendCommand,uint8_t frameCode)
{
    CanTxMsg txMsg;
    //创建ID
    uint32_t sendCommandId = Can2ProcUtilCreatePackID(sendCommand->targetBoardID,sendCommand->channelNo,frameCode,CAN2_PACK_CMD,
                                    CAN2_PACK_LEN_SHORT,CAN2_SINGLE_CODE_SHORT);
    //拷贝数据
    txMsg.DLC = 4 + sendCommand->paramLength;
    //帧ID
    txMsg.ExtId = sendCommandId;
    //扩展帧
    txMsg.IDE = CAN_ID_EXT;
    //数据帧
    txMsg.RTR = CAN_RTR_DATA;
    //首先将指令转换为4字节数组
    Can2ProcUtilConvertUint32ToArray(txMsg.Data,0,sendCommand->commandCode);
    //然后将参数拷贝
    UserMemCopy(txMsg.Data+4,sendCommand->paramBuffer,sendCommand->paramLength);
    //发送数据
    MCU_CAN2_TransMessage(&txMsg);
}

//长包起始包
static void Can2SendCommandWithLongPackStart(CAN2_SEND_CMD* sendCommand,uint8_t frameCode)
{
    CanTxMsg txMsg;
    //创建ID
    uint32_t sendCommandId = Can2ProcUtilCreatePackID(sendCommand->targetBoardID,sendCommand->channelNo,frameCode,CAN2_PACK_CMD,
                                    CAN2_PACK_LEN_LONG,CAN2_SINGLE_CODE_LONG_START);
    //拷贝数据
    txMsg.DLC = 3;
    //帧ID
    txMsg.ExtId = sendCommandId;
    //扩展帧
    txMsg.IDE = CAN_ID_EXT;
    //数据帧
    txMsg.RTR = CAN_RTR_DATA;
    //将数据长度转换到数组
    Can2ProcUtilConvertUint16ToArray(txMsg.Data,0,sendCommand->paramLength + 4);
    //将包数转换到数组
    uint8_t packCount = 0;
    //计算包数
    packCount = (sendCommand->paramLength + 4)/7;
    if((sendCommand->paramLength + 4)%7 != 0)
    {
        //多一点,多一包
        packCount += 1;
    }
    //包数写入
    txMsg.Data[2] = packCount;
    //发送数据
    MCU_CAN2_TransMessage(&txMsg);
}

//长包数据包
static void Can2SendCommandWithLongPackData(CAN2_SEND_CMD* sendCommand,uint8_t frameCode)
{
    uint8_t packCount = 0;
    CanTxMsg txMsg;
    //创建ID
    uint32_t sendCommandId = Can2ProcUtilCreatePackID(sendCommand->targetBoardID,sendCommand->channelNo,frameCode,CAN2_PACK_CMD,
                                    CAN2_PACK_LEN_LONG,CAN2_SINGLE_CODE_LONG_DATA);
    //帧ID
    txMsg.ExtId = sendCommandId;
    //扩展帧
    txMsg.IDE = CAN_ID_EXT;
    //数据帧
    txMsg.RTR = CAN_RTR_DATA;
    //计算包数
    packCount = (sendCommand->paramLength + 4)/7;
    if((sendCommand->paramLength + 4)%7 != 0)
    {
        //多一点,多一包
        packCount += 1;
    }
    uint8_t currentPackIndex = 0;
    //循环发数据包
    for(currentPackIndex = 0; currentPackIndex < packCount; currentPackIndex++)
    {
        if(currentPackIndex == 0)
        {
            //第一包,要带指令
            txMsg.DLC = 8;
            txMsg.Data[0] = currentPackIndex;
            //首先将指令转换为4字节数组
            Can2ProcUtilConvertUint32ToArray(txMsg.Data,1,sendCommand->commandCode);
            //然后将参数拷贝
            UserMemCopy(txMsg.Data+5,sendCommand->paramBuffer,3);
            //发送数据
            MCU_CAN2_TransMessage(&txMsg);
        }
        else
        {
            //不是第一包
            txMsg.Data[0] = currentPackIndex;
            txMsg.DLC = ((currentPackIndex != (packCount-1))?8:(1+((sendCommand->paramLength + 4)%7)));
            //数据拷贝
            UserMemCopy(txMsg.Data+1,sendCommand->paramBuffer + (currentPackIndex*7) - 4,txMsg.DLC -1);
            //发送数据
            MCU_CAN2_TransMessage(&txMsg);
        }
    } 
}
//长包结果包
static void Can2SendCommandWithLongPackEnd(CAN2_SEND_CMD* sendCommand,uint8_t frameCode)
{
    CanTxMsg txMsg;
    //创建ID
    uint32_t sendCommandId = Can2ProcUtilCreatePackID(sendCommand->targetBoardID,sendCommand->channelNo,frameCode,CAN2_PACK_CMD,
                                    CAN2_PACK_LEN_LONG,CAN2_SINGLE_CODE_LONG_END);
    //拷贝数据
    txMsg.DLC = 0;
    //帧ID
    txMsg.ExtId = sendCommandId;
    //扩展帧
    txMsg.IDE = CAN_ID_EXT;
    //数据帧
    txMsg.RTR = CAN_RTR_DATA;
    //发送数据
    MCU_CAN2_TransMessage(&txMsg);
}

//长包数据发送
void Can2SendCommandWithLongPack(CAN2_SEND_CMD* sendCommand,uint8_t frameCode)
{
    Can2SendCommandWithLongPackStart(sendCommand,frameCode);
    Can2SendCommandWithLongPackData(sendCommand,frameCode);
    Can2SendCommandWithLongPackEnd(sendCommand,frameCode);
}




//CAN2发送指令,直到收到ACK,注意,此处因为是异步的,所以会有一个数据等待释放的锁
//也就是说,,调用了这个函数,必须调用wairreturn
LH_ERR Can2SendCommandWhileAck(CAN2_SEND_CMD* sendCommand)
{
    LH_ERR errorCode = LH_ERR_NONE;
    OS_ERR osErr;
    //检查指令结构体
    errorCode = Can2CheckCommandStruct(sendCommand);
    if(LH_ERR_NONE != errorCode)
    {
        return errorCode;
    }
    //通道缓存
    CAN2_RECV_CHANNEL_UNIT* channelUnitPtr = NULL;
    //获取一下通道缓存
    channelUnitPtr = Can2ProcUtilGetChannelCacheUnitPtr(sendCommand->targetBoardID,sendCommand->channelNo,&errorCode);
    //如果没有获取到,那失败
    if(channelUnitPtr == NULL)
    {
        //通道缓存获取失败
        return errorCode;
    }
    //如果获取到了,但是是因为上次没释放,那么需要释放一次
    if(errorCode == LH_ERR_CAN2_MUTEX_OWNER)
    {
        OSMutexPost(&(channelUnitPtr->mutexCan2Receive),OS_OPT_POST_NO_SCHED,&osErr);
        //释放嵌套信号量不对
        if((osErr != OS_ERR_NONE)&&(osErr != OS_ERR_MUTEX_NESTING))
        {
            return errorCode;
        }
    }
    //复位通道缓存
    Can2ResetCommandChannel(channelUnitPtr);
    do
    {
        if(channelUnitPtr->commandSendRetryCount > SYSTEM_CAN2_ACK_RETRY_MAX)
        {
            //大于重发次数最大值,跳出循环
            break;
        }
        if(channelUnitPtr->commandSendRetryCount > 0)
        {
            //如果重发次数大于等于1,就说明进入重发,需要将状态初始化为ACK重发模式
            Can2ResetCommandChannelWithAckRetry(channelUnitPtr);
        }
        //发送数据
        if(sendCommand->paramLength > 4)
        {
            //长包发送
            Can2SendCommandWithLongPack(sendCommand,channelUnitPtr->cmdFrameCode);
        }
        else
        {
            //短包发送
            Can2SendCommandWithShortPack(sendCommand,channelUnitPtr->cmdFrameCode);
        }
        //发送完成,设置状态是等待ACK
        channelUnitPtr->channelState = CAN2_RECV_CMD_STATE_WAIT_ACK;
        //设定当前时间
        channelUnitPtr->commandSendOutStartMs = OSTimeGet(&osErr);
        //设定当前发送次数
        channelUnitPtr->commandSendRetryCount++;
        //延时等待,超时重发
        do
        {
            //1.等待ACK状态变化
            if(channelUnitPtr->channelState != CAN2_RECV_CMD_STATE_WAIT_ACK)
            {
                //状态发生变化
                break;
            }
            //2 延时等待 延时等待放在后面,因为如果在前面,ACK响应可能会遇到1毫秒的硬延时
            CoreDelayMinTick();
        }while(OSTimeGet(&osErr) - channelUnitPtr->commandSendOutStartMs <= SYSTEM_CAN2_ACK_TIMEOUT);

        //查看是为什么退出
        if(channelUnitPtr->channelState != CAN2_RECV_CMD_STATE_WAIT_ACK)
        {
            //状态发生变化
            break;
        }
        //循环重发
    }while(1);

    //运行到此处,检查当前原因
    if(channelUnitPtr->commandSendRetryCount > SYSTEM_CAN2_ACK_RETRY_MAX)
    {
        //重发次数超出,释放内存并返回
        if(channelUnitPtr->dataRecvUnitPtr != NULL)
        {
            UserMemFree(SRAM_IN,channelUnitPtr->dataRecvUnitPtr);
            channelUnitPtr->dataRecvUnitPtr = NULL;
        }
        //释放互斥信号量
        OSMutexPost(&(channelUnitPtr->mutexCan2Receive),OS_OPT_POST_FIFO,&osErr);
        return LH_ERR_CAN2_ACK_RETRY_OUT;
    }
    if(channelUnitPtr->channelState == CAN2_RECV_CMD_STATE_ERR_ACK)
    {
        //接收到错误的ACK代码,释放内存并退出
        if(channelUnitPtr->dataRecvUnitPtr != NULL)
        {
            UserMemFree(SRAM_IN,channelUnitPtr->dataRecvUnitPtr);
            channelUnitPtr->dataRecvUnitPtr = NULL;
        }
        //释放互斥信号量
        OSMutexPost(&(channelUnitPtr->mutexCan2Receive),OS_OPT_POST_FIFO,&osErr);
        return LH_ERR_CAN2_ACK_ERR;
    }

    //返回正常完成,不释放内存,同时,互斥信号量也不释放
    return LH_ERR_NONE;
}

//等待指定板卡的指定通道的数据返回
LH_ERR Can2SendCommandWaitReturn(uint8_t boardID,CAN2_CHANNEL channelNo,uint32_t timeOutMs,
                                        CAN2_READ_DATA* readData)
{
    LH_ERR errorCode = LH_ERR_NONE;
    OS_ERR osErr;
    //获取通道的缓存指针
    CAN2_RECV_CHANNEL_UNIT* channelUnitPtr = NULL;
    //获取一下通道缓存
    channelUnitPtr = Can2ProcUtilGetChannelCacheUnitPtr(boardID,channelNo,&errorCode);
    if(channelUnitPtr == NULL)
    {
        //通道缓存获取失败
        return errorCode;
    }
    //如果错误代码显示已经拥有互斥信号量,那么释放一下
    if(errorCode == LH_ERR_CAN2_MUTEX_OWNER)
    {
        OSMutexPost(&(channelUnitPtr->mutexCan2Receive),OS_OPT_POST_NO_SCHED,&osErr);
        //释放嵌套信号量不对
        if((osErr != OS_ERR_NONE)&&(osErr != OS_ERR_MUTEX_NESTING))
        {
            return errorCode;
        }
    }
    else
    {
        OSMutexPost(&(channelUnitPtr->mutexCan2Receive),OS_OPT_POST_FIFO,&osErr);
        //如果并没有拥有,那么此时等待return是不对的,不该本线程去等待,而是其他线程去等待
        return LH_ERR_CAN2_WAIT_RETURN_LOGIC_ERR;
    }

    //没有逻辑故障,那就等待指令返回
    do
    {
        //状态不在是等待返回
        if(channelUnitPtr->channelState != CAN2_RECV_CMD_STATE_WAIT_RETURN)
        {
            break;
        }
        //延时等待,要放在前面,防止为了等待return虚耗1毫秒的延时
        CoreDelayMinTick();
    }while(OSTimeGet(&osErr) - channelUnitPtr->commandSendOutStartMs <= timeOutMs);

    //指令超时
    if(OSTimeGet(&osErr) - channelUnitPtr->commandSendOutStartMs > timeOutMs)
    {
        //释放内存
        if(channelUnitPtr->dataRecvUnitPtr != NULL)
        {
            UserMemFree(SRAM_IN,channelUnitPtr->dataRecvUnitPtr);
            channelUnitPtr->dataRecvUnitPtr = NULL;
        }
        //状态设置为超时,调试时候可以看见
        channelUnitPtr->channelState = CAN2_RECV_CMD_STATE_ERR_RETURN_TIMEOUT;
        //释放互斥信号量
        OSMutexPost(&(channelUnitPtr->mutexCan2Receive),OS_OPT_POST_FIFO,&osErr);
        //返回超时
        return LH_ERR_CAN2_RETURN_TIME_OUT;
    }
    if(channelUnitPtr->channelState != CAN2_RECV_CMD_STATE_COMPLETE)
    {
        //运行到此处,不知道出了什么稀奇古怪的问题,反正就报错
        //释放内存
        if(channelUnitPtr->dataRecvUnitPtr != NULL)
        {
            UserMemFree(SRAM_IN,channelUnitPtr->dataRecvUnitPtr);
            channelUnitPtr->dataRecvUnitPtr = NULL;
        }
        channelUnitPtr->channelState = CAN2_RECV_CMD_STATE_IDLE;
        //释放互斥信号量
        OSMutexPost(&(channelUnitPtr->mutexCan2Receive),OS_OPT_POST_FIFO,&osErr);
        //返回超时
        return LH_ERR_CAN2_WAIT_RETURN_UNKNOW;
    }
    //指令执行完成,先拷贝数据包
    if(channelUnitPtr->dataRecvUnitPtr->currentDataState == CAN2_RECV_DATA_STATE_END)
    {
        //接收到完成数据包
        readData->recvDatLength = channelUnitPtr->dataRecvUnitPtr->currentRecvDataLength;

        //申请内存
        readData->recvDataPtr = NULL;
        do
        {
            readData->recvDataPtr = UserMemMalloc(SRAM_IN,readData->recvDatLength);
            if(readData->recvDataPtr == NULL)
            {
                CoreDelayMinTick();
            }
        }while(readData->recvDataPtr == NULL);

        //拷贝数据
        UserMemCopy(readData->recvDataPtr,channelUnitPtr->dataRecvUnitPtr->dataBuffer,readData->recvDatLength);
    }
    else
    {
        readData->recvDatLength = 0;
        readData->recvDataPtr = NULL;
    }
    //释放内存
    if(channelUnitPtr->dataRecvUnitPtr != NULL)
    {
        UserMemFree(SRAM_IN,channelUnitPtr->dataRecvUnitPtr);
        channelUnitPtr->dataRecvUnitPtr = NULL;
    }

    //此处,指令返回完成,要等待下位机相应的信道清空状态,不能立即离开
    //否则极有可能下一个相同的信道指令过来,下位机相关信道的状态还没清除,下位机就报错了
    CoreDelayMinTick();
    //释放互斥信号量
    OSMutexPost(&(channelUnitPtr->mutexCan2Receive),OS_OPT_POST_FIFO,&osErr);
    return (LH_ERR)(channelUnitPtr->errCode);
}

//CAN2发送指令,直到收到Return返回
LH_ERR Can2SendCommandWhileReturn(CAN2_SEND_CMD* sendCommand,CAN2_READ_DATA* readData)
{
    LH_ERR errorCode = LH_ERR_NONE;
    OS_ERR osErr;

    //先把接收到的数据和状态清空
    readData->recvDataPtr = NULL;
    readData->recvDatLength = 0;
    
    //检查指令结构体
    errorCode = Can2CheckCommandStruct(sendCommand);
    if(LH_ERR_NONE != errorCode)
    {
        return errorCode;
    }
    //通道缓存
    CAN2_RECV_CHANNEL_UNIT* channelUnitPtr = NULL;
    //获取一下通道缓存
    channelUnitPtr = Can2ProcUtilGetChannelCacheUnitPtr(sendCommand->targetBoardID,sendCommand->channelNo,&errorCode);
    //如果没有获取到,那失败
    if(channelUnitPtr == NULL)
    {
        //通道缓存获取失败
        return errorCode;
    }
    //如果获取到了,但是是因为上次没释放,说明有一次发出了等待ACK却没有等待完成,那么报错
    if(errorCode == LH_ERR_CAN2_MUTEX_OWNER)
    {
        return LH_ERR_CAN2_MUTEX_LAST_ACK_WAIT;
    }
    //复位通道缓存
    Can2ResetCommandChannel(channelUnitPtr);
    do
    {
        if(channelUnitPtr->commandSendRetryCount > SYSTEM_CAN2_ACK_RETRY_MAX)
        {
            //大于重发次数最大值,跳出循环
            break;
        }
        if(channelUnitPtr->commandSendRetryCount > 0)
        {
            //如果重发次数大于等于1,就说明进入重发,需要将状态初始化为ACK重发模式
            Can2ResetCommandChannelWithAckRetry(channelUnitPtr);
        }
        //发送数据
        if(sendCommand->paramLength > 4)
        {
            //长包发送
            Can2SendCommandWithLongPack(sendCommand,channelUnitPtr->cmdFrameCode);
        }
        else
        {
            //短包发送
            Can2SendCommandWithShortPack(sendCommand,channelUnitPtr->cmdFrameCode);
        }
        //发送完成,设置状态是等待ACK
        channelUnitPtr->channelState = CAN2_RECV_CMD_STATE_WAIT_ACK;
        //设定当前时间
        channelUnitPtr->commandSendOutStartMs = OSTimeGet(&osErr);
        //设定当前发送次数
        channelUnitPtr->commandSendRetryCount++;
        //延时等待,超时重发
        do
        {
            //1.等待ACK状态变化
            if(channelUnitPtr->channelState != CAN2_RECV_CMD_STATE_WAIT_ACK)
            {
                //状态发生变化
                break;
            }
            //2 延时等待 延时等待放在后面,因为如果在前面,ACK响应可能会遇到1毫秒的硬延时
            CoreDelayMinTick();
        }while(OSTimeGet(&osErr) - channelUnitPtr->commandSendOutStartMs <= SYSTEM_CAN2_ACK_TIMEOUT);

        //查看是为什么退出
        if(channelUnitPtr->channelState != CAN2_RECV_CMD_STATE_WAIT_ACK)
        {
            //状态发生变化
            break;
        }
        //循环重发
    }while(1);

    //运行到此处,检查当前原因
    if(channelUnitPtr->commandSendRetryCount > SYSTEM_CAN2_ACK_RETRY_MAX)
    {
        //重发次数超出,释放内存并返回
        if(channelUnitPtr->dataRecvUnitPtr != NULL)
        {
            UserMemFree(SRAM_IN,channelUnitPtr->dataRecvUnitPtr);
            channelUnitPtr->dataRecvUnitPtr = NULL;
        }
        //释放互斥信号量
        OSMutexPost(&(channelUnitPtr->mutexCan2Receive),OS_OPT_POST_FIFO,&osErr);
        return LH_ERR_CAN2_ACK_RETRY_OUT;
    }
    if(channelUnitPtr->channelState == CAN2_RECV_CMD_STATE_ERR_ACK)
    {
        //接收到错误的ACK代码,释放内存并退出
        if(channelUnitPtr->dataRecvUnitPtr != NULL)
        {
            UserMemFree(SRAM_IN,channelUnitPtr->dataRecvUnitPtr);
            channelUnitPtr->dataRecvUnitPtr = NULL;
        }
        //释放互斥信号量
        OSMutexPost(&(channelUnitPtr->mutexCan2Receive),OS_OPT_POST_FIFO,&osErr);
        return LH_ERR_CAN2_ACK_ERR;
    }

    //到这里,收到了ACK,开始等待结果数据包
    do
    {
        //状态不在是等待返回
        if(channelUnitPtr->channelState != CAN2_RECV_CMD_STATE_WAIT_RETURN)
        {
            break;
        }
        //延时等待,要放在前面,防止为了等待return虚耗1毫秒的延时
        CoreDelayMinTick();
    }while(OSTimeGet(&osErr) - channelUnitPtr->commandSendOutStartMs <= sendCommand->timeoutMs);

    //指令超时
    if(OSTimeGet(&osErr) - channelUnitPtr->commandSendOutStartMs > sendCommand->timeoutMs)
    {
        //释放内存
        if(channelUnitPtr->dataRecvUnitPtr != NULL)
        {
            UserMemFree(SRAM_IN,channelUnitPtr->dataRecvUnitPtr);
            channelUnitPtr->dataRecvUnitPtr = NULL;
        }
        //状态设置为超时,调试时候可以看见
        channelUnitPtr->channelState = CAN2_RECV_CMD_STATE_ERR_RETURN_TIMEOUT;
        //释放互斥信号量
        OSMutexPost(&(channelUnitPtr->mutexCan2Receive),OS_OPT_POST_FIFO,&osErr);
        //返回超时
        return LH_ERR_CAN2_RETURN_TIME_OUT;
    }

    //运行到此处,不知道出了什么稀奇古怪的问题,反正就报错
    if(channelUnitPtr->channelState != CAN2_RECV_CMD_STATE_COMPLETE)
    {
        
        //释放内存
        if(channelUnitPtr->dataRecvUnitPtr != NULL)
        {
            UserMemFree(SRAM_IN,channelUnitPtr->dataRecvUnitPtr);
            channelUnitPtr->dataRecvUnitPtr = NULL;
        }
        channelUnitPtr->channelState = CAN2_RECV_CMD_STATE_IDLE;
        //释放互斥信号量
        OSMutexPost(&(channelUnitPtr->mutexCan2Receive),OS_OPT_POST_FIFO,&osErr);
        //返回超时
        return LH_ERR_CAN2_WAIT_RETURN_UNKNOW;
    }

    //指令执行完成,先拷贝数据包
    if(channelUnitPtr->dataRecvUnitPtr->currentDataState == CAN2_RECV_DATA_STATE_END)
    {
        //接收到完成数据包
        readData->recvDatLength = channelUnitPtr->dataRecvUnitPtr->currentRecvDataLength;

        //申请内存
        readData->recvDataPtr = NULL;
        do
        {
            readData->recvDataPtr = UserMemMalloc(SRAM_IN,readData->recvDatLength);
            if(readData->recvDataPtr == NULL)
            {
                CoreDelayMinTick();
            }
        }while(readData->recvDataPtr == NULL);

        //拷贝数据
        UserMemCopy(readData->recvDataPtr,channelUnitPtr->dataRecvUnitPtr->dataBuffer,readData->recvDatLength);
    }
    else
    {
        readData->recvDatLength = 0;
        readData->recvDataPtr = NULL;
    }
    //释放内存,无论结果如何
    if(channelUnitPtr->dataRecvUnitPtr != NULL)
    {
        UserMemFree(SRAM_IN,channelUnitPtr->dataRecvUnitPtr);
        channelUnitPtr->dataRecvUnitPtr = NULL;
    }

    //此处,指令返回完成,要等待下位机相应的信道清空状态,不能立即离开
    //否则极有可能下一个相同的信道指令过来,下位机相关信道的状态还没清除,下位机就报错了
    CoreDelayMinTick();
    //释放互斥信号量
    OSMutexPost(&(channelUnitPtr->mutexCan2Receive),OS_OPT_POST_FIFO,&osErr);
    return (LH_ERR)(channelUnitPtr->errCode);
}   














