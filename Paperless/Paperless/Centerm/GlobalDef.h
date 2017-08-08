#ifndef _GLOBAL_DEF_H_
#define _GLOBAL_DEF_H_

#include "PrjDef.h"

/*!<data type define */
typedef unsigned char         U8;
typedef char                  S8;
typedef unsigned int          U32;
typedef int                   S32;
typedef S32 ( *EndCondition )( U8 *, S32 );

/*!<same useful marco define */
#define TABLESIZE(a)                                ( sizeof(a)/sizeof(a[0]) )
#define INVALID_DESCRIPTOR                          (-1)
#define MAX_LEN                                     (1024)
#define PACKET_LEN                                  (2048)
#define ICC_CMD_HAS_LE                              (0x1)
/*!<define return values */
#define RET_SUCCESS                                 (0)
#define ERR_SUCCESS                                 (0)
#define RET_ERROR                                   (-1)
#define RET_INVALID_PARAM                           (-2)
#define RET_FAIL_OPEN_FILE                          (-3)
#define RET_FAIL_TIMEOUT                            (-4)
#define RET_WRITE_ERROR                             (-5)
#define RET_READ_ERROR                              (-6)
#define RET_DEVICE_UNOPEN                           (-7)
#define RET_WRONG_PACKET                            (-8)
#define RET_CARDSLOT_ERROR                          (-9)
#define RET_NO_FOUND_SFI                            (-10)
#define RET_SFI_LEN_ERR                             (-11)
#define RET_MALLOC_ERR                              (-12)
#define RET_DOL_NO_FOUND                            (-13)
#define RET_DOL_DATA_ERR                            (-14)
#define RET_DOL_DATA_NO_FOUND                       (-15)
#define RET_APP_INIT_ERR                            (-16)
#define RET_LOG_NO_ENTRY                            (-17)
#define RET_RECORD_NO_FOUND                         (-18)
#define RET_GET_GAC_TOO_MUCH                        (-19)
#define RET_CMD_ERR                                 (-20)
#define RET_GAC_RESP_ERR                            (-21)
#define RET_AUTH_DATA_ERROR                         (-22)
#define RET_TRADE_ACC                               (-23)
#define RET_NO_SCRIPT                               (-24)
#define RET_WRITE_SCRIPT_ERR                        (-25)
#define RET_SCRIPT_CMD_ERR                          (-26)
#define  ERR_OPENFAILED         					(-27)//打开设备失败
#define  ERR_WRITE									(-28)//发送指令失败----与设备通讯失败
#define  ERR_READ									(-29)//接收数据失败----与设备通讯失败
#define  ERR_PACKAGE_FORMAT							(-30)//接收到的报文格式错误
#define  ERR_READ_CARD								(-31)//读卡操作失败
#define  ERR_WRITE_CARD								(-32)//写卡操作失败
#define  ERR_CANCELED           					(-33)//用户取消
#define  ERR_TIMEOUT            					(-34)//超时
#define  ERR_PARAM              					(-35)//参数异常
//IDcard返回结果
#define     ERR_DEVICE								-36    //设备错误
#define     ERR_RECV								-37    //接收错误
#define     ERR_SEND							    -38   //发送错误
#define     ERR_CHECK								-39    //校验错误
#define     ERR_IMAGE        						-40    //解析头像错误
#define     ERR_SAVEIMG      						-41    //保存图像错误
#define     ERR_NO_CARD      						-42   //未找到卡片
#define     ERR_OTHER        						-100  //其他错误


/*!<define Communication types */
#define TYPE_HID                          (0x0)   //hid
#define TYPE_COM                          (0x1)   //com
#define TYPE_PCSC                         (0x2)   //pc/sc
#define TYPE_USBLIB_HID              (0x03) //usblibapi
#define TYPE_AUX                     (0x04)
#define TYPE_SDT                     (0x05)

/*!<define some flags */
#define TERM_LOCK                        (0x1)
#define TERM_UNLOCK                      !TERM_LOCK
#define TERM_INIT_VALUE                  (0xAA)
#define DEFAULT_TIME_OUT                 (10)
#define DEFAULT_BAUD                     (9600)

/*!<define card reader types */
#define READER_CKB51XX                   (0x0)
#define READER_PCSC                      (0x1)


//卡定义 
//接触式卡
#define  SMART_CARD_BASE                0x0
#define  SMART_CARD                     SMART_CARD_BASE      //接触式卡
#define  SMART_CARD_MAX                 (SMART_CARD_BASE+0xF)//接触式卡最大的卡号
#define  IS_ICC_SLOT( a )               ( ( (a) >= SMART_CARD_BASE )&&( (a) <= SMART_CARD_MAX ) ? 1 : 0 )

 //非接触式卡
