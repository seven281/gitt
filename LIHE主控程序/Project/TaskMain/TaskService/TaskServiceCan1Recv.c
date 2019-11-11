#include "TaskServiceConfig.h"
#include "TSC_Inc.h"
#include "ActionImplementInc.h"
#include "TaskAppConfig.h"
#include "TestProc_CanConfig.h"
#include "TestProc_Result.h"
#include "SystemCRC.h"
#include "SystemConfig.h"

typedef int32_t (*CAN_TX_CMD)(uint32_t, const uint8_t *, uint8_t);
#define APP_CAN_MAKE_EXTID(cmd, dest, src, cell, SrcCell)   (\
                   (uint32_t)((cmd)<<CAN_CMD_CMD_OFFSET)    |\
                   (uint32_t)((dest)<<CAN_CMD_DEST_OFFSET)  |\
                   (uint32_t)((src)<<CAN_CMD_SRC_OFFSET)    |\
                   (uint32_t)((cell)<<CAN_CMD_DST_CELL_OFFSET)    |\
                   (uint32_t)(SrcCell<<CAN_CMD_SRC_CELL_OFFSET))

//任务控制块,8字节对齐
#pragma pack(8)
//任务堆栈
CPU_STK stackBufferTaskServiceCan1Recv[STK_SIZE_TASK_SERVICE_CAN1_RECV];
#pragma pack()

OS_TCB tcbTaskServiceCan1Recv;

static CAN1_RECV_CACHE can1RecvCache@".ccram";

//static CAN1_ID_ANAYSIS_RESULT can1AnsysisID_Result;

//接收数据缓存,为了快速响应CAN中断
//static uint8_t can1CurrentRecvDataArray[8] = {0};
//接收数据长度
//static uint8_t can1CurrentRecvDataLength = 0;

