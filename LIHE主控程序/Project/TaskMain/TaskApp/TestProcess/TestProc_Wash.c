#include "TestProc_Wash.h"

#define MIX_BIT                     (0x01)
#define NEEDLE_BIT                  (0x02)
#define LIQUIDA_BIT                 (0x04)
#define MEASUREROOM_BIT             (0x08)
#define FIRST_STEP_BIT              (0x0F)

typedef enum
{
    WASH_STATE_IDLE = 0,                 // 0
    WASH_STATE_RESET,                    // 1
    WASH_STATE_ROTATE,                   // 2
    WASH_STATE_ROTATE_ING,               // 3
    WASK_STATE_WHILE_STOP,               // 4 第一次清洗、注液A、摇匀
    WASK_STATE_WHILE_STOP_ING,           // 5
    WASH_STATE_NEEDLE_WASHING_2ND,       // 6 第二次清洗
    WASH_STATE_NEEDLE_WASHING_2ND_ING,   // 7 第二次清洗
    WASH_STATE_WASH_STANDBY,             // 8
    WASH_STATE_MAX
}WASH_STATE_E;

typedef enum
{
    FSM_WASH_RESET_IDLE = 0,
    FSM_WASH_RESET_STEP1_ING, // 清洗针、注液A、测量室、混匀复位
    FSM_WASH_RESET_DISK_ING,  // 清洗机构复位中
} FSM_WASH_RESET_E;

typedef enum
{
    MEASUREROOM_STATE_IDLE = 0,             // 0
    MEASUREROOM_STATE_FULLOPEN,             // 1
    MEASUREROOM_STATE_FULLOPEN_ING,         // 2
    MEASUREROOM_STATE_HALFOPEN,             // 3
    MEASUREROOM_STATE_HALFOPEN_ING,         // 4
    MEASUREROOM_STATE_CLOSE,                // 5
    MEASUREROOM_STATE_CLOSE_ING,            // 6
    MEASUREROOM_STATE_MEASURE,              // 7
    MEASUREROOM_STATE_MEASURE_STEP1,        // 8
    MEASUREROOM_STATE_MEASURE_STEP2,        // 9
    MEASUREROOM_STATE_MAX
} MEASUREROOM_STATE_E;

typedef struct
{
    WASH_STATE_E eWashState;
    OS_TICK TimeOut;
    TESTPROC_WARNING_E  eWarning;
}WASH_WORK_T;

typedef struct
{
    MEASUREROOM_STATE_E eMeasureRoomState;
    OS_TICK TimeOut;
    TESTPROC_WARNING_E  eWarning;
}MEASUREROOM_WORK_T;

#define TestProc_WashCleanFSM()    \
    do                             \
    {                              \
        FSM_Wash = WASH_STATE_IDLE; \
    } while (0)


static const WASH_WORK_T g_stWashWorkState[WASH_WORK_MAX] =
{
    { WASH_STATE_RESET,             TESTPROC_TICK_INVALID,      TESTPROC_WARNING_NONE },
    { WASH_STATE_ROTATE,            600,                        TESTPROC_WARNING_TIMEOUT_WASH_ROTATE },       // 300
    { WASK_STATE_WHILE_STOP,        18000,                      TESTPROC_WARNING_TIMEOUT_WASH_CLEAN },        // 17700
    { WASH_STATE_WASH_STANDBY,      TESTPROC_TICK_INVALID,      TESTPROC_WARNING_NONE }
};

static const TESTPROC_WARNING_E g_eWashActionWarning[WASH_STATE_MAX] = 
{
    /*00*/ TESTPROC_WARNING_NONE,                           // WASH_STATE_IDLE
    /*01*/ TESTPROC_WARNING_NONE,                           // WASH_STATE_RESET
    /*02*/ TESTPROC_WARNING_NONE,                           // WASH_STATE_ROTATE
    /*03*/ TESTPROC_WARNING_API_NG_WASHDISK_ROTATE,         // WASH_STATE_ROTATE_ING
    /*04*/ TESTPROC_WARNING_NONE,                           // WASK_STATE_WHILE_STOP
    /*05*/ TESTPROC_WARNING_NONE,                           // WASK_STATE_WHILE_STOP_ING
    /*06*/ TESTPROC_WARNING_API_NG_WASHNEEDLE_CLEAN1,       // WASH_STATE_NEEDLE_WASHING_2ND
    /*07*/ TESTPROC_WARNING_API_NG_WASHNEEDLE_CLEAN2,       // WASH_STATE_NEEDLE_WASHING_2ND_ING
    /*08*/ TESTPROC_WARNING_NONE,                           // WASH_STATE_WASH_STANDBY
};

