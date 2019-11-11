#ifndef __SYSTEM_PARAM_H_
#define __SYSTEM_PARAM_H_
#include "BoardInc.h"


#define SYSTEM_ACTION_PARAM_BASE_ADDR           0X00000000

/***********************杯栈管理区域配置******************************************/
typedef struct SYSTEM_ACTION_PARAM_CUP_MANAGE
{
    int32_t newStackResetCorrection;//新杯栈复位修正
    int32_t emptyStackResetCorrection;//空杯栈复位修正
    int32_t pushResetCorrection;//推杆复位修正
    int32_t clampResetCorrection;//夹手复位修正
    int32_t newStackFullToSensorOffset;//新杯栈放满时候,顶端到检测传感器的偏移
    int32_t newStackOneStackOffset;//新杯栈上升一个杯位的偏移
    int32_t newStackFullToPlatformOffset;//新杯栈满时候,将顶端杯栈放到平台上的偏移
    int32_t emptyStackFullToSensorOffset;//空杯栈满时,顶端杯盘到检测传感器的偏移
    int32_t emptyStackOneStackOffset;//空杯栈上升一个杯位的偏移
    int32_t emptyStackFullToTopOffset;//空杯栈满时,上升到顶端承接旧杯盘的偏移
    int32_t pushFirstPushPosition;//推手第一次推的坐标
    int32_t pushSecondPushOffset;//推手第二次推紧的偏移
    int32_t clampClosePos;//夹手合上坐标
    int32_t clampOpenPos;//夹手打开坐标
    int32_t newStackReseve[5];//新杯栈保留配置
    int32_t emptyStackReseve[5];//空杯栈保留配置
    int32_t pushReseve[5];//推杆保留配置
    int32_t clampReseve[5];//夹手保留配置
}SYSTEM_ACTION_PARAM_CUP_MANAGE;    
typedef enum SYSTEM_ACTION_PARAM_CUP_MANAGE_SUBINDEX
{
    NEWSTACKRESETCORRECTION=0,//新杯栈复位修正
    EMPTYSTACKRESETCORRECTION,//空杯栈复位修正
    PUSHRESETCORRECTION,//推杆复位修正
    CLAMPRESETCORRECTION,//夹手复位修正
    NEWSTACKFULLTOSENSOROFFSET,//新杯栈放满时候,顶端到检测传感器的偏移
    NEWSTACKONESTACKOFFSET,//新杯栈上升一个杯位的偏移
    NEWSTACKFULLTOPLATFORMOFFSET,//新杯栈满时候,将顶端杯栈放到平台上的偏移
    EMPTYSTACKFULLTOSENSOROFFSET,//空杯栈满时,顶端杯盘到检测传感器的偏移
    EMPTYSTACKONESTACKOFFSET,//空杯栈上升一个杯位的偏移
    EMPTYSTACKFULLTOTOPOFFSET,//空杯栈满时,上升到顶端承接旧杯盘的偏移
    PUSHFIRSTPUSHPOSITION,//推手第一次推的坐标
    PUSHSECONDPUSHOFFSET,//推手第二次推紧的偏移
    CLAMPCLOSEPOS,//夹手合上坐标
    CLAMPOPENPOS,//夹手打开坐标
}SYSTEM_ACTION_PARAM_CUP_MANAGE_SUBINDEX;



