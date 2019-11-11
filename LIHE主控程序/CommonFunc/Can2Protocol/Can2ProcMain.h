#ifndef __CAN2_PROC_MAIN_H_
#define __CAN2_PROC_MAIN_H_
#include "Can2ProcUtil.h"
#include "MCU_CAN.h"

//CAN2协议内存,使用SRAM IN

//CAN2接收协议初始化
void Can2ProcDataInit(void);

//CAN2接收处理一个数据单元
void Can2RecvProc(MCU_CAN_RECEIVE_BUFFER_UNIT* rxUnit);

//CAN2发送指令,直到收到ACK,注意,此处因为是异步的,所以会有一个数据等待释放的锁
//也就是说,,调用了这个函数,必须调用wairreturn
LH_ERR Can2SendCommandWhileAck(CAN2_SEND_CMD* sendCommand);

//等待指定板卡的指定通道的数据返回
LH_ERR Can2SendCommandWaitReturn(uint8_t boardID,CAN2_CHANNEL channelNo,uint32_t timeOutMs,
                                    CAN2_READ_DATA* readData);

//CAN2发送指令,直到收到Return返回
LH_ERR Can2SendCommandWhileReturn(CAN2_SEND_CMD* sendCommand,CAN2_READ_DATA* readData);




#endif




