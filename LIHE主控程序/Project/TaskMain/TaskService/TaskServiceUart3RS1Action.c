#include "ActionImplementInc.h"
#include "TSC_Inc.h"
#include "TaskServiceConfig.h"

//任务控制块,8字节对齐
#pragma pack(8)
//任务堆栈
CPU_STK stackBufferTaskServiceUart3RS1Recv[STK_SIZE_TASK_SERVICE_UART3_RS1_RECV];
#pragma pack()

//任务控制块
OS_TCB tcbTaskServiceUart3RS1Recv;

//组合指令,当方法代码是组合指令的时候,需要识别子方法代码
//当方法代码不是组合指令的时候,不需要识别方法代码
#define COMPLEX_FUNC_CODE 0X08

//串口转发数据单元的配置
typedef struct SERVICE_UART3RS1_DISPATCH_UNIT
{
    uint8_t mainFuncCode;        //主功能代码
    uint8_t subFuncCode;         //子功能代码
    OS_TCB* dispatchThreadPtr; //转发信道指针
} SERVICE_UART3RS1_DISPATCH_UNIT;

static const SERVICE_UART3RS1_DISPATCH_UNIT uart3RS1DispatchArray[] = {
    {COMPLEX_FUNC_CODE, 0x01, &tcbTaskServiceDishWashMixLiquidA}, //清洗盘
    {COMPLEX_FUNC_CODE, 0x02, &tcbTaskServiceNeedleWash}, //清洗针
    {COMPLEX_FUNC_CODE, 0x03, &tcbTaskServiceLiquidInjectA}, //A液注液
    {COMPLEX_FUNC_CODE, 0x04, &tcbTaskServiceDishWashMixLiquidA}, //清洗摇匀
    {COMPLEX_FUNC_CODE, 0x06, &tcbTaskServiceMeasureRoomAndLiquidB}, //测量上门
    {COMPLEX_FUNC_CODE, 0x07, &tcbTaskServiceMeasureRoomAndLiquidB}, //测量窗
    {COMPLEX_FUNC_CODE, 0x08, &tcbTaskServiceNeedleReagentBead}, //磁珠试剂针
    {COMPLEX_FUNC_CODE, 0x09, &tcbTaskServiceDishReagent}, //试剂盘
    {COMPLEX_FUNC_CODE, 0x0B, &tcbTaskServiceDishReactionMixReagent}, //试剂混匀
    {COMPLEX_FUNC_CODE, 0x0C, &tcbTaskServiceMixBead}, //磁珠混匀
    {COMPLEX_FUNC_CODE, 0x0D, &tcbTaskServiceDishReactionMixReagent}, //反应盘
    {COMPLEX_FUNC_CODE, 0x0E, &tcbTaskServiceNeedleSample}, //样本针
    {COMPLEX_FUNC_CODE, 0x0F, &tcbTaskServiceHandCup}, //送样机械手
    {COMPLEX_FUNC_CODE, 0x10, &tcbTaskServiceCupStackManagement}, //反应杯装载机构
    {COMPLEX_FUNC_CODE, 0x12, &tcbTaskServiceRankPathWay}, //试管架传送带
    {0X15, 0x00, &tcbTaskServiceDebug},              //读版本号
    {0X09, 0x00, &tcbTaskServiceDebug},              //指定电机复位
    {0X55, 0x00, &tcbTaskServiceDebug},              //写入IO口输出
    {0X0A, 0x00, &tcbTaskServiceDebug},              //指定电机运行指定步数
    {0X04, 0x00, &tcbTaskServiceDebug},              //读取IO输入
    {0X03, 0x00, &tcbTaskServiceDebug},              //读参数
    {0X06, 0x00, &tcbTaskServiceDebug},              //写参数
    {0X02, 0x00, &tcbTaskServiceDebug},              //握手
    {0X10, 0x00, &tcbTaskServiceQRScan},              //条码扫描,进样和试剂盘都有
};

