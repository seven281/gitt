#include "ActionDishWashAndMixLiquidA.h"

//清洗盘与A液混匀

//清洗盘当前杯位
//static uint8_t currentDishWashIndex = 1;

//清洗盘最大能承载杯子数量
#define DISH_WASH_MAX_CUP_NUM       10

//光电检测有效电平
#define DISH_WASH_LIGHT_SENSOR_VALID_HAS_CUP      Bit_SET


//清洗盘初始化
LH_ERR ActionDishWash_Reset(uint16_t* currentHoleIndex,ACTION_SENSOR_STATUS* washLightSensorHadCup)
{
    LH_ERR errorCode;
    BitAction lightCupSensor;
    int32_t yogi;
    //电机复位结构体
    SM_RESET_CMD resetCommand;
    //结构体初始化
    Can2SubSM_ResetCmdDataStructInit(&resetCommand);

    //复位参数设置
    SystemReadActionParamWithIndex(MODULEDISHWASHLIQUIDAPARAM,DISHWASHRESETCORRECTION,&yogi);
    resetCommand.stepMotorIndex = STEP_MOTOR_DISH_WASH_ROTATE;
    resetCommand.correctionPosition = yogi;//actionParam->moduleDishWashLiquidAParam.dishWashResetCorrection;
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
    
    //设置当前杯号
    TaskUtilServiceSetDishWashCurrentCupIndex(1);

	//等待光电稳定
	CoreDelayMs(50);
    errorCode = SystemInputRead(DISH_WASH_LIGHT_SENSOR,&lightCupSensor);
	if(errorCode != LH_ERR_NONE)
	{
        //打印错误日志
        TaskServiceSerialErrMsgShow(errorCode);
        //返回错误代码
		return errorCode;
	}

    //转换出想要的数据
    *currentHoleIndex = TaskUtilServiceGetDishWashCurrentCupIndex();
    if(lightCupSensor == DISH_WASH_LIGHT_SENSOR_VALID_HAS_CUP)
    {
        *washLightSensorHadCup = ACTION_SENSOR_STATUS_EXIST;
    }
    else
    {
        *washLightSensorHadCup = ACTION_SENSOR_STATUS_NOT_EXIST;
    }

    return errorCode;
}

//清洗盘移动到指定杯位
LH_ERR ActionDishWash_Move2SpecialCup(uint16_t targetHoleIndex,uint16_t* currentHoleIndex,ACTION_SENSOR_STATUS* washLightSensorHadCup)
{
    LH_ERR errorCode;
    BitAction lightCupSensor;
    int32_t posValue = 0;
    uint16_t indexOffset;
    uint8_t currentIndex;

    //获取清洗盘当前的杯号
    currentIndex = TaskUtilServiceGetDishWashCurrentCupIndex();
    //参数检测
    if(targetHoleIndex >= currentIndex)
    {
        indexOffset = targetHoleIndex - currentIndex;
    }
    else if(targetHoleIndex < currentIndex)
    {
        indexOffset = DISH_WASH_MAX_CUP_NUM - currentIndex;
        indexOffset += targetHoleIndex;
    }

    posValue = indexOffset * (actionParam->moduleDishWashLiquidAParam.dishWashOneCupOffset);

    SM_RUN_STEP_CMD runStepCommand;
    //走位结构体初始化
    Can2SubSM_RunStepsCmdDataStructInit(&runStepCommand);

    //移动坐标到指定杯位
    runStepCommand.stepMotorIndex = STEP_MOTOR_DISH_WASH_ROTATE;
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
    TaskUtilServiceSetDishWashCurrentCupIndex(targetHoleIndex);

	//等待光电稳定
	CoreDelayMs(50);

    //读取放杯光电信号
	errorCode = SystemInputRead(DISH_WASH_LIGHT_SENSOR,&lightCupSensor);
	if(errorCode != LH_ERR_NONE)
	{
        //打印错误日志
        TaskServiceSerialErrMsgShow(errorCode);
        //返回错误代码
		return errorCode;
	}

    //转换出想要的数据
    *currentHoleIndex = TaskUtilServiceGetDishWashCurrentCupIndex();
    if(lightCupSensor == DISH_WASH_LIGHT_SENSOR_VALID_HAS_CUP)
    {
        *washLightSensorHadCup = ACTION_SENSOR_STATUS_EXIST;
    }
    else
    {
        *washLightSensorHadCup = ACTION_SENSOR_STATUS_NOT_EXIST;
    }
    return errorCode;
}

//清洗盘移动指定个杯位
LH_ERR ActionDishWash_MoveSpecialCups(uint16_t holeOffset,uint16_t* currentHoleIndex,ACTION_SENSOR_STATUS* washLightSensorHadCup)
{
    LH_ERR errorCode;
    BitAction lightCupSensor;
    int32_t posValue = 0;
    uint16_t indexResult = 0;
    uint8_t currentIndex;
    //获取清洗盘当前杯号
    currentIndex = TaskUtilServiceGetDishWashCurrentCupIndex();
    //计算结束之后的坐标
    indexResult = currentIndex + holeOffset;
	if(indexResult > DISH_WASH_MAX_CUP_NUM)
	{
		indexResult -= DISH_WASH_MAX_CUP_NUM;
	}

    //计算运动坐标
    posValue = actionParam->moduleDishWashLiquidAParam.dishWashOneCupOffset;
	posValue *= holeOffset;

    SM_RUN_STEP_CMD runStepCommand;
    //走位结构体初始化
    Can2SubSM_RunStepsCmdDataStructInit(&runStepCommand);

    //移动坐标到指定杯位
    runStepCommand.stepMotorIndex = STEP_MOTOR_DISH_WASH_ROTATE;
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
    TaskUtilServiceSetDishWashCurrentCupIndex(indexResult);

	//等待光电稳定
	CoreDelayMs(50);

    //读取放杯光电信号
	errorCode = SystemInputRead(DISH_WASH_LIGHT_SENSOR,&lightCupSensor);
	if(errorCode != LH_ERR_NONE)
	{
        //打印错误日志
        TaskServiceSerialErrMsgShow(errorCode);
        //返回错误代码
		return errorCode;
	}

    //转换出想要的数据
    *currentHoleIndex = TaskUtilServiceGetDishWashCurrentCupIndex();
    if(lightCupSensor == DISH_WASH_LIGHT_SENSOR_VALID_HAS_CUP)
    {
        *washLightSensorHadCup = ACTION_SENSOR_STATUS_EXIST;
    }
    else
    {
        *washLightSensorHadCup = ACTION_SENSOR_STATUS_NOT_EXIST;
    }

    return errorCode;
}

