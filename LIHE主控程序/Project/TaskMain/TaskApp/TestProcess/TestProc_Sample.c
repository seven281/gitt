#include "TestProc_Sample.h"

typedef enum
{
    FSM_SAMPLE_RESET_IDLE = 0,
    FSM_SAMPLE_RESET_SAMPLE_ING,        // 样本针复位中
    FSM_SAMPLE_RESET_CONVEYORBELT_ING,  // 传动带复位中
}FSM_SAMPLE_RESET_E;

typedef enum
{
    CONVEYORBELT_STATE_IDLE = 0,     // 0  
    CONVEYORBELT_STATE_MOVETOPOS,
    CONVEYORBELT_STATE_MOVING,
    CONVEYORBELT_STATE_MAX
}CONVEYORBELT_STATE_E;
    
typedef enum
{
    SAMPLE_STATE_IDLE = 0,
    SAMPLE_STATE_RESET,
    SAMPLE_STATE_TO_RACK,                // 2
    SAMPLE_STATE_TO_RACK_ING,
    SAMPLE_STATE_TO_ABSORB,
    SAMPLE_STATE_ABSORB_TO_REACT,        // 5
    SAMPLE_STATE_ABSORB_ON_REACT,
    SAMPLE_STATE_INJECT,
    SAMPLE_STATE_INJECT_ING,             // 8
    SAMPLE_STATE_FORCE_CLEAN,            // 9
    SAMPLE_STATE_FORCE_CLEAN_MOVE_ING,   // 10
    SAMPLE_STATE_CLEAN,                  // 11
    SAMPLE_STATE_MOVE_ING,               // 12
    SAMPLE_STATE_CLEAN_ING,
    SAMPLE_STATE_MAX
}SAMPLE_STATE_E;

typedef struct
{
    SAMPLE_STATE_E eSampleState;
    OS_TICK TimeOut;
    TESTPROC_WARNING_E  eWarning;
}SAMPLE_WORK_T;

typedef struct
{
    CONVEYORBELT_STATE_E eConveyorBeltState;
    OS_TICK TimeOut;
    TESTPROC_WARNING_E  eWarning;
}CONVEYORBELT_WORK_T;

#define TestProc_SampleCleanFSM()                   \
    do                                              \
    {                                               \
        FSM_Sample = SAMPLE_STATE_IDLE;              \
    }while(0)
#define TESTPROC_INVALID_SAMPLE_POS                     (0xFF)

static SAMPLE_STATE_E FSM_Sample;
static CONVEYORBELT_STATE_E FSM_ConveyorBelt;
static FSM_SAMPLE_RESET_E FSM_Sample_Reset;
static uint16_t  g_u16Quantity = 0;
static uint8_t   g_u8SamplePos = TESTPROC_INVALID_SAMPLE_POS;
static uint8_t   g_u8SamplePosChanged = 1;
static OS_TICK g_SampleTimeStart = TESTPROC_TICK_INVALID;
static SAMPLE_WORK_E g_eSampleWorking = SAMPLE_WORK_RESET;
static OS_TICK g_ConveyorBeltTimeStart = TESTPROC_TICK_INVALID;
static CONVEYORBELT_WORK_E g_eConveyorBeltWorking = CONVEYORBELT_WORK_MOVETOPOS;

static const SAMPLE_WORK_T g_stSampleWork[SAMPLE_WORK_MAX] = 
{
    { SAMPLE_STATE_RESET,               TESTPROC_TICK_INVALID,          TESTPROC_WARNING_NONE },
    { SAMPLE_STATE_TO_RACK,             600,                            TESTPROC_WARNING_TIMEOUT_SAMPLE_TO_REACT },     // 500
    { SAMPLE_STATE_TO_ABSORB,           5000,                           TESTPROC_WARNING_TIMEOUT_SAMPLE_ABSORB },       // 1600
    { SAMPLE_STATE_INJECT,              1200,                           TESTPROC_WARNING_TIMEOUT_SAMPLE_INJECT },       // 1100
    { SAMPLE_STATE_FORCE_CLEAN,         9300,                           TESTPROC_WARNING_TIMEOUT_SAMPLE_FORCE_CLEAN },  // 9000
    { SAMPLE_STATE_CLEAN,               3300,                           TESTPROC_WARNING_TIMEOUT_SAMPLE_CLEAN }         // 3100
};

