#include "CoreUtil.h"


void WFI_SET(void)
{
	asm("WFI");
}
//关闭所有中断(但是不包括fault和NMI中断)
void INTX_DISABLE(void)
{
	asm("CPSID   I");
	asm("BX      LR");
}
//开启所有中断
void INTX_ENABLE(void)
{
	asm("CPSIE   I");
	asm("BX      LR");
}
//设置栈顶地址
//addr:栈顶地址
//set Main Stack value
void MSR_MSP(uint32_t addr)
{
	asm("MSR MSP, r0");
	asm("BX r14");
}

/*设置默认的中断优先级分组*/
void CoreNVIC_SetGroupDefault(void)
{
    /*系统NVIC中断分组初始化*/
	NVIC_PriorityGroupConfig(SYSTEM_DEFAULT_NVIC_GROUP);
}





