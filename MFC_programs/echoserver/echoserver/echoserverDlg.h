
// echoserverDlg.h : 头文件
//

#pragma once
#include "afxsock.h"


// CechoserverDlg 对话框
class CechoserverDlg : public CDialogEx
{
// 构造
public:
	CechoserverDlg(CWnd* pParent = NULL);	// 标准构造函数
	void EchoTCPServer(CString strIP, int nPort);
	void EchoUDPServer(CString strIP, int nPort);
	void AddOneLog(CString strLog);
	void udp_recv_func(CSocket s);

// 对话框数据
	enum { IDD = IDD_ECHOSERVER_DIALOG };

	protected:
	virtual void DoDataExchange(CDataExchange* pDX);	// DDX/DDV 支持


// 实现
protected:
	HICON m_hIcon;

	// 生成的消息映射函数
	virtual BOOL OnInitDialog();
	afx_msg void OnPaint();
	afx_msg HCURSOR OnQueryDragIcon();
	DECLARE_MESSAGE_MAP()
public:
	afx_msg void OnBnClickedBtnStart();
};
