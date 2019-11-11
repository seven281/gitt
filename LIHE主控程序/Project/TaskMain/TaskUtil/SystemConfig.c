/******************************************************************
 * (c) Copyright 2018-2025, XXXX, All Rights Reserved.
 * THIS IS UNPUBLISHED PROPRIETARY SOURCE CODE OF XXXX, INC.
 * The copyright notice above does not evidence any actual or intended
 * publication of such source code. 
 *
 *  Subsystem:   System
 *  File:        SystemConfig.c
 *  Author:      王殿光
 *  Description: 
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
#include "TaskServiceConfig.h"
#include "TestProc_Typedef.h"
#include "MCU_internalFlash.h"
#include "SystemCRC.h"
#include "SystemConfig.h"
#include "MCU_Uart1.h"

#define SOH                                     (0x7E)
#define EOH                                     (0x0D)
#define ADDR                                    (0xFF)
#define HEAD_SIZE                               (4u)        // 包头长度,不包含SOH

typedef struct
{
    uint32_t u32NewFlashFlag;
    uint8_t  u8CellNumber;  // 0~3
    uint8_t  u8ReportTime;
    uint8_t  u8CellType;
    uint8_t  u8BoardType;   // 电路板类型
    uint8_t  u8Invalid[56];
}SYSTEMCONFIG_CELL_T;

static SYSTEMCONFIG_CELL_T g_stCellConfig;
static uint8_t  g_u8UartRxBuf[512];
static uint16_t  g_u16UartRxIndex = 0;
static uint16_t  g_u16UartDataSize = 0;
static uint16_t  g_u16CheckCountPerSec = 0;  // 1s计时器
static uint8_t  g_u8UartRxState = 0;

/************************AD协议******************************************************
SOH    ADDR    CMD    LEN_L    LEN_H    DATA0  ……  DATAn    CRC_L    CRC_H    EOH

SOH    帧起始符，固定数值0x7E
ADDR   设备地址，RS232通信采用固定值0xFF
CMD	   通信命令
LEN_L  命令体长度，即DATA0~DATAn的长度
LEN_H  命令体长度，即DATA0~DATAn的长度
CRC_L  unsigned short类型数据的低字节
CRC_H  unsigned short类型数据的高字节
EOH	   帧结束符，固定数值0x0D
************************************************************************************/

/****************************************************************************
 *
 * Function Name:    SystemConfig_SendCmd
 * Input:            pu8Data - 发送数据缓冲区
 *                   u16DataSize - 缓冲区长度
 *                   
 * Output:           None
 *                   
 * Returns:          0成功发送;-1参数错误;-2内存不足
 * Description:      
 * Note:             
 ***************************************************************************/
int32_t SystemConfig_SendCmd(uint8_t u8Cmd, const uint8_t *pu8Data, uint16_t u16DataSize)
{
    uint8_t  *pu8TxBuf;

    if (NULL == pu8Data || 0 == u16DataSize)
    {
        return -1;
    }
    pu8TxBuf = UserMemMalloc(SRAM_IS62, u16DataSize+8);
    if (pu8TxBuf != NULL)
    {
        uint16_t u16Index = 0;
        pu8TxBuf[u16Index++] = SOH;
        pu8TxBuf[u16Index++] = ADDR;
        pu8TxBuf[u16Index++] = u8Cmd;
        pu8TxBuf[u16Index++] = (uint8_t)(u16DataSize&0xFF);
        pu8TxBuf[u16Index++] = (uint8_t)((u16DataSize>>8)&0xFF);
        Mem_Copy(&pu8TxBuf[5], pu8Data, u16DataSize);
        u16Index += u16DataSize;
        uint16_t u16CalcCRC = SystemCRC_calcCRC16(&pu8TxBuf[1], u16DataSize+4);        
        pu8TxBuf[u16Index++] = (uint8_t)(u16CalcCRC&0xFF);
        pu8TxBuf[u16Index++] = (uint8_t)((u16CalcCRC>>8)&0xFF);
        pu8TxBuf[u16Index++] = EOH;
        MCU_Uart1SendBuffer(pu8TxBuf, u16Index);
        return 0;
    }
    else
    {
        return -2;
    }
}

