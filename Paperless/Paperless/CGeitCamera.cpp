#include "stdafx.h"
#include <highgui.h>
#include <opencv2/legacy/legacy.hpp>
#include <math.h>
#include "cv.h"
#include "cxcore.h"

#include "CGeitCamera.h"
#include "MyTTrace.h"
#include "utils.h"
#include "opencv/OpencvUtils.h"


CGeitCamera::CGeitCamera()
{

}


CGeitCamera::~CGeitCamera()
{

}


// 重写，文拍摄像头获取身份证照片
int CGeitCamera::MySaveDeskIDPic(const char *pSaveDesktopIDPicFilenm)
{
	GtWriteTrace(30, "%s:%d: 进入文拍摄像头获取身份证照片函数......", __FUNCTION__, __LINE__);
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
	char sIDPicWidth[32] = {0};
	char sIDPicHigh[32] = {0};
	char sPictureFrameTimes[32] = {0};
	// 当前程序运行路径
	CString sIniFilePath;
	sIniFilePath = GetFilePath() + "\\win.ini";
	// 高拍仪 文拍摄像头 序号
	GetPrivateProfileString("GeitCamera", "DeskScanNo", "0", sDeskScanNo, sizeof(sDeskScanNo)-1, sIniFilePath);
	// 高拍仪截取的身份证分辨率宽
	GetPrivateProfileString("GeitCamera", "IDPicWidth", "330", sIDPicWidth, sizeof(sIDPicWidth)-1, sIniFilePath);
	// 高拍仪截取的身份证分辨率高
	GetPrivateProfileString("GeitCamera", "IDPicHigh", "210", sIDPicHigh, sizeof(sIDPicHigh)-1, sIniFilePath);
	// 高拍仪 文拍摄像头 第几帧获取证件照，直接获取图片可能曝光不正常（单位：帧）
	GetPrivateProfileString("GeitCamera", "PictureFrameTimes", "12", sPictureFrameTimes, sizeof(sPictureFrameTimes)-1, sIniFilePath);

	GtWriteTrace(30, "%s:%d: \t参数: sDeskScanNo=[%s], sIDPicWidth=[%s], sIDPicHigh=[%s], sPictureFrameTimes=[%s]",
		__FUNCTION__, __LINE__, sDeskScanNo, sIDPicWidth, sIDPicHigh, sPictureFrameTimes);

	int nFrameIndex = abs(atoi(sPictureFrameTimes));
	if (nFrameIndex > 50)
	{
		GtWriteTrace(30, "%s:%d: 配置 AutoCropWaitTime 帧数 > 50 \n", __FUNCTION__, __LINE__);
		return 105;
	}

	//声明IplImage指针
	// 不需手动释放，其依赖摄像头释放
	IplImage *pFrame = NULL;
	IplImage *pDest = NULL;
#if 0
	pFrame = cvLoadImage("C:\\Users\\Administrator\\Desktop\\test.jpg", CV_LOAD_IMAGE_ANYDEPTH | CV_LOAD_IMAGE_ANYCOLOR);
#else
	int nReadErrCount = 0;
	// 获取摄像头
	GtWriteTrace(30, "%s:%d: \t开始打开摄像头...", __FUNCTION__, __LINE__);
	CvCapture* pCapture = cvCreateCameraCapture(atoi(sDeskScanNo));
	if (pCapture == NULL)
	{
		return 105;
	}
	// 设置摄像头分辨率
	GtWriteTrace(30, "%s:%d: \t开始设置摄像头分辨率...", __FUNCTION__, __LINE__);
	cvSetCaptureProperty(pCapture , CV_CAP_PROP_FRAME_WIDTH , 1280);
	cvSetCaptureProperty(pCapture , CV_CAP_PROP_FRAME_HEIGHT, 960);

	GtWriteTrace(30, "%s:%d: \t开始获取身份证照片...", __FUNCTION__, __LINE__);
	// 等待第几帧获取照片
//	Sleep(atoi(sAutoCropWaitTime));
	int nReadFrame = 0;
	int nReadNullCount = 0;
	while (1)
	{
		nReadFrame++;
		pFrame = cvQueryFrame( pCapture );
		if (pFrame == NULL)
		{
			nReadNullCount++;
			if (nReadNullCount > 6)
			{
				break;
			}
		}
		if (nReadFrame > nFrameIndex)
		{
			break;
		}
		//cvWaitKey(5);
	}
	GtWriteTrace(30, "%s:%d: \t获取身份证照片函数结束", __FUNCTION__, __LINE__);
	if (NULL == pFrame)
	{
		GtWriteTrace(30, "%s:%d: 获取摄像头照片失败 cvQueryFrame()\n", __FUNCTION__, __LINE__);
		cvReleaseCapture(&pCapture);
		return 108;
	}
#endif

	//测试使用，保存摄像头获取的照片
// 	char pSaveDesktopIDPicFilenm_1[256] = {0};
// 	sprintf_s(pSaveDesktopIDPicFilenm_1, sizeof(pSaveDesktopIDPicFilenm_1)-1, "%s\\IDPicture\\pic.jpg", GetAppPath().GetBuffer());
// 	cvSaveImage(pSaveDesktopIDPicFilenm_1, pFrame);

	// 框选图形内的身份证，并存到图像pDest中
	GtWriteTrace(30, "%s:%d: \t开始框选摄像头照片......", __FUNCTION__, __LINE__);
	nRet = MyAutoCrop(pFrame, &pDest);
	GtWriteTrace(30, "%s:%d: \t框选摄像头照片完成，开始保存照片...", __FUNCTION__, __LINE__);
	//nRet = 1;
	if (nRet == 0 && pDest != NULL)
	{
		cvSaveImage(pSaveDesktopIDPicFilenm, pDest);
		//cvSaveImage(pSaveDesktopIDPicFilenm_1, pDest);
		// 释放目标图像
		cvReleaseImage(&pDest);
		pDest = NULL;
	}
	else
	{
		// 无法找到身份证，保存原始图
		cvSaveImage(pSaveDesktopIDPicFilenm, pFrame);
		//cvSaveImage(pSaveDesktopIDPicFilenm_1, pFrame);
		if (pDest != NULL)
		{
			cvReleaseImage(&pDest);
			pDest = NULL;
		}
	}
	// 释放摄像头
	GtWriteTrace(30, "%s:%d: \t摄像头照片获取完毕，开始关闭摄像头...", __FUNCTION__, __LINE__);
	cvReleaseCapture(&pCapture);


//  	GtWriteTrace(30, "%s:%d: \t准备转换成小分辨率图片...", __FUNCTION__, __LINE__);
//  	// 修改身份证的分辨率，大->小
//  	width = atoi(sIDPicWidth);
//  	high = atoi(sIDPicHigh);
//  	// 根据路径载入大图片
//  	imSrc.Load(pSaveDesktopIDPicFilenm);
//  	if (imSrc.IsNull())
//  	{
//  		GtWriteTrace(30, "%s:%d: 分辨率转换时载入源图片失败 Load()\n", __FUNCTION__, __LINE__);
//  		return 110;
//  	}
//  	// 建立小图片
// 	if (!imDest.Create(width, high, 24))
//  	{
//  		GtWriteTrace(30, "%s:%d: 分辨率转换时建立目标图片失败 Create()\n", __FUNCTION__, __LINE__);
//  		return 111;
//  	}
//  	// 获取小图片HDC
//  	destDc = imDest.GetDC();
//  	destRect.SetRect(0, 0, width, high);
//  	// 设置图片不失真
//  	SetStretchBltMode(destDc, STRETCH_HALFTONE);
//  	imSrc.StretchBlt(destDc, destRect, SRCCOPY);
//  	imDest.ReleaseDC();
//  	HRESULT hResult = imDest.Save(pSaveDesktopIDPicFilenm);
//  	//HRESULT hResult = 0;
//  	if(FAILED(hResult))
//  	{
//  		GtWriteTrace(30, "%s:%d: 分辨率转换时保存目标图片失败 Save()\n", __FUNCTION__, __LINE__);
//  		return 112;
//  	}
//  	GtWriteTrace(30, "%s:%d: 转换小分辨率图片完成.\n", __FUNCTION__, __LINE__);
	GtWriteTrace(30, "%s:%d: 文拍摄像头获取身份证照片函数正常退出.\n", __FUNCTION__, __LINE__);
	return 0;
}

