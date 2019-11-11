/***********************************************************************
 *
 * (c) Copyright 2001-2016, XXXX, All Rights Reserved.
 * THIS IS UNPUBLISHED PROPRIETARY SOURCE CODE OF XXXX, INC.
 * The copyright notice above does not evidence any actual or intended
 * publication of such source code. 
 *
 *  Subsystem:   TestProc
 *  File:        TestProc_dlist.h
 *  Author:      王殿光
 *  Description: Double List for C header files.
 *********************************************************************/

/* Define to prevent recursive inclusion -------------------------------------*/
#ifndef __TESTPROC_DLIST_H__
#define __TESTPROC_DLIST_H__

#ifdef __cplusplus
 extern "C" {
#endif

#include "TestProc_Typedef.h"

typedef struct _DNode
{
    void * pData;                     // 节点存储的数据地址
    struct _DNode *prev;              // 指向上一个节点
    struct _DNode *next;              // 指向下一个节点
}TESTPROC_DNODE_T;

typedef struct _DList
{
    uint32_t u32Size;                  // 链表的大小
    TESTPROC_DNODE_T *pstHead;              // 存储链表的第一个节点
    TESTPROC_DNODE_T *pstTail;              // 存储链表的最后一个节点
}TESTPROC_DLIST_T;

extern int32_t TestProc_dlistModuleInit(void);
extern int32_t TestProc_dlistInit(TESTPROC_DLIST_T *pstList);
extern int32_t TestProc_dlistInsertHead(TESTPROC_DLIST_T *pstList, void *pData);
extern int32_t TestProc_dlistInsertTail(TESTPROC_DLIST_T *pstList, void *pData);
extern TESTPROC_DNODE_T *TestProc_dlistFindNodeByData(TESTPROC_DLIST_T *pstList, void *pData);
extern int32_t TestProc_dlistDeleteNode(TESTPROC_DLIST_T *pstList, TESTPROC_DNODE_T *pstNode);

#ifdef __cplusplus
}
#endif

#endif /* __TESTPROC_DLIST_H__ */

/************************ (C) COPYRIGHT STMicroelectronics *****END OF FILE****/
