#ifndef __TASK_APP_TEST_PROCESS_H_
#define __TASK_APP_TEST_PROCESS_H_
#include <stdint.h>
#include "TestProc_Typedef.h"
#include "TestProc_testData.h"

#define TESTPROC_CELL_REAGENT_NBR_MAX                   (4u)    // 含有试剂的模块数量
#define TESTPROC_CELL_1_NMBR                            (0)     // Cell 1
#define TESTPROC_CELL_2_NMBR                            (1)     // Cell 2
#define TESTPROC_CELL_3_NMBR                            (2)     // Cell 3
#define TESTPROC_CELL_4_NMBR                            (3)     // Cell 4
#define TESTPROC_CELL_ISE_NMBR                          (4)     // ise
#define TESTPROC_CELL_PLATFORM_NMBR                     (5)     // platform
#define TESTPROC_CELL_NBR_MAX                           (6u)

#define TESTPROC_INTERFERENCE_REACT_HANDCUP             (0x01)
#define TESTPROC_INTERFERENCE_REACT_REAGENT             (0x02)
#define TESTPROC_INTERFERENCE_REACT_SAMPLE              (0x04)
#define TESTPROC_INTERFERENCE_REACT_MIX1                (0x08)
#define TESTPROC_INTERFERENCE_REACT_MIX2                (0x10)
#define TESTPROC_INTERFERENCE_REACT_REAGENT2            (0x20)
#define TESTPROC_INTERFERENCE_REACT_WASH_HANDCUP        (0x40)
#define TESTPROC_INTERFERENCE_REACT_ALL                 (TESTPROC_INTERFERENCE_REACT_HANDCUP|TESTPROC_INTERFERENCE_REACT_REAGENT|  \
                                                         TESTPROC_INTERFERENCE_REACT_SAMPLE|TESTPROC_INTERFERENCE_REACT_MIX1|      \
                                                         TESTPROC_INTERFERENCE_REACT_MIX2|TESTPROC_INTERFERENCE_REACT_REAGENT2|    \
                                                         TESTPROC_INTERFERENCE_REACT_WASH_HANDCUP)

#define TESTPROC_INTERFERENCE_SAMPLE_CB                 (0x01)  // 传送带干涉样本针
#define TESTPROC_INTERFERENCE_SAMPLE_ALL                (TESTPROC_INTERFERENCE_SAMPLE_CB)

#define TESTPROC_INTERFERENCE_CB_TRACK                  (0x01)  // 轨道干涉传动带
#define TESTPROC_INTERFERENCE_CB_SAMPLE                 (0x04)  // 样本针干涉传动带
#define TESTPROC_INTERFERENCE_CB_ALL                    (TESTPROC_INTERFERENCE_CB_SAMPLE|TESTPROC_INTERFERENCE_CB_TRACK)

#define TESTPROC_INTERFERENCE_WASH_NEEDLE               (0x01)  // 清洗针干涉清洗盘
#define TESTPROC_INTERFERENCE_WASH_HAND                 (0x02)  // 机械手干涉样本盘

#define TESTPROC_ADDR1_R1                               (0x0001)
#define TESTPROC_ADDR1_R2                               (0x0002)
#define TESTPROC_ADDR1_R3                               (0x0004)
#define TESTPROC_ADDR1_M                                (0x0008)
#define TESTPROC_ADDR2_R1                               (0x0100)
#define TESTPROC_ADDR2_R2                               (0x0200)
#define TESTPROC_ADDR2_R3                               (0x0400)
#define TESTPROC_ADDR2_M                                (0x0800)

#define TESTPROC_BEFORE_TEST_ADANDONCUP                 (0x01)
#define TESTPROC_BEFORE_TEST_RECYCLERACK                (0x02)
#define TESTPROC_BEFORE_TEST_ALL                        (TESTPROC_BEFORE_TEST_ADANDONCUP|TESTPROC_BEFORE_TEST_RECYCLERACK)

