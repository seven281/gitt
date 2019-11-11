#include "ActionNeedleWash.h"
#include "TSC_TestMode.h"
#define NEEDLE_WASH_PRIME_STEPS   16000
//清洗针初始化
LH_ERR ActionNeedleWash_Init(void)
{
    //清洗针升降和三个泵同时复位
    LH_ERR errorCode;
    int32_t yogi;
    //电机复位结构体
    SM_RESET_CMD resetCommand;
    //结构体初始化
    Can2SubSM_ResetCmdDataStructInit(&resetCommand);

    //复位参数设置
    SystemReadActionParamWithIndex(MODULENEEDLEWASHPARAM,NEEDLERESETCORRECTION,&yogi);
    resetCommand.stepMotorIndex = STEP_MOTOR_NEEDLE_WASH_UPDOWN;
    resetCommand.correctionPosition = yogi;//actionParam->moduleNeedleWashParam.needleResetCorrection;
    resetCommand.timeOutMs = 60000;
    //清洗针升降复位
    errorCode = Can2SubSM_ResetWhileAck(&resetCommand);

    if(errorCode != LH_ERR_NONE)
	{
        //打印错误日志
        TaskServiceSerialErrMsgShow(errorCode);
        //返回错误代码
		return errorCode;
	}

    //复位参数设置
    resetCommand.stepMotorIndex = STEP_MOTOR_PUMP_CLEAN1;
    resetCommand.correctionPosition = 0;
    resetCommand.timeOutMs = 60000;
    //清洗注射泵1
    errorCode = Can2SubSM_ResetWhileAck(&resetCommand);

    if(errorCode != LH_ERR_NONE)
	{
        //打印错误日志
        TaskServiceSerialErrMsgShow(errorCode);
        //返回错误代码
		return errorCode;
	}

    //复位参数设置
    resetCommand.stepMotorIndex = STEP_MOTOR_PUMP_CLEAN2;
    resetCommand.correctionPosition = 0;
    resetCommand.timeOutMs = 60000;
    //清洗注射泵2
    errorCode = Can2SubSM_ResetWhileAck(&resetCommand);

    if(errorCode != LH_ERR_NONE)
	{
        //打印错误日志
        TaskServiceSerialErrMsgShow(errorCode);
        //返回错误代码
		return errorCode;
	}

    //复位参数设置
    resetCommand.stepMotorIndex = STEP_MOTOR_PUMP_CLEAN3;
    resetCommand.correctionPosition = 0;
    resetCommand.timeOutMs = 60000;
    //清洗注射泵3
    errorCode = Can2SubSM_ResetWhileAck(&resetCommand);

    if(errorCode != LH_ERR_NONE)
	{
        //打印错误日志
        TaskServiceSerialErrMsgShow(errorCode);
        //返回错误代码
		return errorCode;
	}

    //等待执行完成
    errorCode = Can2SubSM_WaitLastReturn(STEP_MOTOR_NEEDLE_WASH_UPDOWN,60000);
    if(errorCode != LH_ERR_NONE)
	{
        //打印错误日志
        TaskServiceSerialErrMsgShow(errorCode);
        //返回错误代码
		return errorCode;
	}

    //等待执行完成
    errorCode = Can2SubSM_WaitLastReturn(STEP_MOTOR_PUMP_CLEAN1,60000);
    if(errorCode != LH_ERR_NONE)
	{
        //打印错误日志
        TaskServiceSerialErrMsgShow(errorCode);
        //返回错误代码
		return errorCode;
	}

    //等待执行完成
    errorCode = Can2SubSM_WaitLastReturn(STEP_MOTOR_PUMP_CLEAN2,60000);
    if(errorCode != LH_ERR_NONE)
	{
        //打印错误日志
        TaskServiceSerialErrMsgShow(errorCode);
        //返回错误代码
		return errorCode;
	}

    //等待执行完成
    errorCode = Can2SubSM_WaitLastReturn(STEP_MOTOR_PUMP_CLEAN3,60000);
    if(errorCode != LH_ERR_NONE)
	{
        //打印错误日志
        TaskServiceSerialErrMsgShow(errorCode);
        //返回错误代码
		return errorCode;
	}

    return errorCode;
}
//清洗针灌注老化
LH_ERR ActionNeedleWash_Prime_Debug(void)
{
    int32_t yogi;
    SM_RUN_COORDINATE_CMD runCoordinateCmd;
    LH_ERR errorCode;
    /*1. 清洗针运行到杯底*/
        Can2SubSM_RunCoordinateDataStructInit(&runCoordinateCmd);
        //升降先运行到杯底
        SystemReadActionParamWithIndex(MODULENEEDLEWASHPARAM,NEEDLEBOTTOMPOS,&yogi);
        runCoordinateCmd.stepMotorIndex = STEP_MOTOR_NEEDLE_WASH_UPDOWN;
        runCoordinateCmd.targetCoordinate = yogi;//actionParam->moduleNeedleWashParam.needleBottomPos;
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
    /*2. 延迟20ms*/
        CoreDelayMs(20);  
    /*3. 清洗针升降回0*/
        SM_RETURN_ZERO_CMD returnZeroCmd;
        Can2SubSM_ReturnZeroDataStructInit(&returnZeroCmd);

        //升降回零并修正
        SystemReadActionParamWithIndex(MODULENEEDLEWASHPARAM,NEEDLERESETCORRECTION,&yogi);
        returnZeroCmd.stepMotorIndex = STEP_MOTOR_NEEDLE_WASH_UPDOWN;
        returnZeroCmd.correctionPosition = yogi;//actionParam->moduleNeedleWashParam.needleResetCorrection;
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

//清洗针灌注正常测试
LH_ERR ActionNeedleWash_Prime_Normal(void)
{
    int32_t yogi;
    SM_RUN_COORDINATE_CMD runCoordinateCmd;
    SM_RUN_STEP_CMD runStepCmd;
    LH_ERR errorCode;
    /*1. V301-V303恢复默认状态*/
        errorCode = SystemOutputWrite(NEEDLEWASH_TEE_1,Bit_SET);
        if(errorCode != LH_ERR_NONE)
        {
            //打印错误日志
            TaskServiceSerialErrMsgShow(errorCode);
            //返回错误代码
            return errorCode;
        }
        errorCode = SystemOutputWrite(NEEDLEWASH_TEE_2,Bit_SET);
        if(errorCode != LH_ERR_NONE)
        {
            //打印错误日志
            TaskServiceSerialErrMsgShow(errorCode);
            //返回错误代码
            return errorCode;
        }
        errorCode = SystemOutputWrite(NEEDLEWASH_TEE_3,Bit_SET);
        if(errorCode != LH_ERR_NONE)
        {
            //打印错误日志
            TaskServiceSerialErrMsgShow(errorCode);
            //返回错误代码
            return errorCode;
        }
    /*2. 打开抽液泵*/
       
        errorCode = SystemOutputWrite(WASTE_LIQUID_PUMP2,Bit_SET);
        if(errorCode != LH_ERR_NONE)
        {
            //打印错误日志
            TaskServiceSerialErrMsgShow(errorCode);
            //返回错误代码
            return errorCode;
        }
            
    /*3. 打开隔膜泵DP1*/
        // errorCode = SystemOutputWrite(WASTE_LIQUID_PUMP,Bit_RESET);
        // if(errorCode != LH_ERR_NONE)
        // {
        //     //打印错误日志
        //     TaskServiceSerialErrMsgShow(errorCode);
        //     //返回错误代码
        //     return errorCode;
        // }
    /*4. 清洗针运行到灌注位*/
        Can2SubSM_RunCoordinateDataStructInit(&runCoordinateCmd);
        //升降先运行到杯底
        SystemReadActionParamWithIndex(MODULENEEDLEWASHPARAM,NEEDLEPRIMEPOS,&yogi);
        runCoordinateCmd.stepMotorIndex = STEP_MOTOR_NEEDLE_WASH_UPDOWN;
        runCoordinateCmd.targetCoordinate = yogi;//actionParam->moduleNeedleWashParam.needlePrimePos;
        runCoordinateCmd.curveSelect = CURVE_NEEDLE_WASH_DOWN_2_CUP_TOP;
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
    /*5. 柱塞泵M30-M32运行指定步数16000*/
        //三个泵同时运行到极限,吸液
        runStepCmd.stepMotorIndex = STEP_MOTOR_PUMP_CLEAN1;
        runStepCmd.steps = NEEDLE_WASH_PRIME_STEPS;
        runStepCmd.curveSelect = SM_CURVE_0;
        runStepCmd.speedRatio = 100;
        runStepCmd.speedMode = SPEED_MODE_UP_START;
        runStepCmd.utilStop1Enable = DISABLE;
        runStepCmd.utilStop2Enable = DISABLE;
        runStepCmd.timeOutMs = 60000;

        //电机走坐标
        errorCode = Can2SubSM_RunSpecialStepsWhileAck(&runStepCmd);

        if(errorCode != LH_ERR_NONE)
        {
            //打印错误日志
            TaskServiceSerialErrMsgShow(errorCode);
            //返回错误代码
            return errorCode;
        }

        runStepCmd.stepMotorIndex = STEP_MOTOR_PUMP_CLEAN2;
        runStepCmd.steps = NEEDLE_WASH_PRIME_STEPS;
        runStepCmd.curveSelect = SM_CURVE_0;
        runStepCmd.speedRatio = 100;
        runStepCmd.speedMode = SPEED_MODE_UP_START;
        runStepCmd.utilStop1Enable = DISABLE;
        runStepCmd.utilStop2Enable = DISABLE;
        runStepCmd.timeOutMs = 60000;

        //电机走坐标
        errorCode = Can2SubSM_RunSpecialStepsWhileAck(&runStepCmd);

        if(errorCode != LH_ERR_NONE)
        {
            //打印错误日志
            TaskServiceSerialErrMsgShow(errorCode);
            //返回错误代码
            return errorCode;
        }

        runStepCmd.stepMotorIndex = STEP_MOTOR_PUMP_CLEAN3;
        runStepCmd.steps = NEEDLE_WASH_PRIME_STEPS;
        runStepCmd.curveSelect = SM_CURVE_0;
        runStepCmd.speedRatio = 100;
        runStepCmd.speedMode = SPEED_MODE_UP_START;
        runStepCmd.utilStop1Enable = DISABLE;
        runStepCmd.utilStop2Enable = DISABLE;
        runStepCmd.timeOutMs = 60000;

        //电机走坐标
        errorCode = Can2SubSM_RunSpecialStepsWhileAck(&runStepCmd);

        if(errorCode != LH_ERR_NONE)
        {
            //打印错误日志
            TaskServiceSerialErrMsgShow(errorCode);
            //返回错误代码
            return errorCode;
        }

        //等待执行完成
        errorCode = Can2SubSM_WaitLastReturn(STEP_MOTOR_PUMP_CLEAN1,60000);
        if(errorCode != LH_ERR_NONE)
        {
            //打印错误日志
            TaskServiceSerialErrMsgShow(errorCode);
            //返回错误代码
            return errorCode;
        }

        //等待执行完成
        errorCode = Can2SubSM_WaitLastReturn(STEP_MOTOR_PUMP_CLEAN2,60000);
        if(errorCode != LH_ERR_NONE)
        {
            //打印错误日志
            TaskServiceSerialErrMsgShow(errorCode);
            //返回错误代码
            return errorCode;
        }

        //等待执行完成
        errorCode = Can2SubSM_WaitLastReturn(STEP_MOTOR_PUMP_CLEAN3,60000);
        if(errorCode != LH_ERR_NONE)
        {
            //打印错误日志
            TaskServiceSerialErrMsgShow(errorCode);
            //返回错误代码
            return errorCode;
        }
    /*6. 延迟20ms*/
        CoreDelayMs(20);
    /*7. 切换V301-V303的状态NC*/ 
        errorCode = SystemOutputWrite(NEEDLEWASH_TEE_1,Bit_RESET);
        if(errorCode != LH_ERR_NONE)
        {
            //打印错误日志
            TaskServiceSerialErrMsgShow(errorCode);
            //返回错误代码
            return errorCode;
        }
        errorCode = SystemOutputWrite(NEEDLEWASH_TEE_2,Bit_RESET);
        if(errorCode != LH_ERR_NONE)
        {
            //打印错误日志
            TaskServiceSerialErrMsgShow(errorCode);
            //返回错误代码
            return errorCode;
        }
        errorCode = SystemOutputWrite(NEEDLEWASH_TEE_3,Bit_RESET);
        if(errorCode != LH_ERR_NONE)
        {
            //打印错误日志
            TaskServiceSerialErrMsgShow(errorCode);
            //返回错误代码
            return errorCode;
        }
    /*8. 柱塞泵M30-M32恢复至0*/
        runStepCmd.stepMotorIndex = STEP_MOTOR_PUMP_CLEAN1;
        runStepCmd.steps = 0 - NEEDLE_WASH_PRIME_STEPS;
        runStepCmd.curveSelect = SM_CURVE_0;
        runStepCmd.speedRatio = 100;
        runStepCmd.speedMode = SPEED_MODE_UP_START;
        runStepCmd.utilStop1Enable = DISABLE;
        runStepCmd.utilStop2Enable = DISABLE;
        runStepCmd.timeOutMs = 60000;

        //电机走坐标
        errorCode = Can2SubSM_RunSpecialStepsWhileAck(&runStepCmd);

        if(errorCode != LH_ERR_NONE)
        {
            //打印错误日志
            TaskServiceSerialErrMsgShow(errorCode);
            //返回错误代码
            return errorCode;
        }

        //清洗注射泵2
        runStepCmd.stepMotorIndex = STEP_MOTOR_PUMP_CLEAN2;
        runStepCmd.steps = 0 - NEEDLE_WASH_PRIME_STEPS;
        runStepCmd.curveSelect = SM_CURVE_0;
        runStepCmd.speedRatio = 100;
        runStepCmd.speedMode = SPEED_MODE_UP_START;
        runStepCmd.utilStop1Enable = DISABLE;
        runStepCmd.utilStop2Enable = DISABLE;
        runStepCmd.timeOutMs = 60000;

        //电机走坐标
        errorCode = Can2SubSM_RunSpecialStepsWhileAck(&runStepCmd);

        if(errorCode != LH_ERR_NONE)
        {
            //打印错误日志
            TaskServiceSerialErrMsgShow(errorCode);
            //返回错误代码
            return errorCode;
        }

        //清洗注射泵3
        runStepCmd.stepMotorIndex = STEP_MOTOR_PUMP_CLEAN3;
        runStepCmd.steps = 0 - NEEDLE_WASH_PRIME_STEPS;
        runStepCmd.curveSelect = SM_CURVE_0;
        runStepCmd.speedRatio = 100;
        runStepCmd.speedMode = SPEED_MODE_UP_START;
        runStepCmd.utilStop1Enable = DISABLE;
        runStepCmd.utilStop2Enable = DISABLE;
        runStepCmd.timeOutMs = 60000;

        //电机走坐标
        errorCode = Can2SubSM_RunSpecialStepsWhileAck(&runStepCmd);

        if(errorCode != LH_ERR_NONE)
        {
            //打印错误日志
            TaskServiceSerialErrMsgShow(errorCode);
            //返回错误代码
            return errorCode;
        }

        //等待执行完成
        errorCode = Can2SubSM_WaitLastReturn(STEP_MOTOR_PUMP_CLEAN1,60000);
        if(errorCode != LH_ERR_NONE)
        {
            //打印错误日志
            TaskServiceSerialErrMsgShow(errorCode);
            //返回错误代码
            return errorCode;
        }

        //等待执行完成
        errorCode = Can2SubSM_WaitLastReturn(STEP_MOTOR_PUMP_CLEAN2,60000);
        if(errorCode != LH_ERR_NONE)
        {
            //打印错误日志
            TaskServiceSerialErrMsgShow(errorCode);
            //返回错误代码
            return errorCode;
        }

        //等待执行完成
        errorCode = Can2SubSM_WaitLastReturn(STEP_MOTOR_PUMP_CLEAN3,60000);
        if(errorCode != LH_ERR_NONE)
        {
            //打印错误日志
            TaskServiceSerialErrMsgShow(errorCode);
            //返回错误代码
            return errorCode;
        }
    /*9. 延迟20ms*/
        CoreDelayMs(20);
    /*10. 切换V301-V303至NO默认状态*/
        errorCode = SystemOutputWrite(NEEDLEWASH_TEE_1,Bit_SET);
        if(errorCode != LH_ERR_NONE)
        {
            //打印错误日志
            TaskServiceSerialErrMsgShow(errorCode);
            //返回错误代码
            return errorCode;
        }
        errorCode = SystemOutputWrite(NEEDLEWASH_TEE_2,Bit_SET);
        if(errorCode != LH_ERR_NONE)
        {
            //打印错误日志
            TaskServiceSerialErrMsgShow(errorCode);
            //返回错误代码
            return errorCode;
        }
        errorCode = SystemOutputWrite(NEEDLEWASH_TEE_3,Bit_SET);
        if(errorCode != LH_ERR_NONE)
        {
            //打印错误日志
            TaskServiceSerialErrMsgShow(errorCode);
            //返回错误代码
            return errorCode;
        }
    /*11. 判断灌注是否完成*/
    /*12. 清洗针升降回0*/
        SM_RETURN_ZERO_CMD returnZeroCmd;
        Can2SubSM_ReturnZeroDataStructInit(&returnZeroCmd);

        //升降回零并修正
        SystemReadActionParamWithIndex(MODULENEEDLEWASHPARAM,NEEDLERESETCORRECTION,&yogi);
        returnZeroCmd.stepMotorIndex = STEP_MOTOR_NEEDLE_WASH_UPDOWN;
        returnZeroCmd.correctionPosition = yogi;//actionParam->moduleNeedleWashParam.needleResetCorrection;
        returnZeroCmd.curveSelect = CURVE_NEEDLE_WASH_DOWN_2_CUP_TOP;
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
    /*13. 关闭隔膜泵*/ 
        // errorCode = SystemOutputWrite(WASTE_LIQUID_PUMP,Bit_SET);
        // if(errorCode != LH_ERR_NONE)
        // {
        //     //打印错误日志
        //     TaskServiceSerialErrMsgShow(errorCode);
        //     //返回错误代码
        //     return errorCode;
        // } 
    /*14. 关闭抽液泵dp2*/
        errorCode = SystemOutputWrite(WASTE_LIQUID_PUMP2,Bit_RESET);
        if(errorCode != LH_ERR_NONE)
        {
            //打印错误日志
            TaskServiceSerialErrMsgShow(errorCode);
            //返回错误代码
            return errorCode;
        } 
    return errorCode;
}
//清洗针灌注
LH_ERR ActionNeedleWash_Prime(void)
{
    LH_ERR errorCode;
    
    TSC_TEST_MODE_SELECTION mode;
    mode = TSC_Read_TestMode();

    if(mode == TSC_TEST_MODE_NORMAL)
    {
        errorCode = ActionNeedleWash_Prime_Normal();//正常测试
    }
    else 
    {
        errorCode = ActionNeedleWash_Prime_Debug();//老化测试
    }
    return errorCode;
}

//清洗针清洗正常
LH_ERR ActionNeedleWash_Clean_Normal(CLEAN_NEEDLE_CMD_DATA clean)
{
    int32_t yogi;
    SM_RUN_ALWAYS_CMD runAlwaysCmd;
    LH_ERR errorCode;
    SM_RUN_COORDINATE_CMD runCoordinateCmd;
    SM_RETURN_ZERO_CMD returnZeroCmd;
    /*1. 抽液蠕动泵开始运行*/
        Can2SubSM_RunAlwaysDataStructInit(&runAlwaysCmd);

        //配置蠕动泵参数
        runAlwaysCmd.stepMotorIndex = STEP_MOTOR_PUMP_WASTE;
        runAlwaysCmd.curveSelect = SM_CURVE_0;
        runAlwaysCmd.speedRatio = 100;
        runAlwaysCmd.speedMode = SPEED_MODE_UP_START;
        //抽废液蠕动泵开始运转
        errorCode = Can2SubSM_RunAlwaysWhileAck(&runAlwaysCmd);
        if(errorCode != LH_ERR_NONE)
        {
            //打印错误日志
            TaskServiceSerialErrMsgShow(errorCode);
            //返回错误代码
            return errorCode;
        }
    /*2. 阀v301-v303切换到NO初始状态*/
        errorCode = SystemOutputWrite(NEEDLEWASH_TEE_1,Bit_SET);
        if(errorCode != LH_ERR_NONE)
        {
            //打印错误日志
            TaskServiceSerialErrMsgShow(errorCode);
            //返回错误代码
            return errorCode;
        }
        errorCode = SystemOutputWrite(NEEDLEWASH_TEE_2,Bit_SET);
        if(errorCode != LH_ERR_NONE)
        {
            //打印错误日志
            TaskServiceSerialErrMsgShow(errorCode);
            //返回错误代码
            return errorCode;
        }
        errorCode = SystemOutputWrite(NEEDLEWASH_TEE_3,Bit_SET);
        if(errorCode != LH_ERR_NONE)
        {
            //打印错误日志
            TaskServiceSerialErrMsgShow(errorCode);
            //返回错误代码
            return errorCode;
        }
    /*3. 柱塞泵m30-m32运行至低位*/

        Can2SubSM_RunCoordinateDataStructInit(&runCoordinateCmd);

        //清洗针1注液
        if(clean.needle.able == ENABLE)
        {
            runCoordinateCmd.stepMotorIndex = STEP_MOTOR_PUMP_CLEAN1;
            if(clean.needle.index == CLEAN_NEEDLE_INDEX_FIRST)//第一次注液
                runCoordinateCmd.targetCoordinate = actionParam->moduleNeedleWashParam.pump1InjectPos;
            else//第二次注液
                runCoordinateCmd.targetCoordinate = actionParam->moduleNeedleWashParam.pump1InjectPos1;
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

        //清洗针2注液
        if(clean.needle1.able == ENABLE)
        {
            runCoordinateCmd.stepMotorIndex = STEP_MOTOR_PUMP_CLEAN2;
            if(clean.needle1.index == CLEAN_NEEDLE_INDEX_FIRST)//第一次注液
                 runCoordinateCmd.targetCoordinate = actionParam->moduleNeedleWashParam.pump2InjectPos;
            else//第二次注液
                runCoordinateCmd.targetCoordinate = actionParam->moduleNeedleWashParam.pump2InjectPos1;
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

        //清洗针3注液
        if(clean.needle2.able ==ENABLE)
        {
            runCoordinateCmd.stepMotorIndex = STEP_MOTOR_PUMP_CLEAN3;
            runCoordinateCmd.targetCoordinate = actionParam->moduleNeedleWashParam.pump3InjectPos;
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
    /*4. 清洗针快速下降到杯口*/
        //升降运行到杯口,快速
        SystemReadActionParamWithIndex(MODULENEEDLEWASHPARAM,NEEDLECUPTOPPOS,&yogi);
        runCoordinateCmd.stepMotorIndex = STEP_MOTOR_NEEDLE_WASH_UPDOWN;
        runCoordinateCmd.targetCoordinate = yogi;//actionParam->moduleNeedleWashParam.needleCupTopPos;
        runCoordinateCmd.curveSelect = CURVE_NEEDLE_WASH_DOWN_2_CUP_TOP;
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
    /*5. 清洗针慢速下降到杯底，清洗针顶起2mm*/
        //升降运行到杯底,慢速
        SystemReadActionParamWithIndex(MODULENEEDLEWASHPARAM,NEEDLEBOTTOMPOS,&yogi);
        runCoordinateCmd.stepMotorIndex = STEP_MOTOR_NEEDLE_WASH_UPDOWN;
        runCoordinateCmd.targetCoordinate = yogi;//actionParam->moduleNeedleWashParam.needleBottomPos;
        runCoordinateCmd.curveSelect = CURVE_NEEDLE_WASH_DOWN_2_CUP_BUTTOM;
        runCoordinateCmd.speedRatio = 80;
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
    /*6. 延迟100ms*/
    	//在杯底抽废液
	    CoreDelayMs(actionParam->moduleNeedleWashParam.timeNeedleAbsorbEffluentMs);
    /*7. 等待柱塞泵运行至低位*/
        //等待三个注射泵运行完成
        if(clean.needle.able == ENABLE)
        {
            errorCode = Can2SubSM_WaitLastReturn(STEP_MOTOR_PUMP_CLEAN1,60000);

            if(errorCode != LH_ERR_NONE)
            {
                //打印错误日志
                TaskServiceSerialErrMsgShow(errorCode);
                //返回错误代码
                return errorCode;
            }
        }

        if(clean.needle1.able == ENABLE)
        {
            errorCode = Can2SubSM_WaitLastReturn(STEP_MOTOR_PUMP_CLEAN2,60000);

            if(errorCode != LH_ERR_NONE)
            {
                //打印错误日志
                TaskServiceSerialErrMsgShow(errorCode);
                //返回错误代码
                return errorCode;
            }
        }

        if(clean.needle2.able == ENABLE)
        {
            errorCode = Can2SubSM_WaitLastReturn(STEP_MOTOR_PUMP_CLEAN3,60000);

            if(errorCode != LH_ERR_NONE)
            {
                //打印错误日志
                TaskServiceSerialErrMsgShow(errorCode);
                //返回错误代码
                return errorCode;
            }
        } 
    /*8. 关闭蠕动泵*/
        //停止抽废液
        errorCode = Can2SubSM_StopGraduallyWhileAck(STEP_MOTOR_PUMP_WASTE);
        
        if(errorCode != LH_ERR_NONE)
        {
            //打印错误日志
            TaskServiceSerialErrMsgShow(errorCode);
            //返回错误代码
            return errorCode;
        }
    /*9. 阀v301-v303切换至nc状态*/
        errorCode = SystemOutputWrite(NEEDLEWASH_TEE_1,Bit_RESET);
        if(errorCode != LH_ERR_NONE)
        {
            //打印错误日志
            TaskServiceSerialErrMsgShow(errorCode);
            //返回错误代码
            return errorCode;
        }
        errorCode = SystemOutputWrite(NEEDLEWASH_TEE_2,Bit_RESET);
        if(errorCode != LH_ERR_NONE)
        {
            //打印错误日志
            TaskServiceSerialErrMsgShow(errorCode);
            //返回错误代码
            return errorCode;
        }
        errorCode = SystemOutputWrite(NEEDLEWASH_TEE_3,Bit_RESET);
        if(errorCode != LH_ERR_NONE)
        {
            //打印错误日志
            TaskServiceSerialErrMsgShow(errorCode);
            //返回错误代码
            return errorCode;
        }        
    /*10. 柱塞泵m30-m32运行至零位*/
        //根据flag,选择指定的泵开始打液
        
        Can2SubSM_ReturnZeroDataStructInit(&returnZeroCmd);

        if(clean.needle.able == ENABLE)
        {
            returnZeroCmd.stepMotorIndex = STEP_MOTOR_PUMP_CLEAN1;
            returnZeroCmd.correctionPosition = 0;
            returnZeroCmd.curveSelect = SM_CURVE_0;
            returnZeroCmd.speedRatio = 100;
            returnZeroCmd.speedMode = SPEED_MODE_UP_START;
            returnZeroCmd.utilStop1Enable = DISABLE;
            returnZeroCmd.utilStop2Enable = DISABLE;
            returnZeroCmd.timeOutMs = 60000;

            //回零
            errorCode = Can2SubSM_ReturnZeroWhileAck(&returnZeroCmd);

            if(errorCode != LH_ERR_NONE)
            {
                //打印错误日志
                TaskServiceSerialErrMsgShow(errorCode);
                //返回错误代码
                return errorCode;
            }
        }

        if(clean.needle1.able == ENABLE)
        {
            returnZeroCmd.stepMotorIndex = STEP_MOTOR_PUMP_CLEAN2;
            returnZeroCmd.correctionPosition = 0;
            returnZeroCmd.curveSelect = SM_CURVE_0;
            returnZeroCmd.speedRatio = 100;
            returnZeroCmd.speedMode = SPEED_MODE_UP_START;
            returnZeroCmd.utilStop1Enable = DISABLE;
            returnZeroCmd.utilStop2Enable = DISABLE;
            returnZeroCmd.timeOutMs = 60000;

            //回零
            errorCode = Can2SubSM_ReturnZeroWhileAck(&returnZeroCmd);

            if(errorCode != LH_ERR_NONE)
            {
                //打印错误日志
                TaskServiceSerialErrMsgShow(errorCode);
                //返回错误代码
                return errorCode;
            }
        }

        if(clean.needle2.able == ENABLE)
        {
            returnZeroCmd.stepMotorIndex = STEP_MOTOR_PUMP_CLEAN3;
            returnZeroCmd.correctionPosition = 0;
            returnZeroCmd.curveSelect = SM_CURVE_0;
            returnZeroCmd.speedRatio = 100;
            returnZeroCmd.speedMode = SPEED_MODE_UP_START;
            returnZeroCmd.utilStop1Enable = DISABLE;
            returnZeroCmd.utilStop2Enable = DISABLE;
            returnZeroCmd.timeOutMs = 60000;

            //回零
            errorCode = Can2SubSM_ReturnZeroWhileAck(&returnZeroCmd);

            if(errorCode != LH_ERR_NONE)
            {
                //打印错误日志
                TaskServiceSerialErrMsgShow(errorCode);
                //返回错误代码
                return errorCode;
            }
        }
    /*11. 清洗针慢速上升,同时需等待三个柱塞泵完成*/
        //清洗针慢速抬起到杯口
        SystemReadActionParamWithIndex(MODULENEEDLEWASHPARAM,NEEDLECUPTOPPOS,&yogi);
        runCoordinateCmd.stepMotorIndex = STEP_MOTOR_NEEDLE_WASH_UPDOWN;
        runCoordinateCmd.targetCoordinate = yogi;//actionParam->moduleNeedleWashParam.needleCupTopPos;
        runCoordinateCmd.curveSelect = CURVE_NEEDLE_WASH_UP_2_CUP_TOP;
        runCoordinateCmd.speedRatio = 80;
        runCoordinateCmd.speedMode = SPEED_MODE_UP_START;
        runCoordinateCmd.utilStop1Enable = DISABLE;
        runCoordinateCmd.utilStop2Enable = DISABLE;
        runCoordinateCmd.timeOutMs = 60000;

        //1电机走坐标
        errorCode = Can2SubSM_RunToCoordinateWhileReturn(&runCoordinateCmd);

        if(errorCode != LH_ERR_NONE)
        {
            //打印错误日志
            TaskServiceSerialErrMsgShow(errorCode);
            //返回错误代码
            return errorCode;
        }
        //2等待柱塞泵注液全部完毕
        if(clean.needle.able == ENABLE)
        {
            errorCode = Can2SubSM_WaitLastReturn(STEP_MOTOR_PUMP_CLEAN1,60000);
            if(errorCode != LH_ERR_NONE)
            {
                //打印错误日志
                TaskServiceSerialErrMsgShow(errorCode);
                //返回错误代码
                return errorCode;
            }
        }
        if(clean.needle1.able == ENABLE)
        {
            errorCode = Can2SubSM_WaitLastReturn(STEP_MOTOR_PUMP_CLEAN2,60000);
            if(errorCode != LH_ERR_NONE)
            {
                //打印错误日志
                TaskServiceSerialErrMsgShow(errorCode);
                //返回错误代码
                return errorCode;
            }
        }
        if(clean.needle2.able == ENABLE)
        {
            errorCode = Can2SubSM_WaitLastReturn(STEP_MOTOR_PUMP_CLEAN3,60000);
            if(errorCode != LH_ERR_NONE)
            {
                //打印错误日志
                TaskServiceSerialErrMsgShow(errorCode);
                //返回错误代码
                return errorCode;
            }
        }



        #if 0
    /*12. 再次开启抽液蠕动泵*/
        Can2SubSM_RunAlwaysDataStructInit(&runAlwaysCmd);

        //配置蠕动泵参数
        runAlwaysCmd.stepMotorIndex = STEP_MOTOR_PUMP_WASTE;
        runAlwaysCmd.curveSelect = SM_CURVE_0;
        runAlwaysCmd.speedRatio = 100;
        runAlwaysCmd.speedMode = SPEED_MODE_UP_START;

        //抽废液蠕动泵开始运转
        errorCode = Can2SubSM_RunAlwaysWhileReturn(&runAlwaysCmd);
        if(errorCode != LH_ERR_NONE)
        {
            //打印错误日志
            TaskServiceSerialErrMsgShow(errorCode);
            //返回错误代码
            return errorCode;
        }
    /*13. 延迟100ms*/
        CoreDelayMs(actionParam->moduleNeedleWashParam.timeNeedleAbsorbEffluentMs);
     
    /*14. 关闭抽液蠕动泵*/
        //停止抽废液
        errorCode = Can2SubSM_StopGraduallyWhileReturn(STEP_MOTOR_PUMP_WASTE);
        
        if(errorCode != LH_ERR_NONE)
        {
            //打印错误日志
            TaskServiceSerialErrMsgShow(errorCode);
            //返回错误代码
            return errorCode;
        }
     #endif   
    
    /*15. 三通阀v301-v303恢复到初始状态*/   
        errorCode = SystemOutputWrite(NEEDLEWASH_TEE_1,Bit_SET);
        if(errorCode != LH_ERR_NONE)
        {
            //打印错误日志
            TaskServiceSerialErrMsgShow(errorCode);
            //返回错误代码
            return errorCode;
        }
        errorCode = SystemOutputWrite(NEEDLEWASH_TEE_2,Bit_SET);
        if(errorCode != LH_ERR_NONE)
        {
            //打印错误日志
            TaskServiceSerialErrMsgShow(errorCode);
            //返回错误代码
            return errorCode;
        }
        errorCode = SystemOutputWrite(NEEDLEWASH_TEE_3,Bit_SET);
        if(errorCode != LH_ERR_NONE)
        {
            //打印错误日志
            TaskServiceSerialErrMsgShow(errorCode);
            //返回错误代码
            return errorCode;
        } 
    
    /*16. 清洗针快速上升至零位*/
        //清洗针回零
        SystemReadActionParamWithIndex(MODULENEEDLEWASHPARAM,NEEDLERESETCORRECTION,&yogi);
        returnZeroCmd.stepMotorIndex = STEP_MOTOR_NEEDLE_WASH_UPDOWN;
        returnZeroCmd.correctionPosition = yogi;//actionParam->moduleNeedleWashParam.needleResetCorrection;
        returnZeroCmd.curveSelect = CURVE_NEEDLE_WASH_UP_2_TOP;
        returnZeroCmd.speedRatio = 100;
        returnZeroCmd.speedMode = SPEED_MODE_UP_START;
        returnZeroCmd.utilStop1Enable = DISABLE;
        returnZeroCmd.utilStop2Enable = DISABLE;
        returnZeroCmd.timeOutMs = 60000;

        //回零
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


//清洗正清洗老化
LH_ERR ActionNeedleWash_Clean_Debug(CLEAN_NEEDLE_CMD_DATA clean)
{
//    SM_RUN_ALWAYS_CMD runAlwaysCmd;
    int32_t yogi;
    LH_ERR errorCode;
    SM_RUN_COORDINATE_CMD runCoordinateCmd;
    SM_RETURN_ZERO_CMD returnZeroCmd;
    /*1. 清洗针快速下降到杯口*/
        //升降运行到杯口,快速
        SystemReadActionParamWithIndex(MODULENEEDLEWASHPARAM,NEEDLECUPTOPPOS,&yogi);
        runCoordinateCmd.stepMotorIndex = STEP_MOTOR_NEEDLE_WASH_UPDOWN;
        runCoordinateCmd.targetCoordinate = yogi;//actionParam->moduleNeedleWashParam.needleCupTopPos;
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
    /*2. 清洗针慢速下降到杯底，清洗针顶起2mm*/
        //升降运行到杯底,慢速
        SystemReadActionParamWithIndex(MODULENEEDLEWASHPARAM,NEEDLEBOTTOMPOS,&yogi);
        runCoordinateCmd.stepMotorIndex = STEP_MOTOR_NEEDLE_WASH_UPDOWN;
        runCoordinateCmd.targetCoordinate = yogi;//actionParam->moduleNeedleWashParam.needleBottomPos;
        runCoordinateCmd.curveSelect = SM_CURVE_0;
        runCoordinateCmd.speedRatio = 80;
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
    /*3. 延迟100ms*/
    	//在杯底抽废液
	    CoreDelayMs(actionParam->moduleNeedleWashParam.timeNeedleAbsorbEffluentMs);
    /*4. 清洗针慢速上升*/
        //清洗针慢速抬起到杯口
        SystemReadActionParamWithIndex(MODULENEEDLEWASHPARAM,NEEDLECUPTOPPOS,&yogi);
        runCoordinateCmd.stepMotorIndex = STEP_MOTOR_NEEDLE_WASH_UPDOWN;
        runCoordinateCmd.targetCoordinate = yogi;//actionParam->moduleNeedleWashParam.needleCupTopPos;
        runCoordinateCmd.curveSelect = SM_CURVE_0;
        runCoordinateCmd.speedRatio = 80;
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
    /*5. 延迟100ms*/
        CoreDelayMs(actionParam->moduleNeedleWashParam.timeNeedleAbsorbEffluentMs);
    /*6. 清洗针快速上升至零位*/
        //清洗针回零
        SystemReadActionParamWithIndex(MODULENEEDLEWASHPARAM,NEEDLERESETCORRECTION,&yogi);
        returnZeroCmd.stepMotorIndex = STEP_MOTOR_NEEDLE_WASH_UPDOWN;
        returnZeroCmd.correctionPosition = yogi;//actionParam->moduleNeedleWashParam.needleResetCorrection;
        returnZeroCmd.curveSelect = SM_CURVE_0;
        returnZeroCmd.speedRatio = 100;
        returnZeroCmd.speedMode = SPEED_MODE_UP_START;
        returnZeroCmd.utilStop1Enable = DISABLE;
        returnZeroCmd.utilStop2Enable = DISABLE;
        returnZeroCmd.timeOutMs = 60000;

        //回零
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


//清洗针清洗返回
LH_ERR ActionNeedleWash_Clean(CLEAN_NEEDLE_CMD_DATA clean_needle)
{
    LH_ERR errorCode;

    TSC_TEST_MODE_SELECTION mode;
    mode = TSC_Read_TestMode();

    if(mode == TSC_TEST_MODE_NORMAL)
    {
        errorCode = ActionNeedleWash_Clean_Normal(clean_needle);//正常测试
    }
    else 
    {
        errorCode = ActionNeedleWash_Clean_Debug(clean_needle);//老化测试
    }   
    return errorCode;
}








