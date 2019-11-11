
#include "TestProc_ReagentDisk.h"

typedef enum
{
    FSM_REAGENTDISK_RESET_IDLE = 0,
    FSM_REAGENTDISK_RESET_ING,          // 试剂针复位中
}FSM_REAGENTDISK_RESET_E;

typedef enum
{
    REAGENTDISK_STATE_IDLE = 0,
    REAGENTDISK_STATE_RESET,
    REAGENTDISK_STATE_MOVETOPOS,
    REAGENTDISK_STATE_MOVETOPOS_ING,
    REAGENTDISK_STATE_MAX
}REAGENTDISK_STATE_E;

typedef struct
{
    REAGENTDISK_STATE_E eReagentDiskState;
    OS_TICK TimeOut;
    TESTPROC_WARNING_E  eWarning;
}REAGENTDISK_WORK_T;


#define TestProc_ReagentDiskCleanFSM()              \
    do                                              \
    {                                               \
        FSM_ReagentDisk = REAGENTDISK_STATE_IDLE;    \
    }while(0)

static const REAGENTDISK_WORK_T g_stReagentDiskState[REAGENTDISK_WORK_MAX] = 
{
    { REAGENTDISK_STATE_RESET,              TESTPROC_TICK_INVALID,      TESTPROC_WARNING_NONE },
    { REAGENTDISK_STATE_MOVETOPOS,          5000,                       TESTPROC_WARNING_TIMEOUT_REAGENTDISK_MOVE }        // 400
};

static const DISH_REAGENT_OFFSET g_eReagentOffset[] =
{
    DISH_REAGENT_OFFSET_NONE,
    DISH_REAGENT_OFFSET_SCAN,
    DISH_REAGENT_OFFSET_R1,
    DISH_REAGENT_OFFSET_R2,
    DISH_REAGENT_OFFSET_BEAD
};

static const TESTPROC_WARNING_E g_eReagentDiskActionWarning[REAGENTDISK_STATE_MAX] = 
{
    /*00*/ TESTPROC_WARNING_NONE,                           // REAGENTDISK_STATE_IDLE
    /*01*/ TESTPROC_WARNING_NONE,                           // REAGENTDISK_STATE_RESET
    /*02*/ TESTPROC_WARNING_NONE,                           // REAGENTDISK_STATE_MOVETOPOS
    /*03*/ TESTPROC_WARNING_API_NG_REAGENTDISK_MOVE,        // REAGENTDISK_STATE_MOVETOPOS_ING
};

static uint8_t g_u8ReagentPos = 0;
static uint8_t g_u8ReagentOffset = 0;
static REAGENTDISK_STATE_E FSM_ReagentDisk;
static FSM_REAGENTDISK_RESET_E FSM_ReagentDisk_Reset;
static OS_TICK g_ReagentDiskTimeStart = TESTPROC_TICK_INVALID;
static REAGENTDISK_WORK_E g_eReagentDiskWorking = REAGENTDISK_WORK_RESET;

/*
static const char g_strReagentDiskWorkName[REAGENTDISK_WORK_MAX][16] =
{
    "Idle",
    "Reset",
};
*/
int TestProc_ReagentDiskInit(void)
{
    //TestProc_Api_ReagentDiskInit();
    //TestProc_Api_ReagentDiskBarInit();

    FSM_ReagentDisk = REAGENTDISK_STATE_IDLE;
    FSM_ReagentDisk_Reset = FSM_REAGENTDISK_RESET_IDLE;
    
    return TESTPROC_RESULT_OK;
}

int TestProc_ReagentDiskStartWork(REAGENTDISK_WORK_E eWork, uint8_t *pu8Param, uint8_t u8Size)
{
    int result = TESTPROC_RESULT_OK;
    OS_ERR err = OS_ERR_NONE;

    if (REAGENTDISK_STATE_IDLE == FSM_ReagentDisk && eWork < REAGENTDISK_WORK_MAX)
    {
        FSM_ReagentDisk = g_stReagentDiskState[eWork].eReagentDiskState;
        g_eReagentDiskWorking = eWork;
        g_ReagentDiskTimeStart = OSTimeGet(&err);
        if (REAGENTDISK_STATE_MOVETOPOS == FSM_ReagentDisk)
        {
            g_u8ReagentPos = pu8Param[0];
            g_u8ReagentOffset = pu8Param[1];
        }
        //DebugMsg("**ReagentDiskStartWork >> %s\r\n", g_strReagentDiskWorkName[eWork]);
    }
    else
    {
        result = TESTPROC_RESULT_ERR_STATUS;
        DebugMsg("E>StartReagentDisk:%d,%d\r\n", FSM_ReagentDisk, eWork);
    }

    return result;
}

static TESTPROC_API_RESULT_E TestProc_ReagentDiskResetFSM(void)
{
    TESTPROC_API_RESULT_E eActionResult = TESTPROC_API_RESULT_WAIT;
    LH_ERR err;

    if(FSM_REAGENTDISK_RESET_IDLE == FSM_ReagentDisk_Reset)
    {
        FSM_ReagentDisk_Reset = FSM_REAGENTDISK_RESET_ING;
        TestProc_Api_ReagentDiskReset();
        
    }
    else if (FSM_REAGENTDISK_RESET_ING == FSM_ReagentDisk_Reset)
    {
        eActionResult = TestProc_Api_ReagentDiskCheckResult(&err);
        if (TestProc_Api_IsResetCompile(eActionResult))
        {
            FSM_ReagentDisk_Reset = FSM_REAGENTDISK_RESET_IDLE;
        }
    }

    return eActionResult;
}

