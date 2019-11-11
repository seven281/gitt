#ifndef __TSC_QR_SCAN_H_
#define __TSC_QR_SCAN_H_
#include "TSC_Base.h"

//QR扫码
typedef enum TSC_QR_SCAN_CMD_INDEX
{
    TSC_QR_SCAN_READ_ONCE = 0,
}TSC_QR_SCAN_CMD_INDEX;

/****************************************************二维码条码扫描********************************************************/
//二维码条码扫描上一次指令的执行状态
TSC_CMD_STATE TSC_QRCodeScanGetLastCommandStatusAndResult(LH_ERR* errCode);

//二维码条码扫描数据结构初始化
void TSC_QRCodeScanResultDataInit(void);

//二维码条码扫描一次
LH_ERR TSC_QRCodeScanReadOnceWhileAck(void);

/****************************************************二维码条码扫描********************************************************/

/*********************************************动作结果中的数据读取***************************************************/
//注意,要用二级指针
//读取上一次的扫描到的二维码的长度和数据指针
uint16_t TSC_QRCodeScanReadLastReturnCode(uint8_t** bufferPtrPtr);


#endif
