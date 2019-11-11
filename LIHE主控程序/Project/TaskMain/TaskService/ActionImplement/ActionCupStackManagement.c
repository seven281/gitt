#include "ActionCupStackManagement.h"

//定义传感器的有效状态
//新杯栈检测存在传感器的触发电平
#define SENSOR_VALID_STATUS_CUP_MANAGE_NEW_STACK_CHECK             Bit_SET

//空杯栈检测存在的传感器的触发电平
#define SENSOR_VALID_STATUS_CUP_MANAGE_EMPTY_STACK_CHECK           Bit_SET

//空杯栈上方正在测试区的传感器的触发电平
#define SENSOR_VALID_STATUS_CUP_MANAGE_TESTING_DISK_CHECK          Bit_RESET

//新杯栈最大存放数量
#define SYSTEM_NEW_STACK_MAX_DISK_COUNT           10

//空杯栈最大存放数量
#define SYSTEM_EMPTY_STACK_MAX_DISK_COUNT         9


//MACHINE_DOOR_STATUS StackManageDoorStatus;


//当前系统内新杯数量
//static uint8_t currentSystemNewStackDiskCount = 0;

//当前系统内空杯数量
//static uint8_t currentSystemEmptyStackDiskCount = 0;

//按键状态计数值

//新杯装载机构初始化,返回新杯装载机构顶部缓存区是否有杯和新杯盘数量
//初始化时,新杯装载机构顶部缓存区不能有杯,如果有,将报错
LH_ERR ActionCupStackManagement_NewStackInit(ACTION_SENSOR_STATUS* newStackTopHasDisk,uint8_t* newStackDiskCount)
{
    LH_ERR errorCode;
    int32_t resetcorrection,newstackoffset;
    uint8_t currentindex;
	//杯盘检测传感器状态
	BitAction newStackDiskTopSensorValue;
	//设定新杯盘数量为0
    TaskUtilServiceSetCupStackManagementNewStackCount(0);

    *newStackTopHasDisk = ACTION_SENSOR_STATUS_NOT_EXIST;
	*newStackDiskCount = TaskUtilServiceGetCupStackManagementNewStackCount();

    //电机复位结构体
    SM_RESET_CMD resetCommand;
    //结构体初始化
    Can2SubSM_ResetCmdDataStructInit(&resetCommand);

    //复位参数设置
    SystemReadActionParamWithIndex(MODULECUPMANAGEPARAM,NEWSTACKRESETCORRECTION,&resetcorrection);
    resetCommand.stepMotorIndex = STEP_MOTOR_NEW_CUP_STACK_UPDOWN;
    resetCommand.correctionPosition = resetcorrection;//actionParam->moduleCupManageParam.newStackResetCorrection
    resetCommand.timeOutMs = 60000;
    //新杯栈复位
    errorCode = Can2SubSM_ResetWhileReturn(&resetCommand);

    if(errorCode != LH_ERR_NONE)
	{
        //打印错误日志
        TaskServiceSerialErrMsgShow(errorCode);
        //返回错误代码
		return errorCode;
	}

    //检测新杯栈平台是否有杯
    errorCode = SystemInputRead(NEW_STACK_TOP_SENSOR,&newStackDiskTopSensorValue);
	if(errorCode != LH_ERR_NONE)
	{
        //打印错误日志
        TaskServiceSerialErrMsgShow(errorCode);
        //返回错误代码
		return errorCode;
	}

    //新杯栈顶端放杯区有杯,不能初始化
	if(newStackDiskTopSensorValue == SENSOR_VALID_STATUS_CUP_MANAGE_NEW_STACK_CHECK)
    {
        //新杯栈顶部存在新盘
		*newStackTopHasDisk = ACTION_SENSOR_STATUS_EXIST;
		//新杯栈装载区有杯,不能初始化
		errorCode = LH_ERR_ACTION_NEW_STACK_DISK_EXIST_CAN_NOT_INIT;
		TaskServiceSerialErrMsgShow(errorCode);
		return errorCode;
    }
    else
    {
        *newStackTopHasDisk = ACTION_SENSOR_STATUS_NOT_EXIST;
    }

    //新杯栈移动到初次顶点
    SM_RUN_COORDINATE_CMD runCoordinateCmd;
    Can2SubSM_RunCoordinateDataStructInit(&runCoordinateCmd);

    //移动到指定位置
    SystemReadActionParamWithIndex(MODULECUPMANAGEPARAM,NEWSTACKFULLTOSENSOROFFSET,&newstackoffset);
    runCoordinateCmd.stepMotorIndex = STEP_MOTOR_NEW_CUP_STACK_UPDOWN;
    runCoordinateCmd.targetCoordinate = newstackoffset;//actionParam->moduleCupManageParam.newStackFullToSensorOffset;
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

    //读取顶部传感器状态
    errorCode = SystemInputRead(NEW_STACK_TOP_SENSOR,&newStackDiskTopSensorValue);
	if(errorCode != LH_ERR_NONE)
	{
        //打印错误日志
        TaskServiceSerialErrMsgShow(errorCode);
        //返回错误代码
		return errorCode;
	}

    SM_RUN_STEP_CMD runStepCommand;
    //走位结构体初始化
    Can2SubSM_RunStepsCmdDataStructInit(&runStepCommand);

    //读取新杯盘初始值也就是0
    currentindex = TaskUtilServiceGetCupStackManagementNewStackCount();
    //循环移动,每次移动完成都检测传感器状态
    SystemReadActionParamWithIndex(MODULECUPMANAGEPARAM,NEWSTACKONESTACKOFFSET,&newstackoffset);
	while(newStackDiskTopSensorValue != SENSOR_VALID_STATUS_CUP_MANAGE_NEW_STACK_CHECK)
	{
		//没检测到
		currentindex++;
		//已经到最大检测值
		if(currentindex == SYSTEM_NEW_STACK_MAX_DISK_COUNT)
		{
			break;
		}
        runStepCommand.stepMotorIndex = STEP_MOTOR_NEW_CUP_STACK_UPDOWN;
        runStepCommand.steps = newstackoffset;//actionParam->moduleCupManageParam.newStackOneStackOffset;
        runStepCommand.curveSelect = SM_CURVE_0;
        runStepCommand.speedRatio = 100;
        runStepCommand.speedMode = SPEED_MODE_UP_START;
        runStepCommand.utilStop1Enable = DISABLE;
        runStepCommand.utilStop2Enable = DISABLE;
        runStepCommand.timeOutMs = 60000;
        //每次移动一个杯位
        errorCode = Can2SubSM_RunSpecialStepsWhileReturn(&runStepCommand);

        if(errorCode != LH_ERR_NONE)
        {
            //打印错误日志
            TaskServiceSerialErrMsgShow(errorCode);
            //返回错误代码
            return errorCode;
        }

		//检测传感器状态
		errorCode = SystemInputRead(NEW_STACK_TOP_SENSOR,&newStackDiskTopSensorValue);
		if(errorCode != LH_ERR_NONE)
		{
			TaskServiceSerialErrMsgShow(errorCode);
			return errorCode;
		}
	}

    //计算杯栈数量
	currentindex = SYSTEM_NEW_STACK_MAX_DISK_COUNT - currentindex;
    //新盘数量返回
	*newStackDiskCount = currentindex;
    //保存新杯盘的数量
    TaskUtilServiceSetCupStackManagementNewStackCount(currentindex);
    //检测完成,回到零位偏移
    runCoordinateCmd.stepMotorIndex = STEP_MOTOR_NEW_CUP_STACK_UPDOWN;
    runCoordinateCmd.targetCoordinate =resetcorrection;// actionParam->moduleCupManageParam.newStackResetCorrection;
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
	
    //检测传感器状态
    errorCode = SystemInputRead(NEW_STACK_TOP_SENSOR,&newStackDiskTopSensorValue);
    if(errorCode != LH_ERR_NONE)
    {
        TaskServiceSerialErrMsgShow(errorCode);
        return errorCode;
    }

    //新杯栈顶端放杯区有杯
	if(newStackDiskTopSensorValue == SENSOR_VALID_STATUS_CUP_MANAGE_NEW_STACK_CHECK)
	{
		//新杯栈顶部存在新盘
		*newStackTopHasDisk = ACTION_SENSOR_STATUS_EXIST;
    }
    else
    {
        //新杯栈顶部不存在新盘
		*newStackTopHasDisk = ACTION_SENSOR_STATUS_NOT_EXIST;
    }

    return errorCode;

}

