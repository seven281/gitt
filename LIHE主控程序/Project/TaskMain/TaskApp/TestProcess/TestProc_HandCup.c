
#include "TestProc_HandCup.h"
#include "TestProc_CupStack.h"

#define FSM_LOG_HANDCUP_WORK                   0
#define FSM_LOG_HANDCUP_STATE                  0

typedef enum
{
    STATE_HANDCUP_IDLE,
    STATE_HANDCUP_RUNNING,
    STATE_HANDCUP_MAX
}STATE_HANDCUP_E;

typedef enum
{
    FSM_HANDCUP_RESET_IDLE = 0,
    FSM_HANDCUP_RESET_ING,
}FSM_HANDCUP_RESET_E;

typedef enum
{
    HANDCUP_STATE_IDLE = 0,
    HANDCUP_STATE_RESET,                     // 1
    // 抓新杯并移动反应盘上方
    HANDCUP_STATE_CATCH_NEWCUP,              // 2
    HANDCUP_STATE_CATCH_TO_CUPSTACK,         // 3
    HANDCUP_STATE_CATCH_ING,                 // 4
    HANDCUP_STATE_CATCH_TO_REACT,            // 5
    //抓新杯结束
    //放杯子到反应盘
    HANDCUP_STATE_PUT_REACT,                 // 6
    HANDCUP_STATE_PUT_REACT_ING,             // 7
    //放杯子结束
    //机械手测量室测量室反应杯
    HANDCUP_STATE_MEASUREROOM_ABANDON,       // 8
    HANDCUP_STATE_TO_MEASUREROOM,            // 9
    HANDCUP_STATE_CATCH_CUP,                 // 10
    HANDCUP_STATE_TO_GARBAGE,                // 11
    HANDCUP_STATE_ABANDON,                   // 12
    //移动测量室上方结束
    //移动到反应盘上方
    HANDCUP_STATE_TO_REACT,                  // 13
    HANDCUP_STATE_TO_REACT_ING,              // 14
    //移动到反应盘上方结束
    //抓取反应盘上的反应杯
    HANDCUP_STATE_CUP_REACT_TO_WASH,         // 15
    HANDCUP_STATE_REACT_TO_WASH,             // 16
    HANDCUP_STATE_PUT_WASH,                  // 17
    //抓取反应盘上的反应杯结束
    //抓取清洗盘上的反应杯到测量室
    HANDCUP_STATE_CUP_TO_MEASUREROOM,        // 18
    HANDCUP_STATE_CUP_TO_MEASUREROOM_1,      // 19
    HANDCUP_STATE_CUP_TO_MEASUREROOM_2,      // 20
    HANDCUP_STATE_CUP_TO_MEASUREROOM_ING,    // 21
    //
    HANDCUP_STATE_CUP_PUT_MEASUREROOM,       // 22
    HANDCUP_STATE_CUP_PUT_MEASUREROOM_ING,   // 23
    //抓取清洗盘上的反应杯到测量室结束
    //
    HANDCUP_STATE_CUP_PUT_TO_WASH,           // 24
    HANDCUP_STATE_CUP_PUTWASH_ING,           // 25
    //
    //抓取清洗反应杯到反应盘
    HANDCUP_STATE_CUP_WASH_TO_REACT,         // 26
    HANDCUP_STATE_CUP_TO_WASH_ING,           // 27
    HANDCUP_STATE_CUP_WASH_CATCH_ING,        // 28
    HANDCUP_STATE_CUP_TO_REACT_ING,          // 29
    //把清洗盘反应杯放入反应盘
    HANDCUP_STATE_CUP_WASH_PUT_REACT,        // 30
    HANDCUP_STATE_CUP_WASH_PUT_REACT_ING,    // 31
    //
    HANDCUP_STATE_MAX
}HANDCUP_STATE_E;

typedef struct
{
    HANDCUP_STATE_E eHandCupState;
    OS_TICK TimeOut;
    TESTPROC_WARNING_E  eWarning;
}HANDCUP_WORK_T;

static HANDCUP_STATE_E FSM_HandCup;
static OS_TICK g_HandCupTimeStart = TESTPROC_TICK_INVALID;
static HANDCUP_WORK_E g_eHandCupWorking = HANDCUP_WORK_RESET;
// Reset FSM
static FSM_HANDCUP_RESET_E FSM_HandCup_Reset;
static uint8_t g_u8CupStackRow;
static uint8_t g_u8CupStackCol;
static ACTION_PARAM_STACK_MANAGE_GARBAGE_NO g_eGarBageNO;

