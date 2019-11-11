#include "ActionHandCup.h"

//反应盘光电信号有效值
#define DISK_REACTION_HAS_CUP_SENSOR_VALID_LEVEL    Bit_SET

//清洗盘光电信号有效值
#define DISK_WASH_HAS_CUP_SENSOR_VALID_LEVEL        Bit_SET

//机械手抓杯延迟时间
#define HAND_CUP_CATCH_DELAY_MS                     100

//机械手放杯延迟时间
#define HAND_CUP_PUT_DELAY_MS                       100

//新杯机械手升降复位
LH_ERR ActionHand_CupUpDownReset(void)
{
    LH_ERR errorCode;
    int32_t yogi;
    //电机复位结构体
    SM_RESET_CMD resetCommand;
    //结构体初始化
    Can2SubSM_ResetCmdDataStructInit(&resetCommand);

    //复位参数设置
    SystemReadActionParamWithIndex(MODULEHANDCUPPARAM,HANDUPDOWNRESETCORRECTION,&yogi);
    resetCommand.stepMotorIndex = STEP_MOTOR_HAND_CUP_UPDOWN;
    resetCommand.correctionPosition = yogi;//actionParam->moduleHandCupParam.upDownResetCorrection;
    resetCommand.timeOutMs = 60000;
    //抓杯机械手升降复位
    errorCode = Can2SubSM_ResetWhileReturn(&resetCommand);

    if(errorCode != LH_ERR_NONE)
	{
        //打印错误日志
        TaskServiceSerialErrMsgShow(errorCode);
        //返回错误代码
		return errorCode;
	}

    return errorCode;
}

//新杯机械手左右复位
LH_ERR ActionHand_CupLeftRightReset(void)
{
    LH_ERR errorCode;
    int32_t yogi;
    //电机复位结构体
    SM_RESET_CMD resetCommand;
    //结构体初始化
    Can2SubSM_ResetCmdDataStructInit(&resetCommand);

    //复位参数设置
    SystemReadActionParamWithIndex(MODULEHANDCUPPARAM,HANDLEFTRIGHTRESETCORRECTION,&yogi);
    resetCommand.stepMotorIndex = STEP_MOTOR_HAND_CUP_LEFTRIGHT;
    resetCommand.correctionPosition = yogi;//actionParam->moduleHandCupParam.leftRightResetCorrection;
    resetCommand.timeOutMs = 60000;
    //抓杯机械手左右复位
    errorCode = Can2SubSM_ResetWhileReturn(&resetCommand);

    if(errorCode != LH_ERR_NONE)
	{
        //打印错误日志
        TaskServiceSerialErrMsgShow(errorCode);
        //返回错误代码
		return errorCode;
	}

    return errorCode;
}

//新杯机械手前后复位
LH_ERR ActionHand_CupFrontBackReset(void)
{
    LH_ERR errorCode;
    int32_t yogi;
    //电机复位结构体
    SM_RESET_CMD resetCommand;
    //结构体初始化
    Can2SubSM_ResetCmdDataStructInit(&resetCommand);

    //复位参数设置
    SystemReadActionParamWithIndex(MODULEHANDCUPPARAM,HANDFRONTBACKRESETCORRECTION,&yogi);
    resetCommand.stepMotorIndex = STEP_MOTOR_HAND_CUP_FRONTBACK;
    resetCommand.correctionPosition = yogi;//actionParam->moduleHandCupParam.frontBackResetCorrection;
    resetCommand.timeOutMs = 60000;
    //抓杯机械手前后复位
    errorCode = Can2SubSM_ResetWhileReturn(&resetCommand);

    if(errorCode != LH_ERR_NONE)
	{
        //打印错误日志
        TaskServiceSerialErrMsgShow(errorCode);
        //返回错误代码
		return errorCode;
	}

    return errorCode;
}

//新杯机械手水平复位
LH_ERR ActionHand_FlatReset(void)
{
    //抓杯机械手左右前后同时复位
    LH_ERR errorCode;
    int32_t yogi;
    //电机复位结构体
    SM_RESET_CMD resetCommand;
    //结构体初始化
    Can2SubSM_ResetCmdDataStructInit(&resetCommand);

    //复位参数设置
    SystemReadActionParamWithIndex(MODULEHANDCUPPARAM,HANDFRONTBACKRESETCORRECTION,&yogi);
    resetCommand.stepMotorIndex = STEP_MOTOR_HAND_CUP_FRONTBACK;
    resetCommand.correctionPosition = yogi;//actionParam->moduleHandCupParam.frontBackResetCorrection;
    resetCommand.timeOutMs = 60000;
    //抓杯机械手前后复位
    errorCode = Can2SubSM_ResetWhileAck(&resetCommand);
    if(errorCode != LH_ERR_NONE)
	{
        //打印错误日志
        TaskServiceSerialErrMsgShow(errorCode);
        //返回错误代码
		return errorCode;
	}
    SystemReadActionParamWithIndex(MODULEHANDCUPPARAM,HANDLEFTRIGHTRESETCORRECTION,&yogi);
    resetCommand.stepMotorIndex = STEP_MOTOR_HAND_CUP_LEFTRIGHT;
    resetCommand.correctionPosition = yogi;//actionParam->moduleHandCupParam.leftRightResetCorrection;
    resetCommand.timeOutMs = 60000;
    //抓杯机械手左右复位
    errorCode = Can2SubSM_ResetWhileAck(&resetCommand);

    if(errorCode != LH_ERR_NONE)
	{
        //打印错误日志
        TaskServiceSerialErrMsgShow(errorCode);
        //返回错误代码
		return errorCode;
	}

    //等待执行完成
    errorCode = Can2SubSM_WaitLastReturn(STEP_MOTOR_HAND_CUP_FRONTBACK,60000);
    if(errorCode != LH_ERR_NONE)
	{
        //打印错误日志
        TaskServiceSerialErrMsgShow(errorCode);
        //返回错误代码
		return errorCode;
	}

    //等待执行完成
    errorCode = Can2SubSM_WaitLastReturn(STEP_MOTOR_HAND_CUP_LEFTRIGHT,60000);
    if(errorCode != LH_ERR_NONE)
	{
        //打印错误日志
        TaskServiceSerialErrMsgShow(errorCode);
        //返回错误代码
		return errorCode;
	}

    return errorCode;
}

