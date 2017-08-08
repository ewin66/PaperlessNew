#include "OpencvUtils.h"
#include "../MyTTrace.h"
#include <math.h>


/** 
* 功能：自动框选出原图内的身份证
* 参数：pImgSrc：待提取的原图像
*		pImgDest：提取出的身份证图像，必须手动释放
* 返回值：0-成功，其他-失败
**/
int MyAutoCrop(IplImage *pImgSrc, IplImage **pImgDest)
{
#define SHOW_IMAGE 0
	GtWriteTrace(30, "%s:%d: 进入到身份证框选函数！", __FUNCTION__, __LINE__);
	if (pImgSrc == NULL)
	{
		GtWriteTrace(30, "%s:%d: 手动裁边函数失败，原图像为NULL", __FUNCTION__, __LINE__);
		return -1;
	}
	// 检测到的轮廓中的最大面积大小
	double lMaxArea = 0;
	int nMaxAreaIndex = 0;
	// 原图像Mat，使用Mat是为了提高图像处理效率，不需手动释放
	cv::Mat srcImg;
	// 灰度图像Mat，新的图像
	cv::Mat grayImg;
	// 均值滤波图像，浅拷贝，在灰度图上直接处理
	cv::Mat avgFilterImg;
	// 二值化图像
	cv::Mat binImg;
	// 容器，检测的轮廓数组
	std::vector<std::vector<cv::Point> > contours;
	std::vector<cv::Vec4i> hierarchy;
	// 轮廓遍历位置
	int index = 0;
	// 第一次裁剪的图像，即找到轮廓正常外接矩形裁剪的图像
	IplImage* pFirstCutImg = NULL;
	// 旋转后的图像，在第一次裁剪基础上，不进行缩放旋转水平偏移角得到
	IplImage* pRemoteImg = NULL;

	// IplImage* 转 cv::Mat，只做浅拷贝，因为只需读取原图数据
	srcImg = pImgSrc;
#if SHOW_IMAGE
	cv::imshow("原图像", srcImg);
	cvWaitKey(0);
	cv::destroyWindow("原图像");
#endif
	// 灰度处理
	cvtColor(srcImg, grayImg, CV_RGB2GRAY);
#if SHOW_IMAGE
	imshow("灰度图", grayImg);
	cvWaitKey(0);
	cv::destroyWindow("灰度图");
#endif
	// 3*3均值滤波图像，从灰度图上做浅拷贝，与灰度图指向同一块数据，之后做均值滤波处理
	avgFilterImg = grayImg;
	blur(grayImg, avgFilterImg, cv::Size(3, 3));
#if SHOW_IMAGE
	imshow("均值滤波图像", avgFilterImg);
	cvWaitKey(0);
	cv::destroyWindow("均值滤波图像");
#endif
	// 二值化图像，从均值滤波图像上做浅拷贝，与均值滤波图像指向同一块数据，之后做二值化处理
	binImg = avgFilterImg;
	threshold(avgFilterImg, binImg, 0, 255, CV_THRESH_OTSU);
#if SHOW_IMAGE 
	imshow("二值化图像", avgFilterImg);
	cvWaitKey(0);
	cv::destroyWindow("二值化图像");
#endif

	// 边缘检测处理
	cv::findContours(binImg, contours, hierarchy, cv::RETR_EXTERNAL, cv::CHAIN_APPROX_NONE);
	// 判断是否检测到轮廓
	if (contours.size() == 0 || hierarchy.size() == 0)
	{
		GtWriteTrace(30, "%s:%d: 未检测到轮廓图，111111！", __FUNCTION__, __LINE__);
		return -2;
	}
	// 遍历轮廓，找到最大面积的轮廓
	for (; index>=0; index = hierarchy[index][0])
	{
		double lArea = cv::contourArea(contours[index]);
		if (lMaxArea < lArea)
		{
			// 保存最大面积值和在vectot轮廓组中的位置
			lMaxArea = lArea;
			nMaxAreaIndex = index;
		}
	}
	// 再次判断是否检测到轮廓
	if (index == 0 && contours.size() == 0 && hierarchy.size() == 0)
	{
		// 未检测到轮廓
		GtWriteTrace(30, "%s:%d: 未检测到轮廓图，222222！", __FUNCTION__, __LINE__);
		return -1;
	}
	double lArea = cv::contourArea(contours[nMaxAreaIndex]);
	if (lMaxArea == lArea)
	{
		GtWriteTrace(30, "%s:%d: 正常检测到最大轮廓，面积=[%.6lf]！\n", __FUNCTION__, __LINE__, lMaxArea);
		// 获取正常外接矩形
		cv::Rect normalRect=cv::boundingRect(contours[nMaxAreaIndex]);

#if SHOW_IMAGE 
		// 绘制轮廓及正常外接矩形，测试使用
		cv::Mat dstImg = srcImg.clone();
		cv::Scalar color1(rand()&255, rand()&255, rand()&255);
		cv::Scalar color2(rand()&255, rand()&255, rand()&255);
		//cv::Mat dstImg = cv::Mat::zeros(srcImg.rows, srcImg.cols, CV_8UC3);
		cv::drawContours(dstImg, contours, index, color2, 1, 8, hierarchy);
		// cvRectangle函数参数： 图片， 左上角， 右下角， 颜色， 线条粗细， 线条类型，点类型    
		rectangle(dstImg, normalRect.tl(), normalRect.br(), color1, 1, 4, 0 );
		imshow("轮廓及正常外接矩形", dstImg);
		cvWaitKey(0);
		cv::destroyWindow("轮廓及正常外接矩形");
#endif

		// 获取最小外接矩形
		cv::RotatedRect minRect  = cv::minAreaRect(contours[nMaxAreaIndex]);
		float minRectWidth = minRect.size.width;
		float minRectHeight = minRect.size.height;
		GtWriteTrace(30, "%s:%d: 最小外接矩形，水平偏移角=[%.6lf] 大小=[%f][%f]！\n", __FUNCTION__, __LINE__, minRect.angle, minRectWidth, minRectHeight);

		// 创建第一裁剪的图像
		pFirstCutImg = cvCreateImage(cvSize(normalRect.width, normalRect.height), pImgSrc->depth, pImgSrc->nChannels);
		// 设置原图像ROI，第一次裁剪正常外接矩形
		cvSetImageROI(pImgSrc, normalRect);
		// 裁剪图像（复制图像）
		cvCopy(pImgSrc, pFirstCutImg, NULL);
		// 原图像用完后，清空ROI
		cvResetImageROI(pImgSrc);
#if SHOW_IMAGE 
		cvShowImage("第一次裁剪后的图像", pFirstCutImg);
		cvWaitKey(0);
		cvDestroyWindow("第一次裁剪后的图像");
#endif
		// 待旋转的角度，取绝对值
		float degree = fabs(minRect.angle);
		// 判断旋转的方向
		if (((90 - degree) < 0.0001) || ((degree - 0) < 0.0001))
		{
			// 角度指宽那条边和水平夹角
			// 如果是0度，且宽<高，旋转90度；如果是90度，且宽>高，旋转90度
			if ( ((minRectWidth < minRectHeight) && ((degree - 0) < 0.0001)) || 
				((minRectWidth > minRectHeight) && ((90 - degree) < 0.0001)) )
			{
				degree = -90;
				pRemoteImg = rotateImage(pFirstCutImg, degree);
			}
			else
			{
				//pRemoteImg = pFirstCutImg;
				pRemoteImg = cvCloneImage(pFirstCutImg);
			}
		}
		else
		{
			// 在 0~90 度之间
			// 通过矩形宽度和高度判断旋转方向
			if (minRectWidth < minRectHeight)
			{
				degree = degree - 90;
				pRemoteImg = rotateImage(pFirstCutImg, degree);
			}
			else
			{
				pRemoteImg = rotateImage(pFirstCutImg, degree);
			}
		}
		// 判断图像旋转是否失败是否成功
		if (pRemoteImg == NULL)
		{
			GtWriteTrace(30, "%s:%d: 图像旋转处理失败！\n", __FUNCTION__, __LINE__);
			goto end;
		}
		// 释放第一次裁剪的图像
		if (pFirstCutImg != NULL)
		{
			cvReleaseImage(&pFirstCutImg);
			pFirstCutImg = NULL;
		}
#if SHOW_IMAGE
		cvShowImage("旋转后的图像", pRemoteImg);
		cvWaitKey(0);
		cvDestroyWindow("旋转后的图像");
#endif
		// 最小外接矩形的真正宽、高（宽 > 高），也就是第二次裁剪图像的宽和高
		float saveWidth = minRectWidth > minRectHeight ? minRectWidth : minRectHeight;
		float saveHeight = minRectWidth < minRectHeight ? minRectWidth : minRectHeight;
		// 创建第二次裁剪的图像（结果图像）
		*pImgDest = cvCreateImage(cvSize((int)saveWidth, (int)saveHeight), pRemoteImg->depth, pRemoteImg->nChannels);
		// 裁剪图像的大小
		CvRect cutRect = cvRect( (pRemoteImg->width - (int)saveWidth) / 2, (pRemoteImg->height - (int)saveHeight) / 2,
			(int)saveWidth, (int)saveHeight);
		// 设置ROI
		cvSetImageROI(pRemoteImg, cutRect);
		// 第二次裁剪得到结果图像（ROI复制）
		cvCopy(pRemoteImg, *pImgDest, NULL);
		// 清空ROI
		cvResetImageROI(pRemoteImg);
		// 释放旋转后的图像
		if (pRemoteImg != NULL)
		{
			cvReleaseImage(&pRemoteImg);
			pRemoteImg = NULL;
		}
		//cvSaveImage("C:\\Users\\Administrator\\Desktop\\dest1.jpg", pImgDest);
#if SHOW_IMAGE
		cvShowImage("保存后的图像", *pImgDest);
		cvWaitKey(0);
		cvDestroyWindow("保存后的图像");
#endif
	}
end:
	// 释放内存
	if (pFirstCutImg != NULL)
	{
		cvReleaseImage(&pFirstCutImg);
		pFirstCutImg = NULL;
	}
	if (pRemoteImg != NULL)
	{
		cvReleaseImage(&pRemoteImg);
		pRemoteImg = NULL;
	}
	GtWriteTrace(30, "%s:%d: 框选图形内的身份证处理结束！\n", __FUNCTION__, __LINE__);
	return 0;
}