#define SERIAL_RS1_DISPATCH_COUNT (sizeof(uart3RS1DispatchArray) / sizeof(uart3RS1DispatchArray[0]))

//任务函数
//接收解析RS1 的消息并处理,主要是执行的转发工作
void TaskServiceFuncUart3RS1Recv(void *p_arg)
{
    OS_ERR err;
    p_arg = p_arg;
    //任务接收数据长度
    OS_MSG_SIZE size;
    //接收的数据指针
    SYSTEM_CMD_UART_RS1 *systemCommandRecvUart = NULL;
    //转发指针
    SYSTEM_CMD *dispatchCmdPtr = NULL;
    while (1)
    {
        //从串口读取消息
        systemCommandRecvUart = NULL;
        //请求消息,堵塞访问,协议处理任务必须实时性极高
        systemCommandRecvUart = OSTaskQPend((OS_TICK)0,
                                            (OS_OPT)OS_OPT_PEND_BLOCKING,
                                            (OS_MSG_SIZE *)&size,
                                            (CPU_TS *)0,
                                            (OS_ERR *)&err);
        //收到切切的串口信息,进行处理
        if ((systemCommandRecvUart != NULL) && (err == OS_ERR_NONE))
        {
            //数据校验
            if (0 == SystemUartRS_ModbusCheckCRC(systemCommandRecvUart->uartRecvCommandArray))
            {
                //数据校验失败,丢弃数据帧,直接释放本次申请的内存
                UserMemFree(SRAM_CCM, (void *)systemCommandRecvUart);
                continue;
            }
            //转发信息,将数据信息转化为协议信息
            SystemUartRS_ModbusCreateCommandPackage(systemCommandRecvUart->uartRecvCommandArray, systemCommandRecvUart);
            //申请系统内存
            do
            {
                dispatchCmdPtr = UserMemMalloc(SRAM_CCM, sizeof(SYSTEM_CMD));
                if (dispatchCmdPtr == NULL)
                {
                    CoreDelayMinTick();
                }
            } while (dispatchCmdPtr == NULL);
            //指针数据赋值
            dispatchCmdPtr->commandSrc = SYSTEM_CMD_SRC_UART_RS1;
            dispatchCmdPtr->systemCommandDataPtr = systemCommandRecvUart;
            //辅助变量
            uint16_t utilIndex = 0;
            //是否转发成功,转发成功,不用清内存,否则,要释放内存
            uint8_t dispatchSuccess = 0;
            //轮询转发数组
            for (utilIndex = 0; utilIndex < SERIAL_RS1_DISPATCH_COUNT; utilIndex++)
            {
                if (systemCommandRecvUart->funcCode == uart3RS1DispatchArray[utilIndex].mainFuncCode)
                {
                    //主功能码匹配上了,现在要看是不是复合功能,复合功能要看子功能码匹配,否则,直接查看转发指针
                    if (systemCommandRecvUart->funcCode == COMPLEX_FUNC_CODE)
                    {
                        //查看子功能码是否匹配
                        if(systemCommandRecvUart->uartRecvCommandArray[5] == uart3RS1DispatchArray[utilIndex].subFuncCode)
                        {
                            //子功能码匹配
                            if(uart3RS1DispatchArray[utilIndex].dispatchThreadPtr != NULL)
                            {
                                //尝试转发
                                OSTaskQPost((OS_TCB *)(uart3RS1DispatchArray[utilIndex].dispatchThreadPtr), //向协议处理线程发送消息
                                            (void *)dispatchCmdPtr,
                                            (OS_MSG_SIZE)sizeof(SYSTEM_CMD *),
                                            (OS_OPT)OS_OPT_POST_FIFO,
                                            (OS_ERR *)&err);
                                //检测是否发送成功
                                if (err != OS_ERR_NONE)
                                {
                                    //转发失败
                                    dispatchSuccess = 0;
                                    //跳出循环
                                    break;
                                }
                                else
                                {
                                    //转发成功
                                    dispatchSuccess = 1;
                                    //跳出循环
                                    break;
                                }
                            }
                            else
                            {
                                //没有转发指针,转发失败
                                dispatchSuccess = 0;
                                //跳出循环
                                break;
                            }
                        }
                        else
                        {
                            //子功能码不匹配,跳过本次
                            continue;
                        }
                    }
                    else
                    {
                        //非复合功能
                        if (uart3RS1DispatchArray[utilIndex].dispatchThreadPtr != NULL)
                        {
                            //有转发指针,尝试转发
                            OSTaskQPost((OS_TCB *)(uart3RS1DispatchArray[utilIndex].dispatchThreadPtr), //向协议处理线程发送消息
                                        (void *)dispatchCmdPtr,
                                        (OS_MSG_SIZE)sizeof(SYSTEM_CMD *),
                                        (OS_OPT)OS_OPT_POST_FIFO,
                                        (OS_ERR *)&err);
                            //检测是否发送成功
                            if (err != OS_ERR_NONE)
                            {
                                //转发失败
                                dispatchSuccess = 0;
                                //跳出循环
                                break;
                            }
                            else
                            {
                                //转发成功
                                dispatchSuccess = 1;
                                //跳出循环
                                break;
                            }
                        }
                        else
                        {
                            //转发失败
                            dispatchSuccess = 0;
                            //跳出循环
                            break;
                        }
                    }
                }
            }
            //要是一直没有转发成功,就会来这里释放内存
            if (dispatchSuccess == 0)
            {
                //转发失败
                UserMemFree(SRAM_CCM, dispatchCmdPtr->systemCommandDataPtr);
                UserMemFree(SRAM_CCM, dispatchCmdPtr);
                continue;
            }
            else
            {
                //转发成功,进入下一轮
            }
        }
    }
}

