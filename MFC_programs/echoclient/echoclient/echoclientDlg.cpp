
// echoclientDlg.cpp : 实现文件
//

#include "stdafx.h"
#include "echoclient.h"
#include "echoclientDlg.h"
#include "afxdialogex.h"


#ifdef _DEBUG
#define new DEBUG_NEW
#endif


// CechoclientDlg 对话框

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


// CechoclientDlg 消息处理程序

BOOL CechoclientDlg::OnInitDialog()
{
	CDialogEx::OnInitDialog();

	// 设置此对话框的图标。当应用程序主窗口不是对话框时，框架将自动
	//  执行此操作
	SetIcon(m_hIcon, TRUE);			// 设置大图标
	SetIcon(m_hIcon, FALSE);		// 设置小图标

	// TODO: 在此添加额外的初始化代码
	CString strPort("9102");
	GetDlgItem(IDC_EDIT_PORT)->SetWindowTextW(strPort);
	GetDlgItem(IDC_IPADDRESS)->SetWindowTextW(_T("127.0.0.1"));
	((CButton*)GetDlgItem(IDC_RADIO_TCP))->SetCheck(true);
	GetDlgItem(IDC_BTN_SEND)->EnableWindow(false);

	return TRUE;  // 除非将焦点设置到控件，否则返回 TRUE
}

// 如果向对话框添加最小化按钮，则需要下面的代码
//  来绘制该图标。对于使用文档/视图模型的 MFC 应用程序，
//  这将由框架自动完成。

void CechoclientDlg::OnPaint()
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
// TODO: 在此添加控件通知处理程序代码
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
	SendDlgItemMessage(IDC_EDIT_LOG, WM_VSCROLL, SB_BOTTOM,0); //滚动条始终在底部
}

void CechoclientDlg::OnBnClickedBtnSend()
{
	// TODO: 在此添加控件通知处理程序代码
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
	// TODO: 在此添加控件通知处理程序代码
	GetDlgItem(IDC_BTN_CONNECT)->EnableWindow(true);
	GetDlgItem(IDC_BTN_SEND)->EnableWindow(false);
}


void CechoclientDlg::OnBnClickedRadioUdp()
{
	// TODO: 在此添加控件通知处理程序代码
	GetDlgItem(IDC_BTN_CONNECT)->EnableWindow(false);
	GetDlgItem(IDC_BTN_SEND)->EnableWindow(true);
}
