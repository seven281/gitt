#ifndef __MCU_UART_COMMON_H_
#define __MCU_UART_COMMON_H_
#include "stm32f4xx.h"
#include "string.h"
#include "stdarg.h"
#include "stdio.h"

/*********************************串口相关枚举定义**************************************/
//串口接收中断处理函数
typedef void (*MCU_UartRecvIntProcFunc)(uint8_t dat);

//字长
typedef enum MCU_UART_LENGTH
{
    MCU_UART_LENGTH8 = USART_WordLength_8b,
    MCU_UART_LENGTH9 = USART_WordLength_9b,
} MCU_UART_LENGTH;

//停止位
typedef enum MCU_UART_STOPBIT
{
    MCU_UART_STOPBIT0_5 = USART_StopBits_0_5,
    MCU_UART_STOPBIT1 = USART_StopBits_1,
    MCU_UART_STOPBIT1_5 = USART_StopBits_1_5,
    MCU_UART_STOPBIT2 = USART_StopBits_2,
} MCU_UART_STOPBIT;

//校验位
typedef enum MCU_UART_CHECK_MODE
{
    MCU_UART_CHECK_MODE_NONE = USART_Parity_No,
    MCU_UART_CHECK_MODE_EVEN = USART_Parity_Even,
    MCU_UART_CHECK_MODE_ODD = USART_Parity_Odd,
} MCU_UART_CHECK_MODE;

//硬件流控制
typedef enum MCU_UART_HARD_CONTROL
{
    MCU_UART_HARD_CONTROL_NONE = USART_HardwareFlowControl_None,
    MCU_UART_HARD_CONTROL_RTS = USART_HardwareFlowControl_RTS,
    MCU_UART_HARD_CONTROL_CTS = USART_HardwareFlowControl_CTS,
    MCU_UART_HARD_CONTROL_RTS_CTS = USART_HardwareFlowControl_RTS_CTS,
} MCU_UART_HARD_CONTROL;
/********************************串口相关枚举定义***************************************/

//变参宏定义,调试printf
#define SystemPrintf(format,args...)            MCU_Uart1Printf(format,##args)	

//动作调试口
#define ActionPrintf(format,args...)            MCU_Uart3Printf(format,##args)

#endif




