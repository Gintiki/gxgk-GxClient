
// GKloginDlg.cpp : ʵ���ļ�
//

#include "stdafx.h"
#include "GKlogin.h"
#include "GKloginDlg.h"
#include "afxdialogex.h"
#include "SettingDlg.h"
#include "afxwin.h"
#ifdef _DEBUG
#define new DEBUG_NEW
#endif

int dpiX;
int dpiY;

CComplaint::CComplaint(CWnd* pParent /*=NULL*/)
	: CDialogEx(CComplaint::IDD, pParent)
{
	m_hIcon = AfxGetApp()->LoadIcon(IDR_MAINFRAME);
}


void CComplaint::DoDataExchange(CDataExchange* pDX)
{
	CDialog::DoDataExchange(pDX);
}

BEGIN_MESSAGE_MAP(CComplaint, CDialog)
	ON_STN_CLICKED(IDC_GXerweima, &CComplaint::OnStnClickedGxerweima)
	ON_WM_CTLCOLOR()
END_MESSAGE_MAP()



CAboutDlg::CAboutDlg() : CDialogEx(CAboutDlg::IDD)
{

}

void CAboutDlg::DoDataExchange(CDataExchange* pDX)
{
	CDialogEx::DoDataExchange(pDX);
	DDX_Control(pDX, IDC_OEM_EDIT, m_AboutUs);
}

BEGIN_MESSAGE_MAP(CAboutDlg, CDialogEx)
	ON_BN_CLICKED(IDC_CHK_Restore, &CAboutDlg::OnBnClickedChkRestore)
END_MESSAGE_MAP()


// CGKloginDlg �Ի���



CGKloginDlg::CGKloginDlg(CWnd* pParent /*=NULL*/)
	: CDialogEx(CGKloginDlg::IDD, pParent)
{
	m_hIcon = AfxGetApp()->LoadIcon(IDR_MAINFRAME);
	//��֤״̬Ϊ δ��֤
	m_bAuth = FALSE;
}

void CGKloginDlg::DoDataExchange(CDataExchange* pDX)
{
	CDialogEx::DoDataExchange(pDX);
	DDX_Control(pDX, IDC_USERNAME, _T(m_ccb_username));
	DDX_Control(pDX, IDC_NETCARD, _T(m_ccbNetCard));
	DDX_Control(pDX, IDC_Banner, Banner_Var);
}

BEGIN_MESSAGE_MAP(CGKloginDlg, CDialogEx)
	ON_WM_SYSCOMMAND()
	ON_WM_PAINT()
	ON_WM_QUERYDRAGICON()
	ON_BN_CLICKED(MENU_SHOW, OnTrayShow)
	ON_MESSAGE(WM_USER_TRAY_NOTIFICATION, OnTrayNotification)
	ON_WM_TIMER()
	//��ť
	ON_BN_CLICKED(IDC_START, OnStart)
	ON_BN_CLICKED(IDC_LOGOFF, OnLogoff)
	ON_BN_CLICKED(IDC_EXIT, OnExit)
	ON_BN_CLICKED(MENU_EXIT, OnExit)
	ON_BN_CLICKED(IDC_SETTING, OnSetting)
	ON_BN_CLICKED(IDC_LOGSHOW, OnLogshow)
	ON_BN_CLICKED(IDC_Feedback, OnBnClickedFeedback)
	ON_STN_CLICKED(IDC_Banner, OnStnClickedBanner)

	ON_CBN_SELCHANGE(IDC_USERNAME, OnSelchangeUsername)
	ON_COMMAND(IDOK, &CGKloginDlg::OnIdok)
END_MESSAGE_MAP()


