#include "MCU_Port.h"

//初始化函数指针
typedef void (*MCU_PortInitFuncPtr)(uint16_t pinIndex, GPIOMode_TypeDef mode, GPIOOType_TypeDef oType, 
                                                        GPIOPuPd_TypeDef pupd, GPIOSpeed_TypeDef speed);
//写入函数指针
typedef void (*MCU_PortWriteSingleFuncPtr)(uint16_t pinIndex, BitAction value);
//读取函数指针
typedef BitAction (*MCU_PortReadSingleFuncPtr)(uint16_t pinIndex);



//PORTA
void MCU_PortInitA(uint16_t pinIndex, GPIOMode_TypeDef mode, GPIOOType_TypeDef oType, GPIOPuPd_TypeDef pupd, GPIOSpeed_TypeDef speed)
{
    GPIO_InitTypeDef GPIO_InitStructure;
    /**使能端口时钟 */
    RCC_AHB1PeriphClockCmd(RCC_AHB1Periph_GPIOA, ENABLE);
    GPIO_InitStructure.GPIO_Pin = pinIndex;
    GPIO_InitStructure.GPIO_Mode = mode;
    GPIO_InitStructure.GPIO_OType = oType;
    GPIO_InitStructure.GPIO_PuPd = pupd;
    GPIO_InitStructure.GPIO_Speed = speed;
    /**调用初始化函数*/
    GPIO_Init(GPIOA, &GPIO_InitStructure);
}
void MCU_PortWriteSingleA(uint16_t pinIndex, BitAction value)
{
    if (Bit_RESET == value)
    {
        GPIO_ResetBits(GPIOA, pinIndex);
    }
    else
    {
        GPIO_SetBits(GPIOA, pinIndex);
    }
}
BitAction MCU_PortReadSingleA(uint16_t pinIndex)
{
    return (BitAction)GPIO_ReadInputDataBit(GPIOA, pinIndex);
}

//PORTB
void MCU_PortInitB(uint16_t pinIndex, GPIOMode_TypeDef mode, GPIOOType_TypeDef oType, GPIOPuPd_TypeDef pupd, GPIOSpeed_TypeDef speed)
{
    GPIO_InitTypeDef GPIO_InitStructure;
    /**使能端口时钟 */
    RCC_AHB1PeriphClockCmd(RCC_AHB1Periph_GPIOB, ENABLE);
    GPIO_InitStructure.GPIO_Pin = pinIndex;
    GPIO_InitStructure.GPIO_Mode = mode;
    GPIO_InitStructure.GPIO_OType = oType;
    GPIO_InitStructure.GPIO_PuPd = pupd;
    GPIO_InitStructure.GPIO_Speed = speed;
    /**调用初始化函数*/
    GPIO_Init(GPIOB, &GPIO_InitStructure);
}
void MCU_PortWriteSingleB(uint16_t pinIndex, BitAction value)
{
    if (Bit_RESET == value)
    {
        GPIO_ResetBits(GPIOB, pinIndex);
    }
    else
    {
        GPIO_SetBits(GPIOB, pinIndex);
    }
}
BitAction MCU_PortReadSingleB(uint16_t pinIndex)
{
    return (BitAction)GPIO_ReadInputDataBit(GPIOB, pinIndex);
}

