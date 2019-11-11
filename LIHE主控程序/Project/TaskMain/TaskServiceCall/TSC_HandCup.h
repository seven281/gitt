#ifndef __TSC_HAND_CUP_H_
#define __TSC_HAND_CUP_H_
#include "TSC_Base.h"


typedef enum TSC_HAND_CUP_CMD_INDEX
{
    TSC_HAND_CUP_INIT = 0,
    TSC_HAND_CUP_MOVE_2_DISH_REACTION = 1,
    TSC_HAND_CUP_MOVE_2_NEW_DISK = 2,
    TSC_HAND_CUP_MOVE_2_GARBAGE = 3,
    TSC_HAND_CUP_MOVE_2_DISH_WASH = 4,
    TSC_HAND_CUP_MOVE_2_MEASURE_ROOM = 5,
    TSC_HAND_CUP_PUT_CUP_2_DISH_REACTION = 6,
    TSC_HAND_CUP_PUT_CUP_2_GARBAGE = 7,
    TSC_HAND_CUP_CATCH_CUP_FROM_DISH_REACTION = 8,
    TSC_HAND_CUP_CATCH_CUP_FROM_NEW_DISK = 9,
    TSC_HAND_CUP_PUT_CUP_2_DISH_WASH = 10,
    TSC_HAND_CUP_CATCH_CUP_FROM_DISH_WASH = 11,
    TSC_HAND_CUP_PUT_CUP_2_MEASURE_ROOM = 12,
    TSC_HAND_CUP_CATCH_CUP_FROM_MEASURE_ROOM = 13,
}TSC_HAND_CUP_CMD_INDEX;

/******************************************************抓杯机械手动作************************************************************/
//查询抓杯机械手上一次的执行状态
TSC_CMD_STATE TSC_HandCupGetLastCommandStatusAndResult(LH_ERR* errCode);

//抓杯机械手数据结构初始化
void TSC_HandCupResultDataInit(void);

//抓杯机械手三维复位
LH_ERR TSC_HandCupInitWhileAck(void);

//抓杯机械手移动到反应盘
LH_ERR TSC_HandCupMove2DishReactionWhileAck(void);

//抓杯机械手移动到新盘装载上方,参数为第几行第几列的杯子
LH_ERR TSC_HandCupMove2NewDiskWhileAck(uint8_t newCupRow,uint8_t newCupCol);

//抓杯机械手移动到垃圾桶上方,参数为垃圾桶1(参数0)或者垃圾桶2(参数1)
LH_ERR TSC_HandCupMove2GarbageWhileAck(ACTION_PARAM_STACK_MANAGE_GARBAGE_NO garbageNo);

//抓杯机械手移动到清洗盘上方
LH_ERR TSC_HandCupMove2DishWashWhileAck(void);

//抓杯机械手移动到测量室上方
LH_ERR TSC_HandCupMove2MeasureRoomWhileAck(void);

//抓杯机械手往反应盘放杯
LH_ERR TSC_HandCupPutCup2DishReactionWhileAck(void);

//抓杯机械手往垃圾桶放杯 参数为垃圾桶编号,垃圾桶1或者垃圾桶2
LH_ERR TSC_HandCupPutCup2GarbageWhileAck(ACTION_PARAM_STACK_MANAGE_GARBAGE_NO garbageNo);

//抓杯机械手从反应盘抓杯
LH_ERR TSC_HandCupCatchCupFromDishReactionWhileAck(void);

//抓杯机械手从新盘上抓杯
LH_ERR TSC_HandCupCatchCupFromNewDiskWhileAck(void);

//抓杯机械手往清洗盘放杯
LH_ERR TSC_HandCupPutCup2DishWashWhileAck(void);

//抓杯机械手从清洗盘抓杯
LH_ERR TSC_HandCupCatchFromDishWashWhileAck(void);

//抓杯机械手往测量室放杯 放杯之前测量室需要半开
LH_ERR TSC_HandCupPutCup2MeasureRoomWhileAck(void);

//抓杯机械手从测量室抓杯,抓杯之前,测量室需要全开
LH_ERR TSC_HandCupCatchFromMeasureRoomWhileAck(void);

//后续会添加抓杯机械手的状态查询,例如反应盘是否有杯 清洗盘是否有杯等



/******************************************************抓杯机械手动作************************************************************/


/*********************************************动作结果中的数据读取***************************************************/

//检查反应盘是否有杯
ACTION_SENSOR_STATUS TSC_HandCheckDishReactionHasCup(void);

//检查清洗盘是否有杯
ACTION_SENSOR_STATUS TSC_HandCheckDishWashHasCup(void);


#endif
