#ifndef __ACTION_MIX_BEAD_H_
#define __ACTION_MIX_BEAD_H_
#include "ActionCommonBase.h"


//磁珠摇匀复位
LH_ERR ActionMixBead_Reset(void);

//磁珠摇匀上升
LH_ERR ActionMixBead_Up(void);

//磁珠摇匀下降
LH_ERR ActionMixBead_Down(void);

//磁珠摇匀混匀指定时间
LH_ERR ActionMixBead_RotateAnyTimes(uint16_t timeMs);


#endif
