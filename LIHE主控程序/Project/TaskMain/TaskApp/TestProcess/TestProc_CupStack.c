
#include "TestProc_CupStack.h"

typedef enum
{
    FSM_CUPSTACK_RESET_IDLE = 0,
    FSM_CUPSTACK_RESET_ING,
}FSM_CUPSTACK_RESET_E;

typedef enum
{
    FSM_CUPSTACK_IDLE = 0,
    FSM_CUPSTACK_RESET,
    FSM_CUPSTACK_PUSH_NEW_CUPSTACK,                     // 2
    FSM_CUPSTACK_PUSH_TO_POSTION,                       // 3
    FSM_CUPSTACK_PUSH_TO_POSTION_ING,                   // 4
    FSM_CUPSTACK_CHANGE_RECYCLE_DISK,                   // 5
    FSM_CUPSTACK_CHANGE_RECYCLE_ING,                    // 6
    FSM_CUPSTACK_CHANGE_UPLOAD_NEW_DISK,                // 7
    FSM_CUPSTACK_CHANGE_PUSH_NEW_DISK,                  // 8
    FSM_CUPSTACK_MAX
}FSM_CUPSTACK_E;
    
typedef struct
{
    FSM_CUPSTACK_E eCupStackState;
    OS_TICK TimeOut;
    TESTPROC_WARNING_E  eWarning;
}CUPSTACK_WORK_T;

extern int32_t TestProc_IsAgeingMode(void);

#define TestProc_CupStackCleanFSM()                    \
    do                                                 \
    {                                                  \
        FSM_CupStack = FSM_CUPSTACK_IDLE;              \
    }while(0)

static FSM_CUPSTACK_E FSM_CupStack;
static FSM_CUPSTACK_RESET_E FSM_CupStack_Reset;
static OS_TICK g_CupStackTimeStart = TESTPROC_TICK_INVALID;
static CUPSTACK_WORK_E g_eCupStackWorking = CUPSTACK_WORK_RESET;
static const char g_strCupStackWorkName[CUPSTACK_WORK_MAX][16] =
{
    "Reset",
    "PushToPosion",
    "ChangeDisk"
};
static const CUPSTACK_WORK_T g_stCupStackState[CUPSTACK_WORK_MAX] = 
{
    { FSM_CUPSTACK_RESET,                   TESTPROC_TICK_INVALID,          TESTPROC_WARNING_NONE },
    { FSM_CUPSTACK_PUSH_NEW_CUPSTACK,       30000,                          TESTPROC_WARNING_TIMEOUT_CUPSTACK_PUSH },
    { FSM_CUPSTACK_CHANGE_RECYCLE_DISK,     18000,                          TESTPROC_WARNING_TIMEOUT_CUPSTACK_CHANGE }
};

static const TESTPROC_WARNING_E g_eCupStackActionWarning[FSM_CUPSTACK_MAX] = 
{
    /*00*/ TESTPROC_WARNING_NONE,                           // FSM_CUPSTACK_IDLE
    /*01*/ TESTPROC_WARNING_NONE,                           // FSM_CUPSTACK_RESET
    /*02*/ TESTPROC_WARNING_NONE,                           // FSM_CUPSTACK_PUSH_NEW_CUPSTACK
    /*03*/ TESTPROC_WARNING_API_NG_CUPSTACK_UPLOAD,         // FSM_CUPSTACK_PUSH_TO_POSTION
    /*04*/ TESTPROC_WARNING_API_NG_CUPSTACK_PUSH,           // FSM_CUPSTACK_PUSH_TO_POSTION_ING
    /*05*/ TESTPROC_WARNING_NONE,                           // FSM_CUPSTACK_CHANGE_RECYCLE_DISK
    /*06*/ TESTPROC_WARNING_API_NG_CUPSTACK_RECYCLE,        // FSM_CUPSTACK_CHANGE_RECYCLE_ING
    /*07*/ TESTPROC_WARNING_API_NG_CUPSTACK_UPLOAD,         // FSM_CUPSTACK_CHANGE_UPLOAD_NEW_DISK
    /*08*/ TESTPROC_WARNING_API_NG_CUPSTACK_PUSH,           // FSM_CUPSTACK_CHANGE_PUSH_NEW_DISK
};

