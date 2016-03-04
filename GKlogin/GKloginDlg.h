
// GKloginDlg.h : ͷ�ļ�
//

#pragma once
#include "Define.h"
#include "Resource.h"
#include "SystemTray.h"
#include "PingThread.h"
#include "Config.h"
#include "afxwin.h"
#define WM_USER_TRAY_NOTIFICATION (WM_USER+0x101)//����
#define DEFAULT_DPI 96.0

extern CConfig Config;

// CGKloginDlg �Ի���
class CGKloginDlg : public CDialogEx
{
// ����
public:
	CGKloginDlg(CWnd* pParent = NULL);	// ��׼���캯��

// �Ի�������
	enum { IDD = IDD_GKLOGIN_DIALOG };
	CComboBox	m_ccb_username;
	CComboBox	m_ccbNetCard;

	protected:
	virtual void DoDataExchange(CDataExchange* pDX);	// DDX/DDV ֧��

// ʵ��
protected:
	HICON m_hIcon;

	// ���ɵ���Ϣӳ�亯��
	virtual BOOL OnInitDialog();
	afx_msg void OnSysCommand(UINT nID, LPARAM lParam);
	afx_msg void OnPaint();
	afx_msg HCURSOR OnQueryDragIcon();
	DECLARE_MESSAGE_MAP()
protected:
	//����
	afx_msg LONG OnTrayNotification(WPARAM wparam, LPARAM lparam);  //���̲˵�
	RECT			m_rc;					//����Ŵ���ľ�������
	CEdit	*editLog;
	bool logshowstatus;
	afx_msg void OnSelchangeUsername();
	void Complaint();
	HBITMAP BannerPicture;

	CSystemTray		m_tray;					//ϵͳ����ͼ��

	char *	GetOSVersion();
	afx_msg void OnTimer(UINT nIDEvent);
	time_t			m_startTime;			//��֤�Ŀ�ʼʱ��

	void initialize();   //������֤��ʼ������
	//������Ϣ
	char *	GetAdapterInfo(const char *descript);

	static char *	ToNPFName(const char *GUID);
	static char *	ToTCPName(const char *GUID);
	DWORD WINAPI IpconfigRenew();
	int TestAdapter(const char *name);
	CStringArray	 m_csAdapters;			//�����б�
	pcap_t			* m_adapterHandle;		//���������

	


	//�������
	bool			ShareServiceStatus = 0; //ICS�����־ 0Ϊ��������ICS����1Ϊ������ICS����
	afx_msg DWORD StopService(LPCTSTR pszName);
	afx_msg DWORD StartServiceA(LPCTSTR pszName);
	afx_msg DWORD DetectServiceStats(LPCTSTR pszName);
	afx_msg int DHCP_Auto();
	static bool EnableDHCP(const char* wcAdapterName, const bool enable);
	int WebPop();

	//����ģ��
	void	SetBubble(char * title, char * content, int timeout = 1000);


	int Notice(int id);
	
	void WebFaultMsg(const char *msg);




	//��ť
	afx_msg void OnStart();
	afx_msg void OnLogoff();
	afx_msg void OnExit();
	afx_msg void OnLogshow();
	afx_msg void OnSetting();
	afx_msg void OnTrayShow();
	afx_msg void OnBnClickedFeedback();

	//��ҳ��֤	
	char*	HttpAuth(BOOL bForce);
	char *  HttpAuthOut(BOOL bForce);
	void	StartWebAuth();

	//��ҳ����
	afx_msg void OnHttpHeart();

	//Banner
	CStatic Banner_Var;
	afx_msg void OnStnClickedBanner();
	afx_msg void Banner_Show();

	//��Ϣ��ȡ
	void GetAccountMsg();
	int HavingConnect();

	//����
	long    tv_sec;         /* seconds */
	long    tv_usec;        /* and microseconds */
	float	Flux;			//����
	static unsigned WINAPI __stdcall NetFlux(void *para);
	static void dispatcher_handler(u_char *, const struct pcap_pkthdr *, const u_char *);
public:
	//����
	CStatusBarCtrl  m_StatusBar;			//״̬��

	bool			m_bAuth;				//�Ƿ��Ѿ���֤
	int CheckUpdate();
	int CheckWinPcapUpdate();
	
