
// MainFrm.cpp : CMainFrame ���ʵ��
//

#include "stdafx.h"
#include "MarkdownEditor.h"

#include "MainFrm.h"
#include "LeftView.h"
#include "MarkdownEditorView.h"
#include "MarkdownEditorDoc.h"
#include "TocView.h"

#ifdef _DEBUG
#define new DEBUG_NEW
#endif

#define IS_VIEWER_KEY  "isViewer"
#define IS_TOC_KEY "isTocVisible"

void saveViewer(bool enable)
{
	int value = enable ? 1 : 0;
	AfxGetApp()->WriteProfileInt("", IS_VIEWER_KEY, value);
}

bool isViewer()
{
	int value = AfxGetApp()->GetProfileInt("", IS_VIEWER_KEY, 0);
	return value == 1;
}

void saveTocVisible(bool enable)
{
	int value = enable ? 1 : 0;
	AfxGetApp()->WriteProfileInt("", IS_TOC_KEY, value);
}

bool isTocVisible()
{
	int value = AfxGetApp()->GetProfileInt("", IS_TOC_KEY, 1);
	return value == 1;
}

// CMainFrame

IMPLEMENT_DYNCREATE(CMainFrame, CFrameWnd)

BEGIN_MESSAGE_MAP(CMainFrame, CFrameWnd)
	ON_WM_CREATE()
	ON_WM_SIZE()
	ON_COMMAND(IDM_SWITCH, &CMainFrame::OnSwitch)
	ON_COMMAND(IDM_ABOUT, &CMainFrame::OnAbout)
	ON_COMMAND(IDM_TOGGLE_TOC, &CMainFrame::OnToggleToc)
END_MESSAGE_MAP()

static UINT indicators[] =
{
	ID_SEPARATOR,
	ID_INDICATOR_CAPS,
	ID_INDICATOR_NUM,
	ID_INDICATOR_SCRL,
};

// CMainFrame ����/����

CMainFrame::CMainFrame()
{
	_bInited = false;
	_bShowLeft = !isViewer();
	_bShowToc = isTocVisible();
}

CMainFrame::~CMainFrame()
{
}

int CMainFrame::OnCreate(LPCREATESTRUCT lpCreateStruct)
{
	if (CFrameWnd::OnCreate(lpCreateStruct) == -1)
		return -1;

	this->MoveWindow(0, 0, 1000, 700);
	this->CenterWindow();

	if (!m_wndStatusBar.Create(this))
	{
		TRACE0("δ�ܴ���״̬��\n");
		return -1;
	}
	m_wndStatusBar.SetIndicators(indicators, sizeof(indicators) / sizeof(UINT));
	return 0;
}

BOOL CMainFrame::OnCreateClient(LPCREATESTRUCT /*lpcs*/,
	CCreateContext* pContext)
{
	CRect rect;
	this->GetWindowRect(&rect);

	if (!m_wndOuterSplitter.CreateStatic(this, 1, 2))
		return FALSE;

	if (!m_wndInnerSplitter.CreateStatic(&m_wndOuterSplitter, 1, 2, 
		WS_CHILD | WS_VISIBLE | WS_BORDER,
		m_wndOuterSplitter.IdFromRowCol(0, 1)))
	{
		m_wndOuterSplitter.DestroyWindow();
		return FALSE;
	}

	CSize tocSize(180, rect.Height() / 2);
	CSize editorSize((rect.Width() - 180) / 2, rect.Height() / 2);
	CSize previewSize((rect.Width() - 180) / 2, rect.Height() / 2);

	if (!m_wndOuterSplitter.CreateView(0, 0, RUNTIME_CLASS(CTocView), tocSize, pContext))
	{
		m_wndOuterSplitter.DestroyWindow();
		m_wndInnerSplitter.DestroyWindow();
		return FALSE;
	}

	if (!m_wndInnerSplitter.CreateView(0, 0, RUNTIME_CLASS(CLeftView), editorSize, pContext) ||
		!m_wndInnerSplitter.CreateView(0, 1, RUNTIME_CLASS(CMarkdownEditorView), previewSize, pContext))
	{
		m_wndOuterSplitter.DestroyWindow();
		m_wndInnerSplitter.DestroyWindow();
		return FALSE;
	}

	_bInited = true;
	return TRUE;
}

BOOL CMainFrame::PreCreateWindow(CREATESTRUCT& cs)
{
	if (!CFrameWnd::PreCreateWindow(cs))
		return FALSE;
	return TRUE;
}

// CMainFrame ���

