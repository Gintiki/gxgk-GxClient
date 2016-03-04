#include "stdafx.h"
#include "GKlogin.h"
#include "GKloginDlg.h"
#include "AutoUpdate.h"
#include "BASE64.h"
#include <winioctl.h>
#include <md5.h>
#include <rc4.h>
#include <HyperLink.h>

#define MAX_BUFFER_SIZE 10204

int CGKloginDlg::Notice(int id)
{
	char http[MAX_BUFFER_SIZE] = { 0 };
	char *msg = (char *)malloc(MAX_BUFFER_SIZE);
	char url[MAX_BUFFER_SIZE] = { 0 };
	char title[MAX_BUFFER_SIZE] = { 0 };
	char content[MAX_BUFFER_SIZE] = { 0 };
	int ret = 0;
	for (int i = 0; i <= 2 && ret <= 0; i++){
		sprintf(url, LAN_URL  NoticeUp "?id=%d", id);
		ret = DownLoadFileToBuffer(http, MAX_BUFFER_SIZE, url, NULL, DOWNTIMEOUT, NULL, NULL, NULL, NULL);
		if (ret <= 0){
			sprintf(url, WEB_URL  NoticeUp "?id=%d", id);
			ret = DownLoadFileToBuffer(http, MAX_BUFFER_SIZE, url, NULL, DOWNTIMEOUT, NULL, NULL, NULL, NULL);
			if (ret <= 0)
			{
				Log(I_MSG, "�޷���ʾ���棬���ӷ�����ʧ�ܣ�");
				continue;
			}
		}
		if (http)UTF8ToGBK((unsigned char*)http, (unsigned char*)http, MAX_BUFFER_SIZE);
		GetHTTPMsg(http, MAX_BUFFER_SIZE, 2, "Title:\"", "Content:\"", NULL, title, content, NULL, NULL, "\"");

		if (strlen(content)){
			SetBubble(title, content,3000);
			sprintf(msg, "%s:%s", title, content);

			Log(I_MSG, "%s", msg);
			break;
		}
	}
	Banner_Show();
	Config.SaveConfig();
	free(msg);
	return 0;
}
void CGKloginDlg::Banner_Show()
{
	char banner[MAX_BUFFER_SIZE]; int ret;
	char banner_url[MAX_BUFFER_SIZE];
	char http[MAX_BUFFER_SIZE] = { 0 };
	ret = DownLoadFileToBuffer(http, MAX_BUFFER_SIZE, LAN_URL  NoticeUrl, NULL, DOWNTIMEOUT, NULL, NULL, NULL, NULL);
	if (ret <= 0)
	{
		ret = DownLoadFileToBuffer(http, MAX_BUFFER_SIZE, WEB_URL  NoticeUrl, NULL, DOWNTIMEOUT, NULL, NULL, NULL, NULL);
		if (ret <= 0){
			Log(I_MSG, "�޷���ȡBannerUrl�����ӷ�����ʧ�ܣ�");
			return;
		}
	}
	GetHTTPMsg(http, MAX_BUFFER_SIZE, 2, "Banner_PNG:\"", "Banner_Url:\"", NULL, banner, banner_url, NULL, NULL, "\"");
	Config.banner_url = banner_url;

	char szTempPath[MAX_PATH], szTempName[MAX_PATH];
	GetTempPath(MAX_PATH, szTempPath);

	sprintf(szTempName, "%sBanner.png",szTempPath);

	ret = DownLoadFile(szTempName, banner, NULL, DOWNTIMEOUT, NULL, NULL);
	if (ret != 0)
	{
		Log(I_MSG, "�޷���ȡBanner�����ӷ�����ʧ�ܣ�");
		return;
	}
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
			Log(I_MSG, "Banner����ʧ�ܣ�");
		}
		else
		{
			if (BannerPicture != NULL)
				DeleteObject(BannerPicture);
		}
		CStatic* pWnd = (CStatic*)GetDlgItem(IDC_Banner);
		BannerPicture = Image.Detach();
		pWnd->SetBitmap(BannerPicture);
		pWnd->SetWindowPos(NULL,
			0,
			0,
			(int)GetDPIX(377),
			(int)GetDPIY(80),
			SWP_NOACTIVATE | SWP_NOMOVE | SWP_NOZORDER);
		//Image.Draw(pDC->m_hDC, 230, 110);
		Image.Destroy();
		ReleaseDC(pDC);
	}
}

