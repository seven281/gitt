#ifndef __MCU_UART3_H_
#define __MCU_UART3_H_
#include "CoreInc.h"
#include "MCU_UartCommon.h"

//发送数据的串口缓冲区的长度
#define LENGTH_UART3_BUFFER_WITH_SEND       512

//使用DMA单块发送数据,单块的长度
#define LENGTH_UART3_BUFFER_WITH_BLOCK      128

//串口用于printf的缓冲区的长度
#define LENGTH_UART3_BUFFER_WITH_FORMAT     128

//串口初始化
void MCU_Uart3Init(uint32_t baud, MCU_UART_LENGTH length, MCU_UART_STOPBIT stopBit,
                      MCU_UART_CHECK_MODE checkMode, MCU_UART_HARD_CONTROL hardWareControl, MCU_UartRecvIntProcFunc rxCallBack);

//串口发送数组
void MCU_Uart3SendBuffer(uint8_t* bufferStartPtr,uint16_t sendLength);

//串口发送字符串
void MCU_Uart3SendString(uint8_t* stringStartPtr);

//串口发送字符串,带格式化
int MCU_Uart3Printf(const char *format, ...);

#endif




