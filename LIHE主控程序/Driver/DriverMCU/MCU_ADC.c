#include "MCU_ADC.h"
#include "MCU_Port.h"

//ADC初始化
void MCU_ADC_Init(void)
{
    ADC_CommonInitTypeDef ADC_CommonInitStructure;
    ADC_InitTypeDef ADC_InitStructure;

    //使能ADC1时钟
    RCC_APB2PeriphClockCmd(RCC_APB2Periph_ADC1, ENABLE);  

    //CHANNEL 10 11 12 13
    MCU_PortInit(MCU_PIN_C_0, GPIO_Mode_AN, GPIO_OType_PP, GPIO_PuPd_NOPULL, GPIO_Speed_25MHz);
    MCU_PortInit(MCU_PIN_C_1, GPIO_Mode_AN, GPIO_OType_PP, GPIO_PuPd_NOPULL, GPIO_Speed_25MHz);
    MCU_PortInit(MCU_PIN_C_2, GPIO_Mode_AN, GPIO_OType_PP, GPIO_PuPd_NOPULL, GPIO_Speed_25MHz);
    MCU_PortInit(MCU_PIN_C_3, GPIO_Mode_AN, GPIO_OType_PP, GPIO_PuPd_NOPULL, GPIO_Speed_25MHz);

    //ADC1复位
    RCC_APB2PeriphResetCmd(RCC_APB2Periph_ADC1, ENABLE);  
    //复位结束
    RCC_APB2PeriphResetCmd(RCC_APB2Periph_ADC1, DISABLE); 

    //使能内部温度传感器
    ADC_TempSensorVrefintCmd(ENABLE);

    ADC_CommonInitStructure.ADC_Mode = ADC_Mode_Independent;                     //独立模式
    ADC_CommonInitStructure.ADC_TwoSamplingDelay = ADC_TwoSamplingDelay_5Cycles; //两个采样阶段之间的延迟5个时钟
    ADC_CommonInitStructure.ADC_DMAAccessMode = ADC_DMAAccessMode_Disabled;      //DMA失能
    ADC_CommonInitStructure.ADC_Prescaler = ADC_Prescaler_Div4;                  //预分频4分频。ADCCLK=PCLK2/4=84/4=21Mhz,ADC时钟最好不要超过36Mhz
    ADC_CommonInit(&ADC_CommonInitStructure);                                    //初始化

    ADC_InitStructure.ADC_Resolution = ADC_Resolution_12b;                      //12位模式
    ADC_InitStructure.ADC_ScanConvMode = DISABLE;                               //非扫描模式
    ADC_InitStructure.ADC_ContinuousConvMode = DISABLE;                         //关闭连续转换
    ADC_InitStructure.ADC_ExternalTrigConvEdge = ADC_ExternalTrigConvEdge_None; //禁止触发检测，使用软件触发
    ADC_InitStructure.ADC_DataAlign = ADC_DataAlign_Right;                      //右对齐
    ADC_InitStructure.ADC_NbrOfConversion = 1;                                  //1个转换在规则序列中 也就是只转换规则序列1
    ADC_Init(ADC1, &ADC_InitStructure);                                         //ADC初始化

    //开启AD转换器
    ADC_Cmd(ADC1, ENABLE); 
}

//ADC读取当前通道值
uint16_t MCU_ADC_ReadChannelValue(uint8_t channel)
{
    uint16_t result = 0;
    float vol = 0.0;
    //设置指定ADC的规则组通道，一个序列，采样时间
    //ADC1,ADC通道,480个周期,提高采样时间可以提高精确度	
	ADC_RegularChannelConfig(ADC1, channel, 1, ADC_SampleTime_480Cycles);			    
    //使能指定的ADC1的软件转换启动功能	
	ADC_SoftwareStartConv(ADC1);		
    //等待转换结束
	while(!ADC_GetFlagStatus(ADC1, ADC_FLAG_EOC ));
    //返回最近一次ADC1规则组的转换结果
	result = ADC_GetConversionValue(ADC1);	
    vol = result;
    vol /= 4096.0;
    vol *= 3300;
    result = (uint16_t)vol;
    return result;
}

//ADC读取当前通道值并取平均
uint16_t MCU_ADC_ReadChannelValueAverage(uint8_t channel,uint16_t count)
{
    uint32_t resultAdd = 0;
    uint16_t index = 0;
    for(index = 0;index < count; index++)
    {
        resultAdd += MCU_ADC_ReadChannelValue(channel);
    }
    return (uint16_t)(resultAdd/count);
}

//ADC读取当前芯片温度
float MCU_ADC_ReadChipTemp(void)
{
    uint32_t adcx;
 	float temperate;
    //读取通道16内部温度传感器通道,10次取平均
	adcx=MCU_ADC_ReadChannelValueAverage(ADC_Channel_16,10);
    //电压值	
	temperate=((float)adcx)/1000.0;		
    //转换为温度值 
	temperate=(temperate-0.76)/0.0025 + 25; 
    return temperate;
}



