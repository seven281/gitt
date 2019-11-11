#include "ActionLiquidInjectA.h"
#include "TSC_TestMode.h"
typedef enum LIQUIDS_STATE
{
    EMPTY,
    HAVE,
}LIQUIDS_STATE;

//A液初始化
LH_ERR ActionLiquidA_InjectInit(void)
{
    LH_ERR errorCode;
    int32_t yogi;
    //电机复位结构体
    SM_RESET_CMD resetCommand;
    //结构体初始化
    Can2SubSM_ResetCmdDataStructInit(&resetCommand);

    //复位参数设置
    SystemReadActionParamWithIndex(MODULEDISHWASHLIQUIDAPARAM,PUMPLIQUIDARESETCORRECTION,&yogi);
    resetCommand.stepMotorIndex = STEP_MOTOR_PUMP_LIQUID_A;
    resetCommand.correctionPosition = yogi;//actionParam->moduleDishWashLiquidAParam.pumpLiquidAResetCorrection;
    resetCommand.timeOutMs = 60000;
    //注A液泵复位
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


//A液注液---------------正常模式
LH_ERR ActionLiquidA_InjectTrigOnce_Normal(void)
{
    LH_ERR errorCode;
    SM_RUN_STEP_CMD runStepCommand;
    int32_t yogi;
    //电机复位结构体
    SM_RESET_CMD resetCommand;
    LIQUIDS_STATE A1bottle_liquids,A2bottle_liquids;
    /*1. 判断哪个瓶子有液体*/
        //  SystemInputRead();传感器没有安装也不清除连接哪个端口
        A1bottle_liquids = HAVE;
    /*2.1. 打开阀v201   前提:A1瓶有液体   
      2.2. 打开阀v202   前提:A2瓶有液体*/
        if(A1bottle_liquids == HAVE)
        {
            errorCode = SystemOutputWrite(DRAINAGE_VALVE_V201,Bit_RESET);
            if(errorCode != LH_ERR_NONE)
            {
                //打印错误日志
                TaskServiceSerialErrMsgShow(errorCode);
                //返回错误代码
                return errorCode;
            }
        }
        else if(A2bottle_liquids == HAVE)
        {
            errorCode = SystemOutputWrite(DRAINAGE_VALVE_V202,Bit_RESET);
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
        
    /*3. 点滴泵m39运行指定步数*/
        Can2SubSM_RunStepsCmdDataStructInit(&runStepCommand);
        SystemReadActionParamWithIndex(MODULEDISHWASHLIQUIDAPARAM,PUMPINJECTASTEP,&yogi);
        runStepCommand.stepMotorIndex = STEP_MOTOR_PUMP_LIQUID_A;
        runStepCommand.steps = yogi;//actionParam->moduleDishWashLiquidAParam.pumpInjectAStep;
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
    /*4. 点滴泵M39复位*/
        //结构体初始化
        Can2SubSM_ResetCmdDataStructInit(&resetCommand);

        //复位参数设置
        SystemReadActionParamWithIndex(MODULEDISHWASHLIQUIDAPARAM,PUMPLIQUIDARESETCORRECTION,&yogi);
        resetCommand.stepMotorIndex = STEP_MOTOR_PUMP_LIQUID_A;
        resetCommand.correctionPosition = yogi;//actionParam->moduleDishWashLiquidAParam.pumpLiquidAResetCorrection;
        resetCommand.timeOutMs = 60000;
        //A液泵复位
        errorCode = Can2SubSM_ResetWhileReturn(&resetCommand);
        if(errorCode != LH_ERR_NONE)
        {
            //打印错误日志
            TaskServiceSerialErrMsgShow(errorCode);
            //返回错误代码
            return errorCode;
        }
    /*5. 关闭阀v201 v202*/
        errorCode = SystemOutputWrite(DRAINAGE_VALVE_V201,Bit_SET);
        if(errorCode != LH_ERR_NONE)
        {
            //打印错误日志
            TaskServiceSerialErrMsgShow(errorCode);
            //返回错误代码
            return errorCode;
        }

        errorCode = SystemOutputWrite(DRAINAGE_VALVE_V202,Bit_SET);
        if(errorCode != LH_ERR_NONE)
        {
            //打印错误日志
            TaskServiceSerialErrMsgShow(errorCode);
            //返回错误代码
            return errorCode;
        }

    
    
    

	return errorCode;
}
//A液注液---------------老化模式
LH_ERR ActionLiquidA_InjectTrigOnce_Debug(void)
{
    LH_ERR errorCode = LH_ERR_NONE;

	return errorCode;
}
//A液注液
LH_ERR ActionLiquidA_InjectTrigOnce(void)
{
    LH_ERR errorCode;

    TSC_TEST_MODE_SELECTION mode;
    mode = TSC_Read_TestMode();

    if(mode == TSC_TEST_MODE_NORMAL)
    {
        errorCode = ActionLiquidA_InjectTrigOnce_Normal();//正常测试
    }
    else 
    {
        errorCode = ActionLiquidA_InjectTrigOnce_Debug();//老化测试
    }

    return errorCode;
}


//A液灌注--------老化
LH_ERR ActionLiquidA_InjectPrimeOnce_Debug(void)
{
    return LH_ERR_NONE;
}
//A液灌注--------正常模式
LH_ERR ActionLiquidA_InjectPrimeOnce_Normal(void)
{
     LH_ERR errorCode;
     int32_t yogi;
    /*1. 判断两个瓶是否有液并开对应的阀门*/
        //SystemInputRead()硬件暂时不支持检测液体
        if(1)//A1有液体
        {
            errorCode = SystemOutputWrite(DRAINAGE_VALVE_V201,Bit_RESET);
            if(errorCode != LH_ERR_NONE)
            {
                //打印错误日志
                TaskServiceSerialErrMsgShow(errorCode);
                //返回错误代码
                return errorCode;
            }
        }
        if(1)//A2有液体
        {
            errorCode = SystemOutputWrite(DRAINAGE_VALVE_V202,Bit_RESET);
            if(errorCode != LH_ERR_NONE)
            {
                //打印错误日志
                TaskServiceSerialErrMsgShow(errorCode);
                //返回错误代码
                return errorCode;
            }
        }
    /*2. 打开抽液泵DP2*/
        errorCode = SystemOutputWrite(WASTE_LIQUID_PUMP2,Bit_SET);
        if(errorCode != LH_ERR_NONE)
        {
            //打印错误日志
            TaskServiceSerialErrMsgShow(errorCode);
            //返回错误代码
            return errorCode;
        }
    /*3. 打开抽液泵dp1
        errorCode = SystemOutputWrite(WASTE_LIQUID_PUMP,Bit_RESET);
        if(errorCode != LH_ERR_NONE)
        {
            //打印错误日志
            TaskServiceSerialErrMsgShow(errorCode);
            //返回错误代码
            return errorCode;
        }
    */
    
    /*4. m39运行指定的步数*/
        SM_RUN_STEP_CMD runStepCommand;
        //走位结构体初始化
        Can2SubSM_RunStepsCmdDataStructInit(&runStepCommand);

        //A液泵注液一次
        SystemReadActionParamWithIndex(MODULEDISHWASHLIQUIDAPARAM,PUMPINJECTASTEP,&yogi);
        runStepCommand.stepMotorIndex = STEP_MOTOR_PUMP_LIQUID_A;
        runStepCommand.steps = yogi;//actionParam->moduleDishWashLiquidAParam.pumpInjectAStep;
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
    /*5. 关闭阀门v201/v202*/
        errorCode = SystemOutputWrite(DRAINAGE_VALVE_V201,Bit_SET);
        if(errorCode != LH_ERR_NONE)
        {
            //打印错误日志
            TaskServiceSerialErrMsgShow(errorCode);
            //返回错误代码
            return errorCode;
        }
        errorCode = SystemOutputWrite(DRAINAGE_VALVE_V202,Bit_SET);
        if(errorCode != LH_ERR_NONE)
        {
            //打印错误日志
            TaskServiceSerialErrMsgShow(errorCode);
            //返回错误代码
            return errorCode;
        }
    /*6. 关闭抽液泵dp1
        errorCode = SystemOutputWrite(WASTE_LIQUID_PUMP,Bit_SET);
        if(errorCode != LH_ERR_NONE)
        {
            //打印错误日志
            TaskServiceSerialErrMsgShow(errorCode);
            //返回错误代码
            return errorCode;
        }
    */
    /*7. 关闭抽液泵DP2*/
        errorCode = SystemOutputWrite(WASTE_LIQUID_PUMP2,Bit_RESET);
        if(errorCode != LH_ERR_NONE)
        {
            //打印错误日志
            TaskServiceSerialErrMsgShow(errorCode);
            //返回错误代码
            return errorCode;
        }
    /*8. m39柱塞泵复位*/

         //电机复位结构体
        SM_RESET_CMD resetCommand;
        //结构体初始化
        Can2SubSM_ResetCmdDataStructInit(&resetCommand);

        //复位参数设置
        SystemReadActionParamWithIndex(MODULEDISHWASHLIQUIDAPARAM,PUMPLIQUIDARESETCORRECTION,&yogi);
        resetCommand.stepMotorIndex = STEP_MOTOR_PUMP_LIQUID_A;
        resetCommand.correctionPosition = yogi;//actionParam->moduleDishWashLiquidAParam.pumpLiquidAResetCorrection;
        resetCommand.timeOutMs = 60000;
        //A液泵复位
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
//A液灌注
LH_ERR ActionLiquidA_InjectPrimeOnce(void)
{
    LH_ERR errorCode;

    TSC_TEST_MODE_SELECTION mode;
    mode = TSC_Read_TestMode();

    if(mode == TSC_TEST_MODE_NORMAL)
    {
        errorCode = ActionLiquidA_InjectPrimeOnce_Normal();//正常测试
    }
    else 
    {
        errorCode = ActionLiquidA_InjectPrimeOnce_Debug();//老化测试
    }

    return errorCode;
}







