#include "BoardSDCard.h"

//SDCARD初始化
LH_ERR BoardSDCardInit(void)
{
    SD_Error sdErr = SD_OK;
    sdErr = SD_Init();
    if(SD_OK == sdErr)
    {
        return LH_ERR_NONE;
    }
    else
    {
        return LH_ERR_MAIN_SD_INIT;
    }
}

//SDCARD读取多个扇区
LH_ERR BoardSDCardReadSectorMulti(uint8_t* bufferPtr,uint32_t sectorAddress,uint8_t sectorCount)
{
    uint8_t result = SD_ReadDisk(bufferPtr,sectorAddress,sectorCount);
    if(result != 0)
    {
        return LH_ERR_MAIN_SD_READ;
    }
    else
    {
        return LH_ERR_NONE;
    }
}

//SDCARD写入多个扇区
LH_ERR BoardSDCardWriteSectorMulti(uint8_t* bufferPtr,uint32_t sectorAddress,uint8_t sectorCount)
{
    uint8_t result = SD_WriteDisk(bufferPtr,sectorAddress,sectorCount);
    if(result != 0)
    {
        return LH_ERR_MAIN_SD_WRITE;
    }
    else
    {
        return LH_ERR_NONE;
    }
}