static WASH_STATE_E FSM_Wash;
static FSM_WASH_RESET_E FSM_Wash_Reset;
static uint8_t g_u8WashWorkMask = 0;
static OS_TICK g_WashTimeStart = TESTPROC_TICK_INVALID;
static WASH_WORK_E g_eWashWorking = WASH_WORK_RESET;

static const MEASUREROOM_WORK_T g_stMeasureRoomWorkState[MEASUREROOM_WORK_MAX] =
{
    { MEASUREROOM_STATE_IDLE,           TESTPROC_TICK_INVALID,      TESTPROC_WARNING_NONE },
    { MEASUREROOM_STATE_FULLOPEN,       500,                        TESTPROC_WARNING_TIMEOUT_MR_FULLOPEN },       // 400
    { MEASUREROOM_STATE_HALFOPEN,       600,                        TESTPROC_WARNING_TIMEOUT_MR_HALFOPEN },       // 500
    { MEASUREROOM_STATE_CLOSE,          500,                        TESTPROC_WARNING_TIMEOUT_MR_CLOSE },          // 400
    { MEASUREROOM_STATE_MEASURE,        TESTPROC_TICK_INVALID,      TESTPROC_WARNING_NONE }
};

static const TESTPROC_WARNING_E g_eMeasureRoomActionWarning[MEASUREROOM_STATE_MAX] = 
{
    /*00*/ TESTPROC_WARNING_NONE,                           // MEASUREROOM_STATE_IDLE
    /*01*/ TESTPROC_WARNING_NONE,                           // MEASUREROOM_STATE_FULLOPEN
    /*02*/ TESTPROC_WARNING_API_NG_MR_FULLOPEN,             // MEASUREROOM_STATE_FULLOPEN_ING
    /*03*/ TESTPROC_WARNING_NONE,                           // MEASUREROOM_STATE_HALFOPEN
    /*04*/ TESTPROC_WARNING_API_NG_MR_HALFOPEN,             // MEASUREROOM_STATE_HALFOPEN_ING
    /*05*/ TESTPROC_WARNING_NONE,                           // MEASUREROOM_STATE_CLOSE
    /*06*/ TESTPROC_WARNING_API_NG_MR_CLOSE,                // MEASUREROOM_STATE_CLOSE_ING
    /*07*/ TESTPROC_WARNING_NONE,                           // MEASUREROOM_STATE_MEASURE
    /*08*/ TESTPROC_WARNING_API_NG_MR_BACK,                 // MEASUREROOM_STATE_MEASURE_STEP1
    /*09*/ TESTPROC_WARNING_API_NG_MR_VALUE,                // MEASUREROOM_STATE_MEASURE_STEP2
};

static MEASUREROOM_STATE_E FSM_MeasureRoom;
static OS_TICK g_MeasureRoomTimeStart = TESTPROC_TICK_INVALID;
static MEASUREROOM_WORK_E g_eMeasureRoomWorking = MEASUREROOM_WORK_IDLE;

/*
static const char g_strWashWorkName[WASH_WORK_MAX][16] =
    {
        "Idle",
        "Reset",
        "Rotate",
        "Rotating",
        "StopWork",
        "StopWorking",
        "Wash",
        "Washing"
};

static const char g_strMeasureRoomName[MEASUREROOM_WORK_MAX][16] =
    {
        "Idle",
        "FullOpen",
        "HalfOpen",
        "Close",
        "Measure",
};
*/
int TestProc_WashInit(void)
{
    //TestProc_Api_WashInit();
    //TestProc_Api_WashLiquidAInit();
    //TestProc_Api_WashMeasureRoomInit();
    //TestProc_Api_WashNeedleInit();

    FSM_Wash = WASH_STATE_IDLE;
    FSM_MeasureRoom = MEASUREROOM_STATE_IDLE;
    FSM_Wash_Reset = FSM_WASH_RESET_IDLE;

    return TESTPROC_RESULT_OK;
}

