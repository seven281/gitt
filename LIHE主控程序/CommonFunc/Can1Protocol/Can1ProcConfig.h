#ifndef __CAN1_PROC_CONFIG_H_
#define __CAN1_PROC_CONFIG_H_
#include "BoardInc.h"



//void Can1DispatchReceive(CAN1_CMD_RECV* can1CommandRecvPtr);
//CAN1解析使用CCM内存

//配置本板卡的模块号和板号



//单包最大值
#define CAN1_SINGLE_PACK_MAX_LENGTH     2048



/**********************************系统CAN指令宏定义*****************************************/
/*****************************透传****************************/
//电机复位透传
#define CAN1_COMMAND_SM_RESET                           0X700

//电机走位透传
#define CAN1_COMMAND_SM_RUN                             0X701

//IO口读取透传
#define CAN1_COMMAND_IO_READ                            0X702

//IO口写入透传
#define CAN1_COMMAND_IO_WRITE                           0X703

//参数读取透传
#define CAN1_COMMAND_PARAM_READ                         0X704

//参数写入透传
#define CAN1_COMMAND_PARAM_WRITE                        0X705

/*****************************清洗盘指令**********************/
//清洗盘复位
#define CAN1_COMMAND_WASHDISH_RESET                     0x70A
//清洗盘旋转N个杯位
#define CAN1_COMMAND_WASHDISH_MOVE_SPECIALCUP           0x70B
//清洗盘旋转到指定杯位
#define CAN1_COMMAND_WASHDISH_MOVE_TO_SPECIALCUP        0x70C
/*****************************清洗针*************************/
//清洗针与泵复位
#define CAN1_COMMAND_NEEDLEWASH_PUMP_RESET              0X710
//清洗针灌注
#define CAN1_COMMAND_NEEDLEWASH_PRIME                   0X711
//清洗针磁珠清洗并返回
#define CAN1_COMMAND_NEEDLEWASH_CLEAN                   0X712
/*****************************A液注液*************************/
//A液注液
#define CAN1_COMMAND_LIQUID_A_INJECT                    0X71A
//A液灌注
#define CAN1_COMMAND_LIQUID_A_INJECT_PRIME              0X71B
//A液泵初始化
#define CAN1_COMMAND_LIQUID_A_INJECT_RESET              0X71C    
/*****************************A液混匀************************/
//A液混匀初始化
#define CAN1_COMMAND_LIQUID_A_MIX_RESET                 0X720
//A液上升
#define CAN1_COMMAND_LIQUID_A_MIX_UP                    0X721
//A液下降
#define CAN1_COMMAND_LIQUID_A_MIX_DOWN                  0X722
//A液混匀指定时间
#define CAN1_COMMAND_LIQUID_A_MIX_TIME                  0X723
//A液自动混匀
#define CAN1_COMMAND_LIQUID_A_MIX_AUTO                  0X724
/*****************************测量室上门***********************/
//测量室初始化
#define CAN1_COMMAND_MEASUREROOM_UP_RESET               0X729
//测量室门全开
#define CAN1_COMMAND_MEASUREROOM_UP_FULL_OPEN           0X72A
//测量室门半开
#define CAN1_COMMAND_MEASUREROOM_UP_HALF_OPEN           0X72B
//测量室门开到灌注位
#define CAN1_COMMAND_MEASUREROOM_UP_PRIME_OPEN          0X72C
//测量室门关闭
#define CAN1_COMMAND_MEASUREROOM_UP_CLOSE               0X72D
/*****************************B液注液*************************/
//B液泵复位
#define CAN1_COMMAND_LIQUID_B_RESET                     0X730
//B液灌注
#define CAN1_COMMAND_LIQUID_B_PRIME                     0X731
//B液注液
#define CAN1_COMMAND_LIQUID_B_INJECT                    0X732
/*************************测量室侧窗**************************/
//测量室窗初始化
#define CAN1_COMMAND_MEASUREROOM_SIDE_RESET             0X73A
//测量室窗打开
#define CAN1_COMMAND_MEASUREROOM_SIDE_OPEN              0X73B
//测量室窗关闭
#define CAN1_COMMAND_MEASUREROOM_SIDE_CLOSE             0X73C
/*************************测光*******************************/
//暗计数读数
#define CAN1_COMMAND_DARK_COUNT                         0X740
//本底读数
#define CAN1_COMMAND_BASE_COUNT                         0X741
//测光值读数
#define CAN1_COMMAND_METER_COUNT                        0X742
/*************************试剂针*****************************/
//垂直初始化
#define CAN1_COMMAND_NEEDLE_REAGENT_VERTICAL_RESET      0X750
//水平初始化
#define CAN1_COMMAND_NEEDLE_REAGENT_LEVEL_RESET         0X751
//垂直水平初始化
#define CAN1_COMMAND_NEEDLE_REAGENT_RESET               0X752
//泵初始化
#define CAN1_COMMAND_NEEDLE_REAGENT_PUMP_RESET          0X753
//转到清洗位
#define CAN1_COMMAND_NEEDLE_REAGENT_ROTATE_WASH         0X754
//转到试剂盘外圈
#define CAN1_COMMAND_NEEDLE_REAGENT_ROTATE_OUTSIDE      0X755
//转到试剂盘中圈
#define CAN1_COMMAND_NEEDLE_REAGENT_ROTATE_MIDDLE       0X756
//转到试剂盘内圈
#define CAN1_COMMAND_NEEDLE_REAGENT_ROTATE_INSIDE       0X757
//转到反应盘
#define CAN1_COMMAND_NEEDLE_REAGENT_ROTATE_REACTION     0X758
//吸试剂
#define CAN1_COMMAND_NEEDLE_REAGENT_ABSORBENT           0X759
//排试剂
#define CAN1_COMMAND_NEEDLE_REAGENT_EXCRETION           0X75A
//清洗
#define CAN1_COMMAND_NEEDLE_REAGENT_WASH                0X75B
//灌注
#define CAN1_COMMAND_NEEDLE_REAGENT_PRIME               0X75C
/*************************试剂盘*****************************/
//试剂盘初始化
#define CAN1_COMMAND_CUP_REAGENT_RESET                  0X760
//试剂盘旋转到指定位置
#define CAN1_COMMAND_CUP_REAGENT_ROTATE_TO_SPECIFIED_PS 0X761
//试剂盘旋转N个杯位
#define CAN1_COMMAND_CUP_REAGENT_ROTATE_N_CUP_POS       0X762
/*************************试剂混匀***************************/
//试剂混匀初始化
#define CAN1_COMMAND_REAGENT_MIX_RESET                  0X76A        
//上升
#define CAN1_COMMAND_REAGENT_MIX_UP                     0X76B
//下降
#define CAN1_COMMAND_REAGENT_MIX_DOWN                   0X76C
//混匀
#define CAN1_COMMAND_REAGENT_MIX                        0X76D
/*************************磁珠混匀***************************/
//初始化
#define CAN1_COMMAND_BEAD_MIX_RESET                     0X770
//上升
#define CAN1_COMMAND_BEAD_MIX_UP                        0X771
//下降
#define CAN1_COMMAND_BEAD_MIX_DOWN                      0X772
//混匀
#define CAN1_COMMAND_BEAD_MIX                           0X773
/**************************反应盘****************************/
//初始化
#define CAN1_COMMAND_REACTION_DISH_RESET                0X77A
//旋转到指定杯位
#define CAN1_COMMAND_REACTION_DISH_ROTATE_TO_SPECIALPOS 0X77B
//旋转N个杯位
#define CAN1_COMMAND_REACTION_DISH_ROTAT_SPECIALPOS     0X77C
/**************************样本针****************************/    
//垂直初始化
#define CAN1_COMMAND_NEEDLE_SAMPLE_VERTICAL_RESET       0X780
//水平初始化
#define CAN1_COMMAND_NEEDLE_SAMPLE_LEVEL_RESET          0X781
//垂直水平初始化
#define CAN1_COMMAND_NEEDLE_SAMPLE_VERTICAL_LEVEL_INIT  0X782
//泵初始化
#define CAN1_COMMAND_NEEDLE_SAMPLE_PUMP_RESET           0X783
//转到清洗位上方
#define CAN1_COMMAND_NEEDLE_SAMPLE_WASH_POS             0X784
//转到强清洗位
#define CAN1_COMMAND_NEEDLE_SAMPLE_FORCE_WASH_POS       0X785
//转到吸样本位
#define CAN1_COMMAND_NEEDLE_SAMPLE_SAMPLING_POS         0X786
//转到样本架上
#define CAN1_COMMAND_NEEDLE_SAMPLE_SAMPLE_FRAME         0X787
//转到反应盘排样本位上
#define CAN1_COMMAND_NEEDLE_SAMPLE_PUSH_POS             0X788
//转到反应盘稀释样本位上
#define CAN1_COMMAND_NEEDLE_SAMPLE_DILUTION_POS         0X789
//吸样本
#define CAN1_COMMAND_NEEDLE_SAMPLE_ABSORB               0X78A
//排样本
#define CAN1_COMMAND_NEEDLE_SAMPLE_ROW                  0X78B
//清洗
#define CAN1_COMMAND_NEEDLE_SAMPLE_WASH                 0X78C
//强清洗
#define CAN1_COMMAND_NEEDLE_SAMPLE_FORCEWASH            0X78D
//灌注
#define CAN1_COMMAND_NEEDLE_SAMPLE_PRIME                0X78E
//强灌注
#define CAN1_COMMAND_NEEDLE_SAMPLE_FORCEPRIME           0X78F
/**************************机械手***************************/
//垂直复位    
#define CAN1_COMMAND_MANIPULATOR_VERTICAL_RESET         0X790
//水平左右复位
#define CAN1_COMMAND_MANIPULATOR_LEVEL_ABOUT_RESET      0X791
//水平前后复位
#define CAN1_COMMAND_MANIPULATOR_LEVEL_AROUND_RESET     0X792
//水平复位       
#define CAN1_COMMAND_MANIPULATOR_LEVEL_RESET            0X793
//垂直水平复位   
#define CAN1_COMMAND_MANIPULATOR_VERTICAL_LEVEL         0X794
//移到反应盘上方
#define CAN1_COMMAND_MANIPULATOR_MOVE_REACTION          0X795
//移到新杯装载上方
#define CAN1_COMMAND_MANIPULATOR_NEW_CUP                0X796
//移到垃圾桶上方
#define CAN1_COMMAND_MANIPULATOR_MOVE_TRASH             0X797
//移到清洗盘上方
#define CAN1_COMMAND_MANIPULATOR_CLEAN_TRAY             0X798
//移到测量室上方
#define CAN1_COMMAND_MANIPULATOR_MEASURE_ROOM           0X799
//向反应盘放杯   
#define CAN1_COMMAND_MANIPULATOR_REACTION_PUTCUP        0X79A
//向垃圾箱放杯   
#define CAN1_COMMAND_MANIPULATOR_TRASH_PUTCUP           0X79B
//从反应盘取杯   
#define CAN1_COMMAND_MANIPULATOR_REATION_CATCHCUP       0X79C
//从新杯区取杯  
#define CAN1_COMMAND_MANIPULATOR_NEW_CATCHCUP           0X79D
//向清洗盘放杯  
#define CAN1_COMMAND_MANIPULATOR_CLEAN_PUTCUP           0X79E
//从清洗盘取杯  
#define CAN1_COMMAND_MANIPULATOR_CLEAN_CATCHCUP         0X79F
//向测量室放杯  
#define CAN1_COMMAND_MANIPULATOR_MEASURE_PUTCUP         0X7A0
//从测量室取杯   
#define CAN1_COMMAND_MANIPULATOR_MEASURE_CATCHCUP       0X7A1
/**************************反应杯装载机构*********************/
//新杯栈初始化          
#define CAN1_COMMAND_FULL_CUP_STACK_RESET               0X7B0
//空架栈初始化              
#define CAN1_COMMAND_EMPTY_CUP_STACK_RESET              0X7B1
//推手初始化                
#define CAN1_COMMAND_PUSH_HAND_RESET                    0X7B2
//夹持机构初始化        
#define CAN1_COMMAND_GRIPPER_RESET                      0X7B3
//初始化全部                
#define CAN1_COMMAND_STACK_GRIPPER_PUSHHAND_RESET       0X7B4
//夹手打开                      
#define CAN1_COMMAND_GRIPPER_OPEN                       0X7B5
//夹手夹紧                  
#define CAN1_COMMAND_GRIPPER_CLOSE                      0X7B6
//长传一个新杯              
#define CAN1_COMMAND_FAR_PUSH_HAND_NEW_CUP              0X7B7
//推一次新杯                
#define CAN1_COMMAND_PUSHONCE_NEW_CUP                   0X7B8
//承接一次空杯              
#define CAN1_COMMAND_UNDERTAKE_NEW_CUP                  0X7B9
//垃圾桶锁定                
#define CAN1_COMMAND_TRASH_LOCK                         0X7BA
//垃圾桶解锁                
#define CAN1_COMMAND_TRASH_UNLOCK                       0X7BB
/************************扫码*******************************/
//扫码
#define CAN1_COMMAND_SWEEP_CODE                         0X7C0
/************************加样传送带**************************/
//初始化
#define CAN1_COMMAND_SAMPLE_CONVEYOR_RESET              0X7CA
//移到吸样位
#define CAN1_COMMAND_SAMPLE_CONVEYOR_MOVE_TO_SAMPLE     0X7CB
//退回样本架
#define CAN1_COMMAND_SAMPLE_CONVEYOR_RETURN_SAMPLE      0X7CC
/************************END********************************/
#endif





