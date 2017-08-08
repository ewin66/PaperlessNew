
// PaperlessPrepare.cpp : ����Ӧ�ó��������Ϊ��
//

#include "stdafx.h"
#include "PaperlessPrepare.h"
#include "PaperlessPrepareDlg.h"

#ifdef _DEBUG
#define new DEBUG_NEW
#endif


// CPaperlessPrepareApp

BEGIN_MESSAGE_MAP(CPaperlessPrepareApp, CWinApp)
	ON_COMMAND(ID_HELP, &CWinApp::OnHelp)
END_MESSAGE_MAP()


// CPaperlessPrepareApp ����

CPaperlessPrepareApp::CPaperlessPrepareApp()
{
	// TODO: �ڴ˴���ӹ�����룬
	// ��������Ҫ�ĳ�ʼ�������� InitInstance ��
}


// Ψһ��һ�� CPaperlessPrepareApp ����

CPaperlessPrepareApp theApp;


// CPaperlessPrepareApp ��ʼ��

BOOL CPaperlessPrepareApp::InitInstance()
{
	CWinApp::InitInstance();

	CString strDir = GetFilePath();
	// �ж�log�ļ��洢Ŀ¼�Ƿ���ڣ��������򴴽�
	CString strLogDir = strDir + "\\log";
	if (!PathIsDirectory(strLogDir))
	{
		//����Ŀ¼,���еĻ���Ӱ��
		::CreateDirectory(strLogDir, NULL);
	}
	// ��־�ļ���ʼ��
	GtSetTraceFilePrefix(GetFilePath()+"\\log\\Update");
	GtSetTraceLevel(EM_TraceDebug);

	// ��һʵ������
	HANDLE m_hMutex = CreateMutex(NULL, FALSE, this->m_pszAppName);// ���������
	if (GetLastError() == ERROR_ALREADY_EXISTS)
	{
		// ������л������������ͷž������λ��������
		CloseHandle(m_hMutex);
		m_hMutex = NULL;
		// �����˳�
		::MessageBoxA(NULL, "�����������У��������ظ�������", "����", MB_OK);
		return FALSE;
	}

	AfxEnableControlContainer();

	// ���� shell ���������Է��Ի������
	// �κ� shell ����ͼ�ؼ��� shell �б���ͼ�ؼ���
	CShellManager *pShellManager = new CShellManager;

	// ��׼��ʼ��
	// ���δʹ����Щ���ܲ�ϣ����С
	// ���տ�ִ���ļ��Ĵ�С����Ӧ�Ƴ�����
	// ����Ҫ���ض���ʼ������
	// �������ڴ洢���õ�ע�����
	// TODO: Ӧ�ʵ��޸ĸ��ַ�����
	// �����޸�Ϊ��˾����֯��
	SetRegistryKey(_T("��ͨ�Ƽ�"));

	CPaperlessPrepareDlg dlg;
	m_pMainWnd = &dlg;
	INT_PTR nResponse = dlg.DoModal();
	if (nResponse == IDOK)
	{
		// TODO: �ڴ˷��ô����ʱ��
		//  ��ȷ�������رնԻ���Ĵ���
	}
	else if (nResponse == IDCANCEL)
	{
		// TODO: �ڴ˷��ô����ʱ��
		//  ��ȡ�������رնԻ���Ĵ���
	}

	// ɾ�����洴���� shell ��������
	if (pShellManager != NULL)
	{
		delete pShellManager;
	}

	// ���ڶԻ����ѹرգ����Խ����� FALSE �Ա��˳�Ӧ�ó���
	//  ����������Ӧ�ó������Ϣ�á�
	return FALSE;
}

