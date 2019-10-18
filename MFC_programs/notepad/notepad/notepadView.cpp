
// notepadView.cpp : CnotepadView ���ʵ��
//

#include "stdafx.h"
// SHARED_HANDLERS ������ʵ��Ԥ��������ͼ������ɸѡ�������
// ATL ��Ŀ�н��ж��壬�����������Ŀ�����ĵ����롣
#ifndef SHARED_HANDLERS
#include "notepad.h"
#endif

#include "notepadDoc.h"
#include "notepadView.h"

#ifdef _DEBUG
#define new DEBUG_NEW
#endif


// CnotepadView

IMPLEMENT_DYNCREATE(CnotepadView, CView)

BEGIN_MESSAGE_MAP(CnotepadView, CView)
	// ��׼��ӡ����
	ON_COMMAND(ID_FILE_PRINT, &CView::OnFilePrint)
	ON_COMMAND(ID_FILE_PRINT_DIRECT, &CView::OnFilePrint)
	ON_COMMAND(ID_FILE_PRINT_PREVIEW, &CnotepadView::OnFilePrintPreview)
	ON_WM_CONTEXTMENU()
	ON_WM_RBUTTONUP()
END_MESSAGE_MAP()

// CnotepadView ����/����

CnotepadView::CnotepadView()
{
	// TODO: �ڴ˴���ӹ������

}

CnotepadView::~CnotepadView()
{
}

BOOL CnotepadView::PreCreateWindow(CREATESTRUCT& cs)
{
	// TODO: �ڴ˴�ͨ���޸�
	//  CREATESTRUCT cs ���޸Ĵ��������ʽ

	return CView::PreCreateWindow(cs);
}

// CnotepadView ����

void CnotepadView::OnDraw(CDC* /*pDC*/)
{
	CnotepadDoc* pDoc = GetDocument();
	ASSERT_VALID(pDoc);
	if (!pDoc)
		return;

	// TODO: �ڴ˴�Ϊ����������ӻ��ƴ���
}


// CnotepadView ��ӡ


void CnotepadView::OnFilePrintPreview()
{
#ifndef SHARED_HANDLERS
	AFXPrintPreview(this);
#endif
}

BOOL CnotepadView::OnPreparePrinting(CPrintInfo* pInfo)
{
	// Ĭ��׼��
	return DoPreparePrinting(pInfo);
}

void CnotepadView::OnBeginPrinting(CDC* /*pDC*/, CPrintInfo* /*pInfo*/)
{
	// TODO: ��Ӷ���Ĵ�ӡǰ���еĳ�ʼ������
}

void CnotepadView::OnEndPrinting(CDC* /*pDC*/, CPrintInfo* /*pInfo*/)
{
	// TODO: ��Ӵ�ӡ����е��������
}

void CnotepadView::OnRButtonUp(UINT /* nFlags */, CPoint point)
{
	ClientToScreen(&point);
	OnContextMenu(this, point);
}

void CnotepadView::OnContextMenu(CWnd* /* pWnd */, CPoint point)
{
#ifndef SHARED_HANDLERS
	theApp.GetContextMenuManager()->ShowPopupMenu(IDR_POPUP_EDIT, point.x, point.y, this, TRUE);
#endif
}


// CnotepadView ���

#ifdef _DEBUG
void CnotepadView::AssertValid() const
{
	CView::AssertValid();
}

void CnotepadView::Dump(CDumpContext& dc) const
{
	CView::Dump(dc);
}

CnotepadDoc* CnotepadView::GetDocument() const // �ǵ��԰汾��������
{
	ASSERT(m_pDocument->IsKindOf(RUNTIME_CLASS(CnotepadDoc)));
	return (CnotepadDoc*)m_pDocument;
}
#endif //_DEBUG


// CnotepadView ��Ϣ�������
