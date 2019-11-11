#include "TaskServiceConfig.h"
#include "TSC_Inc.h"
#include "ActionImplementInc.h"

//任务控制块,8字节对齐
#pragma pack(8)
//任务堆栈
CPU_STK stackBufferTaskServiceCan2Recv[STK_SIZE_TASK_SERVICE_CAN2_RECV];
#pragma pack()

OS_TCB tcbTaskServiceCan2Recv;


//CAN总线接收回调函数
void OS_Can2ReceiveDataCallFunc(MCU_CAN_RECEIVE_BUFFER_UNIT* unitRxMsg)
{
    //将数据转发到处理线程
    OS_ERR err;

    //将申请的数据转发到协议处理线程
    OSTaskQPost((OS_TCB *)&tcbTaskServiceCan2Recv, //向协议处理线程发送消息
                (void *)unitRxMsg,
                (OS_MSG_SIZE)sizeof(MCU_CAN_RECEIVE_BUFFER_UNIT*),
                (OS_OPT)OS_OPT_POST_FIFO,
                (OS_ERR *)&err);

    //检测是否发送成功
    if (err != OS_ERR_NONE)
    {
        //发送失败
        unitRxMsg->waitProcessFlag = 0;
    }
}

//任务函数
void TaskServiceFuncCan2Recv(void *p_arg)
{
    OS_ERR err;
    p_arg = p_arg;
    //任务接收数据长度
	OS_MSG_SIZE size;
	//接收的数据指针
	MCU_CAN_RECEIVE_BUFFER_UNIT *unitRxMsg = NULL;
    //初始化协议栈
    Can2ProcDataInit();
    while (1)
	{
        //请求消息,堵塞访问,协议处理任务必须实时性极高
		unitRxMsg = OSTaskQPend((OS_TICK)0,
								 (OS_OPT)OS_OPT_PEND_BLOCKING,
								 (OS_MSG_SIZE *)&size,
								 (CPU_TS *)0,
								 (OS_ERR *)&err);

		if ((unitRxMsg != NULL)&&(err == OS_ERR_NONE))
		{
            //数据单元处理
            Can2RecvProc(unitRxMsg);
		}
    }
}