void CGKloginDlg::OnStnClickedBanner()
{
	CHyperLink Link;
	//::ShellExecute(this->GetSafeHwnd(), _T("open"), banner_url, NULL, NULL, SW_SHOWNORMAL);
	Link.GotoURL(Config.banner_url, SW_SHOW);
}
void CGKloginDlg::FaultMsg(const char *msg)
{
	if (strlen(msg) != NULL)
	{
		Log(I_MSG, "");
		if (strstr(msg, "http") == 0)
		{
			if (strstr(msg, "������") != 0)
			{
				Log(I_MSG, "	������֤ʧ�ܴ�����ʾ��");
				if (strstr(msg, "������110:�û�����������") != 0)
				{
					Log(I_MSG, "�û����������ƣ�����ԭ��");
					Log(I_MSG, "   1.�˺��ڱ�ĵ����ϵ�½");
					Log(I_MSG, "   2.�˺ű�����");
				}
				else if (strstr(msg, "������207:�˺�û���ڸ�ʱ�ν����Ȩ��") != 0)
				{
					Log(I_MSG, "�˺�û���ڸ�ʱ�ν����Ȩ�ޣ�����ԭ��");
					Log(I_MSG, "   1.�����˺ţ����ڶ�����");
				}
				else if (strstr(msg, "������101:�����ڵ��û�") != 0)
				{
					Log(I_MSG, "�˻�����������������������˻���");
				}
				else if (strstr(msg, "������102:�������") != 0)
				{
					Log(I_MSG, "����������������������������룡");
				}
				else { Log(I_MSG, "%s������ԭ�������ռ����뵽��̳�����������ƿͻ���", msg); }
			}
		}
	}
	else { Log(I_MSG, "�������򽻻������⣬����ѯ����Ӫҵ����"); }
	Log(I_MSG, "�޷����������ѯ����Ӫҵ������̳���������ƿͻ���");
	Log(I_MSG, "��ȴ�1-2���Ӻ��ٴ����ӣ�");
	SetBubble("������֤ʧ��", "�޷����������ѯ����Ӫҵ������̳���������ƿͻ��ˣ�", 15);
	if (logshowstatus)SendMessage(WM_COMMAND, MAKEWPARAM(IDC_LOGSHOW, 0));
}

void CGKloginDlg::WebFaultMsg(const char *msg)
{
	Log(I_MSG, "");
	Log(I_MSG, "	��ҳ��֤������ʾ��");
	if (strstr(msg, "�Ҳ���SDX��Ϣ") != 0)
	{
		Log(I_MSG, "�Ҳ���SDX��Ϣ������ԭ��");
		Log(I_MSG, "   1.��������������֤��");
		Log(I_MSG, "   2.���ڲ���ʹ��������ҳ��֤�Ļ���");
		Log(I_MSG, "   3.���ӵ���Wifi�޷���֤");
	}
	else if (strstr(msg, "û�ж����˲�Ʒ") != 0)
	{
		Log(I_MSG, "û�ж����˲�Ʒ������ԭ��");
		Log(I_MSG, "   1.�¿��˺ſ�ͨ�У���ȴ���ͨ��");
		Log(I_MSG, "   2.������ֻ����˺ţ������ֻ��Ƿ�Ƿ��ͣ��");
		Log(I_MSG, "   3.�˺ű�ͣ��");
		Log(I_MSG, "   4.���ŷ����������쳣");
	}
	else if (strstr(msg, "��ҳ��֤���ӳ�ʱ") != 0)
	{
		Log(I_MSG, "��ҳ��֤���ӳ�ʱ������ԭ��");
		Log(I_MSG, "   1.��������磬���������Ƿ�ͬ�����");
		Log(I_MSG, "   2.������������Ƿ�����");
		Log(I_MSG, "   3.�ϴ���ҳ��֤δ�����˳�");
		Log(I_MSG, "   4.δ����������֤");
		Log(I_MSG, "   5.�������DHCP�Ƿ��Ѿ���");
		Log(I_MSG, "   6.��鱾������DNS�����������Ƿ�Ϊ�Զ���ȡ");
	}
	else if (strstr(msg, "�û���֤ʧ��") != 0)
	{
		Log(I_MSG, "�û���֤ʧ�ܣ�����ԭ��");
		Log(I_MSG, "   1.�û�û�������˺Ŷ��ٴε�¼����ȴ�������");
	}
	else { Log(I_MSG, "%s������ԭ�������ռ����뵽��̳�����������ƿͻ���", msg); }
	Log(I_MSG, "�޷����������ѯ����Ӫҵ������̳���������ƿͻ���\r\n");
	SetBubble("��ҳ��֤ʧ��", "�޷����������ѯ����Ӫҵ������̳���������ƿͻ���", 15);
	if (logshowstatus)SendMessage(WM_COMMAND, MAKEWPARAM(IDC_LOGSHOW, 0));
}

