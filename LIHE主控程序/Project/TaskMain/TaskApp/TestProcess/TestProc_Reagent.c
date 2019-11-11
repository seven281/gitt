
#include "TestProc_Reagent.h"

#define REAGENT_QUANTITY_M_MAX              (200u)
#define REAGENT_QUANTITY_R_MAX              (200u)

typedef enum
{
    FSM_REAGENT_RESET_IDLE = 0,
    FSM_REAGENT_RESET_ING,          // 试剂针复位中
}FSM_REAGENT_RESET_E;

typedef enum
{
    REAGENT_STATE_IDLE = 0,
    REAGENT_STATE_RESET,                            // 1
    // Step2
    REAGENT_STATE_ABSORB_RS1_R1_PREPARE,            // 2
    REAGENT_STATE_ABSORB_RS1_R1_PREPARE_ING,        // 3
    REAGENT_STATE_ABSORB_RS1_R1,                    // 4 吸R1
    REAGENT_STATE_ABSORB_RS1_R1_ING,                // 5 吸液中
    REAGENT_STATE_ABSORB_RS1_R2,                    // 6
    REAGENT_STATE_ABSORB_RS1_R2_TO_WASH,            // 7 移动中
    REAGENT_STATE_ABSORB_RS1_R2_TO_DISK,            // 8 清洗外壁
    REAGENT_STATE_ABSORB_RS1_R2_ING,                // 9
    REAGENT_STATE_ABSORB_RS1_TO_REACT,              // 10
    REAGENT_STATE_ABSORB_RS1_R2_PREPARE,            // 11
    REAGENT_STATE_ABSORB_RS1_R2_PREPARE_ING,        // 12
    REAGENT_STATE_ABSORB_RS1_R2_2STEP,              // 13 吸R1
    REAGENT_STATE_ABSORB_RS1_R2_2STEP_ING,          // 14 吸液中
    REAGENT_STATE_ABSORB_RS1_R2_2STEP_TO_REACT,     // 15 吸液中
    REAGENT_STATE_INJECT_RS1,                       // 16
    REAGENT_STATE_INJECT_RS1_ING,                   // 17
    REAGENT_STATE_INJECT_RS1_TO_CLEAN,              // 18
    REAGENT_STATE_INJECT_RS1_CLEAN_ING,             // 19
    // step 2
    REAGENT_STATE_ABSORB_RS2_R2_PREPARE,            // 20
    REAGENT_STATE_ABSORB_RS2_R2_PREPARE_ING,        // 21
    REAGENT_STATE_ABSORB_RS2_R2,                    // 22
    REAGENT_STATE_ABSORB_RS2_R2_ING,                // 23
    REAGENT_STATE_ABSORB_RS2_R3_CLEAN,              // 24
    REAGENT_STATE_ABSORB_RS2_R3_TO_CLEAN,           // 25
    REAGENT_STATE_ABSORB_RS2_R3_PREPARE,            // 26
    REAGENT_STATE_ABSORB_RS2_R3_PREPARE_ING,        // 27
    REAGENT_STATE_ABSORB_RS2_R3,                    // 28
    REAGENT_STATE_ABSORB_RS2_R3_ING,                // 29
    REAGENT_STATE_ABSORB_RS2_M_CLEAN,               // 30
    REAGENT_STATE_ABSORB_RS2_M_TO_CLEAN,            // 31
    REAGENT_STATE_ABSORB_RS2_M_PREPARE,             // 32
    REAGENT_STATE_ABSORB_RS2_M_PREPARE_ING,         // 33
    REAGENT_STATE_ABSORB_RS2_M,                     // 34
    REAGENT_STATE_ABSORB_RS2_M_ING,                 // 35
    REAGENT_STATE_ABSORB_RS2_M_TO_REACT,            // 36
    REAGENT_STATE_INJECT_RS2,                       // 37
    REAGENT_STATE_INJECT_RS2_ING,                   // 38
    REAGENT_STATE_INJECT_RS2_TO_WASH_ING,           // 39
    REAGENT_STATE_INJECT_RS2_WASH_ING,              // 40
    REAGENT_STATE_MOVE_TO_REACT,                    // 41
    REAGENT_STATE_MOVE_TO_REACT_ING,                // 42
    REAGENT_STATE_MAX
}REAGENT_STATE_E;

typedef struct
{
    REAGENT_STATE_E eReagentState;
    OS_TICK TimeOut;
    TESTPROC_WARNING_E  eWarning;
}REAGENT_WORK_T;

#define TestProc_ReagentCleanFSM()                  \
    do                                              \
    {                                               \
        FSM_Reagent = REAGENT_STATE_IDLE;           \
    }while(0)

static REAGENT_STATE_E FSM_Reagent;
static FSM_REAGENT_RESET_E FSM_Reagent_Reset;
static uint16_t g_u16Quantity;
static uint16_t g_u16MQuantity;
static OS_TICK g_ReagentTimeStart = TESTPROC_TICK_INVALID;
static REAGENT_WORK_E g_eReagentWorking = REAGENT_WORK_RESET;
static uint8_t  g_u8ReagentDiskPos = 0;
static const REAGENT_WORK_T g_stReagentWorkState[REAGENT_WORK_MAX] = 
{
    {REAGENT_STATE_RESET,                       TESTPROC_TICK_INVALID,      TESTPROC_WARNING_NONE},
    {REAGENT_STATE_ABSORB_RS1_R1_PREPARE,       500,                        TESTPROC_WARNING_TIMEOUT_RS1_R1_PREPARE},           // 400
    {REAGENT_STATE_ABSORB_RS1_R1,               1600,                       TESTPROC_WARNING_TIMEOUT_RS1_R1},                   // 700
    {REAGENT_STATE_ABSORB_RS1_R2,               4000,                       TESTPROC_WARNING_TIMEOUT_RS1_R2},                   // 1200
    {REAGENT_STATE_ABSORB_RS1_R2_PREPARE,       500,                        TESTPROC_WARNING_TIMEOUT_RS1_STEP2_R2_PREPARE},     // 400 4 第1步加试剂2准备
    {REAGENT_STATE_ABSORB_RS1_R2_2STEP,         3500,                       TESTPROC_WARNING_TIMEOUT_RS1_STEP2_R2},             // 700 5
    {REAGENT_STATE_INJECT_RS1,                  3000,                       TESTPROC_WARNING_TIMEOUT_RS1_R1_INJECT},            // 1200
    {REAGENT_STATE_ABSORB_RS2_R2_PREPARE,       500,                        TESTPROC_WARNING_TIMEOUT_RS2_R2_PREPARE},           // 400
    {REAGENT_STATE_ABSORB_RS2_R2,               1600,                       TESTPROC_WARNING_TIMEOUT_RS2_R2},                   // 700
    {REAGENT_STATE_ABSORB_RS2_R3_CLEAN,         2000,                       TESTPROC_WARNING_TIMEOUT_RS2_R3_CLEAN},             // 1500
    {REAGENT_STATE_ABSORB_RS2_R3_PREPARE,       500,                        TESTPROC_WARNING_TIMEOUT_RS2_R3_PREPARE},           // 400
    {REAGENT_STATE_ABSORB_RS2_R3,               1600,                       TESTPROC_WARNING_TIMEOUT_RS2_R3},                   // 700
    {REAGENT_STATE_ABSORB_RS2_M_CLEAN,          2000,                       TESTPROC_WARNING_TIMEOUT_RS2_M_CLEAN},              // 1500
    {REAGENT_STATE_ABSORB_RS2_M_PREPARE,        500,                        TESTPROC_WARNING_TIMEOUT_RS2_M_PREPARE},            // 400
    {REAGENT_STATE_ABSORB_RS2_M,                1900,                       TESTPROC_WARNING_TIMEOUT_RS2_M},                    // 700 + 400
    {REAGENT_STATE_INJECT_RS2,                  3000,                       TESTPROC_WARNING_TIMEOUT_RS2_INJECT},               // 700 + 400 + 700
    {REAGENT_STATE_MOVE_TO_REACT,               500,                        TESTPROC_WARNING_TIMEOUT_REAGENT_TO_REACT}                // 700 + 400 + 700
};

