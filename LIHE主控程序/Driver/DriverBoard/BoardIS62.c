#include "BoardIS62.h"

//初始化SRAM接口
LH_ERR BoardIS62Init(void)
{
    FSMC_NORSRAMInitTypeDef  FSMC_NORSRAMInitStructure;
	FSMC_NORSRAMTimingInitTypeDef  readWriteTiming;
    //使能FSMC时钟
	RCC_AHB3PeriphClockCmd(RCC_AHB3Periph_FSMC,ENABLE);

    //复位
    RCC_AHB3PeriphResetCmd(RCC_AHB3Periph_FSMC,ENABLE);
    //解除复位
    RCC_AHB3PeriphResetCmd(RCC_AHB3Periph_FSMC,DISABLE);

    //对应IO口初始化
    //A0 - A18
    MCU_PortInit(MCU_PIN_F_0, GPIO_Mode_AF, GPIO_OType_PP, GPIO_PuPd_UP, GPIO_Speed_100MHz);
    MCU_PortInit(MCU_PIN_F_1, GPIO_Mode_AF, GPIO_OType_PP, GPIO_PuPd_UP, GPIO_Speed_100MHz);
    MCU_PortInit(MCU_PIN_F_2, GPIO_Mode_AF, GPIO_OType_PP, GPIO_PuPd_UP, GPIO_Speed_100MHz);
    MCU_PortInit(MCU_PIN_F_3, GPIO_Mode_AF, GPIO_OType_PP, GPIO_PuPd_UP, GPIO_Speed_100MHz);
    MCU_PortInit(MCU_PIN_F_4, GPIO_Mode_AF, GPIO_OType_PP, GPIO_PuPd_UP, GPIO_Speed_100MHz);
    MCU_PortInit(MCU_PIN_F_5, GPIO_Mode_AF, GPIO_OType_PP, GPIO_PuPd_UP, GPIO_Speed_100MHz);
    MCU_PortInit(MCU_PIN_F_12, GPIO_Mode_AF, GPIO_OType_PP, GPIO_PuPd_UP, GPIO_Speed_100MHz);
    MCU_PortInit(MCU_PIN_F_13, GPIO_Mode_AF, GPIO_OType_PP, GPIO_PuPd_UP, GPIO_Speed_100MHz);
    MCU_PortInit(MCU_PIN_F_14, GPIO_Mode_AF, GPIO_OType_PP, GPIO_PuPd_UP, GPIO_Speed_100MHz);
    MCU_PortInit(MCU_PIN_F_15, GPIO_Mode_AF, GPIO_OType_PP, GPIO_PuPd_UP, GPIO_Speed_100MHz);
    MCU_PortInit(MCU_PIN_G_0, GPIO_Mode_AF, GPIO_OType_PP, GPIO_PuPd_UP, GPIO_Speed_100MHz);
    MCU_PortInit(MCU_PIN_G_1, GPIO_Mode_AF, GPIO_OType_PP, GPIO_PuPd_UP, GPIO_Speed_100MHz);
    MCU_PortInit(MCU_PIN_G_2, GPIO_Mode_AF, GPIO_OType_PP, GPIO_PuPd_UP, GPIO_Speed_100MHz);
    MCU_PortInit(MCU_PIN_G_3, GPIO_Mode_AF, GPIO_OType_PP, GPIO_PuPd_UP, GPIO_Speed_100MHz);
    MCU_PortInit(MCU_PIN_G_4, GPIO_Mode_AF, GPIO_OType_PP, GPIO_PuPd_UP, GPIO_Speed_100MHz);
    MCU_PortInit(MCU_PIN_G_5, GPIO_Mode_AF, GPIO_OType_PP, GPIO_PuPd_UP, GPIO_Speed_100MHz);
    MCU_PortInit(MCU_PIN_D_11, GPIO_Mode_AF, GPIO_OType_PP, GPIO_PuPd_UP, GPIO_Speed_100MHz);
    MCU_PortInit(MCU_PIN_D_12, GPIO_Mode_AF, GPIO_OType_PP, GPIO_PuPd_UP, GPIO_Speed_100MHz);
    MCU_PortInit(MCU_PIN_D_13, GPIO_Mode_AF, GPIO_OType_PP, GPIO_PuPd_UP, GPIO_Speed_100MHz);

    //D0 - D15
    MCU_PortInit(MCU_PIN_D_14, GPIO_Mode_AF, GPIO_OType_PP, GPIO_PuPd_UP, GPIO_Speed_100MHz);
    MCU_PortInit(MCU_PIN_D_15, GPIO_Mode_AF, GPIO_OType_PP, GPIO_PuPd_UP, GPIO_Speed_100MHz);
    MCU_PortInit(MCU_PIN_D_0, GPIO_Mode_AF, GPIO_OType_PP, GPIO_PuPd_UP, GPIO_Speed_100MHz);
    MCU_PortInit(MCU_PIN_D_1, GPIO_Mode_AF, GPIO_OType_PP, GPIO_PuPd_UP, GPIO_Speed_100MHz);
    MCU_PortInit(MCU_PIN_E_7, GPIO_Mode_AF, GPIO_OType_PP, GPIO_PuPd_UP, GPIO_Speed_100MHz);
    MCU_PortInit(MCU_PIN_E_8, GPIO_Mode_AF, GPIO_OType_PP, GPIO_PuPd_UP, GPIO_Speed_100MHz);
    MCU_PortInit(MCU_PIN_E_9, GPIO_Mode_AF, GPIO_OType_PP, GPIO_PuPd_UP, GPIO_Speed_100MHz);
    MCU_PortInit(MCU_PIN_E_10, GPIO_Mode_AF, GPIO_OType_PP, GPIO_PuPd_UP, GPIO_Speed_100MHz);
    MCU_PortInit(MCU_PIN_E_11, GPIO_Mode_AF, GPIO_OType_PP, GPIO_PuPd_UP, GPIO_Speed_100MHz);
    MCU_PortInit(MCU_PIN_E_12, GPIO_Mode_AF, GPIO_OType_PP, GPIO_PuPd_UP, GPIO_Speed_100MHz);
    MCU_PortInit(MCU_PIN_E_13, GPIO_Mode_AF, GPIO_OType_PP, GPIO_PuPd_UP, GPIO_Speed_100MHz);
    MCU_PortInit(MCU_PIN_E_14, GPIO_Mode_AF, GPIO_OType_PP, GPIO_PuPd_UP, GPIO_Speed_100MHz);
    MCU_PortInit(MCU_PIN_E_15, GPIO_Mode_AF, GPIO_OType_PP, GPIO_PuPd_UP, GPIO_Speed_100MHz);
    MCU_PortInit(MCU_PIN_D_8, GPIO_Mode_AF, GPIO_OType_PP, GPIO_PuPd_UP, GPIO_Speed_100MHz);
    MCU_PortInit(MCU_PIN_D_9, GPIO_Mode_AF, GPIO_OType_PP, GPIO_PuPd_UP, GPIO_Speed_100MHz);
    MCU_PortInit(MCU_PIN_D_10, GPIO_Mode_AF, GPIO_OType_PP, GPIO_PuPd_UP, GPIO_Speed_100MHz);

    //NE4
    MCU_PortInit(MCU_PIN_G_12, GPIO_Mode_AF, GPIO_OType_PP, GPIO_PuPd_UP, GPIO_Speed_100MHz);
    //NWE
    MCU_PortInit(MCU_PIN_D_5, GPIO_Mode_AF, GPIO_OType_PP, GPIO_PuPd_UP, GPIO_Speed_100MHz);
    //NOE
    MCU_PortInit(MCU_PIN_D_4, GPIO_Mode_AF, GPIO_OType_PP, GPIO_PuPd_UP, GPIO_Speed_100MHz);
    //NBL0
    MCU_PortInit(MCU_PIN_E_0, GPIO_Mode_AF, GPIO_OType_PP, GPIO_PuPd_UP, GPIO_Speed_100MHz);
    //NBL1
    MCU_PortInit(MCU_PIN_E_1, GPIO_Mode_AF, GPIO_OType_PP, GPIO_PuPd_UP, GPIO_Speed_100MHz);

    //设定端口复用
    //A0 - A18
    GPIO_PinAFConfig(GPIOF,GPIO_PinSource0,GPIO_AF_FSMC);
    GPIO_PinAFConfig(GPIOF,GPIO_PinSource1,GPIO_AF_FSMC);
    GPIO_PinAFConfig(GPIOF,GPIO_PinSource2,GPIO_AF_FSMC);
    GPIO_PinAFConfig(GPIOF,GPIO_PinSource3,GPIO_AF_FSMC);
    GPIO_PinAFConfig(GPIOF,GPIO_PinSource4,GPIO_AF_FSMC);
    GPIO_PinAFConfig(GPIOF,GPIO_PinSource5,GPIO_AF_FSMC);
    GPIO_PinAFConfig(GPIOF,GPIO_PinSource12,GPIO_AF_FSMC);
    GPIO_PinAFConfig(GPIOF,GPIO_PinSource13,GPIO_AF_FSMC);
    GPIO_PinAFConfig(GPIOF,GPIO_PinSource14,GPIO_AF_FSMC);
    GPIO_PinAFConfig(GPIOF,GPIO_PinSource15,GPIO_AF_FSMC);
    GPIO_PinAFConfig(GPIOG,GPIO_PinSource0,GPIO_AF_FSMC);
    GPIO_PinAFConfig(GPIOG,GPIO_PinSource1,GPIO_AF_FSMC);
    GPIO_PinAFConfig(GPIOG,GPIO_PinSource2,GPIO_AF_FSMC);
    GPIO_PinAFConfig(GPIOG,GPIO_PinSource3,GPIO_AF_FSMC);
    GPIO_PinAFConfig(GPIOG,GPIO_PinSource4,GPIO_AF_FSMC);
    GPIO_PinAFConfig(GPIOG,GPIO_PinSource5,GPIO_AF_FSMC);
    GPIO_PinAFConfig(GPIOD,GPIO_PinSource11,GPIO_AF_FSMC);
    GPIO_PinAFConfig(GPIOD,GPIO_PinSource12,GPIO_AF_FSMC);
    GPIO_PinAFConfig(GPIOD,GPIO_PinSource13,GPIO_AF_FSMC);

    //D0-D15
    GPIO_PinAFConfig(GPIOD,GPIO_PinSource14,GPIO_AF_FSMC);
    GPIO_PinAFConfig(GPIOD,GPIO_PinSource15,GPIO_AF_FSMC);
    GPIO_PinAFConfig(GPIOD,GPIO_PinSource0,GPIO_AF_FSMC);
    GPIO_PinAFConfig(GPIOD,GPIO_PinSource1,GPIO_AF_FSMC);
    GPIO_PinAFConfig(GPIOE,GPIO_PinSource7,GPIO_AF_FSMC);
    GPIO_PinAFConfig(GPIOE,GPIO_PinSource8,GPIO_AF_FSMC);
    GPIO_PinAFConfig(GPIOE,GPIO_PinSource9,GPIO_AF_FSMC);
    GPIO_PinAFConfig(GPIOE,GPIO_PinSource10,GPIO_AF_FSMC);
    GPIO_PinAFConfig(GPIOE,GPIO_PinSource11,GPIO_AF_FSMC);
    GPIO_PinAFConfig(GPIOE,GPIO_PinSource12,GPIO_AF_FSMC);
    GPIO_PinAFConfig(GPIOE,GPIO_PinSource13,GPIO_AF_FSMC);
    GPIO_PinAFConfig(GPIOE,GPIO_PinSource14,GPIO_AF_FSMC);
    GPIO_PinAFConfig(GPIOE,GPIO_PinSource15,GPIO_AF_FSMC);
    GPIO_PinAFConfig(GPIOD,GPIO_PinSource8,GPIO_AF_FSMC);
    GPIO_PinAFConfig(GPIOD,GPIO_PinSource9,GPIO_AF_FSMC);
    GPIO_PinAFConfig(GPIOD,GPIO_PinSource10,GPIO_AF_FSMC);

    //NE4
    GPIO_PinAFConfig(GPIOG,GPIO_PinSource12,GPIO_AF_FSMC);
    //NWE
    GPIO_PinAFConfig(GPIOD,GPIO_PinSource5,GPIO_AF_FSMC);
    //NOE
    GPIO_PinAFConfig(GPIOD,GPIO_PinSource4,GPIO_AF_FSMC);
    //NBL0
    GPIO_PinAFConfig(GPIOE,GPIO_PinSource0,GPIO_AF_FSMC);
    //NBL1
    GPIO_PinAFConfig(GPIOE,GPIO_PinSource1,GPIO_AF_FSMC);

    //地址建立时间（ADDSET）为1个HCLK 1/36M=27ns
	readWriteTiming.FSMC_AddressSetupTime = 0x00;
	//地址保持时间（ADDHLD）模式A未用到
	readWriteTiming.FSMC_AddressHoldTime = 0x00;
	//数据保持时间（DATAST）为9个HCLK 6*9=54ns
	readWriteTiming.FSMC_DataSetupTime = 0x08;
	readWriteTiming.FSMC_BusTurnAroundDuration = 0x00;
	readWriteTiming.FSMC_CLKDivision = 0x00;
	readWriteTiming.FSMC_DataLatency = 0x00;
	//模式A
	readWriteTiming.FSMC_AccessMode = FSMC_AccessMode_A;

    //  这里我们使用NE4 ，也就对应BTCR[4],[5]。
	FSMC_NORSRAMInitStructure.FSMC_Bank = FSMC_Bank1_NORSRAM4;
	FSMC_NORSRAMInitStructure.FSMC_DataAddressMux = FSMC_DataAddressMux_Disable;
	// FSMC_MemoryType_SRAM
	FSMC_NORSRAMInitStructure.FSMC_MemoryType =FSMC_MemoryType_SRAM;
	//存储器数据宽度为16bit
	FSMC_NORSRAMInitStructure.FSMC_MemoryDataWidth = FSMC_MemoryDataWidth_16b;
	// FSMC_BurstAccessMode_Disable;
	FSMC_NORSRAMInitStructure.FSMC_BurstAccessMode =FSMC_BurstAccessMode_Disable;
	FSMC_NORSRAMInitStructure.FSMC_WaitSignalPolarity = FSMC_WaitSignalPolarity_Low;
	FSMC_NORSRAMInitStructure.FSMC_AsynchronousWait=FSMC_AsynchronousWait_Disable;
	FSMC_NORSRAMInitStructure.FSMC_WrapMode = FSMC_WrapMode_Disable;
	FSMC_NORSRAMInitStructure.FSMC_WaitSignalActive = FSMC_WaitSignalActive_BeforeWaitState;
	//存储器写使能
	FSMC_NORSRAMInitStructure.FSMC_WriteOperation = FSMC_WriteOperation_Enable;
	FSMC_NORSRAMInitStructure.FSMC_WaitSignal = FSMC_WaitSignal_Disable;
	// 读写使用相同的时序
	FSMC_NORSRAMInitStructure.FSMC_ExtendedMode = FSMC_ExtendedMode_Disable;
	FSMC_NORSRAMInitStructure.FSMC_WriteBurst = FSMC_WriteBurst_Disable;
	FSMC_NORSRAMInitStructure.FSMC_ReadWriteTimingStruct = &readWriteTiming;
	//读写同样时序
	FSMC_NORSRAMInitStructure.FSMC_WriteTimingStruct = &readWriteTiming;
	//初始化FSMC配置
	FSMC_NORSRAMInit(&FSMC_NORSRAMInitStructure);
	// 使能BANK1区域4
	FSMC_NORSRAMCmd(FSMC_Bank1_NORSRAM4, ENABLE);

    return BoardIS62SelfCheck();
}