static const char g_strSampleWorkName[SAMPLE_WORK_MAX][16] =
{
    "Reset",
    "ToRack",
    "ToAbsorb",
    "ToInject",
    "ForceClean",
    "Clean"
};
    
static const CONVEYORBELT_WORK_T g_stConveyorBeltWork[CONVEYORBELT_WORK_MAX] = 
{
    { CONVEYORBELT_STATE_MOVETOPOS,     2200,                           TESTPROC_WARNING_TIMEOUT_CB_MOVE_POSITION },
    { CONVEYORBELT_STATE_MOVETOPOS,     2200,                           TESTPROC_WARNING_TIMEOUT_CB_RECYCLE }           // 2000
};

static const TESTPROC_WARNING_E g_eSampleActionWarning[SAMPLE_STATE_MAX] = 
{
    /*00*/ TESTPROC_WARNING_NONE,                           // SAMPLE_STATE_IDLE
    /*01*/ TESTPROC_WARNING_NONE,                           // SAMPLE_STATE_RESET
    /*02*/ TESTPROC_WARNING_NONE,                           // SAMPLE_STATE_TO_RACK
    /*03*/ TESTPROC_WARNING_API_NG_SAMPLE_ROTATE,           // SAMPLE_STATE_TO_RACK_ING
    /*04*/ TESTPROC_WARNING_NONE,                           // SAMPLE_STATE_TO_ABSORB
    /*05*/ TESTPROC_WARNING_API_NG_ABSORB_SAMPLE,           // SAMPLE_STATE_ABSORB_TO_REACT
    /*06*/ TESTPROC_WARNING_API_NG_SAMPLE_ROTATE,           // SAMPLE_STATE_ABSORB_ON_REACT
    /*07*/ TESTPROC_WARNING_NONE,                           // SAMPLE_STATE_INJECT
    /*08*/ TESTPROC_WARNING_API_NG_INJECT_SAMPLE,           // SAMPLE_STATE_INJECT_ING
    /*09*/ TESTPROC_WARNING_NONE,                           // SAMPLE_STATE_FORCE_CLEAN
    /*10*/ TESTPROC_WARNING_API_NG_SAMPLE_ROTATE,           // SAMPLE_STATE_FORCE_CLEAN_MOVE_ING
    /*11*/ TESTPROC_WARNING_API_NG_SAMPLE_FORCE_CLEAN,      // SAMPLE_STATE_CLEAN
    /*12*/ TESTPROC_WARNING_API_NG_SAMPLE_ROTATE,           // SAMPLE_STATE_MOVE_ING
    /*13*/ TESTPROC_WARNING_API_NG_SAMPLE_CLEAN,            // SAMPLE_STATE_CLEAN_ING
};

static const TESTPROC_WARNING_E g_eConveyorBeltActionWarning[CONVEYORBELT_STATE_MAX] = 
{
    /*00*/ TESTPROC_WARNING_NONE,                           // CONVEYORBELT_STATE_IDLE
    /*01*/ TESTPROC_WARNING_NONE,                           // CONVEYORBELT_STATE_MOVETOPOS
    /*02*/ TESTPROC_WARNING_API_NG_CONVEYORBELT_MOVE,       // CONVEYORBELT_STATE_MOVING
};

/*
static const char g_strCBWorkName[CONVEYORBELT_WORK_MAX][16] =
{
    "MoveToPos",
    "ToRecycle"
};
*/
int TestProc_SampleInit(void)
{
    //TestProc_Api_SampleInit();
    //TestProc_Api_SampleConveyorBeltInit();
    FSM_Sample = SAMPLE_STATE_IDLE;
    FSM_Sample_Reset = FSM_SAMPLE_RESET_IDLE;
    FSM_ConveyorBelt = CONVEYORBELT_STATE_IDLE;
    g_u8SamplePos = TESTPROC_INVALID_SAMPLE_POS;
    g_u8SamplePosChanged = 1;

    return TESTPROC_RESULT_OK;
}