#ifdef _DEBUG
void CMainFrame::AssertValid() const
{
	CFrameWnd::AssertValid();
}

void CMainFrame::Dump(CDumpContext& dc) const
{
	CFrameWnd::Dump(dc);
}
#endif

// CMainFrame ��Ϣ��������

void CMainFrame::OnSize(UINT nType, int cx, int cy)
{
	CFrameWnd::OnSize(nType, cx, cy);
	if (!_bInited)
		return;
	if (cx == 0 || cy == 0)
		return;

	CRect rect;
	GetWindowRect(&rect);

	if (_bShowToc)
	{
		int tocWidth = 180;
		int remaining = cx - tocWidth;

		m_wndOuterSplitter.SetColumnInfo(0, tocWidth, 50);
		m_wndOuterSplitter.RecalcLayout();

		if (_bShowLeft && remaining > 0)
		{
			m_wndInnerSplitter.SetColumnInfo(0, remaining / 2, 10);
			m_wndInnerSplitter.RecalcLayout();
		}
	}
	else
	{
		if (_bShowLeft)
		{
			m_wndInnerSplitter.SetColumnInfo(0, cx / 2, 10);
			m_wndInnerSplitter.RecalcLayout();
		}
	}

	static bool sFirst = true;
	if (sFirst) {
		sFirst = false;
		bool showViewer = isViewer();
		bool showToc = isTocVisible();
		if (showViewer)
			switchViewer(showViewer);
		if (!showToc)
			switchToc(showToc);
	}
}

void CMainFrame::OnSwitch() {
	_bShowLeft = !_bShowLeft;
	switchViewer(!_bShowLeft);
	saveViewer(!_bShowLeft);
}

void CMainFrame::switchViewer(bool viewer) {
	m_wndInnerSplitter.ShowLeft(!viewer);
}

void CMainFrame::OnToggleToc() {
	_bShowToc = !_bShowToc;
	switchToc(_bShowToc);
	saveTocVisible(_bShowToc);
}

void CMainFrame::switchToc(bool show) {
	CWnd* pToc = m_wndOuterSplitter.GetPane(0, 0);
	if (pToc == NULL)
		return;

	CRect rect;
	GetWindowRect(&rect);

	if (show)
	{
		m_wndOuterSplitter.SetColumnInfo(0, 180, 50);
		pToc->ShowWindow(SW_SHOW);
	}
	else
	{
		m_wndOuterSplitter.SetColumnInfo(0, 0, 0);
		pToc->ShowWindow(SW_HIDE);
	}

	m_wndOuterSplitter.RecalcLayout();
}

CMarkdownEditorView* CMainFrame::GetRightPane()
{
	return dynamic_cast<CMarkdownEditorView*>(m_wndInnerSplitter.GetPane(0, 1));
}

CMarkdownEditorView* CMainFrame::GetPreviewView()
{
	return dynamic_cast<CMarkdownEditorView*>(m_wndInnerSplitter.GetPane(0, 1));
}

CLeftView* CMainFrame::GetEditorView()
{
	return dynamic_cast<CLeftView*>(m_wndInnerSplitter.GetPane(0, 0));
}

CTocView* CMainFrame::GetTocView()
{
	return dynamic_cast<CTocView*>(m_wndOuterSplitter.GetPane(0, 0));
}

const string STR_ABOUT = "# MarkdownEditor 1.3\nProject: <https://github.com/jijinggang/MarkdownEditor>\n## Features\n- Native sidebar with H1-H6 headings tree view\n- KaTeX math formula support: $E=mc^2$\n- Full Unicode/UTF-8 support\n\n## Math Examples\n- Inline: $x = \\frac{-b \\pm \\sqrt{b^2-4ac}}{2a}$\n- Block: $$\\sum_{i=1}^{n} i = \\frac{n(n+1)}{2}$$\n\n## Author\njijinggang@gmail.com\n## Copyright\nFree For All";

void CMainFrame::OnAbout()
{
	static bool s_bShowAbout = false;
	CMarkdownEditorView* pView = dynamic_cast<CMarkdownEditorView*>(m_wndInnerSplitter.GetPane(0, 1));
	if (pView == NULL)
		return;
	if (!s_bShowAbout)
		pView->UpdateMd(STR_ABOUT);
	else {
		CLeftView* pLeft = dynamic_cast<CLeftView*>(m_wndInnerSplitter.GetPane(0, 0));
		pView->GetDocument()->UpdateAllViews(pLeft, LPARAM_Update);
	}
	s_bShowAbout = !s_bShowAbout;
}
