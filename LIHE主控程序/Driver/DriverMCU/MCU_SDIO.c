#include "MCU_SDIO.h"
#include "MCU_Port.h"

/*用于sdio初始化的结构体*/
SDIO_InitTypeDef SDIO_InitStructure;
SDIO_CmdInitTypeDef SDIO_CmdInitStructure;
SDIO_DataInitTypeDef SDIO_DataInitStructure;

SD_Error CmdError(void);
SD_Error CmdResp7Error(void);
SD_Error CmdResp1Error(uint8_t cmd);
SD_Error CmdResp3Error(void);
SD_Error CmdResp2Error(void);
SD_Error CmdResp6Error(uint8_t cmd, uint16_t *prca);
SD_Error SDEnWideBus(uint8_t enx);
SD_Error IsCardProgramming(uint8_t *pstatus);
SD_Error FindSCR(uint16_t rca, uint32_t *pscr);
uint8_t convert_from_bytes_to_power_of_two(uint16_t NumberOfBytes);

/**************************************函数预定义******************************************/
//SDIO时钟初始化设置
//clkdiv:时钟分频系数
//CK时钟=SDIOCLK/[clkdiv+2];(SDIOCLK时钟固定为48Mhz)
void SDIO_ClockSet(uint8_t clkdiv);

//卡上电
//查询所有SDIO接口上的卡设备,并查询其电压和配置时钟
//返回值:错误代码;(0,无错误)
SD_Error SD_PowerON(void);

//SD卡 Power OFF
//返回值:错误代码;(0,无错误)
SD_Error SD_PowerOFF(void);

//初始化所有的卡,并让卡进入就绪状态
//返回值:错误代码
SD_Error SD_InitializeCards(void);

//得到卡信息
//cardinfo:卡信息存储区
//返回值:错误状态
SD_Error SD_GetCardInfo(SD_CardInfo *cardinfo);

//设置SDIO总线宽度(MMC卡不支持4bit模式)
//wmode:位宽模式.0,1位数据宽度;1,4位数据宽度;2,8位数据宽度
//SDIO_BusWide_8b: 8-bit data transfer (Only for MMC)
//SDIO_BusWide_4b: 4-bit data transfer
//SDIO_BusWide_1b: 1-bit data transfer (默认)
//返回值:SD卡错误状态
SD_Error SD_EnableWideBusOperation(uint32_t wmode);

//选卡
//发送CMD7,选择相对地址(rca)为addr的卡,取消其他卡.如果为0,则都不选择.
//addr:卡的RCA地址
SD_Error SD_SelectDeselect(uint32_t addr);

//发送卡状态
SD_Error SD_SendStatus(uint32_t *pcardstatus);

//获取卡状态
SDCardState SD_GetState(void);

//SD卡读取一个块
//buf:读数据缓存区(必须4字节对齐!!)
//addr:读取地址
//blksize:块大小
SD_Error SD_ReadBlock(uint8_t *buf, long long addr, uint16_t blksize);

//SD卡读取多个块
//buf:读数据缓存区
//addr:读取地址
//blksize:块大小
//nblks:要读取的块数
//返回值:错误状态
SD_Error SD_ReadMultiBlocks(uint8_t *buf, long long addr, uint16_t blksize, uint32_t nblks);

//SD卡写1个块
//buf:数据缓存区
//addr:写地址
//blksize:块大小
//返回值:错误状态
SD_Error SD_WriteBlock(uint8_t *buf, long long addr, uint16_t blksize);

//SD卡写多个块
//buf:数据缓存区
//addr:写地址
//blksize:块大小
//nblks:要写入的块数
//返回值:错误状态
SD_Error SD_WriteMultiBlocks(uint8_t *buf, long long addr, uint16_t blksize, uint32_t nblks);

//SDIO中断处理函数
//处理SDIO传输过程中的各种中断事务
//返回值:错误代码
SD_Error SD_ProcessIRQSrc(void);

//配置SDIO DMA
//mbuf:存储器地址
//bufsize:传输数据量
//dir:方向;DMA_DIR_MemoryToPeripheral  存储器-->SDIO(写数据);DMA_DIR_PeripheralToMemory SDIO-->存储器(读数据);
void SD_DMA_Config(uint32_t *mbuf, uint32_t bufsize, uint32_t dir);
/**************************************函数预定义******************************************/

//SD卡类型（默认为1.x卡）
static uint8_t CardType = SDIO_STD_CAPACITY_SD_CARD_V1_1;
//SD卡CSD,CID以及相对地址(RCA)数据
static uint32_t CSD_Tab[4], CID_Tab[4], RCA = 0;
//是否发送停止传输标志位,DMA多块读写的时候用到
static uint8_t StopCondition = 0;
//数据传输错误标志,DMA读写时使用
volatile SD_Error TransferError = SD_OK;
//传输结束标志,DMA读写时使用
volatile uint8_t TransferEnd = 0;
//SD卡信息
SD_CardInfo SDCardInfo;

//SD_ReadDisk/SD_WriteDisk函数专用buf,当这两个函数的数据缓存区地址不是4字节对齐的时候,
//需要用到该数组,确保数据缓存区地址是4字节对齐的.
#pragma pack(4)
uint8_t SDIO_DATA_BUFFER[512];
#pragma pack()

//SD卡访问信号量
OS_MUTEX MUTEX_SDCARD;

void SDIO_Register_Deinit()
{
    SDIO->POWER = 0x00000000;
    SDIO->CLKCR = 0x00000000;
    SDIO->ARG = 0x00000000;
    SDIO->CMD = 0x00000000;
    SDIO->DTIMER = 0x00000000;
    SDIO->DLEN = 0x00000000;
    SDIO->DCTRL = 0x00000000;
    SDIO->ICR = 0x00C007FF;
    SDIO->MASK = 0x00000000;
}

//初始化SD卡
//返回值:错误代码;(0,无错误)
SD_Error SD_Init(void)
{
    OS_ERR err;
    GPIO_InitTypeDef GPIO_InitStructure;
    NVIC_InitTypeDef NVIC_InitStructure;
    //创建信号量
    //创建一个互斥信号量,用于SD卡读写
    OSMutexCreate((OS_MUTEX *)&MUTEX_SDCARD,
                  (CPU_CHAR *)"MUTEX_SDCARD",
                  (OS_ERR *)&err);

    SD_Error errorstatus = SD_OK;
    uint8_t clkdiv = 0;
    //使能GPIOC,GPIOD DMA2时钟
    RCC_AHB1PeriphClockCmd(RCC_AHB1Periph_GPIOC | RCC_AHB1Periph_GPIOD, ENABLE);
    //SDIO时钟使能
    RCC_APB2PeriphClockCmd(RCC_APB2Periph_SDIO, ENABLE);
    
    //SDIO复位
    RCC_APB2PeriphResetCmd(RCC_APB2Periph_SDIO, ENABLE);

    //PC8,9,10,11,12复用功能输出
    GPIO_InitStructure.GPIO_Pin = GPIO_Pin_8 | GPIO_Pin_9 | GPIO_Pin_10 | GPIO_Pin_11 | GPIO_Pin_12;
    //复用功能
    GPIO_InitStructure.GPIO_Mode = GPIO_Mode_AF;
    //100M
    GPIO_InitStructure.GPIO_Speed = GPIO_Speed_50MHz;
    GPIO_InitStructure.GPIO_OType = GPIO_OType_PP;
    //上拉
    GPIO_InitStructure.GPIO_PuPd = GPIO_PuPd_UP;
    // PC8,9,10,11,12复用功能输出
    GPIO_Init(GPIOC, &GPIO_InitStructure);

    GPIO_InitStructure.GPIO_Pin = GPIO_Pin_2;
    //PD2复用功能输出
    GPIO_Init(GPIOD, &GPIO_InitStructure);

    //引脚复用映射设置 PC8,AF12
    GPIO_PinAFConfig(GPIOC, GPIO_PinSource8, GPIO_AF_SDIO);
    GPIO_PinAFConfig(GPIOC, GPIO_PinSource9, GPIO_AF_SDIO);
    GPIO_PinAFConfig(GPIOC, GPIO_PinSource10, GPIO_AF_SDIO);
    GPIO_PinAFConfig(GPIOC, GPIO_PinSource11, GPIO_AF_SDIO);
    GPIO_PinAFConfig(GPIOC, GPIO_PinSource12, GPIO_AF_SDIO);
    GPIO_PinAFConfig(GPIOD, GPIO_PinSource2, GPIO_AF_SDIO);
    
    //SDIO结束复位
    RCC_APB2PeriphResetCmd(RCC_APB2Periph_SDIO, DISABLE);

    //SDIO外设寄存器设置为默认值
    SDIO_Register_Deinit();

    NVIC_InitStructure.NVIC_IRQChannel = SDIO_IRQn;
    //抢占优先级
    NVIC_InitStructure.NVIC_IRQChannelPreemptionPriority = SDIO_PREEMPT_PRIORITY;
    //子优先级
    NVIC_InitStructure.NVIC_IRQChannelSubPriority = SDIO_SUB_PRIORITY;
    //IRQ通道使能
    NVIC_InitStructure.NVIC_IRQChannelCmd = ENABLE;
    //根据指定的参数初始化VIC寄存器
    NVIC_Init(&NVIC_InitStructure);

    //SD卡上电
    errorstatus = SD_PowerON();
    //初始化SD卡
    if (errorstatus == SD_OK)
        errorstatus = SD_InitializeCards();
    //获取卡信息
    if (errorstatus == SD_OK)
        errorstatus = SD_GetCardInfo(&SDCardInfo);
    //选中SD卡
    if (errorstatus == SD_OK)
        errorstatus = SD_SelectDeselect((uint32_t)(SDCardInfo.RCA << 16));
    //4位宽度,如果是MMC卡,则不能用4位模式
    if (errorstatus == SD_OK)
        errorstatus = SD_EnableWideBusOperation(SDIO_BusWide_4b);
    if ((errorstatus == SD_OK) || (SDIO_MULTIMEDIA_CARD == CardType))
    {
        if (SDCardInfo.CardType == SDIO_STD_CAPACITY_SD_CARD_V1_1 || SDCardInfo.CardType == SDIO_STD_CAPACITY_SD_CARD_V2_0)
        {
            //V1.1/V2.0卡，设置最高48/4=12Mhz
            clkdiv = SDIO_TRANSFER_CLK_DIV + 2;
        }
        else
            //SDHC等其他卡，设置最高48/2=24Mhz
            clkdiv = SDIO_TRANSFER_CLK_DIV;
        //设置时钟频率,SDIO时钟计算公式:SDIO_CK时钟=SDIOCLK/[clkdiv+2];其中,SDIOCLK固定为48Mhz
        SDIO_ClockSet(clkdiv);
    }
    return errorstatus;
}

//SDIO时钟初始化设置
//clkdiv:时钟分频系数
//CK时钟=SDIOCLK/[clkdiv+2];(SDIOCLK时钟固定为48Mhz)
void SDIO_ClockSet(uint8_t clkdiv)
{
    uint32_t tmpreg = SDIO->CLKCR;
    tmpreg &= 0XFFFFFF00;
    tmpreg |= clkdiv;
    SDIO->CLKCR = tmpreg;
}

