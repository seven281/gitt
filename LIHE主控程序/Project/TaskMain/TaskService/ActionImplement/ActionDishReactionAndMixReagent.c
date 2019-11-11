#include "ActionDishReactionAndMixReagent.h"

//反应盘最大杯数量
#define DISH_REACTION_CUP_MAX   64

//反应盘光电处有无被光电状态
#define REACTION_LIGHT_CUP_SENSOR_VALID_STATUS      Bit_SET

//反应盘初始化
LH_ERR ActionDishReaction_Reset(uint16_t* currentHoleIndex,ACTION_SENSOR_STATUS* reactionLightSensorHadCup)
{
    LH_ERR errorCode;
    BitAction lightCupSensor;
    int32_t yogi;
    //电机复位结构体
    SM_RESET_CMD resetCommand;
    //结构体初始化
    Can2SubSM_ResetCmdDataStructInit(&resetCommand);

    //复位参数设置
    SystemReadActionParamWithIndex(MODULEREACTIONPARAM,RESETCORRECTION,&yogi);
    resetCommand.stepMotorIndex = STEP_MOTOR_DISH_REACTION_ROTATE;
    resetCommand.correctionPosition = yogi;//actionParam->moduleReactionParam.resetCorrection;
    resetCommand.timeOutMs = 60000;
    
    //反应盘步进电机复位
    errorCode = Can2SubSM_ResetWhileReturn(&resetCommand);
    if(errorCode != LH_ERR_NONE)
	{
        //打印错误日志
        TaskServiceSerialErrMsgShow(errorCode);
        //返回错误代码
		return errorCode;
	}

    //读取光电讯号
    CoreDelayMs(50);
    errorCode = SystemInputRead(DISH_REACTION_LIGHT_SENSOR,&lightCupSensor);
	if(errorCode != LH_ERR_NONE)
	{
        //打印错误日志
        TaskServiceSerialErrMsgShow(errorCode);
        //返回错误代码
		return errorCode;
	}
    //设置当前杯号 31
    TaskUtilServiceSetDishReactionCurrentCupIndex(31);

    //转换出想要的数据
    *currentHoleIndex = TaskUtilServiceGetDishReactionCurrentCupIndex();
    if(lightCupSensor == REACTION_LIGHT_CUP_SENSOR_VALID_STATUS)
    {
        *reactionLightSensorHadCup = ACTION_SENSOR_STATUS_EXIST;
    }
    else
    {
        *reactionLightSensorHadCup = ACTION_SENSOR_STATUS_NOT_EXIST;
    }
	return errorCode;
}

