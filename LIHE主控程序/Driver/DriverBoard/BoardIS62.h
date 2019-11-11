#ifndef __BOARD_IS62_H_
#define __BOARD_IS62_H_
#include "MCU_Inc.h"

/**ISWV51216 512*16/2 1M字节*/
//对IS61LV25616/IS62WV25616,地址线范围为A0~A17 
//对IS61LV51216/IS62WV51216,地址线范围为A0~A18
//使用第四块 11 ->C
#define BOARD_IS62_BASE_ADDR    ((uint32_t)(0x6C000000))

//1M容量
#define BOARD_IS62_CAPACITY     (1024*1024)

LH_ERR BoardIS62Init(void);

LH_ERR BoardIS62ReadBuffer(uint32_t baseAddr,uint8_t* bufferPtr,uint32_t length);

LH_ERR BoardIS62WriteBuffer(uint32_t baseAddr,uint8_t* bufferPtr,uint32_t length);

LH_ERR BoardIS62SelfCheck(void);


#endif




