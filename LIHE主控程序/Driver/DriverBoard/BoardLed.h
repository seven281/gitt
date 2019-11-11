#ifndef __BOARD_LED_H_
#define __BOARD_LED_H_
#include "MCU_Inc.h"

//LED灯序号
typedef enum BOARD_LED
{
    BOARD_LED_RED,
    BOARD_LED_GREEN,
    BOARD_LED_YELLOW,
    BOARD_LED_COUNT,
}BOARD_LED;

//LED灯状态
typedef enum BOARD_LED_STATE
{
    BOARD_LED_DARK,
    BOARD_LED_LIGHT,
}BOARD_LED_STATE;


//led初始化
void BoardLedInit(BOARD_LED index,BOARD_LED_STATE initStatus);

//LED设置状态
void BoardLedWrite(BOARD_LED index,BOARD_LED_STATE status);

//LED切换状态
void BoardLedToogle(BOARD_LED index);

#endif





