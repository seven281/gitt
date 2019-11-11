#include "MCU_SPI.h"
#include "MCU_Port.h"

/**系统内置SPI初始化特性 */
static SPI_InitTypeDef  SPI1_InitStructure;

//SPI 初始化
void MCU_SPI1_Init(MCU_SPI_SPEED speed,MCU_SPI_CPOL cpol,MCU_SPI_CPHA cpha)
{
    //时钟初始化
	RCC_APB2PeriphClockCmd(RCC_APB2Periph_SPI1, ENABLE);
    
    //IO口初始化
    //SCK
    MCU_PortInit(MCU_PIN_A_5, GPIO_Mode_AF, GPIO_OType_PP,GPIO_PuPd_UP,GPIO_Speed_100MHz);
    //MISO
    MCU_PortInit(MCU_PIN_A_6, GPIO_Mode_AF, GPIO_OType_PP,GPIO_PuPd_UP,GPIO_Speed_100MHz);
    //MOSI
    MCU_PortInit(MCU_PIN_A_7, GPIO_Mode_AF, GPIO_OType_PP,GPIO_PuPd_UP,GPIO_Speed_100MHz);
    //复用初始化
    GPIO_PinAFConfig(GPIOA,GPIO_PinSource5,GPIO_AF_SPI1);
    GPIO_PinAFConfig(GPIOA,GPIO_PinSource6,GPIO_AF_SPI1);
    GPIO_PinAFConfig(GPIOA,GPIO_PinSource7,GPIO_AF_SPI1);

    //SPI初始化
    //这里只针对SPI口初始化 复位SPI1
	RCC_APB2PeriphResetCmd(RCC_APB2Periph_SPI1,ENABLE);
	//停止复位SPI1
	RCC_APB2PeriphResetCmd(RCC_APB2Periph_SPI1,DISABLE);
    //禁止SPI外设
	SPI_Cmd(SPI1, DISABLE);

    //设置SPI单向或者双向的数据模式:SPI设置为双线双向全双工
	SPI1_InitStructure.SPI_Direction = SPI_Direction_2Lines_FullDuplex;
	//设置SPI工作模式:设置为主SPI
	SPI1_InitStructure.SPI_Mode = SPI_Mode_Master;
	//设置SPI的数据大小:SPI发送接收8位帧结构
	SPI1_InitStructure.SPI_DataSize = SPI_DataSize_8b;
	//串行同步时钟的空闲状态为高电平
	SPI1_InitStructure.SPI_CPOL = cpol;
	//串行同步时钟的第二个跳变沿（上升或下降）数据被采样
	SPI1_InitStructure.SPI_CPHA = cpha;
	//NSS信号由硬件（NSS管脚）还是软件（使用SSI位）管理:内部NSS信号有SSI位控制
	SPI1_InitStructure.SPI_NSS = SPI_NSS_Soft;
	//定义波特率预分频的值:波特率预分频值为256
	SPI1_InitStructure.SPI_BaudRatePrescaler = speed;
	//指定数据传输从MSB位还是LSB位开始:数据传输从MSB位开始
	SPI1_InitStructure.SPI_FirstBit = SPI_FirstBit_MSB;
	//CRC值计算的多项式
	SPI1_InitStructure.SPI_CRCPolynomial = 7;

    //根据SPI_InitStruct中指定的参数初始化外设SPIx寄存器
	SPI_Init(SPI1, &SPI1_InitStructure);
	//使能SPI外设
	SPI_Cmd(SPI1, ENABLE);

    MCU_SPI1_WriteRead(0x00);
}

//读写数据
uint8_t MCU_SPI1_WriteRead(uint8_t writeDat)
{
    uint16_t waitCount = 0;
    //等待发送区空
	while (SPI_I2S_GetFlagStatus(SPI1, SPI_I2S_FLAG_TXE) == RESET);
	//通过外设SPIx发送一个byte  数据
	SPI_I2S_SendData(SPI1, writeDat);
	//等待接收完一个byte
	while (SPI_I2S_GetFlagStatus(SPI1, SPI_I2S_FLAG_RXNE) == RESET)
    {
        waitCount++;
        if(waitCount > MCU_SPI_WAIT_COUNT_MAX)
        {
            return 0;
        }
    }
	//返回通过SPIx最近接收的数据
	return SPI_I2S_ReceiveData(SPI1);
}

//设置速度
void MCU_SPI1_SetSpeed(MCU_SPI_SPEED speed)
{
    MCU_SPI_THREAD_LOCK();
    //禁止SPI外设
	SPI_Cmd(SPI1, DISABLE);
    //定义波特率预分频的值:波特率预分频值为256
	SPI1_InitStructure.SPI_BaudRatePrescaler = speed;
    //根据SPI_InitStruct中指定的参数初始化外设SPIx寄存器
	SPI_Init(SPI1, &SPI1_InitStructure);
    //使能SPI1
	SPI_Cmd(SPI1,ENABLE);
    MCU_SPI_THREAD_UNLOCK();
}

//设置信号极性
void MCU_SPI1_SetPolarity(MCU_SPI_CPOL cpol,MCU_SPI_CPHA cpha)
{
    MCU_SPI_THREAD_LOCK();
    //禁止SPI外设
	SPI_Cmd(SPI1, DISABLE);
    //串行同步时钟的空闲状态为高电平
	SPI1_InitStructure.SPI_CPOL = cpol;
	//串行同步时钟的第二个跳变沿（上升或下降）数据被采样
	SPI1_InitStructure.SPI_CPHA = cpha;
    //根据SPI_InitStruct中指定的参数初始化外设SPIx寄存器
	SPI_Init(SPI1, &SPI1_InitStructure);
    //使能SPI1
	SPI_Cmd(SPI1,ENABLE);
    MCU_SPI_THREAD_UNLOCK();
}