//PORTC
void MCU_PortInitC(uint16_t pinIndex, GPIOMode_TypeDef mode, GPIOOType_TypeDef oType, GPIOPuPd_TypeDef pupd, GPIOSpeed_TypeDef speed)
{
    GPIO_InitTypeDef GPIO_InitStructure;
    /**使能端口时钟 */
    RCC_AHB1PeriphClockCmd(RCC_AHB1Periph_GPIOC, ENABLE);
    GPIO_InitStructure.GPIO_Pin = pinIndex;
    GPIO_InitStructure.GPIO_Mode = mode;
    GPIO_InitStructure.GPIO_OType = oType;
    GPIO_InitStructure.GPIO_PuPd = pupd;
    GPIO_InitStructure.GPIO_Speed = speed;
    /**调用初始化函数*/
    GPIO_Init(GPIOC, &GPIO_InitStructure);
}
void MCU_PortWriteSingleC(uint16_t pinIndex, BitAction value)
{
    if (Bit_RESET == value)
    {
        GPIO_ResetBits(GPIOC, pinIndex);
    }
    else
    {
        GPIO_SetBits(GPIOC, pinIndex);
    }
}
BitAction MCU_PortReadSingleC(uint16_t pinIndex)
{
    return (BitAction)GPIO_ReadInputDataBit(GPIOC, pinIndex);
}

//PORTD
void MCU_PortInitD(uint16_t pinIndex, GPIOMode_TypeDef mode, GPIOOType_TypeDef oType, GPIOPuPd_TypeDef pupd, GPIOSpeed_TypeDef speed)
{
    GPIO_InitTypeDef GPIO_InitStructure;
    /**使能端口时钟 */
    RCC_AHB1PeriphClockCmd(RCC_AHB1Periph_GPIOD, ENABLE);
    GPIO_InitStructure.GPIO_Pin = pinIndex;
    GPIO_InitStructure.GPIO_Mode = mode;
    GPIO_InitStructure.GPIO_OType = oType;
    GPIO_InitStructure.GPIO_PuPd = pupd;
    GPIO_InitStructure.GPIO_Speed = speed;
    /**调用初始化函数*/
    GPIO_Init(GPIOD, &GPIO_InitStructure);
}
void MCU_PortWriteSingleD(uint16_t pinIndex, BitAction value)
{
    if (Bit_RESET == value)
    {
        GPIO_ResetBits(GPIOD, pinIndex);
    }
    else
    {
        GPIO_SetBits(GPIOD, pinIndex);
    }
}
BitAction MCU_PortReadSingleD(uint16_t pinIndex)
{
    return (BitAction)GPIO_ReadInputDataBit(GPIOD, pinIndex);
}

//PORTE
void MCU_PortInitE(uint16_t pinIndex, GPIOMode_TypeDef mode, GPIOOType_TypeDef oType, GPIOPuPd_TypeDef pupd, GPIOSpeed_TypeDef speed)
{
    GPIO_InitTypeDef GPIO_InitStructure;
    /**使能端口时钟 */
    RCC_AHB1PeriphClockCmd(RCC_AHB1Periph_GPIOE, ENABLE);
    GPIO_InitStructure.GPIO_Pin = pinIndex;
    GPIO_InitStructure.GPIO_Mode = mode;
    GPIO_InitStructure.GPIO_OType = oType;
    GPIO_InitStructure.GPIO_PuPd = pupd;
    GPIO_InitStructure.GPIO_Speed = speed;
    /**调用初始化函数*/
    GPIO_Init(GPIOE, &GPIO_InitStructure);
}
void MCU_PortWriteSingleE(uint16_t pinIndex, BitAction value)
{
    if (Bit_RESET == value)
    {
        GPIO_ResetBits(GPIOE, pinIndex);
    }
    else
    {
        GPIO_SetBits(GPIOE, pinIndex);
    }
}
BitAction MCU_PortReadSingleE(uint16_t pinIndex)
{
    return (BitAction)GPIO_ReadInputDataBit(GPIOE, pinIndex);
}