static const HANDCUP_WORK_T g_stHandCupWorkState[HANDCUP_WORK_MAX] = 
{
    { HANDCUP_STATE_RESET,                  TESTPROC_TICK_INVALID,          TESTPROC_WARNING_NONE },
    { HANDCUP_STATE_CATCH_NEWCUP,           3500,                           TESTPROC_WARNING_TIMEOUT_HANDCUP_CATCH_NEWCUP },    // 700 + 800 + 1300
    { HANDCUP_STATE_TO_REACT,               1300,                           TESTPROC_WARNING_TIMEOUT_HANDCUP_TO_REACT },        // 1200
    { HANDCUP_STATE_PUT_REACT,              1200,                           TESTPROC_WARNING_TIMEOUT_HANDCUP_PUT_REACT },       // 800
    { HANDCUP_STATE_CUP_REACT_TO_WASH,      2300,                           TESTPROC_WARNING_TIMEOUT_HANDCUP_REACT_TO_WASH },   // 900 + 1300
    { HANDCUP_STATE_CUP_PUT_TO_WASH,        1200,                           TESTPROC_WARNING_TIMEOUT_HANDCUP_PUT_WASH },        // 1000
    { HANDCUP_STATE_CUP_TO_MEASUREROOM,     3500,                           TESTPROC_WARNING_TIMEOUT_HANDCUP_TO_MR },           // 800 + 600
    { HANDCUP_STATE_CUP_PUT_MEASUREROOM,    1200,                           TESTPROC_WARNING_TIMEOUT_HANDCUP_PUT_MR },          // 900
    { HANDCUP_STATE_MEASUREROOM_ABANDON,    1900,                           TESTPROC_WARNING_TIMEOUT_HANDCUP_MR_ABANDON },      // 800 + 500 + 500
    { HANDCUP_STATE_CUP_WASH_TO_REACT,      3200,                           TESTPROC_WARNING_TIMEOUT_HANDCUP_WASH_TO_REACT },   // 700 + 800 + 1300
    { HANDCUP_STATE_CUP_WASH_PUT_REACT,     1200,                           TESTPROC_WARNING_TIMEOUT_HANDCUP_WASH_PUT_REAC }    // 800
};
    
static const TESTPROC_WARNING_E g_eHandCupActionWarning[HANDCUP_STATE_MAX] = 
{
    /*00*/ TESTPROC_WARNING_NONE,                           // HANDCUP_STATE_IDLE
    /*01*/ TESTPROC_WARNING_NONE,                           // HANDCUP_STATE_RESET
    /*02*/ TESTPROC_WARNING_NONE,                           // HANDCUP_STATE_CATCH_NEWCUP
    /*03*/ TESTPROC_WARNING_API_NG_TO_CUPSTACK,             // HANDCUP_STATE_CATCH_TO_CUPSTACK
    /*04*/ TESTPROC_WARNING_NONE,                           // HANDCUP_STATE_CATCH_ING
    /*05*/ TESTPROC_WARNING_API_NG_ABSORB_SAMPLE,           // HANDCUP_STATE_CATCH_TO_REACT
    /*06*/ TESTPROC_WARNING_API_NG_SAMPLE_ROTATE,           // HANDCUP_STATE_PUT_REACT
    /*07*/ TESTPROC_WARNING_NONE,                           // HANDCUP_STATE_PUT_REACT_ING
    /*08*/ TESTPROC_WARNING_API_NG_INJECT_SAMPLE,           // HANDCUP_STATE_MEASUREROOM_ABANDON
    /*09*/ TESTPROC_WARNING_NONE,                           // HANDCUP_STATE_TO_MEASUREROOM
    /*10*/ TESTPROC_WARNING_API_NG_SAMPLE_ROTATE,           // HANDCUP_STATE_CATCH_CUP
    /*11*/ TESTPROC_WARNING_API_NG_SAMPLE_FORCE_CLEAN,      // HANDCUP_STATE_TO_GARBAGE
    /*12*/ TESTPROC_WARNING_NONE,                           // HANDCUP_STATE_ABANDON
    /*13*/ TESTPROC_WARNING_NONE,                           // HANDCUP_STATE_TO_REACT
    /*14*/ TESTPROC_WARNING_NONE,                           // HANDCUP_STATE_TO_REACT_ING
    /*15*/ TESTPROC_WARNING_NONE,                           // HANDCUP_STATE_CUP_REACT_TO_WASH
    /*16*/ TESTPROC_WARNING_NONE,                           // HANDCUP_STATE_REACT_TO_WASH
    /*17*/ TESTPROC_WARNING_NONE,                           // HANDCUP_STATE_PUT_WASH
    /*18*/ TESTPROC_WARNING_NONE,                           // HANDCUP_STATE_CUP_TO_MEASUREROOM
    /*19*/ TESTPROC_WARNING_NONE,                           // HANDCUP_STATE_CUP_TO_MEASUREROOM_1
    /*20*/ TESTPROC_WARNING_NONE,                           // HANDCUP_STATE_CUP_TO_MEASUREROOM_2
    /*21*/ TESTPROC_WARNING_NONE,                           // HANDCUP_STATE_CUP_TO_MEASUREROOM_ING
    /*22*/ TESTPROC_WARNING_NONE,                           // HANDCUP_STATE_CUP_PUT_MEASUREROOM
    /*23*/ TESTPROC_WARNING_NONE,                           // HANDCUP_STATE_CUP_PUT_MEASUREROOM_ING
    /*24*/ TESTPROC_WARNING_NONE,                           // HANDCUP_STATE_CUP_PUT_TO_WASH
    /*25*/ TESTPROC_WARNING_NONE,                           // HANDCUP_STATE_CUP_PUTWASH_ING
    /*26*/ TESTPROC_WARNING_NONE,                           // HANDCUP_STATE_CUP_WASH_TO_REACT
    /*27*/ TESTPROC_WARNING_NONE,                           // HANDCUP_STATE_CUP_TO_WASH_ING
    /*28*/ TESTPROC_WARNING_NONE,                           // HANDCUP_STATE_CUP_WASH_CATCH_ING
    /*29*/ TESTPROC_WARNING_NONE,                           // HANDCUP_STATE_CUP_TO_REACT_ING
    /*30*/ TESTPROC_WARNING_NONE,                           // HANDCUP_STATE_CUP_WASH_PUT_REACT
    /*31*/ TESTPROC_WARNING_NONE,                           // HANDCUP_STATE_CUP_WASH_PUT_REACT_ING
};

