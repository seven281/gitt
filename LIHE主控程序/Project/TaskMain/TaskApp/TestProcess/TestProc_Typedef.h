#ifndef __TESTPROC_TYPEDEF_H_
#define __TESTPROC_TYPEDEF_H_

#ifdef __cplusplus
 extern "C" {
#endif

#include "TaskAppConfig.h"
#include "TestProc_Result.h"
#include "TestProc_API.h"

#define MAINBOARD_VERSION_MAIN                  1
#define MAINBOARD_VERSION_SLAVE                 99
#define MAINBOARD_VERSION_DEBUG                 52

#define TestProc_AllocMemory(size)              UserMemMalloc(SRAM_IS62, (size))
#define TestProc_FreeMemory(p)                  UserMemFree(SRAM_IS62, (p))
#define DebugMsg                                MCU_Uart1Printf

#define TESTPROC_TICK_INVALID                   (0xFFFFFFFF)
#define OK                                      (1u)
#define NG                                      (0u)
#define TRUE                                    (1)
#define FALSE                                   (0)

#define TESTPROC_RACK_TYPE_NORMAL               (1)
#define TESTPROC_RACK_TYPE_EMERGENT             (2)
#define TESTPROC_RACK_TYPE_CALIBRAT             (3)
#define TESTPROC_RACK_TYPE_QC                   (4)
#define TESTPROC_RACK_TYPE_RECHECK              (5)
#define TestProc_GetType(id)                    ((uint8_t)((id)/10000))

// 测试类型
#define TESTPROC_TEST_TYPE_1                    (1u)    // S + R1 + R2 --9Min--> M --9Min--> 清洗、测量
#define TESTPROC_TEST_TYPE_2                    (2u)    // S + R1 --9Min--> R2 +M --9Min--> 清洗、测量
#define TESTPROC_TEST_TYPE_3                    (3u)    // S + R1 --9Min--> +M --9Min--> 清洗  +R2 --18Min-->清洗、测量
#define TESTPROC_TEST_TYPE_4                    (4u)    // S + R1 + R2 --9Min--> R3+M --9Min--> 清洗、测量
#define TESTPROC_TEST_TYPE_5                    (5u)    // S + R1 --9Min--> R2+R3+M --9Min--> 清洗、测量

// Cell Status
#define TESTPROC_WORK_IDLE                      0u          // 空闲,上电后初始化的状态,等待下位机更新各模块状态
#define TESTPROC_WORK_STANDBY                   1u          // 待机
#define TESTPROC_WORK_RESET                     2u          // 复位
#define TESTPROC_WORK_TEST                      3u          // 测试
#define TESTPROC_WORK_EMSTOP                    4u          // 急停
#define TESTPROC_WORK_SAMPLE_PAUSE              5u          // 加样暂停
#define TESTPROC_WORK_SAMPLE_STOP               6u          // 加样停止
#define TESTPROC_WORK_RACK_PAUSE                7u          // 进架暂停
#define TESTPROC_WORK_RACK_STOP                 8u          // 进架停止
#define TESTPROC_WORK_RESET_NG                  9u          // 复位失败
#define TESTPROC_WORK_SIMULATE                  10u         // 机械动作检查
#define TESTPROC_WORK_CUP_BLANK                 11u         // 杯空白
#define TESTPROC_WORK_CHECK_LIGHT               12u         // 光通量检查
#define TESTPROC_WORK_LIGHT_STABILIZE           13u         // 光稳定性检查
#define TESTPROC_WORK_SCAN_REAGENT_BAR          14u         // 试剂条码扫描
#define TESTPROC_WORK_SCAN_SAMPLE_BAR           15u         // 样本条码扫描
#define TESTPROC_WORK_WASH_CUP                  16u         // 清洗反应杯
#define TESTPROC_WORK_WASH_WARMSINK             17u         // 清洗恒温槽
#define TESTPROC_WORK_WASH_SAMPLE               18u         // 清洗样本针
#define TESTPROC_WORK_WASH_WATERTANK            19u         // 清洗水箱
#define TESTPROC_WORK_INJECT_EXHAUST            20u         // 注射泵排气
#define TESTPROC_WORK_SAMPLE_V_CHECK            21u         // 样本针垂直检查
#define TESTPROC_WORK_SAMPLE_H_CHECK            22u         // 样本针水平检查
#define TESTPROC_WORK_REAGENT_V_CHECK           23u         // 试剂针垂直检查
#define TESTPROC_WORK_REAGENT_H_CHECK           24u         // 试剂针水平检查
#define TESTPROC_WORK_MIX_V_CHECK               25u         // 搅拌垂直检查
#define TESTPROC_WORK_MIX_H_CHECK               26u         // 搅拌水平检查
#define TESTPROC_WORK_REAGENT_QUANTITY          27u         // 试剂余量扫描
#define TESTPROC_WORK_WASH_PIPE_EXHAUST         28u         // 清洗液管路排气
#define TESTPROC_WORK_WASTE_AUTO_EXHAUST        29u         // 浓废液管路自动排气
#define TESTPROC_WORK_WASTE_EXHAUST             30u         // 浓废液管路手动排气
#define TESTPROC_WORK_CHECK_SAMPLE              31u         // 样本机构检查
#define TESTPROC_WORK_CHECK_REAGENT             32u         // 试剂机构检查
#define TESTPROC_WORK_CHECK_MIX                 33u         // 搅拌机构检查
#define TESTPROC_WORK_AD_COLLECT                34u         // 通采
#define TESTPROC_WORK_ABANDON_CUP               35u         // 抛杯
#define TESTPROC_WORK_RECYCLE_RACK              36u         // 回收样本架
#define TESTPROC_WORK_PREHEATING                37u         // 恒温槽预温中
#define TESTPROC_WORK_PRIME                     38u         // 探针灌注
#define TESTPROC_WORK_CI_PREHEATING             39u         // 反应盘预温中
#define TESTPROC_WORK_WATERTANK_IN              40u         // 水箱上水
#define TESTPROC_WORK_WARMSINK_OUT              41u         // 恒温槽排水
#define TESTPROC_WORK_WATERTANK_PREHEATING      42u         // 水箱槽预温中
#define TESTPROC_WORK_WARMSINK_IN               43u         // 恒温槽上水
#define TESTPROC_WORK_M_SEPARATE_AUTO           44u         // 磁分离测试
#define TESTPROC_WORK_M_SEPARATE                45u         // 手动磁分离测试
#define TESTPROC_WORK_MANUAL_TEST               46u         // 人工测值
#define TESTPROC_WORK_DARK_TEST                 47u         // 暗计数
#define TESTPROC_WORK_NEEDLE_CLEAN_OUT          48u         // 清洁针外壁
#define TESTPROC_WORK_NEEDLE_CLEAN_IN           49u         // 清洁针内壁
#define TESTPROC_WORK_LIQUID_PERFUSE            50u         // 液路灌注
#define TESTPROC_WORK_SAMPLE_ACCURACY           51u         // 样本针精度测试
#define TESTPROC_WORK_REAGENT_ACCURACY          52u         // 试剂针精度测试
#define TESTPROC_WORK_WASH_ACCURACY             53u         // 清洗针精度测试
#define TESTPROC_WORK_TEST_TO_STANDBY           54u         // 测试完成,准备待机
#define TESTPROC_WORK_PERFORMANCE_TEST          55u         // 性能测试
#define TESTPROC_WORK_WAIT_SAMPLE_PAUSE         56u         // 加样暂停,等待加试剂完成
#define TESTPROC_WORK_ERROR                     255

	// LittleEnd Format
#define BYTE_TO_HWORD(h, l)                             ((uint16_t)((l)|((h)<<8)))
#define BYTE_TO_WORD(hh, hl, lh, ll)                    ((uint32_t)((ll)|((lh)<<8)|((hl)<<16)|((hh)<<24)))
#define HWORD_TO_BYTE_L(a)                              ((uint8_t)((a)&0xFF))
#define HWORD_TO_BYTE_H(a)                              ((uint8_t)(((a)>>8)&0xFF))
#define WORD_TO_BYTE_LL(a)                              ((uint8_t)((a)&0xFF))
#define WORD_TO_BYTE_LH(a)                              ((uint8_t)(((a)>>8)&0xFF))
#define WORD_TO_BYTE_HL(a)                              ((uint8_t)(((a)>>16)&0xFF))
#define WORD_TO_BYTE_HH(a)                              ((uint8_t)(((a)>>24)&0xFF))

#define TESTPROC_CELL_TYPE_CS                           (0x01)
#define TESTPROC_CELL_TYPE_CI                           (0x02)
#define TESTPROC_CELL_TYPE_ERR                          (0xFF)

// Cup Calculation
#define TESTPROC_CUP_QUANTITY                           (64u)
#define TESTPROC_H2_RESET_CUPID                         (31u)        // 31 - 23
#define TESTPROC_CUP_STEP_1                             (23u)
#define TESTPROC_WASHDISK_POS_QUANTITY                  (10u)
#define TESTPROC_WASHDISK_RESET_CUPID                   (1u)
#define TESTPROC_REAGENTDISK_POS_MAX                    (30u)        // 1 ~ 30
#define TESTPROC_RACK_POS_MIN                           (1u)
#define TESTPROC_RACK_POS_MAX                           (5u)

#define TESTPROC_REAGENT_POS_MAX                        (40u)
#define TESTPROC_REAGENT_POS_CONFIG                     (30u)
#define TESTPROC_REAGENT_TYPE_QUANTITY                  (4u)     // R1,R2,R3,R4
#define TESTPROC_REAGENT_DISK_MAX                       (2u)
#define TESTPROC_REAGENT_CODE_SIZE                      (512u)

typedef enum
{
    TESTPROC_UNIT_REACT = 0,                    // 反应盘
    TESTPROC_UNIT_WASH,                         // 清洗盘
    TESTPROC_UNIT_REAGENTDISK,                  // 试剂盘
    TESTPROC_UNIT_REAGENT,                      // 试剂针
    TESTPROC_UNIT_SAMPLE,                       // 样本针
    TESTPROC_UNIT_HANDCUP,                      // 抓杯机械手
    TESTPROC_UNIT_CUPSTACK,                     // 反应杯盘    
    TESTPROC_UNIT_EVENT,
    TESTPROC_UNIT_MAX,
    // 
    TESTPROC_UNIT_CAN1 = 50,
    TESTPROC_UNIT_UART1,
    TESTPROC_UNIT_PROCTEST,
    TESTPROC_UNIT_ERROR
}TESTPROC_UNIT;

typedef enum
{
    TESTPROC_MSG_CMD_RESET = 0x1001,                        // 0x1001 复位结果    
    TESTPROC_MSG_CMD_TIMEOUT,                               // 0x1002 机构运行超时
    TESTPROC_MSG_CMD_EXEC_NG,                               // 0x1003 机构执行API失败
    // 
    TESTPROC_MSG_CMD_SAMPLE_RESET = 0x2001,                 // 0x2001 样本机构复位结果
    TESTPROC_MSG_CMD_SAMPLENEEDLE_RESET,                    // 0x2002 样本针复位结果
    // React
    TESTPROC_MSG_CMD_REACT_TRIGGER = 0x3001,                // 0x3001 反应盘开始转, Byte0字节参数，参数为转过的杯子数;byte1~4为触发次数(uint32_t)
    TESTPROC_MSG_CMD_REACT_STOP,                            // 0x3002 反应盘触发, Byte0字节参数，参数为转过的杯子数;byte1~4为触发次数(uint32_t)
    TESTPROC_MSG_CMD_REACT_MIX_OK,                          // 0x3003 混匀OK
    // HandCup
    TESTPROC_MSG_CMD_HANDCUP_NEWCUP = 0x4001,               // 0x4001 抓杯机械手抓到新反应杯,并移动到反应盘
    TESTPROC_MSG_CMD_HANDCUP_CUPINREACT,                    // 0x4002 抓杯机械手将反应杯放入反应盘
    TESTPROC_MSG_CMD_HANDCUP_ABANDON,                       // 0x4003 反应杯已丢进垃圾桶
    TESTPROC_MSG_CMD_HANDCUP_ONREACT,                       // 0x4004 机械手已经到达反应盘上方，准备抓取反应杯
    TESTPROC_MSG_CMD_HANDCUP_REACTCATCHOK,                  // 0x4005 反应杯已从反应盘抓出
    TESTPROC_MSG_CMD_HANDCUP_ONWASH,                        // 0x4006 反应杯已经在清洗盘上方
    TESTPROC_MSG_CMD_HANDCUP_INWASH,                        // 0x4007 反应杯已放入清洗盘
    TESTPROC_MSG_CMD_HANDCUP_ONMEASUREROOM,                 // 0x4008 反应杯已在测量室上方    
    TESTPROC_MSG_CMD_HANDCUP_INMEASUREROOM,                 // 0x4008 反应杯已经进入测量室    
    TESTPROC_MSG_CMD_HANDCUP_MEASUREROOMCATCHOK,            // 0x4009 反应杯从测量室抓出
    TESTPROC_MSG_CMD_HANDCUP_WASHCUPONREACT,                // 0x400A 清洗盘反应杯已在反应盘上方
    TESTPROC_MSG_CMD_HANDCUP_WASHCUPINREACT,                // 0x400B 清洗盘反应杯已放入反应盘
    TESTPROC_MSG_CMD_HANDCUP_ON_CUPSTACK,                   // 0x400C 机械手到达清洗盘上方
    // Wash Disk
    TESTPROC_MSG_CMD_WASH_DISK_STOP = 0x5001,               // 0x5001 清洗盘转一个位置
    TESTPROC_MSG_CMD_WASH_NEEDLE_FIRST_OK,                  // 0x5101 清洗针第一次清洗完成
    TESTPROC_MSG_CMD_WASH_NEEDLE_SENOND_OK,                 // 0x5102 清洗针第二次清洗完成
    TESTPROC_MSG_CMD_WASH_MEASUREROOM_OPEN = 0x5301,        // 0x5301 测量室门全开
    TESTPROC_MSG_CMD_WASH_MEASUREROOM_HALFOPEN,             // 0x5302 测量室门半开
    TESTPROC_MSG_CMD_WASH_MEASUREROOM_FULLOPEN,             // 0x5303 测量室门全开
    TESTPROC_MSG_CMD_WASH_MEASUREROOM_CLOSE,                // 0x5304 测量室关门
    TESTPROC_MSG_CMD_WASH_MEASUREROOM_VALUE1,               // 0x5305 本体值
    TESTPROC_MSG_CMD_WASH_MEASUREROOM_VALUE2,               // 0x5306 注液值
    // Reagent
    TESTPROC_MSG_CMD_RS1_R1_PREPARE_OK = 0x6001,            // 0x6001 试剂针清洗完成并移动到试剂盘上方，准备吸R1
    TESTPROC_MSG_CMD_RS1_R1_ABSORB_OK,                      // 0x6002 R1试剂针吸试剂完成
    TESTPROC_MSG_CMD_RS1_R2_ABSORB_OK,                      // 0x6003 R2试剂针吸试剂完成
    TESTPROC_MSG_CMD_RS1_R2_PREPARE_2STEP_OK,               // 0x6004 试剂针清洗完成并移动到试剂盘上方，准备吸R2
    TESTPROC_MSG_CMD_RS1_R2_ABSORB_2STEP_OK,                // 0x6005 R2试剂针吸试剂完成
    TESTPROC_MSG_CMD_RS1_INJECT_OVER,						// 0x6006 第一步吐试剂完成
    TESTPROC_MSG_CMD_RS1_INJECT_OK,                         // 0x6007 吐试剂完成
    TESTPROC_MSG_CMD_RS2_R2_PREPARE_OK,                     // 0x6008 Reagent Step2 试剂针清洗完成并移动到试剂盘上方，准备吸R2
    TESTPROC_MSG_CMD_RS2_R2_ABSORB_OK,                      // 0x6009 Reagent Step2 R2试剂针吸试剂完成
    TESTPROC_MSG_CMD_RS2_R3_PREPARE_OK,                     // 0x600A Reagent Step2 试剂针清洗完成并移动到试剂盘上方，准备吸R3
    TESTPROC_MSG_CMD_RS2_R3_ABSORB_OK,                      // 0x600B Reagent Step2 R3试剂针吸试剂完成
    TESTPROC_MSG_CMD_RS2_M_PREPARE_OK,                      // 0x600C Reagent Step2 试剂针清洗完成并移动到试剂盘上方，准备吸M
    TESTPROC_MSG_CMD_RS2_M_ABSORB_OK,                       // 0x600D Reagent Step2 M试剂针吸试剂完成,并移动到反应盘上方
    TESTPROC_MSG_CMD_RS2_INJECT_OVER,                       // 0x600E 第二步吐试剂完成，并升至最高点
    TESTPROC_MSG_CMD_RS2_INJECT_OK,                         // 0x600F 第二步吐试剂完成
    TESTPROC_MSG_CMD_REAGENT_ON_REACT,                      // 0x6010 试剂针在反应盘上方
    // ReagentDisk
    TESTPROC_MSG_CMD_REAGENTDISK_INPOS = 0x7001,            // 0x7001 试剂盘转动指定位置
    // Sample
    TESTPROC_MSG_CMD_SAMPLE_ONRACK = 0x8001,                // 0x8001 样本针到样本架上方
    TESTPROC_MSG_CMD_SAMPLE_ABSORB_OK,                      // 0x8002 样本针吸样完成
    TESTPROC_MSG_CMD_SAMPLE_INJECT_READY,                   // 0x8003 样本针吸完样本，已到反应盘上方
    TESTPROC_MSG_CMD_SAMPLE_INJECT_OK,                      // 0x8004 样本针吐完样本
    TESTPROC_MSG_CMD_SAMPLE_CLEAN_OK,                       // 0x8005 样本针清洗样本
    TESTPROC_MSG_CMD_SAMPLE_RACK_INPOS,                     // 0x8006 样本架就位
    // Cup Stack
    TESTPROC_MSG_CMD_CUPSTACK_POSTION = 0x9001,             // 0x9001 新杯栈在测试位置就绪
    TESTPROC_MSG_CMD_CUPSTACK_CHANGE_OK,                    // 0x9002 切换新杯栈完成
    // Event
    TESTPROC_MSG_CMD_EVENT_CUPSTACK_KEY_DOWN = 0xA001,
    TESTPROC_MSG_CMD_EVENT_CUPSTACK_KEY_UP,
    TESTPROC_MSG_CMD_EVENT_REAGENTDISK_KEY_DOWN,
    TESTPROC_MSG_CMD_EVENT_REAGENTDISK_KEY_UP,
    TESTPROC_MSG_CMD_EVENT_CUPSTACK_DOOR_CLOSE,
    TESTPROC_MSG_CMD_EVENT_CUPSTACK_DOOR_OPEN,
    TESTPROC_MSG_CMD_INVALID = 0xFFFF
}TESTPROC_MSG_CMD_E;

__INLINE static uint8_t __H2CupID(uint8_t u8H2CupID, uint8_t u8Step)
{
    int cupID = u8H2CupID + u8Step;
    if (cupID > TESTPROC_CUP_QUANTITY)
    {
        return (uint8_t)(cupID - TESTPROC_CUP_QUANTITY);
    }
    else
    {
        return (uint8_t)cupID;
    }
}

__INLINE static uint8_t __SampleCupID(uint8_t u8H2CupID)
{
    int cupID = u8H2CupID - 13;
    if (cupID > 0)
    {
        return (uint8_t)cupID;
    }
    else
    {
        return (uint8_t)(cupID+TESTPROC_CUP_QUANTITY);
    }
}

__INLINE static uint8_t __R123MCupID(uint8_t u8H2CupID)
{
    int cupID = u8H2CupID + 21;
    if (cupID > TESTPROC_CUP_QUANTITY)
    {
        return (uint8_t)(cupID - TESTPROC_CUP_QUANTITY);
    }
    else
    {
        return (uint8_t)cupID;
    }
}

__INLINE static uint8_t __RM1CupID(uint8_t u8H2CupID)
{
    int cupID = u8H2CupID + 20;
    if (cupID > TESTPROC_CUP_QUANTITY)
    {
        return (uint8_t)(cupID - TESTPROC_CUP_QUANTITY);
    }
    else
    {
        return (uint8_t)cupID;
    }
}

__INLINE static uint8_t __RM2CupID(uint8_t u8H2CupID)
{
    int cupID = u8H2CupID - 10;
    if (cupID > 0)
    {
        return (uint8_t)cupID;
    }
    else
    {
        return (uint8_t)(cupID+TESTPROC_CUP_QUANTITY);
    }
}

__INLINE static uint8_t __XCupID(uint8_t u8H2CupID)
{
    int cupID = u8H2CupID - 6;
    if (cupID > 0)
    {
        return (uint8_t)cupID;
    }
    else
    {
        return (uint8_t)(cupID+TESTPROC_CUP_QUANTITY);
    }
}

__INLINE static uint8_t __WashDiskHandCup(uint8_t u8HandCupID)
{
    int cupID = u8HandCupID + 1;
    if (cupID > TESTPROC_WASHDISK_POS_QUANTITY)
    {
        return (uint8_t)(cupID - TESTPROC_WASHDISK_POS_QUANTITY);
    }
    else
    {
        return cupID;
    }
}

__INLINE static uint8_t __Wash12Cup(uint8_t u8H2CupID)
{
    int cupID = u8H2CupID + 7;
    if (cupID > TESTPROC_WASHDISK_POS_QUANTITY)
    {
        return (uint8_t)(cupID - TESTPROC_WASHDISK_POS_QUANTITY);
    }
    else
    {
        return cupID;
    }
}

__INLINE static uint8_t __Wash34Cup(uint8_t u8H2CupID)
{
    int cupID = u8H2CupID + 5;
    if (cupID > TESTPROC_WASHDISK_POS_QUANTITY)
    {
        return (uint8_t)(cupID - TESTPROC_WASHDISK_POS_QUANTITY);
    }
    else
    {
        return cupID;
    }
}

__INLINE static uint8_t __Wash56Cup(uint8_t u8H2CupID)
{
    int cupID = u8H2CupID + 3;
    if (cupID > TESTPROC_WASHDISK_POS_QUANTITY)
    {
        return (uint8_t)(cupID - TESTPROC_WASHDISK_POS_QUANTITY);
    }
    else
    {
        return cupID;
    }
}

__INLINE static uint8_t __WashLiquidACup(uint8_t u8H2CupID)
{
    int cupID = u8H2CupID + 2;
    if (cupID > TESTPROC_WASHDISK_POS_QUANTITY)
    {
        return (uint8_t)(cupID - TESTPROC_WASHDISK_POS_QUANTITY);
    }
    else
    {
        return cupID;
    }
}

__INLINE static uint8_t __WashMixCup(uint8_t u8HandCupID)
{
    int cupID = u8HandCupID + 1;
    if (cupID > TESTPROC_WASHDISK_POS_QUANTITY)
    {
        return (uint8_t)(cupID - TESTPROC_WASHDISK_POS_QUANTITY);
    }
    else
    {
        return cupID;
    }
}

// 从节拍2的H1反应杯推算节拍3(R2)加试剂反应杯
__INLINE static uint8_t __Step4R1ByStep2CupID(uint8_t u8H2CupID)
{
    int cupID = u8H2CupID + 21+9+31;
    if (cupID > TESTPROC_CUP_QUANTITY)
    {
        return (uint8_t)(cupID - TESTPROC_CUP_QUANTITY);
    }
    else
    {
        return (uint8_t)cupID;
    }
}

// 从节拍1的H1反应杯推算节拍3加试剂反应杯
__INLINE static uint8_t __Step3R2ByStep1CupID(uint8_t u8H2CupID)
{
    int cupID = u8H2CupID + 21+9+2;
    if (cupID > TESTPROC_CUP_QUANTITY)
    {
        return (uint8_t)(cupID - TESTPROC_CUP_QUANTITY);
    }
    else
    {
        return (uint8_t)cupID;
    }
}

// 从节拍2的H1反应杯推算节拍3(R2)加试剂反应杯
__INLINE static uint8_t __Step3R2ByStep2CupID(uint8_t u8H2CupID)
{
    int cupID = u8H2CupID + 21+9;
    if (cupID > TESTPROC_CUP_QUANTITY)
    {
        return (uint8_t)(cupID - TESTPROC_CUP_QUANTITY);
    }
    else
    {
        return (uint8_t)cupID;
    }
}

// 从节拍4的H1反应杯推算节拍3(R2)加试剂反应杯
__INLINE static uint8_t __Step3R2ByStep4CupID(uint8_t u8H2CupID)
{
    int cupID = u8H2CupID + 21+23+2+9;
    if (cupID > TESTPROC_CUP_QUANTITY)
    {
        return (uint8_t)(cupID - TESTPROC_CUP_QUANTITY);
    }
    else
    {
        return (uint8_t)cupID;
    }
}

// 从节拍3的H1反应杯推算节拍4(R1)加试剂反应杯
__INLINE static uint8_t __Step4R2ByStep3CupID(uint8_t u8H2CupID)
{
    int cupID = u8H2CupID + 21+31;
    if (cupID > TESTPROC_CUP_QUANTITY)
    {
        return (uint8_t)(cupID - TESTPROC_CUP_QUANTITY);
    }
    else
    {
        return (uint8_t)cupID;
    }
}
/*
__INLINE static OS_ERR TestProc_PostMsg(void *pstMsg)
{
    OS_ERR err;
    OSTaskQPost((OS_TCB *)&tcbTaskAppTestProcess,
                (void *)pstMsg,
                (OS_MSG_SIZE)sizeof(TESTPROC_MSM_T),
                (OS_OPT)OS_OPT_POST_FIFO,
                (OS_ERR *)&err);
    return err;
}
*/
#pragma pack (push)
#pragma pack (1) 
typedef struct 
{
    uint32_t u32TestResult;  // 测试结果
    uint32_t u32TestNmbr;    // 测试号
    uint32_t u32Invalid;     // 无效
    uint8_t  u8CellNmbr;     // 模块号
    uint8_t  u8DataType;     // 数据类型 1-测试AD数据，2-测试杯空白，3-光量数据，4-维护杯空白、255-脏杯的光电数据、8-暗计数 9-底物 10-测量光子
    uint8_t  u8ReagentPos;   // 试剂的物理位置(1 ~ 40)
    uint8_t  u8TestType;     // 0 - 正常测试数据;1 - 调试测试数据
}TESTPROC_ITEM_RESULT_T;

typedef struct
{
    void    *pData;                             // 数据缓冲区
    uint16_t u16DataSize;                       // 数据长度
    TESTPROC_MSG_CMD_E eCommand;                // 命令
    uint8_t  u8Data[8];
    TESTPROC_UNIT  eSrcUnit;                    // 消息发送方
    TESTPROC_UNIT  eDestUnit;                   // 消息接收方
}TESTPROC_MSM_T;

#pragma pack (pop)

extern int32_t TestProc_SendMsg(TESTPROC_MSG_CMD_E eCmd, TESTPROC_UNIT eSrcUnit, uint8_t *pu8Param, uint8_t u8Size);


#ifdef __cplusplus
 }
#endif
#endif // End of “ifndef __TESTPROC_TYPEDEF_H_”

