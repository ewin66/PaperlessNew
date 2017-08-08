/******************************************************************************************
* 
*  Copyright(C), 2002-2013, Centerm Information Co.,Ltd,All Rights Reserved
*
*  FileName: PrjDef.h
*
*  Author:  quexinsheng
*
*  Date:   2014/1/7 14:47:01
*
*  Description: 项目自定义
*
*  History:     
*               
*     <flag>   <author>       <date>          <version>     <description>
*******************************************************************************************/
/*!<
           通信方式说明
目前，PBOC2.0v1.10的代码支持的通信方式有
window环境有：
COM_WIN_MODE         串口通信方式
HID_WIN_MODE         hid通信方式
PCSC_WIN_MODE        smartcard通信方式
Linux环境有：        
COM_LINUX_MODE       串口通信方式
这些方法通过_LINUX_宏控制，未定义了则是window环境
*/

#ifndef _SELF_DEF_H_
#define _SELF_DEF_H_

#define USE_READER_CKB51XX
#define USE_READER_PCSC
#define WINDOWS_EDITION	          //windows驱动

#define USE_TYPE_PCSC
#define USE_TYPE_HID
#define USE_TYPE_COM              //串口通信方式
#define DEBUG_OUTPUT            //打开调试 
#define REG_DEBUG_FLG           //使用注册表控制DEBUG

#define BANGK                   "XX银行"

//定义usb描述符
#define  HID_VID              0x0403
#define  HID_PID              0xBCD7//0xBCD5
#define  HID_PID_BCD5   0xBCD5
#define  HID_PID_BCD7   0xBCD7

typedef struct ST_HID
{
	int pid;
	int vid;
}HID;

#define  BUAD_RATE            115200 //提升默认通讯波特率
#define  BP_CMD               "\x1B\x25" //转口指令
#define  ICC_PORT_25_9600     "\x1b\x25\x49\x34"
#define  ICC_DEFAULT_25_PORT  "\x1b\x25\x42"
#define  ICC_PORT_24_9600     "\x1b\x24\x49\x34"
#define  ICC_DEFAULT_24_PORT  "\x1b\x24\x42"

//定义这个宏，则默认使用A000000333，而不从卡片中获取
//#define  SPEED_UP_NO_GET_AID_FROM_CARD

//在55域数据中增加应用pan和应用pan序列号
#define ARQC_MORE_TAG

//外部认证失败是否退出
//#define  EX_AUTH_FAIL_EXIT
#define  DEVICE_AUTH
#define AUTH_MODE

#ifndef LINUX_EDITION
#define _STDOUT _stdcall 
#include <windows.h>
#else
#define _STDOUT __attribute__  ((visibility("default"))) 
#endif

#endif