void TestProc_ReagentDiskFSM(OS_TICK tick)
{
    TESTPROC_API_RESULT_E eCheckResult;
    int32_t  s32Result = TESTPROC_RESULT_OK;
    LH_ERR eErr;
    REAGENTDISK_STATE_E FSM_Old;
    uint8_t  u8TxBuf[8];

    FSM_Old = FSM_ReagentDisk;
    switch(FSM_ReagentDisk)
    {
        case REAGENTDISK_STATE_RESET:
            eCheckResult = TestProc_ReagentDiskResetFSM();
            if (TestProc_Api_IsResetCompile(eCheckResult))
            {
                u8TxBuf[0] = TESTPROC_API_RESULT_OK == eCheckResult ? OK : NG;
                TestProc_SendMsg(TESTPROC_MSG_CMD_RESET, TESTPROC_UNIT_REAGENTDISK, u8TxBuf, 1);
                TestProc_ReagentDiskCleanFSM();
            }
            break;
        case REAGENTDISK_STATE_MOVETOPOS:
            eErr = TestProc_Api_ReagentDiskMoveToPos(g_u8ReagentPos, g_eReagentOffset[g_u8ReagentOffset]);
            DebugMsg("ReagentDiskMove:%d,%d,%d\r\n", g_u8ReagentPos, g_eReagentOffset[g_u8ReagentOffset], tick);
            if (LH_ERR_NONE == eErr)
            {
                FSM_ReagentDisk = REAGENTDISK_STATE_MOVETOPOS_ING;
            }
            else
            {
                FSM_ReagentDisk = REAGENTDISK_STATE_IDLE;
            }
            break;
        case REAGENTDISK_STATE_MOVETOPOS_ING:
            eCheckResult = TestProc_Api_ReagentDiskCheckResult(&eErr);
            if (TESTPROC_API_RESULT_OK == eCheckResult)
            {
                DebugMsg("ReagentDiskMove OK:%d\r\n", tick);
                TestProc_SendMsg(TESTPROC_MSG_CMD_REAGENTDISK_INPOS, TESTPROC_UNIT_REAGENTDISK, NULL, 0);
                FSM_ReagentDisk = REAGENTDISK_STATE_IDLE;
            }
            else if (TESTPROC_API_RESULT_NG == eCheckResult)
            {
                DebugMsg("WARNING>RDiskMoveToPos\r\n");
                FSM_ReagentDisk = REAGENTDISK_STATE_IDLE;
            }
            break;
        case REAGENTDISK_STATE_IDLE:
        default:

            break;
    }
    
    if (REAGENTDISK_STATE_IDLE == FSM_ReagentDisk)
    {
        g_ReagentDiskTimeStart = TESTPROC_TICK_INVALID;
        if (s32Result == TESTPROC_RESULT_NG && eErr != LH_ERR_NONE && g_eReagentDiskActionWarning[FSM_Old] != TESTPROC_WARNING_NONE)
        {
            DebugMsg("E>ReagentDiskWorkNG:%d,%d,%08X\r\n", g_eReagentDiskWorking, FSM_Old, eErr);
            *(uint16_t *)u8TxBuf[0] = g_eReagentDiskActionWarning[FSM_Old];
            u8TxBuf[2] = TESTPROC_UNIT_REAGENTDISK;
            u8TxBuf[3] = FSM_Old;
            u8TxBuf[4] = WORD_TO_BYTE_LL(eErr);
            u8TxBuf[5] = WORD_TO_BYTE_LL(eErr);
            u8TxBuf[6] = WORD_TO_BYTE_LL(eErr);
            u8TxBuf[7] = WORD_TO_BYTE_LL(eErr);
            TestProc_SendMsg(TESTPROC_MSG_CMD_EXEC_NG, TESTPROC_UNIT_REAGENTDISK, u8TxBuf, 8);
        }
    }
    else
    {
        if (tick > g_ReagentDiskTimeStart && (tick-g_ReagentDiskTimeStart) > g_stReagentDiskState[g_eReagentDiskWorking].TimeOut)
        {
            DebugMsg("E>ReagentDiskWorkTimeout:%d,%d,%d,%d,%d,%d\r\n", g_eReagentDiskWorking, FSM_ReagentDisk, FSM_Old, g_stReagentDiskState[g_eReagentDiskWorking].TimeOut, tick, g_ReagentDiskTimeStart);
            g_ReagentDiskTimeStart = TESTPROC_TICK_INVALID;
            if (g_stReagentDiskState[g_eReagentDiskWorking].eWarning < TESTPROC_WARNING_NONE)
            {
                *(uint16_t *)&u8TxBuf[0] = g_stReagentDiskState[g_eReagentDiskWorking].eWarning;
                u8TxBuf[2] = TESTPROC_UNIT_REAGENTDISK;
                u8TxBuf[3] = FSM_ReagentDisk;
                TestProc_SendMsg(TESTPROC_MSG_CMD_TIMEOUT, TESTPROC_UNIT_EVENT, u8TxBuf, 4);
            }
        }
    }
    
    if (TESTPROC_RESULT_OK != s32Result)
    {
        DebugMsg("E>ReagentDiskFSM:%d,%d\r\n", s32Result, FSM_ReagentDisk);
    }
}



