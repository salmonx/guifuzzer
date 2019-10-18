
// notepad.cpp : ����Ӧ�ó��������Ϊ��
//

#include "stdafx.h"
#include "afxwinappex.h"
#include "afxdialogex.h"
#include "notepad.h"
#include "MainFrm.h"

#include "ChildFrm.h"
#include "notepadDoc.h"
#include "notepadView.h"

#ifdef _DEBUG
#define new DEBUG_NEW
#endif


// CnotepadApp

BEGIN_MESSAGE_MAP(CnotepadApp, CWinAppEx)
	ON_COMMAND(ID_APP_ABOUT, &CnotepadApp::OnAppAbout)
	// �����ļ��ı�׼�ĵ�����
	ON_COMMAND(ID_FILE_NEW, &CWinAppEx::OnFileNew)
	//ON_COMMAND(ID_FILE_OPEN, &CWinAppEx::OnFileOpen)
	ON_COMMAND(ID_FILE_OPEN, OnMyFileOpen)
	ON_COMMAND(ID_FILE_OPEN_AND_PARSE, OnMyFileOpenAndParse)
	// ��׼��ӡ��������
	ON_COMMAND(ID_FILE_PRINT_SETUP, &CWinAppEx::OnFilePrintSetup)
END_MESSAGE_MAP()


// CnotepadApp ����

CnotepadApp::CnotepadApp()
{
	m_bHiColorIcons = TRUE;

	// ֧����������������
	m_dwRestartManagerSupportFlags = AFX_RESTART_MANAGER_SUPPORT_ALL_ASPECTS;
#ifdef _MANAGED
	// ���Ӧ�ó��������ù�����������ʱ֧��(/clr)�����ģ���:
	//     1) �����д˸������ã�������������������֧�ֲ�������������
	//     2) ��������Ŀ�У������밴������˳���� System.Windows.Forms ������á�
	System::Windows::Forms::Application::SetUnhandledExceptionMode(System::Windows::Forms::UnhandledExceptionMode::ThrowException);
#endif

	// TODO: ������Ӧ�ó��� ID �ַ����滻ΪΨһ�� ID �ַ�����������ַ�����ʽ
	//Ϊ CompanyName.ProductName.SubProduct.VersionInformation
	SetAppID(_T("notepad.AppID.NoVersion"));

	// TODO: �ڴ˴���ӹ�����룬
	// ��������Ҫ�ĳ�ʼ�������� InitInstance ��
}

// Ψһ��һ�� CnotepadApp ����

CnotepadApp theApp;


// CnotepadApp ��ʼ��

BOOL CnotepadApp::InitInstance()
{
	// ���һ�������� Windows XP �ϵ�Ӧ�ó����嵥ָ��Ҫ
	// ʹ�� ComCtl32.dll �汾 6 ����߰汾�����ÿ��ӻ���ʽ��
	//����Ҫ InitCommonControlsEx()�����򣬽��޷��������ڡ�
	INITCOMMONCONTROLSEX InitCtrls;
	InitCtrls.dwSize = sizeof(InitCtrls);
	// ��������Ϊ��������Ҫ��Ӧ�ó�����ʹ�õ�
	// �����ؼ��ࡣ
	InitCtrls.dwICC = ICC_WIN95_CLASSES;
	InitCommonControlsEx(&InitCtrls);

	CWinAppEx::InitInstance();


	// ��ʼ�� OLE ��
	if (!AfxOleInit())
	{
		AfxMessageBox(IDP_OLE_INIT_FAILED);
		return FALSE;
	}

	AfxEnableControlContainer();

	EnableTaskbarInteraction();

	// ʹ�� RichEdit �ؼ���Ҫ  AfxInitRichEdit2()	
	// AfxInitRichEdit2();

	// ��׼��ʼ��
	// ���δʹ����Щ���ܲ�ϣ����С
	// ���տ�ִ���ļ��Ĵ�С����Ӧ�Ƴ�����
	// ����Ҫ���ض���ʼ������
	// �������ڴ洢���õ�ע�����
	// TODO: Ӧ�ʵ��޸ĸ��ַ�����
	// �����޸�Ϊ��˾����֯��
	SetRegistryKey(_T("Ӧ�ó��������ɵı���Ӧ�ó���"));
	LoadStdProfileSettings(4);  // ���ر�׼ INI �ļ�ѡ��(���� MRU)


	InitContextMenuManager();

	InitKeyboardManager();

	InitTooltipManager();
	CMFCToolTipInfo ttParams;
	ttParams.m_bVislManagerTheme = TRUE;
	theApp.GetTooltipManager()->SetTooltipParams(AFX_TOOLTIP_TYPE_ALL,
		RUNTIME_CLASS(CMFCToolTipCtrl), &ttParams);

	// ע��Ӧ�ó�����ĵ�ģ�塣�ĵ�ģ��
	// �������ĵ�����ܴ��ں���ͼ֮�������
	CMultiDocTemplate* pDocTemplate;
	pDocTemplate = new CMultiDocTemplate(IDR_notepadTYPE,
		RUNTIME_CLASS(CnotepadDoc),
		RUNTIME_CLASS(CChildFrame), // �Զ��� MDI �ӿ��
		RUNTIME_CLASS(CnotepadView));
	if (!pDocTemplate)
		return FALSE;
	AddDocTemplate(pDocTemplate);

	// ������ MDI ��ܴ���
	CMainFrame* pMainFrame = new CMainFrame;
	if (!pMainFrame || !pMainFrame->LoadFrame(IDR_MAINFRAME))
	{
		delete pMainFrame;
		return FALSE;
	}
	m_pMainWnd = pMainFrame;
	// �������к�׺ʱ�ŵ��� DragAcceptFiles
	//  �� MDI Ӧ�ó����У���Ӧ������ m_pMainWnd ֮����������

	// ������׼ shell ���DDE�����ļ�������������
	CCommandLineInfo cmdInfo;
	ParseCommandLine(cmdInfo);



	// ��������������ָ����������
	// �� /RegServer��/Register��/Unregserver �� /Unregister ����Ӧ�ó����򷵻� FALSE��

	if (!ProcessShellCommand(cmdInfo))
		return FALSE;

	// �������ѳ�ʼ���������ʾ����������и���
	pMainFrame->ShowWindow(m_nCmdShow);
	pMainFrame->UpdateWindow();


	/*
	1. �������������������ȡ�ļ�·��������ParseFile�����ض��ļ����³������
	*/
	CString  m_FilePath;
	if ( __argc >= 2) {
		m_FilePath = __targv[1];

		if (PathFileExists(m_FilePath)){
			ParseFile(m_FilePath);
		}else{
			MessageBox(NULL, m_FilePath, _T("File not exists"), MB_OK);
		}
	}
	return TRUE;
}
void CnotepadApp::ParseFileWrap(CString m_FilePath) {
	char *pBuf = new char[100];
	ParseFile(m_FilePath);
	delete pBuf;
}

void CnotepadApp::ParseFile(CString m_FilePath)
{
	//CString m_FilePath = m_FilePath;

	
	CFile m_file;
	m_file.Open(m_FilePath,  CFile::shareDenyNone);

	int nLen = m_file.GetLength();
	char *pBuf = new char[nLen + 1];
	pBuf[nLen] = 0;
	m_file.Read(pBuf, nLen);

	if (nLen >= 8)
		if (pBuf[0] == 'p')
			if (pBuf[1] == 'a')
				if (pBuf[2] == 's')
					if (pBuf[3] == 's')
						if (pBuf[4] == 'w')
							if (pBuf[5] == 'o')
								if (pBuf[6] == 'r')
									if (pBuf[7] == 'd'){
										char *ptr = NULL;
										ptr[0] = 1;
									}
	CString sLen;
	sLen.Format(_T("�ļ��ַ�����: %d"), nLen);
	

	MessageBox(NULL, sLen, m_FilePath, MB_OK);


	m_file.Close(); 
	
	//CString str;
	//str.Format(_T("%d"), n);
	//MessageBox(NULL, sLen, sLen, MB_OK);
	//delete pBuf;


}


