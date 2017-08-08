#pragma once


const int N_ID_CARD_CODE_COUNT = 9;
const char S_ID_CARD_CODE[][5] = 
	{"000",
	"200",
	"201",
	"202",
	"203",
	"204",
	"205",
	"206",
	"207"};
const char S_ID_CARD_MSG[][128] = 
	{"成功",
	"打开身份证芯片头像照失败",
	"读取身份证芯片头像照失败",
	"读取身份证芯片头像照前请求内存失败",
	"未检测到身份证识读仪，检测是否正常连接以及配置是否正确",
	"读取身份证失败，检测身份证是否放好",
	"读取身份证错误",
	"身份证校验错误",
	"保存身份证芯片头像照失败"};


const int N_READ_PIC_CODE_COUNT = 13;
const char S_READ_PIC_CODE[][5] = 
	{"000",
	"101",
	"102",
	"103",
	"104",
	"105",
	"106",
	"107",
	"108",
	"109",
	"110",
	"111",
	"112"};
const char S_READ_PIC_MSG[][128] = 
	{"成功",
	"打开身份证照片失败",
	"读取身份证照片失败",
	"读取身份证照片前请求内存失败",
	"未检测到高拍仪摄像头",
	"打开高拍仪摄像头失败",
	"设置高拍仪摄像头分辨率失败",
	"设置高拍仪摄像头自动裁边失败",
	"高拍仪拍取照片失败",
	"关闭高拍仪摄像头失败",
	"分辨率转换时载入源图片失败",
	"分辨率转换时载入目标图片失败",
	"分辨率转换时保存目标图片失败"};



// 自定个人信息
typedef struct S_MYPERSONINFO
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
}MYPERSONINFO;


/**
  * 身份证芯片信息读取基类
 **/
class CBaseReadIDCardInfo
{
public:
	CBaseReadIDCardInfo(){}
	virtual ~CBaseReadIDCardInfo(){}
public:
	// 纯虚函数，获取身份证芯片信息
	virtual int MyReadIDCardInfo(const char *pSaveHeadPicFilenm, MYPERSONINFO *pPersonInfo) = 0;
};



/**
  * 高拍仪 摄像头 图片获取基类
 **/
class CBaseSaveCamera
{
public:
	CBaseSaveCamera():F_AREA_RATE((float)2.213), F_HEIGHT_WIDTH_RATE((float)((21 * 1.0) / 17)){}
	//CBaseSaveDeskPic():F_AREA_RATE((float)2.213), F_HEIGHT_WIDTH_RATE((float)((21 * 1.0) / 17)){}
	virtual ~CBaseSaveCamera() {}
public:
	const float F_AREA_RATE;
	const float F_HEIGHT_WIDTH_RATE;
	// 纯虚函数，高拍仪 文拍摄像头 获取身份证照片
	virtual int MySaveDeskIDPic(const char *pSaveDesktopIDPicFilenm) = 0;
	// 纯虚函数，高拍仪 环境摄像头 获取 人像照片
	virtual int MySaveEnvPic(const char *pSaveEnvPicFilenm) = 0;
};