#ifndef _W5500_CONF_H_
#define _W5500_CONF_H_
#include "MCU_Inc.h"
#include "stdio.h"
#include "types.h"

//片选信号
#define W5500_CS_Set(value)   PBout(12) = value

typedef  void (*pFunction)(void);

extern uint8  	remote_ip[4];															/*远端IP地址*/
extern uint16_t 	remote_port;															/*远端端口号*/
extern uint16_t		local_port;																/*定义本地端口*/
extern uint8  	use_dhcp;																	/*是否使用DHCP获取IP*/
extern uint8  	use_eeprom;																/*是否使用EEPROM中的IP配置信息*/


#define MAX_BUF_SIZE		 				1460											 /*定义每个数据包的大小*/
#define KEEP_ALIVE_TIME	     		30	// 30sec
#define TX_RX_MAX_BUF_SIZE      2048							 

extern uint8	ip_from;											               /*选择IP信息配置源*/
#pragma pack(1)
/*此结构体定义了W5500可供配置的主要参数*/
typedef struct _CONFIG_MSG											
{
  uint8 mac[6];																							/*MAC地址*/
  uint8 lip[4];																							/*local IP本地IP地址*/
  uint8 sub[4];																							/*子网掩码*/
  uint8 gw[4];																							/*网关*/	
  uint8 dns[4];																							/*DNS服务器地址*/
  uint8 rip[4];																							/*remote IP远程IP地址*/
	uint8 sw_ver[2];																					/*软件版本号*/

}CONFIG_MSG;
#pragma pack()

#pragma pack(1)
/*此结构体定义了eeprom写入的几个变量，可按需修改*/
typedef struct _EEPROM_MSG	                    
{
	uint8 mac[6];																							/*MAC地址*/
  uint8 lip[4];																							/*local IP本地IP地址*/
  uint8 sub[4];																							/*子网掩码*/
  uint8 gw[4];																							/*网关*/
}EEPROM_MSG_STR;
#pragma pack()

extern CONFIG_MSG  	ConfigMsg;

/*W5500SPI相关函数*/
void IINCHIP_WRITE( uint32 addrbsb,  uint8 data);						/*写入一个8位数据到W5500*/
uint8 IINCHIP_READ(uint32 addrbsb);													/*从W5500读出一个8位数据*/
uint16_t wiz_write_buf(uint32 addrbsb,uint8* buf,uint16_t len);	/*向W5500写入len字节数据*/
uint16_t wiz_read_buf(uint32 addrbsb, uint8* buf,uint16_t len);	/*从W5500读出len字节数据*/

/*W5500基本配置相关函数*/
void set_w5500_mac(void);																		/*配置W5500的MAC地址*/
void set_w5500_ip(void);																		/*配置W5500的IP地址*/
void get_w5500_mac(uint8_t* macBufferPtr);


#endif
