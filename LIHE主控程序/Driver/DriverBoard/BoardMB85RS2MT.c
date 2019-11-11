#include "BoardMB85RS2MT.h"
#include "os.h"

//互斥访问
static OS_MUTEX MUTEX_MB85RS2MT;

static LH_ERR BoardMB85RS2MT_Check(void)
{
    uint8_t dat = 0;
    uint8_t dat2 = 0;
    uint8_t dataCopy = 0;
    //读取地址0数据
    BoardMB85RS2MT_ReadBuffer(0,&dat2,1);
    //要把原来的值保存起来
    dataCopy = dat2;
    //对读取到的数据进行操作
    dat = dat2+111;
    //忘记原来读取到的数据
    dat2 = 0;
    //将地址0数据加上指定数据写入flash
    BoardMB85RS2MT_WriteBuffer(0,&dat,1);
    //从地址0数据处读取刚写入的数据
    BoardMB85RS2MT_ReadBuffer(0,&dat2,1);
    //把原来的值写会去
    BoardMB85RS2MT_WriteBuffer(0,&dataCopy,1);
    //dat2是读到的处理后的数据 dat是写入之前的数据
    if(dat != dat2)
    {
        //两者不等,说明要么写入要么读取失败
        return LH_ERR_MAIN_FRAM_CHECK;
    }
    else
    {
        //两者相等,说明校验通过
        return LH_ERR_NONE;;
    }
}

LH_ERR BoardMB85RS2MT_Init(void)
{
    uint32_t BoardMB85RS2MT_id = 0;
    OS_ERR err;
    //初始化片选CS
    MCU_PortInit(MCU_PIN_A_4, GPIO_Mode_OUT, GPIO_OType_PP, GPIO_PuPd_UP, GPIO_Fast_Speed);
    MCU_PortWriteSingle(MCU_PIN_A_4, Bit_SET);
    //初始化SPI
    MCU_SPI1_Init(MCU_SPI_SPEED_DIV256,MCU_SPI_CPOL_HIGH,MCU_SPI_CPHA_2EDGE);
    //设置传输速度4分频
    MCU_SPI1_SetSpeed(MCU_SPI_SPEED_DIV4);
    //创建互斥信号量
    OSMutexCreate((OS_MUTEX*	)&MUTEX_MB85RS2MT,
				  (CPU_CHAR*	)"MUTEX_MB85RS2MT",
                  (OS_ERR*		)&err);	
    //读取芯片ID
    BoardMB85RS2MT_ReadID(&BoardMB85RS2MT_id);
    if(BoardMB85RS2MT_id == MB85RS2MT_ID)
    {
        return BoardMB85RS2MT_Check();
    }
    else
    {
        return LH_ERR_MAIN_FRAM_ID;
    }
}

void BoardMB85RS2MT_ReadID(uint32_t *id)
{
    OS_ERR err;
    //操作系统启动,禁用调度器
    if(OSRunning)
    {
        //申请互斥信号量,防止多任务抢占
		OSMutexPend (&MUTEX_MB85RS2MT,0,OS_OPT_PEND_BLOCKING,0,&err);
    }
    uint8_t* buf= (uint8_t*)id;
    //片选
	MB85RS2MT_CS_Set(0);
    CoreDelayUs(10);
    //写指令
	MCU_SPI1_WriteRead(MB85RS2MT_CMD_RDID);              
    uint8_t temp;
    do
    {
        temp = MCU_SPI1_WriteRead(0xFF);
    }while(temp != (uint8_t)(MB85RS2MT_ID));
	buf[0] = temp;
	buf[1] = MCU_SPI1_WriteRead(0xFF);
	buf[2] = MCU_SPI1_WriteRead(0xFF);
    buf[3] = MCU_SPI1_WriteRead(0xFF);
	MB85RS2MT_CS_Set(1);
    if(OSRunning)
    {
        //释放互斥信号量
        OSMutexPost(&MUTEX_MB85RS2MT,OS_OPT_POST_NO_SCHED,&err);
    }
}

void BoardMB85RS2MT_WriteBuffer(uint32_t address, uint8_t *pdata, uint32_t length)
{
    OS_ERR err;
	//操作系统启动,禁用调度器
    if(OSRunning)
    {
        //申请互斥信号量,防止多任务抢占
		OSMutexPend (&MUTEX_MB85RS2MT,0,OS_OPT_PEND_BLOCKING,0,&err);
    }
    //片选使能.
	MB85RS2MT_CS_Set(0);           
    //写使能                          
	MCU_SPI1_WriteRead(MB85RS2MT_CMD_WREN);               
    MB85RS2MT_CS_Set(1);
    CoreDelayUs(10);
    MB85RS2MT_CS_Set(0);
    //写指令
	MCU_SPI1_WriteRead(MB85RS2MT_CMD_WRITE);  
    //写地址高两位            
	MCU_SPI1_WriteRead((uint8_t)(address>>16));    
    //写地址中间字节  
	MCU_SPI1_WriteRead((uint8_t)(address>>8));   
    //写地址低字节   
	MCU_SPI1_WriteRead((uint8_t)(address));          
	/*写入数据*/
	for(uint32_t i=0;i<length;i++)
		MCU_SPI1_WriteRead(pdata[i]);
    //片选禁止写保护
	MB85RS2MT_CS_Set(1);                                
    if(OSRunning)
    {
        //释放互斥信号量
        OSMutexPost(&MUTEX_MB85RS2MT,OS_OPT_POST_NO_SCHED,&err);
    }
}

void BoardMB85RS2MT_ReadBuffer(uint32_t address, uint8_t *pdata, uint32_t length)
{
    OS_ERR err;
	//操作系统启动,禁用调度器
    if(OSRunning)
    {
        //申请互斥信号量,防止多任务抢占
		OSMutexPend (&MUTEX_MB85RS2MT,0,OS_OPT_PEND_BLOCKING,0,&err);
    }
    //片选使能
	MB85RS2MT_CS_Set(0);                   
    //读指令             
	MCU_SPI1_WriteRead(MB85RS2MT_CMD_READ);     
    //读地址高两位          
	MCU_SPI1_WriteRead((uint8_t)(address>>16));    
    //读地址中间字节  
	MCU_SPI1_WriteRead((uint8_t)(address>>8)); 
    //读地址低字节      
	MCU_SPI1_WriteRead((uint8_t)(address));          
	/*读出数据*/
	for(uint32_t i=0;i<length;i++)
		pdata[i]=MCU_SPI1_WriteRead(0XFF);
    //片选禁止
	MB85RS2MT_CS_Set(1);                 
    if(OSRunning)
    {
        //释放互斥信号量
        OSMutexPost(&MUTEX_MB85RS2MT,OS_OPT_POST_NO_SCHED,&err);
    }
}