//空杯装载结构初始化
LH_ERR ActionCupStackManagement_EmptytackInit(ACTION_SENSOR_STATUS* emptyStackPushOnPosDiskExist,uint8_t* emptyStackDiskCount)
{
    LH_ERR errorCode;
    uint8_t currentCount = 0;
    int32_t emptyresetcorrect;
    //空杯栈上方检测杯盘有无传感器状态
	BitAction emptyStackDiskExistSensor;
	//空杯栈上方正在测试区域是否有杯盘正在测试传感器状态
	BitAction emptyStackTestingPushOnPosSensor;
	//当前存在的空杯盘数量为0
    TaskUtilServiceSetCupStackManagementEmptyStackCount(0);
    SystemReadActionParamWithIndex(MODULECUPMANAGEPARAM,EMPTYSTACKRESETCORRECTION,&emptyresetcorrect);

    //先设定传入的指针状态
	*emptyStackPushOnPosDiskExist = ACTION_SENSOR_STATUS_NOT_EXIST;
	*emptyStackDiskCount = 0;

    //电机复位结构体
    SM_RESET_CMD resetCommand;
    //结构体初始化
    Can2SubSM_ResetCmdDataStructInit(&resetCommand);

    //复位参数设置
    resetCommand.stepMotorIndex = STEP_MOTOR_EMPTY_CUP_STACK_UPDOWN;
    resetCommand.correctionPosition = emptyresetcorrect;//actionParam->moduleCupManageParam.emptyStackResetCorrection;
    resetCommand.timeOutMs = 60000;
    //空杯栈复位
    errorCode = Can2SubSM_ResetWhileReturn(&resetCommand);

    if(errorCode != LH_ERR_NONE)
	{
        //打印错误日志
        TaskServiceSerialErrMsgShow(errorCode);
        //返回错误代码
		return errorCode;
	}

    //空杯栈移动到初次顶点
    SM_RUN_COORDINATE_CMD runCoordinateCmd;
    Can2SubSM_RunCoordinateDataStructInit(&runCoordinateCmd);

    //移动到指定位置
    runCoordinateCmd.stepMotorIndex = STEP_MOTOR_EMPTY_CUP_STACK_UPDOWN;
    runCoordinateCmd.targetCoordinate = actionParam->moduleCupManageParam.emptyStackFullToSensorOffset;
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

    //检测传感器状态,如果检测到,杯栈数量+1
    errorCode = SystemInputRead(EMPTY_STACK_COUNT_SENSOR,&emptyStackDiskExistSensor);
    if(errorCode != LH_ERR_NONE)
    {
        TaskServiceSerialErrMsgShow(errorCode);
        return errorCode;
    }

    SM_RUN_STEP_CMD runStepCommand;
    //走位结构体初始化
    Can2SubSM_RunStepsCmdDataStructInit(&runStepCommand);
    //直到达到最大空杯栈数量
    while(emptyStackDiskExistSensor != SENSOR_VALID_STATUS_CUP_MANAGE_EMPTY_STACK_CHECK)
	{
		//没检测到
        currentCount++;
        //已经到最大检测值
        if(currentCount == SYSTEM_EMPTY_STACK_MAX_DISK_COUNT)
        {
            break;
        }
        //移动一个杯栈位置
        runStepCommand.stepMotorIndex = STEP_MOTOR_EMPTY_CUP_STACK_UPDOWN;
        runStepCommand.steps = actionParam->moduleCupManageParam.emptyStackOneStackOffset;
        runStepCommand.curveSelect = SM_CURVE_0;
        runStepCommand.speedRatio = 100;
        runStepCommand.speedMode = SPEED_MODE_UP_START;
        runStepCommand.utilStop1Enable = DISABLE;
        runStepCommand.utilStop2Enable = DISABLE;
        runStepCommand.timeOutMs = 60000;
        //每次移动一个杯位
        errorCode = Can2SubSM_RunSpecialStepsWhileReturn(&runStepCommand);

        if(errorCode != LH_ERR_NONE)
        {
            //打印错误日志
            TaskServiceSerialErrMsgShow(errorCode);
            //返回错误代码
            return errorCode;
        }

		//检测信号,如果检测到
        errorCode = SystemInputRead(EMPTY_STACK_COUNT_SENSOR,&emptyStackDiskExistSensor);
        if(errorCode != LH_ERR_NONE)
        {
            TaskServiceSerialErrMsgShow(errorCode);
            return errorCode;
        }
	}

    //检测完成,回到零位偏移
    runCoordinateCmd.stepMotorIndex = STEP_MOTOR_EMPTY_CUP_STACK_UPDOWN;
    runCoordinateCmd.targetCoordinate = emptyresetcorrect;//actionParam->moduleCupManageParam.emptyStackResetCorrection;
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

    //计算存在几个空盘子
    *emptyStackDiskCount = SYSTEM_EMPTY_STACK_MAX_DISK_COUNT - currentCount;
	currentCount = *emptyStackDiskCount;
    //保存空杯数量
    TaskUtilServiceSetCupStackManagementEmptyStackCount(currentCount);
	//读取空杯栈上方是否有杯
    errorCode = SystemInputRead(NEW_STACK_PUSH_IN_POS_SENSOR,&emptyStackTestingPushOnPosSensor);
    if(errorCode != LH_ERR_NONE)
    {
        TaskServiceSerialErrMsgShow(errorCode);
        return errorCode;
    }

	//返回正在使用位置的盘子是否存在
	if(emptyStackTestingPushOnPosSensor == SENSOR_VALID_STATUS_CUP_MANAGE_TESTING_DISK_CHECK)
	{
		*emptyStackPushOnPosDiskExist = ACTION_SENSOR_STATUS_EXIST;
	}
    else
    {
        *emptyStackPushOnPosDiskExist = ACTION_SENSOR_STATUS_NOT_EXIST;
    }
   
    return errorCode;
}