/********************************抓杯机械手配置参数*****************************/
typedef struct SYSTEM_ACTION_PARAM_HAND_CUP
{
    int32_t frontBackResetCorrection;//前后复位修正坐标
    int32_t leftRightResetCorrection;//左右复位修正坐标
    int32_t upDownResetCorrection;//上下复位修正
    int32_t leftRightEmptyCupStartPos;//左右空杯第一个起始位
    int32_t leftRightEmptyCupOneOffset;//左右空杯移动一个位置的偏移
    int32_t leftRightDishReactionPos;//左右移动到反应盘坐标
    int32_t leftRightDishCleanPos;//左右移动到清洗盘坐标
    int32_t leftRightMeasureRoomPos;//左右移动到测量室坐标
    int32_t leftRightGarbage1Pos;//左右移动到垃圾桶1坐标
    int32_t leftRightGarbage2Pos;//左右移动到垃圾桶2坐标
    int32_t frontBackEmptyCupStartPos;//前后空杯第一个起始位
    int32_t frontBackEmptyCupOneOffset;//前后移动一个空杯坐标
    int32_t frontBackDishReactionPos;//前后移动到反应盘坐标
    int32_t frontBackDishCleanPos;//前后移动到清洗盘坐标
    int32_t frontBackMeasureRoomPos;//前后移动到测量室坐标
    int32_t frontBackGarbage1Pos;//前后移动到垃圾桶1坐标
    int32_t frontBackGarbage2Pos;//前后移动到垃圾桶2坐标
    int32_t upDownEmptyCupCatchPos;//上下空杯抓杯坐标
    int32_t upDownDishReactionCatchPos;//上下反应盘抓杯坐标
    int32_t upDownDishReactionPutPos;//上下反应盘放杯坐标
    int32_t upDownDishCleanCatchPos;//上下清洗盘抓杯坐标
    int32_t upDownDishCleanPutPos;//上下清洗盘放杯坐标
    int32_t upDownMeasureRoomCatchPos;//上下测量室抓杯坐标
    int32_t upDownMeasureRoomPutPos;//上下测量室放杯坐标
    int32_t upDownGarbage1PutPos;//上下垃圾桶1放杯坐标
    int32_t upDownGarbage2PutPos;//上下垃圾桶2放杯坐标
    int32_t leftRightReseve[5];//保留坐标
    int32_t frontBackReseve[5];//保留坐标
    int32_t upDownReseve[5];//保留坐标
}SYSTEM_ACTION_PARAM_HAND_CUP;

typedef enum SYSTEM_ACTION_PARAM_HAND_CUP_SUBINDEX
{
    HANDFRONTBACKRESETCORRECTION=0,//前后复位修正坐标
    HANDLEFTRIGHTRESETCORRECTION,//左右复位修正坐标
    HANDUPDOWNRESETCORRECTION,//上下复位修正
    HANDLEFTRIGHTEMPTYCUPSTARTPOS,//左右空杯第一个起始位
    HANDLEFTRIGHTEMPTYCUPONEOFFSET,//左右空杯移动一个位置的偏移
    HANDLEFTRIGHTDISHREACTIONPOS,//左右移动到反应盘坐标
    HANDLEFTRIGHTDISHCLEANPOS,//左右移动到清洗盘坐标
    HANDLEFTRIGHTMEASUREROOMPOS,//左右移动到测量室坐标
    HANDLEFTRIGHTGARBAGE1POS,//左右移动到垃圾桶1坐标
    HANDLEFTRIGHTGARBAGE2POS,//左右移动到垃圾桶2坐标
    HANDFRONTBACKEMPTYCUPSTARTPOS,//前后空杯第一个起始位
    HANDFRONTBACKEMPTYCUPONEOFFSET,//前后移动一个空杯坐标
    HANDFRONTBACKDISHREACTIONPOS,//前后移动到反应盘坐标
    HANDFRONTBACKDISHCLEANPOS,//前后移动到清洗盘坐标
    HANDFRONTBACKMEASUREROOMPOS,//前后移动到测量室坐标
    HANDFRONTBACKGARBAGE1POS,//前后移动到垃圾桶1坐标
    HANDFRONTBACKGARBAGE2POS,//前后移动到垃圾桶2坐标
    HANDUPDOWNEMPTYCUPCATCHPOS,//上下空杯抓杯坐标
    HANDUPDOWNDISHREACTIONCATCHPOS,//上下反应盘抓杯坐标
    HANDUPDOWNDISHREACTIONPUTPOS,//上下反应盘放杯坐标
    HANDUPDOWNDISHCLEANCATCHPOS,//上下清洗盘抓杯坐标
    HANDUPDOWNDISHCLEANPUTPOS,//上下清洗盘放杯坐标
    HANDUPDOWNMEASUREROOMCATCHPOS,//上下测量室抓杯坐标
    HANDUPDOWNMEASUREROOMPUTPOS,//上下测量室放杯坐标
    HANDUPDOWNGARBAGE1PUTPOS,//上下垃圾桶1放杯坐标
    HANDUPDOWNGARBAGE2PUTPOS,//上下垃圾桶2放杯坐标
}SYSTEM_ACTION_PARAM_HAND_CUP_SUBINDEX;