//A液摇匀复位
LH_ERR ActionMixLiquidA_Reset(void)
{
    //首先升降复位
	LH_ERR errorCode;
    //电机复位结构体
    SM_RESET_CMD resetCommand;
    //结构体初始化
    Can2SubSM_ResetCmdDataStructInit(&resetCommand);

    int32_t yogi;
    //摇匀升降步进电机复位
    SystemReadActionParamWithIndex(MODULELIQUIDMIXAPARAM,AMIXUPDOWNRESETCORRECTION,&yogi);
    resetCommand.stepMotorIndex = STEP_MOTOR_SHAKE_WASH_UPDOWN;
    resetCommand.correctionPosition = yogi;//actionParam->moduleLiquidMixAParam.upDownResetCorrection;
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
    SystemReadActionParamWithIndex(MODULELIQUIDMIXAPARAM,AMIXROTATERESETCORRECTION,&yogi);
    resetCommand.stepMotorIndex = STEP_MOTOR_SHAKE_WASH_ROTATE;
    resetCommand.correctionPosition = yogi;//actionParam->moduleLiquidMixAParam.rotateResetCorrection;
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

//A液摇匀上升
LH_ERR ActionMixLiquidA_Up(void)
{
    int32_t yogi;
    LH_ERR errorCode;
    SM_RUN_COORDINATE_CMD runCoordinateCmd;
    Can2SubSM_RunCoordinateDataStructInit(&runCoordinateCmd);

    //摇匀升降电机升到摇匀位置
    SystemReadActionParamWithIndex(MODULELIQUIDMIXAPARAM,AMIXUPDOWNTOPPOS,&yogi);
    runCoordinateCmd.stepMotorIndex = STEP_MOTOR_SHAKE_WASH_UPDOWN;
    runCoordinateCmd.targetCoordinate = yogi;//actionParam->moduleLiquidMixAParam.upDownTopPos;
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

//A液摇匀下降
LH_ERR ActionMixLiquidA_Down(void)
{
    LH_ERR errorCode;
    SM_RUN_COORDINATE_CMD runCoordinateCmd;
    int32_t yogi;
    Can2SubSM_RunCoordinateDataStructInit(&runCoordinateCmd);

    //摇匀升降电机下降到下降位置
    SystemReadActionParamWithIndex(MODULELIQUIDMIXAPARAM,AMIXUPDOWNBUTTOMPOS,&yogi);
    runCoordinateCmd.stepMotorIndex = STEP_MOTOR_SHAKE_WASH_UPDOWN;
    runCoordinateCmd.targetCoordinate = yogi;//actionParam->moduleLiquidMixAParam.upDownButtomPos;
    runCoordinateCmd.curveSelect = SM_CURVE_0;
    runCoordinateCmd.speedRatio = 100;
    runCoordinateCmd.speedMode = SPEED_MODE_UP_START;
    runCoordinateCmd.utilStop1Enable = DISABLE;
    runCoordinateCmd.utilStop2Enable = DISABLE;
    runCoordinateCmd.timeOutMs = 60000;
	errorCode = Can2SubSM_RunToCoordinateWhileReturn(&runCoordinateCmd);
    
	if(errorCode != LH_ERR_NONE)
	{
        //打印错误日志
        TaskServiceSerialErrMsgShow(errorCode);
        //返回错误代码
		return errorCode;
	}

    //A液混匀在完成之后需要复位,降下来以后复位
    //电机复位结构体
    SM_RESET_CMD resetCommand;
    //结构体初始化
    Can2SubSM_ResetCmdDataStructInit(&resetCommand);

    SystemReadActionParamWithIndex(MODULELIQUIDMIXAPARAM,AMIXROTATERESETCORRECTION,&yogi);
    resetCommand.stepMotorIndex = STEP_MOTOR_SHAKE_WASH_ROTATE;
    resetCommand.correctionPosition = yogi;//actionParam->moduleLiquidMixAParam.rotateResetCorrection;
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

//A液摇匀混匀指定时间
LH_ERR ActionMixLiquidA_RotateAnyTimes(uint16_t timeMs)
{
    LH_ERR errorCode;
    SM_RUN_ALWAYS_CMD runAlwaysCmd;
    Can2SubSM_RunAlwaysDataStructInit(&runAlwaysCmd);

    //次序运转
    runAlwaysCmd.stepMotorIndex = STEP_MOTOR_SHAKE_WASH_ROTATE;
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
	errorCode = Can2SubSM_StopGraduallyWhileReturn(STEP_MOTOR_SHAKE_WASH_ROTATE);
	
    if(errorCode != LH_ERR_NONE)
	{
        //打印错误日志
        TaskServiceSerialErrMsgShow(errorCode);
        //返回错误代码
		return errorCode;
	}

    return errorCode;
}







