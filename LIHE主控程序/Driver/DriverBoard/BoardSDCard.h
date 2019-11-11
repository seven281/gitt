#ifndef __BOARD_SDCARD_H_
#define __BOARD_SDCARD_H_
#include "MCU_Inc.h"

//SDCARD初始化
LH_ERR BoardSDCardInit(void);

//SDCARD读取多个扇区
LH_ERR BoardSDCardReadSectorMulti(uint8_t* bufferPtr,uint32_t sectorAddress,uint8_t sectorCount);

//SDCARD写入多个扇区
LH_ERR BoardSDCardWriteSectorMulti(uint8_t* bufferPtr,uint32_t sectorAddress,uint8_t sectorCount);



#endif