LH_ERR BoardIS62ReadBuffer(uint32_t baseAddr,uint8_t* bufferPtr,uint32_t length)
{
    if (baseAddr < BOARD_IS62_BASE_ADDR || baseAddr >(BOARD_IS62_BASE_ADDR + BOARD_IS62_CAPACITY))
        return LH_ERR_MAIN_SRAM_ADDR;
    if ((baseAddr + length-1) > (BOARD_IS62_BASE_ADDR + BOARD_IS62_CAPACITY))
        return LH_ERR_MAIN_SRAM_ADDR;
    for (; length != 0; length--)
    {
        *bufferPtr++ = *(volatile uint8_t*)(baseAddr);
        baseAddr++;
    }
    return LH_ERR_NONE;
}

LH_ERR BoardIS62WriteBuffer(uint32_t baseAddr,uint8_t* bufferPtr,uint32_t length)
{
    if (baseAddr < BOARD_IS62_BASE_ADDR || baseAddr >(BOARD_IS62_BASE_ADDR + BOARD_IS62_CAPACITY))
        return LH_ERR_MAIN_SRAM_ADDR;
    if ((baseAddr + length-1) > (BOARD_IS62_BASE_ADDR + BOARD_IS62_CAPACITY))
        return LH_ERR_MAIN_SRAM_ADDR;
    for (; length != 0; length--)
    {
        *(volatile uint8_t*)(baseAddr) = *bufferPtr;
        baseAddr++;
        bufferPtr++;
    }
    return LH_ERR_NONE;
}


