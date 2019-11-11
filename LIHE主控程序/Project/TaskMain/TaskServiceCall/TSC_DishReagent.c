#include "TSC_DishReagent.h"


//试剂盘状态数据结构
static TSC_CMD_RESULT_DATA_DISH_REAGENT resultDataTSC_DishReagent;

//试剂盘数据结构的互斥信号量
static OS_MUTEX mutexWithTSC_DishReagent;

/*************************************************试剂盘动作********************************************************************/
//试剂盘上一次指令的执行状态
TSC_CMD_STATE TSC_DishReagentGetLastCommandStatusAndResult(LH_ERR* errCode)
{
    OS_ERR err;
    //申请信号量
    OSMutexPend (&mutexWithTSC_DishReagent,0,OS_OPT_PEND_NON_BLOCKING,0,&err);
    if(err != OS_ERR_NONE)
    {
        //申请不到信号量,说明线程正在执行,等待返回
        return TSC_CMD_STATE_WAIT_RETURN;
    }
    //申请到了
    *errCode = resultDataTSC_DishReagent.resultValue;
    //释放信号量
    OSMutexPost(&mutexWithTSC_DishReagent,OS_OPT_POST_NONE,&err);
    //返回动作完成
    return resultDataTSC_DishReagent.status;
}

//试剂盘数据结构初始化
void TSC_DishReagentResultDataInit(void)
{
    OS_ERR err;
    //创建互斥信号量
    OSMutexCreate((OS_MUTEX*	)&mutexWithTSC_DishReagent,
				  (CPU_CHAR*	)"MUTEX_DishReagent",
                  (OS_ERR*		)&err);	
    //状态赋值
    resultDataTSC_DishReagent.dataMutexPtr = &mutexWithTSC_DishReagent;
    resultDataTSC_DishReagent.status = TSC_CMD_STATE_IDLE;
    resultDataTSC_DishReagent.resultValue = LH_ERR_NONE;
    resultDataTSC_DishReagent.offsetFlag = DISH_REAGENT_OFFSET_NONE;
    resultDataTSC_DishReagent.currentHoleIndex = 0;
}


//动作API发送任务给动作API
static LH_ERR TSC_DishReagentSendCommandWhileAck(uint16_t cmdIndex,
            int32_t param1,int32_t param2,int32_t param3,int32_t param4,int32_t param5)
{
    OS_ERR err;
    //自身消息指针
    SYSTEM_CMD* systemCommandPtr = NULL;

    do
    {
        //先申请信号量,不阻塞线程
	    OSMutexPend (&mutexWithTSC_DishReagent,0,OS_OPT_PEND_NON_BLOCKING,0,&err);
        if(err != OS_ERR_NONE)
        {
            CoreDelayMinTick();
        }
    }while(err != OS_ERR_NONE);
    //此处,获取了信号量
    
    //指令状态,等待ACK
    resultDataTSC_DishReagent.status = TSC_CMD_STATE_WAIT_ACK;
    //指令结果值设置为初始化状态
    resultDataTSC_DishReagent.resultValue = LH_ERR_NONE;

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
    selfCommandPtr->commandDataSpecialChannel = (void*)(&resultDataTSC_DishReagent);
    //将申请的指针传递给对应的任务信道做处理
    OSTaskQPost((OS_TCB *)&tcbTaskServiceDishReagent, //向协议处理线程发送消息
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
        resultDataTSC_DishReagent.status = TSC_CMD_STATE_IDLE;
        //表示本次信道处于满状态,直接返回错误,返回之前要释放互斥信号量
        OSMutexPost(&mutexWithTSC_DishReagent,OS_OPT_POST_NONE,&err);
        //动作任务FIFO满员
        return LH_ERR_ACTION_CHANNEL_CMD_FULL;
    }

    //此处发送成功,释放互斥信号量
    OSMutexPost(&mutexWithTSC_DishReagent,OS_OPT_POST_NONE,&err);
    //等待状态不是等待ACK,说明线程已经开始处理
    while(resultDataTSC_DishReagent.status == TSC_CMD_STATE_WAIT_ACK)
    {
        CoreDelayMinTick();
    }

    //只要这个状态变化了,就代表程序收到了ACK,就可以返回
    return LH_ERR_NONE;
}

//试剂盘初始化
LH_ERR TSC_DishReagentInitWhileAck(void)
{
    return TSC_DishReagentSendCommandWhileAck(TSC_DISH_REAGENT_INIT,0,0,0,0,0);
}

//试剂盘移动到指定孔位的指定偏移,参数1为孔位号  参数2为偏移 
LH_ERR TSC_DishReagentMove2SpecialHoleWithOffsetWhileAck(uint8_t holeIndex,DISH_REAGENT_OFFSET offset)
{
    return TSC_DishReagentSendCommandWhileAck(TSC_DISH_REAGENT_MOVE_2_SPECIAL_HOLE_WITH_OFFSET,holeIndex,offset,0,0,0);
}

//试剂盘相对当前位置移动指定个孔位,参数为偏移孔位个数
LH_ERR TSC_DishReagentMove2SpecialHoleStepWhileAck(uint8_t holeStep)
{
    return TSC_DishReagentSendCommandWhileAck(TSC_DISH_REAGENT_MOVE_2_SPECIAL_HOLE_STEP,holeStep,0,0,0,0);
}



/*************************************************试剂盘动作********************************************************************/


/*********************************************动作结果中的数据读取***************************************************/
//读取当前试剂盘的偏移标志
DISH_REAGENT_OFFSET TSC_DishReagentReadCurrentOffset(void)
{
    return resultDataTSC_DishReagent.offsetFlag;
}

//读取当前试剂盘的孔位序号
uint8_t TSC_DishReagentReadCurrentHoleIndex(void)
{
    return resultDataTSC_DishReagent.currentHoleIndex;
}
