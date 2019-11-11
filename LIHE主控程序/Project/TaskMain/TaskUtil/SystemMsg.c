#include "SystemMsg.h"
#include "string.h"
/*
utilServiceMsgPtr结构体存放的是辅助信息
当前执行机构的位置或者状态信息
*/
static UTIL_SERVICE_MSG* utilServiceMsgPtr = NULL;
static REAGENT_DISH_MSG* utilServiceReagentPtr = NULL;
static REAGENT_CUP_STATUS *ReagentStatus = NULL;
//初始化系统服务讯息
LH_ERR TaskUtilServiceMsgInit(void)
{
    //申请内存
    do
    {
        utilServiceMsgPtr = UserMemMalloc(SRAM_IS62,sizeof(UTIL_SERVICE_MSG)/sizeof(uint8_t));
        if(utilServiceMsgPtr == NULL)
        {
            CoreDelayMinTick();
        }
    } while (utilServiceMsgPtr == NULL);
    //申请试剂盘状态内存
    do
    {
        utilServiceReagentPtr = UserMemMalloc(SRAM_IS62,sizeof(REAGENT_DISH_MSG)/sizeof(uint8_t));
        if(utilServiceReagentPtr == NULL)
        {
            CoreDelayMinTick();
        }
    } while (utilServiceReagentPtr == NULL);
    //申请试剂盘上单个试剂杯的状态
    do
    {
        ReagentStatus = UserMemMalloc(SRAM_IS62,sizeof(REAGENT_CUP_STATUS)/sizeof(uint8_t));
        if(ReagentStatus == NULL)
        {
            CoreDelayMinTick();
        }
    } while (ReagentStatus == NULL);

    //初始化内部讯息
    //初始化默认正常模式,运行时上位机指定运行模式
    utilServiceMsgPtr->otherMsg.handShakeServiceMode = HAND_SHAKE_SERVICE_MODE_RELEASE;
    utilServiceMsgPtr->otherMsg.currentSoftVersion = 1000;

    //初始化反应盘信息
    utilServiceMsgPtr->dishReactionMsg.currentCupIndex = 31;

    //初始化试剂盘信息
    utilServiceMsgPtr->dishReagentMsg.currentCupHoleIndex = 1;
    utilServiceMsgPtr->dishReagentMsg.offset = DISH_REAGENT_OFFSET_NONE;

    //初始化清洗盘信息
    utilServiceMsgPtr->dishWashMsg.currentCupHoleIndex = 1;

    //试管架传送带试管号初始化
    utilServiceMsgPtr->rankPathwayMsg.testTubeIndex = 1;

    //杯栈管理信息初始化
    utilServiceMsgPtr->cupStackManagementMsg.newStackCount = 0;
    utilServiceMsgPtr->cupStackManagementMsg.emptyStackCount = 0;
    utilServiceMsgPtr->cupStackManagementMsg.doorState = MACHINE_DOOR_STATUS_OPEN;

    //样本针信息
    utilServiceMsgPtr->needleSampleMsg.offsetRotate = TSC_NEEDLE_SAMPLE_POS_CLEAN;
    utilServiceMsgPtr->needleSampleMsg.pumpUsedUl = 0;
    utilServiceMsgPtr->needleSampleMsg.needlestepsoffaststop = 0;
    utilServiceMsgPtr->needleSampleMsg.needlestepsofstops = 0;
    //磁珠试剂针信息
    utilServiceMsgPtr->needleReagentMsg.offsetRotate = TSC_NEEDLE_REAGENT_BEAD_POS_CLEAN;
    utilServiceMsgPtr->needleReagentMsg.pumpUsedUl = 0;

 

    //清洗机械手
    utilServiceMsgPtr->handWashMsg.offsetRotate = OFFSET_ROTATE_HAND_WASH_UNKNOW;

    //液路单元自动信息
    utilServiceMsgPtr->liquidAutoMsg.mainState = WASTE_TANK_MAIN_STATE_NORMAL;
    utilServiceMsgPtr->liquidAutoMsg.subState = WASTE_TANK_SUB_STATE_NORMAL_EMPTY_STOP;

    //新杯机械手
    utilServiceMsgPtr->handNewCupMsg.handNewCupNow = HAND_NEW_CUP_POS_NEW;
    
    memset(utilServiceReagentPtr,0,sizeof(REAGENT_DISH_MSG));

    return LH_ERR_NONE;
}

