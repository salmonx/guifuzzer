
// echoclientDlg.h : ͷ�ļ�
//
#include "afxsock.h"
#pragma once


// CechoclientDlg �Ի���
class CechoclientDlg : public CDialogEx
{
// ����
public:
	CechoclientDlg(CWnd* pParent = NULL);	// ��׼���캯��
	void TCPConnect();
	
	void CechoclientDlg::TCPSend();
	void CechoclientDlg::UDPSend();
	void AddOneLog(CString strLog);

// �Ի�������
	enum { IDD = IDD_ECHOCLIENT_DIALOG };

	protected:
	virtual void DoDataExchange(CDataExchange* pDX);	// DDX/DDV ֧��


// ʵ��
protected:
	HICON m_hIcon;
	CSocket m_TCPSocket;
	CString m_strIP;
	int m_nPort;


	// ���ɵ���Ϣӳ�亯��
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
