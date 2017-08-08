#pragma once
#include "MyEdit.h"

#ifndef MYTRACKER_
#include "MyTracker.h"
#endif
#include "afxwin.h"
#pragma once
#include "resource.h"

#define IDC_MY_EDIT 20000
#define EDIT_WIDTH 100
#define EDIT_HIGH 20
#define BTN_WIDTH 60
#define BTN_HIGH 20

// CScreenshotDlg dialog
class CScreenshotDlg : public CDialogEx
{
	DECLARE_DYNAMIC(CScreenshotDlg)

public:
	CScreenshotDlg(CWnd* pParent = NULL);   // standard constructor
	virtual ~CScreenshotDlg();

// Dialog Data
	enum { IDD = IDD_SCREENSHOT };

protected:
	virtual void DoDataExchange(CDataExchange* pDX);    // DDX/DDV support
	virtual BOOL OnInitDialog();
	virtual void OnOK();
	virtual void OnCancel();
	afx_msg void OnPaint();
	afx_msg void OnMouseMove(UINT nFlags, CPoint point);
	afx_msg void OnLButtonDown(UINT nFlags, CPoint point);
	afx_msg void OnLButtonUp(UINT nFlags, CPoint point);
	afx_msg void OnLButtonDblClk(UINT nFlags, CPoint point);
	afx_msg BOOL OnEraseBkgnd(CDC* pDC);
	afx_msg BOOL OnSetCursor(CWnd* pWnd, UINT nHitTest, UINT message);
	afx_msg void OnRButtonUp(UINT nFlags, CPoint point);
	afx_msg HBRUSH OnCtlColor(CDC* pDC, CWnd* pWnd, UINT nCtlColor);
	afx_msg void OnBnClickedMyOkButton();
	afx_msg void OnBnClickedMyCancleButton();

public:
	int m_xScreen;
	int m_yScreen;
	BOOL m_bShowMsg;                //显示截取矩形大小信息
	BOOL m_bDraw;                   //是否为截取状态
	BOOL m_bFirstDraw;              //是否为首次截取
	BOOL m_bQuit;                   //是否为退出
	CPoint m_startPt;				//截取矩形左上角
	CMyTracker m_rectTracker;     //像皮筋类
	CBrush m_brush;					//
    HCURSOR m_hCursor;              //光标
    HCURSOR m_hOldCursor;              //原先光标
	CBitmap * m_pBitmap;            //背景位图
	CRgn m_rgn;						//背景擦除区域

	CMyEdit m_tipEdit;
	CButton *myOkButton;
	CButton *myCancelButton;

public:
	HBITMAP CopyScreenToBitmap(LPRECT lpRect,BOOL bSave=FALSE);   //考贝桌面到位图
	void DrawTip();                            //显示操作提示信息
	void DrawMessage(CRect &inRect,CDC * pDC);       //显示截取矩形信息
	void PaintWindow();               //重画窗口
	void ChangeRGB();
	
	BOOL SaveBitmapToFile(HBITMAP hBitmap, LPCTSTR lpFileName);
	// 移动 确定、取消按钮 位置到截图框边缘并显示
	void MoveShowOkCancelBtn();
	// 隐藏 确定、取消按钮
	void HideenOkCancelBtn();
	

	DECLARE_MESSAGE_MAP()
};
