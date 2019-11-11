#ifndef __ACTION_DISH_REAGENT_H_
#define __ACTION_DISH_REAGENT_H_
#include "ActionCommonBase.h"


//试剂盘初始化
LH_ERR ActionDishReagent_Reset(uint16_t* currentHoleIndex,DISH_REAGENT_OFFSET* offsetFlag);

//移动到指定孔位带偏移
LH_ERR ActionDishReagent_Move2SpecialHoleWithOffset(uint16_t* currentHoleIndex,DISH_REAGENT_OFFSET* offsetFlag,uint16_t targetHoleIndex,DISH_REAGENT_OFFSET offset);

//试剂盘移动指定个孔位
LH_ERR ActionDishReagent_MoveAnyHoleStep(uint16_t* currentHoleIndex,DISH_REAGENT_OFFSET* offsetFlag,uint16_t targetHoleOffset);


#endif
