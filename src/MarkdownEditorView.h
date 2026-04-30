
// MarkdownEditorView.h : CMarkdownEditorView ��Ľӿ�
//

#pragma once
#include <string>
using namespace std;

class CMarkdownEditorView : public CHtmlView
{
private:
	bool _bFirstNavigate;
	string _strCSS;
	void initCSS();
public:
	void UpdateMd(const string& strMd);
	string GetMdHtml(const string& str);

private:
	void NavigateHTML(const string& szHtml);
protected: // �������л�����
	CMarkdownEditorView();
	DECLARE_DYNCREATE(CMarkdownEditorView)

// ����
public:
	CMarkdownEditorDoc* GetDocument() const;

// ����
public:

// ��д
public:
	virtual BOOL PreCreateWindow(CREATESTRUCT& cs);
protected:
	virtual void OnInitialUpdate(); // ������һ�ε���

// ʵ��
public:
	virtual ~CMarkdownEditorView();
#ifdef _DEBUG
	virtual void AssertValid() const;
	virtual void Dump(CDumpContext& dc) const;
#endif

protected:

// ���ɵ���Ϣӳ�亯��
protected:
	DECLARE_MESSAGE_MAP()
	virtual void OnUpdate(CView* /*pSender*/, LPARAM /*lHint*/, CObject* /*pHint*/);
public:
//	virtual void OnNavigateComplete2(LPCTSTR strURL);
};

#ifndef _DEBUG  // MarkdownEditorView.cpp �еĵ��԰汾
inline CMarkdownEditorDoc* CMarkdownEditorView::GetDocument() const
   { return reinterpret_cast<CMarkdownEditorDoc*>(m_pDocument); }
#endif

