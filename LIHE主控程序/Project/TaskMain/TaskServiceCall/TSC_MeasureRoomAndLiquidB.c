#include "TSC_MeasureRoomAndLiquidB.h"

//测量室与B液注液状态数据结构
static TSC_CMD_RESULT_DATA_MEASURE_ROOM_LIQUID_B resultDataTSC_MeasureRoom;
//测量室与B液注液数据结构的互斥信号量
static OS_MUTEX mutexWithTSC_MeasureRoom;

/***********************************************测量室与注B液动作****************************************************************/
//测量室与B液注液上一次的执行状态
TSC_CMD_STATE TSC_MeasureRoomGetLastCommandStatusAndResult(LH_ERR* errCode)
{
    OS_ERR err;
    //申请信号量
    OSMutexPend (&mutexWithTSC_MeasureRoom,0,OS_OPT_PEND_NON_BLOCKING,0,&err);
    if(err != OS_ERR_NONE)
    {
        //申请不到信号量,说明线程正在执行,等待返回
        return TSC_CMD_STATE_WAIT_RETURN;
    }
    //申请到了
    *errCode = resultDataTSC_MeasureRoom.resultValue;
    //释放信号量
    OSMutexPost(&mutexWithTSC_MeasureRoom,OS_OPT_POST_NONE,&err);
    //返回动作完成
    return resultDataTSC_MeasureRoom.status;
}

//测量室与B液注液数据结构初始化
void TSC_MeasureRoomResultDataInit(void)
{
    OS_ERR err;
    //创建互斥信号量
    OSMutexCreate((OS_MUTEX*	)&mutexWithTSC_MeasureRoom,
				  (CPU_CHAR*	)"MUTEX_MeasureRoom",
                  (OS_ERR*		)&err);	
    //状态赋值
    resultDataTSC_MeasureRoom.dataMutexPtr = &mutexWithTSC_MeasureRoom;
    resultDataTSC_MeasureRoom.status = TSC_CMD_STATE_IDLE;
    resultDataTSC_MeasureRoom.resultValue = LH_ERR_NONE;
    resultDataTSC_MeasureRoom.lastLightSensorReadValue = 0;
}


//动作API发送任务给动作API
static LH_ERR TSC_MeasureRoomSendCommandWhileAck(uint16_t cmdIndex,
            int32_t param1,int32_t param2,int32_t param3,int32_t param4,int32_t param5)
{
    OS_ERR err;
    //自身消息指针
    SYSTEM_CMD* systemCommandPtr = NULL;

    do
    {
        //先申请信号量,不阻塞线程
	    OSMutexPend (&mutexWithTSC_MeasureRoom,0,OS_OPT_PEND_NON_BLOCKING,0,&err);
        if(err != OS_ERR_NONE)
        {
            CoreDelayMinTick();
        }
    }while(err != OS_ERR_NONE);
    //此处,获取了信号量
    
    //指令状态,等待ACK
    resultDataTSC_MeasureRoom.status = TSC_CMD_STATE_WAIT_ACK;
    //指令结果值设置为初始化状态
    resultDataTSC_MeasureRoom.resultValue = LH_ERR_NONE;

    //申请自身消息数据,一定要申请到
    while(systemCommandPtr == NULL)
    {
        systemCommandPtr = UserMemMalloc(SRAM_CCM,sizeof(SYSTEM_CMD));
        if(systemCommandPtr == NULL)
        {
            CoreDelayMinTick();
        }
    }
    systemCommandPtr->systemCommandDataPtr = NULL;
    systemCommandPtr->commandSrc = SYSTEM_CMD_SRC_SELF;

    //一定要申请到
    while(systemCommandPtr->systemCommandDataPtr == NULL)
    {
        systemCommandPtr->systemCommandDataPtr = UserMemMalloc(SRAM_CCM,sizeof(SYSTEM_CMD_SELF));
        if(systemCommandPtr->systemCommandDataPtr == NULL)
        {
            CoreDelayMinTick();
        }
    }

    SYSTEM_CMD_SELF* selfCommandPtr = systemCommandPtr->systemCommandDataPtr;
    //消息序号
    selfCommandPtr->commandIndex = (uint16_t)cmdIndex;
    //参数设置
    selfCommandPtr->commandParam1 = param1;
    selfCommandPtr->commandParam2 = param2;
    selfCommandPtr->commandParam3 = param3;
    selfCommandPtr->commandParam4 = param4;
    selfCommandPtr->commandParam5 = param5;

    //设置不同信道特有的数据指针
    selfCommandPtr->commandDataSpecialChannel = (void*)(&resultDataTSC_MeasureRoom);
    //将申请的指针传递给对应的任务信道做处理
    OSTaskQPost((OS_TCB *)&tcbTaskServiceMeasureRoomAndLiquidB, //向协议处理线程发送消息
                        (void *)systemCommandPtr,
                        (OS_MSG_SIZE)sizeof(SYSTEM_CMD*),
                        (OS_OPT)OS_OPT_POST_FIFO,
                        (OS_ERR *)&err);
    //检测是否发送成功
    if (err != OS_ERR_NONE)
    {
        //发送失败,直接释放本次申请的内存
        UserMemFree(SRAM_CCM, (void *)systemCommandPtr->systemCommandDataPtr);
        UserMemFree(SRAM_CCM, (void *)systemCommandPtr);
        //指令没发出去,还是回到空闲状态
        resultDataTSC_MeasureRoom.status = TSC_CMD_STATE_IDLE;
        //表示本次信道处于满状态,直接返回错误,返回之前要释放互斥信号量
        OSMutexPost(&mutexWithTSC_MeasureRoom,OS_OPT_POST_NONE,&err);
        //动作任务FIFO满员
        return LH_ERR_ACTION_CHANNEL_CMD_FULL;
    }

    //此处发送成功,释放互斥信号量
    OSMutexPost(&mutexWithTSC_MeasureRoom,OS_OPT_POST_NONE,&err);
    //等待状态不是等待ACK,说明线程已经开始处理
    while(resultDataTSC_MeasureRoom.status == TSC_CMD_STATE_WAIT_ACK)
    {
        CoreDelayMinTick();
    }

    //只要这个状态变化了,就代表程序收到了ACK,就可以返回
    return LH_ERR_NONE;
}

