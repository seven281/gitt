#include "BoardPressure.h"

//压力传感器检测接口初始化
void BoardPressureSensorInit(void)
{
    MCU_ADC_Init();
}

//压力传感器读取当前值
uint16_t BoardPressureSensorReadValueSingle(BOARD_PRESSURE_CHANNEL channel)
{
    return MCU_ADC_ReadChannelValue(channel);
}

//压力传感器读取多次值并平均
uint16_t BoardPressureSensorReadValueAverage(BOARD_PRESSURE_CHANNEL channel,uint16_t count)
{
    return MCU_ADC_ReadChannelValueAverage(channel,count);
}