//卡上电
//查询所有SDIO接口上的卡设备,并查询其电压和配置时钟
//返回值:错误代码;(0,无错误)
SD_Error SD_PowerON(void)
{
    uint8_t i = 0;
    SD_Error errorstatus = SD_OK;
    uint32_t response = 0, count = 0, validvoltage = 0;
    uint32_t SDType = SD_STD_CAPACITY;

    /*初始化时的时钟不能大于400KHz*/
    /* HCLK = 72MHz, SDIOCLK = 72MHz, SDIO_CK = HCLK/(178 + 2) = 400 KHz */
    SDIO_InitStructure.SDIO_ClockDiv = SDIO_INIT_CLK_DIV;
    SDIO_InitStructure.SDIO_ClockEdge = SDIO_ClockEdge_Rising;
    //不使用bypass模式，直接用HCLK进行分频得到SDIO_CK
    SDIO_InitStructure.SDIO_ClockBypass = SDIO_ClockBypass_Disable;
    // 空闲时不关闭时钟电源
    SDIO_InitStructure.SDIO_ClockPowerSave = SDIO_ClockPowerSave_Disable;
    //1位数据线
    SDIO_InitStructure.SDIO_BusWide = SDIO_BusWide_1b;
    //硬件流
    SDIO_InitStructure.SDIO_HardwareFlowControl = SDIO_HardwareFlowControl_Disable;
    SDIO_Init(&SDIO_InitStructure);

    //上电状态,开启卡时钟
    SDIO_SetPowerState(SDIO_PowerState_ON);
    //SDIOCK使能
    SDIO->CLKCR |= 1 << 8;

    for (i = 0; i < 74; i++)
    {
        //发送CMD0进入IDLE STAGE模式命令.
        SDIO_CmdInitStructure.SDIO_Argument = 0x0;
        //cmd0
        SDIO_CmdInitStructure.SDIO_CmdIndex = SD_CMD_GO_IDLE_STATE;
        //无响应
        SDIO_CmdInitStructure.SDIO_Response = SDIO_Response_No;
        SDIO_CmdInitStructure.SDIO_Wait = SDIO_Wait_No;
        //则CPSM在开始发送命令之前等待数据传输结束。
        SDIO_CmdInitStructure.SDIO_CPSM = SDIO_CPSM_Enable;
        //写命令进命令寄存器
        SDIO_SendCommand(&SDIO_CmdInitStructure);
        errorstatus = CmdError();
        if (errorstatus == SD_OK)
            break;
    }
    //返回错误状态
    if (errorstatus)
        return errorstatus;
    //发送CMD8,短响应,检查SD卡接口特性
    SDIO_CmdInitStructure.SDIO_Argument = SD_CHECK_PATTERN;
    //cmd8
    SDIO_CmdInitStructure.SDIO_CmdIndex = SDIO_SEND_IF_COND;
    //r7
    SDIO_CmdInitStructure.SDIO_Response = SDIO_Response_Short;
    //关闭等待中断
    SDIO_CmdInitStructure.SDIO_Wait = SDIO_Wait_No;
    SDIO_CmdInitStructure.SDIO_CPSM = SDIO_CPSM_Enable;
    SDIO_SendCommand(&SDIO_CmdInitStructure);
    //等待R7响应
    errorstatus = CmdResp7Error();
    //R7响应正常
    if (errorstatus == SD_OK)
    {
        //SD 2.0卡
        CardType = SDIO_STD_CAPACITY_SD_CARD_V2_0;
        //高容量卡
        SDType = SD_HIGH_CAPACITY;
    }
    //发送CMD55,短响应
    SDIO_CmdInitStructure.SDIO_Argument = 0x00;
    SDIO_CmdInitStructure.SDIO_CmdIndex = SD_CMD_APP_CMD;
    SDIO_CmdInitStructure.SDIO_Response = SDIO_Response_Short;
    SDIO_CmdInitStructure.SDIO_Wait = SDIO_Wait_No;
    SDIO_CmdInitStructure.SDIO_CPSM = SDIO_CPSM_Enable;
    //发送CMD55,短响应
    SDIO_SendCommand(&SDIO_CmdInitStructure);
    //等待R1响应
    errorstatus = CmdResp1Error(SD_CMD_APP_CMD);
    //SD2.0/SD 1.1,否则为MMC卡
    if (errorstatus == SD_OK)
    {
        //SD卡,发送ACMD41 SD_APP_OP_COND,参数为:0x80100000
        while ((!validvoltage) && (count < SD_MAX_VOLT_TRIAL))
        {
            //发送CMD55,短响应
            SDIO_CmdInitStructure.SDIO_Argument = 0x00;
            //CMD55
            SDIO_CmdInitStructure.SDIO_CmdIndex = SD_CMD_APP_CMD;
            SDIO_CmdInitStructure.SDIO_Response = SDIO_Response_Short;
            SDIO_CmdInitStructure.SDIO_Wait = SDIO_Wait_No;
            SDIO_CmdInitStructure.SDIO_CPSM = SDIO_CPSM_Enable;
            //发送CMD55,短响应
            SDIO_SendCommand(&SDIO_CmdInitStructure);
            //等待R1响应
            errorstatus = CmdResp1Error(SD_CMD_APP_CMD);
            //响应错误
            if (errorstatus != SD_OK)
                return errorstatus;

            //acmd41，命令参数由支持的电压范围及HCS位组成，HCS位置一来区分卡是SDSc还是sdhc
            //发送ACMD41,短响应
            SDIO_CmdInitStructure.SDIO_Argument = SD_VOLTAGE_WINDOW_SD | SDType;
            SDIO_CmdInitStructure.SDIO_CmdIndex = SD_CMD_SD_APP_OP_COND;
            //r3
            SDIO_CmdInitStructure.SDIO_Response = SDIO_Response_Short;
            SDIO_CmdInitStructure.SDIO_Wait = SDIO_Wait_No;
            SDIO_CmdInitStructure.SDIO_CPSM = SDIO_CPSM_Enable;
            SDIO_SendCommand(&SDIO_CmdInitStructure);
            //等待R3响应
            errorstatus = CmdResp3Error();
            //响应错误
            if (errorstatus != SD_OK)
                return errorstatus;
            //得到响应
            response = SDIO->RESP1;
            //判断SD卡上电是否完成
            validvoltage = (((response >> 31) == 1) ? 1 : 0);
            count++;
        }
        if (count >= SD_MAX_VOLT_TRIAL)
        {
            errorstatus = SD_INVALID_VOLTRANGE;
            return errorstatus;
        }
        if (response &= SD_HIGH_CAPACITY)
        {
            CardType = SDIO_HIGH_CAPACITY_SD_CARD;
        }
    }
    //MMC卡
    else
    {
        //MMC卡,发送CMD1 SDIO_SEND_OP_COND,参数为:0x80FF8000
        while ((!validvoltage) && (count < SD_MAX_VOLT_TRIAL))
        {
            //发送CMD1,短响应
            SDIO_CmdInitStructure.SDIO_Argument = SD_VOLTAGE_WINDOW_MMC;
            SDIO_CmdInitStructure.SDIO_CmdIndex = SD_CMD_SEND_OP_COND;
            //r3
            SDIO_CmdInitStructure.SDIO_Response = SDIO_Response_Short;
            SDIO_CmdInitStructure.SDIO_Wait = SDIO_Wait_No;
            SDIO_CmdInitStructure.SDIO_CPSM = SDIO_CPSM_Enable;
            SDIO_SendCommand(&SDIO_CmdInitStructure);
            //等待R3响应
            errorstatus = CmdResp3Error();
            //响应错误
            if (errorstatus != SD_OK)
                return errorstatus;
            //得到响应
            response = SDIO->RESP1;
            ;
            validvoltage = (((response >> 31) == 1) ? 1 : 0);
            count++;
        }
        if (count >= SD_MAX_VOLT_TRIAL)
        {
            errorstatus = SD_INVALID_VOLTRANGE;
            return errorstatus;
        }
        CardType = SDIO_MULTIMEDIA_CARD;
    }
    return (errorstatus);
}

//SD卡 Power OFF
//返回值:错误代码;(0,无错误)
SD_Error SD_PowerOFF(void)
{
    //SDIO电源关闭,时钟停止
    SDIO_SetPowerState(SDIO_PowerState_OFF);
    return SD_OK;
}

//初始化所有的卡,并让卡进入就绪状态
//返回值:错误代码
SD_Error SD_InitializeCards(void)
{
    SD_Error errorstatus = SD_OK;
    uint16_t rca = 0x01;
    //检查电源状态,确保为上电状态
    if (SDIO_GetPowerState() == SDIO_PowerState_OFF)
    {
        errorstatus = SD_REQUEST_NOT_APPLICABLE;
        return (errorstatus);
    }
    //非SECURE_DIGITAL_IO_CARD
    if (SDIO_SECURE_DIGITAL_IO_CARD != CardType)
    {
        //发送CMD2,取得CID,长响应
        SDIO_CmdInitStructure.SDIO_Argument = 0x0;
        SDIO_CmdInitStructure.SDIO_CmdIndex = SD_CMD_ALL_SEND_CID;
        SDIO_CmdInitStructure.SDIO_Response = SDIO_Response_Long;
        SDIO_CmdInitStructure.SDIO_Wait = SDIO_Wait_No;
        SDIO_CmdInitStructure.SDIO_CPSM = SDIO_CPSM_Enable;
        //发送CMD2,取得CID,长响应
        SDIO_SendCommand(&SDIO_CmdInitStructure);
        //等待R2响应
        errorstatus = CmdResp2Error();
        //响应错误
        if (errorstatus != SD_OK)
            return errorstatus;
        CID_Tab[0] = SDIO->RESP1;
        CID_Tab[1] = SDIO->RESP2;
        CID_Tab[2] = SDIO->RESP3;
        CID_Tab[3] = SDIO->RESP4;
    }
    //判断卡类型
    if ((SDIO_STD_CAPACITY_SD_CARD_V1_1 == CardType) || (SDIO_STD_CAPACITY_SD_CARD_V2_0 == CardType) ||
        (SDIO_SECURE_DIGITAL_IO_COMBO_CARD == CardType) || (SDIO_HIGH_CAPACITY_SD_CARD == CardType))
    {
        //发送CMD3,短响应
        SDIO_CmdInitStructure.SDIO_Argument = 0x00;
        //cmd3
        SDIO_CmdInitStructure.SDIO_CmdIndex = SD_CMD_SET_REL_ADDR;
        //r6
        SDIO_CmdInitStructure.SDIO_Response = SDIO_Response_Short;
        SDIO_CmdInitStructure.SDIO_Wait = SDIO_Wait_No;
        SDIO_CmdInitStructure.SDIO_CPSM = SDIO_CPSM_Enable;
        //发送CMD3,短响应
        SDIO_SendCommand(&SDIO_CmdInitStructure);
        //等待R6响应
        errorstatus = CmdResp6Error(SD_CMD_SET_REL_ADDR, &rca);
        //响应错误
        if (errorstatus != SD_OK)
            return errorstatus;
    }
    if (SDIO_MULTIMEDIA_CARD == CardType)
    {
        //发送CMD3,短响应
        SDIO_CmdInitStructure.SDIO_Argument = (uint32_t)(rca << 16);
        //cmd3
        SDIO_CmdInitStructure.SDIO_CmdIndex = SD_CMD_SET_REL_ADDR;
        //r6
        SDIO_CmdInitStructure.SDIO_Response = SDIO_Response_Short;
        SDIO_CmdInitStructure.SDIO_Wait = SDIO_Wait_No;
        SDIO_CmdInitStructure.SDIO_CPSM = SDIO_CPSM_Enable;
        //发送CMD3,短响应
        SDIO_SendCommand(&SDIO_CmdInitStructure);
        //等待R2响应
        errorstatus = CmdResp2Error();
        //响应错误
        if (errorstatus != SD_OK)
            return errorstatus;
    }
    //非SECURE_DIGITAL_IO_CARD
    if (SDIO_SECURE_DIGITAL_IO_CARD != CardType)
    {
        RCA = rca;
        //发送CMD9+卡RCA,取得CSD,长响应
        SDIO_CmdInitStructure.SDIO_Argument = (uint32_t)(rca << 16);
        SDIO_CmdInitStructure.SDIO_CmdIndex = SD_CMD_SEND_CSD;
        SDIO_CmdInitStructure.SDIO_Response = SDIO_Response_Long;
        SDIO_CmdInitStructure.SDIO_Wait = SDIO_Wait_No;
        SDIO_CmdInitStructure.SDIO_CPSM = SDIO_CPSM_Enable;
        SDIO_SendCommand(&SDIO_CmdInitStructure);
        //等待R2响应
        errorstatus = CmdResp2Error();
        //响应错误
        if (errorstatus != SD_OK)
            return errorstatus;
        CSD_Tab[0] = SDIO->RESP1;
        CSD_Tab[1] = SDIO->RESP2;
        CSD_Tab[2] = SDIO->RESP3;
        CSD_Tab[3] = SDIO->RESP4;
    }
    return SD_OK; //卡初始化成功
}

