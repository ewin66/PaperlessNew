// SettingDlg.cpp : implementation file
//

#include "stdafx.h"
#include "Paperless.h"
#include "SettingDlg.h"
#include "afxdialogex.h"
#include "MyTTrace.h"
#include "GHook\GHook.h"
#include "utils.h"


#pragma comment(lib,"GHook.lib")
// SettingDlg dialog

IMPLEMENT_DYNAMIC(SettingDlg, CDialogEx)

SettingDlg::SettingDlg(CWnd* pParent /*=NULL*/)
	: CDialogEx(SettingDlg::IDD, pParent)
{
	// 初始化按键
	combKey.count = 0;
	combKey.keys[0] = 0;
	combKey.keys[1] = 0;
	combKey.keys[2] = 0;
	combKey.keys[3] = 0;

	isKeyUp = TRUE;
}

SettingDlg::~SettingDlg()
{
}

void SettingDlg::DoDataExchange(CDataExchange* pDX)
{
	CDialogEx::DoDataExchange(pDX);
}


BEGIN_MESSAGE_MAP(SettingDlg, CDialogEx)
	ON_BN_CLICKED(IDC_BUTTON_OK, &SettingDlg::OnBnClickedButtonOk)
	ON_BN_CLICKED(IDC_BUTTON_CANCEL, &SettingDlg::OnBnClickedButtonCancel)
END_MESSAGE_MAP()

// SettingDlg message handlers
void SettingDlg::OnBnClickedButtonOk()
{
	int specialKeyCount = 0;
	// 判断待设置的热键值是否仅仅包含控制字符 ctrl alt shift
	for (int i = 0; i < combKey.count; i++)
	{
		// 判断已有按键是否含控制字符
		if (isSpecialKey(combKey.keys[i]))
		{
			// 含控制字符，计数
			specialKeyCount++;
		}
	}
	if (specialKeyCount >= combKey.count)
	{
		MessageBox("热键不能只包含 Ctrl Alt Shift 中的一个或多个按键！");
		return;
	}
	// 修改快捷键
	SetXCombKey(0, combKey);

	// 获取热键名称，写到日志中
	CString cKeysName;
	CEdit *pEditDir = (CEdit *)GetDlgItem(IDC_EDIT_KEY);
	pEditDir->GetWindowText(cKeysName);
	GtWriteTrace(30, "[Setting]Save hot key succeed！key count=[%d], [%d], [%d], [%d], [%d], editBox=[%s]", combKey.count,
		combKey.keys[0], combKey.keys[1], combKey.keys[2], combKey.keys[3], cKeysName.GetBuffer());
	cKeysName.ReleaseBuffer();

	// 新热键写到配置文件中
	CString iniDir = GetAppPath()+"\\win.ini";
	CString tmp;
	// 热键值
	tmp.Format("%d|%d|%d|%d|%d|", combKey.count, combKey.keys[0], combKey.keys[1], combKey.keys[2], combKey.keys[3]);
	WritePrivateProfileString("Information", "HotKeyValue", tmp, iniDir);
	// 热键名称
	WritePrivateProfileString("Information", "HotKeyName", cKeysName, iniDir);

	// 弹窗提示
	CString cTmp;
	cTmp.Format("设置热键[%s]成功", cKeysName.GetBuffer());
	cKeysName.ReleaseBuffer();
	//MessageBox(cTmp);

	// 关闭对话框
	CDialog::OnOK();
}


void SettingDlg::OnBnClickedButtonCancel()
{
	CDialog::OnCancel();
}



BOOL SettingDlg::OnInitDialog()
{
	MyInit();
	// 任务栏显示
	ModifyStyleEx(0, WS_EX_APPWINDOW);
	// 显示居中
	CenterWindow();
	return TRUE;
}

void SettingDlg::MyInit()
{
	// 配置文件获取当前热键
	CString strHotKeyName;
	// 热键名称
	GetPrivateProfileString("Information", "HotKeyName", "Ctrl + ALT + S", strHotKeyName.GetBuffer(31), 31, GetAppPath()+"\\win.ini");
	strHotKeyName.ReleaseBuffer();
	CEdit *pStaHotKeyName = (CEdit *)GetDlgItem(IDC_STA_HOTKEY);
	pStaHotKeyName->SetWindowText(strHotKeyName);
}



