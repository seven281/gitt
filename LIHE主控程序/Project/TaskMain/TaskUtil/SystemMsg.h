#ifndef __SYSTEM_MSG_H_
#define __SYSTEM_MSG_H_
#include "BoardInc.h"

//握手测试模式
typedef enum HAND_SHAKE_SERVICE_MODE
{
    HAND_SHAKE_SERVICE_MODE_DEBUG = 0,//调试模式,液路不动
    HAND_SHAKE_SERVICE_MODE_RELEASE = 1,//测试模式,液路正常运转
}HAND_SHAKE_SERVICE_MODE;

//试剂盘相对偏移
typedef enum OFFSET_DISH_REAGENT
{
    OFFSET_DISH_REAGENT_NONE,//无偏移
    OFFSET_DISH_REAGENT_QRSCAN,//二维码扫码位偏移
    OFFSET_DISH_REAGENT_R1_WITH_NEEDLE_REAGENT,//相对试剂针的R1偏移
    OFFSET_DISH_REAGENT_R2_WITH_NEEDLE_REAGENT,//相对试剂针的R2偏移
    OFFSET_DISH_REAGENT_BEAD_WITH_NEEDLE_REAGENT,//相对试剂针的磁珠位偏移
    OFFSET_DISH_REAGENT_R1_WITH_NEEDLE_BEAD,//相对磁珠针的R1偏移
    OFFSET_DISH_REAGENT_R2_WITH_NEEDLE_BEAD,//相对磁珠针的R2偏移
    OFFSET_DISH_REAGENT_BEAD_WITH_NEEDLE_BEAD,//相对磁珠针的磁珠位偏移
}OFFSET_DISH_REAGENT;

//新杯栈门状态
typedef enum DOOR_STATE_CUP_MANAGEMENT
{
    DOOR_STATE_CUP_MANAGEMENT_CLOSED,//门关闭状态
    DOOR_STATE_CUP_MANAGEMENT_OPENED,//门打开状态
    DOOR_STATE_CUP_MANAGEMENT_WAIT,//等待用户操作状态
}DOOR_STATE_CUP_MANAGEMENT;


//样本针的旋转位置
typedef enum TSC_NEEDLE_SAMPLE_POS
{
    TSC_NEEDLE_SAMPLE_POS_CLEAN = 0,//清洗位
    TSC_NEEDLE_SAMPLE_POS_FORCE_CLEAN = 1,//强清洗位
    TSC_NEEDLE_SAMPLE_POS_RACK = 2,//试管架位
    TSC_NEEDLE_SAMPLE_POS_REACTION_INJECT = 3,//反应盘注液位
    TSC_NEEDLE_SAMPLE_POS_REACTION_ABSORB = 4,//反应盘吸稀释样本位
}TSC_NEEDLE_SAMPLE_POS;

//样本新旧
typedef enum SAMPLE_STATUS
{
    SAMPLE_NEW=0,
    SAMPLE_USED,
}SAMPLE_STATUS;

//当前磁珠试剂针的旋转位置
typedef enum TSC_NEEDLE_REAGENT_BEAD_POS
{
    TSC_NEEDLE_REAGENT_BEAD_POS_CLEAN = 0,//清洗位
    TSC_NEEDLE_REAGENT_BEAD_POS_R1 = 1,//试剂R1位置
    TSC_NEEDLE_REAGENT_BEAD_POS_R2 = 2,//试剂R2位置
    TSC_NEEDLE_REAGENT_BEAD_POS_BEAD = 3,//磁珠位置
    TSC_NEEDLE_REAGENT_BEAD_POS_REACTION = 4,//反应盘上
}TSC_NEEDLE_REAGENT_BEAD_POS;

