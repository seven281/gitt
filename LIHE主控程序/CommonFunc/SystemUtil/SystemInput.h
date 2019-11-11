#ifndef __SYSTEM_INPUT_H_
#define __SYSTEM_INPUT_H_
#include "SystemInputIndex.h"
#include "BoardInc.h"
#include "Can2SubInputRead.h"

//杯栈管理模块 
//新杯盘顶部检测传感器      1有效,代表传感器遮挡                   
#define NEW_STACK_TOP_SENSOR                    SYSTEM_INPUT_LOW_BOARD3_MCU_ARM_SEN9_PE2
//新杯盘推到位检测          1有效,代表传感器遮挡
#define NEW_STACK_PUSH_IN_POS_SENSOR            SYSTEM_INPUT_LOW_BOARD3_MCU_ARM_SEN8_PE3

//空杯盘中部光电传感器      1有效,代表传感器遮挡
#define EMPTY_STACK_COUNT_SENSOR                SYSTEM_INPUT_LOW_BOARD3_MCU_ARM_SEN10_PI7

//垃圾桶1是否存在传感器     1有效,代表垃圾桶存在
#define GARBAGE1_EXIST_SENSOR                   SYSTEM_INPUT_LOW_BOARD3_MCU_FRUBBISH3_PH13     
//垃圾桶1过流传感器         
#define GARBAGE1_OVER_LOAD_SENSOR               SYSTEM_INPUT_LOW_BOARD3_MCU_FRUBBISH_OVER2_PI1

//垃圾桶2是否存在           1有效,代表垃圾桶存在
#define GARBAGE2_EXIST_SENSOR                   SYSTEM_INPUT_LOW_BOARD3_MCU_FRUBBISH4_PC9
//垃圾桶2过流传感器
#define GARBAGE2_OVER_LOAD_SENSOR               SYSTEM_INPUT_LOW_BOARD3_MCU_FRUBBISH_OVER1_PI0

//反应盘放杯光纤检测        1有效
#define DISH_REACTION_LIGHT_SENSOR              SYSTEM_INPUT_LOW_BOARD3_MCU_ARM_SEN16_PB7

//清洗盘放杯光纤检测        1有效
#define DISH_WASH_LIGHT_SENSOR                  SYSTEM_INPUT_LOW_BOARD1_MCU_ARM_SEN10_PI7



//读取系统输入
LH_ERR SystemInputRead(SYSTEM_INPUT_INDEX inputIndex,BitAction* inputValue);

#endif




