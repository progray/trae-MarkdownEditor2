#include "stdafx.h"
#include "MarkdownEditor.h"
#include "FileTreeView.h"
#include "MarkdownEditorDoc.h"
#include "MainFrm.h"
#include "Util.h"
#include <vector>
#include <algorithm>

#ifdef _DEBUG
#define new DEBUG_NEW
#endif

IMPLEMENT_DYNCREATE(CFileTreeView, CTreeView)

BEGIN_MESSAGE_MAP(CFileTreeView, CTreeView)
	ON_NOTIFY_REFLECT(TVN_SELCHANGED, &CFileTreeView::OnTvnSelchanged)
	ON_NOTIFY_REFLECT(NM_DBLCLK, &CFileTreeView::OnNMDblclk)
	ON_WM_CONTEXTMENU()
	ON_COMMAND(ID_REFRESH, &CFileTreeView::OnRefresh)
END_MESSAGE_MAP()

extern bool g_isOpenFile;

CFileTreeView::CFileTreeView()
{
}

CFileTreeView::~CFileTreeView()
{
	CTreeCtrl& treeCtrl = GetTreeCtrl();
	HTREEITEM hItem = treeCtrl.GetRootItem();
	while (hItem)
	{
		DWORD_PTR dwData = treeCtrl.GetItemData(hItem);
		if (dwData != 0)
		{
			CString* pStr = (CString*)dwData;
			delete pStr;
		}
		hItem = treeCtrl.GetNextItem(hItem, TVGN_NEXTVISIBLE);
	}
}

void CFileTreeView::OnInitialUpdate()
{
	CTreeView::OnInitialUpdate();
	
	CTreeCtrl& treeCtrl = GetTreeCtrl();
	treeCtrl.ModifyStyle(0, TVS_HASLINES | TVS_LINESATROOT | TVS_HASBUTTONS | TVS_SHOWSELALWAYS);
	
	m_imageList.Create(16, 16, ILC_COLOR32 | ILC_MASK, 2, 1);
	
	SHFILEINFO sfi;
	::SHGetFileInfo(_T(""), 0, &sfi, sizeof(sfi), SHGFI_ICON | SHGFI_SMALLICON);
	m_imageList.Add(sfi.hIcon);
	
	::SHGetFileInfo(_T(".md"), FILE_ATTRIBUTE_NORMAL, &sfi, sizeof(sfi), 
		SHGFI_ICON | SHGFI_SMALLICON | SHGFI_USEFILEATTRIBUTES);
	m_imageList.Add(sfi.hIcon);
	
	treeCtrl.SetImageList(&m_imageList, TVSIL_NORMAL);
	
	SetRootPath(Util::GetExePath().c_str());
	RefreshTree();
}

void CFileTreeView::SetRootPath(const CString& strPath)
{
	m_strRootPath = strPath;
	if (m_strRootPath.Right(1) != _T("\\") && m_strRootPath.Right(1) != _T("/"))
	{
		m_strRootPath += _T("\\");
	}
}

void CFileTreeView::RefreshTree()
{
	CTreeCtrl& treeCtrl = GetTreeCtrl();
	
	HTREEITEM hItem = treeCtrl.GetRootItem();
	while (hItem)
	{
		DWORD_PTR dwData = treeCtrl.GetItemData(hItem);
		if (dwData != 0)
		{
			CString* pStr = (CString*)dwData;
			delete pStr;
		}
		hItem = treeCtrl.GetNextItem(hItem, TVGN_NEXTVISIBLE);
	}
	
	treeCtrl.DeleteAllItems();
	
	if (m_strRootPath.IsEmpty())
		return;
	
	HTREEITEM hRoot = AddItem(TVI_ROOT, m_strRootPath, 0, TRUE);
	FillDirectory(hRoot, m_strRootPath);
	treeCtrl.Expand(hRoot, TVE_EXPAND);
}

