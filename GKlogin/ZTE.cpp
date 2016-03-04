#include "stdafx.h"
#include "GKlogin.h"
#include "GKloginDlg.h"
#include "AutoUpdate.h"
#include "Winsvc.h"
#include "WebAuth.h"
#include "Packet.h"
#include "aes.h"

#define MAX_BUFFER_SIZE 102040

CPacket Plogin;
void CGKloginDlg::initialize()
{
	//��ʼ������
	status = INIT;
	m_bAuth = FALSE;
	m_DHCPThread = NULL;
	m_WebAuthThread = NULL;
	m_WeblogoutThread = NULL;
	m_AuthThread = NULL;
	m_HttpHeartThread = NULL;
	m_NetFluxThread = NULL;
	m_GetLANAccountMsgThread = NULL;

	//�޸��б�ؼ�
	editLog = (CEdit*)GetDlgItem(IDC_EDIT_LOG);
	GetWindowRect(&m_rc);

	m_rc.top = m_rc.bottom - 5;    //����״̬���ľ�������
	m_StatusBar.Create(WS_CHILD | WS_VISIBLE | CBRS_BOTTOM, m_rc, this, 20000);
	
	int nParts[6] = { (int)GetDPIX(120), (int)GetDPIX(250), (int)GetDPIX(400), (int)GetDPIX(585), (int)GetDPIX(680), -1 };      //�ָ�ߴ�
	m_StatusBar.SetParts(6, nParts);
	m_StatusBar.SetText(_T(""), 0, 0);
	m_StatusBar.SetText(_T(""), 1, 0);
	m_StatusBar.SetText(_T(""), 2, 0);
	m_StatusBar.SetText(_T(""), 3, 0);
	m_StatusBar.SetText(_T(""), 4, 0);
	m_StatusBar.SetText(_T(""), 5, 0);
	//����״̬������ʾ
	SetTimer(1, 1000, NULL);
	//��ʼ��ʱ���Ƚ���־������
	OnLogshow();

	extern int dpiX;
	Log(I_MSG, "��Ļdpi:%d", dpiX);
	Log(I_MSG, "����汾:%s", STR_Version);
	Log(I_MSG, "Winpcap�汾:%s", pcap_lib_version());
	Log(I_MSG, "����ϵͳ�汾��%s", GetOSVersion());


	//�ƶ������ļ�
	char szTemp[MAX_STRING];
	Config.GetFullPathToFile(szTemp, "zte.conf");
	if ((_access(szTemp, 0)) == 0){
		char pszFullPath[MAX_STRING];
		char pszFilename[MAX_STRING] = CONFIGNAME;
		Config.GetFullPathToFile(pszFullPath, pszFilename);
		MoveFile(_T(szTemp), pszFullPath);
	}
    Config.GetDocumentsPath(szTemp, CONFIGNAME);
	if ((_access(szTemp, 0)) == 0){
		char pszFullPath[MAX_STRING];
		char pszFilename[MAX_STRING] = CONFIGNAME;
		Config.GetFullPathToFile(pszFullPath, pszFilename);
		MoveFile(_T(szTemp), pszFullPath);
	}
	//////////////////////////////////////////////////////////////////////////
	//�����˺���Ϣ
	Config.LoadConfig();
	//�Զ��޸Ĵ�������
	if (Config.m_CYClient){
		AfxGetMainWnd()->SetWindowText(STR_AppName "(��Ժ��)");
		GetDlgItem(IDC_Feedback)->EnableWindow(FALSE);
	}
	CheckDlgButton(IDC_REMEMBER, Config.m_bRememberPWD ? BST_CHECKED : BST_UNCHECKED);

	CString user, pass;
	POSITION p = Config.m_UserInfo.GetStartPosition();
	while (p != NULL) {
		Config.m_UserInfo.GetNextAssoc(p, user, pass);
		m_ccb_username.AddString(user);
	}
	int k = m_ccb_username.FindStringExact(-1, _T(Config.m_csLastUser));
	if (k < 0) k = 0;
	if (!Config.m_UserInfo.IsEmpty()) {
		m_ccb_username.SetCurSel(k);
		m_ccb_username.GetWindowText(user);
		if (Config.m_bRememberPWD)
			GetDlgItem(IDC_PWD)->SetWindowText(Config.m_UserInfo[user]);
	}
	//////////////////////////////////////////////////////////////////////////
	//����������Ϣ	
	char errorBuffer[PCAP_ERRBUF_SIZE];		//������Ϣ������
	pcap_if_t		* allAdapters;				//�������б�
	if (pcap_findalldevs(&allAdapters, errorBuffer) == -1 || allAdapters == NULL)
	{
		int result = MessageBox(_T("��ȡ������Ϣʧ�ܣ�����WinPcapû�а�װ����������û�а�װ!�Ƿ�װWinPcap��"), _T("����"), MB_ICONQUESTION | MB_YESNO);
		pcap_freealldevs(allAdapters);
		switch (result)//ע�⣡ʹ��UnicodeӦ��TEXT��Χ�ִ�
		{
		case IDYES:
			if ((_access("WinPcap.exe", 0)) != -1)
			{
				::ShellExecute(this->GetSafeHwnd(), _T("open"), _T(".\\WinPcap.exe"), NULL, NULL, SW_SHOWNORMAL);
				OnExit();
				return;
			}
			else
			{
				AfxMessageBox(_T("�Ҳ���WinPcap��װ�������ֶ����ذ�װ��"), MB_TOPMOST);
				OnExit(); return;
			}
		case IDNO:
			OnExit();
			return;
		}
	}
	k = 0; m_csAdapters.RemoveAll();
	pcap_if_t* adapter;//��ʱ���������
	char *szGuid = NULL;
	Log(I_INFO, "LastNetcard:%s", Config.m_csNetCard);
	for (k = 0, adapter = allAdapters; adapter != NULL; adapter = adapter->next) {
		if (adapter->flags & PCAP_IF_LOOPBACK) continue;
		szGuid = GetGUID(adapter->name);
		if (Config.m_bAutoFilter == 1 && strcmp(adapter->description, "Microsoft") == 0)
			continue;
		if (Config.m_bAutoFilter == 1 && strstr(adapter->description, "VMware") != NULL)
			continue;
		Log(I_INFO, "load netcard:(%d)%s(%s)", k, szGuid, adapter->description);
		m_csAdapters.Add(szGuid); m_ccbNetCard.AddString(adapter->description);
		k++;
	}
	if ((strlen(Config.m_csNetCard) == NULL) || ((TestAdapter(Config.m_csNetCard) != 0)&&(strlen(Config.m_csNetCard) != NULL)))
	{
		bool NetError=FALSE;
		if ((TestAdapter(Config.m_csNetCard) != 0) && (strlen(Config.m_csNetCard) != NULL)){
			NetError = TRUE;
		}
		else
			if (strlen(Config.m_csNetCard) == NULL)
				Config.m_csNetCard = m_csAdapters[0];//������ʱ�Զ�ѡ���һ������
		for (k = 0, adapter = allAdapters; adapter != NULL; adapter = adapter->next, k++)
		{
			szGuid = GetGUID(adapter->name);
			if (strcmp(adapter->description, "Microsoft") != 0)
			{
				if (TestAdapter(szGuid) == 0){
					Config.m_csNetCard = szGuid;
					if (NetError)this->Log(I_ERR, "�ϴε�½ѡ�������Ϊ�������������Զ�������");
				}
			}
		}

	}
	int i = 0;
	for (k = m_ccbNetCard.GetCount() - 1; k >= 0; k--)
	{
		if (_stricmp(m_csAdapters[k], Config.m_csNetCard) == 0)
		{
			i = 1;
			break;
		}
	}
	pcap_freealldevs(allAdapters);
	if (i == 0)
	{
		k = 0;
	}
	if (m_ccbNetCard.GetCount() == 0)
	{
		this->Log(I_ERR, "����ѡ��Ϊ�գ������Ƿ�װ�����������Ƿ�����������������������򣡣�");
	}
	if (k >= 0 && m_ccbNetCard.GetCount() > k) {
		m_ccbNetCard.SetCurSel(k);
		Log(I_INFO, "select netcard:(%d)%s", k, m_csAdapters[k]);
	}

	this->Log(I_MSG, "�����������");

	//////////////////////////////////////////////////////////////////////////

	//ʹ�ÿ�ʼ��ť��Ч�����Ͽ���ť��Ч
	UpdateStatus(FALSE);

	//�Զ���½ѡ��
	//��Ĭ����
	extern int __argc;
	extern char**  __argv;
	bool Forcelogin = FALSE;
	if (__argc == 3){
		if (strcmp(__argv[1], "-updated") == 0){
			Forcelogin = TRUE;
			if (!Config.m_SlienceUp)
				AfxMessageBox("�������!", MB_TOPMOST);
		}
	}
	//�汾�űȽ�
	char LastVersion[20];
	strcpy(LastVersion, Config.LastVersion);
	if (CompareVersion(LastVersion, STR_Version))
	{
		//Config.m_csHeartUrl = "http://www.qq.com";
		Config.m_bHttpHeart = 1;
		Config.m_bAutoUpdate = 1;
		Config.m_SlienceUp = 1;
		Config.m_UEIP = 1;
		//Config.m_AutoBas = 0;
		Config.m_csWebAuthUrl.Replace("enet.10000.gd.cn","125.88.59.131");
		Config.m_csWebLogoutUrl.Replace("enet.10000.gd.cn", "125.88.59.131");
	}
	if (Config.m_iHeartInterval <= 0 && Config.m_iHeartInterval >= 60)Config.m_iHeartInterval = 30;
	if (k >= 0 && Config.m_bAutologon == TRUE || Forcelogin)
	{
		OnStart();
	}
	//SetProcessWorkingSetSize(GetCurrentProcess(), -1, -1);
}
char *CGKloginDlg::GetOSVersion()
{
	static char ver[MAX_STRING];
	OSVERSIONINFO os;
	strncpy(ver, "unknown", MAX_STRING);
	ZeroMemory(&os, sizeof(OSVERSIONINFO));
	os.dwOSVersionInfoSize = sizeof(OSVERSIONINFO);
	if (GetVersionEx(&os)) {
		_snprintf(ver, MAX_STRING, "%d.%d.%d.%d.%s",
			os.dwMajorVersion, os.dwMinorVersion, os.dwBuildNumber,
			os.dwPlatformId, os.szCSDVersion);
	}
	return ver;
}
int CGKloginDlg::CheckUpdate()
{
	int ret = AutoUpdate(Config.m_SlienceUp);
	if (ret == 0) {
		if (this->m_bAuth) {
			this->OnLogoff();
		}
		this->PostMessage(WM_QUIT);
		//	Sleep(1000);
		TerminateProcess(GetCurrentProcess(), 0);
	}
	else if (ret == 4) { return 2; }
	else if (ret == -1) { return -1; }
	return 1;
}
int CGKloginDlg::CheckWinPcapUpdate()
{

	int ret = AutoUpdateWinPcap();
	if (ret == 0) {
		if (this->m_bAuth) {
			this->OnLogoff();
		}
		this->PostMessage(WM_QUIT);
		//	Sleep(1000);
		TerminateProcess(GetCurrentProcess(), 0);
	}
	else if (ret == 4) { return 2; }
	else if (ret == -1) { return -1; }
	return 1;
}

