#ifndef __TSC_DISH_REAGENT_H_
#define __TSC_DISH_REAGENT_H_
#include "TSC_Base.h"


typedef enum TSC_DISH_REAGENT_CMD_INDEX
{
    TSC_DISH_REAGENT_INIT = 0,
    TSC_DISH_REAGENT_MOVE_2_SPECIAL_HOLE_WITH_OFFSET = 1,
    TSC_DISH_REAGENT_MOVE_2_SPECIAL_HOLE_STEP = 2,
}TSC_DISH_REAGENT_CMD_INDEX;

/*************************************************试剂盘动作********************************************************************/
//试剂盘上一次指令的执行状态
TSC_CMD_STATE TSC_DishReagentGetLastCommandStatusAndResult(LH_ERR* errCode);

//试剂盘数据结构初始化
void TSC_DishReagentResultDataInit(void);

//试剂盘初始化
LH_ERR TSC_DishReagentInitWhileAck(void);

//试剂盘移动到指定孔位的指定偏移,参数1为孔位号  参数2为偏移 
LH_ERR TSC_DishReagentMove2SpecialHoleWithOffsetWhileAck(uint8_t holeIndex,DISH_REAGENT_OFFSET offset);

//试剂盘相对当前位置移动指定个孔位,参数为偏移孔位个数
LH_ERR TSC_DishReagentMove2SpecialHoleStepWhileAck(uint8_t holeStep);



/*************************************************试剂盘动作********************************************************************/


/*********************************************动作结果中的数据读取***************************************************/
//读取当前试剂盘的偏移标志
DISH_REAGENT_OFFSET TSC_DishReagentReadCurrentOffset(void);

//读取当前试剂盘的孔位序号
uint8_t TSC_DishReagentReadCurrentHoleIndex(void);




#endif
