
// echoserverDlg.cpp : ʵ���ļ�
//

#include "stdafx.h"
#include "echoserver.h"
#include "echoserverDlg.h"
#include "afxdialogex.h"
#include "afxsock.h"

#ifdef _DEBUG
#define new DEBUG_NEW
#endif


// CechoserverDlg �Ի���

CechoserverDlg::CechoserverDlg(CWnd* pParent /*=NULL*/)
	: CDialogEx(CechoserverDlg::IDD, pParent)
{
	m_hIcon = AfxGetApp()->LoadIcon(IDR_MAINFRAME);
}

void CechoserverDlg::DoDataExchange(CDataExchange* pDX)
{
	CDialogEx::DoDataExchange(pDX);
}

BEGIN_MESSAGE_MAP(CechoserverDlg, CDialogEx)
	ON_WM_PAINT()
	ON_WM_QUERYDRAGICON()
	ON_BN_CLICKED(IDC_BTN_START, &CechoserverDlg::OnBnClickedBtnStart)
END_MESSAGE_MAP()


// CechoserverDlg ��Ϣ�������

BOOL CechoserverDlg::OnInitDialog()
{
	CDialogEx::OnInitDialog();

	// ���ô˶Ի����ͼ�ꡣ��Ӧ�ó��������ڲ��ǶԻ���ʱ����ܽ��Զ�
	//  ִ�д˲���
	SetIcon(m_hIcon, TRUE);			// ���ô�ͼ��
	SetIcon(m_hIcon, FALSE);		// ����Сͼ��

	// TODO: �ڴ���Ӷ���ĳ�ʼ������

	CString strPort("9102");
	GetDlgItem(IDC_PORT)->SetWindowTextW(strPort);

	((CButton*)GetDlgItem(IDC_RADIO_TCP))->SetCheck(true);



	return TRUE;  // ���ǽ��������õ��ؼ������򷵻� TRUE
}

// �����Ի��������С����ť������Ҫ����Ĵ���
//  �����Ƹ�ͼ�ꡣ����ʹ���ĵ�/��ͼģ�͵� MFC Ӧ�ó���
//  �⽫�ɿ���Զ���ɡ�

void CechoserverDlg::OnPaint()
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
HCURSOR CechoserverDlg::OnQueryDragIcon()
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


void CechoserverDlg::udp_recv_func(CSocket UDPServer){
	CString tmp;
	const int ECHOMAX = 1024;
	char szBuf[ECHOMAX+1] = {0};

	SOCKADDR_IN clientAddr; 
		int clientAddrSize = sizeof(clientAddr);
	
		int nrecvSize = UDPServer.ReceiveFrom(szBuf, 
			ECHOMAX, (SOCKADDR*)&clientAddr, &clientAddrSize, 0);

		if (nrecvSize < 0) {
			CString err;
			err.Format(_T("Receive failed: %d"), UDPServer.GetLastError());

			GetDlgItem(IDC_EDIT_LOG)->GetWindowTextW(tmp);
			tmp += err;
			GetDlgItem(IDC_EDIT_LOG)->SetWindowTextW(tmp);
			
		}else{
			USES_CONVERSION;
			tmp.Format(_T("Receive:%s"), A2T(szBuf));
			AddOneLog(tmp);

			parse_buffer(szBuf);


		}

		if (UDPServer.SendTo(szBuf, nrecvSize, (SOCKADDR*)&clientAddr,
			clientAddrSize, 0) != nrecvSize) {
			tmp.Format(_T("Receive failed: %d"), UDPServer.GetLastError());
			AddOneLog(tmp);

		}
}


void CechoserverDlg::EchoTCPServer(CString strIP, int nPort){
	CSocket aSocket, serverSocket;
	CString tmp;
	const int ECHOMAX = 1024;
	char szBuf[ECHOMAX+1] = {0};
	//char szOutMsg[ECHOMAX+1] = {0};


	if (!AfxSocketInit()){
		AfxMessageBox(_T("Failed to Initialize Sockets"), MB_OK | MB_ICONSTOP);
		return;
	}
	
	int ret = aSocket.Socket();
	if (!ret){
		tmp.Format(_T("CSocket create failed: ret: %d errorcode: %d"), ret, aSocket.GetLastError());
		AfxMessageBox(tmp);
		return;
	}
	BOOL bOptVal = TRUE;
	int bOptLen = sizeof(BOOL);
	// avoid 10048 problem
	aSocket.SetSockOpt(SO_REUSEADDR, (void*)&bOptVal, bOptLen, SOL_SOCKET);
	
	if (!aSocket.Bind(nPort, strIP)){
		tmp.Format(_T("CSocket bind failed: %d"), aSocket.GetLastError());
		AfxMessageBox(tmp);
		return;
	}

	if (!aSocket.Listen(10)){
		tmp.Format(_T("Listen Failed:%d"), aSocket.GetLastError());
		AfxMessageBox(tmp);
		return;
	}
	
	while (true){
		memset(szBuf, 0, ECHOMAX);
		int ret = 1;
		ret = aSocket.Accept(serverSocket);
		if (!ret){
			continue;
		}else{
			while (true){
				int recvSize = serverSocket.Receive(szBuf, ECHOMAX);

				if (recvSize <= 0) {
					tmp.Format(_T("Receive error: %d"), serverSocket.GetLastError());
					AddOneLog(tmp);
					break;
				};

				tmp = "Recive:";
				USES_CONVERSION;
				tmp.Append(A2T(szBuf));
				AddOneLog(tmp);

				parse_buffer(szBuf);

				serverSocket.Send(szBuf, strlen(szBuf));

				break;
			}
			serverSocket.Close();
		}
		break;
	}
}




