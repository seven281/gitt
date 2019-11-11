
#include <stdlib.h>
#include <string.h>
#include <stdio.h>
#include "MCU_internalFlash.h"

/* Exported types ------------------------------------------------------------*/
/* Exported constants --------------------------------------------------------*/
 
static uint32_t MCU_FlashGetSector(uint32_t Address);

int MCU_FlashErase(uint32_t u32Addr, uint8_t u8Quantity)
{
    uint8_t i;
    uint16_t u16StartSector;

    if (u8Quantity > FLASH_PROGRAM_SECTOR_QUANTITY)
    {
        return -1;
    }
    u16StartSector = MCU_FlashGetSector(u32Addr);
    if (u16StartSector < FLASH_Sector_4)
    {
        return -2;
    }
    FLASH_Unlock();
    FLASH_ClearFlag(FLASH_FLAG_EOP | FLASH_FLAG_OPERR | FLASH_FLAG_WRPERR | 
                    FLASH_FLAG_PGAERR | FLASH_FLAG_PGPERR | FLASH_FLAG_PGSERR);
    for (i=0; i<u8Quantity; i++)
    {
        if (FLASH_EraseSector(u16StartSector, VoltageRange_3) != FLASH_COMPLETE)
        {
            return -3;
        }
        if (u16StartSector == FLASH_Sector_11)
        {
            u16StartSector += 40;
        } 
        else 
        {
            u16StartSector += 8;
        }
    }
    FLASH_Lock();
		
    return 0;
}

int MCU_FlashProgram(uint32_t u32Addr, const uint8_t *pu8Data, uint32_t u32Size)
{
    uint32_t i;
    uint32_t u32BufIndex = 0;
    uint32_t u32WordSize = u32Size&0xFFFFFFFC;
    uint8_t  u8ByteSize = (uint8_t)(u32Size-u32WordSize);
	
    if ((u32Addr&0x03) > 0)
    {
        return -1;
    }
    if (NULL == pu8Data)
    {
        return -2;
    }

    FLASH_Unlock();
    u32BufIndex = 0;
    for (i=0; i<u32WordSize; i+=4)
    {
        if (FLASH_ProgramWord(u32Addr+u32BufIndex, *(uint32_t *)(pu8Data+u32BufIndex)) == FLASH_COMPLETE)
        {
            u32BufIndex = u32BufIndex + 4;
        }
        else
        {
            return -4;
        }
    }
    for (i=0; i<u8ByteSize; i++)
    {
        if (FLASH_ProgramByte(u32Addr+u32BufIndex, *(uint8_t *)(pu8Data+u32BufIndex)) == FLASH_COMPLETE)
        {
            u32BufIndex = u32BufIndex + 1;
        }
        else
        { 
            return -5;
        }
    }
    FLASH_Lock();
		
    return 0;
}

static uint32_t MCU_FlashGetSector(uint32_t Address)
{
  uint32_t sector = 0;
  
  if((Address < ADDR_FLASH_SECTOR_1) && (Address >= ADDR_FLASH_SECTOR_0))
  {
    sector = FLASH_Sector_0;  
  }
  else if((Address < ADDR_FLASH_SECTOR_2) && (Address >= ADDR_FLASH_SECTOR_1))
  {
    sector = FLASH_Sector_1;  
  }
  else if((Address < ADDR_FLASH_SECTOR_3) && (Address >= ADDR_FLASH_SECTOR_2))
  {
    sector = FLASH_Sector_2;  
  }
  else if((Address < ADDR_FLASH_SECTOR_4) && (Address >= ADDR_FLASH_SECTOR_3))
  {
    sector = FLASH_Sector_3;  
  }
  else if((Address < ADDR_FLASH_SECTOR_5) && (Address >= ADDR_FLASH_SECTOR_4))
  {
    sector = FLASH_Sector_4;  
  }
  else if((Address < ADDR_FLASH_SECTOR_6) && (Address >= ADDR_FLASH_SECTOR_5))
  {
    sector = FLASH_Sector_5;  
  }
  else if((Address < ADDR_FLASH_SECTOR_7) && (Address >= ADDR_FLASH_SECTOR_6))
  {
    sector = FLASH_Sector_6;  
  }
  else if((Address < ADDR_FLASH_SECTOR_8) && (Address >= ADDR_FLASH_SECTOR_7))
  {
    sector = FLASH_Sector_7;  
  }
  else if((Address < ADDR_FLASH_SECTOR_9) && (Address >= ADDR_FLASH_SECTOR_8))
  {
    sector = FLASH_Sector_8;  
  }
  else if((Address < ADDR_FLASH_SECTOR_10) && (Address >= ADDR_FLASH_SECTOR_9))
  {
    sector = FLASH_Sector_9;  
  }
  else if((Address < ADDR_FLASH_SECTOR_11) && (Address >= ADDR_FLASH_SECTOR_10))
  {
    sector = FLASH_Sector_10;  
  }
  else if((Address < (ADDR_FLASH_SECTOR_11+FLASH_SECTOR_SIZE)) && (Address >= ADDR_FLASH_SECTOR_11))
  {
    sector = FLASH_Sector_11;  
  }
  else
  {
    sector = 0;  
  }


  return sector;
}



