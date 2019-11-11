
#include "TestProc_React.h"

#if TASK_SERVICE_WITH_REAL_MACHINE == 0
#define REACT_TRIGGER_TIME_23                       (100u)      // 10ms
#define REACT_TRIGGER_TIME_02                       (300u)      // 10ms
#define REACT_TRIGGER_TIME_09                       (150u)      // 10ms
#define REACT_TRIGGER_TIME_31                       (250u)      // 10ms
#else
#define REACT_TRIGGER_TIME_23                       (410u)      // 10ms
#define REACT_TRIGGER_TIME_02                       (140u)      // 10ms
#define REACT_TRIGGER_TIME_09                       (650u)      // 10ms
#define REACT_TRIGGER_TIME_31                       (600u)      // 10ms
#endif

typedef enum
{
    REACT_STATE_IDLE = 0,           // 0
    REACT_STATE_RESET,              // 1
    REACT_STATE_TEST,               // 2
    REACT_STATE_SIMULATE,           // 3
    REACT_STATE_STEP1_RUN,          // 4
    REACT_STATE_STEP1_RUNNING,      // 5
    REACT_STATE_STEP2_RUN,          // 6
    REACT_STATE_STEP2_RUNNING,      // 7
    REACT_STATE_STEP3_RUN,          // 8
    REACT_STATE_STEP3_RUNNING,      // 9
    REACT_STATE_STEP4_RUN,          // 10
    REACT_STATE_STEP4_RUNNING,      // 11
    REACT_STATE_STOP,               // 12
    REACT_STATE_MIX,                // 13
    REACT_STATE_MIX_ING,            // 14
    REACT_STATE_MAX
}REACT_STATE_E;

typedef enum
{
    FSM_REACT_RESET_IDLE = 0,
    FSM_REACT_MIX_RESET_ING,
    FSM_REACT_MMIX_RESET_ING,
    FSM_REACT_RESET_ING,
}FSM_REACT_RESET_E;
    
typedef struct
{
    REACT_STATE_E eReactState;
    OS_TICK TimeOut;
    TESTPROC_WARNING_E  eWarning;
}REACT_WORK_T;

static OS_TMR g_stReact23Timer;
static OS_TMR g_stReact02Timer;
static OS_TMR g_stReact09Timer;
static OS_TMR g_stReact31Timer;
static uint32_t g_u32Trigger23Count;
static uint32_t g_u32Trigger02Count;
static uint32_t g_u32Trigger09Count;
static uint32_t g_u32Trigger31Count;
static REACT_STATE_E FSM_React;
static FSM_REACT_RESET_E FSM_React_Reset;
static uint8_t  g_u8MixMask;
static uint16_t g_u16MixTimeRM1 = 0;
static uint16_t g_u16MixTimeRM2 = 0;
static OS_TICK g_ReactTimeStart = TESTPROC_TICK_INVALID;
static REACT_WORK_E g_eReactWorking = REACT_WORK_RESET;

#define TestProc_ReactCleanFSM()                    \
    do                                              \
    {                                               \
        FSM_React = REACT_STATE_IDLE;               \
    }while(0)

static const REACT_WORK_T g_stReactWorkState[REACT_WORK_MAX] = 
{
    { REACT_STATE_RESET,            TESTPROC_TICK_INVALID,      TESTPROC_WARNING_NONE },
    { REACT_STATE_TEST,             TESTPROC_TICK_INVALID,      TESTPROC_WARNING_NONE },
    { REACT_STATE_SIMULATE,         TESTPROC_TICK_INVALID,      TESTPROC_WARNING_NONE },
    { REACT_STATE_STEP1_RUN,        800,                        TESTPROC_WARNING_TIMEOUT_REACT_STEP1 },     // 700
    { REACT_STATE_STEP2_RUN,        500,                        TESTPROC_WARNING_TIMEOUT_REACT_STEP2 },     // 400
    { REACT_STATE_STEP3_RUN,        500,                        TESTPROC_WARNING_TIMEOUT_REACT_STEP3 },     // 400
    { REACT_STATE_STEP4_RUN,        800,                        TESTPROC_WARNING_TIMEOUT_REACT_STEP4 },     // 700
    { REACT_STATE_STOP,             TESTPROC_TICK_INVALID,      TESTPROC_WARNING_NONE },
    { REACT_STATE_MIX,              6000,                       TESTPROC_WARNING_TIMEOUT_REACT_MIX }        // 5500
};