/** 
* 功能：不改变大小，旋转图像
* 参数：pImgSrc：待旋转的原图像
*		degree：待旋转的角度（弧度制），正数表示逆时针，负数表示顺时针
* 返回值：旋转后的图像（使用后需要手动释放），为NULL表示失败
**/
IplImage* rotateImage(IplImage* pImgSrc, float lDegree)
{
	// 弧度制角转换成角度制
	double lAngle = lDegree  * CV_PI / 180.;
	// 角度的sin和cos值
	double lSinAngle = sin(lAngle);
	double lCosAngle = cos(lAngle);
	// 原图的宽和高
	int nWidth = pImgSrc->width;
	int nHeight = pImgSrc->height;
	//旋转后的新图尺寸
	int nWidthRotate= int(nHeight * fabs(lSinAngle) + nWidth * fabs(lCosAngle));
	int nHeightRotate=int(nWidth * fabs(lSinAngle) + nHeight * fabs(lCosAngle));
	// 建立旋转后的图像
	IplImage* pImgRotate = cvCreateImage(cvSize(nWidthRotate, nHeightRotate), pImgSrc->depth, pImgSrc->nChannels);
	cvZero(pImgRotate);
	// 保证原图可以任意角度旋转的最小尺寸
	int tempLength = (int)sqrt((double)nWidth * nWidth + (double)nHeight * nHeight) + 10;
	int tempX = (tempLength + 1) / 2 - nWidth / 2;
	int tempY = (tempLength + 1) / 2 - nHeight / 2;
	// 建立临时图像
	IplImage* pImgTemp = cvCreateImage(cvSize(tempLength, tempLength), pImgSrc->depth, pImgSrc->nChannels);
	cvZero(pImgTemp);

	// 将原图复制到临时图像tmp中心
	cvSetImageROI(pImgTemp, cvRect(tempX, tempY, nWidth, nHeight));
	cvCopy(pImgSrc, pImgTemp, NULL);
	cvResetImageROI(pImgTemp);

	//旋转数组map
	// [ m0  m1  m2 ] ===>  [ A11  A12   b1 ]
	// [ m3  m4  m5 ] ===>  [ A21  A22   b2 ]
	float m[6];
	int w = pImgTemp->width;
	int h = pImgTemp->height;
	m[0] = (float)lCosAngle;
	m[1] = (float)lSinAngle;
	m[3] = -m[1];
	m[4] = m[0];
	// 将旋转中心移至图像中间
	m[2] = w * 0.5f;
	m[5] = h * 0.5f;
	CvMat M = cvMat(2, 3, CV_32F, m);
	// 
	cvGetQuadrangleSubPix(pImgTemp, pImgRotate, &M);
	cvReleaseImage(&pImgTemp);

	return pImgRotate;
}


