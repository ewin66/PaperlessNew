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


// ��д����������ͷ��ȡ���֤��Ƭ
int CGeitCamera::MySaveDeskIDPic(const char *pSaveDesktopIDPicFilenm)
{
	GtWriteTrace(30, "%s:%d: ������������ͷ��ȡ���֤��Ƭ����......", __FUNCTION__, __LINE__);
	int nRet = 0;
	CImage imSrc;
	CImage imDest;
	// С�ֱ���ͼƬHDC
	HDC destDc;
	// СͼƬ����
	CRect destRect;
	// ͼƬ�ؼ�����
	CRect picRect;
	// �ֱ���
	int width = 0;
	int high = 0;
	char sDeskScanNo[32] = {0};
	char sIDPicWidth[32] = {0};
	char sIDPicHigh[32] = {0};
	char sPictureFrameTimes[32] = {0};
	// ��ǰ��������·��
	CString sIniFilePath;
	sIniFilePath = GetFilePath() + "\\win.ini";
	// ������ ��������ͷ ���
	GetPrivateProfileString("GeitCamera", "DeskScanNo", "0", sDeskScanNo, sizeof(sDeskScanNo)-1, sIniFilePath);
	// �����ǽ�ȡ�����֤�ֱ��ʿ�
	GetPrivateProfileString("GeitCamera", "IDPicWidth", "330", sIDPicWidth, sizeof(sIDPicWidth)-1, sIniFilePath);
	// �����ǽ�ȡ�����֤�ֱ��ʸ�
	GetPrivateProfileString("GeitCamera", "IDPicHigh", "210", sIDPicHigh, sizeof(sIDPicHigh)-1, sIniFilePath);
	// ������ ��������ͷ �ڼ�֡��ȡ֤���գ�ֱ�ӻ�ȡͼƬ�����عⲻ��������λ��֡��
	GetPrivateProfileString("GeitCamera", "PictureFrameTimes", "12", sPictureFrameTimes, sizeof(sPictureFrameTimes)-1, sIniFilePath);

	GtWriteTrace(30, "%s:%d: \t����: sDeskScanNo=[%s], sIDPicWidth=[%s], sIDPicHigh=[%s], sPictureFrameTimes=[%s]",
		__FUNCTION__, __LINE__, sDeskScanNo, sIDPicWidth, sIDPicHigh, sPictureFrameTimes);

	int nFrameIndex = abs(atoi(sPictureFrameTimes));
	if (nFrameIndex > 50)
	{
		GtWriteTrace(30, "%s:%d: ���� AutoCropWaitTime ֡�� > 50 \n", __FUNCTION__, __LINE__);
		return 105;
	}

	//����IplImageָ��
	// �����ֶ��ͷţ�����������ͷ�ͷ�
	IplImage *pFrame = NULL;
	IplImage *pDest = NULL;
#if 0
	pFrame = cvLoadImage("C:\\Users\\Administrator\\Desktop\\test.jpg", CV_LOAD_IMAGE_ANYDEPTH | CV_LOAD_IMAGE_ANYCOLOR);
#else
	int nReadErrCount = 0;
	// ��ȡ����ͷ
	GtWriteTrace(30, "%s:%d: \t��ʼ������ͷ...", __FUNCTION__, __LINE__);
	CvCapture* pCapture = cvCreateCameraCapture(atoi(sDeskScanNo));
	if (pCapture == NULL)
	{
		return 105;
	}
	// ��������ͷ�ֱ���
	GtWriteTrace(30, "%s:%d: \t��ʼ��������ͷ�ֱ���...", __FUNCTION__, __LINE__);
	cvSetCaptureProperty(pCapture , CV_CAP_PROP_FRAME_WIDTH , 1280);
	cvSetCaptureProperty(pCapture , CV_CAP_PROP_FRAME_HEIGHT, 960);

	GtWriteTrace(30, "%s:%d: \t��ʼ��ȡ���֤��Ƭ...", __FUNCTION__, __LINE__);
	// �ȴ��ڼ�֡��ȡ��Ƭ
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
	GtWriteTrace(30, "%s:%d: \t��ȡ���֤��Ƭ��������", __FUNCTION__, __LINE__);
	if (NULL == pFrame)
	{
		GtWriteTrace(30, "%s:%d: ��ȡ����ͷ��Ƭʧ�� cvQueryFrame()\n", __FUNCTION__, __LINE__);
		cvReleaseCapture(&pCapture);
		return 108;
	}
#endif

	//����ʹ�ã���������ͷ��ȡ����Ƭ
// 	char pSaveDesktopIDPicFilenm_1[256] = {0};
// 	sprintf_s(pSaveDesktopIDPicFilenm_1, sizeof(pSaveDesktopIDPicFilenm_1)-1, "%s\\IDPicture\\pic.jpg", GetAppPath().GetBuffer());
// 	cvSaveImage(pSaveDesktopIDPicFilenm_1, pFrame);

	// ��ѡͼ���ڵ����֤�����浽ͼ��pDest��
	GtWriteTrace(30, "%s:%d: \t��ʼ��ѡ����ͷ��Ƭ......", __FUNCTION__, __LINE__);
	nRet = MyAutoCrop(pFrame, &pDest);
	GtWriteTrace(30, "%s:%d: \t��ѡ����ͷ��Ƭ��ɣ���ʼ������Ƭ...", __FUNCTION__, __LINE__);
	//nRet = 1;
	if (nRet == 0 && pDest != NULL)
	{
		cvSaveImage(pSaveDesktopIDPicFilenm, pDest);
		//cvSaveImage(pSaveDesktopIDPicFilenm_1, pDest);
		// �ͷ�Ŀ��ͼ��
		cvReleaseImage(&pDest);
		pDest = NULL;
	}
	else
	{
		// �޷��ҵ����֤������ԭʼͼ
		cvSaveImage(pSaveDesktopIDPicFilenm, pFrame);
		//cvSaveImage(pSaveDesktopIDPicFilenm_1, pFrame);
		if (pDest != NULL)
		{
			cvReleaseImage(&pDest);
			pDest = NULL;
		}
	}
	// �ͷ�����ͷ
	GtWriteTrace(30, "%s:%d: \t����ͷ��Ƭ��ȡ��ϣ���ʼ�ر�����ͷ...", __FUNCTION__, __LINE__);
	cvReleaseCapture(&pCapture);


//  	GtWriteTrace(30, "%s:%d: \t׼��ת����С�ֱ���ͼƬ...", __FUNCTION__, __LINE__);
//  	// �޸����֤�ķֱ��ʣ���->С
//  	width = atoi(sIDPicWidth);
//  	high = atoi(sIDPicHigh);
//  	// ����·�������ͼƬ
//  	imSrc.Load(pSaveDesktopIDPicFilenm);
//  	if (imSrc.IsNull())
//  	{
//  		GtWriteTrace(30, "%s:%d: �ֱ���ת��ʱ����ԴͼƬʧ�� Load()\n", __FUNCTION__, __LINE__);
//  		return 110;
//  	}
//  	// ����СͼƬ
// 	if (!imDest.Create(width, high, 24))
//  	{
//  		GtWriteTrace(30, "%s:%d: �ֱ���ת��ʱ����Ŀ��ͼƬʧ�� Create()\n", __FUNCTION__, __LINE__);
//  		return 111;
//  	}
//  	// ��ȡСͼƬHDC
//  	destDc = imDest.GetDC();
//  	destRect.SetRect(0, 0, width, high);
//  	// ����ͼƬ��ʧ��
//  	SetStretchBltMode(destDc, STRETCH_HALFTONE);
//  	imSrc.StretchBlt(destDc, destRect, SRCCOPY);
//  	imDest.ReleaseDC();
//  	HRESULT hResult = imDest.Save(pSaveDesktopIDPicFilenm);
//  	//HRESULT hResult = 0;
//  	if(FAILED(hResult))
//  	{
//  		GtWriteTrace(30, "%s:%d: �ֱ���ת��ʱ����Ŀ��ͼƬʧ�� Save()\n", __FUNCTION__, __LINE__);
//  		return 112;
//  	}
//  	GtWriteTrace(30, "%s:%d: ת��С�ֱ���ͼƬ���.\n", __FUNCTION__, __LINE__);
	GtWriteTrace(30, "%s:%d: ��������ͷ��ȡ���֤��Ƭ���������˳�.\n", __FUNCTION__, __LINE__);
	return 0;
}

