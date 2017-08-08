// ManualInputDlg.cpp : implementation file
//

#include "stdafx.h"
#include "Paperless.h"
#include "ManualInputDlg.h"
#include "afxdialogex.h"
#include "MyTTrace.h"
#include "Network/HttpComm.h"


// CManualInputDlg dialog

IMPLEMENT_DYNAMIC(CManualInputDlg, CDialogEx)

CManualInputDlg::CManualInputDlg(CString tip, CWnd* pParent /*=NULL*/)
	: CDialogEx(CManualInputDlg::IDD, pParent)
{
	this->tip = tip;
}

CManualInputDlg::~CManualInputDlg()
{
}

void CManualInputDlg::DoDataExchange(CDataExchange* pDX)
{
	CDialogEx::DoDataExchange(pDX);
}


BEGIN_MESSAGE_MAP(CManualInputDlg, CDialogEx)
	ON_BN_CLICKED(IDC_BTN_SEND, &CManualInputDlg::OnBnClickedBtnSend)
	ON_BN_CLICKED(IDC_BTN_CANCEL, &CManualInputDlg::OnBnClickedBtnCancel)
END_MESSAGE_MAP()
// CManualInputDlg message handlers
void CManualInputDlg::OnBnClickedBtnSend()
{
	GetDlgItem(IDC_STATIC2)->SetWindowText("正在发送消息，请等待......");

	CEdit* pEdit1 = NULL;
	CString str = NULL;
	string msgStr_rtn;

	pEdit1 = (CEdit*) GetDlgItem(IDC_EDIT1);
	pEdit1->GetWindowText(str);

	// 流水号逻辑性检查

	// 发送的json报文预处理
	SendDataPrepare(1, str.GetBuffer());
	str.ReleaseBuffer();

}


void CManualInputDlg::OnBnClickedBtnCancel()
{
	CDialog::OnCancel();
}



BOOL CManualInputDlg::OnInitDialog()
{
	SetTip(this->tip.GetBuffer());
	this->tip.ReleaseBuffer();
	ResetEdit();
	// 任务栏显示
	ModifyStyleEx(0, WS_EX_APPWINDOW);
	// 显示居中
	CenterWindow();
	return false;
}

void CManualInputDlg::SetTip(char *pStrTip)
{
	GetDlgItem(IDC_STATIC2)->SetWindowText(pStrTip);
}

void CManualInputDlg::ResetEdit()
{
	SYSTEMTIME sysTime;
	GetLocalTime(&sysTime);
	CString str;
	str.Format("%04d%02d%02d", sysTime.wYear, sysTime.wMonth, sysTime.wDay);
	CEdit* pEdit1 = (CEdit*) GetDlgItem(IDC_EDIT1);
	pEdit1->SetFocus();
	pEdit1->SetWindowTextA(str);
	pEdit1->SetSel(0, -1);
	pEdit1->SetSel(-1, 0);
}