//串口接收数据计数
static uint8_t recvLengthPC = 0;
//串口接收数据缓存
static uint8_t recvBufferPC[12] = {0};

//接收串口调试讯息
void BoardSerialRS1_RecvProcess(uint8_t recvDat)
{
    OS_ERR err;
    //运行时内存申请的临时指针
    SYSTEM_CMD_UART_RS1 *uartSrcCommandPtr = NULL;
    if (recvLengthPC == 0)
    {
        if (recvDat == 0xaa)
        {
            recvBufferPC[recvLengthPC] = recvDat;
            recvLengthPC++;
        }
    }
    //数据长度为11时,表示此时必须接受到结尾字符,如果不是,那么这一帧数据全部清空
    else if (recvLengthPC == 11)
    {
        if (recvDat == 0xa5)
        {
            recvBufferPC[recvLengthPC] = recvDat;
            //接收到完整的一包,将数据转发到相应处理通道,先转发到接受通道
            uartSrcCommandPtr = UserMemMalloc(SRAM_CCM, sizeof(SYSTEM_CMD_UART_RS1));
            if (uartSrcCommandPtr == NULL)
            {
                //申请不到内存,丢弃
                recvLengthPC = 0;
                return;
            }
            UserMemCopy(uartSrcCommandPtr->uartRecvCommandArray, recvBufferPC, 12);
            //数据转发,将申请的数据转发到协议处理线程
            OSTaskQPost((OS_TCB *)&tcbTaskServiceUart3RS1Recv, //向协议处理线程发送消息
                        (void *)uartSrcCommandPtr,
                        (OS_MSG_SIZE)sizeof(SYSTEM_CMD_UART_RS1 *),
                        (OS_OPT)OS_OPT_POST_FIFO,
                        (OS_ERR *)&err);
            //检测是否发送成功
            if (err != OS_ERR_NONE)
            {
                //发送失败,直接释放本次申请的内存
                UserMemFree(SRAM_CCM, uartSrcCommandPtr);
            }
            recvLengthPC = 0;
        }
        else
        {
            recvLengthPC = 0;
        }
    }
    else
    {
        //正常传输数据,正常保存
        recvBufferPC[recvLengthPC] = recvDat;
        recvLengthPC++;
    }
}