//创建系统处理框架,将CAN口信息转发到指定的线程进行处理
//前一个是ID,后一个是转发处理的任务的堆栈
const CAN1_DISPATCH_UNIT can1DispatchArray[] = {
    /************************调试指令*********************************************该指令转发的信道地址***************/
    
    {CAN1_COMMAND_SM_RESET                                              ,        &tcbTaskServiceDebug       },      //透传指令    
    {CAN1_COMMAND_SM_RUN                                                ,        &tcbTaskServiceDebug       },      //透传指令    
    {CAN1_COMMAND_IO_READ                                               ,        &tcbTaskServiceDebug       },      //透传指令    
    {CAN1_COMMAND_IO_WRITE                                              ,        &tcbTaskServiceDebug       },      //透传指令    
    {CAN1_COMMAND_PARAM_READ                                            ,        &tcbTaskServiceDebug       },      //透传指令    
    {CAN1_COMMAND_PARAM_WRITE                                           ,        &tcbTaskServiceDebug       },      //透传指令    
    {CAN1_COMMAND_WASHDISH_RESET                                        ,        &tcbTaskServiceDishWashMixLiquidA},//清洗盘复位
    {CAN1_COMMAND_WASHDISH_MOVE_SPECIALCUP                              ,        &tcbTaskServiceDishWashMixLiquidA},//清洗盘旋转N个杯位
    {CAN1_COMMAND_WASHDISH_MOVE_TO_SPECIALCUP                           ,        &tcbTaskServiceDishWashMixLiquidA},//清洗盘旋转到指定杯位
    {CAN1_COMMAND_NEEDLEWASH_PUMP_RESET                                 ,        &tcbTaskServiceNeedleWash},//清洗针与泵复位
    {CAN1_COMMAND_NEEDLEWASH_PRIME                                      ,        &tcbTaskServiceNeedleWash},//清洗针灌注
    {CAN1_COMMAND_NEEDLEWASH_CLEAN                                      ,        &tcbTaskServiceNeedleWash},//清洗针磁珠清洗并返回
    {CAN1_COMMAND_LIQUID_A_INJECT                                       ,        &tcbTaskServiceLiquidInjectA},//A液注液
    {CAN1_COMMAND_LIQUID_A_INJECT_PRIME                                 ,        &tcbTaskServiceLiquidInjectA},//A液灌注
    {CAN1_COMMAND_LIQUID_A_INJECT_RESET                                 ,        &tcbTaskServiceLiquidInjectA},//A液泵初始化
    {CAN1_COMMAND_LIQUID_A_MIX_RESET                                    ,        &tcbTaskServiceDishWashMixLiquidA},//A液混匀初始化
    {CAN1_COMMAND_LIQUID_A_MIX_UP                                       ,        &tcbTaskServiceDishWashMixLiquidA},//A液上升
    {CAN1_COMMAND_LIQUID_A_MIX_DOWN                                     ,        &tcbTaskServiceDishWashMixLiquidA},//A液下降
    {CAN1_COMMAND_LIQUID_A_MIX_TIME                                     ,        &tcbTaskServiceDishWashMixLiquidA},//A液混匀指定时间
    {CAN1_COMMAND_LIQUID_A_MIX_AUTO                                     ,        &tcbTaskServiceDishWashMixLiquidA},//A液自动混匀
    {CAN1_COMMAND_MEASUREROOM_UP_RESET                                  ,        &tcbTaskServiceMeasureRoomAndLiquidB},//测量室上门初始化
    {CAN1_COMMAND_MEASUREROOM_UP_FULL_OPEN                              ,        &tcbTaskServiceMeasureRoomAndLiquidB},//测量室上门全开
    {CAN1_COMMAND_MEASUREROOM_UP_HALF_OPEN                              ,        &tcbTaskServiceMeasureRoomAndLiquidB},//测量室上门半开
    {CAN1_COMMAND_MEASUREROOM_UP_PRIME_OPEN                             ,        &tcbTaskServiceMeasureRoomAndLiquidB},//测量室上门开到灌注位 
    {CAN1_COMMAND_MEASUREROOM_UP_CLOSE                                  ,        &tcbTaskServiceMeasureRoomAndLiquidB},//测量室上门关闭
    {CAN1_COMMAND_LIQUID_B_RESET                                        ,        &tcbTaskServiceMeasureRoomAndLiquidB},//B液泵复位
    {CAN1_COMMAND_LIQUID_B_PRIME                                        ,        &tcbTaskServiceMeasureRoomAndLiquidB},//B液灌注
    {CAN1_COMMAND_LIQUID_B_INJECT                                       ,        &tcbTaskServiceMeasureRoomAndLiquidB},//B液注液
    {CAN1_COMMAND_MEASUREROOM_SIDE_RESET                                ,        &tcbTaskServiceMeasureRoomAndLiquidB},//测量室侧窗初始化
    {CAN1_COMMAND_MEASUREROOM_SIDE_OPEN                                 ,        &tcbTaskServiceMeasureRoomAndLiquidB},//测量室侧窗打开
    {CAN1_COMMAND_MEASUREROOM_SIDE_CLOSE                                ,        &tcbTaskServiceMeasureRoomAndLiquidB},//测量室侧窗关闭
    {CAN1_COMMAND_DARK_COUNT                                            ,        &tcbTaskServiceMeasureRoomAndLiquidB},//暗计数读数
    {CAN1_COMMAND_BASE_COUNT                                            ,        &tcbTaskServiceMeasureRoomAndLiquidB},//本底读数
    {CAN1_COMMAND_METER_COUNT                                           ,        &tcbTaskServiceMeasureRoomAndLiquidB},//测光值读数
    {CAN1_COMMAND_NEEDLE_REAGENT_VERTICAL_RESET                         ,        &tcbTaskServiceNeedleReagentBead},//试剂针垂直初始化
    {CAN1_COMMAND_NEEDLE_REAGENT_LEVEL_RESET                            ,        &tcbTaskServiceNeedleReagentBead},//试剂针水平初始化
    {CAN1_COMMAND_NEEDLE_REAGENT_RESET                                  ,        &tcbTaskServiceNeedleReagentBead},//试剂针垂直水平初始化
    {CAN1_COMMAND_NEEDLE_REAGENT_PUMP_RESET                             ,        &tcbTaskServiceNeedleReagentBead},//试剂针泵初始化
    {CAN1_COMMAND_NEEDLE_REAGENT_ROTATE_WASH                            ,        &tcbTaskServiceNeedleReagentBead},//转到清洗位
    {CAN1_COMMAND_NEEDLE_REAGENT_ROTATE_OUTSIDE                         ,        &tcbTaskServiceNeedleReagentBead},//转到试剂盘外圈
    {CAN1_COMMAND_NEEDLE_REAGENT_ROTATE_MIDDLE                          ,        &tcbTaskServiceNeedleReagentBead},//转到试剂盘中圈
    {CAN1_COMMAND_NEEDLE_REAGENT_ROTATE_INSIDE                          ,        &tcbTaskServiceNeedleReagentBead},//转到试剂盘内圈
    {CAN1_COMMAND_NEEDLE_REAGENT_ROTATE_REACTION                        ,        &tcbTaskServiceNeedleReagentBead},//转到反应盘
    {CAN1_COMMAND_NEEDLE_REAGENT_ABSORBENT                              ,        &tcbTaskServiceNeedleReagentBead},//吸试剂
    {CAN1_COMMAND_NEEDLE_REAGENT_EXCRETION                              ,        &tcbTaskServiceNeedleReagentBead},//排试剂
    {CAN1_COMMAND_NEEDLE_REAGENT_WASH                                   ,        &tcbTaskServiceNeedleReagentBead},//清洗
    {CAN1_COMMAND_NEEDLE_REAGENT_PRIME                                  ,        &tcbTaskServiceNeedleReagentBead},//灌注
    {CAN1_COMMAND_CUP_REAGENT_RESET                                     ,        &tcbTaskServiceDishReagent},//试剂盘初始化
    {CAN1_COMMAND_CUP_REAGENT_ROTATE_TO_SPECIFIED_PS                    ,        &tcbTaskServiceDishReagent},//试剂盘旋转到指定位置
    {CAN1_COMMAND_CUP_REAGENT_ROTATE_N_CUP_POS                          ,        &tcbTaskServiceDishReagent},//试剂盘旋转N个杯位
    {CAN1_COMMAND_REAGENT_MIX_RESET                                     ,        &tcbTaskServiceDishReactionMixReagent},//试剂混匀初始化
    {CAN1_COMMAND_REAGENT_MIX_UP                                        ,        &tcbTaskServiceDishReactionMixReagent},//试剂混匀上升
    {CAN1_COMMAND_REAGENT_MIX_DOWN                                      ,        &tcbTaskServiceDishReactionMixReagent},//试剂混匀下降
    {CAN1_COMMAND_REAGENT_MIX                                           ,        &tcbTaskServiceDishReactionMixReagent},//试剂混匀
    {CAN1_COMMAND_BEAD_MIX_RESET                                        ,        &tcbTaskServiceMixBead},//磁珠混匀初始化
    {CAN1_COMMAND_BEAD_MIX_UP                                           ,        &tcbTaskServiceMixBead}, //磁珠混匀上升
    {CAN1_COMMAND_BEAD_MIX_DOWN                                         ,        &tcbTaskServiceMixBead},//磁珠混匀下降
    {CAN1_COMMAND_BEAD_MIX                                              ,        &tcbTaskServiceMixBead},//磁珠混匀
    {CAN1_COMMAND_REACTION_DISH_RESET                                   ,        &tcbTaskServiceDishReactionMixReagent},//反应盘初始化
    {CAN1_COMMAND_REACTION_DISH_ROTATE_TO_SPECIALPOS                    ,        &tcbTaskServiceDishReactionMixReagent},//反应盘旋转到指定杯位
    {CAN1_COMMAND_REACTION_DISH_ROTAT_SPECIALPOS                        ,        &tcbTaskServiceDishReactionMixReagent},//反应盘旋转N个杯位
    {CAN1_COMMAND_NEEDLE_SAMPLE_VERTICAL_RESET                          ,        &tcbTaskServiceNeedleSample},//样本针垂直初始化
    {CAN1_COMMAND_NEEDLE_SAMPLE_LEVEL_RESET                             ,        &tcbTaskServiceNeedleSample},//样本针水平初始化
    {CAN1_COMMAND_NEEDLE_SAMPLE_VERTICAL_LEVEL_INIT                     ,        &tcbTaskServiceNeedleSample},//垂直水平初始化
    {CAN1_COMMAND_NEEDLE_SAMPLE_PUMP_RESET                              ,        &tcbTaskServiceNeedleSample},//泵初始化
    {CAN1_COMMAND_NEEDLE_SAMPLE_WASH_POS                                ,        &tcbTaskServiceNeedleSample},//样本针转到清洗位
    {CAN1_COMMAND_NEEDLE_SAMPLE_FORCE_WASH_POS                          ,        &tcbTaskServiceNeedleSample},//样本针转到强洗位
    {CAN1_COMMAND_NEEDLE_SAMPLE_SAMPLING_POS                            ,        &tcbTaskServiceNeedleSample},//样本针转到吸样本位
    {CAN1_COMMAND_NEEDLE_SAMPLE_SAMPLE_FRAME                            ,        &tcbTaskServiceNeedleSample},//样本针转到样本架
    {CAN1_COMMAND_NEEDLE_SAMPLE_PUSH_POS                                ,        &tcbTaskServiceNeedleSample},//转到反应盘样本位上
    {CAN1_COMMAND_NEEDLE_SAMPLE_DILUTION_POS                            ,        &tcbTaskServiceNeedleSample},//样本针转到反应盘稀释样本位上
    {CAN1_COMMAND_NEEDLE_SAMPLE_ABSORB                                  ,        &tcbTaskServiceNeedleSample},//样本针吸样本
    {CAN1_COMMAND_NEEDLE_SAMPLE_ROW                                     ,        &tcbTaskServiceNeedleSample},//样本针排样本
    {CAN1_COMMAND_NEEDLE_SAMPLE_WASH                                    ,        &tcbTaskServiceNeedleSample},//样本针清洗
    {CAN1_COMMAND_NEEDLE_SAMPLE_FORCEWASH                               ,        &tcbTaskServiceNeedleSample},//样本针强清洗
    {CAN1_COMMAND_NEEDLE_SAMPLE_PRIME                                   ,        &tcbTaskServiceNeedleSample},//样本针灌注
    {CAN1_COMMAND_NEEDLE_SAMPLE_FORCEPRIME                              ,        &tcbTaskServiceNeedleSample},//样本针强灌注
    {CAN1_COMMAND_MANIPULATOR_VERTICAL_RESET                            ,        &tcbTaskServiceHandCup},//垂直复位  
    {CAN1_COMMAND_MANIPULATOR_LEVEL_ABOUT_RESET                         ,        &tcbTaskServiceHandCup},//水平左右复位
    {CAN1_COMMAND_MANIPULATOR_LEVEL_AROUND_RESET                        ,        &tcbTaskServiceHandCup},//水平前后复位
    {CAN1_COMMAND_MANIPULATOR_LEVEL_RESET                               ,        &tcbTaskServiceHandCup},//水平复位  
    {CAN1_COMMAND_MANIPULATOR_VERTICAL_LEVEL                            ,        &tcbTaskServiceHandCup},//垂直水平复位
    {CAN1_COMMAND_MANIPULATOR_MOVE_REACTION                             ,        &tcbTaskServiceHandCup},//移到反应盘上方
    {CAN1_COMMAND_MANIPULATOR_NEW_CUP                                   ,        &tcbTaskServiceHandCup},//移到新杯装载上方
    {CAN1_COMMAND_MANIPULATOR_MOVE_TRASH                                ,        &tcbTaskServiceHandCup},//移到垃圾桶上方
    {CAN1_COMMAND_MANIPULATOR_CLEAN_TRAY                                ,        &tcbTaskServiceHandCup},//移到清洗盘上方
    {CAN1_COMMAND_MANIPULATOR_MEASURE_ROOM                              ,        &tcbTaskServiceHandCup},//移到测量室上方
    {CAN1_COMMAND_MANIPULATOR_REACTION_PUTCUP                           ,        &tcbTaskServiceHandCup},//向反应盘放杯
    {CAN1_COMMAND_MANIPULATOR_TRASH_PUTCUP                              ,        &tcbTaskServiceHandCup},//向垃圾箱放杯
    {CAN1_COMMAND_MANIPULATOR_REATION_CATCHCUP                          ,        &tcbTaskServiceHandCup},//从反应盘取杯 
    {CAN1_COMMAND_MANIPULATOR_NEW_CATCHCUP                              ,        &tcbTaskServiceHandCup},//从新杯区取杯
    {CAN1_COMMAND_MANIPULATOR_CLEAN_PUTCUP                              ,        &tcbTaskServiceHandCup},//向清洗盘放杯
    {CAN1_COMMAND_MANIPULATOR_CLEAN_CATCHCUP                            ,        &tcbTaskServiceHandCup},//从清洗盘取杯
    {CAN1_COMMAND_MANIPULATOR_MEASURE_PUTCUP                            ,        &tcbTaskServiceHandCup},//向测量室放杯
    {CAN1_COMMAND_MANIPULATOR_MEASURE_CATCHCUP                          ,        &tcbTaskServiceHandCup},//从测量室取杯
    {CAN1_COMMAND_FULL_CUP_STACK_RESET                                  ,        &tcbTaskServiceCupStackManagement},//反应杯装载新杯栈初始化 
    {CAN1_COMMAND_EMPTY_CUP_STACK_RESET                                 ,        &tcbTaskServiceCupStackManagement},//反应杯装载空架栈初始化 
    {CAN1_COMMAND_PUSH_HAND_RESET                                       ,        &tcbTaskServiceCupStackManagement},//反应杯装载推手初始化  
    {CAN1_COMMAND_GRIPPER_RESET                                         ,        &tcbTaskServiceCupStackManagement},//反应杯装载夹持机构初始化
    {CAN1_COMMAND_STACK_GRIPPER_PUSHHAND_RESET                          ,        &tcbTaskServiceCupStackManagement},//反应杯装载初始化全部  
    {CAN1_COMMAND_GRIPPER_OPEN                                          ,        &tcbTaskServiceCupStackManagement},//反应杯装载夹手打开   
    {CAN1_COMMAND_GRIPPER_CLOSE                                         ,        &tcbTaskServiceCupStackManagement},//反应杯装载夹手夹紧   
    {CAN1_COMMAND_FAR_PUSH_HAND_NEW_CUP                                 ,        &tcbTaskServiceCupStackManagement},//反应杯装载长传一个新杯 
    {CAN1_COMMAND_PUSHONCE_NEW_CUP                                      ,        &tcbTaskServiceCupStackManagement},//反应杯装载推一次新杯  
    {CAN1_COMMAND_UNDERTAKE_NEW_CUP                                     ,        &tcbTaskServiceCupStackManagement},//反应杯装载承接一次空杯 
    {CAN1_COMMAND_TRASH_LOCK                                            ,        &tcbTaskServiceCupStackManagement},//反应杯装载垃圾桶锁定  
    {CAN1_COMMAND_TRASH_UNLOCK                                          ,        &tcbTaskServiceCupStackManagement},//反应杯装载垃圾桶解锁  
    {CAN1_COMMAND_SWEEP_CODE                                            ,        &tcbTaskServiceQRScan},//二维码扫码
    {CAN1_COMMAND_SAMPLE_CONVEYOR_RESET                                 ,        &tcbTaskServiceRankPathWay},//加样传送带初始化
    {CAN1_COMMAND_SAMPLE_CONVEYOR_MOVE_TO_SAMPLE                        ,        &tcbTaskServiceRankPathWay},//移到吸样位
    {CAN1_COMMAND_SAMPLE_CONVEYOR_RETURN_SAMPLE                         ,        &tcbTaskServiceRankPathWay},//退回样本架
};

