/******************************************************************
 * (c) Copyright 2018-2025, XXXX, All Rights Reserved.
 * THIS IS UNPUBLISHED PROPRIETARY SOURCE CODE OF XXXX, INC.
 * The copyright notice above does not evidence any actual or intended
 * publication of such source code. 
 *
 *  Subsystem:   TestProc
 *  File:        TestProc_testData.c
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
#include "TestProc_boardID.h"
#include "TestProc_testData.h"

#define TESTPROC_TEST_DATA_SIZE             (sizeof(TESTPROC_ITEM_T))
#define TESTPROC_TEST_DATA_MAX              (6000u)
#define TESTPROC_PUST_ONE_TESTDATA(a)       TestProc_dlistInsertTail(&g_stTestDataList, (a))
#define TESTPROC_PUST_ONE_EMTESTDATA(a)     TestProc_dlistInsertTail(&g_stEmTestDataList, (a))
#define TESTPROC_RACK_SIZE                  (sizeof(TESTPROC_RACK_STATE_T))
#define TESTPROC_RACK_MAX                   (64)
#define TestProc_CreatItemPool()            do { OSMemCreate(&g_stTestDataMemPool, "OSMem_TestData", g_pu8TestDataMemBuf, TESTPROC_TEST_DATA_MAX, TESTPROC_TEST_DATA_SIZE, &os_err); }while(0)

uint8_t  *g_pu8TestDataMemBuf = NULL;
static OS_MEM g_stTestDataMemPool;
static TESTPROC_DLIST_T g_stTestDataList;    //
static TESTPROC_DLIST_T g_stEmTestDataList;  //

static int32_t TestProc_testDataPopByNode(TESTPROC_DNODE_T *pstListNode, uint8_t u8EmItem);


/****************************************************************************
 *
 * Function Name:    TestProc_testDataInit
 * Input:            None
 *                   
 * Output:           None
 *                   
 * Returns:          TESTPROC_RESULT_OK
 * Description:      Initialize test data memary pool.
 * Note:             
 ***************************************************************************/
int32_t TestProc_testDataInit(void)
{
    OS_ERR os_err;
    //DebugMsg("TestDataMem:0x%08X,%d,%d\r\n", g_pu8TestDataMemBuf, sizeof(g_pu8TestDataMemBuf), sizeof(APP_TCP_TEST_DATA_T));
    g_pu8TestDataMemBuf = TestProc_AllocMemory(TESTPROC_TEST_DATA_MAX*TESTPROC_TEST_DATA_SIZE);
    if (NULL != g_pu8TestDataMemBuf)
    {
        TestProc_CreatItemPool();
        if (OS_ERR_NONE != os_err)
        {
            DebugMsg("ERR>TestDataMemPool:%d\r\n", os_err);
            return TESTPROC_RESULT_NG;
        }
    }
    else
    {
        DebugMsg("ERR>TestDataBuf\r\n");
    }
    TestProc_dlistInit(&g_stTestDataList);
    TestProc_dlistInit(&g_stEmTestDataList);
    
    return TESTPROC_RESULT_OK;	
}

/****************************************************************************
 *
 * Function Name:    TestProc_testDataPutBack
 * Input:            pOneTestData - Point to one test data struct
 *                   u8Head - TRUE - Put Item to List Head, 
 *                            FALSE - Put Item to List Tail
 *                   u8EmData TRUE - Emergent Item
 *                            FALSE - Normal Item
 * Output:           None
 *                   
 * Returns:          TESTPROC_RESULT_OK,RET_ERR_MEM_OUT,TESTPROC_RESULT_ERR_PARAM
 * Description:      push one test data struct to test data list.
 * Note:             
 ***************************************************************************/
int32_t TestProc_testDataPutBack(void *pOneTestData, uint8_t u8Head, uint8_t u8EmData)
{
    int32_t result = TESTPROC_RESULT_OK;
    TESTPROC_DLIST_T *pstList = &g_stTestDataList;

    if (TRUE == u8EmData)
    {
        pstList = &g_stEmTestDataList;
    }

    if (TRUE == u8Head)
    {
        result = TestProc_dlistInsertHead(pstList, pOneTestData);
    }
    else
    {
        result = TestProc_dlistInsertTail(pstList, pOneTestData);
    }
		
    return result;
}

