
// echoclient.h : PROJECT_NAME Ӧ�ó������ͷ�ļ�
//

#pragma once

#ifndef __AFXWIN_H__
	#error "�ڰ������ļ�֮ǰ������stdafx.h�������� PCH �ļ�"
#endif

#include "resource.h"		// ������


// CechoclientApp:
// �йش����ʵ�֣������ echoclient.cpp
//

class CechoclientApp : public CWinApp
{
public:
	CechoclientApp();

// ��д
public:
	virtual BOOL InitInstance();

// ʵ��

	DECLARE_MESSAGE_MAP()
};

extern CechoclientApp theApp;