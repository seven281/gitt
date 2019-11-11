/***********************************************************************
 *
 * (c) Copyright 2001-2016, XXXX, All Rights Reserved.
 * THIS IS UNPUBLISHED PROPRIETARY SOURCE CODE OF XXXX, INC.
 * The copyright notice above does not evidence any actual or intended
 * publication of such source code. 
 *
 *  Subsystem:   TestProc
 *  File:        TestProc_canCommand.h
 *  Author:      王殿光
 *  Description: Command for can bus.
 *********************************************************************/

/* Define to prevent recursive inclusion -------------------------------------*/
#ifndef __TESTPROC_CANCOMMAND_H__
#define __TESTPROC_CANCOMMAND_H__

#ifdef __cplusplus
 extern "C" {
#endif

#define CAN_CMD_MULTY_FRAME_SIZE                          (6u)
#define CAN_CMD_CMD_OFFSET                                (0u)
#define CAN_CMD_DEST_OFFSET                               (11u)
#define CAN_CMD_SRC_OFFSET                                (17u)
#define CAN_CMD_DST_CELL_OFFSET                           (23u)
#define CAN_CMD_SRC_CELL_OFFSET                           (26u)
#define CAN_CMD_COMMAND_MASK                              (0x7FF)
#define CAN_CMD_DST_BOARD_MASK                            (0x3F)
#define CAN_CMD_SRC_BOARD_MASK                            (0x3F)
#define CAN_CMD_DST_CELL_MASK                             (0x07)
#define CAN_CMD_SRC_CELL_MASK                             (0x07)

#define CAN_CMD_INVALID                                   (0x0800)   // 无效命令
#define CAN_CMD_MULTYHEAD                                 (0x07FD)   // 多包通信命令
#define CAN_CMD_MULTYPACKAGE                              (0x07FE)   // 多包通信命令
#define CAN_CMD_MULTYEND                                  (0x07FF)   // 多包通信命令

#define CAN_CMD_CELL_REACTSTOP                            (0x0001)   // 反应盘停止触发
#define CAN_CMD_CELL_EMSTOP                               (0x0002)   // 急停
#define CAN_CMD_CELL_WARNING                              (0x0003)   // 报警
#define CAN_CMD_CELL_RESET                                (0x0004)   // 复位
#define CAN_CMD_CELL_STATUS                               (0x0005)   // 状态
#define CAN_CMD_CELL_ADDSAMPLE                            (0x0006)   // 加样本
#define CAN_CMD_CELL_ADDREAGENT                           (0x0007)   // 加试剂
#define CAN_CMD_CELL_ADDMIX                               (0x0008)   // 加搅拌
#define CAN_CMD_CELL_ADDDILUTE                            (0x0009)   // 加稀释（吸->吐）
#define CAN_CMD_CELL_SIMULATE                             (0x000A)   // 机械动作检查开始
#define CAN_CMD_CELL_STOP                                 (0x000B)   // 正常停止
#define CAN_CMD_CELL_SAMPLEENABLE                         (0x000C)   // 通知样本针样本架已经进入测试区
#define CAN_CMD_CELL_SAMPLEPAUSE                          (0x000D)   // 加样暂停,一个参数,1 - 加样暂停；0 - 加样继续 
#define CAN_CMD_CELL_START_WORK                           (0x000E)   // 开始测试，1 – 正常测试；2-机械动作检查；3-光量检查；4 – 杯空白测试；5-清洗反应板
#define CAN_CMD_CELL_UPDATA_START                         (0x000F)   // 升级程序的信息
#define CAN_CMD_CELL_UPDATA_DATA                          (0x0010)   // 升级数据
#define CAN_CMD_CELL_UPDATA_END                           (0x0011)   // 升级结束,附带程序校验CRC
#define CAN_CMD_CELL_UPDATA_PREPARE                       (0x0012)   // 准备升级,跳转Boot等操作
#define CAN_CMD_CELL_INSERT_EMRACK                        (0x0013)   // 通知主控板/样本针插入急诊测试架
#define CAN_CMD_CELL_MULIT_REAGENT                        (0x0014)   // 试剂盘多试剂位信息
#define CAN_CMD_CELL_PUMP_AIROUT                          (0x0015)   // 注射泵排气
#define CAN_CMD_CELL_COOL_TEMP                            (0x0016)   // 制冷温度
#define CAN_CMD_CELL_REACT_TEMP                           (0x0017)   // 反应槽问题
#define CAN_CMD_CELL_DEBUG                                (0x0018)   // 机构板调试命令
#define CAN_CMD_CELL_AD_DEBUG                             (0x0019)   // AD板调试命令
#define CAN_CMD_CELL_REAGENTQUANTITY                      (0x001A)   // 试剂余量扫描
#define CAN_CMD_CELL_OUTPOS                               (0x001B)   // 样本架推送至待抓取位置，准备移出模块
#define CAN_CMD_CELL_REAGENTBARCODE                       (0x001C)   // 试剂条码扫描
#define CAN_CMD_CELL_VERSION                              (0x001D)   // 样本针解锁轨道样本架
#define CAN_CMD_CELL_BOOT_VERSION                         (0x001E)   // 读取Boot版本号
#define CAN_CMD_CELL_CONVEYORBELT_RESET                   (0x001F)   // 样本机构的传送带复位,即轨道小车送架位置
#define CAN_CMD_CELL_WAITTOTEST                           (0x0020)   // 样本架由待测区进入测试区
#define CAN_CMD_CELL_REAGENT_V                            (0x0021)   // 试剂针垂直检查
#define CAN_CMD_CELL_REAGENT_H                            (0x0022)   // 试剂针水平检查
#define CAN_CMD_CELL_WASTELIQUID_EXHAUST                  (0x0023)   // 浓废液管路排气
#define CAN_CMD_CELL_WASHLIQUID_EXHAUST                   (0x0024)   // 清洗液管路排气
#define CAN_CMD_CELL_WASH_WARMSINK                        (0x0025)   // 清洗恒温槽
#define CAN_CMD_CELL_WASH_SAMPLE                          (0x0026)   // 清洗样本针
#define CAN_CMD_CELL_POLLUTE_REAGENT                      (0x0027)   // 试剂针交叉污染
#define CAN_CMD_CELL_POLLUTE_SAMPLE                       (0x0028)   // 样本针交叉污染
#define CAN_CMD_CELL_POLLUTE_CUP                          (0x0029)   // 反应杯交叉污染
#define CAN_CMD_CELL_SAMPLE_ACCURACY                      (0x002A)   // 样本针精度测试
#define CAN_CMD_CELL_REAGENT_ACCURACY                     (0x002B)   // 试剂针精度测试
#define CAN_CMD_CELL_WATER_TANK_OK                        (0x002C)   // 水箱上水完成
#define CAN_CMD_CELL_REAGENT_ADD_ANTIBIOSIS               (0x002D)   // 试剂针加抗菌无磷
#define CAN_CMD_CELL_AC_STATUS                            (0x0030)   // 分机交流状态，1 – 水箱上水, 2 – 水槽排水, 3 – 水箱预温, 4 – 水槽上水, 5 – 水槽预温, 6 – 分机交流准备就绪
        // 主控板 <==  ==> 中位机
#define CAN_CMD_COMM_RACKIN                               (0x00A0)   // 样本架进入待测位置
#define CAN_CMD_COMM_RACKCHANGE                           (0x00A1)   // 主控板通知中位机切换样本架
#define CAN_CMD_COMM_GET_ITEM                             (0x00A2)   // 请求数据,发送带样本架号参数,返回值:0有数据；1没有数据；2样本架号错误
#define CAN_CMD_COMM_TESTDATA                             (0x00A3)   // 发送测试数据到中位机
#define CAN_CMD_COMM_TESTITEM                             (0x00A4)   // 中位机通知主控板，样本架可以加样了(附带测试数据)
#define CAN_CMD_COMM_ONOFF_LINE                           (0x00A5)   // 中位机与上位机联机，1 - 联机成功；0 - 脱机成功
#define CAN_CMD_COMM_RACK_PICKUP                          (0x00A6)   // 轨道机械手已经抓起样本架
#define CAN_CMD_COMM_AD_COLLECT_DATA                      (0x00A7)   // 通采数据
#define CAN_CMD_COMM_RECYCLE_RACK                         (0x00A8)   // 0-测试位置样本架;1-待测位置样本架
#define CAN_CMD_COMM_TEST_PARAM                           (0x00A9)   // 测试参数
#define CAN_CMD_COMM_MANUAL_TEST                          (0x00AA)   // 手动测试
#define CAN_CMD_COMM_PREPARE_TEST                         (0x00AB)   // 准备手动测试
#define CAN_CMD_COMM_CUPSTACK_POS                         (0x00AC)   // 更新杯栈位置

#define CAN_CMD_CELL_DEBUG_START                          (0x0700)   // 0x701以后的命令都是Debug命令




    //配置本板卡的模块号和板号
    
    //广播目标板号
#define CAN1_BOARDCAST_BOARD_ID         BOARD_ID_BOARDCAST
    
    //单包最大值
#define CAN1_SINGLE_PACK_MAX_LENGTH     2048
    
    //长包指令起始包
#define CAN1_COMMAND_LONG_PACK_START    CAN_CMD_MULTYHEAD  
    
    //长包指令数据包
#define CAN1_COMMAND_LONG_PACK_DATA     CAN_CMD_MULTYPACKAGE
    
    //长包指令结束包
#define CAN1_COMMAND_LONG_PACK_END      CAN_CMD_MULTYEND
    
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


#ifdef __cplusplus
}
#endif

#endif /* __TESTPROC_CANCOMMAND_H__ */

/************************ (C) COPYRIGHT STMicroelectronics *****END OF FILE****/
