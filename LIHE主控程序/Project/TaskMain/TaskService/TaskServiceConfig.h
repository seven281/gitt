#ifndef __TASK_SERVICE_CONFIG_H_
#define __TASK_SERVICE_CONFIG_H_
#include "BoardInc.h"
#include "SystemParam.h"
#include "Can2SubInc.h"
#include "SystemInput.h"
#include "SystemOutput.h"
#include "SystemCmd.h"
#include "SystemUartRSUtil.h"
#include "TestProc_boardID.H"
#include "SystemMsg.h"
//注意,堆栈八字节对齐

//是否使用真实设备调试,定义为0的话,调试时序,不需要真实设备
#define TASK_SERVICE_WITH_REAL_MACHINE               1

//时序调试的时候,每个动作的延迟时间
#define TASK_SERVICE_DELAY_TIME_MS                   10

//使能底层动作任务的调试信息输出
#define TASK_SERVICE_DEBUG_MSG_SHOW_ENABLE           1

//这个打印信息是动作任务在响应应用任务调用时候的输出信息
#if(TASK_SERVICE_DEBUG_MSG_SHOW_ENABLE == 1)
//输出任务调试讯息
#define TaskServiceSerialDebugMsgShowStart(msgStr,paramStr,paramVal1,paramVal2,paramVal3,paramVal4,paramVal5)       ActionPrintf("%s%s - %d - %d - %d - %d - %d\r\n",(uint8_t*)msgStr, \
                                                                                                                (uint8_t*)paramStr,paramVal1,paramVal2,paramVal3,paramVal4,paramVal5)
                                                                                                            
//输出任务结束讯息
#define TaskServiceSerialDebugMsgShowEnd(endStr,errorCode)     ActionPrintf("%s - 0x%08x\r\n",endStr,errorCode)
#else
#define TaskServiceSerialDebugMsgShowStart(msgStr,paramStr,paramVal1,paramVal2,paramVal3,paramVal4,paramVal5)       asm("nop")
#define TaskServiceSerialDebugMsgShowEnd(endStr,errorCode)         asm("nop")
#endif

//这个信息是当动作任务出错时候的出输出讯息
#if(TASK_SERVICE_DEBUG_MSG_SHOW_ENABLE == 1)
#define TaskServiceSerialErrMsgShow(errorCode)       ActionPrintf("!!Action Error Func : %s , Line : %d , ErrorCode : 0x%08x\r\n",__FUNCTION__,__LINE__,errorCode)
#else
#define TaskServiceSerialErrMsgShow(errorCode)       asm("nop")
#endif


/***************************************定义串口指令处理的函数原型*****************************************/
typedef LH_ERR (*TaskServiceProcPtrWithUartCommand)(SYSTEM_CMD_UART_RS1* uartCommandPtr,uint16_t* resultData);

/***************************************定义自身API数据处理的函数原型**************************************/
typedef LH_ERR (*TaskServicePtrWithSelfCommand)(SYSTEM_CMD_SELF* selfCommandPtr);


int8_t Arraypack_Common(SYSTEM_CMD_CAN1* canCommand,int32_t param,int32_t result);
/*********************************CAN2接收处理函数****************************************/
//CAN总线接收回调函数
void OS_Can2ReceiveDataCallFunc(MCU_CAN_RECEIVE_BUFFER_UNIT* unitRxMsg);
//任务名称
#define NAME_TASK_SERVICE_CAN2_RECV                                            "ServiceCan2Recv Task"
//任务优先级
#define PRIO_TASK_SERVICE_CAN2_RECV                                             1
//任务堆栈大小
#define STK_SIZE_TASK_SERVICE_CAN2_RECV                                         8 * 32
//任务堆栈
extern CPU_STK stackBufferTaskServiceCan2Recv[STK_SIZE_TASK_SERVICE_CAN2_RECV];
//任务控制块
extern OS_TCB tcbTaskServiceCan2Recv;
//任务函数
void TaskServiceFuncCan2Recv(void *p_arg);
/****************************************************************************************/

/*********************************CAN1接收处理函数****************************************/
//CAN总线接收回调函数
void OS_Can1ReceiveDataCallFunc(MCU_CAN_RECEIVE_BUFFER_UNIT* unitRxMsg);
//任务名称
#define NAME_TASK_SERVICE_CAN1_RECV                                            "ServiceCan1Recv Task"
//任务优先级
#define PRIO_TASK_SERVICE_CAN1_RECV                                             2
//任务堆栈大小
#define STK_SIZE_TASK_SERVICE_CAN1_RECV                                         8 * 32
//任务堆栈
extern CPU_STK stackBufferTaskServiceCan1Recv[STK_SIZE_TASK_SERVICE_CAN1_RECV];
//任务控制块
extern OS_TCB tcbTaskServiceCan1Recv;
//任务函数
void TaskServiceFuncCan1Recv(void *p_arg);
/****************************************************************************************/

