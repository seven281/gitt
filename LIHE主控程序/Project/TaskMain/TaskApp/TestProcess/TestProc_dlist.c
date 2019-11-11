/******************************************************************
 * (c) Copyright 2018-2025, XXXX, All Rights Reserved.
 * THIS IS UNPUBLISHED PROPRIETARY SOURCE CODE OF XXXX, INC.
 * The copyright notice above does not evidence any actual or intended
 * publication of such source code. 
 *
 *  Subsystem:   TestProc
 *  File:        TestProc_dlist.c
 *  Author:      王殿光
 *  Description: Double List for C source files.
 *
 *  Function List:  
 *     1. ....
 *  History:        
 *                  
 *     1. Date:
 *        Author:
 *        Modification:
 *     2. ...
 *  
 ***************************************************************/

/* Includes ------------------------------------------------------------------*/
#include "TestProc_dlist.h"

#define TESTPROC_DLIST_NODE_SIZE                 (sizeof(TESTPROC_DNODE_T))
#define TESTPROC_DLIST_NODE_MAX                  (10000u)

uint8_t *g_pu8DListNodeMemBuf;
static OS_MEM g_stDListNodeMemPool;

/****************************************************************************
 *
 * Function Name:    TestProc_dlistModuleInit
 * Input:            None
 *                   
 * Output:           None
 *                   
 * Returns:          TESTPROC_RESULT_OK,TESTPROC_RESULT_NG
 * Description:      alloc mem pool for double list module.
 * Note:             You have to call it once after Reset.
 ***************************************************************************/
int32_t TestProc_dlistModuleInit(void)
{
    OS_ERR os_err;

    g_pu8DListNodeMemBuf = TestProc_AllocMemory(TESTPROC_DLIST_NODE_MAX*TESTPROC_DLIST_NODE_SIZE);
    if (NULL != g_pu8DListNodeMemBuf)
    {
        OSMemCreate(&g_stDListNodeMemPool, "OSMem_DListNode", g_pu8DListNodeMemBuf, 
                    TESTPROC_DLIST_NODE_MAX, TESTPROC_DLIST_NODE_SIZE, &os_err);
        if (OS_ERR_NONE != os_err)
        {
            DebugMsg("ERR>DListNodeMemPool:%d\r\n", os_err);
            return TESTPROC_RESULT_NG;		
        }
    }
    else
    {
        DebugMsg("ERR>DListBuf\r\n");
    }

    return TESTPROC_RESULT_OK;	
}

/****************************************************************************
 *
 * Function Name:    TestProc_dlistInit
 * Input:            pstList - double list for initialize;
 *                   
 * Output:           None
 *                   
 * Returns:          TESTPROC_RESULT_OK,TESTPROC_RESULT_ERR_PARAM
 * Description:      Initialize one double list.
 * Note:             
 ***************************************************************************/
int32_t TestProc_dlistInit(TESTPROC_DLIST_T *pstList)
{
    if (NULL == pstList)
    {
        return TESTPROC_RESULT_ERR_PARAM;
    }
    pstList->u32Size = 0;
    pstList->pstHead = NULL;
    pstList->pstTail = NULL;

    return TESTPROC_RESULT_OK;	
}

/****************************************************************************
 *
 * Function Name:    TestProc_dlistAllocNode
 * Input:            pData - Node Data;
 *                   
 * Output:           None
 *                   
 * Returns:          new node
 * Description:      Alloc One Double List Node From Memary Pool.
 * Note:             
 ***************************************************************************/
static TESTPROC_DNODE_T * TestProc_dlistAllocNode(void *pData)
{
    OS_ERR os_err;
    TESTPROC_DNODE_T *pstNewNode;
	
    pstNewNode = (TESTPROC_DNODE_T *)OSMemGet(&g_stDListNodeMemPool, &os_err);
    if (NULL == pstNewNode)
    {
        return NULL;
    }
    else
    {
        pstNewNode->next  = NULL;
        pstNewNode->prev  = NULL;
        pstNewNode->pData = pData;
        return pstNewNode;		
    }
}

/****************************************************************************
 *
 * Function Name:    TestProc_dlistInsertHead
 * Input:            pstList - The head of double list;
 *                   pData - list data
 * Output:           None
 *                   
 * Returns:          TESTPROC_RESULT_OK,TESTPROC_RESULT_NG,TESTPROC_RESULT_ERR_PARAM
 * Description:      Get a new node, insert to pstList->next.
 * Note:             Double List Had a head, new Node insert to Head->next
 ***************************************************************************/