/********************************反应盘配置参数*******************************/
typedef struct SYSTEM_ACTION_PARAM_REACTION
{
    int32_t resetCorrection;//复位后位置
    int32_t oneCupOffset;//旋转一隔的偏移
    int32_t reseve[5];//保留参数
}SYSTEM_ACTION_PARAM_REACTION;

typedef enum SYSTEM_ACTION_PARAM_REACTION_SUBINDEX
{
    RESETCORRECTION=0,//复位后位置
    ONECUPOFFSET,//旋转一隔的偏移
}SYSTEM_ACTION_PARAM_REACTION_SUBINDEX;
/**************************试管架传送带参数**************************************/
typedef struct SYSTEM_ACTION_PARAM_PATHWAY
{
    int32_t resetCorrection;//复位修正
    int32_t oneTestTubeOffset;//移动一格试管偏移
    int32_t rankOverOffset;//两个架子自检偏移量
    int32_t reseve[5];
}SYSTEM_ACTION_PARAM_PATHWAY;

typedef enum SYSTEM_ACTION_PARAM_PATHWAY_SUBINDEX
{
    PATHWAYRESETCORRECTION=0,//复位修正
    PATHWAYONETESTTUBEOFFSET,//移动一格试管偏移
    PATHWAYRANKOVEROFFSET,//两个架子自检偏移量
}SYSTEM_ACTION_PARAM_PATHWAY_SUBINDEX;
/****************************样本针参数***************************************/
typedef struct SYSTEM_ACTION_PARAM_NEEDLE_SAMPLE
{
    int32_t upDownResetCorrection;//样本针升降复位修正
    int32_t rotateResetCorrection;//样本针旋转复位修正
    int32_t upDownCleanPos;//样本针升降清洗位坐标
    int32_t upDownForceCleanPos;//样本针升降强洗位坐标
    int32_t upDownTestTubeAbsorbMaxPos;//样本针升降试管架吸样极限坐标
    int32_t upDownReactionAbsorbMaxPos;//样本针升降反应盘吸样坐标
    int32_t upDownReactionInjectMaxPos;//样本针升降反应盘排样坐标
    int32_t rotateCleanPos;//样本针旋转清洗位坐标
    int32_t rotateForceCleanPos;//样本针旋转强洗位坐标
    int32_t rotateTestTubeAbsorbPos;//样本针旋转试管架吸样坐标
    int32_t rotateReactionAbsorbPos;//样本针旋转反应盘吸样坐标
    int32_t rotateReactionInjectPos;//样本针旋转反应盘排样坐标
    int32_t timeNeedleCleanMs;//样本针清洗时间,毫秒
    int32_t timeNeedleForceCleanMs;//样本针强清洗时间,毫秒
    int32_t timeNeedlePrimeMs;//样本针灌注时间,毫秒
    int32_t timeNeedleForcePrimeMs;//样本针强灌注时间,毫秒
    int32_t pumpOneUlStep;//样本针注射泵1UL对应电机步数
    int32_t pumpAbsortWithAirUl;//样本针注射泵吸液吸空气量第一次
    int32_t pumpAbsortWithAirU2;//样本针注射泵吸液吸空气量第二次
    int32_t pumpretainWithAir;  //样本针排液保留量
    int32_t upDownTestTubeAbsorbStartPos;//样本针升降试管架吸样开始坐标
    int32_t upDownReseve[2];//保留
    int32_t rotateReseve[5];//保留
}SYSTEM_ACTION_PARAM_NEEDLE_SAMPLE;

