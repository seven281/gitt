#ifndef __ACTION_RANK_PATH_WAY_H_
#define __ACTION_RANK_PATH_WAY_H_
#include "ActionCommonBase.h"

//试管架传送带初始化
LH_ERR ActionRankPathWay_Init(uint8_t* tubeIndexResult);

//试管架传送带移动到指定试管位
LH_ERR ActionRankPathWay_Move2SpecialTube(uint16_t tubeIndex,uint8_t* tubeIndexResult);

//试管架传送带回退一个架子
LH_ERR ActionRankPathWay_BackOneRack(uint8_t* tubeIndexResult);


#endif
