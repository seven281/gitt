#include "SystemParam.h"

SYSTEM_ACTION_PARAM* actionParam = NULL;

//读取系统内部动作参数
void SystemReadActionParamWithIndex(uint8_t mainIndex, uint8_t subIndex,int32_t* paramValue)
{
    uint8_t* targetPtr;
    //先进行整块内存的更新
    SystemLoadActionParamOnce();
    //然后将数据返回
    switch(mainIndex)
    {
        case 0:
            targetPtr = (uint8_t*)(&(actionParam->moduleCupManageParam));
            targetPtr += (subIndex*4);
            *paramValue = *((int32_t*)targetPtr);
            break;
        case 1:
            targetPtr = (uint8_t*)(&(actionParam->moduleHandCupParam));
            targetPtr += (subIndex*4);
            *paramValue = *((int32_t*)targetPtr);
            break;
        case 2:
            targetPtr = (uint8_t*)(&(actionParam->moduleReactionParam));
            targetPtr += (subIndex*4);
            *paramValue = *((int32_t*)targetPtr);
            break;
        case 3:
            targetPtr = (uint8_t*)(&(actionParam->modulePathWayParam));
            targetPtr += (subIndex*4);
            *paramValue = *((int32_t*)targetPtr);
            break;
        case 4:
            targetPtr = (uint8_t*)(&(actionParam->moduleNeedleSampleParam));
            targetPtr += (subIndex*4);
            *paramValue = *((int32_t*)targetPtr);
            break;
        case 5:
            targetPtr = (uint8_t*)(&(actionParam->moduleNeedleReagentBeadParam));
            targetPtr += (subIndex*4);
            *paramValue = *((int32_t*)targetPtr);
            break;
        case 6:
            targetPtr = (uint8_t*)(&(actionParam->moduleNeedleWashParam));
            targetPtr += (subIndex*4);
            *paramValue = *((int32_t*)targetPtr);
            break;
        case 7:
            targetPtr = (uint8_t*)(&(actionParam->moduleMeasureRoomAndLiquidBParam));
            targetPtr += (subIndex*4);
            *paramValue = *((int32_t*)targetPtr);
            break;
        case 8:
            targetPtr = (uint8_t*)(&(actionParam->moduleReagentMixParam));
            targetPtr += (subIndex*4);
            *paramValue = *((int32_t*)targetPtr);
            break;
        case 9:
            targetPtr = (uint8_t*)(&(actionParam->moduleBeadMixParam));
            targetPtr += (subIndex*4);
            *paramValue = *((int32_t*)targetPtr);
            break;
        case 10:
            targetPtr = (uint8_t*)(&(actionParam->moduleLiquidMixAParam));
            targetPtr += (subIndex*4);
            *paramValue = *((int32_t*)targetPtr);
            break;
        case 11:
            targetPtr = (uint8_t*)(&(actionParam->moduleDishWashLiquidAParam));
            targetPtr += (subIndex*4);
            *paramValue = *((int32_t*)targetPtr);
            break;
        case 12:
            targetPtr = (uint8_t*)(&(actionParam->moduleDishReagentParam));
            targetPtr += (subIndex*4);
            *paramValue = *((int32_t*)targetPtr);
            break;
        default:
            *paramValue = 0;
            return;
    }
}

//写入系统内部动作参数
void SystemWriteActionParamWithIndex(uint8_t mainIndex, uint8_t subIndex,int32_t paramValue)
{
    uint8_t* targetPtr;
    //先修改内存中的对应值
    switch(mainIndex)
    {
        case 0:
            targetPtr = (uint8_t*)(&(actionParam->moduleCupManageParam));
            targetPtr += (subIndex*4);
            *((int32_t*)targetPtr) = paramValue;
            break;
        case 1:
            targetPtr = (uint8_t*)(&(actionParam->moduleHandCupParam));
            targetPtr += (subIndex*4);
            *((int32_t*)targetPtr) = paramValue;
            break;
        case 2:
            targetPtr = (uint8_t*)(&(actionParam->moduleReactionParam));
            targetPtr += (subIndex*4);
            *((int32_t*)targetPtr) = paramValue;
            break;
        case 3:
            targetPtr = (uint8_t*)(&(actionParam->modulePathWayParam));
            targetPtr += (subIndex*4);
            *((int32_t*)targetPtr) = paramValue;
            break;
        case 4:
            targetPtr = (uint8_t*)(&(actionParam->moduleNeedleSampleParam));
            targetPtr += (subIndex*4);
            *((int32_t*)targetPtr) = paramValue;
            break;
        case 5:
            targetPtr = (uint8_t*)(&(actionParam->moduleNeedleReagentBeadParam));
            targetPtr += (subIndex*4);
            *((int32_t*)targetPtr) = paramValue;
            break;
        case 6:
            targetPtr = (uint8_t*)(&(actionParam->moduleNeedleWashParam));
            targetPtr += (subIndex*4);
            *((int32_t*)targetPtr) = paramValue;
            break;
        case 7:
            targetPtr = (uint8_t*)(&(actionParam->moduleMeasureRoomAndLiquidBParam));
            targetPtr += (subIndex*4);
            *((int32_t*)targetPtr) = paramValue;
            break;
        case 8:
            targetPtr = (uint8_t*)(&(actionParam->moduleReagentMixParam));
            targetPtr += (subIndex*4);
            *((int32_t*)targetPtr) = paramValue;
            break;
        case 9:
            targetPtr = (uint8_t*)(&(actionParam->moduleBeadMixParam));
            targetPtr += (subIndex*4);
            *((int32_t*)targetPtr) = paramValue;
            break;
        case 10:
            targetPtr = (uint8_t*)(&(actionParam->moduleLiquidMixAParam));
            targetPtr += (subIndex*4);
            *((int32_t*)targetPtr) = paramValue;
            break;
        case 11:
            targetPtr = (uint8_t*)(&(actionParam->moduleDishWashLiquidAParam));
            targetPtr += (subIndex*4);
            *((int32_t*)targetPtr) = paramValue;
            break;
        case 12:
            targetPtr = (uint8_t*)(&(actionParam->moduleDishReagentParam));
            targetPtr += (subIndex*4);
            *((int32_t*)targetPtr) = paramValue;
            break;
        default:
            return;
    }
    //再将整块内存写入
    SystemSaveActionParamOnce();
}

//一次性加载全部系统内部参数,动作参数
void SystemLoadActionParamOnce(void)
{
    //申请内存
    if(actionParam == NULL)
    {
        do
        {
            actionParam = UserMemMalloc(SRAM_IS62,SYSTEM_ACTION_PARAM_SIZE);
            if(actionParam == NULL)
            {
                CoreDelayMs(100);
            }
        }while(actionParam == NULL);
    }
    BoardMB85RS2MT_ReadBuffer(SYSTEM_ACTION_PARAM_BASE_ADDR,(uint8_t*)actionParam,SYSTEM_ACTION_PARAM_SIZE);
}

//一次性存储系统内部全部参数,动作参数
void SystemSaveActionParamOnce(void)
{
    BoardMB85RS2MT_WriteBuffer(SYSTEM_ACTION_PARAM_BASE_ADDR,(uint8_t*)actionParam,SYSTEM_ACTION_PARAM_SIZE);
}



