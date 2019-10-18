
// echoserverDlg.cpp : 实现文件
//

#include "stdafx.h"
#include "echoserver.h"
#include "echoserverDlg.h"
#include "afxdialogex.h"
#include "afxsock.h"

#ifdef _DEBUG
#define new DEBUG_NEW
#endif


// CechoserverDlg 对话框

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


// CechoserverDlg 消息处理程序

BOOL CechoserverDlg::OnInitDialog()
{
	CDialogEx::OnInitDialog();

	// 设置此对话框的图标。当应用程序主窗口不是对话框时，框架将自动
	//  执行此操作
	SetIcon(m_hIcon, TRUE);			// 设置大图标
	SetIcon(m_hIcon, FALSE);		// 设置小图标

	// TODO: 在此添加额外的初始化代码

	CString strPort("9102");
	GetDlgItem(IDC_PORT)->SetWindowTextW(strPort);

	((CButton*)GetDlgItem(IDC_RADIO_TCP))->SetCheck(true);



	return TRUE;  // 除非将焦点设置到控件，否则返回 TRUE
}

// 如果向对话框添加最小化按钮，则需要下面的代码
//  来绘制该图标。对于使用文档/视图模型的 MFC 应用程序，
//  这将由框架自动完成。

void CechoserverDlg::OnPaint()
{
	if (IsIconic())
	{
		CPaintDC dc(this); // 用于绘制的设备上下文

		SendMessage(WM_ICONERASEBKGND, reinterpret_cast<WPARAM>(dc.GetSafeHdc()), 0);

		// 使图标在工作区矩形中居中
		int cxIcon = GetSystemMetrics(SM_CXICON);
		int cyIcon = GetSystemMetrics(SM_CYICON);
		CRect rect;
		GetClientRect(&rect);
		int x = (rect.Width() - cxIcon + 1) / 2;
		int y = (rect.Height() - cyIcon + 1) / 2;

		// 绘制图标
		dc.DrawIcon(x, y, m_hIcon);
	}
	else
	{
		CDialogEx::OnPaint();
	}
}

//当用户拖动最小化窗口时系统调用此函数取得光标
//显示。
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
	SendDlgItemMessage(IDC_EDIT_LOG, WM_VSCROLL, SB_BOTTOM,0); //滚动条始终在底部
}





void CechoserverDlg::OnBnClickedBtnStart()
{
	// TODO: 在此添加控件通知处理程序代码
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
