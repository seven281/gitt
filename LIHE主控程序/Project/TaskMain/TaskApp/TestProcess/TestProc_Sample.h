#ifndef __TESTPROC_SAMPLE_H_
#define __TESTPROC_SAMPLE_H_
#include "TSC_Inc.h"
#include "TestProc_Typedef.h"

#define SAMPLE_POS_1             (1u)       // 1号位置
#define SAMPLE_POS_2             (2u)       // 2号位置
#define SAMPLE_POS_3             (3u)       // 3号位置
#define SAMPLE_POS_4             (4u)       // 4号位置
#define SAMPLE_POS_5             (5u)       // 5号位置
#define SAMPLE_POS_RECYCLE       (6u)       // 6回收位置

typedef enum
{
    SAMPLE_WORK_RESET,
    SAMPLE_WORK_TO_RACK,
    SAMPLE_WORK_TO_ABSORB,
    SAMPLE_WORK_TO_INJECT,
    SAMPLE_WORK_FORCE_CLEAN,
    SAMPLE_WORK_CLEAN,                  // 11
    SAMPLE_WORK_MAX
}SAMPLE_WORK_E;

typedef enum
{
    CONVEYORBELT_WORK_MOVETOPOS,    
    CONVEYORBELT_WORK_TO_RECYCLE,
    CONVEYORBELT_WORK_MAX
}CONVEYORBELT_WORK_E;

extern int TestProc_SampleInit(void);
extern int TestProc_SampleStartWork(SAMPLE_WORK_E eWork, uint8_t *pu8Param, uint8_t u8Size);
extern void TestProc_SampleFSM(OS_TICK tick);
extern int TestProc_ConveyorBeltStartWork(CONVEYORBELT_WORK_E eWork, uint8_t *pu8Param, uint8_t u8Size);
extern void TestProc_SampleCBFSM(OS_TICK tick);

#endif // End of “ifndef __TESTPROC_SAMPLE_H_

