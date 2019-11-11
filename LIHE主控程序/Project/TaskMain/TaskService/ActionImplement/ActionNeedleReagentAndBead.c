#include "ActionNeedleReagentAndBead.h"
#include "TSC_TestMode.h"
#include "math.h"
//当前磁珠试剂针的水平位置
//static TSC_NEEDLE_REAGENT_BEAD_POS currentNeedleReagentBeadPos;

//static int32_t pumpdrawul;//磁珠试剂针吸取的量
#define NEEDLE_REAGENT_LIQUID_OFFSET    200

//磁珠试剂针初始化
LH_ERR ActionNeedleReagentBead_Init(TSC_NEEDLE_REAGENT_BEAD_POS* pos)
{
    LH_ERR errorCode;
    int32_t yogi;
    //电机复位结构体
    SM_RESET_CMD resetCommand;
    //结构体初始化
    Can2SubSM_ResetCmdDataStructInit(&resetCommand);

    //复位参数设置
    SystemReadActionParamWithIndex(MODULENEEDLEREAGENTBEADPARAM,BEADUPDOWNRESETCORRECTION,&yogi);
    resetCommand.stepMotorIndex = STEP_MOTOR_NEEDLE_REAGENT_BEAD_UPDOWN;
    resetCommand.correctionPosition = yogi;//actionParam->moduleNeedleReagentBeadParam.upDownResetCorrection;
    resetCommand.timeOutMs = 60000;
    //升降复位
    errorCode = Can2SubSM_ResetWhileReturn(&resetCommand);

    if(errorCode != LH_ERR_NONE)
	{
        //打印错误日志
        TaskServiceSerialErrMsgShow(errorCode);
        //返回错误代码
		return errorCode;
	}

    //复位参数设置
    SystemReadActionParamWithIndex(MODULENEEDLEREAGENTBEADPARAM,BEADROTATERESETCORRECTION,&yogi);
    resetCommand.stepMotorIndex = STEP_MOTOR_NEEDLE_REAGENT_BEAD_ROTATE;
    resetCommand.correctionPosition = yogi;//actionParam->moduleNeedleReagentBeadParam.rotateResetCorrection;
    resetCommand.timeOutMs = 60000;
    //旋转复位
    errorCode = Can2SubSM_ResetWhileReturn(&resetCommand);

    if(errorCode != LH_ERR_NONE)
	{
        //打印错误日志
        TaskServiceSerialErrMsgShow(errorCode);
        //返回错误代码
		return errorCode;
	}

    //复位参数设置
    resetCommand.stepMotorIndex = STEP_MOTOR_PUMP_NEEDLE_REAGENT_BEAD;
    resetCommand.correctionPosition = 0;
    resetCommand.timeOutMs = 60000;
    //泵复位
    errorCode = Can2SubSM_ResetWhileReturn(&resetCommand);

    if(errorCode != LH_ERR_NONE)
	{
        //打印错误日志
        TaskServiceSerialErrMsgShow(errorCode);
        //返回错误代码
		return errorCode;
	}
    //设置磁珠试剂针初始化位置
    TaskUtilServiceSetNeedleReagentRotatePos(TSC_NEEDLE_REAGENT_BEAD_POS_CLEAN);

    *pos = TaskUtilServiceGetNeedleReagentRotatePos();
    
    return errorCode;
}

//旋转到指定位置
LH_ERR ActionNeedleReagentBead_RotateSpecialPos(TSC_NEEDLE_REAGENT_BEAD_POS targetPos,TSC_NEEDLE_REAGENT_BEAD_POS* rotatePos)
{
    int32_t yogi;
    int32_t pos = 0;
    LH_ERR errorCode;
    switch(targetPos)
    {
        case TSC_NEEDLE_REAGENT_BEAD_POS_CLEAN:
            pos = actionParam->moduleNeedleReagentBeadParam.rotateCleanPos;
            break;
        case TSC_NEEDLE_REAGENT_BEAD_POS_R1:
            pos = actionParam->moduleNeedleReagentBeadParam.rotateReagent1AbsorbPos;
            break;
        case TSC_NEEDLE_REAGENT_BEAD_POS_R2:
            pos = actionParam->moduleNeedleReagentBeadParam.rotateReagent2AbsorbPos;
            break;
        case TSC_NEEDLE_REAGENT_BEAD_POS_BEAD:
            pos = actionParam->moduleNeedleReagentBeadParam.rotateBeadAbsorbPos;
            break;
        case TSC_NEEDLE_REAGENT_BEAD_POS_REACTION:
            pos = actionParam->moduleNeedleReagentBeadParam.rotateReactionInjectPos;
            break;
        default:
            errorCode = LH_ERR_ACTION_PARAM;
            TaskServiceSerialErrMsgShow(errorCode);
            return errorCode;
    }


    SM_RETURN_ZERO_CMD returnZeroCmd;
    Can2SubSM_ReturnZeroDataStructInit(&returnZeroCmd);

    //升降回零并修正
    SystemReadActionParamWithIndex(MODULENEEDLEREAGENTBEADPARAM,BEADUPDOWNRESETCORRECTION,&yogi);
    returnZeroCmd.stepMotorIndex = STEP_MOTOR_NEEDLE_REAGENT_BEAD_UPDOWN;
    returnZeroCmd.correctionPosition = yogi;//actionParam->moduleNeedleReagentBeadParam.upDownResetCorrection;
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

    SM_RUN_COORDINATE_CMD runCoordinateCmd;
    Can2SubSM_RunCoordinateDataStructInit(&runCoordinateCmd);

    //旋转
    runCoordinateCmd.stepMotorIndex = STEP_MOTOR_NEEDLE_REAGENT_BEAD_ROTATE;
    runCoordinateCmd.targetCoordinate = pos;
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
    TaskUtilServiceSetNeedleReagentRotatePos(targetPos);

    *rotatePos = TaskUtilServiceGetNeedleReagentRotatePos();

    return errorCode;
}