BOOL SettingDlg::PreTranslateMessage(MSG* pMsg)
{
	if (WM_KEYFIRST <= pMsg->message && pMsg->message <= WM_KEYLAST)
	{
		do 
		{
			CWnd *pFocusWnd = GetFocus();
			// 如果焦点不在输入框上，不处理，退出循环
			if (pFocusWnd->GetDlgCtrlID() != IDC_EDIT_KEY)
			{
				break;
			}
			// 处理的键码 Shift, Ctrl, Alt, Space, 0~9, A~Z...详见虚拟键码表进行对应
			if (! (pMsg->wParam==VK_SHIFT || pMsg->wParam==VK_CONTROL || pMsg->wParam==VK_MENU || pMsg->wParam==VK_SPACE
				|| (pMsg->wParam>=48 && pMsg->wParam<=90)
				|| (pMsg->wParam>=96 && pMsg->wParam<=123 && pMsg->wParam != 108)
				|| (pMsg->wParam>=160 && pMsg->wParam<=165)
				|| (pMsg->wParam>=186 && pMsg->wParam<=192)
				|| (pMsg->wParam>=219 && pMsg->wParam<=222)
				))
			{
				break;
			}
			char str[32]={0};
			CString temp;
			CEdit *pEditDir = (CEdit *)GetDlgItem(IDC_EDIT_KEY);
			// 判断是否按下键盘按键，包括系统处理的按键 ALT和F10
			if (pMsg->message == WM_KEYDOWN || pMsg->message == WM_SYSKEYDOWN)
			{
				GetKeyNameText(pMsg->lParam, str, 31);
				// pMsg->wParam就是键值,str是键码名称
				//temp.Format("[%d]%d %s[%d]", WM_KEYFIRST, pMsg->wParam, str, WM_KEYLAST);
				//pEditDir->SetWindowText(temp);

				switch(pMsg->wParam)
				{
				case VK_SHIFT:
					if( GetKeyState(VK_LSHIFT) & 0x8000 )
					{
						// 左shift
						//temp.Format("[%d]%d %s[%d]", WM_KEYFIRST, VK_LSHIFT, str, WM_KEYLAST);
						//pEditDir->SetWindowText(temp);
						keyDown(VK_LSHIFT, str);
					}
					else if( GetKeyState(VK_RSHIFT) & 0x8000 )
					{
						// 右shift
						keyDown(VK_RSHIFT, str);
					}
					else if( GetKeyState(VK_SHIFT) & 0x8000 )
					{
						// shift
						keyDown(VK_SHIFT, str);
					}
					break;
				case VK_CONTROL:
					if( GetKeyState(VK_LCONTROL) & 0x8000 )
					{
						// 左ctrl
						keyDown(VK_LCONTROL, str);
					}
					else if( GetKeyState(VK_RCONTROL) & 0x8000 )
					{
						// 右ctrl
						keyDown(VK_RCONTROL, str);
					}
					else if( GetKeyState(VK_CONTROL) & 0x8000 )
					{
						// ctrl
						keyDown(VK_CONTROL, str);
					}
					break;
				case VK_MENU:
					if( GetKeyState(VK_LMENU) & 0x8000 )
					{
						// 左alt
						keyDown(VK_LMENU, str);
					}
					else if( GetKeyState(VK_RMENU) & 0x8000 )
					{
						// 右alt
						keyDown(VK_RMENU, str);
					}
					else if( GetKeyState(VK_MENU) & 0x8000 )
					{
						// alt
						keyDown(VK_MENU, str);
					}
					break;
				default:
					keyDown(pMsg->wParam, str);;
				}
			}
			else if (pMsg->message == WM_KEYUP || pMsg->message == WM_SYSKEYUP)
			{
				keyFirstUp(pMsg->wParam);
			}
			return true;
		} while (0);
	}
	return CDialog::PreTranslateMessage(pMsg);
}

int SettingDlg::keyDown(BYTE key, char *kyeName)
{
	CEdit *pEditDir = (CEdit *)GetDlgItem(IDC_EDIT_KEY);
	if (isKeyUp)
	{
		// 按键弹起之后又按下的事件，清空界面上的显示
		pEditDir->SetWindowText("");

		// 重置临时热键结构体
		combKey.count = 0;
		combKey.keys[0] = 0;
		combKey.keys[1] = 0;
		combKey.keys[2] = 0;
		combKey.keys[3] = 0;
	}
	isKeyUp = false;
	if (combKey.count >= 4)
	{
		// 组合键超过4个，不处理
		return -1;
	}
	else
	{
		if (isSpecialKey(key))
		{
			// 判断当前按键是否是特殊键，特殊键前不能有普通键
			for (int i = 0; i < combKey.count; i++)
			{
				// 判断已有按键是否含普通键
				if (!isSpecialKey(combKey.keys[i]))
				{
					// 含普通键，不处理当前按键
					return -2;
				}
			}
		}
		// 检查完成，将当前按键加入到combKey中，并更新界面上的值
		combKey.keys[combKey.count] = key;
		combKey.count++;
		//CString tmpEdit;
		//pEditDir->GetWindowText(tmpEdit);
		if (combKey.count == 1)
		{
			pEditDir->SetWindowText(kyeName);
		}
		else
		{
			CString tmp;
			tmp.Format(" + %s", kyeName);
			pEditDir->SetSel(-1);
			pEditDir->ReplaceSel(tmp);
		}
	}
	return 0;
}

int SettingDlg::keyFirstUp(BYTE key)
{
	isKeyUp = TRUE;
	return 0;
}

BOOL SettingDlg::isSpecialKey(BYTE key)
{
	if (key == VK_CONTROL || key == VK_LCONTROL || key == VK_RCONTROL
		|| key == VK_MENU || key == VK_LMENU || key == VK_RMENU
		|| key == VK_SHIFT || key == VK_LSHIFT || key == VK_RSHIFT)
	{
		// 特殊键，返回真
		return TRUE;
	}
	return false;
}