//获取当前新杯机械手平面位置
HAND_NEW_CUP_POS_NOW TaskUtilServiceGetHandNewCupCurrentPos(void)
{
    return utilServiceMsgPtr->handNewCupMsg.handNewCupNow;
}

//设置当前新杯机械手皮平面位置
void TaskUtilServiceSetHandNewCupCurrentPos(HAND_NEW_CUP_POS_NOW nowPos)
{
    utilServiceMsgPtr->handNewCupMsg.handNewCupNow = nowPos;
}


//读取系统测试模式
HAND_SHAKE_SERVICE_MODE TaskUtilServiceGetHandShakeServiceMode(void)
{
    return utilServiceMsgPtr->otherMsg.handShakeServiceMode;
}

//写入系统测试模式
void TaskUtilServiceSetHandShakeServiceMode(HAND_SHAKE_SERVICE_MODE shakeMode)
{
    utilServiceMsgPtr->otherMsg.handShakeServiceMode = shakeMode;
}

//读取当前版本号
uint32_t TaskUtilServiceGetSoftwareVersion(void)
{
    return utilServiceMsgPtr->otherMsg.currentSoftVersion;
}

//写入反应盘当前杯位序号
void TaskUtilServiceSetDishReactionCurrentCupIndex(uint16_t index)
{
    utilServiceMsgPtr->dishReactionMsg.currentCupIndex = index;
}

//读取反应盘当前杯位序号
uint16_t TaskUtilServiceGetDishReactionCurrentCupIndex(void)
{
    return utilServiceMsgPtr->dishReactionMsg.currentCupIndex;
}

//写入试剂盘当前杯位序号
void TaskUtilServiceSetDishReagentCurrentCupIndex(uint8_t index)
{
    utilServiceMsgPtr->dishReagentMsg.currentCupHoleIndex = index;
}

//读取试剂盘当前杯位序号
uint8_t TaskUtilServiceGetDishReagentCurrentCupIndex(void)
{
    return utilServiceMsgPtr->dishReagentMsg.currentCupHoleIndex;
}

//写入试剂盘当前偏移
void TaskUtilServiceSetDishReagentCurrentOffset(DISH_REAGENT_OFFSET offset)
{
    utilServiceMsgPtr->dishReagentMsg.offset = offset;
}

//读取试剂盘当前杯位偏移
DISH_REAGENT_OFFSET TaskUtilServiceGetDishReagentCurrentOffset(void)
{
    return utilServiceMsgPtr->dishReagentMsg.offset;
}

//读取清洗盘当前杯号
uint8_t TaskUtilServiceGetDishWashCurrentCupIndex(void)
{
    return utilServiceMsgPtr->dishWashMsg.currentCupHoleIndex;
}

//写入清洗盘当前杯号
void TaskUtilServiceSetDishWashCurrentCupIndex(uint8_t index)
{
    utilServiceMsgPtr->dishWashMsg.currentCupHoleIndex = index;
}

//获取试管架传送带当前试管号
uint8_t TaskUtilServiceGetRankPathWayTestTubeIndex()
{
    return utilServiceMsgPtr->rankPathwayMsg.testTubeIndex;
}

//设置试管架传送带当前试管号
void TaskUtilServiceSetRankPathWayTestTubeIndex(uint8_t index)
{
    utilServiceMsgPtr->rankPathwayMsg.testTubeIndex = index;
}

//获取当前杯栈管理新杯盘数量
uint8_t TaskUtilServiceGetCupStackManagementNewStackCount()
{
    return utilServiceMsgPtr->cupStackManagementMsg.newStackCount;
}

