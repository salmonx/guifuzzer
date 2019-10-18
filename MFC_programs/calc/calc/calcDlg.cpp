
// calcDlg.cpp : 实现文件
//

#include "stdafx.h"
#include "calc.h"
#include "calcDlg.h"
#include "afxdialogex.h"

#include "afxcmn.h"
#include "afxwin.h"


#ifdef _DEBUG
#define new DEBUG_NEW
#endif


// CcalcDlg 对话框




CcalcDlg::CcalcDlg(CWnd* pParent /*=NULL*/)
	: CDialogEx(CcalcDlg::IDD, pParent)
	, m_Number1(1)
	, m_Number2(1)
	, m_Result(0)
{
	m_hIcon = AfxGetApp()->LoadIcon(IDR_MAINFRAME);
}

void CcalcDlg::DoDataExchange(CDataExchange* pDX)
{
	CDialogEx::DoDataExchange(pDX);

	DDX_Text(pDX, IDC_EDIT1, m_Number1);
	DDX_Text(pDX, IDC_EDIT2, m_Number2);
	DDX_Text(pDX, IDC_EDIT3, m_Result);

	DDX_Control(pDX, IDC_COMBO1, m_comboOperator);
}
	//ON_BN_CLICKED(IDOK, &CcalcDlg::OnBnClickedOk)
BEGIN_MESSAGE_MAP(CcalcDlg, CDialogEx)
	ON_WM_PAINT()
	ON_WM_QUERYDRAGICON()

	ON_BN_CLICKED(IDC_BUTTON1, &CcalcDlg::OnBnClickedButton1)
	ON_BN_CLICKED(IDC_BUTTON2, &CcalcDlg::OnBnClickedButton2)
END_MESSAGE_MAP()


// CcalcDlg 消息处理程序

BOOL CcalcDlg::OnInitDialog()
{
	CDialogEx::OnInitDialog();

	// 设置此对话框的图标。当应用程序主窗口不是对话框时，框架将自动
	//  执行此操作
	SetIcon(m_hIcon, TRUE);			// 设置大图标
	SetIcon(m_hIcon, FALSE);		// 设置小图标

	// TODO: 在此添加额外的初始化代码

	m_comboOperator.AddString(_T("+"));
	m_comboOperator.AddString(_T("-"));
	m_comboOperator.AddString(_T("*"));
	m_comboOperator.AddString(_T("/"));


	m_comboOperator.SetCurSel(0);


	return TRUE;  // 除非将焦点设置到控件，否则返回 TRUE
}

// 如果向对话框添加最小化按钮，则需要下面的代码
//  来绘制该图标。对于使用文档/视图模型的 MFC 应用程序，
//  这将由框架自动完成。

void CcalcDlg::OnPaint()
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

	OnBnClickedButton2();
}

//当用户拖动最小化窗口时系统调用此函数取得光标
//显示。
HCURSOR CcalcDlg::OnQueryDragIcon()
{
	return static_cast<HCURSOR>(m_hIcon);
}


void CcalcDlg::OnBnClickedButton1()
{
	// TODO: 在此添加控件通知处理程序代码
	CString op;

	UpdateData(true);

	m_comboOperator.GetLBText(m_comboOperator.GetCurSel(), op);

	if (op == "+")
		m_Result =  m_Number1 + m_Number2;
	else if (op == "-")
		m_Result =  m_Number1 - m_Number2;
	else if (op == "*")
		m_Result =  m_Number1 * m_Number2;
	else if (op == "/")
		m_Result =  m_Number1 / m_Number2;

	UpdateData(false);
}


void CcalcDlg::OnBnClickedButton2()
{
	// TODO: 在此添加控件通知处理程序代码
		// Win32 API
	int m_Number1;
	int m_Number2;
	int m_Result;
	CString op;
	CString tmp;

	CEdit *pEdit1;
	CEdit *pEdit2;
	CEdit *pEditResult;

	CComboBox *pComboOp;

	UpdateData(true);

	pEdit1 =	  (CEdit *) GetDlgItem(IDC_EDIT1);
	pEdit2 =	  (CEdit *) GetDlgItem(IDC_EDIT2);
	pEditResult = (CEdit *) GetDlgItem(IDC_EDIT3);
	pComboOp    = (CComboBox *) GetDlgItem(IDC_COMBO1);

	pEdit1->GetWindowTextW(tmp);
	m_Number1 = _ttoi(tmp);

	CString tmp2;
	tmp2.Format(_T("%d"), m_Number1);
	
	pEdit2->GetWindowTextW(tmp);
	m_Number2 = _ttoi(tmp);

	//pComboOp->GetLBText(pComboOp->GetCurSel(), op);
	pComboOp->GetWindowTextW(op);
	if (op == "+")
		m_Result =  m_Number1 + m_Number2;
	else if (op == "-")
		m_Result =  m_Number1 - m_Number2;
	else if (op == "*"){

		m_Result =  m_Number1 * m_Number2;
		if ( m_Result == 21)
			
			abort();
	}
	else if (op == "/")
		if (!m_Number2) {
			MessageBox(_T("divide zero!"), _T("error"), 0);
			return;
		}else
			m_Result =  m_Number1 / m_Number2;
	else{
		return ;
	}

	

	tmp.Format(_T("%d%s%d=%d"), m_Number1,op, m_Number2, m_Result);
	MessageBox(tmp, _T("m_Result"), 0);

	tmp.Format(_T("%d"), m_Result);
	pEditResult->SetWindowTextW(tmp); 

}