typedef enum SYSTEM_ACTION_PARAM_NEEDLE_SAMPLE_SUBINDEX
{
    SAMPLEUPDOWNRESETCORRECTION=0,//样本针升降复位修正
    SAMPLEROTATERESETCORRECTION,//样本针旋转复位修正
    SAMPLEUPDOWNCLEANPOS,//样本针升降清洗位坐标
    SAMPLEUPDOWNFORCECLEANPOS,//样本针升降强洗位坐标
    SAMPLEUPDOWNTESTTUBEABSORBMAXPOS,//样本针升降试管架吸样极限坐标
    SAMPLEUPDOWNREACTIONABSORBMAXPOS,//样本针升降反应盘吸样坐标
    SAMPLEUPDOWNREACTIONINJECTMAXPOS,//样本针升降反应盘排样坐标
    SAMPLEROTATECLEANPOS,//样本针旋转清洗位坐标
    SAMPLEROTATEFORCECLEANPOS,//样本针旋转强洗位坐标
    SAMPLEROTATETESTTUBEABSORBPOS,//样本针旋转试管架吸样坐标
    SAMPLEROTATEREACTIONABSORBPOS,//样本针旋转反应盘吸样坐标
    SAMPLEROTATEREACTIONINJECTPOS,//样本针旋转反应盘排样坐标
    SAMPLETIMENEEDLECLEANMS,//样本针清洗时间,毫秒
    SAMPLETIMENEEDLEFORCECLEANMS,//样本针强清洗时间,毫秒
    SAMPLETIMENEEDLEPRIMEMS,//样本针灌注时间,毫秒
    SAMPLETIMENEEDLEFORCEPRIMEMS,//样本针强灌注时间,毫秒
    SAMPLEPUMPONEULSTEP,//样本针注射泵1UL对应电机步数
    SAMPLEPUMPABSORTWITHAIRUL,//样本针注射泵吸液吸空气量第一次
    SAMPLEPUMPABSORTWITHAIRU2,//样本针注射泵吸液吸空气量第二次
    SAMPLEPUMPRETAINWITHAIR,  //样本针排液保留量
    SAMPLEUPDOWNTESTTUBEABSORBSTARTPOS,//样本针升降试管架吸样开始坐标
}SYSTEM_ACTION_PARAM_NEEDLE_SAMPLE_SUBINDEX;

/*********************************磁珠试剂针参数***********************/
typedef struct SYSTEM_ACTION_PARAM_NEEDLE_REAGENT_BRAD
{
    int32_t upDownResetCorrection;//试剂磁珠针升降复位修正
    int32_t rotateResetCorrection;//试剂磁珠针旋转复位修正
    int32_t upDownCleanPos;//试剂磁珠针升降清洗位坐标
    int32_t upDownReactionInjectPos;//试剂磁珠针升降反应盘排液坐标
    int32_t upDownReagent1AbsorbPos;//试剂磁珠针升降吸R1极限坐标
    int32_t upDownReagent2AbsorbPos;//试剂磁珠针升降吸R2极限坐标
    int32_t upDownBeadAbsorbPos;//试剂磁珠针升降吸磁珠极限坐标
    int32_t rotateCleanPos;//试剂磁珠针旋转清洗位坐标
    int32_t rotateReactionInjectPos;//试剂磁珠针旋转反应盘注液坐标
    int32_t rotateReagent1AbsorbPos;//试剂磁珠针旋转吸R1坐标
    int32_t rotateReagent2AbsorbPos;//试剂磁珠针旋转吸R2坐标
    int32_t rotateBeadAbsorbPos;//试剂磁珠针旋转吸磁珠坐标
    int32_t timeNeedleCleanMsOut;//试剂磁珠针外壁清洗时间,毫秒
    int32_t timeNeedlePrimeMs;//试剂磁珠针灌注时间,毫秒
    int32_t pumpOneUlStep;//试剂磁珠针注射泵1UL对应步数
    int32_t pumpAbsorbWithAirUl;//试剂磁珠针吸液吸空气量第一次
    int32_t timeNeedleCleanMsInt;//试剂磁珠针内壁清洗时间,毫秒
    int32_t pumpAbsorbWithAirU2;//试剂磁珠针吸液吸空气量第二次
    int32_t pumpInjectRetainAir;//试剂针排液保留的空气量
    int32_t upDownReagentStartPos;//试剂磁珠针开始探液坐标
    int32_t upDownReseve[1];//保留
    int32_t rotateReseve[5];//保留
}SYSTEM_ACTION_PARAM_NEEDLE_REAGENT_BRAD;