// 四舍五入
int round_double(float number)
{
	return (int)((number > 0.0) ? (number + 0.5) : (number - 0.5)); 
}


/** 
* 功能：截取图像中心区域，按面积和宽高比例截取，图像从图片文件中获取
* 参数：pFilePath：待旋转的原图像的路径
*		fAreaRate：面积比例，大面积/小面积，比例需大于1
*		fHeightWidthRate：高宽比，高/宽，比例需大于1
* 返回值：截取后的图像，为NULL表示失败，不为空需要手动释放
**/
IplImage* GetCentreOfImage2(const char *pFilePath, float fAreaRate, float fHeightWidthRate)
{
	IplImage *pSrcImg = cvLoadImage(pFilePath, CV_LOAD_IMAGE_ANYDEPTH | CV_LOAD_IMAGE_ANYCOLOR);
	if (pSrcImg == NULL)
	{
		return NULL;
	}
	IplImage *pDestImg = GetCentreOfImage1(pSrcImg, fAreaRate, fHeightWidthRate);
	if (pDestImg == NULL)
	{
		cvReleaseImage(&pSrcImg);
		return NULL;
	}
	return pDestImg;
}


/** 
* 功能：截取图像中心区域，按面积和宽高比例截取，图像从image中获取
* 参数：pImgSrc：待截取的原图像
*		fAreaRate：面积比例，大面积/小面积，比例需大于1
*		fHeightWidthRate：高宽比，高/宽，比例需大于1
* 返回值：截取后的图像，为NULL表示失败，不为空需要手动释放
**/
IplImage* GetCentreOfImage1(IplImage* pImgSrc, float fAreaRate, float fHeightWidthRate)
{
	if (pImgSrc == NULL || fAreaRate <= 1 || fHeightWidthRate <= 1)
	{
		return NULL;
	}

	IplImage *pImgDest = NULL;
	// 大图像面积
	long dBigArea = 0;
	// 小图像面积
	double dLittleArea = 0;
	// 小图像倍率
	float fTimes = 0;
	int nWidth = 0;
	int nHeight = 0;
	int nX = 0;
	int nY = 0;

	// 计算大面积
	dBigArea = pImgSrc->height * pImgSrc->width;
	// 计算小面积
	dLittleArea = dBigArea / fAreaRate;
	// 通过面积与宽高比计算倍率
	fTimes = (float)sqrt(dLittleArea / fHeightWidthRate);
	nWidth = round_double(fTimes);
	nHeight = round_double(fTimes * fHeightWidthRate);
	GtWriteTrace(30, "%s:%d: 原图像宽=[%d], 高=[%d]，第一次计算的宽=[%d], 高=[%d]！", __FUNCTION__, __LINE__,
		pImgSrc->width, pImgSrc->height, nWidth, nHeight);

	// 判断高、宽是否超过原图像的高、宽，计算待截取的区域
	if (nWidth > pImgSrc->width)
	{
		GtWriteTrace(30, "%s:%d: 计算的宽[%d] > 原图像宽=[%d]！", __FUNCTION__, __LINE__, nWidth,
			pImgSrc->width);
		// 宽值重置
		nWidth = pImgSrc->width;
		// 重新计算高
		nHeight = round_double(nWidth * fHeightWidthRate);
		nX = 0;
		nY = (pImgSrc->height - nHeight) / 2;
	}
	else if (nHeight > pImgSrc->height )
	{
		GtWriteTrace(30, "%s:%d: 计算的高[%d] > 原图像高=[%d]！", __FUNCTION__, __LINE__, nHeight,
			pImgSrc->height);
		// 高值重置
		nHeight = pImgSrc->height;
		// 重新计算宽
		nWidth = round_double(nHeight / fHeightWidthRate);
		nX = (pImgSrc->width - nWidth) / 2;
		nY = 0;
	}
	else
	{
		nX = (pImgSrc->width - nWidth) / 2;
		nY = (pImgSrc->height - nHeight) / 2;
	}
	GtWriteTrace(30, "%s:%d: 计算的待截取区域的坐标(%d, %d)，宽=[%d], 高=[%d]！", __FUNCTION__, __LINE__,
		nX, nY, nWidth, nHeight);

	// 截取图像
	pImgDest = cvCreateImage(cvSize(nWidth, nHeight), pImgSrc->depth, pImgSrc->nChannels);
	// 截取图像的区域
	CvRect cutRect = cvRect( nX, nY, nWidth, nHeight);
	// 设置ROI
	cvSetImageROI(pImgSrc, cutRect);
	// 第二次裁剪得到结果图像（ROI复制）
	cvCopy(pImgSrc, pImgDest, NULL);
	// 清空ROI
	cvResetImageROI(pImgSrc);

	return pImgDest;
}