// CView.cpp

#include "../Include/afxwin.h"

///////////////////////////////////////////////////////////////////////////

CView::CView()
	: m_pDocument(NULL)
{
}

BOOL CView::DoPreparePrinting(CPrintInfo* /*pInfo*/)
{
	return TRUE;
}

CDocument* CView::GetDocument() const
{
	return m_pDocument;
}

BOOL CView::OnPreparePrinting(CPrintInfo* pInfo)
{
	return DoPreparePrinting(pInfo);
}

BEGIN_MESSAGE_MAP(CView, CWnd)
	ON_WM_PAINT()
	ON_COMMAND(ID_FILE_PRINT, OnFilePrint)
	ON_COMMAND(ID_FILE_PRINT_PREVIEW, OnFilePrintPreview)
END_MESSAGE_MAP()

void CView::OnPaint()
{
	CPaintDC dc(this);
	OnDraw(&dc);
}

void CView::OnFilePrint()
{
}

void CView::OnFilePrintPreview()
{
}

IMPLEMENT_DYNAMIC(CView, CWnd)

///////////////////////////////////////////////////////////////////////////

IMPLEMENT_DYNCREATE(CCtrlView, CView)

///////////////////////////////////////////////////////////////////////////

IMPLEMENT_DYNCREATE(CEditView, CCtrlView)

///////////////////////////////////////////////////////////////////////////

IMPLEMENT_DYNCREATE(CListView, CCtrlView)

///////////////////////////////////////////////////////////////////////////

IMPLEMENT_DYNCREATE(CRichEditView, CCtrlView)

///////////////////////////////////////////////////////////////////////////

IMPLEMENT_DYNCREATE(CTreeView, CCtrlView)

///////////////////////////////////////////////////////////////////////////

IMPLEMENT_DYNCREATE(CScrollView, CView)

///////////////////////////////////////////////////////////////////////////

IMPLEMENT_DYNCREATE(CFormView, CScrollView)

///////////////////////////////////////////////////////////////////////////