//转发数组的长度
#define DISPATCH_ELEMENT_LENGTH                         sizeof(can1DispatchArray)/sizeof(can1DispatchArray[0])


static TESTPROC_CAN_MULTY_T g_stCanCommMulty;   // 中位机多包数据
static OS_MUTEX g_stMutexForCan1;

static int32_t CanFrameProcess(CanRxMsg *pstCanFrame);
//CAN总线接收回调函数
void OS_Can1ReceiveDataCallFunc(MCU_CAN_RECEIVE_BUFFER_UNIT* unitRxMsg)
{
    //将数据转发到处理线程
    OS_ERR err;

    //将申请的数据转发到协议处理线程
    OSTaskQPost((OS_TCB *)&tcbTaskServiceCan1Recv, //向协议处理线程发送消息
                (void *)unitRxMsg,
                (OS_MSG_SIZE)sizeof(MCU_CAN_RECEIVE_BUFFER_UNIT*),
                (OS_OPT)OS_OPT_POST_FIFO,
                (OS_ERR *)&err);

    //检测是否发送成功
    if (err != OS_ERR_NONE)
    {
        //发送失败
        unitRxMsg->waitProcessFlag = 0;
    }
}

//任务函数
void TaskServiceFuncCan1Recv(void *p_arg)
{
    OS_ERR err;
    p_arg = p_arg;
    //任务接收数据长度
	OS_MSG_SIZE size;
	//接收的数据指针
	MCU_CAN_RECEIVE_BUFFER_UNIT *unitRxMsg = NULL;
    //接收到的数据
    //CanRxMsg can1RecvMsg;
    
    OSMutexCreate(&g_stMutexForCan1, "MutexForCan1", &err);

    while(1)
    {
        //请求消息,堵塞访问,协议处理任务必须实时性极高
        unitRxMsg = OSTaskQPend((OS_TICK)0,
                                (OS_OPT)OS_OPT_PEND_BLOCKING,
                                (OS_MSG_SIZE *)&size,
                                (CPU_TS *)0,
                                (OS_ERR *)&err);

        if ((unitRxMsg != NULL)&&(err == OS_ERR_NONE))
        {
            //DebugMsg("Can1Rx:%08X,%d\r\n", unitRxMsg->rxMsg.ExtId, unitRxMsg->rxMsg.DLC);
            CanFrameProcess(&unitRxMsg->rxMsg);            
            //处理完成,清除等待处理标志
            unitRxMsg->waitProcessFlag = 0;
            unitRxMsg = NULL;
        }
    }
}