// 重写，环境摄像头获取人像照片
int CGeitCamera::MySaveEnvPic(const char *pSaveEnvPicFilenm)
{
	GtWriteTrace(30, "%s:%d: 进入环境摄像头获取人像照片函数!", __FUNCTION__, __LINE__);
	int nRet = 0;
	char sEnvScanNo[32] = {0};
	char sAutoCropWaitTime[32] = {0};
	// 当前程序运行路径
	CString sIniFilePath;
	sIniFilePath = GetFilePath() + "\\win.ini";
	// 临时字符串
	char sTmpString[32] = {0};
	// 高拍仪 文拍摄像头 序号
	GetPrivateProfileString("GeitCamera", "EnvScanNo", "0", sTmpString, sizeof(sTmpString)-1, sIniFilePath);
	memcpy(sEnvScanNo, (const char*)sTmpString, sizeof(sEnvScanNo)-1);
	// 高拍仪 环境摄像头 获取人像的超时时间（单位：ms）
	GetPrivateProfileString("GeitCamera", "AutoCropWaitTime", "20", sTmpString, sizeof(sTmpString)-1, sIniFilePath);
	memcpy(sAutoCropWaitTime, (const char*)sTmpString, sizeof(sAutoCropWaitTime)-1);

	GtWriteTrace(30, "%s:%d: \t参数: sEnvScanNo=[%s], sAutoCropWaitTime=[%s]", __FUNCTION__, __LINE__, sEnvScanNo, sAutoCropWaitTime);


	//声明IplImage指针
	IplImage* pFrame = NULL;
	IplImage* pDestImg = NULL;
	GtWriteTrace(30, "%s:%d: \t开始打开摄像头...", __FUNCTION__, __LINE__);
	// 获取摄像头
	CvCapture* pCapture = cvCreateCameraCapture(atoi(sEnvScanNo));
	if (pCapture == NULL)
	{
		return 105;
	}
	// 设置摄像头分辨率
	GtWriteTrace(30, "%s:%d: \t开始设置摄像头分辨率...", __FUNCTION__, __LINE__);
	cvSetCaptureProperty(pCapture , CV_CAP_PROP_FRAME_WIDTH , 1280);
	cvSetCaptureProperty(pCapture , CV_CAP_PROP_FRAME_HEIGHT, 960);

	// 等待时间
	GtWriteTrace(30, "%s:%d: \t开始等待 %s 毫秒曝光时间...", __FUNCTION__, __LINE__, sAutoCropWaitTime);
	Sleep(atoi(sAutoCropWaitTime));
	int nReadNullCount = 0;
	GtWriteTrace(30, "%s:%d: \t开始获取人像照片...", __FUNCTION__, __LINE__, sAutoCropWaitTime);
	pFrame = cvQueryFrame( pCapture );
	while (NULL == pFrame)
	{
		nReadNullCount++;
		if (nReadNullCount > 6)
		{
			break;
		}
		pFrame=cvQueryFrame( pCapture );
	}
	if (NULL == pFrame)
	{
		GtWriteTrace(30, "%s:%d: 获取摄像头照片失败 cvQueryFrame()\n", __FUNCTION__, __LINE__);
		// 关闭摄像头
		cvReleaseCapture(&pCapture);
		return 108;
	}
// 	CString sAppPath = GetAppPath();
// 	sAppPath += "\\IDPicture\\EnvPictureTmp.jpg";
// 	pFrame = cvLoadImage(sAppPath.GetBuffer(), CV_LOAD_IMAGE_ANYDEPTH | CV_LOAD_IMAGE_ANYCOLOR);
	// 裁剪中心区域
	GtWriteTrace(30, "%s:%d: \t开始截取人像照中心区域...", __FUNCTION__, __LINE__, sAutoCropWaitTime);
	pDestImg = GetCentreOfImage(pFrame, F_AREA_RATE, F_HEIGHT_WIDTH_RATE);
	if (pDestImg == NULL)
	{
		GtWriteTrace(30, "%s:%d: \t裁剪人像中心区域失败！\n", __FUNCTION__, __LINE__);
		// 保存原始图
		cvSaveImage(pSaveEnvPicFilenm, pFrame);
	}
	else
	{
		GtWriteTrace(30, "%s:%d: \t裁剪人像中心区域成功", __FUNCTION__, __LINE__);
		// 保存裁剪的图像
		cvSaveImage(pSaveEnvPicFilenm, pDestImg);
		// 释放
		cvReleaseImage(&pDestImg);
	}

	// 释放摄像头
	GtWriteTrace(30, "%s:%d: \t开始关闭摄像头...", __FUNCTION__, __LINE__, sAutoCropWaitTime);
	cvReleaseCapture(&pCapture);
	GtWriteTrace(30, "%s:%d: 环境摄像头获取人像照片函数正常退出。\n", __FUNCTION__, __LINE__);
	return 0;
}