void CechoserverDlg::EchoUDPServer(CString strIP, int nPort){

	CString tmp;
	const int ECHOMAX = 1024;
	char szBuf[ECHOMAX+1] = {0};
	// Initialize the AfxSocket
	//AfxSocketInit(NULL);
 
	CSocket UDPServer;
	if (!AfxSocketInit()){
		AfxMessageBox(_T("Failed to Initialize Sockets"), MB_OK | MB_ICONSTOP);
		return;
	}
	if (!UDPServer.Create(nPort, SOCK_DGRAM, strIP)){
		tmp.Format(_T("CSocket create failed: %d"), UDPServer.GetLastError());
		AfxMessageBox(tmp);
		return;
	}

	while (true) {

		CString tmp;
		const int ECHOMAX = 1024;
		char szBuf[ECHOMAX+1] = {0};

		SOCKADDR_IN clientAddr; 
		int clientAddrSize = sizeof(clientAddr);
	
		int nrecvSize = UDPServer.ReceiveFrom(szBuf, 
			ECHOMAX, (SOCKADDR*)&clientAddr, &clientAddrSize, 0);

		if (nrecvSize < 0) {
			CString err;
			err.Format(_T("Receive failed: %d"), UDPServer.GetLastError());
			AfxMessageBox(tmp);
			GetDlgItem(IDC_EDIT_LOG)->GetWindowTextW(tmp);
			tmp += err;
			GetDlgItem(IDC_EDIT_LOG)->SetWindowTextW(tmp);
			
		}else{
			USES_CONVERSION;
			tmp.Format(_T("Receive:%s"), A2T(szBuf));
			//AfxMessageBox(tmp);
			AddOneLog(tmp);

			parse_buffer(szBuf);


		}

		if (UDPServer.SendTo(szBuf, nrecvSize, (SOCKADDR*)&clientAddr,
			clientAddrSize, 0) != nrecvSize) {
			tmp.Format(_T("Receive failed: %d"), UDPServer.GetLastError());
			AfxMessageBox(tmp);
			AddOneLog(tmp);

		}
		
		break;
	}


}

void CechoserverDlg::AddOneLog(CString strLog){
	CString tmp;
	GetDlgItem(IDC_EDIT_LOG)->GetWindowTextW(tmp);
	tmp += strLog;
	tmp += "\r\n";
	CEdit *edit = (CEdit *)GetDlgItem(IDC_EDIT_LOG);
	edit->SetWindowTextW(tmp);
	SendDlgItemMessage(IDC_EDIT_LOG, WM_VSCROLL, SB_BOTTOM,0); //������ʼ���ڵײ�
}





void CechoserverDlg::OnBnClickedBtnStart()
{
	// TODO: �ڴ���ӿؼ�֪ͨ����������
	/*
	CString str;
	int i = 0;	
	CString tmp;
	long t1 = GetTickCount();
	
	while (i++ < 100){

		tmp += "01234567890012345678900123456789001234567890012345678900123456789001234567890012345678900123456789001234567890";
		GetDlgItem(IDC_EDIT_LOG)->SetWindowTextW(tmp);

	}
	//GetDlgItem(IDC_EDIT_LOG)->GetWindowTextW(tmp);
	s
	long t2 = GetTickCount(); //+15ms

	str.Format(_T("time:%dms"),t2-t1);
	AfxMessageBox(str);
	return ;
	*/

	CString strIP("127.0.0.1");
	//int nPort = GetDlgItemInt(IDC_PORT);
	int nPort = 9102;

	GetDlgItem(IDC_BTN_START)->EnableWindow(false);

	if (((CButton*)GetDlgItem(IDC_RADIO_TCP))->GetCheck()){
		EchoTCPServer(strIP, nPort);
	} else {
		EchoUDPServer(strIP, nPort);
	}

}
