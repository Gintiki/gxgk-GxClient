// PingThread.cpp: implementation of the CPingThread class.
//
//////////////////////////////////////////////////////////////////////

#include "stdafx.h"
#include "PingThread.h"
#include <process.h>    /* _beginthread, _endthread */

#ifdef _DEBUG
#undef THIS_FILE
static char THIS_FILE[] = __FILE__;
#define new DEBUG_NEW
#endif

//////////////////////////////////////////////////////////////////////
// Construction/Destruction
//////////////////////////////////////////////////////////////////////

CPingThread::CPingThread()
{
	m_dwID = 0;
	m_hThread = NULL;
	//�����ź��¼�
	m_hKillEvent = CreateEvent(NULL, TRUE, FALSE, NULL);
	m_hSignalEvent = CreateEvent(NULL, FALSE, FALSE, NULL);

	//����һ��ping�߳�
	//m_hThread = (HANDLE)_beginthreadex(NULL,0,ThreadProc,(void*) this,0,&m_dwID);
}

CPingThread::~CPingThread()
{
	SetEvent(m_hKillEvent);
	WaitForSingleObject(m_hThread, INFINITE);

}

//ping�̹߳��̺���
UINT CPingThread::ThreadProc(void* lpParam)
{
	CPingThread* pThis = reinterpret_cast<CPingThread*>(lpParam);

	while (1)
	{
		HANDLE hObjects[2];
		hObjects[0] = pThis->m_hKillEvent;
		hObjects[1] = pThis->m_hSignalEvent;
		pThis->Pingstopsign = &pThis->Pingstop;
		//�ȴ��ź���Ч
		DWORD dwWait = WaitForMultipleObjects(2, hObjects, FALSE, INFINITE);
		if (dwWait == WAIT_OBJECT_0)
			break;

		//��ʼping
		if (dwWait == WAIT_OBJECT_0 + 1)
			pThis->m_ping.Ping(pThis->m_nRetries, pThis->m_strHost, pThis->m_strHost2, pThis->m_hWnd, pThis->Pingstopsign, pThis->m_localip);
	}
	return 0;
}
//��ʼping
void CPingThread::StartPing(UINT nRetries, CString strHost, CString strHost2, HWND hWnd,char * m_ip)
{
	m_nRetries = nRetries;
	m_strHost = strHost;
	m_strHost2 = strHost2;
	m_hWnd = hWnd;
	m_localip = m_ip;
	//�����ź���Ч�����Կ�ʼ�߳���
	SetEvent(m_hSignalEvent);
}
void CPingThread::StopPing()
{
	SetEvent(m_hKillEvent);
	WaitForSingleObject(m_hThread, INFINITE);
}