// CGKloginDlg ��Ϣ�������
BOOL CGKloginDlg::OnInitDialog()
{
	CDialogEx::OnInitDialog();

	// ��������...���˵�����ӵ�ϵͳ�˵��С�

	// IDM_ABOUTBOX ������ϵͳ���Χ�ڡ�
	ASSERT((IDM_ABOUTBOX & 0xFFF0) == IDM_ABOUTBOX);
	ASSERT(IDM_ABOUTBOX < 0xF000);

	CMenu* pSysMenu = GetSystemMenu(FALSE);
	if (pSysMenu != NULL)
	{
		BOOL bNameValid;
		CString strAboutMenu;
		bNameValid = strAboutMenu.LoadString(IDS_ABOUTBOX);
		ASSERT(bNameValid);
		if (!strAboutMenu.IsEmpty())
		{
			pSysMenu->AppendMenu(MF_SEPARATOR);
			pSysMenu->AppendMenu(MF_STRING, IDM_ABOUTBOX, strAboutMenu);
		}
	}

	// ���ô˶Ի����ͼ�ꡣ  ��Ӧ�ó��������ڲ��ǶԻ���ʱ����ܽ��Զ�
	//  ִ�д˲���
	SetIcon(m_hIcon, TRUE);			// ���ô�ͼ��
	m_hIcon = AfxGetApp()->LoadIcon(IDI_SmallICON);
	SetIcon(m_hIcon, FALSE);		// ����Сͼ��
	//��������ͼ��
	m_tray.Create(this, WM_USER_TRAY_NOTIFICATION, STR_AppName, m_hIcon, 0);

	CDC* dc = GetDC();
	int dpiX = GetDeviceCaps(dc->GetSafeHdc(), LOGPIXELSX);
	int dpiY = GetDeviceCaps(dc->GetSafeHdc(), LOGPIXELSY);
	SetDPI(dpiX, dpiY);
	ReleaseDC(dc);
	// TODO:  �ڴ���Ӷ���ĳ�ʼ������
	//��ȡ���õ��ļ����ö�����ȥ
	
	char szTempPath[MAX_PATH], szTempName[MAX_PATH];
	GetTempPath(MAX_PATH, szTempPath);

	sprintf(szTempName, "%sBanner.png", szTempPath);
	if ((_access(szTempName, 0)) != -1)
	{
		HWND hwnd = GetSafeHwnd(); //��ȡ���ڵ�HWND
		::InvalidateRect(hwnd, NULL, true); //���� ::InvalidateRect( hwnd, NULL, false );
		::UpdateWindow(hwnd);
		//��ʹ��ǰ�����ԭ�����Ƶ�ͼƬȥ��������ɾȥ����������
		CDC *pDC = GetDC();
		CImage Image;
		Image.Load(szTempName);
		if (Image.IsNull())
		{
			//Log(I_MSG, "Banner����ʧ�ܣ�");
			DeleteFile(szTempName);
		}
		if (BannerPicture != NULL)
		DeleteObject(BannerPicture);
		CStatic* pWnd = (CStatic*)GetDlgItem(IDC_Banner);
		BannerPicture = Image.Detach();
		pWnd->SetBitmap(BannerPicture);
		int width = 377, height = 80;
		pWnd->SetWindowPos(NULL, 0, 0, (size_t)(width / DEFAULT_DPI*dpiX), (size_t)(height / DEFAULT_DPI*dpiY), SWP_NOACTIVATE | SWP_NOMOVE | SWP_NOZORDER);
		//pWnd->SetWindowPos(NULL,0,0,377,80,SWP_NOACTIVATE | SWP_NOMOVE | SWP_NOZORDER);
		//Image.Draw(pDC->m_hDC, 230, 110);
		Image.Destroy();
		ReleaseDC(pDC);
	}
	else{
		CStatic* pWnd = (CStatic*)GetDlgItem(IDC_Banner); // �õ� Picture Control ���  
		CImage image;
		LoadImageFromResource(&image, IDB_Banner, _T("PNG"));
		BannerPicture = image.Detach();
		pWnd->SetBitmap(BannerPicture);
		int width = 377, height = 80;
		pWnd->SetWindowPos(NULL, 0, 0, (size_t)(width / DEFAULT_DPI*dpiX), (size_t)(height / DEFAULT_DPI*dpiY), SWP_NOACTIVATE | SWP_NOMOVE | SWP_NOZORDER);
		//pWnd->SetWindowPos(NULL,0,0,377,80,SWP_NOACTIVATE | SWP_NOMOVE | SWP_NOZORDER);
	}
	
	initialize();
	return TRUE;  // ���ǽ��������õ��ؼ������򷵻� TRUE
}

void CGKloginDlg::OnSysCommand(UINT nID, LPARAM lParam)
{
	if ((nID & 0xFFF0) == IDM_ABOUTBOX)
	{
		CAboutDlg dlgAbout;
		dlgAbout.DoModal();
	}
	else if ((nID & 0xFFF0) == SC_MINIMIZE || (nID & 0xFFF0) == SC_CLOSE)
	{
		ShowWindow(SW_HIDE);
		//SetProcessWorkingSetSize(GetCurrentProcess(), -1, -1);
		SetBubble("��ʾ", STR_AppName" ���ں�̨�����š���", 1);
	}
	else
	{
		CDialog::OnSysCommand(nID, lParam);
	}
}

