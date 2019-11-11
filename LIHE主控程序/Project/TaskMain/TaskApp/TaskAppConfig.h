#ifndef __TASK_APP_CONFIG_H_
#define __TASK_APP_CONFIG_H_
#include <stdint.h>
#include "os.h"
#include "lib_mem.h"

/******************************************测试流程任务*******************************************/
//启动任务
#define NAME_TASK_APP_TEST_PROCESS                                                          "TestProcess_Task"
//任务优先级
#define PRIO_TASK_APP_TEST_PROCESS                                                          26
//任务堆栈大小
#define STK_SIZE_TASK_APP_TEST_PROCESS                                                      8 * 256
//任务堆栈
extern CPU_STK stackBufferTaskAppTestProcess[STK_SIZE_TASK_APP_TEST_PROCESS];
//任务控制块
extern OS_TCB tcbTaskAppTestProcess;
//任务函数
extern void TaskAppFuncTestProcess(void *p_arg);
/**********************************************************************************************/

#endif