static const TESTPROC_WARNING_E g_eReagentActionWarning[REAGENT_STATE_MAX] = 
{
 /* 0*/   TESTPROC_WARNING_NONE,
 /* 1*/   TESTPROC_WARNING_NONE,
 /* 2*/   TESTPROC_WARNING_NONE,
 /* 3*/   TESTPROC_WARNING_API_NG_REAGENT_ROTATE,                           // REAGENT_STATE_ABSORB_RS1_R1_PREPARE_ING
 /* 4*/   TESTPROC_WARNING_NONE,
 /* 5*/   TESTPROC_WARNING_API_NG_ABSORB_RS1_R1_ING,                        // REAGENT_STATE_ABSORB_RS1_R1_ING
 /* 6*/   TESTPROC_WARNING_NONE,
 /* 7*/   TESTPROC_WARNING_API_NG_REAGENT_ROTATE,                           // REAGENT_STATE_ABSORB_RS1_R2_TO_WASH
 /* 8*/   TESTPROC_WARNING_API_NG_CLEAN_OUT,                                // REAGENT_STATE_ABSORB_RS1_R2_TO_DISK
 /* 9*/   TESTPROC_WARNING_API_NG_ABSORB_RS1_R2_ING,                        // REAGENT_STATE_ABSORB_RS1_R2_ING
 /*10*/   TESTPROC_WARNING_API_NG_REAGENT_ROTATE,                           // REAGENT_STATE_ABSORB_RS1_TO_REACT
 /*11*/   TESTPROC_WARNING_NONE,
 /*12*/   TESTPROC_WARNING_API_NG_REAGENT_ROTATE,                           // REAGENT_STATE_ABSORB_RS1_R2_PREPARE_ING
 /*13*/   TESTPROC_WARNING_NONE,
 /*14*/   TESTPROC_WARNING_API_NG_ABSORB_RS1_STEP2_R2,                      // REAGENT_STATE_ABSORB_RS1_R2_2STEP_ING
 /*15*/   TESTPROC_WARNING_API_NG_REAGENT_ROTATE,                           // REAGENT_STATE_ABSORB_RS1_R2_2STEP_TO_REACT
 /*16*/   TESTPROC_WARNING_NONE,
 /*17*/   TESTPROC_WARNING_API_NG_INJECT_RS1,                               // REAGENT_STATE_INJECT_RS1_ING
 /*18*/   TESTPROC_WARNING_API_NG_REAGENT_ROTATE,                           // REAGENT_STATE_INJECT_RS1_TO_CLEAN
 /*19*/   TESTPROC_WARNING_API_NG_CLEAN_OUT,                                // REAGENT_STATE_INJECT_RS1_CLEAN_ING
 /*20*/   TESTPROC_WARNING_NONE,
 /*21*/   TESTPROC_WARNING_API_NG_REAGENT_ROTATE,                           // REAGENT_STATE_ABSORB_RS2_R2_PREPARE_ING
 /*22*/   TESTPROC_WARNING_NONE,
 /*23*/   TESTPROC_WARNING_API_NG_ABSORB_RS2_R2_ING,                        // REAGENT_STATE_ABSORB_RS2_R2_ING
 /*24*/   TESTPROC_WARNING_NONE,
 /*25*/   TESTPROC_WARNING_API_NG_REAGENT_ROTATE,                           // REAGENT_STATE_ABSORB_RS2_R3_TO_CLEAN
 /*26*/   TESTPROC_WARNING_NONE,
 /*27*/   TESTPROC_WARNING_API_NG_CLEAN_OUT,                                // REAGENT_STATE_ABSORB_RS2_R3_PREPARE_ING
 /*28*/   TESTPROC_WARNING_NONE,
 /*29*/   TESTPROC_WARNING_API_NG_ABSORB_RS2_R3_ING,                        // REAGENT_STATE_ABSORB_RS2_R3_ING
 /*30*/   TESTPROC_WARNING_NONE,
 /*31*/   TESTPROC_WARNING_API_NG_REAGENT_ROTATE,                           // REAGENT_STATE_ABSORB_RS2_M_TO_CLEAN
 /*32*/   TESTPROC_WARNING_NONE,
 /*33*/   TESTPROC_WARNING_API_NG_CLEAN_OUT,                                // REAGENT_STATE_ABSORB_RS2_M_PREPARE_ING
 /*34*/   TESTPROC_WARNING_NONE,
 /*35*/   TESTPROC_WARNING_API_NG_ABSORB_RS2_M_ING,                         // REAGENT_STATE_ABSORB_RS2_M_ING
 /*36*/   TESTPROC_WARNING_API_NG_REAGENT_ROTATE,                           // REAGENT_STATE_ABSORB_RS2_M_TO_REACT
 /*37*/   TESTPROC_WARNING_NONE,
 /*38*/   TESTPROC_WARNING_API_NG_INJECT_RS2,                               // REAGENT_STATE_INJECT_RS2_ING
 /*39*/   TESTPROC_WARNING_API_NG_REAGENT_ROTATE,                           // REAGENT_STATE_INJECT_RS2_TO_WASH_ING
 /*40*/   TESTPROC_WARNING_API_NG_CLEAN_INOUT,                              // REAGENT_STATE_INJECT_RS2_WASH_ING
 /*41*/   TESTPROC_WARNING_NONE,
 /*42*/   TESTPROC_WARNING_API_NG_REAGENT_ROTATE,                           // REAGENT_STATE_MOVE_TO_REACT_ING
};

