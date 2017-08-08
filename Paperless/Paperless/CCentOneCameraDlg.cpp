// CCentOneCameraDlg.cpp : implementation file
#include "stdafx.h"
#include "XXbgService.h"
#include "CCentOneCameraDlg.h"
#include "afxdialogex.h"
#include "MyTTrace.h"
#include "utils.h"


// CCentOneCameraDlg dialog

IMPLEMENT_DYNAMIC(CCentOneCameraDlg, CDialogEx)

CCentOneCameraDlg::CCentOneCameraDlg(CWnd* pParent /*=NULL*/)
	: CDialogEx(CCentOneCameraDlg::IDD, pParent)
{
	GtWriteTrace(EM_TraceDebug, "%s:%d: 构造函数!", __FUNCTION__, __LINE__);
	pVideoWnd = NULL;
	mHighCamera = NULL;
}

CCentOneCameraDlg::~CCentOneCameraDlg()
{
	if (mHighCamera)
	{
		delete mHighCamera;
		mHighCamera = NULL;
	}
}

BOOL CCentOneCameraDlg::OnInitDialog()
{
	pVideoWnd = (CStatic *)GetDlgItem(IDC_STATIC_CAMERA);
	//pVideoWnd->ShowWindow(SW_SHOW);
	//mHighCamera = new CCentermIFImp(pVideoWnd);
	//mHighCamera->OpenDevice(0);
	return TRUE;
}

void CCentOneCameraDlg::DoDataExchange(CDataExchange* pDX)
{
	CDialogEx::DoDataExchange(pDX);
}


BEGIN_MESSAGE_MAP(CCentOneCameraDlg, CDialogEx)
END_MESSAGE_MAP()