//新杯机械手整体复位
LH_ERR ActionHand_SolidReset(void)
{
    LH_ERR errorCode;
    int32_t yogi;
    //电机复位结构体
    SM_RESET_CMD resetCommand;
    //结构体初始化
    Can2SubSM_ResetCmdDataStructInit(&resetCommand);

    //复位参数设置
    SystemReadActionParamWithIndex(MODULEHANDCUPPARAM,HANDUPDOWNRESETCORRECTION,&yogi);
    resetCommand.stepMotorIndex = STEP_MOTOR_HAND_CUP_UPDOWN;
    resetCommand.correctionPosition = yogi;//actionParam->moduleHandCupParam.upDownResetCorrection;
    resetCommand.timeOutMs = 60000;
    //抓杯机械手升降复位
    errorCode = Can2SubSM_ResetWhileReturn(&resetCommand);

    if(errorCode != LH_ERR_NONE)
	{
        //打印错误日志
        TaskServiceSerialErrMsgShow(errorCode);
        //返回错误代码
		return errorCode;
	}

    //复位参数设置
    SystemReadActionParamWithIndex(MODULEHANDCUPPARAM,HANDFRONTBACKRESETCORRECTION,&yogi);
    resetCommand.stepMotorIndex = STEP_MOTOR_HAND_CUP_FRONTBACK;
    resetCommand.correctionPosition = yogi;//actionParam->moduleHandCupParam.frontBackResetCorrection;
    resetCommand.timeOutMs = 60000;
    //抓杯机械手前后复位
    errorCode = Can2SubSM_ResetWhileAck(&resetCommand);
    if(errorCode != LH_ERR_NONE)
	{
        //打印错误日志
        TaskServiceSerialErrMsgShow(errorCode);
        //返回错误代码
		return errorCode;
	}
    SystemReadActionParamWithIndex(MODULEHANDCUPPARAM,HANDLEFTRIGHTRESETCORRECTION,&yogi);
    resetCommand.stepMotorIndex = STEP_MOTOR_HAND_CUP_LEFTRIGHT;
    resetCommand.correctionPosition = yogi;//actionParam->moduleHandCupParam.leftRightResetCorrection;
    resetCommand.timeOutMs = 60000;
    //抓杯机械手左右复位
    errorCode = Can2SubSM_ResetWhileAck(&resetCommand);

    if(errorCode != LH_ERR_NONE)
	{
        //打印错误日志
        TaskServiceSerialErrMsgShow(errorCode);
        //返回错误代码
		return errorCode;
	}

    //等待执行完成
    errorCode = Can2SubSM_WaitLastReturn(STEP_MOTOR_HAND_CUP_FRONTBACK,60000);
    if(errorCode != LH_ERR_NONE)
	{
        //打印错误日志
        TaskServiceSerialErrMsgShow(errorCode);
        //返回错误代码
		return errorCode;
	}

    //等待执行完成
    errorCode = Can2SubSM_WaitLastReturn(STEP_MOTOR_HAND_CUP_LEFTRIGHT,60000);
    if(errorCode != LH_ERR_NONE)
	{
        //打印错误日志
        TaskServiceSerialErrMsgShow(errorCode);
        //返回错误代码
		return errorCode;
	}

    return errorCode;
}

//机械手移动到反应盘
LH_ERR ActionHand_Move2DiskReaction(void)
{
    int32_t yogi;
    LH_ERR errorCode;
    SM_RUN_COORDINATE_CMD runCoordinateCmd;
    Can2SubSM_RunCoordinateDataStructInit(&runCoordinateCmd);

    //前后移动
    SystemReadActionParamWithIndex(MODULEHANDCUPPARAM,HANDFRONTBACKDISHREACTIONPOS,&yogi);
    runCoordinateCmd.stepMotorIndex = STEP_MOTOR_HAND_CUP_FRONTBACK;
    runCoordinateCmd.targetCoordinate = yogi;//actionParam->moduleHandCupParam.frontBackDishReactionPos;
    runCoordinateCmd.curveSelect = SM_CURVE_0;
    runCoordinateCmd.speedRatio = 100;
    runCoordinateCmd.speedMode = SPEED_MODE_UP_START;
    runCoordinateCmd.utilStop1Enable = DISABLE;
    runCoordinateCmd.utilStop2Enable = DISABLE;
    runCoordinateCmd.timeOutMs = 60000;

    //电机走坐标
	errorCode = Can2SubSM_RunToCoordinateWhileAck(&runCoordinateCmd);
    
	if(errorCode != LH_ERR_NONE)
	{
        //打印错误日志
        TaskServiceSerialErrMsgShow(errorCode);
        //返回错误代码
		return errorCode;
	}

    //左右移动
    SystemReadActionParamWithIndex(MODULEHANDCUPPARAM,HANDLEFTRIGHTDISHREACTIONPOS,&yogi);
    runCoordinateCmd.stepMotorIndex = STEP_MOTOR_HAND_CUP_LEFTRIGHT;
    runCoordinateCmd.targetCoordinate = yogi;//actionParam->moduleHandCupParam.leftRightDishReactionPos;
    runCoordinateCmd.curveSelect = HAND_LEFT_RIGHT_TO_REACTION;
    runCoordinateCmd.speedRatio = 100;
    runCoordinateCmd.speedMode = SPEED_MODE_UP_START;
    runCoordinateCmd.utilStop1Enable = DISABLE;
    runCoordinateCmd.utilStop2Enable = DISABLE;
    runCoordinateCmd.timeOutMs = 60000;

    //电机走坐标
	errorCode = Can2SubSM_RunToCoordinateWhileAck(&runCoordinateCmd);
    
	if(errorCode != LH_ERR_NONE)
	{
        //打印错误日志
        TaskServiceSerialErrMsgShow(errorCode);
        //返回错误代码
		return errorCode;
	}

    //等待执行完成
    errorCode = Can2SubSM_WaitLastReturn(STEP_MOTOR_HAND_CUP_FRONTBACK,60000);
    if(errorCode != LH_ERR_NONE)
	{
        //打印错误日志
        TaskServiceSerialErrMsgShow(errorCode);
        //返回错误代码
		return errorCode;
	}

    //等待执行完成
    errorCode = Can2SubSM_WaitLastReturn(STEP_MOTOR_HAND_CUP_LEFTRIGHT,60000);
    if(errorCode != LH_ERR_NONE)
	{
        //打印错误日志
        TaskServiceSerialErrMsgShow(errorCode);
        //返回错误代码
		return errorCode;
	}

	return errorCode;
}

