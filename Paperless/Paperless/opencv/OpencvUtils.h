// opencv 工具函数
#include "../stdafx.h"
#pragma once

#include "cv.h"
#include "cxcore.h"
#include "highgui.h"


/** 
* 功能：自动框选出原图内的身份证
* 参数：pImgSrc：待提取的原图像
*		pImgDest：提取出的身份证图像，必须手动释放
* 返回值：0-成功，其他-失败
**/
int MyAutoCrop(IplImage *pImgSrc, IplImage **pImgDest);

/** 
* 功能：不改变大小，旋转图像
* 参数：pImgSrc：待旋转的原图像
*		fDegree：待旋转的角度（弧度制），正数表示逆时针，负数表示顺时针
* 返回值：旋转后的图像，为NULL表示失败
**/
IplImage* rotateImage(IplImage* pImgSrc, float fDegree);

/** 
* 功能：截取图像中心区域，按面积和宽高比例截取，图像从image中获取
* 参数：pImgSrc：待截取的原图像
*		fAreaRate：面积比例，大面积/小面积，比例需大于1
*		fHeightWidthRate：高宽比，高/宽，比例需大于1
* 返回值：截取后的图像，为NULL表示失败，不为空需要手动释放
**/
IplImage* GetCentreOfImage1(IplImage* pImgSrc, float fAreaRate, float fHeightWidthRate);

/** 
* 功能：截取图像中心区域，按面积和宽高比例截取，图像从图片文件中获取
* 参数：pFilePath：待旋转的原图像的路径
*		fAreaRate：面积比例，大面积/小面积，比例需大于1
*		fHeightWidthRate：高宽比，高/宽，比例需大于1
* 返回值：截取后的图像，为NULL表示失败，不为空需要手动释放
**/
IplImage* GetCentreOfImage2(const char *pFilePath, float fAreaRate, float fHeightWidthRate);
