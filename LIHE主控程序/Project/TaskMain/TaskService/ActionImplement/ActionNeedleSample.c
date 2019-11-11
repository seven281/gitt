#include "ActionNeedleSample.h"
#include "TSC_TestMode.h"
#include <math.h>
#include <stdlib.h>
//样本针当前旋转位置
//TSC_NEEDLE_SAMPLE_POS currentNeedleSampleRotatePos = TSC_NEEDLE_SAMPLE_POS_CLEAN;
//static int32_t samplepumpinjectUL; 
#define NEEDLE_SAMPLE_START_LIQUID_START      200//开始探液的坐标偏移


//样本针初始化
LH_ERR ActionNeedleSample_Init(TSC_NEEDLE_SAMPLE_POS* needleRotatePos)
{
    LH_ERR errorCode;

    //电机复位结构体
    SM_RESET_CMD resetCommand;
    //结构体初始化
    Can2SubSM_ResetCmdDataStructInit(&resetCommand);

    //复位参数设置
    resetCommand.stepMotorIndex = STEP_MOTOR_NEEDLE_SAMPLE_UPDOWN;
    resetCommand.correctionPosition = actionParam->moduleNeedleSampleParam.upDownResetCorrection;
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
    resetCommand.stepMotorIndex = STEP_MOTOR_NEEDLE_SAMPLE_ROTATE;
    resetCommand.correctionPosition = actionParam->moduleNeedleSampleParam.rotateResetCorrection;
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
    
    //记录旋转位置
    TaskUtilServiceSetNeedleSampleRotatePos(TSC_NEEDLE_SAMPLE_POS_CLEAN);

    //复位参数设置
    resetCommand.stepMotorIndex = STEP_MOTOR_PUMP_NEEDLE_SAMPLE;
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

    *needleRotatePos = TaskUtilServiceGetNeedleSampleRotatePos();
    return errorCode;
}

//样本针旋转到指定位置
LH_ERR ActionNeedleSample_RotateSpecialPos(TSC_NEEDLE_SAMPLE_POS targetPos,TSC_NEEDLE_SAMPLE_POS* needleRotatePos)
{
    LH_ERR errorCode;
    int32_t posValue = 0;
    //选择目标参数
    switch(targetPos)
    {
        case TSC_NEEDLE_SAMPLE_POS_CLEAN:
            posValue = actionParam->moduleNeedleSampleParam.rotateCleanPos;
            break;
        case TSC_NEEDLE_SAMPLE_POS_FORCE_CLEAN:
            posValue = actionParam->moduleNeedleSampleParam.rotateForceCleanPos;
            break;
        case TSC_NEEDLE_SAMPLE_POS_RACK:
            posValue = actionParam->moduleNeedleSampleParam.rotateTestTubeAbsorbPos;
            break;
        case TSC_NEEDLE_SAMPLE_POS_REACTION_INJECT:
            posValue = actionParam->moduleNeedleSampleParam.rotateReactionInjectPos;
            break;
        case TSC_NEEDLE_SAMPLE_POS_REACTION_ABSORB:
            posValue = actionParam->moduleNeedleSampleParam.rotateReactionAbsorbPos;
            break;
        default:
            errorCode = LH_ERR_ACTION_PARAM;
            TaskServiceSerialErrMsgShow(errorCode);
            return errorCode;
    }

    SM_RUN_COORDINATE_CMD runCoordinateCmd;
    Can2SubSM_RunCoordinateDataStructInit(&runCoordinateCmd);

    //旋转
    runCoordinateCmd.stepMotorIndex = STEP_MOTOR_NEEDLE_SAMPLE_ROTATE;
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
    TaskUtilServiceSetNeedleSampleRotatePos(targetPos);

    *needleRotatePos = TaskUtilServiceGetNeedleSampleRotatePos();
    return errorCode;
}

//获取升降坐标
static LH_ERR UtilGetNeedleSampleDownPos(int32_t* posDown,uint8_t type)
{
    TSC_NEEDLE_SAMPLE_POS currentNeedleSampleRotatePos;
    currentNeedleSampleRotatePos = TaskUtilServiceGetNeedleSampleRotatePos();
    if(currentNeedleSampleRotatePos == TSC_NEEDLE_SAMPLE_POS_RACK)
    {
        //试管架吸样本位
        if(type == 0)
            *posDown = actionParam->moduleNeedleSampleParam.upDownTestTubeAbsorbStartPos;
        else
            *posDown = actionParam->moduleNeedleSampleParam.upDownTestTubeAbsorbMaxPos;
        
        return LH_ERR_NONE;
    }
    else if(currentNeedleSampleRotatePos == TSC_NEEDLE_SAMPLE_POS_CLEAN)
    {
        //清洗位
        *posDown = actionParam->moduleNeedleSampleParam.upDownCleanPos;
        return LH_ERR_NONE;
    }
    else if(currentNeedleSampleRotatePos == TSC_NEEDLE_SAMPLE_POS_FORCE_CLEAN)
    {
        //强洗位
        *posDown = actionParam->moduleNeedleSampleParam.upDownForceCleanPos;
        return LH_ERR_NONE;
    }
    else if(currentNeedleSampleRotatePos == TSC_NEEDLE_SAMPLE_POS_REACTION_INJECT)
    {
        //反应盘排样本位
        *posDown = actionParam->moduleNeedleSampleParam.upDownReactionInjectMaxPos;
        return LH_ERR_NONE;
    }
    else if(currentNeedleSampleRotatePos == TSC_NEEDLE_SAMPLE_POS_REACTION_ABSORB)
    {
        //反应盘吸样本位
        *posDown = actionParam->moduleNeedleSampleParam.upDownReactionAbsorbMaxPos;
        return LH_ERR_NONE;
    }
    return LH_ERR_ACTION_PARAM;
}

