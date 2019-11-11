#ifndef __ACTION_NEEDLE_SAMPLE_H_
#define __ACTION_NEEDLE_SAMPLE_H_
#include "ActionCommonBase.h"

/*复合08指令 样本针 0x0e OK*/
#define NEEDLE_SAMPLE_SUCCESS                                   0X0000
#define NEEDLE_SAMPLE_ERR_UNDEFINE                              0X0100
#define NEEDLE_SAMPLE_ERR_UPDOWN_RESET                          0X0200
#define NEEDLE_SAMPLE_ERR_UPDOWN_RETURN_ZERO                    0X0300
#define NEEDLE_SAMPLE_ERR_UPDOWN_RUNNING                        0X0400
#define NEEDLE_SAMPLE_ERR_ROTATE_RESET                          0X0500
#define NEEDLE_SAMPLE_ERR_ROTATE_RETURN_ZERO                    0X0600
#define NEEDLE_SAMPLE_ERR_ROTATE_RUNNING                        0X0700
#define NEEDLE_SAMPLE_ERR_UPDOWN_SCRAM                          0X0800
#define NEEDLE_SAMPLE_ERR_LIQUID_DETECT                         0X0900
#define NEEDLE_SAMPLE_ERR_PUMP_RESET                            0X0A00
#define NEEDLE_SAMPLE_ERR_PUMP_RUNNING                          0X0B00
#define NEEDLE_SAMPLE_ERR_PUMP_RETURN_ZERO                      0X0C00
#define NEEDLE_SAMPLE_ERR_ABSORB_ZERO                           0X0D00
#define NEEDLE_SAMPLE_ERR_INJECT_ZERO                           0X0E00
#define NEEDLE_SAMPLE_ERR_DP601_OPEN                            0X0F00
#define NEEDLE_SAMPLE_ERR_DP601_CLOSE                           0X1000
#define NEEDLE_SAMPLE_ERR_V221_OPEN                             0X1100
#define NEEDLE_SAMPLE_ERR_V221_CLOSE                            0X1200
#define NEEDLE_SAMPLE_ERR_V218_OPEN                             0X1300
#define NEEDLE_SAMPLE_ERR_V218_CLOSE                            0X1400
#define NEEDLE_SAMPLE_ERR_DP305_OPEN                            0X1500
#define NEEDLE_SAMPLE_ERR_DP305_CLOSE                           0X1600
#define NEEDLE_SAMPLE_ERR_DP307_OPEN                            0X1700
#define NEEDLE_SAMPLE_ERR_DP307_CLOSE                           0X1800
#define NEEDLE_SAMPLE_ERR_V215_OPEN                             0X1900
#define NEEDLE_SAMPLE_ERR_V215_CLOSE                            0X1A00
#define NEEDLE_SAMPLE_ERR_DP304_OPEN                            0X1B00
#define NEEDLE_SAMPLE_ERR_DP304_CLOSE                           0X1C00
#define NEEDLE_SAMPLE_ERR_ROTATE_POS                            0X1D00
#define NEEDLE_SAMPLE_ERR_LIQUID_DETECT_ADJUST_LOW              0X1E00//样本针探液校准电压升不上去
#define NEEDLE_SAMPLE_ERR_LIQUID_DETECT_ADJUST_HIGH             0X1F00//样本针探液校准电压降不下来
#define NEEDLE_SAMPLE_ERR_LIQUID_DETECT_ADJUST_UNKNOW           0X2000//样本针探液校准失败,找不到合适参数
#define NEEDLE_SAMPLE_ERR_LIQUID_DETECT_CHECK_NOT_EXIST         0X2100//样本针液面探测版通讯失败
#define NEEDLE_SAMPLE_ERR_LIQUID_DETECT_ENABLE_FAIL             0X2200//样本针打开液面探测,针尖电压一直降不下去
#define NEEDLE_SAMPLE_ERR_LIQUID_DETECT_UNKNOW                  0x2300//样本针液面探测相关的未知错误
#define NEEDLE_SAMPLE_ERR_V309_OPEN                            0X2400
#define NEEDLE_SAMPLE_ERR_V309_CLOSE                           0X2500

//样本针初始化
LH_ERR ActionNeedleSample_Init(TSC_NEEDLE_SAMPLE_POS* needleRotatePos);

//样本针旋转到指定位置
LH_ERR ActionNeedleSample_RotateSpecialPos(TSC_NEEDLE_SAMPLE_POS targetPos,TSC_NEEDLE_SAMPLE_POS* needleRotatePos);

//样本针吸样本
LH_ERR ActionNeedleSample_AbsorbLiquid(uint16_t liquidUL,TSC_NEEDLE_SAMPLE_POS* needleRotatePos,SAMPLE_STATUS);

//样本针排样本
LH_ERR ActionNeedleSample_InjectLiquid(uint16_t liquidUL,TSC_NEEDLE_SAMPLE_POS* needleRotatePos);

//样本针清洗
LH_ERR ActionNeedleSample_Clean(TSC_NEEDLE_SAMPLE_CLEAN_OPT opt,TSC_NEEDLE_SAMPLE_POS overTargetPos,TSC_NEEDLE_SAMPLE_POS* needleRotatePos);

//样本针强力清洗
LH_ERR ActionNeedleSample_ForceClean(TSC_NEEDLE_SAMPLE_POS* needleRotatePos);

//样本针灌注
LH_ERR ActionNeedleSample_Prime(TSC_NEEDLE_SAMPLE_PRIME_OPT opt,TSC_NEEDLE_SAMPLE_POS* needleRotatePos);

//样本针强力灌注
LH_ERR ActionNeedleSample_ForcePrime(TSC_NEEDLE_SAMPLE_POS* needleRotatePos);

#define NEEDLE_SAMPLE_UPDOWN_FAST  SM_CURVE_0

#define NEEDLE_SAMPLE_UPDOWN_SLOW  SM_CURVE_1



#endif
