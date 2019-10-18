
// calcDlg.cpp : ʵ���ļ�
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


// CcalcDlg �Ի���




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


// CcalcDlg ��Ϣ�������

BOOL CcalcDlg::OnInitDialog()
{
	CDialogEx::OnInitDialog();

	// ���ô˶Ի����ͼ�ꡣ��Ӧ�ó��������ڲ��ǶԻ���ʱ����ܽ��Զ�
	//  ִ�д˲���
	SetIcon(m_hIcon, TRUE);			// ���ô�ͼ��
	SetIcon(m_hIcon, FALSE);		// ����Сͼ��

	// TODO: �ڴ���Ӷ���ĳ�ʼ������

	m_comboOperator.AddString(_T("+"));
	m_comboOperator.AddString(_T("-"));
	m_comboOperator.AddString(_T("*"));
	m_comboOperator.AddString(_T("/"));


	m_comboOperator.SetCurSel(0);


	return TRUE;  // ���ǽ��������õ��ؼ������򷵻� TRUE
}

// �����Ի��������С����ť������Ҫ����Ĵ���
//  �����Ƹ�ͼ�ꡣ����ʹ���ĵ�/��ͼģ�͵� MFC Ӧ�ó���
//  �⽫�ɿ���Զ���ɡ�

void CcalcDlg::OnPaint()
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

	OnBnClickedButton2();
}

//���û��϶���С������ʱϵͳ���ô˺���ȡ�ù��
//��ʾ��
HCURSOR CcalcDlg::OnQueryDragIcon()
{
	return static_cast<HCURSOR>(m_hIcon);
}


void CcalcDlg::OnBnClickedButton1()
{
	// TODO: �ڴ���ӿؼ�֪ͨ����������
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
	// TODO: �ڴ���ӿؼ�֪ͨ����������
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