//设置当前杯栈管理新杯盘数量
void TaskUtilServiceSetCupStackManagementNewStackCount(uint8_t newStackCount)
{
    utilServiceMsgPtr->cupStackManagementMsg.newStackCount = newStackCount;
}

//获取当前杯栈管理空杯盘数量
uint8_t TaskUtilServiceGetCupStackManagementEmptyStackCount()
{
    return utilServiceMsgPtr->cupStackManagementMsg.emptyStackCount;
}

//设置当前杯栈管理空杯盘数量
void TaskUtilServiceSetCupStackManagementEmptyStackCount(uint8_t emptyStackCount)
{
    utilServiceMsgPtr->cupStackManagementMsg.emptyStackCount = emptyStackCount;
}

//获取当前杯栈管理门状态
MACHINE_DOOR_STATUS TaskUtilServiceGetCupStackManagementDoorState()
{
    return utilServiceMsgPtr->cupStackManagementMsg.doorState;
}

//设置当前杯栈管理门状态
void TaskUtilServiceSetCupStackManagementDoorState(MACHINE_DOOR_STATUS doorState)
{
    utilServiceMsgPtr->cupStackManagementMsg.doorState = doorState;
}


//获取当前样本针旋转位置
TSC_NEEDLE_SAMPLE_POS TaskUtilServiceGetNeedleSampleRotatePos(void)
{
    return utilServiceMsgPtr->needleSampleMsg.offsetRotate;
}

//设置样本针旋转位置
void TaskUtilServiceSetNeedleSampleRotatePos(TSC_NEEDLE_SAMPLE_POS offset)
{
    utilServiceMsgPtr->needleSampleMsg.offsetRotate = offset;
}

//获取当前样本针注射泵内部存储量
uint16_t TaskUtilServiceGetNeedleSamplePumpUesdUl(void)
{
    return utilServiceMsgPtr->needleSampleMsg.pumpUsedUl;
}

//设置当前样本针注射泵内部存储量
void TaskUtilServiceSetNeedleSamplePumpUesdUl(uint16_t ul)
{
    utilServiceMsgPtr->needleSampleMsg.pumpUsedUl = ul;
}
//设置样本针快速下降停止坐标
void TaskUtilServiceSetNeedleSamplePumpStopPos(int32_t pos)
{
    utilServiceMsgPtr->needleSampleMsg.needlestepsoffaststop = pos;
}
//获取样本针快速下降停止坐标
int32_t TaskUtilServiceGetNeedleSamplePumpStopPos(void)
{
    return utilServiceMsgPtr->needleSampleMsg.needlestepsoffaststop;
}
//设置样本针液面探测的坐标
void TaskUtilServiceSetNeedleSamplePumpAlarmPos(int32_t pos)
{
    utilServiceMsgPtr->needleSampleMsg.needlestepsofstops = pos;
}
//获取样本针颜面探测的坐标
int32_t TaskUtilServiceGetNeedleSamplePumpAlarmPos(void)
{
    return utilServiceMsgPtr->needleSampleMsg.needlestepsofstops;
}

//获取当前磁珠试剂针旋转位置
TSC_NEEDLE_REAGENT_BEAD_POS TaskUtilServiceGetNeedleReagentRotatePos(void)
{
    return utilServiceMsgPtr->needleReagentMsg.offsetRotate;
}

//设置当前磁珠试剂针旋转位置
void TaskUtilServiceSetNeedleReagentRotatePos(TSC_NEEDLE_REAGENT_BEAD_POS offset)
{
    utilServiceMsgPtr->needleReagentMsg.offsetRotate = offset;
}

//获取当前磁珠试剂针注射泵内部存储量
uint16_t TaskUtilServiceGetNeedleReagentPumpUesdUl(void)
{
    return utilServiceMsgPtr->needleReagentMsg.pumpUsedUl;
}

//设置当前磁珠试剂针注射泵内部存储量
void TaskUtilServiceSetNeedleReagentPumpUesdUl(uint16_t ul)
{
    utilServiceMsgPtr->needleReagentMsg.pumpUsedUl = ul;
}