typedef enum
{
    TESTPROC_REAGENT_STATE_IDLE = 0,         // 空闲,盘未启动
    // 第一步
    TESTPROC_REAGENT_RS1_R1_READY_ING,       // 内外清洗+去试剂盘R1位置
    TESTPROC_REAGENT_RS1_R1_READY,           // 吸R1准备完成
    TESTPROC_REAGENT_RS1_R1_ABSORB_ING,         // 移动到清洗盘+吐试剂+返回到清洗
    TESTPROC_REAGENT_RS1_R1_ABSORB_OK,       // 吸R1完成
    TESTPROC_REAGENT_RS1_TO_REACT_ING,       // 吸完R1后移动到反应盘上方,没有R2试剂
    TESTPROC_REAGENT_RS1_R2_ABSORB_ING,      // 移动到清洗盘西外壁+吸R2
    TESTPROC_REAGENT_RS1_R2_ABSORB_OK,       // 吸R2完成
    TESTPROC_REAGENT_RS1_R2_2STEP_READY_ING, // 内外清洗+去试剂盘R1位置
    TESTPROC_REAGENT_RS1_R2_2STEP_READY,     // 吸R2准备完成
    TESTPROC_REAGENT_RS1_R2_2STEP_ABSORB_ING,//
    TESTPROC_REAGENT_RS1_INJECT_ING,         // 吐R_1中
    TESTPROC_REAGENT_RS1_INJECT_OK,          // 吐完
    // 第二部
    TESTPROC_REAGENT_RS2_R2_READY_ING,       // 第二步吸R2准备中
    TESTPROC_REAGENT_RS2_R2_READY_OK,        // 第二步R2准备完成
    TESTPROC_REAGENT_RS2_R2_ABSORB_ING,      // 第二步吸R2中
    TESTPROC_REAGENT_RS2_R2_ABSORB_OK,       // 第二步R2完成
    TESTPROC_REAGENT_RS2_R3_READY_ING,       // 第二步吸R3准备中
    TESTPROC_REAGENT_RS2_R3_READY_OK,        // 第二步R3准备完成
    TESTPROC_REAGENT_RS2_R3_ABSORB_ING,      // 第二步吸R3中
    TESTPROC_REAGENT_RS2_R3_ABSORB_OK,       // 第二步R3完成
    TESTPROC_REAGENT_RS2_M_READY_ING,        // 第二步吸M准备中
    TESTPROC_REAGENT_RS2_M_READY_OK,         // 第二步M准备完成,试剂针已经移动到试剂盘上方
    TESTPROC_REAGENT_RS2_M_ABSORB_ING,       // 第二步吸M中
    TESTPROC_REAGENT_RS2_M_ABSORB_OK,        // 第二步M准备完成,试剂针已经移动到试剂盘上方
    TESTPROC_REAGENT_RS2_INJECT_ING,         // 吐R_2中
    TESTPROC_REAGENT_RS2_INJECT_OK,          // 吐完,吐完后停息清洗位置
    //
    TESTPROC_REAGENT_STATE_ERROR             // 
}TESTPROC_REAGENT_STATE_E;


typedef enum
{
    TESTPROC_RACK_STATE_IDLE        = 0,
    TESTPROC_RACK_STATE_TEST        = 1,  // 在测试区,等待测试项或正在获取测试项或正在加样
    TESTPROC_RACK_STATE_TOOUT       = 2,  // 正在已送至待抓取位置
    TESTPROC_RACK_STATE_ATOUT       = 3,  // 到达待抓取位置
    TESTPROC_RACK_STATE_TOWAIT      = 4,  // 常规样本架被送至待测区,被急诊打断
    TESTPROC_RACK_STATE_ATWAIT      = 5,  // 常规样本架被送至待测区,被急诊打断
    TESTPROC_RACK_STATE_TOTEST      = 6,  // 待测区移送至测试区中
    TESTPROC_RACK_STATE_GETITEM     = 7,  // 获取测试项
    TESTPROC_RACK_STATE_ERROR
}TESTPROC_RACK_STATE_E;

typedef enum
{
    TESTPROC_REACT_STATE_IDLE = 0,     // 0   空闲,反应盘未启动
    TESTPROC_REACT_STATE_23_RUN,       // 1   23转
    TESTPROC_REACT_STATE_23_STOP,      // 2   23停
    TESTPROC_REACT_STATE_02_RUN,       // 3   2转
    TESTPROC_REACT_STATE_02_STOP,      // 4   2停
    TESTPROC_REACT_STATE_09_RUN,       // 5   9转
    TESTPROC_REACT_STATE_09_STOP,      // 6   9停
    TESTPROC_REACT_STATE_31_RUN,       // 7   31转
    TESTPROC_REACT_STATE_31_STOP,      // 8   31停
    TESTPROC_REACT_STATE_ERROR         // 9   轨道取架失败
}TESTPROC_REACT_STATE_E;

