#include "ActionMixBead.h"


//磁珠摇匀复位
LH_ERR ActionMixBead_Reset(void)
{
    //首先升降复位
	LH_ERR errorCode;
    //电机复位结构体
    SM_RESET_CMD resetCommand;
    int32_t yogi;
    //结构体初始化
    Can2SubSM_ResetCmdDataStructInit(&resetCommand);

    //摇匀升降步进电机复位
    SystemReadActionParamWithIndex(MODULEBEADMIXPARAM,BEADMIXUPDOWNRESETCORRECTION,&yogi);
    resetCommand.stepMotorIndex = STEP_MOTOR_SHAKE_BEAD_UPDOWN;
    resetCommand.correctionPosition = yogi;//actionParam->moduleBeadMixParam.upDownResetCorrection;
    resetCommand.timeOutMs = 60000;
    errorCode = Can2SubSM_ResetWhileReturn(&resetCommand);

    if(errorCode != LH_ERR_NONE)
	{
        //打印错误日志
        TaskServiceSerialErrMsgShow(errorCode);
        //返回错误代码
		return errorCode;
	}


	//摇匀电机旋转复位
    SystemReadActionParamWithIndex(MODULEBEADMIXPARAM,BEADMIXROTATERESETCORRECTION,&yogi);
    resetCommand.stepMotorIndex = STEP_MOTOR_SHAKE_BEAD_ROTATE;
    resetCommand.correctionPosition = yogi;//actionParam->moduleBeadMixParam.rotateResetCorrection;
    resetCommand.timeOutMs = 60000;
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

//磁珠摇匀上升
LH_ERR ActionMixBead_Up(void)
{
    LH_ERR errorCode;
    SM_RUN_COORDINATE_CMD runCoordinateCmd;
    Can2SubSM_RunCoordinateDataStructInit(&runCoordinateCmd);
    int32_t yogi;
    //摇匀升降电机升到摇匀位置
    SystemReadActionParamWithIndex(MODULEBEADMIXPARAM,BEADMIXUPDOWNTOPPOS,&yogi);
    runCoordinateCmd.stepMotorIndex = STEP_MOTOR_SHAKE_BEAD_UPDOWN;
    runCoordinateCmd.targetCoordinate = yogi;//actionParam->moduleBeadMixParam.upDownTopPos;
    runCoordinateCmd.curveSelect = SM_CURVE_0;
    runCoordinateCmd.speedRatio = 100;
    runCoordinateCmd.speedMode = SPEED_MODE_UP_START;
    runCoordinateCmd.utilStop1Enable = DISABLE;
    runCoordinateCmd.utilStop2Enable = DISABLE;
    runCoordinateCmd.timeOutMs = 60000;

	//摇匀升降电机升到摇匀位置
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

//磁珠摇匀下降
LH_ERR ActionMixBead_Down(void)
{
    LH_ERR errorCode;
    SM_RUN_COORDINATE_CMD runCoordinateCmd;
    Can2SubSM_RunCoordinateDataStructInit(&runCoordinateCmd);
    int32_t yogi;
    //摇匀升降电机下降到下降位置
    SystemReadActionParamWithIndex(MODULEBEADMIXPARAM,BEADMIXUPDOWNBUTTOMPOS,&yogi);
    runCoordinateCmd.stepMotorIndex = STEP_MOTOR_SHAKE_BEAD_UPDOWN;
    runCoordinateCmd.targetCoordinate = yogi;//actionParam->moduleBeadMixParam.upDownButtomPos;
    runCoordinateCmd.curveSelect = SM_CURVE_0;
    runCoordinateCmd.speedRatio = 100;
    runCoordinateCmd.speedMode = SPEED_MODE_UP_START;
    runCoordinateCmd.utilStop1Enable = DISABLE;
    runCoordinateCmd.utilStop2Enable = DISABLE;
    runCoordinateCmd.timeOutMs = 60000;

	//摇匀升降电机升到摇匀位置
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

//磁珠摇匀混匀指定时间
LH_ERR ActionMixBead_RotateAnyTimes(uint16_t timeMs)
{
    LH_ERR errorCode;
    SM_RUN_ALWAYS_CMD runAlwaysCmd;
    Can2SubSM_RunAlwaysDataStructInit(&runAlwaysCmd);

    //持续运转
    runAlwaysCmd.stepMotorIndex = STEP_MOTOR_SHAKE_BEAD_ROTATE;
    runAlwaysCmd.curveSelect = SM_CURVE_0;
    runAlwaysCmd.speedRatio = 100;
    runAlwaysCmd.speedMode = SPEED_MODE_UP_START;

	//摇匀电机开始旋转
    errorCode = Can2SubSM_RunAlwaysWhileReturn(&runAlwaysCmd);
    if(errorCode != LH_ERR_NONE)
	{
        //打印错误日志
        TaskServiceSerialErrMsgShow(errorCode);
        //返回错误代码
		return errorCode;
	}

	//延时指定时间
	CoreDelayMs(timeMs);

	//摇匀电机逐渐停止
	errorCode = Can2SubSM_StopGraduallyWhileReturn(STEP_MOTOR_SHAKE_BEAD_ROTATE);
	
    if(errorCode != LH_ERR_NONE)
	{
        //打印错误日志
        TaskServiceSerialErrMsgShow(errorCode);
        //返回错误代码
		return errorCode;
	}
    return errorCode;
}







