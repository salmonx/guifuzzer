
// notepad.h : notepad Ӧ�ó������ͷ�ļ�
//
#pragma once

#ifndef __AFXWIN_H__
	#error "�ڰ������ļ�֮ǰ������stdafx.h�������� PCH �ļ�"
#endif

#include "resource.h"       // ������


// CnotepadApp:
// �йش����ʵ�֣������ notepad.cpp
//

class CnotepadApp : public CWinAppEx
{
public:
	CnotepadApp();

	void ParseFile(CString filepath);
	//void ParseFile(int i);
	void ParseFileWrap(CString filepath);


// ��д
public:
	virtual BOOL InitInstance();
	virtual int ExitInstance();

// ʵ��
	UINT  m_nAppLook;
	BOOL  m_bHiColorIcons;

	virtual void PreLoadState();
	virtual void LoadCustomState();
	virtual void SaveCustomState();

	afx_msg void OnAppAbout();
	afx_msg void OnMyFileOpen();
	afx_msg void OnMyFileOpenAndParse();
	DECLARE_MESSAGE_MAP()
};

extern CnotepadApp theApp;
