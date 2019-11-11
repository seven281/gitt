#ifndef __CORE_UTIL_H_
#define __CORE_UTIL_H_
#include "stm32f4xx.h"

/*主板中断分组设计,可选值 :
 * NVIC_PriorityGroup_0
 * NVIC_PriorityGroup_1
 * NVIC_PriorityGroup_2
 * NVIC_PriorityGroup_3
 * NVIC_PriorityGroup_4
 * */
#define SYSTEM_DEFAULT_NVIC_GROUP NVIC_PriorityGroup_2

/*设置默认的中断优先级分组*/
void CoreNVIC_SetGroupDefault(void);

void WFI_SET(void);

//关闭所有中断(但是不包括fault和NMI中断)
void INTX_DISABLE(void);

//开启所有中断
void INTX_ENABLE(void);

//设置栈顶地址
//addr:栈顶地址
//set Main Stack value
void MSR_MSP(uint32_t addr);



/**定义系统中断抢占优先级 数值越小,优先级越高*/
typedef enum INT_PRE_PRI
{
    INT_PRE_PRI_0 = 0,
    INT_PRE_PRI_1 = 1,
    INT_PRE_PRI_2 = 2,
    INT_PRE_PRI_3 = 3,
} INT_PRE_PRI;

/**定义系统中断子优先级 数值越小,优先级越高*/
typedef enum INT_SUB_PRI
{
    INT_SUB_PRI_0 = 0,
    INT_SUB_PRI_1 = 1,
    INT_SUB_PRI_2 = 2,
    INT_SUB_PRI_3 = 3,
} INT_SUB_PRI;

/*空引用定义*/
#ifndef NULL
#define NULL 0
#endif


// 这里只定义了 GPIO ODR和IDR这两个寄存器的位带别名区地址，其他寄存器的没有定义

//SRAM 位带区:    0X2000 0000~0X200F 0000
//SRAM 位带别名区:0X2200 0000~0X23FF FFFF

//外设 位带区:    0X4000 0000~0X400F FFFF
//外设 位带别名区:0X4200 0000~0X43FF FFFF

// 把“位带地址+位序号”转换成别名地址的宏
#define BITBAND(addr, bitnum) ((addr & 0xF0000000) + 0x02000000 + ((addr & 0x000FFFFF) << 5) + (bitnum << 2))

// 把一个地址转换成一个指针
#define MEM_ADDR(addr) *((volatile unsigned long *)(addr))

// 把位带别名区地址转换成指针
#define BIT_ADDR(addr, bitnum) MEM_ADDR(BITBAND(addr, bitnum))

// GPIO ODR 和 IDR 寄存器地址映射
#define GPIOA_ODR_Addr (GPIOA_BASE + 20)
#define GPIOB_ODR_Addr (GPIOB_BASE + 20)
#define GPIOC_ODR_Addr (GPIOC_BASE + 20)
#define GPIOD_ODR_Addr (GPIOD_BASE + 20)
#define GPIOE_ODR_Addr (GPIOE_BASE + 20)
#define GPIOF_ODR_Addr (GPIOF_BASE + 20)
#define GPIOG_ODR_Addr (GPIOG_BASE + 20)
#define GPIOH_ODR_Addr (GPIOH_BASE + 20)
#define GPIOI_ODR_Addr (GPIOI_BASE + 20)
#define GPIOJ_ODR_Addr (GPIOJ_BASE + 20)
#define GPIOK_ODR_Addr (GPIOK_BASE + 20)

#define GPIOA_IDR_Addr (GPIOA_BASE + 16)
#define GPIOB_IDR_Addr (GPIOB_BASE + 16)
#define GPIOC_IDR_Addr (GPIOC_BASE + 16)
#define GPIOD_IDR_Addr (GPIOD_BASE + 16)
#define GPIOE_IDR_Addr (GPIOE_BASE + 16)
#define GPIOF_IDR_Addr (GPIOF_BASE + 16)
#define GPIOG_IDR_Addr (GPIOG_BASE + 16)
#define GPIOH_IDR_Addr (GPIOH_BASE + 16)
#define GPIOI_IDR_Addr (GPIOI_BASE + 16)
#define GPIOJ_IDR_Addr (GPIOJ_BASE + 16)
#define GPIOK_IDR_Addr (GPIOK_BASE + 16)

// 单独操作 GPIO的某一个IO口，n(0,1,2...16),n表示具体是哪一个IO口
#define PAout(n) BIT_ADDR(GPIOA_ODR_Addr, n) //输出
#define PAin(n) BIT_ADDR(GPIOA_IDR_Addr, n)  //输入

#define PBout(n) BIT_ADDR(GPIOB_ODR_Addr, n) //输出
#define PBin(n) BIT_ADDR(GPIOB_IDR_Addr, n)  //输入

#define PCout(n) BIT_ADDR(GPIOC_ODR_Addr, n) //输出
#define PCin(n) BIT_ADDR(GPIOC_IDR_Addr, n)  //输入

#define PDout(n) BIT_ADDR(GPIOD_ODR_Addr, n) //输出
#define PDin(n) BIT_ADDR(GPIOD_IDR_Addr, n)  //输入

#define PEout(n) BIT_ADDR(GPIOE_ODR_Addr, n) //输出
#define PEin(n) BIT_ADDR(GPIOE_IDR_Addr, n)  //输入

#define PFout(n) BIT_ADDR(GPIOF_ODR_Addr, n) //输出
#define PFin(n) BIT_ADDR(GPIOF_IDR_Addr, n)  //输入

#define PGout(n) BIT_ADDR(GPIOG_ODR_Addr, n) //输出
#define PGin(n) BIT_ADDR(GPIOG_IDR_Addr, n)  //输入

#define PHout(n) BIT_ADDR(GPIOH_ODR_Addr, n) //输出
#define PHin(n) BIT_ADDR(GPIOH_IDR_Addr, n)  //输入

#define PIout(n) BIT_ADDR(GPIOI_ODR_Addr, n) //输出
#define PIin(n) BIT_ADDR(GPIOI_IDR_Addr, n)  //输入

#define PJout(n) BIT_ADDR(GPIOJ_ODR_Addr, n) //输出
#define PJin(n) BIT_ADDR(GPIOJ_IDR_Addr, n)  //输入

#define PKout(n) BIT_ADDR(GPIOK_ODR_Addr, n) //输出
#define PKin(n) BIT_ADDR(GPIOK_IDR_Addr, n)  //输入

#endif