//机械手移动到新杯区域
//行 列参数 行是前后 列是左右 14行 10列
LH_ERR ActionHand_Move2NewCupRegion(uint16_t row,uint16_t col)
{
    int32_t yogi;
    LH_ERR errorCode;
    SM_RUN_COORDINATE_CMD runCoordinateCmd;
    Can2SubSM_RunCoordinateDataStructInit(&runCoordinateCmd);

    int32_t posValue = 0;
	//移动到新杯栈上方
	//水平左右
	posValue = actionParam->moduleHandCupParam.leftRightEmptyCupStartPos;
    posValue += (col*(actionParam->moduleHandCupParam.leftRightEmptyCupOneOffset));

    runCoordinateCmd.stepMotorIndex = STEP_MOTOR_HAND_CUP_LEFTRIGHT;
    runCoordinateCmd.targetCoordinate = posValue;
    runCoordinateCmd.curveSelect = HAND_LEFT_RIGHT_TO_NEW_CUP;
    runCoordinateCmd.speedRatio = 100;
    runCoordinateCmd.speedMode = SPEED_MODE_UP_START;
    runCoordinateCmd.utilStop1Enable = DISABLE;
    runCoordinateCmd.utilStop2Enable = DISABLE;
    runCoordinateCmd.timeOutMs = 60000;

    //电机走坐标
	errorCode = Can2SubSM_RunToCoordinateWhileAck(&runCoordinateCmd);

    if(errorCode != LH_ERR_NONE)
	{
        //打印错误日志
        TaskServiceSerialErrMsgShow(errorCode);
        //返回错误代码
		return errorCode;
	}

    //水平前后
    SystemReadActionParamWithIndex(MODULEHANDCUPPARAM,HANDFRONTBACKEMPTYCUPSTARTPOS,&yogi);
	posValue = yogi;//actionParam->moduleHandCupParam.frontBackEmptyCupStartPos;
    posValue += (row*(actionParam->moduleHandCupParam.frontBackEmptyCupOneOffset));

    runCoordinateCmd.stepMotorIndex = STEP_MOTOR_HAND_CUP_FRONTBACK;
    runCoordinateCmd.targetCoordinate = posValue;
    runCoordinateCmd.curveSelect = SM_CURVE_1;
    runCoordinateCmd.speedRatio = 100;
    runCoordinateCmd.speedMode = SPEED_MODE_UP_START;
    runCoordinateCmd.utilStop1Enable = DISABLE;
    runCoordinateCmd.utilStop2Enable = DISABLE;
    runCoordinateCmd.timeOutMs = 60000;

    //电机走坐标
	errorCode = Can2SubSM_RunToCoordinateWhileAck(&runCoordinateCmd);
    
	if(errorCode != LH_ERR_NONE)
	{
        //打印错误日志
        TaskServiceSerialErrMsgShow(errorCode);
        //返回错误代码
		return errorCode;
	}

    //等待执行完成
    errorCode = Can2SubSM_WaitLastReturn(STEP_MOTOR_HAND_CUP_FRONTBACK,60000);
    if(errorCode != LH_ERR_NONE)
	{
        //打印错误日志
        TaskServiceSerialErrMsgShow(errorCode);
        //返回错误代码
		return errorCode;
	}

    //等待执行完成
    errorCode = Can2SubSM_WaitLastReturn(STEP_MOTOR_HAND_CUP_LEFTRIGHT,60000);
    if(errorCode != LH_ERR_NONE)
	{
        //打印错误日志
        TaskServiceSerialErrMsgShow(errorCode);
        //返回错误代码
		return errorCode;
	}

	return errorCode;
}

//机械手移动到垃圾桶
LH_ERR ActionHand_Move2Garbage(ACTION_PARAM_STACK_MANAGE_GARBAGE_NO garbageNo)
{
    LH_ERR errorCode;
    SM_RUN_COORDINATE_CMD runCoordinateCmd;
    int32_t yogi;
    Can2SubSM_RunCoordinateDataStructInit(&runCoordinateCmd);

    int32_t posValue = 0;
	//先水平移动
    if(garbageNo == TSC_STACK_MANAGE_GARBAGE_NO1)
    {
        posValue = actionParam->moduleHandCupParam.leftRightGarbage1Pos;
    }
    else
    {
        posValue = actionParam->moduleHandCupParam.leftRightGarbage2Pos;
    }

    //水平左右
    runCoordinateCmd.stepMotorIndex = STEP_MOTOR_HAND_CUP_LEFTRIGHT;
    runCoordinateCmd.targetCoordinate = posValue;
    runCoordinateCmd.curveSelect = HAND_LEFT_RIGHT_TO_TRASH;
    runCoordinateCmd.speedRatio = 100;
    runCoordinateCmd.speedMode = SPEED_MODE_UP_START;
    runCoordinateCmd.utilStop1Enable = DISABLE;
    runCoordinateCmd.utilStop2Enable = DISABLE;
    runCoordinateCmd.timeOutMs = 60000;

    //电机走坐标
	errorCode = Can2SubSM_RunToCoordinateWhileAck(&runCoordinateCmd);

    if(errorCode != LH_ERR_NONE)
	{
        //打印错误日志
        TaskServiceSerialErrMsgShow(errorCode);
        //返回错误代码
		return errorCode;
	}

    //然后前后移动
	if(garbageNo == TSC_STACK_MANAGE_GARBAGE_NO1)
    {
        posValue = actionParam->moduleHandCupParam.frontBackGarbage1Pos;
    }
    else
    {
        posValue = actionParam->moduleHandCupParam.frontBackGarbage2Pos;
    }

    runCoordinateCmd.stepMotorIndex = STEP_MOTOR_HAND_CUP_FRONTBACK;
    runCoordinateCmd.targetCoordinate = posValue;
    runCoordinateCmd.curveSelect = SM_CURVE_1;
    runCoordinateCmd.speedRatio = 100;
    runCoordinateCmd.speedMode = SPEED_MODE_UP_START;
    runCoordinateCmd.utilStop1Enable = DISABLE;
    runCoordinateCmd.utilStop2Enable = DISABLE;
    runCoordinateCmd.timeOutMs = 60000;

    //电机走坐标
	errorCode = Can2SubSM_RunToCoordinateWhileAck(&runCoordinateCmd);
    
	if(errorCode != LH_ERR_NONE)
	{
        //打印错误日志
        TaskServiceSerialErrMsgShow(errorCode);
        //返回错误代码
		return errorCode;
	}

    //等待执行完成
    errorCode = Can2SubSM_WaitLastReturn(STEP_MOTOR_HAND_CUP_FRONTBACK,60000);
    if(errorCode != LH_ERR_NONE)
	{
        //打印错误日志
        TaskServiceSerialErrMsgShow(errorCode);
        //返回错误代码
		return errorCode;
	}

    //等待执行完成
    errorCode = Can2SubSM_WaitLastReturn(STEP_MOTOR_HAND_CUP_LEFTRIGHT,60000);
    if(errorCode != LH_ERR_NONE)
	{
        //打印错误日志
        TaskServiceSerialErrMsgShow(errorCode);
        //返回错误代码
		return errorCode;
	}

    //丢杯
    if(garbageNo == TSC_STACK_MANAGE_GARBAGE_NO1)
    {
        posValue = actionParam->moduleHandCupParam.upDownGarbage1PutPos;
    }
    else
    {
        posValue = actionParam->moduleHandCupParam.upDownGarbage2PutPos;
    }

    //机械手升降
    runCoordinateCmd.stepMotorIndex = STEP_MOTOR_HAND_CUP_UPDOWN;
    runCoordinateCmd.targetCoordinate = posValue;
    runCoordinateCmd.curveSelect = MACHINE_HAND_FAST_EMPTY;
    runCoordinateCmd.speedRatio = 100;
    runCoordinateCmd.speedMode = SPEED_MODE_UP_START;
    runCoordinateCmd.utilStop1Enable = DISABLE;
    runCoordinateCmd.utilStop2Enable = DISABLE;
    runCoordinateCmd.timeOutMs = 60000;

    //电机走坐标
	errorCode = Can2SubSM_RunToCoordinateWhileReturn(&runCoordinateCmd);
    
	if(errorCode != LH_ERR_NONE)
	{
        //打印错误日志
        TaskServiceSerialErrMsgShow(errorCode);
        //返回错误代码
		return errorCode;
	}

    //延时等待
	CoreDelayMs(HAND_CUP_PUT_DELAY_MS);

    SM_RETURN_ZERO_CMD returnZeroCmd;
    Can2SubSM_ReturnZeroDataStructInit(&returnZeroCmd);

    //升降回零并修正
    SystemReadActionParamWithIndex(MODULEHANDCUPPARAM,HANDUPDOWNRESETCORRECTION,&yogi);
    returnZeroCmd.stepMotorIndex = STEP_MOTOR_HAND_CUP_UPDOWN;
    returnZeroCmd.correctionPosition = yogi;//actionParam->moduleHandCupParam.upDownResetCorrection;
    returnZeroCmd.curveSelect = MACHINE_HAND_FAST_EMPTY;
    returnZeroCmd.speedRatio = 100;
    returnZeroCmd.speedMode = SPEED_MODE_UP_START;
    returnZeroCmd.utilStop1Enable = DISABLE;
    returnZeroCmd.utilStop2Enable = DISABLE;
    returnZeroCmd.timeOutMs = 60000;

    errorCode = Can2SubSM_ReturnZeroWhileReturn(&returnZeroCmd);

    if(errorCode != LH_ERR_NONE)
	{
        //打印错误日志
        TaskServiceSerialErrMsgShow(errorCode);
        //返回错误代码
		return errorCode;
	}

    return errorCode;
}

