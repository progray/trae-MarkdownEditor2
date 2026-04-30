
// MarkdownEditorView.cpp : CMarkdownEditorView ���ʵ��
//

#include "stdafx.h"
#include "Util.h"
#include <string>
// SHARED_HANDLERS ������ʵ��Ԥ��������ͼ������ɸѡ�������
// ATL ��Ŀ�н��ж��壬�����������Ŀ�����ĵ����롣
#ifndef SHARED_HANDLERS
#include "MarkdownEditor.h"
#endif

#include "MarkdownEditorDoc.h"
#include "MarkdownEditorView.h"
#include "MyClickEvents.h"

#ifdef _DEBUG
#define new DEBUG_NEW
#endif


// CMarkdownEditorView

IMPLEMENT_DYNCREATE(CMarkdownEditorView, CHtmlView)

BEGIN_MESSAGE_MAP(CMarkdownEditorView, CHtmlView)
END_MESSAGE_MAP()

// CMarkdownEditorView ����/����

CMarkdownEditorView::CMarkdownEditorView()
{
	// TODO: �ڴ˴����ӹ������
	_bFirstNavigate = true;
	initCSS();
}

CMarkdownEditorView::~CMarkdownEditorView()
{
}

BOOL CMarkdownEditorView::PreCreateWindow(CREATESTRUCT& cs)
{
	// TODO: �ڴ˴�ͨ���޸�
	//  CREATESTRUCT cs ���޸Ĵ��������ʽ

	return CHtmlView::PreCreateWindow(cs);
}

void CMarkdownEditorView::OnInitialUpdate()
{
	CHtmlView::OnInitialUpdate();

}


// CMarkdownEditorView ���

#ifdef _DEBUG
void CMarkdownEditorView::AssertValid() const
{
	CHtmlView::AssertValid();
}

void CMarkdownEditorView::Dump(CDumpContext& dc) const
{
	CHtmlView::Dump(dc);
}

CMarkdownEditorDoc* CMarkdownEditorView::GetDocument() const // �ǵ��԰汾��������
{
	ASSERT(m_pDocument->IsKindOf(RUNTIME_CLASS(CMarkdownEditorDoc)));
	return (CMarkdownEditorDoc*)m_pDocument;
}
#endif //_DEBUG

void setClickEvents(IHTMLDocument2* htmlDocument2, const char* dir) {

	static CMyClickEvents clickEvents;
	clickEvents.SetContext(htmlDocument2, dir);
	_variant_t clickDispatch;
	clickDispatch.vt = VT_DISPATCH;
	clickDispatch.pdispVal = &clickEvents;

	htmlDocument2->put_onclick(clickDispatch);
}

// CMarkdownEditorView ��Ϣ��������

BSTR UTF8ToBSTR(const char* utf8Str)
{
	if (utf8Str == NULL || *utf8Str == '\0')
		return SysAllocString(L"");

	int len = MultiByteToWideChar(CP_UTF8, 0, utf8Str, -1, NULL, 0);
	if (len <= 0)
		return SysAllocString(L"");

	BSTR bstr = SysAllocStringLen(NULL, len - 1);
	if (bstr == NULL)
		return NULL;

	MultiByteToWideChar(CP_UTF8, 0, utf8Str, -1, bstr, len);
	return bstr;
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
	BSTR bstr = UTF8ToBSTR(strHtmlUtf8.c_str());

	SAFEARRAY *psaStrings = SafeArrayCreateVector(VT_VARIANT, 0, 1);
	if (psaStrings == NULL) {
		SysFreeString(bstr);
		pHtmlDoc->close();
		return;
	}
	VARIANT *param;
	hr = SafeArrayAccessData(psaStrings, (LPVOID*)&param);
	param->vt = VT_BSTR;
	param->bstrVal = bstr;
	hr = SafeArrayUnaccessData(psaStrings);
	hr = pHtmlDoc->write(psaStrings);

	setClickEvents(pHtmlDoc, GetDocument()->getFilePath().c_str());

	if (psaStrings != NULL) {
		SafeArrayDestroy(psaStrings);
		pHtmlDoc->close();
	}
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
		return ((float)scrollTop)/height ;
    } 
	return 0.0;
}
void setScrollTop(IDispatch* pDisp, float scrollPercent)
{
	CComPtr<IHTMLTextContainer> pTextContainer = getContainer(pDisp);
    if (pTextContainer)
    {
		long top,height;
		pTextContainer->get_scrollTop(&top);
		pTextContainer->get_scrollHeight(&height);
		pTextContainer->put_scrollTop((long)(scrollPercent * height));
    } 
}
void CMarkdownEditorView::OnUpdate(CView* pSender, LPARAM /*lHint*/lParam, CObject* /*pHint*/)
{
	if(_bFirstNavigate){
		_bFirstNavigate = false;
		Navigate2(_T("about:blank"),NULL,NULL);
		//return;
	}
	if(!(lParam & LPARAM_Update))
		return;
	float scrollTop = 0;
	IDispatch* pDisp =GetHtmlDocument();
	
	if(pSender != NULL){
		scrollTop = getScrollTop(pDisp);
	}
	const string& str = GetDocument()->getText();	

	UpdateMd(str);
	if(lParam & LPARAM_MoveEnd){
		scrollTop = 1.0;
	}
	if(pSender != NULL){
		setScrollTop(pDisp,scrollTop);
	}



	// TODO: �ڴ�����ר�ô����/����û���
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
			if (strnicmp(start, "http://", 7) != 0 && strnicmp(start, "https://", 8) != 0)
				str.replace(pos, old_value.length(), new_value);
		}
		else   
			break;
	}
	return   str;
}
const string HTML_TMPL = 
"<!DOCTYPE html>"
"<html>"
"<head>"
"<meta http-equiv=\"X-UA-Compatible\" content=\"IE=edge\" />"
"<meta charset=\"UTF-8\">"
"<style type=\"text/css\">{{0}}</style>"
"<link rel=\"stylesheet\" href=\"https://cdn.jsdelivr.net/npm/katex@0.16.9/dist/katex.min.css\" crossorigin=\"anonymous\">"
"</head>"
"<body>"
"<div id=\"content\">{{1}}</div>"
"<script src=\"https://cdn.jsdelivr.net/npm/katex@0.16.9/dist/katex.min.js\" crossorigin=\"anonymous\"></script>"
"<script src=\"https://cdn.jsdelivr.net/npm/katex@0.16.9/dist/contrib/auto-render.min.js\" crossorigin=\"anonymous\"></script>"
"<script type=\"text/javascript\">"
"function renderMath() {"
"    if (typeof renderMathInElement !== 'undefined' && document.getElementById('content')) {"
"        renderMathInElement(document.getElementById('content'), {"
"            delimiters: ["
"                {left: '$$', right: '$$', display: true},"
"                {left: '$', right: '$', display: false}"
"            ],"
"            throwOnError: false"
"        });"
"    } else {"
"        setTimeout(renderMath, 100);"
"    }"
"}"
"renderMath();"
"</script>"
"</body>"
"</html>";

string CMarkdownEditorView::GetMdHtml(const string& str){
	string strHtml = HTML_TMPL;
	Util::ReplaceAllStr(strHtml,"{{0}}", _strCSS);
	string md = Util::Text2Md(str);
	md = replaceImgSrc(md, GetDocument()->getFilePath());
	Util::ReplaceAllStr(strHtml, "{{1}}", md);
	return strHtml;
}

void CMarkdownEditorView::UpdateMd(const string& strMd)
{
	string strHtml = GetMdHtml(strMd);
	NavigateHTML(strHtml);
}


