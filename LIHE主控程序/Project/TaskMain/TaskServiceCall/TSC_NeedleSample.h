#ifndef __TSC_NEEDLE_SAMPLE_H_
#define __TSC_NEEDLE_SAMPLE_H_
#include "TSC_Base.h"

typedef enum TSC_NEEDLE_SAMPLE_CMD_INDEX
{
    TSC_NEEDLE_SAMPLE_INTI = 0,
    TSC_NEEDLE_SAMPLE_ROTATE_SPECIAL_POSITION = 1,
    TSC_NEEDLE_SAMPLE_ABSORB_ONCE = 2,
    TSC_NEEDLE_SAMPLE_INJECT_ONCE = 3,
    TSC_NEEDLE_SAMPLE_CLEAN = 4,
    TSC_NEEDLE_SAMPLE_FORCE_CLEAN = 5,
    TSC_NEEDLE_SAMPLE_PRIME = 6,
    TSC_NEEDLE_SAMPLE_FORCE_PRIME = 7,
}TSC_NEEDLE_SAMPLE_CMD_INDEX;


/***************************************************样本针动作*************************************************************/
//样本针上一次指令的执行状态
TSC_CMD_STATE TSC_NeedleSampleGetLastCommandStatusAndResult(LH_ERR* errCode);

//样本针数据结构初始化
void TSC_NeedleSampleResultDataInit(void);

//样本针初始化
LH_ERR TSC_NeedleSampleInitWhileAck(void);

//样本针旋转到指定位置,参数为旋转的目的坐标 
LH_ERR TSC_NeedleSampleRotateSpecialPositionWhileAck(TSC_NEEDLE_SAMPLE_POS specialPosition);

//样本针吸液,参数为指定吸液量,单位为微升
LH_ERR TSC_NeedleSampleAbsorbOnceWhileAck(uint16_t absorbLiquidUl,SAMPLE_STATUS use);

//样本针排液,参数为指定排液亮,单位为微升
LH_ERR TSC_NeedleSampleInjectOnceWhileAck(uint16_t injectLiquidUl);

//样本针清洗,参数为清洗完成之后针的目标位置
LH_ERR TSC_NeedleSampleCleanWhileAck(TSC_NEEDLE_SAMPLE_POS targetPositionWhenCleanOver,TSC_NEEDLE_SAMPLE_CLEAN_OPT opt);

//样本针强洗,参数为强洗完成之后针的目标位置
LH_ERR TSC_NeedleSampleForceCleanWhileAck(TSC_NEEDLE_SAMPLE_POS targetPositionWhenCleanOver,TSC_NEEDLE_SAMPLE_CLEAN_OPT opt);

//样本针清洗液灌注,灌注完成保持在清洗位上方不动
LH_ERR TSC_NeedleSampleCleanPrimeWhileAck(TSC_NEEDLE_SAMPLE_PRIME_OPT opt);

//样本针强洗液灌注,灌注完成保持在强洗位上方不动
LH_ERR TSC_NeedleSampleForceCleanPrimeWhileAck(void);

/***************************************************样本针动作*************************************************************/

/*********************************************动作结果中的数据读取***************************************************/
//读取当前样本针水平悬停的位置
TSC_NEEDLE_SAMPLE_POS TSC_NeedleSampleReadCurrentPosition(void);

//读取当前样本针内部包含的液量总值
uint16_t TSC_NeedleSampleReadCurrentLiquidVolume(void);


#endif