//移动到清洗盘
LH_ERR ActionHand_Move2DiskWash(void)
{
    int32_t yogi;
    LH_ERR errorCode;
    SM_RUN_COORDINATE_CMD runCoordinateCmd;
    Can2SubSM_RunCoordinateDataStructInit(&runCoordinateCmd);

    //前后移动
    SystemReadActionParamWithIndex(MODULEHANDCUPPARAM,HANDFRONTBACKDISHCLEANPOS,&yogi);
    runCoordinateCmd.stepMotorIndex = STEP_MOTOR_HAND_CUP_FRONTBACK;
    runCoordinateCmd.targetCoordinate = yogi;//actionParam->moduleHandCupParam.frontBackDishCleanPos;
    runCoordinateCmd.curveSelect = SM_CURVE_0;
    runCoordinateCmd.speedRatio = 100;
    runCoordinateCmd.speedMode = SPEED_MODE_UP_START;
    runCoordinateCmd.utilStop1Enable = DISABLE;
    runCoordinateCmd.utilStop2Enable = DISABLE;
    runCoordinateCmd.timeOutMs = 60000;

    //电机走坐标
	errorCode = Can2SubSM_RunToCoordinateWhileAck(&runCoordinateCmd);
    
	if(errorCode != LH_ERR_NONE)
	{
        //打印错误日志
        TaskServiceSerialErrMsgShow(errorCode);
        //返回错误代码
		return errorCode;
	}

    //左右移动
    SystemReadActionParamWithIndex(MODULEHANDCUPPARAM,HANDLEFTRIGHTDISHCLEANPOS,&yogi);
    runCoordinateCmd.stepMotorIndex = STEP_MOTOR_HAND_CUP_LEFTRIGHT;
    runCoordinateCmd.targetCoordinate = yogi;//actionParam->moduleHandCupParam.leftRightDishCleanPos;
    runCoordinateCmd.curveSelect = HAND_LEFT_RIGHT_TO_WASH;
    runCoordinateCmd.speedRatio = 100;
    runCoordinateCmd.speedMode = SPEED_MODE_UP_START;
    runCoordinateCmd.utilStop1Enable = DISABLE;
    runCoordinateCmd.utilStop2Enable = DISABLE;
    runCoordinateCmd.timeOutMs = 60000;

    //电机走坐标
	errorCode = Can2SubSM_RunToCoordinateWhileAck(&runCoordinateCmd);
    
	if(errorCode != LH_ERR_NONE)
	{
        //打印错误日志
        TaskServiceSerialErrMsgShow(errorCode);
        //返回错误代码
		return errorCode;
	}

    //等待执行完成
    errorCode = Can2SubSM_WaitLastReturn(STEP_MOTOR_HAND_CUP_FRONTBACK,60000);
    if(errorCode != LH_ERR_NONE)
	{
        //打印错误日志
        TaskServiceSerialErrMsgShow(errorCode);
        //返回错误代码
		return errorCode;
	}

    //等待执行完成
    errorCode = Can2SubSM_WaitLastReturn(STEP_MOTOR_HAND_CUP_LEFTRIGHT,60000);
    if(errorCode != LH_ERR_NONE)
	{
        //打印错误日志
        TaskServiceSerialErrMsgShow(errorCode);
        //返回错误代码
		return errorCode;
	}

	return errorCode;
}

//移动到测量室上方
LH_ERR ActionHand_Move2MeasureRoom(void)
{
    int32_t yogi;
    LH_ERR errorCode;
    SM_RUN_COORDINATE_CMD runCoordinateCmd;
    Can2SubSM_RunCoordinateDataStructInit(&runCoordinateCmd);

    //前后移动
    SystemReadActionParamWithIndex(MODULEHANDCUPPARAM,HANDFRONTBACKMEASUREROOMPOS,&yogi);
    runCoordinateCmd.stepMotorIndex = STEP_MOTOR_HAND_CUP_FRONTBACK;
    runCoordinateCmd.targetCoordinate = yogi;//actionParam->moduleHandCupParam.frontBackMeasureRoomPos;
    runCoordinateCmd.curveSelect = SM_CURVE_0;
    runCoordinateCmd.speedRatio = 100;
    runCoordinateCmd.speedMode = SPEED_MODE_UP_START;
    runCoordinateCmd.utilStop1Enable = DISABLE;
    runCoordinateCmd.utilStop2Enable = DISABLE;
    runCoordinateCmd.timeOutMs = 60000;

    //电机走坐标
	errorCode = Can2SubSM_RunToCoordinateWhileAck(&runCoordinateCmd);
    
	if(errorCode != LH_ERR_NONE)
	{
        //打印错误日志
        TaskServiceSerialErrMsgShow(errorCode);
        //返回错误代码
		return errorCode;
	}

    //左右移动
    SystemReadActionParamWithIndex(MODULEHANDCUPPARAM,HANDLEFTRIGHTMEASUREROOMPOS,&yogi);
    runCoordinateCmd.stepMotorIndex = STEP_MOTOR_HAND_CUP_LEFTRIGHT;
    runCoordinateCmd.targetCoordinate = yogi;//actionParam->moduleHandCupParam.leftRightMeasureRoomPos;
    runCoordinateCmd.curveSelect = HAND_LEFT_RIGHT_TO_MEASURE;
    runCoordinateCmd.speedRatio = 100;
    runCoordinateCmd.speedMode = SPEED_MODE_UP_START;
    runCoordinateCmd.utilStop1Enable = DISABLE;
    runCoordinateCmd.utilStop2Enable = DISABLE;
    runCoordinateCmd.timeOutMs = 60000;

    //电机走坐标
	errorCode = Can2SubSM_RunToCoordinateWhileAck(&runCoordinateCmd);
    
	if(errorCode != LH_ERR_NONE)
	{
        //打印错误日志
        TaskServiceSerialErrMsgShow(errorCode);
        //返回错误代码
		return errorCode;
	}

    //等待执行完成
    errorCode = Can2SubSM_WaitLastReturn(STEP_MOTOR_HAND_CUP_FRONTBACK,60000);
    if(errorCode != LH_ERR_NONE)
	{
        //打印错误日志
        TaskServiceSerialErrMsgShow(errorCode);
        //返回错误代码
		return errorCode;
	}

    //等待执行完成
    errorCode = Can2SubSM_WaitLastReturn(STEP_MOTOR_HAND_CUP_LEFTRIGHT,60000);
    if(errorCode != LH_ERR_NONE)
	{
        //打印错误日志
        TaskServiceSerialErrMsgShow(errorCode);
        //返回错误代码
		return errorCode;
	}

	return errorCode;
}