int TestProc_WashStartWork(WASH_WORK_E eWork, uint8_t *pu8Param, uint8_t u8Size)
{
    int result = TESTPROC_RESULT_OK;
    OS_ERR err = OS_ERR_NONE;

    if (WASH_STATE_IDLE == FSM_Wash && eWork < WASH_WORK_MAX)
    {
        FSM_Wash = g_stWashWorkState[eWork].eWashState;
        g_WashTimeStart = OSTimeGet(&err);
        g_eWashWorking = eWork;
        if (WASK_STATE_WHILE_STOP == FSM_Wash)
        {
            g_u8WashWorkMask = pu8Param[0];
        }
        else if (WASH_STATE_RESET == FSM_Wash)
        {
            if (TestProc_WashResetFSM(TRUE) == TESTPROC_API_RESULT_NG)
            {
                result = TESTPROC_RESULT_ERR_EXEC;
            }
        }
        else if (WASH_STATE_WASH_STANDBY == FSM_Wash)
        {
            FSM_Wash = WASH_STATE_IDLE;
            FSM_Wash_Reset = FSM_WASH_RESET_IDLE;
        }
        //DebugMsg("**WaskWork >> %s\r\n", g_strWashWorkName[eWork]);
    }
    else
    {
        result = TESTPROC_RESULT_ERR_STATUS;
        DebugMsg("E>StartWash:%d,%d\r\n", FSM_Wash, eWork);
    }

    return result;
}

TESTPROC_API_RESULT_E TestProc_WashResetFSM(uint8_t u8StartReset)
{
    TESTPROC_API_RESULT_E eActionResult = TESTPROC_API_RESULT_WAIT;
    LH_ERR err = LH_ERR_NONE;

    if (TRUE == u8StartReset)
    {
        if (FSM_Wash_Reset != FSM_WASH_RESET_IDLE)
        {
            DebugMsg("E>WashResetStart:%d\r\n", FSM_Wash_Reset);
            return TESTPROC_API_RESULT_NG;
        }
    }

    if (FSM_WASH_RESET_IDLE == FSM_Wash_Reset)
    {
        g_u8WashWorkMask = 0;
        FSM_Wash_Reset = FSM_WASH_RESET_STEP1_ING;
        TestProc_Api_WashDiskMixReset();
        TestProc_Api_WashLiquidAReset();
        TestProc_Api_WashMeasureRoomReset();
        TestProc_Api_WashNeedleReset();
    }
    else if (FSM_WASH_RESET_STEP1_ING == FSM_Wash_Reset)
    {
        if ((g_u8WashWorkMask & MIX_BIT) == 0)
        {
            eActionResult = TestProc_Api_WashDiskCheckResult(&err);
            if (TestProc_Api_IsResetCompile(eActionResult))
            {
                g_u8WashWorkMask |= MIX_BIT;
            }
        }
        if ((g_u8WashWorkMask & NEEDLE_BIT) == 0)
        {
            eActionResult = TestProc_Api_WashNeedleCheckResult(&err);
            if (TestProc_Api_IsResetCompile(eActionResult))
            {
                g_u8WashWorkMask |= NEEDLE_BIT;
            }
        }
        if ((g_u8WashWorkMask & LIQUIDA_BIT) == 0)
        {
            eActionResult = TestProc_Api_WashLiquidACheckResult(&err);
            if (TestProc_Api_IsResetCompile(eActionResult))
            {
                g_u8WashWorkMask |= LIQUIDA_BIT;
            }
        }
        if ((g_u8WashWorkMask & MEASUREROOM_BIT) == 0)
        {
            eActionResult = TestProc_Api_WashMeasureRoomCheckResult(&err);
            if (TestProc_Api_IsResetCompile(eActionResult))
            {
                g_u8WashWorkMask |= MEASUREROOM_BIT;
            }
        }
        //
        if ((g_u8WashWorkMask & FIRST_STEP_BIT) == FIRST_STEP_BIT)
        {
            FSM_Wash_Reset = FSM_WASH_RESET_DISK_ING;
            TestProc_Api_WashDiskReset();
        }
        eActionResult = TESTPROC_API_RESULT_WAIT;
    }
    else if (FSM_WASH_RESET_DISK_ING == FSM_Wash_Reset)
    {
        eActionResult = TestProc_Api_WashDiskCheckResult(&err);
        if (TestProc_Api_IsResetCompile(eActionResult))
        {
            FSM_Wash_Reset = FSM_WASH_RESET_IDLE;
        }
    }

    return eActionResult;
}

