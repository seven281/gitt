#ifndef __CAN2_SUB_NEEDLE_SAMPLE_LIQUID_DETECT_H_
#define __CAN2_SUB_NEEDLE_SAMPLE_LIQUID_DETECT_H_
#include "Can2SubNeedleLiquidDetectCommon.h"

//检测探液板是否存在
LH_ERR Can2SubNeedleSampleLiquidDetectCheckExist(void);

//读取探液板检测阈值
LH_ERR Can2SubNeedleSampleLiquidDetectReadThresholdValue(uint16_t* paramReadPtr);

//读取探液板阈值检测次数
LH_ERR Can2SubNeedleSampleLiquidDetectReadThresholdCount(uint16_t* paramReadPtr);

//读取探液板斜率检测值
LH_ERR Can2SubNeedleSampleLiquidDetectReadSlopeValue(uint16_t* paramReadPtr);

//读取探液板斜率检测次数
LH_ERR Can2SubNeedleSampleLiquidDetectSlopeCount(uint16_t* paramReadPtr);

//读取探液板信号保持时间
LH_ERR Can2SubNeedleSampleLiquidDetectSignalHoldTimeMs(uint16_t* paramReadPtr);

//读取探液板电子电阻值
LH_ERR Can2SubNeedleSampleLiquidDetectResValue(uint16_t* paramReadPtr);

//写入电子电阻值
LH_ERR Can2SubNeedleSampleLiquidDetectWriteResValue(uint16_t paramWrite);

//执行探液板校准
LH_ERR Can2SubNeedleSampleLiquidDetectAdjust(void);

//打开探液功能
LH_ERR Can2SubNeedleSampleLiquidDetectEnableFunc(void);

//关闭探液功能
LH_ERR Can2SubNeedleSampleLiquidDetectDisableFunc(void);

#endif