#if FSM_LOG_HANDCUP_WORK == 1
static const char g_strHandCupName[HANDCUP_STATE_MAX][20] = 
{
    "Idle",             // 0
    "Reset",            // 1
    "CatchNewCup",      // 2
    "ToCupStack",       // 3
    "CatchNewIng",      // 4
    "NewToReact",       // 5
    "PutReact",         // 6
    "PutReactIng",      // 7
    "MR_Abandon",       // 8
    "To_MR",            // 9
    "Catch_MR_Cup",     // 10
    "ToGarbage",        // 11
    "AbandonCup",       // 12
    "ToReact",          // 13
    "ToReacting",       // 14
    "ReactToWash",      // 15
    "ToWash",           // 16
    "PutWash",          // 17
    "To_MR",            // 18
    "ToMR_Ing",         // 19
    "Put_MR",           // 20
    "Put-MR_Ing",       // 21
    "PutToWash",        // 22
    "PutWashIng",       // 23
    "WashToReact",      // 24
    "ToWashIng",        // 25
    "WashCatchIng",     // 26
    "WashToReactIng",   // 27
    "WashPutReact",     // 28
    "WashPutReactIng"   // 29
};
#define TestProc_HandCupPrintWork()        do {} while(0)//do { DebugMsg("**HandStartWork >> %s\r\n", g_strHandCupName[FSM_HandCup]); } while(0)
#else
#define TestProc_HandCupPrintWork()        do {} while(0)
#endif

#if (FSM_LOG_HANDCUP_STATE == 1 && FSM_LOG_HANDCUP_WORK == 1)
#define TestProc_HandCupPrintFSM(f)        do { if (FSM_HandCup != f) DebugMsg("**FSM_HandCup >> %s\r\n", g_strHandCupName[f]); } while(0)
#else
#define TestProc_HandCupPrintFSM(f)        do {} while(0)
#endif

int TestProc_HandCupInit(void)
{
    FSM_HandCup = HANDCUP_STATE_IDLE;
    FSM_HandCup_Reset = FSM_HANDCUP_RESET_IDLE;
    //TestProc_Api_HandCupInit();

    return TESTPROC_RESULT_OK;
}

int TestProc_HandCupStartWork(HANDCUP_WORK_E eWork, uint8_t *pu8Param, uint8_t u8Size)
{
    int result = TESTPROC_RESULT_OK;
    OS_ERR err = OS_ERR_NONE;

    if (HANDCUP_STATE_IDLE == FSM_HandCup && eWork < HANDCUP_WORK_MAX)
    {
        FSM_HandCup = g_stHandCupWorkState[eWork].eHandCupState;
        g_eHandCupWorking = eWork;
        g_HandCupTimeStart = OSTimeGet(&err);
        if (NULL != pu8Param)
        {
            if (HANDCUP_STATE_CATCH_NEWCUP == FSM_HandCup)
            {
                g_u8CupStackRow = pu8Param[0];
                g_u8CupStackCol = pu8Param[1];
            }
            else if (HANDCUP_STATE_MEASUREROOM_ABANDON == FSM_HandCup)
            {
                g_eGarBageNO = (0 == pu8Param[0] ? TSC_STACK_MANAGE_GARBAGE_NO1 : TSC_STACK_MANAGE_GARBAGE_NO2);
            }
        }        
        TestProc_HandCupPrintWork();
    }
    else
    {
        result = TESTPROC_RESULT_ERR_STATUS;
        DebugMsg("E>StartHandCup:%d,%d\r\n", FSM_HandCup, eWork);
    }

    return result;
}

static TESTPROC_API_RESULT_E TestProc_HandCupResetFSM(void)
{
    TESTPROC_API_RESULT_E eActionResult = TESTPROC_API_RESULT_WAIT;
    LH_ERR errCode = LH_ERR_NONE;

    if(FSM_HANDCUP_RESET_IDLE == FSM_HandCup_Reset)
    {
        TestProc_Api_HandCupReset();
        FSM_HandCup_Reset = FSM_HANDCUP_RESET_ING;
        return TESTPROC_API_RESULT_WAIT;
    }
    else if (FSM_HANDCUP_RESET_ING == FSM_HandCup_Reset)
    {
        eActionResult = TestProc_Api_HandCupCheckResult(&errCode);
        if (TestProc_Api_IsResetCompile(eActionResult))
        {
            FSM_HandCup_Reset = FSM_HANDCUP_RESET_IDLE;
        }
        return eActionResult;
    }

    return eActionResult;
}