//得到卡信息
//cardinfo:卡信息存储区
//返回值:错误状态
SD_Error SD_GetCardInfo(SD_CardInfo *cardinfo)
{
    SD_Error errorstatus = SD_OK;
    uint8_t tmp = 0;
    //卡类型
    cardinfo->CardType = (uint8_t)CardType;
    //卡RCA值
    cardinfo->RCA = (uint16_t)RCA;
    tmp = (uint8_t)((CSD_Tab[0] & 0xFF000000) >> 24);
    //CSD结构
    cardinfo->SD_csd.CSDStruct = (tmp & 0xC0) >> 6;
    //2.0协议还没定义这部分(为保留),应该是后续协议定义的
    cardinfo->SD_csd.SysSpecVersion = (tmp & 0x3C) >> 2;
    cardinfo->SD_csd.Reserved1 = tmp & 0x03;          //2个保留位
    tmp = (uint8_t)((CSD_Tab[0] & 0x00FF0000) >> 16); //第1个字节
    //数据读时间1
    cardinfo->SD_csd.TAAC = tmp;
    tmp = (uint8_t)((CSD_Tab[0] & 0x0000FF00) >> 8); //第2个字节
    //数据读时间2
    cardinfo->SD_csd.NSAC = tmp;
    tmp = (uint8_t)(CSD_Tab[0] & 0x000000FF); //第3个字节
    //传输速度
    cardinfo->SD_csd.MaxBusClkFrec = tmp;
    tmp = (uint8_t)((CSD_Tab[1] & 0xFF000000) >> 24); //第4个字节
    //卡指令类高四位
    cardinfo->SD_csd.CardComdClasses = tmp << 4;
    tmp = (uint8_t)((CSD_Tab[1] & 0x00FF0000) >> 16); //第5个字节
    //卡指令类低四位
    cardinfo->SD_csd.CardComdClasses |= (tmp & 0xF0) >> 4;
    //最大读取数据长度
    cardinfo->SD_csd.RdBlockLen = tmp & 0x0F;
    tmp = (uint8_t)((CSD_Tab[1] & 0x0000FF00) >> 8); //第6个字节
    //允许分块读
    cardinfo->SD_csd.PartBlockRead = (tmp & 0x80) >> 7;
    //写块错位
    cardinfo->SD_csd.WrBlockMisalign = (tmp & 0x40) >> 6;
    //读块错位
    cardinfo->SD_csd.RdBlockMisalign = (tmp & 0x20) >> 5;
    cardinfo->SD_csd.DSRImpl = (tmp & 0x10) >> 4;
    cardinfo->SD_csd.Reserved2 = 0; //保留
    //标准1.1/2.0卡/MMC卡
    if ((CardType == SDIO_STD_CAPACITY_SD_CARD_V1_1) || (CardType == SDIO_STD_CAPACITY_SD_CARD_V2_0) || (SDIO_MULTIMEDIA_CARD == CardType))
    {
        //C_SIZE(12位)
        cardinfo->SD_csd.DeviceSize = (tmp & 0x03) << 10;
        tmp = (uint8_t)(CSD_Tab[1] & 0x000000FF); //第7个字节
        cardinfo->SD_csd.DeviceSize |= (tmp) << 2;
        tmp = (uint8_t)((CSD_Tab[2] & 0xFF000000) >> 24); //第8个字节
        cardinfo->SD_csd.DeviceSize |= (tmp & 0xC0) >> 6;
        cardinfo->SD_csd.MaxRdCurrentVDDMin = (tmp & 0x38) >> 3;
        cardinfo->SD_csd.MaxRdCurrentVDDMax = (tmp & 0x07);
        tmp = (uint8_t)((CSD_Tab[2] & 0x00FF0000) >> 16); //第9个字节
        cardinfo->SD_csd.MaxWrCurrentVDDMin = (tmp & 0xE0) >> 5;
        cardinfo->SD_csd.MaxWrCurrentVDDMax = (tmp & 0x1C) >> 2;
        //C_SIZE_MULT
        cardinfo->SD_csd.DeviceSizeMul = (tmp & 0x03) << 1;
        tmp = (uint8_t)((CSD_Tab[2] & 0x0000FF00) >> 8); //第10个字节
        cardinfo->SD_csd.DeviceSizeMul |= (tmp & 0x80) >> 7;
        //计算卡容量
        cardinfo->CardCapacity = (cardinfo->SD_csd.DeviceSize + 1);
        cardinfo->CardCapacity *= (1 << (cardinfo->SD_csd.DeviceSizeMul + 2));
        //块大小
        cardinfo->CardBlockSize = 1 << (cardinfo->SD_csd.RdBlockLen);
        cardinfo->CardCapacity *= cardinfo->CardBlockSize;
    }
    //高容量卡
    else if (CardType == SDIO_HIGH_CAPACITY_SD_CARD)
    {
        tmp = (uint8_t)(CSD_Tab[1] & 0x000000FF); //第7个字节
        //C_SIZE
        cardinfo->SD_csd.DeviceSize = (tmp & 0x3F) << 16;
        tmp = (uint8_t)((CSD_Tab[2] & 0xFF000000) >> 24); //第8个字节
        cardinfo->SD_csd.DeviceSize |= (tmp << 8);
        tmp = (uint8_t)((CSD_Tab[2] & 0x00FF0000) >> 16); //第9个字节
        cardinfo->SD_csd.DeviceSize |= (tmp);
        tmp = (uint8_t)((CSD_Tab[2] & 0x0000FF00) >> 8); //第10个字节
        double tempDouble = (double)(cardinfo->SD_csd.DeviceSize + 1);
        tempDouble = (tempDouble * 1024.0 * 512.0);
        //计算卡容量
        cardinfo->CardCapacity = tempDouble;
        //块大小固定为512字节
        cardinfo->CardBlockSize = 512;
    }
    cardinfo->SD_csd.EraseGrSize = (tmp & 0x40) >> 6;
    cardinfo->SD_csd.EraseGrMul = (tmp & 0x3F) << 1;
    tmp = (uint8_t)(CSD_Tab[2] & 0x000000FF); //第11个字节
    cardinfo->SD_csd.EraseGrMul |= (tmp & 0x80) >> 7;
    cardinfo->SD_csd.WrProtectGrSize = (tmp & 0x7F);
    tmp = (uint8_t)((CSD_Tab[3] & 0xFF000000) >> 24); //第12个字节
    cardinfo->SD_csd.WrProtectGrEnable = (tmp & 0x80) >> 7;
    cardinfo->SD_csd.ManDeflECC = (tmp & 0x60) >> 5;
    cardinfo->SD_csd.WrSpeedFact = (tmp & 0x1C) >> 2;
    cardinfo->SD_csd.MaxWrBlockLen = (tmp & 0x03) << 2;
    tmp = (uint8_t)((CSD_Tab[3] & 0x00FF0000) >> 16); //第13个字节
    cardinfo->SD_csd.MaxWrBlockLen |= (tmp & 0xC0) >> 6;
    cardinfo->SD_csd.WriteBlockPaPartial = (tmp & 0x20) >> 5;
    cardinfo->SD_csd.Reserved3 = 0;
    cardinfo->SD_csd.ContentProtectAppli = (tmp & 0x01);
    tmp = (uint8_t)((CSD_Tab[3] & 0x0000FF00) >> 8); //第14个字节
    cardinfo->SD_csd.FileFormatGrouop = (tmp & 0x80) >> 7;
    cardinfo->SD_csd.CopyFlag = (tmp & 0x40) >> 6;
    cardinfo->SD_csd.PermWrProtect = (tmp & 0x20) >> 5;
    cardinfo->SD_csd.TempWrProtect = (tmp & 0x10) >> 4;
    cardinfo->SD_csd.FileFormat = (tmp & 0x0C) >> 2;
    cardinfo->SD_csd.ECC = (tmp & 0x03);
    tmp = (uint8_t)(CSD_Tab[3] & 0x000000FF); //第15个字节
    cardinfo->SD_csd.CSD_CRC = (tmp & 0xFE) >> 1;
    cardinfo->SD_csd.Reserved4 = 1;
    tmp = (uint8_t)((CID_Tab[0] & 0xFF000000) >> 24); //第0个字节
    cardinfo->SD_cid.ManufacturerID = tmp;
    tmp = (uint8_t)((CID_Tab[0] & 0x00FF0000) >> 16); //第1个字节
    cardinfo->SD_cid.OEM_AppliID = tmp << 8;
    tmp = (uint8_t)((CID_Tab[0] & 0x000000FF00) >> 8); //第2个字节
    cardinfo->SD_cid.OEM_AppliID |= tmp;
    tmp = (uint8_t)(CID_Tab[0] & 0x000000FF); //第3个字节
    cardinfo->SD_cid.ProdName1 = tmp << 24;
    tmp = (uint8_t)((CID_Tab[1] & 0xFF000000) >> 24); //第4个字节
    cardinfo->SD_cid.ProdName1 |= tmp << 16;
    tmp = (uint8_t)((CID_Tab[1] & 0x00FF0000) >> 16); //第5个字节
    cardinfo->SD_cid.ProdName1 |= tmp << 8;
    tmp = (uint8_t)((CID_Tab[1] & 0x0000FF00) >> 8); //第6个字节
    cardinfo->SD_cid.ProdName1 |= tmp;
    tmp = (uint8_t)(CID_Tab[1] & 0x000000FF); //第7个字节
    cardinfo->SD_cid.ProdName2 = tmp;
    tmp = (uint8_t)((CID_Tab[2] & 0xFF000000) >> 24); //第8个字节
    cardinfo->SD_cid.ProdRev = tmp;
    tmp = (uint8_t)((CID_Tab[2] & 0x00FF0000) >> 16); //第9个字节
    cardinfo->SD_cid.ProdSN = tmp << 24;
    tmp = (uint8_t)((CID_Tab[2] & 0x0000FF00) >> 8); //第10个字节
    cardinfo->SD_cid.ProdSN |= tmp << 16;
    tmp = (uint8_t)(CID_Tab[2] & 0x000000FF); //第11个字节
    cardinfo->SD_cid.ProdSN |= tmp << 8;
    tmp = (uint8_t)((CID_Tab[3] & 0xFF000000) >> 24); //第12个字节
    cardinfo->SD_cid.ProdSN |= tmp;
    tmp = (uint8_t)((CID_Tab[3] & 0x00FF0000) >> 16); //第13个字节
    cardinfo->SD_cid.Reserved1 |= (tmp & 0xF0) >> 4;
    cardinfo->SD_cid.ManufactDate = (tmp & 0x0F) << 8;
    tmp = (uint8_t)((CID_Tab[3] & 0x0000FF00) >> 8); //第14个字节
    cardinfo->SD_cid.ManufactDate |= tmp;
    tmp = (uint8_t)(CID_Tab[3] & 0x000000FF); //第15个字节
    cardinfo->SD_cid.CID_CRC = (tmp & 0xFE) >> 1;
    cardinfo->SD_cid.Reserved2 = 1;
    return errorstatus;
}

//设置SDIO总线宽度(MMC卡不支持4bit模式)
//wmode:位宽模式.0,1位数据宽度;1,4位数据宽度;2,8位数据宽度
//SDIO_BusWide_8b: 8-bit data transfer (Only for MMC)
//SDIO_BusWide_4b: 4-bit data transfer
//SDIO_BusWide_1b: 1-bit data transfer (默认)
//返回值:SD卡错误状态
SD_Error SD_EnableWideBusOperation(uint32_t WideMode)
{
    SD_Error errorstatus = SD_OK;
    if (SDIO_MULTIMEDIA_CARD == CardType)
    {
        errorstatus = SD_UNSUPPORTED_FEATURE;
        return (errorstatus);
    }
    //2.0 sd不支持8bits
    else if ((SDIO_STD_CAPACITY_SD_CARD_V1_1 == CardType) || (SDIO_STD_CAPACITY_SD_CARD_V2_0 == CardType) ||
             (SDIO_HIGH_CAPACITY_SD_CARD == CardType))
    {
        if (SDIO_BusWide_8b == WideMode)
        {
            errorstatus = SD_UNSUPPORTED_FEATURE;
            return (errorstatus);
        }
        else
        {
            errorstatus = SDEnWideBus(WideMode);
            if (SD_OK == errorstatus)
            {
                //清除之前的位宽设置
                SDIO->CLKCR &= ~(3 << 11);
                //1位/4位总线宽度
                SDIO->CLKCR |= WideMode;
                //不开启硬件流控制
                SDIO->CLKCR |= 0 << 14;
            }
        }
    }
    return errorstatus;
}

//选卡
//发送CMD7,选择相对地址(rca)为addr的卡,取消其他卡.如果为0,则都不选择.
//addr:卡的RCA地址
SD_Error SD_SelectDeselect(uint32_t addr)
{
    //发送CMD7,选择卡,短响应
    SDIO_CmdInitStructure.SDIO_Argument = addr;
    SDIO_CmdInitStructure.SDIO_CmdIndex = SD_CMD_SEL_DESEL_CARD;
    SDIO_CmdInitStructure.SDIO_Response = SDIO_Response_Short;
    SDIO_CmdInitStructure.SDIO_Wait = SDIO_Wait_No;
    SDIO_CmdInitStructure.SDIO_CPSM = SDIO_CPSM_Enable;
    //发送CMD7,选择卡,短响应
    SDIO_SendCommand(&SDIO_CmdInitStructure);
    return CmdResp1Error(SD_CMD_SEL_DESEL_CARD);
}