//样本针吸样本--------------------正常模式
LH_ERR ActionNeedleSample_AbsorbLiquid_Normal(uint16_t liquidUL,TSC_NEEDLE_SAMPLE_POS* needleRotatePos,SAMPLE_STATUS use)
{
    LH_ERR errorCode;
    int32_t posDown = 0;
    int32_t AlarmStopPos=0;
    int32_t startpos,maxpos;
    static int32_t presteps=0,currentsteps=0;
    int32_t b;
    SM_RUN_STEP_CMD runStepCommand;
    SM_RUN_COORDINATE_CMD runCoordinateCmd;
    /*0. 判断样本针在样本试剂上方*/
        UtilGetNeedleSampleDownPos(&startpos,0);//获取吸样开始坐标

        if(TaskUtilServiceGetNeedleSampleRotatePos() != TSC_NEEDLE_SAMPLE_POS_RACK)
        {
            return LH_ERR_ACTION_PARAM_ERR;
        }
        else
        {
            if(use == SAMPLE_NEW)
            {
                /*新样本探液开始坐标为默认起始坐标*/
                posDown = startpos;
                presteps = 0;//检测探液板功能中间变量清零
                currentsteps = 0;
            }
            else
            {
                /*旧样本探液开始坐标需要经过计算*/
                posDown = TaskUtilServiceGetNeedleSamplePumpAlarmPos()-NEEDLE_SAMPLE_START_LIQUID_START;
            }
        }
    /*1. 柱塞泵m37复位*/
        Can2SubSM_RunCoordinateDataStructInit(&runCoordinateCmd); 
        runCoordinateCmd.stepMotorIndex = STEP_MOTOR_PUMP_NEEDLE_SAMPLE;
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
    /*2. 柱塞泵m37运行吸一段空气*/
        Can2SubSM_RunStepsCmdDataStructInit(&runStepCommand);

        runStepCommand.stepMotorIndex = STEP_MOTOR_PUMP_NEEDLE_SAMPLE;
        runStepCommand.steps = actionParam->moduleNeedleSampleParam.pumpAbsortWithAirUl;
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
        errorCode = Can2SubNeedleSampleLiquidDetectDisableFunc();
        if(errorCode != LH_ERR_NONE)
        {
            //打印错误日志
            TaskServiceSerialErrMsgShow(errorCode);

            return LH_ERR_LIQUID_DETECT_COMM_FAIL;

        }
    /*4. 样本针快速下降到吸样开始位置---快速 */    
        Can2SubSM_RunCoordinateDataStructInit(&runCoordinateCmd);
        
        //升降下探
        runCoordinateCmd.stepMotorIndex = STEP_MOTOR_NEEDLE_SAMPLE_UPDOWN;
        runCoordinateCmd.targetCoordinate = posDown;
        runCoordinateCmd.curveSelect = NEEDLE_SAMPLE_UPDOWN_FAST;
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
    /*5. 开启探液功能*/
        errorCode = Can2SubNeedleSampleLiquidDetectEnableFunc();
        if(errorCode != LH_ERR_NONE)
        {
            //打印错误日志
            TaskServiceSerialErrMsgShow(errorCode);
            return LH_ERR_LIQUID_DETECT_COMM_FAIL;
        }
    /*6. 样本针下降到样本试管里----慢速 */
        //升降下降到清洗位
        Can2SubSM_RunCoordinateDataStructInit(&runCoordinateCmd);
        UtilGetNeedleSampleDownPos(&maxpos,1);//获取吸样极限坐标
        //升降下探
        runCoordinateCmd.stepMotorIndex = STEP_MOTOR_NEEDLE_SAMPLE_UPDOWN;
        runCoordinateCmd.targetCoordinate = maxpos;
        runCoordinateCmd.curveSelect = NEEDLE_SAMPLE_UPDOWN_SLOW;
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

        //检测探液信号  
        if(errorCode == LH_ERR_SM_UTIL_STOP1)
        {
            Can2SubSM_ReadPositionWhileReturn(STEP_MOTOR_NEEDLE_SAMPLE_UPDOWN,&AlarmStopPos);
            TaskUtilServiceSetNeedleSamplePumpAlarmPos(AlarmStopPos);//设置探液时坐标

            //通过探液面的步数斜率计算检测探液功能是否正常
            presteps = currentsteps;
            currentsteps = AlarmStopPos; 

            if((presteps)&&(currentsteps))
            {
                b = abs(currentsteps - presteps);
                if(b>NEEDLE_SAMPLE_START_LIQUID_START)
                {
                    return LH_ERR_LIQUID_DETECT_FUNCTION;
                }
            }
        }
        else//没有探到液体
        {
            //打印错误日志
            TaskServiceSerialErrMsgShow(LH_ERR_LIQUID_DETECT_EMPTY);
            //返回错误代码
            return LH_ERR_LIQUID_DETECT_EMPTY;
        }
    /*7. 样本针下降电机还需要再走几步吗？？？？*/
    /*8. 柱塞泵M37运行吸取样本*/
        Can2SubSM_RunStepsCmdDataStructInit(&runStepCommand);

        runStepCommand.stepMotorIndex = STEP_MOTOR_PUMP_NEEDLE_SAMPLE;
        runStepCommand.steps = actionParam->moduleNeedleSampleParam.pumpOneUlStep * liquidUL;
        runStepCommand.curveSelect = SM_CURVE_0;
        runStepCommand.speedRatio = 100;
        runStepCommand.speedMode = SPEED_MODE_UP_START;
        runStepCommand.utilStop1Enable = DISABLE;
        runStepCommand.utilStop2Enable = DISABLE;
        runStepCommand.timeOutMs = 60000;
        //电机执行步数
        errorCode = Can2SubSM_RunSpecialStepsWhileReturn(&runStepCommand);
        TaskUtilServiceSetNeedleSamplePumpUesdUl(liquidUL);
        if(errorCode != LH_ERR_NONE)
        {
            //打印错误日志
            TaskServiceSerialErrMsgShow(errorCode);
            //返回错误代码
            return errorCode;
        }
    /*9. 样本针上升*/
        
        Can2SubSM_RunCoordinateDataStructInit(&runCoordinateCmd);

        //升降下探
        runCoordinateCmd.stepMotorIndex = STEP_MOTOR_NEEDLE_SAMPLE_UPDOWN;
        runCoordinateCmd.targetCoordinate = actionParam->moduleNeedleSampleParam.upDownResetCorrection;
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
    /*10. 柱塞泵m37运行吸取空气*/
        Can2SubSM_RunStepsCmdDataStructInit(&runStepCommand);

        runStepCommand.stepMotorIndex = STEP_MOTOR_PUMP_NEEDLE_SAMPLE;
        runStepCommand.steps = actionParam->moduleNeedleSampleParam.pumpAbsortWithAirU2;
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

    *needleRotatePos = TaskUtilServiceGetNeedleSampleRotatePos();
    return errorCode;
}
//样本针吸样本-------------------老化模式
LH_ERR ActionNeedleSample_AbsorbLiquid_Debug(uint16_t liquidUL,TSC_NEEDLE_SAMPLE_POS* needleRotatePos)
{
    LH_ERR errorCode;
    int32_t posDown = 0;
    SM_RUN_COORDINATE_CMD runCoordinateCmd;
    /*1. 样本针下降到样本试管里*/
        //升降下降到清洗位
        errorCode = UtilGetNeedleSampleDownPos(&posDown,0);
        if(errorCode != LH_ERR_NONE)
        {
            TaskServiceSerialErrMsgShow(errorCode);
            return errorCode;
        }
        Can2SubSM_RunCoordinateDataStructInit(&runCoordinateCmd);

        //升降下探
        runCoordinateCmd.stepMotorIndex = STEP_MOTOR_NEEDLE_SAMPLE_UPDOWN;
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
    /*2. 样本针上升*/
        
        Can2SubSM_RunCoordinateDataStructInit(&runCoordinateCmd);

        //升降下探
        runCoordinateCmd.stepMotorIndex = STEP_MOTOR_NEEDLE_SAMPLE_UPDOWN;
        runCoordinateCmd.targetCoordinate = actionParam->moduleNeedleSampleParam.upDownResetCorrection;
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
    *needleRotatePos = TaskUtilServiceGetNeedleSampleRotatePos();
    return errorCode;
}
//样本针吸样本
LH_ERR ActionNeedleSample_AbsorbLiquid(uint16_t liquidUL,TSC_NEEDLE_SAMPLE_POS* needleRotatePos,SAMPLE_STATUS use)
{
    LH_ERR errorCode;

    TSC_TEST_MODE_SELECTION mode;
    mode = TSC_Read_TestMode();

    if(mode == TSC_TEST_MODE_NORMAL)
    {
        errorCode = ActionNeedleSample_AbsorbLiquid_Normal(liquidUL,needleRotatePos,use);//正常测试
    }
    else 
    {
        errorCode = ActionNeedleSample_AbsorbLiquid_Debug(liquidUL,needleRotatePos);//老化测试
    }

    return errorCode;
}