void CFileTreeView::FillDirectory(HTREEITEM hParent, const CString& strDir)
{
	CFileFind finder;
	CString strFind = strDir + _T("*.*");
	BOOL bWorking = finder.FindFile(strFind);
	
	std::vector<CString> dirs;
	std::vector<CString> files;
	
	while (bWorking)
	{
		bWorking = finder.FindNextFile();
		if (finder.IsDots())
			continue;
		
		CString strPath = finder.GetFilePath();
		if (finder.IsDirectory())
		{
			dirs.push_back(strPath);
		}
		else
		{
			CString strName = finder.GetFileName();
			CString strExt = strName.Right(3).MakeLower();
			if (strExt == _T(".md"))
			{
				files.push_back(strPath);
			}
		}
	}
	finder.Close();
	
	std::sort(dirs.begin(), dirs.end());
	std::sort(files.begin(), files.end());
	
	for (size_t i = 0; i < dirs.size(); i++)
	{
		CString strName = ::PathFindFileName(dirs[i]);
		HTREEITEM hItem = AddItem(hParent, strName, 0, TRUE);
		FillDirectory(hItem, dirs[i] + _T("\\"));
	}
	
	for (size_t i = 0; i < files.size(); i++)
	{
		CString strName = ::PathFindFileName(files[i]);
		HTREEITEM hItem = AddItem(hParent, strName, 1, FALSE);
		CString* pFilePath = new CString(files[i]);
		GetTreeCtrl().SetItemData(hItem, (DWORD_PTR)pFilePath);
	}
}

HTREEITEM CFileTreeView::AddItem(HTREEITEM hParent, const CString& strText, int nImage, BOOL bHasChildren)
{
	CTreeCtrl& treeCtrl = GetTreeCtrl();
	TVINSERTSTRUCT tvis;
	tvis.hParent = hParent;
	tvis.hInsertAfter = TVI_LAST;
	tvis.item.mask = TVIF_TEXT | TVIF_IMAGE | TVIF_SELECTEDIMAGE | TVIF_CHILDREN;
	tvis.item.pszText = (LPTSTR)(LPCTSTR)strText;
	tvis.item.iImage = nImage;
	tvis.item.iSelectedImage = nImage;
	tvis.item.cChildren = bHasChildren ? 1 : 0;
	
	return treeCtrl.InsertItem(&tvis);
}

void CFileTreeView::OnTvnSelchanged(NMHDR* pNMHDR, LRESULT* pResult)
{
	LPNMTREEVIEW pNMTreeView = reinterpret_cast<LPNMTREEVIEW>(pNMHDR);
	*pResult = 0;
}

void CFileTreeView::OnNMDblclk(NMHDR* pNMHDR, LRESULT* pResult)
{
	LPNMITEMACTIVATE pNMItemActivate = reinterpret_cast<LPNMITEMACTIVATE>(pNMHDR);
	*pResult = 0;
	
	CTreeCtrl& treeCtrl = GetTreeCtrl();
	HTREEITEM hItem = treeCtrl.GetSelectedItem();
	if (!hItem)
		return;
	
	DWORD_PTR dwData = treeCtrl.GetItemData(hItem);
	if (dwData == 0)
	{
		UINT uState = treeCtrl.GetItemState(hItem, TVIS_EXPANDED);
		if (uState & TVIS_EXPANDED)
			treeCtrl.Expand(hItem, TVE_COLLAPSE);
		else
			treeCtrl.Expand(hItem, TVE_EXPAND);
		return;
	}
	
	CString* pStrPath = (CString*)dwData;
	CString strFilePath = *pStrPath;
	
	CMainFrame* pMainFrame = (CMainFrame*)AfxGetMainWnd();
	if (!pMainFrame)
		return;
	
	CMarkdownEditorDoc* pDoc = (CMarkdownEditorDoc*)pMainFrame->GetActiveDocument();
	if (pDoc && pDoc->IsModified())
	{
		CString strMsg;
		strMsg.Format(_T("Current file modified, save?"));
		int nRet = AfxMessageBox(strMsg, MB_YESNOCANCEL | MB_ICONQUESTION);
		if (nRet == IDCANCEL)
			return;
		if (nRet == IDYES)
		{
			if (!pDoc->OnSaveDocument(pDoc->GetPathName()))
			{
				AfxMessageBox(_T("Failed to save file!"), MB_OK | MB_ICONERROR);
				return;
			}
		}
	}
	
	CWaitCursor wait;
	if (AfxGetApp()->OpenDocumentFile(strFilePath))
	{
		g_isOpenFile = true;
	}
}

void CFileTreeView::OnContextMenu(CWnd* pWnd, CPoint point)
{
	CMenu menu;
	menu.LoadMenu(IDR_TREEVIEW_POPUP);
	CMenu* pPopup = menu.GetSubMenu(0);
	if (pPopup)
	{
		pPopup->TrackPopupMenu(TPM_LEFTALIGN | TPM_RIGHTBUTTON, point.x, point.y, this);
	}
}

void CFileTreeView::OnRefresh()
{
	RefreshTree();
}
