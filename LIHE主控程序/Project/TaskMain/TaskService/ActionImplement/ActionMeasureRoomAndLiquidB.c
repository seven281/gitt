#include "ActionMeasureRoomAndLiquidB.h"
#include "TSC_TestMode.h"

typedef enum LIQUIDS_STATE
{
    EMPTY,
    HAVE,
}LIQUIDS_STATE;
//测量室初始化
LH_ERR ActionMeasureRoom_Init(void)
{
    LH_ERR errorCode;
    //电机复位结构体
    SM_RESET_CMD resetCommand;
    //结构体初始化
    Can2SubSM_ResetCmdDataStructInit(&resetCommand);

    SM_RUN_COORDINATE_CMD runCoordinateCmd;
    Can2SubSM_RunCoordinateDataStructInit(&runCoordinateCmd);
        
    int32_t yogi;
    //复位参数设置
    SystemReadActionParamWithIndex(MODULEMEASUREROOMANDLIQUIDBPARAM,WINDOWRESETCORRECTION,&yogi);
    resetCommand.stepMotorIndex = STEP_MOTOR_MEASUREROOM_WINDOW;
    resetCommand.correctionPosition = yogi;//actionParam->moduleMeasureRoomAndLiquidBParam.windowResetCorrection;
    resetCommand.timeOutMs = 60000;
    //侧门复位
    errorCode = Can2SubSM_ResetWhileReturn(&resetCommand);
    if(errorCode != LH_ERR_NONE)
	{
        //打印错误日志
        TaskServiceSerialErrMsgShow(errorCode);
        //返回错误代码
		return errorCode;
	}

    //复位参数设置
    SystemReadActionParamWithIndex(MODULEMEASUREROOMANDLIQUIDBPARAM,UPDOORRESETCORRECTION,&yogi);
    resetCommand.stepMotorIndex = STEP_MOTOR_MEASUREROOM_UP_DOOR;
    resetCommand.correctionPosition = yogi;//actionParam->moduleMeasureRoomAndLiquidBParam.upDoorResetCorrection;
    resetCommand.timeOutMs = 60000;
    //上门复位
    errorCode = Can2SubSM_ResetWhileReturn(&resetCommand);
    if(errorCode != LH_ERR_NONE)
	{
        //打印错误日志
        TaskServiceSerialErrMsgShow(errorCode);
        //返回错误代码
		return errorCode;
	}

    SM_RETURN_ZERO_CMD returnZeroCmd;
    Can2SubSM_ReturnZeroDataStructInit(&returnZeroCmd);
    //侧门关闭,防止曝光
    SystemReadActionParamWithIndex(MODULEMEASUREROOMANDLIQUIDBPARAM,WINDOWCLOSEPOS,&yogi);
    returnZeroCmd.stepMotorIndex = STEP_MOTOR_MEASUREROOM_WINDOW;
    returnZeroCmd.correctionPosition = yogi;//actionParam->moduleMeasureRoomAndLiquidBParam.windowClosePos;
    returnZeroCmd.curveSelect = SM_CURVE_0;
    returnZeroCmd.speedRatio = 100;
    returnZeroCmd.speedMode = SPEED_MODE_UP_START;
    returnZeroCmd.utilStop1Enable = DISABLE;
    returnZeroCmd.utilStop2Enable = DISABLE;
    returnZeroCmd.timeOutMs = 60000;

    //电机走坐标
	errorCode = Can2SubSM_ReturnZeroWhileReturn(&returnZeroCmd);
    
	if(errorCode != LH_ERR_NONE)
	{
        //打印错误日志
        TaskServiceSerialErrMsgShow(errorCode);
        //返回错误代码
		return errorCode;
	}

    //上门打开,防止B液泵复位异常,B液溅出
    SystemReadActionParamWithIndex(MODULEMEASUREROOMANDLIQUIDBPARAM,UPDOORFULLOPENPOS,&yogi);
    runCoordinateCmd.stepMotorIndex = STEP_MOTOR_MEASUREROOM_UP_DOOR;
    runCoordinateCmd.targetCoordinate = yogi;//actionParam->moduleMeasureRoomAndLiquidBParam.upDoorFullOpenPos;
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

    //复位参数设置
    resetCommand.stepMotorIndex = STEP_MOTOR_PUMP_LIQUID_B;
    resetCommand.correctionPosition = 0;
    resetCommand.timeOutMs = 60000;
    //B液泵复位
    errorCode = Can2SubSM_ResetWhileReturn(&resetCommand);
    if(errorCode != LH_ERR_NONE)
	{
        //打印错误日志
        TaskServiceSerialErrMsgShow(errorCode);
        //返回错误代码
		return errorCode;
	}

    //上门关闭
    SystemReadActionParamWithIndex(MODULEMEASUREROOMANDLIQUIDBPARAM,UPDOORCLOSEPOS,&yogi);
    runCoordinateCmd.stepMotorIndex = STEP_MOTOR_MEASUREROOM_UP_DOOR;
    runCoordinateCmd.targetCoordinate = yogi;//actionParam->moduleMeasureRoomAndLiquidBParam.upDoorClosePos;
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

    return errorCode;
}

