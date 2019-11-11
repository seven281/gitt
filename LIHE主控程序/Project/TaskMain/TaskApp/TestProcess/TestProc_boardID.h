/***********************************************************************
 *
 * (c) Copyright 2001-2016, XXXX, All Rights Reserved.
 * THIS IS UNPUBLISHED PROPRIETARY SOURCE CODE OF XXXX, INC.
 * The copyright notice above does not evidence any actual or intended
 * publication of such source code. 
 *
 *  Subsystem:   TestProc
 *  File:        TestProc_boardID.h
 *  Author:      王殿光
 *  Description: Command for can bus.
 *********************************************************************/

/* Define to prevent recursive inclusion -------------------------------------*/
#ifndef __TESTPROC_BOARDID_H__
#define __TESTPROC_BOARDID_H__

#ifdef __cplusplus
 extern "C" {
#endif

/* Exported constants --------------------------------------------------------*/
#define TESTPROC_CELL_1_NMBR                                (0)     // Cell 1
#define TESTPROC_CELL_2_NMBR                                (1)     // Cell 2
#define TESTPROC_CELL_3_NMBR                                (2)     // Cell 3
#define TESTPROC_CELL_4_NMBR                                (3)     // Cell 4
#define TESTPROC_CELL_ISE_NMBR                              (4)     // ise
#define TESTPROC_CELL_PLATFORM_NMBR                         (5)     // platform
   
#define TESTPROC_CELL_NMBR                                  (TESTPROC_CELL_1_NMBR)

#define BOARD_ID_CELL_MAIN                                  (0x01)    // 主控板
#define BOARD_ID_CELL_REACTE                                (0x02)    // 反应板
#define BOARD_ID_CELL_AD                                    (0x03)    // AD板
#define BOARD_ID_CELL_MIX1                                  (0x04)    // 搅拌1
#define BOARD_ID_CELL_MIX2                                  (0x04)    // 搅拌2
#define BOARD_ID_CELL_MIX3                                  (0x04)    // 搅拌3
#define BOARD_ID_CELL_MIX4                                  (0x04)    // 搅拌4
#define BOARD_ID_CELL_SAMPLE                                (0x05)    // 样本机构
#define BOARD_ID_CELL_R1                                    (0x06)    // 试剂1
#define BOARD_ID_CELL_R2                                    (0x07)    // 试剂2
#define BOARD_ID_CELL_COOLER1                               (0x08)    // 制冷1
#define BOARD_ID_CELL_AC                                    (0x09)    // 模块交流
#define BOARD_ID_CELL_WASHDISK                              (0x0B)    // 清洗机构
#define BOARD_ID_CELL_CUPDISK                               (0x0C)    // 反应杯盘
#define BOARD_ID_CELL_REAGENTDISK                           (0x0D)    // 试剂盘
#define BOARD_ID_CELL_HANDCUP                               (0x0E)    // 反应杯机械手

#define BOARD_ID_TRACK_COMM                                 (0x20)    // 中位机
#define BOARD_ID_TRACK_SFB_TRANS                            (0x21)    // 进架仓
#define BOARD_ID_TRACK_SFB_RECYCLE                          (0x22)    // 出架仓
#define BOARD_ID_TRACK_1                                    (0x23)    // 轨道1
#define BOARD_ID_TRACK_2                                    (0x24)    // 轨道2
#define BOARD_ID_TRACK_AC                                   (0x25)    // 主交流

#define BOARD_ID_CELL_MAX                                   (18)    // 

#define BOARD_ID_BOARDCAST                                  (0x3F)    // 公共板号


#ifdef __cplusplus
}
#endif

#endif /* __TESTPROC_BOARDID_H__ */

