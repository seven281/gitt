/***********************************************************************
 *
 * (c) Copyright 2001-2016, XXXX, All Rights Reserved.
 * THIS IS UNPUBLISHED PROPRIETARY SOURCE CODE OF XXXX, INC.
 * The copyright notice above does not evidence any actual or intended
 * publication of such source code. 
 *
 *  Subsystem:   TestProc
 *  File:        TestProc_testData.h
 *  Author:      王殿光
 *  Description: Double List for C header files.
 *********************************************************************/

/* Define to prevent recursive inclusion -------------------------------------*/
#ifndef __TESTPROC_TESTDATA_H__
#define __TESTPROC_TESTDATA_H__

#ifdef __cplusplus
 extern "C" {
#endif

#include "TestProc_Typedef.h"
#include "TestProc_dlist.h"

// 测试类型标志 0-校准 1-质控 2-急诊 3-常规 
#define TESTPROC_ITEM_TYPE_CALIBRAT            (0)     // 校准生化项目
#define TESTPROC_ITEM_TYPE_QC                  (1)     // 质控生化项目
#define TESTPROC_ITEM_TYPE_EM                  (2)     // 急诊生化项目
#define TESTPROC_ITEM_TYPE_NORMAL              (3)     // 常规生化项目
#define TestProc_testIsNormalItem(type)        (TESTPROC_ITEM_TYPE_NORMAL == (type))

#define TESTPROC_ADDSTEP_TYPE1                 (1)      // S+R1+R2 -->M       -->清洗、测量 
#define TESTPROC_ADDSTEP_TYPE2                 (2)      // S+R1    -->R2+M    -->清洗、测量 
#define TESTPROC_ADDSTEP_TYPE3                 (3)      // S+R1    -->M       -->清洗         -->（2分钟）R2-->(18分钟)清洗、测量
#define TESTPROC_ADDSTEP_TYPE4                 (4)      // S+R1+R2 -->R3+M    -->清洗、测量 
#define TESTPROC_ADDSTEP_TYPE5                 (5)      // S+R1    -->R2+R3+M -->清洗、测量
#define TestProc_testIsAddReagent1R2(type)     (TESTPROC_ADDSTEP_TYPE1 == (type) || TESTPROC_ADDSTEP_TYPE3 == (type) || TESTPROC_ADDSTEP_TYPE4 == (type))
#define TestProc_testIsAddReagent2R2(type)     (TESTPROC_ADDSTEP_TYPE2 == (type) || TESTPROC_ADDSTEP_TYPE5 == (type))
#define TestProc_testIsAddReagent2R3(type)     (TESTPROC_ADDSTEP_TYPE4 == (type) || TESTPROC_ADDSTEP_TYPE5 == (type))
#define TestProc_testIsTwoStepItem(type)       (TESTPROC_ADDSTEP_TYPE3 == (type))

// 两步法测试流程状态
typedef enum
{
    TESTPROC_TWO_STEP_NONE = 0,        // 0  一步法测试项目
    TESTPROC_TWO_STEP_ENABLE,          // 1  两步法测试使能
    TESTPROC_TWO_STEP_DISABLE,         // 2  两步法测试已延周期
}TESTPROC_TWO_STEP_STATE_E;

#pragma pack (push)
#pragma pack (1)    /*指定按1字节对齐*/
typedef struct
{
    uint32_t u32TestID;                     // 测试号
    uint32_t u32RackID;                     // 架子号
    uint16_t u16SampleQuantity;             // 样本量（下位机接收后除以10）
    uint16_t u16SampleDiluteQuantity;       // 稀释用样本量（下位机接收后除以10）
    uint16_t u16DiluteQuantity;             // 稀释液量（下位机接收后除以10）
    uint16_t u16ReagentItemID;              // 项目编号(样本针交叉污染使用)
    uint16_t u16R1ID;                       // R1试剂编号
    uint16_t u16R2ID;                       // R2试剂编号
    uint16_t u16R3ID;                       // R3试剂编号
    uint16_t u16MID;                        // R4试剂编号
    uint16_t u16R1Quantity;                 // R1试剂量
    uint16_t u16R2Quantity;                 // R2试剂量
    uint16_t u16R3Quantity;                 // R3试剂量
    uint16_t u16MQuantity;                  // R4试剂量，磁珠 
    uint16_t u16DiluentID;                  // 稀释液试剂编号
    uint16_t u16CleanID;                    // 清洗液试剂编号
    uint8_t  u8SamplePos;                   // 样本杯位置
    uint8_t  u8SampleType;                  // 样本杯类型
    uint8_t  u8TestType;                    // 测试类型标志 0-校准 1-质控 2-急诊 3-常规 
    uint8_t  u8TestCell;                    // 需要指定到具体模块测试,质控校准需要指定模块
    uint8_t  u8SampleBlank;                 // 样本空白
    uint8_t  u8ForeceWash;                  // 免疫强洗标志1表示强洗，0表示不强洗
    uint8_t  u8ItemType;                    // 数据类型 0 - 生化类型，1 - 免疫类型
    uint8_t  u8AddStep;                     // 免疫测试类型 
}__ITEM_T;

typedef struct
{
    __ITEM_T stItem;
    uint8_t  u8ReagentPos;
    TESTPROC_TWO_STEP_STATE_E  eTwoStepState;   // 两步法测试流程状态
    uint8_t  u8Invalid2;
    uint8_t  u8Invalid3;
}TESTPROC_ITEM_T;

typedef struct
{
    TESTPROC_ITEM_T stItem;
    uint8_t         u8State;
    uint8_t         u8CellNbr;
    uint8_t         u8Reserve2;
    uint8_t         u8Reserve3;
}TESTPROC_ITEM_STATE_T;

typedef struct
{
    uint32_t u32RackID;
    uint8_t  u8SampleCup[10];        // 指示10个样本位置是否有样本杯 0 - 无, 1 - 有
    uint8_t  u8SampleCupType[10];    // 指示10个样本位置类型, 0 表示标准杯，1表示中号杯，2表示大号杯
    uint8_t  u8SampleCode[10][32];   // 是个样本杯的条码
}TESTPROC_RACK_T;

typedef struct
{
    TESTPROC_RACK_T stRack;
    //TESTPROC_DLIST_T     stItemList;
    uint8_t  u8RackState;
    uint8_t  u8ReCheckPos;
    uint8_t  u8Reserve2;
    uint8_t  u8Reserve3;
}TESTPROC_RACK_STATE_T;

#pragma pack (pop)    /*取消指定对齐，恢复缺省对齐*/

extern int32_t TestProc_testDataInit(void);
extern int32_t TestProc_testDataPutBack(void *pOneTestData, uint8_t u8Head, uint8_t u8EmData);
extern void *TestProc_testGetEmptyItem(void);
extern int32_t TestProc_testDataPush(void const *pOneTestData, uint16_t u16Quantity);
extern void *TestProc_ItemPop(void);
extern void *TestProc_EmItemPop(void);
extern int32_t TestProc_testDataFreeItemByID(uint32_t u32TestID);
extern int32_t TestProc_testDataItemFree(void *pItem);
extern void TestProc_testDataClean(void);
extern uint32_t TestProc_testDataCount(void);
extern uint32_t TestProc_testDataEmItemCount(void);
extern void TestProc_testDataShowItem(void);

#ifdef __cplusplus
}
#endif

#endif /* __TESTPROC_TESTDATA_H__ */

/************************ (C) COPYRIGHT STMicroelectronics *****END OF FILE****/
