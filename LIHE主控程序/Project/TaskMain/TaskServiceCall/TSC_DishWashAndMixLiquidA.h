#ifndef __TSC_DISH_WASH_AND_MIX_LIQUID_A_H_
#define __TSC_DISH_WASH_AND_MIX_LIQUID_A_H_
#include "TSC_Base.h"

typedef enum TSC_DISH_WASH_LIQUID_A_MIX_CMD_INDEX
{
    TSC_DISH_WASH_LIQUID_A_MIX_INIT = 0,
    TSC_DISH_WASH_LIQUID_A_MIX_MOVE_ANY_HOLE = 1,
    TSC_DISH_WASH_LIQUID_A_MIX_MOVE_2_SPECIAL_HOLE = 2,
    TSC_DISH_WASH_LIQUID_A_MIX_MIX_INIT = 3,
    TSC_DISH_WASH_LIQUID_A_MIX_ROTATE_ANY_TIME = 4,
}TSC_DISH_WASH_LIQUID_A_MIX_CMD_INDEX;


/**************************************************清洗盘与A液摇匀动作************************************************************/
//清洗盘和A液混匀上一次的执行状态
TSC_CMD_STATE TSC_DishWashAndLiquidAMixGetLastCommandStatusAndResult(LH_ERR* errCode);

//清洗盘和A液混匀数据结构初始化
void TSC_DishWashAndLiquidAMixResultDataInit(void);

//清洗盘初始化
LH_ERR TSC_DishWashInitWhileAck(void);

//清洗盘移动指定个孔位
LH_ERR TSC_DishWashMoveAnyHoleStepWhileAck(uint8_t holeSteps);

//清洗盘移动到指定孔位
LH_ERR TSC_DishWashMove2SpecialHoleWhileAck(uint8_t targetHoleIndex);

//A液混匀初始化
LH_ERR TSC_LiquidAMixInitWhileAck(void);

//A液混匀指定时间,自动升起来 自动混匀指定时间 自动下降
LH_ERR TSC_LiquidAMixRotateAnyTimeMsWhileAck(uint16_t timeMs);


/**************************************************清洗盘与A液摇匀动作************************************************************/


/*********************************************动作结果中的数据读取***************************************************/
//读取清洗盘当前孔位是否有杯
ACTION_SENSOR_STATUS TSC_DishWashCheckCurrentHoleHasCup(void);

//检测清洗盘当前孔位编号
uint8_t TSC_DishWashReadCurrentHoleIndex(void);


#endif
