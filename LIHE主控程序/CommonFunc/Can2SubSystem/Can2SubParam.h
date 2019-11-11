#ifndef __CAN2_SUB_PARAM_H_
#define __CAN2_SUB_PARAM_H_
#include "Can2ProcMain.h"
#include "Can2SubParamIndex.h"

//通道支持的指令
typedef enum CAN2_SUB_PARAM_CMD
{
    CAN2_SUB_PARAM_CMD_SM_READ = 0X00000000,
    CAN2_SUB_PARAM_CMD_SM_WRITE = 0X00000001,
    CAN2_SUB_PARAM_CMD_SM_SAVE = 0X00000002,
}CAN2_SUB_PARAM_CMD;


typedef struct CAN2_SUB_PARAM_SM_READ
{
    //电机编号
    uint8_t stepMotorIndex;
    //参数编号
    uint8_t paramMainIndex;
    //参数子编号
    uint16_t paramSubIndex;
    //读取到的参数结果
    uint32_t paramReadResult;
}CAN2_SUB_PARAM_SM_READ;

typedef struct CAN2_SUB_PARAM_SM_WRITE
{
    //电机编号
    uint8_t stepMotorIndex;
    //参数编号
    uint8_t paramMainIndex;
    //参数子编号
    uint16_t paramSubIndex;
    //读取到的参数结果
    uint32_t paramReadResult;
}CAN2_SUB_PARAM_SM_WRITE;


#endif





