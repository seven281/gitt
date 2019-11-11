#ifndef __BOARD_IN_OUT_CONFIG_H_
#define __BOARD_IN_OUT_CONFIG_H_
#include "stm32f4xx.h"


//板上输出初始化电平配置
extern const BitAction  boardOutInitLevelConfigArray[];

//板上IO初始化的上下拉配置
extern const GPIOPuPd_TypeDef  boardOutInitPullConfigArray[];

//板上输入上拉下拉配置
extern const GPIOPuPd_TypeDef boardInInitPullConfigArray[];




#endif





