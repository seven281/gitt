#include "MCU_Random.h"
#include "os.h"

//随机数初始化
void MCU_RandomInit(void)
{
    //开启RNG时钟,来自PLL48CLK
    RCC_AHB2PeriphClockCmd(RCC_AHB2Periph_RNG, ENABLE);
    //复位
    RCC_AHB2PeriphResetCmd(RCC_AHB2Periph_RNG,ENABLE);
    //取消复位
    RCC_AHB2PeriphResetCmd(RCC_AHB2Periph_RNG,DISABLE);
    //使能RNG
    RNG_Cmd(ENABLE);
    //等待随机数就绪
    while (RNG_GetFlagStatus(RNG_FLAG_DRDY) == RESET)
    {
        asm("nop");
    }
}

//得到随机数
uint32_t MCU_RandomGetNext(void)
{
    int randomValue;
    MCU_RANDOM_LOCK();
    //等待随机数就绪
    while (RNG_GetFlagStatus(RNG_FLAG_DRDY) == RESET);
    //获取随机数值并返回
    randomValue = RNG_GetRandomNumber();
    MCU_RANDOM_UNLOCK();
    return (uint32_t)randomValue;
}

//生成指定范围的随机数
int MCU_RandomGetNextRange(int min, int max)
{
    //此处其实并不是真的随机数发生器,运算不正确
    return MCU_RandomGetNext() % (max - min + 1) + min;
}

//正确计算随机数的方法
int MCU_RandomGetNextRangeReal(int min, int max)
{
    if (max <= min)
        return 0;
    else
    {
        //基础值
        uint32_t baseResult = MCU_RandomGetNext();
        //将取得的值在32位数据中计算到的位置计算出来,阈值在1
        //然后根据本次阈值扩张,然后取整
        uint32_t rangeBase = (uint32_t)((max - min) * (((double)baseResult) / ((double)UINT32_MAX)));
        //最后确定符号位
        return (int)(min + rangeBase);
    }
}
