// Config.h: interface for the CConfig class.
//
//////////////////////////////////////////////////////////////////////

#if !defined(AFX_CONFIG_H__2EE917D0_3B18_4580_BC7D_C675F4866D9A__INCLUDED_)
#define AFX_CONFIG_H__2EE917D0_3B18_4580_BC7D_C675F4866D9A__INCLUDED_

#if _MSC_VER > 1000
#pragma once
#endif // _MSC_VER > 1000

#include <afxtempl.h>
#include "Define.h"

class CConfig  
{
public:
	CConfig();
	virtual ~CConfig();
	void	SaveConfig();
	void	LoadConfig();
	void	GetFullPathToFile(char  *pszFullPath,char * pszFilename);
	void	GetDocumentsPath(char  *pszFullPath, char * pszFilename);
public:
	bool	m_bAutorun;		//�Զ�����
	bool	m_bAutologon;		//�Զ���¼
	bool	m_bRememberPWD;	//��������
	bool	m_bShowBubble;		//��ʾ����
	bool	m_bHttpHeart;	//�Զ���ҳ����

	int		m_iTimeout;		//��ʱʱ��

	bool m_bWebAuth;//�Ƿ������ҳ��֤
	CString m_csWebAuthUrl;//��ҳ��֤��ַ

	bool m_bWebLogout;//�Ƿ������ҳע��
	CString m_csWebLogoutUrl;//��ҳע����ַ

	bool m_bEnableWebAccount;//�Ƿ�������ҳ��֤�ʺ���Ϣ��������������֤�ʺ�һ��
	CString m_csWebUsername;//��ҳ��֤�û���
	CString m_csWebPassword;//��ҳ��֤����

	bool m_bTimingReauth;//��ʱ��֤
	CString m_csReauthTime;//��ʱ��֤ʱ��

	bool m_bAutoUpdate;
	bool m_bAutoFilter;

	bool m_bDHCP;			//�Ƿ�����DHCP

	CString m_csHeartUrl;
	CString m_csHeartCookies;
	int m_iHeartInterval;

	CMapStringToString   m_UserInfo;	//������е��˺�������Ϣ

	CString m_csLastUser;	//�ϴ����õ��û���

	CString	m_csNetCard;	//�ϴ���ѡ�����������

	CString	LastVersion;	//��һ�汾��
	
	CString	m_csEduBas;	//��ҳ��֤������

	CString m_csZTEServer;//��ҳע����ַ
	bool m_bDebug; //�Ƿ����������Ϣ
	bool m_iReauthlog;    //����������֤
	bool m_Ping;			//Ping����
	bool m_Reauth;		//������֤
	bool m_UEIP;		//�û�������Ƽƻ�
	bool m_Changepwd;
	bool m_SlienceUp;	//��Ĭ����
	bool m_ZTEAuth;		//������֤����
	bool m_WebAuth;		//��ҳ��֤����
	bool m_WebAuth2;	//�°���ҳ��֤����
	bool m_CYClient;   //��Ժ
	bool m_AutoBas;    //��ҳ��֤302�Զ���ȡbas
	CString banner_url;
/*==================�������ò�����δ������================

=========================================================*/
};
#endif // !defined(AFX_CONFIG_H__2EE917D0_3B18_4580_BC7D_C675F4866D9A__INCLUDED_)
