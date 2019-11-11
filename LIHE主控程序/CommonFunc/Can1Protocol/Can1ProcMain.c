#include "Can1ProcMain.h"
#include "UserMemManager.h"
#include "SystemCmd.h"
#include "TaskServiceConfig.h"
//CAN1解析使用CCM内存

static CAN1_RECV_CACHE can1RecvCache@".ccram";

//static CAN1_ID_ANAYSIS_RESULT can1AnsysisID_Result;

//接收数据缓存,为了快速响应CAN中断
//static uint8_t can1CurrentRecvDataArray[8] = {0};
//接收数据长度
//static uint8_t can1CurrentRecvDataLength = 0;

//创建系统处理框架,将CAN口信息转发到指定的线程进行处理
//前一个是ID,后一个是转发处理的任务的堆栈
#if 0
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


//发送短包
static void Can1SendCommandPackShort(CAN1_CMD_SEND* can1CmdPtr)
{
    uint32_t sendCmdID = Can1ProcCreateShortID(can1CmdPtr->srcModuleNo,can1CmdPtr->srcBoardAddr,can1CmdPtr->targetModuleNo,
                        can1CmdPtr->targetBoardAddr,can1CmdPtr->command);
    CanTxMsg txMsg;
    txMsg.ExtId = sendCmdID;
    txMsg.DLC = can1CmdPtr->sendDataLength;
    //扩展帧
    txMsg.IDE = CAN_Id_Extended;
    //数据帧
    txMsg.RTR = CAN_RTR_Data;
    for(uint8_t index = 0; index < txMsg.DLC; index++)
    {
        txMsg.Data[index] = can1CmdPtr->sendDataBuffer[index];
    }
    //发送数据
    MCU_CAN1_TransMessage(&txMsg);
}

//发送长包起始包
static void Can1SendCommandPackLongStart(CAN1_CMD_SEND* can1CmdPtr)
{
    uint32_t sendCmdID = Can1ProcCreateLongStartID(can1CmdPtr->srcModuleNo,can1CmdPtr->srcBoardAddr,can1CmdPtr->targetModuleNo,
                        can1CmdPtr->targetBoardAddr);
    CanTxMsg txMsg;
    txMsg.ExtId = sendCmdID;
    //起始数据包固定长度4,两字节命令 两字节长度
    txMsg.DLC = 4;
    //扩展帧
    txMsg.IDE = CAN_Id_Extended;
    //数据帧
    txMsg.RTR = CAN_RTR_Data;
    //指令
    txMsg.Data[0] = (uint8_t)(can1CmdPtr->command);
    txMsg.Data[1] = (uint8_t)((can1CmdPtr->command)>>8);
    //长度
    txMsg.Data[2] = (uint8_t)(can1CmdPtr->sendDataLength);
    txMsg.Data[3] = (uint8_t)((can1CmdPtr->sendDataLength)>>8);
    //发送数据
    MCU_CAN1_TransMessage(&txMsg);
}

//发送长包数据包
static void Can1SendCommandPackLongData(CAN1_CMD_SEND* can1CmdPtr)
{
    //生成ID
    uint32_t sendCmdID = Can1ProcCreateLongDataID(can1CmdPtr->srcModuleNo,can1CmdPtr->srcBoardAddr,can1CmdPtr->targetModuleNo,
                        can1CmdPtr->targetBoardAddr);
    CanTxMsg txMsg;
    txMsg.ExtId = sendCmdID;
    //扩展帧
    txMsg.IDE = CAN_Id_Extended;
    //数据帧
    txMsg.RTR = CAN_RTR_Data;
    //计算最大包数
    uint16_t packCountMax = (can1CmdPtr->sendDataLength%6 == 0)?(can1CmdPtr->sendDataLength/6):(can1CmdPtr->sendDataLength/6)+1;
    //当前包数
    uint16_t currentPackIndex = 0;
    uint8_t byteIndex = 0;
    for(currentPackIndex = 0; currentPackIndex < packCountMax; currentPackIndex++)
    {
        //数据索引
        txMsg.Data[0] = (uint8_t)(currentPackIndex);
        txMsg.Data[1] = (uint8_t)(currentPackIndex>>8);
        for(byteIndex = 0; byteIndex < 6; byteIndex++)
        {
            if((currentPackIndex*6 + byteIndex) < can1CmdPtr->sendDataLength)
            {
                txMsg.Data[2 + byteIndex] = can1CmdPtr->sendDataBuffer[((currentPackIndex*6)+byteIndex)];
            }
            else
            {
                break;
            }
        }
        txMsg.DLC = byteIndex + 2;
        //发送数据
        MCU_CAN1_TransMessage(&txMsg);
    }
}