typedef enum
{
    TESTPROC_REACT_CUP_INIT = 0,       // 0  初始状态
    TESTPROC_REACT_CUP_IN_ING,         // 1  反应杯正在移动中
    TESTPROC_REACT_CUP_IN,             // 2  已放入杯子
    TESTPROC_REACT_CUP_R1,             // 3  已第1次加试剂
    TESTPROC_REACT_CUP_SAMPLE,         // 4  已加样本
    TESTPROC_REACT_CUP_RM1,            // 5  已第1次混匀
    TESTPROC_REACT_CUP_R2,             // 6  已第2次架试剂
    TESTPROC_REACT_CUP_RM2,            // 7  已第2次混匀
    TESTPROC_REACT_CUP_2STEP_CUP_IN,   // 8  2步法反应杯进入反应盘
    TESTPROC_REACT_CUP_2STEP_R1,       // 9  2步法反应杯+R1
    TESTPROC_REACT_CUP_2STEP_SAMPLE,   // 10 2步法反应杯+样本
    TESTPROC_REACT_CUP_2STEP_RM1,      // 11 2步法已第1次混匀
    TESTPROC_REACT_CUP_2STEP_R2,       // 12 2步法已第2次架试剂
    TESTPROC_REACT_CUP_2STEP_RM2,      // 13 2步法已第2次混匀
    TESTPROC_REACT_CUP_ERROR,          // 14
}TESTPROC_REACT_CUP_STATE_E;

//测量室,Measure Room
typedef enum
{
    TESTPROC_MEASUREROOM_STATE_IDLE = 0,    // 0  空闲,测量室没有反应杯
    TESTPROC_MEASUREROOM_STATE_FULLOPENING, // 1  全开门中
    TESTPROC_MEASUREROOM_STATE_FULLOPEN,    // 2  全开
    TESTPROC_MEASUREROOM_STATE_HALFOPEN_ING,// 3  半开门中
    TESTPROC_MEASUREROOM_STATE_HALFOPEN,    // 4  半开
    TESTPROC_MEASUREROOM_STATE_CUPIN_ING,   // 5  放反应杯中
    TESTPROC_MEASUREROOM_STATE_CUPIN,       // 6  反应杯已放入,可以关门
    TESTPROC_MEASUREROOM_STATE_CLOSE_ING,   // 7  关闭中，准备测值
    TESTPROC_MEASUREROOM_STATE_CLOSE,       // 8  关闭
    TESTPROC_MEASUREROOM_STATE_READY,       // 9  准备完成,可以测量
    TESTPROC_MEASUREROOM_STATE_ING,         // 10 测量中
    TESTPROC_MEASUREROOM_STATE_OVER,        // 11 测量完成,可以抛杯
    TESTPROC_MEASUREROOM_STATE_WAITCATCH,   // 12 等待抛杯
    TESTPROC_MEASUREROOM_STATE_ABANDON_ING  // 13 测量室抛杯中
}TESTPROC_MEASUREROOM_STATE_E;

typedef enum
{
    TESTPROC_HANDCUP_STATE_IDLE = 0,        // 0 空闲
    TESTPROC_HANDCUP_STATE_BUSY,            // 1 忙,机械手在运动会中
    TESTPROC_HANDCUP_STATE_CATCH_NEWCUPING, // 2
    TESTPROC_HANDCUP_STATE_PUTREACT,        // 3 在反应盘上方并且含住一个反应杯
    TESTPROC_HANDCUP_STATE_PUTNEWCUPING,    // 4
    TESTPROC_HANDCUP_STATE_TO_REACTFORWASH, // 1 忙,机械手在运动会中    
    TESTPROC_HANDCUP_STATE_ON_REACTGORWASH, // 5 在反应盘上方等待抓取一个反应杯
    TESTPROC_HANDCUP_STATE_ONWASH,          // 6 
    TESTPROC_HANDCUP_STATE_TO_MEASUREROOM,  // 7
    TESTPROC_HANDCUP_STATE_ON_MEASUREROOM,  // 8
    TESTPROC_HANDCUP_STATE_WASH_TO_REACT,   // 9  2步法,清洗反应杯到反应盘中
    TESTPROC_HANDCUP_STATE_WASH_ON_REACT,   // 10 2步法,清洗反应杯已到反应盘上方
    TESTPROC_HANDCUP_STATE_WASH_PUT_REACT,  // 11 2步法,清洗反应杯放到反应盘
    TESTPROC_HANDCUP_STATE_ERROR
}TESTPROC_HANDCUP_STATE_E;

