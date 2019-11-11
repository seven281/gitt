#ifndef __BSP_USER_H_
#define __BSP_USER_H_
#include "stm32f4xx.h"
#include  <cpu_core.h>
#include  <os.h>
#include  <lib_def.h>

//获取当前指定时钟源的时钟频率
CPU_INT32U   BSP_ClkFreqGet(void);

//定时器使能
void  BSP_OS_TickEnable (void);

//定时器禁用
void  BSP_OS_TickDisable(void);




#endif
