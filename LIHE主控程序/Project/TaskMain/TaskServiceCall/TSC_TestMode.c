
#include "TSC_TestMode.h"
#include "TestProc_CanConfig.h"
#include "Can1DataType.h"

static TSC_TEST_MODE_SELECTION testmode;

/**
 * 配置测试模式
 * 
*/
void TSC_TestMode_Selection(TSC_TEST_MODE_SELECTION mode)
{
    uint8_t count;
    testmode = mode;
    count = Arraypack_Common(0,testmode,0);
    CanSendDataToCell(TESTPROC_CELL_PLATFORM_NMBR,BOARD_ID_TRACK_COMM,0x0018,0,12);
}
/**
 * 读取当前的测试模式
*/
TSC_TEST_MODE_SELECTION TSC_Read_TestMode(void)
{
    return testmode;
}