//获取升降坐标
static LH_ERR UtilNeedleReagentBeadGetDownPos(int32_t* pos)
{
    TSC_NEEDLE_REAGENT_BEAD_POS currentNeedleReagentBeadPos;

    currentNeedleReagentBeadPos = TaskUtilServiceGetNeedleReagentRotatePos();

    if(currentNeedleReagentBeadPos == TSC_NEEDLE_REAGENT_BEAD_POS_CLEAN)
    {
        //清洗位
        *pos = actionParam->moduleNeedleReagentBeadParam.upDownCleanPos;
        return LH_ERR_NONE;
    }
    else if(currentNeedleReagentBeadPos == TSC_NEEDLE_REAGENT_BEAD_POS_R1)
    {
        //R1位
        *pos = actionParam->moduleNeedleReagentBeadParam.upDownReagent1AbsorbPos;
        return LH_ERR_NONE;
    }
    else if(currentNeedleReagentBeadPos == TSC_NEEDLE_REAGENT_BEAD_POS_R2)
    {
        //R2位
        *pos = actionParam->moduleNeedleReagentBeadParam.upDownReagent2AbsorbPos;
        return LH_ERR_NONE;
    }
    else if(currentNeedleReagentBeadPos == TSC_NEEDLE_REAGENT_BEAD_POS_BEAD)
    {
        //磁珠位
        *pos = actionParam->moduleNeedleReagentBeadParam.upDownBeadAbsorbPos;
        return LH_ERR_NONE;
    }
    else if(currentNeedleReagentBeadPos == TSC_NEEDLE_REAGENT_BEAD_POS_REACTION)
    {
        //反应盘位
        *pos = actionParam->moduleNeedleReagentBeadParam.upDownReactionInjectPos;
        return LH_ERR_NONE;
    }
    else
    {
        *pos = 0;
        return LH_ERR_ACTION_PARAM;
    }
}

