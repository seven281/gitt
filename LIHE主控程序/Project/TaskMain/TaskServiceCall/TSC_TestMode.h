#ifndef _TSC_TESTMODE_H
#define _TSC_TESTMODE_H

typedef enum TSC_TEST_MODE_SELECTION
{
    TSC_TEST_MODE_NORMAL = 0,           //正常模式包含液路动作
    TSC_TEST_MODE_AGEING,               //老化模式不包含液路动作
}TSC_TEST_MODE_SELECTION;





extern TSC_TEST_MODE_SELECTION testmode;

extern void TSC_TestMode_Selection(TSC_TEST_MODE_SELECTION mode);

extern TSC_TEST_MODE_SELECTION TSC_Read_TestMode(void);

#endif