//反应盘移动到指定杯位
LH_ERR ActionDishReaction_Move2SpecialCup(uint16_t targetHoleIndex,uint16_t* currentHoleIndex,ACTION_SENSOR_STATUS* reactionLightSensorHadCup)
{
    LH_ERR errorCode;
    BitAction lightCupSensor;
    int32_t posValue = 0;
    uint16_t currentindex;
    //参数检测
    if(targetHoleIndex > DISH_REACTION_CUP_MAX)
    {
        errorCode = LH_ERR_ACTION_PARAM;
        TaskServiceSerialErrMsgShow(errorCode);
        return errorCode;
    }
    else if(targetHoleIndex == 0)
    {
        errorCode = LH_ERR_ACTION_PARAM;
        TaskServiceSerialErrMsgShow(errorCode);
        return errorCode;
    }
    //读取反应盘当前序号
    currentindex = TaskUtilServiceGetDishReactionCurrentCupIndex();

    //计算坐标
    if(targetHoleIndex > currentindex)
    {
        posValue = (targetHoleIndex - currentindex)*(actionParam->moduleReactionParam.oneCupOffset);
    }
    else if(targetHoleIndex < currentindex)
    {
        posValue = (DISH_REACTION_CUP_MAX - currentindex + targetHoleIndex)*(actionParam->moduleReactionParam.oneCupOffset);
    }
    else
    {
        posValue = 0;
    }

    SM_RUN_STEP_CMD runStepCommand;
    //走位结构体初始化
    Can2SubSM_RunStepsCmdDataStructInit(&runStepCommand);


    //反应盘运转
    //移动坐标到指定杯位
    runStepCommand.stepMotorIndex = STEP_MOTOR_DISH_REACTION_ROTATE;
    runStepCommand.steps = posValue;
    runStepCommand.curveSelect = SM_CURVE_0;
    runStepCommand.speedRatio = 100;
    runStepCommand.speedMode = SPEED_MODE_UP_START;
    runStepCommand.utilStop1Enable = DISABLE;
    runStepCommand.utilStop2Enable = DISABLE;
    runStepCommand.timeOutMs = 60000;
    errorCode = Can2SubSM_RunSpecialStepsWhileReturn(&runStepCommand);
    if(errorCode != LH_ERR_NONE)
	{
        //打印错误日志
        TaskServiceSerialErrMsgShow(errorCode);
        //返回错误代码
		return errorCode;
	}

    //更新杯位
    TaskUtilServiceSetDishReactionCurrentCupIndex(targetHoleIndex);

    //读取光电讯号
    CoreDelayMs(50);
    errorCode = SystemInputRead(DISH_REACTION_LIGHT_SENSOR,&lightCupSensor);
	if(errorCode != LH_ERR_NONE)
	{
        //打印错误日志
        TaskServiceSerialErrMsgShow(errorCode);
        //返回错误代码
		return errorCode;
	}

    //转换出想要的数据
    *currentHoleIndex = TaskUtilServiceGetDishReactionCurrentCupIndex();
    if(lightCupSensor == REACTION_LIGHT_CUP_SENSOR_VALID_STATUS)
    {
        *reactionLightSensorHadCup = ACTION_SENSOR_STATUS_EXIST;
    }
    else
    {
        *reactionLightSensorHadCup = ACTION_SENSOR_STATUS_NOT_EXIST;
    }
	return errorCode;
}

//反应盘移动指定个杯位
LH_ERR ActionDishReaction_MoveSpecialCups(uint16_t holeOffset,uint16_t* currentHoleIndex,ACTION_SENSOR_STATUS* reactionLightSensorHadCup)
{
    LH_ERR errorCode;
    BitAction lightCupSensor;
    int32_t posValue = 0;
    uint16_t currentindex;
    //参数检测
    if(holeOffset > DISH_REACTION_CUP_MAX)
    {
        errorCode = LH_ERR_ACTION_PARAM;
        TaskServiceSerialErrMsgShow(errorCode);
        return errorCode;
    }
    else if(holeOffset == 0)
    {
        errorCode = LH_ERR_ACTION_PARAM;
        TaskServiceSerialErrMsgShow(errorCode);
        return errorCode;
    }

    //计算偏移
	posValue = holeOffset*(actionParam->moduleReactionParam.oneCupOffset);

    SM_RUN_STEP_CMD runStepCommand;
    //走位结构体初始化
    Can2SubSM_RunStepsCmdDataStructInit(&runStepCommand);

    //反应盘运转
    //移动坐标到指定杯位
    runStepCommand.stepMotorIndex = STEP_MOTOR_DISH_REACTION_ROTATE;
    runStepCommand.steps = posValue;
    runStepCommand.curveSelect = SM_CURVE_0;
    runStepCommand.speedRatio = 100;
    runStepCommand.speedMode = SPEED_MODE_UP_START;
    runStepCommand.utilStop1Enable = DISABLE;
    runStepCommand.utilStop2Enable = DISABLE;
    runStepCommand.timeOutMs = 60000;
    errorCode = Can2SubSM_RunSpecialStepsWhileReturn(&runStepCommand);
    if(errorCode != LH_ERR_NONE)
	{
        //打印错误日志
        TaskServiceSerialErrMsgShow(errorCode);
        //返回错误代码
		return errorCode;
	}
    //获取当前杯位
    currentindex = TaskUtilServiceGetDishReactionCurrentCupIndex();
    //更新杯位
    currentindex += holeOffset;
    if(currentindex > DISH_REACTION_CUP_MAX)
    {
        currentindex -= DISH_REACTION_CUP_MAX;
    }

    //读取光电讯号
    CoreDelayMs(50);
    errorCode = SystemInputRead(DISH_REACTION_LIGHT_SENSOR,&lightCupSensor);
	if(errorCode != LH_ERR_NONE)
	{
        //打印错误日志
        TaskServiceSerialErrMsgShow(errorCode);
        //返回错误代码
		return errorCode;
	}
    //写入当前杯位
    TaskUtilServiceSetDishReactionCurrentCupIndex(currentindex);

    //转换出想要的数据
    *currentHoleIndex = currentindex;
    if(lightCupSensor == REACTION_LIGHT_CUP_SENSOR_VALID_STATUS)
    {
        *reactionLightSensorHadCup = ACTION_SENSOR_STATUS_EXIST;
    }
    else
    {
        *reactionLightSensorHadCup = ACTION_SENSOR_STATUS_NOT_EXIST;
    }
	return errorCode;
}