#define PICC_CARD_SLOT                  (0xFF)

//PSAM卡
#define  PSAM_CARD_BASE                 0x10
#define  PSAM_CARD                      PSAM_CARD_BASE      
#define  PSAM_CARD_MAX                  (PSAM_CARD_BASE+0xF)//PSAM卡最大的卡号
#define  IS_PSAM_SLOT( a )              ( ( (a) >= PSAM_CARD )&&( (a) <= PSAM_CARD_MAX ) ? 1 : 0 )



//GAC命令功能码
#define CMD_GAC_ARQC    (1)
#define CMD_GAC_TC      (2)
#define CMD_GAC_AAC     (3)


//使用升腾指令集
#define SOH						(0x01)
#define EOT						(0x04)
#define CENT_COMMAND_SEPARATOR	"\x20\x01\x7C"  //设置分割符,'|'
#define CENT_COMMAND_RD123		"\x20\x02"  //读磁道123
#define CENT_COMMAND_WT123		"\x20\x03"  //写磁道123
#define CENT_COMMAND_TIMEOUT	"\x20\x04"  //设置超时


//南天磁卡操作命令
#define COMMAND_RESET         "\x1B\x30"//复位
#define COMMAND_STATUS        "\x1B\x6A"//状态查询

#define COMMAND_ISO           "\x1B\x3D"//设置磁道为ISO
#define COMMAND_IBM           "\x1B\x27"//设置磁道格式为IBM
#define COMMAND_ISO_D         "\x1B\x4F"//设置磁道格式为ISO(德卡)
#define COMMAND_IBM_D         "\x1B\x41"//设置磁道格式为IBM(德卡)

//南天读卡命令
#define COMMAND_RD1           "\x1B\x72"
#define COMMAND_RD2           "\x1B\x5D"
#define COMMAND_RD3           "\x1B\x54\x5D"
#define COMMAND_RD12          "\x1B\x44\x5D"
#define COMMAND_RD23          "\x1B\x42\x5D"

//南天写卡命令
#define COMMAND_WT1           "\x1B\x77%s\x1D\x1B\x5C"
#define COMMAND_WT2           "\x1B\x74%s\x1D\x1B\x5C"
#define COMMAND_WT3           "\x1B\x74\x41%s\x1D\x1B\x5C"
#define COMMAND_WT12          "\x1B\x77%s\x41%s\x1D\x1B\x5C"
#define COMMAND_WT23          "\x1B\x74%s\x41%s\x1D\x1B\x5C"



//常量定义
#define  MAXLEN_TRCKDATA          512


//磁道位置定义
#define  MODE_TRACK1             1
#define  MODE_TRACK2             2
#define  MODE_TRACK3             3
#define  MODE_TRACK12            12
#define  MODE_TRACK23            23
#define  MODE_TRACK123           123

#ifndef FALSE
#define FALSE  0
#endif

#ifndef TRUE 
#define TRUE  1
#endif

#ifndef NULL
#define NULL 0
#endif

typedef unsigned long       DWORD;
typedef int                 BOOL;
typedef unsigned char       BYTE;
typedef unsigned short      WORD;
typedef float               FLOAT;

typedef struct 
{
  S32 nInitFlag;/*!< termparam init flag*/

  S32 nTransType;/*!< 通信类型*/
  S32 nCardReader;/*!< 读卡器类型*/
  
  S32 nPID;
  S32 nVID;
  
  S32 nCom;/*!<串口号*/
  S32 nBaud;/*!<串口波特率*/
  
  #if defined( LINUX_EDITION )
  S32 nAuxNo;/*!<辅口号*/ 
  S32 nTermType;/*!<终端类型*/  
  S32 nLockKeyboard;/*!<是否锁键盘*/
  S8  szTermPath[256];
  #endif
  
  S32 nTimeOut;/*!<超时时间设置*/
  EndCondition lpEndCondition;/*!<结束条件判断*/

  #if defined( USE_TYPE_PCSC )
  S8 szPcscReader[256];
  #endif

  //R20006 add by sulin 全局变量添加
  U8 szTermData[1024];          //形成Tc需要的初始化数据
  S32 nTermDataLen;
  
}TERMPARAM;

typedef struct 
{
	U8 szIccAppData[4096] ;
	U8 szAip[2+1];
	U32 nIccAppDataLen ;
	U8 szTcF55[4096];		//add by cjm
	U32 nTcF55Len;			//add by cjm
}ICCARDPARAM;


#define TAG_LEN     32
typedef struct
{
	char UsrTag[ TAG_LEN ];     //用户的标签
	int  UsrTagLen;             //用户标签长度
	char OurTag[ TAG_LEN ];     //我们自己的标签
	int OurTagLen;              //标签长度
} TAGLIST;

#endif


