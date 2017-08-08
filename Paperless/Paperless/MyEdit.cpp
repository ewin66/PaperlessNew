// MyEdit.cpp : 实现文件
//

#include "stdafx.h"
#include "Paperless.h"
#include "MyEdit.h"
#include "MyTTrace.h"


// CMyEdit

IMPLEMENT_DYNAMIC(CMyEdit, CEdit)

CMyEdit::CMyEdit()
{
	bitmap.LoadBitmap(IDB_BITMAP1);
	m_bMove=TRUE;
}

CMyEdit::~CMyEdit()
{
}


BEGIN_MESSAGE_MAP(CMyEdit, CEdit)
	ON_WM_CREATE()
	ON_WM_MOUSEMOVE()
	ON_WM_ERASEBKGND()
	ON_WM_SETFOCUS()
	ON_WM_CTLCOLOR_REFLECT()
END_MESSAGE_MAP()



// CMyEdit 消息处理程序

int CMyEdit::OnCreate(LPCREATESTRUCT lpCreateStruct)
{
	if (CEdit::OnCreate(lpCreateStruct) == -1)
		return -1;

	// TODO:  在此添加您专用的创建代码

	return 0;
}


void CMyEdit::OnMouseMove(UINT nFlags, CPoint point)
{
	// TODO: 在此添加消息处理程序代码和/或调用默认值
	//***********************************************************************************
	
	CRect rect;
	GetWindowRect(&rect);

	int xScreen = GetSystemMetrics(SM_CXSCREEN);
	//int ySCreen = GetSystemMetrics(SM_CYSCREEN);

	if(m_bMove)
	{
		//移动到左上角
		MoveWindow(10,10,rect.Width(),rect.Height());
		m_bMove=FALSE;
	}
	else
	{
		//移动到右上角
		MoveWindow(xScreen-180,10,rect.Width(),rect.Height());
		m_bMove=TRUE;
	}
//**************************************************************************************
	CEdit::OnMouseMove(nFlags, point);
}


BOOL CMyEdit::OnEraseBkgnd(CDC* pDC)
{
	// TODO: 在此添加消息处理程序代码和/或调用默认值
	//**************************************************************************************
    
	//取消文字复选
	this->SetSel(0,0);
	
	CDC dcCompatible;
	dcCompatible.CreateCompatibleDC(pDC);
	dcCompatible.SelectObject(&bitmap);

	BITMAP bmp;
	bitmap.GetBitmap(&bmp);

	CRect rect;
	GetClientRect(&rect);
	pDC->BitBlt(0,0,rect.Width(),rect.Height(),&dcCompatible,0,0,SRCCOPY);
	dcCompatible.DeleteDC();
	return TRUE;
//*********************************************************************************
//	return CEdit::OnEraseBkgnd(pDC);
}

void CMyEdit::OnSetFocus(CWnd* pOldWnd)
{
	CEdit::OnSetFocus(pOldWnd);

	// TODO: 在此处添加消息处理程序代码
	//**********************************************************************************
   //隐藏光标提示符,其实也用不着
	this->HideCaret();
   //*********************************************************************************
}


HBRUSH CMyEdit::CtlColor(CDC* pDC, UINT nCtlColor)
{
//	HBRUSH hbr = CEdit::OnCtlColor(pDC, pWnd, nCtlColor);

	// TODO:  在此更改 DC 的任何特性
	//***************************************************************************************
	//设置文字背景透明
	pDC->SetBkMode(TRANSPARENT);
	return NULL;
	//**************************************************************************************
	// TODO:  如果默认的不是所需画笔，则返回另一个画笔
//	return hbr;
}