//PORTF
void MCU_PortInitF(uint16_t pinIndex, GPIOMode_TypeDef mode, GPIOOType_TypeDef oType, GPIOPuPd_TypeDef pupd, GPIOSpeed_TypeDef speed)
{
    GPIO_InitTypeDef GPIO_InitStructure;
    /**使能端口时钟 */
    RCC_AHB1PeriphClockCmd(RCC_AHB1Periph_GPIOF, ENABLE);
    GPIO_InitStructure.GPIO_Pin = pinIndex;
    GPIO_InitStructure.GPIO_Mode = mode;
    GPIO_InitStructure.GPIO_OType = oType;
    GPIO_InitStructure.GPIO_PuPd = pupd;
    GPIO_InitStructure.GPIO_Speed = speed;
    /**调用初始化函数*/
    GPIO_Init(GPIOF, &GPIO_InitStructure);
}
void MCU_PortWriteSingleF(uint16_t pinIndex, BitAction value)
{
    if (Bit_RESET == value)
    {
        GPIO_ResetBits(GPIOF, pinIndex);
    }
    else
    {
        GPIO_SetBits(GPIOF, pinIndex);
    }
}
BitAction MCU_PortReadSingleF(uint16_t pinIndex)
{
    return (BitAction)GPIO_ReadInputDataBit(GPIOF, pinIndex);
}

//PORTG
void MCU_PortInitG(uint16_t pinIndex, GPIOMode_TypeDef mode, GPIOOType_TypeDef oType, GPIOPuPd_TypeDef pupd, GPIOSpeed_TypeDef speed)
{
    GPIO_InitTypeDef GPIO_InitStructure;
    /**使能端口时钟 */
    RCC_AHB1PeriphClockCmd(RCC_AHB1Periph_GPIOG, ENABLE);
    GPIO_InitStructure.GPIO_Pin = pinIndex;
    GPIO_InitStructure.GPIO_Mode = mode;
    GPIO_InitStructure.GPIO_OType = oType;
    GPIO_InitStructure.GPIO_PuPd = pupd;
    GPIO_InitStructure.GPIO_Speed = speed;
    /**调用初始化函数*/
    GPIO_Init(GPIOG, &GPIO_InitStructure);
}
void MCU_PortWriteSingleG(uint16_t pinIndex, BitAction value)
{
    if (Bit_RESET == value)
    {
        GPIO_ResetBits(GPIOG, pinIndex);
    }
    else
    {
        GPIO_SetBits(GPIOG, pinIndex);
    }
}
BitAction MCU_PortReadSingleG(uint16_t pinIndex)
{
    return (BitAction)GPIO_ReadInputDataBit(GPIOG, pinIndex);
}

//PORTH
void MCU_PortInitH(uint16_t pinIndex, GPIOMode_TypeDef mode, GPIOOType_TypeDef oType, GPIOPuPd_TypeDef pupd, GPIOSpeed_TypeDef speed)
{
    GPIO_InitTypeDef GPIO_InitStructure;
    /**使能端口时钟 */
    RCC_AHB1PeriphClockCmd(RCC_AHB1Periph_GPIOH, ENABLE);
    GPIO_InitStructure.GPIO_Pin = pinIndex;
    GPIO_InitStructure.GPIO_Mode = mode;
    GPIO_InitStructure.GPIO_OType = oType;
    GPIO_InitStructure.GPIO_PuPd = pupd;
    GPIO_InitStructure.GPIO_Speed = speed;
    /**调用初始化函数*/
    GPIO_Init(GPIOH, &GPIO_InitStructure);
}
void MCU_PortWriteSingleH(uint16_t pinIndex, BitAction value)
{
    if (Bit_RESET == value)
    {
        GPIO_ResetBits(GPIOH, pinIndex);
    }
    else
    {
        GPIO_SetBits(GPIOH, pinIndex);
    }
}
BitAction MCU_PortReadSingleH(uint16_t pinIndex)
{
    return (BitAction)GPIO_ReadInputDataBit(GPIOH, pinIndex);
}

