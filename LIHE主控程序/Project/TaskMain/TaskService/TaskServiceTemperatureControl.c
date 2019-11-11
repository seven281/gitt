#include "TaskServiceConfig.h"
#include "TSC_Inc.h"
#include "ActionImplementInc.h"

//任务控制块,8字节对齐
#pragma pack(8)
//任务堆栈
CPU_STK stackBufferTaskServiceTempCtrl[STK_SIZE_TASK_SERVICE_TEMP_CTRL];
#pragma pack()

OS_TCB tcbTaskServiceTempCtrl;

//串口指令处理
static void TaskServiceCommandProc_TempCtrl_UartRS1(SYSTEM_CMD_UART_RS1* uartCommand);

//CAN口指令处理
static void TaskServiceCommandProc_TempCtrl_Can1(SYSTEM_CMD_CAN1* canCommand);

//网口指令处理
static void TaskServiceCommandProc_TempCtrl_NetW5500(SYSTEM_CMD_NET_W5500* netCommand);

//自身指令处理
static void TaskServiceCommandProc_TempCtrl_Self(SYSTEM_CMD_SELF* selfCommand);

//任务函数
void TaskServiceFuncTempCtrl(void *p_arg)
{
    p_arg = p_arg;
    //任务接收指令
    SYSTEM_CMD *taskSystemRecvCommandPtr = NULL;
    //任务接收数据长度
    OS_MSG_SIZE size;
    //操作系统错误代码
    OS_ERR err;
    //开始主线程
    while(1)
    {
        //任务从队列中读取指令
        taskSystemRecvCommandPtr = OSTaskQPend((OS_TICK)0,
                                               (OS_OPT)OS_OPT_PEND_BLOCKING,
                                               (OS_MSG_SIZE *)&size,
                                               (CPU_TS *)0,
                                               (OS_ERR *)&err);
        if ((taskSystemRecvCommandPtr != NULL) && (err == OS_ERR_NONE))
        {
            //判定指令源
            if (SYSTEM_CMD_SRC_UART_RS1 == taskSystemRecvCommandPtr->commandSrc)
            {
                //串口指令源
                SYSTEM_CMD_UART_RS1* uartRS1CommandPtr = (SYSTEM_CMD_UART_RS1*)(taskSystemRecvCommandPtr->systemCommandDataPtr);
                //对接收到的数据进行数据处理
                TaskServiceCommandProc_TempCtrl_UartRS1(uartRS1CommandPtr);
                //处置完成,执行后续操作
                UserMemFree(SRAM_CCM,uartRS1CommandPtr);
            }
            else if (SYSTEM_CMD_SRC_SELF == taskSystemRecvCommandPtr->commandSrc)
            {
                //自身指令源
                SYSTEM_CMD_SELF* selfCommandPtr =  (SYSTEM_CMD_SELF*)(taskSystemRecvCommandPtr->systemCommandDataPtr);
                //对接收到的数据进行处理
                TaskServiceCommandProc_TempCtrl_Self(selfCommandPtr);
                //处置完成,执行后续操作
                UserMemFree(SRAM_CCM,selfCommandPtr);
            }
            else if(SYSTEM_CMD_SRC_NET_W5500 == taskSystemRecvCommandPtr->commandSrc)
            {
                //网口指令源
                SYSTEM_CMD_NET_W5500* netW5500CommandPtr =  (SYSTEM_CMD_NET_W5500*)(taskSystemRecvCommandPtr->systemCommandDataPtr);
                //对接收到的数据进行处理
                TaskServiceCommandProc_TempCtrl_NetW5500(netW5500CommandPtr);
                //处置完成,执行后续操作
                UserMemFree(SRAM_CCM,netW5500CommandPtr);
            }
            else if(SYSTEM_CMD_SRC_CAN1 == taskSystemRecvCommandPtr->commandSrc)
            {
                //CAN1口指令源
                SYSTEM_CMD_CAN1* can1CommandPtr =  (SYSTEM_CMD_CAN1*)(taskSystemRecvCommandPtr->systemCommandDataPtr);
                //对接收到的数据进行处理
                TaskServiceCommandProc_TempCtrl_Can1(can1CommandPtr);
                //处置完成,执行后续操作
                UserMemFree(SRAM_CCM,can1CommandPtr);
            }
            else
            {
                //不支持的指令源,串口报警
                TaskServiceSerialErrMsgShow(LH_ERR_MAIN_CMD_SRC_UNSUPPORT);
            }
            //处理完成,释放指令内存
            taskSystemRecvCommandPtr->systemCommandDataPtr = NULL;
            UserMemFree(SRAM_CCM,taskSystemRecvCommandPtr);
            taskSystemRecvCommandPtr = NULL;
        }
    }
}


/*****************************************************串口指令处理*****************************************************/

static void TaskServiceCommandProc_TempCtrl_UartRS1(SYSTEM_CMD_UART_RS1* uartCommand)
{

}

/********************************************************************************************************************/

/*****************************************************CAN指令处理*****************************************************/

static void TaskServiceCommandProc_TempCtrl_Can1(SYSTEM_CMD_CAN1* canCommand)
{

}

/********************************************************************************************************************/

/*******************************************************网口指令处理**************************************************/

static void TaskServiceCommandProc_TempCtrl_NetW5500(SYSTEM_CMD_NET_W5500* netCommand)
{

}

/********************************************************************************************************************/

/*******************************************************自身指令处理**************************************************/

static void TaskServiceCommandProc_TempCtrl_Self(SYSTEM_CMD_SELF* selfCommand)
{

}

/********************************************************************************************************************/





