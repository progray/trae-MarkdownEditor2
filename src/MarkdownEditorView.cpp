
// MarkdownEditorView.cpp : CMarkdownEditorView ���ʵ��
//

#include "stdafx.h"
#include "Util.h"
#include <string>

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
	_fScrollPercent = 0.0f;
	_nRestoreScrollTimer = 0;
	initCSS();
}

CMarkdownEditorView::~CMarkdownEditorView()
{
	if (_nRestoreScrollTimer != 0)
	{
		KillTimer(_nRestoreScrollTimer);
		_nRestoreScrollTimer = 0;
	}
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

void setClickEvents(IHTMLDocument2* htmlDocument2, const char* dir) {
	static CMyClickEvents clickEvents;
	clickEvents.SetContext(htmlDocument2, dir);
	_variant_t clickDispatch;
	clickDispatch.vt = VT_DISPATCH;
	clickDispatch.pdispVal = &clickEvents;
	htmlDocument2->put_onclick(clickDispatch);
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

void CMarkdownEditorView::OnTimer(UINT_PTR nIDEvent)
{
	if (nIDEvent == _nRestoreScrollTimer)
	{
		IDispatch* pDisp = GetHtmlDocument();
		if (pDisp != NULL && _fScrollPercent > 0.001f)
		{
			CComPtr<IHTMLDocument2> pHtmlDoc;
			if (SUCCEEDED(pDisp->QueryInterface(IID_IHTMLDocument2, (void**)&pHtmlDoc)))
			{
				CComPtr<IHTMLElement> pBody;
				if (SUCCEEDED(pHtmlDoc->get_body(&pBody)) && pBody != NULL)
				{
					CComPtr<IHTMLTextContainer> pTextContainer;
					if (SUCCEEDED(pBody->QueryInterface(IID_IHTMLTextContainer, (LPVOID*)&pTextContainer)))
					{
						long height;
						if (SUCCEEDED(pTextContainer->get_scrollHeight(&height)) && height > 100)
						{
							setScrollTop(pDisp, _fScrollPercent);
							KillTimer(_nRestoreScrollTimer);
							_nRestoreScrollTimer = 0;
							_fScrollPercent = 0.0f;
						}
					}
				}
			}
		}
	}
	
	CHtmlView::OnTimer(nIDEvent);
}

void CMarkdownEditorView::NavigateHTML(const string& strHtml)
{
	IDispatch* pDoc = GetHtmlDocument();
	if(NULL == pDoc)
		return;

	CComPtr<IHTMLDocument2> pHtmlDoc;
	HRESULT hr = pDoc->QueryInterface(IID_IHTMLDocument2, (void**)&pHtmlDoc);
	if (FAILED(hr))
		return;

	string strHtmlUtf8 = Util::ANSIToUTF8(strHtml.c_str());
	
	int len = MultiByteToWideChar(CP_UTF8, 0, strHtmlUtf8.c_str(), -1, NULL, 0);
	if (len <= 0)
		return;

	BSTR bstr = SysAllocStringLen(NULL, len - 1);
	if (bstr == NULL)
		return;
	MultiByteToWideChar(CP_UTF8, 0, strHtmlUtf8.c_str(), -1, bstr, len);

	SAFEARRAY *psaStrings = SafeArrayCreateVector(VT_VARIANT, 0, 1);
	if (psaStrings == NULL) {
		SysFreeString(bstr);
		return;
	}
	
	VARIANT *param;
	hr = SafeArrayAccessData(psaStrings, (LPVOID*)&param);
	param->vt = VT_BSTR;
	param->bstrVal = bstr;
	hr = SafeArrayUnaccessData(psaStrings);
	
	hr = pHtmlDoc->write(psaStrings);

	setClickEvents(pHtmlDoc, GetDocument()->getFilePath().c_str());

	SafeArrayDestroy(psaStrings);
	pHtmlDoc->close();

	if (_fScrollPercent > 0.001f)
	{
		if (_nRestoreScrollTimer != 0)
			KillTimer(_nRestoreScrollTimer);
		_nRestoreScrollTimer = SetTimer(1, 50, NULL);
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
"    function loadScript(url, callback) {\n"
"        var script = document.createElement('script');\n"
"        script.type = 'text/javascript';\n"
"        script.src = url;\n"
"        script.onreadystatechange = function() {\n"
"            if (this.readyState == 'complete' || this.readyState == 'loaded') {\n"
"                if (callback) callback();\n"
"            }\n"
"        };\n"
"        script.onload = function() {\n"
"            if (callback) callback();\n"
"        };\n"
"        document.getElementsByTagName('head')[0].appendChild(script);\n"
"    }\n"
"    \n"
"    var scriptsLoaded = 0;\n"
"    var totalScripts = 2;\n"
"    \n"
"    function checkAllLoaded() {\n"
"        scriptsLoaded++;\n"
"        if (scriptsLoaded >= totalScripts) {\n"
"            renderMath();\n"
"        }\n"
"    }\n"
"    \n"
"    function renderMath() {\n"
"        if (typeof renderMathInElement !== 'undefined') {\n"
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
"        } else {\n"
"            setTimeout(renderMath, 200);\n"
"        }\n"
"    }\n"
"    \n"
"    loadScript('https://cdn.jsdelivr.net/npm/katex@0.16.9/dist/katex.min.js', checkAllLoaded);\n"
"    loadScript('https://cdn.jsdelivr.net/npm/katex@0.16.9/dist/contrib/auto-render.min.js', checkAllLoaded);\n"
"})();\n"
"</script>\n";

const string HTML_TMPL = 
"<!DOCTYPE html>\r\n"
"<html>\r\n"
"<head>\r\n"
"<meta http-equiv=\"X-UA-Compatible\" content=\"IE=edge\" />\r\n"
"<meta charset=\"UTF-8\">\r\n"
"<meta http-equiv=\"Content-Type\" content=\"text/html; charset=utf-8\">\r\n"
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
