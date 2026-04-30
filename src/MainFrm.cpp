
// MainFrm.cpp : CMainFrame class implementation
//

#include "stdafx.h"
#include "MarkdownEditor.h"

#include "MainFrm.h"
#include "LeftView.h"
#include "FileTreeView.h"
#include "MarkdownEditorView.h"
#include "MarkdownEditorDoc.h"
#include "Util.h"

#ifdef _DEBUG
#define new DEBUG_NEW
#endif

#define IS_VIEWER_KEY  "isViewer"
bool s_bOpeningFile = false;

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

IMPLEMENT_DYNCREATE(CMainFrame, CFrameWnd)

BEGIN_MESSAGE_MAP(CMainFrame, CFrameWnd)
	ON_WM_CREATE()
	ON_WM_SIZE()
	ON_WM_DROPFILES()
	ON_COMMAND(IDM_SWITCH, &CMainFrame::OnSwitch)
	ON_COMMAND(IDM_ABOUT, &CMainFrame::OnAbout)
	ON_COMMAND(IDM_EXPORT_HTML, &CMainFrame::OnExportHtml)
END_MESSAGE_MAP()

static UINT indicators[] =
{
	ID_SEPARATOR,
	ID_SEPARATOR,
	ID_SEPARATOR,
	ID_INDICATOR_CAPS,
	ID_INDICATOR_NUM,
	ID_INDICATOR_SCRL,
};

CMainFrame::CMainFrame()
{
	_bInited = false;
	_bShowLeft = !isViewer();
	_bShowTree = true;
	m_strEncoding = _T("UTF-8");
	m_dWordCount = 0;
}

CMainFrame::~CMainFrame()
{
}

int CMainFrame::OnCreate(LPCREATESTRUCT lpCreateStruct)
{
	if (CFrameWnd::OnCreate(lpCreateStruct) == -1)
		return -1;

	this->MoveWindow(0,0,1000,700);
	this->CenterWindow();

	if (!m_wndStatusBar.Create(this))
	{
		TRACE0("Failed to create status bar\n");
		return -1;
	}
	m_wndStatusBar.SetIndicators(indicators, sizeof(indicators)/sizeof(UINT));
	
	m_wndStatusBar.SetPaneInfo(PANE_INFO, ID_SEPARATOR, SBPS_NORMAL, 200);
	m_wndStatusBar.SetPaneInfo(PANE_ENCODING, ID_SEPARATOR, SBPS_NORMAL, 80);
	m_wndStatusBar.SetPaneInfo(PANE_WORDCOUNT, ID_SEPARATOR, SBPS_NORMAL, 100);
	
	DragAcceptFiles(TRUE);
	
	return 0;
}

BOOL CMainFrame::OnCreateClient(LPCREATESTRUCT /*lpcs*/,
	CCreateContext* pContext)
{
	if (!m_wndSplitterMain.CreateStatic(this, 1, 2))
		return FALSE;
	
	CSize sizeTree(200, 100);
	if (!m_wndSplitterMain.CreateView(0, 0, RUNTIME_CLASS(CFileTreeView), sizeTree, pContext))
	{
		m_wndSplitterMain.DestroyWindow();
		return FALSE;
	}
	
	if (!m_wndSplitter.CreateStatic(&m_wndSplitterMain, 1, 2, WS_CHILD | WS_VISIBLE, m_wndSplitterMain.IdFromRowCol(0, 1)))
	{
		m_wndSplitterMain.DestroyWindow();
		return FALSE;
	}

	CSize sizePane(200, 100);

	if (!m_wndSplitter.CreateView(0, 0, RUNTIME_CLASS(CLeftView), sizePane, pContext) ||
		!m_wndSplitter.CreateView(0, 1, RUNTIME_CLASS(CMarkdownEditorView), sizePane, pContext))
	{
		m_wndSplitter.DestroyWindow();
		m_wndSplitterMain.DestroyWindow();
		return FALSE;
	}

	_bInited = true;
	return TRUE;
}

BOOL CMainFrame::PreCreateWindow(CREATESTRUCT& cs)
{
	if( !CFrameWnd::PreCreateWindow(cs) )
		return FALSE;
	return TRUE;
}

#ifdef _DEBUG
void CMainFrame::AssertValid() const
{
	CFrameWnd::AssertValid();
}

void CMainFrame::Dump(CDumpContext& dc) const
{
	CFrameWnd::Dump(dc);
}
#endif //_DEBUG