char *	CGKloginDlg::GetGUID(const char *name)
{
	static char GUID[MAX_STRING];

	if (name == NULL) return NULL;
	const char *a = strstr(name, "{");
	if (a == NULL) return NULL;
	const char *b = strstr(a, "}");
	if (b == NULL) return NULL;

	memcpy(GUID, a, b - a + 1);
	GUID[b - a + 1] = '\0';
	return GUID;
}
//////////////////////////////////////////////////////////////////////////
//����
//////////////////////////////////////////////////////////////////////////
void CGKloginDlg::UpdateStatus(bool bOnline)
{

	const char *m2[] = { "Offline", "Online" };
	m_bAuth = bOnline;
	int i = (bOnline ? 1 : 0);

	this->Log(I_INFO, "status:%s", m2[i]);

	if (m_bAuth) {
		m_startTime = time(NULL);
		//ShowWindow(SW_HIDE);
	}

	GetDlgItem(IDC_USERNAME)->EnableWindow(!bOnline);
	GetDlgItem(IDC_PWD)->EnableWindow(!bOnline);
	GetDlgItem(IDC_NETCARD)->EnableWindow(!bOnline);
	GetDlgItem(IDC_START)->EnableWindow(!bOnline);
	GetDlgItem(IDC_LOGOFF)->EnableWindow(bOnline);
}
void CGKloginDlg::OnSelchangeUsername()
{
	CString str;
	m_ccb_username.GetLBText(m_ccb_username.GetCurSel(), str);
	GetDlgItem(IDC_PWD)->SetWindowText(Config.m_UserInfo[str]);
}

