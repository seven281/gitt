#ifndef __ACTION_HAND_CUP_H_
#define __ACTION_HAND_CUP_H_
#include "ActionCommonBase.h"

//机械手左右移动到反应盘速度选项
#define HAND_LEFT_RIGHT_TO_REACTION    SM_CURVE_0                  
//机械手左右移动到清洗盘速度选项
#define HAND_LEFT_RIGHT_TO_WASH        SM_CURVE_1
//机械手左右移动到测量室速度选项
#define HAND_LEFT_RIGHT_TO_MEASURE     SM_CURVE_2
//机械手左右移动到垃圾桶速度选项
#define HAND_LEFT_RIGHT_TO_TRASH       SM_CURVE_3
//机械手左右移动到新杯盘速度
#define HAND_LEFT_RIGHT_TO_NEW_CUP     SM_CURVE_4


//机械手没有抓杯子时快速度上下运行---通用
#define MACHINE_HAND_EMPTY              SM_CURVE_0
//机械手抓杯慢速上下运行-----------通用
#define MACHINE_HAND_CATCH              SM_CURVE_1
//机械手抓杯慢速上下运行（略慢）
#define MACHINE_HAND_SLOW_CATCH         SM_CURVE_2
//机械手抓杯慢速上下运行（略快）
#define MACHINE_HAND_FAST_CATCH         SM_CURVE_3
//机械手空快速上下运行(略快)
#define MACHINE_HAND_FAST_EMPTY         SM_CURVE_4
//机械手极限最快上下运行
#define MACHINE_HAND_THE_FAST           SM_CURVE_5


//新杯机械手升降复位
LH_ERR ActionHand_CupUpDownReset(void);

//新杯机械手左右复位
LH_ERR ActionHand_CupLeftRightReset(void);

//新杯机械手前后复位
LH_ERR ActionHand_CupFrontBackReset(void);

//新杯机械手水平复位
LH_ERR ActionHand_FlatReset(void);

//新杯机械手整体复位
LH_ERR ActionHand_SolidReset(void);

//机械手移动到反应盘
LH_ERR ActionHand_Move2DiskReaction(void);

//机械手移动到新杯区域
//行 列参数 行是前后 列是左右 14行 10列
LH_ERR ActionHand_Move2NewCupRegion(uint16_t row,uint16_t col);

//机械手移动到垃圾桶
LH_ERR ActionHand_Move2Garbage(ACTION_PARAM_STACK_MANAGE_GARBAGE_NO garbageNo);

//移动到清洗盘
LH_ERR ActionHand_Move2DiskWash(void);

//移动到测量室上方
LH_ERR ActionHand_Move2MeasureRoom(void);

//往反应盘放杯
LH_ERR ActionHand_PutCup2DiskReaction(ACTION_SENSOR_STATUS* diskReactionCupHasExist);

//往垃圾桶放杯
LH_ERR ActionHand_PutCup2Garbage(ACTION_PARAM_STACK_MANAGE_GARBAGE_NO garbageNo);

//从反应盘取杯
LH_ERR ActionHand_CatchCupFromDiskReaction(ACTION_SENSOR_STATUS* diskReactionCupHasExist);

//从新杯区取杯
LH_ERR ActionHand_CatchFromNewCupRegion(void);

//往清洗盘放杯
LH_ERR ActionHand_PutCup2DiskWash(ACTION_SENSOR_STATUS* diskWashCupHasExist);

//从清洗盘取杯
LH_ERR ActionHand_CatchFromDiskWash(ACTION_SENSOR_STATUS* diskWashCupHasExist);

//往测量室放杯
LH_ERR ActionHand_PutCup2MeasureRoom(void);

//从测量室取杯
LH_ERR ActionHand_CatchCupFromMeasureRoom(void);



#endif
