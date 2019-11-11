#include "Can1ProcUtil.h"

//CAN1解析使用CCM内存

//创建ID,短包数据
uint32_t Can1ProcCreateShortID(uint8_t srcModuleNo,uint8_t srcBoardAddr,uint8_t targetModuleNo,uint8_t targetBoardAddr,uint16_t command)
{
    uint32_t idValue = 0;
    //源模块号
    srcModuleNo &= 0x07;
    //目标模块号
    targetModuleNo &= 0x07;
    //本地板号
    srcBoardAddr &= 0x3f;
    //目的板号
    targetBoardAddr &= 0x3f;
    //指令码
    command &= 0x07ff;
    //计算ID代码
    idValue += srcModuleNo;
    idValue <<= 3;
    idValue += targetModuleNo;
    idValue <<= 6;
    idValue += srcBoardAddr;
    idValue <<= 6;
    idValue += targetBoardAddr;
    idValue <<= 11;
    idValue += command;

    return idValue;
}

//创建ID,长包,起始包
uint32_t Can1ProcCreateLongStartID(uint8_t srcModuleNo,uint8_t srcBoardAddr,uint8_t targetModuleNo,uint8_t targetBoardAddr)
{
    uint32_t idValue = 0;
    //源模块号
    srcModuleNo &= 0x07;
    //目标模块号
    targetModuleNo &= 0x07;
    //本地板号
    srcBoardAddr &= 0x3f;
    //目的板号
    targetBoardAddr &= 0x3f;
    //计算ID代码
    idValue += srcModuleNo;
    idValue <<= 3;
    idValue += targetModuleNo;
    idValue <<= 6;
    idValue += srcBoardAddr;
    idValue <<= 6;
    idValue += targetBoardAddr;
    idValue <<= 11;
    idValue += CAN1_COMMAND_LONG_PACK_START;

    return idValue;
}

//创建ID,长包,数据包
uint32_t Can1ProcCreateLongDataID(uint8_t srcModuleNo,uint8_t srcBoardAddr,uint8_t targetModuleNo,uint8_t targetBoardAddr)
{
    uint32_t idValue = 0;
    //源模块号
    srcModuleNo &= 0x07;
    //目标模块号
    targetModuleNo &= 0x07;
    //本地板号
    srcBoardAddr &= 0x3f;
    //目的板号
    targetBoardAddr &= 0x3f;
    //计算ID代码
    idValue += srcModuleNo;
    idValue <<= 3;
    idValue += targetModuleNo;
    idValue <<= 6;
    idValue += srcBoardAddr;
    idValue <<= 6;
    idValue += targetBoardAddr;
    idValue <<= 11;
    idValue += CAN1_COMMAND_LONG_PACK_DATA;

    return idValue;
}

//创建ID,长包,结果包
uint32_t Can1ProcCreateLongEndID(uint8_t srcModuleNo,uint8_t srcBoardAddr,uint8_t targetModuleNo,uint8_t targetBoardAddr)
{
    uint32_t idValue = 0;
    //源模块号
    srcModuleNo &= 0x07;
    //目标模块号
    targetModuleNo &= 0x07;
    //本地板号
    srcBoardAddr &= 0x3f;
    //目的板号
    targetBoardAddr &= 0x3f;
    //计算ID代码
    idValue += srcModuleNo;
    idValue <<= 3;
    idValue += targetModuleNo;
    idValue <<= 6;
    idValue += srcBoardAddr;
    idValue <<= 6;
    idValue += targetBoardAddr;
    idValue <<= 11;
    idValue += CAN1_COMMAND_LONG_PACK_END;

    return idValue;
}

//将无符号32位数转换为buffer
void Can1ProcUtilConvertUint32ToArray(uint8_t* bufferPtr,uint16_t startPos,uint32_t srcDat)
{
    //低位在前,高位在后
    uint32_t useData = srcDat;
    bufferPtr[startPos + 0] = (uint8_t)useData;
    useData >>= 8;
    bufferPtr[startPos + 1] = (uint8_t)useData;
    useData >>= 8;
    bufferPtr[startPos + 2] = (uint8_t)useData;
    useData >>= 8;
    bufferPtr[startPos + 3] = (uint8_t)useData;
}