//往反应盘放杯
LH_ERR ActionHand_PutCup2DiskReaction(ACTION_SENSOR_STATUS* diskReactionCupHasExist)
{
    //向反应盘放杯
	BitAction sensorValue;
    int32_t yogi;
	//先合上反应盘电磁铁
	LH_ERR errorCode;
    SM_RUN_COORDINATE_CMD runCoordinateCmd;
    Can2SubSM_RunCoordinateDataStructInit(&runCoordinateCmd);

    //合上电磁铁
	errorCode = SystemOutputWrite(DISH_REACTION_ELECTRO_MAGNET,Bit_RESET);
	if(errorCode != LH_ERR_NONE)
	{
        //打印错误日志
        TaskServiceSerialErrMsgShow(errorCode);
        //返回错误代码
		return errorCode;
	}

    //升降运动到放杯坐标
    //机械手升降
    SystemReadActionParamWithIndex(MODULEHANDCUPPARAM,HANDUPDOWNDISHREACTIONPUTPOS,&yogi);
    runCoordinateCmd.stepMotorIndex = STEP_MOTOR_HAND_CUP_UPDOWN;
    runCoordinateCmd.targetCoordinate = yogi;// actionParam->moduleHandCupParam.upDownDishReactionPutPos;
    runCoordinateCmd.curveSelect = MACHINE_HAND_CATCH;
    runCoordinateCmd.speedRatio = 100;
    runCoordinateCmd.speedMode = SPEED_MODE_UP_START;
    runCoordinateCmd.utilStop1Enable = DISABLE;
    runCoordinateCmd.utilStop2Enable = DISABLE;
    runCoordinateCmd.timeOutMs = 60000;

    //电机走坐标
	errorCode = Can2SubSM_RunToCoordinateWhileReturn(&runCoordinateCmd);
    
	if(errorCode != LH_ERR_NONE)
	{
        //打印错误日志
        TaskServiceSerialErrMsgShow(errorCode);
        //返回错误代码
		return errorCode;
	}
    
    //延时等待
	CoreDelayMs(HAND_CUP_PUT_DELAY_MS);

    //上下回零,抬起来
    SM_RETURN_ZERO_CMD returnZeroCmd;
    Can2SubSM_ReturnZeroDataStructInit(&returnZeroCmd);

    //升降回零并修正
    SystemReadActionParamWithIndex(MODULEHANDCUPPARAM,HANDUPDOWNRESETCORRECTION,&yogi);
    returnZeroCmd.stepMotorIndex = STEP_MOTOR_HAND_CUP_UPDOWN;
    returnZeroCmd.correctionPosition = yogi;//actionParam->moduleHandCupParam.upDownResetCorrection;
    returnZeroCmd.curveSelect = MACHINE_HAND_EMPTY;
    returnZeroCmd.speedRatio = 100;
    returnZeroCmd.speedMode = SPEED_MODE_UP_START;
    returnZeroCmd.utilStop1Enable = DISABLE;
    returnZeroCmd.utilStop2Enable = DISABLE;
    returnZeroCmd.timeOutMs = 60000;

    errorCode = Can2SubSM_ReturnZeroWhileReturn(&returnZeroCmd);

    if(errorCode != LH_ERR_NONE)
	{
        //打印错误日志
        TaskServiceSerialErrMsgShow(errorCode);
        //返回错误代码
		return errorCode;
	}
    //释放电磁铁
    errorCode = SystemOutputWrite(DISH_REACTION_ELECTRO_MAGNET,Bit_SET);
	if(errorCode != LH_ERR_NONE)
	{
        //打印错误日志
        TaskServiceSerialErrMsgShow(errorCode);
        //返回错误代码
		return errorCode;
	}
    //读取放杯光电讯号并返回
    CoreDelayMs(50);
    errorCode = SystemInputRead(DISH_REACTION_LIGHT_SENSOR,&sensorValue);
	if(errorCode != LH_ERR_NONE)
	{
        //打印错误日志
        TaskServiceSerialErrMsgShow(errorCode);
        //返回错误代码
		return errorCode;
	}

    //有效状态
	if(sensorValue == DISK_REACTION_HAS_CUP_SENSOR_VALID_LEVEL)
    {
        *diskReactionCupHasExist = ACTION_SENSOR_STATUS_EXIST;
    }
    else
    {
        *diskReactionCupHasExist = ACTION_SENSOR_STATUS_NOT_EXIST;
    }

    return errorCode;
}

//往垃圾桶放杯
LH_ERR ActionHand_PutCup2Garbage(ACTION_PARAM_STACK_MANAGE_GARBAGE_NO garbageNo)
{
    LH_ERR errorCode;
    int32_t yogi;
    SM_RUN_COORDINATE_CMD runCoordinateCmd;
    Can2SubSM_RunCoordinateDataStructInit(&runCoordinateCmd);

    int32_t posValue = 0;

    //丢杯
    if(garbageNo == TSC_STACK_MANAGE_GARBAGE_NO1)
    {
        posValue = actionParam->moduleHandCupParam.upDownGarbage1PutPos;
    }
    else
    {
        posValue = actionParam->moduleHandCupParam.upDownGarbage2PutPos;
    }

    //机械手升降
    runCoordinateCmd.stepMotorIndex = STEP_MOTOR_HAND_CUP_UPDOWN;
    runCoordinateCmd.targetCoordinate = posValue;
    runCoordinateCmd.curveSelect = SM_CURVE_0;
    runCoordinateCmd.speedRatio = 100;
    runCoordinateCmd.speedMode = SPEED_MODE_UP_START;
    runCoordinateCmd.utilStop1Enable = DISABLE;
    runCoordinateCmd.utilStop2Enable = DISABLE;
    runCoordinateCmd.timeOutMs = 60000;

    //电机走坐标
	errorCode = Can2SubSM_RunToCoordinateWhileReturn(&runCoordinateCmd);
    
	if(errorCode != LH_ERR_NONE)
	{
        //打印错误日志
        TaskServiceSerialErrMsgShow(errorCode);
        //返回错误代码
		return errorCode;
	}

    //延时等待
	CoreDelayMs(HAND_CUP_PUT_DELAY_MS);

    SM_RETURN_ZERO_CMD returnZeroCmd;
    Can2SubSM_ReturnZeroDataStructInit(&returnZeroCmd);

    //升降回零并修正
    SystemReadActionParamWithIndex(MODULEHANDCUPPARAM,HANDUPDOWNRESETCORRECTION,&yogi);
    returnZeroCmd.stepMotorIndex = STEP_MOTOR_HAND_CUP_UPDOWN;
    returnZeroCmd.correctionPosition = yogi;//actionParam->moduleHandCupParam.upDownResetCorrection;
    returnZeroCmd.curveSelect = SM_CURVE_0;
    returnZeroCmd.speedRatio = 100;
    returnZeroCmd.speedMode = SPEED_MODE_UP_START;
    returnZeroCmd.utilStop1Enable = DISABLE;
    returnZeroCmd.utilStop2Enable = DISABLE;
    returnZeroCmd.timeOutMs = 60000;

    errorCode = Can2SubSM_ReturnZeroWhileReturn(&returnZeroCmd);

    if(errorCode != LH_ERR_NONE)
	{
        //打印错误日志
        TaskServiceSerialErrMsgShow(errorCode);
        //返回错误代码
		return errorCode;
	}

    return errorCode;

}