//测量室上门全开
LH_ERR ActionMeasureRoom_UpDoorOpenFull(void)
{
    LH_ERR errorCode;

    SM_RUN_COORDINATE_CMD runCoordinateCmd;
    Can2SubSM_RunCoordinateDataStructInit(&runCoordinateCmd);
    
    SM_RETURN_ZERO_CMD returnZeroCmd;
    Can2SubSM_ReturnZeroDataStructInit(&returnZeroCmd);

    int32_t yogi;
    //侧门关闭,防止曝光
    SystemReadActionParamWithIndex(MODULEMEASUREROOMANDLIQUIDBPARAM,WINDOWCLOSEPOS,&yogi);
    returnZeroCmd.stepMotorIndex = STEP_MOTOR_MEASUREROOM_WINDOW;
    returnZeroCmd.correctionPosition = yogi;//actionParam->moduleMeasureRoomAndLiquidBParam.windowClosePos;
    returnZeroCmd.curveSelect = SM_CURVE_0;
    returnZeroCmd.speedRatio = 100;
    returnZeroCmd.speedMode = SPEED_MODE_UP_START;
    returnZeroCmd.utilStop1Enable = DISABLE;
    returnZeroCmd.utilStop2Enable = DISABLE;
    returnZeroCmd.timeOutMs = 60000;

    //电机走坐标
	errorCode = Can2SubSM_ReturnZeroWhileReturn(&returnZeroCmd);
    
	if(errorCode != LH_ERR_NONE)
	{
        //打印错误日志
        TaskServiceSerialErrMsgShow(errorCode);
        //返回错误代码
		return errorCode;
	}

     //上门打开
    SystemReadActionParamWithIndex(MODULEMEASUREROOMANDLIQUIDBPARAM,UPDOORFULLOPENPOS,&yogi);
    runCoordinateCmd.stepMotorIndex = STEP_MOTOR_MEASUREROOM_UP_DOOR;
    runCoordinateCmd.targetCoordinate = yogi;//actionParam->moduleMeasureRoomAndLiquidBParam.upDoorFullOpenPos;
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

    return errorCode;
}

//测量室上门半开
LH_ERR ActionMeasureRoom_UpDoorOpenHalf(void)
{
    LH_ERR errorCode;
    
    SM_RUN_COORDINATE_CMD runCoordinateCmd;
    Can2SubSM_RunCoordinateDataStructInit(&runCoordinateCmd);
    
    SM_RETURN_ZERO_CMD returnZeroCmd;
    Can2SubSM_ReturnZeroDataStructInit(&returnZeroCmd);

    int32_t yogi;
    //侧门关闭,防止曝光
    SystemReadActionParamWithIndex(MODULEMEASUREROOMANDLIQUIDBPARAM,WINDOWCLOSEPOS,&yogi);
    returnZeroCmd.stepMotorIndex = STEP_MOTOR_MEASUREROOM_WINDOW;
    returnZeroCmd.correctionPosition = yogi;//actionParam->moduleMeasureRoomAndLiquidBParam.windowClosePos;
    returnZeroCmd.curveSelect = SM_CURVE_0;
    returnZeroCmd.speedRatio = 100;
    returnZeroCmd.speedMode = SPEED_MODE_UP_START;
    returnZeroCmd.utilStop1Enable = DISABLE;
    returnZeroCmd.utilStop2Enable = DISABLE;
    returnZeroCmd.timeOutMs = 60000;

    //电机走坐标
	errorCode = Can2SubSM_ReturnZeroWhileReturn(&returnZeroCmd);
    
	if(errorCode != LH_ERR_NONE)
	{
        //打印错误日志
        TaskServiceSerialErrMsgShow(errorCode);
        //返回错误代码
		return errorCode;
	}

     //上门半开
    SystemReadActionParamWithIndex(MODULEMEASUREROOMANDLIQUIDBPARAM,UPDOORHALFOPENPOS,&yogi); 
    runCoordinateCmd.stepMotorIndex = STEP_MOTOR_MEASUREROOM_UP_DOOR;
    runCoordinateCmd.targetCoordinate = yogi;//actionParam->moduleMeasureRoomAndLiquidBParam.upDoorHalfOpenPos;
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

    return errorCode;
}

//测量室上门打开到灌注位
LH_ERR ActionMeasureRoom_UpDoorOpenPrime(void)
{
    LH_ERR errorCode;
    int32_t yogi;
    
    SM_RUN_COORDINATE_CMD runCoordinateCmd;
    Can2SubSM_RunCoordinateDataStructInit(&runCoordinateCmd);
        
    SM_RETURN_ZERO_CMD returnZeroCmd;
    Can2SubSM_ReturnZeroDataStructInit(&returnZeroCmd);

    //侧门关闭,防止曝光
    SystemReadActionParamWithIndex(MODULEMEASUREROOMANDLIQUIDBPARAM,WINDOWCLOSEPOS,&yogi);
    returnZeroCmd.stepMotorIndex = STEP_MOTOR_MEASUREROOM_WINDOW;
    returnZeroCmd.correctionPosition = yogi;//actionParam->moduleMeasureRoomAndLiquidBParam.windowClosePos;
    returnZeroCmd.curveSelect = SM_CURVE_0;
    returnZeroCmd.speedRatio = 100;
    returnZeroCmd.speedMode = SPEED_MODE_UP_START;
    returnZeroCmd.utilStop1Enable = DISABLE;
    returnZeroCmd.utilStop2Enable = DISABLE;
    returnZeroCmd.timeOutMs = 60000;

    //电机走坐标
	errorCode = Can2SubSM_ReturnZeroWhileReturn(&returnZeroCmd);
    
	if(errorCode != LH_ERR_NONE)
	{
        //打印错误日志
        TaskServiceSerialErrMsgShow(errorCode);
        //返回错误代码
		return errorCode;
	}

     //上门打开到灌注位
    SystemReadActionParamWithIndex(MODULEMEASUREROOMANDLIQUIDBPARAM,UPDOORPRIMEPOS,&yogi); 
    runCoordinateCmd.stepMotorIndex = STEP_MOTOR_MEASUREROOM_UP_DOOR;
    runCoordinateCmd.targetCoordinate = yogi;//actionParam->moduleMeasureRoomAndLiquidBParam.upDoorPrimePos;
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

    return errorCode;
}