//PORTI
void MCU_PortInitI(uint16_t pinIndex, GPIOMode_TypeDef mode, GPIOOType_TypeDef oType, GPIOPuPd_TypeDef pupd, GPIOSpeed_TypeDef speed)
{
    GPIO_InitTypeDef GPIO_InitStructure;
    /**使能端口时钟 */
    RCC_AHB1PeriphClockCmd(RCC_AHB1Periph_GPIOI, ENABLE);
    GPIO_InitStructure.GPIO_Pin = pinIndex;
    GPIO_InitStructure.GPIO_Mode = mode;
    GPIO_InitStructure.GPIO_OType = oType;
    GPIO_InitStructure.GPIO_PuPd = pupd;
    GPIO_InitStructure.GPIO_Speed = speed;
    /**调用初始化函数*/
    GPIO_Init(GPIOI, &GPIO_InitStructure);
}
void MCU_PortWriteSingleI(uint16_t pinIndex, BitAction value)
{
    if (Bit_RESET == value)
    {
        GPIO_ResetBits(GPIOI, pinIndex);
    }
    else
    {
        GPIO_SetBits(GPIOI, pinIndex);
    }
}
BitAction MCU_PortReadSingleI(uint16_t pinIndex)
{
    return (BitAction)GPIO_ReadInputDataBit(GPIOI, pinIndex);
}

//初始化函数指针数组
static const MCU_PortInitFuncPtr MCU_PortInitFuncPtrArray[] = {
    MCU_PortInitA,
    MCU_PortInitB,
    MCU_PortInitC,
    MCU_PortInitD,
    MCU_PortInitE,
    MCU_PortInitF,
    MCU_PortInitG,
    MCU_PortInitH,
    MCU_PortInitI,
};

//写入数据函数指针数组
static const MCU_PortWriteSingleFuncPtr MCU_PortWriteSingleFuncPtrArray[] = {
    MCU_PortWriteSingleA,
    MCU_PortWriteSingleB,
    MCU_PortWriteSingleC,
    MCU_PortWriteSingleD,
    MCU_PortWriteSingleE,
    MCU_PortWriteSingleF,
    MCU_PortWriteSingleG,
    MCU_PortWriteSingleH,
    MCU_PortWriteSingleI,
};

//读取状态函数指针数组
static const MCU_PortReadSingleFuncPtr MCU_PortReadSingleFuncPtrArray[] = {
    MCU_PortReadSingleA,
    MCU_PortReadSingleB,
    MCU_PortReadSingleC,
    MCU_PortReadSingleD,
    MCU_PortReadSingleE,
    MCU_PortReadSingleF,
    MCU_PortReadSingleG,
    MCU_PortReadSingleH,
    MCU_PortReadSingleI,
};

//辅助计算
typedef enum MCU_PORT
{
    MCU_PORT_A,
    MCU_PORT_B,
    MCU_PORT_C,
    MCU_PORT_D,
    MCU_PORT_E,
    MCU_PORT_F,
    MCU_PORT_G,
    MCU_PORT_H,
    MCU_PORT_I,
} MCU_PORT;

//静态的引脚数组,辅助计算
static const uint16_t pinIndexArray[] = {
    GPIO_Pin_0,
    GPIO_Pin_1,
    GPIO_Pin_2,
    GPIO_Pin_3,
    GPIO_Pin_4,
    GPIO_Pin_5,
    GPIO_Pin_6,
    GPIO_Pin_7,
    GPIO_Pin_8,
    GPIO_Pin_9,
    GPIO_Pin_10,
    GPIO_Pin_11,
    GPIO_Pin_12,
    GPIO_Pin_13,
    GPIO_Pin_14,
    GPIO_Pin_15,
};