void CMainFrame::UpdateStatusBar()
{
	if (!_bInited)
		return;
	if (!m_wndStatusBar.GetSafeHwnd())
		return;
	if (!::IsWindow(m_wndStatusBar.GetSafeHwnd()))
		return;
	
	CMarkdownEditorDoc* pDoc = (CMarkdownEditorDoc*)GetActiveDocument();
	CString strInfo;
	
	if (pDoc)
	{
		CString strTitle = pDoc->GetTitle();
		if (pDoc->IsModified())
		{
			strInfo = _T("* ") + strTitle;
		}
		else
		{
			strInfo = strTitle;
		}
		
		m_strEncoding = pDoc->IsUTF8() ? _T("UTF-8") : _T("ANSI");
		
		const string& strText = pDoc->getText();
		double dWordCount = 0;
		for (size_t i = 0; i < strText.length(); )
		{
			unsigned char ch = (unsigned char)strText[i];
			if (ch >= 128)
			{
				dWordCount += 1.0;
				if (ch >= 0xE0)
					i += 3;
				else if (ch >= 0xC0)
					i += 2;
				else
					i += 1;
			}
			else if (isalpha(ch))
			{
				while (i < strText.length() && isalpha((unsigned char)strText[i]))
				{
					i++;
				}
				dWordCount += 0.5;
			}
			else if (isdigit(ch))
			{
				while (i < strText.length() && isdigit((unsigned char)strText[i]))
				{
					i++;
				}
				dWordCount += 0.5;
			}
			else
			{
				i++;
			}
		}
		m_dWordCount = dWordCount;
	}
	else
	{
		strInfo = _T("Untitled");
		m_strEncoding = _T("UTF-8");
	}
	
	m_wndStatusBar.SetPaneText(PANE_INFO, strInfo);
	m_wndStatusBar.SetPaneText(PANE_ENCODING, m_strEncoding);
	
	CString strCount;
	strCount.Format(_T("Words: %.1f"), m_dWordCount);
	m_wndStatusBar.SetPaneText(PANE_WORDCOUNT, strCount);
}

BOOL CMainFrame::PreTranslateMessage(MSG* pMsg)
{
	if (pMsg->message == WM_KEYDOWN || pMsg->message == WM_LBUTTONUP)
	{
		UpdateStatusBar();
	}
	return CFrameWnd::PreTranslateMessage(pMsg);
}

void CMainFrame::OnSize(UINT nType, int cx, int cy)
{
	CFrameWnd::OnSize(nType, cx, cy);
	if(!_bInited)
		return;
	if(cx == 0 || cy == 0)
		return;
	
	if (m_wndSplitterMain.GetSafeHwnd() && m_wndSplitter.GetSafeHwnd())
	{
		int cxCur, cxMin;
		m_wndSplitterMain.GetColumnInfo(0, cxCur, cxMin);
		if(cxCur <= 0)
			cxCur = cx / 5;
		m_wndSplitterMain.SetColumnInfo(0, cx / 5, 50);
		m_wndSplitterMain.RecalcLayout();
		
		m_wndSplitter.GetColumnInfo(0, cxCur, cxMin);
		int cxRight = cx - (cx / 5);
		m_wndSplitter.SetColumnInfo(0, cxRight / 2, 10);
		m_wndSplitter.RecalcLayout();
	}

	static bool sFirst = true;
	if (sFirst) {
		sFirst = false;
		bool show = isViewer();
		if (show)
			switchViewer(show);
		UpdateStatusBar();
	}
}

void CMainFrame::OnSwitch(){
	_bShowLeft = !_bShowLeft;
	switchViewer(!_bShowLeft);
	saveViewer(!_bShowLeft);
}
void CMainFrame::switchViewer(bool viewer) {
	if (!m_wndSplitter.GetSafeHwnd())
		return;
	CWnd* pLeft = m_wndSplitter.GetPane(0, 0);
	if (!pLeft || !pLeft->GetSafeHwnd())
		return;
	m_wndSplitter.ShowLeft(!viewer);
}

const string STR_ABOUT = "# MarkdownEditor 1.2\nProject: <https://github.com/jijinggang/MarkdownEditor>\n## Author\njijinggang@gmail.com\n## Copyright\nFree For All";
void CMainFrame::OnAbout()
{
	static bool s_bShowAbout = false;
	CMarkdownEditorView* pView = dynamic_cast<CMarkdownEditorView*>(m_wndSplitter.GetPane(0,1));
	if(pView == NULL)
		return;
	if(!s_bShowAbout)
		pView->UpdateMd(STR_ABOUT);
	else{
		CLeftView* pLeft = dynamic_cast<CLeftView*>(m_wndSplitter.GetPane(0,0));
		pView->GetDocument()->UpdateAllViews(pLeft, LPARAM_Update);
	}
	s_bShowAbout = !s_bShowAbout;
}