//CRC校验数组
static const uint8_t gabyModbusCRCHi[] =
{
	0x00,0xc1,0x81,0x40,0x01,0xc0,0x80,0x41,0x01,0xc0,
	0x80,0x41,0x00,0xc1,0x81,0x40,0x01,0xc0,0x80,0x41,
	0x00,0xc1,0x81,0x40,0x00,0xc1,0x81,0x40,0x01,0xc0,
	0x80,0x41,0x01,0xc0,0x80,0x41,0x00,0xc1,0x81,0x40,
	0x00,0xc1,0x81,0x40,0x01,0xc0,0x80,0x41,0x00,0xc1,
	0x81,0x40,0x01,0xc0,0x80,0x41,0x01,0xc0,0x80,0x41,
	0x00,0xc1,0x81,0x40,0x01,0xc0,0x80,0x41,0x00,0xc1,
	0x81,0x40,0x00,0xc1,0x81,0x40,0x01,0xc0,0x80,0x41,
	0x00,0xc1,0x81,0x40,0x01,0xc0,0x80,0x41,0x01,0xc0,
	0x80,0x41,0x00,0xc1,0x81,0x40,0x00,0xc1,0x81,0x40,
	0x01,0xc0,0x80,0x41,0x01,0xc0,0x80,0x41,0x00,0xc1,
	0x81,0x40,0x01,0xc0,0x80,0x41,0x00,0xc1,0x81,0x40,
	0x00,0xc1,0x81,0x40,0x01,0xc0,0x80,0x41,0x01,0xc0,
	0x80,0x41,0x00,0xc1,0x81,0x40,0x00,0xc1,0x81,0x40,
	0x01,0xc0,0x80,0x41,0x00,0xc1,0x81,0x40,0x01,0xc0,
	0x80,0x41,0x01,0xc0,0x80,0x41,0x00,0xc1,0x81,0x40,
	0x00,0xc1,0x81,0x40,0x01,0xc0,0x80,0x41,0x01,0xc0,
	0x80,0x41,0x00,0xc1,0x81,0x40,0x01,0xc0,0x80,0x41,
	0x00,0xc1,0x81,0x40,0x00,0xc1,0x81,0x40,0x01,0xc0,
	0x80,0x41,0x00,0xc1,0x81,0x40,0x01,0xc0,0x80,0x41,
	0x01,0xc0,0x80,0x41,0x00,0xc1,0x81,0x40,0x01,0xc0,
	0x80,0x41,0x00,0xc1,0x81,0x40,0x00,0xc1,0x81,0x40,
	0x01,0xc0,0x80,0x41,0x01,0xc0,0x80,0x41,0x00,0xc1,
	0x81,0x40,0x00,0xc1,0x81,0x40,0x01,0xc0,0x80,0x41,
	0x00,0xc1,0x81,0x40,0x01,0xc0,0x80,0x41,0x01,0xc0,
	0x80,0x41,0x00,0xc1,0x81,0x40
};