typedef enum
{
    TESTPROC_RECYCLE_TSC_CMD_STATE_IDLE = 0,     // 空闲
    TESTPROC_RECYCLE_STATUS_BUSY,         // 忙
    TESTPROC_RECYCLE_STATUS_ERROR         // 轨道取架失败
}TESTPROC_RECYCLE_STATUS_E;

typedef enum
{
    TESTPROC_WASH_CUP_INIT = 0,           // 0  初始状态
    TESTPROC_WASH_CUP_IN,                 // 1  已放入杯子
    TESTPROC_WASH_CUP_WASH12,             // 2  清洗12
    TESTPROC_WASH_CUP_WASH34,             // 3  清洗34
    TESTPROC_WASH_CUP_WASH56,             // 4  清洗56
    TESTPROC_WASH_CUP_LIQUIDA,            // 5  注液A
    TESTPROC_WASH_CUP_MIX                 // 6  混匀
}TESTPROC_WASH_CUP_STATE_E;

typedef enum
{
    TESTPROC_REAGENTISK_STATE_IDLE = 0,         // 空闲,盘未启动
    TESTPROC_REAGENTISK_STATE_RUNNING,          // 运动中    
    TESTPROC_REAGENTISK_STATE_POSITION,         // 试剂盘就位   
    TESTPROC_REAGENTISK_STATE_SHAKE_ING,        // 试剂盘摇匀中   
    TESTPROC_REAGENTISK_STATE_ERROR             // 
}TESTPROC_REAGENTDISK_STATE_E;

typedef enum
{
    TESTPROC_SAMPLE_STATE_IDLE = 0,         // 空闲,盘未启动
    TESTPROC_SAMPLE_STATE_TO_RACK_ING,      // 内外清洗+去样本架位置
    TESTPROC_SAMPLE_STATE_ON_RACK,          // 到达样本架位置
    TESTPROC_SAMPLE_STATE_ABSORB_ING,       // 吸样本中
    TESTPROC_SAMPLE_STATE_ABSORB_OK,        // 吸样本完成，并抵达样本盘上方
    TESTPROC_SAMPLE_STATE_INJECT_ING,       // 吐样本中
    TESTPROC_SAMPLE_STATE_INJECT_OK,        // 吐样本完成    
    TESTPROC_SAMPLE_STATE_WASH_ING,         // 清洗中
    TESTPROC_SAMPLE_STATE_ERROR             // 
}TESTPROC_SAMPLE_STATE_E;

typedef enum
{
    TESTPROC_SAMPLE_CB_STATE_IDLE = 0,      // 空闲,盘未启动
    TESTPROC_SAMPLE_CB_STATE_MOVING,        // 内外清洗+去样本架位置
    TESTPROC_SAMPLE_CB_STATE_READY,
    TESTPROC_SAMPLE_CB_STATE_RESET,         // 位置复位,移动到测试位置1
    TESTPROC_SAMPLE_CB_STATE_ERROR          // 
}TESTPROC_SAMPLE_CB_STATE_E;

typedef enum
{
    TESTPROC_TESTRACK_STATE_IDLE = 0,         // 空闲,样本架未运动    
    TESTPROC_TESTRACK_STATE_IN,               // 样本架在测试位置
    TESTPROC_TESTRACK_STATE_RUNNING,          // 样本架运动中
    TESTPROC_TESTRACK_STATE_POSITION,         // 样本架就位
    TESTPROC_TESTRACK_STATE_TO_WAIT,          // 样本架正在进入待测区
    TESTPROC_TESTRACK_STATE_IN_WAIT,          // 样本架已进入待测区
    TESTPROC_TESTRACK_STATE_ERROR             // 
}TESTPROC_TESTRACK_STATE_E;

typedef enum
{
    TESTPROC_WASHDISK_STATE_IDLE = 0,         // 空闲,盘未启动
    TESTPROC_WASHDISK_STATE_RUNNING,          // 运动中    
    TESTPROC_WASHDISK_STATE_ERROR             // 
}TESTPROC_WASHDISK_STATE_E;

typedef enum
{
    TESTPROC_CUPSTACK_STATE_INIT = 0,         // 上电初始、复位后
    TESTPROC_CUPSTACK_STATE_CHANGE,           // 正在换新杯栈
    TESTPROC_CUPSTACK_STATE_READY,            // 准备就绪
    TESTPROC_CUPSTACK_STATE_ERROR             // 
}TESTPROC_CUPSTACK_STATE_E;