void CGKloginDlg::OnTimer(UINT nIDEvent)
{
	static char *csStatus[] = { "��ʼ�����", "������֤��֤��", "������֤�ɹ�", "���ڻ�ȡIP��ַ", "�ɹ���ȡIP��ַ",
		"��ҳ��֤��", "��ҳ��֤�ɹ�", "����", "����" };

	static STATUS lastStatus = INIT;

	time_t t = time(NULL);
	char szTime[MAX_STRING], Text[MAX_STRING];
	strftime(szTime, MAX_STRING, "%H:%M:%S", localtime(&t));
	m_StatusBar.GetText(Text, 0, 0);
	if (strstr(Text, "�����ӳ�") == 0)
		m_StatusBar.SetText(szTime, 0, 0);
	if (status == ONLINE)
	{
		if (!bConnected){
			if (Config.m_Reauth)
			{
				SendMessage(WM_COMMAND, MAKEWPARAM(IDC_LOGOFF, 0));
				bRetryConnect = 1;
			}
		}
	}
	if (m_bAuth == 0 && Config.m_bTimingReauth && _stricmp(szTime, Config.m_csReauthTime) == 0) {
		//OnLogoff();
		SendMessage(WM_COMMAND, MAKEWPARAM(IDC_START, 0));
	}

	if (m_bAuth) {
		t -= m_startTime;
		strftime(szTime, MAX_STRING, "����ʱ��:%H:%M:%S", gmtime(&t));
		m_StatusBar.SetText(szTime, 1, 0);
	}
	else {
		struct tm *t;
		time_t tt;
		time(&tt);
		t = localtime(&tt);
		sprintf(szTime, "%4d��%02d��%02d��", t->tm_year + 1900, t->tm_mon + 1, t->tm_mday);
		m_StatusBar.SetText(szTime, 1, 0);
	}
	m_StatusBar.SetText(csStatus[status], 2, 0);
	if (lastStatus != status) {
		lastStatus = status;
		SetBubble("״̬", csStatus[status]);
	}
	CDialog::OnTimer(nIDEvent);
}
//////////////////////////////////////////////////////////////////////////
//��ʾ
//////////////////////////////////////////////////////////////////////////
void CGKloginDlg::SetBubble(char * title, char * content, int timeout)
{
	if (Config.m_bShowBubble == TRUE)
	{
		m_tray.SetBubble(title, content, timeout);
	}
}
void CGKloginDlg::Log(int level, const char *fmt, ...)
{
	va_list args;
	char msg[MAX_STRING];
	va_start(args, fmt);
	vsnprintf(msg, MAX_STRING, fmt, args);
	va_end(args);

	char szTime[MAX_STRING];
	if (level != I_REM){
		time_t t = time(NULL);
		strftime(szTime, MAX_STRING, "%H:%M:%S", localtime(&t));
		strcat(szTime, " "); strcat(szTime, msg);
	}
	else{
		strcpy(szTime, msg);
	}
	strcat(szTime, "\r\n");
	
	if (level == I_INFO && !Config.m_bDebug) return;

	int nLength = editLog->SendMessage(WM_GETTEXTLENGTH);
	editLog->SetSel(nLength, nLength);
	editLog->ReplaceSel(szTime);
	editLog->SendMessage(WM_VSCROLL, SB_BOTTOM, 0);


	if (level == I_WARN) SetBubble("Warnning", szTime);
	if (level == I_ERR) AfxMessageBox(msg, MB_ICONSTOP |  MB_TOPMOST, 0);
}
void CGKloginDlg::OnLogshow()
{
	static int bShow = TRUE;
	RECT rc1 = { 0 }, rc2 = { 0 };
	if (bShow)
	{
		logshowstatus = 1;
		GetDlgItem(IDC_SPLIT)->GetWindowRect(&rc1);
		GetWindowRect(&m_rc);
		rc2 = m_rc;
		rc2.right = rc1.left - 2;
		MoveWindow(&rc2, TRUE);
		bShow = !bShow;
		GetDlgItem(IDC_LOGSHOW)->SetWindowText(_T("��־>>"));
	}
	else
	{
		logshowstatus = 0;
		GetWindowRect(&rc1);

		m_rc.right += rc1.left - m_rc.left;
		m_rc.bottom += rc1.top - m_rc.top;
		m_rc.left = rc1.left;
		m_rc.top = rc1.top;

		MoveWindow(&m_rc, TRUE);
		bShow = !bShow;
		GetDlgItem(IDC_LOGSHOW)->SetWindowText(_T("��־<<"));
	}
}

char *	CGKloginDlg::ToTCPName(const char *GUID)
{
	static char NPFGUID[MAX_STRING] = "\\Device\\Tcpip_";
	strncpy(NPFGUID + 14, GUID, MAX_STRING - 14);

	return NPFGUID;
}