/*
static const char g_strReagentWorkName[REAGENT_WORK_MAX][24] =
{
    "Reset",
    "RS1_R1_Prepare",
    "AbsorbRS1_R1",
    "AbsorbRS1_R2",
    "InjectR1",
    "AbsorbRS2_R2_P",
    "AbsorbRS2_R2",
    "AbsorbRS2_R3_P",
    "AbsorbRS2_R3",
    "AbsorbRS2_M",    
    "AbsorbRS2_INJECT",
};
*/
int TestProc_ReagentInit(void)
{
    //TestProc_Api_ReagentInit();

    FSM_Reagent = REAGENT_STATE_IDLE;
    FSM_Reagent_Reset = FSM_REAGENT_RESET_IDLE;
    
    return TESTPROC_RESULT_OK;
}

int TestProc_ReagentStartWork(REAGENT_WORK_E eWork, uint8_t *pu8Param, uint8_t u8Size)
{
    int result = TESTPROC_RESULT_OK;
    OS_ERR err = OS_ERR_NONE;

    if (REAGENT_STATE_IDLE == FSM_Reagent && eWork < REAGENT_WORK_MAX)
    {
        FSM_Reagent = g_stReagentWorkState[eWork].eReagentState;
        g_eReagentWorking = eWork;
        g_ReagentTimeStart = OSTimeGet(&err);
        if (REAGENT_STATE_ABSORB_RS1_R1 == FSM_Reagent || 
            REAGENT_STATE_ABSORB_RS1_R2 == FSM_Reagent ||
            REAGENT_STATE_ABSORB_RS2_R2 == FSM_Reagent ||
            REAGENT_STATE_ABSORB_RS2_R3 == FSM_Reagent ||
            REAGENT_STATE_ABSORB_RS1_R2_2STEP == FSM_Reagent)
        {
            g_u16Quantity = *(uint16_t *)pu8Param;
            g_u8ReagentDiskPos = pu8Param[2];
            if (g_u16Quantity > REAGENT_QUANTITY_R_MAX)
            {
                DebugMsg("E>+R_Quantity:%d\r\n", g_u16Quantity);
            }
        }
        else if (REAGENT_STATE_ABSORB_RS2_M == FSM_Reagent)
        {
            g_u16MQuantity = *(uint16_t *)pu8Param;
            g_u8ReagentDiskPos = pu8Param[2];
            if (g_u16MQuantity > REAGENT_QUANTITY_M_MAX)
            {
                DebugMsg("E>+M_Quantity:%d\r\n", g_u16MQuantity);
            }
        }
        //DebugMsg("**ReagentWork >> %s\r\n", g_strReagentWorkName[eWork]);
    }
    else
    {
        result = TESTPROC_RESULT_ERR_STATUS;
        DebugMsg("E>StartReagent:%d,%d\r\n", FSM_Reagent, eWork);
    }

    return result;
}

static TESTPROC_API_RESULT_E TestProc_ReagentResetFSM(void)
{
    TESTPROC_API_RESULT_E eActionResult = TESTPROC_API_RESULT_WAIT;    
    TSC_CMD_STATE eResult;
    LH_ERR eErr;

    if(FSM_REAGENT_RESET_IDLE == FSM_Reagent_Reset)
    {
        FSM_Reagent_Reset = FSM_REAGENT_RESET_ING;
        TestProc_Api_ReagentReset();
    }
    else if (FSM_REAGENT_RESET_ING == FSM_Reagent_Reset)
    {
        eResult = TSC_NeedleReagentBeadGetLastCommandStatusAndResult(&eErr);
        //eActionResult = TestProc_Api_ReagentCheckResult(&eErr);
        if (TSC_CMD_STATE_COMPLETE == eResult || TSC_CMD_STATE_IDLE == eResult)//TestProc_Api_IsResetCompile(eActionResult))
        {
            FSM_Reagent_Reset = FSM_REAGENT_RESET_IDLE;
            eActionResult = TESTPROC_API_RESULT_OK;
        }
        //DebugMsg("ReagentResetting:%d,%d,%d\r\n", eActionResult, eResult, eErr);
    }

    return eActionResult;
}

