#include "TSC_DishReactionAndMixReagent.h"

//反应盘与试剂混匀数据结构
static TSC_CMD_RESULT_DATA_DISH_REACTION_REAGENT_MIX resultDataTSC_DishReactionReagentMix;
//反应盘与试剂混匀数据结构的互斥信号量
static OS_MUTEX mutexWithTSC_DishReactionReagentMix;

/***************************************************反应盘与试剂摇匀动作***********************************************************/
//反应盘和试剂混匀上一次的执行状态
TSC_CMD_STATE TSC_DishReactionAndReagentMixGetLastCommandStatusAndResult(LH_ERR* errCode)
{
    OS_ERR err;
    //申请信号量
    OSMutexPend (&mutexWithTSC_DishReactionReagentMix,0,OS_OPT_PEND_NON_BLOCKING,0,&err);
    if(err != OS_ERR_NONE)
    {
        //申请不到信号量,说明线程正在执行,等待返回
        return TSC_CMD_STATE_WAIT_RETURN;
    }
    //申请到了
    *errCode = resultDataTSC_DishReactionReagentMix.resultValue;
    //释放信号量
    OSMutexPost(&mutexWithTSC_DishReactionReagentMix,OS_OPT_POST_NONE,&err);
    //返回动作完成
    return resultDataTSC_DishReactionReagentMix.status;
}

//反应盘和试剂混匀数据结构初始化
void TSC_DishReactionAndReagentMixResultDataInit(void)
{
    OS_ERR err;
    //创建互斥信号量
    OSMutexCreate((OS_MUTEX*	)&mutexWithTSC_DishReactionReagentMix,
				  (CPU_CHAR*	)"MUTEX_DishWashReagentMix",
                  (OS_ERR*		)&err);	
    //状态赋值
    resultDataTSC_DishReactionReagentMix.dataMutexPtr = &mutexWithTSC_DishReactionReagentMix;
    resultDataTSC_DishReactionReagentMix.status = TSC_CMD_STATE_IDLE;
    resultDataTSC_DishReactionReagentMix.resultValue = LH_ERR_NONE;
    resultDataTSC_DishReactionReagentMix.currentReactionHoleIndex = 0;
    resultDataTSC_DishReactionReagentMix.currentHoleHasCup = ACTION_SENSOR_STATUS_NOT_EXIST;
}



//动作API发送任务给动作API
static LH_ERR TSC_DishReactionAndReagentMixSendCommandWhileAck(uint16_t cmdIndex,
            int32_t param1,int32_t param2,int32_t param3,int32_t param4,int32_t param5)
{
    OS_ERR err;
    //自身消息指针
    SYSTEM_CMD* systemCommandPtr = NULL;

    do
    {
        //先申请信号量,不阻塞线程
	    OSMutexPend (&mutexWithTSC_DishReactionReagentMix,0,OS_OPT_PEND_NON_BLOCKING,0,&err);
        if(err != OS_ERR_NONE)
        {
            CoreDelayMinTick();
        }
    }while(err != OS_ERR_NONE);
    //此处,获取了信号量
    
    //指令状态,等待ACK
    resultDataTSC_DishReactionReagentMix.status = TSC_CMD_STATE_WAIT_ACK;
    //指令结果值设置为初始化状态
    resultDataTSC_DishReactionReagentMix.resultValue = LH_ERR_NONE;

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
    selfCommandPtr->commandDataSpecialChannel = (void*)(&resultDataTSC_DishReactionReagentMix);
    //将申请的指针传递给对应的任务信道做处理
    OSTaskQPost((OS_TCB *)&tcbTaskServiceDishReactionMixReagent, //向协议处理线程发送消息
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
        resultDataTSC_DishReactionReagentMix.status = TSC_CMD_STATE_IDLE;
        //表示本次信道处于满状态,直接返回错误,返回之前要释放互斥信号量
        OSMutexPost(&mutexWithTSC_DishReactionReagentMix,OS_OPT_POST_NONE,&err);
        //动作任务FIFO满员
        return LH_ERR_ACTION_CHANNEL_CMD_FULL;
    }

    //此处发送成功,释放互斥信号量
    OSMutexPost(&mutexWithTSC_DishReactionReagentMix,OS_OPT_POST_NONE,&err);
    //等待状态不是等待ACK,说明线程已经开始处理
    while(resultDataTSC_DishReactionReagentMix.status == TSC_CMD_STATE_WAIT_ACK)
    {
        CoreDelayMinTick();
    }

    //只要这个状态变化了,就代表程序收到了ACK,就可以返回
    return LH_ERR_NONE;
}

//反应盘初始化
LH_ERR TSC_DishReactionInitWhileAck(void)
{
    return TSC_DishReactionAndReagentMixSendCommandWhileAck(TSC_DISH_REACTION_INIT,0,0,0,0,0);
}

//反应盘旋转到指定孔位
LH_ERR TSC_DishReactionMove2SpecialHoleWhileAck(uint8_t targetHoleIndex)
{
    return TSC_DishReactionAndReagentMixSendCommandWhileAck(TSC_DISH_REACTION_MOVE_2_SPECIAL_HOLE,targetHoleIndex,0,0,0,0);
}

//反应盘相对于当前孔位旋转指定个孔位
LH_ERR TSC_DishReactionMoveAnyHoleStepWhileAck(uint8_t holeSteps)
{
    return TSC_DishReactionAndReagentMixSendCommandWhileAck(TSC_DISH_REACTION_MOVE_ANY_HOLE_STEP,holeSteps,0,0,0,0);
}

//试剂混匀复位
LH_ERR TSC_ReagentMixInitWhileAck(void)
{
    return TSC_DishReactionAndReagentMixSendCommandWhileAck(TSC_REAGENT_MIX_INIT,0,0,0,0,0);
}

//试剂混匀开始混匀指定时间,单位毫秒,试剂摇匀自动升起来,摇匀指定时间之后自动降下去
LH_ERR TSC_ReagentMixRotateAnyTimeMsWhileAck(uint16_t timeMs)
{
    return TSC_DishReactionAndReagentMixSendCommandWhileAck(TSC_REAGENT_MIX_ROTATE_ANY_TIME,timeMs,0,0,0,0);
}



/***************************************************反应盘与试剂摇匀动作***********************************************************/

/*********************************************动作结果中的数据读取***************************************************/

//读取当前反应盘孔位坐标
uint8_t TSC_DishReactionReadCurrentHoleIndex(void)
{
    return resultDataTSC_DishReactionReagentMix.currentReactionHoleIndex;
}

//读取当前反应盘孔位是否有杯
ACTION_SENSOR_STATUS TSC_DishReactionCheckCupExist(void)
{
    return resultDataTSC_DishReactionReagentMix.currentHoleHasCup;
}