//磁珠试剂针吸液----------------老化模式
LH_ERR ActionNeedleReagentBead_AbsorbAnyLiquid_Debug(uint16_t liquidUl,TSC_NEEDLE_REAGENT_BEAD_POS* rotatePos)
{
    int32_t yogi;
    int32_t posUtil = 0;
    LH_ERR errorCode;
    //获取在当前旋转位置的升降坐标
    errorCode = UtilNeedleReagentBeadGetDownPos(&posUtil);
    if(errorCode != LH_ERR_NONE)
	{
        TaskServiceSerialErrMsgShow(errorCode);
		return errorCode;
	}

    SM_RUN_COORDINATE_CMD runCoordinateCmd;
    Can2SubSM_RunCoordinateDataStructInit(&runCoordinateCmd);

    //升降下去
    runCoordinateCmd.stepMotorIndex = STEP_MOTOR_NEEDLE_REAGENT_BEAD_UPDOWN;
    runCoordinateCmd.targetCoordinate = posUtil;
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
    //泵吸液

    //升降抬起来
    SM_RETURN_ZERO_CMD returnZeroCmd;
    Can2SubSM_ReturnZeroDataStructInit(&returnZeroCmd);

    //升降回零并修正
    SystemReadActionParamWithIndex(MODULENEEDLEREAGENTBEADPARAM,BEADUPDOWNRESETCORRECTION,&yogi);
    returnZeroCmd.stepMotorIndex = STEP_MOTOR_NEEDLE_REAGENT_BEAD_UPDOWN;
    returnZeroCmd.correctionPosition = yogi;//actionParam->moduleNeedleReagentBeadParam.upDownResetCorrection;
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
    //泵吸空气

    *rotatePos = TaskUtilServiceGetNeedleReagentRotatePos();
    return errorCode;
}
//磁珠试剂针吸液----------------正常模式
LH_ERR ActionNeedleReagentBead_AbsorbAnyLiquid_Normal(uint16_t liquidUl,TSC_NEEDLE_REAGENT_BEAD_POS* rotatePos,ONE_OF_CUPSTATUS *oneofcups)
{
    int32_t yogi;
    int32_t posUtil = 0;
    int32_t liquidpos;
    LH_ERR errorCode;
    SM_RUN_STEP_CMD runStepCommand;
    SM_RUN_COORDINATE_CMD runCoordinateCmd;
    REAGENT_CUP_STATUS *cupstatus;
    /*0. 获取开始探液坐标*/
        cupstatus = TaskUtilServiceGetDishReagentCupState(oneofcups);//获取对应试剂杯的状态
        if(cupstatus->status == DISH_REAGENT_CUP_OK)//只有试剂杯状态正常时才会根据偏移计算探液开始坐标
        {
            posUtil = cupstatus->steps - NEEDLE_REAGENT_LIQUID_OFFSET;
        }
        else 
        {
            posUtil = actionParam->moduleNeedleReagentBeadParam.upDownReagentStartPos;//默认探液坐标
        }

    /*1. 柱塞泵M35复位*/
        Can2SubSM_RunCoordinateDataStructInit(&runCoordinateCmd); 
        runCoordinateCmd.stepMotorIndex = STEP_MOTOR_PUMP_NEEDLE_REAGENT_BEAD;
        runCoordinateCmd.targetCoordinate = 0;
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
    /*2. 柱塞泵m35低位运行吸取空气*/
        Can2SubSM_RunStepsCmdDataStructInit(&runStepCommand);

        runStepCommand.stepMotorIndex = STEP_MOTOR_PUMP_NEEDLE_REAGENT_BEAD;
        runStepCommand.steps = actionParam->moduleNeedleReagentBeadParam.pumpAbsorbWithAirUl;
        runStepCommand.curveSelect = SM_CURVE_0;
        runStepCommand.speedRatio = 100;
        runStepCommand.speedMode = SPEED_MODE_UP_START;
        runStepCommand.utilStop1Enable = DISABLE;
        runStepCommand.utilStop2Enable = DISABLE;
        runStepCommand.timeOutMs = 60000;
        //电机执行步数
        errorCode = Can2SubSM_RunSpecialStepsWhileReturn(&runStepCommand);
        if(errorCode != LH_ERR_NONE)
        {
            //打印错误日志
            TaskServiceSerialErrMsgShow(errorCode);
            //返回错误代码
            return errorCode;
        }
    /*3. 关闭探液功能*/
        errorCode = Can2SubNeedleReagentLiquidDetectDisableFunc();
        if(errorCode != LH_ERR_NONE)
        {
            //打印错误日志
            TaskServiceSerialErrMsgShow(errorCode);

            return LH_ERR_LIQUID_DETECT_COMM_FAIL;

        }
    /*4. 试剂针快速下降到试剂管中*/
        Can2SubSM_RunCoordinateDataStructInit(&runCoordinateCmd);

        //升降下去
        runCoordinateCmd.stepMotorIndex = STEP_MOTOR_NEEDLE_REAGENT_BEAD_UPDOWN;
        runCoordinateCmd.targetCoordinate = posUtil;
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
    /*6. 开启探液功能*/
        errorCode = Can2SubNeedleReagentLiquidDetectEnableFunc();
        if(errorCode != LH_ERR_NONE)
        {
            //打印错误日志
            TaskServiceSerialErrMsgShow(errorCode);
            return LH_ERR_LIQUID_DETECT_COMM_FAIL;
        }
    /*7. 试剂针慢速下降到试剂管极限位置*/
        UtilNeedleReagentBeadGetDownPos(&posUtil);//获取极限坐标

        runCoordinateCmd.stepMotorIndex = STEP_MOTOR_NEEDLE_REAGENT_BEAD_UPDOWN;
        runCoordinateCmd.targetCoordinate = posUtil;
        runCoordinateCmd.curveSelect = SM_CURVE_1;
        runCoordinateCmd.speedRatio = 100;
        runCoordinateCmd.speedMode = SPEED_MODE_UP_START;
        runCoordinateCmd.utilStop1Enable = ENABLE;
        runCoordinateCmd.utilStop2Enable = DISABLE;
        runCoordinateCmd.timeOutMs = 60000;

        //电机走坐标
        errorCode = Can2SubSM_RunToCoordinateWhileReturn(&runCoordinateCmd);

        if((errorCode != LH_ERR_NONE)&&(errorCode != LH_ERR_SM_UTIL_STOP1))
        {
            //打印错误日志
            TaskServiceSerialErrMsgShow(errorCode);
            //返回错误代码
            return errorCode;
        }
        if(errorCode == LH_ERR_SM_UTIL_STOP1)
        {
            Can2SubSM_ReadPositionWhileReturn(STEP_MOTOR_NEEDLE_REAGENT_BEAD_UPDOWN,&liquidpos);
            cupstatus->status = DISH_REAGENT_CUP_OK;
            cupstatus->steps = liquidpos;
            cupstatus->utilization = 100-((liquidpos*100)/posUtil);
            TaskUtilServiceSetDishReagentCupState(oneofcups,cupstatus);
        }

    /*8. 柱塞泵m35运行抽取试剂*/
        Can2SubSM_RunStepsCmdDataStructInit(&runStepCommand);

        runStepCommand.stepMotorIndex = STEP_MOTOR_PUMP_NEEDLE_REAGENT_BEAD;
        runStepCommand.steps = actionParam->moduleNeedleReagentBeadParam.pumpOneUlStep*liquidUl;
        runStepCommand.curveSelect = SM_CURVE_0;
        runStepCommand.speedRatio = 100;
        runStepCommand.speedMode = SPEED_MODE_UP_START;
        runStepCommand.utilStop1Enable = DISABLE;
        runStepCommand.utilStop2Enable = DISABLE;
        runStepCommand.timeOutMs = 60000;
        TaskUtilServiceSetNeedleReagentPumpUesdUl(liquidUl);//保存试剂针吸取的量
        //电机执行步数
        errorCode = Can2SubSM_RunSpecialStepsWhileReturn(&runStepCommand);
        if(errorCode != LH_ERR_NONE)
        {
            //打印错误日志
            TaskServiceSerialErrMsgShow(errorCode);
            //返回错误代码
            return errorCode;
        }
    /*9. 试剂针上升*/
        //升降抬起来
        SM_RETURN_ZERO_CMD returnZeroCmd;
        Can2SubSM_ReturnZeroDataStructInit(&returnZeroCmd);

        //升降回零并修正
        SystemReadActionParamWithIndex(MODULENEEDLEREAGENTBEADPARAM,BEADUPDOWNRESETCORRECTION,&yogi);
        returnZeroCmd.stepMotorIndex = STEP_MOTOR_NEEDLE_REAGENT_BEAD_UPDOWN;
        returnZeroCmd.correctionPosition = yogi;//actionParam->moduleNeedleReagentBeadParam.upDownResetCorrection;
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
    /*10. 柱塞泵m35运行吸取空气*/
        Can2SubSM_RunStepsCmdDataStructInit(&runStepCommand);
        SystemReadActionParamWithIndex(MODULENEEDLEREAGENTBEADPARAM,BEADPUMPABSORBWITHAIRU2,&yogi);
        runStepCommand.stepMotorIndex = STEP_MOTOR_PUMP_NEEDLE_REAGENT_BEAD;
        runStepCommand.steps = yogi;//actionParam->moduleNeedleReagentBeadParam.pumpAbsorbWithAirU2;
        runStepCommand.curveSelect = SM_CURVE_0;
        runStepCommand.speedRatio = 100;
        runStepCommand.speedMode = SPEED_MODE_UP_START;
        runStepCommand.utilStop1Enable = DISABLE;
        runStepCommand.utilStop2Enable = DISABLE;
        runStepCommand.timeOutMs = 60000;
        //电机执行步数
        errorCode = Can2SubSM_RunSpecialStepsWhileReturn(&runStepCommand);
        if(errorCode != LH_ERR_NONE)
        {
            //打印错误日志
            TaskServiceSerialErrMsgShow(errorCode);
            //返回错误代码
            return errorCode;
        }
    *rotatePos = TaskUtilServiceGetNeedleReagentRotatePos();
    return errorCode;
}
//磁珠试剂针吸液
LH_ERR ActionNeedleReagentBead_AbsorbAnyLiquid(uint16_t liquidUl,TSC_NEEDLE_REAGENT_BEAD_POS* rotatePos,ONE_OF_CUPSTATUS *oneofcups)
{
    LH_ERR errorCode;

    TSC_TEST_MODE_SELECTION mode;
    mode = TSC_Read_TestMode();

    if(mode == TSC_TEST_MODE_NORMAL)
    {
        errorCode = ActionNeedleReagentBead_AbsorbAnyLiquid_Normal(liquidUl,rotatePos,oneofcups);//正常测试
    }
    else 
    {
        errorCode = ActionNeedleReagentBead_AbsorbAnyLiquid_Debug(liquidUl,rotatePos);//老化测试
    }

    return errorCode;
}
//磁珠试剂针排液----------------正常模式
LH_ERR ActionNeedleReagentBead_InjectAnyLiquid_Normal(uint16_t liquidUl,TSC_NEEDLE_REAGENT_BEAD_POS* rotatePos)
{
    int32_t posUtil = 0;
    int32_t yogi;
    LH_ERR errorCode;
    SM_RUN_STEP_CMD runStepCommand;
    SM_RUN_COORDINATE_CMD runCoordinateCmd;
    /*1. 试剂针下降到试管内*/
        errorCode = UtilNeedleReagentBeadGetDownPos(&posUtil);
        if(errorCode != LH_ERR_NONE)
        {
            //打印错误日志
            TaskServiceSerialErrMsgShow(errorCode);
            //返回错误代码
            return errorCode;
        }


        Can2SubSM_RunCoordinateDataStructInit(&runCoordinateCmd);

        //升降下去
        runCoordinateCmd.stepMotorIndex = STEP_MOTOR_NEEDLE_REAGENT_BEAD_UPDOWN;
        runCoordinateCmd.targetCoordinate = posUtil;
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
    /*2. 柱塞泵m35排出指定的试剂*/
        Can2SubSM_RunStepsCmdDataStructInit(&runStepCommand);

        runStepCommand.stepMotorIndex = STEP_MOTOR_PUMP_NEEDLE_REAGENT_BEAD;
        //试剂针精度测试
        if(liquidUl == 0)
        {
            yogi = (actionParam->moduleNeedleReagentBeadParam.pumpAbsorbWithAirUl
                    +actionParam->moduleNeedleReagentBeadParam.pumpAbsorbWithAirU2
                    +TaskUtilServiceGetNeedleReagentPumpUesdUl()
                    );
            yogi = yogi - actionParam->moduleNeedleReagentBeadParam.pumpInjectRetainAir;
            runStepCommand.steps = yogi*actionParam->moduleNeedleReagentBeadParam.pumpOneUlStep*(-1);
        }
        //试剂针普通测试根据输入的参数排量
        else
        {
            runStepCommand.steps = actionParam->moduleNeedleReagentBeadParam.pumpOneUlStep*liquidUl*(-1);
        }
        
        runStepCommand.curveSelect = SM_CURVE_0;
        runStepCommand.speedRatio = 100;
        runStepCommand.speedMode = SPEED_MODE_UP_START;
        runStepCommand.utilStop1Enable = DISABLE;
        runStepCommand.utilStop2Enable = DISABLE;
        runStepCommand.timeOutMs = 60000;
        //电机执行步数
        errorCode = Can2SubSM_RunSpecialStepsWhileReturn(&runStepCommand);
        if(errorCode != LH_ERR_NONE)
        {
            //打印错误日志
            TaskServiceSerialErrMsgShow(errorCode);
            //返回错误代码
            return errorCode;
        }
    /*3. 试剂针上升回到原点*/
        //抬起来
        //升降抬起来
        SM_RETURN_ZERO_CMD returnZeroCmd;
        Can2SubSM_ReturnZeroDataStructInit(&returnZeroCmd);

        //升降回零并修正
        SystemReadActionParamWithIndex(MODULENEEDLEREAGENTBEADPARAM,BEADUPDOWNRESETCORRECTION,&yogi);
        returnZeroCmd.stepMotorIndex = STEP_MOTOR_NEEDLE_REAGENT_BEAD_UPDOWN;
        returnZeroCmd.correctionPosition = yogi;//actionParam->moduleNeedleReagentBeadParam.upDownResetCorrection;
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
    

    *rotatePos = TaskUtilServiceGetNeedleReagentRotatePos();
    return errorCode;
}
//磁珠试剂针排液---------------老化模式
LH_ERR ActionNeedleReagentBead_InjectAnyLiquid_Debug(uint16_t liquidUl,TSC_NEEDLE_REAGENT_BEAD_POS* rotatePos)
{
    int32_t yogi;
    int32_t posUtil = 0;
    LH_ERR errorCode;
    errorCode = UtilNeedleReagentBeadGetDownPos(&posUtil);
    if(errorCode != LH_ERR_NONE)
	{
        //打印错误日志
        TaskServiceSerialErrMsgShow(errorCode);
        //返回错误代码
		return errorCode;
	}

    SM_RUN_COORDINATE_CMD runCoordinateCmd;
    Can2SubSM_RunCoordinateDataStructInit(&runCoordinateCmd);

    //升降下去
    runCoordinateCmd.stepMotorIndex = STEP_MOTOR_NEEDLE_REAGENT_BEAD_UPDOWN;
    runCoordinateCmd.targetCoordinate = posUtil;
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

    //排液

    //抬起来
    //升降抬起来
    SM_RETURN_ZERO_CMD returnZeroCmd;
    Can2SubSM_ReturnZeroDataStructInit(&returnZeroCmd);

    //升降回零并修正
    SystemReadActionParamWithIndex(MODULENEEDLEREAGENTBEADPARAM,BEADUPDOWNRESETCORRECTION,&yogi);
    returnZeroCmd.stepMotorIndex = STEP_MOTOR_NEEDLE_REAGENT_BEAD_UPDOWN;
    returnZeroCmd.correctionPosition = yogi;//actionParam->moduleNeedleReagentBeadParam.upDownResetCorrection;
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

    *rotatePos = TaskUtilServiceGetNeedleReagentRotatePos();
    return errorCode;
}
//磁珠试剂针排液
LH_ERR ActionNeedleReagentBead_InjectAnyLiquid(uint16_t liquidUl,TSC_NEEDLE_REAGENT_BEAD_POS* rotatePos)
{
    LH_ERR errorCode;

    TSC_TEST_MODE_SELECTION mode;
    mode = TSC_Read_TestMode();

    if(mode == TSC_TEST_MODE_NORMAL)
    {
        errorCode = ActionNeedleReagentBead_InjectAnyLiquid_Normal(liquidUl,rotatePos);//正常测试
    }
    else 
    {
        errorCode = ActionNeedleReagentBead_InjectAnyLiquid_Debug(liquidUl,rotatePos);//老化测试
    }

    return errorCode;
}

