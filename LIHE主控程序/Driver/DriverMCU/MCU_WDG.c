#include "MCU_Wdg.h"

static MCU_WWDG_CallBackFunc wwdgCallBack = NULL;

//保存WWDG计数器的设置值,默认为最大.
static uint8_t WWDG_CNT = 0X7F;

//窗口看门狗初始化
//初始化窗口看门狗
//tr   :T[6:0],计数器值
//wr   :W[6:0],窗口值
//fprer:分频系数（WDGTB）,仅最低2位有效
//Fwwdg=PCLK1/(4096*2^fprer). 一般PCLK1=42Mhz
//一般 计数器值为7f,窗口寄存器为5f,分频数为8
void MCU_WDG_WindowInit(uint8_t tr, uint8_t wr, uint32_t fprer,MCU_WWDG_CallBackFunc callBack)
{
    NVIC_InitTypeDef NVIC_InitStructure;

    //使能窗口看门狗时钟
    RCC_APB1PeriphClockCmd(RCC_APB1Periph_WWDG, ENABLE);
    //初始化WWDG_CNT.
    WWDG_CNT = tr & WWDG_CNT;
    //设置分频值
    WWDG_SetPrescaler(fprer);
    //设置窗口值
    WWDG_SetWindowValue(wr);
    //开启看门狗
    WWDG_Enable(WWDG_CNT);

    //窗口看门狗中断
    NVIC_InitStructure.NVIC_IRQChannel = WWDG_IRQn;
    //设置优先级
    NVIC_InitStructure.NVIC_IRQChannelPreemptionPriority = MCU_WDG_WINDOW_PREE_PRI;
    NVIC_InitStructure.NVIC_IRQChannelSubPriority = MCU_WDG_WINDOW_SUB_PRI;
    //使能窗口看门狗通道
    NVIC_InitStructure.NVIC_IRQChannelCmd = ENABLE;
    //初始化
    NVIC_Init(&NVIC_InitStructure);

    //清除提前唤醒中断标志位
    WWDG_ClearFlag();
    //开启提前唤醒中断
    WWDG_EnableIT();
    
    wwdgCallBack = callBack;
}

//窗口看门狗喂狗,在中断中喂狗,所以注意中断关闭的时间
void MCU_WDG_WindowFeed(void)
{
    //重设窗口看门狗值
    WWDG_SetCounter(WWDG_CNT);
    //清除提前唤醒中断标志位
    WWDG_ClearFlag();
}

//窗口看门狗中断服务程序
void WWDG_IRQHandler(void)
{
    //重设窗口看门狗值
    WWDG_SetCounter(WWDG_CNT);
    //清除提前唤醒中断标志位
    WWDG_ClearFlag();
    if(wwdgCallBack != NULL)
    {
        wwdgCallBack();
    }
}

//独立看门狗初始化
//prer:分频数:0~7(只有低3位有效!)
//rlr:自动重装载值,0~0XFFF.
//分频因子=4*2^prer.但最大值只能是256!
//rlr:重装载寄存器值:低11位有效.
//时间计算(大概):Tout=((4*2^prer)*rlr)/32 (ms).
//分频数为64(4),重载值为500,溢出时间为1s
void MCU_WDG_IndependentInit(uint8_t prer, uint16_t rlr)
{
    //使能对IWDG->PR IWDG->RLR的写
    IWDG_WriteAccessCmd(IWDG_WriteAccess_Enable);
    //设置IWDG分频系数
    IWDG_SetPrescaler(prer);
    //设置IWDG装载值
    IWDG_SetReload(rlr);
    //reload
    IWDG_ReloadCounter();
    //使能看门狗
    IWDG_Enable();
}

//独立看门狗喂狗,打开之后必须定时喂狗
void MCU_WDG_IndependentFeed(void)
{
    IWDG_ReloadCounter();
}


