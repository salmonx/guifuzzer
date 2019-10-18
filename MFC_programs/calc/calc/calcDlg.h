
// calcDlg.h : 头文件
//

#pragma once
#include "afxcmn.h"
#include "afxwin.h"


// CcalcDlg 对话框
class CcalcDlg : public CDialogEx
{
// 构造
public:
	CcalcDlg(CWnd* pParent = NULL);	// 标准构造函数

// 对话框数据
	enum { IDD = IDD_CALC_DIALOG };

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
	afx_msg void OnEnChangeEdit2();
	afx_msg void OnLvnItemchangedList1(NMHDR *pNMHDR, LRESULT *pResult);
	int m_Number1;
	int m_Number2;
	int m_Result;

	CComboBox m_comboOperator;
	afx_msg void OnBnClickedButton1();
	afx_msg void OnBnClickedButton2();
};
