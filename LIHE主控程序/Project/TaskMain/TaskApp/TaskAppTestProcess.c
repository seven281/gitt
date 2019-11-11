#include "MCU_internalFlash.h"
#include "SystemParam.h"
#include "SystemConfig.h"
#include "TaskAppTestProcess.h"
#include "TestProc_API.h"
#include "TestProc_CanConfig.h"
#include "TestProc_TestData.h"
#include "TestProc_CupStack.h"
#include "TestProc_HandCup.h"
#include "TestProc_React.h"
#include "TestProc_Reagent.h"
#include "TestProc_ReagentDisk.h"
#include "TestProc_Sample.h"
#include "TestProc_Warning.h"
#include "TestProc_Wash.h"
#include "TestProc_boardID.h"
#include "TestProc_canCommand.h"
#include "SystemMsg.h"

#pragma pack(push)
#pragma pack(4)
//任务堆栈
CPU_STK stackBufferTaskAppTestProcess[STK_SIZE_TASK_APP_TEST_PROCESS];
#pragma pack(pop)

OS_TCB tcbTaskAppTestProcess;

#define TASK_TEST_PROCESS_SPEED                     (10)    // ms
#define RESET_BIT_SAMPLENEEDLE                      (0x01)
#define RESET_BIT_REAGENT                           (0x02)
#define RESET_BIT_HANDCUP                           (0x04)
#define RESET_BIT_ALL                               (0x07)
#define RESET_ENABLE_REACTMASK                      (RESET_BIT_SAMPLENEEDLE | RESET_BIT_REAGENT | RESET_BIT_HANDCUP)
#define RESET_ENABLE_TRACKMASK                      (RESET_BIT_SAMPLENEEDLE)
#define RESET_ENABLE_WASHDISKMASK                   (RESET_BIT_HANDCUP)
#define RESET_ENABLE_REAGENTDISKMASK                (RESET_BIT_REAGENT)
#define TIMEOUT_RESET_FOR_SYSTEM                    (60000u)           // ms
#define TIMEOUT_POWERON_FOR_SYSTEM                  (1 * 60 * 1000u) // ms
#define TESTPROC_GET_ITEM_LIMIT                     (100u)              // < 10个测试项时向中位机索要测试数据
#define TESTPROC_INVALID_RACK_ID                    (0u)
#define TESTPROC_RM1_MIX_TIME                       (200u)
#define TESTPROC_RM2_MIX_TIME                       (200u)
#define TESTPROC_COUNT_TESTTOSTANDBY                (85)
#define TestProc_ReortStatus(pstStatus) TestProc_SendDataToComm(BOARD_ID_TRACK_COMM, CAN_CMD_CELL_STATUS, (uint8_t *)&((pstStatus)->stCellStatus), sizeof(TESTPROC_CELLSTATUS))

#define TestProc_CheckResetNG()                                                                                  \
    do                                                                                                           \
    {                                                                                                            \
        for (int i = 0; i < TESTPROC_UNIT_MAX; i++)                                                              \
            {                                                                                                        \
                if (TESTPROC_RESET_NG == STATUS_system.eResetState[i])                                               \
                    {                                                                                                    \
                        DebugMsg("Reset NG:%d\r\n", i);                                                                  \
                            TestProc_ReportWarning(&STATUS_system,                                                           \
                                (TESTPROC_WARNING_E)(TESTPROC_WARNING_RESET_NG_REACT + i), NULL, 0);      \
                    }                                                                                                    \
                else if (TESTPROC_RESET_ONGOING == STATUS_system.eResetState[i])                                     \
                    {                                                                                                    \
                        DebugMsg("Reset Timeout:%d\r\n", i);                                                             \
                            TestProc_ReportWarning(&STATUS_system,                                                           \
                                (TESTPROC_WARNING_E)(TESTPROC_WARNING_RESET_TIMEOUT_REACT + i), NULL, 0); \
                    }                                                                                                    \
            }                                                                                                        \
    } while (0)
                
#define TestProc_H2CupID(step)              __H2CupID(STATUS_system.u8H2CupNumber, (step))
#define TestProc_SampleCupID(id)            __SampleCupID((id))
#define TestProc_R123MCupID(id)             __R123MCupID((id))
#define TestProc_RM1CupID()                 __RM1CupID(STATUS_system.u8H2CupNumber)
#define TestProc_RM2CupID()                 __RM2CupID(STATUS_system.u8H2CupNumber)
#define TestProc_XCupID()                   __XCupID(STATUS_system.u8H2CupNumber)
#define TestProc_WashDiskHandCup()          __WashDiskHandCup(STATUS_system.u8WashDiskHandCup)
#define TestProc_Wash12Cup()                __Wash12Cup(STATUS_system.u8WashDiskHandCup)
#define TestProc_Wash34Cup()                __Wash34Cup(STATUS_system.u8WashDiskHandCup)
#define TestProc_Wash56Cup()                __Wash56Cup(STATUS_system.u8WashDiskHandCup)
#define TestProc_WashLiquidACup()           __WashLiquidACup(STATUS_system.u8WashDiskHandCup)
#define TestProc_WashMixCup()               __WashMixCup(STATUS_system.u8WashDiskHandCup)
#define TestProc_Step4R1ByStep2CupID(id)    __Step4R1ByStep2CupID(id)
#define TestProc_Step3R2ByStep1CupID(id)    __Step3R2ByStep1CupID(id)
#define TestProc_Step3R2ByStep2CupID(id)    __Step3R2ByStep2CupID(id)
#define TestProc_Step3R2ByStep4CupID(id)    __Step3R2ByStep4CupID(id)
#define TestProc_Step4R2ByStep3CupID(id)    __Step4R2ByStep3CupID(id)

typedef enum
{
    FSM_TESTPROCESS_IDLE,        // Idle
    FSM_TESTPROCESS_POWERINIT,   // Init For Power On
    FSM_TESTPROCESS_RESET,       // Reseting
    FSM_TESTPROCESS_TEST,        // Testing
    FSM_TESTPROCESS_SIMULATE,    // Machine check
    FSM_TESTPROCESS_ABANDON_CUP, //
    FSM_TESTPROCESS_RECYCLE_RACK,
    FSM_TESTPROCESS_REAGENTBARCODE,
    FSM_TESTPROCESS_REAGENTQUANTITY,
    FSM_TESTPROCESS_M_SEPARATE_AUTO,
    FSM_TESTPROCESS_M_SEPARATE,
    FSM_TESTPROCESS_MANUAL_TEST,
    FSM_TESTPROCESS_SAMPLE_ACCURACY,
    FSM_TESTPROCESS_REAGENT_ACCURACY,
    FSM_TESTPROCESS_WASH_ACCURACY,
    FSM_TESTPROCESS_DARK_TEST,
    FSM_TESTPROCESS_NEEDLE_CLEAN,
    FSM_TESTPROCESS_LIQUID_PERFUSE,
    FSM_TESTPROCESS_MAX
} FSM_TESTPROCESS_E;

typedef enum
{
    FSM_ABANDONCUP_IDLE,              // 0  Idle
    FSM_ABANDONCUP_HANDCUP_CLEAN,     // 1
    FSM_ABANDONCUP_MR_OPEN,           // 2
    FSM_ABANDONCUP_MR_OPEN_ING,       // 3
    FSM_ABANDONCUP_MR_COME_ING,       // 4
    FSM_ABANDONCUP_MR_CATCH_ING,      // 5
    FSM_ABANDONCUP_MR_ANABDON_ING,    // 6
    FSM_ABANDONCUP_MR_CLOSE_ING,      // 7  测量室抛杯完成,关门
    FSM_ABANDONCUP_WASH_CHECH,        // 8
    FSM_ABANDONCUP_WASH_COME_ING,     // 9
    FSM_ABANDONCUP_WASH_CATCH_ING,    // 10
    FSM_ABANDONCUP_WASH_ANABDON_ING,  // 11
    FSM_ABANDONCUP_WASH_ROTATE,       // 12
    FSM_ABANDONCUP_WASH_ROTATE_ING,   // 13
    FSM_ABANDONCUP_REACT_CHECH,       // 14
    FSM_ABANDONCUP_REACT_COME_ING,    // 15
    FSM_ABANDONCUP_REACT_CATCH_ING,   // 16
    FSM_ABANDONCUP_REACT_ANABDON_ING, // 17
    FSM_ABANDONCUP_REACT_ROTATE,      // 18
    FSM_ABANDONCUP_REACT_ROTATE_ING,  // 19
    FSM_ABANDONCUP_HANDCUP_RESET_ING, // 20
    FSM_ABANDONCUP_DISC_RESET_ING,    // 21
    FSM_ABANDONCUP_FINISH,            // 22
    FSM_ABANDONCUP_MAX
} FSM_ABANDONCUP_E;

typedef enum
{
    FSM_RECYCLERACK_IDLE,              // 0  Idle
    FSM_RECYCLERACK_TEST,              // 1
    FSM_RECYCLERACK_TEST_MOVE_ING,     // 2
    FSM_RECYCLERACK_MAX
} FSM_RECYCLERACK_E;
    
typedef enum
{
    FSM_M_SEPARATE_AUTO_IDLE,               // 0  Idle
    FSM_M_SEPARATE_AUTO_START,              // 1
    FSM_M_SEPARATE_AUTO_WASH_RESET_1,       // 2
    FSM_M_SEPARATE_AUTO_WASH_RESET_2,       // 3
    FSM_M_SEPARATE_AUTO_WASH_RESET_ING,     // 4
    FSM_M_SEPARATE_AUTO_START_STOP_WORK,    // 5
    FSM_M_SEPARATE_AUTO_TO_WASH_FOR_MR,     // 6
    FSM_M_SEPARATE_AUTO_CATCH_WASH_FOR_MR,  // 7
    FSM_M_SEPARATE_AUTO_CUP_TO_MR,          // 8
    FSM_M_SEPARATE_AUTO_PUT_TO_MR,          // 9
    FSM_M_SEPARATE_AUTO_MR_CLOSR,           // 10
    FSM_M_SEPARATE_AUTO_MR_MAKE1,           // 11
    FSM_M_SEPARATE_AUTO_MR_MAKE2,           // 12 
    FSM_M_SEPARATE_AUTO_MR_OPEN,            // 13 
    FSM_M_SEPARATE_AUTO_TO_MR_FOR_GARBAGE,  // 14
    FSM_M_SEPARATE_AUTO_CATCH_MR_CUP,       // 15 
    FSM_M_SEPARATE_AUTO_TO_GARBAGE,         // 16
    FSM_M_SEPARATE_AUTO_PUT_TO_GARBAGE,     // 17
    FSM_M_SEPARATE_AUTO_TO_CUPSTACK,        // 18
    FSM_M_SEPARATE_AUTO_CATCH_NEW_CUP,      // 19
    FSM_M_SEPARATE_AUTO_TO_WASH,            // 20
    FSM_M_SEPARATE_AUTO_PUT_WASH,           // 21
    FSM_M_SEPARATE_AUTO_PUT_WASH_ING,       // 22
    FSM_M_SEPARATE_AUTO_WASH_STOP,          // 23
    FSM_M_SEPARATE_AUTO_ROTATE_ING,         // 24
    FSM_M_SEPARATE_AUTO_MAX
} FSM_M_SEPARATE_AUTO_E;
    
typedef enum
{
    FSM_M_SEPARATE_IDLE,               // 0  Idle
    FSM_M_SEPARATE_START,              // 1
    FSM_M_SEPARATE_WASH_RESET_1,       // 2
    FSM_M_SEPARATE_WASH_RESET_2,       // 3
    FSM_M_SEPARATE_WASH_RESET_ING,     // 4
    FSM_M_SEPARATE_MEASUREROOM_OPEN,   // 5
    FSM_M_SEPARATE_ROTATE,             // 6
    FSM_M_SEPARATE_ROTATE_ING,         // 7
    FSM_M_SEPARATE_WASH,               // 8
    FSM_M_SEPARATE_WASH_ING,           // 9
    FSM_M_SEPARATE_WAIT_TEST,          // 10
    FSM_M_SEPARATE_MAX
} FSM_M_SEPARATE_E;

typedef enum
{
    FSM_MANUAL_TEST_IDLE,               // 0
    FSM_MANUAL_TEST_PREPARE,            // 1
    FSM_MANUAL_TEST_RESET_1,            // 2
    FSM_MANUAL_TEST_RESET_2,            // 3
    FSM_MANUAL_TEST_MEASUREROOM_INIT,   // 4
    FSM_MANUAL_TEST_START_TEST,         // 5
    FSM_MANUAL_TEST_MEASUREROOM_CLOSE,  // 6
    FSM_MANUAL_TEST_MEASUREROOM_TEST,   // 7
    FSM_MANUAL_TEST_MEASUREROOM_MAKE,   // 8
    FSM_MANUAL_TEST_MEASUREROOM_MAKE2,  // 9
    FSM_MANUAL_TEST_MEASUREROOM_END,    // 10
    FSM_MANUAL_TEST_MAX
}FSM_MANUAL_TEST_E;
    
typedef enum
{
    FSM_SAMPLE_ACCURACY_IDLE,               // 0
    FSM_SAMPLE_ACCURACY_START,              // 1
    FSM_SAMPLE_ACCURACY_RESET_1,            // 2
    FSM_SAMPLE_ACCURACY_RESET_2,            // 3
    FSM_SAMPLE_ACCURACY_MOVE_RACK,          // 4
    FSM_SAMPLE_ACCURACY_SAMPLE_ABSORB,      // 5
    FSM_SAMPLE_ACCURACY_TO_REACT,           // 6
    FSM_SAMPLE_ACCURACY_SAMPLE_INJECT,      // 7
    FSM_SAMPLE_ACCURACY_TO_CLEAN,           // 8
    FSM_SAMPLE_ACCURACY_SAMPLE_CLEAN,       // 9
    FSM_SAMPLE_ACCURACY_MAX
}FSM_SAMPLE_ACCURACY_E;
    
typedef enum
{
    FSM_REAGENT_ACCURACY_IDLE,               // 0
    FSM_REAGENT_ACCURACY_START,              // 1
    FSM_REAGENT_ACCURACY_RESET_1,            // 2
    FSM_REAGENT_ACCURACY_RESET_2,            // 3
    FSM_REAGENT_ACCURACY_TO_DISK,            // 4
    FSM_REAGENT_ACCURACY_ABSORB,             // 5
    FSM_REAGENT_ACCURACY_TO_REACT,           // 6
    FSM_REAGENT_ACCURACY_INJECT,             // 7
    FSM_REAGENT_ACCURACY_TO_CLEAN,           // 8
    FSM_REAGENT_ACCURACY_CLEAN_ING,          // 9
    FSM_REAGENT_ACCURACY_MAX
}FSM_REAGENT_ACCURACY_E;

typedef enum
{
    FSM_WASH_ACCURACY_IDLE,                  // 0
    FSM_WASH_ACCURACY_START,                 // 1
    FSM_WASH_ACCURACY_RESET_1,               // 2
    FSM_WASH_ACCURACY_ROTATE_ING,            // 3
    FSM_WASH_ACCURACY_CLEAN_ING,             // 4
    FSM_WASH_ACCURACY_FINISH_ING,            // 5
    FSM_WASH_ACCURACY_MAX
}FSM_WASH_ACCURACY_E;

typedef enum
{
    FSM_NEEDLE_CLEAN_IDLE,                  // 0
    FSM_NEEDLE_CLEAN_START,                 // 1
    FSM_NEEDLE_CLEAN_RESET_1,               // 2
    FSM_NEEDLE_CLEAN_MOVE_ING,              // 3
    FSM_NEEDLE_CLEAN_CLEAN_ING,             // 4
    FSM_NEEDLE_CLEAN_MAX
}FSM_NEEDLE_CLEAN_E;

typedef enum
{
    FSM_DARK_TEST_IDLE,                     // 0
    FSM_DARK_TEST_START,                    // 1
    FSM_DARK_TEST_RESET_1,                  // 2
    FSM_DARK_TEST_MAKE_VALUE,               // 3
    FSM_DARK_TEST_MAX
}FSM_DARK_TEST_E;
    
typedef enum
{
    FSM_LIQUID_PERFUSE_IDLE,                  // 0
    FSM_LIQUID_PERFUSE_START,                 // 1
    FSM_LIQUID_PERFUSE_RESET_1,               // 2
    FSM_LIQUID_PERFUSE_PRIME_1,               // 3
    FSM_LIQUID_PERFUSE_PRIME_2,               // 4
    FSM_LIQUID_PERFUSE_PRIME_3,               // 5
    FSM_LIQUID_PERFUSE_MAX
}FSM_LIQUID_PERFUSE_E;

typedef enum
{
    FSM_REAGENTBARCODE_IDLE,           // 0  Idle
    FSM_REAGENTBARCODE_START,
    FSM_REAGENTBARCODE_MOVE,
    FSM_REAGENTBARCODE_MOVE_ING,
    FSM_REAGENTBARCODE_SCAN_ING,
    FSM_REAGENTBARCODE_NEXT_POS,
    FSM_REAGENTBARCODE_MAX
} FSM_REAGENTBARCODE_E;

typedef enum
{
    FSM_REAGENTQUANTITY_IDLE,          // 0  Idle
    FSM_REAGENTQUANTITY_START,
    FSM_REAGENTQUANTITY_R1_MOVE,
    FSM_REAGENTQUANTITY_R1_MOVE_ING,
    FSM_REAGENTQUANTITY_R1_DETECT_ING,
    FSM_REAGENTQUANTITY_R2_MOVE_ING,
    FSM_REAGENTQUANTITY_R2_DETECT_ING,
    FSM_REAGENTQUANTITY_M_MOVE_ING,
    FSM_REAGENTQUANTITY_M_DETECT_ING,
    FSM_REAGENTQUANTITY_NEXT_POS,
    FSM_REAGENTQUANTITY_MAX
} FSM_REAGENTQUANTITY_E;

typedef enum
{
    FSM_POWERON_IDLE,                           // 0  Idle
    FSM_POWERON_RESET,                          // 1
    FSM_POWERON_RESET_ING,                      // 2
    FSM_POWERON_PRIME_SAMPLE_MOVE,              // 3
    FSM_POWERON_SAMPLE_TO_FORCE_PRIME_ING,      // 4
    FSM_POWERON_SAMPLE_FORCE_PRIME_ING,         // 5
    FSM_POWERON_SAMPLE_TO_PRIME_ING,            // 6
    FSM_POWERON_PRIME_ING,                      // 7
    FSM_POWERON_PREHEATING,                     // 8
    FSM_POWERON_STANDBY,                        // 9
    FSM_POWERON_MAX
} FSM_POWERON_E;
    
typedef enum
{
    FSM_REAGENTEVENT_IDLE,                           // 0  Idle
    FSM_REAGENTEVENT_MOVE,                           // 1
    FSM_REAGENTEVENT_MOVE_ING,                       // 2
    FSM_REAGENTEVENT_MAX
} FSM_REAGENTEVENT_E;
    
typedef enum
{
    FSM_CUPSTACKEVENT_IDLE,                           // 0  Idle
    FSM_CUPSTACKEVENT_DEMAGNETIZE,                    // 1
    FSM_CUPSTACKEVENT_DEMAGNETIZE_ING,                // 2
    FSM_CUPSTACKEVENT_RESET,                          // 3
    FSM_CUPSTACKEVENT_RESET_ING,                      // 4
    FSM_CUPSTACKEVENT_WAIT_CATCH_ING,                 // 5 确认抓新杯动作已完成
    FSM_CUPSTACKEVENT_PUSH_ING,                       // 6
    FSM_CUPSTACKEVENT_MAX
} FSM_CUPSTACKEVENT_E;

// 需要应答的命令
typedef enum
{
    TESTPROC_RSP_CMD_CAN_GETITEM = 0,
    TESTPROC_RSP_CMD_MAX
} TESTPROC_RSP_CMD;
static OS_TICK g_RspCmdSendTime[TESTPROC_RSP_CMD_MAX];
static const OS_TICK g_RspCmdHoldTime[TESTPROC_RSP_CMD_MAX] =
{
    5000, //ms
};

typedef int32_t (*UNIT_MSG_FUNC)(TESTPROC_STATUS_T *pstStatus, const TESTPROC_MSM_T *pstMsg);

static FSM_TESTPROCESS_E FSM_TestProcess;
static FSM_ABANDONCUP_E FSM_AbandonCup;
static FSM_RECYCLERACK_E FSM_RecycleRack;
static FSM_REAGENTBARCODE_E FSM_ReagentBarCode;
static FSM_REAGENTQUANTITY_E FSM_ReagentQuantity;
static FSM_POWERON_E FSM_PowerOn;
static TESTPROC_STATUS_T STATUS_system; // Test Process System Status
static OS_TICK g_SystemResetTimer;
static OS_TICK g_SystemPowerOnTimer;
static OS_TMR g_stStatusReportTimer;
static uint8_t g_u8TestProc_ResetMask;
static uint8_t g_u8ReagentDiskReset_OneShot;
static uint8_t g_u8WashDiskReset_OneShot;
static uint8_t g_u8TrackReset_OneShot;
static uint8_t g_u8ReactReset_OneShot;
static TESTPROC_MULTY_REAGENT_T g_stReagentMulty;
//static uint8_t g_u8DebugParam[16] = {0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0};
static uint8_t g_u8ReagentScanPos[TESTPROC_REAGENT_POS_MAX];
static uint16_t g_u16ReagentLiquitLevel[TESTPROC_REAGENT_POS_MAX * 3];
static FSM_M_SEPARATE_AUTO_E FSM_M_SeparateAuto;
static FSM_M_SEPARATE_E FSM_M_Separate;
static FSM_MANUAL_TEST_E FSM_ManualTest;
static uint16_t g_u16MSeparationQuantity = 0;
static uint32_t g_u32MSeparationTestID = 0;
static uint32_t g_u32MSeparationMixTime = 0;
static FSM_SAMPLE_ACCURACY_E FSM_SampleAccuracy;
static FSM_REAGENT_ACCURACY_E FSM_ReagentAccuracy;
static FSM_WASH_ACCURACY_E FSM_WashAccuracy;
static uint16_t g_u16AccuracyTestQuantity = 0;
static uint8_t  g_u8WashAccuracyMask = 0;
static FSM_NEEDLE_CLEAN_E FSM_NeedleClean;
//针清洗功能 => Bit0:1试剂针外壁清洗,0试剂针外壁不清洗；Bit1:1试剂针内壁清洗,0试剂针内壁不清洗；Bit2:1样本针外壁清洗,0样本针外壁不清洗；Bit3:1样本针内壁清洗,0样本针内壁不清洗
//暗计数灌注 => Bit0:样本针内壁灌注；Bit1:样本针外壁灌注；Bit2:试剂针内壁灌注；Bit3:试剂针外壁灌注；Bit4:清洗针灌注；Bit5:A液灌注；Bit6:B液灌注；Bit7:样本针强灌注。1执行，0不执行
static uint32_t g_u32MaintainWorkFlag = 0;
static FSM_DARK_TEST_E FSM_DarkTest;
static FSM_LIQUID_PERFUSE_E FSM_LiquidPerfuse;
static FSM_REAGENTEVENT_E FSM_ReagentEvent;
static FSM_CUPSTACKEVENT_E FSM_CupStackEvent;
static uint8_t g_u8ReagentDiskShakeEnable = 1;
static TESTPROC_ITEM_T g_stSimulateItem =
{
    .stItem.u32TestID = 70000,
    .stItem.u32RackID = 9999,
    .stItem.u16SampleQuantity = 100,
    .stItem.u16SampleDiluteQuantity = 0,
    .stItem.u16DiluteQuantity = 0,
    .stItem.u16ReagentItemID = 1,
    .stItem.u16R1ID = 0,
    .stItem.u16R2ID = 0,
    .stItem.u16R3ID = 0,
    .stItem.u16MID = 0,
    .stItem.u16R1Quantity = 100,
    .stItem.u16R2Quantity = 100,
    .stItem.u16R3Quantity = 100,
    .stItem.u16MQuantity = 100,
    .stItem.u16DiluentID = 0,
    .stItem.u16CleanID = 0,
    .stItem.u8SamplePos = 0,
    .stItem.u8SampleType = 0,
    .stItem.u8TestType = 3,
    .stItem.u8TestCell = 0,
    .stItem.u8SampleBlank = 0,
    .stItem.u8ForeceWash = 0,
    .stItem.u8ItemType = 0,
    .stItem.u8AddStep = TESTPROC_ADDSTEP_TYPE3,
};
static OS_TMR g_stTestTimer;
static OS_TMR g_stWashDiskRotateTimer;

static int32_t TestProc_CanCmdProcess(TESTPROC_STATUS_T *pstStatus, const TESTPROC_CAN_CMD_T *pstCmd);
static int32_t TestProc_MsgReact(TESTPROC_STATUS_T *pstStatus, const TESTPROC_MSM_T *pstMsg);
static int32_t TestProc_MsgWashDisk(TESTPROC_STATUS_T *pstStatus, const TESTPROC_MSM_T *pstMsg);
static int32_t TestProc_MsgReagentDisk(TESTPROC_STATUS_T *pstStatus, const TESTPROC_MSM_T *pstMsg);
static int32_t TestProc_MsgReagent(TESTPROC_STATUS_T *pstStatus, const TESTPROC_MSM_T *pstMsg);
static int32_t TestProc_MsgSample(TESTPROC_STATUS_T *pstStatus, const TESTPROC_MSM_T *pstMsg);
static int32_t TestProc_MsgHandCup(TESTPROC_STATUS_T *pstStatus, const TESTPROC_MSM_T *pstMsg);
static int32_t TestProc_MsgCupStack(TESTPROC_STATUS_T *pstStatus, const TESTPROC_MSM_T *pstMsg);
static int32_t TestProc_MsgEvent(TESTPROC_STATUS_T *pstStatus, const TESTPROC_MSM_T *pstMsg);
static void TestProc_MainFSM(TESTPROC_STATUS_T *pstStatus);
static int32_t TestProc_TestFlow(TESTPROC_STATUS_T *pstStatus);
static void *TestProc_GetOneItem(TESTPROC_STATUS_T *pstState);
static void TestProc_StatusReport(void *p_tmr, void *p_arg);
static int32_t TestProc_GetReagentPos(TESTPROC_STATUS_T *pstStatus, uint8_t u8CupID, uint8_t u8Offset, uint8_t * pu8OutBuf);
static int32_t TestProc_ReagentDiskShake(TESTPROC_STATUS_T *pstStatus);
static int32_t TestProc_MoveReagentDiskPos(TESTPROC_STATUS_T *pstStatus, uint8_t u8CupID, REAGENTDISK_OFFSET_E eOffset);
static int32_t TestProc_Reset(TESTPROC_STATUS_T *pstStatus);
static TESTPROC_RESET_STATE TestProc_ResetCheckResult(TESTPROC_STATUS_T *pstStatus);
static int32_t TestProc_Standby(TESTPROC_STATUS_T *pstStatus);
static void TestProc_AddReagentStep1(TESTPROC_STATUS_T *pstStatus, uint8_t u8CupID);
static void TestProc_AddReagentStep2(TESTPROC_STATUS_T *pstStatus, uint8_t u8CupID);
static int32_t TestProc_MoveReactCupToWash(TESTPROC_STATUS_T *pstStatus, uint8_t u8CupID);
static int32_t TestProc_MoveWashCupToMeasureRoom(TESTPROC_STATUS_T *pstStatus);
static int32_t TestProc_MoveMeasureRoomCup(TESTPROC_STATUS_T *pstStatus);
static int32_t TestProc_AbandonCupFSM(TESTPROC_STATUS_T *pstStatus);
static int32_t TestProc_RecycleRackFSM(TESTPROC_STATUS_T *pstStatus);
static int32_t TestProc_ReagentBarCodeFSM(TESTPROC_STATUS_T *pstStatus);
static int32_t TestProc_ReagentQuantityFSM(TESTPROC_STATUS_T *pstStatus);
static int32_t TestProc_MSeparateAutoFSM(TESTPROC_STATUS_T *pstStatus);
static int32_t TestProc_MSeparateFSM(TESTPROC_STATUS_T *pstStatus);
static int32_t TestProc_ManualTestFSM(TESTPROC_STATUS_T *pstStatus);
static int32_t TestProc_PowerOnFSM(TESTPROC_STATUS_T *pstStatus);
static int32_t TestProc_SampleAccuracyFSM(TESTPROC_STATUS_T *pstStatus);
static int32_t TestProc_ReagentAccuracyFSM(TESTPROC_STATUS_T *pstStatus);
static int32_t TestProc_WashAccuracyFSM(TESTPROC_STATUS_T *pstStatus);
static int32_t TestProc_DarkTestFSM(TESTPROC_STATUS_T *pstStatus);
static int32_t TestProc_NeedleCleanFSM(TESTPROC_STATUS_T *pstStatus);
static int32_t TestProc_LiquidPerfuseFSM(TESTPROC_STATUS_T *pstStatus);
static int32_t TestProc_ReagentEventFSM(TESTPROC_STATUS_T *pstStatus);
static int32_t TestProc_CupStackEventFSM(TESTPROC_STATUS_T *pstStatus);
static int32_t TestProc_ReportWarning(TESTPROC_STATUS_T *pstStatus, TESTPROC_WARNING_E eWarning, uint8_t *pu8Param, uint16_t u16Size);
static int32_t TestProc_UartCmdProcess(TESTPROC_STATUS_T *pstStatus, const SYSTEMCONFIG_CMD_T *pu8Data);
static void TestProc_TestTimer(void *p_tmr, void *p_arg);
static void TestProc_WashDiskRotateTimer(void *p_tmr, void *p_arg);

static const UNIT_MSG_FUNC g_UnitMsgFunc[TESTPROC_UNIT_MAX] =
{
    TestProc_MsgReact,
    TestProc_MsgWashDisk,
    TestProc_MsgReagentDisk,
    TestProc_MsgReagent,
    TestProc_MsgSample,
    TestProc_MsgHandCup,
    TestProc_MsgCupStack,
    TestProc_MsgEvent
};


/****************************************************************************
*
* Function Name:    TestProc_GetRackItem
* Input:            pstState - 模块的状态指针
*                   
* Output:           None
*                   
* Returns:          None
* Description:      尝试获取测试项
* Note:             
***************************************************************************/
__INLINE static void TestProc_GetRackItem(TESTPROC_STATUS_T *pstStatus, uint16_t u16RackID)
{
    uint8_t u8TxBuf[2];
    OS_ERR os_err;
    
    u8TxBuf[0] = HWORD_TO_BYTE_L(u16RackID);
    u8TxBuf[1] = HWORD_TO_BYTE_H(u16RackID);
    TestProc_SendDataToComm(BOARD_ID_TRACK_COMM, CAN_CMD_COMM_GET_ITEM, u8TxBuf, 2);
    g_RspCmdSendTime[TESTPROC_RSP_CMD_CAN_GETITEM] = OSTimeGet(&os_err);
}

/****************************************************************************
*
* Function Name:    TestProc_IsEmRack
* Input:            u16RackID - 样本架ID
*                   
* Output:           None
*                   
* Returns:          1 - 急诊样本架; 0 - 常规样本架
* Description:      初始化样本架参数
* Note:             
***************************************************************************/
__INLINE static uint8_t TestProc_IsEmRack(uint16_t u16RackID)
{
    uint8_t type = TestProc_GetType(u16RackID);
    if (TESTPROC_RACK_TYPE_NORMAL == type)
    {
        return 0;
    }
    else
    {
        return 1;
    }
}

/****************************************************************************
*
* Function Name:    TestProc_RackMoveCup
* Input:            u8CupID - 反应杯号
*                   
* Output:           None
*                   
* Returns:          
* Description:      
* Note:             
***************************************************************************/
__INLINE static uint8_t TestProc_RackMoveCup(uint8_t u8CupID)
{
    int cupID = u8CupID - 2;
    if (cupID > 0)
    {
        return (uint8_t)cupID;
    }
    else
    {
        return (uint8_t)(cupID + TESTPROC_CUP_QUANTITY);
    }
}

/****************************************************************************
*
* Function Name:    app_testRackToOutPos
* Input:            None
*                   
* Output:           None
*                   
* Returns:          None
* Description:      测试区样本架移送至回收位置
* Note:             
***************************************************************************/
__INLINE static void TestProc_RackToOutPos(TESTPROC_STATUS_T *pstStatus)
{
    //OS_ERR  os_err;
    if (pstStatus->u32EmRackID != TESTPROC_INVALID_RACK_ID &&
        pstStatus->eEmRackState == TESTPROC_RACK_STATE_TEST &&
            TestProc_testDataEmItemCount() == 0)
    {
        pstStatus->eEmRackState = TESTPROC_RACK_STATE_TOOUT;
        pstStatus->eRecycleStatus = TESTPROC_RECYCLE_STATUS_BUSY;
        pstStatus->u8InterferenceSample |= TESTPROC_INTERFERENCE_SAMPLE_CB;
        pstStatus->eConveyorBeltState = TESTPROC_SAMPLE_CB_STATE_MOVING;
        TestProc_ConveyorBeltStartWork(CONVEYORBELT_WORK_TO_RECYCLE, NULL, 0);
    }
    else if (pstStatus->u32RackID != TESTPROC_INVALID_RACK_ID &&
             pstStatus->eRackState == TESTPROC_RACK_STATE_TEST &&
                 TestProc_testDataCount() == 0)
    {
        pstStatus->eRackState = TESTPROC_RACK_STATE_TOOUT;
        pstStatus->eRecycleStatus = TESTPROC_RECYCLE_STATUS_BUSY;
        pstStatus->u8InterferenceSample |= TESTPROC_INTERFERENCE_SAMPLE_CB;
        pstStatus->eConveyorBeltState = TESTPROC_SAMPLE_CB_STATE_MOVING;
        TestProc_ConveyorBeltStartWork(CONVEYORBELT_WORK_TO_RECYCLE, NULL, 0);
    }
}

static void TestProc_ResetInit(TESTPROC_STATUS_T *pstStatus)
{
    int i;
    OS_ERR err;
    
    g_SystemResetTimer = TESTPROC_TICK_INVALID;
    FSM_TestProcess = FSM_TESTPROCESS_IDLE;
    g_u8ReagentDiskReset_OneShot = 0;
    g_u8WashDiskReset_OneShot = 0;
    g_u8TrackReset_OneShot = 0;
    g_u8ReactReset_OneShot = 0;
    g_u16MSeparationQuantity = 0;
    g_u32MSeparationTestID = 0;
    g_u32MSeparationMixTime = 0;
    FSM_ReagentEvent = FSM_REAGENTEVENT_IDLE;
    FSM_CupStackEvent = FSM_CUPSTACKEVENT_IDLE;
    
    pstStatus->eRackState = TESTPROC_RACK_STATE_IDLE;
    pstStatus->eEmRackState = TESTPROC_RACK_STATE_IDLE;
    pstStatus->eRecycleStatus = TESTPROC_RECYCLE_TSC_CMD_STATE_IDLE;
    pstStatus->eHandCupBusy = TESTPROC_HANDCUP_STATE_IDLE;
    pstStatus->eMeasureRoomState = TESTPROC_MEASUREROOM_STATE_IDLE;
    pstStatus->eReactState = TESTPROC_REACT_STATE_IDLE;
    pstStatus->eReagentState = TESTPROC_REAGENT_STATE_IDLE;
    pstStatus->eReagentDiskState = TESTPROC_REAGENTISK_STATE_IDLE;
    pstStatus->eSampleState = TESTPROC_SAMPLE_STATE_IDLE;
    pstStatus->eConveyorBeltState = TESTPROC_SAMPLE_CB_STATE_IDLE;
    pstStatus->eTestRackState = TESTPROC_TESTRACK_STATE_IDLE;
    pstStatus->eWashDiskState = TESTPROC_WASHDISK_STATE_IDLE;
    pstStatus->eCupStackState = TESTPROC_CUPSTACK_STATE_INIT;
    pstStatus->u8H2CupNumber = TESTPROC_H2_RESET_CUPID;
    pstStatus->u8GarbageNO = 0;
    pstStatus->u8WashDiskHandCup = TESTPROC_WASHDISK_RESET_CUPID;
    pstStatus->u8GetItemCount = 0;
    pstStatus->u8InterferenceReact = 0;
    pstStatus->u8InterferenceReagentDisk = 0;
    pstStatus->u8InterferenceConveyorBelt = 0;
    pstStatus->u8InterferenceSample = 0;
    pstStatus->u8InterferenceWash = 0;
    //pstStatus->u8CupStackRow = 0;
    //pstStatus->u8CupStackCol = 0;
    pstStatus->u8WorkBeforeTest = 0;
    pstStatus->u8CupStackDoorClose = 1;    
    pstStatus->u8LastReagentDiskPos = 0;
    pstStatus->u8SamplePauseCount = 0;
    pstStatus->u32RackOutCount = 0;
    pstStatus->u32EmRackOutCount = 0;
    pstStatus->u32Trigger1Count = 0;
    pstStatus->u32Trigger2Count = 0;
    pstStatus->u32Trigger3Count = 0;
    pstStatus->u32Trigger4Count = 0;
    pstStatus->u32RackID = TESTPROC_INVALID_RACK_ID;
    pstStatus->u32EmRackID = TESTPROC_INVALID_RACK_ID;
    pstStatus->u32RecycleRackID = TESTPROC_TICK_INVALID;
    pstStatus->u32MeasureValue[0] = 0;
    pstStatus->u32MeasureValue[1] = 0;
    pstStatus->s32StandbyCount = -1;
    pstStatus->pstHoldItem = NULL;
    pstStatus->pstMeasureRoomItem = NULL;
    for (i = 0; i <= TESTPROC_CUP_QUANTITY; i++)
    {
        pstStatus->pstReactCupItem[i] = NULL;
        pstStatus->eReactCupState[i] = TESTPROC_REACT_CUP_INIT;
    }
    for (i = 0; i <= TESTPROC_WASHDISK_POS_QUANTITY; i++)
    {
        pstStatus->pstWashCupItem[i] = NULL;
        pstStatus->eWashCupState[i] = TESTPROC_WASH_CUP_INIT;
    }
    
    TestProc_testDataClean();
    OSTmrStop(&g_stWashDiskRotateTimer, OS_OPT_TMR_NONE, NULL, &err);
}

__INLINE static void TestProc_SimulateInit(TESTPROC_STATUS_T *pstStatus)
{
    int i;
    /*
    
    g_stSimulateItem.stItem.u32TestID = 1;
    g_stSimulateItem.stItem.u8SamplePos = 0;
    for (i = 1; i <= TESTPROC_CUP_QUANTITY; i++)
    {
        //pstStatus->eReactCupState[i] = TESTPROC_REACT_CUP_RM2;
        //pstStatus->pstReactCupItem[i] = &g_stSimulateItem;
    }
    // HandCup
    //pstStatus->eReactCupState[54] = TESTPROC_REACT_CUP_INIT;
    //pstStatus->pstReactCupItem[54] = NULL;
    // R_1
    //pstStatus->eReactCupState[53] = TESTPROC_REACT_CUP_IN;
    //pstStatus->pstReactCupItem[53] = &g_stSimulateItem;
    // Sample
    //pstStatus->eReactCupState[52] = TESTPROC_REACT_CUP_R1;
    //pstStatus->pstReactCupItem[52] = &g_stSimulateItem;
    
    for (i = 2; i <= TESTPROC_WASHDISK_POS_QUANTITY; i++)
    {
        OS_ERR os_err;
        pstStatus->eWashCupState[i] = TESTPROC_WASH_CUP_MIX;
        pstStatus->pstWashCupItem[i] = TestProc_testGetEmptyItem();
        pstStatus->pstWashCupItem[i]->stItem.u32TestID = 0;
        pstStatus->pstWashCupItem[i]->stItem.u32RackID = 9999;
    }
    //pstStatus->eMeasureRoomState = TESTPROC_MEASUREROOM_STATE_OVER;
    //pstStatus->pstMeasureRoomItem = &g_stSimulateItem;
    */
    for (i=0; i<TESTPROC_REAGENT_POS_MAX; i++)
    {
        g_stReagentMulty.stR1PosInfo[i].u16ReagentID = i+1;
        g_stReagentMulty.stR1PosInfo[i].u8Exist[0] = 1;
        g_stReagentMulty.stR1PosInfo[i].u8Exist[1] = 1;
        g_stReagentMulty.stR1PosInfo[i].u8Exist[2] = 1;        
        g_stReagentMulty.stR1PosInfo[i].u8ReaLoadType = 1;
        g_stReagentMulty.stR1PosInfo[i].u8Priority = 1;

        g_stReagentMulty.stR2PosInfo[i].u16ReagentID = i+1;
        g_stReagentMulty.stR2PosInfo[i].u8Exist[0] = 1;
        g_stReagentMulty.stR2PosInfo[i].u8Exist[1] = 1;
        g_stReagentMulty.stR2PosInfo[i].u8Exist[2] = 1;        
        g_stReagentMulty.stR2PosInfo[i].u8ReaLoadType = 1;
        g_stReagentMulty.stR2PosInfo[i].u8Priority = 1;
    }
}

__INLINE static uint8_t IsReactCupToWash(TESTPROC_STATUS_T *pstStatus, uint8_t u8CupID)
{
    if (NULL != pstStatus->pstReactCupItem[u8CupID] && 
        (TESTPROC_REACT_CUP_RM2 == pstStatus->eReactCupState[u8CupID] || TESTPROC_REACT_CUP_2STEP_RM2 == pstStatus->eReactCupState[u8CupID]))
    {
        return 1;
    }
    else
    {
        return 0;
    }
}

/****************************************************************************
*
* Function Name:    TestProc_StartReact
* Input:            pstStatus - 测试系统状态
*                   eWork - 启动类型
*                   
* Output:           None
*                   
* Returns:          None
* Description:      
* Note:             
***************************************************************************/
static int32_t TestProc_StartReact(TESTPROC_STATUS_T *pstStatus, REACT_WORK_E eWork)
{
    uint8_t u8Param[4];
    
    if (0 == pstStatus->u8InterferenceReact)
    {
        TestProc_ReactStartWork(eWork, NULL, 0);
        return TESTPROC_RESULT_OK;
    }
    else
    {
        DebugMsg("E>ReactInterference:%02X\r\n", pstStatus->u8InterferenceReact);
        if (pstStatus->u8InterferenceReact & TESTPROC_INTERFERENCE_REACT_HANDCUP)
        {
            u8Param[0] = eWork;
            u8Param[1] = pstStatus->u8InterferenceReact;
            TestProc_ReportWarning(pstStatus, TESTPROC_WARNING_REACT_IF_HANDCUP, u8Param, 2);
        }
        if (pstStatus->u8InterferenceReact & TESTPROC_INTERFERENCE_REACT_REAGENT)
        {
            u8Param[0] = eWork;
            u8Param[1] = pstStatus->u8InterferenceReact;
            TestProc_ReportWarning(pstStatus, TESTPROC_WARNING_REACT_IF_REAGENT, u8Param, 2);
        }
        if (pstStatus->u8InterferenceReact & TESTPROC_INTERFERENCE_REACT_SAMPLE)
        {
            u8Param[0] = eWork;
            u8Param[1] = pstStatus->u8InterferenceReact;
            TestProc_ReportWarning(pstStatus, TESTPROC_WARNING_REACT_IF_SAMPLE, u8Param, 2);
        }
        if (pstStatus->u8InterferenceReact & TESTPROC_INTERFERENCE_REACT_MIX1)
        {
            u8Param[0] = eWork;
            u8Param[1] = pstStatus->u8InterferenceReact;
            TestProc_ReportWarning(pstStatus, TESTPROC_WARNING_REACT_IF_MIX1, u8Param, 2);
        }
        if (pstStatus->u8InterferenceReact & TESTPROC_INTERFERENCE_REACT_MIX2)
        {
            u8Param[0] = eWork;
            u8Param[1] = pstStatus->u8InterferenceReact;
            TestProc_ReportWarning(pstStatus, TESTPROC_WARNING_REACT_IF_MIX2, u8Param, 2);
        }
        if (pstStatus->u8InterferenceReact & TESTPROC_INTERFERENCE_REACT_REAGENT2)
        {
            u8Param[0] = eWork;
            u8Param[1] = pstStatus->u8InterferenceReact;
            TestProc_ReportWarning(pstStatus, TESTPROC_WARNING_REACT_IF_REAGENT2, u8Param, 2);
        }
        
        return TESTPROC_RESULT_NG;
    }
}

void TaskAppFuncTestProcess(void *p_arg)
{
    TESTPROC_MSM_T *pstTaskMsg = NULL;
    int32_t i;
    int32_t s32Result = TESTPROC_RESULT_OK;
    OS_TICK TickNow;
    uint16_t u16Count;
    OS_MSG_SIZE size;
    uint8_t u8BufTmp[4];
    OS_ERR err;
    uint8_t u8OldCellStatus = STATUS_system.stCellStatus.u8WorkStatus;
    
    g_SystemPowerOnTimer = OSTimeGet(&err);
    
    FSM_AbandonCup = FSM_ABANDONCUP_IDLE;
    FSM_RecycleRack = FSM_RECYCLERACK_IDLE;
    FSM_ReagentBarCode = FSM_REAGENTBARCODE_IDLE;
    FSM_ReagentQuantity = FSM_REAGENTQUANTITY_IDLE;
    g_u8TestProc_ResetMask = 0;
    g_stReagentMulty.u8ReagentLimit = 0;
    for (i = 0; i < TESTPROC_REAGENT_POS_MAX; i++)
    {
        g_stReagentMulty.stR1PosInfo[i].u16ReagentID = i + 1;
        g_stReagentMulty.stR1PosInfo[i].u8Priority = 1;
        g_stReagentMulty.stR2PosInfo[i].u16ReagentID = i + 1;
        g_stReagentMulty.stR2PosInfo[i].u8Priority = 1;
    }
    Mem_Clr(&g_u8ReagentScanPos[0], sizeof(g_u8ReagentScanPos));
    TestProc_dlistModuleInit();
    TestProc_testDataInit();
    Mem_Clr(&STATUS_system, sizeof(TESTPROC_STATUS_T));
    TestProc_ResetInit(&STATUS_system);
    STATUS_system.stCellStatus.u8WorkStatus = TESTPROC_WORK_STANDBY;
    STATUS_system.eWorkType = TESTPROC_WORK_TYPE_IDLE;
    for (i = 0; i < TESTPROC_UNIT_MAX; i++)
    {
        STATUS_system.eResetState[i] = TESTPROC_RESET_INIT;
    }
    OSTmrCreate(&g_stStatusReportTimer, "StatusReport", 200, 1000, OS_OPT_TMR_PERIODIC, TestProc_StatusReport, (void *)&STATUS_system, &err);
    FSM_TestProcess = FSM_TESTPROCESS_POWERINIT;
    FSM_PowerOn = FSM_POWERON_RESET;
    u16Count = 0;
    TickNow = OSTimeGet(&err);
    DebugMsg("TestProcess\r\nRelease %s %s\r\n", __DATE__, __TIME__);
    TestProc_WashInit();
    TestProc_ReactInit();
    TestProc_ReagentInit();
    TestProc_ReagentDiskInit();
    TestProc_SampleInit();
    TestProc_HandCupInit();
    OSTmrStart(&g_stStatusReportTimer, &err);
    
    OSTmrCreate(&g_stTestTimer, "Test Timer", 3000, 0, OS_OPT_TMR_ONE_SHOT, TestProc_TestTimer, NULL, &err);
    OSTmrCreate(&g_stWashDiskRotateTimer, "WashRotate Timer", 800, 0, OS_OPT_TMR_ONE_SHOT, TestProc_WashDiskRotateTimer, &STATUS_system, &err);
    if (OS_ERR_NONE != err)
    {
        DebugMsg("ERR>CreateReact23:%d\r\n", err);
    }
    
    while (1)
    {
        pstTaskMsg = (TESTPROC_MSM_T *)OSTaskQPend((OS_TICK)TASK_TEST_PROCESS_SPEED,
                                                   (OS_OPT)OS_OPT_PEND_BLOCKING,
                                                   (OS_MSG_SIZE *)&size,
                                                   (CPU_TS *)0,
                                                   (OS_ERR *)&err);
        // Message Process
        if (pstTaskMsg != NULL)
        {
            //DebugMsg("GetMsg:%d,%p,%p\r\n", pstTaskMsg->eSrcUnit, pstTaskMsg, pstTaskMsg->pData);
            if (TESTPROC_UNIT_CAN1 == pstTaskMsg->eSrcUnit)
            {
                s32Result = TestProc_CanCmdProcess(&STATUS_system, pstTaskMsg->pData);
            }
            else if (TESTPROC_UNIT_UART1 == pstTaskMsg->eSrcUnit)
            {
                s32Result = TestProc_UartCmdProcess(&STATUS_system, (SYSTEMCONFIG_CMD_T *)pstTaskMsg->pData);
            }
            else if (pstTaskMsg->eSrcUnit < TESTPROC_UNIT_MAX)
            {
                s32Result = g_UnitMsgFunc[pstTaskMsg->eSrcUnit](&STATUS_system, pstTaskMsg);
            }
            
            if (s32Result != TESTPROC_RESULT_OK)
            {
                DebugMsg("E>ProcMsg:%d,%d,%X\r\n", s32Result, pstTaskMsg->eSrcUnit, pstTaskMsg->eCommand);
                u8BufTmp[0] = pstTaskMsg->eSrcUnit;
                u8BufTmp[1] = (uint8_t)(0 - s32Result);
                *(uint16_t *)&u8BufTmp[2] = pstTaskMsg->eCommand;
                TestProc_ReportWarning(&STATUS_system, TESTPROC_WARNING_DEBUG1_MSG, u8BufTmp, 4);
            }
            // Free pstTestProcMsg->pu8Data Memory
            if (pstTaskMsg->pData != NULL)
            {
                TestProc_FreeMemory(pstTaskMsg->pData);
            }
            // Release Msg
            TestProc_FreeMemory(pstTaskMsg);
            pstTaskMsg = NULL;
        }
        
        // Final State Machine Process
        TickNow = OSTimeGet(&err);
        TestProc_ReactFSM(TickNow);
        TestProc_HandCupFSM(TickNow);
        TestProc_SampleFSM(TickNow);
        TestProc_ReagentFSM(TickNow);
        TestProc_ReagentDiskFSM(TickNow);
        TestProc_WashFSM(TickNow);
        TestProc_MeasureRoomFSM(TickNow);
        TestProc_SampleCBFSM(TickNow);
        TestProc_CupStackFSM(TickNow);
        if (STATUS_system.stCellStatus.u8WorkStatus != TESTPROC_WORK_EMSTOP)
        {
            TestProc_MainFSM(&STATUS_system);
        }
        if (u16Count++ >= (1000 / TASK_TEST_PROCESS_SPEED)) // 1s轮训
        {
            u16Count = 0;
            TickNow = OSTimeGet(&err);
            // Check Reset
            if (TickNow > g_SystemResetTimer && (TickNow - g_SystemResetTimer) > TIMEOUT_RESET_FOR_SYSTEM)
            {
                g_SystemResetTimer = TESTPROC_TICK_INVALID;
                DebugMsg("**ResetTimeOut**\r\n");
                TestProc_CheckResetNG();
                STATUS_system.stCellStatus.u8WorkStatus = TESTPROC_WORK_RESET_NG;
                FSM_TestProcess = FSM_TESTPROCESS_IDLE;
            }
            //
            //DebugMsg("Status >> %d,%d\r\n", u8OldCellStatus, STATUS_system.stCellStatus.u8WorkStatus);
            if (u8OldCellStatus != STATUS_system.stCellStatus.u8WorkStatus)
            {
                u8OldCellStatus = STATUS_system.stCellStatus.u8WorkStatus;
                TestProc_ReortStatus(&STATUS_system);
            }
            // Check Cmd Timer
            for (i = 0; i < TESTPROC_RSP_CMD_MAX; i++)
            {
                if (g_RspCmdSendTime[i] != TESTPROC_TICK_INVALID && (TickNow - g_RspCmdSendTime[i]) > g_RspCmdHoldTime[i])
                {
                    g_RspCmdSendTime[i] = TESTPROC_TICK_INVALID;
                    if (TESTPROC_WORK_TEST == STATUS_system.stCellStatus.u8WorkStatus)
                    {
                        u8BufTmp[0] = (uint8_t)i;
                        u8BufTmp[1] = STATUS_system.stCellStatus.u8WorkStatus;
                        u8BufTmp[2] = FSM_TestProcess;
                        TestProc_ReportWarning(&STATUS_system, TESTPROC_WARNING_CMD_RSP_GETITEM, u8BufTmp, 3);
                    }
                }
            }
            // 杯栈门状态轮询
            if (0 == STATUS_system.u8CupStackDoorClose)
            {
                if (TestProc_Api_IsCupStackDoorClose() == 1)
                {
                    DebugMsg("DoorClose_1\r\n");
                    STATUS_system.u8CupStackDoorClose = 1;
                    if (STATUS_system.u8CupStackCol >= TESTPROC_CUPSTACK_COLUMN_MAX && STATUS_system.u8CupStackRow >= TESTPROC_CUPSTACK_ROW_MAX)
                    {
                        STATUS_system.eCupStackState = TESTPROC_CUPSTACK_STATE_CHANGE;
                        TestProc_CupStackStartWork(CUPSTACK_WORK_CHANGE_DISK, NULL, 0);
                    }
                }
            }
            else
            {
                if (TestProc_Api_IsCupStackDoorClose() == 0)
                {
                    DebugMsg("DoorOpen_1\r\n");
                    STATUS_system.u8CupStackDoorClose = 0;
                }
            }
            
            SystemConfig_CheckState();
        }
    }
}

int32_t TestProc_SendMsg(TESTPROC_MSG_CMD_E eCmd, TESTPROC_UNIT eSrcUnit, uint8_t *pu8Param, uint8_t u8Size)
{
    int32_t s32Result = TESTPROC_RESULT_OK;
    uint8_t u8BufTmp[4];
    OS_ERR err = OS_ERR_NONE;
    TESTPROC_MSM_T *pstMsg = TestProc_AllocMemory(sizeof(TESTPROC_MSM_T));
    
    if (NULL != pstMsg)
    {
        pstMsg->eCommand = eCmd;
        pstMsg->eSrcUnit = eSrcUnit;
        pstMsg->eDestUnit = TESTPROC_UNIT_PROCTEST;
        pstMsg->u16DataSize = u8Size;
        if (u8Size <= 8)
        {
            Mem_Copy(pstMsg->u8Data, pu8Param, u8Size);
            pstMsg->pData = NULL;
        }
        else
        {
            pstMsg->pData = TestProc_AllocMemory(u8Size);
            if (NULL != pstMsg->pData)
            {
                Mem_Copy(pstMsg->pData, pu8Param, u8Size);
            }
            else
            {
                s32Result = TESTPROC_RESULT_ERR_MEM;
            }
        }
    }
    
    if (TESTPROC_RESULT_OK == s32Result)
    {
        OSTaskQPost((OS_TCB *)&tcbTaskAppTestProcess,
                    (void *)pstMsg,
                    (OS_MSG_SIZE)sizeof(TESTPROC_MSM_T),
                    (OS_OPT)OS_OPT_POST_FIFO,
                    (OS_ERR *)&err);
        if (OS_ERR_NONE != err)
        {
            s32Result = TESTPROC_RESULT_NG;
        }
    }
    
    if (TESTPROC_RESULT_OK != s32Result)
    {
        DebugMsg("E>%d SendMsgNG:%d,%X,%d\r\n", eSrcUnit, s32Result, eCmd, err);
        u8BufTmp[0] = eSrcUnit;
        u8BufTmp[1] = (uint8_t)(0 - s32Result);
        *(uint16_t *)&u8BufTmp[2] = eCmd;
        TestProc_ReportWarning(&STATUS_system, TESTPROC_WARNING_DEBUG1_SEND_MSG, u8BufTmp, 4);
    }
    
    return s32Result;
}


/****************************************************************************
*
* Function Name:    TestProc_CanCmdProcess
* Input:            pstStatus - 系统状态
*                   pstCmd - CAN命令
*                   
* Output:           None
*                   
* Returns:          
* Description:      
* Note:             
***************************************************************************/
static int32_t TestProc_CanCmdProcess(TESTPROC_STATUS_T *pstStatus, const TESTPROC_CAN_CMD_T *pstCmd)
{
    const uint8_t *pu8InData;
    int32_t s32Result = TESTPROC_RESULT_OK;
    int32_t i;
    CAN1_CMD_RECV *pstDebugCmd;
    //OS_ERR err;
    uint16_t u16RackID;
    uint8_t u8TxBuf[8];
    uint8_t u8Tmp;
    
    if (TESTPROC_CELL_PLATFORM_NMBR != pstCmd->u8CellNmbr)
    {
        return TESTPROC_RESULT_ERR_CELL;
    }
    
    if (pstCmd->pu8DataBuf != NULL)
    {
        pu8InData = pstCmd->pu8DataBuf;
    }
    else
    {
        pu8InData = pstCmd->u8DataBuf;
    }
    //DebugMsg("CanCmd:%d,%04X,%02X\r\n", pstCmd->u8CellNmbr, pstCmd->u16Cmd, pstCmd->u8SrcBoard);
    if (BOARD_ID_TRACK_COMM == pstCmd->u8SrcBoard)
    {
        // 中位机消息处理
        switch (pstCmd->u16Cmd)
        {
            case CAN_CMD_CELL_RESET:
                u8Tmp = pu8InData[0] & CAN_CMD_SRC_BOARD_MASK;
                DebugMsg("Cell Reset:%d,%02X\r\n", FSM_TestProcess, u8Tmp);
                if (BOARD_ID_BOARDCAST == u8Tmp)
                {
                    if (FSM_TESTPROCESS_IDLE == FSM_TestProcess)
                    {
                        s32Result = TestProc_Reset(pstStatus);
                        if (TESTPROC_RESULT_OK == s32Result)
                        {
                            FSM_TestProcess = FSM_TESTPROCESS_RESET;
                        }
                    }
                    else
                    {
                        s32Result = TESTPROC_RESULT_ERR_STATUS;
                    }
                }
                break;
            case CAN_CMD_CELL_STOP:
                TestProc_Standby(pstStatus);
                break;
            case CAN_CMD_CELL_START_WORK:
                if (FSM_TESTPROCESS_IDLE == FSM_TestProcess)
                {
                    pstStatus->eWorkType = (TESTPROC_WORK_TYPE)pu8InData[0];
                    switch(pu8InData[0])
                    {
                        case TESTPROC_WORK_TYPE_RESET:

                            break;
                        case TESTPROC_WORK_TYPE_TEST:
                            DebugMsg("StartTest:%02X\r\n", pu8InData[1]);
                            if (0 == (pu8InData[1]&TESTPROC_BEFORE_TEST_ALL))
                            {
                                TestProc_ReactStartWork(REACT_WORK_TEST, NULL, 0);
                                FSM_TestProcess = FSM_TESTPROCESS_TEST;
                                pstStatus->stCellStatus.u8WorkStatus = TESTPROC_WORK_TEST;
                                pstStatus->eCupStackState = TESTPROC_CUPSTACK_STATE_INIT;
                                TestProc_CupStackStartWork(CUPSTACK_WORK_PUSH_TO_POSTION, NULL, 0);
                                u8TxBuf[0] = 1;
                                TestProc_SendDataToComm(BOARD_ID_TRACK_COMM, CAN_CMD_CELL_START_WORK, u8TxBuf, 1);
                            }
                            else
                            {
                                pstStatus->u8WorkBeforeTest = 0;
                                if ((pu8InData[1]&0x01) > 0)
                                {
                                    pstStatus->u8WorkBeforeTest |= TESTPROC_BEFORE_TEST_ADANDONCUP;                            
                                }
                                if ((pu8InData[1]&0x02) > 0)
                                {
                                    pstStatus->u8WorkBeforeTest |= TESTPROC_BEFORE_TEST_RECYCLERACK;
                                }
                                
                                if ((pstStatus->u8WorkBeforeTest&TESTPROC_BEFORE_TEST_RECYCLERACK) > 0)
                                {
                                    FSM_TestProcess = FSM_TESTPROCESS_RECYCLE_RACK;
                                    pstStatus->stCellStatus.u8WorkStatus = TESTPROC_WORK_RECYCLE_RACK;
                                }
                                else if ((pstStatus->u8WorkBeforeTest&TESTPROC_BEFORE_TEST_ADANDONCUP) > 0)
                                {
                                    FSM_TestProcess = FSM_TESTPROCESS_ABANDON_CUP;
                                    FSM_AbandonCup = FSM_ABANDONCUP_HANDCUP_CLEAN;
                                    pstStatus->stCellStatus.u8WorkStatus = TESTPROC_WORK_ABANDON_CUP;
                                }
                            }
                            break;
                        case TESTPROC_WORK_TYPE_SIMULATE:
                            TestProc_ReactStartWork(REACT_WORK_SIMULATE, NULL, 0);
                            FSM_TestProcess = FSM_TESTPROCESS_SIMULATE;
                            pstStatus->u32SimulateCount = BYTE_TO_WORD(pu8InData[4], pu8InData[3], pu8InData[2], pu8InData[1]) + 5;
                            pstStatus->stCellStatus.u8WorkStatus = TESTPROC_WORK_SIMULATE;
                            pstStatus->eCupStackState = TESTPROC_CUPSTACK_STATE_INIT;
                            TestProc_CupStackStartWork(CUPSTACK_WORK_PUSH_TO_POSTION, NULL, 0);
                            TestProc_SimulateInit(pstStatus);
                            DebugMsg("Simulate:%d\r\n", pstStatus->u32SimulateCount);
                            //OSTmrStart(&g_stTestTimer, &err);
                            break;
                        case TESTPROC_WORK_TYPE_ABANDON_CUP:
                            FSM_TestProcess = FSM_TESTPROCESS_ABANDON_CUP;
                            FSM_AbandonCup = FSM_ABANDONCUP_HANDCUP_CLEAN;
                            pstStatus->stCellStatus.u8WorkStatus = TESTPROC_WORK_ABANDON_CUP;
                            break;
                        case TESTPROC_WORK_TYPE_RECYCLE_RACK:
                            FSM_TestProcess = FSM_TESTPROCESS_RECYCLE_RACK;
                            pstStatus->stCellStatus.u8WorkStatus = TESTPROC_WORK_RECYCLE_RACK;
                            break;
                        case TESTPROC_WORK_TYPE_REAGENTBARCODE:
                            DebugMsg("Scan Reagent Code\r\n");
                            FSM_TestProcess = FSM_TESTPROCESS_REAGENTBARCODE;
                            pstStatus->stCellStatus.u8WorkStatus = TESTPROC_WORK_SCAN_REAGENT_BAR;
                            FSM_ReagentBarCode = FSM_REAGENTBARCODE_START;
                            break;
                        case TESTPROC_WORK_TYPE_REAGENTQUANTITY:
                            DebugMsg("Scan Reagent Quantity\r\n");
                            FSM_TestProcess = FSM_TESTPROCESS_REAGENTQUANTITY;
                            pstStatus->stCellStatus.u8WorkStatus = TESTPROC_WORK_REAGENT_QUANTITY;
                            FSM_ReagentQuantity = FSM_REAGENTQUANTITY_START;
                            break;
                        case TESTPROC_WORK_TYPE_M_SEPARATE_AUTO:
                            g_u32MSeparationMixTime = pu8InData[1]*100;
                            g_u16MSeparationQuantity = BYTE_TO_HWORD(pu8InData[3], pu8InData[2]);
                            g_u32MSeparationTestID = BYTE_TO_WORD(pu8InData[7], pu8InData[6], pu8InData[5], pu8InData[4]);
                            DebugMsg("M_SeparationAuto:%d,%d,%d\r\n", g_u16MSeparationQuantity, g_u32MSeparationTestID, g_u32MSeparationMixTime);
                            pstStatus->stCellStatus.u8WorkStatus = TESTPROC_WORK_M_SEPARATE_AUTO;
                            FSM_M_SeparateAuto = FSM_M_SEPARATE_AUTO_START;
                            FSM_TestProcess = FSM_TESTPROCESS_M_SEPARATE_AUTO;
                            break;
                        case TESTPROC_WORK_TYPE_M_SEPARATE:
                            g_u32MSeparationMixTime = pu8InData[1]*100;
                            g_u16MSeparationQuantity = BYTE_TO_HWORD(pu8InData[3], pu8InData[2]);
                            g_u32MSeparationTestID = BYTE_TO_WORD(pu8InData[7], pu8InData[6], pu8InData[5], pu8InData[4]);
                            DebugMsg("M_Separation:%d,%d,%d\r\n", g_u16MSeparationQuantity, g_u32MSeparationTestID, g_u32MSeparationMixTime);
                            pstStatus->stCellStatus.u8WorkStatus = TESTPROC_WORK_M_SEPARATE;
                            FSM_M_Separate = FSM_M_SEPARATE_START;
                            FSM_TestProcess = FSM_TESTPROCESS_M_SEPARATE;
                            break;
                        case TESTPROC_WORK_TYPE_SAMPLE_ACCURACY:
                            g_u16AccuracyTestQuantity = BYTE_TO_HWORD(pu8InData[2], pu8InData[1]);
                            pstStatus->stCellStatus.u8WorkStatus = TESTPROC_WORK_SAMPLE_ACCURACY;
                            FSM_SampleAccuracy = FSM_SAMPLE_ACCURACY_START;
                            FSM_TestProcess = FSM_TESTPROCESS_SAMPLE_ACCURACY;
                            DebugMsg("SampleAccuracy:%d\r\n", g_u16AccuracyTestQuantity);
                            break;
                        case TESTPROC_WORK_TYPE_REAGENT_ACCURACY:
                            g_u16AccuracyTestQuantity = BYTE_TO_HWORD(pu8InData[2], pu8InData[1]);
                            FSM_ReagentAccuracy = FSM_REAGENT_ACCURACY_START;
                            FSM_TestProcess = FSM_TESTPROCESS_REAGENT_ACCURACY;
                            pstStatus->stCellStatus.u8WorkStatus = TESTPROC_WORK_REAGENT_ACCURACY;
                            DebugMsg("ReagentAccuracy:%d\r\n", g_u16AccuracyTestQuantity);
                            break;
                        case TESTPROC_WORK_TYPE_WASH_ACCURACY:
                            g_u8WashAccuracyMask = 0;
                            // Wash Needle 12
                            if (1 == pu8InData[1])
                            {
                                g_u8WashAccuracyMask |= 0x01;
                            }
                            else if (2 == pu8InData[1])
                            {
                                g_u8WashAccuracyMask |= 0x10;
                            }
                            // Wash Needle 34
                            if (1 == pu8InData[2])
                            {
                                g_u8WashAccuracyMask |= 0x02;
                            }
                            else if (2 == pu8InData[2])
                            {
                                g_u8WashAccuracyMask |= 0x20;
                            }
                            // Wash Needle 56
                            if (1 == pu8InData[3])
                            {
                                g_u8WashAccuracyMask |= 0x04;
                            }
                            else if (2 == pu8InData[3])
                            {
                                g_u8WashAccuracyMask |= 0x40;
                            }
                            if (((g_u8WashAccuracyMask&0xF0) > 0 && (g_u8WashAccuracyMask&0x0F) > 0) || 0 == g_u8WashAccuracyMask)
                            {
                                DebugMsg("E>WashAccuracyMask:%02X\r\n", g_u8WashAccuracyMask);
                            }
                            else
                            {
                                FSM_WashAccuracy = FSM_WASH_ACCURACY_START;                        
                                FSM_TestProcess = FSM_TESTPROCESS_WASH_ACCURACY;
                                pstStatus->stCellStatus.u8WorkStatus = TESTPROC_WORK_WASH_ACCURACY;
                                DebugMsg("WashAccuracy:%d\r\n", g_u8WashAccuracyMask);
                            }
                            break;
                        case TESTPROC_WORK_TYPE_DARK_TEST:
                            FSM_DarkTest = FSM_DARK_TEST_START;
                            FSM_TestProcess = FSM_TESTPROCESS_DARK_TEST;
                            pstStatus->stCellStatus.u8WorkStatus = TESTPROC_WORK_DARK_TEST;
                            break;
                        case TESTPROC_WORK_TYPE_NEEDLE_CLEAN_OUT:
                            g_u32MaintainWorkFlag = 0;
                            if (1 == pu8InData[1]) // Sample Needle
                            {
                                g_u32MaintainWorkFlag |= 0x04;
                            }
                            if (1 == pu8InData[2]) // Reagent Needle
                            {
                                g_u32MaintainWorkFlag |= 0x01;
                            }
                            if ((g_u32MaintainWorkFlag&0x05) > 0)
                            {
                                FSM_TestProcess = FSM_TESTPROCESS_NEEDLE_CLEAN;
                                FSM_NeedleClean = FSM_NEEDLE_CLEAN_START;
                                pstStatus->stCellStatus.u8WorkStatus = TESTPROC_WORK_NEEDLE_CLEAN_OUT;
                            }
                            break;
                        case TESTPROC_WORK_TYPE_NEEDLE_CLEAN_IN:
                            g_u32MaintainWorkFlag = 0;
                            if (1 == pu8InData[1]) // Sample Needle
                            {
                                g_u32MaintainWorkFlag |= 0x08;
                            }
                            if (1 == pu8InData[2]) // Reagent Needle
                            {
                                g_u32MaintainWorkFlag |= 0x02;
                            }
                            DebugMsg("NeedleClean:%02X\r\n", g_u32MaintainWorkFlag);
                            if ((g_u32MaintainWorkFlag&0x0A) > 0)
                            {
                                FSM_TestProcess = FSM_TESTPROCESS_NEEDLE_CLEAN;
                                FSM_NeedleClean = FSM_NEEDLE_CLEAN_START;
                                pstStatus->stCellStatus.u8WorkStatus = TESTPROC_WORK_NEEDLE_CLEAN_IN;
                            }
                            break;
                        case TESTPROC_WORK_TYPE_LIQUID_PERFUSE:
                            g_u32MaintainWorkFlag = 0;
                            // Sample
                            if (0x01 == pu8InData[1])
                            {
                                g_u32MaintainWorkFlag |= 0x01;
                            }
                            else if (0x02 == pu8InData[1])
                            {
                                g_u32MaintainWorkFlag |= 0x02;
                            }
                            else if (0x03 == pu8InData[1])
                            {
                                g_u32MaintainWorkFlag |= 0x03;
                            }
                            // Reagent
                            if (0x01 == pu8InData[2])
                            {
                                g_u32MaintainWorkFlag |= 0x04;
                            }
                            else if (0x02 == pu8InData[2])
                            {
                                g_u32MaintainWorkFlag |= 0x08;
                            }
                            else if (0x03 == pu8InData[2])
                            {
                                g_u32MaintainWorkFlag |= 0x0C;
                            }
                            // Wash
                            if (0x01 == pu8InData[3])
                            {
                                g_u32MaintainWorkFlag |= 0x10;
                            }
                            // A
                            if (0x01 == pu8InData[4])
                            {
                                g_u32MaintainWorkFlag |= 0x20;
                            }
                            // B
                            if (0x01 == pu8InData[5])
                            {
                                g_u32MaintainWorkFlag |= 0x40;
                            }
                            // Force Sample
                            if (0x01 == pu8InData[6])
                            {
                                g_u32MaintainWorkFlag |= 0x80;
                            }
                            DebugMsg("DarkPerfuse:%02X\r\n", g_u32MaintainWorkFlag);
                            if (g_u32MaintainWorkFlag > 0)
                            {
                                FSM_LiquidPerfuse = FSM_LIQUID_PERFUSE_START;
                                FSM_TestProcess = FSM_TESTPROCESS_LIQUID_PERFUSE;
                                pstStatus->stCellStatus.u8WorkStatus = TESTPROC_WORK_LIQUID_PERFUSE;
                            }
                            break;
                        default:
                            s32Result = TESTPROC_RESULT_ERR_PARAM;
                            break;
                    }
                }
                else
                {
                    s32Result = TESTPROC_RESULT_ERR_STATUS;
                }
                break;
            case CAN_CMD_CELL_EMSTOP:
                DebugMsg("EmStop\r\n");
                TestProc_Standby(pstStatus);
                break;
            case CAN_CMD_CELL_SAMPLEPAUSE:
                if (1 == pu8InData[0])
                {
                    pstStatus->u8SamplePauseCount = 0;
                    u8Tmp = pstStatus->u8WashDiskHandCup;
                    for (i=0; i<=TESTPROC_WASHDISK_POS_QUANTITY; i++)
                    {
                        if (pstStatus->pstWashCupItem[u8Tmp] != NULL)
                        {
                            if (TestProc_testIsTwoStepItem(pstStatus->pstWashCupItem[i]->stItem.u8AddStep))
                            {
                                if (pstStatus->u8SamplePauseCount < (43-i)) pstStatus->u8SamplePauseCount = 43-i;
                                break;
                            }
                        }
                        u8Tmp ++;
                        if (u8Tmp > TESTPROC_WASHDISK_POS_QUANTITY) u8Tmp = 0;
                    }
                    
                    u8Tmp = pstStatus->u8H2CupNumber;
                    for (i=0; i<=TESTPROC_CUP_QUANTITY; i++)
                    {
                        //DebugMsg(" >> %d,%d\r\n", u8Tmp, pstStatus->eReactCupState[u8Tmp]);
                        if (pstStatus->pstReactCupItem[u8Tmp] != NULL)
                        {
                            if (TESTPROC_REACT_CUP_IN == pstStatus->eReactCupState[u8Tmp] || TESTPROC_REACT_CUP_IN_ING == pstStatus->eReactCupState[u8Tmp])
                            {
                                if (pstStatus->u8SamplePauseCount < (33-i)) pstStatus->u8SamplePauseCount = 33-i;
                                break;
                            }
                            else if (TESTPROC_REACT_CUP_R1 == pstStatus->eReactCupState[u8Tmp])
                            {
                                if (pstStatus->u8SamplePauseCount < (32-i)) pstStatus->u8SamplePauseCount = 32-i;
                                break;
                            }
                            else if (TESTPROC_REACT_CUP_SAMPLE == pstStatus->eReactCupState[u8Tmp] || TESTPROC_REACT_CUP_2STEP_RM1 == pstStatus->eReactCupState[u8Tmp])
                            {
                                if (pstStatus->u8SamplePauseCount < (31-i)) pstStatus->u8SamplePauseCount = 31-i;
                                break;
                            }
                        }
                        u8Tmp ++;
                        if (u8Tmp > TESTPROC_CUP_QUANTITY) u8Tmp = 0;
                    }

                    DebugMsg("***SamlePause:%d***\r\n", pstStatus->u8SamplePauseCount);
                    if (pstStatus->u8SamplePauseCount > 0)
                    {
                        pstStatus->stCellStatus.u8WorkStatus = TESTPROC_WORK_WAIT_SAMPLE_PAUSE;
                    }
                    else
                    {
                        pstStatus->stCellStatus.u8WorkStatus = TESTPROC_WORK_SAMPLE_PAUSE;
                    }
                }
                else
                {
                    if (TESTPROC_WORK_WAIT_SAMPLE_PAUSE == pstStatus->stCellStatus.u8WorkStatus ||
                        TESTPROC_WORK_SAMPLE_PAUSE == pstStatus->stCellStatus.u8WorkStatus)
                    {
                        if (TESTPROC_WORK_TYPE_TEST == pstStatus->eWorkType)
                        {
                            pstStatus->stCellStatus.u8WorkStatus = TESTPROC_WORK_TEST;
                        }
                        else if (TESTPROC_WORK_TYPE_SIMULATE == pstStatus->eWorkType)
                        {
                            pstStatus->stCellStatus.u8WorkStatus = TESTPROC_WORK_SIMULATE;
                        }
                        else
                        {
                            DebugMsg("E>SamplePauseCancel:%d\r\n", pstStatus->eWorkType);
                        }
                    }
                }
                break;
            case CAN_CMD_CELL_UPDATA_PREPARE:
                if (BOARD_ID_CELL_MAIN == pu8InData[0])
                {
                    SystemConfig_SaveConfig(CONFIG_TYPE_NEW_APP);
                    NVIC_SystemReset();
                }
                else
                {
                    DebugMsg("E>UpdataPrepare:%02X\r\n", pu8InData[0]);
                }
                break;
            case CAN_CMD_CELL_MULIT_REAGENT:
                DebugMsg("MultyReagent:%d\r\n", pstCmd->u16Size);
                for (i=0; i<(pstCmd->u16Size/sizeof(REAGENT_POS_T)); i++)
                {
                    Mem_Copy(&g_stReagentMulty.stR1PosInfo[i], pu8InData + i * sizeof(REAGENT_POS_T), sizeof(REAGENT_POS_T));
                }
                for (i=0; i<30; i++)
                {
                    if (g_stReagentMulty.stR1PosInfo[i].u16ReagentID > 0)
                    {
                        DebugMsg("Pos:%2d, Id:%4d, Exist:%d,%d,%d, Pri:%d, Type:%d\r\n", i+1, g_stReagentMulty.stR1PosInfo[i].u16ReagentID, g_stReagentMulty.stR1PosInfo[i].u8Exist[0],
                                 g_stReagentMulty.stR1PosInfo[i].u8Exist[1], g_stReagentMulty.stR1PosInfo[i].u8Exist[2], g_stReagentMulty.stR1PosInfo[i].u8Priority,
                                 g_stReagentMulty.stR1PosInfo[i].u8ReaLoadType);
                    }
                }
                break;
            case CAN_CMD_CELL_DEBUG:
                u16RackID = BYTE_TO_HWORD(pu8InData[2], pu8InData[1]);
                DebugMsg("DebugCmd:%02X,%04X,%d\r\n", pu8InData[0], u16RackID, pstCmd->u16Size);
                if (u16RackID >= CAN_CMD_CELL_DEBUG_START)
                {
                    pstDebugCmd = UserMemMalloc(SRAM_CCM, sizeof(CAN1_CMD_RECV));
                    if (pstDebugCmd != NULL)
                    {
                        u8Tmp = SystemConfig_GetCellNumber();
                        pstDebugCmd->command = u16RackID;
                        pstDebugCmd->srcModuleNo = u8Tmp;
                        pstDebugCmd->targetModuleNo = u8Tmp;
                        pstDebugCmd->srcBoardAddr = BOARD_ID_CELL_MAIN;
                        pstDebugCmd->targetBoardAddr = pu8InData[0];
                        pstDebugCmd->recvDataLength = pstCmd->u16Size-3;
                        Mem_Copy(pstDebugCmd->recvDataBuffer, pu8InData+3, 
                            pstDebugCmd->recvDataLength > sizeof(pstDebugCmd->recvDataBuffer) ? sizeof(pstDebugCmd->recvDataBuffer) : pstDebugCmd->recvDataLength);
                        DebugMsg("DebugCmd:%04X,%d,%d,%02X,%02X,%d\r\n", pstDebugCmd->command, pstDebugCmd->srcModuleNo, pstDebugCmd->targetModuleNo,
                            pstDebugCmd->srcBoardAddr, pstDebugCmd->targetBoardAddr, pstDebugCmd->recvDataLength);
                        Can1DispatchReceive(pstDebugCmd);
                    }
                    else
                    {
                        DebugMsg("E>CanDebugCmd,MemOut:%04X\r\n", pstCmd->u16Cmd);
                    }
                }
                else
                {
                    s32Result = TESTPROC_RESULT_ERR_CMD;
                }
                break;
            case CAN_CMD_CELL_REAGENTQUANTITY:
            case CAN_CMD_CELL_REAGENTBARCODE:
                Mem_Copy(&g_u8ReagentScanPos[0], pu8InData + 1, TESTPROC_REAGENT_POS_MAX);
                DebugMsg("Reagent%s:%d\r\n", CAN_CMD_CELL_REAGENTQUANTITY == pstCmd->u16Cmd ? "Quantity" : "BarCode", pstCmd->u16Size);
                for (i = 0; i < TESTPROC_REAGENT_POS_MAX; i++)
                    DebugMsg("%02X ", g_u8ReagentScanPos[i]);
                DebugMsg("\r\n");
                break;
            case CAN_CMD_CELL_VERSION:
                u8TxBuf[0] = BOARD_ID_CELL_MAIN;
                u8TxBuf[1] = MAINBOARD_VERSION_MAIN;
                u8TxBuf[2] = MAINBOARD_VERSION_SLAVE;
                u8TxBuf[3] = MAINBOARD_VERSION_DEBUG;
                TestProc_SendDataToComm(BOARD_ID_TRACK_COMM, CAN_CMD_CELL_VERSION, u8TxBuf, 4);
                DebugMsg("Read Program Version:%d.%d.%d\r\n", u8TxBuf[1], u8TxBuf[2], u8TxBuf[3]);
                break;
            case CAN_CMD_CELL_BOOT_VERSION:
                u8TxBuf[0] = BOARD_ID_CELL_MAIN;
                u8TxBuf[1] = *(uint8_t *)FLASH_BOOT_CONFIG_VER_1;
                u8TxBuf[2] = *(uint8_t *)FLASH_BOOT_CONFIG_VER_2;
                u8TxBuf[3] = *(uint8_t *)FLASH_BOOT_CONFIG_VER_3;
                TestProc_SendDataToComm(BOARD_ID_TRACK_COMM, CAN_CMD_CELL_BOOT_VERSION, u8TxBuf, 4);
                break;
            case CAN_CMD_COMM_TEST_PARAM:
                if (0x01 == pu8InData[0])
                {
                    DebugMsg("Ageing Mode\r\n");
                    TestModeSelection(TSC_TEST_MODE_AGEING);
                }
                else
                {
                    DebugMsg("Normal Mode\r\n");
                    TestModeSelection(TSC_TEST_MODE_NORMAL);
                }
                if (pu8InData[1] < TESTPROC_CUPSTACK_ROW_MAX && pu8InData[2] < TESTPROC_CUPSTACK_COLUMN_MAX)
                {
                    pstStatus->u8CupStackRow = pu8InData[1];
                    pstStatus->u8CupStackCol = pu8InData[2];
                    DebugMsg("CupStackPos:%d,%d\r\n", pstStatus->u8CupStackRow, pstStatus->u8CupStackCol);
                }
                else
                {
                    DebugMsg("E>CupStackPos:%d,%d\r\n", pu8InData[1], pu8InData[2]);
                }
                break;
            case CAN_CMD_COMM_RACKIN:
                pstStatus->eTestRackState = TESTPROC_TESTRACK_STATE_IN;
                u16RackID = BYTE_TO_HWORD(pu8InData[2], pu8InData[1]);
                if (1 == pu8InData[0])
                {
                    if (TESTPROC_INVALID_RACK_ID == pstStatus->u32EmRackID)
                    {
                        pstStatus->u32EmRackID = u16RackID;
                        pstStatus->eEmRackState = TESTPROC_RACK_STATE_GETITEM;
                        pstStatus->u8GetItemCount++;
                    }
                    else
                    {
                        pstStatus->eEmRackState = TESTPROC_RACK_STATE_TEST;
                        DebugMsg("E>EmRackIn:%d\r\n", pstStatus->u32EmRackID);
                        *(uint16_t *)&u8TxBuf[0] = u16RackID;
                        *(uint16_t *)&u8TxBuf[2] = pstStatus->u32EmRackID;
                        TestProc_ReportWarning(pstStatus, TESTPROC_WARNING_FLOW_EMRACK_IN, u8TxBuf, 4);
                    }
                }
                else
                {
                    if (TESTPROC_INVALID_RACK_ID == pstStatus->u32RackID)
                    {
                        pstStatus->u32RackID = u16RackID;
                        pstStatus->eRackState = TESTPROC_RACK_STATE_GETITEM;
                        pstStatus->u8GetItemCount++;
                    }
                    else
                    {
                        pstStatus->eRackState = TESTPROC_RACK_STATE_TEST;
                        DebugMsg("E>RackIn:%d\r\n", pstStatus->u32RackID);
                        *(uint16_t *)&u8TxBuf[0] = u16RackID;
                        *(uint16_t *)&u8TxBuf[2] = pstStatus->u32RackID;
                        TestProc_ReportWarning(pstStatus, TESTPROC_WARNING_FLOW_RACK_IN, u8TxBuf, 4);
                    }
                }
                
                if (TESTPROC_WORK_SIMULATE == pstStatus->stCellStatus.u8WorkStatus)
                {
                    pstStatus->u32RackOutCount = 7;
                }
                if (TESTPROC_RACK_STATE_GETITEM == pstStatus->eRackState || TESTPROC_RACK_STATE_GETITEM == pstStatus->eEmRackState)
                {
                    TestProc_GetRackItem(pstStatus, u16RackID);
                }
                DebugMsg("**RackIn:%d,%d\r\n", pstStatus->u32RackID, pstStatus->u32EmRackID);
                TestProc_SendDataToComm(BOARD_ID_TRACK_COMM, CAN_CMD_COMM_RACKIN, u8TxBuf, 1);
                break;
            case CAN_CMD_COMM_GET_ITEM:
                g_RspCmdSendTime[TESTPROC_RSP_CMD_CAN_GETITEM] = TESTPROC_TICK_INVALID;
                u16RackID = BYTE_TO_HWORD(pstCmd->u8DataBuf[1], pstCmd->u8DataBuf[0]);
                if (1 == pstCmd->u8DataBuf[2])
                {
                    pstStatus->u8GetItemCount = 0;
                    if (TestProc_IsEmRack(u16RackID) == 1)
                    {
                        pstStatus->eEmRackState = TESTPROC_RACK_STATE_TEST;
                    }
                    else
                    {
                        pstStatus->eRackState = TESTPROC_RACK_STATE_TEST;
                    }
                }
                DebugMsg("Rsp>GetItem:%d,%d,%d\r\n", u16RackID, pstStatus->u8GetItemCount, pstCmd->u8DataBuf[2]);
                break;
            case CAN_CMD_COMM_TESTITEM:
                if (NULL != pstCmd->pu8DataBuf)
                {
                    uint16_t u16ItemQuantity = pstCmd->u16Size / sizeof(__ITEM_T);
                    u16RackID = TESTPROC_INVALID_RACK_ID;
                    s32Result = TestProc_testDataPush(pstCmd->pu8DataBuf, u16ItemQuantity);
                    if (s32Result > 0) // Test Data Transfer Over
                    {
                        u16RackID = (uint16_t)(((__ITEM_T *)pstCmd->pu8DataBuf)->u32RackID);
                        if (TestProc_IsEmRack(u16RackID) == 1)
                        {
                            pstStatus->eEmRackState = TESTPROC_RACK_STATE_TEST;
                            pstStatus->u32EmRackOutCount += s32Result;
                            TestProc_GetRackItem(pstStatus, u16RackID);
                        }
                        else
                        {
                            pstStatus->eRackState = TESTPROC_RACK_STATE_TEST;
                            pstStatus->u32RackOutCount += s32Result;
                        }
                        DebugMsg("RxItem:%d,%d,%d\r\n", pstStatus->u32EmRackOutCount, pstStatus->u32RackOutCount, s32Result);
                        //
                        if ((uint16_t)s32Result != u16ItemQuantity)
                        {
                            DebugMsg("WARNING>LostItem:%d,%d\r\n", s32Result, u16ItemQuantity);
                            *(uint16_t *)&u8TxBuf[0] = (uint16_t)s32Result;
                            *(uint16_t *)&u8TxBuf[2] = u16ItemQuantity;
                            TestProc_ReportWarning(pstStatus, TESTPROC_WARNING_FLOW_ITEM_LOST, u8TxBuf, 4);
                            s32Result = TESTPROC_RESULT_ERR_ITEMLOST;
                            u16ItemQuantity = (uint16_t)s32Result;
                        }
                        else
                        {
                            s32Result = TESTPROC_RESULT_OK;
                        }
                    }
                    else
                    {
                        u16ItemQuantity = 0;
                    }
                    u8TxBuf[0] = HWORD_TO_BYTE_L(u16RackID);
                    u8TxBuf[1] = HWORD_TO_BYTE_H(u16RackID);
                    u8TxBuf[2] = HWORD_TO_BYTE_L(u16ItemQuantity);
                    u8TxBuf[3] = HWORD_TO_BYTE_H(u16ItemQuantity);
                    TestProc_SendDataToComm(BOARD_ID_TRACK_COMM, CAN_CMD_COMM_TESTITEM, u8TxBuf, 4);
                }
                else
                {
                    s32Result = TESTPROC_RESULT_ERR_ITEM;
                }
                break;
            case CAN_CMD_COMM_RACKCHANGE:
                if (0 == pu8InData[0])
                {
                    s32Result = TESTPROC_RESULT_NG;
                    pstStatus->eRecycleStatus = TESTPROC_RECYCLE_STATUS_ERROR;
                    DebugMsg("WARNING>RackChangeFail\r\n");
                }
                else
                {
                    DebugMsg("RackChangeOk:%d,%d\r\n", pstStatus->u32RecycleRackID, pstStatus->eRackState);
                    pstStatus->u32RecycleRackID = TESTPROC_INVALID_RACK_ID;
                    pstStatus->eRecycleStatus = TESTPROC_RECYCLE_TSC_CMD_STATE_IDLE;
                }
                break;
            case CAN_CMD_COMM_ONOFF_LINE:
                
                break;
            case CAN_CMD_COMM_RACK_PICKUP:
                DebugMsg("RackPickUp:%d,%d\r\n", pu8InData[0], pstStatus->eRackState);
                pstStatus->u8InterferenceConveyorBelt &= ~TESTPROC_INTERFERENCE_CB_TRACK;
                if (0 == pu8InData[0])
                {
                    // 样本机构回收位置的样本架已经被抓走
                    if (TESTPROC_RACK_STATE_ATWAIT == pstStatus->eRackState)
                    {
                        pstStatus->eRackState = TESTPROC_RACK_STATE_TOTEST;
                        u8TxBuf[0] = 1;
                        DebugMsg("WARNING>RackWaitToTest:%d\r\n", pstStatus->u32RackID);
                    }
                    else
                    {
                        u8TxBuf[0] = 1;
                        pstStatus->eConveyorBeltState = TESTPROC_SAMPLE_CB_STATE_RESET;
                        TestProc_ConveyorBeltStartWork(CONVEYORBELT_WORK_MOVETOPOS, u8TxBuf, 1);
                    }
                }
                else
                {
                    DebugMsg("E>RackPickUp:%d\r\n", pu8InData[0]);
                }
                break;
            case CAN_CMD_COMM_RECYCLE_RACK:
                if (0 == pu8InData[0])
                {
                    FSM_RecycleRack = FSM_RECYCLERACK_TEST;
                }
                else
                {
                    TestProc_SendDataToComm(BOARD_ID_TRACK_COMM, CAN_CMD_COMM_RACKCHANGE, NULL, 0);
                }
                break;
            case CAN_CMD_CELL_POLLUTE_REAGENT:

                break;
            case CAN_CMD_CELL_POLLUTE_SAMPLE:

                break;
            case CAN_CMD_CELL_POLLUTE_CUP:

                break;
            case CAN_CMD_COMM_PREPARE_TEST:
                g_u32MSeparationMixTime = pu8InData[1]*100;
                g_u16MSeparationQuantity = BYTE_TO_HWORD(pu8InData[3], pu8InData[2]);
                g_u32MSeparationTestID = BYTE_TO_WORD(pu8InData[7], pu8InData[6], pu8InData[5], pu8InData[4]);
                DebugMsg("PrepareTest:%d,%d,%d\r\n", g_u16MSeparationQuantity, g_u32MSeparationTestID, g_u32MSeparationMixTime);
                pstStatus->stCellStatus.u8WorkStatus = TESTPROC_WORK_MANUAL_TEST;
                FSM_TestProcess = FSM_TESTPROCESS_MANUAL_TEST;
                FSM_ManualTest = FSM_MANUAL_TEST_PREPARE;
                break;
            case CAN_CMD_COMM_MANUAL_TEST:
                if (FSM_TESTPROCESS_MANUAL_TEST == FSM_TestProcess)
                {
                    FSM_ManualTest = FSM_MANUAL_TEST_START_TEST;
                }
                else
                {
                    DebugMsg("E>ManualTest:%d\r\n", FSM_TestProcess);
                }
                break;
            default:
                s32Result = TESTPROC_RESULT_ERR_CMD;
                break;
        }
    }
    
    if (s32Result != TESTPROC_RESULT_OK)
    {
        DebugMsg("E>CanCmdProc:%d,%04x,%d,%02X,%d,%d,%d\r\n", s32Result, pstCmd->u16Cmd, FSM_TestProcess, u8Tmp, pu8InData[0], pu8InData[1], pu8InData[2]);
    }
    
    if (pstCmd->pu8DataBuf != NULL)
    {
        TestProc_FreeMemory(pstCmd->pu8DataBuf);
    }
    
    return s32Result;
}

static int32_t TestProc_MsgReact(TESTPROC_STATUS_T *pstStatus, const TESTPROC_MSM_T *pstMsg)
{
    int32_t s32Result = TESTPROC_RESULT_OK;
    uint32_t u32TriggerCount = 0;
    OS_ERR err;
    uint8_t u8TxBuf[8];
    uint8_t u8CupID;
    
    switch (pstMsg->eCommand)
    {
        case TESTPROC_MSG_CMD_RESET:
            DebugMsg("ReactResetResult:%d\r\n", pstMsg->u8Data[0]);
            if (OK == pstMsg->u8Data[0])
            {
                pstStatus->eResetState[TESTPROC_UNIT_REACT] = TESTPROC_RESET_OK;
            }
            else
            {
                pstStatus->eResetState[TESTPROC_UNIT_REACT] = TESTPROC_RESET_NG;
            }
            break;
        case TESTPROC_MSG_CMD_REACT_TRIGGER:
            u32TriggerCount = *((uint32_t *)(&pstMsg->u8Data[1]));
            if (REACT_TRIGGER_1_STEP == pstMsg->u8Data[0])
            {
                pstStatus->u32Trigger1Count++;
                if (u32TriggerCount != pstStatus->u32Trigger1Count)
                {
                    DebugMsg("E>LostTrigge1:%d,%d\r\n", u32TriggerCount, pstStatus->u32Trigger1Count);
                    *(uint16_t *)&u8TxBuf[0] = (uint16_t)(u32TriggerCount & 0xFFFF);
                    *(uint16_t *)&u8TxBuf[2] = (uint16_t)(pstStatus->u32Trigger1Count & 0xFFFF);
                    TestProc_ReportWarning(pstStatus, TESTPROC_WARNING_REACT_RUN_TRIGGER_23, u8TxBuf, 4);
                }
                pstStatus->eReactState = TESTPROC_REACT_STATE_23_RUN;
                s32Result = TestProc_StartReact(pstStatus, REACT_WORK_STEP1_RUN);
                DebugMsg("\r\nTrigge1:%d,%d,%d\r\n", u32TriggerCount, pstStatus->u32Trigger1Count, pstStatus->u8H2CupNumber);
                
                if (OSTmrStateGet(&g_stWashDiskRotateTimer, &err) == OS_TMR_STATE_RUNNING)
                {
                    OSTmrStop(&g_stWashDiskRotateTimer, OS_OPT_TMR_NONE, NULL, &err);
                }
                OSTmrStart(&g_stWashDiskRotateTimer, &err);
                g_u8ReagentDiskShakeEnable = 1;
            }
            else if (REACT_TRIGGER_2_STEP == pstMsg->u8Data[0])
            {
                pstStatus->u32Trigger2Count++;
                if (u32TriggerCount != pstStatus->u32Trigger2Count)
                {
                    DebugMsg("E>LostTrigge2:%d,%d\r\n", u32TriggerCount, pstStatus->u32Trigger2Count);
                    *(uint16_t *)&u8TxBuf[0] = (uint16_t)(u32TriggerCount & 0xFFFF);
                    *(uint16_t *)&u8TxBuf[2] = (uint16_t)(pstStatus->u32Trigger2Count & 0xFFFF);
                    TestProc_ReportWarning(pstStatus, TESTPROC_WARNING_REACT_RUN_TRIGGER_2, u8TxBuf, 4);
                }
                pstStatus->eReactState = TESTPROC_REACT_STATE_02_RUN;
                s32Result = TestProc_StartReact(pstStatus, REACT_WORK_STEP2_RUN);
                DebugMsg("Trigge2:%d,%d,%d\r\n", u32TriggerCount, pstStatus->u32Trigger2Count, pstStatus->u8H2CupNumber);
            }
            else if (REACT_TRIGGER_3_STEP == pstMsg->u8Data[0])
            {
                pstStatus->u32Trigger3Count++;
                if (u32TriggerCount != pstStatus->u32Trigger3Count)
                {
                    DebugMsg("E>LostTrigge3:%d,%d\r\n", u32TriggerCount, pstStatus->u32Trigger3Count);
                    *(uint16_t *)&u8TxBuf[0] = (uint16_t)(u32TriggerCount & 0xFFFF);
                    *(uint16_t *)&u8TxBuf[2] = (uint16_t)(pstStatus->u32Trigger3Count & 0xFFFF);
                    TestProc_ReportWarning(pstStatus, TESTPROC_WARNING_REACT_RUN_TRIGGER_9, u8TxBuf, 4);
                }
                pstStatus->eReactState = TESTPROC_REACT_STATE_09_RUN;
                s32Result = TestProc_StartReact(pstStatus, REACT_WORK_STEP3_RUN);
                DebugMsg("Trigge3:%d,%d,%d\r\n", u32TriggerCount, pstStatus->u32Trigger3Count, pstStatus->u8H2CupNumber);
            }
            else if (REACT_TRIGGER_4_STEP == pstMsg->u8Data[0])
            {
                pstStatus->u32Trigger4Count++;
                if (u32TriggerCount != pstStatus->u32Trigger4Count)
                {
                    DebugMsg("E>LostTrigge4:%d,%d\r\n", u32TriggerCount, pstStatus->u32Trigger4Count);
                    *(uint16_t *)&u8TxBuf[0] = (uint16_t)(u32TriggerCount & 0xFFFF);
                    *(uint16_t *)&u8TxBuf[2] = (uint16_t)(pstStatus->u32Trigger4Count & 0xFFFF);
                    TestProc_ReportWarning(pstStatus, TESTPROC_WARNING_REACT_RUN_TRIGGER_31, u8TxBuf, 4);
                }
                pstStatus->eReactState = TESTPROC_REACT_STATE_31_RUN;
                s32Result = TestProc_StartReact(pstStatus, REACT_WORK_STEP4_RUN);
                DebugMsg("Trigge4:%d,%d,%d,%d\r\n", u32TriggerCount, pstStatus->u32Trigger4Count, pstStatus->u8H2CupNumber, TestProc_R123MCupID(TestProc_H2CupID(REACT_TRIGGER_4_STEP)));
                g_u8ReagentDiskShakeEnable = 1;
            }
            else
            {
                DebugMsg("E>ReactTriggerType:%d\r\n", pstMsg->u8Data[0]);
                u8TxBuf[0] = pstMsg->u8Data[0];
                TestProc_ReportWarning(pstStatus, TESTPROC_WARNING_REACT_RUN_TYPE, u8TxBuf, 1);
            }
            break;
        case TESTPROC_MSG_CMD_REACT_STOP:
            u32TriggerCount = *((uint32_t *)(&pstMsg->u8Data[1]));
            if (REACT_TRIGGER_1_STEP == pstMsg->u8Data[0])
            {
                if (u32TriggerCount != pstStatus->u32Trigger1Count)
                {
                    DebugMsg("E>LostStop1:%d,%d\r\n", u32TriggerCount, pstStatus->u32Trigger1Count);
                    *(uint16_t *)&u8TxBuf[0] = (uint16_t)(u32TriggerCount & 0xFFFF);
                    *(uint16_t *)&u8TxBuf[2] = (uint16_t)(pstStatus->u32Trigger1Count & 0xFFFF);
                    TestProc_ReportWarning(pstStatus, TESTPROC_WARNING_REACT_STOP_TRIGGER_23, u8TxBuf, 4);
                }
                pstStatus->eReactState = TESTPROC_REACT_STATE_23_STOP;
                //DebugMsg(" >> %2d,%d\r\n", STATUS_system.u8H2CupNumber, u32TriggerCount);
                if (0 != u32TriggerCount) // 复位后的触发杯号不变
                {
                    pstStatus->u8H2CupNumber = TestProc_H2CupID(REACT_TRIGGER_1_STEP);
                }
                DebugMsg("Stop1:%2d,%d\r\n", pstStatus->u8H2CupNumber, pstStatus->u32Trigger1Count);
                if (pstStatus->u8SamplePauseCount > 0)
                {
                    DebugMsg("SampleP:%d\r\n", pstStatus->u8SamplePauseCount);
                    pstStatus->u8SamplePauseCount --;
                    if (pstStatus->u8SamplePauseCount == 0)
                    {
                        //DebugMsg("SampleP\r\n");
                        pstStatus->stCellStatus.u8WorkStatus = TESTPROC_WORK_SAMPLE_PAUSE;
                    }
                }
            }
            else if (REACT_TRIGGER_2_STEP == pstMsg->u8Data[0])
            {
                if (u32TriggerCount != pstStatus->u32Trigger2Count)
                {
                    DebugMsg("E>LostStop2:%d,%d\r\n", u32TriggerCount, pstStatus->u32Trigger2Count);
                    *(uint16_t *)&u8TxBuf[0] = (uint16_t)(u32TriggerCount & 0xFFFF);
                    *(uint16_t *)&u8TxBuf[2] = (uint16_t)(pstStatus->u32Trigger2Count & 0xFFFF);
                    TestProc_ReportWarning(pstStatus, TESTPROC_WARNING_REACT_STOP_TRIGGER_2, u8TxBuf, 4);
                }
                pstStatus->eReactState = TESTPROC_REACT_STATE_02_STOP;
                pstStatus->u8H2CupNumber = TestProc_H2CupID(REACT_TRIGGER_2_STEP);
                DebugMsg("Stop2:%2d,%d\r\n", pstStatus->u8H2CupNumber, pstStatus->u32Trigger2Count);
            }
            else if (REACT_TRIGGER_3_STEP == pstMsg->u8Data[0])
            {
                if (u32TriggerCount != pstStatus->u32Trigger3Count)
                {
                    DebugMsg("E>LostStop3:%d,%d\r\n", u32TriggerCount, pstStatus->u32Trigger3Count);
                    *(uint16_t *)&u8TxBuf[0] = (uint16_t)(u32TriggerCount & 0xFFFF);
                    *(uint16_t *)&u8TxBuf[2] = (uint16_t)(pstStatus->u32Trigger3Count & 0xFFFF);
                    TestProc_ReportWarning(pstStatus, TESTPROC_WARNING_REACT_STOP_TRIGGER_9, u8TxBuf, 4);
                }
                pstStatus->eReactState = TESTPROC_REACT_STATE_09_STOP;
                pstStatus->u8H2CupNumber = TestProc_H2CupID(REACT_TRIGGER_3_STEP);
                DebugMsg("Stop3:%2d,%d\r\n", pstStatus->u8H2CupNumber, pstStatus->u32Trigger3Count);
            }
            else if (REACT_TRIGGER_4_STEP == pstMsg->u8Data[0])
            {
                if (u32TriggerCount != pstStatus->u32Trigger4Count)
                {
                    DebugMsg("E>LostStop4:%d,%d\r\n", u32TriggerCount, pstStatus->u32Trigger4Count);
                    *(uint16_t *)&u8TxBuf[0] = (uint16_t)(u32TriggerCount & 0xFFFF);
                    *(uint16_t *)&u8TxBuf[2] = (uint16_t)(pstStatus->u32Trigger4Count & 0xFFFF);
                    TestProc_ReportWarning(pstStatus, TESTPROC_WARNING_REACT_STOP_TRIGGER_31, u8TxBuf, 4);
                }
                pstStatus->eReactState = TESTPROC_REACT_STATE_31_STOP;
                pstStatus->u8H2CupNumber = TestProc_H2CupID(REACT_TRIGGER_4_STEP);
                DebugMsg("Stop4:%2d,%d\r\n", pstStatus->u8H2CupNumber, pstStatus->u32Trigger4Count);
                // RM1
                u8TxBuf[0] = 0;
                u8CupID = TestProc_RM1CupID();
                if (TESTPROC_REACT_CUP_SAMPLE == pstStatus->eReactCupState[u8CupID] || TESTPROC_REACT_CUP_2STEP_SAMPLE == pstStatus->eReactCupState[u8CupID])
                {
                    pstStatus->u8InterferenceReact |= TESTPROC_INTERFERENCE_REACT_MIX1;
                    if (TESTPROC_REACT_CUP_2STEP_SAMPLE == pstStatus->eReactCupState[u8CupID])
                    {
                        pstStatus->eReactCupState[u8CupID] = TESTPROC_REACT_CUP_2STEP_RM2;
                    }
                    else
                    {
                        pstStatus->eReactCupState[u8CupID] = TESTPROC_REACT_CUP_RM1;
                    }
                    u8TxBuf[0] |= REACT_RM1_MIX;
                    *(uint16_t *)&u8TxBuf[1] = TESTPROC_RM1_MIX_TIME;
                    DebugMsg(" +RM1:%d,%d\r\n", u8CupID, pstStatus->pstReactCupItem[u8CupID]->stItem.u32TestID);
                }
                // RM2
                u8CupID = TestProc_RM2CupID();
                //DebugMsg(" +RM2_>>:%d,%d\r\n", u8CupID, pstStatus->eReactCupState[u8CupID]);
                if (TESTPROC_REACT_CUP_R2 == pstStatus->eReactCupState[u8CupID] || TESTPROC_REACT_CUP_2STEP_R2 == pstStatus->eReactCupState[u8CupID])
                {
                    pstStatus->u8InterferenceReact |= TESTPROC_INTERFERENCE_REACT_MIX2;
                    pstStatus->eReactCupState[u8CupID] = TESTPROC_REACT_CUP_RM2;
                    u8TxBuf[0] |= REACT_RM1_MIX;
                    *(uint16_t *)&u8TxBuf[3] = TESTPROC_RM2_MIX_TIME;
                    DebugMsg(" +RM2:%d,%d\r\n", u8CupID, pstStatus->pstReactCupItem[u8CupID]->stItem.u32TestID);
                }
                if (u8TxBuf[0] > 0)
                {
                    TestProc_ReactStartWork(REACT_WORK_MIX, u8TxBuf, 5);
                }
                                
                // check Standby
                if (TESTPROC_WORK_SIMULATE == pstStatus->stCellStatus.u8WorkStatus)
                {
                    DebugMsg("Simulate:%d\r\n", pstStatus->u32SimulateCount);
                    if (pstStatus->u32SimulateCount > 0)
                    {
                        pstStatus->u32SimulateCount--;
                    }
                    else
                    {
                        TestProc_Standby(pstStatus);
                    }
                }                
                else if (TESTPROC_WORK_TEST == pstStatus->stCellStatus.u8WorkStatus)
                {
                    if (pstStatus->s32StandbyCount > 0)
                    {
                        pstStatus->s32StandbyCount --;
                        if (0 == pstStatus->s32StandbyCount)
                        {
                            TestProc_Standby(pstStatus);
                        }
                    }
                }
            }
            else
            {
                DebugMsg("E>ReactStopType:%d\r\n", pstMsg->u8Data[0]);
                u8TxBuf[0] = pstMsg->u8Data[0];
                TestProc_ReportWarning(pstStatus, TESTPROC_WARNING_REACT_STOP_TYPE, u8TxBuf, 1);
            }
            break;
        case TESTPROC_MSG_CMD_REACT_MIX_OK:
            pstStatus->u8InterferenceReact &= ~(TESTPROC_INTERFERENCE_REACT_MIX1 | TESTPROC_INTERFERENCE_REACT_MIX2);
            DebugMsg("ReactMixOk\r\n");
            break;
        default:
            s32Result = TESTPROC_RESULT_ERR_CMD;
            break;
    }

    if (TESTPROC_RESULT_OK != s32Result)
    {
        DebugMsg("E>React:%d,%04X,%d,%02X,%d\r\n", s32Result, pstMsg->eCommand, pstMsg->u8Data[0], pstStatus->u8InterferenceReact, u32TriggerCount);
    }
    return s32Result;
}

static int32_t TestProc_MsgWashDisk(TESTPROC_STATUS_T *pstStatus, const TESTPROC_MSM_T *pstMsg)
{
    int32_t s32Result = TESTPROC_RESULT_OK;
    TESTPROC_ITEM_RESULT_T stTestResult;
    //uint16_t u16Tmp;
    uint8_t u8CupID;
    uint8_t u8WashMask;
    
    switch (pstMsg->eCommand)
    {
        case TESTPROC_MSG_CMD_RESET:
            DebugMsg("WashDiskResetResult:%d\r\n", pstMsg->u8Data[0]);
            if (OK == pstMsg->u8Data[0])
            {
                pstStatus->eResetState[TESTPROC_UNIT_WASH] = TESTPROC_RESET_OK;
            }
            else
            {
                pstStatus->eResetState[TESTPROC_UNIT_WASH] = TESTPROC_RESET_NG;
            }
            break;
        case TESTPROC_MSG_CMD_WASH_MEASUREROOM_HALFOPEN:
            if (TESTPROC_MEASUREROOM_STATE_HALFOPEN_ING == pstStatus->eMeasureRoomState)
            {
                pstStatus->eMeasureRoomState = TESTPROC_MEASUREROOM_STATE_HALFOPEN;
            }
            else
            {
                s32Result = TESTPROC_RESULT_ERR_STATUS;
            }
            //DebugMsg("MR_HalfOpen:%d\r\n", pstStatus->eMeasureRoomState);
            break;
        case TESTPROC_MSG_CMD_WASH_MEASUREROOM_FULLOPEN:
            if (TESTPROC_MEASUREROOM_STATE_FULLOPENING == pstStatus->eMeasureRoomState)
            {
                pstStatus->eMeasureRoomState = TESTPROC_MEASUREROOM_STATE_WAITCATCH;
            }
            else
            {
                s32Result = TESTPROC_RESULT_ERR_STATUS;
            }
            //DebugMsg("MR_FullOpen:%d\r\n", pstStatus->eMeasureRoomState);
            break;
        case TESTPROC_MSG_CMD_WASH_MEASUREROOM_CLOSE:
            //DebugMsg("MR_Close:%d\r\n", pstStatus->eMeasureRoomState);
            if (TESTPROC_MEASUREROOM_STATE_CLOSE_ING == pstStatus->eMeasureRoomState)
            {
                pstStatus->eMeasureRoomState = TESTPROC_MEASUREROOM_STATE_READY;
            }
            else if (TESTPROC_MEASUREROOM_STATE_ABANDON_ING == pstStatus->eMeasureRoomState)
            {
                DebugMsg("MR_Idle:%p,%d\r\n", pstStatus->pstMeasureRoomItem, NULL == pstStatus->pstMeasureRoomItem ? 0 : pstStatus->pstMeasureRoomItem->stItem.u32TestID);
                pstStatus->eMeasureRoomState = TESTPROC_MEASUREROOM_STATE_IDLE;
                if (pstStatus->pstMeasureRoomItem != NULL)
                {
                    TestProc_testDataItemFree(pstStatus->pstMeasureRoomItem);
                    pstStatus->pstMeasureRoomItem = NULL;
                }
            }
            else
            {
                s32Result = TESTPROC_RESULT_ERR_STATUS;
            }
            break;
        case TESTPROC_MSG_CMD_WASH_MEASUREROOM_VALUE1:
            pstStatus->u32MeasureValue[0] = *(uint32_t *)&pstMsg->u8Data[0];
            Mem_Clr(&stTestResult, sizeof(TESTPROC_ITEM_RESULT_T));
            stTestResult.u32TestResult = pstStatus->u32MeasureValue[0];
            stTestResult.u32TestNmbr = pstStatus->pstMeasureRoomItem->stItem.u32TestID;
            stTestResult.u8DataType = 9; // 底物
            stTestResult.u8CellNmbr = SystemConfig_GetCellNumber() + 1;
            stTestResult.u8ReagentPos = pstStatus->pstMeasureRoomItem->u8ReagentPos;
            stTestResult.u8TestType = 0;
            TestProc_SendDataToComm(BOARD_ID_TRACK_COMM, CAN_CMD_COMM_TESTDATA, (uint8_t *)&stTestResult, sizeof(TESTPROC_ITEM_RESULT_T));
            DebugMsg("MeasureValue1:%d,%d\r\n", stTestResult.u32TestNmbr, pstStatus->u32MeasureValue[0]);
            break;
        case TESTPROC_MSG_CMD_WASH_MEASUREROOM_VALUE2:
            pstStatus->eMeasureRoomState = TESTPROC_MEASUREROOM_STATE_OVER;
            pstStatus->u32MeasureValue[1] = *(uint32_t *)&pstMsg->u8Data[0];
            Mem_Clr(&stTestResult, sizeof(TESTPROC_ITEM_RESULT_T));
            stTestResult.u32TestResult = pstStatus->u32MeasureValue[1];
            stTestResult.u32TestNmbr = pstStatus->pstMeasureRoomItem->stItem.u32TestID;
            stTestResult.u8DataType = 10; // 测量光子
            stTestResult.u8CellNmbr = SystemConfig_GetCellNumber() + 1;
            stTestResult.u8ReagentPos = pstStatus->pstMeasureRoomItem->u8ReagentPos;
            stTestResult.u8TestType = 0;
            TestProc_SendDataToComm(BOARD_ID_TRACK_COMM, CAN_CMD_COMM_TESTDATA, (uint8_t *)&stTestResult, sizeof(TESTPROC_ITEM_RESULT_T));
            DebugMsg("MeasureValue2:%d,%d\r\n", stTestResult.u32TestNmbr, pstStatus->u32MeasureValue[1]);
            break;
        case TESTPROC_MSG_CMD_WASH_DISK_STOP:
            pstStatus->u8WashDiskHandCup = TestProc_WashDiskHandCup();
            pstStatus->eWashDiskState = TESTPROC_WASHDISK_STATE_IDLE;
            u8WashMask = 0;
            // First Wash
            u8CupID = TestProc_Wash12Cup();
            if (TESTPROC_WASH_CUP_IN == pstStatus->eWashCupState[u8CupID])
            {
                u8WashMask |= WASH_STOP_WORK_WASH12;
                pstStatus->eWashCupState[u8CupID] = TESTPROC_WASH_CUP_WASH12;
            }
            u8CupID = TestProc_Wash34Cup();
            if (TESTPROC_WASH_CUP_WASH12 == pstStatus->eWashCupState[u8CupID])
            {
                u8WashMask |= WASH_STOP_WORK_WASH34;
                pstStatus->eWashCupState[u8CupID] = TESTPROC_WASH_CUP_WASH34;
            }
            u8CupID = TestProc_Wash56Cup();
            if (TESTPROC_WASH_CUP_WASH34 == pstStatus->eWashCupState[u8CupID])
            {
                u8WashMask |= WASH_STOP_WORK_WASH56;
                pstStatus->eWashCupState[u8CupID] = TESTPROC_WASH_CUP_WASH56;
            }
            u8CupID = TestProc_WashLiquidACup();
            if (TESTPROC_WASH_CUP_WASH56 == pstStatus->eWashCupState[u8CupID])
            {
                pstStatus->eWashCupState[u8CupID] = TESTPROC_WASH_CUP_LIQUIDA;
                u8WashMask |= WASH_STOP_WORK_LIQUIDA;
            }
            u8CupID = TestProc_WashMixCup();
            if (TESTPROC_WASH_CUP_LIQUIDA == pstStatus->eWashCupState[u8CupID])
            {
                pstStatus->eWashCupState[u8CupID] = TESTPROC_WASH_CUP_MIX;
                u8WashMask |= WASH_STOP_WORK_MIX;
            }

            DebugMsg("Wash 1:%02X\r\n", u8WashMask);
            TestProc_WashStartWork(WASK_WORK_WHILE_STOP, &u8WashMask, 1);
            pstStatus->u8InterferenceWash |= TESTPROC_INTERFERENCE_WASH_NEEDLE;
            //DebugMsg("##WashStop:%02X##\r\n", u8WashMask);
            //for (int i=1; i<=10; i++)
            //{
            //    DebugMsg("%d,", pstStatus->eWashCupState[i]);
            //}
            //DebugMsg("\r\n");
            break;
        case TESTPROC_MSG_CMD_WASH_NEEDLE_FIRST_OK:
            DebugMsg("Wash1 OK\r\n");
            break;
        case TESTPROC_MSG_CMD_WASH_NEEDLE_SENOND_OK:
            DebugMsg("Wash2 OK\r\n");
            pstStatus->u8InterferenceWash &= ~TESTPROC_INTERFERENCE_WASH_NEEDLE;
            break;
        default:
            s32Result = TESTPROC_RESULT_ERR_CMD;
            break;
    }
    
    if (TESTPROC_RESULT_OK != s32Result)
    {
        DebugMsg("E>WashDiskMsg:%d,%04X,%d,%d\r\n", s32Result, pstMsg->eCommand, pstMsg->u16DataSize, pstStatus->eMeasureRoomState);
    }
    return s32Result;
}

static int32_t TestProc_MsgReagentDisk(TESTPROC_STATUS_T *pstStatus, const TESTPROC_MSM_T *pstMsg)
{
    int32_t s32Result = TESTPROC_RESULT_OK;
    
    switch (pstMsg->eCommand)
    {
        case TESTPROC_MSG_CMD_RESET:
            DebugMsg("ReagentDiskResetResult:%d\r\n", pstMsg->u8Data[0]);
            if (OK == pstMsg->u8Data[0])
            {
                pstStatus->eResetState[TESTPROC_UNIT_REAGENTDISK] = TESTPROC_RESET_OK;
            }
            else
            {
                pstStatus->eResetState[TESTPROC_UNIT_REAGENTDISK] = TESTPROC_RESET_NG;
            }
            break;
        case TESTPROC_MSG_CMD_REAGENTDISK_INPOS:
            //DebugMsg("ReagentDiskInPos\r\n");
            if (TESTPROC_REAGENTISK_STATE_SHAKE_ING == pstStatus->eReagentDiskState)
            {
                DebugMsg("R_M_Shake_OK\r\n");
                pstStatus->eReagentDiskState = TESTPROC_REAGENTISK_STATE_IDLE;
            }
            else
            {
                pstStatus->eReagentDiskState = TESTPROC_REAGENTISK_STATE_POSITION;
            }
            break;
        default:
            s32Result = TESTPROC_RESULT_ERR_CMD;
            break;
    }
    
    if (TESTPROC_RESULT_OK != s32Result)
    {
        DebugMsg("E>ReagentDiskMsg:%d,%04X,%d\r\n", s32Result, pstMsg->eCommand, pstMsg->u16DataSize);
    }
    return s32Result;
}

static int32_t TestProc_MsgReagent(TESTPROC_STATUS_T *pstStatus, const TESTPROC_MSM_T *pstMsg)
{
    int32_t s32Result = TESTPROC_RESULT_OK;
    uint8_t u8TxBuf[4];
    uint8_t u8CupID = 0;
    
    switch (pstMsg->eCommand)
    {
        case TESTPROC_MSG_CMD_RESET:
            DebugMsg("ReagentResult:%d\r\n", pstMsg->u8Data[0]);
            if (OK == pstMsg->u8Data[0])
            {
                pstStatus->eResetState[TESTPROC_UNIT_REAGENT] = TESTPROC_RESET_OK;
                g_u8TestProc_ResetMask |= RESET_BIT_REAGENT;
            }
            else
            {
                pstStatus->eResetState[TESTPROC_UNIT_REAGENT] = TESTPROC_RESET_NG;
            }
            break;
        case TESTPROC_MSG_CMD_RS1_R1_PREPARE_OK:
            pstStatus->eReagentState = TESTPROC_REAGENT_RS1_R1_READY;
            break;
        case TESTPROC_MSG_CMD_RS1_R1_ABSORB_OK:
            pstStatus->eReagentState = TESTPROC_REAGENT_RS1_R1_ABSORB_OK;
            pstStatus->eReagentDiskState = TESTPROC_REAGENTISK_STATE_IDLE;
            break;
        case TESTPROC_MSG_CMD_RS1_R2_ABSORB_OK:
            pstStatus->eReagentState = TESTPROC_REAGENT_RS1_R2_ABSORB_OK;
            pstStatus->eReagentDiskState = TESTPROC_REAGENTISK_STATE_IDLE;
            break;
        case TESTPROC_MSG_CMD_RS1_R2_PREPARE_2STEP_OK:
            pstStatus->eReagentState = TESTPROC_REAGENT_RS1_R2_2STEP_READY;
            break;
        case TESTPROC_MSG_CMD_RS1_R2_ABSORB_2STEP_OK:
            pstStatus->eReagentState = TESTPROC_REAGENT_RS1_R2_ABSORB_OK;
            pstStatus->eReagentDiskState = TESTPROC_REAGENTISK_STATE_IDLE;
            break;
        case TESTPROC_MSG_CMD_RS1_INJECT_OVER:
            if (TESTPROC_REACT_STATE_31_STOP == pstStatus->eReactState)
            {
                u8CupID = TestProc_R123MCupID(pstStatus->u8H2CupNumber);
                DebugMsg(" RS1_InjectOver:%d,%d\r\n", u8CupID, pstStatus->eReactCupState[u8CupID]);
                if (TESTPROC_REACT_CUP_2STEP_CUP_IN == pstStatus->eReactCupState[u8CupID])
                {
                    pstStatus->eReactCupState[u8CupID] = TESTPROC_REACT_CUP_2STEP_SAMPLE;
                }
                else
                {
                    pstStatus->eReactCupState[u8CupID] = TESTPROC_REACT_CUP_R1;
                }
                pstStatus->u8InterferenceReact &= ~TESTPROC_INTERFERENCE_REACT_REAGENT;
                //pstStatus->eReagentDiskState = TESTPROC_REAGENTISK_STATE_IDLE;
            }
            else
            {
                DebugMsg(" E>+RS1_Inject:%d\r\n", pstStatus->eReactState);
                u8TxBuf[0] = pstStatus->eReactState;
                TestProc_ReportWarning(pstStatus, TESTPROC_WARNING_REAGENT_1_INJECT, u8TxBuf, 1);
            }
            break;
        case TESTPROC_MSG_CMD_RS1_INJECT_OK:
            pstStatus->eReagentState = TESTPROC_REAGENT_STATE_IDLE;
            DebugMsg(" RS1_InjectOk\r\n");
			break;
        case TESTPROC_MSG_CMD_RS2_R2_PREPARE_OK:
            pstStatus->eReagentState = TESTPROC_REAGENT_RS2_R2_READY_OK;
            break;
        case TESTPROC_MSG_CMD_RS2_R2_ABSORB_OK:
            pstStatus->eReagentState = TESTPROC_REAGENT_RS2_R2_ABSORB_OK;
            //pstStatus->eReagentDiskState = TESTPROC_REAGENTISK_STATE_IDLE;
            break;
        case TESTPROC_MSG_CMD_RS2_R3_PREPARE_OK:
            pstStatus->eReagentState = TESTPROC_REAGENT_RS2_R3_READY_OK;
            break;
        case TESTPROC_MSG_CMD_RS2_R3_ABSORB_OK:
            pstStatus->eReagentState = TESTPROC_REAGENT_RS2_R3_ABSORB_OK;
            //pstStatus->eReagentDiskState = TESTPROC_REAGENTISK_STATE_IDLE;
            break;
        case TESTPROC_MSG_CMD_RS2_M_PREPARE_OK:
            pstStatus->eReagentState = TESTPROC_REAGENT_RS2_M_READY_OK;
            break;
        case TESTPROC_MSG_CMD_RS2_M_ABSORB_OK:
            pstStatus->eReagentState = TESTPROC_REAGENT_RS2_M_ABSORB_OK;
            pstStatus->eReagentDiskState = TESTPROC_REAGENTISK_STATE_IDLE;
            DebugMsg(" +M Ok\r\n");
            break;
        case TESTPROC_MSG_CMD_RS2_INJECT_OVER:
            pstStatus->u8InterferenceReact &= ~TESTPROC_INTERFERENCE_REACT_REAGENT2;
            //pstStatus->eReagentDiskState = TESTPROC_REAGENTISK_STATE_IDLE;
            if (TESTPROC_REACT_STATE_09_STOP == pstStatus->eReactState)
            {
                u8CupID = TestProc_R123MCupID(pstStatus->u8H2CupNumber);
                pstStatus->eReactCupState[u8CupID] = TESTPROC_REACT_CUP_R2;
                //pstStatus->u8InterferenceReact &= ~TESTPROC_INTERFERENCE_REACT_REAGENT2;
                //pstStatus->eReagentDiskState = TESTPROC_REAGENTISK_STATE_IDLE;
                DebugMsg(" RS2_InjectOver:%d\r\n", u8CupID);
            }
            else
            {
                DebugMsg(" E>+RS2_Inject:%d\r\n", pstStatus->eReactState);
                u8TxBuf[0] = pstStatus->eReactState;
                TestProc_ReportWarning(pstStatus, TESTPROC_WARNING_REAGENT_2_INJECT, u8TxBuf, 1);
            }
            break;
        case TESTPROC_MSG_CMD_RS2_INJECT_OK:
            pstStatus->eReagentState = TESTPROC_REAGENT_STATE_IDLE;
            DebugMsg(" RS2_InjectOk\r\n");
            break;
        case TESTPROC_MSG_CMD_REAGENT_ON_REACT:
            if (TESTPROC_REAGENT_RS1_TO_REACT_ING == pstStatus->eReagentState)
            {
                pstStatus->eReagentState = TESTPROC_REAGENT_RS1_R2_ABSORB_OK;
            }
            else
            {
                DebugMsg("E>ReagentOnReact:%d\r\n", pstStatus->eReagentState);
            }
            break;
        default:
            s32Result = TESTPROC_RESULT_ERR_CMD;
            break;
    }
    
    if (TESTPROC_RESULT_OK != s32Result)
    {
        DebugMsg("E>ReagentMsg:%d,%04X,%d\r\n", s32Result, pstMsg->eCommand, pstMsg->u16DataSize);
    }
    return s32Result;
}

static int32_t TestProc_MsgSample(TESTPROC_STATUS_T *pstStatus, const TESTPROC_MSM_T *pstMsg)
{
    int32_t s32Result = TESTPROC_RESULT_OK;
    uint16_t u16RackID;
    uint8_t u8TxBuf[8];
    
    switch (pstMsg->eCommand)
    {
        case TESTPROC_MSG_CMD_RESET:
            DebugMsg("SampleResetResult:%d\r\n", pstMsg->u8Data[0]);
            if (OK == pstMsg->u8Data[0])
            {
                pstStatus->eResetState[TESTPROC_UNIT_SAMPLE] = TESTPROC_RESET_OK;
            }
            else
            {
                pstStatus->eResetState[TESTPROC_UNIT_SAMPLE] = TESTPROC_RESET_NG;
            }
            break;
        case TESTPROC_MSG_CMD_SAMPLENEEDLE_RESET:
            DebugMsg("SampleNeedleResetResult:%d\r\n", pstMsg->u8Data[0]);
            if (OK == pstMsg->u8Data[0])
            {
                g_u8TestProc_ResetMask |= RESET_BIT_SAMPLENEEDLE;
            }
            break;
        case TESTPROC_MSG_CMD_SAMPLE_ONRACK:
            pstStatus->eSampleState = TESTPROC_SAMPLE_STATE_ON_RACK;
            DebugMsg("SampleOnRack\r\n");
            break;
        case TESTPROC_MSG_CMD_SAMPLE_ABSORB_OK:
            pstStatus->u8InterferenceConveyorBelt &= ~TESTPROC_INTERFERENCE_CB_SAMPLE;
            if (TESTPROC_WORK_SIMULATE != pstStatus->stCellStatus.u8WorkStatus)
            {
                if (pstStatus->u32EmRackOutCount > 0)
                {
                    pstStatus->u32EmRackOutCount--;
                    if (0 == pstStatus->u32EmRackOutCount)
                    {
                        DebugMsg("EmRackOut:%d\r\n", pstStatus->u32EmRackOutCount);
                        TestProc_RackToOutPos(pstStatus);
                    }
                }
                else if (pstStatus->u32RackOutCount > 0)
                {
                    pstStatus->u32RackOutCount--;
                    if (0 == pstStatus->u32RackOutCount)
                    {
                        DebugMsg("RackOut:%d\r\n", pstStatus->u32EmRackOutCount);
                        TestProc_RackToOutPos(pstStatus);
                    }
                }
            }
            break;
        case TESTPROC_MSG_CMD_SAMPLE_INJECT_READY:
            pstStatus->eSampleState = TESTPROC_SAMPLE_STATE_ABSORB_OK;
            break;
        case TESTPROC_MSG_CMD_SAMPLE_INJECT_OK:
            pstStatus->eSampleState = TESTPROC_SAMPLE_STATE_INJECT_OK;
            pstStatus->eTestRackState = TESTPROC_TESTRACK_STATE_IN;
            pstStatus->u8InterferenceReact &= ~TESTPROC_INTERFERENCE_REACT_SAMPLE;
            DebugMsg(" S_InjectOK\r\n");
            break;
        case TESTPROC_MSG_CMD_SAMPLE_CLEAN_OK:
            pstStatus->eSampleState = TESTPROC_SAMPLE_STATE_IDLE;
            break;
        case TESTPROC_MSG_CMD_SAMPLE_RACK_INPOS:
            DebugMsg("RackInPos:%d\r\n", pstMsg->u8Data[0]);
            pstStatus->eConveyorBeltState = TESTPROC_SAMPLE_CB_STATE_READY;
            if (pstMsg->u8Data[0] <= TESTPROC_RACK_POS_MAX)
            {
                if (TESTPROC_SAMPLE_CB_STATE_RESET == pstStatus->eConveyorBeltState)
                {
                    pstStatus->eTestRackState = TESTPROC_TESTRACK_STATE_IDLE;
                    u8TxBuf[0] = 1;
                    TestProc_SendDataToComm(BOARD_ID_TRACK_COMM, CAN_CMD_CELL_CONVEYORBELT_RESET, u8TxBuf, 1);
                }
                else
                {
                    pstStatus->eTestRackState = TESTPROC_TESTRACK_STATE_POSITION;
                }
                pstStatus->u8InterferenceSample &= ~TESTPROC_INTERFERENCE_SAMPLE_CB;
            }
            else
            {
                if (TESTPROC_RACK_STATE_TOOUT == pstStatus->eEmRackState)
                {
                    u16RackID = (uint16_t)pstStatus->u32EmRackID;
                    pstStatus->u32EmRackID = TESTPROC_INVALID_RACK_ID;
                    pstStatus->eEmRackState = TESTPROC_RACK_STATE_IDLE;
                }
                else
                {
                    u16RackID = (uint16_t)pstStatus->u32RackID;
                    pstStatus->u32RackID = TESTPROC_INVALID_RACK_ID;
                    pstStatus->eRackState = TESTPROC_RACK_STATE_IDLE;
                }
                
                pstStatus->u32RecycleRackID = u16RackID;
                pstStatus->eRecycleStatus = TESTPROC_RECYCLE_STATUS_BUSY;
                pstStatus->u8InterferenceConveyorBelt |= TESTPROC_INTERFERENCE_CB_TRACK;
                u8TxBuf[0] = HWORD_TO_BYTE_L(u16RackID);
                u8TxBuf[1] = HWORD_TO_BYTE_H(u16RackID);
                TestProc_SendDataToComm(BOARD_ID_TRACK_COMM, CAN_CMD_COMM_RACKCHANGE, u8TxBuf, 2);
                DebugMsg("RackChange:%d\r\n", u16RackID);
            }
            break;
        default:
            s32Result = TESTPROC_RESULT_ERR_CMD;
            break;
    }
    
    if (TESTPROC_RESULT_OK != s32Result)
    {
        DebugMsg("E>SampleMsg:%d,%04X,%d\r\n", s32Result, pstMsg->eCommand, pstMsg->u16DataSize);
    }
    return s32Result;
}

static int32_t TestProc_MsgHandCup(TESTPROC_STATUS_T *pstStatus, const TESTPROC_MSM_T *pstMsg)
{
    //int i;
    int32_t s32Result = TESTPROC_RESULT_OK;
    uint8_t u8TxBuf[4];
    //OS_ERR err;
    //uint8_t  u8CupID;
    
    switch (pstMsg->eCommand)
    {
        case TESTPROC_MSG_CMD_RESET:
            DebugMsg("HandCupResetResult:%d\r\n", pstMsg->u8Data[0]);
            if (OK == pstMsg->u8Data[0])
            {
                pstStatus->eResetState[TESTPROC_UNIT_HANDCUP] = TESTPROC_RESET_OK;
                g_u8TestProc_ResetMask |= RESET_BIT_HANDCUP;
            }
            else
            {
                pstStatus->eResetState[TESTPROC_UNIT_HANDCUP] = TESTPROC_RESET_NG;
            }
            break;
        case TESTPROC_MSG_CMD_HANDCUP_NEWCUP:
            pstStatus->eHandCupBusy = TESTPROC_HANDCUP_STATE_PUTREACT;
            DebugMsg("CatchNewCupOk:%d,%d\r\n", pstStatus->u8CupStackCol, pstStatus->u8CupStackRow);
            if (pstStatus->u8CupStackCol >= TESTPROC_CUPSTACK_COLUMN_MAX && pstStatus->u8CupStackRow >= TESTPROC_CUPSTACK_ROW_MAX)
            {
                if (TestProc_Api_IsCupStackDoorClose() == 1)
                {
                    pstStatus->eCupStackState = TESTPROC_CUPSTACK_STATE_CHANGE;
                    TestProc_CupStackStartWork(CUPSTACK_WORK_CHANGE_DISK, NULL, 0);
                }
                else
                {
                    if (TESTPROC_WORK_TEST == pstStatus->stCellStatus.u8WorkStatus ||
                        TESTPROC_WORK_SIMULATE == pstStatus->stCellStatus.u8WorkStatus)
                    {
                        pstStatus->stCellStatus.u8WorkStatus = TESTPROC_WORK_SAMPLE_PAUSE;
                        u8TxBuf[0] = pstStatus->stCellStatus.u8WorkStatus;
                        u8TxBuf[1] = pstStatus->u8CupStackDoorClose;
                        u8TxBuf[2] = pstStatus->eCupStackState;
                        TestProc_ReportWarning(pstStatus, TESTPROC_WARNING_CUPSTACK_EMPTY, u8TxBuf, 3);
                    }
                    else
                    {
                        DebugMsg("N>CupStackEmpty,DoorNotClose:%d\r\n", pstStatus->eCupStackState);
                    }
                    pstStatus->u8CupStackDoorClose = 0;
                }
            }
            break;
        case TESTPROC_MSG_CMD_HANDCUP_CUPINREACT:
            pstStatus->eHandCupBusy = TESTPROC_HANDCUP_STATE_IDLE;
            pstStatus->u8InterferenceReact &= ~TESTPROC_INTERFERENCE_REACT_HANDCUP;
            pstStatus->s32StandbyCount = TESTPROC_COUNT_TESTTOSTANDBY;
            if (TESTPROC_REACT_STATE_23_STOP == pstStatus->eReactState)
            {
                if (NULL == pstStatus->pstReactCupItem[pstStatus->u8H2CupNumber] && 1 == pstMsg->u8Data[0])
                {
                    pstStatus->pstReactCupItem[pstStatus->u8H2CupNumber] = pstStatus->pstHoldItem;
                    pstStatus->eReactCupState[pstStatus->u8H2CupNumber] = TESTPROC_REACT_CUP_IN;
                    pstStatus->pstHoldItem = NULL;
                    DebugMsg("PutNewCupOk:%d,%d,%d,%d\r\n", pstStatus->u8H2CupNumber, pstStatus->pstReactCupItem[pstStatus->u8H2CupNumber]->stItem.u32TestID, pstStatus->pstReactCupItem[pstStatus->u8H2CupNumber]->stItem.u32RackID, pstStatus->pstReactCupItem[pstStatus->u8H2CupNumber]->stItem.u8SamplePos);
                }
                else
                {
                    DebugMsg("E>PutReactCup1:%d,%d\r\n", pstStatus->pstReactCupItem[pstStatus->u8H2CupNumber]->stItem.u32TestID, pstMsg->u8Data[0]);
                    if (1 == pstMsg->u8Data[0])
                    {
                        u8TxBuf[0] = pstStatus->u8H2CupNumber;
                        u8TxBuf[1] = pstStatus->eReactCupState[pstStatus->u8H2CupNumber];
                        *(uint16_t *)&u8TxBuf[2] = (uint16_t)((pstStatus->pstReactCupItem[pstStatus->u8H2CupNumber]->stItem.u32TestID >> 16) & 0xFFFF);
                        TestProc_ReportWarning(pstStatus, TESTPROC_WARNING_HANDCUP_INREACT_1, u8TxBuf, 4);
                    }
                    else
                    {
                        
                    }
                }
            }
            else
            {
                DebugMsg("E>PutReactCup2:%d\r\n", pstStatus->eReactState);
                u8TxBuf[0] = pstStatus->u8H2CupNumber;
                u8TxBuf[1] = pstStatus->eReactCupState[pstStatus->u8H2CupNumber];
                *(uint16_t *)&u8TxBuf[2] = (uint16_t)((pstStatus->pstReactCupItem[pstStatus->u8H2CupNumber]->stItem.u32TestID >> 16) & 0xFFFF);
                TestProc_ReportWarning(pstStatus, TESTPROC_WARNING_HANDCUP_INREACT_2, u8TxBuf, 4);
            }
            break;
        case TESTPROC_MSG_CMD_HANDCUP_ONREACT:
            pstStatus->eHandCupBusy = TESTPROC_HANDCUP_STATE_ON_REACTGORWASH;
            //DebugMsg("HandOnReactWichOutCup\r\n");
            break;
        case TESTPROC_MSG_CMD_HANDCUP_REACTCATCHOK:
            pstStatus->u8InterferenceReact &= ~TESTPROC_INTERFERENCE_REACT_HANDCUP;
            break;
        case TESTPROC_MSG_CMD_HANDCUP_ONWASH:
            pstStatus->eHandCupBusy = TESTPROC_HANDCUP_STATE_ONWASH;
            DebugMsg("HandOnWashWithCup\r\n");
            break;
        case TESTPROC_MSG_CMD_HANDCUP_INWASH:
            DebugMsg("CupInWash_%s:%d\r\n", 1 == pstMsg->u8Data[0] ? "OK" : "NG**", pstStatus->u8WashDiskHandCup);
            if (1 == pstMsg->u8Data[0])
            {
                pstStatus->eWashCupState[pstStatus->u8WashDiskHandCup] = TESTPROC_WASH_CUP_IN;
                pstStatus->u8InterferenceWash &= ~TESTPROC_INTERFERENCE_WASH_HAND;
                pstStatus->eHandCupBusy = TESTPROC_HANDCUP_STATE_IDLE;
            }
            else
            {
                // WARNING
            }
            break;
        case TESTPROC_MSG_CMD_HANDCUP_ONMEASUREROOM:
            pstStatus->eHandCupBusy = TESTPROC_HANDCUP_STATE_ON_MEASUREROOM;
            //DebugMsg("HandOn_MR\r\n");
            break;
        case TESTPROC_MSG_CMD_HANDCUP_INMEASUREROOM:
            pstStatus->eHandCupBusy = TESTPROC_HANDCUP_STATE_IDLE;
            pstStatus->eMeasureRoomState = TESTPROC_MEASUREROOM_STATE_CUPIN;
            pstStatus->pstMeasureRoomItem = pstStatus->pstWashCupItem[pstStatus->u8WashDiskHandCup];
            DebugMsg("CupPutIn_MR:%d,%d,%d\r\n", pstStatus->u8WashDiskHandCup, pstStatus->eWashDiskState, NULL == pstStatus->pstMeasureRoomItem ? 0 : pstStatus->pstMeasureRoomItem->stItem.u32TestID);
            pstStatus->pstWashCupItem[pstStatus->u8WashDiskHandCup] = NULL;
            pstStatus->eWashCupState[pstStatus->u8WashDiskHandCup] = TESTPROC_WASH_CUP_INIT;
            break;
        case TESTPROC_MSG_CMD_HANDCUP_ABANDON:
            pstStatus->eHandCupBusy = TESTPROC_HANDCUP_STATE_IDLE;
            DebugMsg("AbandonCupOk:%d,%d\r\n", TestProc_testDataCount(), TestProc_testDataEmItemCount(), pstStatus->stCellStatus.u8WorkStatus);
            break;
        case TESTPROC_MSG_CMD_HANDCUP_MEASUREROOMCATCHOK:
            DebugMsg("MR_CatchCup\r\n");
            TestProc_MeasureRoomStartWork(MEASUREROOM_WORK_CLOSE, NULL, 0);
            break;
        case TESTPROC_MSG_CMD_HANDCUP_WASHCUPONREACT:
            pstStatus->eHandCupBusy = TESTPROC_HANDCUP_STATE_WASH_ON_REACT;
            break;
        case TESTPROC_MSG_CMD_HANDCUP_WASHCUPINREACT:
            pstStatus->u8InterferenceReact &= ~TESTPROC_INTERFERENCE_REACT_WASH_HANDCUP;
            pstStatus->eHandCupBusy = TESTPROC_HANDCUP_STATE_IDLE;
            pstStatus->s32StandbyCount = TESTPROC_COUNT_TESTTOSTANDBY;
            if (TESTPROC_REACT_STATE_23_STOP == pstStatus->eReactState && TESTPROC_WASHDISK_STATE_IDLE == pstStatus->eWashDiskState)
            {
                if (NULL == pstStatus->pstReactCupItem[pstStatus->u8H2CupNumber] && 1 == pstMsg->u8Data[0])
                {
                    if (pstStatus->pstWashCupItem[pstStatus->u8WashDiskHandCup] != NULL)
                    {
                        pstStatus->pstReactCupItem[pstStatus->u8H2CupNumber] = pstStatus->pstWashCupItem[pstStatus->u8WashDiskHandCup];
                        pstStatus->pstWashCupItem[pstStatus->u8WashDiskHandCup] = NULL;
                        pstStatus->eWashCupState[pstStatus->u8WashDiskHandCup] = TESTPROC_WASH_CUP_INIT;
                        pstStatus->eReactCupState[pstStatus->u8H2CupNumber] = TESTPROC_REACT_CUP_2STEP_CUP_IN;
                        DebugMsg("WashPutReactOk:%d>%d,%d,%d,%d\r\n", pstStatus->u8WashDiskHandCup, pstStatus->u8H2CupNumber, pstStatus->pstReactCupItem[pstStatus->u8H2CupNumber]->stItem.u32TestID, pstStatus->pstReactCupItem[pstStatus->u8H2CupNumber]->stItem.u32RackID, pstStatus->pstReactCupItem[pstStatus->u8H2CupNumber]->stItem.u8SamplePos);
                    }
                    else
                    {
                        DebugMsg("E>WashPutReact1:%d\r\n", pstStatus->u8WashDiskHandCup);
                        //u8TxBuf[0] = pstStatus->u8H2CupNumber;
                        //u8TxBuf[1] = pstStatus->eReactCupState[pstStatus->u8H2CupNumber];
                        //*(uint16_t *)&u8TxBuf[2] = (uint16_t)((pstStatus->pstReactCupItem[pstStatus->u8H2CupNumber]->stItem.u32TestID >> 16) & 0xFFFF);
                        //TestProc_ReportWarning(pstStatus, TESTPROC_WARNING_HANDCUP_INREACT_1, u8TxBuf, 4);
                    }
                }
                else
                {
                    DebugMsg("E>WashPutReact2:%d,%d,%d\r\n", pstStatus->u8H2CupNumber, pstStatus->pstReactCupItem[pstStatus->u8H2CupNumber]->stItem.u32TestID, pstMsg->u8Data[0]);
                    //u8TxBuf[0] = pstStatus->u8H2CupNumber;
                    //u8TxBuf[1] = pstStatus->eReactCupState[pstStatus->u8H2CupNumber];
                    //*(uint16_t *)&u8TxBuf[2] = (uint16_t)((pstStatus->pstReactCupItem[pstStatus->u8H2CupNumber]->stItem.u32TestID >> 16) & 0xFFFF);
                    //TestProc_ReportWarning(pstStatus, TESTPROC_WARNING_HANDCUP_INREACT_1, u8TxBuf, 4);
                }
            }
            else
            {
                DebugMsg("E>WashPutReact3:%d,%d\r\n", pstStatus->eReactState, pstStatus->eWashDiskState);
                //u8TxBuf[0] = pstStatus->u8H2CupNumber;
                //u8TxBuf[1] = pstStatus->eReactCupState[pstStatus->u8H2CupNumber];
                //*(uint16_t *)&u8TxBuf[2] = (uint16_t)((pstStatus->pstReactCupItem[pstStatus->u8H2CupNumber]->stItem.u32TestID >> 16) & 0xFFFF);
                //TestProc_ReportWarning(pstStatus, TESTPROC_WARNING_HANDCUP_INREACT_2, u8TxBuf, 4);
            }
            break;
        case TESTPROC_MSG_CMD_HANDCUP_ON_CUPSTACK:
            s32Result = CanSendDataToCell(TESTPROC_CELL_PLATFORM_NMBR, BOARD_ID_TRACK_COMM, CAN_CMD_COMM_CUPSTACK_POS, &pstMsg->u8Data[0], 2);
            break;
        default:
            s32Result = TESTPROC_RESULT_ERR_CMD;
            break;
    }
    
    if (TESTPROC_RESULT_OK != s32Result)
    {
        DebugMsg("E>HandCupMsg:%d,%04X,%d\r\n", s32Result, pstMsg->eCommand, pstMsg->u16DataSize);
    }
    return s32Result;
}

static int32_t TestProc_MsgCupStack(TESTPROC_STATUS_T *pstStatus, const TESTPROC_MSM_T *pstMsg)
{
    int32_t s32Result = TESTPROC_RESULT_OK;
    
    switch (pstMsg->eCommand)
    {
        case TESTPROC_MSG_CMD_RESET:
            DebugMsg("CupStackResetResult:%d\r\n", pstMsg->u8Data[0]);
            if (OK == pstMsg->u8Data[0])
            {
                pstStatus->eResetState[TESTPROC_UNIT_CUPSTACK] = TESTPROC_RESET_OK;
            }
            else
            {
                pstStatus->eResetState[TESTPROC_UNIT_CUPSTACK] = TESTPROC_RESET_NG;
            }
            break;
        case TESTPROC_MSG_CMD_CUPSTACK_CHANGE_OK:
            pstStatus->u8CupStackRow = 0;
            pstStatus->u8CupStackCol = 0;
            DebugMsg("CupStackChangeOK\r\n");
        case TESTPROC_MSG_CMD_CUPSTACK_POSTION:
            pstStatus->eCupStackState = TESTPROC_CUPSTACK_STATE_READY;
            DebugMsg("NewCupStackOK\r\n");
            break;
        default:
            s32Result = TESTPROC_RESULT_ERR_CMD;
            break;
    }
    
    if (TESTPROC_RESULT_OK != s32Result)
    {
        DebugMsg("E>CupStackMsg:%d,%04X,%d\r\n", s32Result, pstMsg->eCommand, pstMsg->u16DataSize);
    }
    return s32Result;
}

/****************************************************************************
*
* Function Name:    TestProc_MsgEvent
* Input:            pstStatus - 系统状态
*                   pstMsg - 事件消息
*                   
* Output:           None
*                   
* Returns:          
* Description:      
* Note:             
***************************************************************************/
static int32_t TestProc_MsgEvent(TESTPROC_STATUS_T *pstStatus, const TESTPROC_MSM_T *pstMsg)
{
    int32_t s32Result = TESTPROC_RESULT_OK;
    const TESTPROC_MSG_CMD_E Event = pstMsg->eCommand;
    TESTPROC_WARNING_E eWarning;
    uint8_t  u8TxBuf[8];

    switch(Event)
    {
        case TESTPROC_MSG_CMD_TIMEOUT:
            eWarning = (TESTPROC_WARNING_E)(*(uint16_t *)&pstMsg->u8Data[0]);
            u8TxBuf[0] = pstMsg->u8Data[2];
            u8TxBuf[1] = pstMsg->u8Data[3];
            s32Result = TestProc_ReportWarning(pstStatus, eWarning, u8TxBuf, 2);
            break;
        case TESTPROC_MSG_CMD_EXEC_NG:
            eWarning = (TESTPROC_WARNING_E)(*(uint16_t *)&pstMsg->u8Data[0]);
            u8TxBuf[0] = pstMsg->u8Data[4];
            u8TxBuf[1] = pstMsg->u8Data[5];
            u8TxBuf[2] = pstMsg->u8Data[6];
            u8TxBuf[3] = pstMsg->u8Data[7];
            s32Result = TestProc_ReportWarning(pstStatus, eWarning, u8TxBuf, 4);
            break;
        case TESTPROC_MSG_CMD_EVENT_CUPSTACK_KEY_DOWN:
            DebugMsg("Cup Stack Key Down\r\n");
            if (FSM_CUPSTACKEVENT_IDLE == FSM_CupStackEvent)
            {
                FSM_CupStackEvent = FSM_CUPSTACKEVENT_DEMAGNETIZE;
            }
            else
            {
                s32Result = TESTPROC_RESULT_ERR_STATUS;
            }
            break;
        case TESTPROC_MSG_CMD_EVENT_REAGENTDISK_KEY_DOWN:
            DebugMsg("Reagent Disk Key Down\r\n");
            if (TESTPROC_WORK_STANDBY == pstStatus->stCellStatus.u8WorkStatus || TESTPROC_WORK_SAMPLE_PAUSE == pstStatus->stCellStatus.u8WorkStatus)
            {
                if (FSM_REAGENTEVENT_IDLE == FSM_ReagentEvent)
                {
                    FSM_ReagentEvent = FSM_REAGENTEVENT_MOVE;
                }
                else
                {
                    s32Result = TESTPROC_RESULT_ERR_STATUS;
                }
            }
            else
            {
                DebugMsg("WARNING>ReagentKeyStatus:%d\r\n", pstStatus->stCellStatus.u8WorkStatus);
            }
            break;
        case TESTPROC_MSG_CMD_EVENT_CUPSTACK_DOOR_CLOSE:
            DebugMsg("Cup Stack Door Close\r\n");
            if (FSM_CUPSTACKEVENT_IDLE == FSM_CupStackEvent)
            {
                FSM_CupStackEvent = FSM_CUPSTACKEVENT_RESET;
            }
            else
            {
                s32Result = TESTPROC_RESULT_ERR_STATUS;
            }
            break;
        default:
            s32Result = TESTPROC_RESULT_ERR_CMD;
            break;
    }

    if (TESTPROC_RESULT_OK != s32Result)
    {
        DebugMsg("E>EventPro:%d,%04X,%d,%d,%d\r\n", s32Result, Event, pstMsg->u16DataSize, FSM_CupStackEvent, FSM_ReagentEvent);
    }

    return s32Result;
}

/****************************************************************************
*
* Function Name:    TestProc_MainFSM
* Input:            pstStatus - 系统状态
*                   
* Output:           None
*                   
* Returns:          
* Description:      
* Note:             
***************************************************************************/
static void TestProc_MainFSM(TESTPROC_STATUS_T *pstStatus)
{
    //int32_t i;
    //uint32_t u32Tmp;
    TESTPROC_RESET_STATE Result = TESTPROC_RESET_OK;
    uint8_t u8TxBuf[8];
    OS_ERR err;
    //uint8_t u8ResetCount = 0;
    //uint8_t u8ResetNG = 0;
    
    switch (FSM_TestProcess)
    {
        case FSM_TESTPROCESS_IDLE:
            
            break;
        case FSM_TESTPROCESS_POWERINIT:
            TestProc_PowerOnFSM(pstStatus);
            if ((g_SystemPowerOnTimer != TESTPROC_TICK_INVALID) && ((OSTimeGet(&err) - g_SystemPowerOnTimer) > TIMEOUT_POWERON_FOR_SYSTEM))
            {
                // Warning
                TestProc_Standby(pstStatus);
            }
            break;
        case FSM_TESTPROCESS_RESET:
            Result = TestProc_ResetCheckResult(pstStatus);
            if (TESTPROC_RESET_OK == Result || TESTPROC_RESET_NG == Result)
            {
                FSM_TestProcess = FSM_TESTPROCESS_IDLE;
                if (TESTPROC_RESET_OK == Result)
                {
                    //TestProc_Standby(pstStatus);
                    TestProc_ResetInit(pstStatus);
                    pstStatus->stCellStatus.u8WorkStatus = TESTPROC_WORK_STANDBY;
                    u8TxBuf[0] = 1;
                    //TestProc_ReactStartWork(REACT_WORK_STOP, NULL, 0);
                }
                else
                {
                    pstStatus->stCellStatus.u8WorkStatus = TESTPROC_WORK_RESET_NG;
                    u8TxBuf[0] = 0;
                }
                TestProc_SendDataToComm(BOARD_ID_TRACK_COMM, CAN_CMD_CELL_RESET, u8TxBuf, 1);
            }
            break;
        case FSM_TESTPROCESS_TEST:
        case FSM_TESTPROCESS_SIMULATE:
            TestProc_TestFlow(pstStatus);
            break;
        case FSM_TESTPROCESS_ABANDON_CUP:
            TestProc_AbandonCupFSM(pstStatus);
            break;
        case FSM_TESTPROCESS_RECYCLE_RACK:
            TestProc_RecycleRackFSM(pstStatus);
            break;
        case FSM_TESTPROCESS_REAGENTBARCODE:
            TestProc_ReagentBarCodeFSM(pstStatus);
            break;
        case FSM_TESTPROCESS_REAGENTQUANTITY:
            TestProc_ReagentQuantityFSM(pstStatus);
            break;
        case FSM_TESTPROCESS_M_SEPARATE_AUTO:
            TestProc_MSeparateAutoFSM(pstStatus);
            break;
        case FSM_TESTPROCESS_M_SEPARATE:
            TestProc_MSeparateFSM(pstStatus);
            break;
        case FSM_TESTPROCESS_MANUAL_TEST:
            TestProc_ManualTestFSM(pstStatus);
            break;
        case FSM_TESTPROCESS_SAMPLE_ACCURACY:
            TestProc_SampleAccuracyFSM(pstStatus);
            break;
        case FSM_TESTPROCESS_REAGENT_ACCURACY:
            TestProc_ReagentAccuracyFSM(pstStatus);
            break;
        case FSM_TESTPROCESS_WASH_ACCURACY:
            TestProc_WashAccuracyFSM(pstStatus);
            break;
        case FSM_TESTPROCESS_DARK_TEST:
            TestProc_DarkTestFSM(pstStatus);
            break;
        case FSM_TESTPROCESS_NEEDLE_CLEAN:
            TestProc_NeedleCleanFSM(pstStatus);
            break;
        case FSM_TESTPROCESS_LIQUID_PERFUSE:
            TestProc_LiquidPerfuseFSM(pstStatus);
            break;
        default:
            FSM_TestProcess = FSM_TESTPROCESS_IDLE;
            break;
    }
    
    TestProc_ReagentEventFSM(pstStatus);
    TestProc_CupStackEventFSM(pstStatus);
}

static int32_t TestProc_TestFlow(TESTPROC_STATUS_T *pstStatus)
{
    int s32Result = TESTPROC_RESULT_OK;
    //TESTPROC_ITEM_T *pstItem;
    uint8_t u8TxBuf[4];
    //uint16_t u16ReagentPos;
    //uint16_t u16RackID;
    uint8_t u8CupID = 0;
    
    if (NULL == pstStatus)
    {
        return TESTPROC_RESULT_ERR_PARAM;
    }
        
    if (TESTPROC_REACT_STATE_23_RUN == pstStatus->eReactState || TESTPROC_REACT_STATE_23_STOP == pstStatus->eReactState)
    {        
        if (TESTPROC_REACT_STATE_23_RUN == pstStatus->eReactState)
        {
            u8CupID = TestProc_H2CupID(REACT_TRIGGER_1_STEP);
        }
        else
        {            
            u8CupID = pstStatus->u8H2CupNumber;
        }
        
        if (TESTPROC_WASHDISK_STATE_IDLE == pstStatus->eWashDiskState)
        {
            uint8_t u8WashToReact = FALSE;
            if (pstStatus->pstWashCupItem[pstStatus->u8WashDiskHandCup] != NULL && TESTPROC_TWO_STEP_ENABLE == pstStatus->pstWashCupItem[pstStatus->u8WashDiskHandCup]->eTwoStepState)
            {
                if (TESTPROC_HANDCUP_STATE_IDLE == pstStatus->eHandCupBusy)
                {
                    u8WashToReact = TRUE;
                    pstStatus->pstWashCupItem[pstStatus->u8WashDiskHandCup]->eTwoStepState = TESTPROC_TWO_STEP_DISABLE;
                    pstStatus->eHandCupBusy = TESTPROC_HANDCUP_STATE_WASH_TO_REACT;
                    TestProc_HandCupStartWork(HANDCUP_WORK_CUP_WASH_TO_REACT, NULL, 0);
                    DebugMsg("WashCupToReact:%d>%d,%d,%d\r\n", pstStatus->u8WashDiskHandCup, u8CupID, pstStatus->pstWashCupItem[pstStatus->u8WashDiskHandCup]->stItem.u32TestID,
                        pstStatus->pstReactCupItem[u8CupID] == NULL ? 0 : pstStatus->pstReactCupItem[u8CupID]->stItem.u32TestID);
                }
            }
            
            // 新样本杯处理
            if (FALSE == u8WashToReact)
            {
                //DebugMsg("23Run:%d,%p,%d,%p\r\n", u8CupID, pstStatus->pstReactCupItem[u8CupID], pstStatus->eHandCupBusy, pstStatus->pstHoldItem);
                if (NULL == pstStatus->pstHoldItem && NULL == pstStatus->pstReactCupItem[u8CupID])
                {
                    if (pstStatus->stCellStatus.u8WorkStatus != TESTPROC_WORK_SAMPLE_STOP &&        // 加样停止
                        pstStatus->stCellStatus.u8WorkStatus != TESTPROC_WORK_SAMPLE_PAUSE &&       // 加样暂停
                        pstStatus->stCellStatus.u8WorkStatus != TESTPROC_WORK_WAIT_SAMPLE_PAUSE &&  // 加样暂停，等待试剂完成
                        FSM_CupStackEvent != FSM_CUPSTACKEVENT_PUSH_ING &&                          // 推杯盘的时候禁止抓新杯
                        TESTPROC_HANDCUP_STATE_IDLE == pstStatus->eHandCupBusy)
                    {
                        if (TESTPROC_CUPSTACK_STATE_READY == pstStatus->eCupStackState)
                        {
                            pstStatus->pstHoldItem = TestProc_GetOneItem(pstStatus);
                            if (pstStatus->pstHoldItem != NULL)
                            {
                                u8TxBuf[0] = pstStatus->u8CupStackRow;
                                u8TxBuf[1] = pstStatus->u8CupStackCol;
                                pstStatus->u8CupStackCol++;
                                if (pstStatus->u8CupStackCol >= TESTPROC_CUPSTACK_COLUMN_MAX)
                                {
                                    pstStatus->u8CupStackRow++;
                                    if (pstStatus->u8CupStackRow >= TESTPROC_CUPSTACK_ROW_MAX)
                                    {
                                        if (pstStatus->stCellStatus.u8WorkStatus != TESTPROC_WORK_TEST)
                                        {
                                            pstStatus->u8CupStackRow = 0;
                                            pstStatus->u8CupStackCol = 0;
                                        }
                                        DebugMsg("WARNING>CupStackNeedChange\r\n");
                                    }
                                    else
                                    {
                                        pstStatus->u8CupStackCol = 0;
                                    }
                                }
                                TestProc_HandCupStartWork(HANDCUP_WORK_CATCH_NEWCUP, u8TxBuf, 2);
                                pstStatus->eHandCupBusy = TESTPROC_HANDCUP_STATE_CATCH_NEWCUPING;
                                pstStatus->eReactCupState[u8CupID] = TESTPROC_REACT_CUP_IN_ING;
                                DebugMsg("PrepareNewCup:%d,%d,%d,%d\r\n", u8CupID, NULL == pstStatus->pstHoldItem ? 0 : pstStatus->pstHoldItem->stItem.u32TestID, pstStatus->pstHoldItem->stItem.u32RackID, pstStatus->pstHoldItem->stItem.u8SamplePos);
                            }
                            else
                            {
                                //DebugMsg("NoItem:%d,%p,%d,%p\r\n", u8CupID, pstStatus->pstReactCupItem[u8CupID], pstStatus->eHandCupBusy, pstStatus->pstHoldItem);
                            }
                        }
                        else
                        {
                            //DebugMsg("WARNING>CatchNewCup:%d\r\n", pstStatus->eCupStackState);
                            //u8TxBuf[0] = pstStatus->eCupStackState;
                            //u8TxBuf[1] = pstStatus->u8CupStackRow;
                            //u8TxBuf[2] = pstStatus->u8CupStackCol;
                            //TestProc_ReportWarning(pstStatus, TESTPROC_WARNING_CUPSTACK_STATUS, u8TxBuf, 3);
                        }
                    }
                }
            }
        }
        
        if (TESTPROC_REACT_STATE_23_STOP == pstStatus->eReactState)
        {
            // 反应盘反应杯处理
            //DebugMsg("23Stop:%d,%p,%d\r\n", u8CupID, pstStatus->pstReactCupItem[u8CupID], pstStatus->eHandCupBusy);
            if (TESTPROC_HANDCUP_STATE_PUTREACT == pstStatus->eHandCupBusy)
            {
                if (NULL == pstStatus->pstReactCupItem[u8CupID])
                {
                    pstStatus->u8InterferenceReact |= TESTPROC_INTERFERENCE_REACT_HANDCUP;
                    pstStatus->eHandCupBusy = TESTPROC_HANDCUP_STATE_PUTNEWCUPING;
                    TestProc_HandCupStartWork(HANDCUP_WORK_PUT_REACT, NULL, 0);
                }
                else
                {
                    DebugMsg(" ->%d,%d\r\n", u8CupID, pstStatus->pstReactCupItem[u8CupID]->stItem.u32TestID);
                }
            }
            else if (TESTPROC_HANDCUP_STATE_WASH_ON_REACT == pstStatus->eHandCupBusy)
            {
                if (NULL == pstStatus->pstReactCupItem[u8CupID])
                {
                    pstStatus->u8InterferenceReact |= TESTPROC_INTERFERENCE_REACT_WASH_HANDCUP;
                    pstStatus->eHandCupBusy = TESTPROC_HANDCUP_STATE_WASH_PUT_REACT;
                    TestProc_HandCupStartWork(HANDCUP_WORK_CUP_WASH_PUT_REACT, NULL, 0);
                }
                else
                {
                    DebugMsg(" +>%d,%d\r\n", u8CupID, pstStatus->pstReactCupItem[u8CupID]->stItem.u32TestID);
                }
            }
            //
            u8CupID = TestProc_H2CupID(REACT_TRIGGER_2_STEP);
            TestProc_MoveReactCupToWash(pstStatus, u8CupID);
        }
        else
        {            
            u8CupID = TestProc_RackMoveCup(TestProc_H2CupID(REACT_TRIGGER_1_STEP));
            //DebugMsg(" +>> %d,%d,%d,%02X,%d,%d,%d,%d\r\n", u8CupID, pstStatus->eReactCupState[u8CupID], pstStatus->eTestRackState, pstStatus->u8InterferenceConveyorBelt,
            //    pstStatus->u32RackID, pstStatus->pstReactCupItem[u8CupID]->stItem.u32TestID, pstStatus->pstReactCupItem[u8CupID]->stItem.u8SamplePos, pstStatus->eConveyorBeltState);
            // 样本架移动
            if (TESTPROC_TESTRACK_STATE_IN == pstStatus->eTestRackState)
            {
                if ((TESTPROC_REACT_CUP_IN == pstStatus->eReactCupState[u8CupID]) || (TESTPROC_REACT_CUP_R1 == pstStatus->eReactCupState[u8CupID]))
                {
                    if (0 == pstStatus->u8InterferenceConveyorBelt)
                    {
                        //if (TESTPROC_INVALID_RACK_ID != pstStatus->u32RackID)
                        {
                            if (pstStatus->pstReactCupItem[u8CupID]->stItem.u8SamplePos <= TESTPROC_RACK_POS_MAX)
                            {
                                if (TESTPROC_SAMPLE_CB_STATE_IDLE == pstStatus->eConveyorBeltState || TESTPROC_SAMPLE_CB_STATE_READY == pstStatus->eConveyorBeltState)
                                {
                                    pstStatus->u8InterferenceSample |= TESTPROC_INTERFERENCE_SAMPLE_CB;
                                    pstStatus->eTestRackState = TESTPROC_TESTRACK_STATE_RUNNING;
                                    pstStatus->eConveyorBeltState = TESTPROC_SAMPLE_CB_STATE_MOVING;
                                    u8TxBuf[0] = pstStatus->pstReactCupItem[u8CupID]->stItem.u8SamplePos;
                                    TestProc_ConveyorBeltStartWork(CONVEYORBELT_WORK_MOVETOPOS, u8TxBuf, 1);
                                    //DebugMsg(">$$RackMoveTo:%d,%d,%d\r\n", pstStatus->pstReactCupItem[u8CupID]->stItem.u32TestID, pstStatus->pstReactCupItem[u8CupID]->stItem.u32RackID, u8TxBuf[0]);
                                }
                            }
                            //else
                            //{
                            //    DebugMsg("E>SamplePos:%\r\n", pstStatus->pstReactCupItem[u8CupID]->stItem.u8SamplePos);
                            //}
                        }
                    }
                }
            }
            else if (TESTPROC_TESTRACK_STATE_IDLE == pstStatus->eTestRackState)
            {
                if (TESTPROC_WORK_SIMULATE == pstStatus->stCellStatus.u8WorkStatus)
                {
                    //pstStatus->eConveyorBeltState = TESTPROC_SAMPLE_CB_STATE_READY;
                    pstStatus->eTestRackState = TESTPROC_TESTRACK_STATE_POSITION;
                }
            }
        }
        // +Reagent Step2
        if (TESTPROC_REACT_STATE_23_RUN == pstStatus->eReactState)
        {
            u8CupID = TestProc_Step3R2ByStep1CupID(TestProc_H2CupID(REACT_TRIGGER_1_STEP));
        }
        else
        {
            u8CupID = TestProc_Step3R2ByStep1CupID(pstStatus->u8H2CupNumber);
        }
        TestProc_AddReagentStep2(pstStatus, u8CupID);
    }
    
    if (TESTPROC_REACT_STATE_02_RUN == pstStatus->eReactState || TESTPROC_REACT_STATE_02_STOP == pstStatus->eReactState)
    {
        if (TESTPROC_REACT_STATE_02_RUN == pstStatus->eReactState)
        {
            u8CupID = TestProc_H2CupID(REACT_TRIGGER_2_STEP);
        }
        else
        {
            u8CupID = pstStatus->u8H2CupNumber;
        }
        // 反应杯从反应盘到清洗盘
        // Step1 机械手移动到反应盘上方
        //DebugMsg("$$HandReactToWash:%d,%d,%p,%d\r\n", u8CupID, pstStatus->eReactCupState[u8CupID], pstStatus->pstReactCupItem[u8CupID], pstStatus->eHandCupBusy);
        // +Reagent Step2
        if (TESTPROC_REACT_STATE_02_RUN == pstStatus->eReactState)
        {
            u8CupID = TestProc_Step3R2ByStep2CupID(TestProc_H2CupID(REACT_TRIGGER_2_STEP));
        }
        else
        {
            u8CupID = TestProc_Step3R2ByStep2CupID(pstStatus->u8H2CupNumber);
        }
        TestProc_AddReagentStep2(pstStatus, u8CupID);
        // 
        if (TESTPROC_REACT_STATE_02_STOP == pstStatus->eReactState)
        {
            u8CupID = TestProc_Step4R1ByStep2CupID(pstStatus->u8H2CupNumber);
            //DebugMsg("TryMoveRD_02:%d,%d,%d,%p\r\n", u8CupID, pstStatus->eReagentState, pstStatus->eReagentDiskState, pstStatus->pstReactCupItem[u8CupID]);
            if (pstStatus->pstReactCupItem[u8CupID] != NULL)
            {
                if ((TESTPROC_REAGENT_STATE_IDLE == pstStatus->eReagentState || pstStatus->eReagentState >= TESTPROC_REAGENT_RS2_M_ABSORB_OK) && 
                    TESTPROC_REAGENTISK_STATE_IDLE == pstStatus->eReagentDiskState)
                {
                    TestProc_MoveReagentDiskPos(pstStatus, u8CupID, TESTPROC_REAGENTDISK_OFFSET_R1);
                }
            }
            else
            {
                // 清洗盘磁珠摇匀
                if (TESTPROC_REAGENTISK_STATE_IDLE == pstStatus->eReagentDiskState && 
                    (pstStatus->eReagentState == TESTPROC_REAGENT_STATE_IDLE || pstStatus->eReagentState >= TESTPROC_REAGENT_RS2_M_ABSORB_OK) &&
                    g_u8ReagentDiskShakeEnable == 1)
                {
                    DebugMsg("RD_Shake02:%d\r\n", u8CupID);
                    TestProc_ReagentDiskShake(pstStatus);
                }
            }
            // STEP2 反应杯从反应盘去清洗
            TestProc_MoveReactCupToWash(pstStatus, pstStatus->u8H2CupNumber);
        }
    }
    
    if (TESTPROC_REACT_STATE_09_RUN == pstStatus->eReactState || TESTPROC_REACT_STATE_09_STOP == pstStatus->eReactState)
    {
        //Reagent Step2
        if (TESTPROC_REACT_STATE_09_RUN == pstStatus->eReactState)
        {
            u8CupID = TestProc_R123MCupID(TestProc_H2CupID(REACT_TRIGGER_3_STEP));
        }
        else
        {
            u8CupID = TestProc_R123MCupID(pstStatus->u8H2CupNumber);
        }
        TestProc_AddReagentStep2(pstStatus, u8CupID);
        //Reagent Step1
        if (TESTPROC_REACT_STATE_09_RUN == pstStatus->eReactState)
        {
            u8CupID = TestProc_Step4R2ByStep3CupID(TestProc_H2CupID(REACT_TRIGGER_3_STEP));
        }
        else
        {
            u8CupID = TestProc_Step4R2ByStep3CupID(pstStatus->u8H2CupNumber);
        }
        TestProc_AddReagentStep1(pstStatus, u8CupID);
        // Sample
        if (TESTPROC_REACT_STATE_09_RUN == pstStatus->eReactState)
        {
            u8CupID = TestProc_SampleCupID(TestProc_H2CupID(REACT_TRIGGER_3_STEP));
        }
        else
        {
            u8CupID = TestProc_SampleCupID(pstStatus->u8H2CupNumber);
        }
        
        if (TESTPROC_REACT_CUP_R1 == pstStatus->eReactCupState[u8CupID])
        {
            //DebugMsg(" S>> %d,%d\r\n", pstStatus->eSampleState, pstStatus->eTestRackState);
            if (TESTPROC_SAMPLE_STATE_IDLE == pstStatus->eSampleState)
            {
                // 转样本针
                TestProc_SampleStartWork(SAMPLE_WORK_TO_RACK, NULL, 0);
                pstStatus->eSampleState = TESTPROC_SAMPLE_STATE_TO_RACK_ING;
                DebugMsg(" PrepareSample:%d,%d,%d\r\n", u8CupID, pstStatus->eTestRackState, NULL == pstStatus->pstReactCupItem[u8CupID] ? 0 : pstStatus->pstReactCupItem[u8CupID]->stItem.u32TestID);
            }
            else if (TESTPROC_SAMPLE_STATE_ON_RACK == pstStatus->eSampleState)
            {
                //DebugMsg(" =>> %d,%d\r\n", u8CupID, pstStatus->eTestRackState);
                if (TESTPROC_TESTRACK_STATE_POSITION == pstStatus->eTestRackState)
                {
                    *(uint16_t *)(&u8TxBuf[0]) = pstStatus->pstReactCupItem[u8CupID]->stItem.u16SampleQuantity;
                    TestProc_SampleStartWork(SAMPLE_WORK_TO_ABSORB, u8TxBuf, 2);
                    pstStatus->eSampleState = TESTPROC_SAMPLE_STATE_ABSORB_ING;
                    pstStatus->u8InterferenceConveyorBelt |= TESTPROC_INTERFERENCE_CB_SAMPLE;
                    DebugMsg(" >>+S:%d,%d,%d,%d<<\r\n", u8CupID, pstStatus->pstReactCupItem[u8CupID]->stItem.u32TestID, pstStatus->pstReactCupItem[u8CupID]->stItem.u8SamplePos, *(uint16_t *)(&u8TxBuf[0]));
                }
            }
            else if (TESTPROC_SAMPLE_STATE_ABSORB_OK == pstStatus->eSampleState)
            {
                if (TESTPROC_REACT_STATE_09_STOP == pstStatus->eReactState)
                {
                    DebugMsg(" SampleInject:%d,%d\r\n", u8CupID, *(uint16_t *)(&u8TxBuf[0]));
                    pstStatus->eSampleState = TESTPROC_SAMPLE_STATE_INJECT_ING;
                    pstStatus->u8InterferenceReact |= TESTPROC_INTERFERENCE_REACT_SAMPLE;
                    *(uint16_t *)(&u8TxBuf[0]) = pstStatus->pstReactCupItem[u8CupID]->stItem.u16SampleQuantity;
                    TestProc_SampleStartWork(SAMPLE_WORK_TO_INJECT, u8TxBuf, 2);
                }
                else
                {
                    DebugMsg(" **SampleInject:%d\r\n", pstStatus->eReactState);
                }
            }
            else if (TESTPROC_SAMPLE_STATE_INJECT_OK == pstStatus->eSampleState)
            {
                if (1 == pstStatus->pstReactCupItem[u8CupID]->stItem.u8ForeceWash)
                {
                    TestProc_SampleStartWork(SAMPLE_WORK_FORCE_CLEAN, NULL, 0);
                }
                else
                {
                    TestProc_SampleStartWork(SAMPLE_WORK_CLEAN, NULL, 0);
                }
                pstStatus->eSampleState = TESTPROC_SAMPLE_STATE_WASH_ING;
                pstStatus->eReactCupState[u8CupID] = TESTPROC_REACT_CUP_SAMPLE;
            }
        }        
        TestProc_MoveMeasureRoomCup(pstStatus);
        TestProc_MoveWashCupToMeasureRoom(pstStatus);
        TestProc_MoveReactCupToWash(pstStatus, 0);
    }
    
    // 反应盘+R1
    if (TESTPROC_REACT_STATE_31_RUN == pstStatus->eReactState || TESTPROC_REACT_STATE_31_STOP == pstStatus->eReactState)
    {
        // Reagent 1
        if (TESTPROC_REACT_STATE_31_RUN == pstStatus->eReactState)
        {
            u8CupID = TestProc_R123MCupID(TestProc_H2CupID(REACT_TRIGGER_4_STEP));
        }
        else
        {
            u8CupID = TestProc_R123MCupID(pstStatus->u8H2CupNumber);            
        }
        //
        //if (pstStatus->u32Trigger4Count > 71 && pstStatus->u32Trigger4Count < 76) DebugMsg(">> %d,%d,%d,%d\r\n", u8CupID, pstStatus->eReagentState, pstStatus->eReagentDiskState, pstStatus->eReactCupState[u8CupID]);
        if (TESTPROC_REACT_CUP_IN == pstStatus->eReactCupState[u8CupID] || TESTPROC_REACT_CUP_2STEP_CUP_IN == pstStatus->eReactCupState[u8CupID])
        {
            TestProc_AddReagentStep1(pstStatus, u8CupID);
        }
        TestProc_MoveMeasureRoomCup(pstStatus);
        TestProc_MoveWashCupToMeasureRoom(pstStatus);

        if (TESTPROC_REACT_STATE_31_STOP == pstStatus->eReactState)
        {
            u8CupID = TestProc_Step3R2ByStep4CupID(pstStatus->u8H2CupNumber);
            //if (pstStatus->u32Trigger4Count > 30) DebugMsg("TryMoveRD_31:%d,%d,%d\r\n", u8CupID, pstStatus->eReagentState, pstStatus->eReagentDiskState);
            if (pstStatus->pstReactCupItem[u8CupID] != NULL)
            {
                if ((TESTPROC_REAGENT_STATE_IDLE == pstStatus->eReagentState || pstStatus->eReagentState >= TESTPROC_REAGENT_RS1_R2_ABSORB_OK) && 
                    TESTPROC_REAGENTISK_STATE_IDLE == pstStatus->eReagentDiskState)
                {
                    if (TestProc_testIsAddReagent2R3(pstStatus->pstReactCupItem[u8CupID]->stItem.u8AddStep) == 1)
                    {
                        TestProc_MoveReagentDiskPos(pstStatus, u8CupID, TESTPROC_REAGENTDISK_OFFSET_R3);
                    }
                    else if (TestProc_testIsAddReagent2R2(pstStatus->pstReactCupItem[u8CupID]->stItem.u8AddStep) == 1)
                    {
                        TestProc_MoveReagentDiskPos(pstStatus, u8CupID, TESTPROC_REAGENTDISK_OFFSET_R2);
                    }
                    else
                    {
                        TestProc_MoveReagentDiskPos(pstStatus, u8CupID, TESTPROC_REAGENTDISK_OFFSET_M);
                    }
                }
            }
            else
            {                
                // 清洗盘磁珠摇匀
                if (TESTPROC_REAGENTISK_STATE_IDLE == pstStatus->eReagentDiskState && 
                    (pstStatus->eReagentState >= TESTPROC_REAGENT_RS1_R2_ABSORB_OK || TESTPROC_REAGENT_STATE_IDLE == pstStatus->eReagentState) &&
                    1 == g_u8ReagentDiskShakeEnable)
                {
                    DebugMsg("RD_Shake31:%d\r\n", u8CupID);
                    TestProc_ReagentDiskShake(pstStatus);
                }
            }
        }
    }
    
    if (TESTPROC_RESULT_OK != s32Result)
    {
        DebugMsg("E>TestFlow:%d,%d,%d,%02X,%d\r\n", s32Result, pstStatus->eReactState, u8CupID, pstStatus->eMeasureRoomState, pstStatus->eHandCupBusy);
    }
    
    return 0;
}

/****************************************************************************
*
* Function Name:    app_testGetOneItem
* Input:            pstState - 模块的状态指针
*                   
* Output:           None
*                   
* Returns:          测试项
* Description:      获取一个测试项
* Note:             
***************************************************************************/
static void *TestProc_GetOneItem(TESTPROC_STATUS_T *pstStatus)
{
    if (TESTPROC_WORK_SIMULATE == pstStatus->stCellStatus.u8WorkStatus)
    {
        if (pstStatus->u32RackOutCount > 0)
        {
            DebugMsg(" --> %d\r\n", pstStatus->u32RackOutCount);
            pstStatus->u32RackOutCount--;
        }
        else // 延一个周期
        {
            if (TESTPROC_RECYCLE_TSC_CMD_STATE_IDLE == pstStatus->eRecycleStatus)
            {
                if (TESTPROC_INVALID_RACK_ID != pstStatus->u32RackID || TESTPROC_INVALID_RACK_ID != pstStatus->u32EmRackID)
                {
                    DebugMsg("SimRackRecycle:%d,%d,%d,%d,%d\r\n", pstStatus->u32RackID, pstStatus->u32EmRackID, TestProc_testDataCount(), pstStatus->eRackState, pstStatus->eEmRackState);
                    TestProc_RackToOutPos(pstStatus);
                }
            }
            else
            {
                DebugMsg("E>SimRecycleBusy:%d,%d\r\n", pstStatus->eRecycleStatus, pstStatus->u32RecycleRackID);
            }
        }
        
        if (pstStatus->u32SimulateCount > 5)
        {
            g_stSimulateItem.stItem.u16R1ID = (pstStatus->u32SimulateCount % 5) + 1;
            g_stSimulateItem.stItem.u16R2ID = (pstStatus->u32SimulateCount % 5) + 1;
            g_stSimulateItem.stItem.u16R3ID = (pstStatus->u32SimulateCount % 5) + 1;
            g_stSimulateItem.stItem.u16MID = (pstStatus->u32SimulateCount % 5) + 1;
            if (pstStatus->u32RackOutCount >= 2 && pstStatus->u32RackOutCount <= 6)
            {
                uint8_t u8Pos[5] = {5, 4, 3, 2, 1};
                g_stSimulateItem.stItem.u8SamplePos = u8Pos[pstStatus->u32RackOutCount - 2];
                g_stSimulateItem.stItem.u32RackID = pstStatus->u32RackID;
            }
            else
            {
                g_stSimulateItem.stItem.u8SamplePos = 0;
                g_stSimulateItem.stItem.u32RackID = 9999; //TESTPROC_INVALID_RACK_ID;
            }
            g_stSimulateItem.stItem.u32TestID++;
            TestProc_testDataPush(&g_stSimulateItem, 1);
            return TestProc_ItemPop();
        }
        else
        {
            return NULL;
        }
    }
    else
    {
        if (TESTPROC_RACK_STATE_TEST == pstStatus->eEmRackState)
        {
            return TestProc_EmItemPop();
        }
        else if (TESTPROC_RACK_STATE_TEST == pstStatus->eRackState)
        {
            if (pstStatus->u8GetItemCount > 0 && TestProc_testDataCount() < TESTPROC_GET_ITEM_LIMIT)
            {
                TestProc_GetRackItem(pstStatus, (uint16_t)pstStatus->u32RackID);
            }
            return TestProc_ItemPop();
        }
        else
        {
            return NULL;
        }
    }
}

/****************************************************************************
*
* Function Name:    app_testStatusReport
* Input:            p_tmr - 定时器指针
*                   p_arg - 测试模块的运行状态
* Output:           None
*                   
* Returns:          None
* Description:      上报模块的状态.
* Note:             
***************************************************************************/
static void TestProc_StatusReport(void *p_tmr, void *p_arg)
{
    //TESTPROC_STATUS_T *pstTestState;
    int32_t s32Result;
    
    //DebugMsg("StatusReport\r\n");
    //pstTestState = (TESTPROC_STATUS_T *)p_arg;
    s32Result = TestProc_ReortStatus((TESTPROC_STATUS_T *)p_arg);
    if (TESTPROC_RESULT_OK != s32Result)
    {
        DebugMsg("E>StatusReport:%d\r\n", s32Result);
    }
}

/****************************************************************************
*
* Function Name:    TestProc_GetAddR1Type
* Input:            pstStatus - 系统参数指针
*                   u8CupID - 反应盘杯号
*                   u8Offset - R1,R2,R3,M
*                   
* Output:           pu8OutBuf - Byte0 - 试剂物理位置， 1 ~ TESTPROC_REAGENTDISK_POS_MAX; Byte1 - 试剂偏移
*                   
* Returns:          Sucess - TESTPROC_RESULT_OK, Fail - Other
* Description:      
* Note:             
***************************************************************************/
static int32_t TestProc_GetReagentPos(TESTPROC_STATUS_T *pstStatus, uint8_t u8CupID, uint8_t u8Offset, uint8_t * pu8OutBuf)
{
    uint16_t i;
    uint8_t u8TxBuf[4];
    uint8_t u8Pos = 0xFF;
    uint8_t u8Priority = 0;

    if (NULL == pu8OutBuf || NULL == pstStatus || u8CupID > TESTPROC_CUP_QUANTITY)
    {
        return TESTPROC_RESULT_ERR_PARAM;
    }
    pu8OutBuf[0] = 0xFF;
    pu8OutBuf[1] = 0xFF;
    if (TESTPROC_REAGENTDISK_OFFSET_R1 == u8Offset) // R1
    {
        for (i=0; i<TESTPROC_REAGENT_POS_MAX; i++)
        {
            if ((TESTPROC_REALOADTYPE_R1R2RM == g_stReagentMulty.stR1PosInfo[i].u8ReaLoadType || TESTPROC_REALOADTYPE_R1R2 == g_stReagentMulty.stR1PosInfo[i].u8ReaLoadType) &&
                g_stReagentMulty.stR1PosInfo[i].u16ReagentID == pstStatus->pstReactCupItem[u8CupID]->stItem.u16R1ID &&
                g_stReagentMulty.stR1PosInfo[i].u8Exist[0] == 1)
            {
                if (0xFF == u8Pos)
                {
                    u8Pos = i;
                }
                if (u8Priority < g_stReagentMulty.stR1PosInfo[i].u8Priority)
                {
                    u8Pos = i;
                    u8Priority = g_stReagentMulty.stR1PosInfo[i].u8Priority;
                }
            }
        }
        pu8OutBuf[1] = DISH_REAGENT_OFFSET_R1;
    }
    else if (TESTPROC_REAGENTDISK_OFFSET_R2 == u8Offset)
    {
        for (i=0; i<TESTPROC_REAGENT_POS_MAX; i++)
        {
            if ((TESTPROC_REALOADTYPE_R1R2RM == g_stReagentMulty.stR1PosInfo[i].u8ReaLoadType || TESTPROC_REALOADTYPE_R1R2 == g_stReagentMulty.stR1PosInfo[i].u8ReaLoadType) &&
                g_stReagentMulty.stR1PosInfo[i].u16ReagentID == pstStatus->pstReactCupItem[u8CupID]->stItem.u16R2ID &&
                g_stReagentMulty.stR1PosInfo[i].u8Exist[1] == 1)
            {
                if (0xFF == u8Pos)
                {
                    u8Pos = i;
                }
                if (u8Priority < g_stReagentMulty.stR1PosInfo[i].u8Priority)
                {
                    u8Pos = i;
                    u8Priority = g_stReagentMulty.stR1PosInfo[i].u8Priority;
                }
            }
        }        
        pu8OutBuf[1] = DISH_REAGENT_OFFSET_R2;
    }
    else if (TESTPROC_REAGENTDISK_OFFSET_R3 == u8Offset)
    {
        for (i=0; i<TESTPROC_REAGENT_POS_MAX; i++)
        {
            if (TESTPROC_REALOADTYPE_R3RM == g_stReagentMulty.stR1PosInfo[i].u8ReaLoadType &&
                g_stReagentMulty.stR1PosInfo[i].u16ReagentID == pstStatus->pstReactCupItem[u8CupID]->stItem.u16R3ID &&
                (g_stReagentMulty.stR1PosInfo[i].u8Exist[0] == 1 || g_stReagentMulty.stR1PosInfo[i].u8Exist[1] == 1))
            {
                if (0xFF == u8Pos)
                {
                    u8Pos = i;
                }
                if (u8Priority < g_stReagentMulty.stR1PosInfo[i].u8Priority)
                {
                    u8Pos = i;
                    u8Priority = g_stReagentMulty.stR1PosInfo[i].u8Priority;
                }
            }
        }
        DebugMsg("GetR3Pos:%d,%d,%d\r\n", u8Pos, g_stReagentMulty.stR1PosInfo[u8Pos].u8Exist[0], g_stReagentMulty.stR1PosInfo[u8Pos].u8Exist[1], g_stReagentMulty.stR1PosInfo[u8Pos].u8Exist[2]);
        if (g_stReagentMulty.stR1PosInfo[u8Pos].u8Exist[0] == 1)
        {
            pu8OutBuf[1] = DISH_REAGENT_OFFSET_R1;
        }
        else if (g_stReagentMulty.stR1PosInfo[u8Pos].u8Exist[1] == 1)
        {
            pu8OutBuf[1] = DISH_REAGENT_OFFSET_R2;
        }
        else
        {
            pu8OutBuf[1] = DISH_REAGENT_OFFSET_R1;
        }
    }
    else if (TESTPROC_REAGENTDISK_OFFSET_M == u8Offset)
    {
        // M放在R1试剂盘
        for (i=0; i<TESTPROC_REAGENT_POS_MAX; i++)
        {
            if ((TESTPROC_REALOADTYPE_R1R2RM == g_stReagentMulty.stR1PosInfo[i].u8ReaLoadType || TESTPROC_REALOADTYPE_R3RM == g_stReagentMulty.stR1PosInfo[i].u8ReaLoadType) &&
                g_stReagentMulty.stR1PosInfo[i].u16ReagentID == pstStatus->pstReactCupItem[u8CupID]->stItem.u16MID &&
                g_stReagentMulty.stR1PosInfo[i].u8Exist[2] == 1)
            {
                if (0xFF == u8Pos)
                {
                    u8Pos = i;
                }
                if (u8Priority < g_stReagentMulty.stR1PosInfo[i].u8Priority)
                {
                    u8Pos = i;
                    u8Priority = g_stReagentMulty.stR1PosInfo[i].u8Priority;
                }
            }
        }
        pu8OutBuf[1] = DISH_REAGENT_OFFSET_BEAD;
    }
    
    if (u8Pos >= TESTPROC_REAGENTDISK_POS_MAX)
    {
        u8Pos = 1;
        DebugMsg("WARNING>GetReagentPos:%d,%d,%d,%d,%d,%d\r\n", u8CupID, u8Offset, u8Pos,
                 pstStatus->pstReactCupItem[u8CupID]->stItem.u16R1ID, pstStatus->pstReactCupItem[u8CupID]->stItem.u16R2ID, pstStatus->pstReactCupItem[u8CupID]->stItem.u16MID);
        u8TxBuf[0] = u8CupID;
        u8TxBuf[1] = u8Offset;
        *(uint16_t *)&u8TxBuf[2] = pstStatus->pstReactCupItem[u8CupID]->stItem.u16R1ID;
        TestProc_ReportWarning(pstStatus, TESTPROC_WARNING_REAGENT_POSTION, u8TxBuf, 4);
        pu8OutBuf[0] = u8Pos;
        return TESTPROC_RESULT_OK;
    }
    else
    {
        u8Pos++; // 0~29  == 1~30
        pu8OutBuf[0] = u8Pos;
        //DebugMsg("$$%d,%d$$\r\n", pu8OutBuf[0], pu8OutBuf[1]);
        return TESTPROC_RESULT_OK;
    }
    //else if(3 == u8Offset)
    //{
    //    u16ID = pstStatus->pstReactCupItem[u8CupID]->stItem.u16MID;
    //}
    
}

/****************************************************************************
*
* Function Name:    TestProc_MoveReagentDiskPos
* Input:            pstState - 模块的状态指针

*                  u8CupID - 反应盘位置
*                   eOffset - 试剂类别,R1,R2,R3,M,SCAN
*                   
* Output:           None
*                   
* Returns:          TESTPROC_RESULT_OK,TESTPROC_RESULT_NG
* Description:      
* Note:             
***************************************************************************/
static int32_t TestProc_MoveReagentDiskPos(TESTPROC_STATUS_T *pstStatus, uint8_t u8CupID, REAGENTDISK_OFFSET_E eOffset)
{
    int32_t s32Result = TESTPROC_RESULT_OK;
    uint8_t u8TxBuf[4];
    uint8_t u8PosBuf[4];
    
    // 试剂盘移动
    if (pstStatus->pstReactCupItem[u8CupID] != NULL)
    {
        s32Result = TestProc_GetReagentPos(pstStatus, u8CupID, eOffset, u8PosBuf);
        if (TESTPROC_RESULT_OK == s32Result)
        {
            if (u8PosBuf[0] <= TESTPROC_REAGENTDISK_POS_MAX && u8PosBuf[1] <= DISH_REAGENT_OFFSET_BEAD)
            {
                if (TESTPROC_REAGENTDISK_OFFSET_R1 == eOffset)
                {
                    pstStatus->pstReactCupItem[u8CupID]->u8ReagentPos = u8PosBuf[0];
                }
                DebugMsg("R_Pos:%d,%d,%d,%d\r\n", u8CupID, eOffset, u8PosBuf[0], u8PosBuf[1]);                
                pstStatus->u8LastReagentDiskPos = u8PosBuf[0];
                g_u8ReagentDiskShakeEnable = 0;
                pstStatus->eReagentDiskState = TESTPROC_REAGENTISK_STATE_RUNNING;
                u8TxBuf[0] = (uint8_t)u8PosBuf[0];
                u8TxBuf[1] = (uint8_t)u8PosBuf[1];
                TestProc_ReagentDiskStartWork(REAGENTDISK_WORK_MOVETOPOS, u8TxBuf, 2);
                //DebugMsg("ReagentDiskToPos:%d,%d\r\n", u16ReagentPos, eOffset);
            }
            else
            {
                DebugMsg("E>ReagentPos:%d,%d\r\n", u8PosBuf[0], u8PosBuf[1]);
                // Warning
            }
        }
        else
        {
            DebugMsg("E>GetReagentPos:%d,%d,%d\r\n", s32Result, u8PosBuf[0], u8PosBuf[1]);
            // // Warning
        }
    }
    else
    {
        if (pstStatus->eReactCupState[u8CupID] >= TESTPROC_REACT_CUP_IN)
        {
            DebugMsg("E>Trigger4_ItemLost:%d,%02X\r\n", u8CupID, pstStatus->u8InterferenceReagentDisk, pstStatus->eReactCupState);
            u8TxBuf[0] = u8CupID;
            u8TxBuf[1] = pstStatus->u8InterferenceReagentDisk;
            u8TxBuf[2] = eOffset;
            u8TxBuf[3] = pstStatus->eReactCupState[u8CupID];
            TestProc_ReportWarning(pstStatus, TESTPROC_WARNING_FLOW_ITEM_LOST_RD, u8TxBuf, 4);
            s32Result = TESTPROC_RESULT_NG;
        }
    }
    
    return s32Result;
}

/****************************************************************************
*
* Function Name:    TestProc_ReagentDiskShake
* Input:            pstState - 模块的状态指针
*                   
* Output:           None
*                   
* Returns:          TESTPROC_RESULT_OK,TESTPROC_RESULT_NG
* Description:      试剂盘磁珠摇匀
* Note:             
***************************************************************************/
static int32_t TestProc_ReagentDiskShake(TESTPROC_STATUS_T *pstStatus)

{
    int32_t s32Result = TESTPROC_RESULT_OK;
    uint8_t u8TxBuf[4];
    uint8_t u8CurrentPos = TestProc_Api_ReagentDiskGetPos();
    uint8_t u8TargetPos = u8CurrentPos + (TESTPROC_REAGENT_POS_CONFIG/2);

    if (u8TargetPos > TESTPROC_REAGENT_POS_CONFIG)
    {
        u8TargetPos -= TESTPROC_REAGENT_POS_CONFIG;
    }

    if (TESTPROC_REAGENTISK_STATE_IDLE == pstStatus->eReagentDiskState)
    {
        DebugMsg("R_M_Shake\r\n");        
        g_u8ReagentDiskShakeEnable = 0;
        pstStatus->eReagentDiskState = TESTPROC_REAGENTISK_STATE_SHAKE_ING;
        u8TxBuf[0] = u8TargetPos;
        u8TxBuf[1] = DISH_REAGENT_OFFSET_R1;
        TestProc_ReagentDiskStartWork(REAGENTDISK_WORK_MOVETOPOS, u8TxBuf, 2);
    }
    else
    {
        s32Result = TESTPROC_RESULT_NG;
        DebugMsg("E>ReagentDiskShake:%d\r\n", pstStatus->eReagentDiskState);
    }

    return s32Result;
}

static int32_t TestProc_Reset(TESTPROC_STATUS_T *pstStatus)
{
    int32_t s32Result = TESTPROC_RESULT_OK;
    int32_t i;
    OS_ERR err;
    
    DebugMsg("**Reset**\r\n");
    pstStatus->eWorkType = TESTPROC_WORK_TYPE_RESET;
    pstStatus->stCellStatus.u8WorkStatus = TESTPROC_WORK_RESET;
    for (i = 0; i < TESTPROC_UNIT_MAX; i++)
    {
        pstStatus->eResetState[i] = TESTPROC_RESET_ONGOING;
    }
    pstStatus->eResetState[TESTPROC_UNIT_EVENT] = TESTPROC_RESET_OK;
    TestProc_ReagentStartWork(REAGENT_WORK_RESET, NULL, 0);
    TestProc_SampleStartWork(SAMPLE_WORK_RESET, NULL, 0);
    TestProc_CupStackStartWork(CUPSTACK_WORK_RESET, NULL, 0);
    TestProc_HandCupStartWork(HANDCUP_WORK_RESET, NULL, 0);
    g_u8TestProc_ResetMask = 0;
    g_u8ReagentDiskReset_OneShot = 0;
    g_u8WashDiskReset_OneShot = 0;
    g_u8TrackReset_OneShot = 0;
    g_u8ReactReset_OneShot = 0;
    g_SystemResetTimer = OSTimeGet(&err);
    
    return s32Result;
}

static TESTPROC_RESET_STATE TestProc_ResetCheckResult(TESTPROC_STATUS_T *pstStatus)
{
    int32_t i;
    TESTPROC_RESET_STATE Result = TESTPROC_RESET_ONGOING;
    uint8_t u8ResetCount;
    uint8_t u8ResetNG;
    
    //DebugMsg(" >> %d,%d\r\n", g_u8TestProc_ResetMask, g_u8ReactReset_OneShot);
    if ((g_u8TestProc_ResetMask & RESET_ENABLE_REACTMASK) == RESET_ENABLE_REACTMASK && 0 == g_u8ReactReset_OneShot)
    {
        DebugMsg("React Reset\r\n");
        TestProc_ReactStartWork(REACT_WORK_RESET, NULL, 0);
        g_u8ReactReset_OneShot = 1;
    }
    if ((g_u8TestProc_ResetMask & RESET_ENABLE_TRACKMASK) == RESET_ENABLE_TRACKMASK && 0 == g_u8TrackReset_OneShot)
    {
        //DebugMsg("Track Reset\r\n");
        //TestProc_ReactStartWork(REACT_WORK_RESET, NULL, 0);
        g_u8TrackReset_OneShot = 1;
    }
    if ((g_u8TestProc_ResetMask & RESET_ENABLE_WASHDISKMASK) == RESET_ENABLE_WASHDISKMASK && 0 == g_u8WashDiskReset_OneShot)
    {
        DebugMsg("Wash Reset\r\n");
        TestProc_WashStartWork(WASH_WORK_RESET, NULL, 0);
        g_u8WashDiskReset_OneShot = 1;
    }
    if ((g_u8TestProc_ResetMask & RESET_ENABLE_REAGENTDISKMASK) == RESET_ENABLE_REAGENTDISKMASK && 0 == g_u8ReagentDiskReset_OneShot)
    {
        DebugMsg("ReagentDisk Reset\r\n");
        TestProc_ReagentDiskStartWork(REAGENTDISK_WORK_RESET, NULL, 0);
        g_u8ReagentDiskReset_OneShot = 1;
    }
    //
    u8ResetCount = 0;
    u8ResetNG = 0;
    for (i = 0; i < TESTPROC_UNIT_MAX; i++)
    {
        if (TESTPROC_RESET_OK == pstStatus->eResetState[i])
        {
            u8ResetCount++;
        }
        else if (TESTPROC_RESET_NG == pstStatus->eResetState[i])
        {
            u8ResetCount++;
            u8ResetNG = 1;
        }
    }
    if (u8ResetCount >= TESTPROC_UNIT_MAX)
    {
        DebugMsg("Reset %s\r\n", 1 == u8ResetNG ? "NG" : "OK");
        TestProc_CheckResetNG();
        if (0 == u8ResetNG)
        {
            Result = TESTPROC_RESET_OK;
        }
        else
        {
            Result = TESTPROC_RESET_NG;
        }
        g_SystemResetTimer = TESTPROC_TICK_INVALID;
    }
    
    return Result;
}

static int32_t TestProc_Standby(TESTPROC_STATUS_T *pstStatus)
{
    int32_t s32Result = TESTPROC_RESULT_OK;
    //int i;
    OS_ERR err;
    
    DebugMsg("**StandBy**\r\n");
    
    g_SystemResetTimer = TESTPROC_TICK_INVALID;
    g_SystemPowerOnTimer = TESTPROC_TICK_INVALID;
    FSM_TestProcess = FSM_TESTPROCESS_IDLE;
    FSM_M_SeparateAuto = FSM_M_SEPARATE_AUTO_IDLE;
    FSM_M_Separate = FSM_M_SEPARATE_IDLE;
    FSM_ManualTest = FSM_MANUAL_TEST_IDLE;
    FSM_SampleAccuracy = FSM_SAMPLE_ACCURACY_IDLE;
    FSM_ReagentAccuracy = FSM_REAGENT_ACCURACY_IDLE;
    FSM_DarkTest = FSM_DARK_TEST_IDLE;
    FSM_WashAccuracy = FSM_WASH_ACCURACY_IDLE;
    FSM_LiquidPerfuse = FSM_LIQUID_PERFUSE_IDLE;
    pstStatus->stCellStatus.u8WorkStatus = TESTPROC_WORK_STANDBY;
    pstStatus->s32StandbyCount = -1;
    TestProc_ReactStartWork(REACT_WORK_STOP, NULL, 0);
    TestProc_WashStartWork(WASH_WORK_WASH_STANDBY, NULL, 0);
    TestProc_SendDataToComm(BOARD_ID_TRACK_COMM, CAN_CMD_CELL_STOP, NULL, 0);
    //DebugMsg("ReactCup:");
    //for (i = 0; i <= TESTPROC_CUP_QUANTITY; i++)
    //{
    //    DebugMsg("%d, ", pstStatus->eReactCupState[i]);
    //}
    //DebugMsg("\r\nWashCup:");
    //for (i = 0; i <= TESTPROC_WASHDISK_POS_QUANTITY; i++)
    //{
    //    DebugMsg("%d, ", pstStatus->eWashCupState[i]);
    //}
    //DebugMsg("\r\n");
    OSTmrStop(&g_stWashDiskRotateTimer, OS_OPT_TMR_NONE, NULL, &err);
    
    return s32Result;
}

/****************************************************************************
 *
 * Function Name:    TestProc_AddReagentStep1
 * Input:            pstState - 模块的状态指针
 *                   
 * Output:           None
 *                   
 * Returns:          None
 * Description:      尝试获取测试项
 * Note:             
 ***************************************************************************/
static void TestProc_AddReagentStep1(TESTPROC_STATUS_T *pstStatus, uint8_t u8CupID)
{
    uint8_t u8TxBuf[8];
    static uint16_t u16InjectQuantity = 0;
    
    //DebugMsg("ARS1:%d,%d,%d,%d\r\n", u8CupID, pstStatus->eReagentState, pstStatus->pstReactCupItem[u8CupID]->eTwoStepState, pstStatus->eReagentDiskState);

    if (NULL == pstStatus->pstReactCupItem[u8CupID])
    {
        return;
    }
    //
    if (pstStatus->eReactCupState[u8CupID] != TESTPROC_REACT_CUP_IN && 
        pstStatus->eReactCupState[u8CupID] != TESTPROC_REACT_CUP_2STEP_CUP_IN)
    {
        return;
    }

    // 准备+R1,清洗移到试剂盘上方
    if (TESTPROC_REAGENT_STATE_IDLE == pstStatus->eReagentState)
    {
        if (pstStatus->pstReactCupItem[u8CupID]->eTwoStepState == TESTPROC_TWO_STEP_DISABLE)
        {
            DebugMsg(" RS1_2Step R2\r\n");
            u16InjectQuantity = 0;
            TestProc_ReagentStartWork(REAGENT_WORK_ABSORB_RS1_R2_PREPARE, NULL, 0);
            pstStatus->eReagentState = TESTPROC_REAGENT_RS1_R2_2STEP_READY_ING;
        }
        else
        {
            u16InjectQuantity = 0;
            TestProc_ReagentStartWork(REAGENT_WORK_ABSORB_RS1_R1_PREPARE, NULL, 0);
            pstStatus->eReagentState = TESTPROC_REAGENT_RS1_R1_READY_ING;
            DebugMsg(" PrepareRS1:%d\r\n", u8CupID);
        }
    }
    // +R1,R2,M;锁定试剂盘
    if (TESTPROC_REAGENT_RS1_R1_READY == pstStatus->eReagentState)
    {
        if (TESTPROC_REAGENTISK_STATE_POSITION == pstStatus->eReagentDiskState)
        {
            u16InjectQuantity += pstStatus->pstReactCupItem[u8CupID]->stItem.u16R1Quantity;
            *(uint16_t *)(&u8TxBuf[0]) = pstStatus->pstReactCupItem[u8CupID]->stItem.u16R1Quantity;
            u8TxBuf[2] = pstStatus->u8LastReagentDiskPos;
            DebugMsg(" +RS1_R1:%d,%d,%d,%d,%d\r\n", u8CupID, pstStatus->pstReactCupItem[u8CupID]->stItem.u32TestID, pstStatus->pstReactCupItem[u8CupID]->stItem.u16R1ID, u8TxBuf[2], *(uint16_t *)(&u8TxBuf[0]));
            TestProc_ReagentStartWork(REAGENT_WORK_ABSORB_RS1_R1, u8TxBuf, 3);
            pstStatus->eReagentState = TESTPROC_REAGENT_RS1_R1_ABSORB_ING;
        }
    }
    else if (TESTPROC_REAGENT_RS1_R1_ABSORB_OK == pstStatus->eReagentState)
    {
        if (pstStatus->pstReactCupItem[u8CupID] != NULL)
        {
            if (TestProc_testIsAddReagent1R2(pstStatus->pstReactCupItem[u8CupID]->stItem.u8AddStep) && 
                pstStatus->pstReactCupItem[u8CupID]->eTwoStepState != TESTPROC_TWO_STEP_ENABLE)
            {
                if (TESTPROC_REAGENTISK_STATE_IDLE == pstStatus->eReagentDiskState)
                {                        
                    TestProc_MoveReagentDiskPos(pstStatus, u8CupID, TESTPROC_REAGENTDISK_OFFSET_R2);
                }
                //
                if (TESTPROC_REAGENTISK_STATE_POSITION == pstStatus->eReagentDiskState)
                {
                    u16InjectQuantity += pstStatus->pstReactCupItem[u8CupID]->stItem.u16R2Quantity;
                    *(uint16_t *)(&u8TxBuf[0]) = pstStatus->pstReactCupItem[u8CupID]->stItem.u16R2Quantity;
                    u8TxBuf[2] = pstStatus->u8LastReagentDiskPos;
                    DebugMsg(" +RS1_R2:%d,%d,%d,%d,%d\r\n", u8CupID, pstStatus->pstReactCupItem[u8CupID]->stItem.u32TestID, 
                        pstStatus->pstReactCupItem[u8CupID]->stItem.u16R2ID, u8TxBuf[2], *(uint16_t *)(&u8TxBuf[0]));
                    TestProc_ReagentStartWork(REAGENT_WORK_ABSORB_RS1_R2, u8TxBuf, 3);
                    pstStatus->eReagentState = TESTPROC_REAGENT_RS1_R2_ABSORB_ING;
                }
            }
            else
            {
                DebugMsg(" RS1_Without R2\r\n");
                pstStatus->eReagentState = TESTPROC_REAGENT_RS1_TO_REACT_ING;
                TestProc_ReagentStartWork(REAGENT_WORK_MOVE_TO_REACT, u8TxBuf, 2);
            }
        }
        else
        {
            DebugMsg("E>RS1_R2_ItemNULL:%d\r\n", u8CupID);
        }
    }
    else if (TESTPROC_REAGENT_RS1_R2_2STEP_READY == pstStatus->eReagentState) // 吸R2准备完成
    {
        if (TESTPROC_REAGENTISK_STATE_POSITION == pstStatus->eReagentDiskState)
        {
            u16InjectQuantity += pstStatus->pstReactCupItem[u8CupID]->stItem.u16R2Quantity;
            *(uint16_t *)(&u8TxBuf[0]) = pstStatus->pstReactCupItem[u8CupID]->stItem.u16R2Quantity;
            u8TxBuf[2] = pstStatus->u8LastReagentDiskPos;
            DebugMsg(" +RS1_R2_2Step:%d,%d,%d,%d,%d\r\n", u8CupID, pstStatus->pstReactCupItem[u8CupID]->stItem.u32TestID, pstStatus->pstReactCupItem[u8CupID]->stItem.u16R2ID, u8TxBuf[2], *(uint16_t *)(&u8TxBuf[0]));
            TestProc_ReagentStartWork(REAGENT_WORK_ABSORB_RS1_R2_2STEP, u8TxBuf, 3);
            pstStatus->eReagentState = TESTPROC_REAGENT_RS1_R2_2STEP_ABSORB_ING;
        }
    }
    else if (TESTPROC_REAGENT_RS1_R2_ABSORB_OK == pstStatus->eReagentState)
    {
        if (TESTPROC_REACT_STATE_31_STOP == pstStatus->eReactState)
        {
            pstStatus->u8InterferenceReact |= TESTPROC_INTERFERENCE_REACT_REAGENT;
            *(uint16_t *)(&u8TxBuf[0]) = u16InjectQuantity;
            TestProc_ReagentStartWork(REAGENT_WORK_INJECT_RS1, u8TxBuf, 2);
            pstStatus->eReagentState = TESTPROC_REAGENT_RS1_INJECT_ING;
            DebugMsg(" InjectRS1:%d,%d,%d\r\n", u8CupID, pstStatus->pstReactCupItem[u8CupID]->stItem.u32TestID, *(uint16_t *)(&u8TxBuf[0]));
        }
    }
}

/****************************************************************************
 *
 * Function Name:    TestProc_AddReagentStep2
 * Input:            pstState - 模块的状态指针
 *                   u8CupID - 目标杯号
 *                   
 * Output:           None
 *                   
 * Returns:          None
 * Description:      
 * Note:             
 ***************************************************************************/
static void TestProc_AddReagentStep2(TESTPROC_STATUS_T *pstStatus, uint8_t u8CupID)
{
    uint8_t  u8TxBuf[4];
    static uint16_t u16InjectQuantity = 0;
    
    //DebugMsg("ARS2:%d,%d,%d,%d\r\n", u8CupID, pstStatus->eReagentState, pstStatus->pstReactCupItem[u8CupID]->eTwoStepState, pstStatus->eReagentDiskState);
    //if (NULL != pstStatus->pstReactCupItem[u8CupID])
    //{
    //    DebugMsg("  %d,%d,%d\r\n", pstStatus->pstReactCupItem[u8CupID]->stItem.u32TestID, pstStatus->pstReactCupItem[u8CupID]->stItem.u16MID, pstStatus->pstReactCupItem[u8CupID]->stItem.u16MQuantity);
    //}
    if (TESTPROC_REACT_CUP_RM1 == pstStatus->eReactCupState[u8CupID])
    {
        if (pstStatus->pstReactCupItem[u8CupID] != NULL)
        {
            if (TESTPROC_REAGENT_STATE_IDLE == pstStatus->eReagentState)
            {
                u16InjectQuantity = 0;
                if (TestProc_testIsAddReagent2R2(pstStatus->pstReactCupItem[u8CupID]->stItem.u8AddStep) == 0)
                {
                    pstStatus->eReagentState = TESTPROC_REAGENT_RS2_R2_ABSORB_OK;
                }
            }
            if (TESTPROC_REAGENT_RS2_R2_ABSORB_OK == pstStatus->eReagentState)
            {
                if (TestProc_testIsAddReagent2R3(pstStatus->pstReactCupItem[u8CupID]->stItem.u8AddStep) == 0)
                {
                    pstStatus->eReagentState = TESTPROC_REAGENT_RS2_R3_ABSORB_OK;
                }
            }
            //
            if (TESTPROC_TWO_STEP_DISABLE == pstStatus->pstReactCupItem[u8CupID]->eTwoStepState)
            {
                pstStatus->eReactCupState[u8CupID] = TESTPROC_REACT_CUP_R2;
                if (TESTPROC_REAGENT_RS2_R2_ABSORB_OK == pstStatus->eReagentState || TESTPROC_REAGENT_RS2_R3_ABSORB_OK == pstStatus->eReagentState)
                {
                    pstStatus->eReagentState = TESTPROC_REAGENT_STATE_IDLE;
                }
                DebugMsg("RS2_Skip:%d,%d\r\n", u8CupID, pstStatus->eReagentState);
            }
            else
            {
                if (TESTPROC_REAGENT_STATE_IDLE == pstStatus->eReagentState)
                {
                    DebugMsg(" PrepareRS2_R2:%d,%d\r\n", u8CupID, pstStatus->pstReactCupItem[u8CupID]->stItem.u32TestID);
                    TestProc_ReagentStartWork(REAGENT_WORK_ABSORB_RS2_R2_PREPARE, u8TxBuf, 2);
                    pstStatus->eReagentState = TESTPROC_REAGENT_RS2_R2_READY_ING;
                }
                else if (TESTPROC_REAGENT_RS2_R2_READY_OK == pstStatus->eReagentState)
                {
                    if (TESTPROC_REAGENTISK_STATE_POSITION == pstStatus->eReagentDiskState)
                    {
                        u16InjectQuantity += pstStatus->pstReactCupItem[u8CupID]->stItem.u16R2Quantity;
                        *(uint16_t *)(&u8TxBuf[0]) = pstStatus->pstReactCupItem[u8CupID]->stItem.u16R2Quantity;
                        u8TxBuf[2] = pstStatus->u8LastReagentDiskPos;
                        DebugMsg(" AbsorbRS2_R2:%d,%d,%d,%d\r\n", u8CupID, pstStatus->pstReactCupItem[u8CupID]->stItem.u32TestID, u8TxBuf[2], u16InjectQuantity);
                        TestProc_ReagentStartWork(REAGENT_WORK_ABSORB_RS2_R2, u8TxBuf, 3);
                        pstStatus->eReagentState = TESTPROC_REAGENT_RS2_R2_ABSORB_ING;
                    }
                }
                else if (TESTPROC_REAGENT_RS2_R2_ABSORB_OK == pstStatus->eReagentState)
                {
                    DebugMsg(" PrepareRS2_R3:%d,%d,%d\r\n", u8CupID, pstStatus->pstReactCupItem[u8CupID]->stItem.u32TestID, u16InjectQuantity, pstStatus->eReagentDiskState);
                    if (TESTPROC_REAGENTISK_STATE_IDLE == pstStatus->eReagentDiskState || TESTPROC_REAGENTISK_STATE_POSITION == pstStatus->eReagentDiskState)
                    {
                        TestProc_MoveReagentDiskPos(pstStatus, u8CupID, TESTPROC_REAGENTDISK_OFFSET_R3);
                    }
                    if (u16InjectQuantity > 0
)
                    {
                        TestProc_ReagentStartWork(REAGENT_WORK_ABSORB_RS2_R3_CLEAN, NULL, 0);
                    }
                    else
                    {
                        TestProc_ReagentStartWork(REAGENT_WORK_ABSORB_RS2_R3_PREPARE, NULL, 0);
                    }
                    pstStatus->eReagentState = TESTPROC_REAGENT_RS2_R3_READY_ING;
                }
                //else if (TESTPROC_REAGENT_RS2_R3_READY_ING == pstStatus->eReagentState)
                //{
                //    if (TESTPROC_REAGENTISK_STATE_IDLE == pstStatus->eReagentDiskState)
                //    {
                //        TestProc_MoveReagentDiskPos(pstStatus, u8CupID, TESTPROC_REAGENTDISK_OFFSET_R3);
                //    }
                //}
                else if (TESTPROC_REAGENT_RS2_R3_READY_OK == pstStatus->eReagentState)
                {
                    if (TESTPROC_REAGENTISK_STATE_POSITION == pstStatus->eReagentDiskState)
                    {
                        u16InjectQuantity += pstStatus->pstReactCupItem[u8CupID]->stItem.u16R3Quantity;
                        *(uint16_t *)(&u8TxBuf[0]) = pstStatus->pstReactCupItem[u8CupID]->stItem.u16R2Quantity;
                        u8TxBuf[2] = pstStatus->u8LastReagentDiskPos;
                        DebugMsg(" AbsorbRS2_R3:%d,%d,%d,%d\r\n", u8CupID, pstStatus->pstReactCupItem[u8CupID]->stItem.u32TestID, u8TxBuf[2], u16InjectQuantity);
                        TestProc_ReagentStartWork(REAGENT_WORK_ABSORB_RS2_R3, u8TxBuf, 3);
                        pstStatus->eReagentState = TESTPROC_REAGENT_RS2_R3_ABSORB_ING;
                    }
                }
                else if (TESTPROC_REAGENT_RS2_R3_ABSORB_OK == pstStatus->eReagentState)
                {
                    DebugMsg(" PrepareRS2_M:%d,%d,%d,%d\r\n", u8CupID, pstStatus->pstReactCupItem[u8CupID]->stItem.u32TestID, u16InjectQuantity, pstStatus->eReagentDiskState);
                    if (TESTPROC_REAGENTISK_STATE_IDLE == pstStatus->eReagentDiskState || TESTPROC_REAGENTISK_STATE_POSITION == pstStatus->eReagentDiskState)
                    {
                        TestProc_MoveReagentDiskPos(pstStatus, u8CupID, TESTPROC_REAGENTDISK_OFFSET_M);
                    }
                    if (u16InjectQuantity > 0)
                    {
                        TestProc_ReagentStartWork(REAGENT_WORK_ABSORB_RS2_M_CLEAN, NULL, 0);
                    }
                    else
                    {
                        TestProc_ReagentStartWork(REAGENT_WORK_ABSORB_RS2_M_PREPARE, NULL, 0);
                    }
                    pstStatus->eReagentState = TESTPROC_REAGENT_RS2_M_READY_ING;
                }
                //else if (TESTPROC_REAGENT_RS2_M_READY_ING == pstStatus->eReagentState)
                //{
                //    if (TESTPROC_REAGENTISK_STATE_IDLE == pstStatus->eReagentDiskState)
                //    {
                //        TestProc_MoveReagentDiskPos(pstStatus, u8CupID, TESTPROC_REAGENTDISK_OFFSET_M);
                //    }
                //}
                else if (TESTPROC_REAGENT_RS2_M_READY_OK == pstStatus->eReagentState)
                {
                    if (TESTPROC_REAGENTISK_STATE_POSITION == pstStatus->eReagentDiskState)
                    {
                        u16InjectQuantity += pstStatus->pstReactCupItem[u8CupID]->stItem.u16MQuantity;
                        *(uint16_t *)(&u8TxBuf[0]) = pstStatus->pstReactCupItem[u8CupID]->stItem.u16MQuantity;
                        u8TxBuf[2] = pstStatus->u8LastReagentDiskPos;
                        DebugMsg(" AbsorbRS2_M:%d,%d,%d,%d\r\n", u8CupID, pstStatus->pstReactCupItem[u8CupID]->stItem.u32TestID, u8TxBuf[2], u16InjectQuantity);
                        TestProc_ReagentStartWork(REAGENT_WORK_ABSORB_RS2_M, u8TxBuf, 3);
                        pstStatus->eReagentState = TESTPROC_REAGENT_RS2_M_ABSORB_ING;
                    }
                }
                else if (TESTPROC_REAGENT_RS2_M_ABSORB_OK == pstStatus->eReagentState)
                {
                    if (TESTPROC_REACT_STATE_09_STOP == pstStatus->eReactState)
                    {
                        pstStatus->u8InterferenceReact |= TESTPROC_INTERFERENCE_REACT_REAGENT2;
                        *(uint16_t *)(&u8TxBuf[0]) = u16InjectQuantity;
                        TestProc_ReagentStartWork(REAGENT_WORK_INJECT_RS2, u8TxBuf, 2);
                        pstStatus->eReagentState = TESTPROC_REAGENT_RS2_INJECT_ING;
                        DebugMsg(" InjectRS2:%d,%d,%d\r\n", u8CupID, pstStatus->pstReactCupItem[u8CupID]->stItem.u32TestID, *(uint16_t *)(&u8TxBuf[0]));
                    }
                }
            }
        }
        else
        {
            DebugMsg("E>RS2_ItemNULL:%d\r\n", u8CupID);
        }
    }
}

/****************************************************************************
 *
 * Function Name:    TestProc_MoveReactCupToWash
 * Input:            pstState - 模块的状态指针
 *                   u8CupID - 目标杯号
 *                   
 * Output:           None
 *                   
 * Returns:          None
 * Description:      
 * Note:             
 ***************************************************************************/
static int32_t TestProc_MoveReactCupToWash(TESTPROC_STATUS_T *pstStatus, uint8_t u8CupID)
{
    int32_t  s32Result = TESTPROC_RESULT_OK;
    uint8_t  u8TxBuf[4];

    if (TESTPROC_HANDCUP_STATE_IDLE == pstStatus->eHandCupBusy)
    {
        if (IsReactCupToWash(pstStatus, u8CupID) > 0)
        {
            if (TESTPROC_WASHDISK_STATE_IDLE == pstStatus->eWashDiskState &&
                TESTPROC_WASH_CUP_INIT == pstStatus->eWashCupState[pstStatus->u8WashDiskHandCup])
            {
                s32Result = TestProc_HandCupStartWork(HANDCUP_WORK_TO_REACT, NULL, 0);
                pstStatus->eHandCupBusy = TESTPROC_HANDCUP_STATE_TO_REACTFORWASH;
                DebugMsg("ReadyToWash_1:%d,%d\r\n", u8CupID, pstStatus->u8WashDiskHandCup);
            }
        }
    }
    else if (TESTPROC_HANDCUP_STATE_ON_REACTGORWASH == pstStatus->eHandCupBusy)
    {
        if (u8CupID > 0 && TESTPROC_REACT_STATE_02_STOP == pstStatus->eReactState)
        {
            s32Result = TestProc_HandCupStartWork(HANDCUP_WORK_CUP_REACT_TO_WASH, NULL, 0);
            pstStatus->u8InterferenceReact |= TESTPROC_INTERFERENCE_REACT_HANDCUP;
            pstStatus->eHandCupBusy = TESTPROC_HANDCUP_STATE_BUSY;
            pstStatus->pstWashCupItem[pstStatus->u8WashDiskHandCup] = pstStatus->pstReactCupItem[u8CupID];
            pstStatus->pstReactCupItem[u8CupID] = NULL;
            pstStatus->eReactCupState[u8CupID] = TESTPROC_REACT_CUP_INIT;
            DebugMsg("ReactCupToWash:%d>%d,%d\r\n", u8CupID, pstStatus->u8WashDiskHandCup, NULL == pstStatus->pstWashCupItem[pstStatus->u8WashDiskHandCup] ? 0 : pstStatus->pstWashCupItem[pstStatus->u8WashDiskHandCup]->stItem.u32TestID);
        }
    }
    else if (TESTPROC_HANDCUP_STATE_ONWASH == pstStatus->eHandCupBusy)
    {
        if (TESTPROC_WASHDISK_STATE_IDLE == pstStatus->eWashDiskState)
        {
            s32Result = TestProc_HandCupStartWork(HANDCUP_WORK_CUP_PUT_TO_WASH, NULL, 0);
            pstStatus->u8InterferenceWash |= TESTPROC_INTERFERENCE_WASH_HAND;
            pstStatus->eHandCupBusy = TESTPROC_HANDCUP_STATE_BUSY;
            DebugMsg("__CupToWash:%d\r\n", pstStatus->u8WashDiskHandCup);
        }
        else
        {
            DebugMsg("E>_CupToWash:%d>%d,%d\r\n", u8CupID, pstStatus->u8WashDiskHandCup, pstStatus->eWashDiskState);
            u8TxBuf[0] = pstStatus->eWashDiskState;
            u8TxBuf[1] = u8CupID;
            u8TxBuf[2] = pstStatus->u8WashDiskHandCup;
            TestProc_ReportWarning(pstStatus, TESTPROC_WARNING_FLOW_WASH_INCUP_STATE, u8TxBuf, 3);
        }
    }

    return s32Result;
}

/****************************************************************************
 *
 * Function Name:    TestProc_MoveWashCupToMeasureRoom
 * Input:            pstState - 模块的状态指针
 *                   u8CupID - 目标杯号
 *                   
 * Output:           None
 *                   
 * Returns:          None
 * Description:      
 * Note:             
 ***************************************************************************/
static int32_t TestProc_MoveWashCupToMeasureRoom(TESTPROC_STATUS_T *pstStatus)
{
    int32_t s32Result = TESTPROC_RESULT_OK;
    
    // 清洗盘反应杯处理
    if (pstStatus->eWashCupState[pstStatus->u8WashDiskHandCup] >= TESTPROC_WASH_CUP_MIX)
    {
        // DebugMsg(" ++>%d,%d,%d,%d\r\n", pstStatus->u8WashDiskHandCup, pstStatus->eHandCupBusy, pstStatus->eWashDiskState, pstStatus->eMeasureRoomState);
        //
        // 机械手抓清洗盘样本杯到测量室上方
        if (TESTPROC_HANDCUP_STATE_IDLE == pstStatus->eHandCupBusy && TESTPROC_WASHDISK_STATE_IDLE == pstStatus->eWashDiskState)
        {
            if (pstStatus->pstWashCupItem[pstStatus->u8WashDiskHandCup] != NULL)
            {
                if (TESTPROC_TWO_STEP_ENABLE != pstStatus->pstWashCupItem[pstStatus->u8WashDiskHandCup]->eTwoStepState)
                {
                    if (TESTPROC_MEASUREROOM_STATE_IDLE == pstStatus->eMeasureRoomState ||
                     TESTPROC_MEASUREROOM_STATE_HALFOPEN == pstStatus->eMeasureRoomState ||
                     TESTPROC_MEASUREROOM_STATE_HALFOPEN_ING == pstStatus->eMeasureRoomState)
                    {
                        s32Result = TestProc_HandCupStartWork(HANDCUP_WORK_CUP_TO_MEASUREROOM, NULL, 0);
                        pstStatus->eHandCupBusy = TESTPROC_HANDCUP_STATE_TO_MEASUREROOM;
                        DebugMsg("WashCupTo_MR:%d,%d,%d\r\n", pstStatus->pstWashCupItem[pstStatus->u8WashDiskHandCup]->stItem.u32TestID, pstStatus->u8WashDiskHandCup, pstStatus->eMeasureRoomState);
                    }
                }
            }
            else
            {
                s32Result = TESTPROC_RESULT_ERR_ITEMLOST;
                DebugMsg("E>WashDiskItemLost:%d\r\n", pstStatus->u8WashDiskHandCup);
            }
        }
        // 半开门
        if (TESTPROC_MEASUREROOM_STATE_IDLE == pstStatus->eMeasureRoomState && 
            TESTPROC_TWO_STEP_ENABLE != pstStatus->pstWashCupItem[pstStatus->u8WashDiskHandCup]->eTwoStepState)
        {
            s32Result = TestProc_MeasureRoomStartWork(MEASUREROOM_WORK_HALFOPEN, NULL, 0);
            pstStatus->eMeasureRoomState = TESTPROC_MEASUREROOM_STATE_HALFOPEN_ING;
            DebugMsg("WashHalfOpen:%d,%d,%d\r\n", s32Result, pstStatus->u8WashDiskHandCup, pstStatus->eMeasureRoomState);
        }
        else if (TESTPROC_MEASUREROOM_STATE_HALFOPEN == pstStatus->eMeasureRoomState) // 放反应杯
        {
            if (TESTPROC_HANDCUP_STATE_ON_MEASUREROOM == pstStatus->eHandCupBusy)
            {
                s32Result = TestProc_HandCupStartWork(HANDCUP_WORK_CUP_PUT_MEASUREROOM, NULL, 0);
                if (TESTPROC_RESULT_OK == s32Result)
                {
                    pstStatus->eHandCupBusy = TESTPROC_HANDCUP_STATE_TO_MEASUREROOM;
                    //pstStatus->eMeasureRoomState = TESTPROC_MEASUREROOM_STATE_CUPIN_ING;
                }
                DebugMsg("WashCupIn_MR:%d,%d,%d\r\n", s32Result, pstStatus->u8WashDiskHandCup, pstStatus->eWashCupState[pstStatus->u8WashDiskHandCup]);
            }
        }
        else
        {
            //DebugMsg("MR_State:%d,%d,%d,%d\r\n", pstStatus->u8WashDiskHandCup, pstStatus->eMeasureRoomState, pstStatus->eHandCupBusy, pstStatus->eWashCupState[pstStatus->u8WashDiskHandCup]);
        }
    }

    return s32Result;
}

/****************************************************************************
 *
 * Function Name:    TestProc_MoveMeasureRoomCup
 * Input:            pstState - 模块的状态指针
 *                   
 * Output:           None
 *                   
 * Returns:          None
 * Description:      
 * Note:             
 ***************************************************************************/
static int32_t TestProc_MoveMeasureRoomCup(TESTPROC_STATUS_T *pstStatus)
{
    int32_t s32Result = TESTPROC_RESULT_OK;
    
    // 测量室反应杯处理
    if (TESTPROC_MEASUREROOM_STATE_CUPIN == pstStatus->eMeasureRoomState) // 新反应杯进入,关门测量
    {
        s32Result = TestProc_MeasureRoomStartWork(MEASUREROOM_WORK_CLOSE, NULL, 0);
        if (TESTPROC_RESULT_OK == s32Result)
        {
            pstStatus->eMeasureRoomState = TESTPROC_MEASUREROOM_STATE_CLOSE_ING;
        }
        //DebugMsg("_MR_Close:%d,%d\r\n", s32Result, pstStatus->eMeasureRoomState);
    }
    else if (TESTPROC_MEASUREROOM_STATE_READY == pstStatus->eMeasureRoomState) // 关门成功,准备测量
    {
        s32Result = TestProc_MeasureRoomStartWork(MEASUREROOM_WORK_MEASURE, NULL, 0);
        //DebugMsg("_MR_Measure:%d\r\n", s32Result);
        if (TESTPROC_RESULT_OK == s32Result)
        {
            pstStatus->eMeasureRoomState = TESTPROC_MEASUREROOM_STATE_ING;
            pstStatus->u32MeasureValue[0] = 0;
            pstStatus->u32MeasureValue[1] = 0;
        }
    }
    else if (TESTPROC_MEASUREROOM_STATE_OVER == pstStatus->eMeasureRoomState)
    {
        s32Result = TestProc_MeasureRoomStartWork(MEASUREROOM_WORK_FULLOPEN, NULL, 0);
        pstStatus->eMeasureRoomState = TESTPROC_MEASUREROOM_STATE_FULLOPENING;
        //DebugMsg("_MR_FOpen:%d\r\n", s32Result);
    }
    else if (TESTPROC_MEASUREROOM_STATE_WAITCATCH == pstStatus->eMeasureRoomState)
    {
        if (TESTPROC_HANDCUP_STATE_IDLE == pstStatus->eHandCupBusy)
        {
            pstStatus->eMeasureRoomState = TESTPROC_MEASUREROOM_STATE_ABANDON_ING;
            s32Result = TestProc_HandCupStartWork(HANDCUP_WORK_MEASUREROOM_ABANDON, &pstStatus->u8GarbageNO, 1);
            pstStatus->eHandCupBusy = TESTPROC_HANDCUP_STATE_BUSY;
            //DebugMsg("_MR_Abandon:%d,%d,%d\r\n", s32Result, pstStatus->eMeasureRoomState, pstStatus->eHandCupBusy);
        }
    }

    return s32Result;
}

/****************************************************************************
*
* Function Name:    TestProc_AbandonCupFSM
* Input:            pstStatus - 系统状态
*                   
* Output:           None
*                   
* Returns:          TESTPROC_RESULT_OK
* Description:      抛杯流程
* Note:             
***************************************************************************/
static int32_t TestProc_AbandonCupFSM(TESTPROC_STATUS_T *pstStatus)
{
    //OS_ERR err;
    LH_ERR errCode = LH_ERR_NONE;
    TESTPROC_API_RESULT_E eCheckResult = TESTPROC_API_RESULT_OK;
    FSM_ABANDONCUP_E eFSM_Old = FSM_AbandonCup;
    //ACTION_SENSOR_STATUS eCupExist;
    uint8_t  u8BufTmp[4];
    static uint16_t u16LoopCount = 0;
    
    //DebugMsg("FSM_AC:%d\r\n", FSM_AbandonCup);
    switch (FSM_AbandonCup)
    {
        case FSM_ABANDONCUP_HANDCUP_CLEAN:
            DebugMsg("HandCup Clean\r\n");
            errCode = TestProc_Api_HandCupMoveToGarbage(TSC_STACK_MANAGE_GARBAGE_NO1);
            if (errCode == LH_ERR_NONE)
            {
                FSM_AbandonCup = FSM_ABANDONCUP_MR_OPEN;
            }
            else
            {
                FSM_AbandonCup = FSM_ABANDONCUP_IDLE;
            }
            break;
        case FSM_ABANDONCUP_MR_OPEN:
            eCheckResult = TestProc_Api_HandCupCheckResult(&errCode);
            if (TESTPROC_API_RESULT_OK == eCheckResult)
            {
                DebugMsg("Abandon MeasureRoom\r\n");
                errCode = TestProc_Api_WashMeasureRoomDoorOperation(TESTPROC_API_DOOR_OP_FULLOPEN);
                if (errCode == LH_ERR_NONE)
                {
                    FSM_AbandonCup = FSM_ABANDONCUP_MR_OPEN_ING;
                }
                else
                {
                    FSM_AbandonCup = FSM_ABANDONCUP_IDLE;
                }
            }
            else if (TESTPROC_API_RESULT_NG == eCheckResult)
            {
                FSM_AbandonCup = FSM_ABANDONCUP_IDLE;
            }
            break;
        case FSM_ABANDONCUP_MR_OPEN_ING:
            eCheckResult = TestProc_Api_WashMeasureRoomCheckResult(&errCode);
            if (TESTPROC_API_RESULT_OK == eCheckResult)
            {
                errCode = TestProc_Api_HandCupMoveToTop(TESTPROC_API_HAND_TARGET_MEASUREROOM);
                if (errCode == LH_ERR_NONE)
                {
                    FSM_AbandonCup = FSM_ABANDONCUP_MR_COME_ING;
                }
                else
                {
                    FSM_AbandonCup = FSM_ABANDONCUP_IDLE;
                }
            }
            else if (TESTPROC_API_RESULT_NG == eCheckResult)
            {
                FSM_AbandonCup = FSM_ABANDONCUP_IDLE;
            }
            break;
        case FSM_ABANDONCUP_MR_COME_ING:
            eCheckResult = TestProc_Api_HandCupCheckResult(&errCode);
            if (TESTPROC_API_RESULT_OK == eCheckResult)
            {
                errCode = TestProc_Api_HandCupCatchCup(TESTPROC_API_HAND_TARGET_MEASUREROOM);
                if (errCode == LH_ERR_NONE)
                {
                    FSM_AbandonCup = FSM_ABANDONCUP_MR_CATCH_ING;
                }
                else
                {
                    FSM_AbandonCup = FSM_ABANDONCUP_IDLE;
                }
            }
            else if (TESTPROC_API_RESULT_NG == eCheckResult)
            {
                FSM_AbandonCup = FSM_ABANDONCUP_IDLE;
            }
            break;
        case FSM_ABANDONCUP_MR_CATCH_ING:
            eCheckResult = TestProc_Api_HandCupCheckResult(&errCode);
            if (TESTPROC_API_RESULT_OK == eCheckResult)
            {
                errCode = TestProc_Api_HandCupMoveToGarbage(TSC_STACK_MANAGE_GARBAGE_NO1);
                if (errCode == LH_ERR_NONE)
                {
                    FSM_AbandonCup = FSM_ABANDONCUP_MR_ANABDON_ING;
                }
                else
                {
                    FSM_AbandonCup = FSM_ABANDONCUP_IDLE;
                }
            }
            else if (TESTPROC_API_RESULT_NG == eCheckResult)
            {
                FSM_AbandonCup = FSM_ABANDONCUP_IDLE;
            }
            break;
        case FSM_ABANDONCUP_MR_ANABDON_ING:
            eCheckResult = TestProc_Api_HandCupCheckResult(&errCode);
            if (TESTPROC_API_RESULT_OK == eCheckResult)
            {
                DebugMsg("AbandonMR_Cup_OK\r\n");
                errCode = TestProc_Api_WashMeasureRoomDoorOperation(TESTPROC_API_DOOR_OP_CLOSE);
                if (errCode == LH_ERR_NONE)
                {
                    FSM_AbandonCup = FSM_ABANDONCUP_MR_CLOSE_ING;
                }
                else
                {
                    FSM_AbandonCup = FSM_ABANDONCUP_IDLE;
                }
            }
            else if (TESTPROC_API_RESULT_NG == eCheckResult)
            {
                FSM_AbandonCup = FSM_ABANDONCUP_IDLE;
            }
            break;
        case FSM_ABANDONCUP_MR_CLOSE_ING:
            eCheckResult = TestProc_Api_HandCupCheckResult(&errCode);
            if (TESTPROC_API_RESULT_OK == eCheckResult)
            {
                FSM_AbandonCup = FSM_ABANDONCUP_WASH_CHECH;
                u16LoopCount = 0;
            }
            else if (TESTPROC_API_RESULT_NG == eCheckResult)
            {
                FSM_AbandonCup = FSM_ABANDONCUP_IDLE;
            }
            break;
        case FSM_ABANDONCUP_WASH_CHECH:
            DebugMsg("AbandonWash:%d,%d\r\n", u16LoopCount);
            if (TestProc_Api_WashDiskIsCupIn())
            {
                DebugMsg("  WashMoveTop\r\n");
                errCode = TestProc_Api_HandCupMoveToTop(TESTPROC_API_HAND_TARGET_WASH);
                if (errCode == LH_ERR_NONE)
                {
                    FSM_AbandonCup = FSM_ABANDONCUP_WASH_COME_ING;
                }
                else
                {
                    FSM_AbandonCup = FSM_ABANDONCUP_IDLE;
                }
            }
            else
            {
                FSM_AbandonCup = FSM_ABANDONCUP_WASH_ROTATE;
            }
            break;
        case FSM_ABANDONCUP_WASH_COME_ING:
            eCheckResult = TestProc_Api_HandCupCheckResult(&errCode);
            if (TESTPROC_API_RESULT_OK == eCheckResult)
            {
                DebugMsg("  WashCatchCup\r\n");
                errCode = TestProc_Api_HandCupCatchCup(TESTPROC_API_HAND_TARGET_WASH);
                if (errCode == LH_ERR_NONE)
                {
                    FSM_AbandonCup = FSM_ABANDONCUP_WASH_CATCH_ING;
                }
                else
                {
                    FSM_AbandonCup = FSM_ABANDONCUP_IDLE;
                }
            }
            else if (TESTPROC_API_RESULT_NG == eCheckResult)
            {
                FSM_AbandonCup = FSM_ABANDONCUP_IDLE;
            }
            break;
        case FSM_ABANDONCUP_WASH_CATCH_ING:
            eCheckResult = TestProc_Api_HandCupCheckResult(&errCode);
            if (TESTPROC_API_RESULT_OK == eCheckResult)
            {
                DebugMsg("  WashMoveGarbage\r\n");
                errCode = TestProc_Api_HandCupMoveToGarbage(TSC_STACK_MANAGE_GARBAGE_NO1);
                if (errCode == LH_ERR_NONE)
                {
                    FSM_AbandonCup = FSM_ABANDONCUP_WASH_ANABDON_ING;
                }
                else
                {
                    FSM_AbandonCup = FSM_ABANDONCUP_IDLE;
                }
            }
            else if (TESTPROC_API_RESULT_NG == eCheckResult)
            {
                FSM_AbandonCup = FSM_ABANDONCUP_IDLE;
            }
            break;
        case FSM_ABANDONCUP_WASH_ANABDON_ING:
            eCheckResult = TestProc_Api_HandCupCheckResult(&errCode);
            if (TESTPROC_API_RESULT_OK == eCheckResult)
            {
                FSM_AbandonCup = FSM_ABANDONCUP_WASH_ROTATE;
            }
            else if (TESTPROC_API_RESULT_NG == eCheckResult)
            {
                FSM_AbandonCup = FSM_ABANDONCUP_IDLE;
            }
            break;
        case FSM_ABANDONCUP_WASH_ROTATE:
            u16LoopCount++;
            DebugMsg("AbandonWashCup:%d\r\n", u16LoopCount);
            if (u16LoopCount < TESTPROC_WASHDISK_POS_QUANTITY)
            {
                errCode = TestProc_Api_WashDiskRotate1Pos();
                if (errCode == LH_ERR_NONE)
                {
                    FSM_AbandonCup = FSM_ABANDONCUP_WASH_ROTATE_ING;
                }
                else
                {
                    FSM_AbandonCup = FSM_ABANDONCUP_IDLE;
                }
            }
            else
            {
                // WashDisk Abandon Is Finish
                u16LoopCount = 0;
                FSM_AbandonCup = FSM_ABANDONCUP_REACT_CHECH;
            }
            break;
        case FSM_ABANDONCUP_WASH_ROTATE_ING:
            eCheckResult = TestProc_Api_WashDiskCheckResult(&errCode);
            if (TESTPROC_API_RESULT_OK == eCheckResult)
            {
                FSM_AbandonCup = FSM_ABANDONCUP_WASH_CHECH;
            }
            else if (TESTPROC_API_RESULT_NG == eCheckResult)
            {
                FSM_AbandonCup = FSM_ABANDONCUP_IDLE;
            }
            break;
        case FSM_ABANDONCUP_REACT_CHECH:
            DebugMsg("AbandonReact:%d,%d\r\n", TSC_DishReactionCheckCupExist(), u16LoopCount);
            if (TestProc_Api_ReactIsCupIn())
            {
                errCode = TestProc_Api_HandCupMoveToTop(TESTPROC_API_HAND_TARGET_REACT);
                if (errCode == LH_ERR_NONE)
                {
                    FSM_AbandonCup = FSM_ABANDONCUP_REACT_COME_ING;
                }
                else
                {
                    FSM_AbandonCup = FSM_ABANDONCUP_IDLE;
                }
            }
            else
            {
                FSM_AbandonCup = FSM_ABANDONCUP_REACT_ROTATE;
            }
            break;
        case FSM_ABANDONCUP_REACT_COME_ING:
            eCheckResult = TestProc_Api_HandCupCheckResult(&errCode);
            if (TESTPROC_API_RESULT_OK == eCheckResult)
            {
                errCode = TestProc_Api_HandCupCatchCup(TESTPROC_API_HAND_TARGET_REACT);
                if (errCode == LH_ERR_NONE)
                {
                    FSM_AbandonCup = FSM_ABANDONCUP_REACT_CATCH_ING;
                }
                else
                {
                    FSM_AbandonCup = FSM_ABANDONCUP_IDLE;
                }
            }
            else if (TESTPROC_API_RESULT_NG == eCheckResult)
            {
                FSM_AbandonCup = FSM_ABANDONCUP_IDLE;
            }
            break;
        case FSM_ABANDONCUP_REACT_CATCH_ING:
            eCheckResult = TestProc_Api_HandCupCheckResult(&errCode);
            if (TESTPROC_API_RESULT_OK == eCheckResult)
            {
                errCode = TestProc_Api_HandCupMoveToGarbage(TSC_STACK_MANAGE_GARBAGE_NO1);
                if (errCode == LH_ERR_NONE)
                {
                    FSM_AbandonCup = FSM_ABANDONCUP_REACT_ANABDON_ING;
                }
                else
                {
                    FSM_AbandonCup = FSM_ABANDONCUP_IDLE;
                }
            }
            else if (TESTPROC_API_RESULT_NG == eCheckResult)
            {
                FSM_AbandonCup = FSM_ABANDONCUP_IDLE;
            }
            break;
        case FSM_ABANDONCUP_REACT_ANABDON_ING:
            eCheckResult = TestProc_Api_HandCupCheckResult(&errCode);
            if (TESTPROC_API_RESULT_OK == eCheckResult)
            {
                FSM_AbandonCup = FSM_ABANDONCUP_REACT_ROTATE;
            }
            else if (TESTPROC_API_RESULT_NG == eCheckResult)
            {
                FSM_AbandonCup = FSM_ABANDONCUP_IDLE;
            }
            break;
        case FSM_ABANDONCUP_REACT_ROTATE:
            u16LoopCount++;
            DebugMsg("AbandonReactCup:%d\r\n", u16LoopCount);
            if (u16LoopCount < TESTPROC_CUP_QUANTITY)
            {
                errCode = TestProc_Api_ReactRotatePos(1);
                if (errCode == LH_ERR_NONE)
                {
                    FSM_AbandonCup = FSM_ABANDONCUP_REACT_ROTATE_ING;
                }
                else
                {
                    FSM_AbandonCup = FSM_ABANDONCUP_IDLE;
                }
            }
            else
            {
                // All Cup Abandon Is Finish
                if ((pstStatus->u8WorkBeforeTest&TESTPROC_BEFORE_TEST_ADANDONCUP) > 0)
                {
                    pstStatus->u8WorkBeforeTest &= ~TESTPROC_BEFORE_TEST_ADANDONCUP;
                    errCode = TestProc_Api_HandCupReset();
                    if (errCode == LH_ERR_NONE)
                    {
                        FSM_AbandonCup = FSM_ABANDONCUP_HANDCUP_RESET_ING;
                    }
                    else
                    {
                        FSM_AbandonCup = FSM_ABANDONCUP_IDLE;
                    }
                }
                else
                {                    
                    TestProc_Standby(pstStatus);
                    FSM_AbandonCup = FSM_ABANDONCUP_IDLE;
                }
            }
            break;
        case FSM_ABANDONCUP_REACT_ROTATE_ING:
            eCheckResult = TestProc_Api_ReactCheckResult();
            if (TESTPROC_API_RESULT_OK == eCheckResult)
            {
                FSM_AbandonCup = FSM_ABANDONCUP_REACT_CHECH;
            }
            else if (TESTPROC_API_RESULT_NG == eCheckResult)
            {
                FSM_AbandonCup = FSM_ABANDONCUP_IDLE;
            }
            break;
        case FSM_ABANDONCUP_HANDCUP_RESET_ING:
            eCheckResult = TestProc_Api_HandCupCheckResult(&errCode);
            if (TESTPROC_API_RESULT_OK == eCheckResult)
            {
                u16LoopCount = 0;
                errCode = TestProc_Api_ReactReset();
                if (errCode == LH_ERR_NONE)
                {
                    u16LoopCount |= 0x01;
                    errCode = TestProc_Api_WashDiskReset();
                    if (errCode == LH_ERR_NONE)
                    {
                        u16LoopCount |= 0x02;
                        FSM_AbandonCup = FSM_ABANDONCUP_DISC_RESET_ING;
                    }
                    else
                    {
                        FSM_AbandonCup = FSM_ABANDONCUP_IDLE;
                    }
                }
                else
                {
                    FSM_AbandonCup = FSM_ABANDONCUP_IDLE;
                }
            }
            else if (TESTPROC_API_RESULT_NG == eCheckResult)
            {
                FSM_AbandonCup = FSM_ABANDONCUP_IDLE;
            }            
            break;
        case FSM_ABANDONCUP_DISC_RESET_ING:
            eCheckResult = TestProc_Api_ReactCheckResult();
            if (TestProc_Api_IsResetCompile(eCheckResult))
            {
                u16LoopCount &= 0xFE;
                if (TESTPROC_API_RESULT_NG == eCheckResult)
                {
                    DebugMsg("E>AbandonCupReactReset\r\n");
                }
            }
            eCheckResult = TestProc_Api_WashDiskCheckResult(&errCode);
            if (TestProc_Api_IsResetCompile(eCheckResult))
            {
                u16LoopCount &= 0xFD;
                if (TESTPROC_API_RESULT_NG == eCheckResult)
                {
                    DebugMsg("E>AbandonCupWashDiskReset\r\n");
                }
            }

            if ((u16LoopCount&0x03) == 0)
            {
                DebugMsg("AbandonCupOK,StartTest\r\n");
                TestProc_ReactStartWork(REACT_WORK_TEST, NULL, 0);
                pstStatus->stCellStatus.u8WorkStatus = TESTPROC_WORK_TEST;
                FSM_TestProcess = FSM_TESTPROCESS_TEST;
                pstStatus->eCupStackState = TESTPROC_CUPSTACK_STATE_INIT;
                TestProc_CupStackStartWork(CUPSTACK_WORK_PUSH_TO_POSTION, NULL, 0);
                u8BufTmp[0] = 1;
                TestProc_SendDataToComm(BOARD_ID_TRACK_COMM, CAN_CMD_CELL_START_WORK, u8BufTmp, 1);
            }
            break;
        default:
            
            break;
    }
    
    if (errCode != LH_ERR_NONE || TESTPROC_API_RESULT_NG == eCheckResult)
    {
        DebugMsg("E>AbandonCup:%d,%d,%d,%d\r\n", errCode, eCheckResult, eFSM_Old, FSM_AbandonCup);
    }
    
    return TESTPROC_RESULT_OK;
}

/****************************************************************************
*
* Function Name:    TestProc_RecycleRackFSM
* Input:            pstStatus - 系统状态
*                   
* Output:           None
*                   
* Returns:          TESTPROC_RESULT_OK
* Description:      样本架流程
* Note:             
***************************************************************************/
static int32_t TestProc_RecycleRackFSM(TESTPROC_STATUS_T *pstStatus)
{
    LH_ERR errCode = LH_ERR_NONE;
    TESTPROC_API_RESULT_E eCheckResult = TESTPROC_API_RESULT_OK;
    FSM_RECYCLERACK_E eFSM_Old = FSM_RecycleRack;
    uint8_t  u8BufTmp[4];

    switch (FSM_RecycleRack)
    {
        case FSM_RECYCLERACK_TEST:
            DebugMsg("RecycleRackTest\r\n");
            errCode = TestProc_Api_SampleConveyorBeltMoveToPos(6);
            if (LH_ERR_NONE == errCode)
            {
                FSM_RecycleRack = FSM_RECYCLERACK_TEST_MOVE_ING;
            }
            else
            {
                FSM_RecycleRack = FSM_RECYCLERACK_IDLE;
            }
            break;
        case FSM_RECYCLERACK_TEST_MOVE_ING:
            eCheckResult = TestProc_Api_SampleConveyorBeltCheckResult(&errCode);
            if (TESTPROC_API_RESULT_OK == eCheckResult)
            {
                DebugMsg("RecycleRackTest_OK\r\n");
                TestProc_SendDataToComm(BOARD_ID_TRACK_COMM, CAN_CMD_COMM_RACKCHANGE, NULL, 0);
                FSM_RecycleRack = FSM_RECYCLERACK_IDLE;
            }
            else if (TESTPROC_API_RESULT_NG == eCheckResult)
            {
                FSM_RecycleRack = FSM_RECYCLERACK_IDLE;
            }
            
            if (FSM_RECYCLERACK_IDLE == FSM_RecycleRack)
            {
                if ((pstStatus->u8WorkBeforeTest&TESTPROC_BEFORE_TEST_RECYCLERACK) > 0)
                {
                    if ((pstStatus->u8WorkBeforeTest&TESTPROC_BEFORE_TEST_ADANDONCUP)> 0)
                    {
                        FSM_TestProcess = FSM_TESTPROCESS_ABANDON_CUP;
                        FSM_AbandonCup = FSM_ABANDONCUP_HANDCUP_CLEAN;
                        pstStatus->stCellStatus.u8WorkStatus = TESTPROC_WORK_ABANDON_CUP;
                    }
                    else
                    {
                        FSM_TestProcess = FSM_TESTPROCESS_TEST;
                        pstStatus->u8WorkBeforeTest &= ~TESTPROC_BEFORE_TEST_RECYCLERACK;
                        DebugMsg("RecycleRackOK,StartTest\r\n");
                        TestProc_ReactStartWork(REACT_WORK_TEST, NULL, 0);
                        pstStatus->stCellStatus.u8WorkStatus = TESTPROC_WORK_TEST;
                        pstStatus->eCupStackState = TESTPROC_CUPSTACK_STATE_INIT;
                        TestProc_CupStackStartWork(CUPSTACK_WORK_PUSH_TO_POSTION, NULL, 0);
                        u8BufTmp[0] = 1;
                        TestProc_SendDataToComm(BOARD_ID_TRACK_COMM, CAN_CMD_CELL_START_WORK, u8BufTmp, 1);
                    }
                }
            }
            break;
        default:
            
            break;
    }
    
    if (errCode != LH_ERR_NONE || TESTPROC_API_RESULT_NG == eCheckResult)
    {
        DebugMsg("E>RecycleRack:%d,%d,%d,%d\r\n", errCode, eCheckResult, eFSM_Old, FSM_AbandonCup);
    }
    
    return TESTPROC_RESULT_OK;
}

/****************************************************************************
*
* Function Name:    TestProc_ReagentBarCodeFSM
* Input:            pstStatus - 系统状态
*                   
* Output:           None
*                   
* Returns:          TESTPROC_RESULT_OK
* Description:      扫描试剂条码
* Note:             
***************************************************************************/
static int32_t TestProc_ReagentBarCodeFSM(TESTPROC_STATUS_T *pstStatus)
{
    uint8_t *pu8DynamicBuf;
    uint8_t u8Loop;
    static uint8_t u8Postion = 0;
    LH_ERR errCode = LH_ERR_NONE;
    TESTPROC_API_RESULT_E eCheckResult = TESTPROC_API_RESULT_OK;
    FSM_REAGENTBARCODE_E eFSM_Old = FSM_ReagentBarCode;
    
    switch (FSM_ReagentBarCode)
    {
        case FSM_REAGENTBARCODE_START:
            u8Postion = 0;
            FSM_ReagentBarCode = FSM_REAGENTBARCODE_NEXT_POS;
            break;
        case FSM_REAGENTBARCODE_MOVE:
            errCode = TestProc_Api_ReagentDiskMoveToPos(u8Postion + 1, DISH_REAGENT_OFFSET_SCAN);
            if (LH_ERR_NONE == errCode)
            {
                FSM_ReagentBarCode = FSM_REAGENTBARCODE_MOVE_ING;
            }
            else
            {
                FSM_ReagentBarCode = FSM_REAGENTBARCODE_IDLE;
            }
            break;
        case FSM_REAGENTBARCODE_MOVE_ING:
            eCheckResult = TestProc_Api_ReagentDiskCheckResult(&errCode);
            if (TESTPROC_API_RESULT_OK == eCheckResult)
            {
                errCode = TestProc_Api_ReagentDiskBarScan();
                DebugMsg("  Scaning:0x%X\r\n", errCode);
                if (LH_ERR_NONE == errCode)
                {
                    FSM_ReagentBarCode = FSM_REAGENTBARCODE_SCAN_ING;
                }
                else
                {
                    FSM_ReagentBarCode = FSM_REAGENTBARCODE_IDLE;
                }
            }
            else if (TESTPROC_API_RESULT_NG == eCheckResult)
            {
                FSM_ReagentBarCode = FSM_REAGENTBARCODE_IDLE;
            }
            break;
        case FSM_REAGENTBARCODE_SCAN_ING:
            eCheckResult = TestProc_Api_ReagentDiskBarCheckResult();
            if (TESTPROC_API_RESULT_OK == eCheckResult)
            {
                FSM_ReagentBarCode = FSM_REAGENTBARCODE_NEXT_POS;
                // Send Code
                pu8DynamicBuf = TestProc_AllocMemory(TESTPROC_REAGENT_CODE_SIZE + 4);
                if (pu8DynamicBuf != NULL)
                {
                    uint8_t *pu8Code = NULL;
                    uint16_t u16ReadLen;
                    uint16_t u16Index = 0;
                    
                    pu8DynamicBuf[u16Index++] = SystemConfig_GetCellNumber() + 1;
                    pu8DynamicBuf[u16Index++] = 1;
                    pu8DynamicBuf[u16Index++] = u8Postion + 1;
                    Mem_Clr(&pu8DynamicBuf[u16Index], TESTPROC_REAGENT_CODE_SIZE);
                    u16ReadLen = TestProc_Api_ReagentDiskBarReadCode(&pu8Code);
                    if (u16ReadLen > TESTPROC_REAGENT_CODE_SIZE)
                        u16ReadLen = TESTPROC_REAGENT_CODE_SIZE;
                    DebugMsg("  ReadReagentCode:%d,%d,%p\r\n", u8Postion + 1, u16ReadLen, pu8Code);
                    if (pu8Code != NULL)
                    {
                        Mem_Copy(&pu8DynamicBuf[u16Index], pu8Code, u16ReadLen);
                        u16Index += TESTPROC_REAGENT_CODE_SIZE;
                        TestProc_SendDataToComm(BOARD_ID_TRACK_COMM, CAN_CMD_CELL_REAGENTBARCODE, pu8DynamicBuf, u16Index);
                    }
                    u8Postion++;
                }
                else
                {
                    DebugMsg("E>ReagentBarCode_MemoryOut:%d\r\n", u8Postion + 1);
                }
            }
            else if (TESTPROC_API_RESULT_NG == eCheckResult)
            {
                FSM_ReagentBarCode = FSM_REAGENTBARCODE_IDLE;
            }
            break;
        case FSM_REAGENTBARCODE_NEXT_POS:
            for (u8Loop = u8Postion; u8Loop < TESTPROC_REAGENT_POS_MAX; u8Loop++)
            {
                u8Postion = u8Loop;
                if (0x01 == g_u8ReagentScanPos[u8Loop])
                {
                    break;
                }
            }
            if (u8Postion >= TESTPROC_REAGENT_POS_CONFIG)
            {
                DebugMsg("ReagentBarCode Finish\r\n");
                FSM_ReagentBarCode = FSM_REAGENTBARCODE_IDLE;
                TestProc_Standby(pstStatus);
            }
            else
            {
                FSM_ReagentBarCode = FSM_REAGENTBARCODE_MOVE;
                DebugMsg("ScanPos:%d\r\n", u8Postion + 1);
            }
            break;
        default:
            eCheckResult = TESTPROC_API_RESULT_NG;
            break;
    }
    
    if (errCode != LH_ERR_NONE || TESTPROC_API_RESULT_NG == eCheckResult)
    {
        DebugMsg("E>ReagentBarCode:%d,%d,%d,%d\r\n", errCode, eCheckResult, eFSM_Old, FSM_ReagentBarCode);
    }
    
    return TESTPROC_RESULT_OK;
}

/****************************************************************************
*
* Function Name:    TestProc_ReagentQuantityFSM
* Input:            pstStatus - 系统状态
*                   
* Output:           None
*                   
* Returns:          TESTPROC_RESULT_OK
* Description:      扫描试剂余量
* Note:             
***************************************************************************/
static int32_t TestProc_ReagentQuantityFSM(TESTPROC_STATUS_T *pstStatus)
{
    //uint8_t *pu8DynamicBuf;
    static uint16_t u16WorkFinishMask = 0;
    uint8_t u8Loop;
    static uint8_t u8Postion = 0;
    LH_ERR errCode = LH_ERR_NONE;
    LH_ERR funcErrCode = LH_ERR_NONE;
    TESTPROC_API_RESULT_E eCheckResult = TESTPROC_API_RESULT_OK;
    FSM_REAGENTQUANTITY_E eFSM_Old = FSM_ReagentQuantity;
    
    switch (FSM_ReagentQuantity)
    {
        case FSM_REAGENTQUANTITY_START:
            u8Postion = 0;
            FSM_ReagentQuantity = FSM_REAGENTQUANTITY_NEXT_POS;
            Mem_Set(&g_u16ReagentLiquitLevel[0], 0xFF, sizeof(g_u16ReagentLiquitLevel));
            break;
        case FSM_REAGENTQUANTITY_R1_MOVE:
            errCode = TestProc_Api_ReagentDiskMoveToPos(u8Postion + 1, DISH_REAGENT_OFFSET_R1);
            if (LH_ERR_NONE == errCode)
            {
                errCode = TestProc_Api_ReagentMoveToPos(TSC_NEEDLE_REAGENT_BEAD_POS_R1);
                if (LH_ERR_NONE == errCode)
                {
                    FSM_ReagentQuantity = FSM_REAGENTQUANTITY_R1_MOVE_ING;
                    u16WorkFinishMask = 0x03;
                }
                else
                {
                    FSM_ReagentQuantity = FSM_REAGENTQUANTITY_IDLE;
                }
            }
            else
            {
                FSM_ReagentQuantity = FSM_REAGENTQUANTITY_IDLE;
            }
            break;
        case FSM_REAGENTQUANTITY_R1_MOVE_ING:
            if (u16WorkFinishMask & 0x01) // Reagent Disk
            {
                eCheckResult = TestProc_Api_ReagentDiskCheckResult(&errCode);
                if (TESTPROC_API_RESULT_OK == eCheckResult)
                {
                    u16WorkFinishMask &= 0xFE;
                }
            }
            if (u16WorkFinishMask & 0x02) // Reagent Probe
            {
                eCheckResult = TestProc_Api_ReagentCheckResult(&funcErrCode);
                if (TESTPROC_API_RESULT_OK == eCheckResult)
                {
                    u16WorkFinishMask &= 0xFD;
                }
            }
            if (0 == u16WorkFinishMask)
            {
                errCode = TestProc_Api_ReagentLevelDetect();
                DebugMsg("  R1_Detecting:0x%X\r\n", errCode);
                if (LH_ERR_NONE == errCode)
                {
                    FSM_ReagentQuantity = FSM_REAGENTQUANTITY_R1_DETECT_ING;
                }
                else
                {
                    FSM_ReagentQuantity = FSM_REAGENTQUANTITY_IDLE;
                }
            }
            else if (TESTPROC_API_RESULT_NG == eCheckResult)
            {
                FSM_ReagentQuantity = FSM_REAGENTQUANTITY_IDLE;
            }
            break;
        case FSM_REAGENTQUANTITY_R1_DETECT_ING:
            eCheckResult = TestProc_Api_ReagentCheckResult(&funcErrCode);
            if (TESTPROC_API_RESULT_OK == eCheckResult)
            {
                errCode = TestProc_Api_ReagentLevelRead(&g_u16ReagentLiquitLevel[u8Postion * 3]);
                if (LH_ERR_NONE == errCode)
                {
                    errCode = TestProc_Api_ReagentDiskMoveToPos(u8Postion + 1, DISH_REAGENT_OFFSET_R2);
                    if (LH_ERR_NONE == errCode)
                    {
                        errCode = TestProc_Api_ReagentMoveToPos(TSC_NEEDLE_REAGENT_BEAD_POS_R2);
                        if (LH_ERR_NONE == errCode)
                        {
                            FSM_ReagentQuantity = FSM_REAGENTQUANTITY_R2_MOVE_ING;
                            u16WorkFinishMask = 0x03;
                        }
                        else
                        {
                            FSM_ReagentQuantity = FSM_REAGENTQUANTITY_IDLE;
                        }
                    }
                    else
                    {
                        FSM_ReagentQuantity = FSM_REAGENTQUANTITY_IDLE;
                    }
                }
                else
                {
                    FSM_ReagentQuantity = FSM_REAGENTQUANTITY_IDLE;
                }
            }
            else if (TESTPROC_API_RESULT_NG == eCheckResult)
            {
                FSM_ReagentQuantity = FSM_REAGENTQUANTITY_IDLE;
            }
            break;
        case FSM_REAGENTQUANTITY_R2_MOVE_ING:
            if (u16WorkFinishMask & 0x01) // Reagent Disk
            {
                eCheckResult = TestProc_Api_ReagentDiskCheckResult(&errCode);
                if (TESTPROC_API_RESULT_OK == eCheckResult)
                {
                    u16WorkFinishMask &= 0xFE;
                }
            }
            if (u16WorkFinishMask & 0x02) // Reagent Probe
            {
                eCheckResult = TestProc_Api_ReagentCheckResult(&funcErrCode);
                if (TESTPROC_API_RESULT_OK == eCheckResult)
                {
                    u16WorkFinishMask &= 0xFD;
                }
            }
            if (0 == u16WorkFinishMask)
            {
                errCode = TestProc_Api_ReagentLevelDetect();
                DebugMsg("  R2_Detecting:0x%X\r\n", errCode);
                if (LH_ERR_NONE == errCode)
                {
                    FSM_ReagentQuantity = FSM_REAGENTQUANTITY_R2_DETECT_ING;
                }
                else
                {
                    FSM_ReagentQuantity = FSM_REAGENTQUANTITY_IDLE;
                }
            }
            else if (TESTPROC_API_RESULT_NG == eCheckResult)
            {
                FSM_ReagentQuantity = FSM_REAGENTQUANTITY_IDLE;
            }
            break;
        case FSM_REAGENTQUANTITY_R2_DETECT_ING:
            eCheckResult = TestProc_Api_ReagentCheckResult(&funcErrCode);
            if (TESTPROC_API_RESULT_OK == eCheckResult)
            {
                errCode = TestProc_Api_ReagentLevelRead(&g_u16ReagentLiquitLevel[u8Postion * 3 + 1]);
                if (LH_ERR_NONE == errCode)
                {
                    errCode = TestProc_Api_ReagentDiskMoveToPos(u8Postion + 1, DISH_REAGENT_OFFSET_BEAD);
                    if (LH_ERR_NONE == errCode)
                    {
                        errCode = TestProc_Api_ReagentMoveToPos(TSC_NEEDLE_REAGENT_BEAD_POS_BEAD);
                        if (LH_ERR_NONE == errCode)
                        {
                            FSM_ReagentQuantity = FSM_REAGENTQUANTITY_M_MOVE_ING;
                            u16WorkFinishMask = 0x03;
                        }
                        else
                        {
                            FSM_ReagentQuantity = FSM_REAGENTQUANTITY_IDLE;
                        }
                    }
                    else
                    {
                        FSM_ReagentQuantity = FSM_REAGENTQUANTITY_IDLE;
                    }
                }
                else
                {
                    FSM_ReagentQuantity = FSM_REAGENTQUANTITY_IDLE;
                }
            }
            else if (TESTPROC_API_RESULT_NG == eCheckResult)
            {
                FSM_ReagentQuantity = FSM_REAGENTQUANTITY_IDLE;
            }
            break;
        case FSM_REAGENTQUANTITY_M_MOVE_ING:
            if (u16WorkFinishMask & 0x01) // Reagent Disk
            {
                eCheckResult = TestProc_Api_ReagentDiskCheckResult(&errCode);
                if (TESTPROC_API_RESULT_OK == eCheckResult)
                {
                    u16WorkFinishMask &= 0xFE;
                }
            }
            if (u16WorkFinishMask & 0x02) // Reagent Probe
            {
                eCheckResult = TestProc_Api_ReagentCheckResult(&funcErrCode);
                if (TESTPROC_API_RESULT_OK == eCheckResult)
                {
                    u16WorkFinishMask &= 0xFD;
                }
            }
            if (0 == u16WorkFinishMask)
            {
                errCode = TestProc_Api_ReagentLevelDetect();
                DebugMsg("  M_Detecting:0x%X\r\n", errCode);
                if (LH_ERR_NONE == errCode)
                {
                    FSM_ReagentQuantity = FSM_REAGENTQUANTITY_M_DETECT_ING;
                }
                else
                {
                    FSM_ReagentQuantity = FSM_REAGENTQUANTITY_IDLE;
                }
            }
            else if (TESTPROC_API_RESULT_NG == eCheckResult)
            {
                FSM_ReagentQuantity = FSM_REAGENTQUANTITY_IDLE;
            }
            break;
        case FSM_REAGENTQUANTITY_M_DETECT_ING:
            eCheckResult = TestProc_Api_ReagentCheckResult(&funcErrCode);
            if (TESTPROC_API_RESULT_OK == eCheckResult)
            {
                errCode = TestProc_Api_ReagentLevelRead(&g_u16ReagentLiquitLevel[u8Postion * 3 + 2]);
                if (LH_ERR_NONE == errCode)
                {
                    FSM_ReagentQuantity = FSM_REAGENTQUANTITY_NEXT_POS;
                    u8Postion++;
                }
                else
                {
                    FSM_ReagentQuantity = FSM_REAGENTQUANTITY_IDLE;
                }
            }
            else if (TESTPROC_API_RESULT_NG == eCheckResult)
            {
                FSM_ReagentQuantity = FSM_REAGENTQUANTITY_IDLE;
            }
            break;
        case FSM_REAGENTQUANTITY_NEXT_POS:
            for (u8Loop = u8Postion; u8Loop < TESTPROC_REAGENT_POS_MAX; u8Loop++)
            {
                u8Postion = u8Loop;
                if (0x01 == g_u8ReagentScanPos[u8Loop])
                {
                    break;
                }
            }
            if (u8Postion >= TESTPROC_REAGENT_POS_CONFIG)
            {
                DebugMsg("ReagentQuantity Finish\r\n");
                FSM_ReagentQuantity = FSM_REAGENTQUANTITY_IDLE;
                TestProc_SendDataToComm(BOARD_ID_TRACK_COMM, CAN_CMD_CELL_REAGENTQUANTITY, (uint8_t *)&g_u16ReagentLiquitLevel[0], sizeof(g_u16ReagentLiquitLevel));
                TestProc_Standby(pstStatus);
            }
            else
            {
                FSM_ReagentQuantity = FSM_REAGENTQUANTITY_R1_MOVE;
                DebugMsg("R1_DetectPos:%d\r\n", u8Postion + 1);
            }
            break;
        default:
            eCheckResult = TESTPROC_API_RESULT_NG;
            break;
    }
    
    if (errCode != LH_ERR_NONE || TESTPROC_API_RESULT_NG == eCheckResult)
    {
        DebugMsg("E>ReagentBarCode:%d,%d,%d,%d\r\n", errCode, eCheckResult, eFSM_Old, FSM_ReagentQuantity);
    }
    
    return TESTPROC_RESULT_OK;
}

/****************************************************************************
*
* Function Name:    TestProc_MSeparateAutoFSM
* Input:            pstStatus - 系统状态
*                   
* Output:           None
*                   
* Returns:          TESTPROC_RESULT_OK
* Description:      
* Note:             
***************************************************************************/
static int32_t TestProc_MSeparateAutoFSM(TESTPROC_STATUS_T *pstStatus)
{
    int32_t i;
    int32_t  s32Result = TESTPROC_RESULT_OK;
    TESTPROC_API_RESULT_E eCheckResult = TESTPROC_API_RESULT_OK;
    static FSM_M_SEPARATE_AUTO_E FSM_Old = FSM_M_SEPARATE_AUTO_IDLE;
    LH_ERR errCode = LH_ERR_NONE;
    LH_ERR errCode2 = LH_ERR_NONE;
    static OS_TICK  Tick;
    static uint16_t u16WorkMask = 0;
    static uint16_t u16WashMask = 0;
    static uint8_t  u8WashNeedleCount = 0;
    static uint8_t  u8WashNeedleState = 0;
    TESTPROC_ITEM_RESULT_T stTestResult;
    OS_ERR os_err;
    uint8_t  u8CupID;

    if (FSM_Old != FSM_M_SeparateAuto)
    {
        //DebugMsg("MSAuto:%d\r\n", FSM_M_SeparateAuto);
    }
    FSM_Old = FSM_M_SeparateAuto;
    
    switch(FSM_M_SeparateAuto)
    {
        case FSM_M_SEPARATE_AUTO_START:
            DebugMsg("MSeparateAuto Start\r\n");
            FSM_M_SeparateAuto = FSM_M_SEPARATE_AUTO_WASH_RESET_1;
            break;
        case FSM_M_SEPARATE_AUTO_WASH_RESET_1:
            FSM_M_SeparateAuto = FSM_M_SEPARATE_AUTO_WASH_RESET_2;
            errCode = TestProc_Api_HandCupReset();
            if (LH_ERR_NONE != errCode)
            {
                FSM_M_SeparateAuto = FSM_M_SEPARATE_AUTO_IDLE;
                s32Result = TESTPROC_RESULT_NG;
            }
            break;
        case FSM_M_SEPARATE_AUTO_WASH_RESET_2:
            eCheckResult = TestProc_Api_HandCupCheckResult(&errCode);
            if (TESTPROC_API_RESULT_OK == eCheckResult)
            {
                u16WorkMask = 0;
                eCheckResult = TestProc_WashResetFSM(TRUE);
                if (eCheckResult == TESTPROC_API_RESULT_NG)
                {
                    FSM_M_SeparateAuto = FSM_M_SEPARATE_AUTO_IDLE;
                    s32Result = TESTPROC_RESULT_NG;
                }
                else 
                {
                    u16WorkMask |= 0x01;
                }
                errCode = TestProc_Api_CupStackReset();
                if (LH_ERR_NONE == errCode)
                {
                    u16WorkMask |= 0x02;
                }
                else
                {
                    FSM_M_SeparateAuto = FSM_M_SEPARATE_AUTO_IDLE;
                    s32Result = TESTPROC_RESULT_NG;
                }                
                DebugMsg("AutoReset:%02X,%d,%08X\r\n", u16WorkMask, eCheckResult, errCode);
                if ((u16WorkMask&0x03) == 0x03)
                {
                    FSM_M_SeparateAuto = FSM_M_SEPARATE_AUTO_WASH_RESET_ING;
                }
                else
                {
                    FSM_M_SeparateAuto = FSM_M_SEPARATE_AUTO_IDLE;
                    s32Result = TESTPROC_RESULT_NG;
                }
            }
            else if (TESTPROC_API_RESULT_NG == eCheckResult)
            {
                FSM_M_SeparateAuto = FSM_M_SEPARATE_AUTO_IDLE;
                s32Result = TESTPROC_RESULT_ERR_EXEC;
            }
            break;
        case FSM_M_SEPARATE_AUTO_WASH_RESET_ING:
            if ((u16WorkMask&0x01) > 0)
            {
                eCheckResult = TestProc_WashResetFSM(FALSE);
                if (TESTPROC_API_RESULT_OK == eCheckResult)
                {
                    u16WorkMask &= ~0x01;
                    
                }
                else if (TESTPROC_API_RESULT_NG == eCheckResult)
                {
                    FSM_M_SeparateAuto = FSM_M_SEPARATE_AUTO_IDLE;
                    s32Result = TESTPROC_RESULT_ERR_EXEC;
                }
            }
            if ((u16WorkMask&0x02) > 0)
            {
                eCheckResult = TestProc_Api_CupStackCheckResult(&errCode);
                if (TESTPROC_API_RESULT_OK == eCheckResult)
                {
                    u16WorkMask &= ~0x02;
                    
                }
                else if (TESTPROC_API_RESULT_NG == eCheckResult)
                {
                    FSM_M_SeparateAuto = FSM_M_SEPARATE_AUTO_IDLE;
                    s32Result = TESTPROC_RESULT_ERR_EXEC;
                }
            }
            if (0 == u16WorkMask)
            {
                DebugMsg("MSeparateAoto Reset OK\r\n");
                Tick = OSTimeGet(&os_err);
                FSM_M_SeparateAuto = FSM_M_SEPARATE_AUTO_START_STOP_WORK;
                pstStatus->u8CupStackCol = 0;
                pstStatus->u8CupStackRow = 0;
                pstStatus->u8WashDiskHandCup = 1;
                g_u16MSeparationQuantity += 10;
                u8WashNeedleState = 1;
                u8WashNeedleCount = 0;
                for (i=0; i<=TESTPROC_WASHDISK_POS_QUANTITY; i++)
                {
                    pstStatus->eWashCupState[i] = TESTPROC_WASH_CUP_INIT;
                }
            }
            break;
        case FSM_M_SEPARATE_AUTO_START_STOP_WORK:
            if (pstStatus->eWashCupState[pstStatus->u8WashDiskHandCup] >= TESTPROC_WASH_CUP_MIX)
            {
                DebugMsg("CatchWashCup\r\n");
                errCode = TestProc_Api_HandCupMoveToTop(TESTPROC_API_HAND_TARGET_WASH);
                if (LH_ERR_NONE == errCode)
                {
                    FSM_M_SeparateAuto = FSM_M_SEPARATE_AUTO_TO_WASH_FOR_MR;
                }
                else
                {
                    FSM_M_SeparateAuto = FSM_M_SEPARATE_AUTO_IDLE;
                    s32Result = TESTPROC_RESULT_NG;
                }
            }
            else
            {
                if (g_u16MSeparationQuantity > 10)
                {
                    FSM_M_SeparateAuto = FSM_M_SEPARATE_AUTO_TO_CUPSTACK;
                }
                else
                {
                    FSM_M_SeparateAuto = FSM_M_SEPARATE_AUTO_WASH_STOP;
                }
            }
            break;
        case FSM_M_SEPARATE_AUTO_TO_WASH_FOR_MR:
            eCheckResult = TestProc_Api_HandCupCheckResult(&errCode);
            if (TESTPROC_API_RESULT_OK == eCheckResult)
            {
                errCode = TestProc_Api_HandCupCatchCup(TESTPROC_API_HAND_TARGET_WASH);
                if (LH_ERR_NONE == errCode)
                {
                    FSM_M_SeparateAuto = FSM_M_SEPARATE_AUTO_CATCH_WASH_FOR_MR;
                }
                else
                {
                    FSM_M_SeparateAuto = FSM_M_SEPARATE_AUTO_IDLE;
                    s32Result = TESTPROC_RESULT_NG;
                }
            }
            else if (TESTPROC_API_RESULT_NG == eCheckResult)
            {
                FSM_M_SeparateAuto = FSM_M_SEPARATE_AUTO_IDLE;
                s32Result = TESTPROC_RESULT_ERR_EXEC;
            }
            break;
        case FSM_M_SEPARATE_AUTO_CATCH_WASH_FOR_MR:
            eCheckResult = TestProc_Api_HandCupCheckResult(&errCode);
            if (TESTPROC_API_RESULT_OK == eCheckResult)
            {
                DebugMsg("WashCupPickUp:%d\r\n", pstStatus->u8WashDiskHandCup);
                pstStatus->eWashCupState[pstStatus->u8WashDiskHandCup] = TESTPROC_WASH_CUP_INIT;
                u16WorkMask = 0;
                errCode = TestProc_Api_HandCupMoveToTop(TESTPROC_API_HAND_TARGET_MEASUREROOM);
                if (LH_ERR_NONE == errCode)
                {
                    u16WorkMask |= 0x01;
                    errCode = TestProc_Api_WashMeasureRoomDoorOperation(TESTPROC_API_DOOR_OP_HALFOPEN);
                    if (LH_ERR_NONE == errCode)
                    {
                        u16WorkMask |= 0x02;
                        FSM_M_SeparateAuto = FSM_M_SEPARATE_AUTO_CUP_TO_MR;
                    }
                    else
                    {
                        FSM_M_SeparateAuto = FSM_M_SEPARATE_AUTO_IDLE;
                        s32Result = TESTPROC_RESULT_NG;
                    }
                }
                else
                {
                    FSM_M_SeparateAuto = FSM_M_SEPARATE_AUTO_IDLE;
                    s32Result = TESTPROC_RESULT_NG;
                }
            }
            else if (TESTPROC_API_RESULT_NG == eCheckResult)
            {
                FSM_M_SeparateAuto = FSM_M_SEPARATE_AUTO_IDLE;
                s32Result = TESTPROC_RESULT_ERR_EXEC;
            }
            break;
        case FSM_M_SEPARATE_AUTO_CUP_TO_MR:
            eCheckResult = TestProc_Api_HandCupCheckResult(&errCode);
            if (TESTPROC_API_RESULT_OK == eCheckResult)
            {
                u16WorkMask &= ~0x01;
            }
            else if (TESTPROC_API_RESULT_NG == eCheckResult)
            {
                FSM_M_SeparateAuto = FSM_M_SEPARATE_AUTO_IDLE;
                s32Result = TESTPROC_RESULT_NG;
            }
            eCheckResult = TestProc_Api_WashMeasureRoomCheckResult(&errCode);
            if (TESTPROC_API_RESULT_OK == eCheckResult)
            {
                u16WorkMask &= ~0x02;
            }
            else if (TESTPROC_API_RESULT_NG == eCheckResult)
            {
                FSM_M_SeparateAuto = FSM_M_SEPARATE_AUTO_IDLE;
                s32Result = TESTPROC_RESULT_NG;
            }
            if (0 == u16WorkMask)
            {
                errCode = TestProc_Api_HandCupPutCup(TESTPROC_API_HAND_TARGET_MEASUREROOM);
                if (LH_ERR_NONE == errCode)
                { 
                    FSM_M_SeparateAuto = FSM_M_SEPARATE_AUTO_PUT_TO_MR;
                }
                else
                {
                    FSM_M_SeparateAuto = FSM_M_SEPARATE_AUTO_IDLE;
                    s32Result = TESTPROC_RESULT_NG;
                }
            }
            break;
        case FSM_M_SEPARATE_AUTO_PUT_TO_MR:
            eCheckResult = TestProc_Api_HandCupCheckResult(&errCode);
            if (TESTPROC_API_RESULT_OK == eCheckResult)
            {
                FSM_M_SeparateAuto = FSM_M_SEPARATE_AUTO_MR_CLOSR;
            }
            else if (TESTPROC_API_RESULT_NG == eCheckResult)
            {
                FSM_M_SeparateAuto = FSM_M_SEPARATE_AUTO_IDLE;
                s32Result = TESTPROC_RESULT_ERR_EXEC;
            }
            break;
        case FSM_M_SEPARATE_AUTO_MR_CLOSR:
            eCheckResult = TestProc_Api_WashMeasureRoomCheckResult(&errCode);
            if (TESTPROC_API_RESULT_OK == eCheckResult)
            {
                FSM_M_SeparateAuto = FSM_M_SEPARATE_AUTO_MR_MAKE1;
                errCode = TestProc_Api_WashMeasureRoomMakeValue(MEASURE_FLAG_BACK, 500);
                if (errCode != LH_ERR_NONE)
                {
                    FSM_M_SeparateAuto = FSM_M_SEPARATE_AUTO_IDLE;
                    s32Result = TESTPROC_RESULT_NG;
                }
            }
            else if (TESTPROC_API_RESULT_NG == eCheckResult)
            {
                FSM_M_SeparateAuto = FSM_M_SEPARATE_AUTO_IDLE;
                s32Result = TESTPROC_RESULT_ERR_EXEC;
            }
            break;
        case FSM_M_SEPARATE_AUTO_MR_MAKE1:
            eCheckResult = TestProc_Api_WashMeasureRoomCheckResult(&errCode);
            if (TESTPROC_API_RESULT_OK == eCheckResult)
            {
                pstStatus->u32MeasureValue[0] = TestProc_Api_WashMeasureRoomReadValue(MEASURE_FLAG_BACK);
                Mem_Clr(&stTestResult, sizeof(TESTPROC_ITEM_RESULT_T));
                stTestResult.u32TestResult = pstStatus->u32MeasureValue[0];
                stTestResult.u32TestNmbr = g_u32MSeparationTestID;
                stTestResult.u8DataType = 9; // 底物
                stTestResult.u8CellNmbr = SystemConfig_GetCellNumber() + 1;
                stTestResult.u8ReagentPos = 0;
                stTestResult.u8TestType = 1;
                TestProc_SendDataToComm(BOARD_ID_TRACK_COMM, CAN_CMD_COMM_TESTDATA, (uint8_t *)&stTestResult, sizeof(TESTPROC_ITEM_RESULT_T));
                FSM_M_SeparateAuto = FSM_M_SEPARATE_AUTO_MR_MAKE2;
                errCode = TestProc_Api_WashMeasureRoomMakeValue(MEASURE_FLAG_READ, 4000);
                if (errCode != LH_ERR_NONE)
                {
                    FSM_M_SeparateAuto = FSM_M_SEPARATE_AUTO_IDLE;
                    s32Result = TESTPROC_RESULT_NG;
                }
            }
            else if (TESTPROC_API_RESULT_NG == eCheckResult)
            {
                FSM_M_SeparateAuto = FSM_M_SEPARATE_AUTO_IDLE;
                s32Result = TESTPROC_RESULT_ERR_EXEC;
            }
            break;
        case FSM_M_SEPARATE_AUTO_MR_MAKE2:
            eCheckResult = TestProc_Api_WashMeasureRoomCheckResult(&errCode);
            if (TESTPROC_API_RESULT_OK == eCheckResult)
            {
                pstStatus->u32MeasureValue[1] = TestProc_Api_WashMeasureRoomReadValue(MEASURE_FLAG_READ);
                Mem_Clr(&stTestResult, sizeof(TESTPROC_ITEM_RESULT_T));
                stTestResult.u32TestResult = pstStatus->u32MeasureValue[1];
                stTestResult.u32TestNmbr = g_u32MSeparationTestID++;
                stTestResult.u8DataType = 10; // 测量光子
                stTestResult.u8CellNmbr = SystemConfig_GetCellNumber() + 1;
                stTestResult.u8ReagentPos = 0;
                stTestResult.u8TestType = 1;
                TestProc_SendDataToComm(BOARD_ID_TRACK_COMM, CAN_CMD_COMM_TESTDATA, (uint8_t *)&stTestResult, sizeof(TESTPROC_ITEM_RESULT_T));
                FSM_M_SeparateAuto = FSM_M_SEPARATE_AUTO_MR_OPEN;
                errCode = TestProc_Api_WashMeasureRoomDoorOperation(TESTPROC_API_DOOR_OP_FULLOPEN);
                if (errCode != LH_ERR_NONE)
                {
                    FSM_M_SeparateAuto = FSM_M_SEPARATE_AUTO_IDLE;
                    s32Result = TESTPROC_RESULT_NG;
                }
            }
            else if (TESTPROC_API_RESULT_NG == eCheckResult)
            {
                FSM_M_SeparateAuto = FSM_M_SEPARATE_AUTO_IDLE;
                s32Result = TESTPROC_RESULT_ERR_EXEC;
            }
            break;
        case FSM_M_SEPARATE_AUTO_MR_OPEN:
            eCheckResult = TestProc_Api_WashMeasureRoomCheckResult(&errCode);
            if (TESTPROC_API_RESULT_OK == eCheckResult)
            {
                FSM_M_SeparateAuto = FSM_M_SEPARATE_AUTO_TO_MR_FOR_GARBAGE;
                errCode = TestProc_Api_HandCupMoveToTop(TESTPROC_API_HAND_TARGET_MEASUREROOM);
                if (errCode != LH_ERR_NONE)
                {
                    FSM_M_SeparateAuto = FSM_M_SEPARATE_AUTO_IDLE;
                    s32Result = TESTPROC_RESULT_NG;
                }
            }
            else if (TESTPROC_API_RESULT_NG == eCheckResult)
            {
                FSM_M_SeparateAuto = FSM_M_SEPARATE_AUTO_IDLE;
                s32Result = TESTPROC_RESULT_ERR_EXEC;
            }
            break;
        case FSM_M_SEPARATE_AUTO_TO_MR_FOR_GARBAGE:
            eCheckResult = TestProc_Api_HandCupCheckResult(&errCode);
            if (TESTPROC_API_RESULT_OK == eCheckResult)
            {
                FSM_M_SeparateAuto = FSM_M_SEPARATE_AUTO_CATCH_MR_CUP;
                errCode = TestProc_Api_HandCupCatchCup(TESTPROC_API_HAND_TARGET_MEASUREROOM);
                if (errCode != LH_ERR_NONE)
                {
                    FSM_M_SeparateAuto = FSM_M_SEPARATE_AUTO_IDLE;
                    s32Result = TESTPROC_RESULT_NG;
                }
            }
            else if (TESTPROC_API_RESULT_NG == eCheckResult)
            {
                FSM_M_SeparateAuto = FSM_M_SEPARATE_AUTO_IDLE;
                s32Result = TESTPROC_RESULT_ERR_EXEC;
            }
            break;
        case FSM_M_SEPARATE_AUTO_CATCH_MR_CUP:
            eCheckResult = TestProc_Api_HandCupCheckResult(&errCode);
            if (TESTPROC_API_RESULT_OK == eCheckResult)
            {
                FSM_M_SeparateAuto = FSM_M_SEPARATE_AUTO_PUT_TO_GARBAGE;
                errCode = TestProc_Api_HandCupMoveToGarbage(TSC_STACK_MANAGE_GARBAGE_NO1);
                if (errCode != LH_ERR_NONE)
                {
                    FSM_M_SeparateAuto = FSM_M_SEPARATE_AUTO_IDLE;
                    s32Result = TESTPROC_RESULT_NG;
                }
            }
            else if (TESTPROC_API_RESULT_NG == eCheckResult)
            {
                FSM_M_SeparateAuto = FSM_M_SEPARATE_AUTO_IDLE;
                s32Result = TESTPROC_RESULT_ERR_EXEC;
            }
            break;
        /*
        case FSM_M_SEPARATE_AUTO_TO_GARBAGE:
            eCheckResult = TestProc_Api_HandCupCheckResult(&errCode);
            if (TESTPROC_API_RESULT_OK == eCheckResult)
            {
                FSM_M_SeparateAuto = FSM_M_SEPARATE_AUTO_PUT_TO_GARBAGE;
                errCode = TestProc_Api_HandCupPutCupToGarbage(TSC_STACK_MANAGE_GARBAGE_NO1);
                if (errCode != LH_ERR_NONE)
                {
                    FSM_M_SeparateAuto = FSM_M_SEPARATE_AUTO_IDLE;
                    s32Result = TESTPROC_RESULT_NG;
                }
            }
            else if (TESTPROC_API_RESULT_NG == eCheckResult)
            {
                FSM_M_SeparateAuto = FSM_M_SEPARATE_AUTO_IDLE;
                s32Result = TESTPROC_RESULT_ERR_EXEC;
            }
            break;
        */
        case FSM_M_SEPARATE_AUTO_PUT_TO_GARBAGE:
            eCheckResult = TestProc_Api_HandCupCheckResult(&errCode);
            if (TESTPROC_API_RESULT_OK == eCheckResult)
            {
                if (g_u16MSeparationQuantity > 10)
                {
                    FSM_M_SeparateAuto = FSM_M_SEPARATE_AUTO_TO_CUPSTACK;
                }
                else
                {
                    FSM_M_SeparateAuto = FSM_M_SEPARATE_AUTO_WASH_STOP;
                }
            }
            else if (TESTPROC_API_RESULT_NG == eCheckResult)
            {
                FSM_M_SeparateAuto = FSM_M_SEPARATE_AUTO_IDLE;
                s32Result = TESTPROC_RESULT_ERR_EXEC;
            }
            break;
        case FSM_M_SEPARATE_AUTO_TO_CUPSTACK:
            DebugMsg("ToCupStack\r\n");
            errCode = TestProc_Api_HandCupMoveToNewDisk(pstStatus->u8CupStackRow, pstStatus->u8CupStackCol);
            if (LH_ERR_NONE == errCode)
            {
                FSM_M_SeparateAuto = FSM_M_SEPARATE_AUTO_CATCH_NEW_CUP;
                pstStatus->u8CupStackCol++;
                if (pstStatus->u8CupStackCol >= TESTPROC_CUPSTACK_COLUMN_MAX)
                {
                    pstStatus->u8CupStackCol = 0;
                    pstStatus->u8CupStackRow++;
                    if (pstStatus->u8CupStackRow >= TESTPROC_CUPSTACK_ROW_MAX)
                    {
                        pstStatus->u8CupStackRow = 0;
                    }
                }
            }
            else
            {
                FSM_M_SeparateAuto = FSM_M_SEPARATE_AUTO_IDLE;
                s32Result = TESTPROC_RESULT_NG;
            }
            break;
        case FSM_M_SEPARATE_AUTO_CATCH_NEW_CUP:
            eCheckResult = TestProc_Api_HandCupCheckResult(&errCode);
            if (TESTPROC_API_RESULT_OK == eCheckResult)
            {
                FSM_M_SeparateAuto = FSM_M_SEPARATE_AUTO_TO_WASH;
                errCode = TestProc_Api_HandCupCatchCup(TESTPROC_API_HAND_TARGET_NEWDISK);
                if (errCode != LH_ERR_NONE)
                {
                    FSM_M_SeparateAuto = FSM_M_SEPARATE_AUTO_IDLE;
                    s32Result = TESTPROC_RESULT_NG;
                }
            }
            else if (TESTPROC_API_RESULT_NG == eCheckResult)
            {
                FSM_M_SeparateAuto = FSM_M_SEPARATE_AUTO_IDLE;
                s32Result = TESTPROC_RESULT_ERR_EXEC;
            }
            break;
        case FSM_M_SEPARATE_AUTO_TO_WASH:
            eCheckResult = TestProc_Api_HandCupCheckResult(&errCode);
            if (TESTPROC_API_RESULT_OK == eCheckResult)
            {
                FSM_M_SeparateAuto = FSM_M_SEPARATE_AUTO_PUT_WASH;
                errCode = TestProc_Api_HandCupMoveToTop(TESTPROC_API_HAND_TARGET_WASH);
                if (errCode != LH_ERR_NONE)
                {
                    FSM_M_SeparateAuto = FSM_M_SEPARATE_AUTO_IDLE;
                    s32Result = TESTPROC_RESULT_NG;
                }
            }
            else if (TESTPROC_API_RESULT_NG == eCheckResult)
            {
                FSM_M_SeparateAuto = FSM_M_SEPARATE_AUTO_IDLE;
                s32Result = TESTPROC_RESULT_ERR_EXEC;
            }
            break;
        case FSM_M_SEPARATE_AUTO_PUT_WASH:
            eCheckResult = TestProc_Api_HandCupCheckResult(&errCode);
            if (TESTPROC_API_RESULT_OK == eCheckResult)
            {
                FSM_M_SeparateAuto = FSM_M_SEPARATE_AUTO_PUT_WASH_ING;
                errCode = TestProc_Api_HandCupPutCup(TESTPROC_API_HAND_TARGET_WASH);
                if (errCode != LH_ERR_NONE)
                {
                    FSM_M_SeparateAuto = FSM_M_SEPARATE_AUTO_IDLE;
                    s32Result = TESTPROC_RESULT_NG;
                }
            }
            else if (TESTPROC_API_RESULT_NG == eCheckResult)
            {
                FSM_M_SeparateAuto = FSM_M_SEPARATE_AUTO_IDLE;
                s32Result = TESTPROC_RESULT_ERR_EXEC;
            }
            break;
        case FSM_M_SEPARATE_AUTO_PUT_WASH_ING:
            eCheckResult = TestProc_Api_HandCupCheckResult(&errCode);
            if (TESTPROC_API_RESULT_OK == eCheckResult)
            {
                DebugMsg("WaskCupIn:%d\r\n", pstStatus->u8WashDiskHandCup);
                FSM_M_SeparateAuto = FSM_M_SEPARATE_AUTO_WASH_STOP;
                pstStatus->eWashCupState[pstStatus->u8WashDiskHandCup] = TESTPROC_WASH_CUP_IN;
            }
            else if (TESTPROC_API_RESULT_NG == eCheckResult)
            {
                FSM_M_SeparateAuto = FSM_M_SEPARATE_AUTO_IDLE;
                s32Result = TESTPROC_RESULT_ERR_EXEC;
            }
            break;
        case FSM_M_SEPARATE_AUTO_WASH_STOP:
            if ((OSTimeGet(&os_err)-Tick) > (18000-TASK_TEST_PROCESS_SPEED))
            {
                DebugMsg("M_SeparateAutoOnce:%d\r\n", g_u16MSeparationQuantity);
                if (g_u16MSeparationQuantity > 0)
                {
                    g_u16MSeparationQuantity --;
                }
                
                if (0 == g_u16MSeparationQuantity)
                {
                    TestProc_Standby(pstStatus);
                }
                else
                {
                    if (0 == u8WashNeedleState)
                    {
                        Tick = OSTimeGet(&os_err);
                        FSM_M_SeparateAuto = FSM_M_SEPARATE_AUTO_ROTATE_ING;
                        errCode = TestProc_Api_WashDiskRotate1Pos();
                        if (LH_ERR_NONE != errCode)
                        {
                            FSM_M_SeparateAuto = FSM_M_SEPARATE_AUTO_IDLE;
                            s32Result = TESTPROC_RESULT_NG;
                        }
                    }
                    else
                    {
                        FSM_M_SeparateAuto = FSM_M_SEPARATE_AUTO_IDLE;
                        s32Result = TESTPROC_RESULT_NG;
                        DebugMsg("E>WashNeedleTimeout:%d,%d\r\n", u8WashNeedleState, u8WashNeedleCount);
                    }
                }
            }
            break;
        case FSM_M_SEPARATE_AUTO_ROTATE_ING:
            eCheckResult = TestProc_Api_WashDiskCheckResult(&errCode);
            if (TESTPROC_API_RESULT_OK == eCheckResult)
            {
                FSM_M_SeparateAuto = FSM_M_SEPARATE_AUTO_START_STOP_WORK;
                u8WashNeedleState = 1;
                u8WashNeedleCount = 0;
                pstStatus->u8WashDiskHandCup = TestProc_WashDiskHandCup();
                DebugMsg("M_SeparateAuto Stop:%d,%d,%d\r\n", pstStatus->u8WashDiskHandCup, pstStatus->eWashCupState[pstStatus->u8WashDiskHandCup], g_u16MSeparationQuantity);
            }
            else if (TESTPROC_API_RESULT_NG == eCheckResult)
            {
                FSM_M_SeparateAuto = FSM_M_SEPARATE_AUTO_IDLE;
                s32Result = TESTPROC_RESULT_ERR_EXEC;
            }
            break;
        default:
            DebugMsg("E>MSeparateAutoFSM:%d\r\n", FSM_M_SeparateAuto);
            break;
    }
    //
    if (1 == u8WashNeedleState)
    {
        u8WashNeedleCount ++;
        FSM_M_Separate = FSM_M_SEPARATE_WASH_ING;
        if (1 == u8WashNeedleCount)
        {
            u16WashMask = 0;
            u8CupID = TestProc_Wash12Cup();
            if (TESTPROC_WASH_CUP_IN == pstStatus->eWashCupState[u8CupID])
            {
                u16WashMask |= WASH_STOP_WORK_WASH12;
                pstStatus->eWashCupState[u8CupID] = TESTPROC_WASH_CUP_WASH12;
            }
            u8CupID = TestProc_Wash34Cup();
            if (TESTPROC_WASH_CUP_WASH12 == pstStatus->eWashCupState[u8CupID])
            {
                u16WashMask |= WASH_STOP_WORK_WASH34;
                pstStatus->eWashCupState[u8CupID] = TESTPROC_WASH_CUP_WASH34;
            }
            u8CupID = TestProc_Wash56Cup();
            if (TESTPROC_WASH_CUP_WASH34 == pstStatus->eWashCupState[u8CupID])
            {
                u16WashMask |= WASH_STOP_WORK_WASH56;
                pstStatus->eWashCupState[u8CupID] = TESTPROC_WASH_CUP_WASH56;
            }
        }
        else
        {
            u16WashMask &= ~WASH_STOP_WORK_WASH56;
        }
        //
        errCode = TestProc_Api_WashNeedleClean((TSC_NEEDLE_WASH_SELECT_FLAG)u16WashMask, 1 == u8WashNeedleCount ? CLEAN_NEEDLE_INDEX_FIRST : CLEAN_NEEDLE_INDEX_SECOND);
        u16WorkMask = 0;
        if (LH_ERR_NONE == errCode)
        {
            u16WorkMask = 0x01;
        }
        //
        u8CupID = TestProc_WashLiquidACup();
        if (TESTPROC_WASH_CUP_WASH56 == pstStatus->eWashCupState[u8CupID])
        {
            pstStatus->eWashCupState[u8CupID] = TESTPROC_WASH_CUP_LIQUIDA;
            errCode = TestProc_Api_WashLiquidAInject();
            if (LH_ERR_NONE == errCode)
            {
                u16WorkMask |= 0x02;
            }
        }
        //
        if (1 == u8WashNeedleCount)
        {
            u8CupID = TestProc_WashMixCup();
            if (TESTPROC_WASH_CUP_LIQUIDA == pstStatus->eWashCupState[u8CupID])
            {
                pstStatus->eWashCupState[u8CupID] = TESTPROC_WASH_CUP_MIX;
                errCode = TestProc_Api_WashDiskMixAction(g_u32MSeparationMixTime);
                if (LH_ERR_NONE == errCode)
                {
                    u16WorkMask |= 0x04;
                }
            }
        }
        u8WashNeedleState = 2;
        DebugMsg("M_SeparateAuto_WashNeedle:%d,%d,%02X,%02X\r\n", u8WashNeedleCount, pstStatus->u8WashDiskHandCup, u16WorkMask, u16WashMask);
        if (errCode != LH_ERR_NONE)
        {
            DebugMsg("E>M_SeparateAutoNeedle:%08X,%d,%04X\r\n", errCode, pstStatus->u8WashDiskHandCup, u16WorkMask);
            FSM_M_SeparateAuto = FSM_M_SEPARATE_AUTO_IDLE;
            s32Result = TESTPROC_RESULT_ERR_STATUS;
        }
    }
    else if (2 == u8WashNeedleState)
    {
        if ((u16WorkMask&0x01) > 0)
        {
            eCheckResult = TestProc_Api_WashNeedleCheckResult(&errCode);
            if (TESTPROC_API_RESULT_OK == eCheckResult)
            {
                u16WorkMask &= ~0x01;
            }
            else if (TESTPROC_API_RESULT_NG == eCheckResult)
            {
                u16WorkMask &= ~0x01;
                DebugMsg("E>M_SeparateNeedle:%08X\r\n", errCode);
            }
        }
        if ((u16WorkMask&0x02) > 0)
        {
            eCheckResult = TestProc_Api_WashLiquidACheckResult(&errCode2);
            if (TESTPROC_API_RESULT_OK == eCheckResult)
            {
                u16WorkMask &= ~0x02;
            }
            else if (TESTPROC_API_RESULT_NG == eCheckResult)
            {
                u16WorkMask &= ~0x02;
                DebugMsg("E>M_SeparateLiquidA\r\n");
            }
        }            
        if ((u16WorkMask&0x04) > 0)
        {
            eCheckResult = TestProc_Api_WashDiskCheckResult(&errCode);
            if (TESTPROC_API_RESULT_OK == eCheckResult)
            {
                u16WorkMask &= ~0x04;
            }
            else if (TESTPROC_API_RESULT_NG == eCheckResult)
            {
                u16WorkMask &= ~0x04;
                DebugMsg("E>M_SeparateMix\r\n");
            }
        }
        if (u8WashNeedleCount >= 2)
        {
            if ((u16WorkMask&0x07) == 0x00)
            {
                u8WashNeedleState = 0;
                DebugMsg("WashNeedle_OK\r\n");
            }
        }
        else
        {
            if ((u16WorkMask&0x01) == 0x00)
            {
                u8WashNeedleState = 1;
            }
        }
    }
    
    if (TESTPROC_RESULT_OK != s32Result)
    {
        DebugMsg("E>MSeparateExe:%d,%d,%d,%08X,%08X,%d,%04X,%04X,%d,%d\r\n", s32Result, FSM_M_Separate, FSM_Old, errCode, errCode2, eCheckResult, u16WorkMask, u16WashMask, u8WashNeedleCount, u8WashNeedleState);
    }

    return s32Result;
}

/****************************************************************************
*
* Function Name:    TestProc_MSeparateFSM
* Input:            pstStatus - 系统状态
*                   
* Output:           None
*                   
* Returns:          TESTPROC_RESULT_OK
* Description:      
* Note:             
***************************************************************************/
static int32_t TestProc_MSeparateFSM(TESTPROC_STATUS_T *pstStatus)
{
    int32_t  s32Result = TESTPROC_RESULT_OK;
    int i;
    static OS_TICK  Tick;
    static uint16_t u16WashMask = 0;
    static uint16_t u16WorkMask = 0;
    static uint8_t  u8Count = 0;
    LH_ERR errCode = LH_ERR_NONE;
    LH_ERR errCode2 = LH_ERR_NONE;
    TESTPROC_API_RESULT_E eCheckResult = TESTPROC_API_RESULT_OK;
    FSM_M_SEPARATE_E FSM_Old = FSM_M_Separate;
    OS_ERR os_err;
    uint8_t  u8CupID;

    //DebugMsg("MS:%d\r\n", FSM_M_Separate);
    switch(FSM_M_Separate)
    {
        case FSM_M_SEPARATE_START:
            DebugMsg("MSeparate Start\r\n");
            FSM_M_Separate = FSM_M_SEPARATE_WASH_RESET_1;
            break;
        case FSM_M_SEPARATE_WASH_RESET_1:
            FSM_M_Separate = FSM_M_SEPARATE_WASH_RESET_2;
            errCode = TestProc_Api_HandCupReset();
            if (LH_ERR_NONE != errCode)
            {
                FSM_M_Separate = FSM_M_SEPARATE_IDLE;
                s32Result = TESTPROC_RESULT_NG;
            }
            break;
        case FSM_M_SEPARATE_WASH_RESET_2:
            eCheckResult = TestProc_Api_HandCupCheckResult(&errCode);
            if (TESTPROC_API_RESULT_OK == eCheckResult)
            {
                FSM_M_Separate = FSM_M_SEPARATE_WASH_RESET_ING;
                if (TestProc_WashResetFSM(TRUE) == TESTPROC_API_RESULT_NG)
                {
                    FSM_M_Separate = FSM_M_SEPARATE_IDLE;
                    s32Result = TESTPROC_RESULT_NG;
                }
            }
            else if (TESTPROC_API_RESULT_NG == eCheckResult)
            {
                FSM_M_Separate = FSM_M_SEPARATE_IDLE;
                s32Result = TESTPROC_RESULT_ERR_EXEC;
            }
            break;
        case FSM_M_SEPARATE_WASH_RESET_ING:
            eCheckResult = TestProc_WashResetFSM(FALSE);
            if (TESTPROC_API_RESULT_OK == eCheckResult)
            {
                FSM_M_Separate = FSM_M_SEPARATE_MEASUREROOM_OPEN;
                errCode = TestProc_Api_WashMeasureRoomDoorOperation(TESTPROC_API_DOOR_OP_FULLOPEN);
                if (LH_ERR_NONE != errCode)
                {
                    FSM_M_Separate = FSM_M_SEPARATE_IDLE;
                    s32Result = TESTPROC_RESULT_NG;
                }
            }
            else if (TESTPROC_API_RESULT_NG == eCheckResult)
            {
                FSM_M_Separate = FSM_M_SEPARATE_IDLE;
                s32Result = TESTPROC_RESULT_ERR_EXEC;
            }
            break;
        case FSM_M_SEPARATE_MEASUREROOM_OPEN:
            eCheckResult = TestProc_Api_WashMeasureRoomCheckResult(&errCode);
            if (TESTPROC_API_RESULT_OK == eCheckResult)
            {
                for (i=0; i<=TESTPROC_WASHDISK_POS_QUANTITY; i++)
                {
                    pstStatus->eWashCupState[i] = TESTPROC_WASH_CUP_INIT;
                }
                Tick = OSTimeGet(&os_err);
                g_u16MSeparationQuantity += 10;
                FSM_M_Separate = FSM_M_SEPARATE_WAIT_TEST;
                pstStatus->u8WashDiskHandCup = 1;
                pstStatus->eWashCupState[pstStatus->u8WashDiskHandCup] = TESTPROC_WASH_CUP_IN;
                DebugMsg("M_Separate Reset OK\r\n");
            }
            else if (TESTPROC_API_RESULT_NG == eCheckResult)
            {
                FSM_M_Separate = FSM_M_SEPARATE_IDLE;
                s32Result = TESTPROC_RESULT_ERR_EXEC;
            }
            break;
        case FSM_M_SEPARATE_ROTATE:
            FSM_M_Separate = FSM_M_SEPARATE_ROTATE_ING;                
            DebugMsg("M_Separate Rotate\r\n");
            Tick = OSTimeGet(&os_err);
            errCode = TestProc_Api_WashDiskRotate1Pos();
            if (LH_ERR_NONE != errCode)
            {
                FSM_M_Separate = FSM_M_SEPARATE_IDLE;
                s32Result = TESTPROC_RESULT_NG;
            }
            break;
        case FSM_M_SEPARATE_ROTATE_ING:
            eCheckResult = TestProc_Api_WashDiskCheckResult(&errCode);
            if (TESTPROC_API_RESULT_OK == eCheckResult)
            {
                FSM_M_Separate = FSM_M_SEPARATE_WASH;
                pstStatus->u8WashDiskHandCup = TestProc_WashDiskHandCup();
                if (g_u16MSeparationQuantity > 10)
                {
                    pstStatus->eWashCupState[pstStatus->u8WashDiskHandCup] = TESTPROC_WASH_CUP_IN;
                }
                else
                {
                    pstStatus->eWashCupState[pstStatus->u8WashDiskHandCup] = TESTPROC_WASH_CUP_INIT;
                }
                u8Count = 0;
                DebugMsg("M_Separate Stop:%d,%d\r\n", pstStatus->u8WashDiskHandCup, g_u16MSeparationQuantity);
            }
            else if (TESTPROC_API_RESULT_NG == eCheckResult)
            {
                FSM_M_Separate = FSM_M_SEPARATE_IDLE;
                s32Result = TESTPROC_RESULT_ERR_EXEC;
            }
            break;
        case FSM_M_SEPARATE_WASH:
            u8Count ++;
            FSM_M_Separate = FSM_M_SEPARATE_WASH_ING;
            if (1 == u8Count)
            {
                u16WashMask = 0;
                u8CupID = TestProc_Wash12Cup();
                if (TESTPROC_WASH_CUP_IN == pstStatus->eWashCupState[u8CupID])
                {
                    u16WashMask |= WASH_STOP_WORK_WASH12;
                    pstStatus->eWashCupState[u8CupID] = TESTPROC_WASH_CUP_WASH12;
                }
                u8CupID = TestProc_Wash34Cup();
                if (TESTPROC_WASH_CUP_WASH12 == pstStatus->eWashCupState[u8CupID])
                {
                    u16WashMask |= WASH_STOP_WORK_WASH34;
                    pstStatus->eWashCupState[u8CupID] = TESTPROC_WASH_CUP_WASH34;
                }
                u8CupID = TestProc_Wash56Cup();
                if (TESTPROC_WASH_CUP_WASH34 == pstStatus->eWashCupState[u8CupID])
                {
                    u16WashMask |= WASH_STOP_WORK_WASH56;
                    pstStatus->eWashCupState[u8CupID] = TESTPROC_WASH_CUP_WASH56;
                }
            }
            else
            {
                u16WashMask &= ~WASH_STOP_WORK_WASH56;
            }
            //
            DebugMsg("MSeparateClean:%d,%02X\r\n", u8Count, u16WashMask);
            errCode = TestProc_Api_WashNeedleClean((TSC_NEEDLE_WASH_SELECT_FLAG)u16WashMask, 1 == u8Count ? CLEAN_NEEDLE_INDEX_FIRST : CLEAN_NEEDLE_INDEX_SECOND);
            u16WorkMask = 0;
            if (LH_ERR_NONE == errCode)
            {
                u16WorkMask = 0x01;
            }
            //
            if (1 == u8Count)
            {
                u8CupID = TestProc_WashLiquidACup();
                if (TESTPROC_WASH_CUP_WASH56 == pstStatus->eWashCupState[u8CupID])
                {
                    pstStatus->eWashCupState[u8CupID] = TESTPROC_WASH_CUP_LIQUIDA;
                    errCode = TestProc_Api_WashLiquidAInject();
                    if (LH_ERR_NONE == errCode)
                    {
                        u16WorkMask |= 0x02;
                    }
                }
                //
                u8CupID = TestProc_WashMixCup();
                if (TESTPROC_WASH_CUP_LIQUIDA == pstStatus->eWashCupState[u8CupID])
                {
                    pstStatus->eWashCupState[u8CupID] = TESTPROC_WASH_CUP_MIX;
                    errCode = TestProc_Api_WashDiskMixAction(g_u32MSeparationMixTime);
                    if (LH_ERR_NONE == errCode)
                    {
                        u16WorkMask |= 0x04;
                    }
                }
            }
            DebugMsg("WashNeedle:%d,%d,%d\r\n", u8CupID, u8Count, u16WorkMask);
            if (errCode != LH_ERR_NONE)
            {
                DebugMsg("E>M_SeparateNeedle:%08X,%04X\r\n", errCode, u16WorkMask);
                FSM_M_Separate = FSM_M_SEPARATE_IDLE;
                s32Result = TESTPROC_RESULT_NG;
            }
            break;
        case FSM_M_SEPARATE_WASH_ING:
            if ((u16WorkMask&0x01) > 0)
            {
                eCheckResult = TestProc_Api_WashNeedleCheckResult(&errCode);
                if (TESTPROC_API_RESULT_OK == eCheckResult)
                {
                    u16WorkMask &= ~0x01;
                }
                else if (TESTPROC_API_RESULT_NG == eCheckResult)
                {
                    u16WorkMask &= ~0x01;
                    DebugMsg("E>M_SeparateNeedle\r\n", errCode);
                }
            }
            if ((u16WorkMask&0x02) > 0)
            {
                eCheckResult = TestProc_Api_WashLiquidACheckResult(&errCode2);
                if (TESTPROC_API_RESULT_OK == eCheckResult)
                {
                    u16WorkMask &= ~0x02;
                }
                else if (TESTPROC_API_RESULT_NG == eCheckResult)
                {
                    u16WorkMask &= ~0x02;
                    DebugMsg("E>M_SeparateLiquidA:%08X\r\n", errCode2);
                }
            }            
            if ((u16WorkMask&0x04) > 0)
            {
                eCheckResult = TestProc_Api_WashDiskCheckResult(&errCode2);
                if (TESTPROC_API_RESULT_OK == eCheckResult)
                {
                    u16WorkMask &= ~0x04;
                }
                else if (TESTPROC_API_RESULT_NG == eCheckResult)
                {
                    u16WorkMask &= ~0x04;
                    DebugMsg("E>M_SeparateMix:%08X\r\n", errCode2);
                }
            }
            
            if ((u16WorkMask&0x07) == 0x00)
            {
                DebugMsg("WashNeedle_OK:%d\r\n", u8Count);
                if (u8Count >= 2)
                {
                    FSM_M_Separate = FSM_M_SEPARATE_WAIT_TEST;
                }
                else
                {
                    FSM_M_Separate = FSM_M_SEPARATE_WASH;
                }
            }
            break;
        case FSM_M_SEPARATE_WAIT_TEST:
            if ((OSTimeGet(&os_err)-Tick) > (18000-TASK_TEST_PROCESS_SPEED))
            {
                FSM_M_Separate = FSM_M_SEPARATE_ROTATE;
                DebugMsg("M_SeparateOnce:%d\r\n", g_u16MSeparationQuantity);
                if (g_u16MSeparationQuantity > 0)
                {
                    g_u16MSeparationQuantity --;
                }
                if (0 == g_u16MSeparationQuantity)
                {
                    TestProc_Standby(pstStatus);
                }
            }
            break;
        case FSM_M_SEPARATE_IDLE:
        default:
            DebugMsg("E>MSeparateFSM:%d\r\n", FSM_M_Separate);
            break;
    }

    if (TESTPROC_RESULT_OK != s32Result)
    {
        DebugMsg("E>MSeparateExe:%d,%d,%d,%d,%d,X04%\r\n", s32Result, FSM_M_Separate, FSM_Old, errCode, eCheckResult, u16WorkMask);
    }
    
    return s32Result;
}

/****************************************************************************
*
* Function Name:    TestProc_ManualTestFSM
* Input:            pstStatus - 系统状态
*                   
* Output:           None
*                   
* Returns:          TESTPROC_RESULT_OK
* Description:      
* Note:             
***************************************************************************/
static int32_t TestProc_ManualTestFSM(TESTPROC_STATUS_T *pstStatus)
{
    int32_t  s32Result = TESTPROC_RESULT_OK;
    TESTPROC_API_RESULT_E eCheckResult = TESTPROC_API_RESULT_OK;
    LH_ERR errCode = LH_ERR_NONE;
    TESTPROC_ITEM_RESULT_T stTestResult;
    FSM_MANUAL_TEST_E FSM_Old = FSM_ManualTest;

    //DebugMsg("MT:%d\r\n", FSM_ManualTest);
    switch (FSM_ManualTest)
    {
        case FSM_MANUAL_TEST_PREPARE:
            FSM_ManualTest = FSM_MANUAL_TEST_RESET_1;
            errCode = TestProc_Api_HandCupReset();
            if (LH_ERR_NONE != errCode)
            {
                FSM_ManualTest = FSM_MANUAL_TEST_IDLE;
                s32Result = TESTPROC_RESULT_NG;
            }
            break;
        case FSM_MANUAL_TEST_RESET_1:
            eCheckResult = TestProc_Api_HandCupCheckResult(&errCode);
            if (TESTPROC_API_RESULT_OK == eCheckResult)
            {
                FSM_ManualTest = FSM_MANUAL_TEST_RESET_2;
                errCode = TestProc_Api_WashMeasureRoomReset();
                if (LH_ERR_NONE != errCode)
                {
                    FSM_ManualTest = FSM_MANUAL_TEST_IDLE;
                    s32Result = TESTPROC_RESULT_NG;
                }
            }
            else if (TESTPROC_API_RESULT_NG == eCheckResult)
            {
                FSM_ManualTest = FSM_MANUAL_TEST_IDLE;
                s32Result = TESTPROC_RESULT_ERR_EXEC;
            }
            break;
        case FSM_MANUAL_TEST_RESET_2:
            eCheckResult = TestProc_Api_WashMeasureRoomCheckResult(&errCode);
            if (TESTPROC_API_RESULT_OK == eCheckResult)
            {
                FSM_ManualTest = FSM_MANUAL_TEST_MEASUREROOM_INIT;
                errCode = TestProc_Api_WashMeasureRoomDoorOperation(TESTPROC_API_DOOR_OP_FULLOPEN);
                if (LH_ERR_NONE != errCode)
                {
                    FSM_ManualTest = FSM_MANUAL_TEST_IDLE;
                    s32Result = TESTPROC_RESULT_NG;
                }
            }
            else if (TESTPROC_API_RESULT_NG == eCheckResult)
            {
                FSM_ManualTest = FSM_MANUAL_TEST_IDLE;
                s32Result = TESTPROC_RESULT_ERR_EXEC;
            }
            break;
        case FSM_MANUAL_TEST_MEASUREROOM_INIT:
            eCheckResult = TestProc_Api_WashMeasureRoomCheckResult(&errCode);
            if (TESTPROC_API_RESULT_OK == eCheckResult)
            {
                DebugMsg("PrepareTest OK\r\n");
                FSM_ManualTest = FSM_MANUAL_TEST_IDLE;
            }
            else if (TESTPROC_API_RESULT_NG == eCheckResult)
            {
                FSM_ManualTest = FSM_MANUAL_TEST_IDLE;
                s32Result = TESTPROC_RESULT_ERR_EXEC;
            }
            break;
        //
        case FSM_MANUAL_TEST_START_TEST:
            DebugMsg("ManualTest\r\n");
            FSM_ManualTest = FSM_MANUAL_TEST_MEASUREROOM_CLOSE;
            errCode = TestProc_Api_WashMeasureRoomDoorOperation(TESTPROC_API_DOOR_OP_CLOSE);
            if (errCode != LH_ERR_NONE)
            {
                FSM_ManualTest = FSM_MANUAL_TEST_IDLE;
                s32Result = TESTPROC_RESULT_NG;
            }
            break;
        case FSM_MANUAL_TEST_MEASUREROOM_CLOSE:
            eCheckResult = TestProc_Api_WashMeasureRoomCheckResult(&errCode);
            if (TESTPROC_API_RESULT_OK == eCheckResult)
            {
                FSM_ManualTest = FSM_MANUAL_TEST_MEASUREROOM_MAKE;
                errCode = TestProc_Api_WashMeasureRoomMakeValue(MEASURE_FLAG_BACK, 500);
                if (errCode != LH_ERR_NONE)
                {
                    FSM_ManualTest = FSM_MANUAL_TEST_IDLE;
                    s32Result = TESTPROC_RESULT_NG;
                }
            }
            else if (TESTPROC_API_RESULT_NG == eCheckResult)
            {
                FSM_ManualTest = FSM_MANUAL_TEST_IDLE;
                s32Result = TESTPROC_RESULT_ERR_EXEC;
            }
            break;
        case FSM_MANUAL_TEST_MEASUREROOM_MAKE:
            eCheckResult = TestProc_Api_WashMeasureRoomCheckResult(&errCode);
            if (TESTPROC_API_RESULT_OK == eCheckResult)
            {
                FSM_ManualTest = FSM_MANUAL_TEST_MEASUREROOM_MAKE2;
                
                pstStatus->u32MeasureValue[0] = TestProc_Api_WashMeasureRoomReadValue(MEASURE_FLAG_BACK);
                Mem_Clr(&stTestResult, sizeof(TESTPROC_ITEM_RESULT_T));
                stTestResult.u32TestResult = pstStatus->u32MeasureValue[0];
                stTestResult.u32TestNmbr = g_u32MSeparationTestID;
                stTestResult.u8DataType = 9; // 底物
                stTestResult.u8CellNmbr = SystemConfig_GetCellNumber() + 1;
                stTestResult.u8ReagentPos = 0;
                stTestResult.u8TestType = 1;
                TestProc_SendDataToComm(BOARD_ID_TRACK_COMM, CAN_CMD_COMM_TESTDATA, (uint8_t *)&stTestResult, sizeof(TESTPROC_ITEM_RESULT_T));
                
                errCode = TestProc_Api_WashMeasureRoomMakeValue(MEASURE_FLAG_READ, 4000);
                if (errCode != LH_ERR_NONE)
                {
                    FSM_ManualTest = FSM_MANUAL_TEST_IDLE;
                    s32Result = TESTPROC_RESULT_NG;
                }
            }
            else if (TESTPROC_API_RESULT_NG == eCheckResult)
            {
                FSM_ManualTest = FSM_MANUAL_TEST_IDLE;
                s32Result = TESTPROC_RESULT_ERR_EXEC;
            }
            break;
        case FSM_MANUAL_TEST_MEASUREROOM_MAKE2:
            eCheckResult = TestProc_Api_WashMeasureRoomCheckResult(&errCode);
            if (TESTPROC_API_RESULT_OK == eCheckResult)
            {
                FSM_ManualTest = FSM_MANUAL_TEST_MEASUREROOM_END;
                
                pstStatus->u32MeasureValue[1] = TestProc_Api_WashMeasureRoomReadValue(MEASURE_FLAG_READ);
                Mem_Clr(&stTestResult, sizeof(TESTPROC_ITEM_RESULT_T));
                stTestResult.u32TestResult = pstStatus->u32MeasureValue[1];
                stTestResult.u32TestNmbr = g_u32MSeparationTestID++;
                stTestResult.u8DataType = 10; // 测量光子
                stTestResult.u8CellNmbr = SystemConfig_GetCellNumber() + 1;
                stTestResult.u8ReagentPos = 0;
                stTestResult.u8TestType = 1;
                TestProc_SendDataToComm(BOARD_ID_TRACK_COMM, CAN_CMD_COMM_TESTDATA, (uint8_t *)&stTestResult, sizeof(TESTPROC_ITEM_RESULT_T));
        
                errCode = TestProc_Api_WashMeasureRoomDoorOperation(TESTPROC_API_DOOR_OP_FULLOPEN);
                if (errCode != LH_ERR_NONE)
                {
                    FSM_ManualTest = FSM_MANUAL_TEST_IDLE;
                    s32Result = TESTPROC_RESULT_NG;
                }
            }
            else if (TESTPROC_API_RESULT_NG == eCheckResult)
            {
                FSM_ManualTest = FSM_MANUAL_TEST_IDLE;
                s32Result = TESTPROC_RESULT_ERR_EXEC;
            }
            break;
        case FSM_MANUAL_TEST_MEASUREROOM_END:
            eCheckResult = TestProc_Api_WashMeasureRoomCheckResult(&errCode);            
            if (TESTPROC_API_RESULT_OK == eCheckResult)
            {
                FSM_ManualTest = FSM_MANUAL_TEST_IDLE;
                DebugMsg("ManualTest OK\r\n");
            }
            else if (TESTPROC_API_RESULT_NG == eCheckResult)
            {
                FSM_ManualTest = FSM_MANUAL_TEST_IDLE;
                s32Result = TESTPROC_RESULT_ERR_EXEC;
            }
            break;
        case FSM_MANUAL_TEST_IDLE:
        default:

            break;
    }
    
    if (TESTPROC_RESULT_OK != s32Result)
    {
        DebugMsg("E>ManualTest:%d,%d,%d,%d,%d\r\n", s32Result, FSM_ManualTest, FSM_Old, errCode, eCheckResult);
    }

    return s32Result;
}

/****************************************************************************
*
* Function Name:    TestProc_PowerOnFSM
* Input:            pstStatus - 系统状态
*                   
* Output:           None
*                   
* Returns:          TESTPROC_RESULT_OK
* Description:      上电流程
* Note:             
***************************************************************************/
static int32_t TestProc_PowerOnFSM(TESTPROC_STATUS_T *pstStatus)
{
    LH_ERR err;
    int32_t  s32Result = TESTPROC_RESULT_OK;
    //OS_ERR os_err;
    TESTPROC_API_RESULT_E result;
    static uint16_t u16FinishWork = 0;
    TESTPROC_RESET_STATE ResetResult;
    FSM_POWERON_E FSM_Old = FSM_PowerOn;

    //DebugMsg("PO:%d,%04X\r\n", FSM_PowerOn, u16FinishWork);
    switch (FSM_PowerOn)
    {
        case FSM_POWERON_RESET:
            //TestProc_Reset(pstStatus);
            FSM_PowerOn = FSM_POWERON_STANDBY; //FSM_POWERON_RESET_ING;FSM_POWERON_STANDBY
            break;
        case FSM_POWERON_RESET_ING:
            ResetResult = TestProc_ResetCheckResult(pstStatus);
            if (TESTPROC_RESET_OK == ResetResult)
            {
                pstStatus->stCellStatus.u8WorkStatus = TESTPROC_WORK_PRIME;
                FSM_PowerOn = FSM_POWERON_PRIME_SAMPLE_MOVE;
            }
            break;
        case FSM_POWERON_PRIME_SAMPLE_MOVE:
            u16FinishWork = 0;
            // Wash
            err = TestProc_Api_WashNeedlePrime();
            if (err == LH_ERR_NONE)
            {
                u16FinishWork |= 0x01;
            }
            else
            {
                DebugMsg("E>PowerONInjectWash:%d\r\n", err);
            }
            // Inject A
            err = TestProc_Api_WashLiquidAPrime();
            if (err == LH_ERR_NONE)
            {
                u16FinishWork |= 0x02;
            }
            else
            {
                DebugMsg("E>PowerONInjectA:%d\r\n", err);
            }
            // Inject B
            err = TestProc_Api_WashMeasureRoomWindowsClose();
            if (err == LH_ERR_NONE)
            {
                u16FinishWork |= 0x04;
            }
            else
            {
                DebugMsg("E>PowerONInjectB:%d\r\n", err);
            }
            // Sample 强灌注
            err = TestProc_Api_SampleMoveToPos(TSC_NEEDLE_SAMPLE_POS_FORCE_CLEAN);
            if (err == LH_ERR_NONE)
            {
                u16FinishWork |= 0x08;
            }
            else
            {
                DebugMsg("E>PowerON_SampleToForceWash:%d\r\n", err);
            }

            err = TestProc_Api_ReagentPrime(TSC_NEEDLE_REAGENT_PRIME_OPT_ALL);
            if (err == LH_ERR_NONE)
            {
                u16FinishWork |= 0x10;
            }
            else
            {
                DebugMsg("E>PowerONInjectReagent:%d\r\n", err);
            }
            if ((u16FinishWork&0x1F) == 0x1F)
            {
                FSM_PowerOn = FSM_POWERON_SAMPLE_TO_FORCE_PRIME_ING;
            }
            else
            {
                FSM_PowerOn = FSM_POWERON_IDLE;
                s32Result = TESTPROC_RESULT_ERR_EXEC;
            }
            break;
        case FSM_POWERON_SAMPLE_TO_FORCE_PRIME_ING:
            if ((u16FinishWork&0x01) > 0)
            {
                result = TestProc_Api_WashNeedleCheckResult(&err);
                if (TESTPROC_API_RESULT_OK == result)
                {
                    u16FinishWork &= ~0x01;
                }
                else if (TESTPROC_API_RESULT_NG == result)
                {
                    u16FinishWork &= ~0x01;
                    u16FinishWork |= 0x100;
                    DebugMsg("E>PowerON_WashNeedle:%08X\r\n", err);
                }
            }
            if ((u16FinishWork&0x02) > 0)
            {
                result = TestProc_Api_WashLiquidACheckResult(&err);
                if (TESTPROC_API_RESULT_OK == result)
                {
                    u16FinishWork &= ~0x02;
                }
                else if (TESTPROC_API_RESULT_NG == result)
                {
                    u16FinishWork &= ~0x02;
                    u16FinishWork |= 0x200;
                    DebugMsg("E>PowerON_LiuqidA:%08X\r\n", err);
                }
            }
            if ((u16FinishWork&0x04) > 0)
            {
                result = TestProc_Api_WashMeasureRoomCheckResult(&err);
                if (TESTPROC_API_RESULT_OK == result)
                {
                    u16FinishWork &= ~0x04;
                }
                else if (TESTPROC_API_RESULT_NG == result)
                {
                    u16FinishWork &= ~0x04;
                    u16FinishWork |= 0x400;
                    DebugMsg("E>PowerON_MR:%08X\r\n", err);
                }
            }
            if ((u16FinishWork&0x08) > 0)
            {
                result = TestProc_Api_SampleCheckResult(&err);
                if (TESTPROC_API_RESULT_OK == result)
                {
                    u16FinishWork &= ~0x08;
                }
                else if (TESTPROC_API_RESULT_NG == result)
                {
                    u16FinishWork &= ~0x08;
                    u16FinishWork |= 0x800;
                    DebugMsg("E>PowerON_Sample:%08X\r\n", err);
                }
            }
            if ((u16FinishWork&0x10) > 0)
            {
                result = TestProc_Api_ReagentCheckResult(&err);
                if (TESTPROC_API_RESULT_OK == result)
                {
                    u16FinishWork &= ~0x10;
                }
                else if (TESTPROC_API_RESULT_NG == result)
                {
                    u16FinishWork &= ~0x10;
                    u16FinishWork |= 0x1000;
                    DebugMsg("E>PowerON_Reagent:%08X\r\n", err);
                }
            }
            
            if ((u16FinishWork&0x1F) == 0)
            {
                if ((u16FinishWork&0x1F00) == 0)
                {
                    u16FinishWork = 0;
                    err = TestProc_Api_SampleForcePrime();
                    if (err == LH_ERR_NONE)
                    {
                        u16FinishWork |= 0x01;
                    }
                    err = TestProc_Api_WashMeasureRoomDoorOperation(TESTPROC_API_DOOR_OP_FULLOPEN);
                    if (err == LH_ERR_NONE)
                    {
                        u16FinishWork |= 0x02;
                    }
                    //
                    if ((u16FinishWork&0x03) == 0x03)
                    {
                        FSM_PowerOn = FSM_POWERON_SAMPLE_FORCE_PRIME_ING;
                    }
                    else
                    {
                        FSM_PowerOn = FSM_POWERON_IDLE;
                        s32Result = TESTPROC_RESULT_NG;
                    }
                }
                else
                {
                    FSM_PowerOn = FSM_POWERON_IDLE;
                    s32Result = TESTPROC_RESULT_ERR_EXEC;
                }
            }
            break;
        case FSM_POWERON_SAMPLE_FORCE_PRIME_ING:
            if ((u16FinishWork&0x01) > 0)
            {
                result = TestProc_Api_SampleCheckResult(&err);
                if (TESTPROC_API_RESULT_OK == result)
                {
                    u16FinishWork &= ~0x01;
                }
                else if (TESTPROC_API_RESULT_NG == result)
                {
                    u16FinishWork &= ~0x01;
                    u16FinishWork |= 0x100;
                    DebugMsg("E>PowerON2_Sample:%08X\r\n", err);
                }
            }
            if ((u16FinishWork&0x02) > 0)
            {
                result = TestProc_Api_WashMeasureRoomCheckResult(&err);
                if (TESTPROC_API_RESULT_OK == result)
                {
                    u16FinishWork &= ~0x02;
                }
                else if (TESTPROC_API_RESULT_NG == result)
                {
                    u16FinishWork &= ~0x02;
                    u16FinishWork |= 0x200;
                    DebugMsg("E>PowerON_MR:%08X\r\n", err);
                }
            }
            //
            if ((u16FinishWork&0x03) == 0)
            {
                if ((u16FinishWork&0x300) == 0)
                {
                    u16FinishWork = 0;
                    err = TestProc_Api_SampleMoveToPos(TSC_NEEDLE_SAMPLE_POS_CLEAN);
                    if (err == LH_ERR_NONE)
                    {
                        u16FinishWork |= 0x01;
                    }
                    err = TestProc_Api_WashMeasureRoomPrime();
                    if (err == LH_ERR_NONE)
                    {
                        u16FinishWork |= 0x02;
                    }
                    //
                    if ((u16FinishWork&0x03) == 0x03)
                    {
                        FSM_PowerOn = FSM_POWERON_SAMPLE_TO_PRIME_ING;
                    }
                    else
                    {
                        FSM_PowerOn = FSM_POWERON_IDLE;
                        s32Result = TESTPROC_RESULT_NG;
                    }
                }
                else
                {
                    FSM_PowerOn = FSM_POWERON_IDLE;
                    s32Result = TESTPROC_RESULT_ERR_EXEC;
                }
            }
            break;
        case FSM_POWERON_SAMPLE_TO_PRIME_ING:
            if ((u16FinishWork&0x01) > 0)
            {
                result = TestProc_Api_SampleCheckResult(&err);
                if (TESTPROC_API_RESULT_OK == result)
                {
                    u16FinishWork &= ~0x01;
                }
                else if (TESTPROC_API_RESULT_NG == result)
                {
                    u16FinishWork &= ~0x01;
                    u16FinishWork |= 0x100;
                    DebugMsg("E>PowerON2_Sample:%08X\r\n", err);
                }
            }
            if ((u16FinishWork&0x02) > 0)
            {
                result = TestProc_Api_WashMeasureRoomCheckResult(&err);
                if (TESTPROC_API_RESULT_OK == result)
                {
                    u16FinishWork &= ~0x02;
                }
                else if (TESTPROC_API_RESULT_NG == result)
                {
                    u16FinishWork &= ~0x02;
                    u16FinishWork |= 0x200;
                    DebugMsg("E>PowerON2_MR:%08X\r\n", err);
                }
            }
            //
            if ((u16FinishWork&0x03) == 0)
            {
                if ((u16FinishWork&0x300) == 0)
                {
                    u16FinishWork = 0;
                    err = TestProc_Api_SamplePrime(TSC_NEEDLE_SAMPLE_PRIME_OPT_ALL);
                    if (err == LH_ERR_NONE)
                    {
                        FSM_PowerOn = FSM_POWERON_PRIME_ING;
                    }
                    else
                    {
                        FSM_PowerOn = FSM_POWERON_IDLE;
                        s32Result = TESTPROC_RESULT_NG;
                    }
                }
                else
                {
                    FSM_PowerOn = FSM_POWERON_IDLE;
                    s32Result = TESTPROC_RESULT_ERR_EXEC;
                }
            }
            break;
        case FSM_POWERON_PRIME_ING:
            result = TestProc_Api_SampleCheckResult(&err);
            if (TESTPROC_API_RESULT_OK == result)
            {
                FSM_PowerOn = FSM_POWERON_STANDBY;
            }
            else if (TESTPROC_API_RESULT_NG == result)
            {
                FSM_PowerOn = FSM_POWERON_IDLE;
                s32Result = TESTPROC_RESULT_ERR_EXEC;
            }
            break;
        case FSM_POWERON_PREHEATING:
            pstStatus->stCellStatus.u8WorkStatus = TESTPROC_WORK_PRIME;
            break;
        case FSM_POWERON_STANDBY: // Test Code
            TestProc_Standby(pstStatus);
            break;
        case FSM_POWERON_IDLE:
        default:
            
            break;
    }
    
    if (TESTPROC_RESULT_OK != s32Result)
    {
        DebugMsg("E>SampleAccuracy:%d,%d,%d,%04X,%08X,%d\r\n", s32Result, FSM_SampleAccuracy, FSM_Old, u16FinishWork, err, result);
    }
    
    return TESTPROC_RESULT_OK;
}

/****************************************************************************
*
* Function Name:    TestProc_SampleAccuracyFSM
* Input:            pstStatus - 系统状态
*                   
* Output:           None
*                   
* Returns:          TESTPROC_RESULT_OK
* Description:      
* Note:             
***************************************************************************/
static int32_t TestProc_SampleAccuracyFSM(TESTPROC_STATUS_T *pstStatus)
{
    int32_t  s32Result = TESTPROC_RESULT_OK;
    TESTPROC_API_RESULT_E eCheckResult = TESTPROC_API_RESULT_OK;
    LH_ERR errCode = LH_ERR_NONE;
    LH_ERR errCode2 = LH_ERR_NONE;
    static FSM_SAMPLE_ACCURACY_E FSM_Old = FSM_SAMPLE_ACCURACY_IDLE;
    static uint16_t u16WorkMask = 0;

    if (FSM_Old != FSM_SampleAccuracy) DebugMsg("SA:%d,%04X\r\n", FSM_SampleAccuracy, u16WorkMask);
    FSM_Old = FSM_SampleAccuracy;
    switch (FSM_SampleAccuracy)
    {
        case FSM_SAMPLE_ACCURACY_START:
            u16WorkMask = 0;
            errCode = TestProc_Api_HandCupReset();
            if (LH_ERR_NONE == errCode)
            {
                u16WorkMask |= 0x01;
            }
            errCode = TestProc_Api_SampleReset();
            if (LH_ERR_NONE == errCode)
            {
                u16WorkMask |= 0x02;
            }
            if ((u16WorkMask&0x03) == 0x03)
            {
                FSM_SampleAccuracy = FSM_SAMPLE_ACCURACY_RESET_1;
            }
            else
            {
                FSM_SampleAccuracy = FSM_SAMPLE_ACCURACY_IDLE;
                s32Result = TESTPROC_RESULT_NG;
            }
            break;
        case FSM_SAMPLE_ACCURACY_RESET_1:
            if ((u16WorkMask&0x01) > 0)
            {
                eCheckResult = TestProc_Api_HandCupCheckResult(&errCode2);
                if (TESTPROC_API_RESULT_OK == eCheckResult)
                {
                    u16WorkMask &= ~0x01;
                }
                else if (TESTPROC_API_RESULT_NG == eCheckResult)
                {
                    u16WorkMask &= ~0x01;
                    u16WorkMask |= 0x100;
                }
            }
            if ((u16WorkMask&0x02) > 0)
            {
                eCheckResult = TestProc_Api_SampleCheckResult(&errCode);
                if (TESTPROC_API_RESULT_OK == eCheckResult)
                {
                    u16WorkMask &= ~0x02;
                }
                else if (TESTPROC_API_RESULT_NG == eCheckResult)
                {
                    u16WorkMask &= ~0x02;
                    u16WorkMask |= 0x200;
                }
            }
            if ((u16WorkMask&0x03) == 0)
            {
                if ((u16WorkMask&0x300) == 0)
                {
                    u16WorkMask = 0;
                    errCode = TestProc_Api_ReactReset();
                    if (LH_ERR_NONE == errCode)
                    {
                        u16WorkMask |= 0x01;
                    }
                    errCode = TestProc_Api_SampleConveyorBeltReset();
                    if (LH_ERR_NONE == errCode)
                    {
                        u16WorkMask |= 0x02;
                    }
                    //
                    if ((u16WorkMask&0x03) == 0x03)
                    {
                        FSM_SampleAccuracy = FSM_SAMPLE_ACCURACY_RESET_2;
                    }
                    else
                    {
                        FSM_SampleAccuracy = FSM_SAMPLE_ACCURACY_IDLE;
                        s32Result = TESTPROC_RESULT_NG;
                    }
                }
                else
                {
                    FSM_SampleAccuracy = FSM_SAMPLE_ACCURACY_IDLE;
                    s32Result = TESTPROC_RESULT_NG;
                }
            }
            break;
        case FSM_SAMPLE_ACCURACY_RESET_2:
            if ((u16WorkMask&0x01) > 0)
            {
                eCheckResult = TestProc_Api_ReactCheckResult();
                if (TESTPROC_API_RESULT_OK == eCheckResult)
                {
                    u16WorkMask &= ~0x01;
                }
                else if (TESTPROC_API_RESULT_NG == eCheckResult)
                {
                    u16WorkMask &= ~0x01;
                    u16WorkMask |= 0x100;
                }
            }
            if ((u16WorkMask&0x02) > 0)
            {
                eCheckResult = TestProc_Api_SampleConveyorBeltCheckResult(&errCode);
                if (TESTPROC_API_RESULT_OK == eCheckResult)
                {
                    u16WorkMask &= ~0x02;
                }
                else if (TESTPROC_API_RESULT_NG == eCheckResult)
                {
                    u16WorkMask &= ~0x02;
                    u16WorkMask |= 0x200;
                }
            }
            //
            if ((u16WorkMask&0x03) == 0)
            {
                if ((u16WorkMask&0x300) == 0)
                {
                    u16WorkMask = 0;
                    errCode = TestProc_Api_SampleMoveToPos(TSC_NEEDLE_SAMPLE_POS_RACK);
                    if (LH_ERR_NONE == errCode)
                    {
                        u16WorkMask |= 0x01;
                    }
                    errCode = TestProc_Api_SampleConveyorBeltMoveToPos(1);
                    if (LH_ERR_NONE == errCode)
                    {
                        u16WorkMask |= 0x02;
                    }
                    //
                    if ((u16WorkMask&0x03) == 0x03)
                    {
                        FSM_SampleAccuracy = FSM_SAMPLE_ACCURACY_MOVE_RACK;
                    }
                    else
                    {
                        FSM_SampleAccuracy = FSM_SAMPLE_ACCURACY_IDLE;
                        s32Result = TESTPROC_RESULT_NG;
                    }
                }
                else
                {
                    FSM_SampleAccuracy = FSM_SAMPLE_ACCURACY_IDLE;
                    s32Result = TESTPROC_RESULT_NG;
                }
            }
            break;
        case FSM_SAMPLE_ACCURACY_MOVE_RACK:
            if ((u16WorkMask&0x01) > 0)
            {
                eCheckResult = TestProc_Api_SampleCheckResult(&errCode);
                if (TESTPROC_API_RESULT_OK == eCheckResult)
                {
                    u16WorkMask &= ~0x01;
                }
                else if (TESTPROC_API_RESULT_NG == eCheckResult)
                {
                    u16WorkMask &= ~0x01;
                    u16WorkMask |= 0x100;
                }
            }
            if ((u16WorkMask&0x02) > 0)
            {
                eCheckResult = TestProc_Api_SampleConveyorBeltCheckResult(&errCode2);
                if (TESTPROC_API_RESULT_OK == eCheckResult)
                {
                    u16WorkMask &= ~0x02;
                }
                else if (TESTPROC_API_RESULT_NG == eCheckResult)
                {
                    u16WorkMask &= ~0x02;
                    u16WorkMask |= 0x200;
                }
            }
            //
            if ((u16WorkMask&0x03) == 0)
            {
                if ((u16WorkMask&0x300) == 0)
                {
                    u16WorkMask = 0;
                    FSM_SampleAccuracy = FSM_SAMPLE_ACCURACY_SAMPLE_ABSORB;
                    errCode = TestProc_Api_SampleAbsorb(g_u16AccuracyTestQuantity, 1);
                    if (errCode != LH_ERR_NONE)
                    {
                        FSM_SampleAccuracy = FSM_SAMPLE_ACCURACY_IDLE;
                        s32Result = TESTPROC_RESULT_NG;
                    }
                }
                else
                {
                    FSM_SampleAccuracy = FSM_SAMPLE_ACCURACY_IDLE;
                    s32Result = TESTPROC_RESULT_ERR_EXEC;
                }
            }
            break;
        case FSM_SAMPLE_ACCURACY_SAMPLE_ABSORB:
            eCheckResult = TestProc_Api_SampleCheckResult(&errCode);
            if (TESTPROC_API_RESULT_OK == eCheckResult)
            {
                FSM_SampleAccuracy = FSM_SAMPLE_ACCURACY_TO_REACT;
                errCode = TestProc_Api_SampleMoveToPos(TSC_NEEDLE_SAMPLE_POS_REACTION_INJECT);
                if (errCode != LH_ERR_NONE)
                {
                    FSM_SampleAccuracy = FSM_SAMPLE_ACCURACY_IDLE;
                    s32Result = TESTPROC_RESULT_NG;
                }
            }
            else if (TESTPROC_API_RESULT_NG == eCheckResult)
            {
                FSM_SampleAccuracy = FSM_SAMPLE_ACCURACY_IDLE;
                s32Result = TESTPROC_RESULT_ERR_EXEC;
            }
            break;
        case FSM_SAMPLE_ACCURACY_TO_REACT:
            eCheckResult = TestProc_Api_SampleCheckResult(&errCode);
            if (TESTPROC_API_RESULT_OK == eCheckResult)
            {
                FSM_SampleAccuracy = FSM_SAMPLE_ACCURACY_SAMPLE_INJECT;
                errCode = TestProc_Api_SampleInject(g_u16AccuracyTestQuantity);
                if (errCode != LH_ERR_NONE)
                {
                    FSM_SampleAccuracy = FSM_SAMPLE_ACCURACY_IDLE;
                    s32Result = TESTPROC_RESULT_NG;
                }
            }
            else if (TESTPROC_API_RESULT_NG == eCheckResult)
            {
                FSM_SampleAccuracy = FSM_SAMPLE_ACCURACY_IDLE;
                s32Result = TESTPROC_RESULT_ERR_EXEC;
            }
            break;
        case FSM_SAMPLE_ACCURACY_SAMPLE_INJECT:
            eCheckResult = TestProc_Api_SampleCheckResult(&errCode);
            if (TESTPROC_API_RESULT_OK == eCheckResult)
            {
                FSM_SampleAccuracy = FSM_SAMPLE_ACCURACY_TO_CLEAN;
                errCode = TestProc_Api_SampleMoveToPos(TSC_NEEDLE_SAMPLE_POS_CLEAN);
                if (errCode != LH_ERR_NONE)
                {
                    FSM_SampleAccuracy = FSM_SAMPLE_ACCURACY_IDLE;
                    s32Result = TESTPROC_RESULT_NG;
                }
            }
            else if (TESTPROC_API_RESULT_NG == eCheckResult)
            {
                FSM_SampleAccuracy = FSM_SAMPLE_ACCURACY_IDLE;
                s32Result = TESTPROC_RESULT_ERR_EXEC;
            }
            break;
        case FSM_SAMPLE_ACCURACY_TO_CLEAN:
            eCheckResult = TestProc_Api_SampleCheckResult(&errCode);
            if (TESTPROC_API_RESULT_OK == eCheckResult)
            {
                FSM_SampleAccuracy = FSM_SAMPLE_ACCURACY_SAMPLE_CLEAN;
                errCode = TestProc_Api_SampleClean(TSC_NEEDLE_SAMPLE_CLEAN_OPT_ALL);
                if (errCode != LH_ERR_NONE)
                {
                    FSM_SampleAccuracy = FSM_SAMPLE_ACCURACY_IDLE;
                    s32Result = TESTPROC_RESULT_NG;
                }
            }
            else if (TESTPROC_API_RESULT_NG == eCheckResult)
            {
                FSM_SampleAccuracy = FSM_SAMPLE_ACCURACY_IDLE;
                s32Result = TESTPROC_RESULT_ERR_EXEC;
            }
            break;
        case FSM_SAMPLE_ACCURACY_SAMPLE_CLEAN:
            if (TESTPROC_API_RESULT_OK == eCheckResult)
            {
                FSM_SampleAccuracy = FSM_SAMPLE_ACCURACY_IDLE;
                TestProc_Standby(pstStatus);
            }
            else if (TESTPROC_API_RESULT_NG == eCheckResult)
            {
                FSM_SampleAccuracy = FSM_SAMPLE_ACCURACY_IDLE;
                s32Result = TESTPROC_RESULT_ERR_EXEC;
            }
            break;
        default:
            DebugMsg("E>SampleAccuracy:%d\r\n", FSM_SampleAccuracy);
            break;
    }
    
    if (TESTPROC_RESULT_OK != s32Result)
    {
        DebugMsg("E>SampleAccuracy:%d,%d,%d,%04X,%08X,%08X,%d\r\n", s32Result, FSM_SampleAccuracy, FSM_Old, u16WorkMask, errCode, errCode2, eCheckResult);
    }

    return s32Result;
}

/****************************************************************************
*
* Function Name:    TestProc_ReagentAccuracyFSM
* Input:            pstStatus - 系统状态
*                   
* Output:           None
*                   
* Returns:          TESTPROC_RESULT_OK
* Description:      
* Note:             
***************************************************************************/
static int32_t TestProc_ReagentAccuracyFSM(TESTPROC_STATUS_T *pstStatus)
{
    int32_t  s32Result = TESTPROC_RESULT_OK;
    TESTPROC_API_RESULT_E eCheckResult = TESTPROC_API_RESULT_OK;
    LH_ERR errCode = LH_ERR_NONE;
    static FSM_REAGENT_ACCURACY_E FSM_Old = FSM_REAGENT_ACCURACY_IDLE;
    static uint16_t u16WorkMask = 0;

    if (FSM_Old != FSM_ReagentAccuracy) DebugMsg("RA:%d,%04X\r\n", FSM_ReagentAccuracy, u16WorkMask);
    FSM_Old = FSM_ReagentAccuracy;
    switch (FSM_ReagentAccuracy)
    {
        case FSM_REAGENT_ACCURACY_START:
            u16WorkMask = 0;
            errCode = TestProc_Api_ReagentReset();
            if (LH_ERR_NONE == errCode)
            {
                u16WorkMask |= 0x01;
            }
            errCode = TestProc_Api_ReactMixReset();
            if (LH_ERR_NONE == errCode)
            {
                u16WorkMask |= 0x02;
            }
            errCode = TestProc_Api_ReactMMixReset();
            if (LH_ERR_NONE == errCode)
            {
                u16WorkMask |= 0x04;
            }
            //
            if ((u16WorkMask&0x07) == 0x07)
            {
                FSM_ReagentAccuracy = FSM_REAGENT_ACCURACY_RESET_1;
            }
            else
            {
                FSM_ReagentAccuracy = FSM_REAGENT_ACCURACY_IDLE;
                s32Result = TESTPROC_RESULT_NG;
            }
            break;
        case FSM_REAGENT_ACCURACY_RESET_1:
            if ((u16WorkMask&0x01) > 0)
            {
                eCheckResult = TestProc_Api_ReagentCheckResult(&errCode);
                if (TESTPROC_API_RESULT_OK == eCheckResult)
                {
                    u16WorkMask &= ~0x01;
                }
                else if (TESTPROC_API_RESULT_NG == eCheckResult)
                {
                    u16WorkMask &= ~0x01;
                    u16WorkMask |= 0x100;
                }
            }
            if ((u16WorkMask&0x02) > 0)
            {
                eCheckResult = TestProc_Api_ReactCheckResult();
                if (TESTPROC_API_RESULT_OK == eCheckResult)
                {
                    u16WorkMask &= ~0x02;
                }
                else if (TESTPROC_API_RESULT_NG == eCheckResult)
                {
                    u16WorkMask &= ~0x02;
                    u16WorkMask |= 0x200;
                }
            }
            if ((u16WorkMask&0x04) > 0)
            {
                eCheckResult = TestProc_Api_ReactMCheckResult();
                if (TESTPROC_API_RESULT_OK == eCheckResult)
                {
                    u16WorkMask &= ~0x04;
                }
                else if (TESTPROC_API_RESULT_NG == eCheckResult)
                {
                    u16WorkMask &= ~0x04;
                    u16WorkMask |= 0x400;
                }
            }
            //
            if ((u16WorkMask&0x07) == 0)
            {
                if ((u16WorkMask&0x700) == 0)
                {
                    u16WorkMask = 0;
                    errCode = TestProc_Api_ReagentDiskReset();
                    if (LH_ERR_NONE == errCode)
                    {
                        u16WorkMask |= 0x01;
                    }
                    errCode = TestProc_Api_ReactReset();
                    if (LH_ERR_NONE == errCode)
                    {
                        u16WorkMask |= 0x02;
                    }
                    if ((u16WorkMask&0x03) == 0x03)
                    {
                        FSM_ReagentAccuracy = FSM_REAGENT_ACCURACY_RESET_2;
                    }
                    else
                    {
                        FSM_ReagentAccuracy = FSM_REAGENT_ACCURACY_IDLE;
                        s32Result = TESTPROC_RESULT_NG;
                    }
                }
                else
                {
                    FSM_ReagentAccuracy = FSM_REAGENT_ACCURACY_IDLE;
                    s32Result = TESTPROC_RESULT_ERR_EXEC;
                }
            }
            break;
        case FSM_REAGENT_ACCURACY_RESET_2:
            if ((u16WorkMask&0x01) > 0)
            {
                eCheckResult = TestProc_Api_ReagentDiskCheckResult(&errCode);
                if (TESTPROC_API_RESULT_OK == eCheckResult)
                {
                    u16WorkMask &= ~0x01;
                }
                else if (TESTPROC_API_RESULT_NG == eCheckResult)
                {
                    u16WorkMask &= ~0x01;
                    u16WorkMask |= 0x100;
                }
            }
            if ((u16WorkMask&0x02) > 0)
            {
                eCheckResult = TestProc_Api_ReactCheckResult();
                if (TESTPROC_API_RESULT_OK == eCheckResult)
                {
                    u16WorkMask &= ~0x02;
                }
                else if (TESTPROC_API_RESULT_NG == eCheckResult)
                {
                    u16WorkMask &= ~0x02;
                    u16WorkMask |= 0x200;
                }
            }
            //
            if ((u16WorkMask&0x03) == 0)
            {
                if ((u16WorkMask&0x300) == 0)
                {
                    u16WorkMask = 0;
                    errCode = TestProc_Api_ReagentDiskMoveToPos(1, DISH_REAGENT_OFFSET_R1);
                    if (LH_ERR_NONE == errCode)
                    {
                        u16WorkMask |= 0x01;
                    }
                    errCode = TestProc_Api_ReagentMoveToPos(TSC_NEEDLE_REAGENT_BEAD_POS_R1);
                    if (LH_ERR_NONE == errCode)
                    {
                        u16WorkMask |= 0x02;
                    }
                    if ((u16WorkMask&0x03) == 0x03)
                    {
                        FSM_ReagentAccuracy = FSM_REAGENT_ACCURACY_TO_DISK;
                    }
                    else
                    {
                        FSM_ReagentAccuracy = FSM_REAGENT_ACCURACY_IDLE;
                        s32Result = TESTPROC_RESULT_NG;
                    }
                }
                else
                {
                    FSM_ReagentAccuracy = FSM_REAGENT_ACCURACY_IDLE;
                    s32Result = TESTPROC_RESULT_ERR_EXEC;
                }
            }
            break;
        case FSM_REAGENT_ACCURACY_TO_DISK:
            if ((u16WorkMask&0x01) > 0)
            {
                eCheckResult = TestProc_Api_ReagentDiskCheckResult(&errCode);
                if (TESTPROC_API_RESULT_OK == eCheckResult)
                {
                    u16WorkMask &= ~0x01;
                }
                else if (TESTPROC_API_RESULT_NG == eCheckResult)
                {
                    u16WorkMask &= ~0x01;
                    u16WorkMask |= 0x100;
                }
            }
            if ((u16WorkMask&0x02) > 0)
            {
                eCheckResult = TestProc_Api_ReagentCheckResult(&errCode);
                if (TESTPROC_API_RESULT_OK == eCheckResult)
                {
                    u16WorkMask &= ~0x02;
                }
                else if (TESTPROC_API_RESULT_NG == eCheckResult)
                {
                    u16WorkMask &= ~0x02;
                    u16WorkMask |= 0x200;
                }
            }
            //
            if ((u16WorkMask&0x03) == 0)
            {
                if ((u16WorkMask&0x300) == 0)
                {
                    u16WorkMask = 0;
                    FSM_ReagentAccuracy = FSM_REAGENT_ACCURACY_ABSORB;
                    errCode = TestProc_Api_ReagentAbsorb(g_u16AccuracyTestQuantity, 0, 0);
                    if (errCode != LH_ERR_NONE)
                    {
                        FSM_ReagentAccuracy = FSM_REAGENT_ACCURACY_IDLE;
                        s32Result = TESTPROC_RESULT_NG;
                    }
                }
                else
                {
                    FSM_ReagentAccuracy = FSM_REAGENT_ACCURACY_IDLE;
                    s32Result = TESTPROC_RESULT_ERR_EXEC;
                }
            }
            break;
        case FSM_REAGENT_ACCURACY_ABSORB:
            eCheckResult = TestProc_Api_ReagentCheckResult(&errCode);
            if (TESTPROC_API_RESULT_OK == eCheckResult)
            {
                FSM_ReagentAccuracy = FSM_REAGENT_ACCURACY_TO_REACT;
                errCode = TestProc_Api_ReagentMoveToPos(TSC_NEEDLE_REAGENT_BEAD_POS_REACTION);
                if (errCode != LH_ERR_NONE)
                {
                    FSM_ReagentAccuracy = FSM_REAGENT_ACCURACY_IDLE;
                    s32Result = TESTPROC_RESULT_NG;
                }
            }
            else if (TESTPROC_API_RESULT_NG == eCheckResult)
            {
                FSM_ReagentAccuracy = FSM_REAGENT_ACCURACY_IDLE;
                s32Result = TESTPROC_RESULT_ERR_EXEC;
            }
            break;
        case FSM_REAGENT_ACCURACY_TO_REACT:
            eCheckResult = TestProc_Api_ReagentCheckResult(&errCode);
            if (TESTPROC_API_RESULT_OK == eCheckResult)
            {
                FSM_ReagentAccuracy = FSM_REAGENT_ACCURACY_INJECT;
                errCode = TestProc_Api_ReagentInject(g_u16AccuracyTestQuantity);
                if (errCode != LH_ERR_NONE)
                {
                    FSM_ReagentAccuracy = FSM_REAGENT_ACCURACY_IDLE;
                    s32Result = TESTPROC_RESULT_NG;
                }
            }
            else if (TESTPROC_API_RESULT_NG == eCheckResult)
            {
                FSM_ReagentAccuracy = FSM_REAGENT_ACCURACY_IDLE;
                s32Result = TESTPROC_RESULT_ERR_EXEC;
            }
            break;
        case FSM_REAGENT_ACCURACY_INJECT:
            eCheckResult = TestProc_Api_ReagentCheckResult(&errCode);
            if (TESTPROC_API_RESULT_OK == eCheckResult)
            {
                FSM_ReagentAccuracy = FSM_REAGENT_ACCURACY_TO_CLEAN;
                errCode = TestProc_Api_ReagentMoveToPos(TSC_NEEDLE_REAGENT_BEAD_POS_CLEAN);
                if (errCode != LH_ERR_NONE)
                {
                    FSM_ReagentAccuracy = FSM_REAGENT_ACCURACY_IDLE;
                    s32Result = TESTPROC_RESULT_NG;
                }
            }
            else if (TESTPROC_API_RESULT_NG == eCheckResult)
            {
                FSM_ReagentAccuracy = FSM_REAGENT_ACCURACY_IDLE;
                s32Result = TESTPROC_RESULT_ERR_EXEC;
            }
            break;
        case FSM_REAGENT_ACCURACY_TO_CLEAN:
            eCheckResult = TestProc_Api_ReagentCheckResult(&errCode);
            if (TESTPROC_API_RESULT_OK == eCheckResult)
            {
                FSM_ReagentAccuracy = FSM_REAGENT_ACCURACY_CLEAN_ING;
                errCode = TestProc_Api_ReagentClean(TSC_NEEDLE_REAGENT_BEAD_POS_CLEAN, TSC_NEEDLE_REAGENT_BEAD_CLEAN_OPT_IN_OUT);
                if (errCode != LH_ERR_NONE)
                {
                    FSM_ReagentAccuracy = FSM_REAGENT_ACCURACY_IDLE;
                    s32Result = TESTPROC_RESULT_NG;
                }
            }
            else if (TESTPROC_API_RESULT_NG == eCheckResult)
            {
                FSM_ReagentAccuracy = FSM_REAGENT_ACCURACY_IDLE;
                s32Result = TESTPROC_RESULT_ERR_EXEC;
            }
            break;
        case FSM_REAGENT_ACCURACY_CLEAN_ING:
            eCheckResult = TestProc_Api_ReagentCheckResult(&errCode);
            if (TESTPROC_API_RESULT_OK == eCheckResult)
            {
                TestProc_Standby(pstStatus);
            }
            else if (TESTPROC_API_RESULT_NG == eCheckResult)
            {
                TestProc_Standby(pstStatus);
                s32Result = TESTPROC_RESULT_ERR_EXEC;
            }
            break;
        default:
            DebugMsg("E>ReagentAccuracy:%d\r\n", FSM_ReagentAccuracy);
            break;
    }
    
    if (TESTPROC_RESULT_OK != s32Result)
    {
        DebugMsg("E>ReagentAccuracy:%d,%d,%d,%04X,%08X,%d\r\n", s32Result, FSM_ReagentAccuracy, FSM_Old, u16WorkMask, errCode, eCheckResult);
    }

    return s32Result;
}

/****************************************************************************
*
* Function Name:    TestProc_WashAccuracyFSM
* Input:            pstStatus - 系统状态
*                   
* Output:           None
*                   
* Returns:          TESTPROC_RESULT_OK
* Description:      
* Note:             
***************************************************************************/
static int32_t TestProc_WashAccuracyFSM(TESTPROC_STATUS_T *pstStatus)
{
    int32_t  s32Result = TESTPROC_RESULT_OK;
    TESTPROC_API_RESULT_E eCheckResult = TESTPROC_API_RESULT_OK;
    LH_ERR errCode = LH_ERR_NONE;
    static FSM_WASH_ACCURACY_E FSM_Old = FSM_WASH_ACCURACY_IDLE;
    static uint16_t u16WorkMask = 0;

    if (FSM_Old != FSM_WashAccuracy) DebugMsg("WA:%d,%04X\r\n", FSM_WashAccuracy, u16WorkMask);
    FSM_Old = FSM_WashAccuracy;
    switch (FSM_WashAccuracy)
    {
        case FSM_WASH_ACCURACY_START:
            u16WorkMask = 0;
            errCode = TestProc_Api_WashDiskMixReset();
            if (LH_ERR_NONE == errCode)
            {
                u16WorkMask |= 0x01;
            }
            errCode = TestProc_Api_WashLiquidAReset();
            if (LH_ERR_NONE == errCode)
            {
                u16WorkMask |= 0x02;
            }
            errCode = TestProc_Api_WashNeedleReset();
            if (LH_ERR_NONE == errCode)
            {
                u16WorkMask |= 0x04;
            }
            //
            if ((u16WorkMask&0x07) == 0x07)
            {
                FSM_WashAccuracy = FSM_WASH_ACCURACY_RESET_1;
            }
            else
            {
                FSM_WashAccuracy = FSM_WASH_ACCURACY_IDLE;
                s32Result = TESTPROC_RESULT_NG;
            }
            break;
        case FSM_WASH_ACCURACY_RESET_1:
            if ((u16WorkMask&0x01) > 0)
            {
                eCheckResult = TestProc_Api_WashDiskCheckResult(&errCode);
                if (TESTPROC_API_RESULT_OK == eCheckResult)
                {
                    u16WorkMask &= ~0x01;
                }
                else if (TESTPROC_API_RESULT_NG == eCheckResult)
                {
                    u16WorkMask &= ~0x01;
                    u16WorkMask |= 0x100;
                    DebugMsg("E>WashAccuracy_WashDisk:%08X\r\n", errCode);
                }
            }
            if ((u16WorkMask&0x02) > 0)
            {
                eCheckResult = TestProc_Api_WashLiquidACheckResult(&errCode);
                if (TESTPROC_API_RESULT_OK == eCheckResult)
                {
                    u16WorkMask &= ~0x02;
                }
                else if (TESTPROC_API_RESULT_NG == eCheckResult)
                {
                    u16WorkMask &= ~0x02;
                    u16WorkMask |= 0x200;
                    DebugMsg("E>WashAccuracy_WashLiquid:%08X\r\n", errCode);
                }
            }
            if ((u16WorkMask&0x04) > 0)
            {
                eCheckResult = TestProc_Api_WashNeedleCheckResult(&errCode);
                if (TESTPROC_API_RESULT_OK == eCheckResult)
                {
                    u16WorkMask &= ~0x04;
                }
                else if (TESTPROC_API_RESULT_NG == eCheckResult)
                {
                    u16WorkMask &= ~0x04;
                    u16WorkMask |= 0x400;
                }
            }
            if ((u16WorkMask&0x07) == 0)
            {
                if ((u16WorkMask&0x700) == 0)
                {
                    u16WorkMask = 0;
                    FSM_WashAccuracy = FSM_WASH_ACCURACY_ROTATE_ING;
                    errCode = TestProc_Api_WashDiskRotate1Pos();
                    if (errCode != LH_ERR_NONE)
                    {
                        FSM_WashAccuracy = FSM_WASH_ACCURACY_IDLE;
                        s32Result = TESTPROC_RESULT_NG;
                    }
                }
                else
                {
                    FSM_WashAccuracy = FSM_WASH_ACCURACY_IDLE;
                    s32Result = TESTPROC_RESULT_ERR_EXEC;
                }
            }
            break;
        case FSM_WASH_ACCURACY_ROTATE_ING:
            eCheckResult = TestProc_Api_WashDiskCheckResult(&errCode);
            if (TESTPROC_API_RESULT_OK == eCheckResult)
            {
                FSM_WashAccuracy = FSM_WASH_ACCURACY_CLEAN_ING;
                if ((g_u8WashAccuracyMask&0x07) > 0)
                {
                    errCode = TestProc_Api_WashNeedleClean((TSC_NEEDLE_WASH_SELECT_FLAG)(g_u8WashAccuracyMask&0x07), CLEAN_NEEDLE_INDEX_FIRST);
                }
                else
                {
                    errCode = TestProc_Api_WashNeedleClean((TSC_NEEDLE_WASH_SELECT_FLAG)((g_u8WashAccuracyMask>>4)&0x07), CLEAN_NEEDLE_INDEX_SECOND);
                }
                if (errCode != LH_ERR_NONE)
                {
                    FSM_WashAccuracy = FSM_WASH_ACCURACY_IDLE;
                    s32Result = TESTPROC_RESULT_NG;
                }
            }
            else if (TESTPROC_API_RESULT_NG == eCheckResult)
            {
                FSM_WashAccuracy = FSM_WASH_ACCURACY_IDLE;
                s32Result = TESTPROC_RESULT_ERR_EXEC;
            }
            break;
        case FSM_WASH_ACCURACY_CLEAN_ING:
            eCheckResult = TestProc_Api_WashNeedleCheckResult(&errCode);
            if (TESTPROC_API_RESULT_OK == eCheckResult)
            {
                FSM_WashAccuracy = FSM_WASH_ACCURACY_FINISH_ING;
                //errCode = TestProc_Api_WashDiskRotate1Pos();
                errCode = TestProc_Api_WashDiskReset();
                if (errCode != LH_ERR_NONE)
                {
                    FSM_WashAccuracy = FSM_WASH_ACCURACY_IDLE;
                    s32Result = TESTPROC_RESULT_NG;
                }
            }
            else if (TESTPROC_API_RESULT_NG == eCheckResult)
            {
                FSM_WashAccuracy = FSM_WASH_ACCURACY_IDLE;
                s32Result = TESTPROC_RESULT_ERR_EXEC;
            }
            break;
        case FSM_WASH_ACCURACY_FINISH_ING:
            eCheckResult = TestProc_Api_WashDiskCheckResult(&errCode);
            if (TESTPROC_API_RESULT_OK == eCheckResult)
            {
                TestProc_Standby(pstStatus);
            }
            else if (TESTPROC_API_RESULT_NG == eCheckResult)
            {
                TestProc_Standby(pstStatus);
                s32Result = TESTPROC_RESULT_ERR_EXEC;
            }
            break;
        default:
            DebugMsg("E>WashAccuracy:%d\r\n", FSM_WashAccuracy);
            break;
    }
    
    if (TESTPROC_RESULT_OK != s32Result)
    {
        DebugMsg("E>WashAccuracy:%d,%d,%d,%04X,%08X,%d\r\n", s32Result, FSM_WashAccuracy, FSM_Old, u16WorkMask, errCode, eCheckResult);
    }

    return s32Result;
}

/****************************************************************************
*
* Function Name:    TestProc_DarkTestFSM
* Input:            pstStatus - 系统状态
*                   
* Output:           None
*                   
* Returns:          TESTPROC_RESULT_OK
* Description:      
* Note:             
***************************************************************************/
static int32_t TestProc_DarkTestFSM(TESTPROC_STATUS_T *pstStatus)
{
    int32_t  s32Result = TESTPROC_RESULT_OK;
    TESTPROC_ITEM_RESULT_T stTestResult;
    TESTPROC_API_RESULT_E eCheckResult = TESTPROC_API_RESULT_OK;
    LH_ERR errCode = LH_ERR_NONE;
    static FSM_DARK_TEST_E FSM_Old = FSM_DARK_TEST_IDLE;
    static uint16_t u16WorkMask = 0;

    if (FSM_Old != FSM_DarkTest) DebugMsg("DT:%d,%04X\r\n", FSM_DarkTest, u16WorkMask);
    FSM_Old = FSM_DarkTest;
    switch (FSM_DarkTest)
    {
        case FSM_DARK_TEST_START:
            FSM_DarkTest = FSM_DARK_TEST_RESET_1;
            errCode = TestProc_Api_WashMeasureRoomReset();
            if (errCode != LH_ERR_NONE)
            {
                FSM_DarkTest = FSM_DARK_TEST_IDLE;
                s32Result = TESTPROC_RESULT_NG;
            }
            break;
        case FSM_DARK_TEST_RESET_1:
            eCheckResult = TestProc_Api_WashMeasureRoomCheckResult(&errCode);
            if (TESTPROC_API_RESULT_OK == eCheckResult)
            {
                FSM_DarkTest = FSM_DARK_TEST_MAKE_VALUE;
                errCode = TestProc_Api_WashMeasureRoomMakeValue(MEASURE_FLAG_DARK, 500);
                if (errCode != LH_ERR_NONE)
                {
                    FSM_DarkTest = FSM_DARK_TEST_IDLE;
                    s32Result = TESTPROC_RESULT_NG;
                }
            }
            else if (TESTPROC_API_RESULT_NG == eCheckResult)
            {
                FSM_DarkTest = FSM_DARK_TEST_IDLE;
                s32Result = TESTPROC_RESULT_NG;
            }
            break;
        case FSM_DARK_TEST_MAKE_VALUE:
            eCheckResult = TestProc_Api_WashMeasureRoomCheckResult(&errCode);
            if (TESTPROC_API_RESULT_OK == eCheckResult)
            {
                pstStatus->u32MeasureValue[0] = TestProc_Api_WashMeasureRoomReadValue(MEASURE_FLAG_BACK);
                Mem_Clr(&stTestResult, sizeof(TESTPROC_ITEM_RESULT_T));
                stTestResult.u32TestResult = pstStatus->u32MeasureValue[0];
                stTestResult.u32TestNmbr = 0;
                stTestResult.u8DataType = 8; // 测量光子
                stTestResult.u8CellNmbr = SystemConfig_GetCellNumber() + 1;
                stTestResult.u8ReagentPos = 0;
                stTestResult.u8TestType = 1;
                TestProc_SendDataToComm(BOARD_ID_TRACK_COMM, CAN_CMD_COMM_TESTDATA, (uint8_t *)&stTestResult, sizeof(TESTPROC_ITEM_RESULT_T));
                TestProc_Standby(pstStatus);
            }
            else if (TESTPROC_API_RESULT_NG == eCheckResult)
            {
                TestProc_Standby(pstStatus);
                s32Result = TESTPROC_RESULT_NG;
            }
            break;
        default:
            DebugMsg("E>WashAccuracy:%d\r\n", FSM_DarkTest);
            break;
    }
    
    if (TESTPROC_RESULT_OK != s32Result)
    {
        DebugMsg("E>DarkTest:%d,%d,%d,%04X,%08X,%d\r\n", s32Result, FSM_DarkTest, FSM_Old, u16WorkMask, errCode, eCheckResult);
    }

    return s32Result;
}

/****************************************************************************
*
* Function Name:    TestProc_NeedleCleanFSM
* Input:            pstStatus - 系统状态
*                   
* Output:           None
*                   
* Returns:          TESTPROC_RESULT_OK
* Description:      
* Note:             
***************************************************************************/
static int32_t TestProc_NeedleCleanFSM(TESTPROC_STATUS_T *pstStatus)
{
    int32_t  s32Result = TESTPROC_RESULT_OK;
    TESTPROC_API_RESULT_E eCheckResult = TESTPROC_API_RESULT_OK;
    LH_ERR errCode = LH_ERR_NONE;
    static FSM_NEEDLE_CLEAN_E FSM_Old = FSM_NEEDLE_CLEAN_IDLE;
    static uint16_t u16WorkMask = 0;

    if (FSM_Old != FSM_NeedleClean) DebugMsg("NC:%d,%04X\r\n", FSM_NeedleClean, u16WorkMask);
    FSM_Old = FSM_NeedleClean;
    switch (FSM_NeedleClean)
    {
        case FSM_NEEDLE_CLEAN_START:
            u16WorkMask = 0;
            if ((g_u32MaintainWorkFlag&0x0C) > 0)
            {
                errCode = TestProc_Api_SampleReset();
                if (LH_ERR_NONE == errCode)
                {
                    u16WorkMask |= 0x01;
                }
            }
            if ((g_u32MaintainWorkFlag&0x03) > 0)
            {
                errCode = TestProc_Api_ReagentReset();
                if (LH_ERR_NONE == errCode)
                {
                    u16WorkMask |= 0x02;
                }
            }
            //
            if ((u16WorkMask&0x03) > 0)
            {
                FSM_NeedleClean = FSM_NEEDLE_CLEAN_RESET_1;
            }
            else
            {
                FSM_NeedleClean = FSM_NEEDLE_CLEAN_IDLE;
                s32Result = TESTPROC_RESULT_NG;
            }
            break;
        case FSM_NEEDLE_CLEAN_RESET_1:
            if ((u16WorkMask&0x01) > 0)
            {
                eCheckResult = TestProc_Api_SampleCheckResult(&errCode);
                if (TESTPROC_API_RESULT_OK == eCheckResult)
                {
                    u16WorkMask &= ~0x01;
                }
                else if (TESTPROC_API_RESULT_NG == eCheckResult)
                {
                    u16WorkMask &= ~0x01;
                    u16WorkMask |= 0x100;
                }
            }
            if ((u16WorkMask&0x02) > 0)
            {
                //eCheckResult = TestProc_Api_ReagentCheckResult(&errCode);
                if (TESTPROC_API_RESULT_OK == eCheckResult)
                {
                    u16WorkMask &= ~0x02;
                }
                else if (TESTPROC_API_RESULT_NG == eCheckResult)
                {
                    u16WorkMask &= ~0x02;
                    u16WorkMask |= 0x200;
                }
            }
            if ((u16WorkMask&0x03) == 0)
            {
                if ((u16WorkMask&0x300) == 0)
                {
                    u16WorkMask = 0;
                    if ((g_u32MaintainWorkFlag&0x0C) > 0)
                    {
                        errCode = TestProc_Api_SampleMoveToPos(TSC_NEEDLE_SAMPLE_POS_CLEAN);
                        if (errCode == LH_ERR_NONE)
                        {
                            u16WorkMask |= 0x01;
                        }
                        else
                        {
                            FSM_NeedleClean = FSM_NEEDLE_CLEAN_IDLE;
                            s32Result = TESTPROC_RESULT_NG;
                        }
                    }
                    if ((g_u32MaintainWorkFlag&0x03) > 0)
                    {
                        errCode = TestProc_Api_ReagentMoveToPos(TSC_NEEDLE_REAGENT_BEAD_POS_CLEAN);
                        if (errCode == LH_ERR_NONE)
                        {
                            u16WorkMask |= 0x02;
                        }
                        else
                        {
                            FSM_NeedleClean = FSM_NEEDLE_CLEAN_IDLE;
                            s32Result = TESTPROC_RESULT_NG;
                        }
                    }
                    //
                    if ((u16WorkMask&0x03) > 0)
                    {
                        FSM_NeedleClean = FSM_NEEDLE_CLEAN_MOVE_ING;
                    }
                    else
                    {
                        FSM_NeedleClean = FSM_NEEDLE_CLEAN_IDLE;
                        s32Result = TESTPROC_RESULT_NG;
                    }
                }
                else
                {
                    FSM_NeedleClean = FSM_NEEDLE_CLEAN_IDLE;
                    s32Result = TESTPROC_RESULT_ERR_EXEC;
                }
            }
            break;
        case FSM_NEEDLE_CLEAN_MOVE_ING:
            if ((u16WorkMask&0x01) > 0)
            {
                eCheckResult = TestProc_Api_SampleCheckResult(&errCode);
                if (TESTPROC_API_RESULT_OK == eCheckResult)
                {
                    u16WorkMask &= ~0x01;
                }
                else if (TESTPROC_API_RESULT_NG == eCheckResult)
                {
                    u16WorkMask &= ~0x01;
                    u16WorkMask |= 0x100;
                }
            }
            if ((u16WorkMask&0x02) > 0)
            {
                eCheckResult = TestProc_Api_ReagentCheckResult(&errCode);
                if (TESTPROC_API_RESULT_OK == eCheckResult)
                {
                    u16WorkMask &= ~0x02;
                }
                else if (TESTPROC_API_RESULT_NG == eCheckResult)
                {
                    u16WorkMask &= ~0x02;
                    u16WorkMask |= 0x200;
                }
            }
            if ((u16WorkMask&0x03) == 0)
            {
                if ((u16WorkMask&0x300) == 0)
                {
                    u16WorkMask = 0;
                    if ((g_u32MaintainWorkFlag&0x04) == 0x04)
                    {
                        errCode = TestProc_Api_SampleClean(TSC_NEEDLE_SAMPLE_CLEAN_OPT_OUT);
                    }
                    else if ((g_u32MaintainWorkFlag&0x08) == 0x08)
                    {
                        errCode = TestProc_Api_SampleClean(TSC_NEEDLE_SAMPLE_CLEAN_OPT_INNER);
                    }
                    else if ((g_u32MaintainWorkFlag&0x0C) == 0x0C)
                    {
                        errCode = TestProc_Api_SampleClean(TSC_NEEDLE_SAMPLE_CLEAN_OPT_ALL);
                    }
                    if ((g_u32MaintainWorkFlag&0x0C) > 0)
                    {
                        if (errCode == LH_ERR_NONE)
                        {
                            u16WorkMask |= 0x01;
                        }
                        else
                        {
                            FSM_NeedleClean = FSM_NEEDLE_CLEAN_IDLE;
                            s32Result = TESTPROC_RESULT_NG;
                        }
                    }
                    //
                    if ((g_u32MaintainWorkFlag&0x01) == 0x01)
                    {
                        errCode = TestProc_Api_ReagentClean(TSC_NEEDLE_REAGENT_BEAD_POS_CLEAN, TSC_NEEDLE_REAGENT_BEAD_CLEAN_OPT_OUT);
                    }
                    else if ((g_u32MaintainWorkFlag&0x02) == 0x02)
                    {
                        errCode = TestProc_Api_ReagentClean(TSC_NEEDLE_REAGENT_BEAD_POS_CLEAN, TSC_NEEDLE_REAGENT_BEAD_CLEAN_OPT_IN);
                    }
                    else if ((g_u32MaintainWorkFlag&0x03) == 0x03)
                    {
                        errCode = TestProc_Api_ReagentClean(TSC_NEEDLE_REAGENT_BEAD_POS_CLEAN, TSC_NEEDLE_REAGENT_BEAD_CLEAN_OPT_IN_OUT);
                    }
                    if ((g_u32MaintainWorkFlag&0x03) > 0)
                    {
                        if (errCode == LH_ERR_NONE)
                        {
                            u16WorkMask |= 0x02;
                        }
                        else
                        {
                            FSM_NeedleClean = FSM_NEEDLE_CLEAN_IDLE;
                            s32Result = TESTPROC_RESULT_NG;
                        }
                    }
                    //
                    if ((u16WorkMask&0x03) > 0)
                    {
                        FSM_NeedleClean = FSM_NEEDLE_CLEAN_CLEAN_ING;
                    }
                    else
                    {
                        FSM_NeedleClean = FSM_NEEDLE_CLEAN_IDLE;
                        s32Result = TESTPROC_RESULT_NG;
                    }
                }
                else
                {
                    FSM_NeedleClean = FSM_NEEDLE_CLEAN_IDLE;
                    s32Result = TESTPROC_RESULT_ERR_EXEC;
                }
            }
            break;
        case FSM_NEEDLE_CLEAN_CLEAN_ING:
            if ((u16WorkMask&0x01) > 0)
            {
                eCheckResult = TestProc_Api_SampleCheckResult(&errCode);
                if (TESTPROC_API_RESULT_OK == eCheckResult)
                {
                    u16WorkMask &= ~0x01;
                }
                else if (TESTPROC_API_RESULT_NG == eCheckResult)
                {
                    u16WorkMask &= ~0x01;
                    u16WorkMask |= 0x100;
                }
            }
            if ((u16WorkMask&0x02) > 0)
            {
                eCheckResult = TestProc_Api_ReagentCheckResult(&errCode);
                if (TESTPROC_API_RESULT_OK == eCheckResult)
                {
                    u16WorkMask &= ~0x02;
                }
                else if (TESTPROC_API_RESULT_NG == eCheckResult)
                {
                    u16WorkMask &= ~0x02;
                    u16WorkMask |= 0x200;
                }
            }
            if ((u16WorkMask&0x03) == 0)
            {
                if ((u16WorkMask&0x300) > 0)
                {
                    s32Result = TESTPROC_RESULT_ERR_EXEC;
                }
                TestProc_Standby(pstStatus);
            }
            break;
        default:
            DebugMsg("E>NeedleClean:%d\r\n", FSM_NeedleClean);
            break;
    }
    
    if (TESTPROC_RESULT_OK != s32Result)
    {
        DebugMsg("E>NeedleClean:%d,%d,%d,%04X,%08X,%d\r\n", s32Result, FSM_NeedleClean, FSM_Old, u16WorkMask, errCode, eCheckResult);
    }

    return s32Result;
}

/****************************************************************************
*
* Function Name:    TestProc_LiquidPerfuseFSM
* Input:            pstStatus - 系统状态
*                   
* Output:           None
*                   
* Returns:          TESTPROC_RESULT_OK
* Description:      
* Note:             
***************************************************************************/
static int32_t TestProc_LiquidPerfuseFSM(TESTPROC_STATUS_T *pstStatus)
{
    int32_t  s32Result = TESTPROC_RESULT_OK;
    TESTPROC_API_RESULT_E eCheckResult = TESTPROC_API_RESULT_OK;
    LH_ERR errCode = LH_ERR_NONE;
    static FSM_LIQUID_PERFUSE_E FSM_Old = FSM_LIQUID_PERFUSE_IDLE;
    static uint16_t u16WorkMask = 0;

    if (FSM_Old != FSM_LiquidPerfuse) DebugMsg("DP:%d,%04X\r\n", FSM_LiquidPerfuse, u16WorkMask);
    FSM_Old = FSM_LiquidPerfuse;
    switch (FSM_LiquidPerfuse)
    {
        case FSM_LIQUID_PERFUSE_START:
            u16WorkMask = 0;
            if ((g_u32MaintainWorkFlag&0x03) > 0)
            {
                errCode = TestProc_Api_SampleReset();
                if (LH_ERR_NONE == errCode)
                {
                    u16WorkMask |= 0x01;
                }
            }
            if ((g_u32MaintainWorkFlag&0x0C) > 0)
            {
                errCode = TestProc_Api_ReagentReset();
                if (LH_ERR_NONE == errCode)
                {
                    u16WorkMask |= 0x02;
                }
            }
            if ((g_u32MaintainWorkFlag&0x10) > 0)
            {
                errCode = TestProc_Api_WashNeedleReset();
                if (LH_ERR_NONE == errCode)
                {
                    u16WorkMask |= 0x04;
                }
            }
            if ((g_u32MaintainWorkFlag&0x20) > 0)
            {
                errCode = TestProc_Api_WashLiquidAReset();
                if (LH_ERR_NONE == errCode)
                {
                    u16WorkMask |= 0x08;
                }
            }
            if ((g_u32MaintainWorkFlag&0x40) > 0)
            {
                errCode = TestProc_Api_WashMeasureRoomReset();
                if (LH_ERR_NONE == errCode)
                {
                    u16WorkMask |= 0x10;
                }
            }
            /*
            if ((g_u32MaintainWorkFlag&0x80) > 0)
            {
                errCode = TestProc_Api_WashMeasureRoomReset();
                if (LH_ERR_NONE == errCode)
                {
                    u16WorkMask |= 0x20;
                }
            }
            */
            //
            if ((u16WorkMask&0x1F) > 0)
            {
                FSM_LiquidPerfuse = FSM_LIQUID_PERFUSE_RESET_1;
            }
            else
            {
                FSM_LiquidPerfuse = FSM_LIQUID_PERFUSE_IDLE;
                s32Result = TESTPROC_RESULT_NG;
            }
            break;
        case FSM_LIQUID_PERFUSE_RESET_1:
            if ((u16WorkMask&0x01) > 0)
            {
                eCheckResult = TestProc_Api_SampleCheckResult(&errCode);
                if (TESTPROC_API_RESULT_OK == eCheckResult)
                {
                    u16WorkMask &= ~0x01;
                }
                else if (TESTPROC_API_RESULT_NG == eCheckResult)
                {
                    u16WorkMask &= ~0x01;
                    u16WorkMask |= 0x100;
                    DebugMsg("E>LiquidPer_Sample:%08X\r\n", errCode);
                }
            }
            if ((u16WorkMask&0x02) > 0)
            {
                eCheckResult = TestProc_Api_ReagentCheckResult(&errCode);
                if (TESTPROC_API_RESULT_OK == eCheckResult)
                {
                    u16WorkMask &= ~0x02;
                }
                else if (TESTPROC_API_RESULT_NG == eCheckResult)
                {
                    u16WorkMask &= ~0x02;
                    u16WorkMask |= 0x200;
                    DebugMsg("E>LiquidPer_Reagent:%08X\r\n", errCode);
                }
            }
            if ((u16WorkMask&0x04) > 0)
            {
                eCheckResult = TestProc_Api_WashNeedleCheckResult(&errCode);
                if (TESTPROC_API_RESULT_OK == eCheckResult)
                {
                    u16WorkMask &= ~0x04;
                }
                else if (TESTPROC_API_RESULT_NG == eCheckResult)
                {
                    u16WorkMask &= ~0x04;
                    u16WorkMask |= 0x400;
                    DebugMsg("E>LiquidPer_WashNeedle:%08X\r\n", errCode);
                }
            }
            if ((u16WorkMask&0x08) > 0)
            {
                eCheckResult = TestProc_Api_WashLiquidACheckResult(&errCode);
                if (TESTPROC_API_RESULT_OK == eCheckResult)
                {
                    u16WorkMask &= ~0x08;
                }
                else if (TESTPROC_API_RESULT_NG == eCheckResult)
                {
                    u16WorkMask &= ~0x08;
                    u16WorkMask |= 0x800;
                    DebugMsg("E>LiquidPer_LiuqiuA:%08X\r\n", errCode);
                }
            }
            if ((u16WorkMask&0x10) > 0)
            {
                eCheckResult = TestProc_Api_WashMeasureRoomCheckResult(&errCode);
                if (TESTPROC_API_RESULT_OK == eCheckResult)
                {
                    u16WorkMask &= ~0x10;
                }
                else if (TESTPROC_API_RESULT_NG == eCheckResult)
                {
                    u16WorkMask &= ~0x10;
                    u16WorkMask |= 0x1000;
                    DebugMsg("E>LiquidPer_MR:%08X\r\n", errCode);
                }
            }
            
            if ((u16WorkMask&0x1F) == 0)
            {
                if ((u16WorkMask&0x1F00) == 0)
                {
                    u16WorkMask = 0;
                    if ((g_u32MaintainWorkFlag&0x80) > 0)
                    {
                        errCode = TestProc_Api_SampleMoveToPos(TSC_NEEDLE_SAMPLE_POS_FORCE_CLEAN);
                    }
                    else if ((g_u32MaintainWorkFlag&0x03) > 0)
                    {
                        errCode = TestProc_Api_SampleMoveToPos(TSC_NEEDLE_SAMPLE_POS_CLEAN);
                    }
                    if (errCode == LH_ERR_NONE)
                    {
                        u16WorkMask |= 0x01;
                    }
                    else
                    {
                        FSM_LiquidPerfuse = FSM_LIQUID_PERFUSE_IDLE;
                        s32Result = TESTPROC_RESULT_NG;
                    }
                    //
                    if ((g_u32MaintainWorkFlag&0x0C) > 0)
                    {
                        errCode = TestProc_Api_ReagentMoveToPos(TSC_NEEDLE_REAGENT_BEAD_POS_CLEAN);
                        if (errCode == LH_ERR_NONE)
                        {
                            u16WorkMask |= 0x02;
                        }
                        else
                        {
                            FSM_LiquidPerfuse = FSM_LIQUID_PERFUSE_IDLE;
                            s32Result = TESTPROC_RESULT_NG;
                        }
                    }
                    //
                    if ((g_u32MaintainWorkFlag&0x10) > 0)
                    {
                        errCode = TestProc_Api_WashNeedlePrime();
                        if (errCode == LH_ERR_NONE)
                        {
                            u16WorkMask |= 0x04;
                        }
                        else
                        {
                            FSM_LiquidPerfuse = FSM_LIQUID_PERFUSE_IDLE;
                            s32Result = TESTPROC_RESULT_NG;
                        }
                    }
                    //
                    if ((g_u32MaintainWorkFlag&0x20) > 0)
                    {
                        errCode = TestProc_Api_WashLiquidAPrime();
                        if (errCode == LH_ERR_NONE)
                        {
                            u16WorkMask |= 0x08;
                        }
                        else
                        {
                            FSM_LiquidPerfuse = FSM_LIQUID_PERFUSE_IDLE;
                            s32Result = TESTPROC_RESULT_NG;
                        }
                    }
                    // 
                    if ((g_u32MaintainWorkFlag&0x40) > 0)
                    {
                        errCode = TestProc_Api_WashMeasureRoomWindowsClose();
                        if (errCode == LH_ERR_NONE)
                        {
                            u16WorkMask |= 0x10;
                        }
                        else
                        {
                            FSM_LiquidPerfuse = FSM_LIQUID_PERFUSE_IDLE;
                            s32Result = TESTPROC_RESULT_NG;
                        }
                    }
                    //
                    if ((u16WorkMask&0x1F) > 0)
                    {
                        FSM_LiquidPerfuse = FSM_LIQUID_PERFUSE_PRIME_1;
                    }
                    else
                    {
                        FSM_LiquidPerfuse = FSM_LIQUID_PERFUSE_IDLE;
                        s32Result = TESTPROC_RESULT_NG;
                    }
                }
                else
                {
                    FSM_LiquidPerfuse = FSM_LIQUID_PERFUSE_IDLE;
                    s32Result = TESTPROC_RESULT_ERR_EXEC;
                }
            }
            break;
        case FSM_LIQUID_PERFUSE_PRIME_1:
            if ((u16WorkMask&0x01) > 0)
            {
                eCheckResult = TestProc_Api_SampleCheckResult(&errCode);
                if (TESTPROC_API_RESULT_OK == eCheckResult)
                {
                    u16WorkMask &= ~0x01;
                }
                else if (TESTPROC_API_RESULT_NG == eCheckResult)
                {
                    u16WorkMask &= ~0x01;
                    u16WorkMask |= 0x100;
                    DebugMsg("E>LiuqidPer2_Sample:%08X\r\n", errCode);
                }
            }
            if ((u16WorkMask&0x02) > 0)
            {
                eCheckResult = TestProc_Api_ReagentCheckResult(&errCode);
                if (TESTPROC_API_RESULT_OK == eCheckResult)
                {
                    u16WorkMask &= ~0x02;
                }
                else if (TESTPROC_API_RESULT_NG == eCheckResult)
                {
                    u16WorkMask &= ~0x02;
                    u16WorkMask |= 0x200;
                    DebugMsg("E>LiuqidPer2_Reagent:%08X\r\n", errCode);
                }
            }
            if ((u16WorkMask&0x04) > 0)
            {
                eCheckResult = TestProc_Api_WashNeedleCheckResult(&errCode);
                if (TESTPROC_API_RESULT_OK == eCheckResult)
                {
                    u16WorkMask &= ~0x04;
                }
                else if (TESTPROC_API_RESULT_NG == eCheckResult)
                {
                    u16WorkMask &= ~0x04;
                    u16WorkMask |= 0x400;
                    DebugMsg("E>LiuqidPer2_WashNeedle:%08X\r\n", errCode);
                }
            }
            if ((u16WorkMask&0x08) > 0)
            {
                eCheckResult = TestProc_Api_WashLiquidACheckResult(&errCode);
                if (TESTPROC_API_RESULT_OK == eCheckResult)
                {
                    u16WorkMask &= ~0x08;
                }
                else if (TESTPROC_API_RESULT_NG == eCheckResult)
                {
                    u16WorkMask &= ~0x08;
                    u16WorkMask |= 0x800;
                    DebugMsg("E>LiuqidPer2_LiquidA:%08X\r\n", errCode);
                }
            }
            if ((u16WorkMask&0x10) > 0)
            {
                eCheckResult = TestProc_Api_WashMeasureRoomCheckResult(&errCode);
                if (TESTPROC_API_RESULT_OK == eCheckResult)
                {
                    u16WorkMask &= ~0x10;
                }
                else if (TESTPROC_API_RESULT_NG == eCheckResult)
                {
                    u16WorkMask &= ~0x10;
                    u16WorkMask |= 0x1000;
                }
            }
            if ((u16WorkMask&0x1F) == 0)
            {
                if ((u16WorkMask&0x1F00) == 0)
                {
                    u16WorkMask = 0;
                    if ((g_u32MaintainWorkFlag&0x80) > 0)
                    {
                        errCode = TestProc_Api_SampleForcePrime();
                    }
                    else if ((g_u32MaintainWorkFlag&0x03) > 0)
                    {
                        if ((g_u32MaintainWorkFlag&0x03) == 0x03) errCode = TestProc_Api_SamplePrime(TSC_NEEDLE_SAMPLE_PRIME_OPT_ALL);
                        else if ((g_u32MaintainWorkFlag&0x01) == 0x01) errCode = TestProc_Api_SamplePrime(TSC_NEEDLE_SAMPLE_PRIME_OPT_INNER);
                        else /*if ((g_u32MaintainWorkFlag&0x02) == 0x02)*/ errCode = TestProc_Api_SamplePrime(TSC_NEEDLE_SAMPLE_PRIME_OPT_OUT);
                        g_u32MaintainWorkFlag &= 0xFFFFFFC0;
                    }
                    if (errCode == LH_ERR_NONE)
                    {
                        u16WorkMask |= 0x01;
                    }
                    else
                    {
                        FSM_LiquidPerfuse = FSM_LIQUID_PERFUSE_IDLE;
                        s32Result = TESTPROC_RESULT_NG;
                    }
                    //
                    if ((g_u32MaintainWorkFlag&0x0C) > 0)
                    {
                        if ((g_u32MaintainWorkFlag&0x0C) == 0x0C) errCode = TestProc_Api_ReagentPrime(TSC_NEEDLE_REAGENT_PRIME_OPT_ALL);
                        else if ((g_u32MaintainWorkFlag&0x04) == 0x04) errCode = TestProc_Api_ReagentPrime(TSC_NEEDLE_REAGENT_PRIME_OPT_INNER);
                        else /*if ((g_u32MaintainWorkFlag&0x08) == 0x08)*/ errCode = TestProc_Api_ReagentPrime(TSC_NEEDLE_REAGENT_PRIME_OPT_OUT);
                        if (errCode == LH_ERR_NONE)
                        {
                            u16WorkMask |= 0x02;
                        }
                        else
                        {
                            FSM_LiquidPerfuse = FSM_LIQUID_PERFUSE_IDLE;
                            s32Result = TESTPROC_RESULT_NG;
                        }
                    }
                    //
                    if ((g_u32MaintainWorkFlag&0x40) > 0)
                    {
                        errCode = TestProc_Api_WashMeasureRoomDoorOperation(TESTPROC_API_DOOR_OP_CLOSE);
                        if (errCode == LH_ERR_NONE)
                        {
                            u16WorkMask |= 0x04;
                        }
                        else
                        {
                            FSM_LiquidPerfuse = FSM_LIQUID_PERFUSE_IDLE;
                            s32Result = TESTPROC_RESULT_NG;
                        }
                    }
                    //
                    if ((u16WorkMask&0x07) > 0)
                    {
                        FSM_LiquidPerfuse = FSM_LIQUID_PERFUSE_PRIME_2;
                    }
                    else
                    {
                        FSM_LiquidPerfuse = FSM_LIQUID_PERFUSE_IDLE;
                        s32Result = TESTPROC_RESULT_NG;
                    }
                }
                else
                {
                    FSM_LiquidPerfuse = FSM_LIQUID_PERFUSE_IDLE;
                    s32Result = TESTPROC_RESULT_ERR_EXEC;
                }
            }
            break;
        case FSM_LIQUID_PERFUSE_PRIME_2:
            if ((u16WorkMask&0x01) > 0)
            {
                eCheckResult = TestProc_Api_SampleCheckResult(&errCode);
                if (TESTPROC_API_RESULT_OK == eCheckResult)
                {
                    u16WorkMask &= ~0x01;
                }
                else if (TESTPROC_API_RESULT_NG == eCheckResult)
                {
                    u16WorkMask &= ~0x01;
                    u16WorkMask |= 0x100;
                    DebugMsg("E>LiquidPer3_Sample:%08X\r\n", errCode);
                }
            }
            if ((u16WorkMask&0x02) > 0)
            {
                eCheckResult = TestProc_Api_ReagentCheckResult(&errCode);
                if (TESTPROC_API_RESULT_OK == eCheckResult)
                {
                    u16WorkMask &= ~0x02;
                }
                else if (TESTPROC_API_RESULT_NG == eCheckResult)
                {
                    u16WorkMask &= ~0x02;
                    u16WorkMask |= 0x200;
                    DebugMsg("E>LiquidPer3_Reagent:%08X\r\n", errCode);
                }
            }
            if ((u16WorkMask&0x04) > 0)
            {
                eCheckResult = TestProc_Api_WashMeasureRoomCheckResult(&errCode);
                if (TESTPROC_API_RESULT_OK == eCheckResult)
                {
                    u16WorkMask &= ~0x04;
                }
                else if (TESTPROC_API_RESULT_NG == eCheckResult)
                {
                    u16WorkMask &= ~0x04;
                    u16WorkMask |= 0x400;
                    DebugMsg("E>LiquidPer3_MR:%08X\r\n", errCode);
                }
            }
            //
            if ((u16WorkMask&0x07) == 0)
            {
                if ((u16WorkMask&0x700) == 0)
                {
                    u16WorkMask = 0;
                    if ((g_u32MaintainWorkFlag&0x03) > 0)
                    {
                        if ((g_u32MaintainWorkFlag&0x03) == 0x03) errCode = TestProc_Api_SamplePrime(TSC_NEEDLE_SAMPLE_PRIME_OPT_ALL);
                        else if ((g_u32MaintainWorkFlag&0x01) == 0x01) errCode = TestProc_Api_SamplePrime(TSC_NEEDLE_SAMPLE_PRIME_OPT_INNER);
                        else /*if ((g_u32MaintainWorkFlag&0x02) == 0x02)*/ errCode = TestProc_Api_SamplePrime(TSC_NEEDLE_SAMPLE_PRIME_OPT_OUT);
                        if (errCode == LH_ERR_NONE)
                        {
                            u16WorkMask |= 0x01;
                        }
                        else
                        {
                            FSM_LiquidPerfuse = FSM_LIQUID_PERFUSE_IDLE;
                            s32Result = TESTPROC_RESULT_NG;
                        }
                    }
                    //
                    if ((g_u32MaintainWorkFlag&0x40) > 0)
                    {
                        errCode = TestProc_Api_WashMeasureRoomPrime();
                        if (errCode == LH_ERR_NONE)
                        {
                            u16WorkMask |= 0x02;
                        }
                        else
                        {
                            FSM_LiquidPerfuse = FSM_LIQUID_PERFUSE_IDLE;
                            s32Result = TESTPROC_RESULT_NG;
                        }
                    }
                    if ((u16WorkMask&0x03) > 0)
                    {
                        FSM_LiquidPerfuse = FSM_LIQUID_PERFUSE_PRIME_3;
                    }
                    else
                    {
                        TestProc_Standby(pstStatus);
                    }
                }
                else
                {
                    FSM_LiquidPerfuse = FSM_LIQUID_PERFUSE_IDLE;
                    s32Result = TESTPROC_RESULT_ERR_EXEC;
                }
            }
            break;
        case FSM_LIQUID_PERFUSE_PRIME_3:
            if ((u16WorkMask&0x01) > 0)
            {
                eCheckResult = TestProc_Api_SampleCheckResult(&errCode);
                if (TESTPROC_API_RESULT_OK == eCheckResult)
                {
                    u16WorkMask &= ~0x01;
                }
                else if (TESTPROC_API_RESULT_NG == eCheckResult)
                {
                    u16WorkMask &= ~0x01;
                    u16WorkMask |= 0x100;
                    DebugMsg("E>LiquidPer4_Sample:%08X\r\n", errCode);
                }
            }
            if ((u16WorkMask&0x02) > 0)
            {
                eCheckResult = TestProc_Api_WashMeasureRoomCheckResult(&errCode);
                if (TESTPROC_API_RESULT_OK == eCheckResult)
                {
                    u16WorkMask &= ~0x02;
                }
                else if (TESTPROC_API_RESULT_NG == eCheckResult)
                {
                    u16WorkMask &= ~0x02;
                    u16WorkMask |= 0x200;
                    DebugMsg("E>LiquidPer4_Sample:%08X\r\n", errCode);
                }
            }
            //
            if ((u16WorkMask&0x07) == 0)
            {
                u16WorkMask = 0;
                if ((u16WorkMask&0x700) == 0)
                {
                    TestProc_Standby(pstStatus);
                }
                else
                {
                    TestProc_Standby(pstStatus);
                    s32Result = TESTPROC_RESULT_ERR_EXEC;
                }
            }
            break;
        default:
            DebugMsg("E>LiquidPerfuse:%d\r\n", FSM_LiquidPerfuse);
            break;
    }
    
    if (TESTPROC_RESULT_OK != s32Result)
    {
        DebugMsg("E>LiquidPerfuse:%d,%d,%d,%04X,%08X,%d\r\n", s32Result, FSM_LiquidPerfuse, FSM_Old, u16WorkMask, errCode, eCheckResult);
    }

    return s32Result;
}

/****************************************************************************
*
* Function Name:    TestProc_ReagentEventFSM
* Input:            pstStatus - 系统状态
*                   
* Output:           None
*                   
* Returns:          TESTPROC_RESULT_OK
* Description:      
* Note:             
***************************************************************************/
static int32_t TestProc_ReagentEventFSM(TESTPROC_STATUS_T *pstStatus)
{
    int32_t  s32Result = TESTPROC_RESULT_OK;
    static FSM_REAGENTEVENT_E FSM_Old = FSM_REAGENTEVENT_IDLE;
    LH_ERR eErr = LH_ERR_NONE;
    TESTPROC_API_RESULT_E eCheckResult;

    if (FSM_ReagentEvent != FSM_REAGENTEVENT_IDLE) DebugMsg("RE:%d\r\n", FSM_ReagentEvent);
    FSM_Old = FSM_ReagentEvent;
    switch(FSM_ReagentEvent)
    {
        case FSM_REAGENTEVENT_MOVE:
            FSM_ReagentEvent = FSM_REAGENTEVENT_MOVE_ING;
            eErr = TestProc_Api_ReagentDiskRotate(5);
            if (eErr != LH_ERR_NONE)
            {
                s32Result = TESTPROC_RESULT_NG;
                FSM_ReagentEvent = FSM_REAGENTEVENT_IDLE;
            }
            break;
        case FSM_REAGENTEVENT_MOVE_ING:
            eCheckResult = TestProc_Api_ReagentDiskCheckResult(&eErr);
            if (TESTPROC_API_RESULT_OK == eCheckResult)
            {
                FSM_ReagentEvent = FSM_REAGENTEVENT_IDLE;
                DebugMsg("ReagentDisk Move OK\r\n");
            }
            else if (TESTPROC_API_RESULT_NG == eCheckResult)
            {
                s32Result = TESTPROC_RESULT_ERR_EXEC;
                FSM_ReagentEvent = FSM_REAGENTEVENT_IDLE;
            }
            break;
        default:

            break;
    }

    if (TESTPROC_RESULT_OK != s32Result)
    {
        DebugMsg("E>FSM_ReagentEvent:%d,%d,%d,%08X,%d\r\n", s32Result, FSM_ReagentEvent, FSM_Old, eErr, eCheckResult);
    }

    return s32Result;
}

/****************************************************************************
*
* Function Name:    TestProc_CupStackEventFSM
* Input:            pstStatus - 系统状态
*                   
* Output:           None
*                   
* Returns:          TESTPROC_RESULT_OK
* Description:      
* Note:             
***************************************************************************/
static int32_t TestProc_CupStackEventFSM(TESTPROC_STATUS_T *pstStatus)
{
    int32_t  s32Result = TESTPROC_RESULT_OK;
    static FSM_CUPSTACKEVENT_E FSM_Old = FSM_CUPSTACKEVENT_IDLE;
    LH_ERR eErr = LH_ERR_NONE;
    TESTPROC_API_RESULT_E eCheckResult;
    MACHINE_DOOR_STATUS eDoorStatus;

    if (FSM_CupStackEvent != FSM_Old) DebugMsg("CSE:%d\r\n", FSM_CupStackEvent);
    FSM_Old = FSM_CupStackEvent;
    switch(FSM_CupStackEvent)
    {
        case FSM_CUPSTACKEVENT_DEMAGNETIZE:
            eDoorStatus = TestProc_Api_CupStackGetDoorStatus();
            if (MACHINE_DOOR_STATUS_CLOSE == eDoorStatus)
            {
                FSM_CupStackEvent = FSM_CUPSTACKEVENT_DEMAGNETIZE_ING;
                eErr = TestProc_Api_CupStackDoorControl(MACHINE_DOOR_UNLOCK);
                if (eErr != LH_ERR_NONE)
                {
                    s32Result = TESTPROC_RESULT_NG;
                    FSM_CupStackEvent = FSM_CUPSTACKEVENT_IDLE;
                }
            }
            else
            {
                DebugMsg("WARNING>Demagnetize:%d\r\n", eDoorStatus);
                FSM_CupStackEvent = FSM_CUPSTACKEVENT_IDLE;
            }
            break;
        case FSM_CUPSTACKEVENT_DEMAGNETIZE_ING:
            eCheckResult = TestProc_Api_CupStackCheckResult(&eErr);
            if (TESTPROC_API_RESULT_OK == eCheckResult)
            {
                FSM_CupStackEvent = FSM_CUPSTACKEVENT_IDLE;
                if (TestProc_Api_IsCupStackDoorClose() == 0)
                {
                    pstStatus->u8CupStackDoorClose = 0;
                }
                DebugMsg("Demagnetize OK\r\n");
            }
            else if (TESTPROC_API_RESULT_NG == eCheckResult)
            {
                s32Result = TESTPROC_RESULT_ERR_EXEC;
                FSM_CupStackEvent = FSM_CUPSTACKEVENT_IDLE;
            }
            break;
        //
        case FSM_CUPSTACKEVENT_RESET:
            FSM_CupStackEvent = FSM_CUPSTACKEVENT_RESET_ING;
            eErr = TestProc_Api_CupStackReset();
            if (eErr != LH_ERR_NONE)
            {
                s32Result = TESTPROC_RESULT_NG;
                FSM_CupStackEvent = FSM_CUPSTACKEVENT_IDLE;
            }
            break;
        case FSM_CUPSTACKEVENT_RESET_ING:
            eCheckResult = TestProc_Api_CupStackCheckResult(&eErr);
            if (TESTPROC_API_RESULT_OK == eCheckResult)
            {
                FSM_CupStackEvent = FSM_CUPSTACKEVENT_WAIT_CATCH_ING;
            }
            else if (TESTPROC_API_RESULT_NG == eCheckResult)
            {
                s32Result = TESTPROC_RESULT_ERR_EXEC;
                FSM_CupStackEvent = FSM_CUPSTACKEVENT_IDLE;
            }
            break;
        case FSM_CUPSTACKEVENT_WAIT_CATCH_ING:
            if (pstStatus->eHandCupBusy != TESTPROC_HANDCUP_STATE_CATCH_NEWCUPING)
            {
                FSM_CupStackEvent = FSM_CUPSTACKEVENT_PUSH_ING;
                eErr = TestProc_Api_CupStackPushToTestPostion();
                if (eErr != LH_ERR_NONE)
                {
                    s32Result = TESTPROC_RESULT_NG;
                    FSM_CupStackEvent = FSM_CUPSTACKEVENT_IDLE;
                }
            }
            break;
        case FSM_CUPSTACKEVENT_PUSH_ING:
            eCheckResult = TestProc_Api_CupStackCheckResult(&eErr);
            if (TESTPROC_API_RESULT_OK == eCheckResult)
            {
                FSM_CupStackEvent = FSM_CUPSTACKEVENT_IDLE;
                pstStatus->u8CupStackDoorClose = 1;
                pstStatus->eCupStackState = TESTPROC_CUPSTACK_STATE_READY;
                DebugMsg("CupStack Ready\r\n");
            }
            else if (TESTPROC_API_RESULT_NG == eCheckResult)
            {
                s32Result = TESTPROC_RESULT_ERR_EXEC;
                FSM_CupStackEvent = FSM_CUPSTACKEVENT_IDLE;
            }
            break;
        default:

            break;
    }

    if (TESTPROC_RESULT_OK != s32Result)
    {
        DebugMsg("E>FSM_CupStack:%d,%d,%d,%d,%08X\r\n", s32Result, FSM_CupStackEvent, FSM_Old, eCheckResult, eErr);
    }

    return s32Result;
}

/****************************************************************************
*
* Function Name:    TestProc_ReportWarning
* Input:            pstStatus - 系统状态
*                   eWarning - 报警
*                   puParam - 报警参数
*                   u16Size - 报警参数大小
*                   
* Output:           None
*                   
* Returns:          TESTPROC_RESULT_OK
* Description:      发送报警参数到中位机
* Note:             
***************************************************************************/
static int32_t TestProc_ReportWarning(TESTPROC_STATUS_T *pstStatus, TESTPROC_WARNING_E eWarning, uint8_t *pu8Param, uint16_t u16Size)
{
    uint16_t i;
    uint8_t u8TxBuf[8];
    uint8_t u8Index = 0;

    if (eWarning >= TESTPROC_WARNING_NONE)
    {
        return TESTPROC_RESULT_ERR_PARAM;
    }
    
    u8TxBuf[u8Index++] = g_stWarningCode[eWarning].u8Board;
    u8TxBuf[u8Index++] = g_stWarningCode[eWarning].u8Level;
    u8TxBuf[u8Index++] = g_stWarningCode[eWarning].u8Type;
    u8TxBuf[u8Index++] = g_stWarningCode[eWarning].u8Code;
    if (pu8Param != NULL)
    {
        for (i = 0; (i < u16Size) && (i < 4); i++)
        {
            u8TxBuf[u8Index++] = pu8Param[i];
        }
    }
    
    if (WARNING_LEVEL_EMSTOP == g_stWarningCode[eWarning].u8Level)
    {
        pstStatus->stCellStatus.u8WorkStatus = TESTPROC_WORK_EMSTOP;
        TestProc_ReactStartWork(REACT_WORK_STOP, NULL, 0);
    }
    else if (WARNING_LEVEL_STOP == g_stWarningCode[eWarning].u8Level)
    {
        pstStatus->stCellStatus.u8WorkStatus = TESTPROC_WORK_SAMPLE_STOP;
        //TestProc_ReactStartWork(REACT_WORK_STOP, NULL, 0);
    }
    
    return CanSendDataToCell(TESTPROC_CELL_PLATFORM_NMBR, BOARD_ID_TRACK_COMM, CAN_CMD_CELL_WARNING, u8TxBuf, u8Index);
}

static int32_t TestProc_UartCmdProcess(TESTPROC_STATUS_T *pstStatus, const SYSTEMCONFIG_CMD_T *pstCmd)
{
    int32_t s32Result = TESTPROC_RESULT_OK;
    uint8_t u8OutBuf[8];
    uint16_t u16BufIndex = 0;
    
    DebugMsg("Uart1 Cmd:%02X,%02X,%d,%04X\r\n", pstCmd->u8Cmd, pstCmd->u8Body[0], pstCmd->u16BodySize, pstCmd->u16RecvCrc);
    
    switch (pstCmd->u8Cmd)
    {
        case 0x01: // 设置参数
            s32Result = SystemConfig_SetParam(pstCmd->u8Body[0], &pstCmd->u8Body[1], pstCmd->u16BodySize - 1);
            if (0 != s32Result)
            {
                DebugMsg("E>ConfigSetFail:%d,%d,%d\r\n", s32Result, pstCmd->u8Body[0], pstCmd->u8Body[1]);
                s32Result = TESTPROC_RESULT_ERR_EXEC;
            }
            else
            {
                s32Result = TESTPROC_RESULT_OK;
            }
            break;
        case 0x02: // 获取参数
            u8OutBuf[0] = pstCmd->u8Body[0];
            u16BufIndex = SystemConfig_GetParam(pstCmd->u8Body[0], &u8OutBuf[1], sizeof(u8OutBuf) - 1);
            u16BufIndex += 1;
            s32Result = SystemConfig_SendCmd(pstCmd->u8Cmd, u8OutBuf, u16BufIndex);
            if (0 != s32Result)
            {
                DebugMsg("E>ConfigGetFail:%d,%d,%d\r\n", s32Result, pstCmd->u8Body[0], pstCmd->u8Body[1]);
                s32Result = TESTPROC_RESULT_ERR_EXEC;
            }
            else
            {
                s32Result = TESTPROC_RESULT_OK;
            }
            break;
        default:
            s32Result = TESTPROC_RESULT_ERR_CMD;
            break;
    }
    
    if (s32Result != TESTPROC_RESULT_OK)
    {
        DebugMsg("E>Uart1Cmd:%d,%02X,%d,%04X\r\n", s32Result, pstCmd->u8Cmd, pstCmd->u16BodySize, pstCmd->u16RecvCrc);
    }
    
    return s32Result;
}

int32_t TestProc_IsAgeingMode(void)
{
    return STATUS_system.stCellStatus.u8WorkStatus == TESTPROC_WORK_SIMULATE ? TRUE : FALSE;
}

static void TestProc_TestTimer(void *p_tmr, void *p_arg)
{
    OS_ERR err;
    static uint8_t count = 0;
    //TestProc_ReportWarning(&STATUS_system, TESTPROC_WARNING_TIMEOUT_REACT_MIX, NULL, 0);
    if (count == 0)
    {
        //OSTmrSet(&g_stTestTimer, 3000, 3000, TestProc_TestTimer, NULL, &err);
        OSTmrStart(&g_stTestTimer, &err);
        TestProc_SendMsg(TESTPROC_MSG_CMD_EVENT_CUPSTACK_KEY_DOWN, TESTPROC_UNIT_EVENT, NULL, 0);
    }
    else if (count == 1)
    {
        TestProc_SendMsg(TESTPROC_MSG_CMD_EVENT_CUPSTACK_DOOR_CLOSE, TESTPROC_UNIT_EVENT, NULL, 0);
    }
    count ++;
    DebugMsg("$$_TestTimer_$$\r\n");
}

static void TestProc_WashDiskRotateTimer(void *p_tmr, void *p_arg)
{
    TESTPROC_STATUS_T *pstStatus = (TESTPROC_STATUS_T *)p_arg;
    
    //转动清洗盘
    if (0 == pstStatus->u8InterferenceWash && TESTPROC_WASHDISK_STATE_IDLE == pstStatus->eWashDiskState)
    {
        DebugMsg("WashDisk Rotate\r\n");
        pstStatus->eWashDiskState = TESTPROC_WASHDISK_STATE_RUNNING;
        TestProc_WashStartWork(WASH_WORK_ROTATE, NULL, 0);
    }
    else
    {
        if ((pstStatus->u8InterferenceWash & TESTPROC_INTERFERENCE_WASH_NEEDLE) > 0)
        {
            TestProc_ReportWarning(pstStatus, TESTPROC_WARNING_WASHDISK_IF_NEEDLE, NULL, 0);
        }
        if ((pstStatus->u8InterferenceWash & TESTPROC_INTERFERENCE_WASH_HAND) > 0)
        {
            TestProc_ReportWarning(pstStatus, TESTPROC_WARNING_WASHDISK_IF_HANDCUP, NULL, 0);
        }
        DebugMsg("E>WashDiskStart:%02X,%d\r\n", pstStatus->u8InterferenceWash, pstStatus->eWashDiskState);
    }
}