void SystemConfig_IsrCallBack(uint8_t u8Char)
{
    //MCU_Uart1Printf(" >> %d,%d,%d,%02X\r\n", g_u8UartRxState, g_u16UartDataSize, g_u16UartRxIndex, u8Char);
    switch(g_u8UartRxState)
    {
        case 0:  // Find SOH
            if (SOH == u8Char)
            {
                g_u8UartRxState = 1;
                g_u16UartRxIndex = 0;
                g_u16CheckCountPerSec = 0;
            }
            break;
        case 1: // check Head
            g_u8UartRxBuf[g_u16UartRxIndex++] = u8Char;
            if (g_u16UartRxIndex >= HEAD_SIZE)
            {
                g_u8UartRxState = 2;
                g_u16UartDataSize = ((uint16_t)g_u8UartRxBuf[3]<<8)|g_u8UartRxBuf[2];
            }
            break;
        case 2: // Check Length
            g_u8UartRxBuf[g_u16UartRxIndex++] = u8Char;
            if (g_u16UartRxIndex >= (g_u16UartDataSize+7))
            {
                uint16_t u16CalcCRC = (uint16_t)(((uint16_t)g_u8UartRxBuf[g_u16UartDataSize+5]<<8)|g_u8UartRxBuf[g_u16UartDataSize+4]);                
                //MCU_Uart1Printf("Uart1 Cmd:%d,%04X\r\n", g_u16UartDataSize, u16CalcCRC);
                //for (int i=0; i<(g_u16UartDataSize+7); i++) MCU_Uart1Printf("%02X ", g_u8UartRxBuf[i]);
                //MCU_Uart1Printf("\r\n");
                g_u8UartRxState = 0;
                
                TESTPROC_MSM_T *pstMsg;
                OS_ERR err;
                pstMsg = UserMemMalloc(SRAM_IS62, sizeof(TESTPROC_MSM_T));
                if (pstMsg != NULL)
                {
                    MCU_Uart1Printf("Post Uart1 Msg\r\n");
                    SYSTEMCONFIG_CMD_T *pstCmd = UserMemMalloc(SRAM_IS62, sizeof(TESTPROC_MSM_T));
                    if (pstCmd != NULL)
                    {
                        if (g_u16UartDataSize > sizeof(pstCmd->u8Body))
                        {
                            pstCmd->u16BodySize = sizeof(pstCmd->u8Body);
                        }
                        else
                        {
                            pstCmd->u16BodySize = g_u16UartDataSize;
                        }
                        Mem_Copy(pstCmd->u8Body, &g_u8UartRxBuf[4], pstCmd->u16BodySize);
                        pstCmd->u16RecvCrc = u16CalcCRC;
                        pstCmd->u8Cmd = g_u8UartRxBuf[1];
                        pstMsg->pData = pstCmd;
                        pstMsg->u16DataSize = sizeof(TESTPROC_MSM_T);
                        pstMsg->eSrcUnit  = TESTPROC_UNIT_UART1;
                        pstMsg->eDestUnit = TESTPROC_UNIT_PROCTEST;
                        OSTaskQPost((OS_TCB *)&tcbTaskAppTestProcess, //向协议处理线程发送消息
                                    (void *)pstMsg,
                                    (OS_MSG_SIZE)sizeof(TESTPROC_MSM_T),
                                    (OS_OPT)OS_OPT_POST_FIFO,
                                    (OS_ERR *)&err);
                    }
                }
                else
                {
                    MCU_Uart1Printf("Uart1 No Memory\r\n");
                }
            }
            break;
        default:
            
            break;
    }
    
}

