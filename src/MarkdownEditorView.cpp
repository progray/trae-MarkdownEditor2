// MarkdownEditorView.cpp : CMarkdownEditorView ���ʵ��
//

#include "stdafx.h"
#include "Util.h"
#include <string>
#include <sstream>

#ifndef SHARED_HANDLERS
#include "MarkdownEditor.h"
#endif

#include "MarkdownEditorDoc.h"
#include "MarkdownEditorView.h"
#include "MyClickEvents.h"

#ifdef _DEBUG
#define new DEBUG_NEW
#endif

IMPLEMENT_DYNCREATE(CMarkdownEditorView, CHtmlView)

BEGIN_MESSAGE_MAP(CMarkdownEditorView, CHtmlView)
	ON_WM_TIMER()
END_MESSAGE_MAP()

CMarkdownEditorView::CMarkdownEditorView()
{
	_bFirstNavigate = true;
	_bPendingScrollRestore = false;
	_fScrollPercent = 0.0f;
	initCSS();
}

CMarkdownEditorView::~CMarkdownEditorView()
{
	CleanupTempHtml();
}

BOOL CMarkdownEditorView::PreCreateWindow(CREATESTRUCT& cs)
{
	return CHtmlView::PreCreateWindow(cs);
}

void CMarkdownEditorView::OnInitialUpdate()
{
	CHtmlView::OnInitialUpdate();
}

#ifdef _DEBUG
void CMarkdownEditorView::AssertValid() const
{
	CHtmlView::AssertValid();
}

void CMarkdownEditorView::Dump(CDumpContext& dc) const
{
	CHtmlView::Dump(dc);
}

CMarkdownEditorDoc* CMarkdownEditorView::GetDocument() const
{
	ASSERT(m_pDocument->IsKindOf(RUNTIME_CLASS(CMarkdownEditorDoc)));
	return (CMarkdownEditorDoc*)m_pDocument;
}
#endif

void CMarkdownEditorView::CleanupTempHtml()
{
	if (!_strTempHtmlPath.empty())
	{
		::DeleteFileA(_strTempHtmlPath.c_str());
		_strTempHtmlPath.clear();
	}
}

string IntToStr(int value)
{
	char buf[32];
	sprintf_s(buf, sizeof(buf), "%d", value);
	return string(buf);
}

bool CMarkdownEditorView::WriteTempHtmlFile(const string& strHtml)
{
	CleanupTempHtml();

	char szTempPath[MAX_PATH] = {0};
	if (::GetTempPathA(MAX_PATH, szTempPath) == 0)
		return false;

	string strTempDir = szTempPath;
	string strTempFile = strTempDir + "md_preview_" + 
		IntToStr(::GetCurrentProcessId()) + ".html";

	FILE* fp = fopen(strTempFile.c_str(), "wb");
	if (fp == NULL)
		return false;

	string strUtf8 = Util::ANSIToUTF8(strHtml.c_str());
	
	fwrite(strUtf8.c_str(), 1, strUtf8.size(), fp);
	fclose(fp);

	_strTempHtmlPath = strTempFile;
	return true;
}

CComPtr<IHTMLTextContainer> getContainer(IDispatch* pDisp){
	if(NULL == pDisp)
		return NULL;
	CComPtr<IHTMLDocument2> pDocument2 = NULL; 
	if (S_OK == pDisp->QueryInterface(IID_IHTMLDocument2, (LPVOID*)&pDocument2)) 
	{ 
		CComPtr<IHTMLElement> pElement = NULL; 
		if (S_OK == pDocument2->get_body(&pElement)) 
		{ 
			CComPtr<IHTMLTextContainer> pTextContainer = NULL; 
			if (S_OK == pElement->QueryInterface(IID_IHTMLTextContainer, (LPVOID*)&pTextContainer)) 
			{ 
				return pTextContainer;
			} 
		}                 
	} 
	return NULL;
}

float getScrollTop(IDispatch* pDisp)
{
	long scrollTop;
	CComPtr<IHTMLTextContainer> pTextContainer = getContainer(pDisp);
	if (pTextContainer &&  S_OK == pTextContainer->get_scrollTop(&scrollTop) ) 
	{
		long height;
		pTextContainer->get_scrollHeight(&height);
		if (height > 0)
			return ((float)scrollTop)/height ;
	} 
	return 0.0;
}

void setScrollTop(IDispatch* pDisp, float scrollPercent)
{
	CComPtr<IHTMLTextContainer> pTextContainer = getContainer(pDisp);
	if (pTextContainer)
	{
		long height;
		pTextContainer->get_scrollHeight(&height);
		if (height > 0)
		{
			pTextContainer->put_scrollTop((long)(scrollPercent * height));
		}
	} 
}

void setClickEvents(IHTMLDocument2* htmlDocument2, const char* dir) {
	static CMyClickEvents clickEvents;
	clickEvents.SetContext(htmlDocument2, dir);
	_variant_t clickDispatch;
	clickDispatch.vt = VT_DISPATCH;
	clickDispatch.pdispVal = &clickEvents;
	htmlDocument2->put_onclick(clickDispatch);
}

