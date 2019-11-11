#ifndef __TSC_MEASURE_ROOM_AND_LIQUID_B_H_
#define __TSC_MEASURE_ROOM_AND_LIQUID_B_H_
#include "TSC_Base.h"

typedef enum TSC_MEASURE_ROOM_CMD_INDEX
{
    TSC_MEASURE_ROOM_INIT = 0,
    TSC_MEASURE_ROOM_UPDOOR_OPEN_FULL = 1,
    TSC_MEASURE_ROOM_UPDOOR_OPEN_HALF = 2,
    TSC_MEASURE_ROOM_UPDOOR_CLOSE = 3,
    TSC_MEASURE_ROOM_PRIME_ONCE = 4,
    TSC_MEASURE_ROOM_INJECT_ONCE = 5,
    TSC_MEASURE_ROOM_READ_VALUE_WITH_INJECT = 6,
    TSC_MEASURE_ROOM_READ_VALUE_WITHOUT_INJECT = 7,
    TSC_MEASURE_ROOM_WINDOW_CLOSE = 8,
    TSC_MEASURE_ROOM_READ_VALUE = 9,
}TSC_MEASURE_ROOM_CMD_INDEX;

/***********************************************测量室与注B液动作****************************************************************/
//测量室与B液注液上一次的执行状态
TSC_CMD_STATE TSC_MeasureRoomGetLastCommandStatusAndResult(LH_ERR* errCode);

//测量室与B液注液数据结构初始化
void TSC_MeasureRoomResultDataInit(void);

//测量室初始化
LH_ERR TSC_MeasureRoomInitWhileAck(void);

//测量室上门全开,打开到可以取杯的位置
LH_ERR TSC_MeasureRoomUpdoorOpenFullWhileAck(void);

//测量室上门半开,打开到可以放杯的位置
LH_ERR TSC_MeasureRoomUpDoorOpenHalfWhileAck(void);

//测量室上门关闭
LH_ERR TSC_MeasureRoomUpDoorCloseWhileAck(void);

//测量室B液灌注一次
LH_ERR TSC_MeasureRoomPrimeOnceWhileAck(void);

//测量室B液注液一次
LH_ERR TSC_MeasureRoomInjectOnceWhileAck(void);

//测量室读值,读值之前自动注液,读取的值在执行完成之后调用模块的函数查询
LH_ERR TSC_MeasureRoomReadValueWithInjectWhileAck(uint16_t measureTimeMs);

//测量室读值,读数据之前不注液,读取的值在执行完成之后调用模块的函数查询
LH_ERR TSC_MeasureRoomReadValueWithOutInjectWhileAck(uint16_t measureTimeMs);

//测量窗关闭
LH_ERR TSC_MeasureRoomWindowsCloseWhileAck(void);


LH_ERR TSC_MeasureRoomWindowsMeasureBackGroundAndValue(uint16_t measureTimeMs,MEASURE_VALUE_FLAG injectFlag);
/***********************************************测量室与注B液动作****************************************************************/


/*********************************************动作结果中的数据读取***************************************************/


//读取上次的测光指令的测光值
uint32_t  TSC_MeasureRoomReadLastLightResult(void);
//读取上次暗计数
uint32_t  TSC_MeasureRoomReadLastDarkResult(void);
//读取上次本底
uint32_t  TSC_MeasureRoomReadLastGroundResult(void);




#endif
