#include "stdafx.h"
#include "MarkdownEditor.h"

#include "MarkdownEditorDoc.h"
#include "TocView.h"
#include "LeftView.h"
#include "MarkdownEditorView.h"
#include "MainFrm.h"

#ifdef _DEBUG
#define new DEBUG_NEW
#endif

IMPLEMENT_DYNCREATE(CTocView, CTreeView)

BEGIN_MESSAGE_MAP(CTocView, CTreeView)
	ON_WM_CREATE()
	ON_NOTIFY_REFLECT(TVN_SELCHANGED, &CTocView::OnTvnSelchanged)
END_MESSAGE_MAP()

CTocView::CTocView()
{
}

CTocView::~CTocView()
{
}

BOOL CTocView::PreCreateWindow(CREATESTRUCT& cs)
{
	BOOL bCreate = CTreeView::PreCreateWindow(cs);
	cs.style |= TVS_HASLINES | TVS_HASBUTTONS | TVS_LINESATROOT;
	return bCreate;
}

void CTocView::OnInitialUpdate()
{
	CTreeView::OnInitialUpdate();
}

int CTocView::OnCreate(LPCREATESTRUCT lpCreateStruct)
{
	if (CTreeView::OnCreate(lpCreateStruct) == -1)
		return -1;

	_font.CreatePointFont(100, (LPCTSTR)"Microsoft YaHei UI");
	SetFont(&_font);

	_imageList.Create(16, 16, ILC_COLOR32 | ILC_MASK, 3, 1);
	HICON hIcon1 = (HICON)::LoadImage(NULL, IDI_INFORMATION, IMAGE_ICON, 16, 16, LR_SHARED);
	_imageList.Add(hIcon1);
	GetTreeCtrl().SetImageList(&_imageList, TVSIL_NORMAL);

	return 0;
}

void CTocView::OnUpdate(CView* pSender, LPARAM lHint, CObject* /*pHint*/)
{
	if (pSender == this || !(lHint & LPARAM_Update))
		return;

	const string& str = GetDocument()->getText();
	ClearTree();
	ExtractHeadings(str);
	BuildTree();
}

void CTocView::ClearTree()
{
	GetTreeCtrl().DeleteAllItems();
	_headings.clear();
}

void CTocView::ExtractHeadings(const string& text)
{
	if (text.empty())
		return;

	CString strText = text.c_str();
	int nLine = 0;
	int nPos = 0;
	int nLength = strText.GetLength();

	while (nPos < nLength)
	{
		nLine++;

		int nEnd = strText.Find(_T("\n"), nPos);
		if (nEnd == -1)
			nEnd = nLength;

		CString strLine = strText.Mid(nPos, nEnd - nPos);
		strLine.TrimRight(_T("\r"));
		strLine.Trim();

		if (strLine.IsEmpty())
		{
			nPos = nEnd + 1;
			continue;
		}

		int nLevel = 0;
		while (nLevel < (int)strLine.GetLength() && strLine[nLevel] == _T('#'))
		{
			nLevel++;
		}

		if (nLevel >= 1 && nLevel <= 6)
		{
			CString strHeading = strLine.Mid(nLevel);
			strHeading.Trim();

			if (!strHeading.IsEmpty())
			{
				_headings.push_back(HeadingInfo(strHeading, nLevel, nLine));
			}
		}

		nPos = nEnd + 1;
	}
}

void CTocView::BuildTree()
{
	CTreeCtrl& treeCtrl = GetTreeCtrl();
	if (_headings.empty())
		return;

	vector<HTREEITEM> levelItems(7, NULL);

	for (size_t i = 0; i < _headings.size(); i++)
	{
		HeadingInfo& heading = _headings[i];
		HTREEITEM hParent = NULL;

		for (int l = heading.level - 1; l >= 1; l--)
		{
			if (levelItems[l] != NULL)
			{
				hParent = levelItems[l];
				break;
			}
		}

		TVITEM tvi;
		ZeroMemory(&tvi, sizeof(tvi));
		tvi.mask = TVIF_TEXT | TVIF_PARAM | TVIF_IMAGE | TVIF_SELECTEDIMAGE;
		tvi.pszText = (LPTSTR)(LPCTSTR)heading.text;
		tvi.lParam = (LPARAM)i;
		tvi.iImage = 0;
		tvi.iSelectedImage = 0;

		TVINSERTSTRUCT tvis;
		ZeroMemory(&tvis, sizeof(tvis));
		tvis.hParent = hParent;
		tvis.hInsertAfter = TVI_LAST;
		tvis.item = tvi;

		HTREEITEM hItem = treeCtrl.InsertItem(&tvis);
		heading.hItem = hItem;
		levelItems[heading.level] = hItem;

		for (int l = heading.level + 1; l <= 6; l++)
		{
			levelItems[l] = NULL;
		}
	}

	for (int l = 1; l <= 6; l++)
	{
		if (levelItems[l] != NULL)
		{
			treeCtrl.Expand(levelItems[l], TVE_EXPAND);
		}
	}
}

