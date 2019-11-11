#ifndef __ACTION_DISH_REACTION_AND_MIX_REAGENT_H_
#define __ACTION_DISH_REACTION_AND_MIX_REAGENT_H_
#include "ActionCommonBase.h"


//反应盘初始化
LH_ERR ActionDishReaction_Reset(uint16_t* currentHoleIndex,ACTION_SENSOR_STATUS* reactionLightSensorHadCup);

//反应盘移动到指定杯位
LH_ERR ActionDishReaction_Move2SpecialCup(uint16_t targetHoleIndex,uint16_t* currentHoleIndex,ACTION_SENSOR_STATUS* reactionLightSensorHadCup);

//反应盘移动指定个杯位
LH_ERR ActionDishReaction_MoveSpecialCups(uint16_t holeOffset,uint16_t* currentHoleIndex,ACTION_SENSOR_STATUS* reactionLightSensorHadCup);

//试剂摇匀复位
LH_ERR ActionMixReagent_Reset(void);

//试剂摇匀上升
LH_ERR ActionMixReagent_Up(void);

//试剂摇匀下降
LH_ERR ActionMixReagent_Down(void);

//试剂摇匀混匀指定时间
LH_ERR ActionMixReagent_RotateAnyTimes(uint16_t timeMs);


#endif
