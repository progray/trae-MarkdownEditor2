
#pragma once
#include "MySplitterWnd.h"
class CMarkdownEditorView;

enum StatusPaneIndex
{
	PANE_INFO = 0,
	PANE_ENCODING,
	PANE_WORDCOUNT,
	PANE_CAPS,
	PANE_NUM,
	PANE_SCRL,
};

class CMainFrame : public CFrameWnd
{
private:
	bool _bInited;
	bool _bShowLeft;
	bool _bShowTree;
	CString m_strEncoding;
	double m_dWordCount;
	
protected:
	CMainFrame();
	DECLARE_DYNCREATE(CMainFrame)

public:
	CMySplitterWnd m_wndSplitter;
	CMySplitterWnd m_wndSplitterMain;

public:
	void UpdateStatusBar();
	void SetEncoding(const CString& strEncoding) { m_strEncoding = strEncoding; }
	void SetWordCount(double dCount) { m_dWordCount = dCount; }

public:
	virtual BOOL OnCreateClient(LPCREATESTRUCT lpcs, CCreateContext* pContext);
	virtual BOOL PreCreateWindow(CREATESTRUCT& cs);
	virtual BOOL PreTranslateMessage(MSG* pMsg);

public:
	virtual ~CMainFrame();
	CMarkdownEditorView* GetRightPane();
#ifdef _DEBUG
	virtual void AssertValid() const;
	virtual void Dump(CDumpContext& dc) const;
#endif

protected:
	CStatusBar        m_wndStatusBar;

protected:
	afx_msg int OnCreate(LPCREATESTRUCT lpCreateStruct);
	afx_msg void OnDropFiles(HDROP hDropInfo);
	afx_msg void OnExportHtml();
	DECLARE_MESSAGE_MAP()
private:
	void switchViewer(bool enable);
public:
	afx_msg void OnSize(UINT nType, int cx, int cy);
	afx_msg void OnSwitch();
	afx_msg void OnAbout();
};

