#ifndef __SYSTEM_UART_RS1_UTIL_H_
#define __SYSTEM_UART_RS1_UTIL_H_
#include "BoardInc.h"
#include "SystemCmd.h"

//计算CRC校验码
void SystemUartRS_ModbusCRC_CalcForProtocol(uint8_t* bufferPtr,uint8_t bufferLength,uint8_t* crcHighPtr,uint8_t* crcLowPtr);

//接收到的一帧数据进行校验,成功返回1,失败返回0
uint8_t SystemUartRS_ModbusCheckCRC(uint8_t* recvBuffer);

//创建一帧数据包
void SystemUartRS_ModbusCreatePackage(uint8_t* sendBuffer,uint8_t destAddr,uint8_t framsNo,uint8_t funcCode,
                               uint16_t regAddr,uint16_t regValue);

//用接收的数据创建一帧Modbus指令数据包
void SystemUartRS_ModbusCreateCommandPackage(uint8_t* recvBuffer,SYSTEM_CMD_UART_RS1* command);




#endif















