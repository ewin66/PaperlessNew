#pragma once

#include "CBaseReadIDCard.h"
#include "CentermIFImp.h"

// CCentOneCameraDlg dialog
class CCentOneCameraDlg : public CDialogEx, public CBaseSaveDeskPic
{
	DECLARE_DYNAMIC(CCentOneCameraDlg)

public:
	CCentOneCameraDlg(CWnd* pParent = NULL);   // standard constructor
	virtual ~CCentOneCameraDlg();
	virtual BOOL OnInitDialog();
public:
	// 重写，文拍摄像头获取身份证照片
	virtual int MySaveDeskIDPic(const char *pSaveDesktopIDPicFilenm);
	// 重写，环境摄像头获取人像照片
	virtual int MySaveEnvPic(const char *pSaveEnvPicFilenm);

// Dialog Data
	enum { IDD = IDD_CENT_CAMERA };

protected:
	virtual void DoDataExchange(CDataExchange* pDX);    // DDX/DDV support
private:
	CStatic *pVideoWnd;
	CCentermIFImp *mHighCamera;

	DECLARE_MESSAGE_MAP()
};
