#ifndef __TESTPROC_RESULT_H_
#define __TESTPROC_RESULT_H_

#define TESTPROC_RESULT_OK                      0           // OK
#define TESTPROC_RESULT_NG                      (-1)        // NG
#define TESTPROC_RESULT_ERR_MOTION              (-2)        // 不支持的动作
#define TESTPROC_RESULT_ERR_CELL                (-3)
#define TESTPROC_RESULT_ERR_BOARD               (-4)
#define TESTPROC_RESULT_ERR_LEN                 (-5)
#define TESTPROC_RESULT_ERR_PARAM               (-6)
#define TESTPROC_RESULT_ERR_CMD                 (-7)
#define TESTPROC_RESULT_ERR_STATUS              (-8)
#define TESTPROC_RESULT_ERR_MEM                 (-9)
#define TESTPROC_RESULT_ERR_MSGMEM              (-10)
#define TESTPROC_RESULT_ERR_CRC                 (-11)
#define TESTPROC_RESULT_ERR_EXEC                (-12)        // 执行返回错误
#define TESTPROC_RESULT_ERR_ITEMLOST            (-13)
#define TESTPROC_RESULT_ERR_ITEM                (-14)

#endif // End of “ifndef __TESTPROC_RESULT_H_”

