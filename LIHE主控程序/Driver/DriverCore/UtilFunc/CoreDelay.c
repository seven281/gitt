#include "CoreDelay.h"

//系统定义的调度频率对应的调度间隔时间,单位为毫秒,延时的时候,大于这个值,将会使用操作系统的调度函数
//当小于该值,将会锁调度器进行延时
#define DELAY_TICK_WITH_OS_SCHEDULER_MIN_MS 			(1000 / OS_CFG_TICK_RATE_HZ)

//延时函数中锁住调度器的宏函数
#define DelayTickFuncWithOS_LockScheduler(errPtr) 		OSSchedLock(errPtr)

//延时函数中解锁调度器的宏函数
#define DelayTickFuncWithOS_UnlockScheduler(errPtr) 	OSSchedUnlock(errPtr)

//OS是否运行标记,0,不运行;1,在运行
#define delayTickFlagWithOS_Running 					OSRunning

//微秒计数值
static uint32_t delayTickCountWithMicroSecond = 0;

//延时函数初始化
void CoreDelayInit(void)
{
    CPU_INT32U  cpu_freq;
    CPU_SR_ALLOC();
	//获取当前CPU时钟
    cpu_freq = BSP_ClkFreqGet();
	//进入临界段
    CPU_CRITICAL_ENTER();
    //先关闭定时器
    NVIC_DisableIRQ(SysTick_IRQn);
	//SYSTICK使用外部时钟源
    SysTick_CLKSourceConfig(SysTick_CLKSource_HCLK);
	//计算微秒定时时候计数的数量
    delayTickCountWithMicroSecond = (cpu_freq/1000000);
	//初始化滴答定时器
    OS_CPU_SysTickInitFreq(cpu_freq);
	//打开SYSTICK中断
    NVIC_EnableIRQ(SysTick_IRQn);
	//先暂停滴答中断                          
    BSP_OS_TickDisable();                                      
    //退出临界段
    CPU_CRITICAL_EXIT();
}

//延时函数延时毫秒
void CoreDelayMs(uint16_t nms)
{
    OS_ERR err;
    if (nms >= DELAY_TICK_WITH_OS_SCHEDULER_MIN_MS)
    {
        //OS延时
        OSTimeDly((nms / DELAY_TICK_WITH_OS_SCHEDULER_MIN_MS), OS_OPT_TIME_PERIODIC, &err);
        //延时之后,剩下的进行不可调度的微秒延时
        nms %= DELAY_TICK_WITH_OS_SCHEDULER_MIN_MS;
    }
    if (nms != 0)
    {
        CoreDelayUs(nms * 1000);
    }
}

//延时函数延时微秒
void CoreDelayUs(uint16_t nus)
{
    OS_ERR err;
    if (nus >= (DELAY_TICK_WITH_OS_SCHEDULER_MIN_MS * 1000))
    {
        //OS延时
        OSTimeDly((nus / (DELAY_TICK_WITH_OS_SCHEDULER_MIN_MS * 1000)), OS_OPT_TIME_PERIODIC, &err);
        //延时之后,剩下的进行不可调度的微秒延时
        nus %= (DELAY_TICK_WITH_OS_SCHEDULER_MIN_MS * 1000);
    }
    if (nus != 0)
    {
        //此时要进行的是不可中断的延时,因为此时延时时间小于最小的调度时间
        uint32_t ticks;
        uint32_t told, tnow, tcnt = 0;
        //LOAD的值
        uint32_t reload = SysTick->LOAD;
        //需要的节拍数
        ticks = nus * delayTickCountWithMicroSecond;
        //阻止OS调度，防止打断us延时
        DelayTickFuncWithOS_LockScheduler(&err);
        //刚进入时的计数器值
        told = SysTick->VAL;
        while (1)
        {
            tnow = SysTick->VAL;
            if (tnow != told)
            {
                //这里注意一下SYSTICK是一个递减的计数器就可以了.
                if (tnow < told)
                {
                    tcnt += told - tnow;
                }
                else
                {
                    tcnt += reload - tnow + told;
                }
                told = tnow;
                //时间超过/等于要延迟的时间,则退出.
                if (tcnt >= ticks)
                    break;
            }
        }
        //恢复OS调度
        DelayTickFuncWithOS_UnlockScheduler(&err);
    }
}

//延时操作系统最小时间基数
void CoreDelayMinTick(void)
{
    OS_ERR err;
    //OS延时
    OSTimeDly(1, OS_OPT_TIME_PERIODIC, &err);
}

//操作系统滴答定时器中断处理向量
void CoreSystickIRQ_Handler()
{
	CPU_SR_ALLOC();

	//进入临界段
    CPU_CRITICAL_ENTER();
	/* Tell uC/OS-III that we are starting an ISR           */
    OSIntEnter();   
	//退出临界段                                            
    CPU_CRITICAL_EXIT();
	/* Call uC/OS-III's OSTimeTick()                        */
    OSTimeTick();                                               
	/* Tell uC/OS-III that we are leaving the ISR           */
    OSIntExit();                                                
}


















