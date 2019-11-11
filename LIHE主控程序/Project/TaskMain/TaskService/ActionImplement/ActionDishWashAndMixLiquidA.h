#ifndef __ACTION_DISH_WASH_AND_MIX_LIQUID_A_H_
#define __ACTION_DISH_WASH_AND_MIX_LIQUID_A_H_
#include "ActionCommonBase.h"


//清洗盘与A液混匀
//清洗盘初始化
LH_ERR ActionDishWash_Reset(uint16_t* currentHoleIndex,ACTION_SENSOR_STATUS* washLightSensorHadCup);

//清洗盘移动到指定杯位
LH_ERR ActionDishWash_Move2SpecialCup(uint16_t targetHoleIndex,uint16_t* currentHoleIndex,ACTION_SENSOR_STATUS* washLightSensorHadCup);

//清洗盘移动指定个杯位
LH_ERR ActionDishWash_MoveSpecialCups(uint16_t holeOffset,uint16_t* currentHoleIndex,ACTION_SENSOR_STATUS* washLightSensorHadCup);

//A液摇匀复位
LH_ERR ActionMixLiquidA_Reset(void);

//A液摇匀上升
LH_ERR ActionMixLiquidA_Up(void);

//A液摇匀下降
LH_ERR ActionMixLiquidA_Down(void);

//A液摇匀混匀指定时间
LH_ERR ActionMixLiquidA_RotateAnyTimes(uint16_t timeMs);


#endif
