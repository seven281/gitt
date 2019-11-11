#include "ActionDishReagent.h"

//试剂盘的试剂仓总数
#define REAGENT_MAX_HOLE_COUNT      30

//当前试剂盘位置 试剂仓位置
//static uint8_t currentReagentIndex = 1; 

//试剂盘当前偏移
//static DISH_REAGENT_OFFSET currentOffsetFlag = DISH_REAGENT_OFFSET_NONE;

//试剂盘初始化
LH_ERR ActionDishReagent_Reset(uint16_t* currentHoleIndex,DISH_REAGENT_OFFSET* offsetFlag)
{
    LH_ERR errorCode;
    int32_t yogi;
    //电机复位结构体
    SM_RESET_CMD resetCommand;
    //结构体初始化
    Can2SubSM_ResetCmdDataStructInit(&resetCommand);

    //复位参数设置
    SystemReadActionParamWithIndex(MODULEDISHREAGENTPARAM,DISHREAGENTRESETCORRECTION,&yogi);
    resetCommand.stepMotorIndex = STEP_MOTOR_DISH_REAGENT_ROTATE;
    resetCommand.correctionPosition = yogi;
    resetCommand.timeOutMs = 60000;
    //试剂盘步进电机复位
    errorCode = Can2SubSM_ResetWhileReturn(&resetCommand);

    if(errorCode != LH_ERR_NONE)
	{
        //打印错误日志
        TaskServiceSerialErrMsgShow(errorCode);
        //返回错误代码
		return errorCode;
	}
    TaskUtilServiceSetDishReagentCurrentCupIndex(1);

    TaskUtilServiceSetDishReagentCurrentOffset(DISH_REAGENT_OFFSET_NONE);

    *currentHoleIndex = TaskUtilServiceGetDishReagentCurrentCupIndex();
    *offsetFlag = TaskUtilServiceGetDishReagentCurrentOffset();
	return errorCode;
}

//移动到指定孔位带偏移
LH_ERR ActionDishReagent_Move2SpecialHoleWithOffset(uint16_t* currentHoleIndex,DISH_REAGENT_OFFSET* offsetFlag,uint16_t targetHoleIndex,DISH_REAGENT_OFFSET offset)
{
    int32_t posValue = 0;
    LH_ERR errorCode;
    if(targetHoleIndex == 0)
    {
        errorCode = LH_ERR_ACTION_PARAM;
        TaskServiceSerialErrMsgShow(errorCode);
		return errorCode;
    }
    if(targetHoleIndex > REAGENT_MAX_HOLE_COUNT)
    {
        errorCode = LH_ERR_ACTION_PARAM;
        TaskServiceSerialErrMsgShow(errorCode);
		return errorCode;
    }

    //计算单纯杯位的偏移
	posValue = actionParam->moduleDishReagentParam.oneCupOffset;
    posValue = posValue*((int16_t)(targetHoleIndex-1));
	//加上零位偏移
	posValue += actionParam->moduleDishReagentParam.dishReagentResetCorrection;

    switch(offset)
    {
        case DISH_REAGENT_OFFSET_NONE:
            posValue += 0;
            break;
        case DISH_REAGENT_OFFSET_SCAN:
            posValue += actionParam->moduleDishReagentParam.scanOffset;
            break;
        case DISH_REAGENT_OFFSET_R1:
            posValue += actionParam->moduleDishReagentParam.reagent1Offset;
            break;
        case DISH_REAGENT_OFFSET_R2:
            posValue += actionParam->moduleDishReagentParam.reagent2Offset;
            break;
        case DISH_REAGENT_OFFSET_BEAD:
            posValue += actionParam->moduleDishReagentParam.beadOffset;
            break;
        default:
            posValue = 0;
            break;
    }

    SM_RUN_COORDINATE_CMD runCoordinateCmd;
    Can2SubSM_RunCoordinateDataStructInit(&runCoordinateCmd);

    //移动到指定位置
    runCoordinateCmd.stepMotorIndex = STEP_MOTOR_DISH_REAGENT_ROTATE;
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
    //更新标志
    TaskUtilServiceSetDishReagentCurrentCupIndex(targetHoleIndex);
    TaskUtilServiceSetDishReagentCurrentOffset(offset);

    *currentHoleIndex = TaskUtilServiceGetDishReagentCurrentCupIndex();
    *offsetFlag = TaskUtilServiceGetDishReagentCurrentOffset();
	return errorCode;
}

//试剂盘移动指定个孔位
LH_ERR ActionDishReagent_MoveAnyHoleStep(uint16_t* currentHoleIndex,DISH_REAGENT_OFFSET* offsetFlag,uint16_t targetHoleOffset)
{
    int32_t posValue = 0;
    uint16_t targetIndex;
    uint8_t currentIndex;
    LH_ERR errorCode;

    if(targetHoleOffset == 0)
    {
        errorCode = LH_ERR_ACTION_PARAM;
        //打印错误日志
        TaskServiceSerialErrMsgShow(errorCode);
        //返回错误代码
		return errorCode;
    }
    if(targetHoleOffset > REAGENT_MAX_HOLE_COUNT)
    {
        errorCode = LH_ERR_ACTION_PARAM;
        //打印错误日志
        TaskServiceSerialErrMsgShow(errorCode);
        //返回错误代码
		return errorCode;
    }
    //计算目标位置
    currentIndex = TaskUtilServiceGetDishReagentCurrentCupIndex();
    targetIndex = targetHoleOffset + currentIndex;
	if(targetIndex > 30)
    {
        targetIndex -= 30;
    }
    //计算偏移
	posValue = (targetHoleOffset*(actionParam->moduleDishReagentParam.oneCupOffset));

    SM_RUN_STEP_CMD runStepCommand;
    //走位结构体初始化
    Can2SubSM_RunStepsCmdDataStructInit(&runStepCommand);

    //移动坐标到指定杯位
    runStepCommand.stepMotorIndex = STEP_MOTOR_DISH_REAGENT_ROTATE;
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
    //更新孔位标识
    TaskUtilServiceSetDishReagentCurrentCupIndex(targetIndex);

    *currentHoleIndex = TaskUtilServiceGetDishReagentCurrentCupIndex();
    *offsetFlag = TaskUtilServiceGetDishReagentCurrentOffset();

	return errorCode;
}










