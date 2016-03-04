
// GKlogin.cpp : ����Ӧ�ó��������Ϊ��
//

#include "stdafx.h"
#include "GKlogin.h"
#include "GKloginDlg.h"

#ifdef _DEBUG
#define new DEBUG_NEW
#endif


// CGKloginApp

BEGIN_MESSAGE_MAP(CGKloginApp, CWinApp)
	ON_COMMAND(ID_HELP, &CWinApp::OnHelp)
END_MESSAGE_MAP()


// CGKloginApp ����

CGKloginApp::CGKloginApp()
{
	// ֧����������������
	m_dwRestartManagerSupportFlags = AFX_RESTART_MANAGER_SUPPORT_RESTART;

	// TODO:  �ڴ˴����ӹ�����룬
	// ��������Ҫ�ĳ�ʼ�������� InitInstance ��
}


// Ψһ��һ�� CGKloginApp ����

CGKloginApp theApp;

//����������

BOOL CGKloginApp::CheckCommandLine()
{
	extern int __argc;
	extern char**  __argv;

	int i;
	if (__argc == 3) {
		if (strcmp(__argv[1], "-update") == 0) {
			for (i = 0; i < 3; i++) {
				//try to cover old version file
				if (CopyFile(__argv[0], __argv[2], FALSE)) {
					//DeleteFile(__argv[0]);
					break;
				}
				Sleep(3000);
			}

			if (i >= 3) {
				AfxMessageBox("�����ļ�ʧ�ܣ����Ժ����³��ԣ�", MB_TOPMOST);
				return FALSE;
			}

			char parameters[MAX_PATH] = "-updated \"";
			strcat(parameters, __argv[0]);

			SHELLEXECUTEINFO ShExecInfo = { 0 };
			ShExecInfo.cbSize = sizeof(SHELLEXECUTEINFO);
			ShExecInfo.fMask = SEE_MASK_NOCLOSEPROCESS;
			ShExecInfo.lpFile = __argv[2];
			ShExecInfo.lpParameters = _T(parameters);
			ShExecInfo.nShow = SW_SHOW;
			ShellExecuteEx(&ShExecInfo);

			return FALSE;

		}
		else if (strcmp(__argv[1], "-updated") == 0) {
			DeleteFile(__argv[2]);
		}
	}
	return TRUE;
}

// CGKloginApp ��ʼ��

BOOL CGKloginApp::InitInstance()
{
	// ���һ�������� Windows XP �ϵ�Ӧ�ó����嵥ָ��Ҫ
	// ʹ�� ComCtl32.dll �汾 6 ����߰汾�����ÿ��ӻ���ʽ��
	//����Ҫ InitCommonControlsEx()��  ���򣬽��޷��������ڡ�
	INITCOMMONCONTROLSEX InitCtrls;
	InitCtrls.dwSize = sizeof(InitCtrls);
	// ��������Ϊ��������Ҫ��Ӧ�ó�����ʹ�õ�
	// �����ؼ��ࡣ
	InitCtrls.dwICC = ICC_WIN95_CLASSES;
	InitCommonControlsEx(&InitCtrls);

	CWinApp::InitInstance();

	if (!CheckCommandLine()) return FALSE;

#ifndef _DEBUG
	HANDLE g_mutex = CreateMutex(NULL, FALSE, "ZTE4GDST");
	HWND findHwnd;
	if (GetLastError() == ERROR_ALREADY_EXISTS)
	{
		//MessageBox(NULL,"���Ѿ�������һ����֤��!","����",MB_OK |MB_ICONERROR);
		if ((findHwnd = FindWindow(NULL, STR_AppName)) != NULL)
		{
			//MessageBox(NULL,"�ҵ��Ǹ���֤����","����",MB_OK |MB_ICONERROR);
			ShowWindow(findHwnd, SW_SHOW);
		}

		MessageBox(findHwnd, "ֻ��������һ����֤��!", "����", MB_OK | MB_ICONERROR);
		return 0;
	}
#endif

	AfxEnableControlContainer();

	// ���� shell ���������Է��Ի������
	// �κ� shell ����ͼ�ؼ��� shell �б���ͼ�ؼ���
	CShellManager *pShellManager = new CShellManager;

	// ���Windows Native���Ӿ����������Ա��� MFC �ؼ�����������
	CMFCVisualManager::SetDefaultManager(RUNTIME_CLASS(CMFCVisualManagerWindows));

	// ��׼��ʼ��
	// ���δʹ����Щ���ܲ�ϣ����С
	// ���տ�ִ���ļ��Ĵ�С����Ӧ�Ƴ�����
	// ����Ҫ���ض���ʼ������
	// �������ڴ洢���õ�ע�����
	// TODO:  Ӧ�ʵ��޸ĸ��ַ�����
	// �����޸�Ϊ��˾����֯��
	SetRegistryKey(_T("Ӧ�ó��������ɵı���Ӧ�ó���"));

	CGKloginDlg dlg;
	m_pMainWnd = &dlg;
	INT_PTR nResponse = dlg.DoModal();
	if (nResponse == IDOK)
	{
		// TODO:  �ڴ˷��ô�����ʱ��
		//  ��ȷ�������رնԻ���Ĵ���
	}
	else if (nResponse == IDCANCEL)
	{
		// TODO:  �ڴ˷��ô�����ʱ��
		//  ��ȡ�������رնԻ���Ĵ���
	}
	else if (nResponse == -1)
	{
		TRACE(traceAppMsg, 0, "����: �Ի��򴴽�ʧ�ܣ�Ӧ�ó���������ֹ��\n");
		TRACE(traceAppMsg, 0, "����: ������ڶԻ�����ʹ�� MFC �ؼ������޷� #define _AFX_NO_MFC_CONTROLS_IN_DIALOGS��\n");
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
int CGKloginApp::ExitInstance()
{

	return CWinApp::ExitInstance();
}