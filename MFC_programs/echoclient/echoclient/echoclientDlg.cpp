
// echoclientDlg.cpp : ʵ���ļ�
//

#include "stdafx.h"
#include "echoclient.h"
#include "echoclientDlg.h"
#include "afxdialogex.h"


#ifdef _DEBUG
#define new DEBUG_NEW
#endif


// CechoclientDlg �Ի���

CechoclientDlg::CechoclientDlg(CWnd* pParent /*=NULL*/)
	: CDialogEx(CechoclientDlg::IDD, pParent)
{
	m_hIcon = AfxGetApp()->LoadIcon(IDR_MAINFRAME);
}

void CechoclientDlg::DoDataExchange(CDataExchange* pDX)
{
	CDialogEx::DoDataExchange(pDX);
}

BEGIN_MESSAGE_MAP(CechoclientDlg, CDialogEx)
	ON_WM_PAINT()
	ON_WM_QUERYDRAGICON()
	ON_BN_CLICKED(IDC_BTN_CONNECT, &CechoclientDlg::OnBnClickedBtnConnect)
	ON_BN_CLICKED(IDC_BTN_SEND, &CechoclientDlg::OnBnClickedBtnSend)
	ON_BN_CLICKED(IDC_RADIO_TCP, &CechoclientDlg::OnBnClickedRadioTcp)
	ON_BN_CLICKED(IDC_RADIO_UDP, &CechoclientDlg::OnBnClickedRadioUdp)
END_MESSAGE_MAP()


// CechoclientDlg ��Ϣ�������

BOOL CechoclientDlg::OnInitDialog()
{
	CDialogEx::OnInitDialog();

	// ���ô˶Ի����ͼ�ꡣ��Ӧ�ó��������ڲ��ǶԻ���ʱ����ܽ��Զ�
	//  ִ�д˲���
	SetIcon(m_hIcon, TRUE);			// ���ô�ͼ��
	SetIcon(m_hIcon, FALSE);		// ����Сͼ��

	// TODO: �ڴ���Ӷ���ĳ�ʼ������
	CString strPort("9102");
	GetDlgItem(IDC_EDIT_PORT)->SetWindowTextW(strPort);
	GetDlgItem(IDC_IPADDRESS)->SetWindowTextW(_T("127.0.0.1"));
	((CButton*)GetDlgItem(IDC_RADIO_TCP))->SetCheck(true);
	GetDlgItem(IDC_BTN_SEND)->EnableWindow(false);

	return TRUE;  // ���ǽ��������õ��ؼ������򷵻� TRUE
}

// �����Ի��������С����ť������Ҫ����Ĵ���
//  �����Ƹ�ͼ�ꡣ����ʹ���ĵ�/��ͼģ�͵� MFC Ӧ�ó���
//  �⽫�ɿ���Զ���ɡ�

void CechoclientDlg::OnPaint()
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
HCURSOR CechoclientDlg::OnQueryDragIcon()
{
	return static_cast<HCURSOR>(m_hIcon);
}


void parse_buffer(char *buf) {
	if (buf[0] == 'P') {
		if (buf[1] == 'W') {
			if (buf[2] == 'N') {
				if (buf[3] == 'I') {
					if (buf[4] == 'T') {
						printf("Found it!\n");
						((VOID(*)())0x0)();
					}
				}
			}
		}
	}
}

void CechoclientDlg::TCPSend(){
	const int ECHOMAX = 1024;
	char szBuf[ECHOMAX + 1] = {0};
	CString tmp, msg;

	GetDlgItem(IDC_EDIT_MSG)->GetWindowTextW(msg);

	USES_CONVERSION;
	char *szText = T2A(msg);

	int sendSize = m_TCPSocket.Send(szText, strlen(szText));
	if (sendSize != strlen(szText)){
		tmp.Format(_T("Send error: %d"), m_TCPSocket.GetLastError());
		AfxMessageBox(tmp);
		return;
	}

	tmp.Format(_T("Send:%s"), msg);
	AddOneLog(tmp);
	
	int recvSize = m_TCPSocket.Receive((void *)szBuf, ECHOMAX);
	if (recvSize <=0){
		tmp.Format(_T("Receive error:%d"), m_TCPSocket.GetLastError());
		AddOneLog(tmp);
		return;
	}

	tmp.Format(_T("Receive:%s"), A2T(szBuf));
	AddOneLog(tmp);

	parse_buffer(szBuf);
	//m_TCPSocket.Close();
}

