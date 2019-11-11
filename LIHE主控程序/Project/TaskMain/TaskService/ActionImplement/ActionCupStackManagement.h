#ifndef __ACTION_CUP_STACK_MANAGEMENT_H_
#define __ACTION_CUP_STACK_MANAGEMENT_H_
#include "ActionCommonBase.h"

//输出门磁
#define OUTPUT_CUP_MANAGEMENT_MAGENT_DOOR      SYSTEM_OUTPUT_MAIN_BOARD_ARM_OUT17_PG7
//绿灯
#define OUTPUT_CUP_MANAGEMENT_LED_GREEN        SYSTEM_OUTPUT_MAIN_BOARD_ARM_OUT1_PG15  
//黄灯
#define OUTPUT_CUP_MANAGEMENT_LED_YELLOW       SYSTEM_OUTPUT_MAIN_BOARD_ARM_OUT2_PB3 
//门磁状态输入
#define INPUT_CUP_MANAGEMENT_MAGENT_DOOR       SYSTEM_INPUT_MAIN_BOARD_ARM_SEN19_PI7

//新杯装载机构初始化,返回新杯装载机构顶部缓存区是否有杯和新杯盘数量
//初始化时,新杯装载机构顶部缓存区不能有杯,如果有,将报错
LH_ERR ActionCupStackManagement_NewStackInit(ACTION_SENSOR_STATUS* newStackTopHasDisk,uint8_t* newStackDiskCount);

//空杯装载结构初始化
LH_ERR ActionCupStackManagement_EmptytackInit(ACTION_SENSOR_STATUS* emptyStackPushOnPosDiskExist,uint8_t* emptyStackDiskCount);

//推杆初始化
LH_ERR ActionCupStackManagement_PushInit(void);

//夹手初始化
LH_ERR ActionCupStackManagement_ClampInit(void);

//夹手打开
LH_ERR ActionCupStackManagement_ClampOpen(void);

//夹手关闭
LH_ERR ActionCupStackManagement_ClampClose(void);

//新杯装载机构上传一个新杯盘
LH_ERR ActionCupStackManagement_NewStackUpload(ACTION_SENSOR_STATUS* newStackTopHasDisk,uint8_t* newStackDiskCount);

//推杆推一个杯盘到位
LH_ERR ActionCupStackManagement_PushOneDisk(ACTION_SENSOR_STATUS* emptyStackPushOnPosDiskExist);

//空杯装载机构上升承载空杯盘
LH_ERR ActionCupStackManagement_EmptyStackAscend(ACTION_SENSOR_STATUS* emptyStackPushOnPosDiskExist,uint8_t* emptyStackDiskCount);

//杯栈管理区域垃圾桶锁住,指定垃圾桶上升,另一个垃圾桶下降
LH_ERR ActionCupStackManagement_GarbageLock(ACTION_PARAM_STACK_MANAGE_GARBAGE_NO garbageNo);

//杯栈管理机构指令垃圾桶下降,不管上升的是哪个
LH_ERR ActionCupStackManagement_GarbageUnlock(ACTION_PARAM_STACK_MANAGE_GARBAGE_NO garbageNo);

//更新杯栈管理机构的当前传感器状态
LH_ERR ActionCupStackManagement_GarbageReflushSensor(void);
//控制门开关
LH_ERR ActionCupStackManagement_DoorControl(MACHINE_DOOR_CMD doorcmd);

void doorstatescan(void);

#endif
