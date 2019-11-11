#ifndef __TESTPROC_REAGENT_H_
#define __TESTPROC_REAGENT_H_
#include "TSC_Inc.h"
#include "TestProc_Typedef.h"

typedef enum
{
    REAGENT_WORK_RESET,
    REAGENT_WORK_ABSORB_RS1_R1_PREPARE,     // 1 第1部加试剂1准备
    REAGENT_WORK_ABSORB_RS1_R1,             // 2
    REAGENT_WORK_ABSORB_RS1_R2,             // 3
    REAGENT_WORK_ABSORB_RS1_R2_PREPARE,     // 4 第1部加试剂2准备
    REAGENT_WORK_ABSORB_RS1_R2_2STEP,       // 5
    REAGENT_WORK_INJECT_RS1,                // 6 第1步注试剂
    REAGENT_WORK_ABSORB_RS2_R2_PREPARE,     // 7 第2部加试剂2准备
    REAGENT_WORK_ABSORB_RS2_R2,             // 8
    REAGENT_WORK_ABSORB_RS2_R3_CLEAN,       // 9 第2部加试剂3准备
    REAGENT_WORK_ABSORB_RS2_R3_PREPARE,     // 10 第2部加试剂3准备
    REAGENT_WORK_ABSORB_RS2_R3,             // 11
    REAGENT_WORK_ABSORB_RS2_M_CLEAN,        // 12
    REAGENT_WORK_ABSORB_RS2_M_PREPARE,      // 13
    REAGENT_WORK_ABSORB_RS2_M,              // 14
    REAGENT_WORK_INJECT_RS2,                // 15
    REAGENT_WORK_MOVE_TO_REACT,             // 16
    REAGENT_WORK_MAX
}REAGENT_WORK_E;

extern int TestProc_ReagentInit(void);
extern int TestProc_ReagentStartWork(REAGENT_WORK_E eWork, uint8_t *pu8Param, uint8_t u8Size);
extern void TestProc_ReagentFSM(OS_TICK tick);


#endif // End of “ifndef __TESTPROC_REAGENT_H_”

