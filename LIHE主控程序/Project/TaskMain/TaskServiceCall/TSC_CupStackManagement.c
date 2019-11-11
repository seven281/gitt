#include "TSC_CupStackManagement.h"
#include "TSC_TestMode.h"
#include "TestProc_Typedef.h"

/*******************************************杯栈管理部分指令**************************************************/

//杯栈状态数据结构
static TSC_CMD_RESULT_DATA_STACK_MANAGE resultDataTSC_StackManage;
//杯栈数据结构的互斥信号量
static OS_MUTEX mutexWithTSC_StackManage;
//初始化杯栈管理部分的数据细节
void TSC_StackManageResultDataInit(void)
{
    OS_ERR err;
    //创建互斥信号量
    OSMutexCreate((OS_MUTEX*	)&mutexWithTSC_StackManage,
				  (CPU_CHAR*	)"mutexWithTSC_StackManage",
                  (OS_ERR*		)&err);	
    //状态赋值
    resultDataTSC_StackManage.dataMutexPtr = &mutexWithTSC_StackManage;
    resultDataTSC_StackManage.status = TSC_CMD_STATE_IDLE;
    resultDataTSC_StackManage.resultValue = LH_ERR_NONE;
    resultDataTSC_StackManage.newDiskCount = 0;
    resultDataTSC_StackManage.emptyDiskCount = 0;
    resultDataTSC_StackManage.platformNewDiskExist = ACTION_SENSOR_STATUS_NOT_EXIST;
    resultDataTSC_StackManage.platformDiskWaitTestExist = ACTION_SENSOR_STATUS_NOT_EXIST;
    resultDataTSC_StackManage.garbage1Exist = ACTION_SENSOR_STATUS_NOT_EXIST;
    resultDataTSC_StackManage.garbage2Exist = ACTION_SENSOR_STATUS_NOT_EXIST;
    resultDataTSC_StackManage.stackManageDoorStatus = MACHINE_DOOR_STATUS_CLOSE;
    
    TSC_TestMode_Selection(TSC_TEST_MODE_NORMAL);//设置测试运行模式
}

//查询上一次指令执行的状态
TSC_CMD_STATE TSC_StackManageGetLastCommandStatusAndResult(LH_ERR* errCode)
{
    OS_ERR err;
    //申请信号量
    OSMutexPend (&mutexWithTSC_StackManage,0,OS_OPT_PEND_NON_BLOCKING,0,&err);
    if(err != OS_ERR_NONE)
    {
        //申请不到信号量,说明线程正在执行,等待返回
        return TSC_CMD_STATE_WAIT_RETURN;
    }
    //申请到了
    *errCode = resultDataTSC_StackManage.resultValue;
    //释放信号量
    OSMutexPost(&mutexWithTSC_StackManage,OS_OPT_POST_NONE,&err);
    //返回动作完成
    return resultDataTSC_StackManage.status;
}

//动作API发送任务给动作API
static LH_ERR TSC_StackManageSendCommandWhileAck(uint16_t cmdIndex,
            int32_t param1,int32_t param2,int32_t param3,int32_t param4,int32_t param5)
{
    OS_ERR err;
    //自身消息指针
    SYSTEM_CMD* systemCommandPtr = NULL;

    do
    {
        //先申请信号量,不阻塞线程
	    OSMutexPend (&mutexWithTSC_StackManage,0,OS_OPT_PEND_NON_BLOCKING,0,&err);
        if(err != OS_ERR_NONE)
        {
            CoreDelayMinTick();
        }
    }while(err != OS_ERR_NONE);
    //此处,获取了信号量
    
    //指令状态,等待ACK
    resultDataTSC_StackManage.status = TSC_CMD_STATE_WAIT_ACK;
    //指令结果值设置为初始化状态
    resultDataTSC_StackManage.resultValue = LH_ERR_NONE;

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
    selfCommandPtr->commandDataSpecialChannel = (void*)(&resultDataTSC_StackManage);
    //将申请的指针传递给对应的任务信道做处理
    OSTaskQPost((OS_TCB *)&tcbTaskServiceCupStackManagement, //向协议处理线程发送消息
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
        resultDataTSC_StackManage.status = TSC_CMD_STATE_IDLE;
        //表示本次信道处于满状态,直接返回错误,返回之前要释放互斥信号量
        OSMutexPost(&mutexWithTSC_StackManage,OS_OPT_POST_NONE,&err);
        //动作任务FIFO满员
        return LH_ERR_ACTION_CHANNEL_CMD_FULL;
    }

    //此处发送成功,释放互斥信号量
    OSMutexPost(&mutexWithTSC_StackManage,OS_OPT_POST_NONE,&err);
    //等待状态不是等待ACK,说明线程已经开始处理
    while(resultDataTSC_StackManage.status == TSC_CMD_STATE_WAIT_ACK)
    {
        CoreDelayMinTick();
    }

    //只要这个状态变化了,就代表程序收到了ACK,就可以返回
    return LH_ERR_NONE;
}