static const TESTPROC_WARNING_E g_eReactActionWarning[REACT_STATE_MAX] = 
{
    /*00*/ TESTPROC_WARNING_NONE,                           // REACT_STATE_IDLE
    /*01*/ TESTPROC_WARNING_NONE,                           // REACT_STATE_RESET
    /*02*/ TESTPROC_WARNING_NONE,                           // REACT_STATE_TEST
    /*03*/ TESTPROC_WARNING_NONE,                           // REACT_STATE_SIMULATE
    /*04*/ TESTPROC_WARNING_NONE,                           // REACT_STATE_STEP1_RUN
    /*05*/ TESTPROC_WARNING_API_NG_REACT_ROTATE,            // REACT_STATE_STEP1_RUN_ING
    /*06*/ TESTPROC_WARNING_NONE,                           // REACT_STATE_STEP2_RUN
    /*07*/ TESTPROC_WARNING_API_NG_REACT_ROTATE,            // REACT_STATE_STEP2_RUN_ING
    /*08*/ TESTPROC_WARNING_NONE,                           // REACT_STATE_STEP3_RUN
    /*09*/ TESTPROC_WARNING_API_NG_REACT_ROTATE,            // REACT_STATE_STEP3_RUN_ING
    /*10*/ TESTPROC_WARNING_NONE,                           // REACT_STATE_STEP4_RUN
    /*11*/ TESTPROC_WARNING_API_NG_REACT_ROTATE,            // REACT_STATE_STEP4_RUN_ING
    /*12*/ TESTPROC_WARNING_NONE,                           // REACT_STATE_STOP
    /*13*/ TESTPROC_WARNING_NONE,                           // REACT_STATE_MIX
    /*14*/ TESTPROC_WARNING_API_NG_REACT_MIX,               // REACT_STATE_MIX_ING
};

/*
static const char g_strReactWorkName[REACT_WORK_MAX][16] = 
{
    "Reset",
    "Test",
    "Simulate",
    "Step1Run",
    "Step2Run",
    "Step3Run",
    "Step4Run",
    "Stop",
    "Mix"
};
*/

static void TestProc_ReactTrigger(void *p_tmr, void *p_arg);

int TestProc_ReactInit(void)
{
    OS_ERR err;
    
    //TestProc_Api_ReactInit();

    FSM_React = REACT_STATE_IDLE;
    FSM_React_Reset = FSM_REACT_RESET_IDLE;
    g_u32Trigger23Count = 0;
    g_u32Trigger02Count = 0;
    g_u32Trigger09Count = 0;
    g_u32Trigger31Count = 0;

    OSTmrCreate(&g_stReact23Timer, "React23 Timer", REACT_TRIGGER_TIME_31, 0, OS_OPT_TMR_ONE_SHOT, TestProc_ReactTrigger, NULL, &err);
    if (OS_ERR_NONE != err)
    {
        DebugMsg("E>CreateReact23:%d\r\n", err);
    }
    OSTmrCreate(&g_stReact02Timer, "React02 Timer", REACT_TRIGGER_TIME_23, 0, OS_OPT_TMR_ONE_SHOT, TestProc_ReactTrigger, NULL, &err);
    if (OS_ERR_NONE != err)
    {
        DebugMsg("E>CreateReact2:%d\r\n", err);
    }
    OSTmrCreate(&g_stReact09Timer, "React09 Timer", REACT_TRIGGER_TIME_02, 0, OS_OPT_TMR_ONE_SHOT, TestProc_ReactTrigger, NULL, &err);
    if (OS_ERR_NONE != err)
    {
        DebugMsg("E>CreateReact9:%d\r\n", err);
    }
    OSTmrCreate(&g_stReact31Timer, "React31 Timer", REACT_TRIGGER_TIME_09, 0, OS_OPT_TMR_ONE_SHOT, TestProc_ReactTrigger, NULL, &err);
    if (OS_ERR_NONE != err)
    {
        DebugMsg("E>CreateReact31:%d\r\n", err);
    }

    return TESTPROC_RESULT_OK;
}