int TestProc_CupStackInit(void)
{
    //TestProc_Api_CupStackInit();

    FSM_CupStack = FSM_CUPSTACK_IDLE;
    FSM_CupStack_Reset = FSM_CUPSTACK_RESET_IDLE;

    return TESTPROC_RESULT_OK;
}

int TestProc_CupStackStartWork(CUPSTACK_WORK_E eWork, uint8_t *pu8Param, uint8_t u8Size)
{
    int result = TESTPROC_RESULT_OK;
    OS_ERR err = OS_ERR_NONE;

    if (FSM_CUPSTACK_IDLE == FSM_CupStack && eWork < CUPSTACK_WORK_MAX)
    {
        FSM_CupStack = g_stCupStackState[eWork].eCupStackState;
        g_eCupStackWorking = eWork;
        g_CupStackTimeStart = OSTimeGet(&err);
        DebugMsg("**CupStackWork >> %s\r\n", g_strCupStackWorkName[eWork]);
    }
    else
    {
        result = TESTPROC_RESULT_ERR_STATUS;
        DebugMsg("E>StartCupStack:%d,%d\r\n", FSM_CupStack, eWork);
    }

    return result;
}

static TESTPROC_API_RESULT_E TestProc_CupStackResetFSM(void)
{
    TESTPROC_API_RESULT_E eActionResult = TESTPROC_API_RESULT_WAIT;
    LH_ERR eErr = LH_ERR_NONE;

    if(FSM_CUPSTACK_RESET_IDLE == FSM_CupStack_Reset)
    {
        FSM_CupStack_Reset = FSM_CUPSTACK_RESET_ING;
        TestProc_Api_CupStackReset();
        
    }    
    else if (FSM_CUPSTACK_RESET_ING == FSM_CupStack_Reset)
    {
        eActionResult = TestProc_Api_CupStackCheckResult(&eErr);
        if (TestProc_Api_IsResetCompile(eActionResult))
        {
            FSM_CupStack_Reset = FSM_CUPSTACK_RESET_IDLE;
        }
    }

    return eActionResult;
}