//样本针排样本--------------正常模式
LH_ERR ActionNeedleSample_InjectLiquid_Normal(uint16_t liquidUL,TSC_NEEDLE_SAMPLE_POS* needleRotatePos)
{
    LH_ERR errorCode;
    int32_t posDown = 0;
    int32_t yogi;
    SM_RUN_STEP_CMD runStepCommand;
    SM_RUN_COORDINATE_CMD runCoordinateCmd;
    
    /*2. 样本针下降到试管里*/
        errorCode = UtilGetNeedleSampleDownPos(&posDown,0);
        if(errorCode != LH_ERR_NONE)
        {
            TaskServiceSerialErrMsgShow(errorCode);
            return errorCode;
        }
        Can2SubSM_RunCoordinateDataStructInit(&runCoordinateCmd);

        //升降下探
        runCoordinateCmd.stepMotorIndex = STEP_MOTOR_NEEDLE_SAMPLE_UPDOWN;
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
    /*3. 柱塞泵m37排除指定的样本*/
        Can2SubSM_RunStepsCmdDataStructInit(&runStepCommand);

        runStepCommand.stepMotorIndex = STEP_MOTOR_PUMP_NEEDLE_SAMPLE;
        if(liquidUL == 0)
        {
            yogi = (actionParam->moduleNeedleSampleParam.pumpAbsortWithAirUl    
                    +actionParam->moduleNeedleSampleParam.pumpAbsortWithAirU2
                    +TaskUtilServiceGetNeedleSamplePumpUesdUl()
            );
            yogi = yogi - actionParam->moduleNeedleSampleParam.pumpretainWithAir;
            runStepCommand.steps = actionParam->moduleNeedleSampleParam.pumpOneUlStep * yogi*(-1);
        }
        else
        {
            runStepCommand.steps = actionParam->moduleNeedleSampleParam.pumpOneUlStep * liquidUL*(-1);
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
    /*4. 样本针上升回到零点*/
        //针回零
        SM_RETURN_ZERO_CMD returnZeroCmd;
        Can2SubSM_ReturnZeroDataStructInit(&returnZeroCmd);
        returnZeroCmd.stepMotorIndex = STEP_MOTOR_NEEDLE_SAMPLE_UPDOWN;
        returnZeroCmd.correctionPosition = actionParam->moduleNeedleSampleParam.upDownResetCorrection;
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
    *needleRotatePos = TaskUtilServiceGetNeedleSampleRotatePos();
    return errorCode;
}
//样本针排样本-------------老化模式
LH_ERR ActionNeedleSample_InjectLiquid_Debug(uint16_t liquidUL,TSC_NEEDLE_SAMPLE_POS* needleRotatePos)
{
    LH_ERR errorCode;
    int32_t posDown = 0;
    errorCode = UtilGetNeedleSampleDownPos(&posDown,0);
    if(errorCode != LH_ERR_NONE)
	{
        TaskServiceSerialErrMsgShow(errorCode);
		return errorCode;
	}

    SM_RUN_COORDINATE_CMD runCoordinateCmd;
    Can2SubSM_RunCoordinateDataStructInit(&runCoordinateCmd);

    //升降下探
    runCoordinateCmd.stepMotorIndex = STEP_MOTOR_NEEDLE_SAMPLE_UPDOWN;
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

    //泵排液

    //针回零
    SM_RETURN_ZERO_CMD returnZeroCmd;
    Can2SubSM_ReturnZeroDataStructInit(&returnZeroCmd);
    returnZeroCmd.stepMotorIndex = STEP_MOTOR_NEEDLE_SAMPLE_UPDOWN;
    returnZeroCmd.correctionPosition = actionParam->moduleNeedleSampleParam.upDownResetCorrection;
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

    *needleRotatePos = TaskUtilServiceGetNeedleSampleRotatePos();
    return errorCode;
}
//样本针排样本
LH_ERR ActionNeedleSample_InjectLiquid(uint16_t liquidUL,TSC_NEEDLE_SAMPLE_POS* needleRotatePos)
{
    LH_ERR errorCode;

    TSC_TEST_MODE_SELECTION mode;
    mode = TSC_Read_TestMode();

    if(mode == TSC_TEST_MODE_NORMAL)
    {
        errorCode = ActionNeedleSample_InjectLiquid_Normal(liquidUL,needleRotatePos);//正常测试
    }
    else 
    {
        errorCode = ActionNeedleSample_AbsorbLiquid_Debug(liquidUL,needleRotatePos);//老化测试
    }

    return errorCode;
}


//样本针清洗--------正常模式
LH_ERR ActionNeedleSample_Clean_Normal(TSC_NEEDLE_SAMPLE_CLEAN_OPT opt,TSC_NEEDLE_SAMPLE_POS overTargetPos,TSC_NEEDLE_SAMPLE_POS* needleRotatePos)
{
    LH_ERR errorCode;
    int32_t posDown = 0;
    /*1. 判断样本针转到清洗位*/
        if(TaskUtilServiceGetNeedleSampleRotatePos() != TSC_NEEDLE_SAMPLE_POS_CLEAN)
        {
            return LH_ERR_ACTION_PARAM_ERR;
        }
        else
        {
            errorCode = UtilGetNeedleSampleDownPos(&posDown,0);
            if(errorCode != LH_ERR_NONE)
            {
                TaskServiceSerialErrMsgShow(errorCode);
                return errorCode;
            }
        }
        
    /*2. 样本针下降到清洗位*/
        //升降下降到清洗位
        SM_RUN_COORDINATE_CMD runCoordinateCmd;
        Can2SubSM_RunCoordinateDataStructInit(&runCoordinateCmd);

        //升降下探
        runCoordinateCmd.stepMotorIndex = STEP_MOTOR_NEEDLE_SAMPLE_UPDOWN;
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
    /*3. 打开抽水泵阀*/
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
    /*4. 柱塞泵m37运行至低位*/
        
        Can2SubSM_RunCoordinateDataStructInit(&runCoordinateCmd); 
        runCoordinateCmd.stepMotorIndex = STEP_MOTOR_PUMP_NEEDLE_SAMPLE;
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
    /*5. 打开阀v218*/
        errorCode = SystemOutputWrite(DRAINAGE_VALVE_V218,Bit_RESET);
        if(errorCode != LH_ERR_NONE)
        {
            //打印错误日志
            TaskServiceSerialErrMsgShow(errorCode);
            //返回错误代码
            return errorCode;
        }   
    /*6. 打开阀v221*/
        errorCode = SystemOutputWrite(DRAINAGE_VALVE_V221,Bit_RESET);
        if(errorCode != LH_ERR_NONE)
        {
            //打印错误日志
            TaskServiceSerialErrMsgShow(errorCode);
            //返回错误代码
            return errorCode;
        }
    /*7. 打开隔膜泵dp305*/
        errorCode = SystemOutputWrite(DIAPHRAGM_PUMP_DP305,Bit_RESET);
        if(errorCode != LH_ERR_NONE)
        {
            //打印错误日志
            TaskServiceSerialErrMsgShow(errorCode);
            //返回错误代码
            return errorCode;
        } 
    /*8. 打开隔膜泵dp307*/
        errorCode = SystemOutputWrite(DIAPHRAGM_PUMP_DP307,Bit_RESET);
        if(errorCode != LH_ERR_NONE)
        {
            //打印错误日志
            TaskServiceSerialErrMsgShow(errorCode);
            //返回错误代码
            return errorCode;
        } 
    /*9. 内外壁清洗延时时间*/
        //指定时间清洗
        CoreDelayMs(actionParam->moduleNeedleSampleParam.timeNeedleCleanMs);
    /*10. 关闭隔膜泵dp307*/
        errorCode = SystemOutputWrite(DIAPHRAGM_PUMP_DP307,Bit_SET);
        if(errorCode != LH_ERR_NONE)
        {
            //打印错误日志
            TaskServiceSerialErrMsgShow(errorCode);
            //返回错误代码
            return errorCode;
        } 
    /*11. 关闭隔膜泵dp305*/
        errorCode = SystemOutputWrite(DIAPHRAGM_PUMP_DP305,Bit_SET);
        if(errorCode != LH_ERR_NONE)
        {
            //打印错误日志
            TaskServiceSerialErrMsgShow(errorCode);
            //返回错误代码
            return errorCode;
        } 
    /*12. 延时100ms*/
        CoreDelayMs(100);           
    /*13. 关闭阀门v221*/
        errorCode = SystemOutputWrite(DRAINAGE_VALVE_V221,Bit_SET);
        if(errorCode != LH_ERR_NONE)
        {
            //打印错误日志
            TaskServiceSerialErrMsgShow(errorCode);
            //返回错误代码
            return errorCode;
        }
    /*14. 关闭阀门v218*/
        errorCode = SystemOutputWrite(DRAINAGE_VALVE_V218,Bit_SET);
        if(errorCode != LH_ERR_NONE)
        {
            //打印错误日志
            TaskServiceSerialErrMsgShow(errorCode);
            //返回错误代码
            return errorCode;
        }   
    /*15. 延时20ms*/
        CoreDelayMs(20);    
    /*16. 柱塞泵M37回零点*/
        
        Can2SubSM_RunCoordinateDataStructInit(&runCoordinateCmd); 
        runCoordinateCmd.stepMotorIndex = STEP_MOTOR_PUMP_NEEDLE_SAMPLE;
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
    /*17. 关隔膜泵dp1*/
        // errorCode = SystemOutputWrite(WASTE_LIQUID_PUMP,Bit_SET);
        // if(errorCode != LH_ERR_NONE)
        // {
        //     //打印错误日志
        //     TaskServiceSerialErrMsgShow(errorCode);
        //     //返回错误代码
        //     return errorCode;
        // }
    /*18. 关阀v307*/
        // errorCode = SystemOutputWrite(DRAINAGE_VALVE_V307,Bit_RESET);
        // if(errorCode != LH_ERR_NONE)
        // {
        //     //打印错误日志
        //     TaskServiceSerialErrMsgShow(errorCode);
        //     //返回错误代码
        //     return errorCode;
        // }
    /*19. 样本针上升到原点位置*/
        //升降回零
        SM_RETURN_ZERO_CMD returnZeroCmd;
        Can2SubSM_ReturnZeroDataStructInit(&returnZeroCmd);
        returnZeroCmd.stepMotorIndex = STEP_MOTOR_NEEDLE_SAMPLE_UPDOWN;
        returnZeroCmd.correctionPosition = actionParam->moduleNeedleSampleParam.upDownResetCorrection;
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
    /*20. 样本针旋转到目标位置*/
        //旋转到清洗完成之后的目标位置
        errorCode = ActionNeedleSample_RotateSpecialPos(overTargetPos,needleRotatePos);
        if(errorCode != LH_ERR_NONE)
        {
            return errorCode;
        }

        return errorCode;
}
//样本针清洗-------老化模式
LH_ERR ActionNeedleSample_Clean_Debug(TSC_NEEDLE_SAMPLE_CLEAN_OPT opt,TSC_NEEDLE_SAMPLE_POS overTargetPos,TSC_NEEDLE_SAMPLE_POS* needleRotatePos)
{
    LH_ERR errorCode; 
    //旋转到清洗位
    errorCode = ActionNeedleSample_RotateSpecialPos(TSC_NEEDLE_SAMPLE_POS_CLEAN,needleRotatePos);
    if(errorCode != LH_ERR_NONE)
    {
        return errorCode;
    }

    //升降下降到清洗位
    SM_RUN_COORDINATE_CMD runCoordinateCmd;
    Can2SubSM_RunCoordinateDataStructInit(&runCoordinateCmd);

    //升降下探
    runCoordinateCmd.stepMotorIndex = STEP_MOTOR_NEEDLE_SAMPLE_UPDOWN;
    runCoordinateCmd.targetCoordinate = actionParam->moduleNeedleSampleParam.upDownCleanPos;
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

    //泵阀的控制

    //指定时间清洗
    CoreDelayMs(actionParam->moduleNeedleSampleParam.timeNeedleCleanMs);

    //清洗完成的泵阀的控制

    //升降回零
    SM_RETURN_ZERO_CMD returnZeroCmd;
    Can2SubSM_ReturnZeroDataStructInit(&returnZeroCmd);
    returnZeroCmd.stepMotorIndex = STEP_MOTOR_NEEDLE_SAMPLE_UPDOWN;
    returnZeroCmd.correctionPosition = actionParam->moduleNeedleSampleParam.upDownResetCorrection;
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

    //旋转到清洗完成之后的目标位置
    errorCode = ActionNeedleSample_RotateSpecialPos(overTargetPos,needleRotatePos);
    if(errorCode != LH_ERR_NONE)
    {
        return errorCode;
    }

    return errorCode;
}

//样本针清洗 内外壁同时清洗
LH_ERR ActionNeedleSample_Clean(TSC_NEEDLE_SAMPLE_CLEAN_OPT opt,TSC_NEEDLE_SAMPLE_POS overTargetPos,TSC_NEEDLE_SAMPLE_POS* needleRotatePos)
{
    LH_ERR errorCode;

    TSC_TEST_MODE_SELECTION mode;
    mode = TSC_Read_TestMode();

    if(mode == TSC_TEST_MODE_NORMAL)
    {
        errorCode = ActionNeedleSample_Clean_Normal(opt,overTargetPos,needleRotatePos);//正常测试
    }
    else 
    {
        errorCode = ActionNeedleSample_Clean_Debug(opt,overTargetPos,needleRotatePos);//老化测试
    }

    return errorCode;
}


//样本针强力清洗-------正常
LH_ERR ActionNeedleSample_ForceClean_Normal(TSC_NEEDLE_SAMPLE_POS* needleRotatePos)
{
    LH_ERR errorCode;
    int32_t posDown = 0;
    /*1. 判断样本针是否在强力清洗位上方*/
        if(TaskUtilServiceGetNeedleSampleRotatePos() != TSC_NEEDLE_SAMPLE_POS_FORCE_CLEAN)
        {
            return LH_ERR_ACTION_PARAM_ERR;
        }
        else
        {
            errorCode = UtilGetNeedleSampleDownPos(&posDown,0);
            if(errorCode != LH_ERR_NONE)
            {
                TaskServiceSerialErrMsgShow(errorCode);
                return errorCode;
            }
        }
        
    /*2. 下降到强力清洗位*/
        //升降下降到强力清洗位
        SM_RUN_COORDINATE_CMD runCoordinateCmd;
        Can2SubSM_RunCoordinateDataStructInit(&runCoordinateCmd);

        //升降下探
        runCoordinateCmd.stepMotorIndex = STEP_MOTOR_NEEDLE_SAMPLE_UPDOWN;
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

    /*3. 打开抽液泵阀*/
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
    /*4. 打开阀门v215*/
        errorCode = SystemOutputWrite(DRAINAGE_VALVE_V215,Bit_RESET);
        if(errorCode != LH_ERR_NONE)
        {
            //打印错误日志
            TaskServiceSerialErrMsgShow(errorCode);
            //返回错误代码
            return errorCode;
        }
    /*5. 打开隔膜泵dp304*/
        errorCode = SystemOutputWrite(DIAPHRAGM_PUMP_DP304,Bit_RESET);
        if(errorCode != LH_ERR_NONE)
        {
            //打印错误日志
            TaskServiceSerialErrMsgShow(errorCode);
            //返回错误代码
            return errorCode;
        } 
    /*6. 清洗时间*/
        //指定时间清洗
        CoreDelayMs(actionParam->moduleNeedleSampleParam.timeNeedleForceCleanMs);
    /*7. 关闭隔膜泵dp304*/
        errorCode = SystemOutputWrite(DIAPHRAGM_PUMP_DP304,Bit_SET);
        if(errorCode != LH_ERR_NONE)
        {
            //打印错误日志
            TaskServiceSerialErrMsgShow(errorCode);
            //返回错误代码
            return errorCode;
        } 
    /*8. 延时20ms*/
        CoreDelayMs(20);
    /*9. 关闭阀v215*/
        errorCode = SystemOutputWrite(DRAINAGE_VALVE_V215,Bit_SET);
        if(errorCode != LH_ERR_NONE)
        {
            //打印错误日志
            TaskServiceSerialErrMsgShow(errorCode);
            //返回错误代码
            return errorCode;
        }
    /*10. 延时20ms*/
        CoreDelayMs(20);
    /*11. 柱塞泵m37吸吐第一次*/
        
        Can2SubSM_RunCoordinateDataStructInit(&runCoordinateCmd); 
        runCoordinateCmd.stepMotorIndex = STEP_MOTOR_PUMP_NEEDLE_SAMPLE;
        runCoordinateCmd.targetCoordinate = 2000;
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

        
        Can2SubSM_RunCoordinateDataStructInit(&runCoordinateCmd); 
        runCoordinateCmd.stepMotorIndex = STEP_MOTOR_PUMP_NEEDLE_SAMPLE;
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
    /*12. 柱塞泵m37吸吐第二次*/
        
        Can2SubSM_RunCoordinateDataStructInit(&runCoordinateCmd); 
        runCoordinateCmd.stepMotorIndex = STEP_MOTOR_PUMP_NEEDLE_SAMPLE;
        runCoordinateCmd.targetCoordinate = 2000;
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

        
        Can2SubSM_RunCoordinateDataStructInit(&runCoordinateCmd); 
        runCoordinateCmd.stepMotorIndex = STEP_MOTOR_PUMP_NEEDLE_SAMPLE;
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
    /*13. 关闭隔膜泵dp1*/
        // errorCode = SystemOutputWrite(WASTE_LIQUID_PUMP,Bit_SET);
        // if(errorCode != LH_ERR_NONE)
        // {
        //     //打印错误日志
        //     TaskServiceSerialErrMsgShow(errorCode);
        //     //返回错误代码
        //     return errorCode;
        // }
    /*14. 关闭阀v307*/
        // errorCode = SystemOutputWrite(DRAINAGE_VALVE_V307,Bit_SET);
        // if(errorCode != LH_ERR_NONE)
        // {
        //     //打印错误日志
        //     TaskServiceSerialErrMsgShow(errorCode);
        //     //返回错误代码
        //     return errorCode;
        // }
    /*15. 样本针上升到零点位置*/
        //升降回零
        SM_RETURN_ZERO_CMD returnZeroCmd;
        Can2SubSM_ReturnZeroDataStructInit(&returnZeroCmd);
        returnZeroCmd.stepMotorIndex = STEP_MOTOR_NEEDLE_SAMPLE_UPDOWN;
        returnZeroCmd.correctionPosition = actionParam->moduleNeedleSampleParam.upDownResetCorrection;
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
//样本针强力清洗-------老化
LH_ERR ActionNeedleSample_ForceClean_Debug(TSC_NEEDLE_SAMPLE_POS* needleRotatePos)
{
    LH_ERR errorCode;
    //旋转到强力清洗位
    errorCode = ActionNeedleSample_RotateSpecialPos(TSC_NEEDLE_SAMPLE_POS_FORCE_CLEAN,needleRotatePos);
    if(errorCode != LH_ERR_NONE)
    {
        return errorCode;
    }

    //升降下降到强力清洗位
    SM_RUN_COORDINATE_CMD runCoordinateCmd;
    Can2SubSM_RunCoordinateDataStructInit(&runCoordinateCmd);

    //升降下探
    runCoordinateCmd.stepMotorIndex = STEP_MOTOR_NEEDLE_SAMPLE_UPDOWN;
    runCoordinateCmd.targetCoordinate = actionParam->moduleNeedleSampleParam.upDownForceCleanPos;
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

    //泵阀的控制

    //指定时间清洗
    CoreDelayMs(actionParam->moduleNeedleSampleParam.timeNeedleCleanMs);

    //清洗完成的泵阀的控制

    //升降回零
    SM_RETURN_ZERO_CMD returnZeroCmd;
    Can2SubSM_ReturnZeroDataStructInit(&returnZeroCmd);
    returnZeroCmd.stepMotorIndex = STEP_MOTOR_NEEDLE_SAMPLE_UPDOWN;
    returnZeroCmd.correctionPosition = actionParam->moduleNeedleSampleParam.upDownResetCorrection;
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
//样本针强力清洗
LH_ERR ActionNeedleSample_ForceClean(TSC_NEEDLE_SAMPLE_POS* needleRotatePos)
{
    LH_ERR errorCode;
    
    TSC_TEST_MODE_SELECTION mode;    
    mode = TSC_Read_TestMode();

    if(mode == TSC_TEST_MODE_NORMAL)
    {
        errorCode = ActionNeedleSample_ForceClean_Normal(needleRotatePos);
    }
    else
    {
        errorCode = ActionNeedleSample_ForceClean_Debug(needleRotatePos);
    }
    
    return errorCode;
}

//样本针灌注-----正常测试
LH_ERR ActionNeedleSample_Prime_Normal(TSC_NEEDLE_SAMPLE_PRIME_OPT opt,TSC_NEEDLE_SAMPLE_POS* needleRotatePos)
{
    LH_ERR errorCode;
    /*0. 电机上升到原点*/
        if((opt == TSC_NEEDLE_SAMPLE_PRIME_OPT_INNER)||(opt == TSC_NEEDLE_SAMPLE_PRIME_OPT_ALL))
        {
            //灌注开始,升降抬起
            SM_RETURN_ZERO_CMD returnZeroCmd;
            Can2SubSM_ReturnZeroDataStructInit(&returnZeroCmd);
            returnZeroCmd.stepMotorIndex = STEP_MOTOR_NEEDLE_SAMPLE_UPDOWN;
            returnZeroCmd.correctionPosition = actionParam->moduleNeedleSampleParam.upDownResetCorrection;
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
    /*1. 电机旋转               限制:内壁灌注或者同时灌注*/
        //旋转到清洗位 
        if((opt == TSC_NEEDLE_SAMPLE_PRIME_OPT_INNER)||(opt == TSC_NEEDLE_SAMPLE_PRIME_OPT_ALL))
        {
            errorCode = ActionNeedleSample_RotateSpecialPos(TSC_NEEDLE_SAMPLE_POS_CLEAN,needleRotatePos);
            if(errorCode != LH_ERR_NONE)
            {
                return errorCode;
            }
        }
    /*2. 样本针下降到清洗位      限制:内壁灌注或者同时灌注*/  
        if((opt == TSC_NEEDLE_SAMPLE_PRIME_OPT_INNER)||(opt == TSC_NEEDLE_SAMPLE_PRIME_OPT_ALL))
        {  
            //升降下降到清洗位
            SM_RUN_COORDINATE_CMD runCoordinateCmd;
            Can2SubSM_RunCoordinateDataStructInit(&runCoordinateCmd);

            //升降下探
            runCoordinateCmd.stepMotorIndex = STEP_MOTOR_NEEDLE_SAMPLE_UPDOWN;
            runCoordinateCmd.targetCoordinate = actionParam->moduleNeedleSampleParam.upDownCleanPos;
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
    /*3. 判断lt12状态决定是否抽液     没有限制*/  
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
    /*4. 柱塞泵运行至低位 限制内壁灌注或者同时灌注*/ 
        if((opt == TSC_NEEDLE_SAMPLE_PRIME_OPT_INNER)||(opt == TSC_NEEDLE_SAMPLE_PRIME_OPT_ALL))
        {
            SM_RUN_COORDINATE_CMD runCoordinateCmd;
            Can2SubSM_RunCoordinateDataStructInit(&runCoordinateCmd); 
            runCoordinateCmd.stepMotorIndex = STEP_MOTOR_PUMP_NEEDLE_SAMPLE;
            runCoordinateCmd.targetCoordinate = 4500;
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
        }
    /*5. 打开阀门*/        
        /*打开阀v221      前提:内壁灌注或者同时灌注*/
            if((opt == TSC_NEEDLE_SAMPLE_PRIME_OPT_INNER)||(opt == TSC_NEEDLE_SAMPLE_PRIME_OPT_ALL))
            {
                errorCode = SystemOutputWrite(DRAINAGE_VALVE_V221,Bit_RESET);
                if(errorCode != LH_ERR_NONE)
                {
                    //打印错误日志
                    TaskServiceSerialErrMsgShow(errorCode);
                    //返回错误代码
                    return errorCode;
                }
            }
        /*打开阀v218      前提:外壁灌注或者同时灌注*/
            if((opt == TSC_NEEDLE_SAMPLE_PRIME_OPT_OUT)||(opt == TSC_NEEDLE_SAMPLE_PRIME_OPT_ALL))
            {
                errorCode = SystemOutputWrite(DRAINAGE_VALVE_V218,Bit_RESET);
                if(errorCode != LH_ERR_NONE)
                {
                    //打印错误日志
                    TaskServiceSerialErrMsgShow(errorCode);
                    //返回错误代码
                    return errorCode;
                }
            }
    /*6. 打开泵*/
        /*打开隔膜泵dp307  限制条件:内壁灌注或者同时灌注*/
            if((opt == TSC_NEEDLE_SAMPLE_PRIME_OPT_INNER)||(opt == TSC_NEEDLE_SAMPLE_PRIME_OPT_ALL))
            {
                errorCode = SystemOutputWrite(DIAPHRAGM_PUMP_DP307,Bit_RESET);
                if(errorCode != LH_ERR_NONE)
                {
                    //打印错误日志
                    TaskServiceSerialErrMsgShow(errorCode);
                    //返回错误代码
                    return errorCode;
                } 
            }
        /*打开隔膜泵dp305  限制条件:外壁灌注或者同时灌注*/
            if((opt == TSC_NEEDLE_SAMPLE_PRIME_OPT_OUT)||(opt == TSC_NEEDLE_SAMPLE_PRIME_OPT_ALL))
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
    /*7. 灌注时间*/
        CoreDelayMs(actionParam->moduleNeedleSampleParam.timeNeedlePrimeMs);
    /*8. 关闭隔膜泵*/
        /*关dp307内壁灌注或者同时灌注*/
            if((opt == TSC_NEEDLE_SAMPLE_PRIME_OPT_INNER)||(opt == TSC_NEEDLE_SAMPLE_PRIME_OPT_ALL))
            {
                errorCode = SystemOutputWrite(DIAPHRAGM_PUMP_DP307,Bit_SET);
                if(errorCode != LH_ERR_NONE)
                {
                    //打印错误日志
                    TaskServiceSerialErrMsgShow(errorCode);
                    //返回错误代码
                    return errorCode;
                } 
            }
        /*关dp305外壁灌注或者同时灌注*/
            if((opt == TSC_NEEDLE_SAMPLE_PRIME_OPT_OUT)||(opt == TSC_NEEDLE_SAMPLE_PRIME_OPT_ALL))
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
    /*9. 延迟*/    
        /*内壁灌注和同时灌注*/
            if((opt == TSC_NEEDLE_SAMPLE_PRIME_OPT_INNER)||(opt == TSC_NEEDLE_SAMPLE_PRIME_OPT_ALL))
            {
                CoreDelayMs(100);
            }
        /*外壁灌注*/
            if(opt == TSC_NEEDLE_SAMPLE_PRIME_OPT_OUT)
            {
                CoreDelayMs(20);
            }
    /*10. 关闭阀门*/
        /*关闭v221  内壁灌注或者同时灌注*/
            if((opt == TSC_NEEDLE_SAMPLE_PRIME_OPT_INNER)||(opt == TSC_NEEDLE_SAMPLE_PRIME_OPT_ALL))
            {
                errorCode = SystemOutputWrite(DRAINAGE_VALVE_V221,Bit_SET);
                if(errorCode != LH_ERR_NONE)
                {
                    //打印错误日志
                    TaskServiceSerialErrMsgShow(errorCode);
                    //返回错误代码
                    return errorCode;
                }
            }
        /*关闭v218  外壁灌注或者同时灌注*/
            if((opt == TSC_NEEDLE_SAMPLE_PRIME_OPT_OUT)||(opt == TSC_NEEDLE_SAMPLE_PRIME_OPT_ALL))
            {
                errorCode = SystemOutputWrite(DRAINAGE_VALVE_V218,Bit_SET);
                if(errorCode != LH_ERR_NONE)
                {
                    //打印错误日志
                    TaskServiceSerialErrMsgShow(errorCode);
                    //返回错误代码
                    return errorCode;
                }
            }
    /*11. 延迟*/    
        CoreDelayMs(20);
    /*12. 柱塞泵m37复位 限制:内壁灌注或者同时灌注*/
        if((opt == TSC_NEEDLE_SAMPLE_PRIME_OPT_INNER)||(opt == TSC_NEEDLE_SAMPLE_PRIME_OPT_ALL))
        {
            SM_RUN_COORDINATE_CMD runCoordinateCmd;
            Can2SubSM_RunCoordinateDataStructInit(&runCoordinateCmd); 
            runCoordinateCmd.stepMotorIndex = STEP_MOTOR_PUMP_NEEDLE_SAMPLE;
            runCoordinateCmd.targetCoordinate = 0;
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
        }
    /*13. 关闭隔膜泵  没有限制*/
        // errorCode = SystemOutputWrite(WASTE_LIQUID_PUMP,Bit_SET);
        // if(errorCode != LH_ERR_NONE)
        // {
        //     //打印错误日志
        //     TaskServiceSerialErrMsgShow(errorCode);
        //     //返回错误代码
        //     return errorCode;
        // }
    /*14. 关闭阀门 没有限制*/
        // errorCode = SystemOutputWrite(DRAINAGE_VALVE_V307,Bit_SET);
        // if(errorCode != LH_ERR_NONE)
        // {
        //     //打印错误日志
        //     TaskServiceSerialErrMsgShow(errorCode);
        //     //返回错误代码
        //     return errorCode;
        // }
    /*15. 样本针上升     限制:内壁灌注或者同时灌注*/
        if((opt == TSC_NEEDLE_SAMPLE_PRIME_OPT_INNER)||(opt == TSC_NEEDLE_SAMPLE_PRIME_OPT_ALL))
        {
            //灌注完成,升降抬起
            SM_RETURN_ZERO_CMD returnZeroCmd;
            Can2SubSM_ReturnZeroDataStructInit(&returnZeroCmd);
            returnZeroCmd.stepMotorIndex = STEP_MOTOR_NEEDLE_SAMPLE_UPDOWN;
            returnZeroCmd.correctionPosition = actionParam->moduleNeedleSampleParam.upDownResetCorrection;
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
    return errorCode;
}
//样本针灌注-------老化测试
LH_ERR ActionNeedleSample_Prime_Debug(TSC_NEEDLE_SAMPLE_PRIME_OPT opt,TSC_NEEDLE_SAMPLE_POS* needleRotatePos)
{
    LH_ERR errorCode;
    /*1. 电机旋转               限制:内壁灌注或者同时灌注*/
        //旋转到清洗位 
        if((opt == TSC_NEEDLE_SAMPLE_PRIME_OPT_INNER)||(opt == TSC_NEEDLE_SAMPLE_PRIME_OPT_ALL))
        {
            errorCode = ActionNeedleSample_RotateSpecialPos(TSC_NEEDLE_SAMPLE_POS_CLEAN,needleRotatePos);
            if(errorCode != LH_ERR_NONE)
            {
                return errorCode;
            }
        }
    /*2. 样本针下降到清洗位      限制:内壁灌注或者同时灌注*/  
        if((opt == TSC_NEEDLE_SAMPLE_PRIME_OPT_INNER)||(opt == TSC_NEEDLE_SAMPLE_PRIME_OPT_ALL))
        {  
            //升降下降到清洗位
            SM_RUN_COORDINATE_CMD runCoordinateCmd;
            Can2SubSM_RunCoordinateDataStructInit(&runCoordinateCmd);

            //升降下探
            runCoordinateCmd.stepMotorIndex = STEP_MOTOR_NEEDLE_SAMPLE_UPDOWN;
            runCoordinateCmd.targetCoordinate = actionParam->moduleNeedleSampleParam.upDownCleanPos;
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
    /*3. 样本针上升     限制:内壁灌注或者同时灌注*/
        if((opt == TSC_NEEDLE_SAMPLE_PRIME_OPT_INNER)||(opt == TSC_NEEDLE_SAMPLE_PRIME_OPT_ALL))
        {
            //灌注完成,升降抬起
            SM_RETURN_ZERO_CMD returnZeroCmd;
            Can2SubSM_ReturnZeroDataStructInit(&returnZeroCmd);
            returnZeroCmd.stepMotorIndex = STEP_MOTOR_NEEDLE_SAMPLE_UPDOWN;
            returnZeroCmd.correctionPosition = actionParam->moduleNeedleSampleParam.upDownResetCorrection;
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
    return errorCode;
}
//样本针灌注
LH_ERR ActionNeedleSample_Prime(TSC_NEEDLE_SAMPLE_PRIME_OPT opt,TSC_NEEDLE_SAMPLE_POS* needleRotatePos)
{
    LH_ERR errorCode;
    TSC_TEST_MODE_SELECTION mode;    
    mode = TSC_Read_TestMode();
    if(mode == TSC_TEST_MODE_NORMAL)
    {
        errorCode = ActionNeedleSample_Prime_Normal(opt,needleRotatePos);//正常测试
    }
    else 
    {
        errorCode = ActionNeedleSample_Prime_Debug(opt,needleRotatePos);//老化测试
    }
    return errorCode;
}


//样本针强洗灌注-----正常测试
LH_ERR ActionNeedleSample_ForcePrime_Normal(TSC_NEEDLE_SAMPLE_POS* needleRotatePos)
{
    LH_ERR errorCode;
    /*1. 判断lt12状态决定是否抽液*/  
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
    /*2. 打开阀门v215*/
        errorCode = SystemOutputWrite(DRAINAGE_VALVE_V215,Bit_RESET);
        if(errorCode != LH_ERR_NONE)
        {
            //打印错误日志
            TaskServiceSerialErrMsgShow(errorCode);
            //返回错误代码
            return errorCode;
        }   
    /*3. 打开隔膜泵dp304*/
        errorCode = SystemOutputWrite(DIAPHRAGM_PUMP_DP304,Bit_RESET);
        if(errorCode != LH_ERR_NONE)
        {
            //打印错误日志
            TaskServiceSerialErrMsgShow(errorCode);
            //返回错误代码
            return errorCode;
        } 
    /*4. 清洗延迟*/
        CoreDelayMs(actionParam->moduleNeedleSampleParam.timeNeedleForcePrimeMs);
    /*5. 关闭隔膜泵dp304*/
        errorCode = SystemOutputWrite(DIAPHRAGM_PUMP_DP304,Bit_SET);
        if(errorCode != LH_ERR_NONE)
        {
            //打印错误日志
            TaskServiceSerialErrMsgShow(errorCode);
            //返回错误代码
            return errorCode;
        } 
    /*6. 延迟20ms*/
        CoreDelayMs(20);
    /*7. 关闭阀v215*/
        errorCode = SystemOutputWrite(DRAINAGE_VALVE_V215,Bit_SET);
        if(errorCode != LH_ERR_NONE)
        {
            //打印错误日志
            TaskServiceSerialErrMsgShow(errorCode);
            //返回错误代码
            return errorCode;
        }
    /*8. 延迟20ms*/
        CoreDelayMs(20);
    /*9. 关闭隔膜泵dp1*/
        //关闭隔膜泵dp1
        // errorCode = SystemOutputWrite(WASTE_LIQUID_PUMP,Bit_SET);
        // if(errorCode != LH_ERR_NONE)
        // {
        //     //打印错误日志
        //     TaskServiceSerialErrMsgShow(errorCode);
        //     //返回错误代码
        //     return errorCode;
        // }
    /*10. 关闭阀v307*/
        //关闭v307阀门
        // errorCode = SystemOutputWrite(DRAINAGE_VALVE_V307,Bit_SET);
        // if(errorCode != LH_ERR_NONE)
        // {
        //     //打印错误日志
        //     TaskServiceSerialErrMsgShow(errorCode);
        //     //返回错误代码
        //     return errorCode;
        // }
    return errorCode;
}
//样本针强洗灌注-----老化测试
LH_ERR ActionNeedleSample_ForcePrime_Debug(TSC_NEEDLE_SAMPLE_POS* needleRotatePos)
{
    return LH_ERR_NONE;
}
//样本针强洗灌注
LH_ERR ActionNeedleSample_ForcePrime(TSC_NEEDLE_SAMPLE_POS* needleRotatePos)
{
    LH_ERR errorCode;
    TSC_TEST_MODE_SELECTION mode;    
    mode = TSC_Read_TestMode();
    if(mode == TSC_TEST_MODE_NORMAL)
    {
        errorCode = ActionNeedleSample_ForcePrime_Normal(needleRotatePos);//正常测试
    }
    else 
    {
        errorCode = ActionNeedleSample_ForcePrime_Debug(needleRotatePos);//老化测试
    }
    return errorCode;
}



//样本针液面探测校准
uint16_t ServiceActionNeedleSampleLiquidDetectAdjust(void)
{
    LH_ERR errorCode = LH_ERR_NONE;

    /*获取当前系统模式是老化模式还是测试模式 */
        HAND_SHAKE_SERVICE_MODE serviceMode = TaskUtilServiceGetHandShakeServiceMode();

    /*老化模式,直接返回 */
        if(serviceMode == HAND_SHAKE_SERVICE_MODE_DEBUG)
        {
            return NEEDLE_SAMPLE_SUCCESS;
        }
    
    /*测试模式,开始校准 */
        errorCode = Can2SubNeedleSampleLiquidDetectAdjust();
        if(errorCode != LH_ERR_NONE)
        {
            //打印错误日志
            TaskServiceSerialErrMsgShow(errorCode);
        }
    
    /*根据结果决定返回值 */
        if(errorCode == LH_ERR_NONE)
        {
            return NEEDLE_SAMPLE_SUCCESS;//校准完成
        }
        else if(errorCode == LH_ERR_LIQUID_DETECT_COMM_FAIL)
        {
            return NEEDLE_SAMPLE_ERR_LIQUID_DETECT_CHECK_NOT_EXIST;//通讯失败
        }
        else if(errorCode == LH_ERR_LIQUID_DETECT_ADJUST_MAX_NOT_4)
        {
            return NEEDLE_SAMPLE_ERR_LIQUID_DETECT_ADJUST_LOW;//电压升不上去
        }
        else if(errorCode == LH_ERR_LIQUID_DETECT_ADJUST_MIN_NOT_1)
        {
            return NEEDLE_SAMPLE_ERR_LIQUID_DETECT_ADJUST_HIGH;//电压降不下来
        }
        else if(errorCode == LH_ERR_LIQUID_DETECT_ADJUST_FAIL)
        {
            return NEEDLE_SAMPLE_ERR_LIQUID_DETECT_ADJUST_UNKNOW;//校准找不到合适参数
        }
        else
        {
            return NEEDLE_SAMPLE_ERR_LIQUID_DETECT_UNKNOW;//未知探液板错误
        }
}

//样本针液面探测使能
uint16_t ServiceActionNeedleSampleLiquidDetectEnable(void)
{
    LH_ERR errorCode = LH_ERR_NONE;

    /*获取当前系统模式是老化模式还是测试模式 */
        HAND_SHAKE_SERVICE_MODE serviceMode = TaskUtilServiceGetHandShakeServiceMode();

    /*老化模式,直接返回 */
        if(serviceMode == HAND_SHAKE_SERVICE_MODE_DEBUG)
        {
            return NEEDLE_SAMPLE_SUCCESS;
        }
    
    /*测试模式*/
        errorCode = Can2SubNeedleSampleLiquidDetectEnableFunc();
        if(errorCode != LH_ERR_NONE)
        {
            //打印错误日志
            TaskServiceSerialErrMsgShow(errorCode);
        }

    if(errorCode == LH_ERR_NONE)
    {
        return NEEDLE_SAMPLE_SUCCESS;//校准完成
    }
    else if(errorCode == LH_ERR_LIQUID_DETECT_SINGAL_HOLE_TOO_LONG)
    {
        return NEEDLE_SAMPLE_ERR_LIQUID_DETECT_ENABLE_FAIL;//探液电压降不下去
    }
    else if(errorCode == LH_ERR_LIQUID_DETECT_COMM_FAIL)
    {
        return NEEDLE_SAMPLE_ERR_LIQUID_DETECT_CHECK_NOT_EXIST;
    }
    else 
    {
        return NEEDLE_SAMPLE_ERR_LIQUID_DETECT_UNKNOW;
    }
}

//样本针液面探测关闭
uint16_t ServiceActionNeedleSampleLiquidDetectDisable(void)
{
    LH_ERR errorCode = LH_ERR_NONE;
    /*测试模式*/
    errorCode = Can2SubNeedleSampleLiquidDetectDisableFunc();
    if(errorCode != LH_ERR_NONE)
    {
        //打印错误日志
        TaskServiceSerialErrMsgShow(errorCode);
    }

    if(errorCode == LH_ERR_NONE)
    {
        return NEEDLE_SAMPLE_SUCCESS;//校准完成
    }
    else if(errorCode == LH_ERR_LIQUID_DETECT_COMM_FAIL)
    {
        return NEEDLE_SAMPLE_ERR_LIQUID_DETECT_CHECK_NOT_EXIST;//探液板不存在
    }
    else
    {
        return NEEDLE_SAMPLE_ERR_LIQUID_DETECT_UNKNOW;//未知错误
    }
}

//样本针液面探测检测是否存在
uint16_t ServiceActionNeedleSampleLiquidDetectCheckExist(void)
{
    LH_ERR errorCode = LH_ERR_NONE;

    /*测试模式*/
    errorCode = Can2SubNeedleSampleLiquidDetectCheckExist();
    if(errorCode != LH_ERR_NONE)
    {
        //打印错误日志
        TaskServiceSerialErrMsgShow(errorCode);
    }

    if(errorCode == LH_ERR_NONE)
    {
        return NEEDLE_SAMPLE_SUCCESS;//检测成功
    }
    else if(errorCode == LH_ERR_LIQUID_DETECT_COMM_FAIL)
    {
        return NEEDLE_SAMPLE_ERR_LIQUID_DETECT_CHECK_NOT_EXIST;//通讯异常
    }
    else
    {
        return NEEDLE_SAMPLE_ERR_LIQUID_DETECT_UNKNOW;//未知错误
    }
}
#if 0
//样本针液路维护
uint16_t ServiceActionNeedleSampleMainTain(void)
{
    LH_ERR errorCode;
    //打开V309,使用纯水
        errorCode = SystemOutputWrite(VALVE_V309_OUTPUT_LIQUID,Bit_RESET);
        if(errorCode != LH_ERR_NONE)
        {
            //打印错误日志
            TaskServiceSerialErrMsgShow(errorCode);
            //返回错误代码
            return NEEDLE_SAMPLE_ERR_V309_OPEN;
        }

    //内针灌注
        uint16_t actionResult = ServiceActionNeedleSamplePrime(0x00,0x01);
        if(actionResult != NEEDLE_SAMPLE_SUCCESS)
        {
            return actionResult;
        }

    //关闭V309,使用实际清洗液
        errorCode = SystemOutputWrite(VALVE_V309_OUTPUT_LIQUID,Bit_SET);
        if(errorCode != LH_ERR_NONE)
        {
            //打印错误日志
            TaskServiceSerialErrMsgShow(errorCode);
            //返回错误代码
            return NEEDLE_SAMPLE_ERR_V309_CLOSE;
        }

    return NEEDLE_SAMPLE_SUCCESS;
}

#endif


