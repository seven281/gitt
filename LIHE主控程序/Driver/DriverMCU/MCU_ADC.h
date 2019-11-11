#ifndef __MCU_ADC_H_
#define __MCU_ADC_H_
#include "CoreInc.h"

//ADC初始化
void MCU_ADC_Init(void);

//ADC读取当前通道值,电压值,得到的结果是 毫伏为单位
uint16_t MCU_ADC_ReadChannelValue(uint8_t channel);

//ADC读取当前通道值并取平均
uint16_t MCU_ADC_ReadChannelValueAverage(uint8_t channel,uint16_t count);

//ADC读取当前芯片温度
float MCU_ADC_ReadChipTemp(void);

#endif





