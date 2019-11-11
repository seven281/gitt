#include "ActionRankPathWay.h"

//当前试管编号
//static uint8_t currentTubeIndex = 1;

//试管架传送带初始化
LH_ERR ActionRankPathWay_Init(uint8_t* tubeIndexResult)
{
    LH_ERR errorCode;

    //电机复位结构体
    SM_RESET_CMD resetCommand;
    //结构体初始化
    Can2SubSM_ResetCmdDataStructInit(&resetCommand);

    //复位参数设置
    resetCommand.stepMotorIndex = STEP_MOTOR_TEST_TUBE_RACK_CONVEYOR;
    resetCommand.correctionPosition = actionParam->modulePathWayParam.resetCorrection;
    resetCommand.timeOutMs = 60000;
    //轨道传送带复位
    errorCode = Can2SubSM_ResetWhileReturn(&resetCommand);

    if(errorCode != LH_ERR_NONE)
	{
        //打印错误日志
        TaskServiceSerialErrMsgShow(errorCode);
        //返回错误代码
		return errorCode;
	}
    TaskUtilServiceSetRankPathWayTestTubeIndex(1);

    *tubeIndexResult = TaskUtilServiceGetRankPathWayTestTubeIndex();
	return errorCode;
}

//试管架传送带移动到指定试管位
LH_ERR ActionRankPathWay_Move2SpecialTube(uint16_t tubeIndex,uint8_t* tubeIndexResult)
{
    int32_t posMove = 0;
	LH_ERR errorCode;

	if(tubeIndex == 0)
    {
        //参数错误
        errorCode = LH_ERR_ACTION_PARAM;
        //打印错误日志
        TaskServiceSerialErrMsgShow(errorCode);
        //返回错误代码
		return errorCode;
    }
    //参数错误
    else if(tubeIndex > 10)
    {
        errorCode = LH_ERR_ACTION_PARAM;
        //打印错误日志
        TaskServiceSerialErrMsgShow(errorCode);
        //返回错误代码
		return errorCode;
    }

	//不跨越格子
    if((tubeIndex>=1)&&(tubeIndex<=5))
	{
		//一格试管架偏移
		posMove = actionParam->modulePathWayParam.oneTestTubeOffset;
		//移动几个试管架
		posMove *= (tubeIndex - TaskUtilServiceGetRankPathWayTestTubeIndex());
		//记录新试管架坐标
        TaskUtilServiceSetRankPathWayTestTubeIndex(tubeIndex);
	}
	else
	{
		//首先将当前试管架运行完成
        posMove = (5-TaskUtilServiceGetRankPathWayTestTubeIndex())*(actionParam->modulePathWayParam.oneTestTubeOffset);
		//然后运行一个架子之间偏移
        posMove += (actionParam->modulePathWayParam.rankOverOffset);
		//新架子上运行的偏移加上去
        posMove += (tubeIndex-6)*(actionParam->modulePathWayParam.oneTestTubeOffset);
		//计算新的index
        TaskUtilServiceSetRankPathWayTestTubeIndex(tubeIndex%5);
	}


    SM_RUN_STEP_CMD runStepCommand;
    //走位结构体初始化
    Can2SubSM_RunStepsCmdDataStructInit(&runStepCommand);

    //移动坐标到指定杯位
    runStepCommand.stepMotorIndex = STEP_MOTOR_TEST_TUBE_RACK_CONVEYOR;
    runStepCommand.steps = posMove;
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

    *tubeIndexResult = TaskUtilServiceGetRankPathWayTestTubeIndex();
    return errorCode;
}

//试管架传送带回退一个架子
LH_ERR ActionRankPathWay_BackOneRack(uint8_t* tubeIndexResult)
{
    LH_ERR errorCode;
    int32_t posMove = 0;
    
	//移动四个试管号
    posMove = (actionParam->modulePathWayParam.oneTestTubeOffset);
    posMove *= 4;
    //移动一个空隙
    posMove += (actionParam->modulePathWayParam.rankOverOffset);
	//反向运行
    posMove = 0-posMove;
    
    SM_RUN_STEP_CMD runStepCommand;
    //走位结构体初始化
    Can2SubSM_RunStepsCmdDataStructInit(&runStepCommand);

    //移动坐标到指定杯位
    runStepCommand.stepMotorIndex = STEP_MOTOR_TEST_TUBE_RACK_CONVEYOR;
    runStepCommand.steps = posMove;
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

    //移动了一整个架子,所以试管号码不变
    *tubeIndexResult = TaskUtilServiceGetRankPathWayTestTubeIndex();
	return errorCode;
}









