
// GKlogin.h : PROJECT_NAME Ӧ�ó������ͷ�ļ�
//

#pragma once

#ifndef __AFXWIN_H__
	#error "�ڰ������ļ�֮ǰ������stdafx.h�������� PCH �ļ�"
#endif

#include "resource.h"		// ������


// CGKloginApp: 
// �йش����ʵ�֣������ GKlogin.cpp
//

class CGKloginApp : public CWinApp
{
public:
	CGKloginApp();
	BOOL CheckCommandLine();   //Ϊ����������
// ��д
public:
	virtual BOOL InitInstance();
	virtual int ExitInstance();
// ʵ��
	DECLARE_MESSAGE_MAP()
};

extern CGKloginApp theApp;