//测量室上门关闭
LH_ERR ActionMeasureRoom_UpDoorClose(void)
{
    LH_ERR errorCode;

    SM_RUN_COORDINATE_CMD runCoordinateCmd;
    Can2SubSM_RunCoordinateDataStructInit(&runCoordinateCmd);
    
    SM_RETURN_ZERO_CMD returnZeroCmd;
    Can2SubSM_ReturnZeroDataStructInit(&returnZeroCmd);

    int32_t yogi;
    //侧门关闭,防止曝光
    SystemReadActionParamWithIndex(MODULEMEASUREROOMANDLIQUIDBPARAM,WINDOWCLOSEPOS,&yogi);
    returnZeroCmd.stepMotorIndex = STEP_MOTOR_MEASUREROOM_WINDOW;
    returnZeroCmd.correctionPosition = yogi;//actionParam->moduleMeasureRoomAndLiquidBParam.windowClosePos;
    returnZeroCmd.curveSelect = SM_CURVE_0;
    returnZeroCmd.speedRatio = 100;
    returnZeroCmd.speedMode = SPEED_MODE_UP_START;
    returnZeroCmd.utilStop1Enable = DISABLE;
    returnZeroCmd.utilStop2Enable = DISABLE;
    returnZeroCmd.timeOutMs = 60000;

    //电机走坐标
	errorCode = Can2SubSM_ReturnZeroWhileReturn(&returnZeroCmd);
    
	if(errorCode != LH_ERR_NONE)
	{
        //打印错误日志
        TaskServiceSerialErrMsgShow(errorCode);
        //返回错误代码
		return errorCode;
	}

     //上门关闭
    SystemReadActionParamWithIndex(MODULEMEASUREROOMANDLIQUIDBPARAM,UPDOORCLOSEPOS,&yogi);
    runCoordinateCmd.stepMotorIndex = STEP_MOTOR_MEASUREROOM_UP_DOOR;
    runCoordinateCmd.targetCoordinate = yogi;//actionParam->moduleMeasureRoomAndLiquidBParam.upDoorClosePos;
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

    return errorCode;
}