/****************************************************************************
 *
 * Function Name:    CanMultyPacketed
 * Input:            pstMulty - 多包数据结构
 *                   u16Cmd - 多包数据命令(CAN_CMD_MULTYHEAD,CAN_CMD_MULTYPACKAGE,CAN_CMD_MULTYEND)
 *                   pData - CAN帧数据
 *                   u8DataSize - CAN帧数据长度
 * Output:           None
 *                   
 * Returns:          TESTPROC_RESULT_OK - 成功
 *                   TESTPROC_RESULT_ERR_PARAM - 参数错误
 *                   TESTPROC_RESULT_ERR_MEM - 获取内存失败
 *                   TESTPROC_RESULT_ERR_CMD - 不支持的命令
 * Description:      封装多包数据
 * Note:             
 ***************************************************************************/
static int32_t CanMultyPacketed(TESTPROC_CAN_MULTY_T *pstMulty, uint16_t u16Cmd, const uint8_t *pData, uint8_t u8DataSize)
{
    int32_t s32Ret = TESTPROC_RESULT_OK;
    uint16_t u16Offset = 0;
    uint16_t u16CalcCRC16 = 0xFFFF;
    OS_ERR	 err;

    if (NULL == pstMulty || NULL == pData || u8DataSize > 8)
    {
        return TESTPROC_RESULT_ERR_PARAM;
    }

    if (NULL == pstMulty->pstCmd)
    {
        DebugMsg("ERR>CanMulitParam\r\n");
        return TESTPROC_RESULT_ERR_CMD;
    }
	
    switch(u16Cmd)
    {
        case CAN_CMD_MULTYHEAD:
            pstMulty->u16DataCount = 0;
            pstMulty->u16CRC = 0;
            pstMulty->pstCmd->pu8DataBuf = TestProc_AllocMemory(pstMulty->pstCmd->u16Size);
            //DebugMsg("CAN_MULIT_HEAD:%04X,%d,%08X\r\n", pstMulty->pstCmd->u16Cmd, pstMulty->pstCmd->u16Size, pstMulty->pstCmd->pu8DataBuf);
            if (NULL == pstMulty->pstCmd->pu8DataBuf)
            {
                DebugMsg("ERR>CanMulty(0x%04X),MemOut:%d,%d\r\n", pstMulty->pstCmd->u16Cmd, pstMulty->pstCmd->u16Size, &err);
                s32Ret = TESTPROC_RESULT_ERR_MEM;
            }
            break;
        case CAN_CMD_MULTYPACKAGE:
            u16Offset = BYTE_TO_HWORD(pData[1], pData[0]);
            //DebugMsg("CAN_MULIT_DATA:%d,%d\r\n", u16Offset, u8DataSize);
            if (NULL != pstMulty->pstCmd->pu8DataBuf)
            {
                if ((u16Offset*CAN_CMD_MULTY_FRAME_SIZE) < pstMulty->pstCmd->u16Size)
                {
                    Mem_Copy(pstMulty->pstCmd->pu8DataBuf+u16Offset*CAN_CMD_MULTY_FRAME_SIZE, pData+2, u8DataSize-2);
                    pstMulty->u16DataCount += (u8DataSize-2);                    
                }
                else
                {
                    DebugMsg("ERR>CanMultyOffset:%d\r\n", u16Offset);
                    s32Ret = TESTPROC_RESULT_ERR_LEN;
                }
            }
            else
            {
                DebugMsg("ERR>CanMultyPkg:%d\r\n", u16Offset);
                s32Ret = TESTPROC_RESULT_ERR_MEM;
            }
            break;
        case CAN_CMD_MULTYEND:
            if (NULL != pstMulty->pstCmd->pu8DataBuf)
            {
                if (pstMulty->u16DataCount != pstMulty->pstCmd->u16Size)
                {
                    DebugMsg("ERR>CanMultyLenMatch:%04X,%d,%d\r\n", pstMulty->pstCmd->u16Cmd, pstMulty->u16DataCount, pstMulty->pstCmd->u16Size);
                    s32Ret = TESTPROC_RESULT_ERR_LEN;					
                }

                if (TESTPROC_RESULT_OK == s32Ret)
                {
                    //DebugMsg("Can_MulitData:%02X,%02X,%d,%04X,%d\r\n", pstMulty->pstCmd->u8DestBoard, pstMulty->pstCmd->u8SrcBoard, pstMulty->pstCmd->u8CellNmbr, pstMulty->pstCmd->u16Cmd, pstMulty->pstCmd->u16Size);
                    //for (i=0; i<pstMulty->pstCmd->u16Size; i++) DebugMsg("%02X ", pstMulty->pstCmd->pu8DataBuf[i]);
                    //DebugMsg("\r\n");
                    pstMulty->u16CRC = BYTE_TO_HWORD(pData[3], pData[2]);
                    // check crc16
                    u16CalcCRC16 = SystemCRC_calcCRC16(pstMulty->pstCmd->pu8DataBuf, pstMulty->pstCmd->u16Size);
                    if (u16CalcCRC16 != pstMulty->u16CRC)
                    {
                        DebugMsg("ERR>CanMultyCRC:%04X,%04X\r\n", u16CalcCRC16, pstMulty->u16CRC);
                        s32Ret = TESTPROC_RESULT_ERR_CRC;
                    }
                }

                if (TESTPROC_RESULT_OK == s32Ret)
                {                    
                    TESTPROC_MSM_T *pstMsg;
                    pstMsg = TestProc_AllocMemory(sizeof(TESTPROC_MSM_T));
                    if (pstMsg != NULL)
                    {
                        pstMsg->pData = pstMulty->pstCmd;
                        pstMsg->u16DataSize = sizeof(TESTPROC_MSM_T);
                        pstMsg->eSrcUnit  = TESTPROC_UNIT_CAN1;
                        pstMsg->eDestUnit = TESTPROC_UNIT_PROCTEST;
                        OSTaskQPost((OS_TCB *)&tcbTaskAppTestProcess,
                                    (void *)pstMsg,
                                    (OS_MSG_SIZE)sizeof(TESTPROC_MSM_T),
                                    (OS_OPT)OS_OPT_POST_FIFO,
                                    (OS_ERR *)&err);
                        //DebugMsg("Can_MulitData:%02X,%02X,%d,%04X,%d\r\n", pstMulty->pstCmd->u8DestBoard, pstMulty->pstCmd->u8SrcBoard, pstMulty->pstCmd->u8CellNmbr, pstMulty->pstCmd->u16Cmd, pstMulty->pstCmd->u16Size);
                        pstMulty->pstCmd = NULL;
                    }
                    else
                    {
                        s32Ret = TESTPROC_RESULT_ERR_MSGMEM;
                    }
                }			
            }
            else
            {
                s32Ret = TESTPROC_RESULT_ERR_MEM;
            }
            break;
        default:
            s32Ret = TESTPROC_RESULT_ERR_CMD;
            break;		
    }
    if (TESTPROC_RESULT_OK != s32Ret)
    {
        DebugMsg("ERR>CanMultyEnd:%d,%04X,%04X,%d\r\n", s32Ret, u16Cmd, pstMulty->pstCmd->u16Cmd, pstMulty->u16DataCount);
        if (pstMulty->pstCmd != NULL)
        {
            if (pstMulty->pstCmd->pu8DataBuf != NULL)
            {
                TestProc_FreeMemory(pstMulty->pstCmd->pu8DataBuf);
            }
            TestProc_FreeMemory(pstMulty->pstCmd);
            pstMulty->pstCmd = NULL;
        }
    }

    return s32Ret;
}

