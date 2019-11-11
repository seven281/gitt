#include "TSC_HandCup.h"

//抓杯机械手状态数据结构
static TSC_CMD_RESULT_DATA_HAND_CUP resultDataTSC_HandCup;
//抓杯机械手数据结构的互斥信号量
static OS_MUTEX mutexWithTSC_HandCup;

/******************************************************抓杯机械手动作************************************************************/
//查询抓杯机械手上一次的执行状态
TSC_CMD_STATE TSC_HandCupGetLastCommandStatusAndResult(LH_ERR* errCode)
{
    OS_ERR err;
    //申请信号量
    OSMutexPend (&mutexWithTSC_HandCup,0,OS_OPT_PEND_NON_BLOCKING,0,&err);
    if(err != OS_ERR_NONE)
    {
        //申请不到信号量,说明线程正在执行,等待返回
        return TSC_CMD_STATE_WAIT_RETURN;
    }
    //申请到了
    *errCode = resultDataTSC_HandCup.resultValue;
    //释放信号量
    OSMutexPost(&mutexWithTSC_HandCup,OS_OPT_POST_NONE,&err);
    //返回动作完成
    return resultDataTSC_HandCup.status;
}

//抓杯机械手数据结构初始化
void TSC_HandCupResultDataInit(void)
{
    OS_ERR err;
    //创建互斥信号量
    OSMutexCreate((OS_MUTEX*	)&mutexWithTSC_HandCup,
				  (CPU_CHAR*	)"mutexWithTSC_HandCup",
                  (OS_ERR*		)&err);	
    //状态赋值
    resultDataTSC_HandCup.dataMutexPtr = &mutexWithTSC_HandCup;
    resultDataTSC_HandCup.status = TSC_CMD_STATE_IDLE;
    resultDataTSC_HandCup.resultValue = LH_ERR_NONE;
    resultDataTSC_HandCup.dishReactionHasCup = ACTION_SENSOR_STATUS_NOT_EXIST;
    resultDataTSC_HandCup.dishWashHasCup = ACTION_SENSOR_STATUS_NOT_EXIST;
}


//动作API发送任务给动作API
static LH_ERR TSC_HandCupSendCommandWhileAck(uint16_t cmdIndex,
            int32_t param1,int32_t param2,int32_t param3,int32_t param4,int32_t param5)
{
    OS_ERR err;
    //自身消息指针
    SYSTEM_CMD* systemCommandPtr = NULL;

    do
    {
        //先申请信号量,不阻塞线程
	    OSMutexPend (&mutexWithTSC_HandCup,0,OS_OPT_PEND_NON_BLOCKING,0,&err);
        if(err != OS_ERR_NONE)
        {
            CoreDelayMinTick();
        }
    }while(err != OS_ERR_NONE);
    //此处,获取了信号量
    
    //指令状态,等待ACK
    resultDataTSC_HandCup.status = TSC_CMD_STATE_WAIT_ACK;
    //指令结果值设置为初始化状态
    resultDataTSC_HandCup.resultValue = LH_ERR_NONE;

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
    selfCommandPtr->commandDataSpecialChannel = (void*)(&resultDataTSC_HandCup);
    //将申请的指针传递给对应的任务信道做处理
    OSTaskQPost((OS_TCB *)&tcbTaskServiceHandCup, //向协议处理线程发送消息
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
        resultDataTSC_HandCup.status = TSC_CMD_STATE_IDLE;
        //表示本次信道处于满状态,直接返回错误,返回之前要释放互斥信号量
        OSMutexPost(&mutexWithTSC_HandCup,OS_OPT_POST_NONE,&err);
        //动作任务FIFO满员
        return LH_ERR_ACTION_CHANNEL_CMD_FULL;
    }

    //此处发送成功,释放互斥信号量
    OSMutexPost(&mutexWithTSC_HandCup,OS_OPT_POST_NONE,&err);
    //等待状态不是等待ACK,说明线程已经开始处理
    while(resultDataTSC_HandCup.status == TSC_CMD_STATE_WAIT_ACK)
    {
        CoreDelayMinTick();
    }

    //只要这个状态变化了,就代表程序收到了ACK,就可以返回
    return LH_ERR_NONE;
}

//抓杯机械手三维复位
LH_ERR TSC_HandCupInitWhileAck(void)
{
    return TSC_HandCupSendCommandWhileAck(TSC_HAND_CUP_INIT,0,0,0,0,0);
}

//抓杯机械手移动到反应盘
LH_ERR TSC_HandCupMove2DishReactionWhileAck(void)
{
    return TSC_HandCupSendCommandWhileAck(TSC_HAND_CUP_MOVE_2_DISH_REACTION,0,0,0,0,0);
}