//杯栈管理模块的初始化
LH_ERR TSC_StackManageInitWhileAck(void)
{
    return TSC_StackManageSendCommandWhileAck(TSC_STACK_MANAGE_INIT,0,0,0,0,0);
}

//新杯盘承载机构上传一个新盘到新盘装载平台
LH_ERR TSC_StackManageUploadNewDisk2NewDiskPlatformWhileAck(void)
{
    return TSC_StackManageSendCommandWhileAck(TSC_STACK_MANAGE_UPLOAD_NEW_DISK,0,0,0,0,0);
}

//推手前推,将一个新盘从新盘装载平台推送到测试平台
LH_ERR TSC_StackManagePushNewDisk2WaitTestPlatformWhileAck(void)
{
    return TSC_StackManageSendCommandWhileAck(TSC_STACK_MANAGE_PUSH_NEW_DISK,0,0,0,0,0);
}

//空盘装载机构抬起到承载位置,夹手自动松开将已经测试完成的空盘放到空盘装载机构上,然后空盘装载机构回到零点
LH_ERR TSC_StackManageAscendingEmptyDiskFromWaitTestPlatformWhileAck(void)
{
    return TSC_StackManageSendCommandWhileAck(TSC_STACK_MANAGE_ASCEND_EMPTY_DISK,0,0,0,0,0);
}

//杯盘管理机构的选定的垃圾桶锁定,也就是指定的垃圾桶抬起来,这样可以在测试时增加新盘和去除空盘
//抬起的垃圾桶将留在机器内部,未抬起的垃圾桶可以被清理
LH_ERR TSC_StackManageLockGarbageWhileAck(ACTION_PARAM_STACK_MANAGE_GARBAGE_NO garbageNo)
{
    return TSC_StackManageSendCommandWhileAck(TSC_STACK_MANAGE_GARBAGE_LOCK,garbageNo,0,0,0,0);
}

//杯盘管理机构的选定的垃圾桶解锁,也就是指定的垃圾桶降下去
LH_ERR TSC_StackManageUnlockGarbageWhileAck(ACTION_PARAM_STACK_MANAGE_GARBAGE_NO garbageNo)
{
    return TSC_StackManageSendCommandWhileAck(TSC_STACK_MANAGE_GARBAGE_UNLOCK,garbageNo,0,0,0,0);
}
//新杯盏门开关闭合控制
LH_ERR TSC_StackManageDoorControlWhileAck(MACHINE_DOOR_CMD doorcmd)
{
    return TSC_StackManageSendCommandWhileAck(TSC_STACK_MANAGE_DOOR_CONTROL,doorcmd,0,0,0,0);
}

/*******************************************杯栈管理部分指令**************************************************/

/*********************************************动作结果中的数据读取***************************************************/


//新盘装载平台是否有盘
ACTION_SENSOR_STATUS TSC_StackManageCheckNewDiskPlatformHasDisk(void)
{
    return resultDataTSC_StackManage.platformNewDiskExist;
}