// �����Ի��������С����ť������Ҫ����Ĵ���
//  �����Ƹ�ͼ�ꡣ  ����ʹ���ĵ�/��ͼģ�͵� MFC Ӧ�ó���
//  �⽫�ɿ���Զ���ɡ�

void CGKloginDlg::OnPaint()
{
	if (IsIconic())
	{
		CPaintDC dc(this); // ���ڻ��Ƶ��豸������

		SendMessage(WM_ICONERASEBKGND, reinterpret_cast<WPARAM>(dc.GetSafeHdc()), 0);

		// ʹͼ���ڹ����������о���
		int cxIcon = GetSystemMetrics(SM_CXICON);
		int cyIcon = GetSystemMetrics(SM_CYICON);
		CRect rect;
		GetClientRect(&rect);
		int x = (rect.Width() - cxIcon + 1) / 2;
		int y = (rect.Height() - cyIcon + 1) / 2;

		// ����ͼ��
		dc.DrawIcon(x, y, m_hIcon);
	}
	else
	{
		CDialogEx::OnPaint();
	}
}

//���û��϶���С������ʱϵͳ���ô˺���ȡ�ù��
//��ʾ��
HCURSOR CGKloginDlg::OnQueryDragIcon()
{
	return static_cast<HCURSOR>(m_hIcon);
}

void CGKloginDlg::OnTrayShow()
{
	if (IsWindowVisible())
	{
		ShowWindow(SW_HIDE);
		//SetProcessWorkingSetSize(GetCurrentProcess(), -1, -1);
	}
	else
	{
		SetWindowPos(&wndTopMost, 0, 0, 0, 0, SWP_NOMOVE | SWP_NOSIZE);
		ShowWindow(SW_SHOWNA);
		BringWindowToTop();
	}
}

LONG CGKloginDlg::OnTrayNotification(WPARAM wparam, LPARAM lparam)
{
	CMenu	m_menu;
	CPoint	m_point;
	switch (lparam)
	{
	case WM_RBUTTONDOWN:
		m_menu.CreatePopupMenu();
		if (IsWindowVisible())
		{
			m_menu.AppendMenu(MF_ENABLED, MENU_SHOW, _T("���ش���"));
		}
		else
		{
			m_menu.AppendMenu(MF_ENABLED, MENU_SHOW, _T("��ʾ����"));
		}
		m_menu.AppendMenu(MF_ENABLED, MF_SEPARATOR);
		if (!m_bAuth)
			m_menu.AppendMenu(MF_ENABLED, IDC_START, _T("��½��֤"));
		else
			m_menu.AppendMenu(MF_ENABLED, IDC_LOGOFF, _T("�˳���֤"));
		m_menu.AppendMenu(MF_ENABLED, MF_SEPARATOR);
		m_menu.AppendMenu(MF_ENABLED, MENU_EXIT, _T("�� ��"));
		GetCursorPos(&m_point);
		m_menu.TrackPopupMenu(TPM_LEFTALIGN, m_point.x, m_point.y, this);
		break;
	case WM_LBUTTONDOWN:
		if (IsWindowVisible())
		{
			ShowWindow(SW_HIDE);
			//SetProcessWorkingSetSize(GetCurrentProcess(), -1, -1);
		}
		else
		{
			SetWindowPos(&wndTopMost, 0, 0, 0, 0, SWP_NOMOVE | SWP_NOSIZE);
			ShowWindow(SW_SHOWNA);
			BringWindowToTop();
		}
		break;
	}
	return 0;
}
void CGKloginDlg::OnBnClickedFeedback()
{
	
	// TODO:  �ڴ���ӿؼ�֪ͨ����������
	CComplaint dlg;
	dlg.parent = this;
	dlg.DoModal();
}
void CGKloginDlg::OnSetting()
{
	CSettingDlg dlg;
	dlg.parent = this;
	//dlg.SetParent(this);	
	dlg.DoModal();
}
BOOL CAboutDlg::OnInitDialog()
{
	CDialogEx::OnInitDialog();
	HICON m_hIcon;
	m_hIcon = AfxGetApp()->LoadIcon(IDI_SmallICON);
	SetIcon(m_hIcon, FALSE);		// ����Сͼ��

	GetDlgItem(IDC_NameVersion)->SetWindowText(_T(STR_AppName "  �汾�ţ�" STR_Version));
	CEdit *editshow;
	editshow = (CEdit*)GetDlgItem(IDC_OEM_EDIT);
	int nLength = editshow->SendMessage(WM_GETTEXTLENGTH);
	editshow->SetSel(nLength, nLength);
	editshow->ReplaceSel("      ��лyzx��ʦ������ݸ��ͻ��˵�Դ���룬��ԭ�������԰����ָ�Ϊݸ��ͻ��ˣ�����汾��Ҫ�Ƕ�֮ǰУ԰���ͻ��˳�����Ż��������������ı�������Ϊ�������µĸ����޷���֤���⣡");
	editshow->ReplaceSel("\r\n���\r\n      ÿ�����ڳɳ������ж����ⷸһЩ��������Ҳһ��������Ĺ���Ҳ�����ǲ�ϣ�������ġ��������ǲ���������ÿ���û����⣬�������ǽ��ܸ��ֽ������������������Ȥ���ۼ���һ��Ϊ�˸��Ե������Ŭ����ϣ�����ǵĹ����ܵõ���ҵ�֧�֣�����Ǹ����������Ļر���");
	CEdit *editshow2;
	editshow2 = (CEdit*)GetDlgItem(IDC_OEM_EDIT2);
	int nLength2 = editshow2->SendMessage(WM_GETTEXTLENGTH);
	editshow2->SetSel(nLength2, nLength2);
	editshow2->ReplaceSel("ݸ������ѧ���Խ��Ļ����������Ŷӡ��Ŷӳ�����2012��5�£���У԰��̳�𲽣��𽥷�չ��У԰����Դ����վ��ý�����ġ�΢�����ֵ���Ŀ�����������ķ�չ��ݸ�����Ѿ���ΪУ��֪����ý����֯��ѧ��ƽ̨�����Ŷӳ�Ա���ǹ�Ƶ���У������ҵ���Ŷӳ�Ա�����в�Ʒ��Ӫ��ý��༭������Ա������ߡ���Ӱʦ�ȡ�");

	return TRUE;  // ���ǽ��������õ��ؼ������򷵻� TRUE
}