typedef enum
{
    TESTPROC_RESET_INIT,                        // 0 - 上电后的初始状态
    TESTPROC_RESET_OK,                          // 1 - 复位成功
    TESTPROC_RESET_ONGOING,                     // 2 - 正在复位
    TESTPROC_RESET_TIMEOUT,                     // 3 - 复位超时
    TESTPROC_RESET_NG,                          // 4 - 复位失败
    TESTPROC_RESET_ERROR                        // 5 - 复位的错误状态,不应该出现
}TESTPROC_RESET_STATE;

typedef enum
{
    TESTPROC_WORK_TYPE_RESET                = 0,
    TESTPROC_WORK_TYPE_TEST                 = 1,
    TESTPROC_WORK_TYPE_SIMULATE             = 2,
    TESTPROC_WORK_TYPE_LIGHT_CHECK          = 3,
    TESTPROC_WORK_TYPE_BLANK                = 4,
    TESTPROC_WORK_TYPE_CUP_WASH             = 5,
    TESTPROC_WORK_TYPE_ABANDON_CUP          = 7,
    TESTPROC_WORK_TYPE_RECYCLE_RACK         = 8,
    TESTPROC_WORK_TYPE_REAGENTBARCODE       = 9,
    TESTPROC_WORK_TYPE_REAGENTQUANTITY      = 10,
    TESTPROC_WORK_TYPE_REAGENT_V            = 11,
    TESTPROC_WORK_TYPE_WASTE_EXHAUST        = 12,
    TESTPROC_WORK_TYPE_WASH_EXHAUST         = 13,
    TESTPROC_WORK_TYPE_CLEANCUP             = 14,
    TESTPROC_WORK_TYPE_WASH_WARMSINK        = 15,
    TESTPROC_WORK_TYPE_WASH_SAMPLE          = 16,
    TESTPROC_WORK_TYPE_M_SEPARATE           = 17,
    TESTPROC_WORK_TYPE_M_SEPARATE_AUTO      = 18,
    TESTPROC_WORK_TYPE_SAMPLE_ACCURACY      = 19,
    TESTPROC_WORK_TYPE_REAGENT_ACCURACY     = 20,
    TESTPROC_WORK_TYPE_WASH_ACCURACY        = 21,
    TESTPROC_WORK_TYPE_DARK_TEST            = 22,
    TESTPROC_WORK_TYPE_NEEDLE_CLEAN_OUT     = 23,
    TESTPROC_WORK_TYPE_NEEDLE_CLEAN_IN      = 24,
    TESTPROC_WORK_TYPE_LIQUID_PERFUSE       = 25,
    TESTPROC_WORK_TYPE_IDLE                 = 254,
    TESTPROC_WORK_TYPE_ERROR
}TESTPROC_WORK_TYPE;

#pragma pack (push)
#pragma pack (1)

typedef struct
{
    uint8_t  u8WorkStatus;                      // uint8_t  工作状态
    uint8_t  u8Reserve;
    uint8_t  u8ReactTemp[2];                    // uint8_t  反应槽温度
    uint8_t  u8CoolTemp[2];                     // uint8_t  制冷温度
    uint8_t  u8WashTemp[2];                     // uint8_t  清洗盘温度
}TESTPROC_CELLSTATUS;