//待测盘平台是否有盘
ACTION_SENSOR_STATUS TSC_StackManageCheckWaitTestDiskPlatformHasDisk(void)
{
    return resultDataTSC_StackManage.platformDiskWaitTestExist;
}

//垃圾桶1是否存在
ACTION_SENSOR_STATUS TSC_StackManageCheckGarbage1(void)
{
    return resultDataTSC_StackManage.garbage1Exist;
}

//垃圾桶2是否存在
ACTION_SENSOR_STATUS TSC_StackManageCheckGarbage2(void)
{
    return resultDataTSC_StackManage.garbage2Exist;
}

//检查新盘装载机构上有几个盘
uint8_t TSC_StackManageCheckNewDiskCount(void)
{
    return resultDataTSC_StackManage.newDiskCount;
}

//检查空盘装载机构上有几个盘
uint8_t TSC_StackManageChecEmptyDiskCount(void)
{
    return resultDataTSC_StackManage.emptyDiskCount;
}

//检查杯栈管理的门状态
MACHINE_DOOR_STATUS TSC_StackManageCheckDoorStatus(void)
{
    return TaskUtilServiceGetCupStackManagementDoorState();
}


/*******************************************底层驱动发送信息给APP**************************************************/
//按键状态上传给流程
LH_ERR TSC_KeyStateSendCommand(KEY_OPT opt)
{
    TESTPROC_MSM_T *pstMsg;
    OS_ERR err = OS_ERR_NONE;
    pstMsg = TestProc_AllocMemory(sizeof(TESTPROC_MSM_T));
    if (pstMsg != NULL)
    {
        pstMsg->pData = NULL;
        pstMsg->u16DataSize = sizeof(TESTPROC_MSM_T);
        pstMsg->eSrcUnit  = TESTPROC_UNIT_EVENT;
        pstMsg->eDestUnit = TESTPROC_UNIT_PROCTEST;

        if(opt == CUPSTACK_KEY)
            pstMsg->eCommand  = TESTPROC_MSG_CMD_EVENT_CUPSTACK_KEY_DOWN;

        if(opt == REAGENTDISK_KEY)
            pstMsg->eCommand  = TESTPROC_MSG_CMD_EVENT_REAGENTDISK_KEY_DOWN;

        OSTaskQPost((OS_TCB *)&tcbTaskAppTestProcess,
                    (void *)pstMsg,
                    (OS_MSG_SIZE)sizeof(TESTPROC_MSM_T),
                    (OS_OPT)OS_OPT_POST_FIFO,
                    (OS_ERR *)&err);

    }
    return LH_ERR_NONE;
}
//门状态上传流程
LH_ERR TSC_DoorStateSendCommand(DOOR_STATE_OPT state)
{
    TESTPROC_MSM_T *pstMsg;
    OS_ERR err = OS_ERR_NONE;
    pstMsg = TestProc_AllocMemory(sizeof(TESTPROC_MSM_T));
    if (pstMsg != NULL)
    {
        pstMsg->pData = NULL;
        pstMsg->u16DataSize = sizeof(TESTPROC_MSM_T);
        pstMsg->eSrcUnit  = TESTPROC_UNIT_EVENT;
        pstMsg->eDestUnit = TESTPROC_UNIT_PROCTEST;

        if(state == DOOR_STATE_OPEN)
            pstMsg->eCommand  = TESTPROC_MSG_CMD_EVENT_CUPSTACK_DOOR_OPEN;

        if(state == DOOR_STATE_CLOSE)
            pstMsg->eCommand  = TESTPROC_MSG_CMD_EVENT_CUPSTACK_DOOR_CLOSE;

        OSTaskQPost((OS_TCB *)&tcbTaskAppTestProcess,
                    (void *)pstMsg,
                    (OS_MSG_SIZE)sizeof(TESTPROC_MSM_T),
                    (OS_OPT)OS_OPT_POST_FIFO,
                    (OS_ERR *)&err);

    }
    return LH_ERR_NONE;
}