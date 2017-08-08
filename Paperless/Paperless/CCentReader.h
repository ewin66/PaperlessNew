#include "stdafx.h"
#include "CBaseReadIDCard.h"
#include "Centerm/CENT_IDCard.h"

#pragma once

/************************************************************************/
/* 升腾读卡器 - 身份证识读仪                                            */
/************************************************************************/
class CCentReader : public CBaseReadIDCardInfo
{
public:
	CCentReader() {}
	virtual ~CCentReader() {}
public:
	// 重写，获取身份证芯片信息
	virtual int MyReadIDCardInfo(const char *pSaveHeadPicFilenm, MYPERSONINFO *pPersonInfo);
};