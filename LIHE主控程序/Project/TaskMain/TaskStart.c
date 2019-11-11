#include "BoardInc.h"
#include "SystemConfig.h"
#include "SystemTaskConfig.h"
#include "bsp_user.h"
#include <cpu.h>
#include "TSC_Inc.h"

//任务控制块,8字节对齐
#pragma pack(8)
//任务堆栈
CPU_STK stackBufferTaskStart[STK_SIZE_TASK_START];
#pragma pack()

OS_TCB tcbTaskStart;

//主板设备初始化函数
void BoardDeviceInit(void);

//启动任务
void TaskFuncStart(void *p_arg)
{
    (void)p_arg;

    OS_ERR os_err;

    CPU_SR_ALLOC();

    //默认为空的追踪函数
    OS_TRACE_INIT();

    /* 使能定时器以及启动定时器运行 */
    BSP_OS_TickEnable();

//状态初始化
#if OS_CFG_STAT_TASK_EN > 0u
    OSStatTaskCPUUsageInit(&os_err);
#endif

//中断时间测量初始化
#ifdef CPU_CFG_INT_DIS_MEAS_EN
    CPU_IntDisMeasMaxCurReset();
#endif

    BoardDeviceInit();

    //进入临界区
    CPU_CRITICAL_ENTER();

    /**************************************************创建系统服务任务**************************************************/
    //创建心跳任务
    OSTaskCreate((OS_TCB *)&tcbTaskServiceHeart,                    //任务控制块
                 (CPU_CHAR *)NAME_TASK_SERVICE_HEART,               //任务名字
                 (OS_TASK_PTR)TaskServiceFuncHeart,                 //任务函数
                 (void *)0,                                         //传递给任务函数的参数
                 (OS_PRIO)PRIO_TASK_SERVICE_HEART,                  //任务优先级
                 (CPU_STK *)&stackBufferTaskServiceHeart[0],        //任务堆栈基地址
                 (CPU_STK_SIZE)STK_SIZE_TASK_SERVICE_HEART / 10,    //任务堆栈深度限位
                 (CPU_STK_SIZE)STK_SIZE_TASK_SERVICE_HEART,         //任务堆栈大小
                 (OS_MSG_QTY)1,                                     //任务内部消息队列能够接收的最大消息数目,为0时禁止接收消息
                 (OS_TICK)0,                                        //当使能时间片轮转时的时间片长度，为0时为默认长度，
                 (void *)0,                                         //用户补充的存储区
                 (OS_OPT)OS_OPT_TASK_STK_CHK | OS_OPT_TASK_STK_CLR, //任务选项
                 (OS_ERR *)&os_err);                                //存放该函数错误时的返回值

    //CAN2下位机数据接收任务
    OSTaskCreate((OS_TCB *)&tcbTaskServiceCan2Recv,                  //任务控制块
                 (CPU_CHAR *)NAME_TASK_SERVICE_CAN2_RECV,            //任务名字
                 (OS_TASK_PTR)TaskServiceFuncCan2Recv,               //任务函数
                 (void *)0,                                          //传递给任务函数的参数
                 (OS_PRIO)PRIO_TASK_SERVICE_CAN2_RECV,               //任务优先级
                 (CPU_STK *)&stackBufferTaskServiceCan2Recv[0],      //任务堆栈基地址
                 (CPU_STK_SIZE)STK_SIZE_TASK_SERVICE_CAN2_RECV / 10, //任务堆栈深度限位
                 (CPU_STK_SIZE)STK_SIZE_TASK_SERVICE_CAN2_RECV,      //任务堆栈大小
                 (OS_MSG_QTY)MCU_CAN_RECEIVE_BUFFER_LENGTH,          //任务内部消息队列能够接收的最大消息数目,为0时禁止接收消息
                 (OS_TICK)0,                                         //当使能时间片轮转时的时间片长度，为0时为默认长度，
                 (void *)0,                                          //用户补充的存储区
                 (OS_OPT)OS_OPT_TASK_STK_CHK | OS_OPT_TASK_STK_CLR,  //任务选项
                 (OS_ERR *)&os_err);                                 //存放该函数错误时的返回值

    //CAN1中位机接收处理任务
    OSTaskCreate((OS_TCB *)&tcbTaskServiceCan1Recv,                  //任务控制块
                 (CPU_CHAR *)NAME_TASK_SERVICE_CAN1_RECV,            //任务名字
                 (OS_TASK_PTR)TaskServiceFuncCan1Recv,               //任务函数
                 (void *)0,                                          //传递给任务函数的参数
                 (OS_PRIO)PRIO_TASK_SERVICE_CAN1_RECV,               //任务优先级
                 (CPU_STK *)&stackBufferTaskServiceCan1Recv[0],      //任务堆栈基地址
                 (CPU_STK_SIZE)STK_SIZE_TASK_SERVICE_CAN1_RECV / 10, //任务堆栈深度限位
                 (CPU_STK_SIZE)STK_SIZE_TASK_SERVICE_CAN1_RECV,      //任务堆栈大小
                 (OS_MSG_QTY)MCU_CAN_RECEIVE_BUFFER_LENGTH,          //任务内部消息队列能够接收的最大消息数目,为0时禁止接收消息
                 (OS_TICK)0,                                         //当使能时间片轮转时的时间片长度，为0时为默认长度，
                 (void *)0,                                          //用户补充的存储区
                 (OS_OPT)OS_OPT_TASK_STK_CHK | OS_OPT_TASK_STK_CLR,  //任务选项
                 (OS_ERR *)&os_err);                                 //存放该函数错误时的返回值

    //串口动作调试任务
    OSTaskCreate((OS_TCB *)&tcbTaskServiceUart3RS1Recv,                   //任务控制块
                 (CPU_CHAR *)NAME_TASK_SERVICE_UART3_RS1_RECV,            //任务名字
                 (OS_TASK_PTR)TaskServiceFuncUart3RS1Recv,                //任务函数
                 (void *)0,                                               //传递给任务函数的参数
                 (OS_PRIO)PRIO_TASK_SERVICE_UART3_RS1_RECV,               //任务优先级
                 (CPU_STK *)&stackBufferTaskServiceUart3RS1Recv[0],       //任务堆栈基地址
                 (CPU_STK_SIZE)STK_SIZE_TASK_SERVICE_UART3_RS1_RECV / 10, //任务堆栈深度限位
                 (CPU_STK_SIZE)STK_SIZE_TASK_SERVICE_UART3_RS1_RECV,      //任务堆栈大小
                 (OS_MSG_QTY)20,                                          //任务内部消息队列能够接收的最大消息数目,为0时禁止接收消息
                 (OS_TICK)0,                                              //当使能时间片轮转时的时间片长度，为0时为默认长度，
                 (void *)0,                                               //用户补充的存储区
                 (OS_OPT)OS_OPT_TASK_STK_CHK | OS_OPT_TASK_STK_CLR,       //任务选项
                 (OS_ERR *)&os_err);                                      //存放该函数错误时的返回值

    //温度控制任务
    OSTaskCreate((OS_TCB *)&tcbTaskServiceTempCtrl,                  //任务控制块
                 (CPU_CHAR *)NAME_TASK_SERVICE_TEMP_CTRL,            //任务名字
                 (OS_TASK_PTR)TaskServiceFuncTempCtrl,               //任务函数
                 (void *)0,                                          //传递给任务函数的参数
                 (OS_PRIO)PRIO_TASK_SERVICE_TEMP_CTRL,               //任务优先级
                 (CPU_STK *)&stackBufferTaskServiceTempCtrl[0],      //任务堆栈基地址
                 (CPU_STK_SIZE)STK_SIZE_TASK_SERVICE_TEMP_CTRL / 10, //任务堆栈深度限位
                 (CPU_STK_SIZE)STK_SIZE_TASK_SERVICE_TEMP_CTRL,      //任务堆栈大小
                 (OS_MSG_QTY)1,                                      //任务内部消息队列能够接收的最大消息数目,为0时禁止接收消息
                 (OS_TICK)0,                                         //当使能时间片轮转时的时间片长度，为0时为默认长度，
                 (void *)0,                                          //用户补充的存储区
                 (OS_OPT)OS_OPT_TASK_STK_CHK | OS_OPT_TASK_STK_CLR,  //任务选项
                 (OS_ERR *)&os_err);                                 //存放该函数错误时的返回值

    //新杯栈与空杯栈管理任务
    OSTaskCreate((OS_TCB *)&tcbTaskServiceCupStackManagement,        //任务控制块
                 (CPU_CHAR *)NAME_TASK_SERVICE_CUP_STACK,            //任务名字
                 (OS_TASK_PTR)TaskServiceFuncCupStackManagement,     //任务函数
                 (void *)0,                                          //传递给任务函数的参数
                 (OS_PRIO)PRIO_TASK_SERVICE_CUP_STACK,               //任务优先级
                 (CPU_STK *)&stackBufferTaskServiceCupStack[0],      //任务堆栈基地址
                 (CPU_STK_SIZE)STK_SIZE_TASK_SERVICE_CUP_STACK / 10, //任务堆栈深度限位
                 (CPU_STK_SIZE)STK_SIZE_TASK_SERVICE_CUP_STACK,      //任务堆栈大小
                 (OS_MSG_QTY)1,                                      //任务内部消息队列能够接收的最大消息数目,为0时禁止接收消息
                 (OS_TICK)0,                                         //当使能时间片轮转时的时间片长度，为0时为默认长度，
                 (void *)0,                                          //用户补充的存储区
                 (OS_OPT)OS_OPT_TASK_STK_CHK | OS_OPT_TASK_STK_CLR,  //任务选项
                 (OS_ERR *)&os_err);                                 //存放该函数错误时的返回值

    //调试任务
    OSTaskCreate((OS_TCB *)&tcbTaskServiceDebug,                    //任务控制块
                 (CPU_CHAR *)NAME_TASK_SERVICE_DEBUG,               //任务名字
                 (OS_TASK_PTR)TaskServiceFuncDebug,                 //任务函数
                 (void *)0,                                         //传递给任务函数的参数
                 (OS_PRIO)PRIO_TASK_SERVICE_DEBUG,                  //任务优先级
                 (CPU_STK *)&stackBufferTaskServiceDebug[0],        //任务堆栈基地址
                 (CPU_STK_SIZE)STK_SIZE_TASK_SERVICE_DEBUG / 10,    //任务堆栈深度限位
                 (CPU_STK_SIZE)STK_SIZE_TASK_SERVICE_DEBUG,         //任务堆栈大小
                 (OS_MSG_QTY)1,                                     //任务内部消息队列能够接收的最大消息数目,为0时禁止接收消息
                 (OS_TICK)0,                                        //当使能时间片轮转时的时间片长度，为0时为默认长度，
                 (void *)0,                                         //用户补充的存储区
                 (OS_OPT)OS_OPT_TASK_STK_CHK | OS_OPT_TASK_STK_CLR, //任务选项
                 (OS_ERR *)&os_err);                                //存放该函数错误时的返回值

    //反应盘与试剂混匀任务
    OSTaskCreate((OS_TCB *)&tcbTaskServiceDishReactionMixReagent,                    //任务控制块
                 (CPU_CHAR *)NAME_TASK_SERVICE_DISH_REACTION_MIX_REAGENT,            //任务名字
                 (OS_TASK_PTR)TaskServiceFuncDishReactionMixReagent,                 //任务函数
                 (void *)0,                                                          //传递给任务函数的参数
                 (OS_PRIO)PRIO_TASK_SERVICE_DISH_REACTION_MIX_REAGENT,               //任务优先级
                 (CPU_STK *)&stackBufferTaskServiceDishReactionMixReagent[0],        //任务堆栈基地址
                 (CPU_STK_SIZE)STK_SIZE_TASK_SERVICE_DISH_REACTION_MIX_REAGENT / 10, //任务堆栈深度限位
                 (CPU_STK_SIZE)STK_SIZE_TASK_SERVICE_DISH_REACTION_MIX_REAGENT,      //任务堆栈大小
                 (OS_MSG_QTY)1,                                                      //任务内部消息队列能够接收的最大消息数目,为0时禁止接收消息
                 (OS_TICK)0,                                                         //当使能时间片轮转时的时间片长度，为0时为默认长度，
                 (void *)0,                                                          //用户补充的存储区
                 (OS_OPT)OS_OPT_TASK_STK_CHK | OS_OPT_TASK_STK_CLR,                  //任务选项
                 (OS_ERR *)&os_err);                                                 //存放该函数错误时的返回值

    //试剂盘任务
    OSTaskCreate((OS_TCB *)&tcbTaskServiceDishReagent,                  //任务控制块
                 (CPU_CHAR *)NAME_TASK_SERVICE_DISH_REAGENT,            //任务名字
                 (OS_TASK_PTR)TaskServiceFuncDishReagent,               //任务函数
                 (void *)0,                                             //传递给任务函数的参数
                 (OS_PRIO)PRIO_TASK_SERVICE_DISH_REAGENT,               //任务优先级
                 (CPU_STK *)&stackBufferTaskServiceDishReagent[0],      //任务堆栈基地址
                 (CPU_STK_SIZE)STK_SIZE_TASK_SERVICE_DISH_REAGENT / 10, //任务堆栈深度限位
                 (CPU_STK_SIZE)STK_SIZE_TASK_SERVICE_DISH_REAGENT,      //任务堆栈大小
                 (OS_MSG_QTY)1,                                         //任务内部消息队列能够接收的最大消息数目,为0时禁止接收消息
                 (OS_TICK)0,                                            //当使能时间片轮转时的时间片长度，为0时为默认长度，
                 (void *)0,                                             //用户补充的存储区
                 (OS_OPT)OS_OPT_TASK_STK_CHK | OS_OPT_TASK_STK_CLR,     //任务选项
                 (OS_ERR *)&os_err);                                    //存放该函数错误时的返回值

    //清洗盘与A液混匀任务
    OSTaskCreate((OS_TCB *)&tcbTaskServiceDishWashMixLiquidA,                     //任务控制块
                 (CPU_CHAR *)NAME_TASK_SERVICE_DISH_WASH_MIX_LIQUID_A,            //任务名字
                 (OS_TASK_PTR)TaskServiceFuncDishWashMixLiquidA,                  //任务函数
                 (void *)0,                                                       //传递给任务函数的参数
                 (OS_PRIO)PRIO_TASK_SERVICE_DISH_WASH_MIX_LIQUID_A,               //任务优先级
                 (CPU_STK *)&stackBufferTaskServiceDishWashMixLiquidA[0],         //任务堆栈基地址
                 (CPU_STK_SIZE)STK_SIZE_TASK_SERVICE_DISH_WASH_MIX_LIQUID_A / 10, //任务堆栈深度限位
                 (CPU_STK_SIZE)STK_SIZE_TASK_SERVICE_DISH_WASH_MIX_LIQUID_A,      //任务堆栈大小
                 (OS_MSG_QTY)1,                                                   //任务内部消息队列能够接收的最大消息数目,为0时禁止接收消息
                 (OS_TICK)0,                                                      //当使能时间片轮转时的时间片长度，为0时为默认长度，
                 (void *)0,                                                       //用户补充的存储区
                 (OS_OPT)OS_OPT_TASK_STK_CHK | OS_OPT_TASK_STK_CLR,               //任务选项
                 (OS_ERR *)&os_err);                                              //存放该函数错误时的返回值

    //抓杯机械手任务
    OSTaskCreate((OS_TCB *)&tcbTaskServiceHandCup,                  //任务控制块
                 (CPU_CHAR *)NAME_TASK_SERVICE_HAND_CUP,            //任务名字
                 (OS_TASK_PTR)TaskServiceFuncHandCup,               //任务函数
                 (void *)0,                                         //传递给任务函数的参数
                 (OS_PRIO)PRIO_TASK_SERVICE_HAND_CUP,               //任务优先级
                 (CPU_STK *)&stackBufferTaskServiceHandCup[0],      //任务堆栈基地址
                 (CPU_STK_SIZE)STK_SIZE_TASK_SERVICE_HAND_CUP / 10, //任务堆栈深度限位
                 (CPU_STK_SIZE)STK_SIZE_TASK_SERVICE_HAND_CUP,      //任务堆栈大小
                 (OS_MSG_QTY)1,                                     //任务内部消息队列能够接收的最大消息数目,为0时禁止接收消息
                 (OS_TICK)0,                                        //当使能时间片轮转时的时间片长度，为0时为默认长度，
                 (void *)0,                                         //用户补充的存储区
                 (OS_OPT)OS_OPT_TASK_STK_CHK | OS_OPT_TASK_STK_CLR, //任务选项
                 (OS_ERR *)&os_err);                                //存放该函数错误时的返回值

    //液路的检测任务
    OSTaskCreate((OS_TCB *)&tcbTaskServiceLiquidAuto,                  //任务控制块
                 (CPU_CHAR *)NAME_TASK_SERVICE_LIQUID_AUTO,            //任务名字
                 (OS_TASK_PTR)TaskServiceFuncLiquidAuto,               //任务函数
                 (void *)0,                                            //传递给任务函数的参数
                 (OS_PRIO)PRIO_TASK_SERVICE_LIQUID_AUTO,               //任务优先级
                 (CPU_STK *)&stackBufferTaskServiceLiquidAuto[0],      //任务堆栈基地址
                 (CPU_STK_SIZE)STK_SIZE_TASK_SERVICE_LIQUID_AUTO / 10, //任务堆栈深度限位
                 (CPU_STK_SIZE)STK_SIZE_TASK_SERVICE_LIQUID_AUTO,      //任务堆栈大小
                 (OS_MSG_QTY)1,                                        //任务内部消息队列能够接收的最大消息数目,为0时禁止接收消息
                 (OS_TICK)0,                                           //当使能时间片轮转时的时间片长度，为0时为默认长度，
                 (void *)0,                                            //用户补充的存储区
                 (OS_OPT)OS_OPT_TASK_STK_CHK | OS_OPT_TASK_STK_CLR,    //任务选项
                 (OS_ERR *)&os_err);                                   //存放该函数错误时的返回值

    //A液注液任务
    OSTaskCreate((OS_TCB *)&tcbTaskServiceLiquidInjectA,                   //任务控制块
                 (CPU_CHAR *)NAME_TASK_SERVICE_LIQUID_INJECT_A,            //任务名字
                 (OS_TASK_PTR)TaskServiceFuncLiquidInjectA,                //任务函数
                 (void *)0,                                                //传递给任务函数的参数
                 (OS_PRIO)PRIO_TASK_SERVICE_LIQUID_INJECT_A,               //任务优先级
                 (CPU_STK *)&stackBufferTaskServiceLiquidInjectA[0],       //任务堆栈基地址
                 (CPU_STK_SIZE)STK_SIZE_TASK_SERVICE_LIQUID_INJECT_A / 10, //任务堆栈深度限位
                 (CPU_STK_SIZE)STK_SIZE_TASK_SERVICE_LIQUID_INJECT_A,      //任务堆栈大小
                 (OS_MSG_QTY)1,                                            //任务内部消息队列能够接收的最大消息数目,为0时禁止接收消息
                 (OS_TICK)0,                                               //当使能时间片轮转时的时间片长度，为0时为默认长度，
                 (void *)0,                                                //用户补充的存储区
                 (OS_OPT)OS_OPT_TASK_STK_CHK | OS_OPT_TASK_STK_CLR,        //任务选项
                 (OS_ERR *)&os_err);                                       //存放该函数错误时的返回值

    //测量室与B液注液任务
    OSTaskCreate((OS_TCB *)&tcbTaskServiceMeasureRoomAndLiquidB,                     //任务控制块
                 (CPU_CHAR *)NAME_TASK_SERVICE_MEASURE_ROOM_AND_LIQUID_B,            //任务名字
                 (OS_TASK_PTR)TaskServiceFuncMeasureRoomAndLiquidB,                  //任务函数
                 (void *)0,                                                          //传递给任务函数的参数
                 (OS_PRIO)PRIO_TASK_SERVICE_MEASURE_ROOM_AND_LIQUID_B,               //任务优先级
                 (CPU_STK *)&stackBufferTaskServiceMeasureRoomAndLiquidB[0],         //任务堆栈基地址
                 (CPU_STK_SIZE)STK_SIZE_TASK_SERVICE_MEASURE_ROOM_AND_LIQUID_B / 10, //任务堆栈深度限位
                 (CPU_STK_SIZE)STK_SIZE_TASK_SERVICE_MEASURE_ROOM_AND_LIQUID_B,      //任务堆栈大小
                 (OS_MSG_QTY)1,                                                      //任务内部消息队列能够接收的最大消息数目,为0时禁止接收消息
                 (OS_TICK)0,                                                         //当使能时间片轮转时的时间片长度，为0时为默认长度，
                 (void *)0,                                                          //用户补充的存储区
                 (OS_OPT)OS_OPT_TASK_STK_CHK | OS_OPT_TASK_STK_CLR,                  //任务选项
                 (OS_ERR *)&os_err);                                                 //存放该函数错误时的返回值

    //磁珠混匀任务
    OSTaskCreate((OS_TCB *)&tcbTaskServiceMixBead,                  //任务控制块
                 (CPU_CHAR *)NAME_TASK_SERVICE_MIX_BEAD,            //任务名字
                 (OS_TASK_PTR)TaskServiceFuncMixBead,               //任务函数
                 (void *)0,                                         //传递给任务函数的参数
                 (OS_PRIO)PRIO_TASK_SERVICE_MIX_BEAD,               //任务优先级
                 (CPU_STK *)&stackBufferTaskServiceMixBead[0],      //任务堆栈基地址
                 (CPU_STK_SIZE)STK_SIZE_TASK_SERVICE_MIX_BEAD / 10, //任务堆栈深度限位
                 (CPU_STK_SIZE)STK_SIZE_TASK_SERVICE_MIX_BEAD,      //任务堆栈大小
                 (OS_MSG_QTY)1,                                     //任务内部消息队列能够接收的最大消息数目,为0时禁止接收消息
                 (OS_TICK)0,                                        //当使能时间片轮转时的时间片长度，为0时为默认长度，
                 (void *)0,                                         //用户补充的存储区
                 (OS_OPT)OS_OPT_TASK_STK_CHK | OS_OPT_TASK_STK_CLR, //任务选项
                 (OS_ERR *)&os_err);                                //存放该函数错误时的返回值

    //试剂磁珠针任务
    OSTaskCreate((OS_TCB *)&tcbTaskServiceNeedleReagentBead,                   //任务控制块
                 (CPU_CHAR *)NAME_TASK_SERVICE_NEEDLE_REAGENT_BEAD,            //任务名字
                 (OS_TASK_PTR)TaskServiceFuncNeedleReagentBead,                //任务函数
                 (void *)0,                                                    //传递给任务函数的参数
                 (OS_PRIO)PRIO_TASK_SERVICE_NEEDLE_REAGENT_BEAD,               //任务优先级
                 (CPU_STK *)&stackBufferTaskServiceNeedleReagentBead[0],       //任务堆栈基地址
                 (CPU_STK_SIZE)STK_SIZE_TASK_SERVICE_NEEDLE_REAGENT_BEAD / 10, //任务堆栈深度限位
                 (CPU_STK_SIZE)STK_SIZE_TASK_SERVICE_NEEDLE_REAGENT_BEAD,      //任务堆栈大小
                 (OS_MSG_QTY)1,                                                //任务内部消息队列能够接收的最大消息数目,为0时禁止接收消息
                 (OS_TICK)0,                                                   //当使能时间片轮转时的时间片长度，为0时为默认长度，
                 (void *)0,                                                    //用户补充的存储区
                 (OS_OPT)OS_OPT_TASK_STK_CHK | OS_OPT_TASK_STK_CLR,            //任务选项
                 (OS_ERR *)&os_err);                                           //存放该函数错误时的返回值

    //样本针任务
    OSTaskCreate((OS_TCB *)&tcbTaskServiceNeedleSample,                  //任务控制块
                 (CPU_CHAR *)NAME_TASK_SERVICE_NEEDLE_SAMPLE,            //任务名字
                 (OS_TASK_PTR)TaskServiceFuncNeedleSample,               //任务函数
                 (void *)0,                                              //传递给任务函数的参数
                 (OS_PRIO)PRIO_TASK_SERVICE_NEEDLE_SAMPLE,               //任务优先级
                 (CPU_STK *)&stackBufferTaskServiceNeedleSample[0],      //任务堆栈基地址
                 (CPU_STK_SIZE)STK_SIZE_TASK_SERVICE_NEEDLE_SAMPLE / 10, //任务堆栈深度限位
                 (CPU_STK_SIZE)STK_SIZE_TASK_SERVICE_NEEDLE_SAMPLE,      //任务堆栈大小
                 (OS_MSG_QTY)1,                                          //任务内部消息队列能够接收的最大消息数目,为0时禁止接收消息
                 (OS_TICK)0,                                             //当使能时间片轮转时的时间片长度，为0时为默认长度，
                 (void *)0,                                              //用户补充的存储区
                 (OS_OPT)OS_OPT_TASK_STK_CHK | OS_OPT_TASK_STK_CLR,      //任务选项
                 (OS_ERR *)&os_err);                                     //存放该函数错误时的返回值

    //清洗针任务
    OSTaskCreate((OS_TCB *)&tcbTaskServiceNeedleWash,                  //任务控制块
                 (CPU_CHAR *)NAME_TASK_SERVICE_NEEDLE_WASH,            //任务名字
                 (OS_TASK_PTR)TaskServiceFuncNeedleWash,               //任务函数
                 (void *)0,                                            //传递给任务函数的参数
                 (OS_PRIO)PRIO_TASK_SERVICE_NEEDLE_WASH,               //任务优先级
                 (CPU_STK *)&stackBufferTaskServiceNeedleWash[0],      //任务堆栈基地址
                 (CPU_STK_SIZE)STK_SIZE_TASK_SERVICE_NEEDLE_WASH / 10, //任务堆栈深度限位
                 (CPU_STK_SIZE)STK_SIZE_TASK_SERVICE_NEEDLE_WASH,      //任务堆栈大小
                 (OS_MSG_QTY)1,                                        //任务内部消息队列能够接收的最大消息数目,为0时禁止接收消息
                 (OS_TICK)0,                                           //当使能时间片轮转时的时间片长度，为0时为默认长度，
                 (void *)0,                                            //用户补充的存储区
                 (OS_OPT)OS_OPT_TASK_STK_CHK | OS_OPT_TASK_STK_CLR,    //任务选项
                 (OS_ERR *)&os_err);                                   //存放该函数错误时的返回值

    //试剂盘试剂二维码扫描任务
    OSTaskCreate((OS_TCB *)&tcbTaskServiceQRScan,                   //任务控制块
                 (CPU_CHAR *)NAME_TASK_SERVICE_QR_SCAN,             //任务名字
                 (OS_TASK_PTR)TaskServiceFuncQRScan,                //任务函数
                 (void *)0,                                         //传递给任务函数的参数
                 (OS_PRIO)PRIO_TASK_SERVICE_QR_SCAN,                //任务优先级
                 (CPU_STK *)&stackBufferTaskServiceQRScan[0],       //任务堆栈基地址
                 (CPU_STK_SIZE)STK_SIZE_TASK_SERVICE_QR_SCAN / 10,  //任务堆栈深度限位
                 (CPU_STK_SIZE)STK_SIZE_TASK_SERVICE_QR_SCAN,       //任务堆栈大小
                 (OS_MSG_QTY)1,                                     //任务内部消息队列能够接收的最大消息数目,为0时禁止接收消息
                 (OS_TICK)0,                                        //当使能时间片轮转时的时间片长度，为0时为默认长度，
                 (void *)0,                                         //用户补充的存储区
                 (OS_OPT)OS_OPT_TASK_STK_CHK | OS_OPT_TASK_STK_CLR, //任务选项
                 (OS_ERR *)&os_err);                                //存放该函数错误时的返回值

    //试管架传送带任务
    OSTaskCreate((OS_TCB *)&tcbTaskServiceRankPathWay,                   //任务控制块
                 (CPU_CHAR *)NAME_TASK_SERVICE_RANK_PATH_WAY,            //任务名字
                 (OS_TASK_PTR)TaskServiceFuncRankPathWay,                //任务函数
                 (void *)0,                                              //传递给任务函数的参数
                 (OS_PRIO)PRIO_TASK_SERVICE_RANK_PATH_WAY,               //任务优先级
                 (CPU_STK *)&stackBufferTaskServiceRankPathWay[0],       //任务堆栈基地址
                 (CPU_STK_SIZE)STK_SIZE_TASK_SERVICE_RANK_PATH_WAY / 10, //任务堆栈深度限位
                 (CPU_STK_SIZE)STK_SIZE_TASK_SERVICE_RANK_PATH_WAY,      //任务堆栈大小
                 (OS_MSG_QTY)1,                                          //任务内部消息队列能够接收的最大消息数目,为0时禁止接收消息
                 (OS_TICK)0,                                             //当使能时间片轮转时的时间片长度，为0时为默认长度，
                 (void *)0,                                              //用户补充的存储区
                 (OS_OPT)OS_OPT_TASK_STK_CHK | OS_OPT_TASK_STK_CLR,      //任务选项
                 (OS_ERR *)&os_err);                                     //存放该函数错误时的返回值
    /**************************************************初始化信息**********************************************************/
    TaskUtilServiceMsgInit();
    /*************************************************系统服务调用接口初始化**********************************************/

    //杯栈管理
    TSC_StackManageResultDataInit();
    //反应盘与试剂混匀
    TSC_DishReactionAndReagentMixResultDataInit();
    //试剂盘
    TSC_DishReagentResultDataInit();
    //清洗盘与A液混匀
    TSC_DishWashAndLiquidAMixResultDataInit();
    //磁珠混匀
    TSC_BeadMixResultDataInit();
    //机械手
    TSC_HandCupResultDataInit();
    //测量室与B液注液
    TSC_MeasureRoomResultDataInit();
    //试剂磁珠针
    TSC_NeedleReagentBeadResultDataInit();
    //样本针
    TSC_NeedleSampleResultDataInit();
    //清洗针
    TSC_NeedleWashResultDataInit();
    //试剂盘二维码扫描
    TSC_QRCodeScanResultDataInit();
    //试管架传动带
    TSC_RankPathWayResultDataInit();
    //注A液
    TSC_LiquidInjectAResultDataInit();


    /**************************************************创建系统应用程序**************************************************/
    //测试流程
    OSTaskCreate((OS_TCB *)&tcbTaskAppTestProcess,                  //任务控制块
                 (CPU_CHAR *)NAME_TASK_APP_TEST_PROCESS,            //任务名字
                 (OS_TASK_PTR)TaskAppFuncTestProcess,               //任务函数
                 (void *)0,                                         //传递给任务函数的参数
                 (OS_PRIO)PRIO_TASK_APP_TEST_PROCESS,               //任务优先级
                 (CPU_STK *)&stackBufferTaskAppTestProcess[0],      //任务堆栈基地址
                 (CPU_STK_SIZE)STK_SIZE_TASK_APP_TEST_PROCESS / 10, //任务堆栈深度限位
                 (CPU_STK_SIZE)STK_SIZE_TASK_APP_TEST_PROCESS,      //任务堆栈大小
                 (OS_MSG_QTY)16,                                    //任务内部消息队列能够接收的最大消息数目,为0时禁止接收消息
                 (OS_TICK)0,                                        //当使能时间片轮转时的时间片长度，为0时为默认长度，
                 (void *)0,                                         //用户补充的存储区
                 (OS_OPT)OS_OPT_TASK_STK_CHK | OS_OPT_TASK_STK_CLR, //任务选项
                 (OS_ERR *)&os_err);

    //退出临界区
    CPU_CRITICAL_EXIT();

    //删除start_task任务自身
    OSTaskDel((OS_TCB *)0, &os_err);
}

