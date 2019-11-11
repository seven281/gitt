#ifndef __TESTPROC_RDISK_H_
#define __TESTPROC_RDISK_H_
#include "TSC_Inc.h"
#include "TestProc_Typedef.h"

typedef enum
{
    TESTPROC_REAGENTDISK_OFFSET_NONE = 0,
    TESTPROC_REAGENTDISK_OFFSET_SCAN,
    TESTPROC_REAGENTDISK_OFFSET_R1,
    TESTPROC_REAGENTDISK_OFFSET_R2,    
    TESTPROC_REAGENTDISK_OFFSET_R3,    // 特殊位置
    TESTPROC_REAGENTDISK_OFFSET_M
}REAGENTDISK_OFFSET_E;

typedef enum
{
    REAGENTDISK_WORK_RESET,
    REAGENTDISK_WORK_MOVETOPOS,
    REAGENTDISK_WORK_MAX
}REAGENTDISK_WORK_E;

extern int TestProc_ReagentDiskInit(void);
extern int TestProc_ReagentDiskStartWork(REAGENTDISK_WORK_E eWork, uint8_t *pu8Param, uint8_t u8Size);
extern void TestProc_ReagentDiskFSM(OS_TICK tick);

#endif // End of “ifndef __TESTPROC_RDISK_H_”

