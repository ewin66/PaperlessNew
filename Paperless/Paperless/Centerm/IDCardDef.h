////////////////////////////////////////////////////////////////////////////
//作者：  guocuiping
//文件名：IDCardDef.h
//描述：  定义返回值
//日期：  2013.03.05
////////////////////////////////////////////////////////////////////////////
#ifndef   _IDCARD_DEF_
#define   _IDCARD_DEF_

#include "GlobalDef.h"


//常量——要获取的身份证图片类型定义
#define		IMG_HEAD			0
#define		IMG_FRONT			1
#define		IMG_BACK			2
#define		IMG_IDCARD          3
#define     IMG_FRONT_LOGO      4

//个人信息
typedef struct PERSONINFO
{
	char name[80];      //【FZ】中文姓名  30字节  
	char EngName[120];  //【F】英文姓名  120字节
	char version[10];   //【F】证件版本号 4字节
	char govCode[10];   //【F】当次申请受理机关代码  8字节
	char cardType[10];  //【F】证件类型标志 2字节
	char otherData[10]; //【F】预留项  6字节
	char sex[10];       //[Z 通过性别代码转换成性别]
	char nation[50];    //[Z 通过民族编号进行转换]
	char birthday[30];  //【FZ】出生日期   16字节
	char address[180];  //【Z】居住地址 70字节
	char cardId[50];    //【FZ】永久居留证号码  30字节   【身份证号36个字节】
	char police[80];    //【Z】派出所地址 30字节
	char validStart[30];//【FZ】证件签发时间  16字节
	char validEnd[30];  //【FZ】证件终止时间  16字节
	char sexCode[10];   //【FZ】性别  2字节
	char nationCode[10];//【FZ】国籍或者地区代码   6个字节  [民族 4字节]
	char appendMsg[180];//【Z】附加信息 70字节
	int  iFlag;    

}PERSONINFO;


#endif