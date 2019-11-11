#ifndef __TSC_DISH_REACTION_AND_MIX_REAGENT_H_
#define __TSC_DISH_REACTION_AND_MIX_REAGENT_H_
#include "TSC_Base.h"

typedef enum TSC_DISH_REACTION_REAGENT_MIX_CMD_INDEX
{
    TSC_DISH_REACTION_INIT = 0,
    TSC_DISH_REACTION_MOVE_2_SPECIAL_HOLE = 1,
    TSC_DISH_REACTION_MOVE_ANY_HOLE_STEP = 2,
    TSC_REAGENT_MIX_INIT = 3,
    TSC_REAGENT_MIX_ROTATE_ANY_TIME = 4,
}TSC_DISH_REACTION_REAGENT_MIX_CMD_INDEX;

/***************************************************反应盘与试剂摇匀动作***********************************************************/
//反应盘和试剂混匀上一次的执行状态
TSC_CMD_STATE TSC_DishReactionAndReagentMixGetLastCommandStatusAndResult(LH_ERR* errCode);

//反应盘和试剂混匀数据结构初始化
void TSC_DishReactionAndReagentMixResultDataInit(void);

//反应盘初始化
LH_ERR TSC_DishReactionInitWhileAck(void);

//反应盘旋转到指定孔位
LH_ERR TSC_DishReactionMove2SpecialHoleWhileAck(uint8_t targetHoleIndex);

//反应盘相对于当前孔位旋转指定个孔位
LH_ERR TSC_DishReactionMoveAnyHoleStepWhileAck(uint8_t holeSteps);

//试剂混匀复位
LH_ERR TSC_ReagentMixInitWhileAck(void);

//试剂混匀开始混匀指定时间,单位毫秒,试剂摇匀自动升起来,摇匀指定时间之后自动降下去
LH_ERR TSC_ReagentMixRotateAnyTimeMsWhileAck(uint16_t timeMs);



/***************************************************反应盘与试剂摇匀动作***********************************************************/

/*********************************************动作结果中的数据读取***************************************************/

//读取当前反应盘孔位坐标
uint8_t TSC_DishReactionReadCurrentHoleIndex(void);

//读取当前反应盘孔位是否有杯
ACTION_SENSOR_STATUS TSC_DishReactionCheckCupExist(void);



#endif