// 重写，文拍摄像头获取人像照片
int CCentOneCameraDlg::MySaveDeskIDPic(const char *pSaveDesktopIDPicFilenm)
{
	GtWriteTrace(30, "%s:%d: 进入文拍摄像头获取身份证照片函数!", __FUNCTION__, __LINE__);
	int nRet = 0;
	CImage imSrc;
	CImage imDest;
	// 小分辨率图片HDC
	HDC destDc;
	// 小图片区域
	CRect destRect;
	// 图片控件区域
	CRect picRect;
	// 分辨率
	int width = 0;
	int high = 0;
	char sDeskScanNo[32] = {0};
	char sDeskScanSize[32] = {0};
	char sIDPicWidth[32] = {0};
	char sIDPicHigh[32] = {0};
	char sAutoCropWaitTime[32] = {0};
	// 当前程序运行路径
	CString sIniFilePath;
	sIniFilePath = GetAppPath() + "\\win.ini";
	// 临时字符串
	char sTmpString[32] = {0};
	// 高拍仪 文拍摄像头 序号
	GetPrivateProfileString("CentermOneMachine", "DeskScanNo", "0", sTmpString, sizeof(sTmpString)-1, sIniFilePath);
	memcpy(sDeskScanNo, (const char*)sTmpString, sizeof(sDeskScanNo)-1);
	// 高拍仪 文拍摄像头 分辨率级别
	GetPrivateProfileString("CentermOneMachine", "DeskScanSize", "5", sTmpString, sizeof(sTmpString)-1, sIniFilePath);
	memcpy(sDeskScanSize, (const char*)sTmpString, sizeof(sDeskScanSize)-1);
	// 高拍仪截取的身份证分辨率宽
	GetPrivateProfileString("CentermOneMachine", "IDPicWidth", "330", sTmpString, sizeof(sTmpString)-1, sIniFilePath);
	memcpy(sIDPicWidth, (const char*)sTmpString, sizeof(sIDPicWidth)-1);
	// 高拍仪截取的身份证分辨率高
	GetPrivateProfileString("CentermOneMachine", "IDPicHigh", "210", sTmpString, sizeof(sTmpString)-1, sIniFilePath);
	memcpy(sIDPicHigh, (const char*)sTmpString, sizeof(sIDPicHigh)-1);
	// 高拍仪设置自动框选区域等待时长，单位：ms（不等待直接获取图片可能无法框选）
	GetPrivateProfileString("CentermOneMachine", "AutoCropWaitTime", "100", sTmpString, sizeof(sTmpString)-1, sIniFilePath);
	memcpy(sAutoCropWaitTime, (const char*)sTmpString, sizeof(sAutoCropWaitTime)-1);

	GtWriteTrace(30, "%s:%d: 参数: sDeskScanNo=[%s], sDeskScanSize=[%s], sIDPicWidth=[%s], sIDPicHigh=[%s], sAutoCropWaitTime=[%s]",
		__FUNCTION__, __LINE__, sDeskScanNo, sDeskScanSize, sIDPicWidth, sIDPicHigh, sAutoCropWaitTime);

	if (mHighCamera == NULL)
	{
		pVideoWnd = (CStatic *)GetDlgItem(IDC_STATIC_CAMERA);
		mHighCamera = new CCentermIFImp(pVideoWnd);
	}
	GtWriteTrace(30, "%s:%d: 检查摄像头!", __FUNCTION__, __LINE__);
	// 检查摄像头个数
	nRet = mHighCamera->DetectDevice();
	if (nRet <= 0)
	{
		GtWriteTrace(30, "%s:%d: 检查摄像头个数 DetectDevice() nRet = %d", __FUNCTION__, __LINE__, nRet);
		return 104;
	}
	GtWriteTrace(30, "%s:%d: 打开摄像头!", __FUNCTION__, __LINE__);
	// 打开摄像头
	nRet = mHighCamera->OpenDevice(atoi(sDeskScanNo));
	if (nRet != 0)
	{
		GtWriteTrace(30, "%s:%d: 打开摄像头 OpenDevice() nRet = %d", __FUNCTION__, __LINE__, nRet);
		return 105;
	}
	GtWriteTrace(30, "%s:%d: 设置分辨率!", __FUNCTION__, __LINE__);
	// 设置分辨率
	nRet = mHighCamera->SetScanSize(atoi(sDeskScanSize), atoi(sDeskScanNo));
	if (nRet != 0)
	{
		GtWriteTrace(30, "%s:%d: 设置分辨率 SetScanSize() nRet = %d", __FUNCTION__, __LINE__, nRet);
		mHighCamera->CloseDevice();
		return 106;
	}
	GtWriteTrace(30, "%s:%d: 设置自动裁边!", __FUNCTION__, __LINE__);
	// 设置自动裁边
	nRet = mHighCamera->SetAutoCrop(true, atoi(sDeskScanNo));
	if (nRet != 0)
	{
		GtWriteTrace(30, "%s:%d: 设置自动裁边 SetAutoCrop() nRet = %d", __FUNCTION__, __LINE__, nRet);
		mHighCamera->CloseDevice();
		return 107;
	}
	Sleep(atoi(sAutoCropWaitTime));
	// 调高拍仪接口保存图片
	CString tmpDir = pSaveDesktopIDPicFilenm;
	nRet = mHighCamera->ScanImage(tmpDir);
	if (nRet != 0)
	{
		GtWriteTrace(30, "%s:%d: 调高拍仪接口保存图片 ScanImage() nRet = %d", __FUNCTION__, __LINE__, nRet);
		mHighCamera->CloseDevice();
		return 108;
	}
	// 关闭设备
	nRet = mHighCamera->CloseDevice();
	GtWriteTrace(30, "%s:%d: 关闭设备返回值 CloseDevice() nRet = [%d]", __FUNCTION__, __LINE__, nRet);

	// 修改身份证的分辨率，大->小
	width = atoi(sIDPicWidth);
	high = atoi(sIDPicHigh);
	// 根据路径载入大图片
	imSrc.Load(pSaveDesktopIDPicFilenm);
	if (imSrc.IsNull())
	{
		GtWriteTrace(30, "%s:%d: 分辨率转换时载入源图片失败 Load()", __FUNCTION__, __LINE__);
		return 110;
	}
	// 建立小图片
	if (!imDest.Create(width, high, 24))
	{
		GtWriteTrace(30, "%s:%d: 分辨率转换时建立目标图片失败 Create()", __FUNCTION__, __LINE__);
		return 111;
	}
	// 获取小图片HDC
	destDc = imDest.GetDC();
	destRect.SetRect(0, 0, width, high);
	// 设置图片不失真
	SetStretchBltMode(destDc, STRETCH_HALFTONE);
	imSrc.StretchBlt(destDc, destRect, SRCCOPY);
	imDest.ReleaseDC();
	HRESULT hResult = imDest.Save(pSaveDesktopIDPicFilenm);
	if(FAILED(hResult))
	{
		GtWriteTrace(30, "%s:%d: 分辨率转换时保存目标图片失败 Save()", __FUNCTION__, __LINE__);
		return 112;
	}
	return 0;
}


