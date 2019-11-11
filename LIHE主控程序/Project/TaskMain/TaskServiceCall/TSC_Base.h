#ifndef __TSC_BASE_H_
#define __TSC_BASE_H_
#include "BoardInc.h"
#include "os.h"
#include "TaskServiceConfig.h"
#include "ActionCommonBase.h"

//系统内部指令处理的阶段定义
typedef enum TSC_CMD_STATE
{
    TSC_CMD_STATE_IDLE = 0,        //空闲
    TSC_CMD_STATE_WAIT_ACK = 1,    //等待ACK
    TSC_CMD_STATE_WAIT_RETURN = 2, //等待结果
    TSC_CMD_STATE_COMPLETE = 3,//执行完成
}TSC_CMD_STATE;

//系统动作调用API,分为三种 发送动作并等待ACK(ACK无错误代表动作开始执行)  发送动作并等待结果(无错误代表动作执行完成)  读取上一次动作的结果
//如果调用了发送动作并等待ACK,那么同一个模块在下次发送动作之前必须读取上一次的动作结果,否则后一次调用动作报错.


//各个动作机构的数据结构
typedef struct TSC_CMD_RESULT_DATA_BEAD_MIX
{
    TSC_CMD_STATE status; //指令的状态
    LH_ERR resultValue;//指令结果
    OS_MUTEX* dataMutexPtr;//指令数据域操作互斥信号量
}TSC_CMD_RESULT_DATA_BEAD_MIX;

typedef struct TSC_CMD_RESULT_DATA_DISH_REACTION_REAGENT_MIX
{
    TSC_CMD_STATE status; //指令的状态
    LH_ERR resultValue;//指令结果
    OS_MUTEX* dataMutexPtr;//指令数据域操作互斥信号量
    uint8_t currentReactionHoleIndex;//当前孔位的序号
    ACTION_SENSOR_STATUS currentHoleHasCup;//当前孔位是否有杯
}TSC_CMD_RESULT_DATA_DISH_REACTION_REAGENT_MIX;

typedef struct TSC_CMD_RESULT_DATA_DISH_REAGENT
{
    TSC_CMD_STATE status; //指令的状态
    LH_ERR resultValue;//指令结果
    OS_MUTEX* dataMutexPtr;//指令数据域操作互斥信号量
    uint8_t currentHoleIndex;//当前孔位
    DISH_REAGENT_OFFSET offsetFlag;//当前相对偏移
}TSC_CMD_RESULT_DATA_DISH_REAGENT;

typedef struct TSC_CMD_RESULT_DATA_DISH_WASH_LIQUID_A_MIX
{
    TSC_CMD_STATE status; //指令的状态
    LH_ERR resultValue;//指令结果
    OS_MUTEX* dataMutexPtr;//指令数据域操作互斥信号量
    uint8_t currentHoleIndex;//当前孔位的序号
    ACTION_SENSOR_STATUS currentHoleHasCup;//当前孔位是否有杯
}TSC_CMD_RESULT_DATA_DISH_WASH_LIQUID_A_MIX;

typedef struct TSC_CMD_RESULT_DATA_HAND_CUP
{
    TSC_CMD_STATE status; //指令的状态
    LH_ERR resultValue;//指令结果
    OS_MUTEX* dataMutexPtr;//指令数据域操作互斥信号量
    ACTION_SENSOR_STATUS dishReactionHasCup;
    ACTION_SENSOR_STATUS dishWashHasCup;
}TSC_CMD_RESULT_DATA_HAND_CUP;

typedef struct TSC_CMD_RESULT_DATA_LIQUID_INJECT_A
{
    TSC_CMD_STATE status; //指令的状态
    LH_ERR resultValue;//指令结果
    OS_MUTEX* dataMutexPtr;//指令数据域操作互斥信号量
}TSC_CMD_RESULT_DATA_LIQUID_INJECT_A;