int32_t SystemConfig_SaveConfig(CONFIG_TYPE_E eType)
{
    uint8_t  *pu8BootConfigBuff;
    uint8_t  *pu8AppConfigBuff;
    int s32Result = 0;
    uint32_t crc32;
    int i;

    pu8BootConfigBuff = UserMemMalloc(SRAM_IS62,FLASH_BOOT_CONFIG_SIZE);
    if (NULL == pu8BootConfigBuff)
    {
        UserMemFree(SRAM_IS62, pu8BootConfigBuff);
        return -1;
    }
    pu8AppConfigBuff = UserMemMalloc(SRAM_IS62,FLASH_BOOT_CONFIG_SIZE);
    if (NULL == pu8AppConfigBuff)
    {
        UserMemFree(SRAM_IS62, pu8BootConfigBuff);
        UserMemFree(SRAM_IS62, pu8AppConfigBuff);
        return -2;
    }
    //Boot
    for (i=0; i<FLASH_BOOT_CONFIG_SIZE; i++)
    {
        pu8BootConfigBuff[i] = *(uint8_t *)(FLASH_BOOT_CONFIG_ADDR+i);
    }
    //App
    for (i=0; i<FLASH_APP_CONFIG_SIZE; i++)
    {
        pu8AppConfigBuff[i] = *(uint8_t *)(FLASH_APP_CONFIG_ADDR+i);
    }
    switch(eType)
    {
        case CONFIG_TYPE_NEW_APP:
            *(uint32_t *)&pu8BootConfigBuff[FLASH_BOOT_CONFIG_NEW_APP_ADDR-FLASH_BOOT_CONFIG_ADDR] = ~FLASH_PROGRAM_NEW_FUNC;
            break;
        case CONFIG_TYPE_APP_CONFIG:
            Mem_Copy(pu8AppConfigBuff, (uint8_t *)&g_stCellConfig, sizeof(SYSTEMCONFIG_CELL_T));
            break;
        default:
            s32Result = -3;
            break;
    }
    if (0 == s32Result)
    {
        MCU_FlashErase(FLASH_BOOT_CONFIG_ADDR, 1);
        
        crc32 = SystemCRC_calcCRC32(pu8BootConfigBuff, FLASH_BOOT_CONFIG_SIZE-4);
        pu8BootConfigBuff[FLASH_BOOT_CONFIG_SIZE-4] = WORD_TO_BYTE_LL(crc32);
        pu8BootConfigBuff[FLASH_BOOT_CONFIG_SIZE-3] = WORD_TO_BYTE_LH(crc32);
        pu8BootConfigBuff[FLASH_BOOT_CONFIG_SIZE-2] = WORD_TO_BYTE_HL(crc32);
        pu8BootConfigBuff[FLASH_BOOT_CONFIG_SIZE-1] = WORD_TO_BYTE_HH(crc32);
        MCU_FlashProgram(FLASH_BOOT_CONFIG_ADDR, pu8BootConfigBuff, FLASH_BOOT_CONFIG_SIZE);
        
        crc32 = SystemCRC_calcCRC32(pu8AppConfigBuff, FLASH_APP_CONFIG_SIZE-4);
        pu8AppConfigBuff[FLASH_APP_CONFIG_SIZE-4] = WORD_TO_BYTE_LL(crc32);
        pu8AppConfigBuff[FLASH_APP_CONFIG_SIZE-3] = WORD_TO_BYTE_LH(crc32);
        pu8AppConfigBuff[FLASH_APP_CONFIG_SIZE-2] = WORD_TO_BYTE_HL(crc32);
        pu8AppConfigBuff[FLASH_APP_CONFIG_SIZE-1] = WORD_TO_BYTE_HH(crc32);
        MCU_FlashProgram(FLASH_APP_CONFIG_ADDR, pu8AppConfigBuff, FLASH_APP_CONFIG_SIZE);
    }
    UserMemFree(SRAM_IS62, pu8BootConfigBuff);
    UserMemFree(SRAM_IS62, pu8AppConfigBuff);

    return s32Result;
}

/****************************************************************************
 *
 * Function Name:    SystemConfig_LoadConfig
 * Input:            None
 *                   
 * Output:           None
 *                   
 * Returns:          
 * Description:      
 * Note:             在CAN的初始化前被调用
 ***************************************************************************/
