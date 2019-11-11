#ifndef __CAN2_DATA_TYPE_H_
#define __CAN2_DATA_TYPE_H_
#include "BoardInc.h"
#include "Can2ProcConfig.h"
#include "ErrorCode.h"

//对通道的定义
typedef enum CAN2_CHANNEL
{
    CAN2_CHANNEL_HEART_AUTO_UPDATE = 0X00,
    CAN2_CHANNEL_INPUT = 0X01,
    CAN2_CHANNEL_OUTPUT = 0X02,
    CAN2_CHANNEL_SERIAL1 = 0X03,
    CAN2_CHANNEL_SERIAL3_UTIL = 0X04,
    CAN2_CHANNEL_SM1 = 0X05,//步进电机1-12
    CAN2_CHANNEL_SM2 = 0X06,
    CAN2_CHANNEL_SM3 = 0X07,
    CAN2_CHANNEL_SM4 = 0X08,
    CAN2_CHANNEL_SM5 = 0X09,
    CAN2_CHANNEL_SM6 = 0X0A,
    CAN2_CHANNEL_SM7 = 0X0B,
    CAN2_CHANNEL_SM8 = 0X0C,
    CAN2_CHANNEL_SM9 = 0X0D,
    CAN2_CHANNEL_SM10 = 0X0E,
    CAN2_CHANNEL_SM11 = 0X0F,
    CAN2_CHANNEL_SM12 = 0X10,
    CAN2_CHANNEL_PARAM = 0X11,
    CAN2_CHANNEL_IAP = 0X12,
    CAN2_CHANNEL_APP_DISH_REACTION = 0X13,//反应盘任务
}CAN2_CHANNEL;

//一定要注意,board1的值为1,不是0
typedef enum CAN2_SUB_BOARD
{
    CAN2_SUB_BOARD1 = 1,
    CAN2_SUB_BOARD2,
    CAN2_SUB_BOARD3,
    CAN2_SUB_BOARD4,
    CAN2_SUB_BOARD5,
    CAN2_SUB_BOARD6,
    CAN2_SUB_BOARD7,
    CAN2_SUB_BOARD8,
    CAN2_SUB_BOARD9,
    CAN2_SUB_BOARD10,
}CAN2_SUB_BOARD;

//CAN总线通讯协议解析之数据标识
typedef enum CAN2_SUB_DATA_FLAG
{
    CAN2_SUB_DATA_HEART = 0x0000,//心跳包
    CAN2_SUB_DATA_MOTOR = 0X0001,//电机数据
    CAN2_SUB_DATA_IO_IN = 0X0002,//IO状态
    CAN2_SUB_DATA_SERIAL_RS1 = 0X0003,//串口数据1
    CAN2_SUB_DATA_SERIAL_RS2 = 0X0004,//串口数据2
    CAN2_SUB_DATA_PARAM = 0X0005,//参数
    CAN2_SUB_DATA_IAP_FLAG = 0X1001,//IAP符号
    CAN2_SUB_DATA_SOFTVERSION = 0X1002,//软件版本
    CAN2_SUB_DATA_DISH_REACTION_INDEX = 0X2000,//当前反应盘序号
}CAN2_SUB_DATA_FLAG;

//ID解析包类型
typedef enum CAN2_PACK_TYPE
{
    CAN2_PACK_AUTO_UPDATE = 0X00,//主动上报帧
    CAN2_PACK_CMD = 0X01,//指令帧
    CAN2_PACK_ACK = 0X02,//响应帧
    CAN2_PACK_HEART = 0X03,//心跳帧
    CAN2_PACK_DATA = 0X04,//数据帧
    CAN2_PACK_RESULT = 0X05,//结果帧
}CAN2_PACK_TYPE;

//长短包标志
typedef enum CAN2_PACK_LEN_FLAG
{
    CAN2_PACK_LEN_SHORT = 0X00,//短包
    CAN2_PACK_LEN_LONG = 0X01,//长包
}CAN2_PACK_LEN_FLAG;

//单帧识别码
typedef enum CAN2_SINGLE_CODE
{
    CAN2_SINGLE_CODE_SHORT = 0X00,//短包单帧
    CAN2_SINGLE_CODE_LONG_START = 0X01,//长包起始帧
    CAN2_SINGLE_CODE_LONG_DATA = 0X02,//长包数据帧
    CAN2_SINGLE_CODE_LONG_END = 0X03,//长包结束帧
}CAN2_SINGLE_CODE;

//CAN2接收数据包解析结果
typedef struct CAN2_RECV_ID_PROC_RESULT
{
    //目标地址
    uint8_t targetBoardAddr;
    //发送地址
    uint8_t senderBoardAddr;
    //通道号码
    uint8_t channelNumber;
    //包类型
    CAN2_PACK_TYPE packType;
    //包长度
    CAN2_PACK_LEN_FLAG packLenFlag;
    //单帧识别码
    CAN2_SINGLE_CODE singleCode;
    //帧号
    uint32_t frameCode;
}CAN2_RECV_ID_PROC_RESULT;