//测量窗复位
LH_ERR ActionMeasureRoom_WindowReset(void)
{
    LH_ERR errorCode;
    //电机复位结构体
    SM_RESET_CMD resetCommand;
    //结构体初始化
    Can2SubSM_ResetCmdDataStructInit(&resetCommand);

    int32_t yogi;
    //复位参数设置
    SystemReadActionParamWithIndex(MODULEMEASUREROOMANDLIQUIDBPARAM,WINDOWRESETCORRECTION,&yogi);
    resetCommand.stepMotorIndex = STEP_MOTOR_MEASUREROOM_WINDOW;
    resetCommand.correctionPosition = yogi;//actionParam->moduleMeasureRoomAndLiquidBParam.windowResetCorrection;
    resetCommand.timeOutMs = 60000;
    //侧门复位
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
//测量窗打开
LH_ERR ActionMeasureRoom_WindowOpen(void)
{
    LH_ERR errorCode;
    int32_t yogi;
    SM_RETURN_ZERO_CMD returnZeroCmd;
    Can2SubSM_ReturnZeroDataStructInit(&returnZeroCmd);

    //
    SystemReadActionParamWithIndex(MODULEMEASUREROOMANDLIQUIDBPARAM,WINDOWOPENPOS,&yogi);
    returnZeroCmd.stepMotorIndex = STEP_MOTOR_MEASUREROOM_WINDOW;
    returnZeroCmd.correctionPosition = yogi;//actionParam->moduleMeasureRoomAndLiquidBParam.windowOpenPos;
    returnZeroCmd.curveSelect = SM_CURVE_0;
    returnZeroCmd.speedRatio = 100;
    returnZeroCmd.speedMode = SPEED_MODE_UP_START;
    returnZeroCmd.utilStop1Enable = DISABLE;
    returnZeroCmd.utilStop2Enable = DISABLE;
    returnZeroCmd.timeOutMs = 60000;

    //电机走坐标
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
//测量窗关闭
LH_ERR ActionMeasureRoom_WindowClose(void)
{
    LH_ERR errorCode;
    int32_t yogi;
    SM_RETURN_ZERO_CMD returnZeroCmd;
    Can2SubSM_ReturnZeroDataStructInit(&returnZeroCmd);

    //
    SystemReadActionParamWithIndex(MODULEMEASUREROOMANDLIQUIDBPARAM,WINDOWCLOSEPOS,&yogi);
    returnZeroCmd.stepMotorIndex = STEP_MOTOR_MEASUREROOM_WINDOW;
    returnZeroCmd.correctionPosition = yogi;//actionParam->moduleMeasureRoomAndLiquidBParam.windowClosePos;
    returnZeroCmd.curveSelect = SM_CURVE_0;
    returnZeroCmd.speedRatio = 100;
    returnZeroCmd.speedMode = SPEED_MODE_UP_START;
    returnZeroCmd.utilStop1Enable = DISABLE;
    returnZeroCmd.utilStop2Enable = DISABLE;
    returnZeroCmd.timeOutMs = 60000;

    //电机走坐标
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
//测量室B液灌注----------老化
LH_ERR ActionMeasureRoom_PrimeOnce_Debug(void)
{
    return LH_ERR_NONE;
}
//测量室B液灌注----------正常
LH_ERR ActionMeasureRoom_PrimeOnce_Normal(void)
{
    LH_ERR errorCode;
    
    SM_RUN_COORDINATE_CMD runCoordinateCmd;
    Can2SubSM_RunCoordinateDataStructInit(&runCoordinateCmd);
    int32_t yogi;
    /*1. 判断哪个屏内有液体，并打开对应阀门*/
        //SystemInputRead()硬件暂时不支持检测液体
        if(1)//A1有液体
        {
            errorCode = SystemOutputWrite(DRAINAGE_VALVE_V203,Bit_RESET);
            if(errorCode != LH_ERR_NONE)
            {
                //打印错误日志
                TaskServiceSerialErrMsgShow(errorCode);
                //返回错误代码
                return errorCode;
            }
        }
        else if(0)//A2有液体
        {
            errorCode = SystemOutputWrite(DRAINAGE_VALVE_V204,Bit_RESET);
            if(errorCode != LH_ERR_NONE)
            {
                //打印错误日志
                TaskServiceSerialErrMsgShow(errorCode);
                //返回错误代码
                return errorCode;
            }
        }
    /*2. 测量室侧门关闭*/
        SM_RETURN_ZERO_CMD returnZeroCmd;
        Can2SubSM_ReturnZeroDataStructInit(&returnZeroCmd);

        //侧门关闭,防止曝光
        SystemReadActionParamWithIndex(MODULEMEASUREROOMANDLIQUIDBPARAM,WINDOWCLOSEPOS,&yogi);
        returnZeroCmd.stepMotorIndex = STEP_MOTOR_MEASUREROOM_WINDOW;
        returnZeroCmd.correctionPosition = yogi;//actionParam->moduleMeasureRoomAndLiquidBParam.windowClosePos;
        returnZeroCmd.curveSelect = SM_CURVE_0;
        returnZeroCmd.speedRatio = 100;
        returnZeroCmd.speedMode = SPEED_MODE_UP_START;
        returnZeroCmd.utilStop1Enable = DISABLE;
        returnZeroCmd.utilStop2Enable = DISABLE;
        returnZeroCmd.timeOutMs = 60000;

        //电机走坐标
        errorCode = Can2SubSM_ReturnZeroWhileReturn(&returnZeroCmd);
        
        if(errorCode != LH_ERR_NONE)
        {
            //打印错误日志
            TaskServiceSerialErrMsgShow(errorCode);
            //返回错误代码
            return errorCode;
        }
    /*3. 测量室上门打开到灌注位*/
        //上门打开到灌注位
        SystemReadActionParamWithIndex(MODULEMEASUREROOMANDLIQUIDBPARAM,UPDOORPRIMEPOS,&yogi); 
        runCoordinateCmd.stepMotorIndex = STEP_MOTOR_MEASUREROOM_UP_DOOR;
        runCoordinateCmd.targetCoordinate = yogi;//actionParam->moduleMeasureRoomAndLiquidBParam.upDoorPrimePos;
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
    /*4. 打开抽液泵dp2*/
        errorCode = SystemOutputWrite(WASTE_LIQUID_PUMP2,Bit_SET);
        if(errorCode != LH_ERR_NONE)
        {
            //打印错误日志
            TaskServiceSerialErrMsgShow(errorCode);
            //返回错误代码
            return errorCode;
        }
    /*5. 打开泵dp1
        errorCode = SystemOutputWrite(WASTE_LIQUID_PUMP,Bit_RESET);
        if(errorCode != LH_ERR_NONE)
        {
            //打印错误日志
            TaskServiceSerialErrMsgShow(errorCode);
            //返回错误代码
            return errorCode;
        }
    */
    /*6. B液柱塞泵m40运行指定步数*/
        SM_RUN_STEP_CMD runStepCommand;
        //走位结构体初始化
        Can2SubSM_RunStepsCmdDataStructInit(&runStepCommand);

        //B液泵注液一次
        runStepCommand.stepMotorIndex = STEP_MOTOR_PUMP_LIQUID_B;
        runStepCommand.steps = actionParam->moduleMeasureRoomAndLiquidBParam.liquidBInjectPos;
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
    /*7. 关闭泵dp1
        errorCode = SystemOutputWrite(WASTE_LIQUID_PUMP,Bit_SET);
        if(errorCode != LH_ERR_NONE)
        {
            //打印错误日志
            TaskServiceSerialErrMsgShow(errorCode);
            //返回错误代码
            return errorCode;
        }
    */
    /*8. 关闭抽液泵dp2*/
        errorCode = SystemOutputWrite(WASTE_LIQUID_PUMP2,Bit_RESET);
        if(errorCode != LH_ERR_NONE)
        {
            //打印错误日志
            TaskServiceSerialErrMsgShow(errorCode);
            //返回错误代码
            return errorCode;
        }
    /*9. B液柱塞泵m40复位*/
        //电机复位结构体
        SM_RESET_CMD resetCommand;
        //结构体初始化
        Can2SubSM_ResetCmdDataStructInit(&resetCommand);

        //复位参数设置
        resetCommand.stepMotorIndex = STEP_MOTOR_PUMP_LIQUID_B;
        resetCommand.correctionPosition = 0;
        resetCommand.timeOutMs = 60000;
        //B液泵复位
        errorCode = Can2SubSM_ResetWhileReturn(&resetCommand);
        if(errorCode != LH_ERR_NONE)
        {
            //打印错误日志
            TaskServiceSerialErrMsgShow(errorCode);
            //返回错误代码
            return errorCode;
        }
    /*10. 关闭阀v203 v204*/
        errorCode = SystemOutputWrite(DRAINAGE_VALVE_V203,Bit_SET);
        if(errorCode != LH_ERR_NONE)
        {
            //打印错误日志
            TaskServiceSerialErrMsgShow(errorCode);
            //返回错误代码
            return errorCode;
        }

        errorCode = SystemOutputWrite(DRAINAGE_VALVE_V204,Bit_SET);
        if(errorCode != LH_ERR_NONE)
        {
            //打印错误日志
            TaskServiceSerialErrMsgShow(errorCode);
            //返回错误代码
            return errorCode;
        }

    return errorCode;
}


//测量室B液灌注
LH_ERR ActionMeasureRoom_PrimeOnce(void)
{
    LH_ERR errorCode;

    TSC_TEST_MODE_SELECTION mode;
    mode = TSC_Read_TestMode();

    if(mode == TSC_TEST_MODE_NORMAL)
    {
        errorCode = ActionMeasureRoom_PrimeOnce_Normal();//正常测试
    }
    else 
    {
        errorCode = ActionMeasureRoom_PrimeOnce_Debug();//老化测试
    }

    return errorCode;
}

//测量室B液注液
LH_ERR ActionMeasureRoom_InjectOnce_Normal(void)
{
    LH_ERR errorCode;
    int32_t yogi;
    LIQUIDS_STATE B1bottle_liquids,B2bottle_liquids;
    /*1. 判断哪个瓶子有液体*/
        //  SystemInputRead();传感器没有安装也不清除连接哪个端口
        B1bottle_liquids = HAVE;
    /*2. 打开阀v203 前提B1瓶有液体
         打开阀v204 前提B2瓶有液体*/
        if(B1bottle_liquids == HAVE)
        {
            errorCode = SystemOutputWrite(DRAINAGE_VALVE_V203,Bit_RESET);
            if(errorCode != LH_ERR_NONE)
            {
                //打印错误日志
                TaskServiceSerialErrMsgShow(errorCode);
                //返回错误代码
                return errorCode;
            }
        }
        else if(B2bottle_liquids == HAVE)
        {
            errorCode = SystemOutputWrite(DRAINAGE_VALVE_V204,Bit_RESET);
            if(errorCode != LH_ERR_NONE)
            {
                //打印错误日志
                TaskServiceSerialErrMsgShow(errorCode);
                //返回错误代码
                return errorCode;
            }
        }
        else
        {
           return LH_ERR_A_AND_B_EMPTY;
        }
    /*3. 关闭上门*/
        SM_RUN_COORDINATE_CMD runCoordinateCmd;
        Can2SubSM_RunCoordinateDataStructInit(&runCoordinateCmd);
         //上门关闭
        SystemReadActionParamWithIndex(MODULEMEASUREROOMANDLIQUIDBPARAM,UPDOORCLOSEPOS,&yogi);
        runCoordinateCmd.stepMotorIndex = STEP_MOTOR_MEASUREROOM_UP_DOOR;
        runCoordinateCmd.targetCoordinate = yogi;//actionParam->moduleMeasureRoomAndLiquidBParam.upDoorClosePos;
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

    
    /*4. 点滴泵m40运行指定的步数*/
        SM_RUN_STEP_CMD runStepCommand;
        //走位结构体初始化
        Can2SubSM_RunStepsCmdDataStructInit(&runStepCommand);

        //B液泵注液一次
        runStepCommand.stepMotorIndex = STEP_MOTOR_PUMP_LIQUID_B;
        runStepCommand.steps = actionParam->moduleMeasureRoomAndLiquidBParam.liquidBInjectPos;
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
    /*5. 点滴泵m40复位*/
        //电机复位结构体
        SM_RESET_CMD resetCommand;
        //结构体初始化
        Can2SubSM_ResetCmdDataStructInit(&resetCommand);

        //复位参数设置
        resetCommand.stepMotorIndex = STEP_MOTOR_PUMP_LIQUID_B;
        resetCommand.correctionPosition = 0;
        resetCommand.timeOutMs = 60000;
        //B液泵复位
        errorCode = Can2SubSM_ResetWhileReturn(&resetCommand);
        if(errorCode != LH_ERR_NONE)
        {
            //打印错误日志
            TaskServiceSerialErrMsgShow(errorCode);
            //返回错误代码
            return errorCode;
        }
    /*6. 关闭阀v203,v204*/
        errorCode = SystemOutputWrite(DRAINAGE_VALVE_V203,Bit_SET);
        if(errorCode != LH_ERR_NONE)
        {
            //打印错误日志
            TaskServiceSerialErrMsgShow(errorCode);
            //返回错误代码
            return errorCode;
        }

        errorCode = SystemOutputWrite(DRAINAGE_VALVE_V204,Bit_SET);
        if(errorCode != LH_ERR_NONE)
        {
            //打印错误日志
            TaskServiceSerialErrMsgShow(errorCode);
            //返回错误代码
            return errorCode;
        }
    return errorCode;
}
LH_ERR ActionMeasureRoom_InjectOnce_Debug(void)
{
    LH_ERR errorCode;
    SM_RUN_COORDINATE_CMD runCoordinateCmd;
    Can2SubSM_RunCoordinateDataStructInit(&runCoordinateCmd);
    
    SM_RETURN_ZERO_CMD returnZeroCmd;
    Can2SubSM_ReturnZeroDataStructInit(&returnZeroCmd);
    int32_t yogi;
    //侧门关闭,防止曝光
    SystemReadActionParamWithIndex(MODULEMEASUREROOMANDLIQUIDBPARAM,WINDOWCLOSEPOS,&yogi);
    returnZeroCmd.stepMotorIndex = STEP_MOTOR_MEASUREROOM_WINDOW;
    returnZeroCmd.correctionPosition = yogi;//actionParam->moduleMeasureRoomAndLiquidBParam.windowClosePos;
    returnZeroCmd.curveSelect = SM_CURVE_0;
    returnZeroCmd.speedRatio = 100;
    returnZeroCmd.speedMode = SPEED_MODE_UP_START;
    returnZeroCmd.utilStop1Enable = DISABLE;
    returnZeroCmd.utilStop2Enable = DISABLE;
    returnZeroCmd.timeOutMs = 60000;

    //电机走坐标
    errorCode = Can2SubSM_ReturnZeroWhileReturn(&returnZeroCmd);

    if(errorCode != LH_ERR_NONE)
    {
        //打印错误日志
        TaskServiceSerialErrMsgShow(errorCode);
        //返回错误代码
        return errorCode;
    }

     //上门关闭

    SystemReadActionParamWithIndex(MODULEMEASUREROOMANDLIQUIDBPARAM,UPDOORCLOSEPOS,&yogi); 
    runCoordinateCmd.stepMotorIndex = STEP_MOTOR_MEASUREROOM_UP_DOOR;
    runCoordinateCmd.targetCoordinate = yogi;//actionParam->moduleMeasureRoomAndLiquidBParam.upDoorClosePos;
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
    return errorCode;
}
LH_ERR ActionMeasureRoom_InjectOnce(void)
{
    LH_ERR errorCode;

    TSC_TEST_MODE_SELECTION mode;
    mode = TSC_Read_TestMode();

    if(mode == TSC_TEST_MODE_NORMAL)
    {
        errorCode = ActionMeasureRoom_InjectOnce_Normal();//正常测试
    }
    else 
    {
        errorCode = ActionMeasureRoom_InjectOnce_Debug();//老化测试
    }

    return errorCode;
}
//测量室光子测量
LH_ERR ActionMeasureRoom_ReadValueWithInject(uint16_t measureTimeMs,uint32_t* result)
{
    LH_ERR errorCode;

    // SM_RUN_COORDINATE_CMD runCoordinateCmd;
    // Can2SubSM_RunCoordinateDataStructInit(&runCoordinateCmd);

    TSC_TEST_MODE_SELECTION mode;
    mode = TSC_Read_TestMode();

    if(mode == TSC_TEST_MODE_AGEING)//老化
    {
        CoreDelayMs(measureTimeMs);

        *result = MCU_RandomGetNextRangeReal(100,2000);
        errorCode = LH_ERR_NONE;
    }
    else
    {
        /*上门关闭*/
        errorCode =  ActionMeasureRoom_UpDoorClose();
        if(errorCode != LH_ERR_NONE)
        {
            //打印错误日志
            TaskServiceSerialErrMsgShow(errorCode);
            //返回错误代码
            return errorCode;
        }
        /*给光子光子测量器发送测光指令 */
        errorCode = Can2SubPhotonCountSingleMeasureStart(actionParam->moduleMeasureRoomAndLiquidBParam.timeMeasureNormalMs);
        if(errorCode != LH_ERR_NONE)
        {
            //打印错误日志
            TaskServiceSerialErrMsgShow(errorCode);
            //返回错误代码
            return TESTER_WINDOW_ERR_LIGHT_SENSOR_NO_DATA;
        }
        /*B液注液*/
        errorCode = ActionMeasureRoom_InjectOnce();
        if(errorCode != LH_ERR_NONE)
        {
            //打印错误日志
            TaskServiceSerialErrMsgShow(errorCode);
            //返回错误代码
            return errorCode;
        }
        /*读取测光结果值 */
        errorCode = Can2SubPhotonCountGetSingleMeasureResult(result,measureTimeMs+4000);
        if(errorCode != LH_ERR_NONE)
        {
            //打印错误日志
            TaskServiceSerialErrMsgShow(errorCode);
            //没数据,设置为0
            *result = 0;
            //返回错误代码
            return TESTER_WINDOW_ERR_LIGHT_SENSOR_NO_DATA;
        }  
        /*如果结果为0XFFFFFFFF,也当成无数据返回 */
        if(*result == 0XFFFFFFFF)
        {
            errorCode = LH_ERR_ACTION_PARAM_ERR;
            //打印错误日志
            TaskServiceSerialErrMsgShow(errorCode);
            //返回错误代码
            return TESTER_WINDOW_ERR_LIGHT_SENSOR_NO_DATA;
        }
    }

    return errorCode;
}

//测量室暗计数
LH_ERR ActionMeasureRoom_ReadValueDark(uint16_t measureTimeMs,uint32_t* result)
{
    LH_ERR errorCode;

    TSC_TEST_MODE_SELECTION mode;
    mode = TSC_Read_TestMode();

    if(mode == TSC_TEST_MODE_AGEING)//老化
    {
        CoreDelayMs(measureTimeMs);

        *result = MCU_RandomGetNextRangeReal(100,2000);
        errorCode = LH_ERR_NONE;
    }
    else
    {
        /*上门关闭*/
        errorCode =  ActionMeasureRoom_UpDoorClose();
        if(errorCode != LH_ERR_NONE)
        {
            //打印错误日志
            TaskServiceSerialErrMsgShow(errorCode);
            //返回错误代码
            return errorCode;
        }
        /*给光子光子测量器发送测光指令 */
        errorCode = Can2SubPhotonCountSingleMeasureStart(actionParam->moduleMeasureRoomAndLiquidBParam.timeMeasureNormalMs);
        if(errorCode != LH_ERR_NONE)
        {
            //打印错误日志
            TaskServiceSerialErrMsgShow(errorCode);
            //返回错误代码
            return TESTER_WINDOW_ERR_LIGHT_SENSOR_NO_DATA;
        }
        /*读取测光结果值 */
        errorCode = Can2SubPhotonCountGetSingleMeasureResult(result,measureTimeMs+4000);
        if(errorCode != LH_ERR_NONE)
        {
            //打印错误日志
            TaskServiceSerialErrMsgShow(errorCode);
            //没数据,设置为0
            *result = 0;
            //返回错误代码
            return TESTER_WINDOW_ERR_LIGHT_SENSOR_NO_DATA;
        } 
        /*如果结果为0XFFFFFFFF,也当成无数据返回 */
        if(*result == 0XFFFFFFFF)
        {
            errorCode = LH_ERR_ACTION_PARAM_ERR;
            //打印错误日志
            TaskServiceSerialErrMsgShow(errorCode);
            //返回错误代码
            return TESTER_WINDOW_ERR_LIGHT_SENSOR_NO_DATA;
        }
    }

    return errorCode;
}
//测量室读本底
LH_ERR ActionMeasureRoom_ReadValueBackgroud(uint16_t measureTimeMs,uint32_t* result)
{
    LH_ERR errorCode;

    TSC_TEST_MODE_SELECTION mode;
    mode = TSC_Read_TestMode();

    if(mode == TSC_TEST_MODE_AGEING)//老化
    {
        CoreDelayMs(measureTimeMs);

        *result = MCU_RandomGetNextRangeReal(100,2000);
        errorCode = LH_ERR_NONE;
    }
    else
    {
        /*上门关闭*/
        errorCode =  ActionMeasureRoom_UpDoorClose();
        if(errorCode != LH_ERR_NONE)
        {
            //打印错误日志
            TaskServiceSerialErrMsgShow(errorCode);
            //返回错误代码
            return errorCode;
        }
        /*给光子光子测量器发送测光指令 */
        errorCode = Can2SubPhotonCountSingleMeasureStart(actionParam->moduleMeasureRoomAndLiquidBParam.timeMeasureNormalMs);
        if(errorCode != LH_ERR_NONE)
        {
            //打印错误日志
            TaskServiceSerialErrMsgShow(errorCode);
            //返回错误代码
            return TESTER_WINDOW_ERR_LIGHT_SENSOR_NO_DATA;
        }
        /*读取测光结果值 */
        errorCode = Can2SubPhotonCountGetSingleMeasureResult(result,measureTimeMs+4000);
        if(errorCode != LH_ERR_NONE)
        {
            //打印错误日志
            TaskServiceSerialErrMsgShow(errorCode);
            //没数据,设置为0
            *result = 0;
            //返回错误代码
            return TESTER_WINDOW_ERR_LIGHT_SENSOR_NO_DATA;
        }  
        /*如果结果为0XFFFFFFFF,也当成无数据返回 */
        if(*result == 0XFFFFFFFF)
        {
            errorCode = LH_ERR_ACTION_PARAM_ERR;
            //打印错误日志
            TaskServiceSerialErrMsgShow(errorCode);
            //返回错误代码
            return TESTER_WINDOW_ERR_LIGHT_SENSOR_NO_DATA;
        }
    }
    return errorCode;
}

LH_ERR ActionMeasureRoom_ReadValue(uint16_t measureTimeMs,MEASURE_VALUE_FLAG measureflag,uint32_t* result)
{
    LH_ERR errorCode;
    int32_t measuretime;
    TSC_TEST_MODE_SELECTION mode;
    mode = TSC_Read_TestMode();
    int32_t yogi;
    if(mode == TSC_TEST_MODE_AGEING)//老化
    {
        CoreDelayMs(measureTimeMs);

        *result = MCU_RandomGetNextRangeReal(100,2000);
        errorCode = LH_ERR_NONE;
    }
    else
    {
        /*1.上门关闭*/
            errorCode =  ActionMeasureRoom_UpDoorClose();
            if(errorCode != LH_ERR_NONE)
            {
                //打印错误日志
                TaskServiceSerialErrMsgShow(errorCode);
                //返回错误代码
                return errorCode;
            }
        /*2.根据参数判断是否打开侧窗*/
            SM_RETURN_ZERO_CMD returnZeroCmd;
            Can2SubSM_ReturnZeroDataStructInit(&returnZeroCmd);
            returnZeroCmd.stepMotorIndex = STEP_MOTOR_MEASUREROOM_WINDOW;
            //本底和测光
            if((measureflag == MEASURE_FLAG_BACK)||(measureflag == MEASURE_FLAG_READ))
                returnZeroCmd.correctionPosition = actionParam->moduleMeasureRoomAndLiquidBParam.windowOpenPos;
            //暗计数
            else if(measureflag == MEASURE_FLAG_DARK)
                returnZeroCmd.correctionPosition = actionParam->moduleMeasureRoomAndLiquidBParam.windowClosePos;
            returnZeroCmd.curveSelect = SM_CURVE_0;
            returnZeroCmd.speedRatio = 100;
            returnZeroCmd.speedMode = SPEED_MODE_UP_START;
            returnZeroCmd.utilStop1Enable = DISABLE;
            returnZeroCmd.utilStop2Enable = DISABLE;
            returnZeroCmd.timeOutMs = 60000;

            //电机走坐标
            errorCode = Can2SubSM_ReturnZeroWhileReturn(&returnZeroCmd);
            
            if(errorCode != LH_ERR_NONE)
            {
                //打印错误日志
                TaskServiceSerialErrMsgShow(errorCode);
                //返回错误代码
                return errorCode;
            }
        /*3.给光子光子测量器发送测光指令 */
            if(measureflag == MEASURE_FLAG_READ)
                measuretime = actionParam->moduleMeasureRoomAndLiquidBParam.timeMeasureNormalMs;
            else 
                measuretime = actionParam->moduleMeasureRoomAndLiquidBParam.timeMeasureBaseMs;
            errorCode = Can2SubPhotonCountSingleMeasureStart(measuretime);
            if(errorCode != LH_ERR_NONE)
            {
                //打印错误日志
                TaskServiceSerialErrMsgShow(errorCode);
                //返回错误代码
                return TESTER_WINDOW_ERR_LIGHT_SENSOR_NO_DATA;
            }
        /*4.B液注液只有传递参数为测光时才会注液*/
            if(measureflag == MEASURE_FLAG_READ)
            {
                errorCode = ActionMeasureRoom_InjectOnce();
                if(errorCode != LH_ERR_NONE)
                {
                    //打印错误日志
                    TaskServiceSerialErrMsgShow(errorCode);
                    //返回错误代码
                    return errorCode;
                }
            }
        /*5.读取测光结果值 */
            errorCode = Can2SubPhotonCountGetSingleMeasureResult(result,measureTimeMs+4000);
            if(errorCode != LH_ERR_NONE)
            {
                //打印错误日志
                TaskServiceSerialErrMsgShow(errorCode);
                //没数据,设置为0
                *result = 0;
                //返回错误代码
                return TESTER_WINDOW_ERR_LIGHT_SENSOR_NO_DATA;
            }  
        /*6. 关闭侧窗*/  
            SystemReadActionParamWithIndex(MODULEMEASUREROOMANDLIQUIDBPARAM,WINDOWCLOSEPOS,&yogi);
            returnZeroCmd.stepMotorIndex = STEP_MOTOR_MEASUREROOM_WINDOW;
            returnZeroCmd.correctionPosition = yogi;//actionParam->moduleMeasureRoomAndLiquidBParam.windowClosePos;
            returnZeroCmd.curveSelect = SM_CURVE_0;
            returnZeroCmd.speedRatio = 100;
            returnZeroCmd.speedMode = SPEED_MODE_UP_START;
            returnZeroCmd.utilStop1Enable = DISABLE;
            returnZeroCmd.utilStop2Enable = DISABLE;
            returnZeroCmd.timeOutMs = 60000;

            //电机走坐标
            errorCode = Can2SubSM_ReturnZeroWhileReturn(&returnZeroCmd);
            
            if(errorCode != LH_ERR_NONE)
            {
                //打印错误日志
                TaskServiceSerialErrMsgShow(errorCode);
                //返回错误代码
                return errorCode;
            }  
        /*如果结果为0XFFFFFFFF,也当成无数据返回 */
            if(*result == 0XFFFFFFFF)
            {
                errorCode = LH_ERR_ACTION_PARAM_ERR;
                //打印错误日志
                TaskServiceSerialErrMsgShow(errorCode);
                //返回错误代码
                return TESTER_WINDOW_ERR_LIGHT_SENSOR_NO_DATA;
            }
    }

    return errorCode;

}