int32_t TestProc_dlistInsertHead(TESTPROC_DLIST_T *pstList, void * pData)
{
    TESTPROC_DNODE_T *pstNewNode;

    if (NULL == pstList)
    {
        return TESTPROC_RESULT_ERR_PARAM;
    }
	
    pstNewNode = TestProc_dlistAllocNode(pData);	
    if (NULL == pstNewNode)	
    {
        return TESTPROC_RESULT_NG;
    }
    else
    {
        if (pstList->u32Size == 0)
        {
            pstList->pstHead = pstNewNode;
            pstList->pstTail = pstNewNode;
            pstNewNode->next = NULL;
            pstNewNode->prev = NULL;
        }
        else
        {
            pstNewNode->prev = NULL;
			pstList->pstHead->prev = pstNewNode;
			pstNewNode->next = pstList->pstHead;
            pstList->pstHead = pstNewNode;
        }
        pstList->u32Size++;		
    }

    return TESTPROC_RESULT_OK;
}

/****************************************************************************
 *
 * Function Name:    TestProc_dlistInsertTail
 * Input:            pstList - The head of double list;
 *                   pData - list data
 * Output:           None
 *                   
 * Returns:          TESTPROC_RESULT_OK,TESTPROC_RESULT_NG
 * Description:      Get a new node, insert to the tail of the dlist.
 * Note:             Double List Had a head, new Node insert to Head->next
 ***************************************************************************/
int32_t TestProc_dlistInsertTail(TESTPROC_DLIST_T *pstList, void *pData)
{
    TESTPROC_DNODE_T *pstNewNode;

    if (NULL == pstList)
    {
        return TESTPROC_RESULT_ERR_PARAM;
    }
	
    pstNewNode = TestProc_dlistAllocNode(pData);	
    if (NULL == pstNewNode)	
    {
        return TESTPROC_RESULT_NG;
    }
    else
    {
        if (pstList->u32Size == 0)
        {
            pstList->pstHead = pstNewNode;
            pstList->pstTail = pstNewNode;
            pstNewNode->next = NULL;
            pstNewNode->prev = NULL;
        }
        else
        {
            pstNewNode->next = NULL;
            pstList->pstTail->next = pstNewNode;
            pstNewNode->prev = pstList->pstTail;
            pstList->pstTail = pstNewNode;
        }
        pstList->u32Size++;		
    }

    return TESTPROC_RESULT_OK;
}

/****************************************************************************
 *
 * Function Name:    TestProc_dlistFindNodeByData
 * Input:            pstList - The double list;
 *                   pData - the node of pData
 * Output:           None
 *                   
 * Returns:          TESTPROC_RESULT_OK,TESTPROC_RESULT_ERR_PARAM
 * Description:      find one node which ->pData == pData.
 * Note:             
 ***************************************************************************/
TESTPROC_DNODE_T *TestProc_dlistFindNodeByData(TESTPROC_DLIST_T *pstList, void *pData)
{
    TESTPROC_DNODE_T *pstNodeTmp;
    uint8_t u8FindNodeOk = FALSE;

    if (NULL == pstList || NULL == pData)
    {
        return NULL;
    }
    pstNodeTmp = pstList->pstHead;
    while(NULL != pstNodeTmp)
    {
        if(pstNodeTmp->pData == pData)
        {
            u8FindNodeOk = TRUE;
            break;
        }
        pstNodeTmp = pstNodeTmp->next;
    }

    return (u8FindNodeOk == FALSE ? NULL : pstNodeTmp);
}

/****************************************************************************
 *
 * Function Name:    TestProc_dlistDeleteNode
 * Input:            pstList - The double list;
 *                   pstNode - the node of to delete
 * Output:           None
 *                   
 * Returns:          TESTPROC_RESULT_OK,TESTPROC_RESULT_ERR_PARAM
 * Description:      delete node from dlist.
 * Note:             
 ***************************************************************************/
int32_t TestProc_dlistDeleteNode(TESTPROC_DLIST_T *pstList, TESTPROC_DNODE_T *pstNode)
{
    OS_ERR os_err;

    if (NULL == pstList || NULL == pstNode)
    {
        return TESTPROC_RESULT_ERR_PARAM;
    }

    if (NULL == pstNode->prev)    //删除的是头部节点
    {
        pstList->pstHead = pstNode->next;
        pstList->pstHead->prev = NULL;		
    }

    if (NULL == pstNode->next)    //删除的是尾部节点
    {
        pstList->pstTail = pstNode->prev;
        pstList->pstTail->next = NULL;		
    }
    else                          //删除的是中间节点
    {
        pstNode->next->prev = pstNode->prev;
        pstNode->prev->next = pstNode->next;
    }
    pstList->u32Size--; 	

    OSMemPut(&g_stDListNodeMemPool, pstNode, &os_err);
    if(OS_ERR_NONE != os_err)
    {
         DebugMsg("ERR>DListFree:%d\r\n", os_err);
    }
	
    return TESTPROC_RESULT_OK;	
}

/************************ (C) COPYRIGHT STMicroelectronics *****END OF FILE****/