void TestProc_WashFSM(OS_TICK tick)
{
    int32_t s32Result = TESTPROC_RESULT_OK;
    TESTPROC_API_RESULT_E eCheckResult;
    LH_ERR eActionResult = LH_ERR_NONE;
    LH_ERR err = LH_ERR_NONE;
    uint8_t u8TxBuf[8];
    WASH_STATE_E FSM_Old = FSM_Wash;
    uint8_t u8WashMask = TSC_NEEDLE_WASH_SELECT_NONE;

    switch (FSM_Wash)
    {
        case WASH_STATE_RESET:
            eCheckResult = TestProc_WashResetFSM(FALSE);
            if (TestProc_Api_IsResetCompile(eCheckResult))
            {
                u8TxBuf[0] = TESTPROC_API_RESULT_OK == eCheckResult ? OK : NG;
                TestProc_SendMsg(TESTPROC_MSG_CMD_RESET, TESTPROC_UNIT_WASH, u8TxBuf, 1);
                TestProc_WashCleanFSM();
            }
            break;
        //****************清洗盘转一圈*************************
        case WASH_STATE_ROTATE:
            eActionResult = TestProc_Api_WashDiskRotate1Pos();
            if (LH_ERR_NONE == eActionResult)
            {
                FSM_Wash = WASH_STATE_ROTATE_ING;
            }
            else
            {
                s32Result = TESTPROC_RESULT_NG;
                FSM_Wash = WASH_STATE_IDLE;
            }

            break;
        case WASH_STATE_ROTATE_ING:
            eCheckResult = TestProc_Api_WashDiskCheckResult(&err);
            //DebugMsg("WashDiskRotate:%d\r\n", eCheckResult);
            if (TESTPROC_API_RESULT_OK == eCheckResult)
            {
                TestProc_SendMsg(TESTPROC_MSG_CMD_WASH_DISK_STOP, TESTPROC_UNIT_WASH, NULL, 0);
                FSM_Wash = WASH_STATE_IDLE;
            }
            else if (TESTPROC_API_RESULT_NG == eCheckResult)
            {
                s32Result = TESTPROC_RESULT_NG;
                FSM_Wash = WASH_STATE_IDLE;
            }
            break;
        //***************清洗针清洗、注液A、摇匀*****************
        case WASK_STATE_WHILE_STOP:
            if ((g_u8WashWorkMask & WASH_STOP_WORK_WASH12) > 0)
            {
                u8WashMask |= TSC_NEEDLE_WASH_SELECT_1;
            }
            if ((g_u8WashWorkMask & WASH_STOP_WORK_WASH34) > 0)
            {
                u8WashMask |= TSC_NEEDLE_WASH_SELECT_2;
            }
            if ((g_u8WashWorkMask & WASH_STOP_WORK_WASH56) > 0)
            {
                u8WashMask |= TSC_NEEDLE_WASH_SELECT_3;
            }
            //DebugMsg("WashClean1:%02X,%d\r\n", g_u8WashWorkMask, tick);
            if (u8WashMask > 0)
            {
                eActionResult = TestProc_Api_WashNeedleClean((TSC_NEEDLE_WASH_SELECT_FLAG)u8WashMask, CLEAN_NEEDLE_INDEX_FIRST);
            }
            if ((g_u8WashWorkMask & WASH_STOP_WORK_LIQUIDA) > 0)
            {
                eActionResult = TestProc_Api_WashLiquidAInject();
            }
            if ((g_u8WashWorkMask & WASH_STOP_WORK_MIX) > 0)
            {
                eActionResult = TestProc_Api_WashDiskMixAction(3000); // 6s
            }
            FSM_Wash = WASH_STATE_NEEDLE_WASHING_2ND;
            break;
        case WASH_STATE_NEEDLE_WASHING_2ND:
            eCheckResult = TestProc_Api_WashNeedleCheckResult(&err);
            if (TESTPROC_API_RESULT_OK == eCheckResult)
            {
                u8WashMask = 0;
                if ((g_u8WashWorkMask & WASH_STOP_WORK_WASH12) > 0)
                {
                    u8WashMask |= TSC_NEEDLE_WASH_SELECT_1;
                }
                if ((g_u8WashWorkMask & WASH_STOP_WORK_WASH34) > 0)
                {
                    u8WashMask |= TSC_NEEDLE_WASH_SELECT_2;
                }
                //if ((g_u8WashWorkMask & WASH_STOP_WORK_WASH56) > 0)
                //{
                //    u8WashMask |= TSC_NEEDLE_WASH_SELECT_3;
                //}
                //DebugMsg("WashClean2:%02X,%d\r\n", g_u8WashWorkMask, tick);
                if (u8WashMask > 0)
                {
                    eActionResult = TestProc_Api_WashNeedleClean((TSC_NEEDLE_WASH_SELECT_FLAG)u8WashMask, CLEAN_NEEDLE_INDEX_SECOND);
                }
                FSM_Wash = WASH_STATE_NEEDLE_WASHING_2ND_ING;
                TestProc_SendMsg(TESTPROC_MSG_CMD_WASH_NEEDLE_FIRST_OK, TESTPROC_UNIT_WASH, NULL, 0);
            }
            else if (TESTPROC_API_RESULT_NG == eCheckResult)
            {
                s32Result = TESTPROC_RESULT_NG;
                FSM_Wash = WASH_STATE_IDLE;
            }
            break;
        case WASH_STATE_NEEDLE_WASHING_2ND_ING:
            u8WashMask = 0;
            // Wash
            eCheckResult = TestProc_Api_WashNeedleCheckResult(&err);
            if (TESTPROC_API_RESULT_OK == eCheckResult)
            {
                g_u8WashWorkMask &= ~WASH_STOP_WORK_WASH_MASK;
                u8WashMask |= 0x01;
            }
            else if (TESTPROC_API_RESULT_NG == eCheckResult)
            {
                u8WashMask |= 0x10;
                s32Result = TESTPROC_RESULT_NG;
                FSM_Wash = WASH_STATE_IDLE;
            }
            // Liquid A
            if ((g_u8WashWorkMask & WASH_STOP_WORK_LIQUIDA) > 0)
            {
                eCheckResult = TestProc_Api_WashLiquidACheckResult(&err);
                if (TESTPROC_API_RESULT_OK == eCheckResult)
                {
                    u8WashMask |= 0x02;
                    g_u8WashWorkMask &= ~WASH_STOP_WORK_LIQUIDA;
                }
                else if (TESTPROC_API_RESULT_NG == eCheckResult)
                {
                    s32Result = TESTPROC_RESULT_NG;
                    FSM_Wash = WASH_STATE_IDLE;
                    u8WashMask |= 0x20;
                }
            }
            else
            {
                u8WashMask |= 0x02;
            }
            // Mix
            if ((g_u8WashWorkMask & WASH_STOP_WORK_MIX) > 0)
            {
                eCheckResult = TestProc_Api_WashDiskCheckResult(&err);
                if (TESTPROC_API_RESULT_OK == eCheckResult)
                {
                    g_u8WashWorkMask &= ~WASH_STOP_WORK_MIX;
                    u8WashMask |= 0x04;
                }
                else if (TESTPROC_API_RESULT_NG == eCheckResult)
                {
                    s32Result = TESTPROC_RESULT_NG;
                    FSM_Wash = WASH_STATE_IDLE;
                    u8WashMask |= 0x40;
                }
            }
            else
            {
                u8WashMask |= 0x04;
            }
            //DebugMsg("WORK_WHILE_STOP_CHECK:%02X,%d\r\n", g_u8WashWorkMask, s32Result);
            // check result
            if ((u8WashMask&0x07) == 0x07)
            {
                DebugMsg("Washing OK\r\n");
                FSM_Wash = WASH_STATE_IDLE;
                TestProc_SendMsg(TESTPROC_MSG_CMD_WASH_NEEDLE_SENOND_OK, TESTPROC_UNIT_WASH, NULL, 0);
            }
            break;
        case WASH_STATE_IDLE:
        default:

            break;
    }
    
    if (WASH_STATE_IDLE == FSM_Wash)
    {
        g_WashTimeStart = TESTPROC_TICK_INVALID;
        if (s32Result == TESTPROC_RESULT_NG && err != LH_ERR_NONE && g_eWashActionWarning[FSM_Old] != TESTPROC_WARNING_NONE)
        {
            DebugMsg("E>WashWorkNG:%d,%d,%08X\r\n", g_eWashWorking, FSM_Old, err);
            *(uint16_t *)u8TxBuf[0] = g_eWashActionWarning[FSM_Old];
            u8TxBuf[2] = TESTPROC_UNIT_WASH;
            u8TxBuf[3] = FSM_Old;
            u8TxBuf[4] = WORD_TO_BYTE_LL(err);
            u8TxBuf[5] = WORD_TO_BYTE_LL(err);
            u8TxBuf[6] = WORD_TO_BYTE_LL(err);
            u8TxBuf[7] = WORD_TO_BYTE_LL(err);
            TestProc_SendMsg(TESTPROC_MSG_CMD_EXEC_NG, TESTPROC_UNIT_WASH, u8TxBuf, 8);
        }
    }
    else
    {
        if (tick > g_WashTimeStart && (tick-g_WashTimeStart) > g_stWashWorkState[g_eWashWorking].TimeOut)
        {
            DebugMsg("E>WashWorkTimeout:%d,%d,%d,%d,%d,%d\r\n", g_eWashWorking, FSM_Wash, FSM_Old, g_stWashWorkState[g_eWashWorking].TimeOut, tick, g_WashTimeStart);
            g_WashTimeStart = TESTPROC_TICK_INVALID;
            if (g_stWashWorkState[g_eWashWorking].eWarning < TESTPROC_WARNING_NONE)
            {
                *(uint16_t *)&u8TxBuf[0] = g_stWashWorkState[g_eWashWorking].eWarning;
                u8TxBuf[2] = TESTPROC_UNIT_WASH;
                u8TxBuf[3] = FSM_Wash;
                TestProc_SendMsg(TESTPROC_MSG_CMD_TIMEOUT, TESTPROC_UNIT_EVENT, u8TxBuf, 4);
            }
        }
    }

    if (TESTPROC_RESULT_OK != s32Result)
    {
        DebugMsg("E>WashWork:%d,%08X,%d,%02X,%d,%d,%08X\r\n", s32Result, eActionResult, eCheckResult, u8WashMask, FSM_Wash, FSM_Old, err);
    }
}

