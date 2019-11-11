#ifndef __BOARD_PRESSURE_H_
#define __BOARD_PRESSURE_H_
#include "MCU_Inc.h"

typedef enum BOARD_PRESSURE_CHANNEL
{
    BOARD_PRESSURE_CHANNEL1 = ADC_Channel_10,
    BOARD_PRESSURE_CHANNEL2 = ADC_Channel_11,
    BOARD_PRESSURE_CHANNEL3 = ADC_Channel_12,
    BOARD_PRESSURE_CHANNEL4 = ADC_Channel_13,
}BOARD_PRESSURE_CHANNEL;

//压力传感器检测接口初始化
void BoardPressureSensorInit(void);

//压力传感器读取当前值
uint16_t BoardPressureSensorReadValueSingle(BOARD_PRESSURE_CHANNEL channel);

//压力传感器读取多次值并平均
uint16_t BoardPressureSensorReadValueAverage(BOARD_PRESSURE_CHANNEL channel,uint16_t count);

#endif



