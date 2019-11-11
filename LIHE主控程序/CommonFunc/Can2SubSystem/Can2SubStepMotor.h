#ifndef __CAN2_SUB_STEP_MOTOR_H_
#define __CAN2_SUB_STEP_MOTOR_H_
#include "Can2SubStepMotorIndex.h"
#include "Can2ProcMain.h"

//步进电机支持的指令
typedef enum SM_CMD
{
    SM_CMD_RESET            = 0X00000000,//复位
    SM_CMD_RUN_STEPS        = 0X00000001,//运行指定步数
    SM_CMD_RUN_CORDINATE    = 0X00000002,//运行到指定位置
    SM_CMD_RETURN_ZERO      = 0X00000003,//回零
    SM_CMD_RUN_ALWAYS       = 0X00000004,//持续运转
    SM_CMD_STOP_IMME        = 0X00000005,//立即停止
    SM_CMD_STOP_DACC        = 0X00000006,//减速停止
    SM_CMD_READ_POS         = 0X00000007,//读取当前坐标
}SM_CMD;

//电机速度曲线选择
typedef enum SM_CURVE_SELECT
{
    SM_CURVE_0,
    SM_CURVE_1,
    SM_CURVE_2,
    SM_CURVE_3,
    SM_CURVE_4,
    SM_CURVE_5,
}SM_CURVE_SELECT;

//电机速度配比模式
typedef enum SM_SPEED_MODE
{
    //全局按照实际比例计算速度
    SPEED_MODE_GLOBAL,
    //启动速度不变,计算速度比例时,叠加启动速度和启动速度之上的速度比例
    SPEED_MODE_UP_START,
}SM_SPEED_MODE;

//步进电机复位表示
typedef struct SM_RESET_CMD
{
    //电机编号
    CAN2_SUB_SM_INDEX stepMotorIndex;
    //复位修正
    int32_t correctionPosition;
    //超时时间
    uint32_t timeOutMs;
}SM_RESET_CMD;

//步进电机运行指定步数
typedef struct SM_RUN_STEP_CMD
{
    //电机编号
    CAN2_SUB_SM_INDEX stepMotorIndex;
    //运行步数
    int32_t steps;
    //选择加减速曲线
    SM_CURVE_SELECT curveSelect;
    //速度比例
    uint8_t speedRatio;
    //速度模式
    SM_SPEED_MODE speedMode;
    //辅助急停1使能
    FunctionalState utilStop1Enable;
    //辅助急停2使能
    FunctionalState utilStop2Enable;
    //超时时间
    uint32_t timeOutMs;
}SM_RUN_STEP_CMD;

//运行到指定坐标
typedef struct SM_RUN_COORDINATE_CMD
{
    //电机编号
    CAN2_SUB_SM_INDEX stepMotorIndex;
    //目标坐标
    int32_t targetCoordinate;
    //选择加减速曲线
    SM_CURVE_SELECT curveSelect;
    //速度比例
    uint8_t speedRatio;
    //速度模式
    SM_SPEED_MODE speedMode;
    //辅助急停1使能
    FunctionalState utilStop1Enable;
    //辅助急停2使能
    FunctionalState utilStop2Enable;
    //超时时间
    uint32_t timeOutMs;
}SM_RUN_COORDINATE_CMD;

//回零
typedef struct SM_RETURN_ZERO_CMD
{
    //电机编号
    CAN2_SUB_SM_INDEX stepMotorIndex;
    //回零修正
    int32_t correctionPosition;
    //选择加减速曲线
    SM_CURVE_SELECT curveSelect;
    //速度比例
    uint8_t speedRatio;
    //速度模式
    SM_SPEED_MODE speedMode;
    //辅助急停1使能
    FunctionalState utilStop1Enable;
    //辅助急停2使能
    FunctionalState utilStop2Enable;
    //超时时间
    uint32_t timeOutMs;
}SM_RETURN_ZERO_CMD;

