// PingThread.h: interface for the CPingThread class.
//
//////////////////////////////////////////////////////////////////////

#if !defined(AFX_PINGTHREAD_H__88E7C296_2B7A_4A67_AA7B_C89EA8D3BAAC__INCLUDED_)
#define AFX_PINGTHREAD_H__88E7C296_2B7A_4A67_AA7B_C89EA8D3BAAC__INCLUDED_

#if _MSC_VER > 1000
#pragma once
#endif // _MSC_VER > 1000

#include "Ping.h"
class CPingThread
{
public:
	CPingThread();
	virtual ~CPingThread();

	UINT	m_dwID;				//�߳�ID
	HANDLE	m_hThread;			//�߳̾��
	HANDLE	m_hKillEvent;
	HANDLE	m_hSignalEvent;

	static UINT __stdcall  ThreadProc(void* pThis);

	void StartPing(UINT nRetries, CString strHost, CString strHost2, HWND hWnd, char * m_ip);
	void StopPing();

	CPing	m_ping;				//ping����
	CString m_strHost;			//������ַ
	CString m_strHost2;			//������ַ2
	HWND	m_hWnd;				//������
	UINT	m_nRetries;			//ping�Ĵ���
	char *m_localip;					//��������IP
	bool *Pingstopsign;
	bool Pingstop;
};

#endif // !defined(AFX_PINGTHREAD_H__88E7C296_2B7A_4A67_AA7B_C89EA8D3BAAC__INCLUDED_)
