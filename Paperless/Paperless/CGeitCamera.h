#include "CBaseReadIDCard.h"
#include <cv.h> 

#pragma once


/************************************************************************/
/* 国光 - 高拍仪摄像头                                            */
/************************************************************************/
class CGeitCamera : public CBaseSaveCamera
{
public:
	CGeitCamera();
	virtual ~CGeitCamera();
public:
	// 重写，文拍摄像头获取身份证照片
	virtual int MySaveDeskIDPic(const char *pSaveDesktopIDPicFilenm);
	// 重写，环境摄像头获取人像照片
	virtual int MySaveEnvPic(const char *pSaveEnvPicFilenm);

};