//将有符号32位数转换为buffer
void Can1ProcUtilConvertInt32ToArray(uint8_t* bufferPtr,uint16_t startPos,int32_t srcDat)
{
    //低位在前,高位在后
    uint32_t useData = *((uint32_t*)(&srcDat));
    bufferPtr[startPos + 0] = (uint8_t)useData;
    useData >>= 8;
    bufferPtr[startPos + 1] = (uint8_t)useData;
    useData >>= 8;
    bufferPtr[startPos + 2] = (uint8_t)useData;
    useData >>= 8;
    bufferPtr[startPos + 3] = (uint8_t)useData;
}

//将有符号32位数转换为buffer
void Can1ProcUtilConvertInt32ToArrayForParam(uint8_t* bufferPtr,uint16_t startPos,int32_t srcDat)
{
    //低位在前,高位在后
    uint32_t useData = *((uint32_t*)(&srcDat));
    bufferPtr[startPos + 3] = (uint8_t)useData;
    useData >>= 8;
    bufferPtr[startPos + 2] = (uint8_t)useData;
    useData >>= 8;
    bufferPtr[startPos + 1] = (uint8_t)useData;
    useData >>= 8;
    bufferPtr[startPos + 0] = (uint8_t)useData;
}

//将无符号16位数转换为buffer
void Can1ProcUtilConvertUint16ToArray(uint8_t* bufferPtr,uint16_t startPos,uint16_t srcDat)
{
    //低位在前,高位在后
    uint16_t useData = srcDat;
    bufferPtr[startPos + 0] = (uint8_t)useData;
    useData >>= 8;
    bufferPtr[startPos + 1] = (uint8_t)useData;
}

//将有符号16位数转换为buffer
void Can1ProcUtilConvertInt16ToArray(uint8_t* bufferPtr,uint16_t startPos,int16_t srcDat)
{
    //低位在前,高位在后
    uint16_t useData = *((uint16_t*)(&srcDat));
    bufferPtr[startPos + 0] = (uint8_t)useData;
    useData >>= 8;
    bufferPtr[startPos + 1] = (uint8_t)useData;
}

//将buffer转换为无符号32位数
uint32_t Can1ProcUtilConvertArrayToUint32(uint8_t* bufferPtr,uint16_t startPos)
{
    uint32_t destUint32 = 0;
    destUint32 += bufferPtr[3 + startPos];
    destUint32 <<= 8;
    destUint32 += bufferPtr[2 + startPos];
    destUint32 <<= 8;
    destUint32 += bufferPtr[1 + startPos];
    destUint32 <<= 8;
    destUint32 += bufferPtr[0 + startPos];

    return destUint32;
}

//将buffer转换为有符号32位数
int32_t Can1ProcUtilConvertArrayToInt32(uint8_t* bufferPtr,uint16_t startPos)
{
    uint32_t destUint32 = 0;
    int32_t destInt32 = 0;
    destUint32 += bufferPtr[3 + startPos];
    destUint32 <<= 8;
    destUint32 += bufferPtr[2 + startPos];
    destUint32 <<= 8;
    destUint32 += bufferPtr[1 + startPos];
    destUint32 <<= 8;
    destUint32 += bufferPtr[0 + startPos];

    destInt32 = *((int32_t*)(&destUint32));
    return destInt32;
}

int32_t Can1ProcUtilConvertArrayToInt32ForParam(uint8_t* bufferPtr,uint16_t startPos)
{
    uint32_t destUint32 = 0;
    int32_t destInt32 = 0;
    destUint32 += bufferPtr[0 + startPos];
    destUint32 <<= 8;
    destUint32 += bufferPtr[1 + startPos];
    destUint32 <<= 8;
    destUint32 += bufferPtr[2 + startPos];
    destUint32 <<= 8;
    destUint32 += bufferPtr[3 + startPos];

    destInt32 = *((int32_t*)(&destUint32));
    return destInt32;
}

//将buffer转换为无符号16位数
uint16_t Can1ProcUtilConvertArrayToUint16(uint8_t* bufferPtr,uint16_t startPos)
{
    uint16_t destUint16 = 0;
    destUint16 += bufferPtr[1 + startPos];
    destUint16 <<= 8;
    destUint16 += bufferPtr[0 + startPos];
    
    return destUint16;
}

//将buffer转换为有符号16位数
int16_t Can1ProcUtilConvertArrayToInt16(uint8_t* bufferPtr,uint16_t startPos)
{
    uint16_t destUint16 = 0;
    int16_t destInt16 = 0;
    destUint16 += bufferPtr[1 + startPos];
    destUint16 <<= 8;
    destUint16 += bufferPtr[0 + startPos];

    destInt16 = *((int16_t*)(&destUint16));
    return destInt16;
}











