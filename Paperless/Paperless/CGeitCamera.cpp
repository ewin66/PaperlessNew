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
	char sIDPicWidth[32] = {0};
	char sIDPicHigh[32] = {0};
	char sAutoCropWaitTime[32] = {0};
	// 当前程序运行路径
	CString sIniFilePath;
	sIniFilePath = GetFilePath() + "\\win.ini";
	// 临时字符串
	char sTmpString[32] = {0};
	// 高拍仪 文拍摄像头 序号
	GetPrivateProfileString("GeitCamera", "DeskScanNo", "0", sTmpString, sizeof(sTmpString)-1, sIniFilePath);
	memcpy(sDeskScanNo, (const char*)sTmpString, sizeof(sDeskScanNo)-1);
	// 高拍仪截取的身份证分辨率宽
	GetPrivateProfileString("GeitCamera", "IDPicWidth", "330", sTmpString, sizeof(sTmpString)-1, sIniFilePath);
	memcpy(sIDPicWidth, (const char*)sTmpString, sizeof(sIDPicWidth)-1);
	// 高拍仪截取的身份证分辨率高
	GetPrivateProfileString("GeitCamera", "IDPicHigh", "210", sTmpString, sizeof(sTmpString)-1, sIniFilePath);
	memcpy(sIDPicHigh, (const char*)sTmpString, sizeof(sIDPicHigh)-1);
	// 高拍仪设置自动框选区域等待时长，单位：ms（不等待直接获取图片可能无法框选）
	GetPrivateProfileString("GeitCamera", "AutoCropWaitTime", "100", sTmpString, sizeof(sTmpString)-1, sIniFilePath);
	memcpy(sAutoCropWaitTime, (const char*)sTmpString, sizeof(sAutoCropWaitTime)-1);

	GtWriteTrace(30, "%s:%d: 参数: sDeskScanNo=[%s], sIDPicWidth=[%s], sIDPicHigh=[%s], sAutoCropWaitTime=[%s]",
		__FUNCTION__, __LINE__, sDeskScanNo, sIDPicWidth, sIDPicHigh, sAutoCropWaitTime);

	int nFrameIndex = abs(atoi(sAutoCropWaitTime));
	if (nFrameIndex > 50)
	{
		GtWriteTrace(30, "%s:%d: 配置 AutoCropWaitTime 帧数 > 50 ", __FUNCTION__, __LINE__);
		return 105;
	}
	GtWriteTrace(30, "%s:%d: 开始获取摄像头照片完成", __FUNCTION__, __LINE__);

	//声明IplImage指针
	IplImage *pImgSrc = NULL;
	IplImage *pDest = NULL;
#if 0
	pImgSrc = cvLoadImage("C:\\Users\\Administrator\\Desktop\\test.jpg", CV_LOAD_IMAGE_ANYDEPTH | CV_LOAD_IMAGE_ANYCOLOR);
#else
	IplImage *pFrame = NULL;
	int nReadErrCount = 0;
	// 获取摄像头
	CvCapture* pCapture = cvCreateCameraCapture(atoi(sDeskScanNo));
	if (pCapture == NULL)
	{
		return 105;
	}
	// 设置摄像头分辨率
	cvSetCaptureProperty(pCapture , CV_CAP_PROP_FRAME_WIDTH , 1280);
	cvSetCaptureProperty(pCapture , CV_CAP_PROP_FRAME_HEIGHT, 960);

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
	GtWriteTrace(30, "%s:%d: 获取摄像头照片完成", __FUNCTION__, __LINE__);
	if (NULL == pFrame)
	{
		GtWriteTrace(30, "%s:%d: 获取摄像头照片失败 cvQueryFrame()", __FUNCTION__, __LINE__);
		cvReleaseCapture(&pCapture);
		return 108;
	}
	pImgSrc = cvCreateImage(cvSize(pFrame->width, pFrame->height), pFrame->depth, pFrame->nChannels);
	// 复制图像帧
	cvCopy(pFrame, pImgSrc, NULL); 
	// 释放摄像头
	cvReleaseCapture(&pCapture);
#endif

