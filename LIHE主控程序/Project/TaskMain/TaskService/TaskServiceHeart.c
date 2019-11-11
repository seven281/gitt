#include "TaskServiceConfig.h"
#include "TSC_Inc.h"
#include "ActionImplementInc.h"

//任务控制块,8字节对齐
#pragma pack(8)
//任务堆栈
CPU_STK stackBufferTaskServiceHeart[STK_SIZE_TASK_SERVICE_HEART];
#pragma pack()
#define KEY_CUPSTACK SYSTEM_INPUT_MAIN_BOARD_ARM_SEN1_PH10
#define KEY_REAGENTDISK SYSTEM_INPUT_MAIN_BOARD_ARM_SEN3_PH8



//任务控制块
OS_TCB tcbTaskServiceHeart;
static uint8_t keybuff[KEY_COUNT];

void keyscan(void);
void privatetestbywj(void);
//函数指针
void TaskServiceFuncHeart(void *p_arg)
{
 //   LH_ERR errorCode;
    p_arg = p_arg;
    static uint8_t count1s;
    static uint32_t count6min=0;
    OS_ERR err;
    CoreDelayMs(1000);
    
    while(1)
    {
//        CoreDelayMs(1000);
        if(count1s++>=100)
        {
            BoardLedToogle(BOARD_LED_GREEN);
            privatetestbywj();
            count1s = 0;
        }
        count6min++;
        if(count6min < 36000)
        {
            /*关闭抽液泵阀*/
            SystemOutputWrite(WASTE_LIQUID_PUMP,Bit_RESET);
            
            SystemOutputWrite(DRAINAGE_VALVE_V307,Bit_RESET);
            
        }
        else if(count6min < (36000+6000))
        {
            /*打开抽液泵阀*/
            SystemOutputWrite(DRAINAGE_VALVE_V307,Bit_SET);
            
            SystemOutputWrite(WASTE_LIQUID_PUMP,Bit_SET);
        }
        else
        {
            count6min = 0;
        }
        
        //按键扫描
        keyscan();
        //门扫描
        doorstatescan();

        OSTimeDlyHMSM(0,0,0,10,OS_OPT_TIME_HMSM_STRICT,&err);
    }
}


//按键状态扫描
void keyscan(void)
{
    BitAction keystate;
    static uint8_t keyflag1,keyflag2;
    
    SystemInputRead(KEY_CUPSTACK,&keystate);
    if(keystate == Bit_SET)keybuff[CUPSTACK_KEY]++;
    else{keybuff[CUPSTACK_KEY]=0;keyflag1 = 1;}

    SystemInputRead(KEY_REAGENTDISK,&keystate);
    if(keystate == Bit_SET)keybuff[REAGENTDISK_KEY]++;
    else{ keybuff[REAGENTDISK_KEY] = 0;keyflag2 = 1;}

    if((keybuff[CUPSTACK_KEY] >= 100)&&(keyflag1 == 1))
    {
        keyflag1 = 0;
        //触发新杯盏门按键
        TSC_KeyStateSendCommand(CUPSTACK_KEY);
    }

    if((keybuff[REAGENTDISK_KEY] >= 10)&&(keyflag2 == 1))
    {
        keyflag2 = 0;
        //触发试剂盘按键
        TSC_KeyStateSendCommand(REAGENTDISK_KEY);
    }

}
/**
 * 私人测试函数正常测试时请屏蔽
*/
void privatetestbywj(void)
{
#if 0
    ONE_OF_CUPSTATUS OneOfCup;
    OneOfCup.cupholder = 1;
    OneOfCup.cups = 1;
    static REAGENT_CUP_STATUS *cupstatus = NULL;
    cupstatus = TSC_NeedlReagentBeadReadCUPStatus(&OneOfCup);
//    TSC_NeedleReagentBeadRotateSpecialPositionWhileAck(TSC_NEEDLE_REAGENT_BEAD_POS_R1);
//    TSC_NeedleReagentBeadAbsorbWhileAck(100,OneOfCup);
//    TSC_NeedleReagentBeadRotateSpecialPositionWhileAck(TSC_NEEDLE_REAGENT_BEAD_POS_CLEAN);
//    TSC_NeedleReagentBeadInjectWhileAck(0);
#endif
}