void CAboutDlg::OnBnClickedChkRestore()
{
	int result = MessageBox(_T("�����������������˺�������Ϣ����ȷ���Ƿ�ָ�Ĭ�����ã�"), _T("����"), MB_ICONQUESTION | MB_YESNO);
	switch (result)//ע�⣡ʹ��UnicodeӦ��TEXT��Χ�ִ�
	{
	case IDYES:
	{
		char pszFullPath[MAX_STRING];
		char pszFilename[MAX_STRING] = CONFIGNAME;

		Config.GetFullPathToFile(pszFullPath, pszFilename);

		DeleteFile(pszFullPath);

		PostMessage(WM_QUIT);
		Sleep(800);
		//��ȡexe����ǰ·��  
		TCHAR szAppName[MAX_PATH];
		::GetModuleFileName(theApp.m_hInstance, szAppName, MAX_PATH);
		CString strAppFullName;
		strAppFullName.Format(_T("%s"), szAppName);
		//��������  
		STARTUPINFO StartInfo;
		PROCESS_INFORMATION procStruct;
		memset(&StartInfo, 0, sizeof(STARTUPINFO));
		StartInfo.cb = sizeof(STARTUPINFO);
		::CreateProcess(
			(LPCTSTR)strAppFullName,
			NULL,
			NULL,
			NULL,
			FALSE,
			NORMAL_PRIORITY_CLASS,
			NULL,
			NULL,
			&StartInfo,
			&procStruct);
	}
	case IDNO:
		return;
	}

}
BOOL LoadImageFromResource(CImage *pImage, UINT nResID, LPCTSTR lpTyp)
{
	if (pImage == NULL)
		return false;
	pImage->Destroy();
	// ������Դ
	HRSRC hRsrc = ::FindResource(AfxGetResourceHandle(), MAKEINTRESOURCE(nResID), lpTyp);
	if (hRsrc == NULL)
		return false;
	// ������Դ
	HGLOBAL hImgData = ::LoadResource(AfxGetResourceHandle(), hRsrc);
	if (hImgData == NULL)
	{
		::FreeResource(hImgData);
		return false;
	}
	// �����ڴ��е�ָ����Դ
	LPVOID lpVoid = ::LockResource(hImgData);
	LPSTREAM pStream = NULL;
	DWORD dwSize = ::SizeofResource(AfxGetResourceHandle(), hRsrc);
	HGLOBAL hNew = ::GlobalAlloc(GHND, dwSize);
	LPBYTE lpByte = (LPBYTE)::GlobalLock(hNew);
	::memcpy(lpByte, lpVoid, dwSize);
	// ����ڴ��е�ָ����Դ
	::GlobalUnlock(hNew);
	// ��ָ���ڴ洴��������
	HRESULT ht = ::CreateStreamOnHGlobal(hNew, TRUE, &pStream);
	if (ht != S_OK)
	{
		GlobalFree(hNew);
	}
	else
	{
		// ����ͼƬ
		pImage->Load(pStream);
		GlobalFree(hNew);
	}
	// �ͷ���Դ
	::FreeResource(hImgData);
	return true;
}

