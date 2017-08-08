#pragma once


// CMyEdit
#if !defined(AFX_MYEDIT_H__A34EEA6D_E8FC_4D15_B03C_BAA42FDF6FCB__INCLUDED_)
#define AFX_MYEDIT_H__A34EEA6D_E8FC_4D15_B03C_BAA42FDF6FCB__INCLUDED_

#if _MSC_VER > 1000
#pragma once
#endif // _MSC_VER > 1000
class CMyEdit : public CEdit
{
	DECLARE_DYNAMIC(CMyEdit)

public:
	CMyEdit();
	virtual ~CMyEdit();

protected:
	afx_msg int OnCreate(LPCREATESTRUCT lpCreateStruct);
	afx_msg void OnMouseMove(UINT nFlags, CPoint point);
	afx_msg BOOL OnEraseBkgnd(CDC* pDC);
	afx_msg void OnSetFocus(CWnd* pOldWnd);
	afx_msg HBRUSH CtlColor(CDC* pDC, UINT nCtlColor);
	DECLARE_MESSAGE_MAP()
public:
	BOOL m_bMove;
private:
	//…Ë÷√ŒªÕº±≥æ∞
	CBitmap bitmap;
	
};


#endif // !defined(AFX_MYEDIT_H__A34EEA6D_E8FC_4D15_B03C_BAA42FDF6FCB__INCLUDED_)