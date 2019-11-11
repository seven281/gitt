#include "BoardRS.h"

//串口初始化
void BoardRS_Init(BOARD_RS_SERIAL serialNo,uint32_t baud, MCU_UART_LENGTH length, MCU_UART_STOPBIT stopBit,
                      MCU_UART_CHECK_MODE checkMode, MCU_UART_HARD_CONTROL hardWareControl, MCU_UartRecvIntProcFunc rxCallBack)
{
    switch(serialNo)
    {
        case BOARD_RS1_UART3:
            MCU_Uart3Init(baud,length,stopBit,checkMode,hardWareControl,rxCallBack);
            break;
        case BOARD_RS2_UART1:
            MCU_Uart1Init(baud,length,stopBit,checkMode,hardWareControl,rxCallBack);
            break;
        case BOARD_RS3_UART2:
            MCU_Uart2Init(baud,length,stopBit,checkMode,hardWareControl,rxCallBack);
            break;
        case BOARD_RS4_UART6:
            MCU_Uart6Init(baud,length,stopBit,checkMode,hardWareControl,rxCallBack);
            break;
    }
}

//串口发送数组
void BoardRS_SendBuffer(BOARD_RS_SERIAL serialNo,uint8_t* bufferStartPtr,uint16_t sendLength)
{
    switch(serialNo)
    {
        case BOARD_RS1_UART3:
            MCU_Uart3SendBuffer(bufferStartPtr,sendLength);
            break;
        case BOARD_RS2_UART1:
            MCU_Uart1SendBuffer(bufferStartPtr,sendLength);
            break;
        case BOARD_RS3_UART2:
            MCU_Uart2SendBuffer(bufferStartPtr,sendLength);
            break;
        case BOARD_RS4_UART6:
            MCU_Uart6SendBuffer(bufferStartPtr,sendLength);
            break;
    }
}

//串口发送字符串
void BoardRS_SendString(BOARD_RS_SERIAL serialNo,uint8_t* stringStartPtr)
{
    switch(serialNo)
    {
        case BOARD_RS1_UART3:
            MCU_Uart3SendString(stringStartPtr);
            break;
        case BOARD_RS2_UART1:
            MCU_Uart1SendString(stringStartPtr);
            break;
        case BOARD_RS3_UART2:
            MCU_Uart2SendString(stringStartPtr);
            break;
        case BOARD_RS4_UART6:
            MCU_Uart6SendString(stringStartPtr);
            break;
    }
}