/*********************************Uart Action接收处理函数*********************************/
//接收串口调试讯息
void BoardSerialRS1_RecvProcess(uint8_t recvDat);
//任务名称
#define NAME_TASK_SERVICE_UART3_RS1_RECV                                            "ServiceUart3RS1Recv Task"
//任务优先级
#define PRIO_TASK_SERVICE_UART3_RS1_RECV                                             3
//任务堆栈大小
#define STK_SIZE_TASK_SERVICE_UART3_RS1_RECV                                         8 * 32
//任务堆栈
extern CPU_STK stackBufferTaskServiceUart3RS1Recv[STK_SIZE_TASK_SERVICE_UART3_RS1_RECV];
//任务控制块
extern OS_TCB tcbTaskServiceUart3RS1Recv;
//任务函数
void TaskServiceFuncUart3RS1Recv(void *p_arg);
/****************************************************************************************/

/***********************************心跳函数*******************************************/
//任务名称
#define NAME_TASK_SERVICE_HEART                                                 "ServiceHeart Task"
//任务优先级
#define PRIO_TASK_SERVICE_HEART                                                 4
//任务堆栈大小
#define STK_SIZE_TASK_SERVICE_HEART                                             8 * 32
//任务堆栈
extern CPU_STK stackBufferTaskServiceHeart[STK_SIZE_TASK_SERVICE_HEART];
//任务控制块
extern OS_TCB tcbTaskServiceHeart;
//任务函数
void TaskServiceFuncHeart(void *p_arg);
/*************************************************************************************/

/***************************************温度控制任务**************************************/
//任务名称
#define NAME_TASK_SERVICE_TEMP_CTRL                                            "ServiceTempCtrl Task"
//任务优先级
#define PRIO_TASK_SERVICE_TEMP_CTRL                                             5
//任务堆栈大小
#define STK_SIZE_TASK_SERVICE_TEMP_CTRL                                         8 * 32
//任务堆栈
extern CPU_STK stackBufferTaskServiceTempCtrl[STK_SIZE_TASK_SERVICE_TEMP_CTRL];
//任务控制块
extern OS_TCB tcbTaskServiceTempCtrl;
//任务函数
void TaskServiceFuncTempCtrl(void *p_arg);
/****************************************************************************************/

/**************************************新杯栈与空杯栈管理任务******************************/
//任务名称
#define NAME_TASK_SERVICE_CUP_STACK                                            "ServiceCupStack Task"
//任务优先级
#define PRIO_TASK_SERVICE_CUP_STACK                                             6
//任务堆栈大小
#define STK_SIZE_TASK_SERVICE_CUP_STACK                                         8 * 32
//任务堆栈
extern CPU_STK stackBufferTaskServiceCupStack[STK_SIZE_TASK_SERVICE_CUP_STACK];
//任务控制块
extern OS_TCB tcbTaskServiceCupStackManagement;
//任务函数
void TaskServiceFuncCupStackManagement(void *p_arg);
/****************************************************************************************/

/********************************************调试任务*************************************/
//任务名称
#define NAME_TASK_SERVICE_DEBUG                                            "ServiceDebug Task"
//任务优先级
#define PRIO_TASK_SERVICE_DEBUG                                             7
//任务堆栈大小
#define STK_SIZE_TASK_SERVICE_DEBUG                                         8 * 32
//任务堆栈
extern CPU_STK stackBufferTaskServiceDebug[STK_SIZE_TASK_SERVICE_DEBUG];
//任务控制块
extern OS_TCB tcbTaskServiceDebug;
//任务函数
void TaskServiceFuncDebug(void *p_arg);
/****************************************************************************************/

/**************************************反应盘与试剂混匀任务****************************/
//任务名称
#define NAME_TASK_SERVICE_DISH_REACTION_MIX_REAGENT                               "ServiceDishReactionMixReagent Task"
//任务优先级
#define PRIO_TASK_SERVICE_DISH_REACTION_MIX_REAGENT                                8
//任务堆栈大小
#define STK_SIZE_TASK_SERVICE_DISH_REACTION_MIX_REAGENT                            8 * 32
//任务堆栈
extern CPU_STK stackBufferTaskServiceDishReactionMixReagent[STK_SIZE_TASK_SERVICE_DISH_REACTION_MIX_REAGENT];
//任务控制块
extern OS_TCB tcbTaskServiceDishReactionMixReagent;
//任务函数
void TaskServiceFuncDishReactionMixReagent(void *p_arg);
/****************************************************************************************/