static const uint8_t gabyModbusCRCLo[] = 
{
	0x00,0xc0,0xc1,0x01,0xc3,0x03,0x02,0xc2,0xc6,0x06,
	0x07,0xc7,0x05,0xc5,0xc4,0x04,0xcc,0x0c,0x0d,0xcd,
	0x0f,0xcf,0xce,0x0e,0x0a,0xca,0xcb,0x0b,0xc9,0x09,
	0x08,0xc8,0xd8,0x18,0x19,0xd9,0x1b,0xdb,0xda,0x1a,
	0x1e,0xde,0xdf,0x1f,0xdd,0x1d,0x1c,0xdc,0x14,0xd4,
	0xd5,0x15,0xd7,0x17,0x16,0xd6,0xd2,0x12,0x13,0xd3,
	0x11,0xd1,0xd0,0x10,0xf0,0x30,0x31,0xf1,0x33,0xf3,
	0xf2,0x32,0x36,0xf6,0xf7,0x37,0xf5,0x35,0x34,0xf4,
	0x3c,0xfc,0xfd,0x3d,0xff,0x3f,0x3e,0xfe,0xfa,0x3a,
	0x3b,0xfb,0x39,0xf9,0xf8,0x38,0x28,0xe8,0xe9,0x29,
	0xeb,0x2b,0x2a,0xea,0xee,0x2e,0x2f,0xef,0x2d,0xed,
	0xec,0x2c,0xe4,0x24,0x25,0xe5,0x27,0xe7,0xe6,0x26,
	0x22,0xe2,0xe3,0x23,0xe1,0x21,0x20,0xe0,0xa0,0x60,
	0x61,0xa1,0x63,0xa3,0xa2,0x62,0x66,0xa6,0xa7,0x67,
	0xa5,0x65,0x64,0xa4,0x6c,0xac,0xad,0x6d,0xaf,0x6f,
	0x6e,0xae,0xaa,0x6a,0x6b,0xab,0x69,0xa9,0xa8,0x68,
	0x78,0xb8,0xb9,0x79,0xbb,0x7b,0x7a,0xba,0xbe,0x7e,
	0x7f,0xbf,0x7d,0xbd,0xbc,0x7c,0xb4,0x74,0x75,0xb5,
	0x77,0xb7,0xb6,0x76,0x72,0xb2,0xb3,0x73,0xb1,0x71,
	0x70,0xb0,0x50,0x90,0x91,0x51,0x93,0x53,0x52,0x92,
	0x96,0x56,0x57,0x97,0x55,0x95,0x94,0x54,0x9c,0x5c,
	0x5d,0x9d,0x5f,0x9f,0x9e,0x5e,0x5a,0x9a,0x9b,0x5b,
	0x99,0x59,0x58,0x98,0x88,0x48,0x49,0x89,0x4b,0x8b,
	0x8a,0x4a,0x4e,0x8e,0x8f,0x4f,0x8d,0x4d,0x4c,0x8c,
	0x44,0x84,0x85,0x45,0x87,0x47,0x46,0x86,0x82,0x42,
	0x43,0x83,0x41,0x81,0x80,0x40
};

static void Can1ProcCalcCRC(uint8_t* bufferPtr,uint16_t bufferLength,uint8_t* crcHighPtr,uint8_t* crcLowPtr)
{
    uint8_t index = 0;
    *crcHighPtr = 0xff;
	*crcLowPtr = 0xff;
    
	while(bufferLength--)
	{
		index   = (*crcHighPtr)^(*(bufferPtr++));
		*crcHighPtr = (*crcLowPtr) ^ gabyModbusCRCHi[index];
		*crcLowPtr = gabyModbusCRCLo[index];
	}
}

//发送长包结果包
static void Can1SendCommandPackLongEnd(CAN1_CMD_SEND* can1CmdPtr)
{
    uint32_t sendCmdID = Can1ProcCreateLongEndID(can1CmdPtr->srcModuleNo,can1CmdPtr->srcBoardAddr,can1CmdPtr->targetModuleNo,
                        can1CmdPtr->targetBoardAddr);
    CanTxMsg txMsg;
    txMsg.ExtId = sendCmdID;
    //起始数据包固定长度4,两字节命令 两字节长度
    txMsg.DLC = 4;
    //扩展帧
    txMsg.IDE = CAN_Id_Extended;
    //数据帧
    txMsg.RTR = CAN_RTR_Data;
    //指令
    txMsg.Data[0] = (uint8_t)(can1CmdPtr->command);
    txMsg.Data[1] = (uint8_t)((can1CmdPtr->command)>>8);
    //CRC计算
    uint8_t crcHigh = 0;
    uint8_t crcLow = 0;
    //计算CRC
    Can1ProcCalcCRC(can1CmdPtr->sendDataBuffer,can1CmdPtr->sendDataLength,&crcHigh,&crcLow);
    txMsg.Data[2] = (uint8_t)(crcHigh);
    txMsg.Data[3] = (uint8_t)(crcLow);
    MCU_CAN1_TransMessage(&txMsg);
}