//清洗机械手当前的位置
typedef enum OFFSET_ROTATE_HAND_WASH
{
    OFFSET_ROTATE_HAND_WASH_UNKNOW,//未知位置
    OFFSET_ROTATE_HAND_WASH_DISH_REACTION,//反应盘
    OFFSET_ROTATE_HAND_WASH_MEASURE_ROOM,//测量室
    OFFSET_ROTATE_HAND_WASH_DISH_WASH,//清洗盘
}OFFSET_ROTATE_HAND_WASH;

/*废液桶主状态 */
typedef enum WASTE_TANK_MAIN_STATE
{
    WASTE_TANK_MAIN_STATE_NORMAL,//废液桶正常运行
    WASTE_TANK_MAIN_STATE_ERR,//废液桶运行异常
}WASTE_TANK_MAIN_STATE;

/*废液桶子状态 */
typedef enum WASTE_TANK_SUB_STATE
{
    WASTE_TANK_SUB_STATE_NORMAL_EMPTY_STOP,//废液桶空,停止抽废液状态
    WASTE_TANK_SUB_STATE_NORMAL_MID_STOP,//废液桶中间液量,停止抽废液状态
    WASTE_TANK_SUB_STATE_NORMAL_MID_RUNNING,//废液桶中间液量,正在抽废液液量
    WASTE_TANK_SUB_STATE_NORMAL_FULL_RUNNING,//废液桶满,正在抽废液状态
    WASTE_TANK_SUB_STATE_ERR_FULL_TIME_LIMIT,//废液桶从满到不满的超时
    WASTE_TANK_SUB_STATE_ERR_EMPTY_TIME_LIMIT,//废液桶从不满到空的超时
    WASTE_TANK_SUB_STATE_ERR_SENSOR_LOGIC,//废液桶传感器逻辑错误
}WASTE_TANK_SUB_STATE;

/*新杯机械手当前的平面位置 */
typedef enum HAND_NEW_CUP_POS_NOW
{
    HAND_NEW_CUP_POS_NEW,//新杯区
    HAND_NEW_CUP_POS_REACTION,//反应盘
    HAND_NEW_CUP_POS_GARBAGE1,//垃圾桶1
    HAND_NEW_CUP_POS_GARBAGE2,//垃圾桶2
}HAND_NEW_CUP_POS_NOW;


//设备的门的状态打开还是关闭
typedef enum MACHINE_DOOR_STATUS
{
    MACHINE_DOOR_STATUS_CLOSE,//门关闭
    MACHINE_DOOR_STATUS_OPEN,//门打开
    MACHINE_DOOR_STATUS_WAIT,//门等待
}MACHINE_DOOR_STATUS;

//试剂盘偏移
typedef enum DISH_REAGENT_OFFSET
{
    DISH_REAGENT_OFFSET_NONE,//无偏移
    DISH_REAGENT_OFFSET_SCAN,//扫描偏移
    DISH_REAGENT_OFFSET_R1,//试剂1偏移
    DISH_REAGENT_OFFSET_R2,//试剂2偏移
    DISH_REAGENT_OFFSET_BEAD,//磁珠偏移
}DISH_REAGENT_OFFSET;
//试剂盘对应杯状态
typedef enum DISH_REAGENT_CUP_STATUS
{
    DISH_REAGENT_CUP_UNINIT=0,//还没有获得试剂杯状态，初始为0不确定
    DISH_REAGENT_CUP_OK,      //试剂杯中有试剂  
    DISH_REAGENT_CUP_EMPTY,   //试剂杯中无试剂
    DISH_REAGENT_CUP_NG,      //试剂针有故障（目前没有）  
}DISH_REAGENT_CUP_STATUS;
//试剂盘具体杯
typedef struct ONE_OF_CUPSTATUS
{
    uint8_t cupholder;  //杯架
    uint8_t cups;       //杯号
}ONE_OF_CUPSTATUS;
//系统底层服务信息的子信息
typedef struct UTIL_SERVICE_MSG_SUB_OTHER
{
    HAND_SHAKE_SERVICE_MODE handShakeServiceMode;
    //当前软件版本
    uint32_t currentSoftVersion;
}UTIL_SERVICE_MSG_SUB_OTHER;