//推杆初始化
LH_ERR ActionCupStackManagement_PushInit(void)
{
    LH_ERR errorCode;
    int32_t pushresetcorrect;
    //电机复位结构体
    SM_RESET_CMD resetCommand;
    //结构体初始化
    Can2SubSM_ResetCmdDataStructInit(&resetCommand);
    SystemReadActionParamWithIndex(MODULECUPMANAGEPARAM,PUSHRESETCORRECTION,&pushresetcorrect);
    //复位参数设置
    resetCommand.stepMotorIndex = STEP_MOTOR_CUP_PUSH;
    resetCommand.correctionPosition = pushresetcorrect;//actionParam->moduleCupManageParam.pushResetCorrection;
    resetCommand.timeOutMs = 60000;
    //推手复位
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

//夹手初始化
LH_ERR ActionCupStackManagement_ClampInit(void)
{
    LH_ERR errorCode;
    int32_t clamreset;
    //电机复位结构体
    SM_RESET_CMD resetCommand;
    //结构体初始化
    Can2SubSM_ResetCmdDataStructInit(&resetCommand);
    SystemReadActionParamWithIndex(MODULECUPMANAGEPARAM,CLAMPRESETCORRECTION,&clamreset);
    //复位参数设置
    resetCommand.stepMotorIndex = STEP_MOTOR_CUP_HOLDER;
    resetCommand.correctionPosition = clamreset;//actionParam->moduleCupManageParam.clampResetCorrection;
    resetCommand.timeOutMs = 60000;
    //夹手复位
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

//夹手打开
LH_ERR ActionCupStackManagement_ClampOpen(void)
{
    LH_ERR errorCode;
    SM_RUN_COORDINATE_CMD runCoordinateCmd;
    int32_t yogi;
    Can2SubSM_RunCoordinateDataStructInit(&runCoordinateCmd);

    //移动到指定位置
    SystemReadActionParamWithIndex(MODULECUPMANAGEPARAM,CLAMPOPENPOS,&yogi);
    runCoordinateCmd.stepMotorIndex = STEP_MOTOR_CUP_HOLDER;
    runCoordinateCmd.targetCoordinate = yogi;//actionParam->moduleCupManageParam.clampOpenPos;
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

//夹手关闭
LH_ERR ActionCupStackManagement_ClampClose(void)
{
    LH_ERR errorCode;
    SM_RUN_COORDINATE_CMD runCoordinateCmd;
    int32_t yogi;
    Can2SubSM_RunCoordinateDataStructInit(&runCoordinateCmd);

    //移动到指定位置
    SystemReadActionParamWithIndex(MODULECUPMANAGEPARAM,CLAMPCLOSEPOS,&yogi);
    runCoordinateCmd.stepMotorIndex = STEP_MOTOR_CUP_HOLDER;
    runCoordinateCmd.targetCoordinate =yogi;// actionParam->moduleCupManageParam.clampClosePos;
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


//新杯装载机构上传一个新杯盘
LH_ERR ActionCupStackManagement_NewStackUpload(ACTION_SENSOR_STATUS* newStackTopHasDisk,uint8_t* newStackDiskCount)
{
    LH_ERR errorCode;
	BitAction newStackDiskTopSensorValue;
	int32_t posValue;
    int32_t resetcorrect;
    int32_t yogi;
    uint8_t currentCount;
    //读取新杯盘数量
    currentCount = TaskUtilServiceGetCupStackManagementNewStackCount();
	//初始默认不存在
	*newStackTopHasDisk = ACTION_SENSOR_STATUS_NOT_EXIST;
    *newStackDiskCount = currentCount;
    
    //新杯栈空的时候,不动
	if(currentCount == 0)
    {
        errorCode = LH_ERR_ACTION_NEW_STACK_DISK_COUNT_ZERO;
		TaskServiceSerialErrMsgShow(errorCode);
		return errorCode;;
    }

    //电机复位结构体
    SM_RESET_CMD resetCommand;
    //结构体初始化
    Can2SubSM_ResetCmdDataStructInit(&resetCommand);

    //复位参数设置
    SystemReadActionParamWithIndex(MODULECUPMANAGEPARAM,NEWSTACKRESETCORRECTION,&resetcorrect);
    resetCommand.stepMotorIndex = STEP_MOTOR_NEW_CUP_STACK_UPDOWN;
    resetCommand.correctionPosition =resetcorrect;// actionParam->moduleCupManageParam.newStackResetCorrection;
    resetCommand.timeOutMs = 60000;
    //新杯栈复位
    errorCode = Can2SubSM_ResetWhileReturn(&resetCommand);

    if(errorCode != LH_ERR_NONE)
	{
        //打印错误日志
        TaskServiceSerialErrMsgShow(errorCode);
        //返回错误代码
		return errorCode;
	}

    //先要检测传感器,如果已经有盘子在上面,那么不能上传
    errorCode = SystemInputRead(NEW_STACK_TOP_SENSOR,&newStackDiskTopSensorValue);
	if(errorCode != LH_ERR_NONE)
	{
        //打印错误日志
        TaskServiceSerialErrMsgShow(errorCode);
        //返回错误代码
		return errorCode;
	}

    //新杯栈顶端放杯区有杯,不能初始化
	if(newStackDiskTopSensorValue == SENSOR_VALID_STATUS_CUP_MANAGE_NEW_STACK_CHECK)
    {
        //新杯栈顶部存在新盘
		*newStackTopHasDisk = ACTION_SENSOR_STATUS_EXIST;
		//新杯栈装载区有杯,不能初始化
		errorCode = LH_ERR_ACTION_NEW_STACK_DISK_EXIST_CAN_NOT_UPLOAD;
		TaskServiceSerialErrMsgShow(errorCode);
		return errorCode;
    }
    else
    {
        *newStackTopHasDisk = ACTION_SENSOR_STATUS_NOT_EXIST;
    }

    //计算上升距离
    SystemReadActionParamWithIndex(MODULECUPMANAGEPARAM,NEWSTACKONESTACKOFFSET,&yogi);
	posValue = actionParam->moduleCupManageParam.newStackFullToPlatformOffset;

    posValue += (SYSTEM_NEW_STACK_MAX_DISK_COUNT - currentCount)*(yogi);
    //actionParam->moduleCupManageParam.newStackOneStackOffset
    SM_RUN_STEP_CMD runStepCommand;
    //走位结构体初始化
    Can2SubSM_RunStepsCmdDataStructInit(&runStepCommand);

    runStepCommand.stepMotorIndex = STEP_MOTOR_NEW_CUP_STACK_UPDOWN;
    runStepCommand.steps = posValue;
    runStepCommand.curveSelect = SM_CURVE_0;
    runStepCommand.speedRatio = 100;
    runStepCommand.speedMode = SPEED_MODE_UP_START;
    runStepCommand.utilStop1Enable = DISABLE;
    runStepCommand.utilStop2Enable = DISABLE;
    runStepCommand.timeOutMs = 60000;
    //每次移动一个杯位
    errorCode = Can2SubSM_RunSpecialStepsWhileReturn(&runStepCommand);

    if(errorCode != LH_ERR_NONE)
    {
        //打印错误日志
        TaskServiceSerialErrMsgShow(errorCode);
        //返回错误代码
        return errorCode;
    }

    SM_RETURN_ZERO_CMD returnZeroCmd;
    Can2SubSM_ReturnZeroDataStructInit(&returnZeroCmd);

    //升降回零并修正
    returnZeroCmd.stepMotorIndex = STEP_MOTOR_NEW_CUP_STACK_UPDOWN;
    returnZeroCmd.correctionPosition =resetcorrect;// actionParam->moduleCupManageParam.newStackResetCorrection;
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

    //读取光电讯号,检测杯栈传感器状态,确定是否完成上传
    errorCode = SystemInputRead(NEW_STACK_TOP_SENSOR,&newStackDiskTopSensorValue);
	if(errorCode != LH_ERR_NONE)
	{
        //打印错误日志
        TaskServiceSerialErrMsgShow(errorCode);
        //返回错误代码
		return errorCode;
	}

    if(newStackDiskTopSensorValue == SENSOR_VALID_STATUS_CUP_MANAGE_NEW_STACK_CHECK)
	{
		//新杯栈顶部存在新盘,代表上传了一个有效的盘子上去
		*newStackTopHasDisk = ACTION_SENSOR_STATUS_EXIST;
        //杯栈数量-1
	    currentCount -= 1;
        TaskUtilServiceSetCupStackManagementNewStackCount(currentCount);
	}
    else
    {
        //上传失败,没有上传有效盘子上去
        *newStackTopHasDisk = ACTION_SENSOR_STATUS_NOT_EXIST;
        //杯栈数量不减少
        *newStackDiskCount = currentCount;
        //返回上传失败
        errorCode =LH_ERR_ACTION_NEW_STACK_UPLOAD_FAILED_NO_STACK;
        TaskServiceSerialErrMsgShow(errorCode);
        return errorCode;
    }

	*newStackDiskCount = currentCount;
	return errorCode;
}


//推杆推一个杯盘到位
LH_ERR ActionCupStackManagement_PushOneDisk(ACTION_SENSOR_STATUS* emptyStackPushOnPosDiskExist)
{
    LH_ERR errorCode;
	BitAction emptyStackTestingOnPosSensor;
    int32_t pushresetcorrect,clamreset;
    int32_t yogi;
    *emptyStackPushOnPosDiskExist = ACTION_SENSOR_STATUS_NOT_EXIST;

	//夹手复位,运行到张开却不掉下去的水平
    SM_RESET_CMD resetCommand;
    //结构体初始化
    Can2SubSM_ResetCmdDataStructInit(&resetCommand);
    SystemReadActionParamWithIndex(MODULECUPMANAGEPARAM,CLAMPRESETCORRECTION,&clamreset);
    //复位参数设置
    resetCommand.stepMotorIndex = STEP_MOTOR_CUP_HOLDER;
    resetCommand.correctionPosition = clamreset;//actionParam->moduleCupManageParam.clampResetCorrection;
    resetCommand.timeOutMs = 60000;
    //夹手复位
    errorCode = Can2SubSM_ResetWhileReturn(&resetCommand);

    if(errorCode != LH_ERR_NONE)
	{
        //打印错误日志
        TaskServiceSerialErrMsgShow(errorCode);
        //返回错误代码
		return errorCode;
	}

    //推手第一次前推
    SM_RUN_COORDINATE_CMD runCoordinateCmd;
    Can2SubSM_RunCoordinateDataStructInit(&runCoordinateCmd);

    //移动到指定位置
    runCoordinateCmd.stepMotorIndex = STEP_MOTOR_CUP_PUSH;
    runCoordinateCmd.targetCoordinate = actionParam->moduleCupManageParam.pushFirstPushPosition;
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

    //检测传感器的状态,读取空杯栈上方是否有杯
    errorCode = SystemInputRead(NEW_STACK_PUSH_IN_POS_SENSOR,&emptyStackTestingOnPosSensor);
	if(errorCode != LH_ERR_NONE)
	{
        //打印错误日志
        TaskServiceSerialErrMsgShow(errorCode);
        //返回错误代码
		return errorCode;
	}

    //如果杯状态存在,那么再往前推一次
	if(emptyStackTestingOnPosSensor == SENSOR_VALID_STATUS_CUP_MANAGE_TESTING_DISK_CHECK)
	{
        SM_RUN_STEP_CMD runStepCommand;
        //走位结构体初始化
        Can2SubSM_RunStepsCmdDataStructInit(&runStepCommand);
        SystemReadActionParamWithIndex(MODULECUPMANAGEPARAM,PUSHSECONDPUSHOFFSET,&yogi);
        runStepCommand.stepMotorIndex = STEP_MOTOR_CUP_PUSH;
        runStepCommand.steps = yogi;//actionParam->moduleCupManageParam.pushSecondPushOffset;
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
	}

    SM_RETURN_ZERO_CMD returnZeroCmd;
    Can2SubSM_ReturnZeroDataStructInit(&returnZeroCmd);

    //推手回零
    SystemReadActionParamWithIndex(MODULECUPMANAGEPARAM,PUSHRESETCORRECTION,&pushresetcorrect);
    returnZeroCmd.stepMotorIndex = STEP_MOTOR_CUP_PUSH;
    returnZeroCmd.correctionPosition = pushresetcorrect;//actionParam->moduleCupManageParam.pushResetCorrection;
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

	//加持机构夹紧
    SystemReadActionParamWithIndex(MODULECUPMANAGEPARAM,CLAMPCLOSEPOS,&yogi);
    runCoordinateCmd.stepMotorIndex = STEP_MOTOR_CUP_HOLDER;
    runCoordinateCmd.targetCoordinate = yogi;//actionParam->moduleCupManageParam.clampClosePos;
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

    //检测是否有杯
	errorCode = SystemInputRead(NEW_STACK_PUSH_IN_POS_SENSOR,&emptyStackTestingOnPosSensor);
	if(errorCode != LH_ERR_NONE)
	{
        //打印错误日志
        TaskServiceSerialErrMsgShow(errorCode);
        //返回错误代码
		return errorCode;
	}

    //更新是否有杯
    if(emptyStackTestingOnPosSensor == SENSOR_VALID_STATUS_CUP_MANAGE_TESTING_DISK_CHECK)
    {
        *emptyStackPushOnPosDiskExist = ACTION_SENSOR_STATUS_EXIST;
    }
    else
    {
        *emptyStackPushOnPosDiskExist = ACTION_SENSOR_STATUS_NOT_EXIST;
    }

	return errorCode;
}


//空杯装载机构上升承载空杯盘
LH_ERR ActionCupStackManagement_EmptyStackAscend(ACTION_SENSOR_STATUS* emptyStackPushOnPosDiskExist,uint8_t* emptyStackDiskCount)
{
    LH_ERR errorCode;
	BitAction emptyStackTestingOnPosSensor;
	int32_t posValue;
    uint8_t currentCount;
    int32_t emptyresetcorrect,clamreset,yogi;
    currentCount = TaskUtilServiceGetCupStackManagementEmptyStackCount();
    *emptyStackDiskCount = currentCount;
    SystemReadActionParamWithIndex(MODULECUPMANAGEPARAM,EMPTYSTACKRESETCORRECTION,&emptyresetcorrect);
    //检测是否有杯
    errorCode = SystemInputRead(NEW_STACK_PUSH_IN_POS_SENSOR,&emptyStackTestingOnPosSensor);
	if(errorCode != LH_ERR_NONE)
	{
        //打印错误日志
        TaskServiceSerialErrMsgShow(errorCode);
        //返回错误代码
		return errorCode;
	}

    //如果检测不到杯,那就不需要承载上升承载新杯
    if(emptyStackTestingOnPosSensor != SENSOR_VALID_STATUS_CUP_MANAGE_TESTING_DISK_CHECK)
    {
        //检测不到杯,那就不需要承载上升承载新杯
        *emptyStackPushOnPosDiskExist = ACTION_SENSOR_STATUS_NOT_EXIST;
        errorCode = LH_ERR_ACTION_EMPTY_STACK_TESTING_STACK_NOT_EXIST;
        TaskServiceSerialErrMsgShow(errorCode);
        return errorCode;
    }
    else
    {
        *emptyStackPushOnPosDiskExist = ACTION_SENSOR_STATUS_EXIST;
    }

    //如果已经达到最大空杯存放数量
	if(currentCount == SYSTEM_EMPTY_STACK_MAX_DISK_COUNT)
    {
        errorCode = LH_ERR_ACTION_EMPTY_STACK_FULL;
        TaskServiceSerialErrMsgShow(errorCode);
        return errorCode;
    }

	//计算偏移,根据当前计数,移动到空杯栈位置杯栈上去
    posValue = actionParam->moduleCupManageParam.emptyStackFullToTopOffset;
    posValue += (SYSTEM_EMPTY_STACK_MAX_DISK_COUNT - currentCount)*(actionParam->moduleCupManageParam.emptyStackOneStackOffset);

    SM_RUN_STEP_CMD runStepCommand;
    //走位结构体初始化
    Can2SubSM_RunStepsCmdDataStructInit(&runStepCommand);

    runStepCommand.stepMotorIndex = STEP_MOTOR_EMPTY_CUP_STACK_UPDOWN;
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

    //夹手打开
    SM_RUN_COORDINATE_CMD runCoordinateCmd;
    Can2SubSM_RunCoordinateDataStructInit(&runCoordinateCmd);

    //移动到指定位置
    SystemReadActionParamWithIndex(MODULECUPMANAGEPARAM,CLAMPOPENPOS,&yogi);
    runCoordinateCmd.stepMotorIndex = STEP_MOTOR_CUP_HOLDER;
    runCoordinateCmd.targetCoordinate = yogi;//actionParam->moduleCupManageParam.clampOpenPos;
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

    //等待1S,盘子掉下去
	CoreDelayMs(1000);

    SM_RETURN_ZERO_CMD returnZeroCmd;
    Can2SubSM_ReturnZeroDataStructInit(&returnZeroCmd);

    //空杯栈退回
    returnZeroCmd.stepMotorIndex = STEP_MOTOR_EMPTY_CUP_STACK_UPDOWN;
    returnZeroCmd.correctionPosition = emptyresetcorrect;//actionParam->moduleCupManageParam.emptyStackResetCorrection;
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

    //电机复位结构体
    SM_RESET_CMD resetCommand;
    //结构体初始化
    Can2SubSM_ResetCmdDataStructInit(&resetCommand);

    //复位参数设置
    SystemReadActionParamWithIndex(MODULECUPMANAGEPARAM,CLAMPRESETCORRECTION,&clamreset);
    resetCommand.stepMotorIndex = STEP_MOTOR_CUP_HOLDER;
    resetCommand.correctionPosition = clamreset;//actionParam->moduleCupManageParam.clampResetCorrection;
    resetCommand.timeOutMs = 60000;
    //夹手复位
    errorCode = Can2SubSM_ResetWhileReturn(&resetCommand);

    if(errorCode != LH_ERR_NONE)
	{
        //打印错误日志
        TaskServiceSerialErrMsgShow(errorCode);
        //返回错误代码
		return errorCode;
	}

    //检测是否有杯
    errorCode = SystemInputRead(NEW_STACK_PUSH_IN_POS_SENSOR,&emptyStackTestingOnPosSensor);
	if(errorCode != LH_ERR_NONE)
	{
        //打印错误日志
        TaskServiceSerialErrMsgShow(errorCode);
        //返回错误代码
		return errorCode;
	}

    if(emptyStackTestingOnPosSensor == SENSOR_VALID_STATUS_CUP_MANAGE_TESTING_DISK_CHECK)
    {
        //还存在新杯,说明上一次承载新杯失败
        *emptyStackPushOnPosDiskExist = ACTION_SENSOR_STATUS_EXIST;

        *emptyStackDiskCount = currentCount;

        errorCode = LH_ERR_ACTION_EMPTY_STACK_CAN_NOT_FALL;

        TaskServiceSerialErrMsgShow(errorCode);
        return errorCode;
    }
    else
    {
        emptyStackPushOnPosDiskExist = ACTION_SENSOR_STATUS_NOT_EXIST;
        //空杯盘数量+1
	    currentCount += 1;

        *emptyStackDiskCount = currentCount;

        TaskUtilServiceSetCupStackManagementEmptyStackCount(currentCount);
    }
	return errorCode;
}


//杯栈管理区域垃圾桶锁住,指定垃圾桶上升,另一个垃圾桶下降
LH_ERR ActionCupStackManagement_GarbageLock(ACTION_PARAM_STACK_MANAGE_GARBAGE_NO garbageNo)
{
    LH_ERR errorCode;
	if(garbageNo == TSC_STACK_MANAGE_GARBAGE_NO1)
    {
        CoreDelayMs(700);
    }
    else if(garbageNo == TSC_STACK_MANAGE_GARBAGE_NO2)
    {
        CoreDelayMs(1400);
    }
    else
    {
        errorCode = LH_ERR_ACTION_PARAM;
        TaskServiceSerialErrMsgShow(errorCode);
        return errorCode;
    }
    return errorCode;
}


//杯栈管理机构指令垃圾桶下降,不管上升的是哪个
LH_ERR ActionCupStackManagement_GarbageUnlock(ACTION_PARAM_STACK_MANAGE_GARBAGE_NO garbageNo)
{
    LH_ERR errorCode = LH_ERR_NONE;
	if(garbageNo == TSC_STACK_MANAGE_GARBAGE_NO1)
    {
        CoreDelayMs(700);
    }
    else if(garbageNo == TSC_STACK_MANAGE_GARBAGE_NO2)
    {
        CoreDelayMs(1400);
    }
    else
    {
        errorCode = LH_ERR_ACTION_PARAM;
        //打印错误日志
        TaskServiceSerialErrMsgShow(errorCode);
        //返回错误代码
		return errorCode;
    }
    return errorCode;
}
//杯栈门磁开关控制
LH_ERR ActionCupStackManagement_DoorControl(MACHINE_DOOR_CMD doorcmd)
{
    LH_ERR errCode;
    if(doorcmd == MACHINE_DOOR_UNLOCK)
    {
        //断开电磁门
        errCode = SystemOutputWrite(OUTPUT_CUP_MANAGEMENT_MAGENT_DOOR,Bit_SET);
        if(errCode != LH_ERR_NONE)
        {
            //打印错误日志
            TaskServiceSerialErrMsgShow(errCode);
            //返回错误代码
            return errCode;
        }
        //绿灯亮黄灯灭
        errCode = SystemOutputWrite(OUTPUT_CUP_MANAGEMENT_LED_GREEN,Bit_RESET);
        if(errCode != LH_ERR_NONE)
        {
            //打印错误日志
            TaskServiceSerialErrMsgShow(errCode);
            //返回错误代码
            return errCode;
        }
        errCode = SystemOutputWrite(OUTPUT_CUP_MANAGEMENT_LED_YELLOW,Bit_SET);
        if(errCode != LH_ERR_NONE)
        {
            //打印错误日志
            TaskServiceSerialErrMsgShow(errCode);
            //返回错误代码
            return errCode;
        }
        //等待5S
        CoreDelayMs(5000);
        //电磁门自动重新上电
        errCode = SystemOutputWrite(OUTPUT_CUP_MANAGEMENT_MAGENT_DOOR,Bit_RESET);
        if(errCode != LH_ERR_NONE)
        {
            //打印错误日志
            TaskServiceSerialErrMsgShow(errCode);
            //返回错误代码
            return errCode;
        }
    }
    else
    {
        //打开电磁门
        errCode = SystemOutputWrite(OUTPUT_CUP_MANAGEMENT_MAGENT_DOOR,Bit_RESET);
        if(errCode != LH_ERR_NONE)
        {
            //打印错误日志
            TaskServiceSerialErrMsgShow(errCode);
            //返回错误代码
            return errCode;
        }
        //绿灯灭黄灯亮
        errCode = SystemOutputWrite(OUTPUT_CUP_MANAGEMENT_LED_GREEN,Bit_SET);
        if(errCode != LH_ERR_NONE)
        {
            //打印错误日志
            TaskServiceSerialErrMsgShow(errCode);
            //返回错误代码
            return errCode;
        }
        errCode = SystemOutputWrite(OUTPUT_CUP_MANAGEMENT_LED_YELLOW,Bit_RESET);
        if(errCode != LH_ERR_NONE)
        {
            //打印错误日志
            TaskServiceSerialErrMsgShow(errCode);
            //返回错误代码
            return errCode;
        }
    }
    
    return errCode;
}
//更新杯栈管理机构的当前传感器状态
LH_ERR ActionCupStackManagement_GarbageReflushSensor(void)
{
    CoreDelayMs(1000);
    return LH_ERR_NONE;
}



//门状态扫描
void doorstatescan(void)
{
    uint8_t result;
    LH_ERR errCode;
    BitAction Kstate,Pstate;
    result = BoardOutReadSingle(GPIOG,GPIO_Pin_7);
    Kstate = (BitAction)result;
    //电磁铁通电
    if(Kstate == Bit_SET)
    {
        SystemInputRead(INPUT_CUP_MANAGEMENT_MAGENT_DOOR,&Pstate);
        //新杯门打开
        if(Pstate == Bit_RESET)
        {
            //绿灯亮
            errCode = SystemOutputWrite(OUTPUT_CUP_MANAGEMENT_LED_GREEN,Bit_RESET);
            
            //黄灯灭
            errCode = SystemOutputWrite(OUTPUT_CUP_MANAGEMENT_LED_YELLOW,Bit_SET);
            
            TaskUtilServiceSetCupStackManagementDoorState(MACHINE_DOOR_STATUS_OPEN);
        }
        //新杯门关闭
        if(Pstate == Bit_SET)
        {
            //绿灯灭
            errCode = SystemOutputWrite(OUTPUT_CUP_MANAGEMENT_LED_GREEN,Bit_SET);
            
            //黄灯亮
            errCode = SystemOutputWrite(OUTPUT_CUP_MANAGEMENT_LED_YELLOW,Bit_RESET);
            
            TaskUtilServiceSetCupStackManagementDoorState(MACHINE_DOOR_STATUS_CLOSE);

        }
    }
    //电磁铁没有通电
    else if(Kstate == Bit_RESET)
    {
        TaskUtilServiceSetCupStackManagementDoorState(MACHINE_DOOR_STATUS_WAIT);
    }
}