//从反应盘取杯
LH_ERR ActionHand_CatchCupFromDiskReaction(ACTION_SENSOR_STATUS* diskReactionCupHasExist)
{
	BitAction sensorValue;
	LH_ERR errorCode;
    int32_t yogi;
    SM_RUN_COORDINATE_CMD runCoordinateCmd;
    Can2SubSM_RunCoordinateDataStructInit(&runCoordinateCmd);

    //打开电磁铁
	errorCode = SystemOutputWrite(DISH_REACTION_ELECTRO_MAGNET,Bit_SET);
	if(errorCode != LH_ERR_NONE)
	{
        //打印错误日志
        TaskServiceSerialErrMsgShow(errorCode);
        //返回错误代码
		return errorCode;
	}

    //机械手升降
    SystemReadActionParamWithIndex(MODULEHANDCUPPARAM,HANDUPDOWNDISHREACTIONCATCHPOS,&yogi);
    runCoordinateCmd.stepMotorIndex = STEP_MOTOR_HAND_CUP_UPDOWN;
    runCoordinateCmd.targetCoordinate = yogi;//actionParam->moduleHandCupParam.upDownDishReactionCatchPos;
    runCoordinateCmd.curveSelect = MACHINE_HAND_EMPTY;
    runCoordinateCmd.speedRatio = 100;
    runCoordinateCmd.speedMode = SPEED_MODE_UP_START;
    runCoordinateCmd.utilStop1Enable = DISABLE;
    runCoordinateCmd.utilStop2Enable = DISABLE;
    runCoordinateCmd.timeOutMs = 60000;

    //电机走坐标
	errorCode = Can2SubSM_RunToCoordinateWhileReturn(&runCoordinateCmd);
    
	if(errorCode != LH_ERR_NONE)
	{
        //打印错误日志
        TaskServiceSerialErrMsgShow(errorCode);
        //返回错误代码
		return errorCode;
	}
    
    //延时等待
	CoreDelayMs(HAND_CUP_CATCH_DELAY_MS);

    //抬起来
    SystemReadActionParamWithIndex(MODULEHANDCUPPARAM,HANDUPDOWNRESETCORRECTION,&yogi);
    runCoordinateCmd.stepMotorIndex = STEP_MOTOR_HAND_CUP_UPDOWN;
    runCoordinateCmd.targetCoordinate = yogi;//actionParam->moduleHandCupParam.upDownResetCorrection;
    runCoordinateCmd.curveSelect = MACHINE_HAND_SLOW_CATCH;
    runCoordinateCmd.speedRatio = 100;
    runCoordinateCmd.speedMode = SPEED_MODE_UP_START;
    runCoordinateCmd.utilStop1Enable = DISABLE;
    runCoordinateCmd.utilStop2Enable = DISABLE;
    runCoordinateCmd.timeOutMs = 60000;

    //电机走坐标
	errorCode = Can2SubSM_RunToCoordinateWhileReturn(&runCoordinateCmd);
    
	if(errorCode != LH_ERR_NONE)
	{
        //打印错误日志
        TaskServiceSerialErrMsgShow(errorCode);
        //返回错误代码
		return errorCode;
	}

    //读取放杯光电讯号并返回
    CoreDelayMs(50);
    errorCode = SystemInputRead(DISH_REACTION_LIGHT_SENSOR,&sensorValue);
	if(errorCode != LH_ERR_NONE)
	{
        //打印错误日志
        TaskServiceSerialErrMsgShow(errorCode);
        //返回错误代码
		return errorCode;
	}

    //有效状态
	if(sensorValue == DISK_REACTION_HAS_CUP_SENSOR_VALID_LEVEL)
    {
        *diskReactionCupHasExist = ACTION_SENSOR_STATUS_EXIST;
    }
    else
    {
        *diskReactionCupHasExist = ACTION_SENSOR_STATUS_NOT_EXIST;
    }

    return errorCode;
}

//从新杯区取杯
LH_ERR ActionHand_CatchFromNewCupRegion(void)
{
    LH_ERR errorCode;
    int32_t yogi;
    SM_RUN_COORDINATE_CMD runCoordinateCmd;
    Can2SubSM_RunCoordinateDataStructInit(&runCoordinateCmd);
    SM_RETURN_ZERO_CMD returnZeroCmd;
    Can2SubSM_ReturnZeroDataStructInit(&returnZeroCmd);

    //机械手升降
    SystemReadActionParamWithIndex(MODULEHANDCUPPARAM,HANDUPDOWNEMPTYCUPCATCHPOS,&yogi);
    runCoordinateCmd.stepMotorIndex = STEP_MOTOR_HAND_CUP_UPDOWN;
    runCoordinateCmd.targetCoordinate = yogi;//actionParam->moduleHandCupParam.upDownEmptyCupCatchPos;
    runCoordinateCmd.curveSelect = MACHINE_HAND_EMPTY;
    runCoordinateCmd.speedRatio = 100;
    runCoordinateCmd.speedMode = SPEED_MODE_UP_START;
    runCoordinateCmd.utilStop1Enable = DISABLE;
    runCoordinateCmd.utilStop2Enable = DISABLE;
    runCoordinateCmd.timeOutMs = 60000;

    //电机走坐标
	errorCode = Can2SubSM_RunToCoordinateWhileReturn(&runCoordinateCmd);
    
	if(errorCode != LH_ERR_NONE)
	{
        //打印错误日志
        TaskServiceSerialErrMsgShow(errorCode);
        //返回错误代码
		return errorCode;
	}

    //延时等待
	CoreDelayMs(HAND_CUP_CATCH_DELAY_MS);

    //回零并修正
    SystemReadActionParamWithIndex(MODULEHANDCUPPARAM,HANDUPDOWNRESETCORRECTION,&yogi);
    returnZeroCmd.stepMotorIndex = STEP_MOTOR_HAND_CUP_UPDOWN;
    returnZeroCmd.correctionPosition = yogi;//actionParam->moduleHandCupParam.upDownResetCorrection;
    returnZeroCmd.curveSelect = MACHINE_HAND_CATCH;
    returnZeroCmd.speedRatio = 100;
    returnZeroCmd.speedMode = SPEED_MODE_UP_START;
    returnZeroCmd.utilStop1Enable = DISABLE;
    returnZeroCmd.utilStop2Enable = DISABLE;
    returnZeroCmd.timeOutMs = 60000;

    errorCode = Can2SubSM_ReturnZeroWhileReturn(&returnZeroCmd);

    if(errorCode != LH_ERR_NONE)
	{
        //打印错误日志
        TaskServiceSerialErrMsgShow(errorCode);
        //返回错误代码
		return errorCode;
	}

    return errorCode;
}