//�˳��ַ�����ʼλ�õĿո�
void TrimStart(LPTSTR pBuf)
{
	if (*pBuf != 0x20)
		return;

	LPTSTR pDest = pBuf;
	LPTSTR pSrc = pBuf + 1;
	while (*pSrc == 0x20)
		++pSrc;

	while (*pSrc)
	{
		*pDest = *pSrc;
		++pDest;
		++pSrc;
	}
	*pDest = 0;
}

//��WORD��������ֽ���Ϊlittle-endian�����˳��ַ�����β�Ŀո�
void ToLittleEndian(PUSHORT pWords, int nFirstIndex, int nLastIndex, LPTSTR pBuf)
{
	int index;
	LPTSTR pDest = pBuf;
	for (index = nFirstIndex; index <= nLastIndex; ++index)
	{
		pDest[0] = pWords[index] >> 8;
		pDest[1] = pWords[index] & 0xFF;
		pDest += 2;
	}
	*pDest = 0;

	//trim space at the endof string; 0x20: _T(' ')
	--pDest;
	while (*pDest == 0x20)
	{
		*pDest = 0;
		--pDest;
	}
}
//
// Model Number: 40 ASCII Chars
// SerialNumber: 20 ASCII Chars
//
BOOL GetPhyDriveSerial(LPTSTR pModelNo, LPTSTR pSerialNo)
{
	//-1����Ϊ SENDCMDOUTPARAMS �Ľ�β�� BYTE bBuffer[1];
	BYTE IdentifyResult[sizeof(SENDCMDOUTPARAMS) + IDENTIFY_BUFFER_SIZE - 1];
	DWORD dwBytesReturned;
	GETVERSIONINPARAMS get_version;
	SENDCMDINPARAMS send_cmd = { 0 };

	HANDLE hFile = CreateFile(_T("\\\\.\\PHYSICALDRIVE0"), GENERIC_READ | GENERIC_WRITE,
		FILE_SHARE_READ | FILE_SHARE_WRITE, NULL, OPEN_EXISTING, 0, NULL);
	if (hFile == INVALID_HANDLE_VALUE)
		return FALSE;

	//get version
	DeviceIoControl(hFile, SMART_GET_VERSION, NULL, 0,
		&get_version, sizeof(get_version), &dwBytesReturned, NULL);

	//identify device
	send_cmd.irDriveRegs.bCommandReg = (get_version.bIDEDeviceMap & 0x10) ? ATAPI_ID_CMD : ID_CMD;
	DeviceIoControl(hFile, SMART_RCV_DRIVE_DATA, &send_cmd, sizeof(SENDCMDINPARAMS) - 1,
		IdentifyResult, sizeof(IdentifyResult), &dwBytesReturned, NULL);
	CloseHandle(hFile);

	//adjust the byte order
	PUSHORT pWords = (USHORT*)(((SENDCMDOUTPARAMS*)IdentifyResult)->bBuffer);
	ToLittleEndian(pWords, 27, 46, pModelNo);
	ToLittleEndian(pWords, 10, 19, pSerialNo);
	return TRUE;
}

int CGKloginDlg::PCsign(char *DiskSign) {
	TCHAR szModelNo[48], szSerialNo[24];
	if (GetPhyDriveSerial(szModelNo, szSerialNo))
	{
		//_tprintf(_T("Model No: %s\n"), szModelNo);
		TrimStart(szSerialNo);
		//_tprintf(_T("Serial No: %s\n"), szSerialNo);
	}

	MD5_CTX mdContext;
	int bytes;
	unsigned char data[1024];
	char md5[33];
	int i;

	int md5_len = 16;
	strcpy((char *)data, szSerialNo);
	bytes = strlen((char *)data);

	static u_char digest[16];

	MD5Init(&mdContext);
	if (strcmp((char *)data, "") != 0)
	{
		MD5Update(&mdContext, data, bytes);
	}
	MD5Final(digest, &mdContext);


	memset(md5, 0, (md5_len + 1));

	if (md5_len == 16)
	{
		for (i = 4; i<12; i++)
		{
			sprintf(&md5[(i - 4) * 2], "%02X", digest[i]);
		}
	}
	if (strcmp((char *)md5, "") == 0)
	{
		//fprintf(stderr, "failed.\n");
		return NULL;
	}
	//_tprintf(_T("SN Hash: %s\n"), md5);
	strcpy(DiskSign, md5);
	return 1;
}