//抓杯机械手移动到新盘装载上方,参数为第几行第几列的杯子
LH_ERR TSC_HandCupMove2NewDiskWhileAck(uint8_t newCupRow,uint8_t newCupCol)
{
    return TSC_HandCupSendCommandWhileAck(TSC_HAND_CUP_MOVE_2_NEW_DISK,newCupRow,newCupCol,0,0,0);
}

//抓杯机械手移动到垃圾桶上方,参数为垃圾桶1(参数0)或者垃圾桶2(参数1)
LH_ERR TSC_HandCupMove2GarbageWhileAck(ACTION_PARAM_STACK_MANAGE_GARBAGE_NO garbageNo)
{
    return TSC_HandCupSendCommandWhileAck(TSC_HAND_CUP_MOVE_2_GARBAGE,garbageNo,0,0,0,0);
}

//抓杯机械手移动到清洗盘上方
LH_ERR TSC_HandCupMove2DishWashWhileAck(void)
{
    return TSC_HandCupSendCommandWhileAck(TSC_HAND_CUP_MOVE_2_DISH_WASH,0,0,0,0,0);
}

//抓杯机械手移动到测量室上方
LH_ERR TSC_HandCupMove2MeasureRoomWhileAck(void)
{
    return TSC_HandCupSendCommandWhileAck(TSC_HAND_CUP_MOVE_2_MEASURE_ROOM,0,0,0,0,0);
}

//抓杯机械手往反应盘放杯
LH_ERR TSC_HandCupPutCup2DishReactionWhileAck(void)
{
    return TSC_HandCupSendCommandWhileAck(TSC_HAND_CUP_PUT_CUP_2_DISH_REACTION,0,0,0,0,0);
}

//抓杯机械手往垃圾桶放杯 参数为垃圾桶编号,垃圾桶1或者垃圾桶2
LH_ERR TSC_HandCupPutCup2GarbageWhileAck(ACTION_PARAM_STACK_MANAGE_GARBAGE_NO garbageNo)
{
    return TSC_HandCupSendCommandWhileAck(TSC_HAND_CUP_PUT_CUP_2_GARBAGE,garbageNo,0,0,0,0);
}

//抓杯机械手从反应盘抓杯
LH_ERR TSC_HandCupCatchCupFromDishReactionWhileAck(void)
{
    return TSC_HandCupSendCommandWhileAck(TSC_HAND_CUP_CATCH_CUP_FROM_DISH_REACTION,0,0,0,0,0);
}

//抓杯机械手从新盘上抓杯
LH_ERR TSC_HandCupCatchCupFromNewDiskWhileAck(void)
{
    return TSC_HandCupSendCommandWhileAck(TSC_HAND_CUP_CATCH_CUP_FROM_NEW_DISK,0,0,0,0,0);
}

//抓杯机械手往清洗盘放杯
LH_ERR TSC_HandCupPutCup2DishWashWhileAck(void)
{
    return TSC_HandCupSendCommandWhileAck(TSC_HAND_CUP_PUT_CUP_2_DISH_WASH,0,0,0,0,0);
}

//抓杯机械手从清洗盘抓杯
LH_ERR TSC_HandCupCatchFromDishWashWhileAck(void)
{
    return TSC_HandCupSendCommandWhileAck(TSC_HAND_CUP_CATCH_CUP_FROM_DISH_WASH,0,0,0,0,0);
}

//抓杯机械手往测量室放杯 放杯之前测量室需要半开
LH_ERR TSC_HandCupPutCup2MeasureRoomWhileAck(void)
{
    return TSC_HandCupSendCommandWhileAck(TSC_HAND_CUP_PUT_CUP_2_MEASURE_ROOM,0,0,0,0,0);
}

//抓杯机械手从测量室抓杯,抓杯之前,测量室需要全开
LH_ERR TSC_HandCupCatchFromMeasureRoomWhileAck(void)
{
    return TSC_HandCupSendCommandWhileAck(TSC_HAND_CUP_CATCH_CUP_FROM_MEASURE_ROOM,0,0,0,0,0);
}

//后续会添加抓杯机械手的状态查询,例如反应盘是否有杯 清洗盘是否有杯等



/******************************************************抓杯机械手动作************************************************************/


/*********************************************动作结果中的数据读取***************************************************/

//检查反应盘是否有杯
ACTION_SENSOR_STATUS TSC_HandCheckDishReactionHasCup(void)
{
    return resultDataTSC_HandCup.dishReactionHasCup;
}

//检查清洗盘是否有杯
ACTION_SENSOR_STATUS TSC_HandCheckDishWashHasCup(void)
{
    return resultDataTSC_HandCup.dishWashHasCup;
}


