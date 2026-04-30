
#pragma once

class CFileTreeView : public CTreeView
{
protected:
	CFileTreeView();
	DECLARE_DYNCREATE(CFileTreeView)

public:
	virtual ~CFileTreeView();
	void RefreshTree();
	void SetRootPath(const CString& strPath);

protected:
	CString m_strRootPath;
	CImageList m_imageList;
	
	void FillDirectory(HTREEITEM hParent, const CString& strDir);
	HTREEITEM AddItem(HTREEITEM hParent, const CString& strText, int nImage, BOOL bHasChildren);
	virtual void OnInitialUpdate();
	afx_msg void OnTvnSelchanged(NMHDR* pNMHDR, LRESULT* pResult);
	afx_msg void OnNMDblclk(NMHDR* pNMHDR, LRESULT* pResult);
	afx_msg void OnContextMenu(CWnd* pWnd, CPoint point);
	afx_msg void OnRefresh();
	DECLARE_MESSAGE_MAP()
};
