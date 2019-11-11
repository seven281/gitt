#ifndef __CORE_NVIC_PRIO_H_
#define __CORE_NVIC_PRIO_H_
#include "stm32f4xx.h"

//因为系统的中断四位,所以,慎重起见,将中断感知级别设置为7 ,也就是抢占优先级1,子优先级3
//只有在这之上的中断,才能调用系统函数

//系统内部NVIC中断的优先级设定


/*****************************不会调用系统内核函数的中断优先级************************************/
//窗口看门狗 最高优先级
#define MCU_WDG_WINDOW_PREE_PRI             INT_PRE_PRI_0
#define MCU_WDG_WINDOW_SUB_PRI              INT_SUB_PRI_0

//文件系统驱动 SDIO
#define SDIO_PREEMPT_PRIORITY               INT_PRE_PRI_0
#define SDIO_SUB_PRIORITY                   INT_SUB_PRI_3

//串口1发送 DMA中断
#define MCU_DMA2_STREAM7_PREE_PRI           INT_PRE_PRI_1
#define MCU_DMA2_STREAM7_SUB_PRI            INT_SUB_PRI_2

//串口3发送,DMA中断
#define MCU_DMA1_STREAM3_PREE_PRI           INT_PRE_PRI_1
#define MCU_DMA1_STREAM3_SUB_PRI            INT_SUB_PRI_3


/************************************会调用系统内核函数的中断优先级********************************/

//CAN2 中断优先级 RX CAN
#define CAN2_RX_INT_PREE_PRI                INT_PRE_PRI_2
#define CAN2_RX_INT_SUB_PRI                 INT_SUB_PRI_0

//CAN1 中断优先级RX CAN
#define CAN1_RX_INT_PREE_PRI                INT_PRE_PRI_2
#define CAN1_RX_INT_SUB_PRI                 INT_SUB_PRI_1

//CAN2 中断优先级 TX CAN
#define CAN2_TX_INT_PREE_PRI                INT_PRE_PRI_2
#define CAN2_TX_INT_SUB_PRI                 INT_SUB_PRI_2

//CAN1 中断优先级 TX CAN
#define CAN1_TX_INT_PREE_PRI                INT_PRE_PRI_2
#define CAN1_TX_INT_SUB_PRI                 INT_SUB_PRI_3

//串口2中断,发送中断也在这里
#define MCU_UART2_PREE_PRI                  INT_PRE_PRI_3
#define MCU_UART2_SUB_PRI                   INT_SUB_PRI_0

//串口6中断,发送中断也在这里
#define MCU_UART6_PREE_PRI                  INT_PRE_PRI_3
#define MCU_UART6_SUB_PRI                   INT_SUB_PRI_1

//串口1接收中断
#define MCU_UART1_DMA_PREE_PRI              INT_PRE_PRI_3
#define MCU_UART1_DMA_SUB_PRI               INT_SUB_PRI_2

//串口3接收中断
#define MCU_UART3_PREE_PRI                  INT_PRE_PRI_3
#define MCU_UART3_SUB_PRI                   INT_SUB_PRI_2



#endif