//往清洗盘放杯
LH_ERR ActionHand_PutCup2DiskWash(ACTION_SENSOR_STATUS* diskWashCupHasExist)
{
    //向反应盘放杯
	BitAction sensorValue;
    int32_t yogi;
	//先合上清洗盘电磁铁
	LH_ERR errorCode;
    SM_RUN_COORDINATE_CMD runCoordinateCmd;
    Can2SubSM_RunCoordinateDataStructInit(&runCoordinateCmd);

    //合上电磁铁
	errorCode = SystemOutputWrite(DISH_WASH_ELECTRO_MAGNET,Bit_RESET);
	if(errorCode != LH_ERR_NONE)
	{
        //打印错误日志
        TaskServiceSerialErrMsgShow(errorCode);
        //返回错误代码
		return errorCode;
	}

    //升降运动到放杯坐标
    //机械手升降
    SystemReadActionParamWithIndex(MODULEHANDCUPPARAM,HANDUPDOWNDISHCLEANPUTPOS,&yogi);
    runCoordinateCmd.stepMotorIndex = STEP_MOTOR_HAND_CUP_UPDOWN;
    runCoordinateCmd.targetCoordinate = yogi;//actionParam->moduleHandCupParam.upDownDishCleanPutPos;
    runCoordinateCmd.curveSelect = MACHINE_HAND_CATCH;
    runCoordinateCmd.speedRatio = 100;
    runCoordinateCmd.speedMode = SPEED_MODE_UP_START;
    runCoordinateCmd.utilStop1Enable = DISABLE;
    runCoordinateCmd.utilStop2Enable = DISABLE;
    runCoordinateCmd.timeOutMs = 60000;

    //电机走坐标
	errorCode = Can2SubSM_RunToCoordinateWhileReturn(&runCoordinateCmd);
    
	if(errorCode != LH_ERR_NONE)
	{
        //打印错误日志
        TaskServiceSerialErrMsgShow(errorCode);
        //返回错误代码
		return errorCode;
	}
    
    //延时等待
	CoreDelayMs(HAND_CUP_PUT_DELAY_MS);

    //上下回零,抬起来
    SM_RETURN_ZERO_CMD returnZeroCmd;
    Can2SubSM_ReturnZeroDataStructInit(&returnZeroCmd);

    //升降回零并修正
    SystemReadActionParamWithIndex(MODULEHANDCUPPARAM,HANDUPDOWNRESETCORRECTION,&yogi);
    returnZeroCmd.stepMotorIndex = STEP_MOTOR_HAND_CUP_UPDOWN;
    returnZeroCmd.correctionPosition = yogi;//actionParam->moduleHandCupParam.upDownResetCorrection;
    returnZeroCmd.curveSelect = MACHINE_HAND_EMPTY;
    returnZeroCmd.speedRatio = 100;
    returnZeroCmd.speedMode = SPEED_MODE_UP_START;
    returnZeroCmd.utilStop1Enable = DISABLE;
    returnZeroCmd.utilStop2Enable = DISABLE;
    returnZeroCmd.timeOutMs = 60000;

    errorCode = Can2SubSM_ReturnZeroWhileReturn(&returnZeroCmd);

    if(errorCode != LH_ERR_NONE)
	{
        //打印错误日志
        TaskServiceSerialErrMsgShow(errorCode);
        //返回错误代码
		return errorCode;
	}
    //释放电磁铁
    errorCode = SystemOutputWrite(DISH_WASH_ELECTRO_MAGNET,Bit_SET);
	if(errorCode != LH_ERR_NONE)
	{
        //打印错误日志
        TaskServiceSerialErrMsgShow(errorCode);
        //返回错误代码
		return errorCode;
	}

    //读取放杯光电讯号并返回
    CoreDelayMs(50);
    errorCode = SystemInputRead(DISH_WASH_LIGHT_SENSOR,&sensorValue);
	if(errorCode != LH_ERR_NONE)
	{
        //打印错误日志
        TaskServiceSerialErrMsgShow(errorCode);
        //返回错误代码
		return errorCode;
	}

    //有效状态
	if(sensorValue == DISK_WASH_HAS_CUP_SENSOR_VALID_LEVEL)
    {
        *diskWashCupHasExist = ACTION_SENSOR_STATUS_EXIST;
    }
    else
    {
        *diskWashCupHasExist = ACTION_SENSOR_STATUS_NOT_EXIST;
    }

    return errorCode;
}

//从清洗盘取杯
LH_ERR ActionHand_CatchFromDiskWash(ACTION_SENSOR_STATUS* diskWashCupHasExist)
{
    int32_t yogi;
    BitAction sensorValue;
	LH_ERR errorCode;
    SM_RUN_COORDINATE_CMD runCoordinateCmd;
    Can2SubSM_RunCoordinateDataStructInit(&runCoordinateCmd);

    //打开电磁铁
	errorCode = SystemOutputWrite(DISH_WASH_ELECTRO_MAGNET,Bit_SET);
	if(errorCode != LH_ERR_NONE)
	{
        //打印错误日志
        TaskServiceSerialErrMsgShow(errorCode);
        //返回错误代码
		return errorCode;
	}

    //机械手升降
    SystemReadActionParamWithIndex(MODULEHANDCUPPARAM,HANDUPDOWNDISHCLEANCATCHPOS,&yogi);
    runCoordinateCmd.stepMotorIndex = STEP_MOTOR_HAND_CUP_UPDOWN;
    runCoordinateCmd.targetCoordinate = yogi;//actionParam->moduleHandCupParam.upDownDishCleanCatchPos;
    runCoordinateCmd.curveSelect = MACHINE_HAND_FAST_EMPTY;
    runCoordinateCmd.speedRatio = 100;
    runCoordinateCmd.speedMode = SPEED_MODE_UP_START;
    runCoordinateCmd.utilStop1Enable = DISABLE;
    runCoordinateCmd.utilStop2Enable = DISABLE;
    runCoordinateCmd.timeOutMs = 60000;

    //电机走坐标
	errorCode = Can2SubSM_RunToCoordinateWhileReturn(&runCoordinateCmd);
    
	if(errorCode != LH_ERR_NONE)
	{
        //打印错误日志
        TaskServiceSerialErrMsgShow(errorCode);
        //返回错误代码
		return errorCode;
	}
    
    //延时等待
	CoreDelayMs(HAND_CUP_CATCH_DELAY_MS);

    //抬起来
    SystemReadActionParamWithIndex(MODULEHANDCUPPARAM,HANDUPDOWNRESETCORRECTION,&yogi);
    runCoordinateCmd.stepMotorIndex = STEP_MOTOR_HAND_CUP_UPDOWN;
    runCoordinateCmd.targetCoordinate = yogi;//actionParam->moduleHandCupParam.upDownResetCorrection;
    runCoordinateCmd.curveSelect = MACHINE_HAND_FAST_CATCH;
    runCoordinateCmd.speedRatio = 100;
    runCoordinateCmd.speedMode = SPEED_MODE_UP_START;
    runCoordinateCmd.utilStop1Enable = DISABLE;
    runCoordinateCmd.utilStop2Enable = DISABLE;
    runCoordinateCmd.timeOutMs = 60000;

    //电机走坐标
	errorCode = Can2SubSM_RunToCoordinateWhileReturn(&runCoordinateCmd);
    
	if(errorCode != LH_ERR_NONE)
	{
        //打印错误日志
        TaskServiceSerialErrMsgShow(errorCode);
        //返回错误代码
		return errorCode;
	}

    //读取放杯光电讯号并返回
    CoreDelayMs(50);
    errorCode = SystemInputRead(DISH_WASH_LIGHT_SENSOR,&sensorValue);
	if(errorCode != LH_ERR_NONE)
	{
        //打印错误日志
        TaskServiceSerialErrMsgShow(errorCode);
        //返回错误代码
		return errorCode;
	}

    //有效状态
	if(sensorValue == DISK_WASH_HAS_CUP_SENSOR_VALID_LEVEL)
    {
        *diskWashCupHasExist = ACTION_SENSOR_STATUS_EXIST;
    }
    else
    {
        *diskWashCupHasExist = ACTION_SENSOR_STATUS_NOT_EXIST;
    }

    return errorCode;
}

