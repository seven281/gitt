#ifndef __CAN1_DATA_TYPE_H_
#define __CAN1_DATA_TYPE_H_

#include "TestProc_CanConfig.h"


//CAN1处理函数
typedef LH_ERR (*AppTaskCan1CommandProcFuncPtr)(SYSTEM_CMD_CAN1* can1CmdPtr);
//CAN1信息处理的单元配置
typedef struct APP_TASK_CAN1_PROC_UNIT
{
    uint16_t command;
    AppTaskCan1CommandProcFuncPtr can1CommandProcFunc;
}APP_TASK_CAN1_PROC_UNIT;




#endif