//板上硬件初始化
void BoardDeviceInit(void)
{
    LH_ERR err;
    //LED初始化
    BoardLedInit(BOARD_LED_RED, BOARD_LED_LIGHT);
    BoardLedInit(BOARD_LED_GREEN, BOARD_LED_LIGHT);
    BoardLedInit(BOARD_LED_YELLOW, BOARD_LED_LIGHT);
    //串口初始化,六针串口,用作动作Action调试
    BoardRS_Init(BOARD_RS1_UART3, 115200, MCU_UART_LENGTH8, MCU_UART_STOPBIT1, MCU_UART_CHECK_MODE_NONE,
                 MCU_UART_HARD_CONTROL_NONE, BoardSerialRS1_RecvProcess);
    //串口初始化,四针串口,用作流程调试
    BoardRS_Init(BOARD_RS2_UART1, 115200, MCU_UART_LENGTH8, MCU_UART_STOPBIT1, MCU_UART_CHECK_MODE_NONE,
                 MCU_UART_HARD_CONTROL_NONE, SystemConfig_IsrCallBack);
    //串口初始化,六针串口,用作条码枪
    BoardRS_Init(BOARD_RS3_UART2, 115200, MCU_UART_LENGTH8, MCU_UART_STOPBIT1, MCU_UART_CHECK_MODE_NONE,
                 MCU_UART_HARD_CONTROL_NONE, NULL);
    //串口初始化,四针串口,用作光子测量
    BoardRS_Init(BOARD_RS4_UART6, 115200, MCU_UART_LENGTH8, MCU_UART_STOPBIT1, MCU_UART_CHECK_MODE_NONE,
                 MCU_UART_HARD_CONTROL_NONE, NULL);
    //输出系统启动讯息
    SystemPrintf("BoardDeviceInit Start\r\n");
    //IO状态初始化
    BoardInOutAllInitOnce();
    //CCM内存管理
    UserMemInit(SRAM_CCM);
    //内部内存管理
    UserMemInit(SRAM_IN);
    //随机数初始化
    MCU_RandomInit();
    //蜂鸣器初始化并提示复位
    BoardBeepInit(BEEP_OFF);
    //beep响铃展示,系统每次复位都会响铃
    BoardBeepSetState(BEEP_ON);
    //延时发生
    CoreDelayMs(500);
    //关闭蜂鸣器
    BoardBeepSetState(BEEP_OFF);

    //MB85RS2MT初始化
    do
    {
        err = BoardMB85RS2MT_Init();
        if (err != LH_ERR_NONE)
        {
            //发送出错讯息
            SystemPrintf("BoardMB85RS2MT_Init Failed,ErrorCode : 0X%8X\r\n", err);
            //延时
            CoreDelayMs(2000);
            //红灯闪烁
            BoardLedToogle(BOARD_LED_RED);
        }
    } while (err != LH_ERR_NONE);

    //SRAM初始化
    do
    {
        err = BoardIS62Init();
        if (err != LH_ERR_NONE)
        {
            //发送出错讯息
            SystemPrintf("BoardIS62Init Failed,ErrorCode : 0X%8X\r\n", err);
            //延时
            CoreDelayMs(2000);
            //红灯闪烁
            BoardLedToogle(BOARD_LED_RED);
        }
    } while (err != LH_ERR_NONE);

    //初始化外部内存管理
    UserMemInit(SRAM_IS62);

    //尝试初始化SDCARD
    do
    {
        err = BoardSDCardInit();
        if (err != LH_ERR_NONE)
        {
            //发送出错讯息
            SystemPrintf("BoardSDCardInit Failed,ErrorCode : 0X%8X\r\n", err);
            //延时
            CoreDelayMs(2000);
            //红灯闪烁
            BoardLedToogle(BOARD_LED_RED);
        }
        else
        {
            //显示SDCARD讯息
            ConsoleSendCardMessage();
        }
        //不强求初始化成功,但是还是得试一下
    } while (0); //err != LH_ERR_NONE);

    //加载系统配置
    SystemConfig_LoadConfig();
    //CAN总线初始化
    MCU_CAN_Init(MCU_CAN_BAUD_1000K, MCU_CAN_BAUD_1000K,
                 OS_Can1ReceiveDataCallFunc, OS_Can2ReceiveDataCallFunc);
    //板上压力传感器初始化
    BoardPressureSensorInit();
    //系统动作参数加载
    SystemLoadActionParamOnce();
    //防止循环扫码,关闭循环扫描模式
    BoardRS_SendString(QR_BAR_SCAN_PORT, "\r\nCDOPSMD0\r\n");
    //打开单次扫描,扫描完成自动关闭
    BoardRS_SendString(QR_BAR_SCAN_PORT, "\r\nRDCMXEV1,P11,P20\r\n");

    //运行到心跳线程,保险起见,将三个灯关闭掉
    BoardLedWrite(BOARD_LED_RED, BOARD_LED_DARK);
    BoardLedWrite(BOARD_LED_GREEN, BOARD_LED_DARK);
    BoardLedWrite(BOARD_LED_YELLOW, BOARD_LED_DARK);
}