//测量室初始化
LH_ERR TSC_MeasureRoomInitWhileAck(void)
{
    return TSC_MeasureRoomSendCommandWhileAck(TSC_MEASURE_ROOM_INIT,0,0,0,0,0);
}

//测量室上门全开,打开到可以取杯的位置
LH_ERR TSC_MeasureRoomUpdoorOpenFullWhileAck(void)
{
    return TSC_MeasureRoomSendCommandWhileAck(TSC_MEASURE_ROOM_UPDOOR_OPEN_FULL,0,0,0,0,0);
}

//测量室上门半开,打开到可以放杯的位置
LH_ERR TSC_MeasureRoomUpDoorOpenHalfWhileAck(void)
{
    return TSC_MeasureRoomSendCommandWhileAck(TSC_MEASURE_ROOM_UPDOOR_OPEN_HALF,0,0,0,0,0);
}

//测量室上门关闭
LH_ERR TSC_MeasureRoomUpDoorCloseWhileAck(void)
{
    return TSC_MeasureRoomSendCommandWhileAck(TSC_MEASURE_ROOM_UPDOOR_CLOSE,0,0,0,0,0);
}

//测量窗关闭
LH_ERR TSC_MeasureRoomWindowsCloseWhileAck(void)
{
    return TSC_MeasureRoomSendCommandWhileAck(TSC_MEASURE_ROOM_WINDOW_CLOSE,0,0,0,0,0);
}


//测量室B液灌注一次
LH_ERR TSC_MeasureRoomPrimeOnceWhileAck(void)
{
    return TSC_MeasureRoomSendCommandWhileAck(TSC_MEASURE_ROOM_PRIME_ONCE,0,0,0,0,0);
}

//测量室B液注液一次
LH_ERR TSC_MeasureRoomInjectOnceWhileAck(void)
{
    return TSC_MeasureRoomSendCommandWhileAck(TSC_MEASURE_ROOM_INJECT_ONCE,0,0,0,0,0);
}

//测量室读值,读值之前自动注液,读取的值在执行完成之后调用模块的函数查询
LH_ERR TSC_MeasureRoomReadValueWithInjectWhileAck(uint16_t measureTimeMs)
{
    return TSC_MeasureRoomSendCommandWhileAck(TSC_MEASURE_ROOM_READ_VALUE_WITH_INJECT,measureTimeMs,0,0,0,0);
}

//测量室读值,读数据之前不注液,读取的值在执行完成之后调用模块的函数查询
LH_ERR TSC_MeasureRoomReadValueWithOutInjectWhileAck(uint16_t measureTimeMs)
{
    return TSC_MeasureRoomSendCommandWhileAck(TSC_MEASURE_ROOM_READ_VALUE_WITHOUT_INJECT,measureTimeMs,0,0,0,0);
}
//新增测量室读值
LH_ERR TSC_MeasureRoomWindowsMeasureBackGroundAndValue(uint16_t measureTimeMs,MEASURE_VALUE_FLAG injectFlag)
{
    return TSC_MeasureRoomSendCommandWhileAck(TSC_MEASURE_ROOM_READ_VALUE,measureTimeMs,(int32_t)injectFlag,0,0,0);
}

/***********************************************测量室与注B液动作****************************************************************/


/*********************************************动作结果中的数据读取***************************************************/


//读取上次的测光指令的测光值
uint32_t  TSC_MeasureRoomReadLastLightResult(void)
{
    return resultDataTSC_MeasureRoom.lastLightSensorReadValue;
}
//读取上次的暗计数
uint32_t  TSC_MeasureRoomReadLastDarkResult(void)
{
    return resultDataTSC_MeasureRoom.lastLightSensorReadDark;
}
//读取上次本底
uint32_t  TSC_MeasureRoomReadLastGroundResult(void)
{
    return resultDataTSC_MeasureRoom.lastLightSensorReadBackGround;
}