/****************************************************************************
 *
 * Function Name:    TestProc_testGetEmptyItem
 * Input:            None
 *                   
 * Output:           None
 *                   
 * Returns:          Empty Item
 * Description:      
 * Note:             
 ***************************************************************************/
void *TestProc_testGetEmptyItem(void)
{
    OS_ERR os_err;

    return OSMemGet(&g_stTestDataMemPool, &os_err);
}

/****************************************************************************
 *
 * Function Name:    TestProc_testDataPush
 * Input:            pstOneTestData - Point to one test data struct
 *                   u16Quantity - test data struct quantity
 * Output:           None
 *                   
 * Returns:          TESTPROC_RESULT_ERR_PARAM,push data count
 * Description:      push some test data struct to test data list.
 * Note:             
 ***************************************************************************/
int32_t TestProc_testDataPush(void const *pOneTestData, uint16_t u16Quantity)
{
    int32_t i;
	int32_t pustCount;
    int32_t s32Result;
    OS_ERR  os_err;
    TESTPROC_ITEM_T	*pstTestData;
    __ITEM_T	*pstRecvItem;

    if (NULL == pOneTestData || 0 == u16Quantity)
    {
        return TESTPROC_RESULT_ERR_PARAM;
    }

    DebugMsg("PushItem:%d\r\n", u16Quantity);
    pustCount = 0;
    for (i=0; i<u16Quantity; i++)
    {
        if (g_stTestDataMemPool.NbrFree < 200)
        {
            break;
        }
    
        pstTestData = OSMemGet(&g_stTestDataMemPool, &os_err);
        if (NULL == pstTestData)
        {
            DebugMsg("ERR>NoItemMemory\r\n");
            break;
        }
        pstRecvItem  = (__ITEM_T *)((uint8_t *)pOneTestData + sizeof(__ITEM_T)*i);
        if (TestProc_testIsNormalItem(pstRecvItem->u8TestType))
        {
            s32Result = TESTPROC_PUST_ONE_TESTDATA(pstTestData);
            DebugMsg(" _Item:%4d,%d,%3d,%3d,%3d,%3d,%3d,%3d,%3d,%3d,%3d,%3d,%d\r\n", pstRecvItem->u32TestID, pstRecvItem->u8AddStep, pstRecvItem->u16SampleQuantity, pstRecvItem->u16SampleDiluteQuantity,
                pstRecvItem->u16R1ID, pstRecvItem->u16R2ID, pstRecvItem->u16R3ID, pstRecvItem->u16MID, pstRecvItem->u16R1Quantity, pstRecvItem->u16R2Quantity, pstRecvItem->u16R3Quantity, pstRecvItem->u16MQuantity, s32Result);
        }
        else  // 
        {
            s32Result = TESTPROC_PUST_ONE_EMTESTDATA(pstTestData);
            DebugMsg(" _EmItem:%4d,%d,%3d,%3d,%3d,%3d,%3d,%3d,%d\r\n", pstRecvItem->u32TestID, pstRecvItem->u8AddStep, pstRecvItem->u16SampleQuantity, pstRecvItem->u16SampleDiluteQuantity,
                pstRecvItem->u16R1Quantity, pstRecvItem->u16R2Quantity, pstRecvItem->u16R3Quantity, pstRecvItem->u16MQuantity, s32Result);
        }
        if (s32Result == TESTPROC_RESULT_OK)
        {
            pstTestData->u8ReagentPos = 0;
            Mem_Copy(&pstTestData->stItem, pstRecvItem, sizeof(TESTPROC_ITEM_T));
            if (TestProc_testIsTwoStepItem(pstTestData->stItem.u8AddStep))
            {
                DebugMsg("TwoStepItem\r\n");
                pstTestData->eTwoStepState = TESTPROC_TWO_STEP_ENABLE;
            }
            else
            {
                DebugMsg("OneStepItem\r\n");
                pstTestData->eTwoStepState = TESTPROC_TWO_STEP_NONE;
            }
            //DebugMsg("ItemInMem:%p,%d,%d\r\n", pstTestData, pstTestData->u32TestID, pstTestData->u32TestID);
        }
        else
        {
            DebugMsg("ERR>ItemPust:%dr\n", s32Result);
            OSMemPut(&g_stTestDataMemPool, pstTestData, &os_err);
            break;
        }
        pustCount++;
    }

    return pustCount;	
}