//持续运转
typedef struct SM_RUN_ALWAYS_CMD
{
    //电机编号
    CAN2_SUB_SM_INDEX stepMotorIndex;
    //选择加减速曲线
    SM_CURVE_SELECT curveSelect;
    //速度比例
    uint8_t speedRatio;
    //速度模式
    SM_SPEED_MODE speedMode;
}SM_RUN_ALWAYS_CMD;

/**********************************步进电机的参数初始化**********************************/

//复位指令的初始化
void Can2SubSM_ResetCmdDataStructInit(SM_RESET_CMD* cmdPtr);

//走步数指令的初始化
void Can2SubSM_RunStepsCmdDataStructInit(SM_RUN_STEP_CMD* cmdPtr);

//走坐标指令的初始化
void Can2SubSM_RunCoordinateDataStructInit(SM_RUN_COORDINATE_CMD* cmdPtr);

//回零指令的初始化
void Can2SubSM_ReturnZeroDataStructInit(SM_RETURN_ZERO_CMD* cmdPtr);

//持续运转指令的初始化
void Can2SubSM_RunAlwaysDataStructInit(SM_RUN_ALWAYS_CMD* cmdPtr);

/******************************************************************************************/

/***********************************************************发送电机指令并等待ACK返回********************************************************************/
//系统内部电机复位
//修正步数 超时时间
LH_ERR Can2SubSM_ResetWhileAck(SM_RESET_CMD* resetCmdPtr);

//系统内部电机执行执行步数
LH_ERR Can2SubSM_RunSpecialStepsWhileAck(SM_RUN_STEP_CMD* runStepsCmdPtr);

//系统内部电机运行到指定坐标
LH_ERR Can2SubSM_RunToCoordinateWhileAck(SM_RUN_COORDINATE_CMD* runCoordinateCmdPtr);

//系统内部电机回零
LH_ERR Can2SubSM_ReturnZeroWhileAck(SM_RETURN_ZERO_CMD* returnZeroCmdPtr);

//系统内部电机电机持续运转
LH_ERR Can2SubSM_RunAlwaysWhileAck(SM_RUN_ALWAYS_CMD* runAlwaysCmdPtr);

//系统内部电机急停
LH_ERR Can2SubSM_StopImmediatelyWhileAck(CAN2_SUB_SM_INDEX motorIndex);

//系统内部电机减速停止
LH_ERR Can2SubSM_StopGraduallyWhileAck(CAN2_SUB_SM_INDEX motorIndex);


/**********************************************************发送电机指令并等待电机执行完成***************************************************/
//系统内部电机复位
//修正步数 超时时间
LH_ERR Can2SubSM_ResetWhileReturn(SM_RESET_CMD* resetCmdPtr);

//系统内部电机执行执行步数
LH_ERR Can2SubSM_RunSpecialStepsWhileReturn(SM_RUN_STEP_CMD* runStepsCmdPtr);

//系统内部电机运行到指定坐标
LH_ERR Can2SubSM_RunToCoordinateWhileReturn(SM_RUN_COORDINATE_CMD* runCoordinateCmdPtr);

//系统内部电机回零
LH_ERR Can2SubSM_ReturnZeroWhileReturn(SM_RETURN_ZERO_CMD* returnZeroCmdPtr);

//系统内部电机电机持续运转
LH_ERR Can2SubSM_RunAlwaysWhileReturn(SM_RUN_ALWAYS_CMD* runAlwaysCmdPtr);

//系统内部电机急停
LH_ERR Can2SubSM_StopImmediatelyWhileReturn(CAN2_SUB_SM_INDEX motorIndex);

//系统内部电机减速停止
LH_ERR Can2SubSM_StopGraduallyWhileReturn(CAN2_SUB_SM_INDEX motorIndex);

//系统读取指定电机当前位置
LH_ERR Can2SubSM_ReadPositionWhileReturn(CAN2_SUB_SM_INDEX motorIndex,int32_t* motorPos);

/*********************************************************等待对应的电机动作执行完成**************************************************************/
LH_ERR Can2SubSM_WaitLastReturn(CAN2_SUB_SM_INDEX motorIndex,uint32_t timeOutMs);

#endif