//设备自检,先全部写成0xff,检查,后全部写成0x00,检查
//经过自检,SRAM的所有数据均为0
LH_ERR BoardIS62SelfCheck(void)
{
    uint32_t baseAddr = BOARD_IS62_BASE_ADDR;
    uint32_t length = 0;
    //首先全部写入
    for(length = 0; length < BOARD_IS62_CAPACITY;length++)
    {
        *(volatile uint8_t*)(baseAddr) = (length%0XFF);
        baseAddr++;
    }

    //检查是不是写入的值
    baseAddr = BOARD_IS62_BASE_ADDR;
    for(length = 0; length < BOARD_IS62_CAPACITY;length++)
    {
        if((length%0XFF) != (*(volatile uint8_t*)(baseAddr)))
        {
            return LH_ERR_MAIN_SRAM_CHECK;
        }
        baseAddr++;
    }

    //然后全部写入0x00
    baseAddr = BOARD_IS62_BASE_ADDR;
    for(length = 0; length < BOARD_IS62_CAPACITY;length++)
    {
        *(volatile uint8_t*)(baseAddr) = 0x00;
        baseAddr++;
    }

    //检查是不是全部是0x00
    baseAddr = BOARD_IS62_BASE_ADDR;
    for(length = 0; length < BOARD_IS62_CAPACITY;length++)
    {
        if(0x00 != (*(volatile uint8_t*)(baseAddr)))
        {
            return LH_ERR_MAIN_SRAM_CHECK;
        }
        baseAddr++;
    }
    return LH_ERR_NONE;
}