//	cvNamedWindow("pImgSrc", 1);
// 	cvShowImage("pImgSrc", pImgSrc);
// 	cvWaitKey(0);
// 	cvDestroyWindow("pImgSrc");

	if (pImgSrc == NULL)
	{
		GtWriteTrace(30, "%s:%d: 获取帧照片失败 cvQueryFrame()", __FUNCTION__, __LINE__);
		return 108;
	}
	// 框选图形内的身份证，并存到图像pDest中
	nRet = MyAutoCrop(pImgSrc, &pDest);
	char pSaveDesktopIDPicFilenm_1[256] = {0};
	sprintf_s(pSaveDesktopIDPicFilenm_1, sizeof(pSaveDesktopIDPicFilenm_1)-1, "%s\\IDPicture\\pic.jpg", GetAppPath().GetBuffer());
	cvSaveImage(pSaveDesktopIDPicFilenm_1, pImgSrc);
	GtWriteTrace(30, "%s:%d: 框选摄像头照片完成", __FUNCTION__, __LINE__);
	//nRet = 1;
	if (nRet == 0 && pDest != NULL)
	{
		cvSaveImage(pSaveDesktopIDPicFilenm, pDest);
		// 释放目标图像
		cvReleaseImage(&pDest);
	}
	else
	{
		// 无法找到身份证，保存原始图
		cvSaveImage(pSaveDesktopIDPicFilenm, pImgSrc);
	}

	GtWriteTrace(30, "%s:%d: 源图像获取完成，准备转换成小分辨率图片...", __FUNCTION__, __LINE__);
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
	//HRESULT hResult = 0;
	if(FAILED(hResult))
	{
		GtWriteTrace(30, "%s:%d: 分辨率转换时保存目标图片失败 Save()", __FUNCTION__, __LINE__);
		return 112;
	}
	GtWriteTrace(30, "%s:%d: 转换小分辨率图片完成.", __FUNCTION__, __LINE__);
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
	// 高拍仪设置自动框选区域等待时长，单位：ms（不等待直接获取图片可能无法框选）
	GetPrivateProfileString("GeitCamera", "AutoCropWaitTime", "5", sTmpString, sizeof(sTmpString)-1, sIniFilePath);
	memcpy(sAutoCropWaitTime, (const char*)sTmpString, sizeof(sAutoCropWaitTime)-1);

	GtWriteTrace(30, "%s:%d: 参数: sEnvScanNo=[%s], sAutoCropWaitTime=[%s]", __FUNCTION__, __LINE__, sEnvScanNo, sAutoCropWaitTime);


	//声明IplImage指针
	IplImage* pFrame = NULL;
	IplImage* pDestImg = NULL;
	// 获取摄像头
	CvCapture* pCapture = cvCreateCameraCapture(atoi(sEnvScanNo));
	if (pCapture == NULL)
	{
		return 105;
	}
	// 设置摄像头分辨率
	cvSetCaptureProperty(pCapture , CV_CAP_PROP_FRAME_WIDTH , 1600);
	cvSetCaptureProperty(pCapture , CV_CAP_PROP_FRAME_HEIGHT, 1200);

	// 等待时间
	Sleep(atoi(sAutoCropWaitTime));
	int nReadNullCount = 0;
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
		GtWriteTrace(30, "%s:%d: 获取摄像头照片失败 cvQueryFrame()", __FUNCTION__, __LINE__);
		cvReleaseCapture(&pCapture);
		return 108;
	}
// 	CString sAppPath = GetAppPath();
// 	sAppPath += "\\IDPicture\\EnvPictureTmp.jpg";
// 	pFrame = cvLoadImage(sAppPath.GetBuffer(), CV_LOAD_IMAGE_ANYDEPTH | CV_LOAD_IMAGE_ANYCOLOR);
	// 裁剪中心区域
	pDestImg = GetCentreOfImage1(pFrame, F_AREA_RATE, F_HEIGHT_WIDTH_RATE);
	if (pDestImg == NULL)
	{
		GtWriteTrace(30, "%s:%d: 裁剪人像中心区域失败！", __FUNCTION__, __LINE__);
		// 保存原始图
		cvSaveImage(pSaveEnvPicFilenm, pFrame);
	}
	else
	{
		GtWriteTrace(30, "%s:%d: 裁剪人像中心区域成功！", __FUNCTION__, __LINE__);
		// 保存裁剪的图像
		cvSaveImage(pSaveEnvPicFilenm, pDestImg);
		// 释放
		cvReleaseImage(&pDestImg);
	}

	// 释放摄像头
	cvReleaseCapture(&pCapture);
	return 0;
}