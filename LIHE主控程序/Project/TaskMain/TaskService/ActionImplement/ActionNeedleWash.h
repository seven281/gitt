#ifndef __ACTION_NEEDLE_WASH_H_
#define __ACTION_NEEDLE_WASH_H_
#include "ActionCommonBase.h"



/*清洗针升降的速度 */
    //清洗针顶点降到杯口速度
    #define CURVE_NEEDLE_WASH_DOWN_2_CUP_TOP                 SM_CURVE_0
    //清洗针杯口降到杯底速度
    #define CURVE_NEEDLE_WASH_DOWN_2_CUP_BUTTOM              SM_CURVE_1
    //清洗针升到杯口速度
    #define CURVE_NEEDLE_WASH_UP_2_CUP_TOP                   SM_CURVE_2
    //清洗针升到顶点速度
    #define CURVE_NEEDLE_WASH_UP_2_TOP                       SM_CURVE_3
/*清洗针注射泵的速度 */
    //清洗针注射泵吸液速度
    #define CURVE_PUMP_WASH_ABSORB                           SM_CURVE_0
    //清洗针注射泵注液速度
    #define CURVE_PUMP_WASH_INJECT                           SM_CURVE_1
    //清洗针注射泵灌注速度
    #define CURVE_PUMP_WASH_PRIME                            SM_CURVE_2
    
//清洗针初始化
LH_ERR ActionNeedleWash_Init(void);

//清洗针灌注
LH_ERR ActionNeedleWash_Prime(void);

//清洗针清洗返回
LH_ERR ActionNeedleWash_Clean(CLEAN_NEEDLE_CMD_DATA clean_needle);



#endif
