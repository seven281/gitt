#ifndef __SYSTEM_TASK_CONFIG_H_
#define __SYSTEM_TASK_CONFIG_H_

#include "os.h"
#include "TaskServiceConfig.h"
#include "TaskAppConfig.h"


/*********************************启动任务**********************************/
//启动任务
#define NAME_TASK_START                             "Start Task"
//任务优先级
#define PRIO_TASK_START                             0
//任务堆栈大小
#define STK_SIZE_TASK_START                         8 * 32
//任务堆栈
extern CPU_STK stackBufferTaskStart[STK_SIZE_TASK_START];
//任务控制块
extern OS_TCB tcbTaskStart;
//任务函数
void TaskFuncStart(void *p_arg);
/**************************************************************************/

#endif





