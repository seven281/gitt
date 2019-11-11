#ifndef __ACTION_NEEDLE_REAGENT_AND_BEAD_H_
#define __ACTION_NEEDLE_REAGENT_AND_BEAD_H_
#include "ActionCommonBase.h"


//磁珠试剂针初始化
LH_ERR ActionNeedleReagentBead_Init(TSC_NEEDLE_REAGENT_BEAD_POS* pos);

//旋转到指定位置
LH_ERR ActionNeedleReagentBead_RotateSpecialPos(TSC_NEEDLE_REAGENT_BEAD_POS targetPos,TSC_NEEDLE_REAGENT_BEAD_POS* pos);

//磁珠试剂针吸液
LH_ERR ActionNeedleReagentBead_AbsorbAnyLiquid(uint16_t liquidUl,TSC_NEEDLE_REAGENT_BEAD_POS* rotatePos,ONE_OF_CUPSTATUS *oneofcups);

//磁珠试剂针排液
LH_ERR ActionNeedleReagentBead_InjectAnyLiquid(uint16_t liquidUl,TSC_NEEDLE_REAGENT_BEAD_POS* rotatePos);

//磁珠试剂针清洗
LH_ERR ActionNeedleReagentBead_Clean(TSC_NEEDLE_REAGENT_BEAD_CLEAN_OPT opt,TSC_NEEDLE_REAGENT_BEAD_POS* rotatePos);

//磁珠试剂针灌注
LH_ERR ActionNeedleReagentBead_Prime(TSC_NEEDLE_REAGENT_PRIME_OPT opt,TSC_NEEDLE_REAGENT_BEAD_POS* rotatePos);


#endif