int TestProc_ReactStartWork(REACT_WORK_E eWork, uint8_t *pu8Param, uint8_t u8Size)
{
    int result = TESTPROC_RESULT_OK;
    OS_ERR err;

    if (REACT_WORK_STOP == eWork)
    {
        OSTmrStop(&g_stReact23Timer, OS_OPT_TMR_NONE, NULL, &err);
        OSTmrStop(&g_stReact02Timer, OS_OPT_TMR_NONE, NULL, &err);
        OSTmrStop(&g_stReact09Timer, OS_OPT_TMR_NONE, NULL, &err);
        OSTmrStop(&g_stReact31Timer, OS_OPT_TMR_NONE, NULL, &err);
        FSM_React = REACT_STATE_IDLE;
        DebugMsg("**ReactStop**\r\n");
    }
    else
    {
        if (REACT_STATE_IDLE == FSM_React && eWork < REACT_WORK_MAX)
        {
            FSM_React = g_stReactWorkState[eWork].eReactState;
            g_ReactTimeStart = OSTimeGet(&err);
            g_eReactWorking = eWork;
            if (REACT_STATE_MIX == FSM_React)
            {
                g_u8MixMask = (pu8Param[0]&REACT_MIX_MASK);
                g_u16MixTimeRM1 = *(uint16_t *)&pu8Param[1];
                g_u16MixTimeRM2 = *(uint16_t *)&pu8Param[3];
            }
            //DebugMsg("**ReactStartWork >> %s\r\n", g_strReactWorkName[eWork]);
        }
        else
        {
            result = TESTPROC_RESULT_ERR_STATUS;
            DebugMsg("E>StartReact:%d,%d\r\n", FSM_React, eWork);
        }
    }

    return result;
}

static TESTPROC_API_RESULT_E TestProc_ReactResetFSM(void)
{
    TESTPROC_API_RESULT_E eActionResult = TESTPROC_API_RESULT_WAIT;

    if(FSM_REACT_RESET_IDLE == FSM_React_Reset)
    {
        FSM_React_Reset = FSM_REACT_MIX_RESET_ING;
        TestProc_Api_ReactMixReset();
    }
    else if (FSM_REACT_MIX_RESET_ING == FSM_React_Reset)
    {
        eActionResult = TestProc_Api_ReactCheckResult();
        if (TestProc_Api_IsResetCompile(eActionResult))
        {
            FSM_React_Reset = FSM_REACT_MMIX_RESET_ING;
            TestProc_Api_ReactMMixReset();
            //DebugMsg("M_MixReset\r\n");
        }
        eActionResult = TESTPROC_API_RESULT_WAIT;
    }
    else if (FSM_REACT_MMIX_RESET_ING == FSM_React_Reset)
    {
        eActionResult = TestProc_Api_ReactMCheckResult();
        if (TestProc_Api_IsResetCompile(eActionResult))
        {
            FSM_React_Reset = FSM_REACT_RESET_ING;
            TestProc_Api_ReactReset();
        }
        eActionResult = TESTPROC_API_RESULT_WAIT;
    }
    else if (FSM_REACT_RESET_ING == FSM_React_Reset)
    {
        eActionResult = TestProc_Api_ReactCheckResult();
        if (TestProc_Api_IsResetCompile(eActionResult))
        {
            FSM_React_Reset = FSM_REACT_RESET_IDLE;
        }
    }

    return eActionResult;
}