void CTocView::OnTvnSelchanged(NMHDR* pNMHDR, LRESULT* pResult)
{
	LPNMTREEVIEW pNMTreeView = reinterpret_cast<LPNMTREEVIEW>(pNMHDR);
	*pResult = 0;

	HTREEITEM hItem = pNMTreeView->itemNew.hItem;
	if (hItem == NULL)
		return;

	TVITEM tvi;
	ZeroMemory(&tvi, sizeof(tvi));
	tvi.mask = TVIF_PARAM;
	tvi.hItem = hItem;

	if (!GetTreeCtrl().GetItem(&tvi))
		return;

	int nIndex = (int)tvi.lParam;
	if (nIndex < 0 || nIndex >= (int)_headings.size())
		return;

	int nLine = _headings[nIndex].line;
	SyncEditor(nLine);
	SyncPreview(nLine);
}

void CTocView::SyncEditor(int line)
{
	CMainFrame* pMainFrame = dynamic_cast<CMainFrame*>(AfxGetMainWnd());
	if (pMainFrame == NULL)
		return;

	CLeftView* pLeftView = pMainFrame->GetEditorView();
	if (pLeftView == NULL)
		return;

	CRichEditCtrl& editCtrl = pLeftView->GetRichEditCtrl();
	editCtrl.LineScroll(-editCtrl.GetLineCount());

	int nCharIndex = editCtrl.LineIndex(line - 1);
	if (nCharIndex < 0)
		return;

	editCtrl.SetSel(nCharIndex, nCharIndex);
	editCtrl.SetFocus();
}

void CTocView::SyncPreview(int line)
{
	CMainFrame* pMainFrame = dynamic_cast<CMainFrame*>(AfxGetMainWnd());
	if (pMainFrame == NULL)
		return;

	CMarkdownEditorView* pPreview = pMainFrame->GetPreviewView();
	if (pPreview == NULL)
		return;

	const string& text = GetDocument()->getText();
	CString strText = text.c_str();
	int totalLines = 0;
	int pos = 0;
	while ((pos = strText.Find(_T('\n'), pos)) != -1)
	{
		totalLines++;
		pos++;
	}
	totalLines++;

	float scrollPercent = 0.0f;
	if (totalLines > 1)
	{
		scrollPercent = (float)(line - 1) / (float)(totalLines - 1);
	}

	IDispatch* pDisp = pPreview->GetHtmlDocument();
	if (pDisp == NULL)
		return;

	CComPtr<IHTMLDocument2> pDocument2 = NULL;
	if (SUCCEEDED(pDisp->QueryInterface(IID_IHTMLDocument2, (LPVOID*)&pDocument2)))
	{
		CComPtr<IHTMLElement> pElement = NULL;
		if (SUCCEEDED(pDocument2->get_body(&pElement)))
		{
			CComPtr<IHTMLTextContainer> pTextContainer = NULL;
			if (SUCCEEDED(pElement->QueryInterface(IID_IHTMLTextContainer, (LPVOID*)&pTextContainer)))
			{
				long height;
				pTextContainer->get_scrollHeight(&height);
				pTextContainer->put_scrollTop((long)(scrollPercent * height));
			}
		}
	}
}

#ifdef _DEBUG
void CTocView::AssertValid() const
{
	CTreeView::AssertValid();
}

void CTocView::Dump(CDumpContext& dc) const
{
	CTreeView::Dump(dc);
}

CMarkdownEditorDoc* CTocView::GetDocument()
{
	ASSERT(m_pDocument->IsKindOf(RUNTIME_CLASS(CMarkdownEditorDoc)));
	return (CMarkdownEditorDoc*)m_pDocument;
}
#endif