/****************************************************************************
 *
 * Function Name:    TestProc_ItemPop
 * Input:            pstListNode - Point to double list node
 *                   
 * Output:           None
 *                   
 * Returns:          Test Item Point
 * Description:      .
 * Note:             
 ***************************************************************************/
void *TestProc_ItemPop(void)
{
    TESTPROC_DNODE_T *pstNodeTmp;
    TESTPROC_ITEM_T *pstItem = NULL;

    pstNodeTmp = g_stTestDataList.pstHead;
    if (NULL != pstNodeTmp) 
    {
        pstItem = (TESTPROC_ITEM_T *)pstNodeTmp->pData;
        TestProc_dlistDeleteNode(&g_stTestDataList, pstNodeTmp);
    }

    return (void *)pstItem;	
}

/****************************************************************************
 *
 * Function Name:    TestProc_EmItemPop
 * Input:            pstListNode - Point to double list node
 *                   
 * Output:           None
 *                   
 * Returns:          Emergent Test Item Point
 * Description:      .
 * Note:             
 ***************************************************************************/
void *TestProc_EmItemPop(void)
{
    TESTPROC_DNODE_T *pstNodeTmp;
    TESTPROC_ITEM_T *pstItem = NULL;

    pstNodeTmp = g_stEmTestDataList.pstHead;
    if (NULL != pstNodeTmp)
    {
        pstItem = (TESTPROC_ITEM_T *)pstNodeTmp->pData;
        TestProc_dlistDeleteNode(&g_stEmTestDataList, pstNodeTmp);
    }
    
    return (void *)pstItem;	
}

/****************************************************************************
 *
 * Function Name:    app_testDataPopByNode
 * Input:            pstListNode - Point to double list node
 *                   
 * Output:           None
 *                   
 * Returns:          TESTPROC_RESULT_OK,TESTPROC_RESULT_ERR_MEM_OUT,TESTPROC_RESULT_ERR_PARAM
 * Description:      .
 * Note:             
 ***************************************************************************/
static int32_t TestProc_testDataPopByNode(TESTPROC_DNODE_T *pstListNode, uint8_t u8EmItem)
{
    OS_ERR os_err;
	
    if (NULL == pstListNode)
    {
        return TESTPROC_RESULT_ERR_PARAM;
    }

    if(pstListNode->pData != NULL)
    {
        OSMemPut(&g_stTestDataMemPool, pstListNode->pData, &os_err);
        if(OS_ERR_NONE != os_err)
        {
            DebugMsg("ERR>app_testDataPopByNode:%d\r\n", os_err);
            return TESTPROC_RESULT_NG;			
        }
    }
    if (TRUE == u8EmItem)
    {
	    TestProc_dlistDeleteNode(&g_stEmTestDataList, pstListNode);
    }
    else
	{
	    TestProc_dlistDeleteNode(&g_stTestDataList, pstListNode);
    }
    
    return TESTPROC_RESULT_OK;	
}

/****************************************************************************
 *
 * Function Name:    app_testDataFreeItemByID
 * Input:            u32TestID - A test item ID
 *                   
 * Output:           None
 *                   
 * Returns:          TESTPROC_RESULT_OK,TESTPROC_RESULT_NG
 * Description:      delete a test item
 * Note:             
 ***************************************************************************/
int32_t TestProc_testDataFreeItemByID(uint32_t u32TestID)
{
    TESTPROC_DNODE_T *pstNodeTmp;
    TESTPROC_ITEM_T *pstTestItem;	
    uint8_t u8DeleteOk = FALSE;	

    // Normal Test Item
    pstNodeTmp = g_stTestDataList.pstHead;
    while(NULL != pstNodeTmp)	
    {
        pstTestItem = (TESTPROC_ITEM_T *)pstNodeTmp->pData;
        if (pstTestItem->stItem.u32TestID == u32TestID)
        {
            if (TestProc_testDataPopByNode(pstNodeTmp, FALSE) == TESTPROC_RESULT_OK)
            {
                u8DeleteOk = TRUE;			
            }
            break;
        }
        pstNodeTmp = pstNodeTmp->next;		
    }
    if (FALSE == u8DeleteOk)
    {
        // Emergent Test Item
        pstNodeTmp = g_stEmTestDataList.pstHead;
        while(NULL != pstNodeTmp)	
        {
            pstTestItem = (TESTPROC_ITEM_T *)pstNodeTmp->pData;
            if (pstTestItem->stItem.u32TestID == u32TestID)
            {
                if (TestProc_testDataPopByNode(pstNodeTmp, TRUE) == TESTPROC_RESULT_OK)
                {
                    u8DeleteOk = TRUE;			
                }
                break;
            }
            pstNodeTmp = pstNodeTmp->next;		
        }
    }
	
    return (TRUE == u8DeleteOk ? TESTPROC_RESULT_OK : TESTPROC_RESULT_NG);	
}