typedef struct
{
    TESTPROC_ITEM_T *pstHoldItem;               // 待加入样本盘的测试项
    TESTPROC_ITEM_T *pstMeasureRoomItem;        // 待加入样本盘的测试项
    TESTPROC_ITEM_T *pstReactCupItem[TESTPROC_CUP_QUANTITY+1]; // 65
    TESTPROC_ITEM_T *pstWashCupItem[TESTPROC_WASHDISK_POS_QUANTITY+1]; // 11
    TESTPROC_CELLSTATUS stCellStatus;
    uint32_t u32Trigger1Count;
    uint32_t u32Trigger2Count;
    uint32_t u32Trigger3Count;
    uint32_t u32Trigger4Count;
    uint32_t u32RackID;                         // 正在测试的样本架号
    uint32_t u32EmRackID;                       // 正在测试的急诊样本架号
    //uint32_t u32WaitRackID;                     // 等待测试的样本架号
    uint32_t u32RecycleRackID;                  // 回收的样本架号
    uint32_t u32SimulateCount;                  // uint32_t 仿真测试次数
    uint32_t u32MeasureValue[2];                // 2磁测量的结果
    uint32_t u32RackOutCount;                    // 机械动作检查时样本架移出计数
    uint32_t u32EmRackOutCount;                    // 机械动作检查时样本架移出计数
    int32_t  s32StandbyCount; 
    TESTPROC_REACT_CUP_STATE_E eReactCupState[TESTPROC_CUP_QUANTITY+4];
    TESTPROC_WASH_CUP_STATE_E eWashCupState[TESTPROC_WASHDISK_POS_QUANTITY+2]; // 12
    TESTPROC_RESET_STATE eResetState[TESTPROC_UNIT_MAX];
    TESTPROC_WORK_TYPE eWorkType;
    TESTPROC_RACK_STATE_E eRackState;             // 常规样本架状态
    TESTPROC_RACK_STATE_E eEmRackState;           // 急诊样本架状态
    TESTPROC_RECYCLE_STATUS_E eRecycleStatus;     // 0 - 空闲,1 - 正在回收, 2 - 回收失败
    TESTPROC_HANDCUP_STATE_E  eHandCupBusy;       // TRUE - 抓杯机械手忙; FALSE - 抓杯机械手空闲
    TESTPROC_HANDCUP_STATE_E  eHandCupState;
    TESTPROC_REACT_STATE_E eReactState;
    TESTPROC_MEASUREROOM_STATE_E eMeasureRoomState;
    TESTPROC_REAGENT_STATE_E eReagentState;
    TESTPROC_REAGENTDISK_STATE_E eReagentDiskState;
    TESTPROC_SAMPLE_STATE_E eSampleState;
    TESTPROC_SAMPLE_CB_STATE_E eConveyorBeltState;
    TESTPROC_TESTRACK_STATE_E eTestRackState;    // 测试位置样本架状态
    TESTPROC_WASHDISK_STATE_E eWashDiskState;
    TESTPROC_CUPSTACK_STATE_E eCupStackState;
    uint8_t  u8H2CupNumber;
    uint8_t  u8WashDiskHandCup;                 // 机械手操作位置
    uint8_t  u8GetItemCount;                    // 常规测试项 > 0表示中位机还有数据;=0表示中位机没有数据
    uint8_t  u8InterferenceReact;               // 反应盘干涉
    uint8_t  u8InterferenceReagentDisk;         // 试剂盘干涉
    uint8_t  u8InterferenceConveyorBelt;        // 传动带干涉
    uint8_t  u8InterferenceSample;              // 样本针干涉
    uint8_t  u8InterferenceWash;                // 清洗盘干涉
    uint8_t  u8GarbageNO;                       // 0,1
    uint8_t  u8CupStackRow;                     // 行 Max=14
    uint8_t  u8CupStackCol;                     // 列 Max=10
    uint8_t  u8WorkBeforeTest;                  // 测试前的准备工作         bit0 == 1表示抛杯, bit1 == 1表示样本架回收
    uint8_t  u8CupStackDoorClose;               // 1 - 门关闭状态；0 - 门开启或等待状态
    uint8_t  u8SamplePauseCount;                // 加样暂停倒计时
    uint8_t  u8LastReagentDiskPos;              // 试剂盘最后停靠位置
}TESTPROC_STATUS_T;

#define TESTPROC_REALOADTYPE_NONE               0
#define TESTPROC_REALOADTYPE_R1R2RM             1
#define TESTPROC_REALOADTYPE_R1R2               2
#define TESTPROC_REALOADTYPE_R3RM               3
#define TESTPROC_REALOADTYPE_DILUTE             4
typedef struct __REAGENT_POS
{
    uint16_t u16ReagentID;
    uint8_t  u8Exist[3];        // 0无，1有
    uint8_t  u8ReaLoadType;     // 试剂瓶类型;0无,1->R1R2Rm;2->R1R2;3->R3Rm;4->稀释液R1R2
    uint8_t  u8Priority;        // 0 未使用; 1最高
    uint8_t  u8Reserved;
}REAGENT_POS_T;

typedef struct
{
    struct __REAGENT_POS stR1PosInfo[TESTPROC_REAGENT_POS_MAX];
    struct __REAGENT_POS stR2PosInfo[TESTPROC_REAGENT_POS_MAX];
    uint8_t  u8ReagentLimit;                    // 最小试剂量
    uint8_t  u8Reserve1;
    uint8_t  u8Reserve2;
    uint8_t  u8Reserve3;
}TESTPROC_MULTY_REAGENT_T;

#pragma pack (pop)

#endif