//往测量室放杯
LH_ERR ActionHand_PutCup2MeasureRoom(void)
{
    int32_t yogi;
	LH_ERR errorCode;
    SM_RUN_COORDINATE_CMD runCoordinateCmd;
    Can2SubSM_RunCoordinateDataStructInit(&runCoordinateCmd);

    //升降运动到放杯坐标
    //机械手升降
    SystemReadActionParamWithIndex(MODULEHANDCUPPARAM,HANDUPDOWNMEASUREROOMPUTPOS,&yogi);
    runCoordinateCmd.stepMotorIndex = STEP_MOTOR_HAND_CUP_UPDOWN;
    runCoordinateCmd.targetCoordinate = yogi;//actionParam->moduleHandCupParam.upDownMeasureRoomPutPos;
    runCoordinateCmd.curveSelect = MACHINE_HAND_FAST_CATCH;
    runCoordinateCmd.speedRatio = 100;
    runCoordinateCmd.speedMode = SPEED_MODE_UP_START;
    runCoordinateCmd.utilStop1Enable = DISABLE;
    runCoordinateCmd.utilStop2Enable = DISABLE;
    runCoordinateCmd.timeOutMs = 60000;

    //电机走坐标
	errorCode = Can2SubSM_RunToCoordinateWhileReturn(&runCoordinateCmd);
    
	if(errorCode != LH_ERR_NONE)
	{
        //打印错误日志
        TaskServiceSerialErrMsgShow(errorCode);
        //返回错误代码
		return errorCode;
	}
    
    //延时等待
	CoreDelayMs(HAND_CUP_PUT_DELAY_MS);

    //上下回零,抬起来
    SM_RETURN_ZERO_CMD returnZeroCmd;
    Can2SubSM_ReturnZeroDataStructInit(&returnZeroCmd);

    //升降回零并修正
    SystemReadActionParamWithIndex(MODULEHANDCUPPARAM,HANDUPDOWNRESETCORRECTION,&yogi);
    returnZeroCmd.stepMotorIndex = STEP_MOTOR_HAND_CUP_UPDOWN;
    returnZeroCmd.correctionPosition = yogi;//actionParam->moduleHandCupParam.upDownResetCorrection;
    returnZeroCmd.curveSelect = MACHINE_HAND_FAST_EMPTY;
    returnZeroCmd.speedRatio = 100;
    returnZeroCmd.speedMode = SPEED_MODE_UP_START;
    returnZeroCmd.utilStop1Enable = DISABLE;
    returnZeroCmd.utilStop2Enable = DISABLE;
    returnZeroCmd.timeOutMs = 60000;

    errorCode = Can2SubSM_ReturnZeroWhileReturn(&returnZeroCmd);

    if(errorCode != LH_ERR_NONE)
	{
        //打印错误日志
        TaskServiceSerialErrMsgShow(errorCode);
        //返回错误代码
		return errorCode;
	}

    return errorCode;
}

//从测量室取杯
LH_ERR ActionHand_CatchCupFromMeasureRoom(void)
{
    int32_t yogi;
	LH_ERR errorCode;
    SM_RUN_COORDINATE_CMD runCoordinateCmd;
    Can2SubSM_RunCoordinateDataStructInit(&runCoordinateCmd);

    //机械手升降
    SystemReadActionParamWithIndex(MODULEHANDCUPPARAM,HANDUPDOWNMEASUREROOMCATCHPOS,&yogi);
    runCoordinateCmd.stepMotorIndex = STEP_MOTOR_HAND_CUP_UPDOWN;
    runCoordinateCmd.targetCoordinate = yogi;//actionParam->moduleHandCupParam.upDownMeasureRoomCatchPos;
    runCoordinateCmd.curveSelect = MACHINE_HAND_FAST_EMPTY;
    runCoordinateCmd.speedRatio = 100;
    runCoordinateCmd.speedMode = SPEED_MODE_UP_START;
    runCoordinateCmd.utilStop1Enable = DISABLE;
    runCoordinateCmd.utilStop2Enable = DISABLE;
    runCoordinateCmd.timeOutMs = 60000;

    //电机走坐标
	errorCode = Can2SubSM_RunToCoordinateWhileReturn(&runCoordinateCmd);
    
	if(errorCode != LH_ERR_NONE)
	{
        //打印错误日志
        TaskServiceSerialErrMsgShow(errorCode);
        //返回错误代码
		return errorCode;
	}
    
    //延时等待
	CoreDelayMs(HAND_CUP_CATCH_DELAY_MS);

    //抬起来
    SystemReadActionParamWithIndex(MODULEHANDCUPPARAM,HANDUPDOWNRESETCORRECTION,&yogi);
    runCoordinateCmd.stepMotorIndex = STEP_MOTOR_HAND_CUP_UPDOWN;
    runCoordinateCmd.targetCoordinate = yogi;//actionParam->moduleHandCupParam.upDownResetCorrection;
    runCoordinateCmd.curveSelect = MACHINE_HAND_FAST_CATCH;
    runCoordinateCmd.speedRatio = 100;
    runCoordinateCmd.speedMode = SPEED_MODE_UP_START;
    runCoordinateCmd.utilStop1Enable = DISABLE;
    runCoordinateCmd.utilStop2Enable = DISABLE;
    runCoordinateCmd.timeOutMs = 60000;

    //电机走坐标
	errorCode = Can2SubSM_RunToCoordinateWhileReturn(&runCoordinateCmd);
    
	if(errorCode != LH_ERR_NONE)
	{
        //打印错误日志
        TaskServiceSerialErrMsgShow(errorCode);
        //返回错误代码
		return errorCode;
	}

    return errorCode;
}