//磁珠试剂针清洗-------老化模式
LH_ERR ActionNeedleReagentBead_Clean_Debug(TSC_NEEDLE_REAGENT_BEAD_CLEAN_OPT opt,TSC_NEEDLE_REAGENT_BEAD_POS* rotatePos)
{
    LH_ERR errorCode;
    int32_t yogi;
    //升降抬起来
    SM_RETURN_ZERO_CMD returnZeroCmd;
    Can2SubSM_ReturnZeroDataStructInit(&returnZeroCmd);

    //升降回零并修正
    SystemReadActionParamWithIndex(MODULENEEDLEREAGENTBEADPARAM,BEADUPDOWNRESETCORRECTION,&yogi);
    returnZeroCmd.stepMotorIndex = STEP_MOTOR_NEEDLE_REAGENT_BEAD_UPDOWN;
    returnZeroCmd.correctionPosition = yogi;//actionParam->moduleNeedleReagentBeadParam.upDownResetCorrection;
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

    SM_RUN_COORDINATE_CMD runCoordinateCmd;
    Can2SubSM_RunCoordinateDataStructInit(&runCoordinateCmd);

    //旋转运行到清洗位
    SystemReadActionParamWithIndex(MODULENEEDLEREAGENTBEADPARAM,BEADROTATECLEANPOS,&yogi);
    runCoordinateCmd.stepMotorIndex = STEP_MOTOR_NEEDLE_REAGENT_BEAD_ROTATE;
    runCoordinateCmd.targetCoordinate = yogi;//actionParam->moduleNeedleReagentBeadParam.rotateCleanPos;
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
    TaskUtilServiceSetNeedleReagentRotatePos(TSC_NEEDLE_REAGENT_BEAD_POS_CLEAN);

    //升降运行到清洗位
    SystemReadActionParamWithIndex(MODULENEEDLEREAGENTBEADPARAM,BEADUPDOWNCLEANPOS,&yogi);
    runCoordinateCmd.stepMotorIndex = STEP_MOTOR_NEEDLE_REAGENT_BEAD_UPDOWN;
    runCoordinateCmd.targetCoordinate = yogi;//actionParam->moduleNeedleReagentBeadParam.upDownCleanPos;
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

    //延时等待清洗
    CoreDelayMs(actionParam->moduleNeedleReagentBeadParam.timeNeedleCleanMsInt);

    //升降回零并修正
    SystemReadActionParamWithIndex(MODULENEEDLEREAGENTBEADPARAM,BEADUPDOWNRESETCORRECTION,&yogi);
    returnZeroCmd.stepMotorIndex = STEP_MOTOR_NEEDLE_REAGENT_BEAD_UPDOWN;
    returnZeroCmd.correctionPosition = yogi;//actionParam->moduleNeedleReagentBeadParam.upDownResetCorrection;
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

    *rotatePos = TaskUtilServiceGetNeedleReagentRotatePos();

    return errorCode;
}
//磁珠试剂针清洗------正常模式
LH_ERR ActionNeedleReagentBead_Clean_Normal(TSC_NEEDLE_REAGENT_BEAD_CLEAN_OPT opt,TSC_NEEDLE_REAGENT_BEAD_POS* rotatePos)
{
    LH_ERR errorCode;
    int32_t posDown;
    int32_t yogi;
    /*2. 判断试剂针在清洗位上方*/
        if(TaskUtilServiceGetNeedleReagentRotatePos() != TSC_NEEDLE_REAGENT_BEAD_POS_CLEAN)
        {
            return LH_ERR_ACTION_PARAM_ERR;
        }
        else
        {
            errorCode = UtilNeedleReagentBeadGetDownPos(&posDown);
            if(errorCode != LH_ERR_NONE)
            {
                TaskServiceSerialErrMsgShow(errorCode);
                return errorCode;
            }
        }
    /*3. 试剂针下降*/
        SM_RUN_COORDINATE_CMD runCoordinateCmd;
        Can2SubSM_RunCoordinateDataStructInit(&runCoordinateCmd);
        //升降运行到清洗位
        runCoordinateCmd.stepMotorIndex = STEP_MOTOR_NEEDLE_REAGENT_BEAD_UPDOWN;
        runCoordinateCmd.targetCoordinate = posDown;
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
    /*4. 打开抽水阀门*/
        //  SystemInputRead();传感器没有安装也不清除连接哪个端口
        //  默认满
        if(1)
        {
            //打开v307阀门
            // errorCode = SystemOutputWrite(DRAINAGE_VALVE_V307,Bit_RESET);
            // if(errorCode != LH_ERR_NONE)
            // {
            //     //打印错误日志
            //     TaskServiceSerialErrMsgShow(errorCode);
            //     //返回错误代码
            //     return errorCode;
            // }
            // //打开隔膜泵dp1
            // errorCode = SystemOutputWrite(WASTE_LIQUID_PUMP,Bit_RESET);
            // if(errorCode != LH_ERR_NONE)
            // {
            //     //打印错误日志
            //     TaskServiceSerialErrMsgShow(errorCode);
            //     //返回错误代码
            //     return errorCode;
            // }
        }
    /*5. 柱塞泵m35运行至低位     条件:内壁或内外同时清洗*/
        if((opt==TSC_NEEDLE_REAGENT_BEAD_CLEAN_OPT_IN)||(opt==TSC_NEEDLE_REAGENT_BEAD_CLEAN_OPT_IN_OUT))
        {
            SM_RUN_COORDINATE_CMD runCoordinateCmd;
            Can2SubSM_RunCoordinateDataStructInit(&runCoordinateCmd); 
            runCoordinateCmd.stepMotorIndex = STEP_MOTOR_PUMP_NEEDLE_REAGENT_BEAD;
            runCoordinateCmd.targetCoordinate = 4500;
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
        }
    /*6. 打开阀v216     条件：外壁或内外同时清洗*/
        if((opt==TSC_NEEDLE_REAGENT_BEAD_CLEAN_OPT_OUT)||(opt==TSC_NEEDLE_REAGENT_BEAD_CLEAN_OPT_IN_OUT))
        {
            errorCode = SystemOutputWrite(DRAINAGE_VALVE_V216,Bit_RESET);
            if(errorCode != LH_ERR_NONE)
            {
                //打印错误日志
                TaskServiceSerialErrMsgShow(errorCode);
                //返回错误代码
                return errorCode;
            }   
        }
    /*7. 打开阀v219     条件:内壁或内外同时清洗*/
        if((opt==TSC_NEEDLE_REAGENT_BEAD_CLEAN_OPT_IN)||(opt==TSC_NEEDLE_REAGENT_BEAD_CLEAN_OPT_IN_OUT))
        {
            errorCode = SystemOutputWrite(DRAINAGE_VALVE_V219,Bit_RESET);
            if(errorCode != LH_ERR_NONE)
            {
                //打印错误日志
                TaskServiceSerialErrMsgShow(errorCode);
                //返回错误代码
                return errorCode;
            }   
        }
    /*8. 延时20ms*/
        CoreDelayMs(20);
    /*8. 打开隔膜泵dp305 条件：外壁或内外同时清洗*/
        if((opt==TSC_NEEDLE_REAGENT_BEAD_CLEAN_OPT_OUT)||(opt==TSC_NEEDLE_REAGENT_BEAD_CLEAN_OPT_IN_OUT))
        {
            errorCode = SystemOutputWrite(DIAPHRAGM_PUMP_DP305,Bit_RESET);
            if(errorCode != LH_ERR_NONE)
            {
                //打印错误日志
                TaskServiceSerialErrMsgShow(errorCode);
                //返回错误代码
                return errorCode;
            }   
        }
    /*9. 打开隔膜泵dp306 条件：内壁或内外同时清洗*/
        if((opt==TSC_NEEDLE_REAGENT_BEAD_CLEAN_OPT_IN)||(opt==TSC_NEEDLE_REAGENT_BEAD_CLEAN_OPT_IN_OUT))
        {
            errorCode = SystemOutputWrite(DIAPHRAGM_PUMP_DP306,Bit_RESET);
            if(errorCode != LH_ERR_NONE)
            {
                //打印错误日志
                TaskServiceSerialErrMsgShow(errorCode);
                //返回错误代码
                return errorCode;
            }  
        }
    /*10. 清洗时间*/
        CoreDelayMs(actionParam->moduleNeedleReagentBeadParam.timeNeedleCleanMsInt);
    /*11. 关闭隔膜泵dp306  条件：内壁清洗或内外壁同时*/
        if((opt==TSC_NEEDLE_REAGENT_BEAD_CLEAN_OPT_IN)||(opt==TSC_NEEDLE_REAGENT_BEAD_CLEAN_OPT_IN_OUT))
        {
            errorCode = SystemOutputWrite(DIAPHRAGM_PUMP_DP306,Bit_SET);
            if(errorCode != LH_ERR_NONE)
            {
                //打印错误日志
                TaskServiceSerialErrMsgShow(errorCode);
                //返回错误代码
                return errorCode;
            }  
        }
    /*12. 延时等待*/
        CoreDelayMs(100);
    /*13. 关闭阀v219       条件：内壁或内外壁同时*/
        if((opt==TSC_NEEDLE_REAGENT_BEAD_CLEAN_OPT_IN)||(opt==TSC_NEEDLE_REAGENT_BEAD_CLEAN_OPT_IN_OUT))
        {
            errorCode = SystemOutputWrite(DRAINAGE_VALVE_V219,Bit_SET);
            if(errorCode != LH_ERR_NONE)
            {
                //打印错误日志
                TaskServiceSerialErrMsgShow(errorCode);
                //返回错误代码
                return errorCode;
            }   
        }
    /*14. 柱塞泵m35归零    条件：内壁或内外壁同时清洗*/
        if((opt==TSC_NEEDLE_REAGENT_BEAD_CLEAN_OPT_IN)||(opt==TSC_NEEDLE_REAGENT_BEAD_CLEAN_OPT_IN_OUT))
        {
            SM_RUN_COORDINATE_CMD runCoordinateCmd;
            Can2SubSM_RunCoordinateDataStructInit(&runCoordinateCmd); 
            runCoordinateCmd.stepMotorIndex = STEP_MOTOR_PUMP_NEEDLE_REAGENT_BEAD;
            runCoordinateCmd.targetCoordinate = 0;
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
        }
    /*15. 关闭隔膜泵dp305  条件：外壁清洗或内外壁同时*/
        if((opt==TSC_NEEDLE_REAGENT_BEAD_CLEAN_OPT_OUT)||(opt==TSC_NEEDLE_REAGENT_BEAD_CLEAN_OPT_IN_OUT))
        {
            errorCode = SystemOutputWrite(DIAPHRAGM_PUMP_DP305,Bit_SET);
            if(errorCode != LH_ERR_NONE)
            {
                //打印错误日志
                TaskServiceSerialErrMsgShow(errorCode);
                //返回错误代码
                return errorCode;
            }   
        }
    /*16. 延时等待*/
        CoreDelayMs(100);
    
    /*17. 关闭阀v216       条件：外壁或内外壁同时*/
        if((opt==TSC_NEEDLE_REAGENT_BEAD_CLEAN_OPT_OUT)||(opt==TSC_NEEDLE_REAGENT_BEAD_CLEAN_OPT_IN_OUT))
        {
            errorCode = SystemOutputWrite(DRAINAGE_VALVE_V216,Bit_SET);
            if(errorCode != LH_ERR_NONE)
            {
                //打印错误日志
                TaskServiceSerialErrMsgShow(errorCode);
                //返回错误代码
                return errorCode;
            }   
        }
    
    /*18. 关闭隔膜泵dp1*/
        // errorCode = SystemOutputWrite(WASTE_LIQUID_PUMP,Bit_SET);
        // if(errorCode != LH_ERR_NONE)
        // {
        //     //打印错误日志
        //     TaskServiceSerialErrMsgShow(errorCode);
        //     //返回错误代码
        //     return errorCode;
        // }
    /*19. 关闭阀v307*/
        // errorCode = SystemOutputWrite(DRAINAGE_VALVE_V307,Bit_SET);
        // if(errorCode != LH_ERR_NONE)
        // {
        //     //打印错误日志
        //     TaskServiceSerialErrMsgShow(errorCode);
        //     //返回错误代码
        //     return errorCode;
        // }
    /*20. 试剂针上升*/
        SM_RETURN_ZERO_CMD returnZeroCmd;
        Can2SubSM_ReturnZeroDataStructInit(&returnZeroCmd);
        //升降回零并修正
        SystemReadActionParamWithIndex(MODULENEEDLEREAGENTBEADPARAM,BEADUPDOWNRESETCORRECTION,&yogi);
        returnZeroCmd.stepMotorIndex = STEP_MOTOR_NEEDLE_REAGENT_BEAD_UPDOWN;
        returnZeroCmd.correctionPosition = yogi;//actionParam->moduleNeedleReagentBeadParam.upDownResetCorrection;
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

    *rotatePos = TaskUtilServiceGetNeedleReagentRotatePos();

    return errorCode;
}


