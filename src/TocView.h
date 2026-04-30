#pragma once
#include <vector>
using namespace std;

struct HeadingInfo {
	CString text;
	int level;
	int line;
	HTREEITEM hItem;

	HeadingInfo() : level(1), line(0), hItem(NULL) {}
	HeadingInfo(const CString& t, int lvl, int ln) : text(t), level(lvl), line(ln), hItem(NULL) {}
};

class CTocView : public CTreeView
{
private:
	CFont _font;
	vector<HeadingInfo> _headings;
	CImageList _imageList;

	void ClearTree();
	void ExtractHeadings(const string& text);
	void BuildTree();
	void SyncEditor(int line);
	void SyncPreview(int line);

protected:
	CTocView();
	DECLARE_DYNCREATE(CTocView)

public:
	virtual ~CTocView();
	CMarkdownEditorDoc* GetDocument();

	virtual BOOL PreCreateWindow(CREATESTRUCT& cs);
	virtual void OnInitialUpdate();
	virtual void OnUpdate(CView* pSender, LPARAM lHint, CObject* pHint);

#ifdef _DEBUG
	virtual void AssertValid() const;
	virtual void Dump(CDumpContext& dc) const;
#endif

protected:
	DECLARE_MESSAGE_MAP()
public:
	afx_msg void OnTvnSelchanged(NMHDR* pNMHDR, LRESULT* pResult);
	afx_msg int OnCreate(LPCREATESTRUCT lpCreateStruct);
};

#ifndef _DEBUG
inline CMarkdownEditorDoc* CTocView::GetDocument()
{ return reinterpret_cast<CMarkdownEditorDoc*>(m_pDocument); }
#endif
