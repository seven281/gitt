#ifndef __TESTPROC_HANDCUP_H_
#define __TESTPROC_HANDCUP_H_
#include "TSC_Inc.h"
#include "TestProc_Typedef.h"

typedef enum
{
    HANDCUP_WORK_RESET,
    HANDCUP_WORK_CATCH_NEWCUP,
    HANDCUP_WORK_TO_REACT,
    HANDCUP_WORK_PUT_REACT,
    HANDCUP_WORK_CUP_REACT_TO_WASH,
    HANDCUP_WORK_CUP_PUT_TO_WASH,
    HANDCUP_WORK_CUP_TO_MEASUREROOM,
    HANDCUP_WORK_CUP_PUT_MEASUREROOM,
    HANDCUP_WORK_MEASUREROOM_ABANDON,
    HANDCUP_WORK_CUP_WASH_TO_REACT,
    HANDCUP_WORK_CUP_WASH_PUT_REACT,
    HANDCUP_WORK_MAX
}HANDCUP_WORK_E;
    
extern int TestProc_HandCupInit(void);
extern int TestProc_HandCupStartWork(HANDCUP_WORK_E eWork, uint8_t *pu8Param, uint8_t u8Size);
extern void TestProc_HandCupFSM(OS_TICK tick);

#endif // End of “ifndef __TESTPROC_HANDCUP_H_”

