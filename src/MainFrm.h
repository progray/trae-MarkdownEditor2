
// MainFrm.h : CMainFrame ��Ľӿ�
//

#pragma once
#include "MySplitterWnd.h"
class CMarkdownEditorView;
class CLeftView;
class CTocView;

class CMainFrame : public CFrameWnd
{
private:
	bool _bInited;
	bool _bShowLeft;
	bool _bShowToc;
	
protected: // �����л�����
	CMainFrame();
	DECLARE_DYNCREATE(CMainFrame)

// ����
protected:
	CMySplitterWnd m_wndOuterSplitter;
	CMySplitterWnd m_wndInnerSplitter;
public:

// ����
public:

// ��д
public:
	virtual BOOL OnCreateClient(LPCREATESTRUCT lpcs, CCreateContext* pContext);
	virtual BOOL PreCreateWindow(CREATESTRUCT& cs);

// ʵ��
public:
	virtual ~CMainFrame();
	CMarkdownEditorView* GetRightPane();
	CMarkdownEditorView* GetPreviewView();
	CLeftView* GetEditorView();
	CTocView* GetTocView();
#ifdef _DEBUG
	virtual void AssertValid() const;
	virtual void Dump(CDumpContext& dc) const;
#endif

protected:  // �ؼ���Ƕ���Ա
	CStatusBar        m_wndStatusBar;

// ���ɵ���Ϣӳ�亯��
protected:
	afx_msg int OnCreate(LPCREATESTRUCT lpCreateStruct);
	DECLARE_MESSAGE_MAP()
private:
	void switchViewer(bool enable);
	void switchToc(bool enable);
public:
	afx_msg void OnSize(UINT nType, int cx, int cy);
	afx_msg void OnSwitch();
	afx_msg void OnAbout();
	afx_msg void OnToggleToc();
};


