#ifndef __SYSTEM_CMD_H_
#define __SYSTEM_CMD_H_
#include "MCU_Inc.h"



#define CAN1_SINGLE_PACK_MAX_LENGTH     2048


//系统指令源
typedef enum SYSTEM_CMD_SRC
{
    SYSTEM_CMD_SRC_UART_RS1,
    SYSTEM_CMD_SRC_CAN1,
    SYSTEM_CMD_SRC_NET_W5500,
    SYSTEM_CMD_SRC_SELF,
}SYSTEM_CMD_SRC;

//系统指令数据
typedef struct SYSTEM_CMD
{
    //系统指令指针
    void* systemCommandDataPtr;
    //系统指令源表示
    SYSTEM_CMD_SRC commandSrc;
}SYSTEM_CMD;

//自身指令,使用外部SRAM
typedef struct SYSTEM_CMD_SELF
{
    uint16_t commandIndex;//需要调用的指令的序号
    int32_t commandParam1;//调用的指令的参数1,若没有,则不用管
    int32_t commandParam2;//调用的指令的参数2，试管架,试管杯
    int32_t commandParam3;//调用的指令的参数3，
    int32_t commandParam4;//调用的指令的参数4
    int32_t commandParam5;//调用的指令的参数5
    void* commandDataSpecialChannel;//指令传递的时候附带传递的数据,各个任务通道是不同的
}SYSTEM_CMD_SELF;

//CAN1口指令
typedef struct SYSTEM_CMD_CAN1
{
    uint8_t srcModuleNo;
    uint8_t srcBoardAddr;
    uint8_t targetModuleNo;
    uint8_t targetBoardAddr;
    uint16_t command;
    uint16_t recvDataLength;
    uint8_t can1RecvDataBuffer[CAN1_SINGLE_PACK_MAX_LENGTH];
}SYSTEM_CMD_CAN1;


//串口指令,使用CCRAM
typedef struct SYSTEM_CMD_UART_RS1
{
    uint8_t uartRecvCommandArray[12];
    uint8_t srcDeviceAddr;//源设备地址
    uint8_t frameNo;//帧号
    uint8_t funcCode;//方法代码
    uint16_t regAddr;//寄存器地址
    uint16_t regValue;//寄存器数据
}SYSTEM_CMD_UART_RS1;

//网口指令,使用外部SRAM
typedef struct SYSTEM_CMD_NET_W5500
{
    uint8_t netRecvBuffer[16];
}SYSTEM_CMD_NET_W5500;

#endif





