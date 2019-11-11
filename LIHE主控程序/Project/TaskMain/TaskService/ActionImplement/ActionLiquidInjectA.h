#ifndef __ACTION_LIQUID_INJECT_A_H_
#define __ACTION_LIQUID_INJECT_A_H_
#include "ActionCommonBase.h"

//A液初始化
LH_ERR ActionLiquidA_InjectInit(void);

//A液注液
LH_ERR ActionLiquidA_InjectTrigOnce(void);

//A液灌注
LH_ERR ActionLiquidA_InjectPrimeOnce(void);



#endif