// ��д����������ͷ��ȡ������Ƭ
int CGeitCamera::MySaveEnvPic(const char *pSaveEnvPicFilenm)
{
	GtWriteTrace(30, "%s:%d: ���뻷������ͷ��ȡ������Ƭ����!", __FUNCTION__, __LINE__);
	int nRet = 0;
	char sEnvScanNo[32] = {0};
	char sAutoCropWaitTime[32] = {0};
	// ��ǰ��������·��
	CString sIniFilePath;
	sIniFilePath = GetFilePath() + "\\win.ini";
	// ��ʱ�ַ���
	char sTmpString[32] = {0};
	// ������ ��������ͷ ���
	GetPrivateProfileString("GeitCamera", "EnvScanNo", "0", sTmpString, sizeof(sTmpString)-1, sIniFilePath);
	memcpy(sEnvScanNo, (const char*)sTmpString, sizeof(sEnvScanNo)-1);
	// ������ ��������ͷ ��ȡ����ĳ�ʱʱ�䣨��λ��ms��
	GetPrivateProfileString("GeitCamera", "AutoCropWaitTime", "20", sTmpString, sizeof(sTmpString)-1, sIniFilePath);
	memcpy(sAutoCropWaitTime, (const char*)sTmpString, sizeof(sAutoCropWaitTime)-1);

	GtWriteTrace(30, "%s:%d: \t����: sEnvScanNo=[%s], sAutoCropWaitTime=[%s]", __FUNCTION__, __LINE__, sEnvScanNo, sAutoCropWaitTime);


	//����IplImageָ��
	IplImage* pFrame = NULL;
	IplImage* pDestImg = NULL;
	GtWriteTrace(30, "%s:%d: \t��ʼ������ͷ...", __FUNCTION__, __LINE__);
	// ��ȡ����ͷ
	CvCapture* pCapture = cvCreateCameraCapture(atoi(sEnvScanNo));
	if (pCapture == NULL)
	{
		return 105;
	}
	// ��������ͷ�ֱ���
	GtWriteTrace(30, "%s:%d: \t��ʼ��������ͷ�ֱ���...", __FUNCTION__, __LINE__);
	cvSetCaptureProperty(pCapture , CV_CAP_PROP_FRAME_WIDTH , 1280);
	cvSetCaptureProperty(pCapture , CV_CAP_PROP_FRAME_HEIGHT, 960);

	// �ȴ�ʱ��
	GtWriteTrace(30, "%s:%d: \t��ʼ�ȴ� %s �����ع�ʱ��...", __FUNCTION__, __LINE__, sAutoCropWaitTime);
	Sleep(atoi(sAutoCropWaitTime));
	int nReadNullCount = 0;
	GtWriteTrace(30, "%s:%d: \t��ʼ��ȡ������Ƭ...", __FUNCTION__, __LINE__, sAutoCropWaitTime);
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
		GtWriteTrace(30, "%s:%d: ��ȡ����ͷ��Ƭʧ�� cvQueryFrame()\n", __FUNCTION__, __LINE__);
		// �ر�����ͷ
		cvReleaseCapture(&pCapture);
		return 108;
	}