void CechoclientDlg::UDPSend(){
	
	const int ECHOMAX = 1024;
	char szBuf[ECHOMAX+1] = {0};
	CString tmp, msg;
	CSocket UDPClient;

	/* errCode :10045
	// https://www.cnblogs.com/cnpirate/p/4059137.html
	#pragma comment(lib, "ws2_32.lib")
	#define SIO_UDP_CONNRESET _WSAIOW(IOC_VENDOR, 12)

	BOOL bNewBehavior = FALSE;
	DWORD dwBytesReturned = 0;
	*/
	

	GetDlgItem(IDC_EDIT_MSG)->GetWindowTextW(msg);
	if (!msg.GetLength()){
		return;
	}

	AfxSocketInit();

	if (!UDPClient.Create(0, SOCK_DGRAM, m_strIP)){
		tmp.Format(_T("CSocket create failed: %d"), UDPClient.GetLastError());
		AfxMessageBox(tmp);
		return;
	}
	
	USES_CONVERSION;
	char *wStr = W2A(msg);
	int nLen = strlen(wStr);
	int sendSize = UDPClient.SendTo(wStr, nLen, m_nPort, m_strIP);
	if (sendSize != nLen){

		tmp.Format(_T("Send error, sendSize:%d buf_len:%d\n"), sendSize, nLen);
		AfxMessageBox(tmp);
		return;
	}

	tmp.Format(_T("Client send:%s"),  msg);
	AddOneLog(tmp);

	SOCKADDR_IN fromAddr;
	int fromSize = sizeof(fromAddr);
	int recvSize = UDPClient.ReceiveFrom(szBuf, ECHOMAX, (SOCKADDR*)&fromAddr, &fromSize, 0);
	if (recvSize <= 0 ){
		tmp.Format(_T("Receive error: %d"), UDPClient.GetLastError());
		AddOneLog(tmp);
		AfxMessageBox(tmp);
		return;
	}

	tmp = "Server echo:";
	//USES_CONVERSION;
	tmp.Append(A2T(szBuf));
	AddOneLog(tmp);

	parse_buffer(szBuf);

	//UDPClient.Close();
}

void CechoclientDlg::TCPConnect(){
// TODO: �ڴ���ӿؼ�֪ͨ����������
	CString tmp;

	if (!AfxSocketInit()){
		AfxMessageBox(_T("Failed to Initialize Sockets"), MB_OK | MB_ICONSTOP);
		return;
	}
	if (!m_TCPSocket.Create()){
		tmp.Format(_T("CSocket create failed: %d"), m_TCPSocket.GetLastError());
		AfxMessageBox(tmp);
		return;
	}
	if (m_TCPSocket.Connect(m_strIP, m_nPort)){
		CString tmp;
		tmp = _T("Connected to server.");
		AddOneLog(tmp);
		GetDlgItem(IDC_BTN_SEND)->EnableWindow(true);
		GetDlgItem(IDC_BTN_CONNECT)->EnableWindow(false);
	} else {
		tmp.Format(_T("CSocket connect failed: %d"), m_TCPSocket.GetLastError());
		AfxMessageBox(tmp);
		return;
	}
}

void CechoclientDlg::OnBnClickedBtnConnect()
{
	GetDlgItem(IDC_IPADDRESS)->GetWindowTextA(m_strIP);
	GetDlgItem(IDC_IPADDRESS)->GetWindowTextW(m_strIP);

	m_nPort = GetDlgItemInt(IDC_EDIT_PORT);

	if (((CButton*)GetDlgItem(IDC_RADIO_TCP))->GetCheck()){
		TCPConnect();
	} else {
		;
	}
}

void CechoclientDlg::AddOneLog(CString strLog){
	CString tmp;
	GetDlgItem(IDC_EDIT_LOG)->GetWindowTextW(tmp);
	tmp += strLog;
	tmp += "\r\n";
	GetDlgItem(IDC_EDIT_LOG)->SetWindowTextW(tmp);
	SendDlgItemMessage(IDC_EDIT_LOG, WM_VSCROLL, SB_BOTTOM,0); //������ʼ���ڵײ�
}

void CechoclientDlg::OnBnClickedBtnSend()
{
	// TODO: �ڴ���ӿؼ�֪ͨ����������
	GetDlgItem(IDC_IPADDRESS)->GetWindowTextW(m_strIP);
	m_nPort = GetDlgItemInt(IDC_EDIT_PORT);

	if (((CButton*)GetDlgItem(IDC_RADIO_TCP))->GetCheck()){
		TCPSend();
	} else {
		int x = GetTickCount();
		UDPSend();
		int y = GetTickCount() - x;
		CString str;
		str.Format(_T("ticks: %d"), y);
		AfxMessageBox(str);
	}
}


void CechoclientDlg::OnBnClickedRadioTcp()
{
	// TODO: �ڴ���ӿؼ�֪ͨ����������
	GetDlgItem(IDC_BTN_CONNECT)->EnableWindow(true);
	GetDlgItem(IDC_BTN_SEND)->EnableWindow(false);
}


void CechoclientDlg::OnBnClickedRadioUdp()
{
	// TODO: �ڴ���ӿؼ�֪ͨ����������
	GetDlgItem(IDC_BTN_CONNECT)->EnableWindow(false);
	GetDlgItem(IDC_BTN_SEND)->EnableWindow(true);
}