/**************************************试剂盘任务************************************/
//任务名称
#define NAME_TASK_SERVICE_DISH_REAGENT                               "ServiceDishReagent Task"
//任务优先级
#define PRIO_TASK_SERVICE_DISH_REAGENT                                9
//任务堆栈大小
#define STK_SIZE_TASK_SERVICE_DISH_REAGENT                            8 * 32
//任务堆栈
extern CPU_STK stackBufferTaskServiceDishReagent[STK_SIZE_TASK_SERVICE_DISH_REAGENT];
//任务控制块
extern OS_TCB tcbTaskServiceDishReagent;
//任务函数
void TaskServiceFuncDishReagent(void *p_arg);
/****************************************************************************************/

/***************************************清洗盘与A液混匀任务***************************/
//任务名称
#define NAME_TASK_SERVICE_DISH_WASH_MIX_LIQUID_A                     "ServiceDishWashMixLiquidA Task"
//任务优先级
#define PRIO_TASK_SERVICE_DISH_WASH_MIX_LIQUID_A                      10
//任务堆栈大小
#define STK_SIZE_TASK_SERVICE_DISH_WASH_MIX_LIQUID_A                  8 * 32
//任务堆栈
extern CPU_STK stackBufferTaskServiceDishWashMixLiquidA[STK_SIZE_TASK_SERVICE_DISH_WASH_MIX_LIQUID_A];
//任务控制块
extern OS_TCB tcbTaskServiceDishWashMixLiquidA;
//任务函数
void TaskServiceFuncDishWashMixLiquidA(void *p_arg);
/****************************************************************************************/

/**************************************抓杯机械手任务****************************/
//任务名称
#define NAME_TASK_SERVICE_HAND_CUP                     "ServiceHandCup Task"
//任务优先级
#define PRIO_TASK_SERVICE_HAND_CUP                      11
//任务堆栈大小
#define STK_SIZE_TASK_SERVICE_HAND_CUP                  8 * 32
//任务堆栈
extern CPU_STK stackBufferTaskServiceHandCup[STK_SIZE_TASK_SERVICE_HAND_CUP];
//任务控制块
extern OS_TCB tcbTaskServiceHandCup;
//任务函数
void TaskServiceFuncHandCup(void *p_arg);
/****************************************************************************************/

/***************************************液路的检测任务***************************/
//任务名称
#define NAME_TASK_SERVICE_LIQUID_AUTO                     "ServiceLiquidAuto Task"
//任务优先级
#define PRIO_TASK_SERVICE_LIQUID_AUTO                      12
//任务堆栈大小
#define STK_SIZE_TASK_SERVICE_LIQUID_AUTO                  8 * 32
//任务堆栈
extern CPU_STK stackBufferTaskServiceLiquidAuto[STK_SIZE_TASK_SERVICE_LIQUID_AUTO];
//任务控制块
extern OS_TCB tcbTaskServiceLiquidAuto;
//任务函数
void TaskServiceFuncLiquidAuto(void *p_arg);
/****************************************************************************************/

/***************************************A液注液任务***************************/
//任务名称
#define NAME_TASK_SERVICE_LIQUID_INJECT_A                     "ServiceLiquidInjectA Task"
//任务优先级
#define PRIO_TASK_SERVICE_LIQUID_INJECT_A                      13
//任务堆栈大小
#define STK_SIZE_TASK_SERVICE_LIQUID_INJECT_A                  8 * 32
//任务堆栈
extern CPU_STK stackBufferTaskServiceLiquidInjectA[STK_SIZE_TASK_SERVICE_LIQUID_INJECT_A];
//任务控制块
extern OS_TCB tcbTaskServiceLiquidInjectA;
//任务函数
void TaskServiceFuncLiquidInjectA(void *p_arg);
/****************************************************************************************/

/***************************************测量室与B液注液任务***************************/
//任务名称
#define NAME_TASK_SERVICE_MEASURE_ROOM_AND_LIQUID_B                     "ServiceMeasureRoomAndLiquidB Task"
//任务优先级
#define PRIO_TASK_SERVICE_MEASURE_ROOM_AND_LIQUID_B                      14
//任务堆栈大小
#define STK_SIZE_TASK_SERVICE_MEASURE_ROOM_AND_LIQUID_B                  8 * 32
//任务堆栈
extern CPU_STK stackBufferTaskServiceMeasureRoomAndLiquidB[STK_SIZE_TASK_SERVICE_MEASURE_ROOM_AND_LIQUID_B];
//任务控制块
extern OS_TCB tcbTaskServiceMeasureRoomAndLiquidB;
//任务函数
void TaskServiceFuncMeasureRoomAndLiquidB(void *p_arg);
/****************************************************************************************/