typedef enum SYSTEM_ACTION_PARAM_NEEDLE_REAGENT_BRAD_SUBINDEX
{
    BEADUPDOWNRESETCORRECTION=0,//试剂磁珠针升降复位修正
    BEADROTATERESETCORRECTION,//试剂磁珠针旋转复位修正
    BEADUPDOWNCLEANPOS,//试剂磁珠针升降清洗位坐标
    BEADUPDOWNREACTIONINJECTPOS,//试剂磁珠针升降反应盘排液坐标
    BEADUPDOWNREAGENT1ABSORBPOS,//试剂磁珠针升降吸R1极限坐标
    BEADUPDOWNREAGENT2ABSORBPOS,//试剂磁珠针升降吸R2极限坐标
    BEADUPDOWNBEADABSORBPOS,//试剂磁珠针升降吸磁珠极限坐标
    BEADROTATECLEANPOS,//试剂磁珠针旋转清洗位坐标
    BEADROTATEREACTIONINJECTPOS,//试剂磁珠针旋转反应盘注液坐标
    BEADROTATEREAGENT1ABSORBPOS,//试剂磁珠针旋转吸R1坐标
    BEADROTATEREAGENT2ABSORBPOS,//试剂磁珠针旋转吸R2坐标
    BEADROTATEBEADABSORBPOS,//试剂磁珠针旋转吸磁珠坐标
    BEADTIMENEEDLECLEANMSOUT,//试剂磁珠针外壁清洗时间,毫秒
    BEADTIMENEEDLEPRIMEMS,//试剂磁珠针灌注时间,毫秒
    BEADPUMPONEULSTEP,//试剂磁珠针注射泵1UL对应步数
    BEADPUMPABSORBWITHAIRUL,//试剂磁珠针吸液吸空气量第一次
    BEADTIMENEEDLECLEANMSINT,//试剂磁珠针内壁清洗时间,毫秒
    BEADPUMPABSORBWITHAIRU2,//试剂磁珠针吸液吸空气量第二次
    BEADPUMPINJECTRETAINAIR,//试剂针排液保留的空气量
    BEADUPDOWNREAGENTSTARTPOS,//试剂磁珠针开始探液坐标
}SYSTEM_ACTION_PARAM_NEEDLE_REAGENT_BRAD_SUBINDEX;

/*************************************清洗针参数****************************************/
typedef struct SYSTEM_ACTION_PARAM_NEEDLE_WASH
{
    int32_t needleResetCorrection;//清洗针复位后修正坐标
    int32_t needleCupTopPos;//清洗针到杯口坐标
    int32_t needleBottomPos;//清洗针到杯底坐标
    int32_t pump1InjectPos;//清洗针注射泵1注液步数第一次
    int32_t pump2InjectPos;//清洗针注射泵2注液步数第一次
    int32_t pump3InjectPos;//清洗针注射泵3注液步数
    int32_t timeNeedlePrimeMs;//清洗针灌注时间
    int32_t timeNeedleAbsorbEffluentMs;//清洗针废液泵抽废液时间
    int32_t pump1InjectPos1;//清洗针注射泵1注液步数第2次
    int32_t pump2InjectPos1;//清洗针注射泵2注液步数第2次
    int32_t needlePrimePos;//清洗针灌注位置
    int32_t reseve[2];//保留
}SYSTEM_ACTION_PARAM_NEEDLE_WASH;

