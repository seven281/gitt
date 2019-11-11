#ifndef __TESTPROC_WASH_H_
#define __TESTPROC_WASH_H_
#include "TSC_Inc.h"
#include "TestProc_Typedef.h"

#define WASH_STOP_WORK_WASH12                         (0x01)
#define WASH_STOP_WORK_WASH34                         (0x02)
#define WASH_STOP_WORK_WASH56                         (0x04)
#define WASH_STOP_WORK_LIQUIDA                        (0x10)
#define WASH_STOP_WORK_MIX                            (0x20)
#define WASH_STOP_WORK_WASH_MASK                      (0x07)
#define WASH_STOP_WORK_MASK                           (WASH_STOP_WORK_WASH12|WASH_STOP_WORK_WASH34|     \
                                                       WASH_STOP_WORK_WASH56|WASH_STOP_WORK_LIQUIDA|WASH_STOP_WORK_MIX)
typedef enum
{
    WASH_WORK_RESET,
    WASH_WORK_ROTATE,
    WASK_WORK_WHILE_STOP,
    WASH_WORK_WASH_STANDBY,
    WASH_WORK_MAX
}WASH_WORK_E;

typedef enum
{
    MEASUREROOM_WORK_IDLE = 0,     // 0  
    MEASUREROOM_WORK_FULLOPEN,
    MEASUREROOM_WORK_HALFOPEN,
    MEASUREROOM_WORK_CLOSE,
    MEASUREROOM_WORK_MEASURE,
    MEASUREROOM_WORK_MAX
}MEASUREROOM_WORK_E;

extern int TestProc_WashInit(void);
extern int TestProc_WashStartWork(WASH_WORK_E eWork, uint8_t *pu8Param, uint8_t u8Size);
extern TESTPROC_API_RESULT_E TestProc_WashResetFSM(uint8_t u8StartReset);
extern void TestProc_WashFSM(OS_TICK tick);
extern int TestProc_MeasureRoomStartWork(MEASUREROOM_WORK_E eWork, uint8_t *pu8Param, uint8_t u8Size);
extern void TestProc_MeasureRoomFSM(OS_TICK tick);

#endif // End of “ifndef __TESTPROC_WASH_H_”

