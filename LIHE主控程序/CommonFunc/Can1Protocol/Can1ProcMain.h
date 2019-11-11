#ifndef __CAN1_PROC_MAIN_H_
#define __CAN1_PROC_MAIN_H_
#include "Can1ProcUtil.h"

//CAN1解析使用CCM内存

//CAN1数据初始化
void Can1ProcDataInit(void);

//CAN1接收处理一个数据单元
void Can1RecvProc(MCU_CAN_RECEIVE_BUFFER_UNIT* rxUnit);

//CAN1接收完成的数据的转发
void Can1DispatchReceive(CAN1_CMD_RECV* can1CommandRecvPtr);

//CAN1发送指令的数据单元的初始化
void Can1SendCommandDataStructInit(CAN1_CMD_SEND* can1CmdPtr);

//发送数据包
void Can1SendCommandPack(CAN1_CMD_SEND* can1CmdPtr);

//发送数据包,在接收到的数据包的基础上
void Can1SendCommandPackWithRecvCmd(CAN1_CMD_RECV* can1CommandPtr,uint16_t commandValue,uint16_t dataBufferLength,uint8_t* dataBufferPtr);

//发生错误报警
void Can1ReportErrorCode(CAN1_REPORT_ERR_LEVEL level,CAN1_REPORT_ERR_CODE errCode,uint8_t param1,
                            uint8_t param2,uint8_t param3,uint8_t param4);
int32_t CanSendDataToCell(uint8_t u8CellNmbr, uint8_t u8Dest, uint16_t u16Cmd, const uint8_t *pu8Body, uint16_t u16Size);
#endif





