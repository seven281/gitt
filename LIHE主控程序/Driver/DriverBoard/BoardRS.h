#ifndef __BOARD_RS_H_
#define __BOARD_RS_H_
#include "MCU_Inc.h"

//从上到下,串口顺序是
//BOARD_RS3_UART2
//BOARD_RS1_UART3
//BOARD_RS2_UART1
//BOARD_RS4_UART6

//主板串口
typedef enum BOARD_RS_SERIAL
{
    BOARD_RS1_UART3,//六针 带供电,动作调试
    BOARD_RS2_UART1,//四针 不带供电,流程调试
    BOARD_RS3_UART2,//六针,带供电,条码枪串口,
    BOARD_RS4_UART6,//四针,不带供电,温控端口
}BOARD_RS_SERIAL;

#define QR_BAR_SCAN_PORT    BOARD_RS3_UART2
#define ACTION_DEBUG_PORT   BOARD_RS1_UART3


//串口初始化
void BoardRS_Init(BOARD_RS_SERIAL serialNo,uint32_t baud, MCU_UART_LENGTH length, MCU_UART_STOPBIT stopBit,
                      MCU_UART_CHECK_MODE checkMode, MCU_UART_HARD_CONTROL hardWareControl, MCU_UartRecvIntProcFunc rxCallBack);

//串口发送数组
void BoardRS_SendBuffer(BOARD_RS_SERIAL serialNo,uint8_t* bufferStartPtr,uint16_t sendLength);

//串口发送字符串
void BoardRS_SendString(BOARD_RS_SERIAL serialNo,uint8_t* stringStartPtr);


#endif