//SD卡读取一个块
//buf:读数据缓存区(必须4字节对齐!!)
//addr:读取地址
//blksize:块大小
SD_Error SD_ReadBlock(uint8_t *buf, long long addr, uint16_t blksize)
{
    SD_Error errorstatus = SD_OK;
    uint8_t power;
    //转换为uint32_t指针
    uint32_t count = 0, *tempbuff = (uint32_t *)buf;
    uint32_t timeout = SDIO_DATATIMEOUT;
    if (NULL == buf)
        return SD_INVALID_PARAMETER;
    //数据控制寄存器清零(关DMA)
    SDIO->DCTRL = 0x0;

    //大容量卡
    if (CardType == SDIO_HIGH_CAPACITY_SD_CARD)
    {
        blksize = 512;
        addr >>= 9;
    }
    //清除DPSM状态机配置
    SDIO_DataInitStructure.SDIO_DataBlockSize = SDIO_DataBlockSize_1b;
    SDIO_DataInitStructure.SDIO_DataLength = 0;
    SDIO_DataInitStructure.SDIO_DataTimeOut = SD_DATATIMEOUT;
    SDIO_DataInitStructure.SDIO_DPSM = SDIO_DPSM_Enable;
    SDIO_DataInitStructure.SDIO_TransferDir = SDIO_TransferDir_ToCard;
    SDIO_DataInitStructure.SDIO_TransferMode = SDIO_TransferMode_Block;
    SDIO_DataConfig(&SDIO_DataInitStructure);

    //卡锁了
    if (SDIO->RESP1 & SD_CARD_LOCKED)
        return SD_LOCK_UNLOCK_FAILED;
    if ((blksize > 0) && (blksize <= 2048) && ((blksize & (blksize - 1)) == 0))
    {
        power = convert_from_bytes_to_power_of_two(blksize);
        SDIO_CmdInitStructure.SDIO_Argument = blksize;
        SDIO_CmdInitStructure.SDIO_CmdIndex = SD_CMD_SET_BLOCKLEN;
        SDIO_CmdInitStructure.SDIO_Response = SDIO_Response_Short;
        SDIO_CmdInitStructure.SDIO_Wait = SDIO_Wait_No;
        SDIO_CmdInitStructure.SDIO_CPSM = SDIO_CPSM_Enable;
        //发送CMD16+设置数据长度为blksize,短响应
        SDIO_SendCommand(&SDIO_CmdInitStructure);
        //等待R1响应
        errorstatus = CmdResp1Error(SD_CMD_SET_BLOCKLEN);
        //响应错误
        if (errorstatus != SD_OK)
            return errorstatus;
    }
    else
        return SD_INVALID_PARAMETER;
    //清除DPSM状态机配置
    SDIO_DataInitStructure.SDIO_DataBlockSize = power << 4;
    SDIO_DataInitStructure.SDIO_DataLength = blksize;
    SDIO_DataInitStructure.SDIO_DataTimeOut = SD_DATATIMEOUT;
    SDIO_DataInitStructure.SDIO_DPSM = SDIO_DPSM_Enable;
    SDIO_DataInitStructure.SDIO_TransferDir = SDIO_TransferDir_ToSDIO;
    SDIO_DataInitStructure.SDIO_TransferMode = SDIO_TransferMode_Block;
    SDIO_DataConfig(&SDIO_DataInitStructure);

    SDIO_CmdInitStructure.SDIO_Argument = addr;
    SDIO_CmdInitStructure.SDIO_CmdIndex = SD_CMD_READ_SINGLE_BLOCK;
    SDIO_CmdInitStructure.SDIO_Response = SDIO_Response_Short;
    SDIO_CmdInitStructure.SDIO_Wait = SDIO_Wait_No;
    SDIO_CmdInitStructure.SDIO_CPSM = SDIO_CPSM_Enable;
    //发送CMD17+从addr地址出读取数据,短响应
    SDIO_SendCommand(&SDIO_CmdInitStructure);
    //等待R1响应
    errorstatus = CmdResp1Error(SD_CMD_READ_SINGLE_BLOCK);
    //响应错误
    if (errorstatus != SD_OK)
        return errorstatus;
    //查询模式,轮询数据

    //关闭总中断(POLLING模式,严禁中断打断SDIO读写操作!!!)
    INTX_DISABLE();
    //无上溢/CRC/超时/完成(标志)/起始位错误
    while (!(SDIO->STA & ((1 << 5) | (1 << 1) | (1 << 3) | (1 << 10) | (1 << 9))))
    {
        //接收区半满,表示至少存了8个字
        if (SDIO_GetFlagStatus(SDIO_FLAG_RXFIFOHF) != RESET)
        {
            //循环读取数据
            for (count = 0; count < 8; count++)
            {
                *(tempbuff + count) = SDIO->FIFO;
            }
            tempbuff += 8;
            //读数据溢出时间
            timeout = 0X7FFFFF;
        }
        else //处理超时
        {
            if (timeout == 0)
                return SD_DATA_TIMEOUT;
            timeout--;
        }
    }
    //数据超时错误
    if (SDIO_GetFlagStatus(SDIO_FLAG_DTIMEOUT) != RESET)
    {
        //清错误标志
        SDIO_ClearFlag(SDIO_FLAG_DTIMEOUT);
        return SD_DATA_TIMEOUT;
    }
    //数据块CRC错误
    else if (SDIO_GetFlagStatus(SDIO_FLAG_DCRCFAIL) != RESET)
    {
        //清错误标志
        SDIO_ClearFlag(SDIO_FLAG_DCRCFAIL);
        return SD_DATA_CRC_FAIL;
    }
    //接收fifo上溢错误
    else if (SDIO_GetFlagStatus(SDIO_FLAG_RXOVERR) != RESET)
    {
        //清错误标志
        SDIO_ClearFlag(SDIO_FLAG_RXOVERR);
        return SD_RX_OVERRUN;
    }
    //接收起始位错误
    else if (SDIO_GetFlagStatus(SDIO_FLAG_STBITERR) != RESET)
    {
        //清错误标志
        SDIO_ClearFlag(SDIO_FLAG_STBITERR);
        return SD_START_BIT_ERR;
    }
    //FIFO里面,还存在可用数据
    while (SDIO_GetFlagStatus(SDIO_FLAG_RXDAVL) != RESET)
    {
        //循环读取数据
        *tempbuff = SDIO->FIFO;
        tempbuff++;
    }
    //开启总中断
    INTX_ENABLE();
    //清除所有标记
    SDIO_ClearFlag(SDIO_STATIC_FLAGS);
    return errorstatus;
}

//SD卡读取多个块
//buf:读数据缓存区
//addr:读取地址
//blksize:块大小
//nblks:要读取的块数
//返回值:错误状态
#pragma pack(4)
uint32_t *tempbuff;
#pragma pack()

SD_Error SD_ReadMultiBlocks(uint8_t *buf, long long addr, uint16_t blksize, uint32_t nblks)
{
    SD_Error errorstatus = SD_OK;
    uint8_t power;
    uint32_t count = 0;
    uint32_t timeout = SDIO_DATATIMEOUT;
    //转换为uint32_t指针
    tempbuff = (uint32_t *)buf;

    //数据控制寄存器清零(关DMA)
    SDIO->DCTRL = 0x0;
    //大容量卡
    if (CardType == SDIO_HIGH_CAPACITY_SD_CARD)
    {
        blksize = 512;
        addr >>= 9;
    }
    //清除DPSM状态机配置
    SDIO_DataInitStructure.SDIO_DataBlockSize = 0;
    SDIO_DataInitStructure.SDIO_DataLength = 0;
    SDIO_DataInitStructure.SDIO_DataTimeOut = SD_DATATIMEOUT;
    SDIO_DataInitStructure.SDIO_DPSM = SDIO_DPSM_Enable;
    SDIO_DataInitStructure.SDIO_TransferDir = SDIO_TransferDir_ToCard;
    SDIO_DataInitStructure.SDIO_TransferMode = SDIO_TransferMode_Block;
    SDIO_DataConfig(&SDIO_DataInitStructure);
    //卡锁了
    if (SDIO->RESP1 & SD_CARD_LOCKED)
        return SD_LOCK_UNLOCK_FAILED;
    if ((blksize > 0) && (blksize <= 2048) && ((blksize & (blksize - 1)) == 0))
    {
        power = convert_from_bytes_to_power_of_two(blksize);
        //发送CMD16+设置数据长度为blksize,短响应
        SDIO_CmdInitStructure.SDIO_Argument = blksize;
        SDIO_CmdInitStructure.SDIO_CmdIndex = SD_CMD_SET_BLOCKLEN;
        SDIO_CmdInitStructure.SDIO_Response = SDIO_Response_Short;
        SDIO_CmdInitStructure.SDIO_Wait = SDIO_Wait_No;
        SDIO_CmdInitStructure.SDIO_CPSM = SDIO_CPSM_Enable;
        SDIO_SendCommand(&SDIO_CmdInitStructure);
        //等待R1响应
        errorstatus = CmdResp1Error(SD_CMD_SET_BLOCKLEN);
        //响应错误
        if (errorstatus != SD_OK)
            return errorstatus;
    }
    else
        return SD_INVALID_PARAMETER;
    //多块读
    if (nblks > 1)
    {
        //判断是否超过最大接收长度
        if (nblks * blksize > SD_MAX_DATA_LENGTH)
            return SD_INVALID_PARAMETER;
        //nblks*blksize,512块大小,卡到控制器
        SDIO_DataInitStructure.SDIO_DataBlockSize = power << 4;
        ;
        SDIO_DataInitStructure.SDIO_DataLength = nblks * blksize;
        SDIO_DataInitStructure.SDIO_DataTimeOut = SD_DATATIMEOUT;
        SDIO_DataInitStructure.SDIO_DPSM = SDIO_DPSM_Enable;
        SDIO_DataInitStructure.SDIO_TransferDir = SDIO_TransferDir_ToSDIO;
        SDIO_DataInitStructure.SDIO_TransferMode = SDIO_TransferMode_Block;
        SDIO_DataConfig(&SDIO_DataInitStructure);
        //发送CMD18+从addr地址出读取数据,短响应
        SDIO_CmdInitStructure.SDIO_Argument = addr;
        SDIO_CmdInitStructure.SDIO_CmdIndex = SD_CMD_READ_MULT_BLOCK;
        SDIO_CmdInitStructure.SDIO_Response = SDIO_Response_Short;
        SDIO_CmdInitStructure.SDIO_Wait = SDIO_Wait_No;
        SDIO_CmdInitStructure.SDIO_CPSM = SDIO_CPSM_Enable;
        SDIO_SendCommand(&SDIO_CmdInitStructure);
        //等待R1响应
        errorstatus = CmdResp1Error(SD_CMD_READ_MULT_BLOCK);
        //响应错误
        if (errorstatus != SD_OK)
            return errorstatus;

        //关闭总中断(POLLING模式,严禁中断打断SDIO读写操作!!!)
        INTX_DISABLE();
        //无上溢/CRC/超时/完成(标志)/起始位错误
        while (!(SDIO->STA & ((1 << 5) | (1 << 1) | (1 << 3) | (1 << 8) | (1 << 9))))
        {
            //接收区半满,表示至少存了8个字
            if (SDIO_GetFlagStatus(SDIO_FLAG_RXFIFOHF) != RESET)
            {
                //循环读取数据
                for (count = 0; count < 8; count++)
                {
                    *(tempbuff + count) = SDIO->FIFO;
                }
                tempbuff += 8;
                //读数据溢出时间
                timeout = 0X7FFFFF;
            }
            //处理超时
            else
            {
                if (timeout == 0)
                    return SD_DATA_TIMEOUT;
                timeout--;
            }
        }
        //数据超时错误
        if (SDIO_GetFlagStatus(SDIO_FLAG_DTIMEOUT) != RESET)
        {
            //清错误标志
            SDIO_ClearFlag(SDIO_FLAG_DTIMEOUT);
            return SD_DATA_TIMEOUT;
        }
        //数据块CRC错误
        else if (SDIO_GetFlagStatus(SDIO_FLAG_DCRCFAIL) != RESET)
        {
            //清错误标志
            SDIO_ClearFlag(SDIO_FLAG_DCRCFAIL);
            return SD_DATA_CRC_FAIL;
        }
        //接收fifo上溢错误
        else if (SDIO_GetFlagStatus(SDIO_FLAG_RXOVERR) != RESET)
        {
            //清错误标志
            SDIO_ClearFlag(SDIO_FLAG_RXOVERR);
            return SD_RX_OVERRUN;
        }
        //接收起始位错误
        else if (SDIO_GetFlagStatus(SDIO_FLAG_STBITERR) != RESET)
        {
            //清错误标志
            SDIO_ClearFlag(SDIO_FLAG_STBITERR);
            return SD_START_BIT_ERR;
        }
        //FIFO里面,还存在可用数据
        while (SDIO_GetFlagStatus(SDIO_FLAG_RXDAVL) != RESET)
        {
            //循环读取数据
            *tempbuff = SDIO->FIFO;
            tempbuff++;
        }
        //接收结束
        if (SDIO_GetFlagStatus(SDIO_FLAG_DATAEND) != RESET)
        {
            if ((SDIO_STD_CAPACITY_SD_CARD_V1_1 == CardType) || (SDIO_STD_CAPACITY_SD_CARD_V2_0 == CardType) || (SDIO_HIGH_CAPACITY_SD_CARD == CardType))
            {
                //发送CMD12+结束传输
                SDIO_CmdInitStructure.SDIO_Argument = 0;
                SDIO_CmdInitStructure.SDIO_CmdIndex = SD_CMD_STOP_TRANSMISSION;
                SDIO_CmdInitStructure.SDIO_Response = SDIO_Response_Short;
                SDIO_CmdInitStructure.SDIO_Wait = SDIO_Wait_No;
                SDIO_CmdInitStructure.SDIO_CPSM = SDIO_CPSM_Enable;
                SDIO_SendCommand(&SDIO_CmdInitStructure);
                //等待R1响应
                errorstatus = CmdResp1Error(SD_CMD_STOP_TRANSMISSION);
                if (errorstatus != SD_OK)
                    return errorstatus;
            }
        }
        //开启总中断
        INTX_ENABLE();
        //清除所有标记
        SDIO_ClearFlag(SDIO_STATIC_FLAGS);
    }
    return errorstatus;
}

