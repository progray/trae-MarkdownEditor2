
// MarkdownEditorDoc.cpp : CMarkdownEditorDoc ���ʵ��
//

#include "stdafx.h"
#include "./Util.h"
#include <memory>
// SHARED_HANDLERS ������ʵ��Ԥ��������ͼ������ɸѡ�������
// ATL ��Ŀ�н��ж��壬�����������Ŀ�����ĵ����롣
#ifndef SHARED_HANDLERS
#include "MarkdownEditor.h"
#endif

#include "MarkdownEditorDoc.h"
#include "MainFrm.h"

#include <propkey.h>

#ifdef _DEBUG
#define new DEBUG_NEW
#endif

// CMarkdownEditorDoc

IMPLEMENT_DYNCREATE(CMarkdownEditorDoc, CDocument)

BEGIN_MESSAGE_MAP(CMarkdownEditorDoc, CDocument)

END_MESSAGE_MAP()


// CMarkdownEditorDoc ����/����

CMarkdownEditorDoc::CMarkdownEditorDoc()
{
	// TODO: �ڴ�����һ���Թ������
	resetData();
}

CMarkdownEditorDoc::~CMarkdownEditorDoc()
{
}

BOOL CMarkdownEditorDoc::OnNewDocument()
{
	if (!CDocument::OnNewDocument())
		return FALSE;
	resetData();
	this->UpdateAllViews(NULL, LPARAM_Update);
	// TODO: �ڴ��������³�ʼ������
	// (SDI �ĵ������ø��ĵ�)
	return TRUE;
}




// CMarkdownEditorDoc ���л�

void CMarkdownEditorDoc::Serialize(CArchive& ar)
{
	string strFile = ar.GetFile()->GetFilePath();
	_strPath = Util::GetFilePath(strFile,true);

	if (ar.IsStoring())
	{
		ar.WriteString(Util::ANSIToUTF8(_strText.c_str()).c_str());
	}
	else
	{
		CFile* pFile = ar.GetFile();
		ULONGLONG dwLen = pFile->GetLength();
		if (dwLen > 0)
		{
			char* pBuf = new char[(size_t)dwLen + 2];
			pBuf[dwLen] = 0;
			pBuf[dwLen + 1] = 0;
			pFile->Read(pBuf, (UINT)dwLen);
			
			_bIsUTF8 = (Util::IsTextUTF8(pBuf, (long)dwLen) != 0);
			
			_strText = Util::AnyToANSI(pBuf, (int)dwLen);
			Util::ReplaceAllStr(_strText,"\r\n", "\n");
			Util::ReplaceAllStr(_strText,"\n", "\r\n");
			
			delete[] pBuf;
		}
		else
		{
			_bIsUTF8 = true;
			_strText = "";
		}
		
		CMainFrame* pMainFrame = (CMainFrame*)AfxGetMainWnd();
		if (pMainFrame)
		{
			pMainFrame->SetEncoding(_bIsUTF8 ? _T("UTF-8") : _T("ANSI"));
			pMainFrame->UpdateStatusBar();
		}
		
		this->UpdateAllViews(NULL,LPARAM_Update);
	}
}

#ifdef SHARED_HANDLERS

// ����ͼ��֧��
void CMarkdownEditorDoc::OnDrawThumbnail(CDC& dc, LPRECT lprcBounds)
{
	// �޸Ĵ˴����Ի����ĵ�����
	dc.FillSolidRect(lprcBounds, RGB(255, 255, 255));

	CString strText = _T("TODO: implement thumbnail drawing here");
	LOGFONT lf;

	CFont* pDefaultGUIFont = CFont::FromHandle((HFONT) GetStockObject(DEFAULT_GUI_FONT));
	pDefaultGUIFont->GetLogFont(&lf);
	lf.lfHeight = 36;

	CFont fontDraw;
	fontDraw.CreateFontIndirect(&lf);

	CFont* pOldFont = dc.SelectObject(&fontDraw);
	dc.DrawText(strText, lprcBounds, DT_CENTER | DT_WORDBREAK);
	dc.SelectObject(pOldFont);
}

// �������������֧��
void CMarkdownEditorDoc::InitializeSearchContent()
{
	CString strSearchContent;
	// ���ĵ����������������ݡ�
	// ���ݲ���Ӧ�ɡ�;���ָ�

	// ����:  strSearchContent = _T("point;rectangle;circle;ole object;")��
	SetSearchContent(strSearchContent);
}

void CMarkdownEditorDoc::SetSearchContent(const CString& value)
{
	if (value.IsEmpty())
	{
		RemoveChunk(PKEY_Search_Contents.fmtid, PKEY_Search_Contents.pid);
	}
	else
	{
		CMFCFilterChunkValueImpl *pChunk = NULL;
		ATLTRY(pChunk = new CMFCFilterChunkValueImpl);
		if (pChunk != NULL)
		{
			pChunk->SetTextValue(PKEY_Search_Contents, value, CHUNK_TEXT);
			SetChunkValue(pChunk);
		}
	}
}

#endif // SHARED_HANDLERS

// CMarkdownEditorDoc ���

#ifdef _DEBUG
void CMarkdownEditorDoc::AssertValid() const
{
	CDocument::AssertValid();
}

void CMarkdownEditorDoc::Dump(CDumpContext& dc) const
{
	CDocument::Dump(dc);
}
#endif //_DEBUG
const CString PREFIX_MODIFIED = "* ";
void setModified(CMarkdownEditorDoc*pDoc, bool modified) {
	pDoc->SetModifiedFlag(modified);
	//string path = pDoc->GetTitle();
	CString title = pDoc->GetTitle();
	if (modified) {
		CString strTitle;
		AfxGetMainWnd()->GetWindowText(strTitle);
		if (modified && strTitle.Find(PREFIX_MODIFIED) !=0)
			AfxGetMainWnd()->SetWindowText("* " + strTitle);
	}
}
// CMarkdownEditorDoc ����
//�����ı�����
void CMarkdownEditorDoc::UpdateText(const string& text, CView* pSender, bool bMoveToEnd){
	_strText = text;
	int lParam = LPARAM_Update;
	if(bMoveToEnd)
		lParam |= LPARAM_MoveEnd;
	this->UpdateAllViews(pSender, lParam);
	setModified(this, true);
	//this->SetModifiedFlag();
}

void CMarkdownEditorDoc::resetData(void)
{
	_strText = "";
	_strPath = "";
	_bIsUTF8 = true;
}
