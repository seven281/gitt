#include "BoardLed.h"

//LED初始化函数指针类型
typedef void (*BoardLedInitFuncPtr)(BOARD_LED_STATE initState);
//LED写入函数指针类型
typedef void (*BoardLedWriteFuncPtr)(BOARD_LED_STATE state);
//LED状态切换函数指针类型
typedef void (*BoardLedToogleFuncPtr)(void);


//RED
static void BoardLedInitRed(BOARD_LED_STATE initState)
{
    BitAction ioState;
    //确定写入状态
    ioState = (initState == BOARD_LED_DARK) ? Bit_SET:Bit_RESET;
    //初始化IO口
    MCU_PortInit(MCU_PIN_H_14, GPIO_Mode_OUT, GPIO_OType_PP, GPIO_PuPd_UP, GPIO_Fast_Speed);
    MCU_PortWriteSingle(MCU_PIN_H_14, ioState);
}
static void BoardLedWriteRed(BOARD_LED_STATE State)
{
    BitAction ioState;
    ioState = (State == BOARD_LED_DARK) ? Bit_SET:Bit_RESET;
    MCU_PortWriteSingle(MCU_PIN_H_14, ioState);
}
static void BoardLedToogleRed()
{
    MCU_PortToogleSingle(MCU_PIN_H_14);
}

//green
static void BoardLedInitGreen(BOARD_LED_STATE initState)
{
    BitAction ioState;
    //确定写入状态
    ioState = (initState == BOARD_LED_DARK) ? Bit_SET:Bit_RESET;
    //初始化IO口
    MCU_PortInit(MCU_PIN_H_15, GPIO_Mode_OUT, GPIO_OType_PP, GPIO_PuPd_UP, GPIO_Fast_Speed);
    MCU_PortWriteSingle(MCU_PIN_H_15, ioState);
}
static void BoardLedWriteGreen(BOARD_LED_STATE State)
{
    BitAction ioState;
    ioState = (State == BOARD_LED_DARK) ? Bit_SET:Bit_RESET;
    MCU_PortWriteSingle(MCU_PIN_H_15, ioState);
}
static void BoardLedToogleGreen()
{
    MCU_PortToogleSingle(MCU_PIN_H_15);
}

//yellow
static void BoardLedInitYellow(BOARD_LED_STATE initState)
{
    BitAction ioState;
    //确定写入状态
    ioState = (initState == BOARD_LED_DARK) ? Bit_SET:Bit_RESET;
    //初始化IO口
    MCU_PortInit(MCU_PIN_H_13, GPIO_Mode_OUT, GPIO_OType_PP, GPIO_PuPd_UP, GPIO_Fast_Speed);
    MCU_PortWriteSingle(MCU_PIN_H_13, ioState);
}
static void BoardLedWriteYellow(BOARD_LED_STATE State)
{
    BitAction ioState;
    ioState = (State == BOARD_LED_DARK) ? Bit_SET:Bit_RESET;
    MCU_PortWriteSingle(MCU_PIN_H_13, ioState);
}
static void BoardLedToogleYellow()
{
    MCU_PortToogleSingle(MCU_PIN_H_13);
}


static const BoardLedInitFuncPtr BoardLedInitFuncPtrArray[] = {
    BoardLedInitRed,
    BoardLedInitGreen,
    BoardLedInitYellow,
};

static const BoardLedWriteFuncPtr BoardLedWriteFuncPtrArray[] = {
    BoardLedWriteRed,
    BoardLedWriteGreen,
    BoardLedWriteYellow,
};  

static const BoardLedToogleFuncPtr BoardLedToogleFuncPtrArray[] = {
    BoardLedToogleRed,
    BoardLedToogleGreen,
    BoardLedToogleYellow,
};


//led初始化
void BoardLedInit(BOARD_LED index,BOARD_LED_STATE initState)
{
    if(index >= BOARD_LED_COUNT)
    {
        return;
    }
    if(initState > BOARD_LED_LIGHT)
    {
        return;
    }
    BoardLedInitFuncPtrArray[index](initState);
}

//LED设置状态
void BoardLedWrite(BOARD_LED index,BOARD_LED_STATE State)
{
    if(index >= BOARD_LED_COUNT)
    {
        return;
    }
    if(State > BOARD_LED_LIGHT)
    {
        return;
    }
    BoardLedWriteFuncPtrArray[index](State);
}

//LED切换状态
void BoardLedToogle(BOARD_LED index)
{
    if(index >= BOARD_LED_COUNT)
    {
        return;
    }
    BoardLedToogleFuncPtrArray[index]();
}