void TestProc_ReactFSM(OS_TICK tick)
{
    TESTPROC_API_RESULT_E eCheckResult = TESTPROC_API_RESULT_WAIT;
    int32_t  s32Result = TESTPROC_RESULT_OK;
    LH_ERR eErr;
    uint8_t  u8TxBuf[8];
    REACT_STATE_E FSM_Old = FSM_React;
    OS_ERR err;

    switch(FSM_React)
    {
        case REACT_STATE_RESET:
            eCheckResult = TestProc_ReactResetFSM();
            if (TestProc_Api_IsResetCompile(eCheckResult))
            {
                u8TxBuf[0] = TESTPROC_API_RESULT_OK == eCheckResult ? OK : NG;
                TestProc_SendMsg(TESTPROC_MSG_CMD_RESET, TESTPROC_UNIT_REACT, u8TxBuf, 1);
                TestProc_ReactCleanFSM();
            }
            break;
        case REACT_STATE_TEST:
        case REACT_STATE_SIMULATE:            
            OSTmrStop(&g_stReact23Timer, OS_OPT_TMR_NONE, NULL, &err);
            OSTmrStop(&g_stReact02Timer, OS_OPT_TMR_NONE, NULL, &err);
            OSTmrStop(&g_stReact09Timer, OS_OPT_TMR_NONE, NULL, &err);
            OSTmrStop(&g_stReact31Timer, OS_OPT_TMR_NONE, NULL, &err);
            g_u32Trigger23Count = 0;
            g_u32Trigger02Count = 0;
            g_u32Trigger09Count = 0;
            g_u32Trigger31Count = 0;

            OSTmrStart(&g_stReact23Timer, &err);
            //u8TxBuf[0] = REACT_TRIGGER_1_STEP;
            //*((uint32_t *)(&u8TxBuf[1])) = g_u32Trigger23Count;
            //TestProc_SendMsg(TESTPROC_MSG_CMD_REACT_STOP, TESTPROC_UNIT_REACT, u8TxBuf, 5);
            FSM_React = REACT_STATE_IDLE;
            break;
        case REACT_STATE_STEP1_RUN:
            TestProc_Api_ReactRotatePos(REACT_TRIGGER_1_STEP);
            //DebugMsg("__&&RotatePos 23:%d\r\n", OSTimeGet(&err));
            FSM_React = REACT_STATE_STEP1_RUNNING;
            break;
        case REACT_STATE_STEP1_RUNNING:
            eCheckResult = TestProc_Api_ReactCheckResult();
            if (TESTPROC_API_RESULT_OK == eCheckResult)
            {
                //OSTmrStart(&g_stReact23Timer, &err);
                u8TxBuf[0] = REACT_TRIGGER_1_STEP;
                *((uint32_t *)(&u8TxBuf[1])) = g_u32Trigger23Count;
                TestProc_SendMsg(TESTPROC_MSG_CMD_REACT_STOP, TESTPROC_UNIT_REACT, u8TxBuf, 5);
                FSM_React = REACT_STATE_IDLE;
            }
            else if (TESTPROC_API_RESULT_NG == eCheckResult)
            {
                DebugMsg("W>ReactStep1NG\r\n");
                FSM_React = REACT_STATE_IDLE;
            }
            break;
        case REACT_STATE_STEP2_RUN:
            TestProc_Api_ReactRotatePos(REACT_TRIGGER_2_STEP);
            //DebugMsg("__&&RotatePos 2:%d\r\n", OSTimeGet(&err));
            FSM_React = REACT_STATE_STEP2_RUNNING;
            break;
        case REACT_STATE_STEP2_RUNNING:
            eCheckResult = TestProc_Api_ReactCheckResult();
            if (TESTPROC_API_RESULT_OK == eCheckResult)
            {
                u8TxBuf[0] = REACT_TRIGGER_2_STEP;
                *((uint32_t *)(&u8TxBuf[1])) = g_u32Trigger02Count;
                TestProc_SendMsg(TESTPROC_MSG_CMD_REACT_STOP, TESTPROC_UNIT_REACT, u8TxBuf, 5);
                FSM_React = REACT_STATE_IDLE;
            }
            else if (TESTPROC_API_RESULT_NG == eCheckResult)
            {
                DebugMsg("W>ReactStep2NG\r\n");
                FSM_React = REACT_STATE_IDLE;
            }
            break;
        case REACT_STATE_STEP3_RUN:
            TestProc_Api_ReactRotatePos(REACT_TRIGGER_3_STEP);            
            //DebugMsg("__&&RotatePos 9:%d\r\n", OSTimeGet(&err));
            FSM_React = REACT_STATE_STEP3_RUNNING;
            break;
        case REACT_STATE_STEP3_RUNNING:
            eCheckResult = TestProc_Api_ReactCheckResult();
            if (TESTPROC_API_RESULT_OK == eCheckResult)
            {
                u8TxBuf[0] = REACT_TRIGGER_3_STEP;
                *((uint32_t *)(&u8TxBuf[1])) = g_u32Trigger09Count;
                TestProc_SendMsg(TESTPROC_MSG_CMD_REACT_STOP, TESTPROC_UNIT_REACT, u8TxBuf, 5);
                FSM_React = REACT_STATE_IDLE;
            }
            else if (TESTPROC_API_RESULT_NG == eCheckResult)
            {
                DebugMsg("W>ReactStep3NG\r\n");
                FSM_React = REACT_STATE_IDLE;
            }
            break;
        case REACT_STATE_STEP4_RUN:
            TestProc_Api_ReactRotatePos(REACT_TRIGGER_4_STEP);            
            //DebugMsg("__&&RotatePos 31:%d\r\n", OSTimeGet(&err));
            FSM_React = REACT_STATE_STEP4_RUNNING;
            break;
        case REACT_STATE_STEP4_RUNNING:
            eCheckResult = TestProc_Api_ReactCheckResult();
            if (TESTPROC_API_RESULT_OK == eCheckResult)
            {
                u8TxBuf[0] = REACT_TRIGGER_4_STEP;
                *((uint32_t *)(&u8TxBuf[1])) = g_u32Trigger31Count;
                TestProc_SendMsg(TESTPROC_MSG_CMD_REACT_STOP, TESTPROC_UNIT_REACT, u8TxBuf, 5);
                FSM_React = REACT_STATE_IDLE;
            }
            else if (TESTPROC_API_RESULT_NG == eCheckResult)
            {
                DebugMsg("W>ReactStep4NG\r\n");
                FSM_React = REACT_STATE_IDLE;
            }
            break;
        // 
        case REACT_STATE_MIX:
            if (g_u8MixMask&REACT_RM1_MIX)
            {
               eErr =TestProc_Api_ReactMMixAction(g_u16MixTimeRM1);
               //DebugMsg("A_RM1\r\n");
            }
            if (g_u8MixMask&REACT_RM2_MIX)
            {
               eErr =TestProc_Api_ReactMixAction(g_u16MixTimeRM2);
               //DebugMsg("A_RM2\r\n");
            }
            if (LH_ERR_NONE == eErr)
            {
                FSM_React = REACT_STATE_MIX_ING;
            }
            else
            {
                s32Result = TESTPROC_RESULT_ERR_EXEC;
                FSM_React = REACT_STATE_IDLE;
            }
            break;
        case REACT_STATE_MIX_ING:
            eCheckResult = TestProc_Api_ReactMCheckResult();
            if (TESTPROC_API_RESULT_OK == eCheckResult)
            {
                g_u8MixMask &= ~REACT_RM1_MIX;
            }
            eCheckResult = TestProc_Api_ReactCheckResult();
            if (TESTPROC_API_RESULT_OK == eCheckResult)
            {
                g_u8MixMask &= ~REACT_RM2_MIX;
            }
            
            if (0 == g_u8MixMask)
            {
                TestProc_SendMsg(TESTPROC_MSG_CMD_REACT_MIX_OK, TESTPROC_UNIT_REACT, NULL, 0);
                FSM_React = REACT_STATE_IDLE;
            }
            else if (TESTPROC_API_RESULT_NG == eCheckResult)
            {
                s32Result = TESTPROC_RESULT_NG;
                FSM_React = REACT_STATE_IDLE;
            }
            break;
        case REACT_STATE_IDLE:
        default:

            break;
    }
    
    if (REACT_STATE_IDLE == FSM_React)
    {
        g_ReactTimeStart = TESTPROC_TICK_INVALID;
        if (s32Result == TESTPROC_RESULT_NG && eErr != LH_ERR_NONE && g_eReactActionWarning[FSM_Old] != TESTPROC_WARNING_NONE)
        {
            DebugMsg("E>ReactWorkNG:%d,%d,%08X\r\n", g_eReactWorking, FSM_Old, eErr);
            *(uint16_t *)u8TxBuf[0] = g_eReactActionWarning[FSM_Old];
            u8TxBuf[2] = TESTPROC_UNIT_REACT;
            u8TxBuf[3] = FSM_Old;
            u8TxBuf[4] = WORD_TO_BYTE_LL(eErr);
            u8TxBuf[5] = WORD_TO_BYTE_LL(eErr);
            u8TxBuf[6] = WORD_TO_BYTE_LL(eErr);
            u8TxBuf[7] = WORD_TO_BYTE_LL(eErr);
            TestProc_SendMsg(TESTPROC_MSG_CMD_EXEC_NG, TESTPROC_UNIT_REACT, u8TxBuf, 8);
        }
    }
    else
    {
        if (tick > g_ReactTimeStart && (tick-g_ReactTimeStart) > g_stReactWorkState[g_eReactWorking].TimeOut)
        {
            DebugMsg("E>ReactWorkTimeout:%d,%d,%d,%d,%d,%d\r\n", g_eReactWorking, FSM_React, FSM_Old, g_stReactWorkState[g_eReactWorking].TimeOut, tick, g_ReactTimeStart);
            g_ReactTimeStart = TESTPROC_TICK_INVALID;
            if (g_stReactWorkState[g_eReactWorking].eWarning < TESTPROC_WARNING_NONE)
            {
                *(uint16_t *)&u8TxBuf[0] = g_stReactWorkState[g_eReactWorking].eWarning;
                u8TxBuf[2] = TESTPROC_UNIT_REACT;
                u8TxBuf[3] = FSM_React;
                TestProc_SendMsg(TESTPROC_MSG_CMD_TIMEOUT, TESTPROC_UNIT_EVENT, u8TxBuf, 4);
            }
        }
    }
    
    if (TESTPROC_RESULT_OK != s32Result)
    {
        DebugMsg("E>ReactFSM:%d,%d\r\n", s32Result, FSM_React);
    }
}


