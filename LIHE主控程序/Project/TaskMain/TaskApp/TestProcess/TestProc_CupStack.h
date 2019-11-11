#ifndef __TESTPROC_CUPSTACK_H_
#define __TESTPROC_CUPSTACK_H_
#include "TSC_Inc.h"
#include "TestProc_Typedef.h"

#define TESTPROC_CUPSTACK_ROW_MAX                   (14u)  // 14行
#define TESTPROC_CUPSTACK_COLUMN_MAX                (10u)  // 10列

typedef enum
{
    CUPSTACK_WORK_RESET = 0,
    CUPSTACK_WORK_PUSH_TO_POSTION,
    CUPSTACK_WORK_CHANGE_DISK,
    CUPSTACK_WORK_MAX
}CUPSTACK_WORK_E;

extern int TestProc_CupStackInit(void);
extern int TestProc_CupStackStartWork(CUPSTACK_WORK_E eWork, uint8_t *pu8Param, uint8_t u8Size);
extern void TestProc_CupStackFSM(OS_TICK tick);

#endif // End of “ifndef __TESTPROC_CUPSTACK_H_”

