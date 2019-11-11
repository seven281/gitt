#ifndef __MCU_RANDOM_H_
#define __MCU_RANDOM_H_
#include "CoreInc.h"

#define MCU_RANDOM_LOCK()       CPU_IntDis()

#define MCU_RANDOM_UNLOCK()     CPU_IntEn()

//随机数初始化
void MCU_RandomInit(void);

//得到随机数
uint32_t MCU_RandomGetNext(void);

//生成指定范围的随机数
int MCU_RandomGetNextRange(int min, int max);

//正确计算随机数的方法
int MCU_RandomGetNextRangeReal(int min, int max);


#endif