//SD卡写1个块
//buf:数据缓存区
//addr:写地址
//blksize:块大小
//返回值:错误状态
SD_Error SD_WriteBlock(uint8_t *buf, long long addr, uint16_t blksize)
{
    SD_Error errorstatus = SD_OK;
    uint8_t power = 0, cardstate = 0;
    uint32_t timeout = 0, bytestransferred = 0;
    uint32_t cardstatus = 0, count = 0, restwords = 0;
    //总长度(字节)
    uint32_t tlen = blksize;
    uint32_t *tempbuff = (uint32_t *)buf;
    //参数错误
    if (buf == NULL)
        return SD_INVALID_PARAMETER;
    //数据控制寄存器清零(关DMA)
    SDIO->DCTRL = 0x0;
    //清除DPSM状态机配置
    SDIO_DataInitStructure.SDIO_DataBlockSize = 0;
    SDIO_DataInitStructure.SDIO_DataLength = 0;
    SDIO_DataInitStructure.SDIO_DataTimeOut = SD_DATATIMEOUT;
    SDIO_DataInitStructure.SDIO_DPSM = SDIO_DPSM_Enable;
    SDIO_DataInitStructure.SDIO_TransferDir = SDIO_TransferDir_ToCard;
    SDIO_DataInitStructure.SDIO_TransferMode = SDIO_TransferMode_Block;
    SDIO_DataConfig(&SDIO_DataInitStructure);
    //卡锁了
    if (SDIO->RESP1 & SD_CARD_LOCKED)
        return SD_LOCK_UNLOCK_FAILED;
    //大容量卡
    if (CardType == SDIO_HIGH_CAPACITY_SD_CARD)
    {
        blksize = 512;
        addr >>= 9;
    }
    if ((blksize > 0) && (blksize <= 2048) && ((blksize & (blksize - 1)) == 0))
    {
        power = convert_from_bytes_to_power_of_two(blksize);
        //发送CMD16+设置数据长度为blksize,短响应
        SDIO_CmdInitStructure.SDIO_Argument = blksize;
        SDIO_CmdInitStructure.SDIO_CmdIndex = SD_CMD_SET_BLOCKLEN;
        SDIO_CmdInitStructure.SDIO_Response = SDIO_Response_Short;
        SDIO_CmdInitStructure.SDIO_Wait = SDIO_Wait_No;
        SDIO_CmdInitStructure.SDIO_CPSM = SDIO_CPSM_Enable;
        SDIO_SendCommand(&SDIO_CmdInitStructure);
        //等待R1响应
        errorstatus = CmdResp1Error(SD_CMD_SET_BLOCKLEN);
        //响应错误
        if (errorstatus != SD_OK)
            return errorstatus;
    }
    else
        return SD_INVALID_PARAMETER;
    //发送CMD13,查询卡的状态,短响应
    SDIO_CmdInitStructure.SDIO_Argument = (uint32_t)RCA << 16;
    SDIO_CmdInitStructure.SDIO_CmdIndex = SD_CMD_SEND_STATUS;
    SDIO_CmdInitStructure.SDIO_Response = SDIO_Response_Short;
    SDIO_CmdInitStructure.SDIO_Wait = SDIO_Wait_No;
    SDIO_CmdInitStructure.SDIO_CPSM = SDIO_CPSM_Enable;
    SDIO_SendCommand(&SDIO_CmdInitStructure);
    //等待R1响应
    errorstatus = CmdResp1Error(SD_CMD_SEND_STATUS);
    if (errorstatus != SD_OK)
        return errorstatus;
    cardstatus = SDIO->RESP1;
    timeout = SD_DATATIMEOUT;
    //检查READY_FOR_DATA位是否置位
    while (((cardstatus & 0x00000100) == 0) && (timeout > 0))
    {
        timeout--;
        //发送CMD13,查询卡的状态,短响应
        SDIO_CmdInitStructure.SDIO_Argument = (uint32_t)RCA << 16;
        SDIO_CmdInitStructure.SDIO_CmdIndex = SD_CMD_SEND_STATUS;
        SDIO_CmdInitStructure.SDIO_Response = SDIO_Response_Short;
        SDIO_CmdInitStructure.SDIO_Wait = SDIO_Wait_No;
        SDIO_CmdInitStructure.SDIO_CPSM = SDIO_CPSM_Enable;
        SDIO_SendCommand(&SDIO_CmdInitStructure);
        //等待R1响应
        errorstatus = CmdResp1Error(SD_CMD_SEND_STATUS);
        if (errorstatus != SD_OK)
            return errorstatus;
        cardstatus = SDIO->RESP1;
    }
    if (timeout == 0)
        return SD_ERROR;
    //发送CMD24,写单块指令,短响应
    SDIO_CmdInitStructure.SDIO_Argument = addr;
    SDIO_CmdInitStructure.SDIO_CmdIndex = SD_CMD_WRITE_SINGLE_BLOCK;
    SDIO_CmdInitStructure.SDIO_Response = SDIO_Response_Short;
    SDIO_CmdInitStructure.SDIO_Wait = SDIO_Wait_No;
    SDIO_CmdInitStructure.SDIO_CPSM = SDIO_CPSM_Enable;
    SDIO_SendCommand(&SDIO_CmdInitStructure);
    //等待R1响应
    errorstatus = CmdResp1Error(SD_CMD_WRITE_SINGLE_BLOCK);
    if (errorstatus != SD_OK)
        return errorstatus;
    //单块写,不需要发送停止传输指令
    StopCondition = 0;
    //blksize, 控制器到卡
    SDIO_DataInitStructure.SDIO_DataBlockSize = power << 4;
    ;
    SDIO_DataInitStructure.SDIO_DataLength = blksize;
    SDIO_DataInitStructure.SDIO_DataTimeOut = SD_DATATIMEOUT;
    SDIO_DataInitStructure.SDIO_DPSM = SDIO_DPSM_Enable;
    SDIO_DataInitStructure.SDIO_TransferDir = SDIO_TransferDir_ToCard;
    SDIO_DataInitStructure.SDIO_TransferMode = SDIO_TransferMode_Block;
    SDIO_DataConfig(&SDIO_DataInitStructure);
    timeout = SDIO_DATATIMEOUT;

    //关闭总中断(POLLING模式,严禁中断打断SDIO读写操作!!!)
    INTX_DISABLE();
    //数据块发送成功/下溢/CRC/超时/起始位错误
    while (!(SDIO->STA & ((1 << 10) | (1 << 4) | (1 << 1) | (1 << 3) | (1 << 9))))
    {
        //发送区半空,表示至少存了8个字
        if (SDIO_GetFlagStatus(SDIO_FLAG_TXFIFOHE) != RESET)
        {
            //不够32字节了
            if ((tlen - bytestransferred) < SD_HALFFIFOBYTES)
            {
                restwords = ((tlen - bytestransferred) % 4 == 0) ? ((tlen - bytestransferred) / 4) : ((tlen - bytestransferred) / 4 + 1);
                for (count = 0; count < restwords; count++, tempbuff++, bytestransferred += 4)
                {
                    SDIO->FIFO = *tempbuff;
                }
            }
            else
            {
                for (count = 0; count < 8; count++)
                {
                    SDIO->FIFO = *(tempbuff + count);
                }
                tempbuff += 8;
                bytestransferred += 32;
            }
            //写数据溢出时间
            timeout = 0X3FFFFFFF;
        }
        else
        {
            if (timeout == 0)
                return SD_DATA_TIMEOUT;
            timeout--;
        }
    }
    //数据超时错误
    if (SDIO_GetFlagStatus(SDIO_FLAG_DTIMEOUT) != RESET)
    {
        //清错误标志
        SDIO_ClearFlag(SDIO_FLAG_DTIMEOUT);
        return SD_DATA_TIMEOUT;
    }
    //数据块CRC错误
    else if (SDIO_GetFlagStatus(SDIO_FLAG_DCRCFAIL) != RESET)
    {
        //清错误标志
        SDIO_ClearFlag(SDIO_FLAG_DCRCFAIL);
        return SD_DATA_CRC_FAIL;
    }
    //接收fifo下溢错误
    else if (SDIO_GetFlagStatus(SDIO_FLAG_TXUNDERR) != RESET)
    {
        //清错误标志
        SDIO_ClearFlag(SDIO_FLAG_TXUNDERR);
        return SD_TX_UNDERRUN;
    }
    //接收起始位错误
    else if (SDIO_GetFlagStatus(SDIO_FLAG_STBITERR) != RESET)
    {
        //清错误标志
        SDIO_ClearFlag(SDIO_FLAG_STBITERR);
        return SD_START_BIT_ERR;
    }
    //开启总中断
    INTX_ENABLE();
    //清除所有标记
    SDIO_ClearFlag(SDIO_STATIC_FLAGS);
    //清除所有标记
    SDIO_ClearFlag(SDIO_STATIC_FLAGS);
    errorstatus = IsCardProgramming(&cardstate);
    while ((errorstatus == SD_OK) && ((cardstate == SD_CARD_PROGRAMMING) || (cardstate == SD_CARD_RECEIVING)))
    {
        errorstatus = IsCardProgramming(&cardstate);
    }
    return errorstatus;
}

//SD卡写多个块
//buf:数据缓存区
//addr:写地址
//blksize:块大小
//nblks:要写入的块数
//返回值:错误状态
SD_Error SD_WriteMultiBlocks(uint8_t *buf, long long addr, uint16_t blksize, uint32_t nblks)
{
    SD_Error errorstatus = SD_OK;
    uint8_t power = 0, cardstate = 0;
    uint32_t timeout = 0, bytestransferred = 0;
    uint32_t count = 0, restwords = 0;
    uint32_t tlen = nblks * blksize; //总长度(字节)
    uint32_t *tempbuff = (uint32_t *)buf;
    if (buf == NULL)
        return SD_INVALID_PARAMETER; //参数错误
    SDIO->DCTRL = 0x0;               //数据控制寄存器清零(关DMA)

    SDIO_DataInitStructure.SDIO_DataBlockSize = 0;
    ; //清除DPSM状态机配置
    SDIO_DataInitStructure.SDIO_DataLength = 0;
    SDIO_DataInitStructure.SDIO_DataTimeOut = SD_DATATIMEOUT;
    SDIO_DataInitStructure.SDIO_DPSM = SDIO_DPSM_Enable;
    SDIO_DataInitStructure.SDIO_TransferDir = SDIO_TransferDir_ToCard;
    SDIO_DataInitStructure.SDIO_TransferMode = SDIO_TransferMode_Block;
    SDIO_DataConfig(&SDIO_DataInitStructure);

    if (SDIO->RESP1 & SD_CARD_LOCKED)
        return SD_LOCK_UNLOCK_FAILED;           //卡锁了
    if (CardType == SDIO_HIGH_CAPACITY_SD_CARD) //大容量卡
    {
        blksize = 512;
        addr >>= 9;
    }
    if ((blksize > 0) && (blksize <= 2048) && ((blksize & (blksize - 1)) == 0))
    {
        power = convert_from_bytes_to_power_of_two(blksize);

        SDIO_CmdInitStructure.SDIO_Argument = blksize; //发送CMD16+设置数据长度为blksize,短响应
        SDIO_CmdInitStructure.SDIO_CmdIndex = SD_CMD_SET_BLOCKLEN;
        SDIO_CmdInitStructure.SDIO_Response = SDIO_Response_Short;
        SDIO_CmdInitStructure.SDIO_Wait = SDIO_Wait_No;
        SDIO_CmdInitStructure.SDIO_CPSM = SDIO_CPSM_Enable;
        SDIO_SendCommand(&SDIO_CmdInitStructure);

        errorstatus = CmdResp1Error(SD_CMD_SET_BLOCKLEN); //等待R1响应

        if (errorstatus != SD_OK)
            return errorstatus; //响应错误
    }
    else
        return SD_INVALID_PARAMETER;
    if (nblks > 1)
    {
        if (nblks * blksize > SD_MAX_DATA_LENGTH)
            return SD_INVALID_PARAMETER;
        if ((SDIO_STD_CAPACITY_SD_CARD_V1_1 == CardType) || (SDIO_STD_CAPACITY_SD_CARD_V2_0 == CardType) || (SDIO_HIGH_CAPACITY_SD_CARD == CardType))
        {
            //提高性能
            SDIO_CmdInitStructure.SDIO_Argument = (uint32_t)RCA << 16; //发送ACMD55,短响应
            SDIO_CmdInitStructure.SDIO_CmdIndex = SD_CMD_APP_CMD;
            SDIO_CmdInitStructure.SDIO_Response = SDIO_Response_Short;
            SDIO_CmdInitStructure.SDIO_Wait = SDIO_Wait_No;
            SDIO_CmdInitStructure.SDIO_CPSM = SDIO_CPSM_Enable;
            SDIO_SendCommand(&SDIO_CmdInitStructure);

            errorstatus = CmdResp1Error(SD_CMD_APP_CMD); //等待R1响应

            if (errorstatus != SD_OK)
                return errorstatus;

            SDIO_CmdInitStructure.SDIO_Argument = nblks; //发送CMD23,设置块数量,短响应
            SDIO_CmdInitStructure.SDIO_CmdIndex = SD_CMD_SET_BLOCK_COUNT;
            SDIO_CmdInitStructure.SDIO_Response = SDIO_Response_Short;
            SDIO_CmdInitStructure.SDIO_Wait = SDIO_Wait_No;
            SDIO_CmdInitStructure.SDIO_CPSM = SDIO_CPSM_Enable;
            SDIO_SendCommand(&SDIO_CmdInitStructure);

            errorstatus = CmdResp1Error(SD_CMD_SET_BLOCK_COUNT); //等待R1响应

            if (errorstatus != SD_OK)
                return errorstatus;
        }

        SDIO_CmdInitStructure.SDIO_Argument = addr; //发送CMD25,多块写指令,短响应
        SDIO_CmdInitStructure.SDIO_CmdIndex = SD_CMD_WRITE_MULT_BLOCK;
        SDIO_CmdInitStructure.SDIO_Response = SDIO_Response_Short;
        SDIO_CmdInitStructure.SDIO_Wait = SDIO_Wait_No;
        SDIO_CmdInitStructure.SDIO_CPSM = SDIO_CPSM_Enable;
        SDIO_SendCommand(&SDIO_CmdInitStructure);

        errorstatus = CmdResp1Error(SD_CMD_WRITE_MULT_BLOCK); //等待R1响应

        if (errorstatus != SD_OK)
            return errorstatus;

        SDIO_DataInitStructure.SDIO_DataBlockSize = power << 4;
        ; //blksize, 控制器到卡
        SDIO_DataInitStructure.SDIO_DataLength = nblks * blksize;
        SDIO_DataInitStructure.SDIO_DataTimeOut = SD_DATATIMEOUT;
        SDIO_DataInitStructure.SDIO_DPSM = SDIO_DPSM_Enable;
        SDIO_DataInitStructure.SDIO_TransferDir = SDIO_TransferDir_ToCard;
        SDIO_DataInitStructure.SDIO_TransferMode = SDIO_TransferMode_Block;
        SDIO_DataConfig(&SDIO_DataInitStructure);

        timeout = SDIO_DATATIMEOUT;
        INTX_DISABLE();                                                               //关闭总中断(POLLING模式,严禁中断打断SDIO读写操作!!!)
        while (!(SDIO->STA & ((1 << 4) | (1 << 1) | (1 << 8) | (1 << 3) | (1 << 9)))) //下溢/CRC/数据结束/超时/起始位错误
        {
            if (SDIO_GetFlagStatus(SDIO_FLAG_TXFIFOHE) != RESET) //发送区半空,表示至少存了8字(32字节)
            {
                if ((tlen - bytestransferred) < SD_HALFFIFOBYTES) //不够32字节了
                {
                    restwords = ((tlen - bytestransferred) % 4 == 0) ? ((tlen - bytestransferred) / 4) : ((tlen - bytestransferred) / 4 + 1);
                    for (count = 0; count < restwords; count++, tempbuff++, bytestransferred += 4)
                    {
                        SDIO->FIFO = *tempbuff;
                    }
                }
                else //发送区半空,可以发送至少8字(32字节)数据
                {
                    for (count = 0; count < SD_HALFFIFO; count++)
                    {
                        SDIO->FIFO = *(tempbuff + count);
                    }
                    tempbuff += SD_HALFFIFO;
                    bytestransferred += SD_HALFFIFOBYTES;
                }
                timeout = 0X3FFFFFFF; //写数据溢出时间
            }
            else
            {
                if (timeout == 0)
                    return SD_DATA_TIMEOUT;
                timeout--;
            }
        }
        if (SDIO_GetFlagStatus(SDIO_FLAG_DTIMEOUT) != RESET) //数据超时错误
        {
            SDIO_ClearFlag(SDIO_FLAG_DTIMEOUT); //清错误标志
            return SD_DATA_TIMEOUT;
        }
        else if (SDIO_GetFlagStatus(SDIO_FLAG_DCRCFAIL) != RESET) //数据块CRC错误
        {
            SDIO_ClearFlag(SDIO_FLAG_DCRCFAIL); //清错误标志
            return SD_DATA_CRC_FAIL;
        }
        else if (SDIO_GetFlagStatus(SDIO_FLAG_TXUNDERR) != RESET) //接收fifo下溢错误
        {
            SDIO_ClearFlag(SDIO_FLAG_TXUNDERR); //清错误标志
            return SD_TX_UNDERRUN;
        }
        else if (SDIO_GetFlagStatus(SDIO_FLAG_STBITERR) != RESET) //接收起始位错误
        {
            SDIO_ClearFlag(SDIO_FLAG_STBITERR); //清错误标志
            return SD_START_BIT_ERR;
        }

        if (SDIO_GetFlagStatus(SDIO_FLAG_DATAEND) != RESET) //发送结束
        {
            if ((SDIO_STD_CAPACITY_SD_CARD_V1_1 == CardType) || (SDIO_STD_CAPACITY_SD_CARD_V2_0 == CardType) || (SDIO_HIGH_CAPACITY_SD_CARD == CardType))
            {
                SDIO_CmdInitStructure.SDIO_Argument = 0; //发送CMD12+结束传输
                SDIO_CmdInitStructure.SDIO_CmdIndex = SD_CMD_STOP_TRANSMISSION;
                SDIO_CmdInitStructure.SDIO_Response = SDIO_Response_Short;
                SDIO_CmdInitStructure.SDIO_Wait = SDIO_Wait_No;
                SDIO_CmdInitStructure.SDIO_CPSM = SDIO_CPSM_Enable;
                SDIO_SendCommand(&SDIO_CmdInitStructure);

                errorstatus = CmdResp1Error(SD_CMD_STOP_TRANSMISSION); //等待R1响应
                if (errorstatus != SD_OK)
                    return errorstatus;
            }
        }
        INTX_ENABLE();                     //开启总中断
        SDIO_ClearFlag(SDIO_STATIC_FLAGS); //清除所有标记
    }
    //清除所有标记
    SDIO_ClearFlag(SDIO_STATIC_FLAGS);
    errorstatus = IsCardProgramming(&cardstate);
    while ((errorstatus == SD_OK) && ((cardstate == SD_CARD_PROGRAMMING) || (cardstate == SD_CARD_RECEIVING)))
    {
        errorstatus = IsCardProgramming(&cardstate);
    }
    return errorstatus;
}