CMarkdownEditorView* CMainFrame::GetRightPane()
{
	return dynamic_cast<CMarkdownEditorView*>(m_wndSplitter.GetPane(0, 1));
}

string ReadFileToBase64(const CString& strFilePath)
{
	CFile file;
	if (!file.Open(strFilePath, CFile::modeRead))
		return "";
	
	ULONGLONG dwSize = file.GetLength();
	if (dwSize == 0)
		return "";
	
	BYTE* pBuffer = new BYTE[(size_t)dwSize];
	file.Read(pBuffer, (UINT)dwSize);
	file.Close();
	
	static const char* base64_chars = 
		"ABCDEFGHIJKLMNOPQRSTUVWXYZ"
		"abcdefghijklmnopqrstuvwxyz"
		"0123456789+/";
	
	string strResult;
	int i = 0;
	BYTE char_array_3[3];
	BYTE char_array_4[4];
	
	ULONGLONG len = dwSize;
	BYTE* bytes_to_encode = pBuffer;
	
	while (len--)
	{
		char_array_3[i++] = *(bytes_to_encode++);
		if (i == 3)
		{
			char_array_4[0] = (char_array_3[0] & 0xfc) >> 2;
			char_array_4[1] = ((char_array_3[0] & 0x03) << 4) + ((char_array_3[1] & 0xf0) >> 4);
			char_array_4[2] = ((char_array_3[1] & 0x0f) << 2) + ((char_array_3[2] & 0xc0) >> 6);
			char_array_4[3] = char_array_3[2] & 0x3f;
			
			for (i = 0; i < 4; i++)
				strResult += base64_chars[char_array_4[i]];
			i = 0;
		}
	}
	
	if (i)
	{
		for (int j = i; j < 3; j++)
			char_array_3[j] = '\0';
		
		char_array_4[0] = (char_array_3[0] & 0xfc) >> 2;
		char_array_4[1] = ((char_array_3[0] & 0x03) << 4) + ((char_array_3[1] & 0xf0) >> 4);
		char_array_4[2] = ((char_array_3[1] & 0x0f) << 2) + ((char_array_3[2] & 0xc0) >> 6);
		char_array_4[3] = char_array_3[2] & 0x3f;
		
		for (int j = 0; j < i + 1; j++)
			strResult += base64_chars[char_array_4[j]];
		
		while (i++ < 3)
			strResult += '=';
	}
	
	delete[] pBuffer;
	return strResult;
}

CString GetImageMimeType(const CString& strExt)
{
	CString strLower = strExt;
	strLower.MakeLower();
	
	if (strLower == _T(".png"))
		return _T("image/png");
	if (strLower == _T(".jpg") || strLower == _T(".jpeg"))
		return _T("image/jpeg");
	if (strLower == _T(".gif"))
		return _T("image/gif");
	if (strLower == _T(".bmp"))
		return _T("image/bmp");
	if (strLower == _T(".webp"))
		return _T("image/webp");
	if (strLower == _T(".ico"))
		return _T("image/x-icon");
	
	return _T("image/png");
}

string ReplaceImagesWithBase64(const string& strHtml, const string& strBasePath)
{
	string strResult = strHtml;
	string::size_type pos = 0;
	
	while ((pos = strResult.find("<img src=\"", pos)) != string::npos)
	{
		string::size_type startPos = pos + 10;
		string::size_type endPos = strResult.find("\"", startPos);
		
		if (endPos == string::npos)
			break;
		
		string strSrc = strResult.substr(startPos, endPos - startPos);
		
		if (strSrc.substr(0, 7) != "http://" && 
			strSrc.substr(0, 8) != "https://" &&
			strSrc.substr(0, 5) != "data:")
		{
			CString strImgPath;
			if (::PathIsRelative(CString(strSrc.c_str())))
			{
				strImgPath = CString(strBasePath.c_str()) + CString(strSrc.c_str());
			}
			else
			{
				strImgPath = CString(strSrc.c_str());
			}
			
			CString strExt = ::PathFindExtension(strImgPath);
			CString strMimeType = GetImageMimeType(strExt);
			string strBase64 = ReadFileToBase64(strImgPath);
			
			if (!strBase64.empty())
			{
				string strDataUri = "data:";
				strDataUri += CT2A(strMimeType);
				strDataUri += ";base64,";
				strDataUri += strBase64;
				
				strResult.replace(startPos, endPos - startPos, strDataUri);
				pos = startPos + strDataUri.length();
				continue;
			}
		}
		
		pos = endPos + 1;
	}
	
	return strResult;
}