//反应盘信息
typedef struct UTIL_SERVICE_MSG_DISH_REACTION
{
    uint16_t currentCupIndex;//当前杯位序号
}UTIL_SERVICE_MSG_DISH_REACTION;

//试剂盘信息
typedef struct UTIL_SERVICE_MSG_DISH_REAGENT
{
    uint8_t currentCupHoleIndex;//当前杯位序号
    DISH_REAGENT_OFFSET offset;//当前偏移
}UTIL_SERVICE_MSG_DISH_REAGENT;

//清洗盘信息
typedef struct UTIL_SERVICE_MSG_DISH_WASH
{
    uint8_t currentCupHoleIndex;
}UTIL_SERVICE_MSG_DISH_WASH;

//试管架传送带的试管号
typedef struct UTIL_SERVICE_MSG_RANK_PATHWAY
{
    uint8_t testTubeIndex;
}UTIL_SERVICE_MSG_RANK_PATHWAY;

//杯栈管理状态
typedef struct  UTIL_SERVICE_MSG_CUPSTACK_MANAGEMENT
{
    MACHINE_DOOR_STATUS doorState;//杯栈门状态
    uint8_t newStackCount;//新杯盘数量
    uint8_t emptyStackCount;//空杯盘数量
}UTIL_SERVICE_MSG_CUPSTACK_MANAGEMENT;

//样本针状态
typedef struct UTIL_SERVICE_MSG_NEEDLE_SAMPLE
{
    TSC_NEEDLE_SAMPLE_POS offsetRotate;//当前旋转位置
    uint16_t pumpUsedUl;//当前泵中使用量,单位微升
    int32_t needlestepsoffaststop;//样本针快速下降停止点
    int32_t needlestepsofstops;//样本针移动直到液面探测报警时坐标
}UTIL_SERVICE_MSG_NEEDLE_SAMPLE;

//磁珠试剂针状态
typedef struct UTIL_SERVICE_MSG_NEEDLE_REAGENT
{
    TSC_NEEDLE_REAGENT_BEAD_POS offsetRotate;//当前旋转位置
    uint16_t pumpUsedUl;//当前泵中使用量,单位微升
}UTIL_SERVICE_MSG_NEEDLE_REAGENT;



//清洗机械手状态
typedef struct UTIL_SERVICE_MSG_HAND_WASH
{
    OFFSET_ROTATE_HAND_WASH offsetRotate;
}UTIL_SERVICE_MSG_HAND_WASH;

//液路单元当前状态
typedef struct UTIL_SERVICE_MSG_LIQUID_AUTO
{
    WASTE_TANK_MAIN_STATE mainState;//废液桶主状态
    WASTE_TANK_SUB_STATE subState;//废液桶子状态
}UTIL_SERVICE_MSG_LIQUID_AUTO;

//新杯机械手当前状态
typedef struct UTIL_SERVICE_HAND_NEW_CUP
{
    HAND_NEW_CUP_POS_NOW handNewCupNow;//新杯机械手当前平面位置
}UTIL_SERVICE_HAND_NEW_CUP;