/****************************************************************************
 *
 * Function Name:    TestProc_testDataItemFree
 * Input:            pItem - Item pointer
 *                   
 * Output:           None
 *                   
 * Returns:          TESTPROC_RESULT_OK,TESTPROC_RESULT_NG
 * Description:      
 * Note:             
 ***************************************************************************/
int32_t TestProc_testDataItemFree(void *pItem)
{
    int32_t  s32Result = TESTPROC_RESULT_OK;
    OS_ERR os_err;

    if (NULL == pItem)
    {
        return TESTPROC_RESULT_ERR_PARAM;
    }
	
    OSMemPut(&g_stTestDataMemPool, pItem, &os_err);
    if (OS_ERR_NONE != os_err)
    {
        DebugMsg("ERR>testDataItemFree:%d\r\n", os_err);
        s32Result = TESTPROC_RESULT_NG;
    }

    return s32Result;
}

/****************************************************************************
 *
 * Function Name:    app_testDataClean
 * Input:            None
 *                   
 * Output:           None
 *                   
 * Returns:          TESTPROC_RESULT_OK,TESTPROC_RESULT_NG
 * Description:      delete all test item
 * Note:             
 ***************************************************************************/
void TestProc_testDataClean(void)
{
    TESTPROC_DNODE_T *pstNodeTmp;
    OS_ERR os_err;
    
    pstNodeTmp = g_stEmTestDataList.pstHead;
    while(NULL != pstNodeTmp)	
    {
        TestProc_testDataPopByNode(pstNodeTmp, TRUE);
        pstNodeTmp = pstNodeTmp->next;		
    }

    pstNodeTmp = g_stTestDataList.pstHead;
    while(NULL != pstNodeTmp)	
    {
        TestProc_testDataPopByNode(pstNodeTmp, FALSE);
        pstNodeTmp = pstNodeTmp->next;		
    }
    
    TestProc_CreatItemPool();  // For Clean Item Pool
    
    return ;	
}

/****************************************************************************
 *
 * Function Name:    TestProc_testDataCount
 * Input:            None
 *                   
 * Output:           None
 *                   
 * Returns:          TESTPROC_RESULT_OK,TESTPROC_RESULT_NG
 * Description:      Get The Number Of The Test Item
 * Note:             
 ***************************************************************************/
uint32_t TestProc_testDataCount(void)
{
    return g_stTestDataList.u32Size;
}

/****************************************************************************
 *
 * Function Name:    TestProc_testDataEmItemCount
 * Input:            None
 *                   
 * Output:           None
 *                   
 * Returns:          TESTPROC_RESULT_OK,TESTPROC_RESULT_NG
 * Description:      Get The Number Of The Test Item
 * Note:             
 ***************************************************************************/
uint32_t TestProc_testDataEmItemCount(void)
{
    return g_stEmTestDataList.u32Size;
}

/****************************************************************************
 *
 * Function Name:    TestProc_testDataShowItem
 * Input:            None
 *                   
 * Output:           None
 *                   
 * Returns:          TESTPROC_RESULT_OK,TESTPROC_RESULT_NG
 * Description:      List test item
 * Note:             
 ***************************************************************************/
void TestProc_testDataShowItem(void)
{
    TESTPROC_DNODE_T *pstNodeTmp;

    pstNodeTmp = g_stEmTestDataList.pstHead;
    while(NULL != pstNodeTmp)	
    {
        DebugMsg("_EmItem %d\r\n", ((TESTPROC_ITEM_T *)pstNodeTmp->pData)->stItem.u32TestID);		
        pstNodeTmp = pstNodeTmp->next;		
    }
    
    pstNodeTmp = g_stTestDataList.pstHead;
    while(NULL != pstNodeTmp)	
    {
        DebugMsg("_Item %d\r\n", ((TESTPROC_ITEM_T *)pstNodeTmp->pData)->stItem.u32TestID);		
        pstNodeTmp = pstNodeTmp->next;		
    }
}

/************************ (C) COPYRIGHT STMicroelectronics *****END OF FILE****/