int32_t SystemConfig_LoadConfig(void)
{
    uint32_t u32ReadCRC, u32CalcCRC;

    Mem_Copy(&g_stCellConfig, (uint8_t *)FLASH_APP_CONFIG_ADDR, sizeof(SYSTEMCONFIG_CELL_T));
    
    if (FLASH_NEW_FLAG != g_stCellConfig.u32NewFlashFlag)
    {
        MCU_Uart1Printf("##New Board##\r\n");
        Mem_Clr(&g_stCellConfig, sizeof(SYSTEMCONFIG_CELL_T));
        g_stCellConfig.u32NewFlashFlag = FLASH_NEW_FLAG;
        g_stCellConfig.u8CellNumber = 0; // Cell 1
        g_stCellConfig.u8ReportTime = 5;
        g_stCellConfig.u8CellType = TESTPROC_CELL_TYPE_CI;
        g_stCellConfig.u8BoardType = 2;  // H200 MainBoard, STM32F407IGTx + 1MB SRAM + 2MB FRAM
        SystemConfig_SaveConfig(CONFIG_TYPE_APP_CONFIG);
        MCU_Uart1Printf("InitConfig:%d,%d,%d\r\n", g_stCellConfig.u8CellNumber, g_stCellConfig.u8ReportTime, g_stCellConfig.u8CellType);
    }
    
    u32CalcCRC = SystemCRC_calcCRC32((uint8_t *)FLASH_APP_CONFIG_ADDR, FLASH_BOOT_CONFIG_SIZE-4);
    u32ReadCRC = BYTE_TO_WORD(*(uint8_t *)(FLASH_APP_CONFIG_ADDR+FLASH_APP_CONFIG_SIZE-1),
                              *(uint8_t *)(FLASH_APP_CONFIG_ADDR+FLASH_APP_CONFIG_SIZE-2),
                              *(uint8_t *)(FLASH_APP_CONFIG_ADDR+FLASH_APP_CONFIG_SIZE-3),
                              *(uint8_t *)(FLASH_APP_CONFIG_ADDR+FLASH_APP_CONFIG_SIZE-4));
    
    if (u32ReadCRC != u32CalcCRC)
    {
        MCU_Uart1Printf("**ERR>LoadCellConfig_CRC:%04X,%04X\r\n", u32ReadCRC, u32CalcCRC);
        return -1;
    }
    else
    {
        MCU_Uart1Printf("CellConfig:%d,%d,%d\r\n", g_stCellConfig.u8CellNumber, g_stCellConfig.u8ReportTime, g_stCellConfig.u8CellType);
        return 0;
    }
}

/****************************************************************************
 *
 * Function Name:    SystemConfig_SetCellNumber
 * Input:            u8CellNumber - 模块号
 *                   
 * Output:           None
 *                   
 * Returns:          
 * Description:      
 * Note:             
 ***************************************************************************/
static int32_t SystemConfig_SetCellNumber(uint8_t u8CellNumber)
{
    g_stCellConfig.u8CellNumber = u8CellNumber;
    SystemConfig_SaveConfig(CONFIG_TYPE_APP_CONFIG);
    return 0;
}

/****************************************************************************
 *
 * Function Name:    SystemConfig_SetParam
 * Input:            u8SubID - 类别
 *                   pu8Data - 数据
 *                   u16DataSize - 数据长度
 *                   
 * Output:           None
 *                   
 * Returns:          
 * Description:      
 * Note:             
 ***************************************************************************/
int32_t SystemConfig_SetParam(uint8_t u8SubID, const uint8_t *pu8Data, uint16_t u16DataSize)
{
    int32_t  s32Result = 0;

    switch(u8SubID)
    {
        case 0x01:  // Set Cell Number
            s32Result = SystemConfig_SetCellNumber(pu8Data[0]);
            break;
        default:
            s32Result = -1;
            break;
    }

    return s32Result;
}

/****************************************************************************
 *
 * Function Name:    SystemConfig_GetCellNumber
 * Input:            None
 *                   
 * Output:           None
 *                   
 * Returns:          
 * Description:      
 * Note:             
 ***************************************************************************/
uint8_t SystemConfig_GetCellNumber(void)
{
    return g_stCellConfig.u8CellNumber;
}

/****************************************************************************
 *
 * Function Name:    SystemConfig_GetParam
 * Input:            u8SubID - 类别
 *                   u16BufSize - 缓冲区长度
 *
 * Output:           pu8OutBuf - 输出缓冲区
 *                   
 * Returns:          Buffer Length
 * Description:      
 * Note:             
 ***************************************************************************/
int32_t SystemConfig_GetParam(uint8_t u8SubID, uint8_t *pu8OutBuf, uint16_t BufSize)
{
    int32_t  s32Result = 0;

    switch(u8SubID)
    {
        case 0x01:  // Get Cell Number
            pu8OutBuf[0] = SystemConfig_GetCellNumber();
            s32Result = 1;
            break;
        default:
            s32Result = -1;
            break;
    }

    return s32Result;
}

/****************************************************************************
 *
 * Function Name:    SystemConfig_CheckState
 * Input:            None
 *
 * Output:           None
 *                   
 * Returns:          
 * Description:      
 * Note:             
 ***************************************************************************/
void SystemConfig_CheckState(void)
{
    if (g_u8UartRxState > 0)
    {
        g_u16CheckCountPerSec ++;
    }
    if (g_u16CheckCountPerSec > 1)
    {
        g_u8UartRxState = 0;
        g_u16CheckCountPerSec = 0;
        MCU_Uart1Printf("Uart1RxReset\r\n");
    }
}

/************************ (C) COPYRIGHT STMicroelectronics *****END OF FILE****/