//试剂摇匀复位
LH_ERR ActionMixReagent_Reset(void)
{
    //首先升降复位
	LH_ERR errorCode;
    //电机复位结构体
    SM_RESET_CMD resetCommand;
    //结构体初始化
    Can2SubSM_ResetCmdDataStructInit(&resetCommand);

    //摇匀升降步进电机复位
    resetCommand.stepMotorIndex = STEP_MOTOR_SHAKE_REAGENT_UPDOWN;
    resetCommand.correctionPosition = actionParam->moduleReagentMixParam.upDownResetCorrection;
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
    resetCommand.stepMotorIndex = STEP_MOTOR_SHAKE_REAGENT_ROTATE;
    resetCommand.correctionPosition = actionParam->moduleReagentMixParam.rotateResetCorrection;
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

//试剂摇匀上升
LH_ERR ActionMixReagent_Up(void)
{
    LH_ERR errorCode;
    SM_RUN_COORDINATE_CMD runCoordinateCmd;
    Can2SubSM_RunCoordinateDataStructInit(&runCoordinateCmd);

    //摇匀升降电机升到摇匀位置
    runCoordinateCmd.stepMotorIndex = STEP_MOTOR_SHAKE_REAGENT_UPDOWN;
    runCoordinateCmd.targetCoordinate = actionParam->moduleReagentMixParam.upDownTopPos;
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

//试剂摇匀下降
LH_ERR ActionMixReagent_Down(void)
{
    LH_ERR errorCode;
    SM_RUN_COORDINATE_CMD runCoordinateCmd;
    Can2SubSM_RunCoordinateDataStructInit(&runCoordinateCmd);

    //摇匀升降电机下降到下降位置
    runCoordinateCmd.stepMotorIndex = STEP_MOTOR_SHAKE_REAGENT_UPDOWN;
    runCoordinateCmd.targetCoordinate = actionParam->moduleReagentMixParam.upDownButtomPos;
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

//试剂摇匀混匀指定时间
LH_ERR ActionMixReagent_RotateAnyTimes(uint16_t timeMs)
{
    LH_ERR errorCode;
    SM_RUN_ALWAYS_CMD runAlwaysCmd;
    Can2SubSM_RunAlwaysDataStructInit(&runAlwaysCmd);

    //次序运转
    runAlwaysCmd.stepMotorIndex = STEP_MOTOR_SHAKE_REAGENT_ROTATE;
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
	errorCode = Can2SubSM_StopGraduallyWhileReturn(STEP_MOTOR_SHAKE_REAGENT_ROTATE);
	
    if(errorCode != LH_ERR_NONE)
	{
        //打印错误日志
        TaskServiceSerialErrMsgShow(errorCode);
        //返回错误代码
		return errorCode;
	}
    return errorCode;
}