/****************************************磁珠混匀任务**************************/
//任务名称
#define NAME_TASK_SERVICE_MIX_BEAD                     "ServiceMixBead Task"
//任务优先级
#define PRIO_TASK_SERVICE_MIX_BEAD                      15
//任务堆栈大小
#define STK_SIZE_TASK_SERVICE_MIX_BEAD                  8 * 32
//任务堆栈
extern CPU_STK stackBufferTaskServiceMixBead[STK_SIZE_TASK_SERVICE_MIX_BEAD];
//任务控制块
extern OS_TCB tcbTaskServiceMixBead;
//任务函数
void TaskServiceFuncMixBead(void *p_arg);
/****************************************************************************************/

/*****************************************试剂磁珠针任务*************************/
//任务名称
#define NAME_TASK_SERVICE_NEEDLE_REAGENT_BEAD                     "ServiceNeedleReagentBead Task"
//任务优先级
#define PRIO_TASK_SERVICE_NEEDLE_REAGENT_BEAD                      16
//任务堆栈大小
#define STK_SIZE_TASK_SERVICE_NEEDLE_REAGENT_BEAD                  8 * 32
//任务堆栈
extern CPU_STK stackBufferTaskServiceNeedleReagentBead[STK_SIZE_TASK_SERVICE_NEEDLE_REAGENT_BEAD];
//任务控制块
extern OS_TCB tcbTaskServiceNeedleReagentBead;
//任务函数
void TaskServiceFuncNeedleReagentBead(void *p_arg);
/****************************************************************************************/

/*****************************************样本针任务*************************/
//任务名称
#define NAME_TASK_SERVICE_NEEDLE_SAMPLE                     "ServiceNeedleSample Task"
//任务优先级
#define PRIO_TASK_SERVICE_NEEDLE_SAMPLE                      17
//任务堆栈大小
#define STK_SIZE_TASK_SERVICE_NEEDLE_SAMPLE                  8 * 32
//任务堆栈
extern CPU_STK stackBufferTaskServiceNeedleSample[STK_SIZE_TASK_SERVICE_NEEDLE_SAMPLE];
//任务控制块
extern OS_TCB tcbTaskServiceNeedleSample;
//任务函数
void TaskServiceFuncNeedleSample(void *p_arg);
/****************************************************************************************/

/*****************************************清洗针任务*************************/
//任务名称
#define NAME_TASK_SERVICE_NEEDLE_WASH                     "ServiceNeedleWash Task"
//任务优先级
#define PRIO_TASK_SERVICE_NEEDLE_WASH                      18
//任务堆栈大小
#define STK_SIZE_TASK_SERVICE_NEEDLE_WASH                  8 * 32
//任务堆栈
extern CPU_STK stackBufferTaskServiceNeedleWash[STK_SIZE_TASK_SERVICE_NEEDLE_WASH];
//任务控制块
extern OS_TCB tcbTaskServiceNeedleWash;
//任务函数
void TaskServiceFuncNeedleWash(void *p_arg);
/****************************************************************************************/

/*****************************************试剂盘试剂二维码扫描任务*************************/
//任务名称
#define NAME_TASK_SERVICE_QR_SCAN                     "ServiceQRScan Task"
//任务优先级
#define PRIO_TASK_SERVICE_QR_SCAN                      19
//任务堆栈大小
#define STK_SIZE_TASK_SERVICE_QR_SCAN                  8 * 32
//任务堆栈
extern CPU_STK stackBufferTaskServiceQRScan[STK_SIZE_TASK_SERVICE_QR_SCAN];
//任务控制块
extern OS_TCB tcbTaskServiceQRScan;
//任务函数
void TaskServiceFuncQRScan(void *p_arg);
/****************************************************************************************/

/*****************************************试管架传送带任务*******************************/
//任务名称
#define NAME_TASK_SERVICE_RANK_PATH_WAY                     "ServiceRankPathWay Task"
//任务优先级
#define PRIO_TASK_SERVICE_RANK_PATH_WAY                      20
//任务堆栈大小
#define STK_SIZE_TASK_SERVICE_RANK_PATH_WAY                  8 * 32
//任务堆栈
extern CPU_STK stackBufferTaskServiceRankPathWay[STK_SIZE_TASK_SERVICE_RANK_PATH_WAY];
//任务控制块
extern OS_TCB tcbTaskServiceRankPathWay;
//任务函数
void TaskServiceFuncRankPathWay(void *p_arg);
/****************************************************************************************/


#endif