/****************************************************************************
 *
 * Function Name:    CanFrameProcess
 * Input:            pstHead - can帧头
 *                   pData - 包体
 * Output:           None
 *                   
 * Returns:          TESTPROC_RESULT_OK - 成功
 *                   TESTPROC_RESULT_ERR_PARAM - 参数错误
 *                   TESTPROC_RESULT_ERR_MEM - 获取内存失败
 * Description:      处理模块CAN总线的数据
 * Note:             
 ***************************************************************************/
static int32_t CanFrameProcess(CanRxMsg *pstCanFrame)
{
    int32_t  s32Ret = 0;
    TESTPROC_CAN_MULTY_T *pstMulty;    
    TESTPROC_CAN_CMD_T *pstCmd;
    TESTPROC_MSM_T *pstMsg;
    //uint8_t  u8DataBuf[32];
    OS_ERR   err;
    uint16_t u16Cmd;
    uint8_t  u8SrcBoard, u8CellNmbr;

    if (NULL == pstCanFrame)
    {
        return TESTPROC_RESULT_ERR_PARAM;
    }
    u16Cmd = (uint16_t)((pstCanFrame->ExtId >>CAN_CMD_CMD_OFFSET)&CAN_CMD_COMMAND_MASK);
    u8SrcBoard  = (uint8_t )((pstCanFrame->ExtId>>CAN_CMD_SRC_OFFSET)&CAN_CMD_SRC_BOARD_MASK);
    u8CellNmbr = (uint8_t )((pstCanFrame->ExtId>>CAN_CMD_SRC_CELL_OFFSET)&CAN_CMD_SRC_CELL_MASK);
    //DebugMsg("CanFrame:%d,%04X,%02X\r\n", u8CellNmbr, u16Cmd, u8SrcBoard);
    switch(u16Cmd)
    {
        case CAN_CMD_MULTYHEAD:
            pstCmd = TestProc_AllocMemory(sizeof(TESTPROC_CAN_CMD_T));
            if (NULL == pstCmd)
            {
                DebugMsg("**ERR>CanCmdNoMemory\r\n**");
                s32Ret = TESTPROC_RESULT_ERR_MEM;
            }
            else
            {
                pstCmd->pu8DataBuf   = NULL;
                pstCmd->u16Cmd       = BYTE_TO_HWORD(pstCanFrame->Data[1], pstCanFrame->Data[0]);
                pstCmd->u16Size      = BYTE_TO_HWORD(pstCanFrame->Data[3], pstCanFrame->Data[2]);
                pstCmd->u8DestBoard  = (uint8_t )((pstCanFrame->ExtId>>CAN_CMD_DEST_OFFSET)&CAN_CMD_DST_BOARD_MASK);
                pstCmd->u8SrcBoard   = u8SrcBoard;
                pstCmd->u8CellNmbr   = u8CellNmbr;
                pstMulty = NULL;
                if (TESTPROC_CELL_PLATFORM_NMBR == u8CellNmbr)
                {
                    if (BOARD_ID_TRACK_COMM == u8SrcBoard)
                    {
                        pstMulty = &g_stCanCommMulty;
                    }
                }
                
                if (NULL != pstMulty)
                {
                    if (pstMulty->pstCmd != NULL)
                    {
                        TestProc_FreeMemory(pstMulty->pstCmd);
                    }
                    //DebugMsg("MultyHead:%d,%04X,%02X,%02X,%d\r\n", pstCmd->u8CellNmbr, pstCmd->u16Cmd, pstCmd->u8SrcBoard, pstCmd->u8DestBoard, pstCmd->u16Size);
                    pstMulty->pstCmd = pstCmd;
                    s32Ret = CanMultyPacketed(pstMulty, u16Cmd, pstCanFrame->Data, pstCanFrame->DLC);
                }
                else
                {
                    DebugMsg("ERR>MultyHead:%d,%04X,%02X,%02X\r\n", pstCmd->u8CellNmbr, u16Cmd, pstCmd->u8SrcBoard, pstCmd->u8DestBoard);
                    TestProc_FreeMemory(pstCmd);
                    s32Ret = TESTPROC_RESULT_ERR_STATUS;
                }
            }
            break;
        case CAN_CMD_MULTYPACKAGE:
        case CAN_CMD_MULTYEND:
            pstMulty = NULL;
            if (TESTPROC_CELL_PLATFORM_NMBR == u8CellNmbr)
            {
                if (BOARD_ID_TRACK_COMM == u8SrcBoard)
                {
                    pstMulty = &g_stCanCommMulty;
                }
            }
            
            if (NULL != pstMulty)
            {
                s32Ret = CanMultyPacketed(pstMulty, u16Cmd, pstCanFrame->Data, pstCanFrame->DLC);
            }
            break;
        default:
            // Single Frame
            pstCmd = TestProc_AllocMemory(sizeof(TESTPROC_CAN_CMD_T));
            if (NULL == pstCmd)
            {
                DebugMsg("**ERR>CanCmdNoMemory\r\n**");
                s32Ret = TESTPROC_RESULT_ERR_MEM;
            }
            else
            {
                pstCmd->pu8DataBuf   = NULL;
                pstCmd->u16Cmd       = u16Cmd;
                pstCmd->u8DestBoard  = (uint8_t )((pstCanFrame->ExtId>>CAN_CMD_DEST_OFFSET)&CAN_CMD_DST_BOARD_MASK);
                pstCmd->u8SrcBoard   = u8SrcBoard;
                pstCmd->u8CellNmbr   = u8CellNmbr;
                pstCmd->u16Size      = pstCanFrame->DLC > 8 ? 8 : pstCanFrame->DLC;
                Mem_Copy(pstCmd->u8DataBuf, pstCanFrame->Data, pstCmd->u16Size);
                pstMsg = TestProc_AllocMemory(sizeof(TESTPROC_MSM_T));
                if (pstMsg != NULL)
                {
                    //DebugMsg("PostMsg:%p,%p\r\n", pstMsg, pstCmd);
                    pstMsg->pData = pstCmd;
                    pstMsg->u16DataSize = sizeof(TESTPROC_MSM_T);
                    pstMsg->eSrcUnit  = TESTPROC_UNIT_CAN1;
                    pstMsg->eDestUnit = TESTPROC_UNIT_PROCTEST;
                    OSTaskQPost((OS_TCB *)&tcbTaskAppTestProcess, //向协议处理线程发送消息
                                (void *)pstMsg,
                                (OS_MSG_SIZE)sizeof(TESTPROC_MSM_T),
                                (OS_OPT)OS_OPT_POST_FIFO,
                                (OS_ERR *)&err);
                }
                else
                {
                    s32Ret = TESTPROC_RESULT_ERR_MSGMEM;
                    TestProc_FreeMemory(pstCmd);
                }
            }
            break;
    }

    if (TESTPROC_RESULT_OK != s32Ret)
    {
        DebugMsg("ERR>CanProCell:%04X,%02X,%d,%d\r\n", u16Cmd, u8SrcBoard, u8CellNmbr, s32Ret);
    }

    return s32Ret;	
}