//通过参数转换成调用的实际参数
static void MCU_PinCalcIndex(MCU_PIN pin, uint8_t *funcIndex, uint16_t *pinIndex)
{
    if (pin <= MCU_PIN_A_15)
    {
        *funcIndex = MCU_PORT_A;
        *pinIndex = pinIndexArray[pin];
    }
    else if (pin <= MCU_PIN_B_15)
    {
        *funcIndex = MCU_PORT_B;
        *pinIndex = pinIndexArray[pin - MCU_PIN_B_0];
    }
    else if (pin <= MCU_PIN_C_13)
    {
        *funcIndex = MCU_PORT_C;
        *pinIndex = pinIndexArray[pin - MCU_PIN_C_0];
    }
    else if (pin <= MCU_PIN_D_15)
    {
        *funcIndex = MCU_PORT_D;
        *pinIndex = pinIndexArray[pin - MCU_PIN_D_0];
    }
    else if (pin <= MCU_PIN_E_15)
    {
        *funcIndex = MCU_PORT_E;
        *pinIndex = pinIndexArray[pin - MCU_PIN_E_0];
    }
    else if (pin <= MCU_PIN_F_15)
    {
        *funcIndex = MCU_PORT_F;
        *pinIndex = pinIndexArray[pin - MCU_PIN_F_0];
    }
    else if (pin <= MCU_PIN_G_15)
    {
        *funcIndex = MCU_PORT_G;
        *pinIndex = pinIndexArray[pin - MCU_PIN_G_0];
    }
    else if (pin <= MCU_PIN_H_15)
    {
        *funcIndex = MCU_PORT_H;
        //此处需要修正,因为0 1被晶振占用,最小为2
        *pinIndex = pinIndexArray[pin - MCU_PIN_H_2 + 2];
    }
    else if (pin <= MCU_PIN_I_11)
    {
        *funcIndex = MCU_PORT_I;
        *pinIndex = pinIndexArray[pin - MCU_PIN_I_0];
    }
}

//初始化指定引脚
void MCU_PortInit(MCU_PIN pin,GPIOMode_TypeDef mode, GPIOOType_TypeDef oType,
                                GPIOPuPd_TypeDef pupd, GPIOSpeed_TypeDef speed)
{
    uint8_t funcIndex = 0;
    uint16_t pinIndex = 0;
    //判定状态
    if (pin >= MCU_PIN_COUNT)
    {
        return;
    }
    //计算序号
    MCU_PinCalcIndex(pin, &funcIndex, &pinIndex);
    //设置初始化
    MCU_PortInitFuncPtrArray[funcIndex](pinIndex, mode, oType, pupd, speed);
}

//设置指定引脚的值
void MCU_PortWriteSingle(MCU_PIN pin,BitAction value)
{
    uint8_t funcIndex = 0;
    uint16_t pinIndex = 0;
    //判定状态
    if (pin >= MCU_PIN_COUNT)
    {
        return;
    }
    //计算序号
    MCU_PinCalcIndex(pin, &funcIndex, &pinIndex);
    //写入状态
    MCU_PortWriteSingleFuncPtrArray[funcIndex](pinIndex, value);
}

//读取指定引脚的值
BitAction MCU_PortReadSingle(MCU_PIN pin)
{
    uint8_t funcIndex = 0;
    uint16_t pinIndex = 0;
    //判定状态
    if (pin >= MCU_PIN_COUNT)
    {
        return Bit_RESET;
    }
    //计算序号
    MCU_PinCalcIndex(pin, &funcIndex, &pinIndex);
    //读取状态
    return MCU_PortReadSingleFuncPtrArray[funcIndex](pinIndex);
}

//翻转指定引脚的值
void MCU_PortToogleSingle(MCU_PIN pin)
{
    uint8_t funcIndex = 0;
    uint16_t pinIndex = 0;
    //判定状态
    if (pin >= MCU_PIN_COUNT)
    {
        return;
    }
    //计算序号
    MCU_PinCalcIndex(pin, &funcIndex, &pinIndex);
    //读取数据
    BitAction pinValue = MCU_PortReadSingleFuncPtrArray[funcIndex](pinIndex);
    //数据翻转
    pinValue = (pinValue == Bit_RESET) ? Bit_SET : Bit_RESET;
    //写入数据
    MCU_PortWriteSingleFuncPtrArray[funcIndex](pinIndex, pinValue);
}