typedef enum SYSTEM_ACTION_PARAM_NEEDLE_WASH_SUBINDEX
{
    NEEDLERESETCORRECTION=0,//清洗针复位后修正坐标
    NEEDLECUPTOPPOS,//清洗针到杯口坐标
    NEEDLEBOTTOMPOS,//清洗针到杯底坐标
    PUMP1INJECTPOS,//清洗针注射泵1注液步数第一次
    PUMP2INJECTPOS,//清洗针注射泵2注液步数第一次
    PUMP3INJECTPOS,//清洗针注射泵3注液步数
    TIMENEEDLEPRIMEMS,//清洗针灌注时间
    TIMENEEDLEABSORBEFFLUENTMS,//清洗针废液泵抽废液时间
    PUMP1INJECTPOS1,//清洗针注射泵1注液步数第2次
    PUMP2INJECTPOS1,//清洗针注射泵2注液步数第2次
    NEEDLEPRIMEPOS,//清洗针灌注位置
}SYSTEM_ACTION_PARAM_NEEDLE_WASH_SUBINDEX;
/*******************************测量室与注B液参数******************************************/
typedef struct SYSTEM_ACTION_PARAM_MEASUREROOM_LIQUID_B
{
    int32_t upDoorResetCorrection;//测量上门复位后修正坐标
    int32_t windowResetCorrection;//测量侧门复位后修正坐标
    int32_t upDoorClosePos;//上门关闭坐标
    int32_t upDoorPrimePos;//上门灌注位坐标
    int32_t upDoorHalfOpenPos;//上门半开坐标
    int32_t upDoorFullOpenPos;//上门全开坐标
    int32_t windowClosePos;//测量侧门关闭坐标
    int32_t windowOpenPos;//测量侧门打开坐标
    int32_t timeMeasureBaseMs;//光子测量本底测量时间,毫秒
    int32_t timeMeasureNormalMs;//光子测量正常测试时间,毫秒
    int32_t liquidBInjectPos;//B液注液步数
    int32_t reseve[5];//保留
}SYSTEM_ACTION_PARAM_MEASUREROOM_LIQUID_B;

typedef enum SYSTEM_ACTION_PARAM_MEASUREROOM_LIQUID_B_SUBINDEX
{
    UPDOORRESETCORRECTION=0,//测量上门复位后修正坐标
    WINDOWRESETCORRECTION,//测量侧门复位后修正坐标
    UPDOORCLOSEPOS,//上门关闭坐标
    UPDOORPRIMEPOS,//上门灌注位坐标
    UPDOORHALFOPENPOS,//上门半开坐标
    UPDOORFULLOPENPOS,//上门全开坐标
    WINDOWCLOSEPOS,//测量侧门关闭坐标
    WINDOWOPENPOS,//测量侧门打开坐标
    TIMEMEASUREBASEMS,//光子测量本底测量时间,毫秒
    TIMEMEASURENORMALMS,//光子测量正常测试时间,毫秒
    LIQUIDBINJECTPOS,//B液注液步数
}SYSTEM_ACTION_PARAM_MEASUREROOM_LIQUID_B_SUBINDEX;

/**********************************试剂混匀参数***********************************/
typedef struct SYSTEM_ACTION_PARAM_REAGENT_MIX
{
    int32_t rotateResetCorrection;//试剂混匀旋转复位修正坐标
    int32_t upDownResetCorrection;//试剂混匀升降复位修正坐标
    int32_t upDownTopPos;//试剂混匀升降到顶部坐标
    int32_t upDownButtomPos;//试剂混匀升降到底部坐标
    int32_t reseve[5];//保留
}SYSTEM_ACTION_PARAM_REAGENT_MIX;

typedef enum SYSTEM_ACTION_PARAM_REAGENT_MIX_SUBINDEX
{
    ROTATERESETCORRECTION=0,//试剂混匀旋转复位修正坐标
    UPDOWNRESETCORRECTION,//试剂混匀升降复位修正坐标
    UPDOWNTOPPOS,//试剂混匀升降到顶部坐标
    UPDOWNBUTTOMPOS,//试剂混匀升降到底部坐标
}SYSTEM_ACTION_PARAM_REAGENT_MIX_SUBINDEX;

