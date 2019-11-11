
#ifndef __TESTPROC_CANCONFIG_H_
#define __TESTPROC_CANCONFIG_H_

#include "TestProc_canCommand.h"
#include "TestProc_boardID.h"
#include "TaskAppConfig.h"
#include "TestProc_Typedef.h"

//当前接收缓存状态
typedef enum CAN1_RECV_STATE
{
    CAN1_RECV_STATE_IDLE,
    CAN1_RECV_STATE_START,
    CAN1_RECV_STATE_DATA,
    CAN1_RECV_STATE_WAIT_END,
    CAN1_RECV_STATE_END,
    CAN1_RECV_STATE_ERR,
}CAN1_RECV_STATE;

//当前接收缓存错误标识
typedef enum CAN1_RECV_ERR
{
    CAN1_RECV_ERR_NONE = 0X0000,
    CAN1_RECV_ERR_START_LOSS = 0X0001,
    CAN1_RECV_ERR_DATA_LOSS = 0X0002,
    CAN1_RECV_ERR_DATA_OVERFLOW = 0X0003,
    CAN1_RECV_ERR_CRC_CHECK = 0X0004,
    CAN1_RECV_ERR_UNKNOW = 0X0005,
}CAN1_RECV_ERR;


//错误代码的错误级别
typedef enum CAN1_REPORT_ERR_LEVEL
{
    CAN1_REPORT_ERR_LEVEL_EMERGENCY_STOP = 1,//急停级报警
    CAN1_REPORT_ERR_LEVEL_NORMAL_STOP = 2,//停止级报警
    CAN1_REPORT_ERR_LEVEL_WARNING = 3,//注意级报警
    CAN1_REPORT_ERR_LEVEL_NORMAL = 4,//常规报警
}CAN1_REPORT_ERR_LEVEL;

//错误代码
typedef enum CAN1_REPORT_ERR_CODE
{
    CAN1_REPORT_ERR_CODE_RACK_LOST = 0X0001,//试管架丢失
    CAN1_REPORT_ERR_CODE_BARSCAN_NO_RESP = 0X0002,//条码枪无返回
    CAN1_REPORT_ERR_CODE_RACK_PUSHOUT1_FULL = 0X0003,//出样1满
    CAN1_REPORT_ERR_CODE_RACK_PUSHOUT1_BLOCK = 0X0004,//出样1堵住
    CAN1_REPORT_ERR_CODE_RACK_PUSHOUT2_FULL = 0X0005,//出样2满
    CAN1_REPORT_ERR_CODE_RACK_PUSHOUT2_BLOCK = 0X0006,//出样2堵住
    CAN1_REPORT_ERR_CODE_RACK_PUSHIN_RACK_NOT_EXIST = 0X0007,//进样试管架不存在
    CAN1_REPORT_ERR_CODE_MOTOR_RUNNING_FAILED = 0X0008,//电机运行异常,运行失败
    CAN1_REPORT_ERR_CODE_MOTOR_RESET_FAILED = 0X0009,//电机复位异常,复位失败
    CAN1_REPORT_ERR_CODE_RACK_FULL = 0X000A,//试管架区域满
    CAN1_REPORT_ERR_CODE_RACK_NOT_EXIST = 0X000B,//试管架区域不存在指定ID试管架
    CAN1_REPORT_ERR_CODE_OUTPUT_WRITE = 0X000C,//写入失败
    CAN1_REPORT_ERR_CODE_INPUT_READ = 0X000D,//读取失败
}CAN1_REPORT_ERR_CODE;

#pragma pack (push)
#pragma pack (1)

//接收到的指令结构体
typedef struct CAN1_CMD_RECV
{
    uint8_t srcModuleNo;
    uint8_t srcBoardAddr;
    uint8_t targetModuleNo;
    uint8_t targetBoardAddr;
    uint16_t command;
    uint16_t recvDataLength;
    uint8_t recvDataBuffer[CAN1_SINGLE_PACK_MAX_LENGTH];
}CAN1_CMD_RECV;

//接收指令缓存
typedef struct CAN1_RECV_CACHE
{
    //接收状态
    CAN1_RECV_STATE cmdRecvState;
    //接收错误代码
    CAN1_RECV_ERR errorState;
    //接收缓存
    CAN1_CMD_RECV cmdRecv;
    //预期接收的数据总量
    uint16_t exceptDataByteCount;
    //预期接收的数据包总数
    uint16_t exceptDataPackCount;
    //当前接收的数据总数
    uint16_t currentDataByteCount;
    //当前接收的数据包
    uint16_t currentDataPackCount;
}CAN1_RECV_CACHE;

//CAN1的ID解析结果
typedef struct CAN1_ID_ANAYSIS_RESULT
{
    //源模块号
    uint8_t srcModuleNo;
    //源板号
    uint8_t srcBoardAddr;
    //目标模块号
    uint8_t targetModuleNo;
    //目标板号
    uint8_t targetBoardAddr;
    //指令
    uint16_t command;
}CAN1_ID_ANAYSIS_RESULT;

//CAN1发送数据包的时候的指令结构体
typedef struct CAN1_CMD_SEND
{
    uint8_t srcModuleNo;
    uint8_t srcBoardAddr;
    uint8_t targetModuleNo;
    uint8_t targetBoardAddr;
    uint16_t command;
    uint16_t sendDataLength;
    uint8_t* sendDataBuffer;
}CAN1_CMD_SEND;

//CAN1数据转发的基础元素
typedef struct CAN1_DISPATCH_UNIT
{
    uint16_t commandValue;//CAN1指令
    OS_TCB* tcbTaskPtr;//CAN1指令与对应的处理函数
}CAN1_DISPATCH_UNIT;

typedef struct
{
    uint8_t *pu8DataBuf;
    uint16_t u16Size;
    uint16_t u16Cmd;
    uint8_t  u8DataBuf[8];
    uint8_t  u8CellNmbr;
    uint8_t  u8SrcBoard;
    uint8_t  u8DestBoard;
    uint8_t  u8Param;
}TESTPROC_CAN_CMD_T;

typedef struct
{
    TESTPROC_CAN_CMD_T *pstCmd;
    uint16_t u16DataCount;
    uint16_t u16CRC;
}TESTPROC_CAN_MULTY_T;

#pragma pack (pop)

extern int32_t CanSendDataToCell(uint8_t u8CellNmbr, uint8_t u8Dest, uint16_t u16Cmd, const uint8_t *pu8Body, uint16_t u16Size);
#define TestProc_SendCmdToComm(DestBoard, Cmd)                     CanSendDataToCell(TESTPROC_CELL_PLATFORM_NMBR, (DestBoard), (Cmd), NULL, 0)
#define TestProc_SendDataToComm(DestBoard, Cmd, pData, Size)       CanSendDataToCell(TESTPROC_CELL_PLATFORM_NMBR, (DestBoard), (Cmd), (pData), (Size))
extern void Can1DispatchReceive(CAN1_CMD_RECV* can1CommandRecvPtr);

#endif // End of “ifndef __TESTPROC_WASH_H__

