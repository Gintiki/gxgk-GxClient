//
// Ping.h
//

#pragma pack(1)

#define ICMP_ECHOREPLY	0
#define ICMP_ECHOREQ	8
#define REQ_DATASIZE 32		// Echo �������ݵĴ�С
#define PING_1_END 1		// Ping������Ϣ��־
#define PING_2_END 2		// Ping������Ϣ��־
#define PING_1_Status 1		// Ping��Ϣ��־
#define PING_2_Status 2		// Ping��Ϣ��־
// IP Header -- RFC 791
typedef struct tagIPHDR
{
	u_char  VIHL;			// Version and IHL
	u_char	TOS;			// Type Of Service
	short	TotLen;			// Total Length
	short	ID;				// Identification
	short	FlagOff;		// Flags and Fragment Offset
	u_char	TTL;			// Time To Live
	u_char	Protocol;		// Protocol
	u_short	Checksum;		// Checksum
	struct	in_addr iaSrc;	// Internet Address - Source
	struct	in_addr iaDst;	// Internet Address - Destination
}IPHDR, *PIPHDR;


// ICMP Header - RFC 792
typedef struct tagICMPHDR
{
	u_char	Type;			// Type
	u_char	Code;			// Code
	u_short	Checksum;		// Checksum
	u_short	ID;				// Identification
	u_short	Seq;			// Sequence
	char	Data;			// Data
}ICMPHDR, *PICMPHDR;


//ICMP�ײ��ṹ��
typedef struct tagICMPHEADER
{
	unsigned char type;//����
	unsigned char code;//����
	unsigned short checknum;//�����
	unsigned short id;//��ʶ��
	unsigned short seq_num;//���к�
	unsigned char data[REQ_DATASIZE];
}ICMPHEADER;
//IP�ײ��ṹ��
typedef struct _ip_hdr
{
	unsigned char ihl : 4;   //�ײ�����
	unsigned char version : 4; //�汾 
	unsigned char tos;   //��������
	unsigned short tot_len; //�ܳ���
	unsigned short id;    //��־
	unsigned short frag_off; //��Ƭƫ��
	unsigned char ttl;   //����ʱ��
	unsigned char protocol; //Э��
	unsigned short chk_sum; //�����
	struct in_addr srcaddr; //ԴIP��ַ
	struct in_addr dstaddr; //Ŀ��IP��ַ
}IPHEADER;

// ICMP Echo Request
typedef struct tagECHOREQUEST
{
	ICMPHEADER icmpHdr;
}ECHOREQUEST, *PECHOREQUEST;

// ICMP Echo Reply
typedef struct tagECHOREPLY
{
	IPHDR	ipHdr;
	ICMPHEADER icmpHdr;
	char    cFiller[256];
}ECHOREPLY, *PECHOREPLY;


class CPing
{
public:
	HWND m_hWnd;	//���ھ��
	void Ping(UINT nRetries, LPCSTR pstrHost, LPCSTR pstrHost2, HWND hWnd, bool *Pingstopsign,char *m_ip);

protected:
	unsigned short checksum(unsigned short *buffer, int size);
	int  WaitForEchoReply(SOCKET s);
	//ICMP��Ӧ������ͻش���
	int		SendEchoRequest(SOCKET, ECHOREQUEST*, LPSOCKADDR_IN, LARGE_INTEGER  *);
	DWORD RecvEchoReply(SOCKET, ECHOREPLY *, LPSOCKADDR_IN, LARGE_INTEGER  *, u_char *);
	void WSAError(LPCSTR pstrFrom);
};

#pragma pack()