/***********************************磁珠混匀参数*****************************/
typedef struct SYSTEM_ACTION_PARAM_BEAD_MIX
{
    int32_t rotateResetCorrection;//磁珠混匀旋转复位修正坐标
    int32_t upDownResetCorrection;//磁珠混匀升降复位修正坐标
    int32_t upDownTopPos;//磁珠混匀升降到顶部坐标
    int32_t upDownButtomPos;//磁珠混匀升降到底部坐标
    int32_t reseve[5];//保留
}SYSTEM_ACTION_PARAM_BEAD_MIX;

typedef enum SYSTEM_ACTION_PARAM_BEAD_MIX_SUBINDEX
{
    BEADMIXROTATERESETCORRECTION=0,//磁珠混匀旋转复位修正坐标
    BEADMIXUPDOWNRESETCORRECTION,//磁珠混匀升降复位修正坐标
    BEADMIXUPDOWNTOPPOS,//磁珠混匀升降到顶部坐标
    BEADMIXUPDOWNBUTTOMPOS,//磁珠混匀升降到底部坐标
}SYSTEM_ACTION_PARAM_BEAD_MIX_SUBINDEX;
/***********************************A液混匀参数*******************************/
typedef struct SYSTEM_ACTION_PARAM_LIQUID_A_MIX
{
    int32_t rotateResetCorrection;//A液混匀旋转复位修正坐标
    int32_t upDownResetCorrection;//A液混匀旋转复位修正坐标
    int32_t upDownTopPos;//A液混匀升降到顶部坐标
    int32_t upDownButtomPos;//A液混匀升降到底部坐标
    int32_t reseve[5];//保留
}SYSTEM_ACTION_PARAM_LIQUID_A_MIX;

typedef enum SYSTEM_ACTION_PARAM_LIQUID_A_MIX_SUBINDEX
{
    AMIXROTATERESETCORRECTION=0,//A液混匀旋转复位修正坐标
    AMIXUPDOWNRESETCORRECTION,//A液混匀旋转复位修正坐标
    AMIXUPDOWNTOPPOS,//A液混匀升降到顶部坐标
    AMIXUPDOWNBUTTOMPOS,//A液混匀升降到底部坐标
}SYSTEM_ACTION_PARAM_LIQUID_A_MIX_SUBINDEX;
/*********************************清洗盘参数**********************************/
typedef struct SYSTEM_ACTION_PARAM_DISH_WASH_LIQUID_A
{
    int32_t dishWashResetCorrection;//清洗盘复位修正坐标
    int32_t pumpLiquidAResetCorrection;//A液注液泵复位修正坐标
    int32_t dishWashOneCupOffset;//清洗盘移动一个杯位偏移
    int32_t pumpInjectAStep;//注A液泵单次注液步数
    int32_t reseve[5];//保留
}SYSTEM_ACTION_PARAM_DISH_WASH_LIQUID_A;

typedef enum SYSTEM_ACTION_PARAM_DISH_WASH_LIQUID_A_SUBINDEX
{
    DISHWASHRESETCORRECTION=0,//清洗盘复位修正坐标
    PUMPLIQUIDARESETCORRECTION,//A液注液泵复位修正坐标
    DISHWASHONECUPOFFSET,//清洗盘移动一个杯位偏移
    PUMPINJECTASTEP,//注A液泵单次注液步数
}SYSTEM_ACTION_PARAM_DISH_WASH_LIQUID_A_SUBINDEX;
/*********************************试剂盘参数**********************************/
typedef struct SYSTEM_ACTION_PARAM_DISH_REAGENT
{
    int32_t dishReagentResetCorrection;//试剂盘复位后修正坐标
    int32_t oneCupOffset;//试剂盘移动一个杯位偏移
    int32_t reagent1Offset;//试剂位1相对偏移
    int32_t reagent2Offset;//试剂位2相对偏移
    int32_t beadOffset;//磁珠位相对偏移
    int32_t scanOffset;//扫码位相对偏移
    int32_t reseve[5];//保留
}SYSTEM_ACTION_PARAM_DISH_REAGENT;