void CMainFrame::OnExportHtml()
{
	CMarkdownEditorDoc* pDoc = (CMarkdownEditorDoc*)GetActiveDocument();
	if (!pDoc)
	{
		AfxMessageBox(_T("No document to export!"), MB_OK | MB_ICONWARNING);
		return;
	}
	
	CMarkdownEditorView* pView = GetRightPane();
	if (!pView)
		return;
	
	const string& strText = pDoc->getText();
	string strHtml = pView->GetMdHtml(strText);
	
	string strBasePath = pDoc->getFilePath();
	if (strBasePath.empty())
	{
		strBasePath = Util::GetExePath();
	}
	
	strHtml = ReplaceImagesWithBase64(strHtml, strBasePath);
	
	CFileDialog dlg(FALSE, _T("html"), NULL, 
		OFN_HIDEREADONLY | OFN_OVERWRITEPROMPT,
		_T("HTML Files (*.html)|*.html|All Files (*.*)|*.*||"), this);
	
	if (dlg.DoModal() != IDOK)
		return;
	
	CString strSavePath = dlg.GetPathName();
	string strUtf8Html = Util::ANSIToUTF8(strHtml.c_str());
	
	CFile file;
	if (file.Open(strSavePath, CFile::modeCreate | CFile::modeWrite))
	{
		file.Write(strUtf8Html.c_str(), (UINT)strUtf8Html.length());
		file.Close();
		
		CString strMsg;
		strMsg.Format(_T("Export successful!\n%s"), strSavePath);
		AfxMessageBox(strMsg, MB_OK | MB_ICONINFORMATION);
	}
	else
	{
		AfxMessageBox(_T("Export failed, cannot write file!"), MB_OK | MB_ICONERROR);
	}
}

void CMainFrame::OnDropFiles(HDROP hDropInfo)
{
	extern bool s_bOpeningFile;
	if (s_bOpeningFile)
	{
		::DragFinish(hDropInfo);
		CFrameWnd::OnDropFiles(hDropInfo);
		return;
	}
	
	UINT nFiles = ::DragQueryFile(hDropInfo, 0xFFFFFFFF, NULL, 0);
	
	for (UINT i = 0; i < nFiles; i++)
	{
		TCHAR szFileName[MAX_PATH];
		::DragQueryFile(hDropInfo, i, szFileName, MAX_PATH);
		
		CString strFile(szFileName);
		CString strExt = ::PathFindExtension(strFile);
		strExt.MakeLower();
		
		if (strExt == _T(".md"))
		{
			CMarkdownEditorDoc* pDoc = (CMarkdownEditorDoc*)GetActiveDocument();
			if (pDoc && pDoc->IsModified())
			{
				CString strMsg;
				strMsg.Format(_T("Current file modified, save?"));
				int nRet = AfxMessageBox(strMsg, MB_YESNOCANCEL | MB_ICONQUESTION);
				if (nRet == IDCANCEL)
				{
					::DragFinish(hDropInfo);
					CFrameWnd::OnDropFiles(hDropInfo);
					return;
				}
				if (nRet == IDYES)
				{
					CString strPathName = pDoc->GetPathName();
					if (strPathName.IsEmpty())
					{
						CFileDialog dlg(FALSE, _T("md"), NULL, 
							OFN_HIDEREADONLY | OFN_OVERWRITEPROMPT,
							_T("Markdown Files (*.md)|*.md|All Files (*.*)|*.*||"), this);
						if (dlg.DoModal() != IDOK)
						{
							::DragFinish(hDropInfo);
							CFrameWnd::OnDropFiles(hDropInfo);
							return;
						}
						strPathName = dlg.GetPathName();
					}
					if (!pDoc->OnSaveDocument(strPathName))
					{
						AfxMessageBox(_T("Failed to save file!"), MB_OK | MB_ICONERROR);
						::DragFinish(hDropInfo);
						CFrameWnd::OnDropFiles(hDropInfo);
						return;
					}
				}
			}
			
			extern bool g_isOpenFile;
			s_bOpeningFile = true;
			if (AfxGetApp()->OpenDocumentFile(strFile))
			{
				g_isOpenFile = true;
			}
			s_bOpeningFile = false;
			
			break;
		}
	}
	
	::DragFinish(hDropInfo);
	CFrameWnd::OnDropFiles(hDropInfo);
}