typedef struct TSC_CMD_RESULT_DATA_MEASURE_ROOM_LIQUID_B
{
    TSC_CMD_STATE status; //指令的状态
    LH_ERR resultValue;//指令结果
    OS_MUTEX* dataMutexPtr;//指令数据域操作互斥信号量
    uint32_t lastLightSensorReadDark;//上次测量室暗计数
    uint32_t lastLightSensorReadBackGround;//上次测量室本底
    uint32_t lastLightSensorReadValue;//上一次测量室读值读到的数据
}TSC_CMD_RESULT_DATA_MEASURE_ROOM_LIQUID_B;

typedef struct TSC_CMD_RESULT_DATA_NEEDLE_REAGENT_BEAD
{
    TSC_CMD_STATE status; //指令的状态
    LH_ERR resultValue;//指令结果
    OS_MUTEX* dataMutexPtr;//指令数据域操作互斥信号量
    TSC_NEEDLE_REAGENT_BEAD_POS currentNeedlePosition;//当前针的位置
    uint16_t currentPumpLiquidUl;//当前针注射泵内的液体微升数
}TSC_CMD_RESULT_DATA_NEEDLE_REAGENT_BEAD;

typedef struct TSC_CMD_RESULT_DATA_NEEDLE_SAMPLE
{
    TSC_CMD_STATE status; //指令的状态
    LH_ERR resultValue;//指令结果
    OS_MUTEX* dataMutexPtr;//指令数据域操作互斥信号量
    TSC_NEEDLE_SAMPLE_POS currentNeedlePosition;//当前样本针的位置
    uint16_t currentPumpLiquidUl;//当前样本针注射泵内的液体微升数
}TSC_CMD_RESULT_DATA_NEEDLE_SAMPLE;

typedef struct TSC_CMD_RESULT_DATA_NEEDLE_WASH
{
    TSC_CMD_STATE status; //指令的状态
    LH_ERR resultValue;//指令结果
    OS_MUTEX* dataMutexPtr;//指令数据域操作互斥信号量
}TSC_CMD_RESULT_DATA_NEEDLE_WASH;

typedef struct TSC_CMD_RESULT_DATA_QR_CODE_SCAN
{
    TSC_CMD_STATE status; //指令的状态
    LH_ERR resultValue;//指令结果
    OS_MUTEX* dataMutexPtr;//指令数据域操作互斥信号量
    uint16_t lastQRCodeLength;//上次接收到的二维码的长度
    uint8_t qrCodeDataBuffer[400];//二维码的数据缓冲区
}TSC_CMD_RESULT_DATA_QR_CODE_SCAN;

typedef struct TSC_CMD_RESULT_DATA_RACK_PATH_WAY
{
    TSC_CMD_STATE status; //指令的状态
    LH_ERR resultValue;//指令结果
    OS_MUTEX* dataMutexPtr;//指令数据域操作互斥信号量
    uint8_t currentTubeIndex;//当前试管位的序号
}TSC_CMD_RESULT_DATA_RACK_PATH_WAY;

//杯栈管理API交互信息结构体,后期可能扩展
typedef struct TSC_CMD_RESULT_DATA_STACK_MANAGE
{
    TSC_CMD_STATE status; //指令的状态
    LH_ERR resultValue;//指令结果
    OS_MUTEX* dataMutexPtr;//指令数据域操作互斥信号量
    uint8_t newDiskCount;//当前系统新杯盘数量
    uint8_t emptyDiskCount;//当前系统空杯盘数量
    ACTION_SENSOR_STATUS platformNewDiskExist;//新盘装载平台是否有盘
    ACTION_SENSOR_STATUS platformDiskWaitTestExist;//新盘测试平台是否有盘
    ACTION_SENSOR_STATUS garbage1Exist;//垃圾桶1是否存在
    ACTION_SENSOR_STATUS garbage2Exist;//垃圾桶2是否存在
    MACHINE_DOOR_STATUS stackManageDoorStatus;//杯栈管理门是否打开
}TSC_CMD_RESULT_DATA_STACK_MANAGE;






#endif




