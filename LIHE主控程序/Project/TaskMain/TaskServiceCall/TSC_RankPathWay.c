#include "TSC_RankPathWay.h"

//试管架传送带状态数据结构
static TSC_CMD_RESULT_DATA_RACK_PATH_WAY resultDataTSC_RankPathWay;

//试管架传送带数据结构的互斥信号量
static OS_MUTEX mutexWithTSC_RankPathWay;

/***********************************************试管架传送带动作****************************************************************/
//试管架传送带上一次指令的执行状态
TSC_CMD_STATE TSC_RankPathWayGetLastCommandStatusAndResult(LH_ERR* errCode)
{
    OS_ERR err;
    //申请信号量
    OSMutexPend (&mutexWithTSC_RankPathWay,0,OS_OPT_PEND_NON_BLOCKING,0,&err);
    if(err != OS_ERR_NONE)
    {
        //申请不到信号量,说明线程正在执行,等待返回
        return TSC_CMD_STATE_WAIT_RETURN;
    }
    //申请到了
    *errCode = resultDataTSC_RankPathWay.resultValue;
    //释放信号量
    OSMutexPost(&mutexWithTSC_RankPathWay,OS_OPT_POST_NONE,&err);
    //返回动作完成
    return resultDataTSC_RankPathWay.status;
}

//试管架传送带数据结构初始化
void TSC_RankPathWayResultDataInit(void)
{
    OS_ERR err;
    //创建互斥信号量
    OSMutexCreate((OS_MUTEX*	)&mutexWithTSC_RankPathWay,
				  (CPU_CHAR*	)"MUTEX_RankPathWay",
                  (OS_ERR*		)&err);	

    //状态赋值
    resultDataTSC_RankPathWay.dataMutexPtr = &mutexWithTSC_RankPathWay;
    resultDataTSC_RankPathWay.status = TSC_CMD_STATE_IDLE;
    resultDataTSC_RankPathWay.resultValue = LH_ERR_NONE;
    resultDataTSC_RankPathWay.currentTubeIndex = 0;
}


//动作API发送任务给动作API
static LH_ERR TSC_RankPathWaySendCommandWhileAck(uint16_t cmdIndex,
            int32_t param1,int32_t param2,int32_t param3,int32_t param4,int32_t param5)
{
    OS_ERR err;
    //自身消息指针
    SYSTEM_CMD* systemCommandPtr = NULL;

    do
    {
        //先申请信号量,不阻塞线程
	    OSMutexPend (&mutexWithTSC_RankPathWay,0,OS_OPT_PEND_NON_BLOCKING,0,&err);
        if(err != OS_ERR_NONE)
        {
            CoreDelayMinTick();
        }
    }while(err != OS_ERR_NONE);
    //此处,获取了信号量
    
    //指令状态,等待ACK
    resultDataTSC_RankPathWay.status = TSC_CMD_STATE_WAIT_ACK;
    //指令结果值设置为初始化状态
    resultDataTSC_RankPathWay.resultValue = LH_ERR_NONE;

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
    selfCommandPtr->commandDataSpecialChannel = (void*)(&resultDataTSC_RankPathWay);
    //将申请的指针传递给对应的任务信道做处理
    OSTaskQPost((OS_TCB *)&tcbTaskServiceRankPathWay, //向协议处理线程发送消息
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
        resultDataTSC_RankPathWay.status = TSC_CMD_STATE_IDLE;
        //表示本次信道处于满状态,直接返回错误,返回之前要释放互斥信号量
        OSMutexPost(&mutexWithTSC_RankPathWay,OS_OPT_POST_NONE,&err);
        //动作任务FIFO满员
        return LH_ERR_ACTION_CHANNEL_CMD_FULL;
    }

    //此处发送成功,释放互斥信号量
    OSMutexPost(&mutexWithTSC_RankPathWay,OS_OPT_POST_NONE,&err);
    //等待状态不是等待ACK,说明线程已经开始处理
    while(resultDataTSC_RankPathWay.status == TSC_CMD_STATE_WAIT_ACK)
    {
        CoreDelayMinTick();
    }

    //只要这个状态变化了,就代表程序收到了ACK,就可以返回
    return LH_ERR_NONE;
}

//试管架传送带初始化
LH_ERR TSC_RankPathWayInitWhileAck(void)
{
    return TSC_RankPathWaySendCommandWhileAck(TSC_RANK_PATH_WAY_INIT,0,0,0,0,0);
}

//试管架传送带移动到指定的试管位,试管位参数取值0到5,当设置为6时,代表移动到下一个架子的1号位
//参数取值1-10
LH_ERR TSC_RankPathWayMove2SpecialTubeWhileAck(uint8_t tubeIndex)
{
    return TSC_RankPathWaySendCommandWhileAck(TSC_RANK_PATH_WAY_MOVE_2_SPECIAL_TUBE,tubeIndex,0,0,0,0);
}

//试管架传送到回退一整个试管架位置,例如当前试管位为3,调用该指令,回退到上一个试管架的3号位
LH_ERR TSC_RankPathWayBackOneRackWhileAck(void)
{
    return TSC_RankPathWaySendCommandWhileAck(TSC_RANK_PATH_WAY_BACK_ONE_RACK,0,0,0,0,0);
}

/***********************************************试管架传送带动作****************************************************************/