//系统底层服务状态
typedef struct UTIL_SERVICE_MSG
{
    UTIL_SERVICE_MSG_SUB_OTHER otherMsg;//其他信息
    UTIL_SERVICE_MSG_DISH_REACTION dishReactionMsg;//反应盘讯息
    UTIL_SERVICE_MSG_DISH_REAGENT dishReagentMsg;//试剂盘讯息
    UTIL_SERVICE_MSG_DISH_WASH dishWashMsg;//清洗盘信息
    UTIL_SERVICE_MSG_RANK_PATHWAY rankPathwayMsg;//试管架传送带信息
    UTIL_SERVICE_MSG_CUPSTACK_MANAGEMENT cupStackManagementMsg;//杯栈管理信息
    UTIL_SERVICE_MSG_NEEDLE_SAMPLE needleSampleMsg;//样本针信息
    UTIL_SERVICE_MSG_NEEDLE_REAGENT needleReagentMsg;//试剂针信息
//    UTIL_SERVICE_MSG_NEEDLE_BEAD needleBeadMsg;//磁珠针信息
    UTIL_SERVICE_MSG_HAND_WASH handWashMsg;//清洗机械手信息
    UTIL_SERVICE_MSG_LIQUID_AUTO liquidAutoMsg;//液路自动单元信息
    UTIL_SERVICE_HAND_NEW_CUP handNewCupMsg;//新杯机械手单元状态
}UTIL_SERVICE_MSG;


typedef struct REAGENT_CUP_STATUS    //每个杯的状态
{
    DISH_REAGENT_CUP_STATUS status;//试管的状态     
    int32_t steps; //试剂针到液面的总步数
    uint8_t utilization;//使用率100%：代表杯中的液体满格

}REAGENT_CUP_STATUS;

typedef struct REAGENT_STATUS
{
    REAGENT_CUP_STATUS reagentcup[3];//每个试管架有3个杯
}REAGENT_STATUS;


typedef struct REAGENT_DISH_MSG
{
    REAGENT_STATUS reagent[30];//共30个试管架
}REAGENT_DISH_MSG;


//初始化系统服务讯息
LH_ERR TaskUtilServiceMsgInit(void);

//读取系统测试模式
HAND_SHAKE_SERVICE_MODE TaskUtilServiceGetHandShakeServiceMode(void);

//写入系统测试模式
void TaskUtilServiceSetHandShakeServiceMode(HAND_SHAKE_SERVICE_MODE shakeMode);

//读取当前版本号
uint32_t TaskUtilServiceGetSoftwareVersion(void);

//写入反应盘当前杯位序号
void TaskUtilServiceSetDishReactionCurrentCupIndex(uint16_t index);

//读取反应盘当前杯位序号
uint16_t TaskUtilServiceGetDishReactionCurrentCupIndex(void);

//写入试剂盘当前杯位序号
void TaskUtilServiceSetDishReagentCurrentCupIndex(uint8_t index);

//读取试剂盘当前杯位序号
uint8_t TaskUtilServiceGetDishReagentCurrentCupIndex(void);

//写入试剂盘当前偏移
void TaskUtilServiceSetDishReagentCurrentOffset(DISH_REAGENT_OFFSET offset);

//读取试剂盘当前杯位偏移
DISH_REAGENT_OFFSET TaskUtilServiceGetDishReagentCurrentOffset(void);

//读取清洗盘当前杯号
uint8_t TaskUtilServiceGetDishWashCurrentCupIndex(void);

//写入清洗盘当前杯号
void TaskUtilServiceSetDishWashCurrentCupIndex(uint8_t index);

//获取试管架传送带当前试管号
uint8_t TaskUtilServiceGetRankPathWayTestTubeIndex();

//设置试管架传送带当前试管号
void TaskUtilServiceSetRankPathWayTestTubeIndex(uint8_t index);

//获取当前杯栈管理新杯盘数量
uint8_t TaskUtilServiceGetCupStackManagementNewStackCount();

//设置当前杯栈管理新杯盘数量
void TaskUtilServiceSetCupStackManagementNewStackCount(uint8_t newStackCount);

//获取当前杯栈管理空杯盘数量
uint8_t TaskUtilServiceGetCupStackManagementEmptyStackCount();

//设置当前杯栈管理空杯盘数量
void TaskUtilServiceSetCupStackManagementEmptyStackCount(uint8_t emptyStackCount);

//获取当前杯栈管理门状态
MACHINE_DOOR_STATUS TaskUtilServiceGetCupStackManagementDoorState();