// 	CString sAppPath = GetAppPath();
// 	sAppPath += "\\IDPicture\\EnvPictureTmp.jpg";
// 	pFrame = cvLoadImage(sAppPath.GetBuffer(), CV_LOAD_IMAGE_ANYDEPTH | CV_LOAD_IMAGE_ANYCOLOR);
	// �ü���������
	GtWriteTrace(30, "%s:%d: \t��ʼ��ȡ��������������...", __FUNCTION__, __LINE__, sAutoCropWaitTime);
	pDestImg = GetCentreOfImage(pFrame, F_AREA_RATE, F_HEIGHT_WIDTH_RATE);
	if (pDestImg == NULL)
	{
		GtWriteTrace(30, "%s:%d: \t�ü�������������ʧ�ܣ�\n", __FUNCTION__, __LINE__);
		// ����ԭʼͼ
		cvSaveImage(pSaveEnvPicFilenm, pFrame);
	}
	else
	{
		GtWriteTrace(30, "%s:%d: \t�ü�������������ɹ�", __FUNCTION__, __LINE__);
		// ����ü���ͼ��
		cvSaveImage(pSaveEnvPicFilenm, pDestImg);
		// �ͷ�
		cvReleaseImage(&pDestImg);
	}

	// �ͷ�����ͷ
	GtWriteTrace(30, "%s:%d: \t��ʼ�ر�����ͷ...", __FUNCTION__, __LINE__, sAutoCropWaitTime);
	cvReleaseCapture(&pCapture);
	GtWriteTrace(30, "%s:%d: ��������ͷ��ȡ������Ƭ���������˳���\n", __FUNCTION__, __LINE__);
	return 0;
}