void TestProc_ReagentFSM(OS_TICK tick)
{
    TESTPROC_API_RESULT_E eCheckResult;
    int32_t  s32Result = TESTPROC_RESULT_OK;
    LH_ERR eErrCode;
    LH_ERR eErr;
    uint8_t  u8TxBuf[8];
    REAGENT_STATE_E FSM_old = FSM_Reagent;

    switch(FSM_Reagent)
    {
        case REAGENT_STATE_RESET:
            eCheckResult = TestProc_ReagentResetFSM();
            if (TestProc_Api_IsResetCompile(eCheckResult))
            {
                u8TxBuf[0] = TESTPROC_API_RESULT_OK == eCheckResult ? OK : NG;
                TestProc_SendMsg(TESTPROC_MSG_CMD_RESET, TESTPROC_UNIT_REAGENT, u8TxBuf, 1);
                TestProc_ReagentCleanFSM();
            }
            break;
        //************移动到R1位置*******************
        case REAGENT_STATE_ABSORB_RS1_R1_PREPARE:
            eErr =TestProc_Api_ReagentMoveToPos(TSC_NEEDLE_REAGENT_BEAD_POS_R1);
            if (LH_ERR_NONE == eErr)
            {
                FSM_Reagent = REAGENT_STATE_ABSORB_RS1_R1_PREPARE_ING;
            }
            else
            {
                s32Result = TESTPROC_RESULT_ERR_EXEC;
                FSM_Reagent = REAGENT_STATE_IDLE;
            }
            break;
        case REAGENT_STATE_ABSORB_RS1_R1_PREPARE_ING:
            eCheckResult = TestProc_Api_ReagentCheckResult(&eErrCode);
            if (TESTPROC_API_RESULT_OK == eCheckResult)
            {
                TestProc_SendMsg(TESTPROC_MSG_CMD_RS1_R1_PREPARE_OK, TESTPROC_UNIT_REAGENT, NULL, 0);
                FSM_Reagent = REAGENT_STATE_IDLE;
            }
            else if (TESTPROC_API_RESULT_NG == eCheckResult)
            {
                s32Result = TESTPROC_RESULT_NG;
                FSM_Reagent = REAGENT_STATE_IDLE;
                TestProc_SendMsg(TESTPROC_MSG_CMD_TIMEOUT, TESTPROC_UNIT_EVENT, u8TxBuf, 4);
            }
            break;
        //***************吸R1**********************
        case REAGENT_STATE_ABSORB_RS1_R1:
            if (g_u16Quantity > 0)
            {
                eErr = TestProc_Api_ReagentAbsorb(g_u16Quantity, g_u8ReagentDiskPos, 1);
                if (LH_ERR_NONE == eErr)
                {
                    FSM_Reagent = REAGENT_STATE_ABSORB_RS1_R1_ING;
                }
                else
                {
                    s32Result = TESTPROC_RESULT_ERR_EXEC;
                    FSM_Reagent = REAGENT_STATE_IDLE;
                }
            }
            else
            {
                TestProc_SendMsg(TESTPROC_MSG_CMD_RS1_R1_ABSORB_OK, TESTPROC_UNIT_REAGENT, NULL, 0);
                FSM_Reagent = REAGENT_STATE_IDLE;
            }
            break;
        case REAGENT_STATE_ABSORB_RS1_R1_ING:
            eCheckResult = TestProc_Api_ReagentCheckResult(&eErrCode);
            if (TESTPROC_API_RESULT_OK == eCheckResult)
            {
                TestProc_SendMsg(TESTPROC_MSG_CMD_RS1_R1_ABSORB_OK, TESTPROC_UNIT_REAGENT, NULL, 0);
                FSM_Reagent = REAGENT_STATE_IDLE;
            }
            else if (TESTPROC_API_RESULT_NG == eCheckResult)
            {
                s32Result = TESTPROC_RESULT_NG;
                FSM_Reagent = REAGENT_STATE_IDLE;
            }
            break;
        //***************吸R2**********************
        case REAGENT_STATE_ABSORB_RS1_R2:
            eErr = TestProc_Api_ReagentMoveToPos(TSC_NEEDLE_REAGENT_BEAD_POS_CLEAN);
            if (LH_ERR_NONE == eErr)
            {
                FSM_Reagent = REAGENT_STATE_ABSORB_RS1_R2_TO_WASH;
            }
            else
            {
                s32Result = TESTPROC_RESULT_ERR_EXEC;
                FSM_Reagent = REAGENT_STATE_IDLE;
            }
            break;
        case REAGENT_STATE_ABSORB_RS1_R2_TO_WASH:
            eCheckResult = TestProc_Api_ReagentCheckResult(&eErrCode);
            if (TESTPROC_API_RESULT_OK == eCheckResult)
            {
                eErr = TestProc_Api_ReagentClean(TSC_NEEDLE_REAGENT_BEAD_POS_R2, TSC_NEEDLE_REAGENT_BEAD_CLEAN_OPT_OUT);
                if (LH_ERR_NONE == eErr)
                {
                    FSM_Reagent = REAGENT_STATE_ABSORB_RS1_R2_TO_DISK;
                }
                else
                {
                    s32Result = TESTPROC_RESULT_ERR_EXEC;
                }
            }
            else if (TESTPROC_API_RESULT_NG == eCheckResult)
            {
                s32Result = TESTPROC_RESULT_NG;
                FSM_Reagent = REAGENT_STATE_IDLE;
            }
            break;
        case REAGENT_STATE_ABSORB_RS1_R2_TO_DISK:
            eCheckResult = TestProc_Api_ReagentCheckResult(&eErrCode);
            if (TESTPROC_API_RESULT_OK == eCheckResult)
            {
                if (g_u16Quantity > 0)
                {
                    eErr = TestProc_Api_ReagentAbsorb(g_u16Quantity, g_u8ReagentDiskPos, 2);
                    if (LH_ERR_NONE == eErr)
                    {
                        FSM_Reagent = REAGENT_STATE_ABSORB_RS1_R2_ING;
                    }
                    else
                    {
                        s32Result = TESTPROC_RESULT_ERR_EXEC;
                    }
                }
                else
                {
                    TestProc_SendMsg(TESTPROC_MSG_CMD_RS1_R2_ABSORB_OK, TESTPROC_UNIT_REAGENT, NULL, 0);
                    FSM_Reagent = REAGENT_STATE_IDLE;
                }
            }
            else if (TESTPROC_API_RESULT_NG == eCheckResult)
            {
                s32Result = TESTPROC_RESULT_NG;
                FSM_Reagent = REAGENT_STATE_IDLE;
            }
            break;
        case REAGENT_STATE_ABSORB_RS1_R2_ING:
            eCheckResult = TestProc_Api_ReagentCheckResult(&eErrCode);
            if (TESTPROC_API_RESULT_OK == eCheckResult)
            {
                TestProc_SendMsg(TESTPROC_MSG_CMD_RS1_R2_ABSORB_OK, TESTPROC_UNIT_REAGENT, NULL, 0);
                eErr = TestProc_Api_ReagentMoveToPos(TSC_NEEDLE_REAGENT_BEAD_POS_REACTION);
                if (LH_ERR_NONE == eErr)
                {
                    FSM_Reagent = REAGENT_STATE_ABSORB_RS1_TO_REACT;
                }
                else
                {
                    s32Result = TESTPROC_RESULT_ERR_EXEC;
                }
            }
            else if (TESTPROC_API_RESULT_NG == eCheckResult)
            {
                s32Result = TESTPROC_RESULT_NG;
                FSM_Reagent = REAGENT_STATE_IDLE;
            }
            break;
        case REAGENT_STATE_ABSORB_RS1_TO_REACT:
            eCheckResult = TestProc_Api_ReagentCheckResult(&eErrCode);
            if (TESTPROC_API_RESULT_OK == eCheckResult)
            {
                FSM_Reagent = REAGENT_STATE_IDLE;
            }
            else if (TESTPROC_API_RESULT_NG == eCheckResult)
            {
                s32Result = TESTPROC_RESULT_NG;
                FSM_Reagent = REAGENT_STATE_IDLE;
            }
            break;
        //************2步法移动到R2位置*******************
        case REAGENT_STATE_ABSORB_RS1_R2_PREPARE:
            eErr =TestProc_Api_ReagentMoveToPos(TSC_NEEDLE_REAGENT_BEAD_POS_R2);
            if (LH_ERR_NONE == eErr)
            {
                FSM_Reagent = REAGENT_STATE_ABSORB_RS1_R2_PREPARE_ING;
            }
            else
            {
                s32Result = TESTPROC_RESULT_ERR_EXEC;
                FSM_Reagent = REAGENT_STATE_IDLE;
            }
            break;
        case REAGENT_STATE_ABSORB_RS1_R2_PREPARE_ING:
            eCheckResult = TestProc_Api_ReagentCheckResult(&eErrCode);
            if (TESTPROC_API_RESULT_OK == eCheckResult)
            {
                TestProc_SendMsg(TESTPROC_MSG_CMD_RS1_R2_PREPARE_2STEP_OK, TESTPROC_UNIT_REAGENT, NULL, 0);
                FSM_Reagent = REAGENT_STATE_IDLE;
            }
            else if (TESTPROC_API_RESULT_NG == eCheckResult)
            {
                s32Result = TESTPROC_RESULT_NG;
                FSM_Reagent = REAGENT_STATE_IDLE;
            }
            break;
        //***************2步法吸R2**********************
        case REAGENT_STATE_ABSORB_RS1_R2_2STEP:
            if (g_u16Quantity > 0)
            {
                eErr = TestProc_Api_ReagentAbsorb(g_u16Quantity, g_u8ReagentDiskPos, 2);
                if (LH_ERR_NONE == eErr)
                {
                    FSM_Reagent = REAGENT_STATE_ABSORB_RS1_R2_2STEP_ING;
                }
                else
                {
                    s32Result = TESTPROC_RESULT_ERR_EXEC;
                    FSM_Reagent = REAGENT_STATE_IDLE;
                }
            }
            else
            {
                TestProc_SendMsg(TESTPROC_MSG_CMD_RS1_R2_ABSORB_2STEP_OK, TESTPROC_UNIT_REAGENT, NULL, 0);
                FSM_Reagent = REAGENT_STATE_IDLE;
            }
            break;
        case REAGENT_STATE_ABSORB_RS1_R2_2STEP_ING:
            eCheckResult = TestProc_Api_ReagentCheckResult(&eErrCode);
            if (TESTPROC_API_RESULT_OK == eCheckResult)
            {
                TestProc_SendMsg(TESTPROC_MSG_CMD_RS1_R2_ABSORB_2STEP_OK, TESTPROC_UNIT_REAGENT, NULL, 0);
                eErr =TestProc_Api_ReagentMoveToPos(TSC_NEEDLE_REAGENT_BEAD_POS_REACTION);
                if (LH_ERR_NONE == eErr)
                {
                    FSM_Reagent = REAGENT_STATE_ABSORB_RS1_R2_2STEP_TO_REACT;
                }
                else
                {
                    s32Result = TESTPROC_RESULT_ERR_EXEC;
                    FSM_Reagent = REAGENT_STATE_IDLE;
                }
            }
            else if (TESTPROC_API_RESULT_NG == eCheckResult)
            {
                s32Result = TESTPROC_RESULT_NG;
                FSM_Reagent = REAGENT_STATE_IDLE;
            }
            break;
        case REAGENT_STATE_ABSORB_RS1_R2_2STEP_TO_REACT:
            eCheckResult = TestProc_Api_ReagentCheckResult(&eErrCode);
            if (TESTPROC_API_RESULT_OK == eCheckResult)
            {
                FSM_Reagent = REAGENT_STATE_IDLE;
            }
            else if (TESTPROC_API_RESULT_NG == eCheckResult)
            {
                s32Result = TESTPROC_RESULT_NG;
                FSM_Reagent = REAGENT_STATE_IDLE;
            }
            break;
        // 注射R1试剂
        case REAGENT_STATE_INJECT_RS1:
            eErr = TestProc_Api_ReagentInject(g_u16Quantity);
            if (LH_ERR_NONE == eErr)
            {
                FSM_Reagent = REAGENT_STATE_INJECT_RS1_ING;
            }
            else
            {
                s32Result = TESTPROC_RESULT_ERR_EXEC;
                FSM_Reagent = REAGENT_STATE_IDLE;
            }
            break;
        case REAGENT_STATE_INJECT_RS1_ING:
            eCheckResult = TestProc_Api_ReagentCheckResult(&eErrCode);
            if (TESTPROC_API_RESULT_OK == eCheckResult)
            {
                TestProc_SendMsg(TESTPROC_MSG_CMD_RS1_INJECT_OVER, TESTPROC_UNIT_REAGENT, NULL, 0);
                eErr = TestProc_Api_ReagentMoveToPos(TSC_NEEDLE_REAGENT_BEAD_POS_CLEAN);
                if (LH_ERR_NONE == eErr)
                {
                    FSM_Reagent = REAGENT_STATE_INJECT_RS1_TO_CLEAN;
                }
                else
                {
                    s32Result = TESTPROC_RESULT_ERR_EXEC;
                }
            }
            else if (TESTPROC_API_RESULT_NG == eCheckResult)
            {
                s32Result = TESTPROC_RESULT_NG;
                FSM_Reagent = REAGENT_STATE_IDLE;
            }
            break;
        case REAGENT_STATE_INJECT_RS1_TO_CLEAN:
            eCheckResult = TestProc_Api_ReagentCheckResult(&eErrCode);
            if (TESTPROC_API_RESULT_OK == eCheckResult)
            {
                eErr = TestProc_Api_ReagentClean(TSC_NEEDLE_REAGENT_BEAD_POS_CLEAN, TSC_NEEDLE_REAGENT_BEAD_CLEAN_OPT_IN_OUT);
                if (LH_ERR_NONE == eErr)
                {
                    FSM_Reagent = REAGENT_STATE_INJECT_RS1_CLEAN_ING;
                }
                else
                {
                    s32Result = TESTPROC_RESULT_ERR_EXEC;
                }
            }
            else if (TESTPROC_API_RESULT_NG == eCheckResult)
            {
                s32Result = TESTPROC_RESULT_NG;
                FSM_Reagent = REAGENT_STATE_IDLE;
            }
            break;
        case REAGENT_STATE_INJECT_RS1_CLEAN_ING:
            eCheckResult = TestProc_Api_ReagentCheckResult(&eErrCode);
            if (TESTPROC_API_RESULT_OK == eCheckResult)
            {
                TestProc_SendMsg(TESTPROC_MSG_CMD_RS1_INJECT_OK, TESTPROC_UNIT_REAGENT, NULL, 0);
                FSM_Reagent = REAGENT_STATE_IDLE;
            }
            else if (TESTPROC_API_RESULT_NG == eCheckResult)
            {
                s32Result = TESTPROC_RESULT_NG;
                FSM_Reagent = REAGENT_STATE_IDLE;
            }
            break;
        // ***************Step2 Prepare R2***************
        case REAGENT_STATE_ABSORB_RS2_R2_PREPARE:
            eErr = TestProc_Api_ReagentMoveToPos(TSC_NEEDLE_REAGENT_BEAD_POS_R2);
            if (LH_ERR_NONE == eErr)
            {
                FSM_Reagent = REAGENT_STATE_ABSORB_RS2_R2_PREPARE_ING;
            }
            else
            {
                s32Result = TESTPROC_RESULT_ERR_EXEC;
                FSM_Reagent = REAGENT_STATE_IDLE;
            }
            break;
        case REAGENT_STATE_ABSORB_RS2_R2_PREPARE_ING:
            eCheckResult = TestProc_Api_ReagentCheckResult(&eErrCode);
            if (TESTPROC_API_RESULT_OK == eCheckResult)
            {
                TestProc_SendMsg(TESTPROC_MSG_CMD_RS2_R2_PREPARE_OK, TESTPROC_UNIT_REAGENT, NULL, 0);
                FSM_Reagent = REAGENT_STATE_IDLE;
            }
            else if (TESTPROC_API_RESULT_NG == eCheckResult)
            {
                s32Result = TESTPROC_RESULT_NG;
                FSM_Reagent = REAGENT_STATE_IDLE;
            }
            break;
        //***************Step2 吸R2**********************
        case REAGENT_STATE_ABSORB_RS2_R2:
            if (g_u16Quantity > 0)
            {
                eErr = TestProc_Api_ReagentAbsorb(g_u16Quantity, g_u8ReagentDiskPos, 2);
                if (LH_ERR_NONE == eErr)
                {
                    FSM_Reagent = REAGENT_STATE_ABSORB_RS2_R2_ING;
                }
                else
                {
                    s32Result = TESTPROC_RESULT_ERR_EXEC;
                    FSM_Reagent = REAGENT_STATE_IDLE;
                }
            }
            else
            {
                TestProc_SendMsg(TESTPROC_MSG_CMD_RS2_R2_ABSORB_OK, TESTPROC_UNIT_REAGENT, NULL, 0);
                FSM_Reagent = REAGENT_STATE_IDLE;
            }
            break;
        case REAGENT_STATE_ABSORB_RS2_R2_ING:
            eCheckResult = TestProc_Api_ReagentCheckResult(&eErrCode);
            if (TESTPROC_API_RESULT_OK == eCheckResult)
            {
                TestProc_SendMsg(TESTPROC_MSG_CMD_RS2_R2_ABSORB_OK, TESTPROC_UNIT_REAGENT, NULL, 0);
                FSM_Reagent = REAGENT_STATE_IDLE;
            }
            else if (TESTPROC_API_RESULT_NG == eCheckResult)
            {
                s32Result = TESTPROC_RESULT_NG;
                FSM_Reagent = REAGENT_STATE_IDLE;
            }
            break;
        // ***************Step2 Prepare R3***************
        case REAGENT_STATE_ABSORB_RS2_R3_CLEAN:
            eErr = TestProc_Api_ReagentMoveToPos(TSC_NEEDLE_REAGENT_BEAD_POS_CLEAN);
            if (LH_ERR_NONE == eErr)
            {
                FSM_Reagent = REAGENT_STATE_ABSORB_RS2_R3_TO_CLEAN;
            }
            else
            {
                s32Result = TESTPROC_RESULT_ERR_EXEC;
                FSM_Reagent = REAGENT_STATE_IDLE;
            }
            break;
        case REAGENT_STATE_ABSORB_RS2_R3_TO_CLEAN:
            eCheckResult = TestProc_Api_ReagentCheckResult(&eErrCode);
            if (TESTPROC_API_RESULT_OK == eCheckResult)
            {
                eErr = TestProc_Api_ReagentClean(TSC_NEEDLE_REAGENT_BEAD_POS_R1, TSC_NEEDLE_REAGENT_BEAD_CLEAN_OPT_OUT);
                if (LH_ERR_NONE == eErr)
                {
                    FSM_Reagent = REAGENT_STATE_ABSORB_RS2_R3_PREPARE_ING;
                }
                else
                {
                    s32Result = TESTPROC_RESULT_ERR_EXEC;
                }
            }
            else if (TESTPROC_API_RESULT_NG == eCheckResult)
            {
                s32Result = TESTPROC_RESULT_NG;
                FSM_Reagent = REAGENT_STATE_IDLE;
            }
            break;
        case REAGENT_STATE_ABSORB_RS2_R3_PREPARE:
            eErr = TestProc_Api_ReagentMoveToPos(TSC_NEEDLE_REAGENT_BEAD_POS_R1); // R3
            if (LH_ERR_NONE == eErr)
            {
                FSM_Reagent = REAGENT_STATE_ABSORB_RS2_R3_PREPARE_ING;
            }
            else
            {
                s32Result = TESTPROC_RESULT_ERR_EXEC;
                FSM_Reagent = REAGENT_STATE_IDLE;
            }
            break;
        case REAGENT_STATE_ABSORB_RS2_R3_PREPARE_ING:
            eCheckResult = TestProc_Api_ReagentCheckResult(&eErrCode);
            if (TESTPROC_API_RESULT_OK == eCheckResult)
            {
                TestProc_SendMsg(TESTPROC_MSG_CMD_RS2_R3_PREPARE_OK, TESTPROC_UNIT_REAGENT, NULL, 0);
                FSM_Reagent = REAGENT_STATE_IDLE;
            }
            else if (TESTPROC_API_RESULT_NG == eCheckResult)
            {
                s32Result = TESTPROC_RESULT_NG;
                FSM_Reagent = REAGENT_STATE_IDLE;
            }
            break;
        //***************Step2 吸R3**********************
        case REAGENT_STATE_ABSORB_RS2_R3:
            if (g_u16Quantity > 0)
            {
                eErr = TestProc_Api_ReagentAbsorb(g_u16Quantity, g_u8ReagentDiskPos, 1);
                if (LH_ERR_NONE == eErr)
                {
                    FSM_Reagent = REAGENT_STATE_ABSORB_RS2_R3_ING;
                }
                else
                {
                    s32Result = TESTPROC_RESULT_ERR_EXEC;
                    FSM_Reagent = REAGENT_STATE_IDLE;
                }
            }
            else
            {
                TestProc_SendMsg(TESTPROC_MSG_CMD_RS2_R3_ABSORB_OK, TESTPROC_UNIT_REAGENT, NULL, 0);
                FSM_Reagent = REAGENT_STATE_IDLE;
            }
            break;
        case REAGENT_STATE_ABSORB_RS2_R3_ING:
            eCheckResult = TestProc_Api_ReagentCheckResult(&eErrCode);
            if (TESTPROC_API_RESULT_OK == eCheckResult)
            {
                TestProc_SendMsg(TESTPROC_MSG_CMD_RS2_R3_ABSORB_OK, TESTPROC_UNIT_REAGENT, NULL, 0);
                FSM_Reagent = REAGENT_STATE_IDLE;
            }
            else if (TESTPROC_API_RESULT_NG == eCheckResult)
            {
                s32Result = TESTPROC_RESULT_NG;
                FSM_Reagent = REAGENT_STATE_IDLE;
            }
            break;
        //***********移动到磁珠位置******************
        case REAGENT_STATE_ABSORB_RS2_M_CLEAN:
            eErr = TestProc_Api_ReagentMoveToPos(TSC_NEEDLE_REAGENT_BEAD_POS_CLEAN);
            if (LH_ERR_NONE == eErr)
            {
                FSM_Reagent = REAGENT_STATE_ABSORB_RS2_M_TO_CLEAN;
            }
            else
            {
                s32Result = TESTPROC_RESULT_ERR_EXEC;
                FSM_Reagent = REAGENT_STATE_IDLE;
            }
            break;
        case REAGENT_STATE_ABSORB_RS2_M_TO_CLEAN:
            eCheckResult = TestProc_Api_ReagentCheckResult(&eErrCode);
            if (TESTPROC_API_RESULT_OK == eCheckResult)
            {
                eErr = TestProc_Api_ReagentClean(TSC_NEEDLE_REAGENT_BEAD_POS_BEAD, TSC_NEEDLE_REAGENT_BEAD_CLEAN_OPT_OUT);
                if (LH_ERR_NONE == eErr)
                {
                    FSM_Reagent = REAGENT_STATE_ABSORB_RS2_M_PREPARE_ING;
                }
                else
                {
                    s32Result = TESTPROC_RESULT_ERR_EXEC;
                }
            }
            else if (TESTPROC_API_RESULT_NG == eCheckResult)
            {
                s32Result = TESTPROC_RESULT_NG;
                FSM_Reagent = REAGENT_STATE_IDLE;
            }
            break;
        case REAGENT_STATE_ABSORB_RS2_M_PREPARE:
            eErr = TestProc_Api_ReagentMoveToPos(TSC_NEEDLE_REAGENT_BEAD_POS_BEAD);
            if (LH_ERR_NONE == eErr)
            {
                FSM_Reagent = REAGENT_STATE_ABSORB_RS2_M_PREPARE_ING;
            }
            else
            {
                s32Result = TESTPROC_RESULT_ERR_EXEC;
                FSM_Reagent = REAGENT_STATE_IDLE;
            }
            break;
        case REAGENT_STATE_ABSORB_RS2_M_PREPARE_ING:
            eCheckResult = TestProc_Api_ReagentCheckResult(&eErrCode);
            if (TESTPROC_API_RESULT_OK == eCheckResult)
            {
                TestProc_SendMsg(TESTPROC_MSG_CMD_RS2_M_PREPARE_OK, TESTPROC_UNIT_REAGENT, NULL, 0);
                FSM_Reagent = REAGENT_STATE_IDLE;
            }
            else if (TESTPROC_API_RESULT_NG == eCheckResult)
            {
                s32Result = TESTPROC_RESULT_NG;
                FSM_Reagent = REAGENT_STATE_IDLE;
            }
            break;
        //***************吸磁珠**********************
        case REAGENT_STATE_ABSORB_RS2_M:
            if (g_u16MQuantity > 0)
            {
                eErr = TestProc_Api_ReagentAbsorb(g_u16MQuantity, g_u8ReagentDiskPos, 3);
                if (LH_ERR_NONE == eErr)
                {
                    FSM_Reagent = REAGENT_STATE_ABSORB_RS2_M_ING;
                }
                else
                {
                    s32Result = TESTPROC_RESULT_ERR_EXEC;
                    FSM_Reagent = REAGENT_STATE_IDLE;
                }
            }
            else
            {
                TestProc_SendMsg(TESTPROC_MSG_CMD_RS2_M_ABSORB_OK, TESTPROC_UNIT_REAGENT, NULL, 0);
                FSM_Reagent = REAGENT_STATE_IDLE;
            }
            break;
        case REAGENT_STATE_ABSORB_RS2_M_ING:
            eCheckResult = TestProc_Api_ReagentCheckResult(&eErrCode);
            if (TESTPROC_API_RESULT_OK == eCheckResult)
            {
                eErr = TestProc_Api_ReagentMoveToPos(TSC_NEEDLE_REAGENT_BEAD_POS_REACTION);
                if (LH_ERR_NONE == eErr)
                {
                    FSM_Reagent = REAGENT_STATE_ABSORB_RS2_M_TO_REACT;
                }
                else
                {
                    s32Result = TESTPROC_RESULT_ERR_EXEC;
                }
            }
            else if (TESTPROC_API_RESULT_NG == eCheckResult)
            {
                s32Result = TESTPROC_RESULT_NG;
                FSM_Reagent = REAGENT_STATE_IDLE;
            }
            break;
        case REAGENT_STATE_ABSORB_RS2_M_TO_REACT:
            eCheckResult = TestProc_Api_ReagentCheckResult(&eErrCode);
            if (TESTPROC_API_RESULT_OK == eCheckResult)
            {
                TestProc_SendMsg(TESTPROC_MSG_CMD_RS2_M_ABSORB_OK, TESTPROC_UNIT_REAGENT, NULL, 0);
                FSM_Reagent = REAGENT_STATE_IDLE;
            }
            else if (TESTPROC_API_RESULT_NG == eCheckResult)
            {
                s32Result = TESTPROC_RESULT_NG;
                FSM_Reagent = REAGENT_STATE_IDLE;
            }
            break;
        // 第二步吐试剂
        case REAGENT_STATE_INJECT_RS2:
            eErr = TestProc_Api_ReagentInject(g_u16Quantity);
            if (LH_ERR_NONE == eErr)
            {
                FSM_Reagent = REAGENT_STATE_INJECT_RS2_ING;
            }
            else
            {
                s32Result = TESTPROC_RESULT_ERR_EXEC;
            }
            break;
        case REAGENT_STATE_INJECT_RS2_ING:
            eCheckResult = TestProc_Api_ReagentCheckResult(&eErrCode);
            if (TESTPROC_API_RESULT_OK == eCheckResult)
            {
                TestProc_SendMsg(TESTPROC_MSG_CMD_RS2_INJECT_OVER, TESTPROC_UNIT_REAGENT, NULL, 0);
                eErr = TestProc_Api_ReagentMoveToPos(TSC_NEEDLE_REAGENT_BEAD_POS_CLEAN);
                if (LH_ERR_NONE == eErr)
                {
                    FSM_Reagent = REAGENT_STATE_INJECT_RS2_TO_WASH_ING;
                }
                else
                {
                    s32Result = TESTPROC_RESULT_ERR_EXEC;
                }
            }
            else if (TESTPROC_API_RESULT_NG == eCheckResult)
            {
                s32Result = TESTPROC_RESULT_NG;
                FSM_Reagent = REAGENT_STATE_IDLE;
            }
            break;
        case REAGENT_STATE_INJECT_RS2_TO_WASH_ING:
            eCheckResult = TestProc_Api_ReagentCheckResult(&eErrCode);
            if (TESTPROC_API_RESULT_OK == eCheckResult)
            {
                eErr = TestProc_Api_ReagentClean(TSC_NEEDLE_REAGENT_BEAD_POS_CLEAN, TSC_NEEDLE_REAGENT_BEAD_CLEAN_OPT_IN_OUT);
                if (LH_ERR_NONE == eErr)
                {
                    FSM_Reagent = REAGENT_STATE_INJECT_RS2_WASH_ING;
                }
                else
                {
                    s32Result = TESTPROC_RESULT_ERR_EXEC;
                }
            }
            else if (TESTPROC_API_RESULT_NG == eCheckResult)
            {
                s32Result = TESTPROC_RESULT_NG;
                FSM_Reagent = REAGENT_STATE_IDLE;
            }
            break;
        case REAGENT_STATE_INJECT_RS2_WASH_ING:
            eCheckResult = TestProc_Api_ReagentCheckResult(&eErrCode);
            if (TESTPROC_API_RESULT_OK == eCheckResult)
            {
                TestProc_SendMsg(TESTPROC_MSG_CMD_RS2_INJECT_OK, TESTPROC_UNIT_REAGENT, NULL, 0);
                FSM_Reagent = REAGENT_STATE_IDLE;
            }
            else if (TESTPROC_API_RESULT_NG == eCheckResult)
            {
                s32Result = TESTPROC_RESULT_NG;
                FSM_Reagent = REAGENT_STATE_IDLE;
            }
            break;
        //
        case REAGENT_STATE_MOVE_TO_REACT:
            eErr = TestProc_Api_ReagentMoveToPos(TSC_NEEDLE_REAGENT_BEAD_POS_REACTION);
            if (LH_ERR_NONE == eErr)
            {
                FSM_Reagent = REAGENT_STATE_MOVE_TO_REACT_ING;
            }
            else
            {
                s32Result = TESTPROC_RESULT_ERR_EXEC;
            }
            break;
        case REAGENT_STATE_MOVE_TO_REACT_ING:
            eCheckResult = TestProc_Api_ReagentCheckResult(&eErrCode);
            if (TESTPROC_API_RESULT_OK == eCheckResult)
            {
                TestProc_SendMsg(TESTPROC_MSG_CMD_REAGENT_ON_REACT, TESTPROC_UNIT_REAGENT, NULL, 0);
                FSM_Reagent = REAGENT_STATE_IDLE;
            }
            else if (TESTPROC_API_RESULT_NG == eCheckResult)
            {
                s32Result = TESTPROC_RESULT_NG;
                FSM_Reagent = REAGENT_STATE_IDLE;
            }
            break;
        default:

            break;
    }

    if (REAGENT_STATE_IDLE == FSM_Reagent)
    {
        g_ReagentTimeStart = TESTPROC_TICK_INVALID;
        if (s32Result == TESTPROC_RESULT_NG && eErrCode != LH_ERR_NONE && g_eReagentActionWarning[FSM_old] != TESTPROC_WARNING_NONE)
        {
            DebugMsg("E>ReagentWorkNG:%d,%d,%08X,%08X\r\n", g_eReagentWorking, FSM_old, eErrCode, eErr);
            *(uint16_t *)u8TxBuf[0] = g_eReagentActionWarning[FSM_old];
            u8TxBuf[2] = TESTPROC_UNIT_REAGENT;
            u8TxBuf[3] = FSM_old;
            u8TxBuf[4] = WORD_TO_BYTE_LL(eErrCode);
            u8TxBuf[5] = WORD_TO_BYTE_LL(eErrCode);
            u8TxBuf[6] = WORD_TO_BYTE_LL(eErrCode);
            u8TxBuf[7] = WORD_TO_BYTE_LL(eErrCode);
            TestProc_SendMsg(TESTPROC_MSG_CMD_EXEC_NG, TESTPROC_UNIT_REAGENT, u8TxBuf, 8);
        }
    }
    else
    {
        if (tick > g_ReagentTimeStart && (tick-g_ReagentTimeStart) > g_stReagentWorkState[g_eReagentWorking].TimeOut)
        {
            DebugMsg("E>ReagentWorkTimeout:%d,%d,%d,%d,%d,%d\r\n", g_eReagentWorking, FSM_Reagent, FSM_old, g_stReagentWorkState[g_eReagentWorking].TimeOut, tick, g_ReagentTimeStart);
            g_ReagentTimeStart = TESTPROC_TICK_INVALID;
            if (g_stReagentWorkState[g_eReagentWorking].eWarning < TESTPROC_WARNING_NONE)
            {
                *(uint16_t *)&u8TxBuf[0] = g_stReagentWorkState[g_eReagentWorking].eWarning;
                u8TxBuf[2] = TESTPROC_UNIT_REAGENT;
                u8TxBuf[3] = FSM_Reagent;
                TestProc_SendMsg(TESTPROC_MSG_CMD_TIMEOUT, TESTPROC_UNIT_EVENT, u8TxBuf, 4);
            }
        }
    }
    
    if (TESTPROC_RESULT_OK != s32Result)
    {
        DebugMsg("E>ReagentFSM:%d,%08X,%d,%d,%08X\r\n", s32Result, eErrCode, FSM_Reagent, FSM_old, eErr);
    }
}