//设置当前杯栈管理门状态
void TaskUtilServiceSetCupStackManagementDoorState(MACHINE_DOOR_STATUS doorState);

//获取当前样本针旋转位置
TSC_NEEDLE_SAMPLE_POS TaskUtilServiceGetNeedleSampleRotatePos(void);

//设置样本针旋转位置
void TaskUtilServiceSetNeedleSampleRotatePos(TSC_NEEDLE_SAMPLE_POS offset);

//获取当前样本针注射泵内部存储量
uint16_t TaskUtilServiceGetNeedleSamplePumpUesdUl(void);

//设置当前样本针注射泵内部存储量
void TaskUtilServiceSetNeedleSamplePumpUesdUl(uint16_t ul);

//设置样本针快速下降停止坐标
void TaskUtilServiceSetNeedleSamplePumpStopPos(int32_t pos);

//获取样本针快速下降停止坐标
int32_t TaskUtilServiceGetNeedleSamplePumpStopPos(void);

//设置样本针液面探测的坐标
void TaskUtilServiceSetNeedleSamplePumpAlarmPos(int32_t pos);

//获取样本针颜面探测的坐标
int32_t TaskUtilServiceGetNeedleSamplePumpAlarmPos(void);

//获取当前试剂针旋转位置
TSC_NEEDLE_REAGENT_BEAD_POS TaskUtilServiceGetNeedleReagentRotatePos(void);

//设置当前试剂针旋转位置
void TaskUtilServiceSetNeedleReagentRotatePos(TSC_NEEDLE_REAGENT_BEAD_POS offset);

//获取当前试剂针注射泵内部存储量
uint16_t TaskUtilServiceGetNeedleReagentPumpUesdUl(void);

//设置当前试剂针注射泵内部存储量
void TaskUtilServiceSetNeedleReagentPumpUesdUl(uint16_t ul);

// //获取当前磁珠针旋转位置
// OFFSET_ROTATE_NEEDLE_BEAD TaskUtilServiceGetNeedleBeadRotatePos(void);

// //设置当前磁珠针旋转位置
// void TaskUtilServiceSetNeedleBeadRotatePos(OFFSET_ROTATE_NEEDLE_BEAD offset);

// //获取当前磁珠针注射泵内部存储量
// uint16_t TaskUtilServiceGetNeedleBeadPumpUesdUl(void);

// //设置当前磁珠针注射泵内部存储量
// void TaskUtilServiceSetNeedleBeadPumpUesdUl(uint16_t ul);

//设置清洗机械手当前位置
void TaskUtilServiceSetHandWashCupRotateOffset(OFFSET_ROTATE_HAND_WASH offset);

//获取清洗机械手当前位置
OFFSET_ROTATE_HAND_WASH TaskUtilServiceGetHandWashCupRotateOffset();

//废液桶的状态设置
void TaskUtilServiceSetLiquidAutoWasteTankState(WASTE_TANK_MAIN_STATE mainState,WASTE_TANK_SUB_STATE subState);

//废液桶的状态获取
void TaskUtilServiceGetLiquidAutoWasteTankState(WASTE_TANK_MAIN_STATE* mainStatePtr,WASTE_TANK_SUB_STATE* subStatePtr);


//获取当前新杯机械手平面位置
HAND_NEW_CUP_POS_NOW TaskUtilServiceGetHandNewCupCurrentPos(void);

//设置当前新杯机械手皮平面位置
void TaskUtilServiceSetHandNewCupCurrentPos(HAND_NEW_CUP_POS_NOW nowPos);

//获取某个试剂杯状态，以及对应液面探测步数
REAGENT_CUP_STATUS* TaskUtilServiceGetDishReagentCupState(ONE_OF_CUPSTATUS *oneofcups);

//设置试剂杯状态，以及对应步数
void TaskUtilServiceSetDishReagentCupState(ONE_OF_CUPSTATUS *oneofcups,REAGENT_CUP_STATUS *cupstatus);





#endif


