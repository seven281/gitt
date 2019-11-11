#ifndef __ACTION_MEASURE_ROOM_AND_LIQUID_B_H_
#define __ACTION_MEASURE_ROOM_AND_LIQUID_B_H_
#include "ActionCommonBase.h"


//测量室初始化
LH_ERR ActionMeasureRoom_Init(void);

//测量室上门全开
LH_ERR ActionMeasureRoom_UpDoorOpenFull(void);

//测量室上门半开
LH_ERR ActionMeasureRoom_UpDoorOpenHalf(void);

//测量室上门打开到灌注位
LH_ERR ActionMeasureRoom_UpDoorOpenPrime(void);

//测量室上门关闭
LH_ERR ActionMeasureRoom_UpDoorClose(void);

//测量室B液灌注
LH_ERR ActionMeasureRoom_PrimeOnce(void);

//测量室B液注液
LH_ERR ActionMeasureRoom_InjectOnce(void);

//测量室光子测量,自动注液
LH_ERR ActionMeasureRoom_ReadValueWithInject(uint16_t measureTimeMs,uint32_t* result);

//测量室暗计数
LH_ERR ActionMeasureRoom_ReadValueDark(uint16_t measureTimeMs,uint32_t* result);

//测量室本底测量
LH_ERR ActionMeasureRoom_ReadValueBackgroud(uint16_t measureTimeMs,uint32_t* result);

//测量窗复位
LH_ERR ActionMeasureRoom_WindowReset(void);

//测量窗打开
LH_ERR ActionMeasureRoom_WindowOpen(void);

//测量窗关闭
LH_ERR ActionMeasureRoom_WindowClose(void);
//测量光计数
LH_ERR ActionMeasureRoom_ReadValue(uint16_t measureTimeMs,MEASURE_VALUE_FLAG measureflag,uint32_t* result);
#endif