static void TestProc_ReactTrigger(void *p_tmr, void *p_arg)
{
    OS_ERR err;
    //OS_TICK tick;
    OS_STATE tmrState;
    uint8_t  u8TxBuf[8];

    //tick = OSTimeGet(&err);
    if (&g_stReact23Timer == p_tmr)
    {
        tmrState = OSTmrStateGet(&g_stReact02Timer, &err);
        if (OS_TMR_STATE_RUNNING == tmrState)
        {
            OSTmrStop(&g_stReact02Timer, OS_OPT_TMR_NONE, NULL, &err);
        }
        OSTmrStart(&g_stReact02Timer, &err);
        g_u32Trigger23Count ++;
        u8TxBuf[0] = REACT_TRIGGER_1_STEP;
        *((uint32_t *)(&u8TxBuf[1])) = g_u32Trigger23Count;
        TestProc_SendMsg(TESTPROC_MSG_CMD_REACT_TRIGGER, TESTPROC_UNIT_REACT, u8TxBuf, 5);
    }
    else if (&g_stReact02Timer == p_tmr)
    {
        tmrState = OSTmrStateGet(&g_stReact09Timer, &err);
        if (OS_TMR_STATE_RUNNING == tmrState)
        {
            OSTmrStop(&g_stReact09Timer, OS_OPT_TMR_NONE, NULL, &err);
        }
        OSTmrStart(&g_stReact09Timer, &err);
        g_u32Trigger02Count ++;
        u8TxBuf[0] = REACT_TRIGGER_2_STEP;
        *((uint32_t *)(&u8TxBuf[1])) = g_u32Trigger02Count;
        TestProc_SendMsg(TESTPROC_MSG_CMD_REACT_TRIGGER, TESTPROC_UNIT_REACT, u8TxBuf, 5);
    }
    else if (&g_stReact09Timer == p_tmr)
    {
        tmrState = OSTmrStateGet(&g_stReact31Timer, &err);
        if (OS_TMR_STATE_RUNNING == tmrState)
        {
            OSTmrStop(&g_stReact31Timer, OS_OPT_TMR_NONE, NULL, &err);
        }
        OSTmrStart(&g_stReact31Timer, &err);
        g_u32Trigger09Count ++;
        u8TxBuf[0] = REACT_TRIGGER_3_STEP;
        *((uint32_t *)(&u8TxBuf[1])) = g_u32Trigger09Count;
        TestProc_SendMsg(TESTPROC_MSG_CMD_REACT_TRIGGER, TESTPROC_UNIT_REACT, u8TxBuf, 5);
    }
    else if (&g_stReact31Timer == p_tmr)
    {
        tmrState = OSTmrStateGet(&g_stReact23Timer, &err);
        if (OS_TMR_STATE_RUNNING == tmrState)
        {
            OSTmrStop(&g_stReact23Timer, OS_OPT_TMR_NONE, NULL, &err);
        }
        OSTmrStart(&g_stReact23Timer, &err);
        g_u32Trigger31Count ++;
        u8TxBuf[0] = REACT_TRIGGER_4_STEP;
        *((uint32_t *)(&u8TxBuf[1])) = g_u32Trigger31Count;
        TestProc_SendMsg(TESTPROC_MSG_CMD_REACT_TRIGGER, TESTPROC_UNIT_REACT, u8TxBuf, 5);
    }
    else
    {
        DebugMsg("E>ReactTrigger\r\n");
    }
}