//当前数据包的状态
typedef enum CAN2_RECV_DATA_STATE
{
    CAN2_RECV_DATA_STATE_INIT,//数据单元刚初始化
    CAN2_RECV_DATA_STATE_START,//数据单元接收到起始
    CAN2_RECV_DATA_STATE_DATA,//数据单元正在接收数据
    CAN2_RECV_DATA_STATE_WAIT_END,//数据单元正在等待结果
    CAN2_RECV_DATA_STATE_END,//数据单元接收完成
    CAN2_RECV_DATA_STATE_ERR_START,//起始信息错误
    CAN2_RECV_DATA_STATE_ERR_PACK_LOSS,//数据单元丢失
    CAN2_RECV_DATA_STATE_ERR_PACK_OVERFLOW,//数据单元溢出
}CAN2_RECV_DATA_STATE;

//CAN2协议,接收数据单元
typedef struct CAN2_RECV_DATA_UNIT
{
    //当前数据接收的状态
    CAN2_RECV_DATA_STATE currentDataState;
    //当前预期接收到的数据总数
    uint8_t exceptRecvPackCount;
    //当前预期接收到的数据包总数(不包含起始包和结束包)
    uint16_t exceptRecvDataLength;
    //当前接收的数据包总数
    uint8_t currentRecvPackCount;
    //当前接收到数据字节长度
    uint16_t currentRecvDataLength;
    //当前接收数据的缓存区
    uint8_t dataBuffer[SYSTEM_CAN2_SINGLE_RECV_DATA_MAX];
}CAN2_RECV_DATA_UNIT;

//总线指令接收状态
typedef enum CAN2_RECV_CMD_STATE
{
    CAN2_RECV_CMD_STATE_IDLE,//空闲状态
    CAN2_RECV_CMD_STATE_WAIT_ACK,//等待ACK
    CAN2_RECV_CMD_STATE_WAIT_RETURN,//等待返回
    CAN2_RECV_CMD_STATE_COMPLETE,//指令结束
    CAN2_RECV_CMD_STATE_ERR_ACK,//ACK返回错误
    CAN2_RECV_CMD_STATE_ERR_ACK_TIME_OUT,//ACK响应超时
    CAN2_RECV_CMD_STATE_ERR_ACK_RETRY_OUT,//ACK重发超时
    CAN2_RECV_CMD_STATE_ERR_RETURN_TIMEOUT,//指令返回超时
}CAN2_RECV_CMD_STATE;

//当前通道状态
typedef struct CAN2_RECV_CHANNEL_UNIT
{
    OS_MUTEX mutexCan2Receive;//信道互斥信号量
    CAN2_RECV_CMD_STATE channelState;//当前通道的状态
    uint8_t cmdFrameCode;//信道帧号,每次发送时候自动增加,1-15计数
    uint32_t commandSendOutStartMs;//指令发送完成的时间
    uint32_t commandSendRetryCount;//指令发送次数
    uint32_t ackCode;//指令的ACK代码
    uint32_t errCode;//指令的结果代码
    CAN2_RECV_DATA_UNIT* dataRecvUnitPtr;//数据包,使用动态申请内存的形式使用,否则对系统内存的占用太大了
}CAN2_RECV_CHANNEL_UNIT;

//每块板子上的数据单元
typedef struct CAN2_RECV_BOARD_UNIT
{
    CAN2_RECV_CHANNEL_UNIT channelCacheArray[SYSTEM_CAN2_BOARD_CHANNEL_MAX];
}CAN2_RECV_BOARD_UNIT;

//整个系统对于CAN2的接收支持
typedef struct CAN2_RECV_CACHE
{
    //这是接收缓存
    CAN2_RECV_BOARD_UNIT boardCacheArray[SYSTEM_CAN2_BOARD_MAX];
}CAN2_RECV_CACHE;

//CAN2指令发送结构体
typedef struct CAN2_SEND_CMD
{
    CAN2_SUB_BOARD targetBoardID;//目标板号
    CAN2_CHANNEL channelNo;//指令通道号
    uint32_t timeoutMs;//超时时间
    uint32_t commandCode;//指令码
    uint8_t* paramBuffer;//参数缓存
    uint16_t paramLength;//参数长度
}CAN2_SEND_CMD;

//CAN2指令返回结构体
typedef struct CAN2_READ_DATA
{
    //驱动会主动去为这个指针申请内存
    uint8_t* recvDataPtr;
    //接收到的数据包长度
    uint16_t recvDatLength;
}CAN2_READ_DATA;


#endif