BOOL CComplaint::OnInitDialog()
{
	m_hIcon = AfxGetApp()->LoadIcon(IDI_SmallICON);
	this->SetIcon(m_hIcon, FALSE);		// ����Сͼ��
	
	CStatic* pWnd = (CStatic*)GetDlgItem(IDC_GXerweima); // �õ� Picture Control ���  
	CImage image;
	LoadImageFromResource(&image, IDB_erweima, _T("PNG"));
	HBITMAP hBmp = image.Detach();
	pWnd->SetBitmap(hBmp);
	int width = 240, height = 240;
	pWnd->SetWindowPos(NULL, 0, 0, (size_t)(width / DEFAULT_DPI*dpiX), (size_t)(height / DEFAULT_DPI*dpiY), SWP_NOACTIVATE | SWP_NOMOVE | SWP_NOZORDER);
	fontTitle.CreatePointFont(150, "΢���ź�");
	ShowMsg(FALSE);
	return TRUE;
}



void CComplaint::OnStnClickedGxerweima()
{
	// TODO:  �ڴ���ӿؼ�֪ͨ����������
	CHyperLink Link;
	Link.GotoURL(URL_Help, SW_SHOW);
	//::ShellExecute(this->GetSafeHwnd(), _T("open"), URL_Help, NULL, NULL, SW_SHOWNORMAL);

}
HBRUSH CComplaint::OnCtlColor(CDC* pDC, CWnd* pWnd, UINT nCtlColor)
{
	HBRUSH hbr = CDialog::OnCtlColor(pDC, pWnd, nCtlColor);

	// TODO:  �ڴ˸��� DC ���κ�����   

	switch (pWnd->GetDlgCtrlID())
	{
	case IDC_STATICBig1:
		//pDC->SetTextColor(RGB(255, 0, 0));
		pDC->SelectObject(&fontTitle);
		break;
	case IDC_STATICBig2:
		//pDC->SetTextColor(RGB(255, 0, 0));
		pDC->SelectObject(&fontTitle);
		break;
	case IDC_STATICBig3:
		//pDC->SetTextColor(RGB(255, 0, 0));
		pDC->SelectObject(&fontTitle);
		break;
		// TODO:  ���Ĭ�ϵĲ������軭�ʣ��򷵻���һ������   	
	}
	return hbr;
}



void moveWindow(CWnd &wnd)
{
	CRect rect;
	wnd.GetWindowRect(rect);
	wnd.GetParent()->ScreenToClient(rect);
	rect.left = (int)(rect.left*DEFAULT_DPI / dpiX);
	rect.right = (int)(rect.right*DEFAULT_DPI / dpiX);
	rect.top = (int)(rect.top*DEFAULT_DPI / dpiY);
	rect.bottom = (int)(rect.bottom*DEFAULT_DPI / dpiY);
	rect.OffsetRect(CPoint((int)((dpiX - DEFAULT_DPI) / 2), (int)((dpiY - DEFAULT_DPI) / 2)));

	wnd.MoveWindow(rect);
}

void SetDPI(int x, int y)
{
	dpiX = x;
	dpiY = y;
}

double GetDPIX(double X)
{
	X = X / DEFAULT_DPI * dpiX;
	return X;
}

double GetDPIY(double Y)
{
	Y = Y / DEFAULT_DPI * dpiY;
	return Y;
}

