#include "BoardBeep.h"

//蜂鸣器初始化
void BoardBeepInit(BEEP_STATE initState)
{
    //PH11
    BitAction ioStatus;
    //确定写入状态
    ioStatus = (initState == BEEP_OFF) ? Bit_RESET:Bit_SET;
    //初始化IO口
    MCU_PortInit(MCU_PIN_H_11, GPIO_Mode_OUT, GPIO_OType_PP, GPIO_PuPd_UP, GPIO_Fast_Speed);
    MCU_PortWriteSingle(MCU_PIN_H_11, ioStatus);
}

//设置蜂鸣器状态
void BoardBeepSetState(BEEP_STATE state)
{
    BitAction ioStatus;
    ioStatus = (state == BEEP_OFF) ?Bit_RESET:Bit_SET;
    MCU_PortWriteSingle(MCU_PIN_H_11, ioStatus);
}