/****************************************************************************
 *
 * Function Name:    Can1SendFrame
 * Input:            u8Dect - Target Board ID
 *                   u16Cmd - Command
 *                   pBody - Data Buffer
 *                   u8Size - Data Size
 * Output:           None
 *                   
 * Returns:          TESTPROC_RESULT_OK,TESTPROC_RESULT_ERR_PARAM
 * Description:      
 * Note:             
 ***************************************************************************/
static int32_t Can1SendFrame(uint32_t u32ExtID, const uint8_t *pBody, uint8_t u8Size)
{
    int32_t  ret = TESTPROC_RESULT_OK;
    CanTxMsg txMsg;

    if (u8Size > 8) 
    {
        return TESTPROC_RESULT_ERR_PARAM;
    }
    txMsg.ExtId = u32ExtID;
    txMsg.IDE = CAN_ID_EXT;
    txMsg.DLC = u8Size;
    txMsg.RTR = CAN_RTR_DATA;
    Mem_Copy(txMsg.Data, pBody, u8Size);
    //DebugMsg("txCAN_1:%04X,%d,%d,%02X\r\n", u32ExtID, txMsg.DLC, u8Size, u8Size > 0 ? txBuf[0] : 0);
    MCU_CAN1_TransMessage(&txMsg);
	
    return ret;
}