char *	CGKloginDlg::ToNPFName(const char *GUID)
{
	static char TCPGUID[MAX_STRING] = "\\DEVICE\\NPF_";
	strncpy(TCPGUID + 12, GUID, MAX_STRING - 12);
	return TCPGUID;
}
DWORD WINAPI CGKloginDlg::GetMacIP(const char *adaptername, char *ip, unsigned char *mac)
{
	PIP_ADAPTER_INFO AdapterInfo = NULL;

	DWORD dwBufLen = sizeof(IP_ADAPTER_INFO);
	DWORD dwStatus;
	AdapterInfo = (PIP_ADAPTER_INFO)malloc(dwBufLen);
	if (AdapterInfo == NULL) return -1;
	dwStatus = GetAdaptersInfo(AdapterInfo, &dwBufLen);
	if (dwStatus == ERROR_BUFFER_OVERFLOW) {
		free(AdapterInfo);
		AdapterInfo = (PIP_ADAPTER_INFO)malloc(dwBufLen);
		dwStatus = GetAdaptersInfo(AdapterInfo, &dwBufLen);
		if (AdapterInfo == NULL) return -1;
	}
	if (dwStatus != NO_ERROR) {
		if (AdapterInfo != NULL) free(AdapterInfo);
		return -2;
	}

	if (ip) memset(ip, 0, 16);
	if (mac) memset(mac, 0, 6);

	PIP_ADAPTER_INFO pAdapterInfo = AdapterInfo;
	if (adaptername != NULL) {
		while (pAdapterInfo) {
			if (_stricmp(adaptername, pAdapterInfo->AdapterName) == 0) {
				if (mac) memcpy(mac, pAdapterInfo->Address, 6);
				if (ip) strncpy(ip, pAdapterInfo->IpAddressList.IpAddress.String, 16);
				break;
			}
			pAdapterInfo = pAdapterInfo->Next;
		}
	}

	free(AdapterInfo);
	if (pAdapterInfo == NULL) return -3;
	return 0;
}
int CGKloginDlg::TestAdapter(const char *name)
{
	//Ѱ����ѡ��������MAC
	char errbuf[PCAP_ERRBUF_SIZE];   /* error buffer */
	u_char mac[6];
	if (GetMacIP(name, NULL, mac) != 0) return -1;
	//////////////////////////////////////////////////////////////////////////
	// ��ָ��������
	pcap_t *handle = pcap_open_live(ToNPFName(name), 65536, 1, Config.m_iTimeout, errbuf);
	if (handle == NULL) {
		Log(I_INFO, "pcap_open_live:%s", errbuf);
		return -2;
	}

	char	FilterStr[100];		//�������ַ���
	struct bpf_program	mfcode;

	sprintf(FilterStr, "(ether proto 0x888e) and (ether dst host %02x:%02x:%02x:%02x:%02x:%02x)",
		mac[0], mac[1], mac[2], mac[3], mac[4], mac[5]);

	int retcode;
	if ((retcode = pcap_compile(handle, &mfcode, FilterStr, 1, 0xff)) == -1 || (retcode = pcap_setfilter(handle, &mfcode)) == -1)
	{
		pcap_close(handle);
		return -3;
	}

	//////////////////////////////////////////////////////////////////////////
	///��ʼ��֤��
	Plogin.initialize((char *)User.m_username, (char *)User.m_password, User.m_usernameLen, User.m_usernameLen, handle, mac, this);
	Plogin.init_frames();
    if (!(retcode = Plogin.send_eap_packet(Plogin.EAPOL_START))) {
		pcap_close(handle); return -4;
	}
	u_char *captured = NULL;
	struct pcap_pkthdr      *header = NULL;
	char m_errorBuffer[PCAP_ERRBUF_SIZE];		//������Ϣ������
	retcode = pcap_next_ex(handle, &header, (const u_char **)&captured);
		//	 /*(EAP_Code)*/captured[15] == 0 && captured[12] == 0x88 && captured[13] == 0x8e && captured[18] == 2)
	if (retcode == -1)
	{
		retcode = pcap_next_ex(handle, &header, (const u_char **)&captured);
		strncpy(m_errorBuffer, pcap_strerror(retcode), PCAP_ERRBUF_SIZE);
		Log(I_INFO, "����������Ϣ��%s", m_errorBuffer);
		pcap_close(handle);
		return -5;
	}
	int i = 1;
	//Log(I_INFO, "��0�Σ�%d", retcode);
	for (; retcode == 0;i++){
		retcode = pcap_next_ex(handle, &header, (const u_char **)&captured);
		//Log(I_INFO, "��%d�Σ�%d", i, retcode);
		if (i >= 2)
		{
			pcap_close(handle);
			return -6;//��ʱ
		}
	}
	pcap_close(handle);
	return 0;//�ɹ�
}
void CGKloginDlg::OnStart()
{
	UpdateStatus(TRUE);

	bConnected = 1;
	bRetryConnect = 0;
	//////////////////////////////////////////////////////////////////////////
	//		�ȴ�����ò���
	CString strTemp;

	//ȡ���û�������
	GetDlgItem(IDC_USERNAME)->GetWindowText((char*)User.m_username, sizeof(User.m_username));
	User.m_usernameLen = strlen((char*)User.m_username);
	GetDlgItem(IDC_PWD)->GetWindowText((char*)User.m_password, sizeof(User.m_password));
	User.m_passwordLen = strlen((char*)User.m_password);
	
	
	if (User.m_usernameLen<1 || User.m_passwordLen<1)	{
		this->Log(I_ERR, "�û�����������̫��!"); UpdateStatus(FALSE); return;
	}
	if (((CComboBox*)GetDlgItem(IDC_NETCARD))->GetCurSel() == -1)
	{
		this->Log(I_ERR, "ѡ����������!"); UpdateStatus(FALSE); return;
	}//��������û��ѡ�����������δ֪����������
	CString NetCard = m_csAdapters[((CComboBox*)GetDlgItem(IDC_NETCARD))->GetCurSel()];

	if (Config.m_csNetCard.CompareNoCase(NetCard) != 0 && Config.m_csNetCard.CompareNoCase("") != 0){
		int result = AfxMessageBox(_T("��⵽�������ϴ�ѡ���������ͬ��ȷ���޸�������\n\rע�⣺�޸������ᵼ���´������������"), MB_ICONQUESTION | MB_YESNO |  MB_TOPMOST);
		if (result == IDNO)
		{
			UpdateStatus(FALSE);
			return;
		}
	}
	Config.m_csNetCard = m_csAdapters[((CComboBox*)GetDlgItem(IDC_NETCARD))->GetCurSel()];

	if (IsDlgButtonChecked(IDC_REMEMBER)) {
		Config.m_bRememberPWD = TRUE;
	}
	else {
		Config.m_bRememberPWD = FALSE;
	}

	Config.m_csLastUser = (char*)User.m_username;
	if (Config.m_bRememberPWD)
		Config.m_UserInfo[(char*)User.m_username] = (char*)User.m_password;

	Config.SaveConfig();
	//////////////////////////////////////////////////////////////////////////
	//�Զ�����Ϊ�Զ���ȡip
	if (DHCP_Auto() == 0)return;
	WebPop();
	//////////////////////////////////////////////////////////////////////////
	SetBubble("��ʾ", STR_AppName" ��ʼ������֤����");

	char m_errorBuffer[PCAP_ERRBUF_SIZE];
	int retcode = 0;
	/////////////////////////////////////////////////////////////////////////
	//Ѱ����ѡ��������MAC	
	if (GetMacIP(Config.m_csNetCard, NULL, User.m_MacAdd) != 0)
	{
		if (User.m_MacAdd[0] == 0 && User.m_MacAdd[1] == 0 && User.m_MacAdd[2] == 0 &&
			User.m_MacAdd[3] == 0 && User.m_MacAdd[4] == 0 && User.m_MacAdd[5] == 0)
		{
			Log(I_ERR, "��ȡ������ѡ������MAC��ַ"); UpdateStatus(FALSE);
			return;
		}
	}
	else
	{
		Log(I_INFO, "MAC:%02X-%02X-%02X-%02X-%02X-%02X", User.m_MacAdd[0], User.m_MacAdd[1], User.m_MacAdd[2],
			User.m_MacAdd[3], User.m_MacAdd[4], User.m_MacAdd[5]);
	}
	//////////////////////////////////////////////////////////////////////////
	// ��ָ��������
	m_adapterHandle = pcap_open_live(ToNPFName(Config.m_csNetCard), 65536, 1, Config.m_iTimeout, m_errorBuffer);
	if (m_adapterHandle == NULL) {
		Log(I_INFO, "pcap_open_live:%s", m_errorBuffer);
		UpdateStatus(FALSE);
		return;
	}

	char	FilterStr[100];		//�������ַ���
	struct bpf_program	mfcode;

	sprintf(FilterStr, "(ether proto 0x888e) and (ether dst host %02x:%02x:%02x:%02x:%02x:%02x)",
		User.m_MacAdd[0], User.m_MacAdd[1], User.m_MacAdd[2], User.m_MacAdd[3], User.m_MacAdd[4], User.m_MacAdd[5]);

	if ((retcode = pcap_compile(m_adapterHandle, &mfcode, FilterStr, 1, 0xff)) == -1 || (retcode = pcap_setfilter(m_adapterHandle, &mfcode)) == -1)
	{
		Log(I_INFO, "pcap_compile & pcap_setfilter:%s", pcap_strerror(retcode));
		pcap_close(m_adapterHandle); UpdateStatus(FALSE);
		return;
	}
	if (!Config.m_ZTEAuth){
		if (retcode == 0 && (Config.m_bWebAuth && Config.m_csWebAuthUrl.GetLength() > 0) && m_bAuth){
			status = HTTPING;
			StartWebAuth();
			//if(Config.m_bHttpHeart)	Dlg->OnHttpHeart();	
		}
		return ;
	}
	status = AUTHING;

	//////////////////////////////////////////////////////////////////////////
	//���������
	int msg;
	if (Config.m_bDHCP)
	{
		if (msg = StartService(_T("Dhcp")) == SERVICE_RUNNING)
			Log(I_MSG, "��⵽DHCP����ֹͣ��DHCP�������óɹ�");//����ϵͳDHCP����
		else if (msg != 0)
			Log(I_MSG, "DHCP��������ʧ�ܣ����ܻ�ȡ����IP,�������:%d", msg);
	}
	if (msg = StopService(_T("dot3svc")) == SERVICE_ACCEPT_STOP)
		Log(I_MSG, "ֹͣϵͳ�Դ�802.1x����ɹ�,����ʮ�����Ӷ�������");//ͣ��ϵͳ�Դ�802.1x����
	else if (msg != 0)
		Log(I_MSG, "ֹͣϵͳ�Դ�802.1x����ʧ��,���ܳ���ʮ�����Ӷ�������,�������:%d", msg);
	if (msg = StopService(_T("SharedAccess")) == SERVICE_ACCEPT_STOP)
	{
		ShareServiceStatus = 1;
		Log(I_MSG, "ֹͣϵͳICS����ɹ�,�����ȡIPʧ�ܣ���Ӱ��Wifi������");//ͣ��ϵͳICS����
	}
	else if (msg != 0)
		Log(I_MSG, "ֹͣϵͳICS����ʧ��,��ȡIPʧ�ܿ���ʧ��,�������:%d", msg);
	//////////////////////////////////////////////////////////////////////////

	Plogin.initialize((char *)User.m_username, (char *)User.m_password, User.m_usernameLen, User.m_usernameLen, m_adapterHandle, User.m_MacAdd,this);
	Plogin.init_frames();
	//////////////////////////////////////////////////////////////////////////
	///��ʼ��֤��
	if (!(retcode = Plogin.send_eap_packet(Plogin.EAPOL_START))) {
		UpdateStatus(FALSE);
		return;
	}
	Log(I_MSG, "������֤��ʼ...");
	m_AuthThread = (HANDLE)_beginthreadex(NULL, 0, eap_thread, this, 0, 0);
	if (m_AuthThread == NULL) Log(I_INFO, "(HANDLE)_beginthreadex:(%d)", GetLastError());
}
void CGKloginDlg::get_packet(u_char *args, const struct pcap_pkthdr *pcaket_header, const u_char *packet)
{
	int retcode;
	static int good = 0;
	static int ErrorKind = 0;
	CGKloginDlg *Dlg = (CGKloginDlg*)args;

	/* declare pointers to packet headers */
	retcode = Plogin.get_packet(args, pcaket_header, packet);
	if (retcode == Plogin.EAP_SUCCESS)
	{//successful
		Dlg->status = AUTHED;
		//SetProcessWorkingSetSize(GetCurrentProcess(), -1, -1);
		Dlg->UpdateStatus(TRUE);
		Dlg->Log(I_MSG, "������֤�ɹ�.");

		if (Dlg->m_DHCPThread == NULL)
			Dlg->m_DHCPThread = (HANDLE)_beginthreadex(NULL, 0, dhcp_thread, Dlg, 0, 0);
		//Dlg->ShowWindow(SW_HIDE);
		
		ErrorKind = 1;
	}
	if (retcode == Plogin.EAP_FAILURE)
	{//fail
		
		Dlg->UpdateStatus(FALSE);
		Dlg->Log(I_MSG, "������֤ʧ��.");

		Dlg->status = OFFLINE;

		if (Config.m_Reauth&&ErrorKind)
		{
			if (Config.m_iReauthlog == 1){
				Config.m_iReauthlog = 0;
				Sleep(1000);
				Dlg->SendMessage(WM_COMMAND, MAKEWPARAM(IDC_START, 0));
			}
		}
		Dlg->CloseThread();
		ExitThread(0);
		return;
	}
	if (retcode == Plogin.EAP_KEEP_ALIVE){
		if (good <= 3){ 
			good++; 
			if(good==3) 
				Dlg->Log(I_MSG, "��������...");
		}
		if (good >= 3) Dlg->status = ONLINE;

		//Dlg->HttpAuth();
	}
	return;
}
unsigned WINAPI __stdcall CGKloginDlg::eap_thread(void *para)
{
	CGKloginDlg *Dlg = (CGKloginDlg*)para;
	pcap_loop(Dlg->m_adapterHandle, -1, CGKloginDlg::get_packet, (u_char*)Dlg);

	pcap_close(Dlg->m_adapterHandle);
	return 0;
}
unsigned WINAPI __stdcall CGKloginDlg::dhcp_thread(void *para)
{
	CGKloginDlg *Dlg = (CGKloginDlg*)para;

	Dlg->status = DHCPING;
	int retcode = 0;
	Dlg->Log(I_MSG, "��ȡIP�߳�����");
	if (Config.m_bDHCP) {
		char *info = Dlg->GetAdapterInfo((LPCSTR)Config.m_csNetCard);
		if (info == NULL)retcode = Dlg->IpconfigRenew();
		else Dlg->Log(I_MSG, info);
		if (retcode != 0) Dlg->Log(I_MSG, "Ipconfig/Renew return %d", retcode);
		else Dlg->status = DHCPED;
	}
	Dlg->Log(I_MSG, "ֹͣ���Ի�ȡIP.");
	//��ȡ�˺���Ϣ
	Dlg->m_GetLANAccountMsgThread = (HANDLE)_beginthreadex(NULL, NULL, CComplaint::GetLANAccountMsg, Dlg, 0, NULL);
	if (Config.m_WebAuth&&retcode == 0 && (Config.m_bWebAuth && Config.m_csWebAuthUrl.GetLength() > 0) && Dlg->m_bAuth){
		Dlg->status = HTTPING;
		Dlg->StartWebAuth();
		//if(Config.m_bHttpHeart)	Dlg->OnHttpHeart();	
	}
	else
	{
		Sleep(2000);
		Dlg->HavingConnect();
	}
	Dlg->m_DHCPThread = NULL;
	return 0;
}
#define MAX_DHCP_TIMES	10