/**系统内置SPI初始化特性 */
static SPI_InitTypeDef  SPI2_InitStructure;

//SPI 初始化
void MCU_SPI2_Init(MCU_SPI_SPEED speed,MCU_SPI_CPOL cpol,MCU_SPI_CPHA cpha)
{
    //时钟初始化
	RCC_APB1PeriphClockCmd(RCC_APB1Periph_SPI2, ENABLE);

    //IO口初始化
    //SCK
    MCU_PortInit(MCU_PIN_B_13, GPIO_Mode_AF, GPIO_OType_PP,GPIO_PuPd_UP,GPIO_Speed_100MHz);
    //MISO
    MCU_PortInit(MCU_PIN_B_14, GPIO_Mode_AF, GPIO_OType_PP,GPIO_PuPd_UP,GPIO_Speed_100MHz);
    //MOSI
    MCU_PortInit(MCU_PIN_B_15, GPIO_Mode_AF, GPIO_OType_PP,GPIO_PuPd_UP,GPIO_Speed_100MHz);

    //复用初始化
    GPIO_PinAFConfig(GPIOB,GPIO_PinSource13,GPIO_AF_SPI2);
    GPIO_PinAFConfig(GPIOB,GPIO_PinSource14,GPIO_AF_SPI2);
    GPIO_PinAFConfig(GPIOB,GPIO_PinSource15,GPIO_AF_SPI2);
    
    //SPI初始化
    //这里只针对SPI口初始化 复位SPI1
	RCC_APB1PeriphResetCmd(RCC_APB1Periph_SPI2,ENABLE);
	//停止复位SPI1
	RCC_APB1PeriphResetCmd(RCC_APB1Periph_SPI2,DISABLE);
    //禁止SPI外设
	SPI_Cmd(SPI2, DISABLE);

    //设置SPI单向或者双向的数据模式:SPI设置为双线双向全双工
	SPI2_InitStructure.SPI_Direction = SPI_Direction_2Lines_FullDuplex;
	//设置SPI工作模式:设置为主SPI
	SPI2_InitStructure.SPI_Mode = SPI_Mode_Master;
	//设置SPI的数据大小:SPI发送接收8位帧结构
	SPI2_InitStructure.SPI_DataSize = SPI_DataSize_8b;
	//串行同步时钟的空闲状态为高电平
	SPI2_InitStructure.SPI_CPOL = cpol;
	//串行同步时钟的第二个跳变沿（上升或下降）数据被采样
	SPI2_InitStructure.SPI_CPHA = cpha;
	//NSS信号由硬件（NSS管脚）还是软件（使用SSI位）管理:内部NSS信号有SSI位控制
	SPI2_InitStructure.SPI_NSS = SPI_NSS_Soft;
	//定义波特率预分频的值:波特率预分频值为256
	SPI2_InitStructure.SPI_BaudRatePrescaler = speed;
	//指定数据传输从MSB位还是LSB位开始:数据传输从MSB位开始
	SPI2_InitStructure.SPI_FirstBit = SPI_FirstBit_MSB;
	//CRC值计算的多项式
	SPI2_InitStructure.SPI_CRCPolynomial = 7;

    //根据SPI_InitStruct中指定的参数初始化外设SPIx寄存器
	SPI_Init(SPI2, &SPI2_InitStructure);
	//使能SPI外设
	SPI_Cmd(SPI2, ENABLE);

    MCU_SPI2_WriteRead(0x00);
}

//读写数据
uint8_t MCU_SPI2_WriteRead(uint8_t writeDat)
{
    uint16_t waitCount = 0;
    //等待发送区空
	while (SPI_I2S_GetFlagStatus(SPI2, SPI_I2S_FLAG_TXE) == RESET);
	//通过外设SPIx发送一个byte  数据
	SPI_I2S_SendData(SPI2, writeDat);
	//等待接收完一个byte
	while (SPI_I2S_GetFlagStatus(SPI2, SPI_I2S_FLAG_RXNE) == RESET)
    {
        waitCount++;
        if(waitCount > MCU_SPI_WAIT_COUNT_MAX)
        {
            return 0;
        }
    }
	//返回通过SPIx最近接收的数据
	return SPI_I2S_ReceiveData(SPI2);
}


//设置速度
void MCU_SPI2_SetSpeed(MCU_SPI_SPEED speed)
{
    MCU_SPI_THREAD_LOCK();
    //禁止SPI外设
	SPI_Cmd(SPI2, DISABLE);
    //定义波特率预分频的值:波特率预分频值为256
	SPI2_InitStructure.SPI_BaudRatePrescaler = speed;
    //根据SPI_InitStruct中指定的参数初始化外设SPIx寄存器
	SPI_Init(SPI2, &SPI2_InitStructure);
    //使能SPI2
	SPI_Cmd(SPI2,ENABLE);
    MCU_SPI_THREAD_UNLOCK();
}

//设置信号极性
void MCU_SPI2_SetPolarity(MCU_SPI_CPOL cpol,MCU_SPI_CPHA cpha)
{
    MCU_SPI_THREAD_LOCK();
    //禁止SPI外设
	SPI_Cmd(SPI2, DISABLE);
    //串行同步时钟的空闲状态为高电平
	SPI2_InitStructure.SPI_CPOL = cpol;
	//串行同步时钟的第二个跳变沿（上升或下降）数据被采样
	SPI2_InitStructure.SPI_CPHA = cpha;
    //根据SPI_InitStruct中指定的参数初始化外设SPIx寄存器
	SPI_Init(SPI2, &SPI2_InitStructure);
    //使能SPI1
	SPI_Cmd(SPI2,ENABLE);
    MCU_SPI_THREAD_UNLOCK();
}