// ���ڴ��ļ��Ի����Ӧ�ó�������
void CnotepadApp::OnMyFileOpen()
{

	CFileDialog  fileDlg(TRUE);//�����true��ʾ���ļ��򿪶Ի���FALSE�Ļ������ļ�����Ի���

	fileDlg.m_ofn.lpstrFilter = _T("All files(*.*)");
	if (IDOK == fileDlg.DoModal())
	{
		/*
		if (!PathFileExists(fileDlg.GetPathName())) {
			MessageBox(NULL, _T("�ļ������ڣ�"), fileDlg.GetPathName(), MB_OK);
			return;
		}
		*/
		
		CString m_FilePath = fileDlg.GetPathName();
		if (!PathFileExists(m_FilePath)) {
			MessageBox(NULL, _T("�ļ������ڣ�"), m_FilePath, MB_OK);
			return;
		}else{
			MessageBox(NULL, m_FilePath, _T("�ļ����ڣ�"), MB_OK);
		}
		
		ParseFile(m_FilePath);
	}
}

// ���ڴ��ļ��Ի����Ӧ�ó�������
// ���������ڲ���ɴ��ļ��ͽ���
void CnotepadApp::OnMyFileOpenAndParse()
{

	CFileDialog  fileDlg(TRUE);//�����true��ʾ���ļ��򿪶Ի���FALSE�Ļ������ļ�����Ի���


	fileDlg.m_ofn.lpstrFilter = _T("All files(*.*)");
	if (IDOK == fileDlg.DoModal())
	{
		CString m_FilePath = fileDlg.GetPathName();
		if (!PathFileExists(m_FilePath)) {

			//p = (int*)&m_FilePath;


			//str1.Format(_T("%s"), p);
			//str2.Format(_T("%s"), p);

			MessageBox(NULL, m_FilePath,  _T("m_FilePath�ļ������ڣ�"), MB_OK);
			return;
		}
			
		CFile m_file;
		m_file.Open(m_FilePath, CFile::modeRead);

		int nLen = m_file.GetLength();
		char *pBuf = new char[nLen + 1];
		pBuf[nLen] = 0;
		m_file.Read(pBuf, nLen);

		if (nLen >= 8)
			if (pBuf[0] == 'p')
				if (pBuf[1] == 'a')
					if (pBuf[2] == 's')
						if (pBuf[3] == 's')
							if (pBuf[4] == 'w')
								if (pBuf[5] == 'o')
									if (pBuf[6] == 'r')
										if (pBuf[7] == 'd') {
											char *ptr = NULL;
											ptr[0] = 1;
										}
		CString sLen;
		sLen.Format(_T("�ļ��ַ�����: %d"), nLen);

		MessageBox(NULL, sLen, m_FilePath, MB_OK);

		delete pBuf;
	}
}



int CnotepadApp::ExitInstance()
{
	//TODO: �����������ӵĸ�����Դ
	AfxOleTerm(FALSE);

	return CWinAppEx::ExitInstance();
}

// CnotepadApp ��Ϣ�������


// ����Ӧ�ó��򡰹��ڡ��˵���� CAboutDlg �Ի���

class CAboutDlg : public CDialogEx
{
public:
	CAboutDlg();

// �Ի�������
	enum { IDD = IDD_ABOUTBOX };

protected:
	virtual void DoDataExchange(CDataExchange* pDX);    // DDX/DDV ֧��

// ʵ��
protected:
	DECLARE_MESSAGE_MAP()
};

CAboutDlg::CAboutDlg() : CDialogEx(CAboutDlg::IDD)
{
}

void CAboutDlg::DoDataExchange(CDataExchange* pDX)
{
	CDialogEx::DoDataExchange(pDX);
}

BEGIN_MESSAGE_MAP(CAboutDlg, CDialogEx)
END_MESSAGE_MAP()

// �������жԻ����Ӧ�ó�������
void CnotepadApp::OnAppAbout()
{
	CAboutDlg aboutDlg;
	aboutDlg.DoModal();
}

// CnotepadApp �Զ������/���淽��

void CnotepadApp::PreLoadState()
{
	BOOL bNameValid;
	CString strName;
	bNameValid = strName.LoadString(IDS_EDIT_MENU);
	ASSERT(bNameValid);
	GetContextMenuManager()->AddMenu(strName, IDR_POPUP_EDIT);
	bNameValid = strName.LoadString(IDS_EXPLORER);
	ASSERT(bNameValid);
	GetContextMenuManager()->AddMenu(strName, IDR_POPUP_EXPLORER);
}

void CnotepadApp::LoadCustomState()
{
}

void CnotepadApp::SaveCustomState()
{
}

// CnotepadApp ��Ϣ�������



