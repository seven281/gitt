#include "SystemInput.h"

//读取系统输入
LH_ERR SystemInputRead(SYSTEM_INPUT_INDEX inputIndex,BitAction* inputValue)
{
    LH_ERR errorCode;
    BitAction inputValueLocal;
    if(inputIndex > SYSTEM_INPUT_MAIN_BOARD_ARM_SEN32_PE4)
    {
        return LH_ERR_SYSTEM_IN_RANGE;
    }
    if(inputIndex >= SYSTEM_INPUT_MAIN_BOARD_ARM_SEN1_PH10)
    {
        //主控板资源
        inputValueLocal = BoardInReadSingle((BOARD_IN_PIN)(inputIndex - SYSTEM_INPUT_MAIN_BOARD_ARM_SEN1_PH10));
        *inputValue = inputValueLocal;
        return LH_ERR_NONE;
    }
    else
    {
        //下位机资源
        CAN2_SUB_INPUT can2Input;
        //初始化
        Can2SubPortInDataStructInit(&can2Input);
        can2Input.inputPinIndex = (CAN2_SUB_IN_PIN_INDEX)(inputIndex - SYSTEM_INPUT_LOW_BOARD1_CPLD_NOP1);
        //读取状态
        errorCode = Can2SubPortInReadState(&can2Input);
        if(errorCode == LH_ERR_NONE)
        {
            *inputValue = can2Input.pinValue;
        }
        return errorCode;
    }
}
















