/***********************************************************************
 *
 * (c) Copyright 2001-2016, XXXX, All Rights Reserved.
 * THIS IS UNPUBLISHED PROPRIETARY SOURCE CODE OF XXXX, INC.
 * The copyright notice above does not evidence any actual or intended
 * publication of such source code. 
 *
 *  Subsystem:   System
 *  File:        SystemConfig.h
 *  Author:      Íõµî¹â
 *  Description: Double List for C header files.
 *********************************************************************/

/* Define to prevent recursive inclusion -------------------------------------*/
#ifndef __SYSTEM_CONFIG_H__
#define __SYSTEM_CONFIG_H__

#ifdef __cplusplus
 extern "C" {
#endif


typedef enum
{
    CONFIG_TYPE_NEW_APP = 0,
    CONFIG_TYPE_APP_CONFIG,
    CONFIG_TYPE_ERROR,
}CONFIG_TYPE_E;

typedef struct
{
    uint8_t  u8Body[120];
    uint16_t u16BodySize;
    uint16_t u16RecvCrc;
    uint8_t  u8Cmd;
    uint8_t  u8Invalid1;
    uint8_t  u8Invalid2;
    uint8_t  u8Invalid3;
}SYSTEMCONFIG_CMD_T;


extern void SystemConfig_IsrCallBack(uint8_t u8Char);
extern int32_t SystemConfig_SendCmd(uint8_t u8Cmd, const uint8_t *pu8Data, uint16_t u16DataSize);
extern int32_t SystemConfig_LoadConfig(void);
extern uint8_t SystemConfig_GetCellNumber(void);
extern int32_t SystemConfig_SetParam(uint8_t u8SubID, const uint8_t *pu8Data, uint16_t u16DataSize);
extern int32_t SystemConfig_GetParam(uint8_t u8SubID, uint8_t *pu8OutBuf, uint16_t BufSize);
extern void SystemConfig_CheckState(void);
extern int32_t SystemConfig_SaveConfig(CONFIG_TYPE_E eType);

#ifdef __cplusplus
}
#endif

#endif /* __SYSTEM_CONFIG_H__ */

/************************ (C) COPYRIGHT STMicroelectronics *****END OF FILE****/