void TestProc_CupStackFSM(OS_TICK tick)
{
    TESTPROC_API_RESULT_E eCheckResult;
    int32_t  s32Result = TESTPROC_RESULT_OK;
    //LH_ERR eErrCode;
    LH_ERR eErr;
    uint8_t  u8TxBuf[8];
    FSM_CUPSTACK_E FSM_Old = FSM_CupStack;

    switch(FSM_CupStack)
    {
        //
        case FSM_CUPSTACK_RESET:
            eCheckResult = TestProc_CupStackResetFSM();
            if (TestProc_Api_IsResetCompile(eCheckResult))
            {
                u8TxBuf[0] = TESTPROC_API_RESULT_OK == eCheckResult ? OK : NG;
                TestProc_SendMsg(TESTPROC_MSG_CMD_RESET, TESTPROC_UNIT_CUPSTACK, u8TxBuf, 1);
                TestProc_CupStackCleanFSM();
            }
            break;
        //
        case FSM_CUPSTACK_PUSH_NEW_CUPSTACK:
            if (TestProc_Api_CupStackIsReady() == 1)
            {
                TestProc_SendMsg(TESTPROC_MSG_CMD_CUPSTACK_POSTION, TESTPROC_UNIT_CUPSTACK, NULL, 0);
                FSM_CupStack = FSM_CUPSTACK_IDLE;
            }
			else if (TestProc_Api_CupStackIsWaitReady() == 1)
            {
                FSM_CupStack = FSM_CUPSTACK_PUSH_TO_POSTION;
            }
            else
            {
                DebugMsg("UploadNewCupStack\r\n");
                eErr = TestProc_Api_CupStackUploadDisk();
                if (LH_ERR_NONE == eErr)
                {
                    FSM_CupStack = FSM_CUPSTACK_PUSH_TO_POSTION;
                }
                else
                {
                    s32Result = TESTPROC_RESULT_ERR_EXEC;
                    FSM_CupStack = FSM_CUPSTACK_IDLE;
                }
            }
            break;
        case FSM_CUPSTACK_PUSH_TO_POSTION:
            eCheckResult = TestProc_Api_CupStackCheckResult(&eErr);
            if (TESTPROC_API_RESULT_OK == eCheckResult)
            {
                DebugMsg("PushNewCupStack\r\n");
                eErr = TestProc_Api_CupStackPushToTestPostion();
                if (LH_ERR_NONE == eErr)
                {
                    FSM_CupStack = FSM_CUPSTACK_PUSH_TO_POSTION_ING;
                }
                else
                {
                    s32Result = TESTPROC_RESULT_ERR_EXEC;
                    FSM_CupStack = FSM_CUPSTACK_IDLE;
                }
            }
            else if (TESTPROC_API_RESULT_NG == eCheckResult)
            {
                s32Result = TESTPROC_RESULT_NG;
                FSM_CupStack = FSM_CUPSTACK_IDLE;
            }
            break;
        case FSM_CUPSTACK_PUSH_TO_POSTION_ING:
            eCheckResult = TestProc_Api_CupStackCheckResult(&eErr);
            if (TESTPROC_API_RESULT_OK == eCheckResult)
            {                
                DebugMsg("PushCupStackOK\r\n");
                TestProc_SendMsg(TESTPROC_MSG_CMD_CUPSTACK_POSTION, TESTPROC_UNIT_CUPSTACK, NULL, 0);
                FSM_CupStack = FSM_CUPSTACK_IDLE;
            }
            else if (TESTPROC_API_RESULT_NG == eCheckResult)
            {
                s32Result = TESTPROC_RESULT_NG;
                FSM_CupStack = FSM_CUPSTACK_IDLE;
            }
            break;
        //
        case FSM_CUPSTACK_CHANGE_RECYCLE_DISK:
            eErr = TestProc_Api_CupStackRecycleDisk();
            if (LH_ERR_NONE == eErr)
            {
                FSM_CupStack = FSM_CUPSTACK_CHANGE_RECYCLE_ING;
            }
            else
            {
                s32Result = TESTPROC_RESULT_ERR_EXEC;
                FSM_CupStack = FSM_CUPSTACK_IDLE;
            }
            break;
        case FSM_CUPSTACK_CHANGE_RECYCLE_ING:
            eCheckResult = TestProc_Api_CupStackCheckResult(&eErr);
            if (TESTPROC_API_RESULT_OK == eCheckResult)
            {
                FSM_CupStack = FSM_CUPSTACK_CHANGE_UPLOAD_NEW_DISK;
                eErr = TestProc_Api_CupStackUploadDisk();
                if (eErr != LH_ERR_NONE)
                {
                    s32Result = TESTPROC_RESULT_ERR_EXEC;
                    FSM_CupStack = FSM_CUPSTACK_IDLE;
                }
            }
            else if (TESTPROC_API_RESULT_NG == eCheckResult)
            {
                s32Result = TESTPROC_RESULT_NG;
                FSM_CupStack = FSM_CUPSTACK_IDLE;
            }
            break;
        case FSM_CUPSTACK_CHANGE_UPLOAD_NEW_DISK:
            eCheckResult = TestProc_Api_CupStackCheckResult(&eErr);
            if (TESTPROC_API_RESULT_OK == eCheckResult)
            {
                FSM_CupStack = FSM_CUPSTACK_CHANGE_PUSH_NEW_DISK;
                eErr = TestProc_Api_CupStackPushToTestPostion();
                if (eErr != LH_ERR_NONE)
                {
                    s32Result = TESTPROC_RESULT_ERR_EXEC;
                    FSM_CupStack = FSM_CUPSTACK_IDLE;
                }
            }
            else if (TESTPROC_API_RESULT_NG == eCheckResult)
            {
                s32Result = TESTPROC_RESULT_NG;
                FSM_CupStack = FSM_CUPSTACK_IDLE;
            }
            break;
        case FSM_CUPSTACK_CHANGE_PUSH_NEW_DISK:
            eCheckResult = TestProc_Api_CupStackCheckResult(&eErr);
            if (TESTPROC_API_RESULT_OK == eCheckResult)
            {
                TestProc_SendMsg(TESTPROC_MSG_CMD_CUPSTACK_CHANGE_OK, TESTPROC_UNIT_CUPSTACK, NULL, 0);
                FSM_CupStack = FSM_CUPSTACK_IDLE;
            }
            else if (TESTPROC_API_RESULT_NG == eCheckResult)
            {
                s32Result = TESTPROC_RESULT_NG;
                FSM_CupStack = FSM_CUPSTACK_IDLE;
            }
            break;
        default:

            break;
    }
    
    if (FSM_CUPSTACK_IDLE == FSM_CupStack)
    {
        g_CupStackTimeStart = TESTPROC_TICK_INVALID;
        if (s32Result == TESTPROC_RESULT_NG && eErr != LH_ERR_NONE && g_eCupStackActionWarning[FSM_Old] != TESTPROC_WARNING_NONE)
        {
            DebugMsg("E>CupStackWorkNG:%d,%d,%08X\r\n", g_eCupStackWorking, FSM_Old, eErr);
            *(uint16_t *)u8TxBuf[0] = g_eCupStackActionWarning[FSM_Old];
            u8TxBuf[2] = TESTPROC_UNIT_CUPSTACK;
            u8TxBuf[3] = FSM_Old;
            u8TxBuf[4] = WORD_TO_BYTE_LL(eErr);
            u8TxBuf[5] = WORD_TO_BYTE_LL(eErr);
            u8TxBuf[6] = WORD_TO_BYTE_LL(eErr);
            u8TxBuf[7] = WORD_TO_BYTE_LL(eErr);
            TestProc_SendMsg(TESTPROC_MSG_CMD_EXEC_NG, TESTPROC_UNIT_CUPSTACK, u8TxBuf, 8);
        }
    }
    else
    {
        if (tick > g_CupStackTimeStart && (tick-g_CupStackTimeStart) > g_stCupStackState[g_eCupStackWorking].TimeOut)
        {
            DebugMsg("E>CupStackWorkTimeout:%d,%d,%d,%d,%d,%d\r\n", g_eCupStackWorking, FSM_CupStack, FSM_Old, g_stCupStackState[g_eCupStackWorking].TimeOut, tick, g_CupStackTimeStart);
            g_CupStackTimeStart = TESTPROC_TICK_INVALID;
            if (g_stCupStackState[g_eCupStackWorking].eWarning < TESTPROC_WARNING_NONE)
            {
                *(uint16_t *)&u8TxBuf[0] = g_stCupStackState[g_eCupStackWorking].eWarning;
                u8TxBuf[2] = TESTPROC_UNIT_CUPSTACK;
                u8TxBuf[3] = TESTPROC_UNIT_CUPSTACK;
                TestProc_SendMsg(TESTPROC_MSG_CMD_TIMEOUT, TESTPROC_UNIT_EVENT, u8TxBuf, 4);
            }
        }
    }
    
    if (TESTPROC_RESULT_OK != s32Result)
    {
        DebugMsg("E>CupStackFSM:%d,%d,%d,%d,%08X\r\n", s32Result, FSM_CupStack, FSM_Old, eCheckResult, eErr);
    }
}


