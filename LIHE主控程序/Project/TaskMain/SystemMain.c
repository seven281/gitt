#include "stm32f4xx.h"
#include "SystemTaskConfig.h"
#include <cpu.h>
#include "CoreInc.h"

int main(void)
{
    OS_ERR  os_err;
    
    //设置中断分组
    CoreNVIC_SetGroupDefault();

    //打开端口复用时钟
    //使能SYSCFG时钟
	RCC_APB2PeriphClockCmd(RCC_APB2Periph_SYSCFG, ENABLE);
    
    //初始化中断线连接
    SYSCFG_DeInit();

    //滴答定时器初始化
    CoreDelayInit();

    //关闭中断
    CPU_IntDis(); 

    //MCU内核模块初始化
    CPU_Init();                                                 

    //操作系统初始化
    OSInit(&os_err); 
    
    //初始化状态判定                                           
    if (os_err != OS_ERR_NONE) {
        while (1);
    }

    //设置系统内置回调函数
    App_OS_SetAllHooks();  
    
    //创建开始任务
    OSTaskCreate((OS_TCB *)&tcbTaskStart,                           //任务控制块
                 (CPU_CHAR *)NAME_TASK_START,                       //任务名字
                 (OS_TASK_PTR)TaskFuncStart,                        //任务函数
                 (void *)0,                                         //传递给任务函数的参数
                 (OS_PRIO)PRIO_TASK_START,                          //任务优先级
                 (CPU_STK *)&stackBufferTaskStart[0],               //任务堆栈基地址
                 (CPU_STK_SIZE)STK_SIZE_TASK_START / 10,            //任务堆栈深度限位
                 (CPU_STK_SIZE)STK_SIZE_TASK_START,                 //任务堆栈大小
                 (OS_MSG_QTY)0,                                     //任务内部消息队列能够接收的最大消息数目,为0时禁止接收消息
                 (OS_TICK)0,                                        //当使能时间片轮转时的时间片长度，为0时为默认长度，
                 (void *)0,                                         //用户补充的存储区
                 (OS_OPT)OS_OPT_TASK_STK_CHK | OS_OPT_TASK_STK_CLR, //任务选项
                 (OS_ERR *)&os_err);                                //存放该函数错误时的返回值

    if (os_err != OS_ERR_NONE) {
        while (1);
    }                                          
    //打开中断
    CPU_IntEn();

    //启动系统
    OSStart(&os_err); 
    
    //原则上永远不会运行到这里
    while (DEF_ON) {                                            
        ;
    }
}