void CMarkdownEditorView::SetClickEventHandler()
{
	IDispatch* pDisp = GetHtmlDocument();
	if (pDisp == NULL)
		return;

	CComPtr<IHTMLDocument2> pHtmlDoc;
	if (SUCCEEDED(pDisp->QueryInterface(IID_IHTMLDocument2, (void**)&pHtmlDoc)))
	{
		setClickEvents(pHtmlDoc, GetDocument()->getFilePath().c_str());
	}
}

void CMarkdownEditorView::RestoreScrollPosition()
{
	IDispatch* pDisp = GetHtmlDocument();
	if (pDisp == NULL)
		return;

	CComPtr<IHTMLDocument2> pHtmlDoc;
	if (FAILED(pDisp->QueryInterface(IID_IHTMLDocument2, (void**)&pHtmlDoc)))
		return;

	CComPtr<IHTMLElement> pBody;
	if (FAILED(pHtmlDoc->get_body(&pBody)) || pBody == NULL)
		return;

	CComPtr<IHTMLTextContainer> pTextContainer;
	if (FAILED(pBody->QueryInterface(IID_IHTMLTextContainer, (LPVOID*)&pTextContainer)))
		return;

	long height;
	if (FAILED(pTextContainer->get_scrollHeight(&height)) || height <= 100)
	{
		_bPendingScrollRestore = true;
		return;
	}

	setScrollTop(pDisp, _fScrollPercent);
	_bPendingScrollRestore = false;
}

void CMarkdownEditorView::OnTimer(UINT_PTR nIDEvent)
{
	if (nIDEvent == 1)
	{
		if (_bPendingScrollRestore)
		{
			RestoreScrollPosition();
			if (!_bPendingScrollRestore)
			{
				KillTimer(1);
			}
		}
		else
		{
			KillTimer(1);
		}
	}
	
	CHtmlView::OnTimer(nIDEvent);
}

void CMarkdownEditorView::NavigateHTML(const string& strHtml)
{
	if (!WriteTempHtmlFile(strHtml))
		return;

	CString strUrl = _strTempHtmlPath.c_str();
	strUrl.Replace("\\", "/");
	strUrl = "file:///" + strUrl;

	Navigate2(strUrl, NULL, NULL);
}

void CMarkdownEditorView::OnNavigateComplete2(LPCTSTR strURL)
{
	CHtmlView::OnNavigateComplete2(strURL);
}

void CMarkdownEditorView::OnDocumentComplete(LPCTSTR lpszURL)
{
	CHtmlView::OnDocumentComplete(lpszURL);

	SetClickEventHandler();
	
	if (_fScrollPercent > 0.001f)
	{
		_bPendingScrollRestore = true;
		RestoreScrollPosition();
		if (_bPendingScrollRestore)
		{
			SetTimer(1, 50, NULL);
		}
	}
}

void CMarkdownEditorView::OnUpdate(CView* pSender, LPARAM /*lHint*/lParam, CObject* /*pHint*/)
{
	if(_bFirstNavigate){
		_bFirstNavigate = false;
		Navigate2(_T("about:blank"),NULL,NULL);
	}
	if(!(lParam & LPARAM_Update))
		return;
	
	_fScrollPercent = 0.0f;
	_bPendingScrollRestore = false;
	IDispatch* pDisp = GetHtmlDocument();
	
	if(pSender != NULL && pDisp != NULL){
		_fScrollPercent = getScrollTop(pDisp);
	}
	
	const string& str = GetDocument()->getText();	

	if(lParam & LPARAM_MoveEnd){
		_fScrollPercent = 1.0f;
	}

	UpdateMd(str);
}

void CMarkdownEditorView::initCSS(){
	string strUserCss = Util::GetExePath() + "user.css";
	if(PathFileExists(strUserCss.c_str())){
		_strCSS = Util::ReadStringFile(strUserCss.c_str());
	}else{
		Util::LoadStringRes(IDR_CSS,"CSS",_strCSS); 
	}
}

string&  replaceImgSrc(string& str, string path)
{
	if (path.size() == 0)
		return str;
	string old_value = "<img src=\"";
	string new_value = "<img src=\"" + path;
	for (string::size_type pos(0); pos != string::npos; pos += old_value.length())   {
		if ((pos = str.find(old_value, pos)) != string::npos){
			const char* start = str.c_str() + pos + old_value.length();
			if (_strnicmp(start, "http://", 7) != 0 && _strnicmp(start, "https://", 8) != 0)
				str.replace(pos, old_value.length(), new_value);
		}
		else   
			break;
	}
	return   str;
}

