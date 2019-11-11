#ifndef __MCU_SPI_H_
#define __MCU_SPI_H_
#include "CoreInc.h"

/**************************************SPI相关定义*****************************************/
//SPI速度
typedef enum MCU_SPI_SPEED
{
    MCU_SPI_SPEED_DIV2 = SPI_BaudRatePrescaler_2,
    MCU_SPI_SPEED_DIV4 = SPI_BaudRatePrescaler_4,
    MCU_SPI_SPEED_DIV8 = SPI_BaudRatePrescaler_8,
    MCU_SPI_SPEED_DIV16 = SPI_BaudRatePrescaler_16,
    MCU_SPI_SPEED_DIV32 = SPI_BaudRatePrescaler_32,
    MCU_SPI_SPEED_DIV64 = SPI_BaudRatePrescaler_64,
    MCU_SPI_SPEED_DIV128 = SPI_BaudRatePrescaler_128,
    MCU_SPI_SPEED_DIV256 = SPI_BaudRatePrescaler_256,
}MCU_SPI_SPEED;

//SPI信号极性
typedef enum MCU_SPI_CPOL
{
    MCU_SPI_CPOL_HIGH = SPI_CPOL_High,
    MCU_SPI_CPOL_LOW = SPI_CPOL_Low,
}MCU_SPI_CPOL;

//SPI采样点设置
typedef enum MCU_SPI_CPHA
{
    MCU_SPI_CPHA_1EDGE = SPI_CPHA_1Edge,
    MCU_SPI_CPHA_2EDGE = SPI_CPHA_2Edge,
}MCU_SPI_CPHA;

//SPI数据等待最大计数值
#define MCU_SPI_WAIT_COUNT_MAX      50000
/**************************************SPI相关定义*****************************************/

//SPI操作系统锁定调度
#define MCU_SPI_THREAD_LOCK()       CPU_IntDis()

//SPI操作系统锁定调度
#define MCU_SPI_THREAD_UNLOCK()     CPU_IntEn()


//SPI1 初始化
void MCU_SPI1_Init(MCU_SPI_SPEED speed,MCU_SPI_CPOL cpol,MCU_SPI_CPHA cpha);

//读写数据
uint8_t MCU_SPI1_WriteRead(uint8_t writeDat);

//设置速度
void MCU_SPI1_SetSpeed(MCU_SPI_SPEED speed);

//设置信号极性
void MCU_SPI1_SetPolarity(MCU_SPI_CPOL cpol,MCU_SPI_CPHA cpha);


//SPI2 初始化
void MCU_SPI2_Init(MCU_SPI_SPEED speed,MCU_SPI_CPOL cpol,MCU_SPI_CPHA cpha);

//读写数据
uint8_t MCU_SPI2_WriteRead(uint8_t writeDat);

//设置速度
void MCU_SPI2_SetSpeed(MCU_SPI_SPEED speed);

//设置信号极性
void MCU_SPI2_SetPolarity(MCU_SPI_CPOL cpol,MCU_SPI_CPHA cpha);


#endif




