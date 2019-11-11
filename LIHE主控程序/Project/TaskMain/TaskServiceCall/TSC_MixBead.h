#ifndef __TSC_MIX_BEAD_H_
#define __TSC_MIX_BEAD_H_
#include "TSC_Base.h"

typedef enum TSC_BEAD_MIX_CMD_INDEX
{
    TSC_BEAD_MIX_INIT = 0,
    TSC_BEAD_MIX_ROTATE_ANY_TIME = 1,
}TSC_BEAD_MIX_CMD_INDEX;

/************************************************磁珠摇匀动作*****************************************************************/
//磁珠摇匀上一次指令的执行状态
TSC_CMD_STATE TSC_BeadMixGetLastCommandStatusAndResult(LH_ERR* errCode);

//磁珠摇匀数据结构初始化
void TSC_BeadMixResultDataInit(void);

//磁珠摇匀初始化
LH_ERR TSC_BeadMixInitWhileAck(void);

//磁珠摇匀摇匀指定时间,该指令将会让磁珠摇匀自动升起来,摇匀指定时间之后自动降下去
LH_ERR TSC_BeadMixRotateSpecialTimeWhileAck(uint16_t timeMs);

/************************************************磁珠摇匀动作*****************************************************************/


#endif
