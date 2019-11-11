#ifndef __CORE_DELAY_H_
#define __CORE_DELAY_H_
#include "stm32f4xx.h"
#include "bsp_user.h"
#include  <cpu_core.h>
#include  "os.h"
#include "os_app_hooks.h"


//延时函数初始化
void CoreDelayInit(void);

//延时函数延时毫秒
void CoreDelayMs(uint16_t nms);

//延时函数延时微秒
void CoreDelayUs(uint16_t nus);

//延时操作系统最小时间基数
void CoreDelayMinTick(void);

#endif




