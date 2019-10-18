
// echoclientDlg.h : 头文件
//
#include "afxsock.h"
#pragma once


// CechoclientDlg 对话框
class CechoclientDlg : public CDialogEx
{
// 构造
public:
	CechoclientDlg(CWnd* pParent = NULL);	// 标准构造函数
	void TCPConnect();
	
	void CechoclientDlg::TCPSend();
	void CechoclientDlg::UDPSend();
	void AddOneLog(CString strLog);

// 对话框数据
	enum { IDD = IDD_ECHOCLIENT_DIALOG };

	protected:
	virtual void DoDataExchange(CDataExchange* pDX);	// DDX/DDV 支持


// 实现
protected:
	HICON m_hIcon;
	CSocket m_TCPSocket;
	CString m_strIP;
	int m_nPort;


	// 生成的消息映射函数
	virtual BOOL OnInitDialog();
	afx_msg void OnPaint();
	afx_msg HCURSOR OnQueryDragIcon();
	DECLARE_MESSAGE_MAP()
public:
	afx_msg void OnBnClickedBtnConnect();
	afx_msg void OnBnClickedBtnSend();
	afx_msg void OnBnClickedRadioTcp();
	afx_msg void OnBnClickedRadioUdp();
};