int TestProc_SampleStartWork(SAMPLE_WORK_E eWork, uint8_t *pu8Param, uint8_t u8Size)
{
    int result = TESTPROC_RESULT_OK;
    OS_ERR err = OS_ERR_NONE;

    if (SAMPLE_STATE_IDLE == FSM_Sample && eWork < SAMPLE_WORK_MAX)
    {
        FSM_Sample = g_stSampleWork[eWork].eSampleState;
        g_eSampleWorking = eWork;
        g_SampleTimeStart = OSTimeGet(&err);
        if (SAMPLE_STATE_TO_ABSORB == FSM_Sample)
        {
            g_u16Quantity = *(uint16_t *)(&pu8Param[0]);
        }
        DebugMsg(" >>SampleWork >> %s\r\n", g_strSampleWorkName[eWork]);
    }
    else
    {
        result = TESTPROC_RESULT_ERR_STATUS;
        DebugMsg("E>StartSample:%d,%d\r\n", FSM_Sample, eWork);
    }

    return result;
}

static TESTPROC_API_RESULT_E TestProc_SampleResetFSM(void)
{
    TESTPROC_API_RESULT_E eActionResult = TESTPROC_API_RESULT_WAIT;
    LH_ERR eErr = LH_ERR_NONE;
    uint8_t u8VauleTmp;

    if(FSM_SAMPLE_RESET_IDLE == FSM_Sample_Reset)
    {
        TestProc_Api_SampleReset();
        FSM_Sample_Reset = FSM_SAMPLE_RESET_SAMPLE_ING;
    }
    else if (FSM_SAMPLE_RESET_SAMPLE_ING == FSM_Sample_Reset)
    {
        eActionResult = TestProc_Api_SampleCheckResult(&eErr);
        if (TestProc_Api_IsResetCompile(eActionResult))
        {
            if (TESTPROC_API_RESULT_OK ==eActionResult)
            {
                TestProc_Api_SampleConveyorBeltReset();
                FSM_Sample_Reset = FSM_SAMPLE_RESET_CONVEYORBELT_ING;
                //
                u8VauleTmp = OK;
                TestProc_SendMsg(TESTPROC_MSG_CMD_SAMPLENEEDLE_RESET, TESTPROC_UNIT_SAMPLE, &u8VauleTmp, 1);
            }
            else
            {
                DebugMsg("Sample Reset:%08X\r\n", eErr);
                FSM_Sample_Reset = FSM_SAMPLE_RESET_IDLE;
            }
        }
        eActionResult = TESTPROC_API_RESULT_WAIT;
    }    
    else if (FSM_SAMPLE_RESET_CONVEYORBELT_ING == FSM_Sample_Reset)
    {
        eActionResult = TestProc_Api_HandCupCheckResult(&eErr);
        if (TestProc_Api_IsResetCompile(eActionResult))
        {
            FSM_Sample_Reset = FSM_SAMPLE_RESET_IDLE;
        }
    }

    return eActionResult;
}