// 重写，环境摄像头获取身份证照片
int CCentOneCameraDlg::MySaveEnvPic(const char *pSaveEnvPicFilenm)
{
	GtWriteTrace(30, "%s:%d: 进入环境摄像头获取人像照片函数!", __FUNCTION__, __LINE__);
	int nRet = 0;
	char sEnvScanNo[32] = {0};
	char sEnvScanSize[32] = {0};
	// 当前程序运行路径
	CString sIniFilePath;
	sIniFilePath = GetAppPath() + "\\win.ini";
	// 临时字符串
	char sTmpString[32] = {0};
	// 高拍仪 环境摄像头 序号
	GetPrivateProfileString("CentermOneMachine", "EnvScanNo", "1", sTmpString, sizeof(sTmpString)-1, sIniFilePath);
	memcpy(sEnvScanNo, (const char*)sTmpString, sizeof(sEnvScanNo)-1);
	// 高拍仪 环境摄像头 分辨率级别
	GetPrivateProfileString("CentermOneMachine", "EnvScanSize", "5", sTmpString, sizeof(sTmpString)-1, sIniFilePath);
	memcpy(sEnvScanSize, (const char*)sTmpString, sizeof(sEnvScanSize)-1);

	GtWriteTrace(30, "%s:%d: 参数: sCentEnvScanNo=[%s], sCentEnvScanSize=[%s]", __FUNCTION__, __LINE__,
		sEnvScanNo, sEnvScanSize);

	if (mHighCamera == NULL)
	{
		pVideoWnd = (CStatic *)GetDlgItem(IDC_STATIC_CAMERA);
		mHighCamera = new CCentermIFImp(pVideoWnd);
	}
	nRet = mHighCamera->DetectDevice();
	GtWriteTrace(30, "%s:%d: DetectDevice nRet = %d", __FUNCTION__, __LINE__, nRet);
	// 打开摄像头
	nRet = mHighCamera->OpenDevice(atoi(sEnvScanNo));
	if (nRet != 0)
	{
		GtWriteTrace(30, "%s:%d: 打开摄像头 OpenDevice() nRet = %d", __FUNCTION__, __LINE__, nRet);
		return 105;
	}
	// 设置分辨率
	nRet = mHighCamera->SetScanSize(atoi(sEnvScanSize), atoi(sEnvScanNo));
	if (nRet != 0)
	{
		GtWriteTrace(30, "%s:%d: 设置分辨率 SetScanSize() nRet = %d", __FUNCTION__, __LINE__, nRet);
		mHighCamera->CloseDevice();
		return 106;
	}
	// 调高拍仪接口保存图片
	CString tmpDir = pSaveEnvPicFilenm;
	nRet = mHighCamera->ScanImage(tmpDir);
	if (nRet != 0)
	{
		GtWriteTrace(30, "%s:%d: 调高拍仪接口保存图片 ScanImage() nRet = %d", __FUNCTION__, __LINE__, nRet);
		mHighCamera->CloseDevice();
		return 108;
	}
	// 关闭设备
	nRet = mHighCamera->CloseDevice();
	GtWriteTrace(30, "%s:%d: 关闭设备返回值 CloseDevice() nRet = [%d]", __FUNCTION__, __LINE__, nRet);

	return 0;
}