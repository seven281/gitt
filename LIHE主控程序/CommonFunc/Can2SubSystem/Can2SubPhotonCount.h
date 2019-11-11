#ifndef __CAN2_SUB_PHOTON_COUNT_H_
#define __CAN2_SUB_PHOTON_COUNT_H_
#include "Can2ProcMain.h"

//光子测量单次测量开始
LH_ERR Can2SubPhotonCountSingleMeasureStart(uint16_t nms);

//获取光子单次测量的测量结果
LH_ERR Can2SubPhotonCountGetSingleMeasureResult(uint32_t* result,uint32_t timeOut);

//光子测量多次测量开始
LH_ERR Can2SubPhotonCountMultiMeasureStartWhileAck(uint16_t nms,uint16_t count);

//光子测量多次测量开始并等待结束
LH_ERR Can2SubPhotonCountMultiMeasureStartWhileReturn(uint16_t nms,uint16_t count);

//等待光子测量结束
LH_ERR Can2SubPhotonCountWaitLastReturn(uint32_t timeOutMs);

//光子测量获取多次测量结果
LH_ERR Can2SubPhotonCountGetMultiMeasureResult(uint16_t exceptCount,uint16_t* resultCount,uint32_t** resultArrayPtr);


#endif




