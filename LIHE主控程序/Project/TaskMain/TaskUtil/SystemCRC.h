/***********************************************************************
 *
 * (c) Copyright 2001-2016, XXXX, All Rights Reserved.
 * THIS IS UNPUBLISHED PROPRIETARY SOURCE CODE OF XXXX, INC.
 * The copyright notice above does not evidence any actual or intended
 * publication of such source code. 
 *
 *  Subsystem:   System
 *  File:        SystemCRC.h
 *  Author:      王殿光
 *  Description: Double List for C header files.
 *********************************************************************/

/* Define to prevent recursive inclusion -------------------------------------*/
#ifndef __SYSTEM_CRC_H__
#define __SYSTEM_CRC_H__

#ifdef __cplusplus
 extern "C" {
#endif

#include <stdint.h>

extern uint16_t SystemCRC_calcCRC16(const uint8_t *pu8Data, uint32_t u32Size);
extern uint32_t SystemCRC_calcCRC32(const uint8_t *pu8Data, uint32_t u32Size);

#ifdef __cplusplus
}
#endif

#endif /* __SYSTEM_CRC_H__ */

/************************ (C) COPYRIGHT STMicroelectronics *****END OF FILE****/
