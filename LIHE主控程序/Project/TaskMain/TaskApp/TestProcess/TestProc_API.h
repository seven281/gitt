#ifndef __TESTPROC_API_H_
#define __TESTPROC_API_H_
#ifdef __cplusplus
extern "C"
{
#endif
    
#include "TSC_Inc.h"
#include "TestProc_Warning.h"
#include "TestProc_Typedef.h"

typedef enum
{
    TESTPROC_API_HAND_TARGET_REACT = 0,
    TESTPROC_API_HAND_TARGET_WASH,
    TESTPROC_API_HAND_TARGET_MEASUREROOM,
    TESTPROC_API_HAND_TARGET_NEWDISK
} TESTPROC_API_HAND_TARGET;
    
typedef enum
{
    TESTPROC_API_RESULT_OK = 0,  // 动作执行成功
    TESTPROC_API_RESULT_NG,      // 动作执行失败
    TESTPROC_API_RESULT_WAIT,    // 动作执行
    TESTPROC_API_RESULT_TIMEOUT, // 动作执行超时
    TESTPROC_API_RESULT_EMPTY    // 当前没有动作执行
} TESTPROC_API_RESULT_E;

#define TestProc_Api_IsResetCompile(result) (TESTPROC_API_RESULT_OK == (result) || TESTPROC_API_RESULT_NG == (result))
    
typedef enum
{
    TESTPROC_API_DOOR_OP_FULLOPEN = 0,
    TESTPROC_API_DOOR_OP_HALFOPEN,
    TESTPROC_API_DOOR_OP_CLOSE,
} TESTPROC_API_DOOR_OP;

// ==========================================================================
//
// Reaction Disk API
//
//===========================================================================

/****************************************************************************
*
* Function Name:    TestProc_Api_ReactInit
* Input:            None
*
* Output:           None
*                   
* Returns:          
* Description:      Reaction Disk Data Initialization
* Note:             
***************************************************************************/
__INLINE static void TestProc_Api_ReactInit(void)
{
    TSC_DishReactionAndReagentMixResultDataInit();
}

/****************************************************************************
*
* Function Name:    TestProc_Api_ReactReset
* Input:            None
*
* Output:           None
*                   
* Returns:          
* Description:      Reaction Disk Reset
* Note:             
***************************************************************************/
__INLINE static LH_ERR TestProc_Api_ReactReset(void)
{
    LH_ERR eResult;
    
    eResult = TSC_DishReactionInitWhileAck();
    
    return eResult;
}

/****************************************************************************
*
* Function Name:    TestProc_Api_ReactMoveToPos
* Input:            u8Pos - Target Position
*
* Output:           None
*                   
* Returns:          
* Description:      Rotate To The Target Position
* Note:             
***************************************************************************/
__INLINE static LH_ERR TestProc_Api_ReactMoveToPos(uint8_t u8Pos)
{
    LH_ERR eResult;
    
    eResult = TSC_DishReactionMove2SpecialHoleWhileAck(u8Pos);
    
    return eResult;
}

/****************************************************************************
*
* Function Name:    TestProc_Api_ReactRotatePos
* Input:            u8Quantity - 
*
* Output:           None
*                   
* Returns:          
* Description:      Rotate To The Target Position
* Note:             
***************************************************************************/
__INLINE static LH_ERR TestProc_Api_ReactRotatePos(uint8_t u8Quantity)
{
    LH_ERR eResult;
    
    eResult = TSC_DishReactionMoveAnyHoleStepWhileAck(u8Quantity);
    
    return eResult;
}

/****************************************************************************
*
* Function Name:    TestProc_Api_ReactMMixReset
* Input:            None
*
* Output:           None
*                   
* Returns:          
* Description:      M Mix Reset
* Note:             
***************************************************************************/
__INLINE static LH_ERR TestProc_Api_ReactMMixReset(void)
{
    LH_ERR eResult;
    
    eResult = TSC_BeadMixInitWhileAck();
    
    return eResult;
}

/****************************************************************************
*
* Function Name:    TestProc_Api_ReactMMixAction
* Input:            u16Ms - Millisecond
*
* Output:           None
*                   
* Returns:          
* Description:      M Mix In Reaction Disk
* Note:             
***************************************************************************/
__INLINE static LH_ERR TestProc_Api_ReactMMixAction(uint16_t u16Ms)
{
    LH_ERR eResult;
    
    eResult = TSC_BeadMixRotateSpecialTimeWhileAck(u16Ms);
    
    return eResult;
}

/****************************************************************************
*
* Function Name:    TestProc_Api_ReactMCheckResult
* Input:            None
*
* Output:           None
*                   
* Returns:          
* Description:      Rotate To The Target Position
* Note:             
***************************************************************************/
__INLINE static TESTPROC_API_RESULT_E TestProc_Api_ReactMCheckResult(void)
{
    TSC_CMD_STATE eResult;
    LH_ERR err;
    
    eResult = TSC_BeadMixGetLastCommandStatusAndResult(&err);
    if (TSC_CMD_STATE_IDLE == eResult || TSC_CMD_STATE_COMPLETE == eResult)
    {
        if (LH_ERR_NONE == err)
        {
            return TESTPROC_API_RESULT_OK;
        }
        else
        {
            return TESTPROC_API_RESULT_NG;
        }
    }
    else
    {
        return TESTPROC_API_RESULT_WAIT;
    }
}

/****************************************************************************
*
* Function Name:    TestProc_Api_ReactMixReset
* Input:            None
*
* Output:           None
*                   
* Returns:          
* Description:      Reaction Disk Reset
* Note:             
***************************************************************************/
__INLINE static LH_ERR TestProc_Api_ReactMixReset(void)
{
    LH_ERR eResult;
    
    eResult = TSC_ReagentMixInitWhileAck();
    
    return eResult;
}

/****************************************************************************
*
* Function Name:    TestProc_Api_ReactMixReset
* Input:            u16Ms - Millisecond
*
* Output:           None
*                   
* Returns:          
* Description:      Reagent Mix In Reaction Disk
* Note:             
***************************************************************************/
__INLINE static LH_ERR TestProc_Api_ReactMixAction(uint16_t u16Ms)
{
    LH_ERR eResult;
    
    eResult = TSC_ReagentMixRotateAnyTimeMsWhileAck(u16Ms);
    
    return eResult;
}

/****************************************************************************
*
* Function Name:    TestProc_Api_ReactIsCupIn
* Input:            None
*
* Output:           None
*                   
* Returns:          
* Description:      
* Note:             
***************************************************************************/
__INLINE static uint8_t TestProc_Api_ReactIsCupIn(void)
{
    return TSC_DishReactionCheckCupExist() == ACTION_SENSOR_STATUS_EXIST ? 1 : 0;
}

/****************************************************************************
*
* Function Name:    TestProc_Api_ReactRotateToPos
* Input:            None
*
* Output:           None
*                   
* Returns:          
* Description:      Rotate To The Target Position
* Note:             
***************************************************************************/
__INLINE static TESTPROC_API_RESULT_E TestProc_Api_ReactCheckResult(void)
{
    TSC_CMD_STATE eResult;
    LH_ERR err;
    
    eResult = TSC_DishReactionAndReagentMixGetLastCommandStatusAndResult(&err);
    if (TSC_CMD_STATE_IDLE == eResult || TSC_CMD_STATE_COMPLETE == eResult)
    {
        if (LH_ERR_NONE == err)
        {
            return TESTPROC_API_RESULT_OK;
        }
        else
        {
            return TESTPROC_API_RESULT_NG;
        }
    }
    else
    {
        return TESTPROC_API_RESULT_WAIT;
    }
}
    
// ==========================================================================
//
// Reagent Disk API
//
//===========================================================================

/****************************************************************************
*
* Function Name:    TestProc_Api_ReagentDiskInit
* Input:            None
*
* Output:           None
*                   
* Returns:          
* Description:      Reaction Disk Data Initialization
* Note:             
***************************************************************************/
__INLINE static void TestProc_Api_ReagentDiskInit(void)
{
    TSC_DishReagentResultDataInit();
}

/****************************************************************************
*
* Function Name:    TestProc_Api_ReagentDiskReset
* Input:            None
*
* Output:           None
*                   
* Returns:          
* Description:      Reaction Disk Reset
* Note:             
***************************************************************************/
__INLINE static LH_ERR TestProc_Api_ReagentDiskReset(void)
{
    LH_ERR eResult;
    
    eResult = TSC_DishReagentInitWhileAck();
    
    return eResult;
}

/****************************************************************************
*
* Function Name:    TestProc_Api_ReagentDiskMoveToPos
* Input:            u8Pos - The Position In The Reagent Disk
*                   eOffset - Offset
*
* Output:           None
*                   
* Returns:          
* Description:      Reaction Disk Data Initialization
* Note:             
***************************************************************************/
__INLINE static LH_ERR TestProc_Api_ReagentDiskMoveToPos(uint8_t u8Pos, DISH_REAGENT_OFFSET eOffset)
{
    LH_ERR eResult;
    
    eResult = TSC_DishReagentMove2SpecialHoleWithOffsetWhileAck(u8Pos, eOffset);
    
    return eResult;
}

/****************************************************************************
*
* Function Name:    TestProc_Api_ReagentDiskRotate
* Input:            u8Quantity - Rotate Quantity
*
* Output:           None
*                   
* Returns:          
* Description:      
* Note:             
***************************************************************************/
__INLINE static LH_ERR TestProc_Api_ReagentDiskRotate(uint8_t u8Quantity)
{
    LH_ERR eResult;
    
    eResult = TSC_DishReagentMove2SpecialHoleStepWhileAck(u8Quantity);
    
    return eResult;
}

/****************************************************************************
*
* Function Name:    TestProc_Api_ReagentDiskGetPos
* Input:            None
*
* Output:           None
*                   
* Returns:          Current Pos
* Description:      Reagent Disk Current Pos
* Note:             
***************************************************************************/
__INLINE static uint8_t TestProc_Api_ReagentDiskGetPos(void)
{
    return TSC_DishReagentReadCurrentHoleIndex();
}

/****************************************************************************
*
* Function Name:    TestProc_Api_ReagentDiskCheckResult
* Input:            None
*
* Output:           None
*                   
* Returns:          
* Description:      Check Result
* Note:             
***************************************************************************/
__INLINE static TESTPROC_API_RESULT_E TestProc_Api_ReagentDiskCheckResult(LH_ERR *pErr)
{
    TSC_CMD_STATE eResult;
    
    eResult = TSC_DishReagentGetLastCommandStatusAndResult(pErr);
    if (TSC_CMD_STATE_IDLE == eResult || TSC_CMD_STATE_COMPLETE == eResult)
    {
        if (LH_ERR_NONE == *pErr)
        {
            return TESTPROC_API_RESULT_OK;
        }
        else
        {
            return TESTPROC_API_RESULT_NG;
        }
    }
    else
    {
        return TESTPROC_API_RESULT_WAIT;
    }
}
// Scan Bar Code
/****************************************************************************
*
* Function Name:    TestProc_Api_ReagentDiskBarInit
* Input:            None
*
* Output:           None
*                   
* Returns:          
* Description:      Reaction Disk Bar Data Initialization
* Note:             
***************************************************************************/
__INLINE static void TestProc_Api_ReagentDiskBarInit(void)
{
    TSC_QRCodeScanResultDataInit();
}

/****************************************************************************
*
* Function Name:    TestProc_Api_ReagentDiskBarScan
* Input:            None
*
* Output:           None
*                   
* Returns:          
* Description:      Reaction Disk Bar Reset
* Note:             
***************************************************************************/
__INLINE static LH_ERR TestProc_Api_ReagentDiskBarScan(void)
{
    LH_ERR eResult;
    
    eResult = TSC_QRCodeScanReadOnceWhileAck();
    
    return eResult;
}

/****************************************************************************
*
* Function Name:    TestProc_Api_ReagentDiskBarCheckResult
* Input:            None
*
* Output:           None
*                   
* Returns:          
* Description:      Check Result
* Note:             
***************************************************************************/
__INLINE static TESTPROC_API_RESULT_E TestProc_Api_ReagentDiskBarCheckResult(void)
{
    TSC_CMD_STATE eResult;
    LH_ERR err;
    
    eResult = TSC_QRCodeScanGetLastCommandStatusAndResult(&err);
    if (TSC_CMD_STATE_IDLE == eResult || TSC_CMD_STATE_COMPLETE == eResult)
    {
        if (LH_ERR_NONE == err)
        {
            return TESTPROC_API_RESULT_OK;
        }
        else
        {
            return TESTPROC_API_RESULT_NG;
        }
    }
    else
    {
        return TESTPROC_API_RESULT_WAIT;
    }
}

/****************************************************************************
*
* Function Name:    TestProc_Api_ReagentDiskBarScan
* Input:            None
*
* Output:           Read Length
*                   
* Returns:          
* Description:      Reaction Disk Bar Reset
* Note:             
***************************************************************************/
__INLINE static uint16_t TestProc_Api_ReagentDiskBarReadCode(uint8_t **ppu8Buf)
{
    return TSC_QRCodeScanReadLastReturnCode(ppu8Buf);
}

// ==========================================================================
//
// Reagent API
//
//===========================================================================
/****************************************************************************
*
* Function Name:    TestProc_Api_ReagentInit
* Input:            None
*
* Output:           None
*                   
* Returns:          
* Description:      Reaction Disk Data Initialization
* Note:             
***************************************************************************/
__INLINE static void TestProc_Api_ReagentInit(void)
{
    TSC_NeedleReagentBeadResultDataInit();
}

/****************************************************************************
*
* Function Name:    TestProc_Api_ReagentReset
* Input:            None
*
* Output:           None
*                   
* Returns:          
* Description:      Reaction Disk Reset
* Note:             
***************************************************************************/
__INLINE static LH_ERR TestProc_Api_ReagentReset(void)
{
    LH_ERR eResult;
    
    eResult = TSC_NeedleReagentBeadInitWhileAck();
    
    return eResult;
}

/****************************************************************************
*
* Function Name:    TestProc_Api_ReagentMoveToPos
* Input:            ePos - Postion
*
* Output:           None
*                   
* Returns:          
* Description:      Reaction Disk Reset
* Note:             
***************************************************************************/
__INLINE static LH_ERR TestProc_Api_ReagentMoveToPos(TSC_NEEDLE_REAGENT_BEAD_POS ePos)
{
    LH_ERR eResult;
    
    eResult = TSC_NeedleReagentBeadRotateSpecialPositionWhileAck(ePos);
    
    return eResult;
}

/****************************************************************************
*
* Function Name:    TestProc_Api_ReagentAbsorb
* Input:            u16Quantity - Quantity UL
*                   oneofcup - 调用的杯架以及杯号
                                杯架输入1-30
                                杯号输入1-3;1是外圈，2是中圈，3是内圈
* Output:           None
*                   
* Returns:          
* Description:      Reaction Disk Reset
* Note:             
***************************************************************************/
__INLINE static LH_ERR TestProc_Api_ReagentAbsorb(uint16_t u16Quantity, uint8_t u8Pos, uint8_t u8Offset)
{
    LH_ERR eResult;
    ONE_OF_CUPSTATUS OneOfCup;

    OneOfCup.cupholder = u8Pos;
    OneOfCup.cups = u8Offset;
    eResult = TSC_NeedleReagentBeadAbsorbWhileAck(u16Quantity, OneOfCup);
    
    return eResult;
}
/****************************************************************************
*
* Function Name:    TestProc_Api_GetReagentCupStatus
* Input:            杯架以及杯号
                    u8Pos      杯架输入1-30
                    u8Offset   杯号输入1-3;1是外圈，2是中圈，3是内圈
* Output:           None
*                   
* Returns:          根据输入杯号返回当前状态
                    REAGENT_CUP_STATUS
                       |---- DISH_REAGENT_CUP_STATUS 指定试管杯当前的状态     
                       |---- steps试管针到该试管杯的液面需要走的步数
                       |---- utilization 试管杯中液量百分比值50等于50%:杯中还有一半的液量值为100等于100%:杯中满格
                                         读取百分比有个前提就是杯子的状态为OK
* Description:      
* Note:   
*
****************************************************************************/
__INLINE static REAGENT_CUP_STATUS* TestProc_Api_GetReagentCupStatus(uint8_t u8Pos, uint8_t u8Offset)
{
    ONE_OF_CUPSTATUS OneOfCup;
    
    OneOfCup.cupholder = u8Pos;
    OneOfCup.cups = u8Offset;
    return TSC_NeedlReagentBeadReadCUPStatus(&OneOfCup);
}
/****************************************************************************
*
* Function Name:    TestProc_Api_ReagentInject
* Input:            u16Quantity - Quantity UL
*
* Output:           None
*                   
* Returns:          
* Description:      Reaction Disk Reset
* Note:             
***************************************************************************/
__INLINE static LH_ERR TestProc_Api_ReagentInject(uint16_t u16Quantity)
{
    LH_ERR eResult;
    
    eResult = TSC_NeedleReagentBeadInjectWhileAck(u16Quantity);
    
    return eResult;
}

/****************************************************************************
*
* Function Name:    TestProc_Api_ReagentClean
* Input:            None
*
* Output:           None
*                   
* Returns:          
* Description:      Reaction Disk Reset
* Note:             
***************************************************************************/
__INLINE static LH_ERR TestProc_Api_ReagentClean(TSC_NEEDLE_REAGENT_BEAD_POS ePos, TSC_NEEDLE_REAGENT_BEAD_CLEAN_OPT eOpt)
{
    LH_ERR eResult;
    
    eResult = TSC_NeedleReagentBeadCleanWhileAck(ePos, eOpt);
    
    return eResult;
}

/****************************************************************************
*
* Function Name:    TestProc_Api_ReagentLevelDetect
* Input:            None
*
* Output:           None
*                   
* Returns:          
* Description:      Reagent Liquid level detection
* Note:             
***************************************************************************/
__INLINE static LH_ERR TestProc_Api_ReagentLevelDetect(void)
{
    LH_ERR eResult = LH_ERR_NONE;
    
    //eResult = TSC_NeedleReagentBeadInjectWhileAck(u16Quantity);
    
    return eResult;
}

/****************************************************************************
*
* Function Name:    TestProc_Api_ReagentLevelRead
* Input:            None
*
* Output:           pu16Level - Reagent Liquid level
*                   
* Returns:          
* Description:      Read Reagent Liquid level
* Note:             
***************************************************************************/
__INLINE static LH_ERR TestProc_Api_ReagentLevelRead(uint16_t *pu16Level)
{
    LH_ERR eResult = LH_ERR_NONE;
    
    //eResult = TSC_NeedleReagentBeadInjectWhileAck(u16Quantity);
    *pu16Level = 0x1234;
    
    return eResult;
}

/****************************************************************************
*
* Function Name:    TestProc_Api_ReagentPrime
* Input:            None
*
* Output:           None
*                   
* Returns:          
* Description:      
* Note:             
***************************************************************************/
__INLINE static LH_ERR TestProc_Api_ReagentPrime(TSC_NEEDLE_REAGENT_PRIME_OPT opt)
{
    LH_ERR eResult;
    
    eResult = TSC_NeedleReagentBeadPrimeWhileAck(opt);
    
    return eResult;
}

/****************************************************************************
*
* Function Name:    TestProc_Api_ReagentCheckResult
* Input:            None
*
* Output:           None
*                   
* Returns:          
* Description:      Check Result
* Note:             
***************************************************************************/
__INLINE static TESTPROC_API_RESULT_E TestProc_Api_ReagentCheckResult(LH_ERR *pErr)
{
    TSC_CMD_STATE eResult;
    //LH_ERR err = LH_ERR_NONE;
    
    eResult = TSC_NeedleReagentBeadGetLastCommandStatusAndResult(pErr);
    if (TSC_CMD_STATE_IDLE == eResult || TSC_CMD_STATE_COMPLETE == eResult)
    {
        if (LH_ERR_NONE == *pErr)
        {
            return TESTPROC_API_RESULT_OK;
        }
        else
        {
            return TESTPROC_API_RESULT_NG;
        }
    }
    else
    {
        return TESTPROC_API_RESULT_WAIT;
    }
}

// ==========================================================================
//
// Sample API
//
//===========================================================================
/****************************************************************************
*
* Function Name:    TestProc_Api_SampleInit
* Input:            None
*
* Output:           None
*                   
* Returns:          
* Description:      Sample Data Initialization
* Note:             
***************************************************************************/
__INLINE static void TestProc_Api_SampleInit(void)
{
    TSC_NeedleSampleResultDataInit();
}

/****************************************************************************
*
* Function Name:    TestProc_Api_SampleReset
* Input:            None
*
* Output:           None
*                   
* Returns:          
* Description:      Sample Reset
* Note:             
***************************************************************************/
__INLINE static LH_ERR TestProc_Api_SampleReset(void)
{
    LH_ERR eResult;
    
    eResult = TSC_NeedleSampleInitWhileAck();
    
    return eResult;
}

/****************************************************************************
*
* Function Name:    TestProc_Api_SampleMoveToPos
* Input:            ePos - Postion
*
* Output:           None
*                   
* Returns:          
* Description:      Sample Reset
* Note:             
***************************************************************************/
__INLINE static LH_ERR TestProc_Api_SampleMoveToPos(TSC_NEEDLE_SAMPLE_POS ePos)
{
    LH_ERR eResult;
    
    eResult = TSC_NeedleSampleRotateSpecialPositionWhileAck(ePos);
    
    return eResult;
}

/****************************************************************************
*
* Function Name:    TestProc_Api_SampleAbsorb
* Input:            u16Quantity - Quantity UL
*
* Output:           None
*                   
* Returns:          
* Description:      Reaction Disk Reset
* Note:             
***************************************************************************/
__INLINE static LH_ERR TestProc_Api_SampleAbsorb(uint16_t u16Quantity, uint8_t u8PosChanged)
{
    LH_ERR eResult;
    
    eResult = TSC_NeedleSampleAbsorbOnceWhileAck(u16Quantity, u8PosChanged == 1 ? SAMPLE_NEW : SAMPLE_USED);
    
    return eResult;
}

/****************************************************************************
*
* Function Name:    TestProc_Api_SampleInject
* Input:            u16Quantity - Quantity UL
*
* Output:           None
*                   
* Returns:          
* Description:      Reaction Disk Reset
* Note:             
***************************************************************************/
__INLINE static LH_ERR TestProc_Api_SampleInject(uint16_t u16Quantity)
{
    LH_ERR eResult;
    
    eResult = TSC_NeedleSampleInjectOnceWhileAck(u16Quantity);
    
    return eResult;
}

/****************************************************************************
*
* Function Name:    TestProc_Api_SampleClean
* Input:            TSC_NEEDLE_SAMPLE_PRIME_OPT
*                   样本针灌注类型
* Output:           None
*                   
* Returns:          
* Description:      Sample Reset
* Note:             
***************************************************************************/
__INLINE static LH_ERR TestProc_Api_SampleClean(TSC_NEEDLE_SAMPLE_CLEAN_OPT opt)
{
    LH_ERR eResult;
    
    eResult = TSC_NeedleSampleCleanWhileAck(TSC_NEEDLE_SAMPLE_POS_CLEAN, opt);
    
    return eResult;
}

/****************************************************************************
*
* Function Name:    TestProc_Api_SampleForceClean
* Input:            None
*
* Output:           None
*                   
* Returns:          
* Description:      Sample Reset
* Note:             
***************************************************************************/
__INLINE static LH_ERR TestProc_Api_SampleForceClean(TSC_NEEDLE_SAMPLE_POS pos, TSC_NEEDLE_SAMPLE_CLEAN_OPT opt)
{
    LH_ERR eResult;
    
    eResult = TSC_NeedleSampleForceCleanWhileAck(pos, opt);
    
    return eResult;
}

/****************************************************************************
*
* Function Name:    TestProc_Api_SamplePrime
* Input:            TSC_NEEDLE_SAMPLE_PRIME_OPT
*                   样本针灌注类型
* Output:           None
*                   
* Returns:          
* Description:      Sample Reset
* Note:             
***************************************************************************/
__INLINE static LH_ERR TestProc_Api_SamplePrime(TSC_NEEDLE_SAMPLE_PRIME_OPT opt)
{
    LH_ERR eResult;
    
    eResult = TSC_NeedleSampleCleanPrimeWhileAck(opt);
    
    return eResult;
}

/****************************************************************************
*
* Function Name:    TestProc_Api_SampleForcePrime
* Input:            None
*
* Output:           None
*                   
* Returns:          
* Description:      Sample Reset
* Note:             
***************************************************************************/
__INLINE static LH_ERR TestProc_Api_SampleForcePrime(void)
{
    LH_ERR eResult;
    
    eResult = TSC_NeedleSampleForceCleanPrimeWhileAck();
    
    return eResult;
}

/****************************************************************************
*
* Function Name:    TestProc_Api_SampleCheckResult
* Input:            None
*
* Output:           None
*                   
* Returns:          
* Description:      Check Result
* Note:             
***************************************************************************/
__INLINE static TESTPROC_API_RESULT_E TestProc_Api_SampleCheckResult(LH_ERR *pErr)
{
    TSC_CMD_STATE eResult;
    //LH_ERR err = LH_ERR_NONE;
    
    eResult = TSC_NeedleSampleGetLastCommandStatusAndResult(pErr);
    if (TSC_CMD_STATE_IDLE == eResult || TSC_CMD_STATE_COMPLETE == eResult)
    {
        if (LH_ERR_NONE == *pErr)
        {
            return TESTPROC_API_RESULT_OK;
        }
        else
        {
            return TESTPROC_API_RESULT_NG;
        }
    }
    else
    {
        return TESTPROC_API_RESULT_WAIT;
    }
}
    
// Rack Conveyor Belt
/****************************************************************************
*
* Function Name:    TestProc_Api_SampleConveyorBeltInit
* Input:            None
*
* Output:           None
*                   
* Returns:          
* Description:      Rack Conveyor Belt Data Initialization
* Note:             
***************************************************************************/
__INLINE static void TestProc_Api_SampleConveyorBeltInit(void)
{
    TSC_RankPathWayResultDataInit();
}

/****************************************************************************
*
* Function Name:    TestProc_Api_SampleConveyorBeltReset
* Input:            None
*
* Output:           None
*                   
* Returns:          
* Description:      Sample Reset
* Note:             
***************************************************************************/
__INLINE static LH_ERR TestProc_Api_SampleConveyorBeltReset(void)
{
    LH_ERR eResult;
    
    eResult = TSC_RankPathWayInitWhileAck();
    
    return eResult;
}

/****************************************************************************
*
* Function Name:    TestProc_Api_SampleConveyorBeltMoveToPos
* Input:            None
*
* Output:           None
*                   
* Returns:          
* Description:      Move To Sample Pos, 0~5,6 Mean 1 Pos Of The Next Rack
* Note:             
***************************************************************************/
__INLINE static LH_ERR TestProc_Api_SampleConveyorBeltMoveToPos(uint8_t u8Pos)
{
    LH_ERR eResult;
    
    eResult = TSC_RankPathWayMove2SpecialTubeWhileAck(u8Pos);
    
    return eResult;
}

/****************************************************************************
*
* Function Name:    TestProc_Api_SampleConveyorBeltCheckResult
* Input:            None
*
* Output:           None
*                   
* Returns:          
* Description:      Check Result
* Note:             
***************************************************************************/
__INLINE static TESTPROC_API_RESULT_E TestProc_Api_SampleConveyorBeltCheckResult(LH_ERR *pErr)
{
    TSC_CMD_STATE eResult;
    
    eResult = TSC_RankPathWayGetLastCommandStatusAndResult(pErr);
    if (TSC_CMD_STATE_IDLE == eResult || TSC_CMD_STATE_COMPLETE == eResult)
    {
        if (LH_ERR_NONE == *pErr)
        {
            return TESTPROC_API_RESULT_OK;
        }
        else
        {
            return TESTPROC_API_RESULT_NG;
        }
    }
    else
    {
        return TESTPROC_API_RESULT_WAIT;
    }
}

// ==========================================================================
//
// Wash Disk API
//
//===========================================================================

/****************************************************************************
*
* Function Name:    TestProc_Api_WashInit
* Input:            None
*
* Output:           None
*                   
* Returns:          
* Description:      Wash Disk Data Initialization
* Note:             
***************************************************************************/
__INLINE static void TestProc_Api_WashInit(void)
{
    TSC_DishWashAndLiquidAMixResultDataInit();
}

/****************************************************************************
*
* Function Name:    TestProc_Api_WashDiskReset
* Input:            None
*
* Output:           None
*                   
* Returns:          
* Description:      Wash Disk Reset
* Note:             
***************************************************************************/
__INLINE static LH_ERR TestProc_Api_WashDiskReset(void)
{
    LH_ERR eResult;
    
    eResult = TSC_DishWashInitWhileAck();
    
    return eResult;
}

/****************************************************************************
*
* Function Name:    TestProc_Api_WashDiskMoveToPos
* Input:            u8Pos - Target Position
*
* Output:           None
*                   
* Returns:          
* Description:      Rotate To The Target Position
* Note:             
***************************************************************************/
__INLINE static LH_ERR TestProc_Api_WashDiskMoveToPos(uint8_t u8Pos)
{
    LH_ERR eResult;
    
    eResult = TSC_DishWashMove2SpecialHoleWhileAck(u8Pos);
    
    return eResult;
}

/****************************************************************************
*
* Function Name:    TestProc_Api_WashDiskMoveToPos
* Input:            u8Pos - Target Position
*
* Output:           None
*                   
* Returns:          
* Description:      Rotate To The Target Position
* Note:             
***************************************************************************/
__INLINE static LH_ERR TestProc_Api_WashDiskRotate1Pos(void)
{
    LH_ERR eResult;
    
    eResult = TSC_DishWashMoveAnyHoleStepWhileAck(1);
    
    return eResult;
}

/****************************************************************************
*
* Function Name:    TestProc_Api_WashDiskMixReset
* Input:            None
*
* Output:           None
*                   
* Returns:          
* Description:      Wash Disk Reset
* Note:             
***************************************************************************/
__INLINE static LH_ERR TestProc_Api_WashDiskMixReset(void)
{
    LH_ERR eResult;
    
    eResult = TSC_LiquidAMixInitWhileAck();
    
    return eResult;
}

/****************************************************************************
*
* Function Name:    TestProc_Api_WashDiskMixAction
* Input:            u16Ms - Millisecond
*
* Output:           None
*                   
* Returns:          
* Description:      The Mix In Wash Disk
* Note:             
***************************************************************************/
__INLINE static LH_ERR TestProc_Api_WashDiskMixAction(uint16_t u16Ms)
{
    LH_ERR eResult;
    
    eResult = TSC_LiquidAMixRotateAnyTimeMsWhileAck(u16Ms);
    
    return eResult;
}

/****************************************************************************
*
* Function Name:    TestProc_Api_WashDiskIsCupIn
* Input:            None
*
* Output:           None
*                   
* Returns:          
* Description:      
* Note:             
***************************************************************************/
__INLINE static uint8_t TestProc_Api_WashDiskIsCupIn(void)
{
    return TSC_DishWashCheckCurrentHoleHasCup() == ACTION_SENSOR_STATUS_EXIST ? 1 : 0;
}

/****************************************************************************
*
* Function Name:    TestProc_Api_WashDiskCheckResult
* Input:            None
*
* Output:           None
*                   
* Returns:          
* Description:      Check Result
* Note:             
***************************************************************************/
__INLINE static TESTPROC_API_RESULT_E TestProc_Api_WashDiskCheckResult(LH_ERR *pErr)
{
    TSC_CMD_STATE eResult;
    
    eResult = TSC_DishWashAndLiquidAMixGetLastCommandStatusAndResult(pErr);
    if (TSC_CMD_STATE_IDLE == eResult || TSC_CMD_STATE_COMPLETE == eResult)
    {
        if (LH_ERR_NONE == *pErr)
        {
            return TESTPROC_API_RESULT_OK;
        }
        else
        {
            return TESTPROC_API_RESULT_NG;
        }
    }
    else
    {
        return TESTPROC_API_RESULT_WAIT;
    }
}

// Liquid A Inject
/****************************************************************************
*
* Function Name:    TestProc_Api_WashLiquidAInit
* Input:            None
*
* Output:           None
*                   
* Returns:          
* Description:      Wash Liquid A Inject Data Initialization
* Note:             
***************************************************************************/
__INLINE static void TestProc_Api_WashLiquidAInit(void)
{
    TSC_LiquidInjectAResultDataInit();
}

/****************************************************************************
*
* Function Name:    TestProc_Api_WashLiquidAReset
* Input:            None
*
* Output:           None
*                   
* Returns:          
* Description:      Wash Liquid A Inject Reset
* Note:             
***************************************************************************/
__INLINE static LH_ERR TestProc_Api_WashLiquidAReset(void)
{
    LH_ERR eResult;
    
    eResult = TSC_LiquidInjectAInitWhileAck();
    
    return eResult;
}

/****************************************************************************
*
* Function Name:    TestProc_Api_WashLiquidAPrime
* Input:            None
*
* Output:           None
*                   
* Returns:          
* Description:      
* Note:             
***************************************************************************/
__INLINE static LH_ERR TestProc_Api_WashLiquidAPrime(void)
{
    LH_ERR eResult;
    
    eResult = TSC_LiquidInjectAPrimeOnceWhileAck();
    
    return eResult;
}

/****************************************************************************
*
* Function Name:    TestProc_Api_WashLiquidAInject
* Input:            None
*
* Output:           None
*                   
* Returns:          
* Description:      
* Note:             
***************************************************************************/
__INLINE static LH_ERR TestProc_Api_WashLiquidAInject(void)
{
    LH_ERR eResult;
    
    eResult = TSC_LiquidInjectAInjectOnceWhileAck();
    
    return eResult;
}

/****************************************************************************
*
* Function Name:    TestProc_Api_WashLiquidACheckResult
* Input:            None
*
* Output:           None
*                   
* Returns:          
* Description:      Check Result
* Note:             
***************************************************************************/
__INLINE static TESTPROC_API_RESULT_E TestProc_Api_WashLiquidACheckResult(LH_ERR *pErr)
{
    TSC_CMD_STATE eResult;
    
    eResult = TSC_LiquidInjectAGetLastCommandStatusAndResult(pErr);
    if (TSC_CMD_STATE_IDLE == eResult || TSC_CMD_STATE_COMPLETE == eResult)
    {
        if (LH_ERR_NONE == *pErr)
        {
            return TESTPROC_API_RESULT_OK;
        }
        else
        {
            return TESTPROC_API_RESULT_NG;
        }
    }
    else
    {
        return TESTPROC_API_RESULT_WAIT;
    }
}

// Measure Room
/****************************************************************************
*
* Function Name:    TestProc_Api_WashMeasureRoomInit
* Input:            None
*
* Output:           None
*                   
* Returns:          
* Description:      Wash Measure Room Data Initialization
* Note:             
***************************************************************************/
__INLINE static void TestProc_Api_WashMeasureRoomInit(void)
{
    TSC_MeasureRoomResultDataInit();
}

/****************************************************************************
*
* Function Name:    TestProc_Api_WashMeasureRoomReset
* Input:            None
*
* Output:           None
*                   
* Returns:          
* Description:      Wash Measure Room Reset
* Note:             
***************************************************************************/
__INLINE static LH_ERR TestProc_Api_WashMeasureRoomReset(void)
{
    LH_ERR eResult;
    
    eResult = TSC_MeasureRoomInitWhileAck();
    
    return eResult;
}

/****************************************************************************
*
* Function Name:    TestProc_Api_WashMeasureRoomDoorOperation
* Input:            eOP - Open,Half Open,Close
*
* Output:           None
*                   
* Returns:          
* Description:      
* Note:             
***************************************************************************/
__INLINE static LH_ERR TestProc_Api_WashMeasureRoomDoorOperation(TESTPROC_API_DOOR_OP eOP)
{
    LH_ERR eResult = LH_ERR_ACTION_PARAM;
    
    if (TESTPROC_API_DOOR_OP_FULLOPEN == eOP)
    {
        eResult = TSC_MeasureRoomUpdoorOpenFullWhileAck();
    }
    else if (TESTPROC_API_DOOR_OP_HALFOPEN == eOP)
    {
        eResult = TSC_MeasureRoomUpDoorOpenHalfWhileAck();
    }
    else if (TESTPROC_API_DOOR_OP_CLOSE == eOP)
    {
        eResult = TSC_MeasureRoomUpDoorCloseWhileAck();
    }
    
    return eResult;
}
/****************************************************************************
*
* Function Name:    TestProc_Api_WashMeasureRoomWindowsClose
* Input:            none
*
* Output:           None
*                   
* Returns:          
* Description:      
* Note:             测量室窗户关闭
***************************************************************************/
__INLINE static LH_ERR TestProc_Api_WashMeasureRoomWindowsClose(void)
{
    LH_ERR eResult;

    eResult = TSC_MeasureRoomWindowsCloseWhileAck();

    return eResult;
}

/****************************************************************************
*
* Function Name:    TestProc_Api_WashMeasureRoomPrime
* Input:            None
*
* Output:           None
*                   
* Returns:          
* Description:      
* Note:             
***************************************************************************/
__INLINE static LH_ERR TestProc_Api_WashMeasureRoomPrime(void)
{
    LH_ERR eResult;
    
    eResult = TSC_MeasureRoomPrimeOnceWhileAck();
    
    return eResult;
}

/****************************************************************************
*
* Function Name:    TestProc_Api_WashMeasureRoomInject
* Input:            None
*
* Output:           None
*                   
* Returns:          
* Description:      
* Note:             
***************************************************************************/
__INLINE static LH_ERR TestProc_Api_WashMeasureRoomInject(void)
{
    LH_ERR eResult;
    
    eResult = TSC_MeasureRoomInjectOnceWhileAck();
    
    return eResult;
}

/****************************************************************************
*
* Function Name:    TestProc_Api_WashMeasureRoomInject
* Input:            MEASURE_VALUE_FLAG 测量类型
*
* Output:           None
*                   
* Returns:          
* Description:      
* Note:             
***************************************************************************/
__INLINE static LH_ERR TestProc_Api_WashMeasureRoomMakeValue(MEASURE_VALUE_FLAG u8LiquidInject, uint16_t u16Ms)
{
    LH_ERR eResult;

    eResult = TSC_MeasureRoomWindowsMeasureBackGroundAndValue(u16Ms, u8LiquidInject);
    
    return eResult;
}
/****************************************************************************
*
* Function Name:    TestProc_Api_WashMeasureRoomReadValue
* Input:            None
*
* Output:           None
*                   
* Returns:          
* Description:      
* Note:             
***************************************************************************/
__INLINE static uint32_t TestProc_Api_WashMeasureRoomReadValue(MEASURE_VALUE_FLAG injectFlag)
{
    uint32_t u32Result;

    if(injectFlag == MEASURE_FLAG_READ)                     //测光值
    {
        u32Result = TSC_MeasureRoomReadLastLightResult();
    }
    else if(injectFlag == MEASURE_FLAG_DARK)                //暗计数
    {
        u32Result = TSC_MeasureRoomReadLastDarkResult();
    }
    else if(injectFlag == MEASURE_FLAG_BACK)                //本底值
    {
        u32Result = TSC_MeasureRoomReadLastGroundResult();
    }

    return u32Result;
}

/****************************************************************************
*
* Function Name:    TestProc_Api_WashMeasureRoomCheckResult
* Input:            None
*
* Output:           None
*                   
* Returns:          
* Description:      Check Result
* Note:             
***************************************************************************/
__INLINE static TESTPROC_API_RESULT_E TestProc_Api_WashMeasureRoomCheckResult(LH_ERR *pErr)
{
    TSC_CMD_STATE eResult;
    
    eResult = TSC_MeasureRoomGetLastCommandStatusAndResult(pErr);
    if (TSC_CMD_STATE_IDLE == eResult || TSC_CMD_STATE_COMPLETE == eResult)
    {
        if (LH_ERR_NONE == *pErr)
        {
            return TESTPROC_API_RESULT_OK;
        }
        else
        {
            return TESTPROC_API_RESULT_NG;
        }
    }
    else
    {
        return TESTPROC_API_RESULT_WAIT;
    }
}

// Wash Needle
/****************************************************************************
*
* Function Name:    TestProc_Api_WashNeedleInit
* Input:            None
*
* Output:           None
*                   
* Returns:          
* Description:      Wash Needle Data Initialization
* Note:             
***************************************************************************/
__INLINE static void TestProc_Api_WashNeedleInit(void)
{
    TSC_NeedleWashResultDataInit();
}

/****************************************************************************
*
* Function Name:    TestProc_Api_WashNeedleReset
* Input:            None
*
* Output:           None
*                   
* Returns:          
* Description:      Wash Needle Reset
* Note:             
***************************************************************************/
__INLINE static LH_ERR TestProc_Api_WashNeedleReset(void)
{
    LH_ERR eResult;
    
    eResult = TSC_NeedleWashInitWhileAck();
    
    return eResult;
}

/****************************************************************************
*
* Function Name:    TestProc_Api_WashNeedlePrime
* Input:            None
*
* Output:           None
*                   
* Returns:          
* Description:      
* Note:             
***************************************************************************/
__INLINE static LH_ERR TestProc_Api_WashNeedlePrime(void)
{
    LH_ERR eResult;
    
    eResult = TSC_NeedleWashPrimeOnceWhileAck();
    
    return eResult;
}

/****************************************************************************
*
* Function Name:    TestProc_Api_WashNeedleClean
* Input:            TSC_NEEDLE_WASH_SELECT_FLAG 选中清洗针
*                   CLEAN_NEEDLE_INDEX      第几次清洗
* Output:           None
*                   
* Returns:          
* Description:      
* Note:             
***************************************************************************/
__INLINE static LH_ERR TestProc_Api_WashNeedleClean(TSC_NEEDLE_WASH_SELECT_FLAG select_flag, CLEAN_NEEDLE_INDEX clean_index)
{
    LH_ERR eResult;
    
    eResult = TSC_NeedleWashCleanOnceWhileAck(select_flag, clean_index);
    
    return eResult;
}

/****************************************************************************
*
* Function Name:    TestProc_Api_WashNeedleCheckResult
* Input:            None
*
* Output:           None
*                   
* Returns:          
* Description:      Check Result
* Note:             
***************************************************************************/
__INLINE static TESTPROC_API_RESULT_E TestProc_Api_WashNeedleCheckResult(LH_ERR *pErr)
{
    TSC_CMD_STATE eResult;
    
    eResult = TSC_NeedleWashGetLastCommandStatusAndResult(pErr);
    if (TSC_CMD_STATE_IDLE == eResult || TSC_CMD_STATE_COMPLETE == eResult)
    {
        if (LH_ERR_NONE == *pErr)
        {
            return TESTPROC_API_RESULT_OK;
        }
        else
        {
            return TESTPROC_API_RESULT_NG;
        }
    }
    else
    {
        return TESTPROC_API_RESULT_WAIT;
    }
}

// ==========================================================================
//
// Hand Cup API
//
//===========================================================================

/****************************************************************************
*
* Function Name:    TestProc_Api_HandInit
* Input:            None
*
* Output:           None
*                   
* Returns:          
* Description:      Had Cup Data Initialization
* Note:             
***************************************************************************/
__INLINE static void TestProc_Api_HandCupInit(void)
{
    TSC_HandCupResultDataInit();
}

/****************************************************************************
*
* Function Name:    TestProc_Api_HandCupReset
* Input:            None
*
* Output:           None
*                   
* Returns:          
* Description:      Hand Cup Reset
* Note:             
***************************************************************************/
__INLINE static LH_ERR TestProc_Api_HandCupReset(void)
{
    LH_ERR eResult;
    
    eResult = TSC_HandCupInitWhileAck();
    
    return eResult;
}

/****************************************************************************
*
* Function Name:    TestProc_Api_HandCupReset
* Input:            eTarget - Target
*
* Output:           None
*                   
* Returns:          
* Description:      
* Note:             
***************************************************************************/
__INLINE static LH_ERR TestProc_Api_HandCupMoveToTop(TESTPROC_API_HAND_TARGET eTarget)
{
    LH_ERR eResult = LH_ERR_ACTION_PARAM;
    
    if (TESTPROC_API_HAND_TARGET_REACT == eTarget)
    {
        eResult = TSC_HandCupMove2DishReactionWhileAck();
    }
    else if (TESTPROC_API_HAND_TARGET_WASH == eTarget)
    {
        eResult = TSC_HandCupMove2DishWashWhileAck();
    }
    else if (TESTPROC_API_HAND_TARGET_MEASUREROOM == eTarget)
    {
        eResult = TSC_HandCupMove2MeasureRoomWhileAck();
    }
    
    return eResult;
}

/****************************************************************************
*
* Function Name:    TestProc_Api_HandCupCatchCup
* Input:            eTarget - Target
*
* Output:           None
*                   
* Returns:          
* Description:      
* Note:             
***************************************************************************/
__INLINE static LH_ERR TestProc_Api_HandCupCatchCup(TESTPROC_API_HAND_TARGET eTarget)
{
    LH_ERR eResult = LH_ERR_ACTION_PARAM;
    
    if (TESTPROC_API_HAND_TARGET_REACT == eTarget)
    {
        eResult = TSC_HandCupCatchCupFromDishReactionWhileAck();
    }
    else if (TESTPROC_API_HAND_TARGET_WASH == eTarget)
    {
        eResult = TSC_HandCupCatchFromDishWashWhileAck();
    }
    else if (TESTPROC_API_HAND_TARGET_MEASUREROOM == eTarget)
    {
        eResult = TSC_HandCupCatchFromMeasureRoomWhileAck();
    }
    else if (TESTPROC_API_HAND_TARGET_NEWDISK == eTarget)
    {
        eResult = TSC_HandCupCatchCupFromNewDiskWhileAck();
    }
    
    return eResult;
}

/****************************************************************************
*
* Function Name:    TestProc_Api_HandCupPutCup
* Input:            eTarget - Target
*
* Output:           None
*                   
* Returns:          
* Description:      
* Note:             
***************************************************************************/
__INLINE static LH_ERR TestProc_Api_HandCupPutCup(TESTPROC_API_HAND_TARGET eTarget)
{
    LH_ERR eResult = LH_ERR_ACTION_PARAM;
    
    if (TESTPROC_API_HAND_TARGET_REACT == eTarget)
    {
        eResult = TSC_HandCupPutCup2DishReactionWhileAck();
    }
    else if (TESTPROC_API_HAND_TARGET_WASH == eTarget)
    {
        eResult = TSC_HandCupPutCup2DishWashWhileAck();
    }
    else if (TESTPROC_API_HAND_TARGET_MEASUREROOM == eTarget)
    {
        eResult = TSC_HandCupPutCup2MeasureRoomWhileAck();
    }
    
    return eResult;
}
/****************************************************************************
*
* Function Name:    TestProc_Api_HandCupMoveToNewDisk
* Input:            eTarget - Target
*
* Output:           None
*                   
* Returns:          
* Description:      
* Note:             
***************************************************************************/
__INLINE static LH_ERR TestProc_Api_HandCupMoveToNewDisk(uint8_t u8CupRow, uint8_t u8CupCol)
{
    LH_ERR eResult;
    
    eResult = TSC_HandCupMove2NewDiskWhileAck(u8CupRow, u8CupCol);
    
    return eResult;
}

/****************************************************************************
*
* Function Name:    TestProc_Api_HandCupMoveToGarbage
* Input:            eTarget - No1 Or No2
*
* Output:           None
*                   
* Returns:          
* Description:      
* Note:             
***************************************************************************/
__INLINE static LH_ERR TestProc_Api_HandCupMoveToGarbage(ACTION_PARAM_STACK_MANAGE_GARBAGE_NO eTarget)
{
    LH_ERR eResult;
    
    eResult = TSC_HandCupMove2GarbageWhileAck(eTarget);
    
    return eResult;
}

/****************************************************************************
*
* Function Name:    TestProc_Api_HandCupPutCupToGarbage
* Input:            eTarget - No1 Or No2
*
* Output:           None
*                   
* Returns:          
* Description:      
* Note:             
***************************************************************************/
__INLINE static LH_ERR TestProc_Api_HandCupPutCupToGarbage(ACTION_PARAM_STACK_MANAGE_GARBAGE_NO eTarget)
{
    LH_ERR eResult;
    
    eResult = TSC_HandCupPutCup2GarbageWhileAck(eTarget);
    
    return eResult;
}

/****************************************************************************
*
* Function Name:    TestProc_Api_HandCupIsCupInReact
* Input:            None
*
* Output:           None
*                   
* Returns:          
* Description:      
* Note:             
***************************************************************************/
__INLINE static uint8_t TestProc_Api_HandCupIsCupInReact(void)
{
    return TSC_HandCheckDishReactionHasCup() == ACTION_SENSOR_STATUS_EXIST ? 1 : 0;
}

/****************************************************************************
*
* Function Name:    TestProc_Api_HandCupIsCupInReact
* Input:            None
*
* Output:           None
*                   
* Returns:          
* Description:      
* Note:             
***************************************************************************/
__INLINE static uint8_t TestProc_Api_HandCupIsCupInWash(void)
{
    return TSC_HandCheckDishWashHasCup() == ACTION_SENSOR_STATUS_EXIST ? 1 : 0;
}

/****************************************************************************
*
* Function Name:    TestProc_Api_WashNeedleCheckResult
* Input:            None
*
* Output:           None
*                   
* Returns:          
* Description:      Check Result
* Note:             
***************************************************************************/
__INLINE static TESTPROC_API_RESULT_E TestProc_Api_HandCupCheckResult(LH_ERR *pErr)
{
    TSC_CMD_STATE eResult;
    
    eResult = TSC_HandCupGetLastCommandStatusAndResult(pErr);
    if (TSC_CMD_STATE_IDLE == eResult || TSC_CMD_STATE_COMPLETE == eResult)
    {
        if (LH_ERR_NONE == *pErr)
        {
            return TESTPROC_API_RESULT_OK;
        }
        else
        {
            return TESTPROC_API_RESULT_NG;
        }
    }
    else
    {
        return TESTPROC_API_RESULT_WAIT;
    }
}

// ==========================================================================
//
// Cup Stack API
//
//===========================================================================
/****************************************************************************
*
* Function Name:    TestProc_Api_CupStackInit
* Input:            None
*
* Output:           None
*                   
* Returns:          
* Description:      Cup Stack Data Initialization
* Note:             
***************************************************************************/
__INLINE static void TestProc_Api_CupStackInit(void)
{
    TSC_StackManageResultDataInit();
}

/****************************************************************************
*
* Function Name:    TestProc_Api_CupStackReset
* Input:            None
*
* Output:           None
*                   
* Returns:          
* Description:      Cup Stack Reset
* Note:             
***************************************************************************/
__INLINE static LH_ERR TestProc_Api_CupStackReset(void)
{
    LH_ERR eResult;
    
    eResult = TSC_StackManageInitWhileAck();
    
    return eResult;
}

/****************************************************************************
*
* Function Name:    TestProc_Api_CupStackRecycleDisk
* Input:            None
*
* Output:           None
*                   
* Returns:          
* Description:      Pop One Cup Disk
* Note:             
***************************************************************************/
__INLINE static LH_ERR TestProc_Api_CupStackRecycleDisk(void)
{
    LH_ERR eResult;
    
    eResult = TSC_StackManageAscendingEmptyDiskFromWaitTestPlatformWhileAck();
    
    return eResult;
}

/****************************************************************************
*
* Function Name:    TestProc_Api_CupStackUploadDisk
* Input:            None
*
* Output:           None
*                   
* Returns:          
* Description:      Pop One Cup Disk
* Note:             
***************************************************************************/
__INLINE static LH_ERR TestProc_Api_CupStackUploadDisk(void)
{
    LH_ERR eResult;
    
    eResult = TSC_StackManageUploadNewDisk2NewDiskPlatformWhileAck ();
    
    return eResult;
}

/****************************************************************************
*
* Function Name:    TestProc_Api_CupStackGarbageLock
* Input:            None
*
* Output:           None
*                   
* Returns:          
* Description:      Lock The Garbage And Push Up
* Note:             
***************************************************************************/
__INLINE static LH_ERR TestProc_Api_CupStackGarbageLock(ACTION_PARAM_STACK_MANAGE_GARBAGE_NO eNo)
{
    LH_ERR eResult;
    
    eResult = TSC_StackManageLockGarbageWhileAck(eNo);
    
    return eResult;
}

/****************************************************************************
*
* Function Name:    TestProc_Api_CupStackGarbageUnlock
* Input:            None
*
* Output:           None
*                   
* Returns:          
* Description:      Unlock The Garbage And Push Down
* Note:             
***************************************************************************/
__INLINE static LH_ERR TestProc_Api_CupStackGarbageUnlock(ACTION_PARAM_STACK_MANAGE_GARBAGE_NO eNo)
{
    LH_ERR eResult;
    
    eResult = TSC_StackManageUnlockGarbageWhileAck(eNo);
    
    return eResult;
}

/****************************************************************************
*
* Function Name:    TestProc_Api_CupStackDoorControl
* Input:            电磁开/关
*
* Output:           None
*                   
* Returns:          
* Description:      控制电磁通电/断电
* Note:             
***************************************************************************/


__INLINE static LH_ERR TestProc_Api_CupStackDoorControl(MACHINE_DOOR_CMD keycmd)
{
    LH_ERR eResult;
    eResult = TSC_StackManageDoorControlWhileAck(keycmd);

    return eResult;
}

/****************************************************************************
*
* Function Name:    TestProc_Api_CupStackNewDiskIsExist
* Input:            None
*
* Output:           None
*                   
* Returns:           
* Description:      
* Note:             Do Not Need Check Result
***************************************************************************/
__INLINE static ACTION_SENSOR_STATUS TestProc_Api_CupStackNewDiskIsExist(void)
{
    
    return TSC_StackManageCheckNewDiskPlatformHasDisk();
}

/****************************************************************************
*
* Function Name:    TestProc_Api_CupStackWaitDiskIsExist
* Input:            None
*
* Output:           None
*                   
* Returns:           
* Description:      
* Note:             Do Not Need Check Result
***************************************************************************/
__INLINE static ACTION_SENSOR_STATUS TestProc_Api_CupStackWaitDiskIsExist(void)
{
    
    return TSC_StackManageCheckWaitTestDiskPlatformHasDisk();
}

/****************************************************************************
*
* Function Name:    TestProc_Api_CupStackGarbage1IsExist
* Input:            None
*
* Output:           None
*                   
* Returns:           
* Description:      
* Note:             Do Not Need Check Result
***************************************************************************/
__INLINE static ACTION_SENSOR_STATUS TestProc_Api_CupStackGarbage1IsExist(void)
{
    
    return TSC_StackManageCheckGarbage1();
}

/****************************************************************************
*
* Function Name:    TestProc_Api_CupStackGarbage2IsExist
* Input:            None
*
* Output:           None
*                   
* Returns:           
* Description:      
* Note:             Do Not Need Check Result
***************************************************************************/
__INLINE static ACTION_SENSOR_STATUS TestProc_Api_CupStackGarbage2IsExist(void)
{
    
    return TSC_StackManageCheckGarbage2();
}

/****************************************************************************
*
* Function Name:    TestProc_Api_CupStackPushToTestPostion
* Input:            None
*
* Output:           None
*                   
* Returns:          
* Description:      Set Cup Stact In Postion
* Note:             
***************************************************************************/
__INLINE static LH_ERR TestProc_Api_CupStackPushToTestPostion(void)
{
    LH_ERR eResult;
    
    eResult = TSC_StackManagePushNewDisk2WaitTestPlatformWhileAck();
    
    return eResult;
}

/****************************************************************************
*
* Function Name:    TestProc_Api_CupStackNewDiskCount
* Input:            u8NewDisk - 1, New Disk Count
*                               0, Empty Disk Count
*
* Output:           None
*                   
* Returns:           
* Description:      
* Note:             Do Not Need Check Result
***************************************************************************/
__INLINE static uint8_t TestProc_Api_CupStackDiskCount(uint8_t u8NewDisk)
{
    
    if (1 == u8NewDisk)
    {
        return TSC_StackManageCheckNewDiskCount();
    }
    else
    {
        return TSC_StackManageChecEmptyDiskCount();
    }
}

/****************************************************************************
*
* Function Name:    TestProc_Api_CupStackGetDoorStatus
* Input:            None
*
* Output:           None
*                   
* Returns:          Door Status
* Description:      
* Note:             Do Not Need Check Result
***************************************************************************/
__INLINE static MACHINE_DOOR_STATUS TestProc_Api_CupStackGetDoorStatus(void)
{
    return TSC_StackManageCheckDoorStatus();
}

/****************************************************************************
*
* Function Name:    TestProc_Api_IsCupStackDoorClose
* Input:            None
*
* Output:           None
*                   
* Returns:          1 - Door Close; 0 - Door Open Or Wait Opt
* Description:      
* Note:             
***************************************************************************/
__INLINE static uint8_t TestProc_Api_IsCupStackDoorClose(void)
{
    return TSC_StackManageCheckDoorStatus() == MACHINE_DOOR_STATUS_CLOSE ? 1 : 0;
}

/****************************************************************************
*
* Function Name:    TestProc_Api_CupStackIsWaitReady
* Input:            None
*
* Output:           None
*                   
* Returns:          1 - 待测平台有杯盘; 0 - 待测平台无杯盘
* Description:      
* Note:             
***************************************************************************/
__INLINE static uint8_t TestProc_Api_CupStackIsWaitReady(void)
{
    return TSC_StackManageCheckWaitTestDiskPlatformHasDisk() == ACTION_SENSOR_STATUS_EXIST ? 1 : 0;
}

/****************************************************************************
*
* Function Name:    TestProc_Api_CupStackIsReady
* Input:            None
*
* Output:           None
*                   
* Returns:          1 - 装载平台有杯盘; 0 - 装载平台无杯盘
* Description:      
* Note:             
***************************************************************************/
__INLINE static uint8_t TestProc_Api_CupStackIsReady(void)
{
    return TSC_StackManageCheckNewDiskPlatformHasDisk() == ACTION_SENSOR_STATUS_EXIST ? 1 : 0;
}

/****************************************************************************
*
* Function Name:    TestProc_Api_CupStackCheckResult
* Input:            None
*
* Output:           None
*                   
* Returns:          
* Description:      Check Result
* Note:             
***************************************************************************/
__INLINE static TESTPROC_API_RESULT_E TestProc_Api_CupStackCheckResult(LH_ERR *pErr)
{
    TSC_CMD_STATE eResult;
    
    eResult = TSC_StackManageGetLastCommandStatusAndResult(pErr);
    if (TSC_CMD_STATE_IDLE == eResult || TSC_CMD_STATE_COMPLETE == eResult)
    {
        if (LH_ERR_NONE == *pErr)
        {
            return TESTPROC_API_RESULT_OK;
        }
        else
        {
            return TESTPROC_API_RESULT_NG;
        }
    }
    else
    {
        return TESTPROC_API_RESULT_WAIT;
    }
}
/***************************************************************************
 * 
 *  Function Name:    TestModeSelection
 *  Input:            TSC_TEST_MODE_SELECTION
 *  Output:           None
 *  Returns:          
 *  Description:        配置测试模式
 * *************************************************************************/    
__INLINE static void TestModeSelection(TSC_TEST_MODE_SELECTION mode)
{
    TSC_TestMode_Selection(mode);
}


#ifdef __cplusplus
}
#endif

#endif // End of “ifndef __TESTPROC_API_H_�?