//SDIO中断服务函数
void SDIO_IRQHandler(void)
{
    //处理所有SDIO相关中断
    SD_ProcessIRQSrc();
}

//SDIO中断处理函数
//处理SDIO传输过程中的各种中断事务
//返回值:错误代码
SD_Error SD_ProcessIRQSrc(void)
{
    if (SDIO_GetFlagStatus(SDIO_FLAG_DATAEND) != RESET) //接收完成中断
    {
        if (StopCondition == 1)
        {
            //发送CMD12+结束传输
            SDIO_CmdInitStructure.SDIO_Argument = 0;
            SDIO_CmdInitStructure.SDIO_CmdIndex = SD_CMD_STOP_TRANSMISSION;
            SDIO_CmdInitStructure.SDIO_Response = SDIO_Response_Short;
            SDIO_CmdInitStructure.SDIO_Wait = SDIO_Wait_No;
            SDIO_CmdInitStructure.SDIO_CPSM = SDIO_CPSM_Enable;
            SDIO_SendCommand(&SDIO_CmdInitStructure);
            TransferError = CmdResp1Error(SD_CMD_STOP_TRANSMISSION);
        }
        else
            TransferError = SD_OK;
        //清除完成中断标记
        SDIO->ICR |= 1 << 8;
        //关闭相关中断
        SDIO->MASK &= ~((1 << 1) | (1 << 3) | (1 << 8) | (1 << 14) | (1 << 15) | (1 << 4) | (1 << 5) | (1 << 9));
        TransferEnd = 1;
        return (TransferError);
    }
    //数据CRC错误
    if (SDIO_GetFlagStatus(SDIO_FLAG_DCRCFAIL) != RESET)
    {
        //清错误标志
        SDIO_ClearFlag(SDIO_FLAG_DCRCFAIL);
        //关闭相关中断
        SDIO->MASK &= ~((1 << 1) | (1 << 3) | (1 << 8) | (1 << 14) | (1 << 15) | (1 << 4) | (1 << 5) | (1 << 9));
        TransferError = SD_DATA_CRC_FAIL;
        return (SD_DATA_CRC_FAIL);
    }
    //数据超时错误
    if (SDIO_GetFlagStatus(SDIO_FLAG_DTIMEOUT) != RESET)
    {
        //清中断标志
        SDIO_ClearFlag(SDIO_FLAG_DTIMEOUT);
        //关闭相关中断
        SDIO->MASK &= ~((1 << 1) | (1 << 3) | (1 << 8) | (1 << 14) | (1 << 15) | (1 << 4) | (1 << 5) | (1 << 9));
        TransferError = SD_DATA_TIMEOUT;
        return (SD_DATA_TIMEOUT);
    }
    if (SDIO_GetFlagStatus(SDIO_FLAG_RXOVERR) != RESET) //FIFO上溢错误
    {
        //清中断标志
        SDIO_ClearFlag(SDIO_FLAG_RXOVERR);
        //关闭相关中断
        SDIO->MASK &= ~((1 << 1) | (1 << 3) | (1 << 8) | (1 << 14) | (1 << 15) | (1 << 4) | (1 << 5) | (1 << 9));
        TransferError = SD_RX_OVERRUN;
        return (SD_RX_OVERRUN);
    }
    //FIFO下溢错误
    if (SDIO_GetFlagStatus(SDIO_FLAG_TXUNDERR) != RESET)
    {
        //清中断标志
        SDIO_ClearFlag(SDIO_FLAG_TXUNDERR);
        //关闭相关中断
        SDIO->MASK &= ~((1 << 1) | (1 << 3) | (1 << 8) | (1 << 14) | (1 << 15) | (1 << 4) | (1 << 5) | (1 << 9));
        TransferError = SD_TX_UNDERRUN;
        return (SD_TX_UNDERRUN);
    }
    //起始位错误
    if (SDIO_GetFlagStatus(SDIO_FLAG_STBITERR) != RESET)
    {
        //清中断标志
        SDIO_ClearFlag(SDIO_FLAG_STBITERR);
        //关闭相关中断
        SDIO->MASK &= ~((1 << 1) | (1 << 3) | (1 << 8) | (1 << 14) | (1 << 15) | (1 << 4) | (1 << 5) | (1 << 9));
        TransferError = SD_START_BIT_ERR;
        return (SD_START_BIT_ERR);
    }
    return (SD_OK);
}

//检查CMD0的执行状态
//返回值:sd卡错误码
SD_Error CmdError(void)
{
    SD_Error errorstatus = SD_OK;
    uint32_t timeout = SDIO_CMD0TIMEOUT;
    while (timeout--)
    {
        //命令已发送(无需响应)
        if (SDIO_GetFlagStatus(SDIO_FLAG_CMDSENT) != RESET)
            break;
    }
    if (timeout == 0)
        return SD_CMD_RSP_TIMEOUT;
    //清除所有标记
    SDIO_ClearFlag(SDIO_STATIC_FLAGS);
    return errorstatus;
}

//检查R7响应的错误状态
//返回值:sd卡错误码
SD_Error CmdResp7Error(void)
{
    SD_Error errorstatus = SD_OK;
    uint32_t status;
    uint32_t timeout = SDIO_CMD0TIMEOUT;
    while (timeout--)
    {
        status = SDIO->STA;
        //CRC错误/命令响应超时/已经收到响应(CRC校验成功)
        if (status & ((1 << 0) | (1 << 2) | (1 << 6)))
            break;
    }
    //响应超时
    if ((timeout == 0) || (status & (1 << 2)))
    {
        //当前卡不是2.0兼容卡,或者不支持设定的电压范围
        errorstatus = SD_CMD_RSP_TIMEOUT;
        //清除命令响应超时标志
        SDIO_ClearFlag(SDIO_FLAG_CTIMEOUT);
        return errorstatus;
    }
    //成功接收到响应
    if (status & 1 << 6)
    {
        errorstatus = SD_OK;
        //清除响应标志
        SDIO_ClearFlag(SDIO_FLAG_CMDREND);
    }
    return errorstatus;
}

//检查R1响应的错误状态
//cmd:当前命令
//返回值:sd卡错误码
SD_Error CmdResp1Error(uint8_t cmd)
{
    uint32_t status;
    while (1)
    {
        status = SDIO->STA;
        //CRC错误/命令响应超时/已经收到响应(CRC校验成功)
        if (status & ((1 << 0) | (1 << 2) | (1 << 6)))
            break;
    }
    //响应超时
    if (SDIO_GetFlagStatus(SDIO_FLAG_CTIMEOUT) != RESET)
    {
        //清除命令响应超时标志
        SDIO_ClearFlag(SDIO_FLAG_CTIMEOUT);
        return SD_CMD_RSP_TIMEOUT;
    }
    //CRC错误
    if (SDIO_GetFlagStatus(SDIO_FLAG_CCRCFAIL) != RESET)
    {
        //清除标志
        SDIO_ClearFlag(SDIO_FLAG_CCRCFAIL);
        return SD_CMD_CRC_FAIL;
    }
    //命令不匹配
    if (SDIO->RESPCMD != cmd)
        return SD_ILLEGAL_CMD;
    //清除所有标记
    SDIO_ClearFlag(SDIO_STATIC_FLAGS);
    //返回卡响应
    return (SD_Error)(SDIO->RESP1 & SD_OCR_ERRORBITS);
}

//检查R3响应的错误状态
//返回值:错误状态
SD_Error CmdResp3Error(void)
{
    uint32_t status;
    while (1)
    {
        status = SDIO->STA;
        //CRC错误/命令响应超时/已经收到响应(CRC校验成功)
        if (status & ((1 << 0) | (1 << 2) | (1 << 6)))
            break;
    }
    //响应超时
    if (SDIO_GetFlagStatus(SDIO_FLAG_CTIMEOUT) != RESET)
    {
        //清除命令响应超时标志
        SDIO_ClearFlag(SDIO_FLAG_CTIMEOUT);
        return SD_CMD_RSP_TIMEOUT;
    }
    //清除所有标记
    SDIO_ClearFlag(SDIO_STATIC_FLAGS);
    return SD_OK;
}

//检查R2响应的错误状态
//返回值:错误状态
SD_Error CmdResp2Error(void)
{
    SD_Error errorstatus = SD_OK;
    uint32_t status;
    uint32_t timeout = SDIO_CMD0TIMEOUT;
    while (timeout--)
    {
        status = SDIO->STA;
        //CRC错误/命令响应超时/已经收到响应(CRC校验成功)
        if (status & ((1 << 0) | (1 << 2) | (1 << 6)))
            break;
    }
    //响应超时
    if ((timeout == 0) || (status & (1 << 2)))
    {
        errorstatus = SD_CMD_RSP_TIMEOUT;
        //清除命令响应超时标志
        SDIO_ClearFlag(SDIO_FLAG_CTIMEOUT);
        return errorstatus;
    }
    //CRC错误
    if (SDIO_GetFlagStatus(SDIO_FLAG_CCRCFAIL) != RESET)
    {
        errorstatus = SD_CMD_CRC_FAIL;
        //清除响应标志
        SDIO_ClearFlag(SDIO_FLAG_CCRCFAIL);
    }
    //清除所有标记
    SDIO_ClearFlag(SDIO_STATIC_FLAGS);
    return errorstatus;
}