const string KATEX_LOADER_JS = 
"<script type=\"text/javascript\">\n"
"(function() {\n"
"    'use strict';\n"
"    \n"
"    window.onerror = function(msg, url, lineNo, columnNo, error) {\n"
"        return true;\n"
"    };\n"
"    \n"
"    var katexLoaded = false;\n"
"    var autoRenderLoaded = false;\n"
"    var renderAttempts = 0;\n"
"    var maxAttempts = 30;\n"
"    var failed = false;\n"
"    \n"
"    function createScript(url, onLoad, onError) {\n"
"        try {\n"
"            var head = document.getElementsByTagName('head')[0];\n"
"            if (!head) {\n"
"                if (onError) onError();\n"
"                return;\n"
"            }\n"
"            \n"
"            var script = document.createElement('script');\n"
"            script.type = 'text/javascript';\n"
"            script.src = url;\n"
"            script.charset = 'UTF-8';\n"
"            \n"
"            script.onreadystatechange = function() {\n"
"                try {\n"
"                    if (this.readyState === 'complete' || this.readyState === 'loaded') {\n"
"                        if (onLoad) onLoad();\n"
"                    }\n"
"                } catch(e) {}\n"
"            };\n"
"            script.onload = function() {\n"
"                try {\n"
"                    if (onLoad) onLoad();\n"
"                } catch(e) {}\n"
"            };\n"
"            script.onerror = function() {\n"
"                try {\n"
"                    failed = true;\n"
"                    if (onError) onError();\n"
"                } catch(e) {}\n"
"            };\n"
"            \n"
"            head.appendChild(script);\n"
"        } catch(e) {\n"
"            failed = true;\n"
"            if (onError) onError();\n"
"        }\n"
"    }\n"
"    \n"
"    function checkAndRender() {\n"
"        if (failed) return;\n"
"        \n"
"        try {\n"
"            if (typeof katex === 'undefined' || typeof renderMathInElement === 'undefined') {\n"
"                renderAttempts++;\n"
"                if (renderAttempts < maxAttempts) {\n"
"                    setTimeout(checkAndRender, 200);\n"
"                }\n"
"                return;\n"
"            }\n"
"            \n"
"            var content = document.getElementById('content');\n"
"            if (content) {\n"
"                renderMathInElement(content, {\n"
"                    delimiters: [\n"
"                        {left: '$$', right: '$$', display: true},\n"
"                        {left: '$', right: '$', display: false}\n"
"                    ],\n"
"                    throwOnError: false\n"
"                });\n"
"            }\n"
"        } catch(e) {\n"
"            failed = true;\n"
"        }\n"
"    }\n"
"    \n"
"    function onScriptLoad() {\n"
"        katexLoaded = true;\n"
"        if (autoRenderLoaded) checkAndRender();\n"
"    }\n"
"    \n"
"    function onAutoRenderLoad() {\n"
"        autoRenderLoaded = true;\n"
"        if (katexLoaded) checkAndRender();\n"
"    }\n"
"    \n"
"    function onScriptError() {\n"
"        failed = true;\n"
"    }\n"
"    \n"
"    try {\n"
"        createScript(\n"
"            'https://cdn.jsdelivr.net/npm/katex@0.16.9/dist/katex.min.js',\n"
"            onScriptLoad,\n"
"            onScriptError\n"
"        );\n"
"        \n"
"        createScript(\n"
"            'https://cdn.jsdelivr.net/npm/katex@0.16.9/dist/contrib/auto-render.min.js',\n"
"            onAutoRenderLoad,\n"
"            onScriptError\n"
"        );\n"
"    } catch(e) {\n"
"        failed = true;\n"
"    }\n"
"})();\n"
"</script>\n";

const string HTML_TMPL = 
"<!DOCTYPE html>\r\n"
"<html>\r\n"
"<head>\r\n"
"<meta http-equiv=\"X-UA-Compatible\" content=\"IE=edge\" />\r\n"
"<meta http-equiv=\"Content-Type\" content=\"text/html; charset=utf-8\" />\r\n"
"<meta name=\"viewport\" content=\"width=device-width, initial-scale=1.0\" />\r\n"
"<style type=\"text/css\">\r\n"
"{{0}}\r\n"
"</style>\r\n"
"<link rel=\"stylesheet\" href=\"https://cdn.jsdelivr.net/npm/katex@0.16.9/dist/katex.min.css\">\r\n"
"</head>\r\n"
"<body>\r\n"
"<div id=\"content\">{{1}}</div>\r\n"
"{{2}}\r\n"
"</body>\r\n"
"</html>\r\n";

string CMarkdownEditorView::GetMdHtml(const string& str){
	string strHtml = HTML_TMPL;
	Util::ReplaceAllStr(strHtml,"{{0}}", _strCSS);
	string md = Util::Text2Md(str);
	md = replaceImgSrc(md, GetDocument()->getFilePath());
	Util::ReplaceAllStr(strHtml, "{{1}}", md);
	Util::ReplaceAllStr(strHtml, "{{2}}", KATEX_LOADER_JS);
	return strHtml;
}

void CMarkdownEditorView::UpdateMd(const string& strMd)
{
	string strHtml = GetMdHtml(strMd);
	NavigateHTML(strHtml);
}
