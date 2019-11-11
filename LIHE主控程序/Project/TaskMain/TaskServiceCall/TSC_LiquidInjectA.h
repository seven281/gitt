#ifndef __TSC_LIQUID_INJECT_A_H_
#define __TSC_LIQUID_INJECT_A_H_
#include "TSC_Base.h"

typedef enum TSC_LIQUID_A_INJECT_CMD_INDEX
{
    TSC_LIQUID_A_INJECT_INIT = 0,
    TSC_LIQUID_A_INJECT_PRIME = 1,
    TSC_LIQUID_A_INJECT_INJECT_ONCE = 2,
}TSC_LIQUID_A_INJECT_CMD_INDEX;


/*************************************************注A液动作***********************************************************************/
//注A液上一次指令的执行状态
TSC_CMD_STATE TSC_LiquidInjectAGetLastCommandStatusAndResult(LH_ERR* errCode);

//注A液数据结构初始化
void TSC_LiquidInjectAResultDataInit(void);

//注A液初始化
LH_ERR TSC_LiquidInjectAInitWhileAck(void);

//注A液灌注一次
LH_ERR TSC_LiquidInjectAPrimeOnceWhileAck(void);

//注A液注液一次
LH_ERR TSC_LiquidInjectAInjectOnceWhileAck(void);

/*************************************************注A液动作***********************************************************************/


/*********************************************动作结果中的数据读取***************************************************/



#endif