//磁珠试剂针清洗
LH_ERR ActionNeedleReagentBead_Clean(TSC_NEEDLE_REAGENT_BEAD_CLEAN_OPT opt,TSC_NEEDLE_REAGENT_BEAD_POS* rotatePos)
{
    LH_ERR errorCode;

    TSC_TEST_MODE_SELECTION mode;
    mode = TSC_Read_TestMode();

    if(mode == TSC_TEST_MODE_NORMAL)
    {
        errorCode = ActionNeedleReagentBead_Clean_Normal(opt,rotatePos);//正常测试
    }
    else 
    {
        errorCode = ActionNeedleReagentBead_Clean_Debug(opt,rotatePos);//老化测试
    }

    return errorCode;
}


//磁珠试剂针灌注-----------正常测试
LH_ERR ActionNeedleReagentBead_Prime_Normal(TSC_NEEDLE_REAGENT_PRIME_OPT opt,TSC_NEEDLE_REAGENT_BEAD_POS* rotatePos)
{
    int32_t yogi;
    LH_ERR errorCode;
    SM_RETURN_ZERO_CMD returnZeroCmd;
    SM_RUN_COORDINATE_CMD runCoordinateCmd;
    /*1. 试剂针升到初始位  前提:内壁灌注*/
        if(opt == TSC_NEEDLE_REAGENT_PRIME_OPT_INNER)
        {
            //升降抬起来
            Can2SubSM_ReturnZeroDataStructInit(&returnZeroCmd);

            //升降回零并修正
            SystemReadActionParamWithIndex(MODULENEEDLEREAGENTBEADPARAM,BEADUPDOWNRESETCORRECTION,&yogi);
            returnZeroCmd.stepMotorIndex = STEP_MOTOR_NEEDLE_REAGENT_BEAD_UPDOWN;
            returnZeroCmd.correctionPosition = yogi;//actionParam->moduleNeedleReagentBeadParam.upDownResetCorrection;
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
        }
    /*2. 试剂针旋转至清洗位 前提:内壁灌注*/
        if(opt == TSC_NEEDLE_REAGENT_PRIME_OPT_INNER)
        {
            Can2SubSM_RunCoordinateDataStructInit(&runCoordinateCmd);
            //旋转运行到清洗位
            SystemReadActionParamWithIndex(MODULENEEDLEREAGENTBEADPARAM,BEADROTATECLEANPOS,&yogi);
            runCoordinateCmd.stepMotorIndex = STEP_MOTOR_NEEDLE_REAGENT_BEAD_ROTATE;
            runCoordinateCmd.targetCoordinate = yogi;//actionParam->moduleNeedleReagentBeadParam.rotateCleanPos;
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
        }
    TaskUtilServiceSetNeedleReagentRotatePos(TSC_NEEDLE_REAGENT_BEAD_POS_CLEAN);
    /*3. 试剂针降到清洗位 前提:内壁灌注*/
         //升降运行到清洗位
        if(opt == TSC_NEEDLE_REAGENT_PRIME_OPT_INNER)
        {
            SystemReadActionParamWithIndex(MODULENEEDLEREAGENTBEADPARAM,BEADUPDOWNCLEANPOS,&yogi);
            runCoordinateCmd.stepMotorIndex = STEP_MOTOR_NEEDLE_REAGENT_BEAD_UPDOWN;
            runCoordinateCmd.targetCoordinate = yogi;//actionParam->moduleNeedleReagentBeadParam.upDownCleanPos;
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
        }
    /*4. 判断LT12是否满，否则启动泵阀抽废液 无限制*/
        //  SystemInputRead();传感器没有安装也不清除连接哪个端口
        //  默认满
        if(1)
        {
            //打开v307阀门
            // errorCode = SystemOutputWrite(DRAINAGE_VALVE_V307,Bit_RESET);
            // if(errorCode != LH_ERR_NONE)
            // {
            //     //打印错误日志
            //     TaskServiceSerialErrMsgShow(errorCode);
            //     //返回错误代码
            //     return errorCode;
            // }
            //打开隔膜泵dp1
            // errorCode = SystemOutputWrite(WASTE_LIQUID_PUMP,Bit_RESET);
            // if(errorCode != LH_ERR_NONE)
            // {
            //     //打印错误日志
            //     TaskServiceSerialErrMsgShow(errorCode);
            //     //返回错误代码
            //     return errorCode;
            // }
        }
    /*5. 柱塞泵m35运行至低位 前提:内壁灌注*/
        if(opt == TSC_NEEDLE_REAGENT_PRIME_OPT_INNER)
        {
            SM_RUN_COORDINATE_CMD runCoordinateCmd;
            Can2SubSM_RunCoordinateDataStructInit(&runCoordinateCmd); 
            runCoordinateCmd.stepMotorIndex = STEP_MOTOR_PUMP_NEEDLE_REAGENT_BEAD;
            runCoordinateCmd.targetCoordinate = 4500;
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
        }
    /*6. 打开阀门*/
        /*打开阀门v219  前提:内壁灌注*/
            if(opt == TSC_NEEDLE_REAGENT_PRIME_OPT_INNER)
            {
                errorCode = SystemOutputWrite(DRAINAGE_VALVE_V219,Bit_RESET);
                if(errorCode != LH_ERR_NONE)
                {
                    //打印错误日志
                    TaskServiceSerialErrMsgShow(errorCode);
                    //返回错误代码
                    return errorCode;
                }
            }
        /*打开阀门v216  前提:外壁灌注*/
            if(opt == TSC_NEEDLE_REAGENT_PRIME_OPT_OUT)
            {
                errorCode = SystemOutputWrite(DRAINAGE_VALVE_V216,Bit_RESET);
                if(errorCode != LH_ERR_NONE)
                {
                    //打印错误日志
                    TaskServiceSerialErrMsgShow(errorCode);
                    //返回错误代码
                    return errorCode;
                }
            }
    /*7. 打开泵*/
        /*打开泵dp306   前提:内壁灌注*/
            if(opt == TSC_NEEDLE_REAGENT_PRIME_OPT_INNER)
            {
                errorCode = SystemOutputWrite(DIAPHRAGM_PUMP_DP306,Bit_RESET);
                if(errorCode != LH_ERR_NONE)
                {
                    //打印错误日志
                    TaskServiceSerialErrMsgShow(errorCode);
                    //返回错误代码
                    return errorCode;
                } 
            }
        /*打开泵dp305   前提:外壁灌注*/
            if(opt == TSC_NEEDLE_REAGENT_PRIME_OPT_OUT)
            {
                errorCode = SystemOutputWrite(DIAPHRAGM_PUMP_DP305,Bit_RESET);
                if(errorCode != LH_ERR_NONE)
                {
                    //打印错误日志
                    TaskServiceSerialErrMsgShow(errorCode);
                    //返回错误代码
                    return errorCode;
                } 
            }
    /*8. 清洗时间*/    
        /*内壁灌注*/
            if(opt == TSC_NEEDLE_REAGENT_PRIME_OPT_INNER)
            {
                CoreDelayMs(actionParam->moduleNeedleReagentBeadParam.timeNeedlePrimeMs);//
            }
        /*外壁灌注*/
            if(opt == TSC_NEEDLE_REAGENT_PRIME_OPT_OUT)
            {
                CoreDelayMs(actionParam->moduleNeedleReagentBeadParam.timeNeedlePrimeMs);//
            }
    /*9. 关闭泵*/
        /*关闭泵dp306   前提:内壁灌注*/
            if(opt == TSC_NEEDLE_REAGENT_PRIME_OPT_INNER)
            {
                errorCode = SystemOutputWrite(DIAPHRAGM_PUMP_DP306,Bit_SET);
                if(errorCode != LH_ERR_NONE)
                {
                    //打印错误日志
                    TaskServiceSerialErrMsgShow(errorCode);
                    //返回错误代码
                    return errorCode;
                } 
            }
        /*关闭泵dp305   前提:外壁灌注*/
            if(opt == TSC_NEEDLE_REAGENT_PRIME_OPT_OUT)
            {
                errorCode = SystemOutputWrite(DIAPHRAGM_PUMP_DP305,Bit_SET);
                if(errorCode != LH_ERR_NONE)
                {
                    //打印错误日志
                    TaskServiceSerialErrMsgShow(errorCode);
                    //返回错误代码
                    return errorCode;
                } 
            }
    /*10. 延迟100ms*/
        CoreDelayMs(100);
    /*11. 关闭阀门*/
        /*关闭阀门v219  前提:内壁灌注*/
            if(opt == TSC_NEEDLE_REAGENT_PRIME_OPT_INNER)
            {
                errorCode = SystemOutputWrite(DRAINAGE_VALVE_V219,Bit_SET);
                if(errorCode != LH_ERR_NONE)
                {
                    //打印错误日志
                    TaskServiceSerialErrMsgShow(errorCode);
                    //返回错误代码
                    return errorCode;
                }
            }
        /*关闭阀门v216  前提:外壁灌注*/
            if(opt == TSC_NEEDLE_REAGENT_PRIME_OPT_OUT)
            {
                errorCode = SystemOutputWrite(DRAINAGE_VALVE_V216,Bit_SET);
                if(errorCode != LH_ERR_NONE)
                {
                    //打印错误日志
                    TaskServiceSerialErrMsgShow(errorCode);
                    //返回错误代码
                    return errorCode;
                }
            }
    /*12. 延迟20ms*/
        CoreDelayMs(20);
    /*13. 柱塞泵回到零点 前提：内壁灌注*/
        if(opt == TSC_NEEDLE_REAGENT_PRIME_OPT_INNER)
        {
            SM_RUN_COORDINATE_CMD runCoordinateCmd;
            Can2SubSM_RunCoordinateDataStructInit(&runCoordinateCmd); 
            runCoordinateCmd.stepMotorIndex = STEP_MOTOR_PUMP_NEEDLE_REAGENT_BEAD;
            runCoordinateCmd.targetCoordinate = 0;
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
        }
    /*14. 关闭隔膜泵dp1*/
        // errorCode = SystemOutputWrite(WASTE_LIQUID_PUMP,Bit_SET);
        // if(errorCode != LH_ERR_NONE)
        // {
        //     //打印错误日志
        //     TaskServiceSerialErrMsgShow(errorCode);
        //     //返回错误代码
        //     return errorCode;
        // }
    /*15. 关闭阀v307*/
        // errorCode = SystemOutputWrite(DRAINAGE_VALVE_V307,Bit_SET);
        // if(errorCode != LH_ERR_NONE)
        // {
        //     //打印错误日志
        //     TaskServiceSerialErrMsgShow(errorCode);
        //     //返回错误代码
        //     return errorCode;
        // }
    /*16. 试剂针回到零点*/
        //升降回零并修正
        SystemReadActionParamWithIndex(MODULENEEDLEREAGENTBEADPARAM,BEADUPDOWNRESETCORRECTION,&yogi);
        returnZeroCmd.stepMotorIndex = STEP_MOTOR_NEEDLE_REAGENT_BEAD_UPDOWN;
        returnZeroCmd.correctionPosition = yogi;//actionParam->moduleNeedleReagentBeadParam.upDownResetCorrection;
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
    
   
    *rotatePos = TaskUtilServiceGetNeedleReagentRotatePos();
    return errorCode;
}
//磁珠试剂针灌注----------老化测试
LH_ERR ActionNeedleReagentBead_Prime_Debug(TSC_NEEDLE_REAGENT_PRIME_OPT opt,TSC_NEEDLE_REAGENT_BEAD_POS* rotatePos)
{
    return LH_ERR_NONE;
}
//磁珠试剂针灌注
LH_ERR ActionNeedleReagentBead_Prime(TSC_NEEDLE_REAGENT_PRIME_OPT opt,TSC_NEEDLE_REAGENT_BEAD_POS* rotatePos)
{
    LH_ERR errorCode;

    TSC_TEST_MODE_SELECTION mode;    

    mode = TSC_Read_TestMode();

    if(mode == TSC_TEST_MODE_NORMAL)
    {
        errorCode = ActionNeedleReagentBead_Prime_Normal(opt,rotatePos);//正常测试
    }
    else 
    {
        errorCode = ActionNeedleReagentBead_Prime_Debug(opt,rotatePos);//老化测试
    }
    

    return errorCode;
}