int TestProc_MeasureRoomStartWork(MEASUREROOM_WORK_E eWork, uint8_t *pu8Param, uint8_t u8Size)
{
    int result = TESTPROC_RESULT_OK;
    OS_ERR err = OS_ERR_NONE;

    if (MEASUREROOM_STATE_IDLE == FSM_MeasureRoom)
    {
        FSM_MeasureRoom = g_stMeasureRoomWorkState[eWork].eMeasureRoomState;
        g_MeasureRoomTimeStart = OSTimeGet(&err);
        g_eMeasureRoomWorking = eWork;
        //DebugMsg("**MR_Work >> %d,%d,%s\r\n", eWork, FSM_MeasureRoom, g_strMeasureRoomName[eWork]);
    }
    else
    {
        result = TESTPROC_RESULT_ERR_STATUS;
        DebugMsg("E>MR_Wash:%d,%d\r\n", FSM_MeasureRoom, eWork);
    }

    return result;
}

void TestProc_MeasureRoomFSM(OS_TICK tick)
{
    int32_t s32Result = TESTPROC_RESULT_OK;
    TESTPROC_API_RESULT_E eCheckResult;
    LH_ERR eActionResult = LH_ERR_NONE;
    uint8_t u8TxBuf[8];
    MEASUREROOM_STATE_E FSM_Old = FSM_MeasureRoom;
    //uint8_t u8WashMask = TSC_NEEDLE_WASH_SELECT_NONE;

    switch (FSM_MeasureRoom)
    {
        //*******************半开门,准备丢杯子**************************
        case MEASUREROOM_STATE_HALFOPEN:
            eActionResult = TestProc_Api_WashMeasureRoomDoorOperation(TESTPROC_API_DOOR_OP_HALFOPEN);
            if (LH_ERR_NONE == eActionResult)
            {
                FSM_MeasureRoom = MEASUREROOM_STATE_HALFOPEN_ING;
            }
            else
            {
                s32Result = TESTPROC_RESULT_NG;
                FSM_MeasureRoom = MEASUREROOM_STATE_IDLE;
            }
            break;
        case MEASUREROOM_STATE_HALFOPEN_ING:
            eCheckResult = TestProc_Api_WashMeasureRoomCheckResult(&eActionResult);
            if (LH_ERR_NONE == eActionResult) //TESTPROC_API_RESULT_OK == eCheckResult)
            {
                TestProc_SendMsg(TESTPROC_MSG_CMD_WASH_MEASUREROOM_HALFOPEN, TESTPROC_UNIT_WASH, NULL, 0);
                FSM_MeasureRoom = MEASUREROOM_STATE_IDLE;
            }
            else if (TESTPROC_API_RESULT_NG == eCheckResult)
            {
                s32Result = TESTPROC_RESULT_NG;
                FSM_MeasureRoom = MEASUREROOM_STATE_IDLE;
            }
            break;
        //*******************全开门,准备丢杯子**************************
        case MEASUREROOM_STATE_FULLOPEN:
            eActionResult = TestProc_Api_WashMeasureRoomDoorOperation(TESTPROC_API_DOOR_OP_FULLOPEN);
            if (LH_ERR_NONE == eActionResult)
            {
                FSM_MeasureRoom = MEASUREROOM_STATE_FULLOPEN_ING;
            }
            else
            {
                s32Result = TESTPROC_RESULT_NG;
                FSM_MeasureRoom = MEASUREROOM_STATE_IDLE;
            }
            break;
        case MEASUREROOM_STATE_FULLOPEN_ING:
            eCheckResult = TestProc_Api_WashMeasureRoomCheckResult(&eActionResult);
            if (TESTPROC_API_RESULT_OK == eCheckResult)
            {
                TestProc_SendMsg(TESTPROC_MSG_CMD_WASH_MEASUREROOM_FULLOPEN, TESTPROC_UNIT_WASH, NULL, 0);
                FSM_MeasureRoom = MEASUREROOM_STATE_IDLE;
            }
            else if (TESTPROC_API_RESULT_NG == eCheckResult)
            {
                s32Result = TESTPROC_RESULT_NG;
                FSM_MeasureRoom = MEASUREROOM_STATE_IDLE;
            }
            break;
        //**********************关门**************************
        case MEASUREROOM_STATE_CLOSE:
            eActionResult = TestProc_Api_WashMeasureRoomDoorOperation(TESTPROC_API_DOOR_OP_CLOSE);
            if (LH_ERR_NONE == eActionResult)
            {
                FSM_MeasureRoom = MEASUREROOM_STATE_CLOSE_ING;
            }
            else
            {
                s32Result = TESTPROC_RESULT_NG;
                FSM_MeasureRoom = MEASUREROOM_STATE_IDLE;
            }
            break;
        case MEASUREROOM_STATE_CLOSE_ING:
            eCheckResult = TestProc_Api_WashMeasureRoomCheckResult(&eActionResult);
            if (TESTPROC_API_RESULT_OK == eCheckResult)
            {
                TestProc_SendMsg(TESTPROC_MSG_CMD_WASH_MEASUREROOM_CLOSE, TESTPROC_UNIT_WASH, NULL, 0);
                FSM_MeasureRoom = MEASUREROOM_STATE_IDLE;
            }
            else if (TESTPROC_API_RESULT_NG == eCheckResult)
            {
                s32Result = TESTPROC_RESULT_NG;
                FSM_MeasureRoom = MEASUREROOM_STATE_IDLE;
            }
            break;
        //****************测量值*************************
        case MEASUREROOM_STATE_MEASURE:
            eActionResult = TestProc_Api_WashMeasureRoomMakeValue(MEASURE_FLAG_BACK, 500);
            if (LH_ERR_NONE == eActionResult)
            {
                FSM_MeasureRoom = MEASUREROOM_STATE_MEASURE_STEP1;
            }
            else
            {
                s32Result = TESTPROC_RESULT_NG;
                FSM_MeasureRoom = MEASUREROOM_STATE_IDLE;
            }
            break;
        case MEASUREROOM_STATE_MEASURE_STEP1:
            eCheckResult = TestProc_Api_WashMeasureRoomCheckResult(&eActionResult);
            if (TESTPROC_API_RESULT_OK == eCheckResult)
            {
                *(uint32_t *)&u8TxBuf[0] = TestProc_Api_WashMeasureRoomReadValue(MEASURE_FLAG_BACK);
                TestProc_SendMsg(TESTPROC_MSG_CMD_WASH_MEASUREROOM_VALUE1, TESTPROC_UNIT_WASH, u8TxBuf, 4);
                eActionResult = TestProc_Api_WashMeasureRoomMakeValue(MEASURE_FLAG_READ, 4000);
                if (LH_ERR_NONE == eActionResult)
                {
                    FSM_MeasureRoom = MEASUREROOM_STATE_MEASURE_STEP2;
                }
                else
                {
                    s32Result = TESTPROC_RESULT_ERR_EXEC;
                    FSM_MeasureRoom = MEASUREROOM_STATE_IDLE;
                }
            }
            else if (TESTPROC_API_RESULT_NG == eCheckResult)
            {
                s32Result = TESTPROC_RESULT_NG;
                FSM_MeasureRoom = MEASUREROOM_STATE_IDLE;
            }
            break;
        case MEASUREROOM_STATE_MEASURE_STEP2:
            eCheckResult = TestProc_Api_WashMeasureRoomCheckResult(&eActionResult);
            if (TESTPROC_API_RESULT_OK == eCheckResult)
            {
                *(uint32_t *)&u8TxBuf[0] = TestProc_Api_WashMeasureRoomReadValue(MEASURE_FLAG_READ);
                TestProc_SendMsg(TESTPROC_MSG_CMD_WASH_MEASUREROOM_VALUE2, TESTPROC_UNIT_WASH, u8TxBuf, 4);
                FSM_MeasureRoom = MEASUREROOM_STATE_IDLE;
            }
            else if (TESTPROC_API_RESULT_NG == eCheckResult)
            {
                s32Result = TESTPROC_RESULT_NG;
                FSM_MeasureRoom = MEASUREROOM_STATE_IDLE;
            }
            break;
        case MEASUREROOM_STATE_IDLE:
        default:

            break;
    }
    
    if (MEASUREROOM_STATE_IDLE == FSM_MeasureRoom)
    {
        g_MeasureRoomTimeStart = TESTPROC_TICK_INVALID;
        if (s32Result == TESTPROC_RESULT_NG && eActionResult != LH_ERR_NONE && g_eMeasureRoomActionWarning[FSM_Old] != TESTPROC_WARNING_NONE)
        {
            DebugMsg("E>MeasureRoomWorkNG:%d,%d,%08X\r\n", g_eMeasureRoomWorking, FSM_Old, eActionResult);
            *(uint16_t *)u8TxBuf[0] = g_eMeasureRoomActionWarning[FSM_Old];
            u8TxBuf[2] = TESTPROC_UNIT_WASH;
            u8TxBuf[3] = FSM_Old;
            u8TxBuf[4] = WORD_TO_BYTE_LL(eActionResult);
            u8TxBuf[5] = WORD_TO_BYTE_LL(eActionResult);
            u8TxBuf[6] = WORD_TO_BYTE_LL(eActionResult);
            u8TxBuf[7] = WORD_TO_BYTE_LL(eActionResult);
            TestProc_SendMsg(TESTPROC_MSG_CMD_EXEC_NG, TESTPROC_UNIT_WASH, u8TxBuf, 8);
        }
    }
    else
    {
        if (tick > g_MeasureRoomTimeStart && (tick-g_MeasureRoomTimeStart) > g_stMeasureRoomWorkState[g_eMeasureRoomWorking].TimeOut)
        {
            DebugMsg("E>MeasureRoomWorkTimeout:%d,%d,%d,%d,%d,%d\r\n", g_eMeasureRoomWorking, FSM_MeasureRoom, FSM_Old, g_stMeasureRoomWorkState[g_eMeasureRoomWorking].TimeOut, tick, g_MeasureRoomTimeStart);
            g_MeasureRoomTimeStart = TESTPROC_TICK_INVALID;
            if (g_stMeasureRoomWorkState[g_eMeasureRoomWorking].eWarning < TESTPROC_WARNING_NONE)
            {
                *(uint16_t *)&u8TxBuf[0] = g_stMeasureRoomWorkState[g_eMeasureRoomWorking].eWarning;
                u8TxBuf[2] = TESTPROC_UNIT_WASH;
                u8TxBuf[3] = FSM_MeasureRoom;
                TestProc_SendMsg(TESTPROC_MSG_CMD_TIMEOUT, TESTPROC_UNIT_EVENT, u8TxBuf, 4);
            }
        }
    }

    if (TESTPROC_RESULT_OK != s32Result)
    {
        DebugMsg("E>MeasureRoom:%d,%08X,%d,%d,%d\r\n", s32Result, eActionResult, eCheckResult, FSM_MeasureRoom, FSM_Old);
    }
}
