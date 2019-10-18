
// echoserverDlg.h : ͷ�ļ�
//

#pragma once
#include "afxsock.h"


// CechoserverDlg �Ի���
class CechoserverDlg : public CDialogEx
{
// ����
public:
	CechoserverDlg(CWnd* pParent = NULL);	// ��׼���캯��
	void EchoTCPServer(CString strIP, int nPort);
	void EchoUDPServer(CString strIP, int nPort);
	void AddOneLog(CString strLog);
	void udp_recv_func(CSocket s);

// �Ի�������
	enum { IDD = IDD_ECHOSERVER_DIALOG };

	protected:
	virtual void DoDataExchange(CDataExchange* pDX);	// DDX/DDV ֧��


// ʵ��
protected:
	HICON m_hIcon;

	// ���ɵ���Ϣӳ�亯��
	virtual BOOL OnInitDialog();
	afx_msg void OnPaint();
	afx_msg HCURSOR OnQueryDragIcon();
	DECLARE_MESSAGE_MAP()
public:
	afx_msg void OnBnClickedBtnStart();
};
