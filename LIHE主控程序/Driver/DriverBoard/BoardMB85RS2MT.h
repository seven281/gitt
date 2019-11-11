#ifndef __BOARD_MB85RS2MT_H_
#define __BOARD_MB85RS2MT_H_

#include "MCU_Inc.h"


//MB85操作指令集
#define MB85RS2MT_CMD_WREN          0x06
#define MB85RS2MT_CMD_WRDI          0x04
#define MB85RS2MT_CMD_RDSR          0x05
#define MB85RS2MT_CMD_WRSR          0x01
#define MB85RS2MT_CMD_READ          0x03
#define MB85RS2MT_CMD_WRITE         0x02
#define MB85RS2MT_CMD_SLEEP         0xB9
#define MB85RS2MT_CMD_RDID          0x9F

//铁电存储器的存储空间大小,也是最大地址
#define MB85RS2MT_SIZE                  0x40000

//铁电存储器的芯片ID
#define MB85RS2MT_ID                    0X03287F04

//FRAM的片选信号
#define MB85RS2MT_CS_Set(value)         PAout(4) = value

//初始化铁电存储器
LH_ERR BoardMB85RS2MT_Init(void);

//读取芯片ID
void BoardMB85RS2MT_ReadID(uint32_t *id);

//写缓存
void BoardMB85RS2MT_WriteBuffer(uint32_t address, uint8_t *pdata, uint32_t length);

//读缓存
void BoardMB85RS2MT_ReadBuffer(uint32_t address, uint8_t *pdata, uint32_t length);






#endif




