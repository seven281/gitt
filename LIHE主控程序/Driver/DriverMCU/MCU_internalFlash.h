#ifndef __INTERNAL_FLASH_H
#define	__INTERNAL_FLASH_H

#include "stm32f4xx.h"

#define FLASH_PROGRAM_SECTOR_QUANTITY                 (6)
#define FLASH_PROGRAM_NEW_FUNC                        (0xFFA5005A)

#define FLASH_PROGRAM_ADDR                            (0x08020000)
#define FLASH_BOOT_CONFIG_ADDR                        (0x08010000)
#define FLASH_BOOT_CONFIG_SIZE                        (2048)
#define FLASH_BOOT_CONFIG_NEW_APP_ADDR                (0x08010000)
#define FLASH_BOOT_CONFIG_VER_1                       (0x08010004)
#define FLASH_BOOT_CONFIG_VER_2                       (0x08010005)
#define FLASH_BOOT_CONFIG_VER_3                       (0x08010006)

#define FLASH_APP_CONFIG_ADDR                         (FLASH_BOOT_CONFIG_ADDR+FLASH_BOOT_CONFIG_SIZE)
#define FLASH_APP_CONFIG_SIZE                         (2048)
#define FLASH_NEW_FLAG                                (0xA5A50A5A)

/* Base address of the Flash sectors */ 
#define ADDR_FLASH_SECTOR_0     ((uint32_t)0x08000000) /* Base address of Sector 0, 16 Kbytes   */
#define ADDR_FLASH_SECTOR_1     ((uint32_t)0x08004000) /* Base address of Sector 1, 16 Kbytes   */
#define ADDR_FLASH_SECTOR_2     ((uint32_t)0x08008000) /* Base address of Sector 2, 16 Kbytes   */
#define ADDR_FLASH_SECTOR_3     ((uint32_t)0x0800C000) /* Base address of Sector 3, 16 Kbytes   */
#define ADDR_FLASH_SECTOR_4     ((uint32_t)0x08010000) /* Base address of Sector 4, 64 Kbytes   */
#define ADDR_FLASH_SECTOR_5     ((uint32_t)0x08020000) /* Base address of Sector 5, 128 Kbytes  */
#define ADDR_FLASH_SECTOR_6     ((uint32_t)0x08040000) /* Base address of Sector 6, 128 Kbytes  */
#define ADDR_FLASH_SECTOR_7     ((uint32_t)0x08060000) /* Base address of Sector 7, 128 Kbytes  */
#define ADDR_FLASH_SECTOR_8     ((uint32_t)0x08080000) /* Base address of Sector 8, 128 Kbytes  */
#define ADDR_FLASH_SECTOR_9     ((uint32_t)0x080A0000) /* Base address of Sector 9, 128 Kbytes  */
#define ADDR_FLASH_SECTOR_10    ((uint32_t)0x080C0000) /* Base address of Sector 10, 128 Kbytes */
#define ADDR_FLASH_SECTOR_11    ((uint32_t)0x080E0000) /* Base address of Sector 11, 128 Kbytes */

#define ADDR_FLASH_SECTOR_12    ((uint32_t)0x08100000) /* Base address of Sector 12, 16 Kbytes  */
#define FLASH_SECTOR_SIZE       (0x20000)

int MCU_FlashErase(uint32_t u32Addr, uint8_t u8Quantity);
int MCU_FlashProgram(uint32_t u32Addr, const uint8_t *pu8Data, uint32_t u32Size);


#endif /* __INTERNAL_FLASH_H */

