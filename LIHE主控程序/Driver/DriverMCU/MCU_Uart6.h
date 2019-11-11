#ifndef __MCU_UART6_H_
#define __MCU_UART6_H_
#include "CoreInc.h"
#include "MCU_UartCommon.h"

//发送数据的串口缓冲区的长度
#define LENGTH_UART6_BUFFER_WITH_SEND       512


//串口用于printf的缓冲区的长度
#define LENGTH_UART6_BUFFER_WITH_FORMAT     128

//串口初始化
void MCU_Uart6Init(uint32_t baud, MCU_UART_LENGTH length, MCU_UART_STOPBIT stopBit,
                      MCU_UART_CHECK_MODE checkMode, MCU_UART_HARD_CONTROL hardWareControl, MCU_UartRecvIntProcFunc rxCallBack);

//串口发送数组
void MCU_Uart6SendBuffer(uint8_t* bufferStartPtr,uint16_t sendLength);

//串口发送字符串
void MCU_Uart6SendString(uint8_t* stringStartPtr);

//串口发送字符串,带格式化
int MCU_Uart6Printf(const char *format, ...);

#endif