void TestProc_SampleFSM(OS_TICK tick)
{
    TESTPROC_API_RESULT_E eCheckResult;
    int32_t  s32Result = TESTPROC_RESULT_OK;
    LH_ERR eErr;
    uint8_t  u8TxBuf[8];
    SAMPLE_STATE_E FSM_Old = FSM_Sample;

    switch(FSM_Sample)
    {
        case SAMPLE_STATE_RESET:
            eCheckResult = TestProc_SampleResetFSM();
            if (TestProc_Api_IsResetCompile(eCheckResult))
            {
                u8TxBuf[0] = TESTPROC_API_RESULT_OK == eCheckResult ? OK : NG;
                TestProc_SendMsg(TESTPROC_MSG_CMD_RESET, TESTPROC_UNIT_SAMPLE, u8TxBuf, 1);
                TestProc_SampleCleanFSM();
            }
            break;
        case SAMPLE_STATE_TO_RACK:
            eErr =TestProc_Api_SampleMoveToPos(TSC_NEEDLE_SAMPLE_POS_RACK);
            if (LH_ERR_NONE == eErr)
            {
                FSM_Sample = SAMPLE_STATE_TO_RACK_ING;
            }
            else
            {
                s32Result = TESTPROC_RESULT_ERR_EXEC;
                FSM_Sample = SAMPLE_STATE_IDLE;
            }
            break;
        case SAMPLE_STATE_TO_RACK_ING:
            eCheckResult = TestProc_Api_SampleCheckResult(&eErr);
            if (TESTPROC_API_RESULT_OK == eCheckResult)
            {
                TestProc_SendMsg(TESTPROC_MSG_CMD_SAMPLE_ONRACK, TESTPROC_UNIT_SAMPLE, NULL, 0);
                FSM_Sample = SAMPLE_STATE_IDLE;
            }
            else if (TESTPROC_API_RESULT_NG == eCheckResult)
            {
                s32Result = TESTPROC_RESULT_NG;
                FSM_Sample = SAMPLE_STATE_IDLE;
            }
            break;
        //
        case SAMPLE_STATE_TO_ABSORB:
            eErr =TestProc_Api_SampleAbsorb(g_u16Quantity, g_u8SamplePosChanged);
            if (LH_ERR_NONE == eErr)
            {
                FSM_Sample = SAMPLE_STATE_ABSORB_TO_REACT;
            }
            else
            {
                s32Result = TESTPROC_RESULT_ERR_EXEC;
                FSM_Sample = SAMPLE_STATE_IDLE;
            }
            break;
        case SAMPLE_STATE_ABSORB_TO_REACT:
            eCheckResult = TestProc_Api_SampleCheckResult(&eErr);
            if (TESTPROC_API_RESULT_OK == eCheckResult)
            {
                TestProc_SendMsg(TESTPROC_MSG_CMD_SAMPLE_ABSORB_OK, TESTPROC_UNIT_SAMPLE, NULL, 0);
                eErr = TestProc_Api_SampleMoveToPos(TSC_NEEDLE_SAMPLE_POS_REACTION_INJECT);
                if (LH_ERR_NONE == eErr)
                {
                    FSM_Sample = SAMPLE_STATE_ABSORB_ON_REACT;
                }
                else
                {
                    s32Result = TESTPROC_RESULT_ERR_EXEC;
                }
            }
            else if (TESTPROC_API_RESULT_NG == eCheckResult)
            {
                s32Result = TESTPROC_RESULT_NG;
                FSM_Sample = SAMPLE_STATE_IDLE;
            }
            break;
        case SAMPLE_STATE_ABSORB_ON_REACT:
            eCheckResult = TestProc_Api_SampleCheckResult(&eErr);
            if (TESTPROC_API_RESULT_OK == eCheckResult)
            {
                TestProc_SendMsg(TESTPROC_MSG_CMD_SAMPLE_INJECT_READY, TESTPROC_UNIT_SAMPLE, NULL, 0);
                FSM_Sample = SAMPLE_STATE_IDLE;
            }
            else if (TESTPROC_API_RESULT_NG == eCheckResult)
            {
                s32Result = TESTPROC_RESULT_NG;
                FSM_Sample = SAMPLE_STATE_IDLE;
            }
            break;
        //
        case SAMPLE_STATE_INJECT:
            eErr =TestProc_Api_SampleInject(g_u16Quantity);
            if (LH_ERR_NONE == eErr)
            {
                FSM_Sample = SAMPLE_STATE_INJECT_ING;
            }
            else
            {
                s32Result = TESTPROC_RESULT_ERR_EXEC;
                FSM_Sample = SAMPLE_STATE_IDLE;
            }
            break;
        case SAMPLE_STATE_INJECT_ING:
            eCheckResult = TestProc_Api_SampleCheckResult(&eErr);
            if (TESTPROC_API_RESULT_OK == eCheckResult)
            {
                TestProc_SendMsg(TESTPROC_MSG_CMD_SAMPLE_INJECT_OK, TESTPROC_UNIT_SAMPLE, NULL, 0);
                FSM_Sample = SAMPLE_STATE_IDLE;
            }
            else if (TESTPROC_API_RESULT_NG == eCheckResult)
            {
                s32Result = TESTPROC_RESULT_NG;
                FSM_Sample = SAMPLE_STATE_IDLE;
            }
            break;
        //
        case SAMPLE_STATE_FORCE_CLEAN:
            eErr =TestProc_Api_SampleMoveToPos(TSC_NEEDLE_SAMPLE_POS_FORCE_CLEAN);
            if (LH_ERR_NONE == eErr)
            {
                FSM_Sample = SAMPLE_STATE_FORCE_CLEAN_MOVE_ING;
            }
            else
            {
                s32Result = TESTPROC_RESULT_ERR_EXEC;
                FSM_Sample = SAMPLE_STATE_IDLE;
            }
            break;
        case SAMPLE_STATE_FORCE_CLEAN_MOVE_ING:
            eCheckResult = TestProc_Api_SampleCheckResult(&eErr);
            if (TESTPROC_API_RESULT_OK == eCheckResult)
            {
                eErr =TestProc_Api_SampleForceClean(TSC_NEEDLE_SAMPLE_POS_CLEAN, TSC_NEEDLE_SAMPLE_CLEAN_OPT_ALL);
                if (LH_ERR_NONE == eErr)
                {
                    FSM_Sample = SAMPLE_STATE_CLEAN;
                }
                else
                {
                    s32Result = TESTPROC_RESULT_ERR_EXEC;
                    FSM_Sample = SAMPLE_STATE_IDLE;
                }
            }
            else if (TESTPROC_API_RESULT_NG == eCheckResult)
            {
                s32Result = TESTPROC_RESULT_NG;
                FSM_Sample = SAMPLE_STATE_IDLE;
            }
            break;
        //
        case SAMPLE_STATE_CLEAN:
            eCheckResult = TestProc_Api_SampleCheckResult(&eErr);
            if (TESTPROC_API_RESULT_OK == eCheckResult)
            {
                eErr =TestProc_Api_SampleMoveToPos(TSC_NEEDLE_SAMPLE_POS_CLEAN);
                if (LH_ERR_NONE == eErr)
                {
                    FSM_Sample = SAMPLE_STATE_MOVE_ING;
                }
                else
                {
                    s32Result = TESTPROC_RESULT_ERR_EXEC;
                    FSM_Sample = SAMPLE_STATE_IDLE;
                }
            }
            else if (TESTPROC_API_RESULT_NG == eCheckResult)
            {
                s32Result = TESTPROC_RESULT_NG;
                FSM_Sample = SAMPLE_STATE_IDLE;
            }
            break;
        case SAMPLE_STATE_MOVE_ING:
            eCheckResult = TestProc_Api_SampleCheckResult(&eErr);
            if (TESTPROC_API_RESULT_OK == eCheckResult)
            {
                eErr = TestProc_Api_SampleClean(TSC_NEEDLE_SAMPLE_CLEAN_OPT_ALL);
                if (LH_ERR_NONE == eErr)
                {
                    FSM_Sample = SAMPLE_STATE_CLEAN_ING;
                }
                else
                {
                    s32Result = TESTPROC_RESULT_ERR_EXEC;
                }
            }
            else if (TESTPROC_API_RESULT_NG == eCheckResult)
            {
                s32Result = TESTPROC_RESULT_NG;
                FSM_Sample = SAMPLE_STATE_IDLE;
            }
            break;
        case SAMPLE_STATE_CLEAN_ING:
            eCheckResult = TestProc_Api_SampleCheckResult(&eErr);
            if (TESTPROC_API_RESULT_OK == eCheckResult)
            {
                TestProc_SendMsg(TESTPROC_MSG_CMD_SAMPLE_CLEAN_OK, TESTPROC_UNIT_SAMPLE, NULL, 0);
                FSM_Sample = SAMPLE_STATE_IDLE;
            }
            else if (TESTPROC_API_RESULT_NG == eCheckResult)
            {
                s32Result = TESTPROC_RESULT_NG;
                FSM_Sample = SAMPLE_STATE_IDLE;
            }
            break;
        case SAMPLE_STATE_IDLE:
        default:

            break;
    }
    
    if (SAMPLE_STATE_IDLE == FSM_Sample)
    {
        g_SampleTimeStart = TESTPROC_TICK_INVALID;
        if (s32Result == TESTPROC_RESULT_NG && eErr != LH_ERR_NONE && g_eSampleActionWarning[FSM_Old] != TESTPROC_WARNING_NONE)
        {
            DebugMsg("E>SampleWorkNG:%d,%d,%08X,%08X\r\n", g_eSampleWorking, FSM_Old, eErr);
            *(uint16_t *)u8TxBuf[0] = g_eSampleActionWarning[FSM_Old];
            u8TxBuf[2] = TESTPROC_UNIT_SAMPLE;
            u8TxBuf[3] = FSM_Old;
            u8TxBuf[4] = WORD_TO_BYTE_LL(eErr);
            u8TxBuf[5] = WORD_TO_BYTE_LL(eErr);
            u8TxBuf[6] = WORD_TO_BYTE_LL(eErr);
            u8TxBuf[7] = WORD_TO_BYTE_LL(eErr);
            TestProc_SendMsg(TESTPROC_MSG_CMD_EXEC_NG, TESTPROC_UNIT_SAMPLE, u8TxBuf, 8);
        }
    }
    else
    {
        if (tick > g_SampleTimeStart && (tick-g_SampleTimeStart) > g_stSampleWork[g_eSampleWorking].TimeOut)
        {
            DebugMsg("E>SampleWorkTimeout:%d,%d,%d,%d,%d,%d\r\n", g_eSampleWorking, FSM_Sample, FSM_Old, g_stSampleWork[g_eSampleWorking].TimeOut, tick, g_SampleTimeStart);
            g_SampleTimeStart = TESTPROC_TICK_INVALID;
            if (g_stSampleWork[g_eSampleWorking].eWarning < TESTPROC_WARNING_NONE)
            {
                *(uint16_t *)&u8TxBuf[0] = g_stSampleWork[g_eSampleWorking].eWarning;
                u8TxBuf[2] = TESTPROC_UNIT_SAMPLE;
                u8TxBuf[3] = FSM_Sample;
                TestProc_SendMsg(TESTPROC_MSG_CMD_TIMEOUT, TESTPROC_UNIT_EVENT, u8TxBuf, 4);
            }
        }
    }
    
    if (TESTPROC_RESULT_OK != s32Result)
    {
        DebugMsg("E>SampleFSM:%d,%d,%d,%d\r\n", s32Result, FSM_Sample, FSM_Old, eErr);
    }
}