//检查R6响应的错误状态
//cmd:之前发送的命令
//prca:卡返回的RCA地址
//返回值:错误状态
SD_Error CmdResp6Error(uint8_t cmd, uint16_t *prca)
{
    SD_Error errorstatus = SD_OK;
    uint32_t status;
    uint32_t rspr1;
    while (1)
    {
        status = SDIO->STA;
        //CRC错误/命令响应超时/已经收到响应(CRC校验成功)
        if (status & ((1 << 0) | (1 << 2) | (1 << 6)))
            break;
    }
    //响应超时
    if (SDIO_GetFlagStatus(SDIO_FLAG_CTIMEOUT) != RESET)
    {
        //清除命令响应超时标志
        SDIO_ClearFlag(SDIO_FLAG_CTIMEOUT);
        return SD_CMD_RSP_TIMEOUT;
    }
    //CRC错误
    if (SDIO_GetFlagStatus(SDIO_FLAG_CCRCFAIL) != RESET)
    {
        //清除响应标志
        SDIO_ClearFlag(SDIO_FLAG_CCRCFAIL);
        return SD_CMD_CRC_FAIL;
    }
    //判断是否响应cmd命令
    if (SDIO->RESPCMD != cmd)
    {
        return SD_ILLEGAL_CMD;
    }
    //清除所有标记
    SDIO_ClearFlag(SDIO_STATIC_FLAGS);
    //得到响应
    rspr1 = SDIO->RESP1;
    if (SD_ALLZERO == (rspr1 & (SD_R6_GENERAL_UNKNOWN_ERROR | SD_R6_ILLEGAL_CMD | SD_R6_COM_CRC_FAILED)))
    {
        //右移16位得到,rca
        *prca = (uint16_t)(rspr1 >> 16);
        return errorstatus;
    }
    if (rspr1 & SD_R6_GENERAL_UNKNOWN_ERROR)
        return SD_GENERAL_UNKNOWN_ERROR;
    if (rspr1 & SD_R6_ILLEGAL_CMD)
        return SD_ILLEGAL_CMD;
    if (rspr1 & SD_R6_COM_CRC_FAILED)
        return SD_COM_CRC_FAILED;
    return errorstatus;
}

//SDIO使能宽总线模式
//enx:0,不使能;1,使能;
//返回值:错误状态
SD_Error SDEnWideBus(uint8_t enx)
{
    SD_Error errorstatus = SD_OK;
    uint32_t scr[2] = {0, 0};
    uint8_t arg = 0X00;
    if (enx)
        arg = 0X02;
    else
        arg = 0X00;
    //SD卡处于LOCKED状态
    if (SDIO->RESP1 & SD_CARD_LOCKED)
        return SD_LOCK_UNLOCK_FAILED;
    //得到SCR寄存器数据
    errorstatus = FindSCR(RCA, scr);
    if (errorstatus != SD_OK)
        return errorstatus;
    //支持宽总线
    if ((scr[1] & SD_WIDE_BUS_SUPPORT) != SD_ALLZERO)
    {
        //发送CMD55+RCA,短响应
        SDIO_CmdInitStructure.SDIO_Argument = (uint32_t)RCA << 16;
        SDIO_CmdInitStructure.SDIO_CmdIndex = SD_CMD_APP_CMD;
        SDIO_CmdInitStructure.SDIO_Response = SDIO_Response_Short;
        SDIO_CmdInitStructure.SDIO_Wait = SDIO_Wait_No;
        SDIO_CmdInitStructure.SDIO_CPSM = SDIO_CPSM_Enable;
        SDIO_SendCommand(&SDIO_CmdInitStructure);

        errorstatus = CmdResp1Error(SD_CMD_APP_CMD);

        if (errorstatus != SD_OK)
            return errorstatus;
        //发送ACMD6,短响应,参数:10,4位;00,1位.
        SDIO_CmdInitStructure.SDIO_Argument = arg;
        SDIO_CmdInitStructure.SDIO_CmdIndex = SD_CMD_APP_SD_SET_BUSWIDTH;
        SDIO_CmdInitStructure.SDIO_Response = SDIO_Response_Short;
        SDIO_CmdInitStructure.SDIO_Wait = SDIO_Wait_No;
        SDIO_CmdInitStructure.SDIO_CPSM = SDIO_CPSM_Enable;
        SDIO_SendCommand(&SDIO_CmdInitStructure);

        errorstatus = CmdResp1Error(SD_CMD_APP_SD_SET_BUSWIDTH);

        return errorstatus;
    }
    //不支持宽总线设置
    else
        return SD_REQUEST_NOT_APPLICABLE;
}

//检查卡是否正在执行写操作
//pstatus:当前状态.
//返回值:错误代码
SD_Error IsCardProgramming(uint8_t *pstatus)
{
    volatile uint32_t respR1 = 0, status = 0;

    //卡相对地址参数
    SDIO_CmdInitStructure.SDIO_Argument = (uint32_t)RCA << 16;
    //发送CMD13
    SDIO_CmdInitStructure.SDIO_CmdIndex = SD_CMD_SEND_STATUS;
    SDIO_CmdInitStructure.SDIO_Response = SDIO_Response_Short;
    SDIO_CmdInitStructure.SDIO_Wait = SDIO_Wait_No;
    SDIO_CmdInitStructure.SDIO_CPSM = SDIO_CPSM_Enable;
    SDIO_SendCommand(&SDIO_CmdInitStructure);

    status = SDIO->STA;

    //等待操作完成
    while (!(status & ((1 << 0) | (1 << 6) | (1 << 2))))
        status = SDIO->STA;
    //CRC检测失败
    if (SDIO_GetFlagStatus(SDIO_FLAG_CCRCFAIL) != RESET)
    {
        //清除错误标记
        SDIO_ClearFlag(SDIO_FLAG_CCRCFAIL);
        return SD_CMD_CRC_FAIL;
    }
    //命令超时
    if (SDIO_GetFlagStatus(SDIO_FLAG_CTIMEOUT) != RESET)
    {
        //清除错误标记
        SDIO_ClearFlag(SDIO_FLAG_CTIMEOUT);
        return SD_CMD_RSP_TIMEOUT;
    }
    if (SDIO->RESPCMD != SD_CMD_SEND_STATUS)
        return SD_ILLEGAL_CMD;
    //清除所有标记
    SDIO_ClearFlag(SDIO_STATIC_FLAGS);
    respR1 = SDIO->RESP1;
    *pstatus = (uint8_t)((respR1 >> 9) & 0x0000000F);
    return SD_OK;
}

//读取当前卡状态
//pcardstatus:卡状态
//返回值:错误代码
SD_Error SD_SendStatus(uint32_t *pcardstatus)
{
    SD_Error errorstatus = SD_OK;
    if (pcardstatus == NULL)
    {
        errorstatus = SD_INVALID_PARAMETER;
        return errorstatus;
    }
    //发送CMD13,短响应
    SDIO_CmdInitStructure.SDIO_Argument = (uint32_t)RCA << 16;
    SDIO_CmdInitStructure.SDIO_CmdIndex = SD_CMD_SEND_STATUS;
    SDIO_CmdInitStructure.SDIO_Response = SDIO_Response_Short;
    SDIO_CmdInitStructure.SDIO_Wait = SDIO_Wait_No;
    SDIO_CmdInitStructure.SDIO_CPSM = SDIO_CPSM_Enable;
    SDIO_SendCommand(&SDIO_CmdInitStructure);
    //查询响应状态
    errorstatus = CmdResp1Error(SD_CMD_SEND_STATUS);
    if (errorstatus != SD_OK)
        return errorstatus;
    //读取响应值
    *pcardstatus = SDIO->RESP1;
    return errorstatus;
}

//返回SD卡的状态
//返回值:SD卡状态
SDCardState SD_GetState(void)
{
    uint32_t resp1 = 0;
    if (SD_SendStatus(&resp1) != SD_OK)
        return SD_CARD_ERROR;
    else
        return (SDCardState)((resp1 >> 9) & 0x0F);
}

//查找SD卡的SCR寄存器值
//rca:卡相对地址
//pscr:数据缓存区(存储SCR内容)
//返回值:错误状态
SD_Error FindSCR(uint16_t rca, uint32_t *pscr)
{
    uint32_t index = 0;
    SD_Error errorstatus = SD_OK;
    uint32_t tempscr[2] = {0, 0};
    //发送CMD16,短响应,设置Block Size为8字节
    SDIO_CmdInitStructure.SDIO_Argument = (uint32_t)8;
    //	 cmd16
    SDIO_CmdInitStructure.SDIO_CmdIndex = SD_CMD_SET_BLOCKLEN;
    //r1
    SDIO_CmdInitStructure.SDIO_Response = SDIO_Response_Short;
    SDIO_CmdInitStructure.SDIO_Wait = SDIO_Wait_No;
    SDIO_CmdInitStructure.SDIO_CPSM = SDIO_CPSM_Enable;
    SDIO_SendCommand(&SDIO_CmdInitStructure);

    errorstatus = CmdResp1Error(SD_CMD_SET_BLOCKLEN);

    if (errorstatus != SD_OK)
        return errorstatus;

    SDIO_CmdInitStructure.SDIO_Argument = (uint32_t)RCA << 16;
    //发送CMD55,短响应
    SDIO_CmdInitStructure.SDIO_CmdIndex = SD_CMD_APP_CMD;
    SDIO_CmdInitStructure.SDIO_Response = SDIO_Response_Short;
    SDIO_CmdInitStructure.SDIO_Wait = SDIO_Wait_No;
    SDIO_CmdInitStructure.SDIO_CPSM = SDIO_CPSM_Enable;
    SDIO_SendCommand(&SDIO_CmdInitStructure);

    errorstatus = CmdResp1Error(SD_CMD_APP_CMD);
    if (errorstatus != SD_OK)
        return errorstatus;

    SDIO_DataInitStructure.SDIO_DataTimeOut = SD_DATATIMEOUT;
    //8个字节长度,block为8字节,SD卡到SDIO.
    SDIO_DataInitStructure.SDIO_DataLength = 8;
    //块大小8byte
    SDIO_DataInitStructure.SDIO_DataBlockSize = SDIO_DataBlockSize_8b;
    SDIO_DataInitStructure.SDIO_TransferDir = SDIO_TransferDir_ToSDIO;
    SDIO_DataInitStructure.SDIO_TransferMode = SDIO_TransferMode_Block;
    SDIO_DataInitStructure.SDIO_DPSM = SDIO_DPSM_Enable;
    SDIO_DataConfig(&SDIO_DataInitStructure);

    SDIO_CmdInitStructure.SDIO_Argument = 0x0;
    //发送ACMD51,短响应,参数为0
    SDIO_CmdInitStructure.SDIO_CmdIndex = SD_CMD_SD_APP_SEND_SCR;
    //r1
    SDIO_CmdInitStructure.SDIO_Response = SDIO_Response_Short;
    SDIO_CmdInitStructure.SDIO_Wait = SDIO_Wait_No;
    SDIO_CmdInitStructure.SDIO_CPSM = SDIO_CPSM_Enable;
    SDIO_SendCommand(&SDIO_CmdInitStructure);

    errorstatus = CmdResp1Error(SD_CMD_SD_APP_SEND_SCR);
    if (errorstatus != SD_OK)
        return errorstatus;
    while (!(SDIO->STA & (SDIO_FLAG_RXOVERR | SDIO_FLAG_DCRCFAIL | SDIO_FLAG_DTIMEOUT | SDIO_FLAG_DBCKEND | SDIO_FLAG_STBITERR)))
    {
        //接收FIFO数据可用
        if (SDIO_GetFlagStatus(SDIO_FLAG_RXDAVL) != RESET)
        {
            //读取FIFO内容
            *(tempscr + index) = SDIO->FIFO;
            index++;
            if (index >= 2)
                break;
        }
    }
    //数据超时错误
    if (SDIO_GetFlagStatus(SDIO_FLAG_DTIMEOUT) != RESET)
    {
        //清错误标志
        SDIO_ClearFlag(SDIO_FLAG_DTIMEOUT);
        return SD_DATA_TIMEOUT;
    }
    //数据块CRC错误
    else if (SDIO_GetFlagStatus(SDIO_FLAG_DCRCFAIL) != RESET)
    {
        //清错误标志
        SDIO_ClearFlag(SDIO_FLAG_DCRCFAIL);
        return SD_DATA_CRC_FAIL;
    }
    //接收fifo上溢错误
    else if (SDIO_GetFlagStatus(SDIO_FLAG_RXOVERR) != RESET)
    {
        //清错误标志
        SDIO_ClearFlag(SDIO_FLAG_RXOVERR);
        return SD_RX_OVERRUN;
    }
    //接收起始位错误
    else if (SDIO_GetFlagStatus(SDIO_FLAG_STBITERR) != RESET)
    {
        //清错误标志
        SDIO_ClearFlag(SDIO_FLAG_STBITERR);
        return SD_START_BIT_ERR;
    }
    //清除所有标记
    SDIO_ClearFlag(SDIO_STATIC_FLAGS);
    //把数据顺序按8位为单位倒过来.
    *(pscr + 1) = ((tempscr[0] & SD_0TO7BITS) << 24) | ((tempscr[0] & SD_8TO15BITS) << 8) | ((tempscr[0] & SD_16TO23BITS) >> 8) | ((tempscr[0] & SD_24TO31BITS) >> 24);
    *(pscr) = ((tempscr[1] & SD_0TO7BITS) << 24) | ((tempscr[1] & SD_8TO15BITS) << 8) | ((tempscr[1] & SD_16TO23BITS) >> 8) | ((tempscr[1] & SD_24TO31BITS) >> 24);
    return errorstatus;
}

//得到NumberOfBytes以2为底的指数.
//NumberOfBytes:字节数.
//返回值:以2为底的指数值
uint8_t convert_from_bytes_to_power_of_two(uint16_t NumberOfBytes)
{
    uint8_t count = 0;
    while (NumberOfBytes != 1)
    {
        NumberOfBytes >>= 1;
        count++;
    }
    return count;
}