typedef enum SYSTEM_ACTION_PARAM_DISH_REAGENT_SUBINDEX
{
    DISHREAGENTRESETCORRECTION=0,//试剂盘复位后修正坐标
    REAGENTONECUPOFFSET,//试剂盘移动一个杯位偏移
    REAGENT1OFFSET,//试剂位1相对偏移
    REAGENT2OFFSET,//试剂位2相对偏移
    BEADOFFSET,//磁珠位相对偏移
    SCANOFFSET,//扫码位相对偏移
}SYSTEM_ACTION_PARAM_DISH_REAGENT_SUBINDEX;
/********************************系统内部参数***********************************************/
typedef struct SYSTEM_ACTION_PARAM
{
    SYSTEM_ACTION_PARAM_CUP_MANAGE moduleCupManageParam;//杯栈管理参数集合
    SYSTEM_ACTION_PARAM_HAND_CUP moduleHandCupParam;//抓杯机械手参数集合
    SYSTEM_ACTION_PARAM_REACTION moduleReactionParam;//反应盘参数集合
    SYSTEM_ACTION_PARAM_PATHWAY modulePathWayParam;//传送带参数集合
    SYSTEM_ACTION_PARAM_NEEDLE_SAMPLE moduleNeedleSampleParam;//样本针参数集合
    SYSTEM_ACTION_PARAM_NEEDLE_REAGENT_BRAD moduleNeedleReagentBeadParam;//试剂磁珠针参数集合
    SYSTEM_ACTION_PARAM_NEEDLE_WASH moduleNeedleWashParam;//清洗针参数集合
    SYSTEM_ACTION_PARAM_MEASUREROOM_LIQUID_B moduleMeasureRoomAndLiquidBParam;//光子测量与B液注液参数集合
    SYSTEM_ACTION_PARAM_REAGENT_MIX moduleReagentMixParam;//试剂混匀参数
    SYSTEM_ACTION_PARAM_BEAD_MIX moduleBeadMixParam;//磁珠混匀参数
    SYSTEM_ACTION_PARAM_LIQUID_A_MIX moduleLiquidMixAParam;//A液混匀参数
    SYSTEM_ACTION_PARAM_DISH_WASH_LIQUID_A moduleDishWashLiquidAParam;//清洗盘与A液注液参数
    SYSTEM_ACTION_PARAM_DISH_REAGENT moduleDishReagentParam;//试剂盘参数
}SYSTEM_ACTION_PARAM;

typedef enum SYSTEM_ACTION_PARAM_MAININDEX
{
    MODULECUPMANAGEPARAM=0,//杯栈管理参数集合
    MODULEHANDCUPPARAM,//抓杯机械手参数集合
    MODULEREACTIONPARAM,//反应盘参数集合
    MODULEPATHWAYPARAM,//传送带参数集合
    MODULENEEDLESAMPLEPARAM,//样本针参数集合
    MODULENEEDLEREAGENTBEADPARAM,//试剂磁珠针参数集合
    MODULENEEDLEWASHPARAM,//清洗针参数集合
    MODULEMEASUREROOMANDLIQUIDBPARAM,//光子测量与B液注液参数集合
    MODULEREAGENTMIXPARAM,//试剂混匀参数
    MODULEBEADMIXPARAM,//磁珠混匀参数
    MODULELIQUIDMIXAPARAM,//A液混匀参数
    MODULEDISHWASHLIQUIDAPARAM,//清洗盘与A液注液参数
    MODULEDISHREAGENTPARAM,//试剂盘参数
}SYSTEM_ACTION_PARAM_MAININDEX;

//

//系统动作参数占用的空间大小
#define SYSTEM_ACTION_PARAM_SIZE        (sizeof(SYSTEM_ACTION_PARAM)/sizeof(uint8_t))

extern SYSTEM_ACTION_PARAM* actionParam;

//读取系统内部动作参数
void SystemReadActionParamWithIndex(uint8_t mainIndex, uint8_t subIndex,int32_t* paramValue);

//写入系统内部动作参数
void SystemWriteActionParamWithIndex(uint8_t mainIndex, uint8_t subIndex,int32_t paramValue);

//一次性加载全部系统内部参数,动作参数
void SystemLoadActionParamOnce(void);

//一次性存储系统内部全部参数,动作参数
void SystemSaveActionParamOnce(void);

#endif