static int32_t CanSendData(CAN_TX_CMD pFunc, uint8_t u8CellNmbr, uint8_t u8Dest, uint16_t u16Cmd, const uint8_t *pu8Body, uint16_t u16Size)
{
    int32_t  s32Ret = TESTPROC_RESULT_OK;
    uint32_t u32ExtID = 0;
    uint8_t  u8SendData[8];
    const uint8_t  u8SrcCell = SystemConfig_GetCellNumber();

    if (u16Size <= 8)
    {
        u32ExtID = APP_CAN_MAKE_EXTID(u16Cmd, u8Dest, BOARD_ID_CELL_MAIN, u8CellNmbr, u8SrcCell);
        s32Ret = pFunc(u32ExtID, pu8Body, (uint8_t)u16Size);
    }
    else
    {
        // send head
        u8SendData[0] = HWORD_TO_BYTE_L(u16Cmd);
        u8SendData[1] = HWORD_TO_BYTE_H(u16Cmd);
        u8SendData[2] = HWORD_TO_BYTE_L(u16Size);
        u8SendData[3] = HWORD_TO_BYTE_H(u16Size);
        u32ExtID = APP_CAN_MAKE_EXTID(CAN_CMD_MULTYHEAD, u8Dest, BOARD_ID_CELL_MAIN, u8CellNmbr, u8SrcCell);
        s32Ret = pFunc(u32ExtID, u8SendData, 4);
	
        // send Data
        if (TESTPROC_RESULT_OK == s32Ret)
        {
            uint16_t i;
            uint16_t u16DataOffset;
            uint16_t u16SendQuantity = (u16Size/CAN_CMD_MULTY_FRAME_SIZE);

            u32ExtID = APP_CAN_MAKE_EXTID(CAN_CMD_MULTYPACKAGE, u8Dest, BOARD_ID_CELL_MAIN, u8CellNmbr, u8SrcCell);
            u16DataOffset = 0;			
            for (i=0; i<u16SendQuantity; i++)
            {
                u8SendData[0] = HWORD_TO_BYTE_L(i);
                u8SendData[1] = HWORD_TO_BYTE_H(i);
                u8SendData[2] = pu8Body[u16DataOffset++];
                u8SendData[3] = pu8Body[u16DataOffset++];
                u8SendData[4] = pu8Body[u16DataOffset++];
                u8SendData[5] = pu8Body[u16DataOffset++];
                u8SendData[6] = pu8Body[u16DataOffset++];
                u8SendData[7] = pu8Body[u16DataOffset++];
                s32Ret = pFunc(u32ExtID, u8SendData, 8);
                if (TESTPROC_RESULT_OK != s32Ret)
                {
                    DebugMsg("ERR>CanSendMultyData:%d\r\n", s32Ret);
                }
            }
            // send last one if necessary
            u16SendQuantity = (u16Size%CAN_CMD_MULTY_FRAME_SIZE);	
            if (u16SendQuantity)
            {
                u8SendData[0] = HWORD_TO_BYTE_L(i);
                u8SendData[1] = HWORD_TO_BYTE_H(i);
                for (i=0; i<u16SendQuantity; i++)
                {
                    u8SendData[i+2] = pu8Body[u16DataOffset++];
                }
                s32Ret = pFunc(u32ExtID, u8SendData, u16SendQuantity+2);
                if (TESTPROC_RESULT_OK != s32Ret)
                {
                    DebugMsg("ERR>CanSendMultyData2:%d\r\n", s32Ret);
                }
            }
        }
        else
        {
            DebugMsg("ERR>CanTxMultyHead:%d\r\n", s32Ret);
        }

        // send end
        if(TESTPROC_RESULT_OK == s32Ret)
        {
            uint16_t u16CRC = SystemCRC_calcCRC16(pu8Body, u16Size);
		
            u8SendData[0] = HWORD_TO_BYTE_L(u16Cmd);
            u8SendData[1] = HWORD_TO_BYTE_H(u16Cmd);
            u8SendData[2] = HWORD_TO_BYTE_L(u16CRC);
            u8SendData[3] = HWORD_TO_BYTE_H(u16CRC);
            u32ExtID = APP_CAN_MAKE_EXTID(CAN_CMD_MULTYEND, u8Dest, BOARD_ID_CELL_MAIN, u8CellNmbr, u8SrcCell);
            s32Ret = pFunc(u32ExtID, u8SendData, 4);
        }
    }

    return s32Ret;
}

