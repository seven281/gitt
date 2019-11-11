#ifndef __TESTPROC_REACT_H_
#define __TESTPROC_REACT_H_
#include "TSC_Inc.h"
#include "TestProc_Typedef.h"

#define REACT_TRIGGER_1_STEP                (23u)
#define REACT_TRIGGER_2_STEP                (2u)
#define REACT_TRIGGER_3_STEP                (9u)
#define REACT_TRIGGER_4_STEP                (31u)

#define REACT_RM1_MIX                       (0x01)
#define REACT_RM2_MIX                       (0x02)
#define REACT_MIX_MASK                      (0x03)

typedef enum
{
    REACT_WORK_RESET,       // 0
    REACT_WORK_TEST,        // 1
    REACT_WORK_SIMULATE,    // 2
    REACT_WORK_STEP1_RUN,   // 3
    REACT_WORK_STEP2_RUN,   // 4
    REACT_WORK_STEP3_RUN,   // 5
    REACT_WORK_STEP4_RUN,   // 6
    REACT_WORK_STOP,        // 7
    REACT_WORK_MIX,         // 8
    REACT_WORK_MAX
}REACT_WORK_E;

extern int TestProc_ReactInit(void);
extern int TestProc_ReactStartWork(REACT_WORK_E eWork, uint8_t *pu8Param, uint8_t u8Size);
extern void TestProc_ReactFSM(OS_TICK tick);

#endif // End of “ifndef __TESTPROC_REACT_H_”