void TestProc_HandCupFSM(OS_TICK tick)
{
    int result = TESTPROC_RESULT_OK;
    TESTPROC_API_RESULT_E eCheckResult;
    LH_ERR eErr =LH_ERR_NONE;
    uint8_t u8TxBuf[8];
    static HANDCUP_STATE_E FSM_Old = HANDCUP_STATE_IDLE;
    
    TestProc_HandCupPrintFSM(FSM_Old);
    FSM_Old = FSM_HandCup;
    switch(FSM_HandCup)
    {
        case HANDCUP_STATE_RESET:
            eCheckResult = TestProc_HandCupResetFSM();
            if (TestProc_Api_IsResetCompile(eCheckResult))
            {
                u8TxBuf[0] = TESTPROC_API_RESULT_OK == eCheckResult ? OK : NG;
                TestProc_SendMsg(TESTPROC_MSG_CMD_RESET, TESTPROC_UNIT_HANDCUP, u8TxBuf, 1);
                FSM_HandCup = HANDCUP_STATE_IDLE;
            }
            break;
        //**********取新杯到反应盘上**********
        case HANDCUP_STATE_CATCH_NEWCUP:
            if (g_u8CupStackRow < TESTPROC_CUPSTACK_ROW_MAX && g_u8CupStackCol < TESTPROC_CUPSTACK_COLUMN_MAX)
            {
                eErr = TestProc_Api_HandCupMoveToNewDisk(g_u8CupStackRow, g_u8CupStackCol);
                if (LH_ERR_NONE == eErr)
                {
                    FSM_HandCup = HANDCUP_STATE_CATCH_TO_CUPSTACK;
                }
                else
                {
                    result = TESTPROC_RESULT_ERR_EXEC;
                    FSM_HandCup = HANDCUP_STATE_IDLE;
                }
            }
            else
            {
                result = TESTPROC_RESULT_ERR_PARAM;
                FSM_HandCup = HANDCUP_STATE_IDLE;
            }
            break;
        case HANDCUP_STATE_CATCH_TO_CUPSTACK:
            eCheckResult = TestProc_Api_HandCupCheckResult(&eErr);
            if (TESTPROC_API_RESULT_OK == eCheckResult)
            {
                u8TxBuf[0] = g_u8CupStackRow;
                u8TxBuf[1] = g_u8CupStackCol;
                TestProc_SendMsg(TESTPROC_MSG_CMD_HANDCUP_ON_CUPSTACK, TESTPROC_UNIT_HANDCUP, u8TxBuf, 2);
                eErr = TestProc_Api_HandCupCatchCup(TESTPROC_API_HAND_TARGET_NEWDISK);
                if (LH_ERR_NONE == eErr)
                {
                    FSM_HandCup = HANDCUP_STATE_CATCH_ING;
                }
                else
                {
                    result = TESTPROC_RESULT_ERR_EXEC;
                    FSM_HandCup = HANDCUP_STATE_IDLE;
                }
            }
            else if (TESTPROC_API_RESULT_NG == eCheckResult)
            {
                result = TESTPROC_RESULT_NG;
                FSM_HandCup = HANDCUP_STATE_IDLE;
            }
            break;
        case HANDCUP_STATE_CATCH_ING:
            eCheckResult = TestProc_Api_HandCupCheckResult(&eErr);
            if (TESTPROC_API_RESULT_OK == eCheckResult)
            {
                eErr = TestProc_Api_HandCupMoveToTop(TESTPROC_API_HAND_TARGET_REACT);
                if (LH_ERR_NONE == eErr)
                {
                    FSM_HandCup = HANDCUP_STATE_CATCH_TO_REACT;
                }
                else
                {
                    result = TESTPROC_RESULT_ERR_EXEC;
                    FSM_HandCup = HANDCUP_STATE_IDLE;
                }
            }
            else if (TESTPROC_API_RESULT_NG == eCheckResult)
            {
                result = TESTPROC_RESULT_NG;
                FSM_HandCup = HANDCUP_STATE_IDLE;
            }
            break;
        case HANDCUP_STATE_CATCH_TO_REACT:
            eCheckResult = TestProc_Api_HandCupCheckResult(&eErr);
            if (TESTPROC_API_RESULT_OK == eCheckResult)
            {
                TestProc_SendMsg(TESTPROC_MSG_CMD_HANDCUP_NEWCUP, TESTPROC_UNIT_HANDCUP, NULL, 0);
                FSM_HandCup = HANDCUP_STATE_IDLE;
            }
            else if (TESTPROC_API_RESULT_NG == eCheckResult)
            {
                result = TESTPROC_RESULT_NG;
                FSM_HandCup = HANDCUP_STATE_IDLE;
            }
            break;
        //***************放新杯到反应盘******************
        case HANDCUP_STATE_PUT_REACT:
            eErr =TestProc_Api_HandCupPutCup(TESTPROC_API_HAND_TARGET_REACT);
            if (LH_ERR_NONE == eErr)
            {
                FSM_HandCup = HANDCUP_STATE_PUT_REACT_ING;
            }
            else
            {
                result = TESTPROC_RESULT_ERR_EXEC;
                FSM_HandCup = HANDCUP_STATE_IDLE;
            }
            break;
        case HANDCUP_STATE_PUT_REACT_ING:
            eCheckResult = TestProc_Api_HandCupCheckResult(&eErr);
            if (TESTPROC_API_RESULT_OK == eCheckResult)
            {
                if (TestProc_Api_HandCupIsCupInReact() == 0)
                {
                    DebugMsg("**NewCupNotInReact**\r\n");
                    u8TxBuf[0] = 1;
                }
                else
                {
                    u8TxBuf[0] = 1;
                }
                TestProc_SendMsg(TESTPROC_MSG_CMD_HANDCUP_CUPINREACT, TESTPROC_UNIT_HANDCUP, u8TxBuf, 1);
                FSM_HandCup = HANDCUP_STATE_IDLE;
            }
            else if (TESTPROC_API_RESULT_NG == eCheckResult)
            {
                result = TESTPROC_RESULT_NG;
                FSM_HandCup = HANDCUP_STATE_IDLE;
            }
            break;
        //**************丢反应杯到垃圾桶******************
        case HANDCUP_STATE_MEASUREROOM_ABANDON:
            eErr = TestProc_Api_HandCupMoveToTop(TESTPROC_API_HAND_TARGET_MEASUREROOM);
            if (LH_ERR_NONE == eErr)
            {
                FSM_HandCup = HANDCUP_STATE_TO_MEASUREROOM;
            }
            else
            {
                result = TESTPROC_RESULT_ERR_EXEC;
                FSM_HandCup = HANDCUP_STATE_IDLE;
            }
            break;
        case HANDCUP_STATE_TO_MEASUREROOM:
            eCheckResult = TestProc_Api_HandCupCheckResult(&eErr);
            if (TESTPROC_API_RESULT_OK == eCheckResult)
            {
                eErr = TestProc_Api_HandCupCatchCup(TESTPROC_API_HAND_TARGET_MEASUREROOM);
                if (LH_ERR_NONE == eErr)
                {
                    FSM_HandCup = HANDCUP_STATE_CATCH_CUP;
                }
                else
                {
                    result = TESTPROC_RESULT_ERR_EXEC;
                    FSM_HandCup = HANDCUP_STATE_IDLE;
                }
            }
            else if (TESTPROC_API_RESULT_NG == eCheckResult)
            {
                result = TESTPROC_RESULT_NG;
                FSM_HandCup = HANDCUP_STATE_IDLE;
            }
            break;
        case HANDCUP_STATE_CATCH_CUP:
            eCheckResult = TestProc_Api_HandCupCheckResult(&eErr);
            if (TESTPROC_API_RESULT_OK == eCheckResult)
            {
                TestProc_SendMsg(TESTPROC_MSG_CMD_HANDCUP_MEASUREROOMCATCHOK, TESTPROC_UNIT_HANDCUP, NULL, 0);
                eErr = TestProc_Api_HandCupMoveToGarbage(g_eGarBageNO);
                if (LH_ERR_NONE == eErr)
                {
                    FSM_HandCup = HANDCUP_STATE_TO_GARBAGE;
                }
                else
                {
                    result = TESTPROC_RESULT_ERR_EXEC;
                    FSM_HandCup = HANDCUP_STATE_IDLE;
                }
            }
            else if (TESTPROC_API_RESULT_NG == eCheckResult)
            {
                result = TESTPROC_RESULT_NG;
                FSM_HandCup = HANDCUP_STATE_IDLE;
            }
            break;
        case HANDCUP_STATE_TO_GARBAGE:
            eCheckResult = TestProc_Api_HandCupCheckResult(&eErr);
            if (TESTPROC_API_RESULT_OK == eCheckResult)
            {
                TestProc_SendMsg(TESTPROC_MSG_CMD_HANDCUP_ABANDON, TESTPROC_UNIT_HANDCUP, NULL, 0);
                FSM_HandCup = HANDCUP_STATE_IDLE;
            }
            else if (TESTPROC_API_RESULT_NG == eCheckResult)
            {
                result = TESTPROC_RESULT_NG;
                FSM_HandCup = HANDCUP_STATE_IDLE;
            }
            break;
        //case HANDCUP_STATE_ABANDON:
        //    eCheckResult = TestProc_Api_HandCupCheckResult(&eErr);
        //    if (TESTPROC_API_RESULT_OK == eCheckResult)
        //    {
        //        TestProc_SendMsg(TESTPROC_MSG_CMD_HANDCUP_ABANDON, TESTPROC_UNIT_HANDCUP, NULL, 0);
        //        FSM_HandCup = HANDCUP_STATE_IDLE;
        //    }
        //    else if (TESTPROC_API_RESULT_NG == eCheckResult)
        //    {
        //        result = TESTPROC_RESULT_NG;
        //        FSM_HandCup = HANDCUP_STATE_IDLE;
        //    }
        //    break;
        //**************移动到反应盘上方*******************
        case HANDCUP_STATE_TO_REACT:
            eErr =TestProc_Api_HandCupMoveToTop(TESTPROC_API_HAND_TARGET_REACT);
            if (LH_ERR_NONE == eErr)
            {
                FSM_HandCup = HANDCUP_STATE_TO_REACT_ING;
            }
            else
            {
                result = TESTPROC_RESULT_ERR_EXEC;
                FSM_HandCup = HANDCUP_STATE_IDLE;
            }
            break;
        case HANDCUP_STATE_TO_REACT_ING:
            eCheckResult = TestProc_Api_HandCupCheckResult(&eErr);
            if (TESTPROC_API_RESULT_OK == eCheckResult)
            {
                TestProc_SendMsg(TESTPROC_MSG_CMD_HANDCUP_ONREACT, TESTPROC_UNIT_HANDCUP, NULL, 0);
                FSM_HandCup = HANDCUP_STATE_IDLE;
            }
            else if (TESTPROC_API_RESULT_NG == eCheckResult)
            {
                result = TESTPROC_RESULT_NG;
                FSM_HandCup = HANDCUP_STATE_IDLE;
            }
            break;        
        //**************反应杯从反应盘移动到清洗�?*******************
        case HANDCUP_STATE_CUP_REACT_TO_WASH:
            eErr =TestProc_Api_HandCupCatchCup(TESTPROC_API_HAND_TARGET_REACT);
            if (LH_ERR_NONE == eErr)
            {
                FSM_HandCup = HANDCUP_STATE_REACT_TO_WASH;
            }
            else
            {
                result = TESTPROC_RESULT_ERR_EXEC;
                FSM_HandCup = HANDCUP_STATE_IDLE;
            }
            break;
        case HANDCUP_STATE_REACT_TO_WASH:
            eCheckResult = TestProc_Api_HandCupCheckResult(&eErr);
            if (TESTPROC_API_RESULT_OK == eCheckResult)
            {
                TestProc_SendMsg(TESTPROC_MSG_CMD_HANDCUP_REACTCATCHOK, TESTPROC_UNIT_HANDCUP, NULL, 0);
                eErr = TestProc_Api_HandCupMoveToTop(TESTPROC_API_HAND_TARGET_WASH);
                if (LH_ERR_NONE == eErr)
                {
                    FSM_HandCup = HANDCUP_STATE_PUT_WASH;
                }
                else
                {
                    result = TESTPROC_RESULT_ERR_EXEC;
                    FSM_HandCup = HANDCUP_STATE_IDLE;
                }
            }
            else if (TESTPROC_API_RESULT_NG == eCheckResult)
            {
                result = TESTPROC_RESULT_NG;
                FSM_HandCup = HANDCUP_STATE_IDLE;
            }
            break;
        case HANDCUP_STATE_PUT_WASH:
            eCheckResult = TestProc_Api_HandCupCheckResult(&eErr);
            if (TESTPROC_API_RESULT_OK == eCheckResult)
            {
                TestProc_SendMsg(TESTPROC_MSG_CMD_HANDCUP_ONWASH, TESTPROC_UNIT_HANDCUP, NULL, 0);
                FSM_HandCup = HANDCUP_STATE_IDLE;
            }
            else if (TESTPROC_API_RESULT_NG == eCheckResult)
            {
                result = TESTPROC_RESULT_NG;
                FSM_HandCup = HANDCUP_STATE_IDLE;
            }
            break;
        //**************反应杯从清洗盘移动到测量室*******************
        case HANDCUP_STATE_CUP_TO_MEASUREROOM:
            eErr =TestProc_Api_HandCupMoveToTop(TESTPROC_API_HAND_TARGET_WASH);
            if (LH_ERR_NONE == eErr)
            {
                FSM_HandCup = HANDCUP_STATE_CUP_TO_MEASUREROOM_1;
            }
            else
            {
                result = TESTPROC_RESULT_ERR_EXEC;
                FSM_HandCup = HANDCUP_STATE_IDLE;
            }
            break;
        case HANDCUP_STATE_CUP_TO_MEASUREROOM_1:
            eCheckResult = TestProc_Api_HandCupCheckResult(&eErr);
            if (TESTPROC_API_RESULT_OK == eCheckResult)
            {
                eErr = TestProc_Api_HandCupCatchCup(TESTPROC_API_HAND_TARGET_WASH);
                if (LH_ERR_NONE == eErr)
                {
                    FSM_HandCup = HANDCUP_STATE_CUP_TO_MEASUREROOM_2;
                }
                else
                {
                    result = TESTPROC_RESULT_ERR_EXEC;
                    FSM_HandCup = HANDCUP_STATE_IDLE;
                }
            }
            else if (TESTPROC_API_RESULT_NG == eCheckResult)
            {
                result = TESTPROC_RESULT_NG;
                FSM_HandCup = HANDCUP_STATE_IDLE;
            }
            break;
        case HANDCUP_STATE_CUP_TO_MEASUREROOM_2:
            eCheckResult = TestProc_Api_HandCupCheckResult(&eErr);
            if (TESTPROC_API_RESULT_OK == eCheckResult)
            {
                eErr = TestProc_Api_HandCupMoveToTop(TESTPROC_API_HAND_TARGET_MEASUREROOM);
                if (LH_ERR_NONE == eErr)
                {
                    FSM_HandCup = HANDCUP_STATE_CUP_TO_MEASUREROOM_ING;
                }
                else
                {
                    result = TESTPROC_RESULT_ERR_EXEC;
                    FSM_HandCup = HANDCUP_STATE_IDLE;
                }
            }
            else if (TESTPROC_API_RESULT_NG == eCheckResult)
            {
                result = TESTPROC_RESULT_NG;
                FSM_HandCup = HANDCUP_STATE_IDLE;
            }
            break;
        case HANDCUP_STATE_CUP_TO_MEASUREROOM_ING:
            eCheckResult = TestProc_Api_HandCupCheckResult(&eErr);
            if (TESTPROC_API_RESULT_OK == eCheckResult)
            {
                TestProc_SendMsg(TESTPROC_MSG_CMD_HANDCUP_ONMEASUREROOM, TESTPROC_UNIT_HANDCUP, NULL, 0);
                FSM_HandCup = HANDCUP_STATE_IDLE;
            }
            else if (TESTPROC_API_RESULT_NG == eCheckResult)
            {
                result = TESTPROC_RESULT_NG;
                FSM_HandCup = HANDCUP_STATE_IDLE;
            }
            break;
        //
        case HANDCUP_STATE_CUP_PUT_MEASUREROOM:
            eErr =TestProc_Api_HandCupPutCup(TESTPROC_API_HAND_TARGET_MEASUREROOM);
            if (LH_ERR_NONE == eErr)
            {
                FSM_HandCup = HANDCUP_STATE_CUP_PUT_MEASUREROOM_ING;
            }
            else
            {
                result = TESTPROC_RESULT_ERR_EXEC;
                FSM_HandCup = HANDCUP_STATE_IDLE;
            }
            break;
        case HANDCUP_STATE_CUP_PUT_MEASUREROOM_ING:
            eCheckResult = TestProc_Api_HandCupCheckResult(&eErr);
            if (TESTPROC_API_RESULT_OK == eCheckResult)
            {
                TestProc_SendMsg(TESTPROC_MSG_CMD_HANDCUP_INMEASUREROOM, TESTPROC_UNIT_HANDCUP, NULL, 0);
                FSM_HandCup = HANDCUP_STATE_IDLE;
            }
            else if (TESTPROC_API_RESULT_NG == eCheckResult)
            {
                result = TESTPROC_RESULT_NG;
                FSM_HandCup = HANDCUP_STATE_IDLE;
            }
            break;
        // 
        case HANDCUP_STATE_CUP_PUT_TO_WASH:
            eErr =TestProc_Api_HandCupPutCup(TESTPROC_API_HAND_TARGET_WASH);
            if (LH_ERR_NONE == eErr)
            {
                FSM_HandCup = HANDCUP_STATE_CUP_PUTWASH_ING;
            }
            else
            {
                result = TESTPROC_RESULT_ERR_EXEC;
                FSM_HandCup = HANDCUP_STATE_IDLE;
            }
            break;
        case HANDCUP_STATE_CUP_PUTWASH_ING:
            eCheckResult = TestProc_Api_HandCupCheckResult(&eErr);
            if (TESTPROC_API_RESULT_OK == eCheckResult)
            {
                if (TestProc_Api_HandCupIsCupInWash() == 0)
                {
                    DebugMsg("**CupNotInWash**\r\n");
                    u8TxBuf[0] = 1;
                }
                else
                {
                    u8TxBuf[0] = 1;
                }
                TestProc_SendMsg(TESTPROC_MSG_CMD_HANDCUP_INWASH, TESTPROC_UNIT_HANDCUP, u8TxBuf, 1);
                FSM_HandCup = HANDCUP_STATE_IDLE;
            }
            else if (TESTPROC_API_RESULT_NG == eCheckResult)
            {
                result = TESTPROC_RESULT_NG;
                FSM_HandCup = HANDCUP_STATE_IDLE;
            }
            break;
        // 清洗盘反应杯到反应盘上方
        case HANDCUP_STATE_CUP_WASH_TO_REACT:
            eErr =TestProc_Api_HandCupMoveToTop(TESTPROC_API_HAND_TARGET_WASH);
            if (LH_ERR_NONE == eErr)
            {
                FSM_HandCup = HANDCUP_STATE_CUP_TO_WASH_ING;
            }
            else
            {
                result = TESTPROC_RESULT_ERR_EXEC;
                FSM_HandCup = HANDCUP_STATE_IDLE;
            }
            break;
        case HANDCUP_STATE_CUP_TO_WASH_ING:
            eCheckResult = TestProc_Api_HandCupCheckResult(&eErr);
            if (TESTPROC_API_RESULT_OK == eCheckResult)
            {
                eErr = TestProc_Api_HandCupCatchCup(TESTPROC_API_HAND_TARGET_WASH);
                if (LH_ERR_NONE == eErr)
                {
                    FSM_HandCup = HANDCUP_STATE_CUP_WASH_CATCH_ING;
                }
                else
                {
                    result = TESTPROC_RESULT_ERR_EXEC;
                    FSM_HandCup = HANDCUP_STATE_IDLE;
                }
            }
            else if (TESTPROC_API_RESULT_NG == eCheckResult)
            {
                result = TESTPROC_RESULT_NG;
                FSM_HandCup = HANDCUP_STATE_IDLE;
            }
            break;
        case HANDCUP_STATE_CUP_WASH_CATCH_ING:
            eCheckResult = TestProc_Api_HandCupCheckResult(&eErr);
            if (TESTPROC_API_RESULT_OK == eCheckResult)
            {
                eErr = TestProc_Api_HandCupMoveToTop(TESTPROC_API_HAND_TARGET_REACT);
                if (LH_ERR_NONE == eErr)
                {
                    FSM_HandCup = HANDCUP_STATE_CUP_TO_REACT_ING;
                }
                else
                {
                    result = TESTPROC_RESULT_ERR_EXEC;
                    FSM_HandCup = HANDCUP_STATE_IDLE;
                }
            }
            else if (TESTPROC_API_RESULT_NG == eCheckResult)
            {
                result = TESTPROC_RESULT_NG;
                FSM_HandCup = HANDCUP_STATE_IDLE;
            }
            break;
        case HANDCUP_STATE_CUP_TO_REACT_ING:
            eCheckResult = TestProc_Api_HandCupCheckResult(&eErr);
            if (TESTPROC_API_RESULT_OK == eCheckResult)
            {
                TestProc_SendMsg(TESTPROC_MSG_CMD_HANDCUP_WASHCUPONREACT, TESTPROC_UNIT_HANDCUP, NULL, 0);
                FSM_HandCup = HANDCUP_STATE_IDLE;
            }
            else if (TESTPROC_API_RESULT_NG == eCheckResult)
            {
                result = TESTPROC_RESULT_NG;
                FSM_HandCup = HANDCUP_STATE_IDLE;
            }
            break;
        //把清洗盘反应杯放入反应盘
        case HANDCUP_STATE_CUP_WASH_PUT_REACT:
            eErr =TestProc_Api_HandCupPutCup(TESTPROC_API_HAND_TARGET_REACT);
            if (LH_ERR_NONE == eErr)
            {
                FSM_HandCup = HANDCUP_STATE_CUP_WASH_PUT_REACT_ING;
            }
            else
            {
                result = TESTPROC_RESULT_ERR_EXEC;
                FSM_HandCup = HANDCUP_STATE_IDLE;
            }
            break;
        case HANDCUP_STATE_CUP_WASH_PUT_REACT_ING:
            eCheckResult = TestProc_Api_HandCupCheckResult(&eErr);
            if (TESTPROC_API_RESULT_OK == eCheckResult)
            {
                if (TestProc_Api_HandCupIsCupInReact() == 0)
                {
                    DebugMsg("**WashCupNotInReact**\r\n");
                    u8TxBuf[0] = 1;
                }
                else
                {
                    u8TxBuf[0] = 1;
                }
                TestProc_SendMsg(TESTPROC_MSG_CMD_HANDCUP_WASHCUPINREACT, TESTPROC_UNIT_HANDCUP, u8TxBuf, 1);
                FSM_HandCup = HANDCUP_STATE_IDLE;
            }
            else if (TESTPROC_API_RESULT_NG == eCheckResult)
            {
                result = TESTPROC_RESULT_NG;
                FSM_HandCup = HANDCUP_STATE_IDLE;
            }
            break;
        case HANDCUP_STATE_IDLE:
        default:

            break;
    }
    
    if (HANDCUP_STATE_IDLE == FSM_HandCup)
    {
        g_HandCupTimeStart = TESTPROC_TICK_INVALID;
        if (result == TESTPROC_RESULT_NG && eErr != LH_ERR_NONE && g_eHandCupActionWarning[FSM_Old] != TESTPROC_WARNING_NONE)
        {
            DebugMsg("E>HandCupWorkNG:%d,%d,%08X\r\n", g_eHandCupWorking, FSM_Old, eErr);
            *(uint16_t *)u8TxBuf[0] = g_eHandCupActionWarning[FSM_Old];
            u8TxBuf[2] = TESTPROC_UNIT_HANDCUP;
            u8TxBuf[3] = FSM_Old;
            u8TxBuf[4] = WORD_TO_BYTE_LL(eErr);
            u8TxBuf[5] = WORD_TO_BYTE_LL(eErr);
            u8TxBuf[6] = WORD_TO_BYTE_LL(eErr);
            u8TxBuf[7] = WORD_TO_BYTE_LL(eErr);
            TestProc_SendMsg(TESTPROC_MSG_CMD_EXEC_NG, TESTPROC_UNIT_HANDCUP, u8TxBuf, 8);
        }
    }
    else
    {
        if (tick > g_HandCupTimeStart && (tick-g_HandCupTimeStart) > g_stHandCupWorkState[g_eHandCupWorking].TimeOut)
        {
            DebugMsg("E>HandCupWorkTimeout:%d,%d,%d,%d,%d,%d\r\n", g_eHandCupWorking, FSM_HandCup, FSM_Old, g_stHandCupWorkState[g_eHandCupWorking].TimeOut, tick, g_HandCupTimeStart);
            g_HandCupTimeStart = TESTPROC_TICK_INVALID;
            if (g_stHandCupWorkState[g_eHandCupWorking].eWarning < TESTPROC_WARNING_NONE)
            {
                *(uint16_t *)&u8TxBuf[0] = g_stHandCupWorkState[g_eHandCupWorking].eWarning;
                u8TxBuf[2] = TESTPROC_UNIT_HANDCUP;
                u8TxBuf[3] = FSM_HandCup;
                TestProc_SendMsg(TESTPROC_MSG_CMD_TIMEOUT, TESTPROC_UNIT_EVENT, u8TxBuf, 4);
            }
        }
    }

    if (TESTPROC_RESULT_OK != result)
    {
        DebugMsg("E>HandCupFSM:%d,%d,%d,%d,%d,%d,0x%08X\r\n", result, FSM_HandCup, FSM_Old, g_u8CupStackRow, g_u8CupStackCol, g_eGarBageNO, eErr);
    }
    
    return;
}