DWORD WINAPI CGKloginDlg::IpconfigRenew()
{
	int i;
	DWORD ret;
	char  adaptername[MAX_ADAPTER_NAME];
	int count = 0;

	DWORD dwOutBufLen = sizeof(IP_INTERFACE_INFO);
	PIP_INTERFACE_INFO pIfTable = (PIP_INTERFACE_INFO)malloc(dwOutBufLen);
	if (pIfTable == NULL) return -1;

	ret = GetInterfaceInfo(pIfTable, &dwOutBufLen);
	if (ret == ERROR_INSUFFICIENT_BUFFER) {
		free(pIfTable); pIfTable = (PIP_INTERFACE_INFO)malloc(dwOutBufLen);
		if (pIfTable == NULL) return -2;
		ret = GetInterfaceInfo(pIfTable, &dwOutBufLen);
	}

	if (ret != NO_ERROR)
	{
		if (ret == ERROR_NO_DATA) {
			Log(I_MSG, "��ȡ�����������ݣ�");
		}
		else{
			Log(I_MSG, "��ȡ������Ϣʧ�ܣ�������룺%d", ret);
		}
		return -3;
	}
	for (i = 0; i < pIfTable->NumAdapters; i++) {
		wcstombs(adaptername, pIfTable->Adapter[i].Name, MAX_ADAPTER_NAME);

		if (_stricmp(adaptername, ToTCPName(Config.m_csNetCard)) == 0) {

			EnableDHCP(GetGUID(adaptername), true);

			while (status != OFFLINE) {
				if (count <= MAX_DHCP_TIMES) Log(I_MSG, "���ڻ�ȡIP��ַ...");
				IpReleaseAddress(&pIfTable->Adapter[i]);
				ret = IpRenewAddress(&pIfTable->Adapter[i]);
				if (ret == NO_ERROR) break;
				if (count <= MAX_DHCP_TIMES) {
					if (count == MAX_DHCP_TIMES) {
						count++;
						Log(I_MSG, "�ﵽ��ȡIP�����������ޣ������Ƿ�����");
						break;
					}
					else {
						Log(I_WARN, "��ȡIP��ַʧ��,������룺%d�������Ƿ��ֶ�����IP�������²������߻�����������", ret);
						count++;
					}
				}
				Sleep(4000);
			}
			char *info = GetAdapterInfo((LPCSTR)Config.m_csNetCard);
			if (info != NULL) Log(I_MSG, info);

			break;
		}
	}
	free(pIfTable);
	return 0;
}
char *CGKloginDlg::GetAdapterInfo(const char *name)
{
	static char info[MAX_STRING];
	char *adaptername = ToTCPName(name);
	char temp[MAX_STRING];

	if (adaptername == NULL){ Log(I_MSG, "����������Ϣ����ȷ����ѡ����������"); return NULL; }

	_snprintf(temp, MAX_STRING, "adapter name:%s\r\n", adaptername); strncpy(info, temp, MAX_STRING);

	PIP_ADAPTER_INFO AdapterInfo = NULL;

	DWORD dwBufLen = sizeof(IP_ADAPTER_INFO);
	DWORD dwStatus;
	AdapterInfo = (PIP_ADAPTER_INFO)malloc(dwBufLen);
	if (AdapterInfo == NULL){ Log(I_MSG, "GetAdapterInfo:malloc1 failed"); return NULL; }
	dwStatus = GetAdaptersInfo(AdapterInfo, &dwBufLen);
	if (dwStatus == ERROR_BUFFER_OVERFLOW) {
		free(AdapterInfo);
		AdapterInfo = (PIP_ADAPTER_INFO)malloc(dwBufLen);
		if (AdapterInfo == NULL) { Log(I_MSG, "GetAdapterInfo:malloc2 failed"); return NULL; }
		dwStatus = GetAdaptersInfo(AdapterInfo, &dwBufLen);
	}
	if (dwStatus != NO_ERROR) {
		if (AdapterInfo != NULL) free(AdapterInfo);
		Log(I_MSG, "GetAdapterInfo:GetAdaptersInfo failed");
		return NULL;
	}

	if (name != NULL) {
		PIP_ADAPTER_INFO pAdapterInfo = AdapterInfo;
		while (pAdapterInfo) {
			if (_stricmp(name, pAdapterInfo->AdapterName) == 0) {
				_snprintf(temp, MAX_STRING, "Descript:%s\r\n", pAdapterInfo->Description);
				strncpy(info, temp, MAX_STRING);

				_snprintf(temp, MAX_STRING, "\tDHCP Enabled:%s", pAdapterInfo->DhcpEnabled ? "YES" : "NO");
				strcat(info, temp); strcat(info, "\r\n");

				_snprintf(temp, MAX_STRING, "\tIP:%s", pAdapterInfo->IpAddressList.IpAddress.String);
				strcat(info, temp); strcat(info, "\r\n");

				_snprintf(temp, MAX_STRING, "\tMASK:%s", pAdapterInfo->IpAddressList.IpMask.String);
				strcat(info, temp); strcat(info, "\r\n");

				_snprintf(temp, MAX_STRING, "\tGateWay:%s", pAdapterInfo->GatewayList.IpAddress.String);
				strcat(info, temp); strcat(info, "\r\n");

				if (MIB_IF_TYPE_ETHERNET != pAdapterInfo->Type) {
					_snprintf(temp, MAX_STRING, "\tType:%d [warning]", pAdapterInfo->Type);
					strcat(info, temp); strcat(info, "\r\n");
				}

				_snprintf(temp, MAX_STRING, "\tDHCP Server:%s", pAdapterInfo->DhcpServer.IpAddress.String);
				strcat(info, temp);
				break;
			}
			pAdapterInfo = pAdapterInfo->Next;
		};
		if ((strstr(pAdapterInfo->IpAddressList.IpAddress.String, "169.254") != 0) || (strstr(pAdapterInfo->IpAddressList.IpAddress.String, "192.168") != 0)){
			free(AdapterInfo);  
			Log(I_MSG, "��ȡIPʧ�ܣ�"); 
			return NULL;
		}
		if (pAdapterInfo && Config.m_bDHCP && !pAdapterInfo->DhcpEnabled) {
			Log(I_WARN, "DHCP Config not match ! ");
			Log(I_MSG, "DHCP is set to fetch IP. ");
		}
	}
	else {
		Log(I_MSG, "GetAdapterInfo:DescriptionToName failed");
		return NULL;
	}
	free(AdapterInfo);
	return info;
}
//////////////////////////////////////////////////////////////////////////
//��ҳ��֤
//////////////////////////////////////////////////////////////////////////
void CGKloginDlg::StartWebAuth()
{
	if (m_WebAuthThread) {
		TerminateThread(m_WebAuthThread, 0); m_WebAuthThread = NULL;
		Log(I_MSG, "ֹͣ��ҳ��֤");
	}
	
		m_WebAuthThread = (HANDLE)_beginthreadex(NULL, NULL, web_auth_thread, this, 0, NULL);

	Log(I_MSG, "��ʼ��ҳ��֤����");
}
char * CGKloginDlg::HttpAuth(BOOL bForce = FALSE)
{
	char *msg = NULL;
	if ((Config.m_bWebAuth && Config.m_csWebAuthUrl.GetLength() > 0) || bForce) {
		if (Config.m_bEnableWebAccount && (strcmp(Config.m_csWebUsername, "") != 0)) {
			msg = WebAuth(Config.m_csWebUsername, Config.m_csWebPassword, User.m_ip, Config.m_csWebAuthUrl, Config.m_iTimeout, Config.m_csHeartUrl);
		}
		else {
			msg = WebAuth((const char *)User.m_username, (const char *)User.m_password, User.m_ip, Config.m_csWebAuthUrl, Config.m_iTimeout, Config.m_csHeartUrl);
		}
		//if(strcmp("������֤��ʱ", msg) == 0) Config.m_iTimeout++;
	}
	return msg;
}

