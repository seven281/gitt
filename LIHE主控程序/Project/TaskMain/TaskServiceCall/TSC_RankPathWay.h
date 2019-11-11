#ifndef __TSC_RANK_PATH_WAY_H_
#define __TSC_RANK_PATH_WAY_H_
#include "TSC_Base.h"

typedef enum TSC_RANK_PATH_WAY_CMD_INDEX
{
    TSC_RANK_PATH_WAY_INIT = 0,
    TSC_RANK_PATH_WAY_MOVE_2_SPECIAL_TUBE = 1,
    TSC_RANK_PATH_WAY_BACK_ONE_RACK,
}TSC_RANK_PATH_WAY_CMD_INDEX;

/***********************************************试管架传送带动作****************************************************************/
//试管架传送带上一次指令的执行状态
TSC_CMD_STATE TSC_RankPathWayGetLastCommandStatusAndResult(LH_ERR* errCode);

//试管架传送带数据结构初始化
void TSC_RankPathWayResultDataInit(void);

//试管架传送带初始化
LH_ERR TSC_RankPathWayInitWhileAck(void);

//试管架传送带移动到指定的试管位,试管位参数取值0到5,当设置为6时,代表移动到下一个架子的1号位
//参数取值1-10
LH_ERR TSC_RankPathWayMove2SpecialTubeWhileAck(uint8_t tubeIndex);

//试管架传送到回退一整个试管架位置,例如当前试管位为3,调用该指令,回退到上一个试管架的3号位
LH_ERR TSC_RankPathWayBackOneRackWhileAck(void);

/***********************************************试管架传送带动作****************************************************************/


#endif
