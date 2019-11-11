#ifndef __MCU_CAN_H_
#define __MCU_CAN_H_
#include "CoreInc.h"
//MCU的CAN总线驱动
/**************************************CAN总线相关定义****************************************/
//波特率
typedef enum MCU_CAN_BAUD
{
    MCU_CAN_BAUD_1000K,
    MCU_CAN_BAUD_500K,
    MCU_CAN_BAUD_250K,
    MCU_CAN_BAUD_125K,
}MCU_CAN_BAUD;
/**************************************CAN总线相关定义****************************************/
//总线接收缓存区长度
#define MCU_CAN_RECEIVE_BUFFER_LENGTH               20

//CAN口接收数据单元
typedef struct MCU_CAN_RECEIVE_BUFFER_UNIT
{
    //缓存区
    CanRxMsg rxMsg;
    //等待处理标识,数据转发出去之后设置为1,转发线程处理完成,设置为0
    volatile uint8_t waitProcessFlag;
}MCU_CAN_RECEIVE_BUFFER_UNIT;

//CAN口接收数据缓存区
typedef struct MCU_CAN_RECEIVE_BUFFER
{
    //接收数据数组
    MCU_CAN_RECEIVE_BUFFER_UNIT receiveBufferArray[MCU_CAN_RECEIVE_BUFFER_LENGTH];
    //当前接收到的位置
    volatile uint16_t currentReceiveIndex;
}MCU_CAN_RECEIVE_BUFFER;

//CAN口中断回调函数类型
typedef void (*MCU_CAN_RecvIntProcFuncPtr)(MCU_CAN_RECEIVE_BUFFER_UNIT* rxMsgUnit);

//同时初始化CAN1与CAN2,CAN2与下位机通讯,带有主板ID
void MCU_CAN_Init(MCU_CAN_BAUD baud1, MCU_CAN_BAUD baud2, 
                        MCU_CAN_RecvIntProcFuncPtr callBackCan1, 
                        MCU_CAN_RecvIntProcFuncPtr callBackCan2);

//CAN总线传输讯息
void MCU_CAN1_TransMessage(CanTxMsg* txMsg);

//CAN总线传输讯息
void MCU_CAN2_TransMessage(CanTxMsg* txMsg);

//获取电压检测器状态
BitAction MCU_CAN1_GetSensorStatus(void);

//获取电压检测器状态
BitAction MCU_CAN2_GetSensorStatus(void);

#endif




