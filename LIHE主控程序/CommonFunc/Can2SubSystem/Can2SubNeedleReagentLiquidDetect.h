#ifndef __CAN2_SUB_NEEDLE_REAGENT_LIQUID_DETECT_H_
#define __CAN2_SUB_NEEDLE_REAGENT_LIQUID_DETECT_H_
#include "Can2SubNeedleLiquidDetectCommon.h"

//检测探液板是否存在
LH_ERR Can2SubNeedleReagentLiquidDetectCheckExist(void);

//读取探液板检测阈值
LH_ERR Can2SubNeedleReagentLiquidDetectReadThresholdValue(uint16_t* paramReadPtr);

//读取探液板阈值检测次数
LH_ERR Can2SubNeedleReagentLiquidDetectReadThresholdCount(uint16_t* paramReadPtr);

//读取探液板斜率检测值
LH_ERR Can2SubNeedleReagentLiquidDetectReadSlopeValue(uint16_t* paramReadPtr);

//读取探液板斜率检测次数
LH_ERR Can2SubNeedleReagentLiquidDetectSlopeCount(uint16_t* paramReadPtr);

//读取探液板信号保持时间
LH_ERR Can2SubNeedleReagentLiquidDetectSignalHoldTimeMs(uint16_t* paramReadPtr);

//读取探液板电子电阻值
LH_ERR Can2SubNeedleReagentLiquidDetectResValue(uint16_t* paramReadPtr);

//写入电子电阻值
LH_ERR Can2SubNeedleReagentLiquidDetectWriteResValue(uint16_t paramWrite);

//执行探液板校准
LH_ERR Can2SubNeedleReagentLiquidDetectAdjust(void);

//打开探液功能
LH_ERR Can2SubNeedleReagentLiquidDetectEnableFunc(void);

//关闭探液功能
LH_ERR Can2SubNeedleReagentLiquidDetectDisableFunc(void);

#endif