unsigned __stdcall CGKloginDlg::web_auth_thread(void *para)
{
	CGKloginDlg* Dlg = (CGKloginDlg*)para;

	char *msg = NULL;
	int Errortry = 0;
	while (Dlg->status != OFFLINE) {
		if (Dlg->GetMacIP(Config.m_csNetCard, Dlg->User.m_ip, Dlg->User.m_MacAdd) == 0) {
			msg = Dlg->HttpAuth(FALSE);
			if (msg == NULL) {
				Dlg->Log(I_MSG, "��ҳ��֤�ɹ�.");
				Dlg->Log(I_MSG, "��ҳ��֤������ֹͣ");
				Dlg->ShowWindow(SW_HIDE);
				Dlg->status = HTTPED;
				if (Config.m_bHttpHeart) Dlg->OnHttpHeart();
				break;
			}
			else {
				Dlg->Log(I_MSG, "��ҳ��֤���󣬷�������Ϣ:%s", msg);
				//WebLogout(Dlg->User.m_ip, Config.m_csWebLogoutUrl, Config.m_iTimeout);
				Errortry++;
				if (Errortry == 5){
					Dlg->Log(I_MSG, "��ҳ��֤ʧ�ܣ����ԶϿ�Wifi");
					ShellExecute(NULL, _T("open"), "netsh.exe", "wlan disconnect", _T(""), SW_HIDE);
				}
			}
		}
		Sleep(2000);
	}
	if (msg != NULL)
	{
		Dlg->WebFaultMsg(msg);
	}
	if (Dlg->status == ONLINE || Dlg->status == HTTPED)
	{
		if (Dlg->ShareServiceStatus)
		{
			if (int ErrorMSG = Dlg->StartService(_T("SharedAccess")) == SERVICE_RUNNING)
			{
				Dlg->Log(I_MSG, "��������ICS���񣬻ָ�Wifi����");
				Dlg->ShareServiceStatus = 0;
			}
			else if (ErrorMSG == 0)
				Dlg->Log(I_MSG, "ICS����������");
			else
				Dlg->Log(I_MSG, "ICS��������ʧ�ܣ�Wifi������δ�ָܻ������ֶ�����Wifi,�������:%d", ErrorMSG);
		}
		if (Config.m_Ping)
		{
			//Dlg->m_pingThread.StartPing(1, "www.baidu.com", Config.m_csZTEServer.Mid(7), Dlg->m_hWnd, Dlg->User.m_ip);
		}
		Dlg->Log(I_MSG, "�ѿ�����������");

		Config.m_iReauthlog = 1;

		Dlg->GetAccountMsg();
		Dlg->HavingConnect();
	}

	Dlg->m_WebAuthThread = NULL;
	return 0;
}
int CGKloginDlg::HavingConnect() {
	status = ONLINE;
	if (m_NetFluxThread == NULL) {
		m_NetFluxThread = (HANDLE)_beginthreadex(NULL, 0, NetFlux, this, 0, 0);
	}
	if (Config.m_bAutoUpdate == 1)
	{
		CheckUpdate();
		//Dlg->CheckWinPcapUpdate();
	}
	return 0;
}
char * CGKloginDlg::HttpAuthOut(BOOL bForce = FALSE)
{
	char *msg = NULL;
	if ((Config.m_bWebLogout && Config.m_csWebLogoutUrl.GetLength() > 0) || bForce) {
		msg = WebLogout(User.m_ip, Config.m_csWebLogoutUrl, Config.m_iTimeout);
		//if(strcmp("������֤��ʱ", msg) == 0) Config.m_iTimeout++;
	}
	return msg;
}