int TestProc_ConveyorBeltStartWork(CONVEYORBELT_WORK_E eWork, uint8_t *pu8Param, uint8_t u8Size)
{
    int result = TESTPROC_RESULT_OK;
    OS_ERR err = OS_ERR_NONE;

    if (CONVEYORBELT_STATE_IDLE == FSM_ConveyorBelt && eWork < CONVEYORBELT_WORK_MAX)
    {
        FSM_ConveyorBelt = g_stConveyorBeltWork[eWork].eConveyorBeltState;
        g_eConveyorBeltWorking = eWork;
        g_ConveyorBeltTimeStart = OSTimeGet(&err);
        if (CONVEYORBELT_WORK_MOVETOPOS == eWork)
        {
            FSM_ConveyorBelt = CONVEYORBELT_STATE_MOVETOPOS;
            if (g_u8SamplePos != pu8Param[0])
            {
                g_u8SamplePos = pu8Param[0];
                g_u8SamplePosChanged = 1;
            }
            else
            {
                g_u8SamplePosChanged = 0;
            }
        }        
        else if (CONVEYORBELT_WORK_TO_RECYCLE == eWork)
        {
            FSM_ConveyorBelt = CONVEYORBELT_STATE_MOVETOPOS;
            g_u8SamplePos = SAMPLE_POS_RECYCLE;
        }
        //DebugMsg(" **CB_Work >> %s\r\n", g_strCBWorkName[eWork]);
    }
    else
    {
        result = TESTPROC_RESULT_ERR_STATUS;
        DebugMsg("E>StartCB:%d,%d\r\n", FSM_Sample, eWork);
    }

    return result;
}


