#ifndef __TSC_CUP_STACK_MANAGEMENT_H_
#define __TSC_CUP_STACK_MANAGEMENT_H_
#include "TSC_Base.h"

//定义杯栈管理的指令列表宏
typedef enum TSC_STACK_MANAGE_CMD_INDEX
{
    TSC_STACK_MANAGE_INIT = 0,//杯栈管理初始化
    TSC_STACK_MANAGE_UPLOAD_NEW_DISK = 1,//上传一个新杯盘
    TSC_STACK_MANAGE_PUSH_NEW_DISK = 2,//推一个新杯盘
    TSC_STACK_MANAGE_ASCEND_EMPTY_DISK = 3,//上去承载一个就杯盘
    TSC_STACK_MANAGE_GARBAGE_LOCK = 4,//垃圾桶上锁
    TSC_STACK_MANAGE_GARBAGE_UNLOCK = 5,//垃圾桶解锁
    TSC_STACK_MANAGE_DOOR_CONTROL = 6,//新杯盏门开关
}TSC_STACK_MANAGE_CMD_INDEX;

#define DOOR_STATE  SYSTEM_INPUT_MAIN_BOARD_ARM_SEN19_PI7
/*******************************************************杯栈管理动作*************************************************************/
//查询上一次指令执行的状态,每次发送指令之后必须查询到指令完成TSC_CMD_STATE_COMPLETE
//才能继续发送下一个指令,否则以后的指令都发不出去了
TSC_CMD_STATE TSC_StackManageGetLastCommandStatusAndResult(LH_ERR* errCode);

//初始化杯栈管理数据结构
void TSC_StackManageResultDataInit(void);

//杯栈管理模块初始化
LH_ERR TSC_StackManageInitWhileAck(void);

//新杯盘承载机构上传一个新盘到新盘装载平台
LH_ERR TSC_StackManageUploadNewDisk2NewDiskPlatformWhileAck(void);

//推手前推,将一个新盘从新盘装载平台推送到测试平台
LH_ERR TSC_StackManagePushNewDisk2WaitTestPlatformWhileAck(void);

//空盘装载机构抬起到承载位置,夹手自动松开将已经测试完成的空盘放到空盘装载机构上,然后空盘装载机构回到零点
LH_ERR TSC_StackManageAscendingEmptyDiskFromWaitTestPlatformWhileAck(void);

//杯盘管理机构的选定的垃圾桶锁定,也就是指定的垃圾桶抬起来,这样可以在测试时增加新盘和去除空盘
//抬起的垃圾桶将留在机器内部,未抬起的垃圾桶可以被清理
LH_ERR TSC_StackManageLockGarbageWhileAck(ACTION_PARAM_STACK_MANAGE_GARBAGE_NO garbageNo);

//杯盘管理机构的选定的垃圾桶解锁,也就是指定的垃圾桶降下去
LH_ERR TSC_StackManageUnlockGarbageWhileAck(ACTION_PARAM_STACK_MANAGE_GARBAGE_NO garbageNo);



/*******************************************************杯栈管理动作*************************************************************/

/*********************************************动作结果中的数据读取***************************************************/
//后续会添加一些查询状态的指令,例如杯栈当前有效存储数量 有无等状态
//新盘装载平台是否有盘
ACTION_SENSOR_STATUS TSC_StackManageCheckNewDiskPlatformHasDisk(void);

//待测盘平台是否有盘
ACTION_SENSOR_STATUS TSC_StackManageCheckWaitTestDiskPlatformHasDisk(void);

//垃圾桶1是否存在
ACTION_SENSOR_STATUS TSC_StackManageCheckGarbage1(void);

//垃圾桶2是否存在
ACTION_SENSOR_STATUS TSC_StackManageCheckGarbage2(void);

//检查新盘装载机构上有几个盘
uint8_t TSC_StackManageCheckNewDiskCount(void);

//检查空盘装载机构上有几个盘
uint8_t TSC_StackManageChecEmptyDiskCount(void);

//检查杯栈管理的门状态
MACHINE_DOOR_STATUS TSC_StackManageCheckDoorStatus(void);

//控制电磁门
LH_ERR TSC_StackManageDoorControlWhileAck(MACHINE_DOOR_CMD doorcmd);

LH_ERR TSC_DoorStateSendCommand(DOOR_STATE_OPT state);

LH_ERR TSC_KeyStateSendCommand(KEY_OPT keyopt);
#endif





