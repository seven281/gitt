#ifndef __CAN2_SUB_SERIAL_H_
#define __CAN2_SUB_SERIAL_H_
#include "Can2ProcMain.h"
#include "Can2SubSerialIndex.h"

//下位机串口写入指令
typedef struct CAN2_SERIAL_WRITE_CMD
{
    //选择的串口
    CAN2_SUB_SERIAL serialIndex;
    //写入数据长度
    uint16_t bufferLength;
    //写入数据的缓存
    uint8_t* bufferPtr;
}CAN2_SERIAL_WRITE_CMD;

//下位机串口写入并读取指令
typedef struct CAN2_SERIAL_WRITE_DELAY_READ_CMD
{
    //选择的串口
    CAN2_SUB_SERIAL serialIndex;
    uint16_t bufferLength;
    uint8_t* bufferPtr;
    uint32_t waitDelayTimeMs;
    uint16_t bufferReceiveLength;
    uint8_t* bufferReceivePtr;
}CAN2_SERIAL_WRITE_DELAY_READ_CMD;

//下位机串口读取指令
typedef struct CAN2_SERIAL_READ_CMD
{
    //选择的串口
    CAN2_SUB_SERIAL serialIndex;
    uint16_t bufferReceiveLength;
    uint8_t* bufferReceivePtr;
}CAN2_SERIAL_READ_CMD;

//串口通道宏
typedef enum CAN2_SERIAL_CMD
{
    CAN2_SERIAL_CMD_WRITE = 0X00000000,
    CAN2_SERIAL_CMD_WRITE_READ = 0X00000001,
    CAN2_SERIAL_CMD_READ = 0X00000002,
}CAN2_SERIAL_CMD;

//写指令初始化
void Can2SubSerialWriteCmdDataStructInit(CAN2_SERIAL_WRITE_CMD* writeCmdPtr);

//写并延时读指令初始化
void Can2SubSerialWriteDelayReadCmdDataStructInit(CAN2_SERIAL_WRITE_DELAY_READ_CMD* writeDelayReadCmdPtr);

//读指令初始化
void Can2SubSerialReadCmdDataStructInit(CAN2_SERIAL_READ_CMD* readCmdPtr);


//下位机串口写入
LH_ERR Can2SubSerialWriteBuffer(CAN2_SERIAL_WRITE_CMD* writeCmdPtr);

//下位机串口写入并读取
LH_ERR Can2SubSerialWriteDelayReadBuffer(CAN2_SERIAL_WRITE_DELAY_READ_CMD* writeDelayReadCmdPtr);

//下位机串口读取
LH_ERR Can2SubSerialReadBuffer(CAN2_SERIAL_READ_CMD* readCmdPtr);


#endif