void TestProc_SampleCBFSM(OS_TICK tick)
{
    TESTPROC_API_RESULT_E eCheckResult;
    int32_t  s32Result = TESTPROC_RESULT_OK;
    LH_ERR eErr;
    uint8_t  u8TxBuf[8];
    CONVEYORBELT_STATE_E FSM_Old = FSM_ConveyorBelt;

    switch(FSM_ConveyorBelt)
    {
        case CONVEYORBELT_STATE_MOVETOPOS:
            if (0 == g_u8SamplePos)
            {
                u8TxBuf[0] = g_u8SamplePos;
                TestProc_SendMsg(TESTPROC_MSG_CMD_SAMPLE_RACK_INPOS, TESTPROC_UNIT_SAMPLE, u8TxBuf, 1);
                FSM_ConveyorBelt = CONVEYORBELT_STATE_IDLE;
            }
            else
            {
                eErr =TestProc_Api_SampleConveyorBeltMoveToPos(g_u8SamplePos);
                if (LH_ERR_NONE == eErr)
                {
                    FSM_ConveyorBelt = CONVEYORBELT_STATE_MOVING;
                }
                else
                {
                    s32Result = TESTPROC_RESULT_ERR_EXEC;
                    FSM_ConveyorBelt = CONVEYORBELT_STATE_IDLE;
                }
            }
            break;
        case CONVEYORBELT_STATE_MOVING:
            eCheckResult = TestProc_Api_SampleConveyorBeltCheckResult(&eErr);
            if (TESTPROC_API_RESULT_OK == eCheckResult)
            {
                u8TxBuf[0] = g_u8SamplePos;
                TestProc_SendMsg(TESTPROC_MSG_CMD_SAMPLE_RACK_INPOS, TESTPROC_UNIT_SAMPLE, u8TxBuf, 1);
                FSM_ConveyorBelt = CONVEYORBELT_STATE_IDLE;
                if (SAMPLE_POS_RECYCLE == g_u8SamplePos)
                {
                    g_u8SamplePos = TESTPROC_INVALID_SAMPLE_POS;
                }
            }
            else if (TESTPROC_API_RESULT_NG == eCheckResult)
            {
                s32Result = TESTPROC_RESULT_NG;
                FSM_ConveyorBelt = CONVEYORBELT_STATE_IDLE;
            }
            break;
        case CONVEYORBELT_STATE_IDLE:
        default:
            
            break;
    }
    
    if (CONVEYORBELT_STATE_IDLE == FSM_ConveyorBelt)
    {
        g_ConveyorBeltTimeStart = TESTPROC_TICK_INVALID;
        if (s32Result == TESTPROC_RESULT_NG && eErr != LH_ERR_NONE && g_eConveyorBeltActionWarning[FSM_Old] != TESTPROC_WARNING_NONE)
        {
            DebugMsg("E>ConveyorBeltWorkNG:%d,%d,%08X,%08X\r\n", g_eConveyorBeltWorking, FSM_Old, eErr);
            *(uint16_t *)u8TxBuf[0] = g_eConveyorBeltActionWarning[FSM_Old];
            u8TxBuf[2] = TESTPROC_UNIT_SAMPLE;
            u8TxBuf[3] = FSM_Old;
            u8TxBuf[4] = WORD_TO_BYTE_LL(eErr);
            u8TxBuf[5] = WORD_TO_BYTE_LL(eErr);
            u8TxBuf[6] = WORD_TO_BYTE_LL(eErr);
            u8TxBuf[7] = WORD_TO_BYTE_LL(eErr);
            TestProc_SendMsg(TESTPROC_MSG_CMD_EXEC_NG, TESTPROC_UNIT_SAMPLE, u8TxBuf, 8);
        }
    }
    else
    {
        if (tick > g_ConveyorBeltTimeStart && (tick-g_ConveyorBeltTimeStart) > g_stConveyorBeltWork[g_eConveyorBeltWorking].TimeOut)
        {
            DebugMsg("E>ConveyorBeltWorkTimeout:%d,%d,%d,%d,%d,%d\r\n", g_eConveyorBeltWorking, FSM_ConveyorBelt, FSM_Old, g_stConveyorBeltWork[g_eConveyorBeltWorking].TimeOut, tick, g_ConveyorBeltTimeStart);
            g_ConveyorBeltTimeStart = TESTPROC_TICK_INVALID;
            if (g_stConveyorBeltWork[g_eConveyorBeltWorking].eWarning < TESTPROC_WARNING_NONE)
            {
                *(uint16_t *)&u8TxBuf[0] = g_stConveyorBeltWork[g_eConveyorBeltWorking].eWarning;
                u8TxBuf[2] = TESTPROC_UNIT_CUPSTACK;
                u8TxBuf[3] = FSM_ConveyorBelt;
                TestProc_SendMsg(TESTPROC_MSG_CMD_TIMEOUT, TESTPROC_UNIT_EVENT, u8TxBuf, 4);
            }
        }
    }
    
    if (TESTPROC_RESULT_OK != s32Result)
    {
        DebugMsg("E>CB_FSM:%d,%d,%d\r\n", s32Result, FSM_ConveyorBelt, FSM_Old, eErr);
    }
}