//发送长包
static void Can1SendCommandPackLong(CAN1_CMD_SEND* can1CmdPtr)
{
    //发送起始包
    Can1SendCommandPackLongStart(can1CmdPtr);
    //发送过程包
    Can1SendCommandPackLongData(can1CmdPtr);
    //发送结果包
    Can1SendCommandPackLongEnd(can1CmdPtr);
}

//CAN1发送指令的数据单元的初始化
void Can1SendCommandDataStructInit(CAN1_CMD_SEND* can1CmdPtr)
{
    can1CmdPtr->srcModuleNo = CAN1_SELF_MODULE_NO;
    can1CmdPtr->srcBoardAddr = CAN1_SELF_BOARD_ID;
    can1CmdPtr->targetModuleNo = CAN1_COMM_MODULE_NO;
    can1CmdPtr->targetBoardAddr = CAN1_COMM_BOARD_ID;
    can1CmdPtr->command = 0x0000;
    can1CmdPtr->sendDataLength = 0;
    can1CmdPtr->sendDataBuffer = NULL;
}

//发送数据包
void Can1SendCommandPack(CAN1_CMD_SEND* can1CmdPtr)
{
    if(can1CmdPtr->sendDataLength <= 8)
    {
        //短包发送
        Can1SendCommandPackShort(can1CmdPtr);
    }
    else
    {
        //长包发送
        Can1SendCommandPackLong(can1CmdPtr);
    }
}

//发送数据包,在接收到的数据包的基础上
void Can1SendCommandPackWithRecvCmd(CAN1_CMD_RECV* can1CommandPtr,uint16_t commandValue,uint16_t dataBufferLength,uint8_t* dataBufferPtr)
{
    CAN1_CMD_SEND can1CmdSendResult;
    can1CmdSendResult.srcModuleNo = can1CommandPtr->targetModuleNo;
    can1CmdSendResult.srcBoardAddr = can1CommandPtr->targetBoardAddr;
    can1CmdSendResult.targetModuleNo = can1CommandPtr->srcModuleNo;
    can1CmdSendResult.targetBoardAddr = can1CommandPtr->srcBoardAddr;
    can1CmdSendResult.command = commandValue;
    can1CmdSendResult.sendDataLength = dataBufferLength;
    can1CmdSendResult.sendDataBuffer = dataBufferPtr;
    //发送结果
    Can1SendCommandPack(&can1CmdSendResult);
}


//发生错误报警
void Can1ReportErrorCode(CAN1_REPORT_ERR_LEVEL level,CAN1_REPORT_ERR_CODE errCode,uint8_t param1,
                            uint8_t param2,uint8_t param3,uint8_t param4)
{
    CAN1_CMD_SEND can1SendCommand;
    Can1SendCommandDataStructInit(&can1SendCommand);
    uint8_t* reportBuffer = NULL;
    //申请内存
    do
    {
        reportBuffer = UserMemMalloc(SRAM_CCM,7);
    }while(reportBuffer == NULL);
    //报警级别
    reportBuffer[0] = level;
    //报警码
    reportBuffer[1] = (uint8_t)(errCode&0xff);
    reportBuffer[2] = (uint8_t)((level&0xff00)>>8);
    //四个参数
    reportBuffer[3] = param1;
    reportBuffer[4] = param2;
    reportBuffer[5] = param3;
    reportBuffer[6] = param4;
    //设置发送参数
    can1SendCommand.sendDataBuffer = reportBuffer;
    can1SendCommand.sendDataLength = 7;
    can1SendCommand.command = CAN1_COMMAND_RUN_REPORT_ERR;
    //发送一帧
    Can1SendCommandPack(&can1SendCommand);
    //发送完成,释放内存
    UserMemFree(SRAM_CCM,reportBuffer);
}

#endif