//配置SDIO DMA
//mbuf:存储器地址
//bufsize:传输数据量
//dir:方向;DMA_DIR_MemoryToPeripheral  存储器-->SDIO(写数据);DMA_DIR_PeripheralToMemory SDIO-->存储器(读数据);
void SD_DMA_Config(uint32_t *mbuf, uint32_t bufsize, uint32_t dir)
{

    DMA_InitTypeDef DMA_InitStructure;
    //等待DMA可配置
    while (DMA_GetCmdStatus(DMA2_Stream3) != DISABLE)
        ;
    //清空之前该stream3上的所有中断标志
    DMA_DeInit(DMA2_Stream3);
    //通道选择
    DMA_InitStructure.DMA_Channel = DMA_Channel_4;
    //DMA外设地址
    DMA_InitStructure.DMA_PeripheralBaseAddr = (uint32_t)&SDIO->FIFO;
    //DMA 存储器0地址
    DMA_InitStructure.DMA_Memory0BaseAddr = (uint32_t)mbuf;
    //存储器到外设模式
    DMA_InitStructure.DMA_DIR = dir;
    //数据传输量
    DMA_InitStructure.DMA_BufferSize = 0;
    //外设非增量模式
    DMA_InitStructure.DMA_PeripheralInc = DMA_PeripheralInc_Disable;
    //存储器增量模式
    DMA_InitStructure.DMA_MemoryInc = DMA_MemoryInc_Enable;
    //外设数据长度:32位
    DMA_InitStructure.DMA_PeripheralDataSize = DMA_PeripheralDataSize_Word;
    //存储器数据长度:32位
    DMA_InitStructure.DMA_MemoryDataSize = DMA_MemoryDataSize_Word;
    // 使用普通模式
    DMA_InitStructure.DMA_Mode = DMA_Mode_Normal;
    //最高优先级
    DMA_InitStructure.DMA_Priority = DMA_Priority_VeryHigh;
    //FIFO使能
    DMA_InitStructure.DMA_FIFOMode = DMA_FIFOMode_Enable;
    //全FIFO
    DMA_InitStructure.DMA_FIFOThreshold = DMA_FIFOThreshold_Full;
    //外设突发4次传输
    DMA_InitStructure.DMA_MemoryBurst = DMA_MemoryBurst_INC4;
    //存储器突发4次传输
    DMA_InitStructure.DMA_PeripheralBurst = DMA_PeripheralBurst_INC4;
    //初始化DMA Stream
    DMA_Init(DMA2_Stream3, &DMA_InitStructure);
    //外设流控制
    DMA_FlowControllerConfig(DMA2_Stream3, DMA_FlowCtrl_Peripheral);
    //开启DMA传输
    DMA_Cmd(DMA2_Stream3, ENABLE);
}

//读SD卡
//buf:读数据缓存区
//sector:扇区地址
//cnt:扇区个数
//返回值:错误状态;0,正常;其他,错误代码;
uint8_t SD_ReadDisk(uint8_t *buf, uint32_t sector, uint8_t cnt)
{
    uint8_t sta = SD_OK;
    long long lsector = sector;
    uint8_t n;
    lsector <<= 9;
    OS_ERR err;
    //获取信号量
    if (OSRunning)
    {
        //申请互斥信号量,防止多任务抢占
        OSMutexPend(&MUTEX_SDCARD, 0, OS_OPT_PEND_BLOCKING, 0, &err);
    }
    if ((uint32_t)buf % 4 != 0)
    {
        for (n = 0; n < cnt; n++)
        {
            //单个sector的读操作
            sta = SD_ReadBlock(SDIO_DATA_BUFFER, lsector + 512 * n, 512);
            memcpy(buf, SDIO_DATA_BUFFER, 512);
            buf += 512;
        }
    }
    else
    {
        //单个sector的读操作
        if (cnt == 1)
            sta = SD_ReadBlock(buf, lsector, 512);
        //多个sector
        else
            sta = SD_ReadMultiBlocks(buf, lsector, 512, cnt);
    }
    //释放信号量
    if (OSRunning)
    {
        //释放互斥信号量
        OSMutexPost(&MUTEX_SDCARD, OS_OPT_POST_NO_SCHED, &err);
    }
    return sta;
}
//写SD卡
//buf:写数据缓存区
//sector:扇区地址
//cnt:扇区个数
//返回值:错误状态;0,正常;其他,错误代码;
uint8_t SD_WriteDisk(uint8_t *buf, uint32_t sector, uint8_t cnt)
{
    uint8_t sta = SD_OK;
    uint8_t n;
    long long lsector = sector;
    lsector <<= 9;
    OS_ERR err;
    //获取信号量
    if (OSRunning)
    {
        //申请互斥信号量,防止多任务抢占
        OSMutexPend(&MUTEX_SDCARD, 0, OS_OPT_PEND_BLOCKING, 0, &err);
    }
    if ((uint32_t)buf % 4 != 0)
    {
        for (n = 0; n < cnt; n++)
        {
            memcpy(SDIO_DATA_BUFFER, buf, 512);
            //单个sector的写操作
            sta = SD_WriteBlock(SDIO_DATA_BUFFER, lsector + 512 * n, 512);
            buf += 512;
        }
    }
    else
    {
        //单个sector的写操作
        if (cnt == 1)
            sta = SD_WriteBlock(buf, lsector, 512);
        //多个sector
        else
            sta = SD_WriteMultiBlocks(buf, lsector, 512, cnt);
    }
    //释放信号量
    if (OSRunning)
    {
        //释放互斥信号量
        OSMutexPost(&MUTEX_SDCARD, OS_OPT_POST_NO_SCHED, &err);
    }
    return sta;
}

void ConsoleSendCardMessage(void)
{
    sd_sdio_debug_printf("SDCardInfo.CardType = %d\r\n", SDCardInfo.CardType);
    sd_sdio_debug_printf("SDCardInfo.CardCapacity = %d MB\r\n", (uint32_t)(SDCardInfo.CardCapacity / (1024 * 1024)));
    sd_sdio_debug_printf("SDCardInfo.CardBlockSize = %d\r\n", SDCardInfo.CardBlockSize);
    sd_sdio_debug_printf("SDCardInfo.RCA = %d\r\n", SDCardInfo.RCA);

    sd_sdio_debug_printf("SDCardInfo.SD_cid.CID_CRC = 0x%X\r\n", SDCardInfo.SD_cid.CID_CRC);
    sd_sdio_debug_printf("SDCardInfo.SD_cid.ManufactDate = 0x%X\r\n", SDCardInfo.SD_cid.ManufactDate);
    sd_sdio_debug_printf("SDCardInfo.SD_cid.ManufacturerID = 0x%X\r\n", SDCardInfo.SD_cid.ManufacturerID);
    sd_sdio_debug_printf("SDCardInfo.SD_cid.OEM_AppliID = 0x%X\r\n", SDCardInfo.SD_cid.OEM_AppliID);
    sd_sdio_debug_printf("SDCardInfo.SD_cid.ProdName1 = 0x%X\r\n", SDCardInfo.SD_cid.ProdName1);
    sd_sdio_debug_printf("SDCardInfo.SD_cid.ProdName2 = 0x%X\r\n", SDCardInfo.SD_cid.ProdName2);
    sd_sdio_debug_printf("SDCardInfo.SD_cid.ProdRev = 0x%X\r\n", SDCardInfo.SD_cid.ProdRev);
    sd_sdio_debug_printf("SDCardInfo.SD_cid.ProdSN = 0x%X\r\n", SDCardInfo.SD_cid.ProdSN);
    sd_sdio_debug_printf("SDCardInfo.SD_cid.Reserved1 = 0x%X\r\n", SDCardInfo.SD_cid.Reserved1);
    sd_sdio_debug_printf("SDCardInfo.SD_cid.Reserved2 = 0x%X\r\n", SDCardInfo.SD_cid.Reserved2);

    sd_sdio_debug_printf("SDCardInfo.SD_csd.CardComdClasses = %d\r\n", SDCardInfo.SD_csd.CardComdClasses);
    sd_sdio_debug_printf("SDCardInfo.SD_csd.ContentProtectAppli = %d\r\n", SDCardInfo.SD_csd.ContentProtectAppli);
    sd_sdio_debug_printf("SDCardInfo.SD_csd.CopyFlag = %d\r\n", SDCardInfo.SD_csd.CopyFlag);
    sd_sdio_debug_printf("SDCardInfo.SD_csd.CSDStruct = %d\r\n", SDCardInfo.SD_csd.CSDStruct);
    sd_sdio_debug_printf("SDCardInfo.SD_csd.CSD_CRC = %d\r\n", SDCardInfo.SD_csd.CSD_CRC);
    sd_sdio_debug_printf("SDCardInfo.SD_csd.DeviceSize = %d\r\n", SDCardInfo.SD_csd.DeviceSize);
    sd_sdio_debug_printf("SDCardInfo.SD_csd.DeviceSizeMul = %d\r\n", SDCardInfo.SD_csd.DeviceSizeMul);
    sd_sdio_debug_printf("SDCardInfo.SD_csd.DSRImpl = %d\r\n", SDCardInfo.SD_csd.DSRImpl);
    sd_sdio_debug_printf("SDCardInfo.SD_csd.ECC = %d\r\n", SDCardInfo.SD_csd.ECC);
    sd_sdio_debug_printf("SDCardInfo.SD_csd.EraseGrMul = %d\r\n", SDCardInfo.SD_csd.EraseGrMul);
    sd_sdio_debug_printf("SDCardInfo.SD_csd.EraseGrSize = %d\r\n", SDCardInfo.SD_csd.EraseGrSize);
    sd_sdio_debug_printf("SDCardInfo.SD_csd.FileFormat = %d\r\n", SDCardInfo.SD_csd.FileFormat);
    sd_sdio_debug_printf("SDCardInfo.SD_csd.FileFormatGrouop = %d\r\n", SDCardInfo.SD_csd.FileFormatGrouop);
    sd_sdio_debug_printf("SDCardInfo.SD_csd.ManDeflECC = %d\r\n", SDCardInfo.SD_csd.ManDeflECC);
    sd_sdio_debug_printf("SDCardInfo.SD_csd.MaxBusClkFrec = %d\r\n", SDCardInfo.SD_csd.MaxBusClkFrec);
    sd_sdio_debug_printf("SDCardInfo.SD_csd.MaxRdCurrentVDDMax = %d\r\n", SDCardInfo.SD_csd.MaxRdCurrentVDDMax);
    sd_sdio_debug_printf("SDCardInfo.SD_csd.MaxRdCurrentVDDMin = %d\r\n", SDCardInfo.SD_csd.MaxRdCurrentVDDMin);
    sd_sdio_debug_printf("SDCardInfo.SD_csd.MaxWrBlockLen = %d\r\n", SDCardInfo.SD_csd.MaxWrBlockLen);
    sd_sdio_debug_printf("SDCardInfo.SD_csd.MaxWrCurrentVDDMax = %d\r\n", SDCardInfo.SD_csd.MaxWrCurrentVDDMax);
    sd_sdio_debug_printf("SDCardInfo.SD_csd.MaxWrCurrentVDDMin = %d\r\n", SDCardInfo.SD_csd.MaxWrCurrentVDDMin);
    sd_sdio_debug_printf("SDCardInfo.SD_csd.NSAC = %d\r\n", SDCardInfo.SD_csd.NSAC);
    sd_sdio_debug_printf("SDCardInfo.SD_csd.PartBlockRead = %d\r\n", SDCardInfo.SD_csd.PartBlockRead);
    sd_sdio_debug_printf("SDCardInfo.SD_csd.PermWrProtect = %d\r\n", SDCardInfo.SD_csd.PermWrProtect);
    sd_sdio_debug_printf("SDCardInfo.SD_csd.RdBlockLen = %d\r\n", SDCardInfo.SD_csd.RdBlockLen);
    sd_sdio_debug_printf("SDCardInfo.SD_csd.RdBlockMisalign = %d\r\n", SDCardInfo.SD_csd.RdBlockMisalign);
    sd_sdio_debug_printf("SDCardInfo.SD_csd.Reserved1 = %d\r\n", SDCardInfo.SD_csd.Reserved1);
    sd_sdio_debug_printf("SDCardInfo.SD_csd.Reserved2 = %d\r\n", SDCardInfo.SD_csd.Reserved2);
    sd_sdio_debug_printf("SDCardInfo.SD_csd.Reserved3 = %d\r\n", SDCardInfo.SD_csd.Reserved3);
    sd_sdio_debug_printf("SDCardInfo.SD_csd.Reserved4 = %d\r\n", SDCardInfo.SD_csd.Reserved4);
    sd_sdio_debug_printf("SDCardInfo.SD_csd.SysSpecVersion = %d\r\n", SDCardInfo.SD_csd.SysSpecVersion);
    sd_sdio_debug_printf("SDCardInfo.SD_csd.TAAC = %d\r\n", SDCardInfo.SD_csd.TAAC);
    sd_sdio_debug_printf("SDCardInfo.SD_csd.TempWrProtect = %d\r\n", SDCardInfo.SD_csd.TempWrProtect);
    sd_sdio_debug_printf("SDCardInfo.SD_csd.WrBlockMisalign = %d\r\n", SDCardInfo.SD_csd.WrBlockMisalign);
    sd_sdio_debug_printf("SDCardInfo.SD_csd.WriteBlockPaPartial = %d\r\n", SDCardInfo.SD_csd.WriteBlockPaPartial);
    sd_sdio_debug_printf("SDCardInfo.SD_csd.WrProtectGrEnable = %d\r\n", SDCardInfo.SD_csd.WrProtectGrEnable);
    sd_sdio_debug_printf("SDCardInfo.SD_csd.WrProtectGrSize = %d\r\n", SDCardInfo.SD_csd.WrProtectGrSize);
    sd_sdio_debug_printf("SDCardInfo.SD_csd.WrSpeedFact = %d\r\n", SDCardInfo.SD_csd.WrSpeedFact);
}