	//�˺�����
struct{
	u_char			m_username[50];
	int				m_usernameLen;
	u_char			m_password[50];
	int				m_passwordLen;
	char			m_ip[16];
	char			m_adaptername[256];
	u_char			m_MacAdd[6];			//���MAC��ַ
	}User;
struct UserMsg{
	char AccountName[20], AccountTeam[20], BegainDate[20], EndDate[20], Address[20], Phone[12], Speed[20];
};
UserMsg Account = {};
	enum DEBUG_LEVEL{
		I_INFO,
		I_MSG,
		I_WARN,
		I_ERR,
		I_REM
	};
	void	Log(int level, const char *fmt, ...);
	void	UpdateStatus(bool bOnline);

	//�߳̿���
	HANDLE			m_AuthThread;			//��֤���߳̾��,�Ա�����߳�
	HANDLE			m_DHCPThread;			//DHCP���߳̾��,�Ա�����߳�
	HANDLE			m_WebAuthThread;		//��ҳ��֤�ػ��߳�,�Ա�����߳�
	HANDLE			m_WeblogoutThread;		//��ҳ��֤�ػ��߳�,�Ա�����߳�
	HANDLE			m_HttpHeartThread;		//��ҳ��֤�ػ��߳�,�Ա�����߳�
	HANDLE			m_NetFluxThread;		//����״̬�ػ��߳�,�Ա�����߳�
	HANDLE			m_GetLANAccountMsgThread;
	void CloseThread();
	//������֤
	static void	get_packet(u_char *args, const struct pcap_pkthdr *pcaket_header, const u_char *packet);
	static unsigned WINAPI __stdcall eap_thread(void *para);
	static unsigned WINAPI __stdcall web_auth_thread(void *para);
	static unsigned WINAPI __stdcall logout_thread(void *para);
	static unsigned WINAPI __stdcall dhcp_thread(void *para);
	static unsigned WINAPI __stdcall http_heart_thread(void *para);

	//״̬
	enum STATUS {
		INIT,
		AUTHING,
		AUTHED,
		DHCPING,
		DHCPED,
		HTTPING,
		HTTPED,
		ONLINE,
		OFFLINE
	};
	STATUS status;
	BOOL bConnected;
	BOOL bRetryConnect;
	void NetStatus();

	void FaultMsg(const char *msg);
	int PCsign(char *DiskSign);
	//������Ϣ
	DWORD WINAPI GetMacIP(const char *adaptername, char *ip, unsigned char *mac);
	static char *	GetGUID(const char *name);


	afx_msg void OnIdok();
};
BOOL LoadImageFromResource(CImage *pImage, UINT nResID, LPCTSTR lpTyp);
// ����Ӧ�ó��򡰹��ڡ��˵���� CAboutDlg �Ի���

class CAboutDlg : public CDialogEx
{
public:
	CAboutDlg();

	// �Ի�������
	enum { IDD = IDD_ABOUTBOX };

protected:
	virtual BOOL OnInitDialog();
	virtual void DoDataExchange(CDataExchange* pDX);    // DDX/DDV ֧��
	// ʵ��
protected:
	DECLARE_MESSAGE_MAP()
public:
	CEdit m_AboutUs;
	afx_msg void OnBnClickedChkRestore();
};
class CComplaint : public CDialogEx
{
public:
	CComplaint(CWnd* pParent = NULL);
	
	// �Ի�������
	enum { IDD = IDD_Complaint };

protected:
	HICON m_hIcon;
	virtual BOOL OnInitDialog();
	virtual void DoDataExchange(CDataExchange* pDX);    // DDX/DDV ֧��

	// ʵ��
protected:
	DECLARE_MESSAGE_MAP()
	CFont  fontTitle;
public:
	afx_msg void OnStnClickedGxerweima();
	afx_msg HBRUSH OnCtlColor(CDC* pDC, CWnd* pWnd, UINT nCtlColor);
	//��Ϣ��ȡ
	static unsigned WINAPI __stdcall GetLANAccountMsg(void *para);
	CWnd *parent;
	void ShowMsg(BOOL bForce);
};
double GetDPIX(double X);
double GetDPIY(double Y);
void moveWindow(CWnd &wnd);
void SetDPI(int x, int y);