unsigned WINAPI __stdcall CGKloginDlg::logout_thread(void *para)
{
	CGKloginDlg* Dlg = (CGKloginDlg*)para;
	char *msg = NULL;
	if (Config.m_WebAuth){
		if (Dlg->status == HTTPED || Dlg->status == ONLINE){
			if (Dlg->GetMacIP(Config.m_csNetCard, Dlg->User.m_ip, NULL) == 0) {
				msg = Dlg->HttpAuthOut(FALSE);
				if (msg != NULL)
				{
					Dlg->Log(I_MSG, "%s", msg);
				}
				else
				{
					Dlg->Log(I_MSG, "��ҳ��֤����ʧ�ܣ��޷�����Ϣ");
				}
			}
		}
	}
	if (Config.m_ZTEAuth){
		msg = pcap_geterr(Dlg->m_adapterHandle);
		if (msg != NULL){
			if (strcmp(msg, "") == 0)
			{
				Plogin.send_eap_packet(Plogin.EAPOL_LOGOFF);
				//pcap_close(m_adapterHandle);
				pcap_breakloop(Dlg->m_adapterHandle);
				Dlg->Log(I_MSG, "������֤��ע��.");
			}
			else
			{
				if (strcmp(msg, "������������"))
					Dlg->Log(I_MSG, "������֤ע��ʧ�ܣ�����������", msg);
				else
					Dlg->Log(I_MSG, "������֤ע��ʧ�ܣ�������Ϣ��%s", msg);
			}
		}
		if (::WaitForSingleObject(Dlg->m_AuthThread, 300) == WAIT_TIMEOUT)
		{
			::TerminateThread(Dlg->m_AuthThread, 0); Dlg->m_AuthThread = NULL;
		}
	}
	Dlg->UpdateStatus(FALSE);
	Dlg->status = OFFLINE;
	if (Dlg->bRetryConnect)
		Dlg->SendMessage(WM_COMMAND, MAKEWPARAM(IDC_START, 0));
	Dlg->CloseThread();
	Dlg->m_WeblogoutThread = NULL;
	return 0;
}
void CGKloginDlg::CloseThread()
{
	Plogin.exit_flag = 1;
	//�ȴ������300ms��û���Զ��˳�����ǿ�ƽ���
	if (::WaitForSingleObject(m_HttpHeartThread, 300) == WAIT_TIMEOUT)
	{
		::TerminateThread(m_HttpHeartThread, 0); m_HttpHeartThread = NULL;
	}
	if (::WaitForSingleObject(m_DHCPThread, 300) == WAIT_TIMEOUT)
	{
		::TerminateThread(m_DHCPThread, 0); m_DHCPThread = NULL;
	}
	if (::WaitForSingleObject(m_NetFluxThread, 300) == WAIT_TIMEOUT)
	{
		::TerminateThread(m_NetFluxThread, 0); m_NetFluxThread = NULL;
	}
	if (::WaitForSingleObject(m_GetLANAccountMsgThread, 300) == WAIT_TIMEOUT)
	{
		::TerminateThread(m_GetLANAccountMsgThread, 0); m_GetLANAccountMsgThread = NULL;
	}
	m_StatusBar.SetText(_T(""), 4, 0);
	m_StatusBar.SetText(_T(""), 3, 0);
}
///////////////////////////////////////////////////////////////////
//��ҳ����
/////////////////////////////////////////////////////////////////
void CGKloginDlg::OnHttpHeart()
{
	// TODO: Add your control notification handler code here

	if (m_HttpHeartThread) {
		Log(I_MSG, "ֹͣ���������̣߳�һ��ʱ����û�����ݰ�������ҳ��֤�����Զ��˳�");
		TerminateThread(m_HttpHeartThread, 0); m_HttpHeartThread = NULL;
		//GetDlgItem(IDC_HTTP_HEART)->SetWindowText(_T("��ʼ��ҳ����"));
	}
	else {
		Log(I_MSG, "��ʼ���������߳�");
		m_HttpHeartThread = (HANDLE)_beginthreadex(NULL, NULL, http_heart_thread, this, 0, NULL);
		//GetDlgItem(IDC_WEB_AUTH)->EnableWindow(FALSE);
		//GetDlgItem(IDC_HTTP_HEART)->SetWindowText(_T("ֹͣ��ҳ����"));
	}
}
unsigned WINAPI __stdcall CGKloginDlg::http_heart_thread(void *para)
{
	CGKloginDlg* Dlg = (CGKloginDlg*)para;
	char *msg = NULL;
	static int lastid = 0, id = 0;
	while (Dlg->status == ONLINE || Dlg->status == Dlg->HTTPED) {
		if (Config.m_Reauth){
			Dlg->NetStatus();
		}
		if (id != lastid&&id != 0)
		{
			lastid = id;
			Dlg->Notice(id);
		}
		if (id == -233)
			Dlg->SendMessage(WM_COMMAND, MAKEWPARAM(IDC_LOGOFF, 0));
		//DownLoadFileToBuffer(NULL, 0, Config.m_csHeartUrl, Dlg->User.m_ip, DOWNTIMEOUT, Config.m_csHeartCookies, NULL, NULL);
		Sleep(Config.m_iHeartInterval * 1000);
	}
	Dlg->Log(I_MSG, "�������״̬�̹߳ر�");
	Dlg->m_HttpHeartThread = NULL;
	//Dlg->GetDlgItem(IDC_HTTP_HEART)->SetWindowText(_T("��ʼ��ҳ����"));
	return 0;
}
//========================OnLogoff=====================================
void CGKloginDlg::OnLogoff()
{
	if (m_WeblogoutThread==NULL)
		m_WeblogoutThread = (HANDLE)_beginthreadex(NULL, 0, logout_thread, this, 0, 0);
	else
		Log(I_MSG, "���ڶϿ���֤�����Ժ�...");
}
//========================OnLogoff======================================
//////////////////////////////////////////////////////////////////////////
//��Ӧ�˳���ť������Ѿ���֤�����ȷ����˳���֤��
void CGKloginDlg::OnExit()
{
	ShowWindow(SW_HIDE);
	if (m_bAuth) {
		this->OnLogoff();
	}
	SendMessage(WM_CLOSE, MAKEWPARAM(NULL, 0));
}
void CGKloginDlg::OnIdok()
{
	OnStart();
	// TODO:  �ڴ���������������
}

