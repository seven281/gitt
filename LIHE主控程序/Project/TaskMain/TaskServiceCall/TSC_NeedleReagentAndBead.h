#ifndef __TSC_NEEDLE_REAGENT_AND_BEAD_H_
#define __TSC_NEEDLE_REAGENT_AND_BEAD_H_
#include "TSC_Base.h"


typedef enum TSC_NEEDLE_REAGENT_BEAD_CMD_INDEX
{
    TSC_NEEDLE_REAGENT_BEAD_INIT = 0,
    TSC_NEEDLE_REAGENT_BEAD_ROTATE_SPECIAL_POSITION = 1,
    TSC_NEEDLE_REAGENT_BEAD_ABSORB = 2,
    TSC_NEEDLE_REAGENT_BEAD_INJECT = 3,
    TSC_NEEDLE_REAGENT_BEAD_CLEAN = 4,
    TSC_NEEDLE_REAGENT_BEAD_PRIME = 5,
}TSC_NEEDLE_REAGENT_BEAD_CMD_INDEX;


/***************************************************磁珠试剂针动作*********************************************************/
//试剂磁珠针上一次指令的执行状态
TSC_CMD_STATE TSC_NeedleReagentBeadGetLastCommandStatusAndResult(LH_ERR* errCode);

//试剂磁珠针数据结构初始化
void TSC_NeedleReagentBeadResultDataInit(void);

//试剂磁珠针初始化
LH_ERR TSC_NeedleReagentBeadInitWhileAck();

//试剂磁珠针旋转到指定位置 
LH_ERR TSC_NeedleReagentBeadRotateSpecialPositionWhileAck(TSC_NEEDLE_REAGENT_BEAD_POS targetPosition);

//试剂磁珠针吸液 参数为吸液量,单位UL
LH_ERR TSC_NeedleReagentBeadAbsorbWhileAck(uint16_t absorbLiquidUl,ONE_OF_CUPSTATUS oneofcup);

//试剂磁珠针,参数为注液量 单位UL
LH_ERR TSC_NeedleReagentBeadInjectWhileAck(uint16_t injectLiquidUl);

//试剂磁珠针 实际磁珠针清洗,参数为清洗完成以后旋转到的位置
LH_ERR TSC_NeedleReagentBeadCleanWhileAck(TSC_NEEDLE_REAGENT_BEAD_POS targetPositionWhenCleanOver,TSC_NEEDLE_REAGENT_BEAD_CLEAN_OPT cleanOpt);

//试剂磁珠针灌注,灌注完成保持在清洗位上方不动
LH_ERR TSC_NeedleReagentBeadPrimeWhileAck(TSC_NEEDLE_REAGENT_PRIME_OPT opt);


/***************************************************磁珠试剂针动作*********************************************************/


/*********************************************动作结果中的数据读取***************************************************/

//读取当前试剂磁珠针水平悬停的位置
TSC_NEEDLE_REAGENT_BEAD_POS TSC_NeedleReagentBeadReadCurrentPosition(void);

//读取当前实际磁珠针内部包含的液量总值
uint16_t TSC_NeedleReagentBeadReadCurrentLiquidVolume(void);

REAGENT_CUP_STATUS* TSC_NeedlReagentBeadReadCUPStatus(ONE_OF_CUPSTATUS *oneofcups);
#endif