/****************************************************************************
 *
 * Function Name:    app_canSendDataToCell
 * Input:            u8CellNmbr - Cell Number
 *                   u8Dect - Target Board ID
 *                   u16Cmd - Command
 *                   pu8Body - Data Buffer
 *                   u16Size - Data Buffer Size
 * Output:           None
 *                   
 * Returns:          RET_OK,RET_ERR_CAN2_TX_TIMEOUT,
 * Description:      
 * Note:             
 ***************************************************************************/
int32_t CanSendDataToCell(uint8_t u8CellNmbr, uint8_t u8Dest, uint16_t u16Cmd, const uint8_t *pu8Body, uint16_t u16Size)
{
    int32_t s32Result = TESTPROC_RESULT_OK;
    OS_ERR os_err;
	
    if (u8CellNmbr > 7 || (NULL == pu8Body && u16Size > 0) || u16Cmd >= CAN_CMD_INVALID)
    {
        return TESTPROC_RESULT_ERR_PARAM;
    }
    u8CellNmbr &= CAN_CMD_DST_CELL_MASK;
    u8Dest &= CAN_CMD_DST_BOARD_MASK;
    u16Cmd &= CAN_CMD_COMMAND_MASK;

    //DebugMsg("TxCAN:%d,%02X,%04X\r\n", u8CellNmbr, u8Dest, u16Cmd);
    OSMutexPend(&g_stMutexForCan1, 0, OS_OPT_PEND_BLOCKING, 0, &os_err);
    s32Result = CanSendData(Can1SendFrame, u8CellNmbr, u8Dest, u16Cmd, pu8Body, u16Size);
    OSMutexPost(&g_stMutexForCan1, OS_OPT_POST_NONE, &os_err);

    return s32Result;
}

//转发数据
void Can1DispatchReceive(CAN1_CMD_RECV* can1CommandRecvPtr)
{
        OS_ERR err;

        //一帧接收完成,要进行输出转发
        SYSTEM_CMD* systemCmdPtr = NULL;

        do
        {
            systemCmdPtr = UserMemMalloc(SRAM_IS62,sizeof(SYSTEM_CMD)/sizeof(uint8_t));
            if(systemCmdPtr == NULL)
            {
                //一定要申请到
                CoreDelayMinTick();
            }
        }while(systemCmdPtr == NULL);
        //设置指令源
        systemCmdPtr->commandSrc = SYSTEM_CMD_SRC_CAN1;

        //设置指针
        systemCmdPtr->systemCommandDataPtr = (void*)can1CommandRecvPtr;
        uint8_t indexOfElement = 0;
        for(indexOfElement = 0; indexOfElement < DISPATCH_ELEMENT_LENGTH; indexOfElement++)
        {
            if(can1CommandRecvPtr->command == can1DispatchArray[indexOfElement].commandValue)
            {
                break;
            }
        }
        //运行到这里,查看是否找到
        if(indexOfElement == DISPATCH_ELEMENT_LENGTH)
        {
            //没找到,释放内存
            UserMemFree(SRAM_CCM,can1CommandRecvPtr);
            UserMemFree(SRAM_IS62,systemCmdPtr);
        }
        else
        {
            //找到了,数据转发
            if(can1DispatchArray[indexOfElement].tcbTaskPtr == NULL)
            {
                //没有配置转发目标
                UserMemFree(SRAM_CCM,can1CommandRecvPtr);
                UserMemFree(SRAM_IS62,systemCmdPtr);
                return;
            }
            //向协议处理线程发送消息,注意,任务需要释放这一段消息的内存
            OSTaskQPost((OS_TCB *)can1DispatchArray[indexOfElement].tcbTaskPtr, 
                        (void *)(systemCmdPtr),
                        (OS_MSG_SIZE)sizeof(SYSTEM_CMD *),
                        (OS_OPT)OS_OPT_POST_FIFO,
                        (OS_ERR *)&err);
            if(err == OS_ERR_NONE)
            {
                //转发成功,不管啦
            }
            else
            {
                //转发失败释放内存
                UserMemFree(SRAM_CCM,can1CommandRecvPtr);
                UserMemFree(SRAM_IS62,systemCmdPtr);
                return;
            }
        }
}
/**
 * Arraypack_Common函数
 * 说明：
 * 
 * 
*/
int8_t Arraypack_Common(SYSTEM_CMD_CAN1* canCommand,int32_t param,int32_t result)
{
    uint8_t cellnum =  SystemConfig_GetCellNumber();
    canCommand->can1RecvDataBuffer[0] = cellnum+1;//canCommand->targetModuleNo;
    canCommand->can1RecvDataBuffer[1] = 0x01;//canCommand->targetBoardAddr;
    canCommand->can1RecvDataBuffer[2] = canCommand->command&0xff;
    canCommand->can1RecvDataBuffer[3] = (canCommand->command>>8)&0xff;
    canCommand->can1RecvDataBuffer[4] = (param)&0xff;
    canCommand->can1RecvDataBuffer[5] = (param>>8)&0xff;
    canCommand->can1RecvDataBuffer[6] = (param>>16)&0xff;
    canCommand->can1RecvDataBuffer[7] = (param>>24)&0xff;
    canCommand->can1RecvDataBuffer[8] = (result)&0xff;
    canCommand->can1RecvDataBuffer[9] = (result>>8)&0xff;
    canCommand->can1RecvDataBuffer[10] = (result>>16)&0xff;
    canCommand->can1RecvDataBuffer[11] = (result>>24)&0xff;
    return 12;
}