// //获取当前磁珠针旋转位置
// OFFSET_ROTATE_NEEDLE_BEAD TaskUtilServiceGetNeedleBeadRotatePos(void)
// {
//     return utilServiceMsgPtr->needleBeadMsg.offsetRotate;
// }

// //设置当前磁珠针旋转位置
// void TaskUtilServiceSetNeedleBeadRotatePos(OFFSET_ROTATE_NEEDLE_BEAD offset)
// {
//     utilServiceMsgPtr->needleBeadMsg.offsetRotate = offset;
// }

// //获取当前磁珠针注射泵内部存储量
// uint16_t TaskUtilServiceGetNeedleBeadPumpUesdUl(void)
// {
//     return utilServiceMsgPtr->needleBeadMsg.pumpUsedUl;
// }

// //设置当前磁珠针注射泵内部存储量
// void TaskUtilServiceSetNeedleBeadPumpUesdUl(uint16_t ul)
// {
//     utilServiceMsgPtr->needleBeadMsg.pumpUsedUl = ul;
// }

//设置清洗机械手当前位置
void TaskUtilServiceSetHandWashCupRotateOffset(OFFSET_ROTATE_HAND_WASH offset)
{
    utilServiceMsgPtr->handWashMsg.offsetRotate = offset;
}

//获取清洗机械手当前位置
OFFSET_ROTATE_HAND_WASH TaskUtilServiceGetHandWashCupRotateOffset()
{
    return utilServiceMsgPtr->handWashMsg.offsetRotate;
}

//废液桶的状态设置
void TaskUtilServiceSetLiquidAutoWasteTankState(WASTE_TANK_MAIN_STATE mainState,WASTE_TANK_SUB_STATE subState)
{
    utilServiceMsgPtr->liquidAutoMsg.mainState = mainState;
    utilServiceMsgPtr->liquidAutoMsg.subState = subState;
}

//废液桶的状态获取
void TaskUtilServiceGetLiquidAutoWasteTankState(WASTE_TANK_MAIN_STATE* mainStatePtr,WASTE_TANK_SUB_STATE* subStatePtr)
{
    *mainStatePtr = utilServiceMsgPtr->liquidAutoMsg.mainState;
    *subStatePtr = utilServiceMsgPtr->liquidAutoMsg.subState;
}
//获取某个试剂杯状态，以及对应液面探测步数
//试剂架1-30   试剂杯1-3
//参数1：定位到具体哪个试剂杯
//return:返回该试剂杯的状态和步数--指针
REAGENT_CUP_STATUS* TaskUtilServiceGetDishReagentCupState(ONE_OF_CUPSTATUS *oneofcups)
{
    ReagentStatus->status = utilServiceReagentPtr->reagent[oneofcups->cupholder-1].reagentcup[oneofcups->cups-1].status; 
    ReagentStatus->steps = utilServiceReagentPtr->reagent[oneofcups->cupholder-1].reagentcup[oneofcups->cups-1].steps;
    ReagentStatus->utilization = utilServiceReagentPtr->reagent[oneofcups->cupholder-1].reagentcup[oneofcups->cups-1].utilization;
    return ReagentStatus;
}
//设置试剂杯状态，以及对应步数
//试剂架1-30   试剂杯1-3 
//参数1  定位到具体哪个试剂杯
//参数2  分别设置该试剂杯的状态和步数
void TaskUtilServiceSetDishReagentCupState(ONE_OF_CUPSTATUS *oneofcups,REAGENT_CUP_STATUS *cupstatus)
{
    utilServiceReagentPtr->reagent[oneofcups->cupholder-1].reagentcup[oneofcups->cups-1].status = cupstatus->status;
    utilServiceReagentPtr->reagent[oneofcups->cupholder-1].reagentcup[oneofcups->cups-1].steps = cupstatus->steps;
    utilServiceReagentPtr->reagent[oneofcups->cupholder-1].reagentcup[oneofcups->cups-1].utilization = cupstatus->utilization;
}




