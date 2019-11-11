#ifndef __TSC_NEEDLE_WASH_H_
#define __TSC_NEEDLE_WASH_H_
#include "TSC_Base.h"

typedef enum TSC_NEEDLE_WASH_CMD_INDEX
{
    TSC_NEEDLE_WASH_INIT = 0,
    TSC_NEEDLE_WASH_PRIME_ONCE = 1,
    TSC_NEEDLE_WASH_CLEAN_ONCE = 2,
}TSC_NEEDLE_WASH_CMD_INDEX;


/*************************************************清洗针动作********************************************************************/
//清洗针上一次指令的执行状态
TSC_CMD_STATE TSC_NeedleWashGetLastCommandStatusAndResult(LH_ERR* errCode);

//清洗针数据结构初始化
void TSC_NeedleWashResultDataInit(void);

//清洗针初始化
LH_ERR TSC_NeedleWashInitWhileAck(void);

//清洗针执行一次灌注
LH_ERR TSC_NeedleWashPrimeOnceWhileAck(void);

//清洗针执行一次清洗动作,参数表示选择哪几根针注液
LH_ERR TSC_NeedleWashCleanOnceWhileAck(TSC_NEEDLE_WASH_SELECT_FLAG select_flag,CLEAN_NEEDLE_INDEX clean_index);

/*************************************************清洗针动作********************************************************************/

/*********************************************动作结果中的数据读取***************************************************/



#endif
