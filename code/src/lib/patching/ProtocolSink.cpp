/****************************************************************************
 * ProtocolSink.cpp : Implementation of ProtocolSink
 * Copyright 2013 Salsita Software (http://www.salsitasoft.com).
 * Author: Arne Seib <arne@salsitasoft.com>
 ****************************************************************************/

#include "stdafx.h"
#include "ProtocolSink.h"

namespace protocolpatchLib
{

#define LOG_ProtocolSink
#ifdef LOG_ProtocolSink
#define ProtocolSink_TRACE(...) \
  ATLTRACE(__FUNCTION__); \
  ATLTRACE(_T(": ")); \
  ATLTRACE(__VA_ARGS__); \
  ATLTRACE(_T("\n"));
#else
#define ProtocolSink_TRACE(...)
#endif

/*============================================================================
 * class ProtocolSink
 */
PROTOCOLDATA ProtocolSink::sProtocolData[3] = {
  {PD_FORCE_SWITCH, SWITCH_START, 0, 0},
  {PD_FORCE_SWITCH, SWITCH_START_EX, 0, 0},
  {PD_FORCE_SWITCH, SWITCH_REPORT_RESULT, 0, 0}
};
PROTOCOLDATA * ProtocolSink::sProtocolDataStart = &ProtocolSink::sProtocolData[0];
PROTOCOLDATA * ProtocolSink::sProtocolDataStartEx = &ProtocolSink::sProtocolData[1];
PROTOCOLDATA * ProtocolSink::sProtocolDataReportResult = &ProtocolSink::sProtocolData[2];

//----------------------------------------------------------------------------
//  CTOR
ProtocolSink::ProtocolSink() :
    m_bindVerb(-1), mDocumentThreadId(0)
{
}

//----------------------------------------------------------------------------
//  initRequest
HRESULT ProtocolSink::initRequest(IFrameRecord * aFrameRecord, IUri * aUri, DWORD aDocumentThreadId)
{
  mDocumentThreadId = aDocumentThreadId;
  return mRequestRecord.initRequest(aFrameRecord, aUri);
}

//----------------------------------------------------------------------------
//  SwitchStart
HRESULT ProtocolSink::SwitchStart()
{
  if (mDocumentThreadId == ::GetCurrentThreadId()) {
    return ContinueStart();
  }
  // this will get released in Protocol::Continue(...)
  ((IInternetProtocolSink*)this)->AddRef();
  return Switch(sProtocolDataStart);
}

//----------------------------------------------------------------------------
//  SwitchStartEx
HRESULT ProtocolSink::SwitchStartEx()
{
  if (mDocumentThreadId == ::GetCurrentThreadId()) {
    return ContinueStartEx();
  }
  // this will get released in Protocol::Continue(...)
  ((IInternetProtocolSink*)this)->AddRef();
  return Switch(sProtocolDataStartEx);
}

//----------------------------------------------------------------------------
//  SwitchReportResult
HRESULT ProtocolSink::SwitchReportResult()
{
  // Sometimes Continue() is not called when Switch()ing on the document
  // thread.
  // To avoid this we switch only when we are not already on the
  // document thread.
  // More mysteriously this seems to happen only for ReportResult() - probably
  // because this is the last call before the request is finished.
  if (mDocumentThreadId == ::GetCurrentThreadId()) {
    return ContinueReportResult();
  }
  // this will get released in Protocol::Continue(...)
  ((IInternetProtocolSink*)this)->AddRef();
  return Switch(sProtocolDataReportResult);
}

//----------------------------------------------------------------------------
//  ContinueStart
HRESULT ProtocolSink::ContinueStart()
{
  ATLASSERT(mStartParams.pTargetProtocol);
  if (!mStartParams.pTargetProtocol) {
    return E_UNEXPECTED;
  }
  ProtocolSink_TRACE(L"%s", mStartParams.sUri);
  // Actually start the request by calling Start() on the native protocol
  HRESULT hr = mStartParams.pTargetProtocol->Start(
            mStartParams.sUri,
            this,
            this,
            mStartParams.grfPI,
            mStartParams.dwReserved);
  ProtocolSink_TRACE(L"0x%08x", hr);
  return hr;
}

//----------------------------------------------------------------------------
//  ContinueStartEx
HRESULT ProtocolSink::ContinueStartEx()
{
  CComQIPtr<IInternetProtocolEx> targetProtocolEx(mStartParams.pTargetProtocol);
  if (!targetProtocolEx) {
    return E_NOINTERFACE;
  }
  ProtocolSink_TRACE(L"%s", mStartParams.sUri);
  // Actually start the request by calling StartEx() on the native protocol
  HRESULT hr = targetProtocolEx->StartEx(
            mStartParams.pUri,
            this,
            this,
            mStartParams.grfPI,
            mStartParams.dwReserved);
  ProtocolSink_TRACE(L"0x%08x", hr);
  return hr;
}

//----------------------------------------------------------------------------
//  ContinueReportResult
HRESULT ProtocolSink::ContinueReportResult()
{
  CComPtr<IFrameRecord> frameRecord = mRequestRecord.frameRecord;
  if (frameRecord) {
    CComPtr<IWebBrowser2> browser;
    frameRecord->getBrowser(&browser.p);
    if (browser) {
      CComPtr<IDispatch> tmp;
      browser->get_Document(&tmp);
      CComQIPtr<IHTMLDocument2> document(tmp);
      if (document) {
        DocumentSink::prepareDocumentNotification(mRequestRecord, document);
      }
    }
  }
  return S_OK;
}

//----------------------------------------------------------------------------
//  OnStart
HRESULT ProtocolSink::OnStart(
  LPCWSTR szUrl, IInternetProtocolSink *pOIProtSink,
  IInternetBindInfo *pOIBindInfo,  DWORD grfPI, HANDLE_PTR dwReserved,
  IInternetProtocol* pTargetProtocol)
{
  ProtocolSink_TRACE(L"%s", szUrl);
  // initialize internal members
  HRESULT hr = __super::OnStart(szUrl, pOIProtSink, pOIBindInfo, grfPI, dwReserved, pTargetProtocol);
  if (FAILED(hr)) {
    return hr;
  }

  // setup arguments for Start
  mStartParams.dwReserved = dwReserved;
  mStartParams.grfPI = grfPI;
  mStartParams.pOIBindInfo = pOIBindInfo;
  mStartParams.pOIProtSink = pOIProtSink;
  mStartParams.pTargetProtocol = pTargetProtocol;
  mStartParams.sUri = szUrl;

  mStartParams.pUri.Release();
  ::CreateUri(szUrl, Uri_CREATE_CANONICALIZE | Uri_CREATE_NO_DECODE_EXTRA_INFO, 0, &mStartParams.pUri);
  return S_OK;
}

//----------------------------------------------------------------------------
//  OnStartEx
HRESULT ProtocolSink::OnStartEx(
  IUri* pUri, IInternetProtocolSink *pOIProtSink,
  IInternetBindInfo *pOIBindInfo,  DWORD grfPI, HANDLE_PTR dwReserved,
  IInternetProtocolEx* pTargetProtocol)
{
  ATLASSERT(pUri);
  if (!pUri) {
    return E_INVALIDARG;
  }
  // initialize internal members
  HRESULT hr = __super::OnStartEx(pUri, pOIProtSink, pOIBindInfo, grfPI, dwReserved, pTargetProtocol);
  if (FAILED(hr)) {
    return hr;
  }

  // setup arguments for StartEx
  mStartParams.dwReserved = dwReserved;
  mStartParams.grfPI = grfPI;
  mStartParams.pOIBindInfo = pOIBindInfo;
  mStartParams.pOIProtSink = pOIProtSink;
  mStartParams.pTargetProtocol = pTargetProtocol;
  mStartParams.pUri = pUri;

  mStartParams.sUri.Empty();
  CComBSTR bs;
  hr = pUri->GetAbsoluteUri(&bs);
  mStartParams.sUri = bs;
  ProtocolSink_TRACE(L"%s", bs);

  return hr;
}

//----------------------------------------------------------------------------
//  beforeRequest
HRESULT ProtocolSink::beforeRequest()
{
  // get the "Accept" header for the request type
  // (http://developer.chrome.com/extensions/webRequest.html#event-onBeforeRequest)
  LPCWSTR requestType = NULL;
  CComQIPtr<IWinInetHttpInfo> httpInfo(mStartParams.pTargetProtocol);
  if (httpInfo) {
    CStringA acceptHeader;
    DWORD size = 0;
    DWORD flags = 0;
    httpInfo->QueryInfo(HTTP_QUERY_ACCEPT|HTTP_QUERY_FLAG_REQUEST_HEADERS, NULL, &size, &flags, 0);
    if (size) {
      httpInfo->QueryInfo(HTTP_QUERY_ACCEPT|HTTP_QUERY_FLAG_REQUEST_HEADERS, acceptHeader.GetBuffer(size), &size, &flags, 0);
      acceptHeader.ReleaseBuffer();
    }
    if (-1 != acceptHeader.Find("image")) {
      requestType = L"image";
    }
    else if (-1 != acceptHeader.Find("css")) {
      requestType = L"stylesheet";
    }
    else if (-1 != acceptHeader.Find("script")) {
      requestType = L"script";
    }
  }

  CComPtr<IUri> redirectUri;
#ifdef _DEBUG
  ProtocolSink_TRACE(L"fire_onBeforeRequest %s", mRequestRecord.mUrlString);
#endif
  // fire event
  HRESULT hr = mRequestRecord.fire_onBeforeRequest(requestType, &redirectUri.p);

  if (E_ABORT == hr) {
#ifdef _DEBUG
   ProtocolSink_TRACE(L"aborted %s", mRequestRecord.mUrlString);
#endif
   ReportResult(hr, 0, L"aborted");
    return hr;
  }

  // redirect?
  if (redirectUri) {
    CComBSTR url;
    redirectUri->GetAbsoluteUri(&url);
    ProtocolSink_TRACE(L"DO redirect to %s", url);
    ReportProgress(64, L"302");
    ReportProgress(BINDSTATUS_REDIRECTING, url);
    ReportResult(INET_E_REDIRECTING, 0, url);
    ProtocolSink_TRACE(L"return INET_E_REDIRECTING");
    return INET_E_REDIRECTING;
  }

  return S_OK;
}

//----------------------------------------------------------------------------
//  BeginningTransaction
STDMETHODIMP ProtocolSink::BeginningTransaction(
  /* [in] */  LPCWSTR   szURL,
  /* [in] */  LPCWSTR   szHeaders,
  /* [in] */  DWORD     dwReserved,
  /* [out] */ LPWSTR  * pszAdditionalHeaders)
{
  CComPtr<IHttpNegotiate> spHttpNegotiate;
  QueryServiceFromClient(&spHttpNegotiate);
  CStringW sHdrs;

  // get current headers from IWinInetHttpInfo
  CComQIPtr<IWinInetHttpInfo> httpInfo(mStartParams.pTargetProtocol);
  if (httpInfo) {
    CStringA allHeaders;
    DWORD size = 0;
    DWORD flags = 0;
    httpInfo->QueryInfo(HTTP_QUERY_RAW_HEADERS_CRLF|HTTP_QUERY_FLAG_REQUEST_HEADERS, NULL, &size, &flags, 0);
    if (size) {
      httpInfo->QueryInfo(HTTP_QUERY_RAW_HEADERS_CRLF|HTTP_QUERY_FLAG_REQUEST_HEADERS, allHeaders.GetBuffer(size), &size, &flags, 0);
      if (size >= 2) {
        // strip last CRLF
        size -= 2;
      }
      allHeaders.ReleaseBuffer(size);
    }
    sHdrs += CA2W(allHeaders);
  }

  // add headers passed in here
  sHdrs += szHeaders;

  // add additional headers
  if (pszAdditionalHeaders) {
    sHdrs += L"\r\n";
    sHdrs += (*pszAdditionalHeaders);
    sHdrs += L"\r\n";
  }

  // fire onBeforeRequest first
  ProtocolSink_TRACE(L"beforeRequest %s", szURL);
  HRESULT hr = beforeRequest();
  if (INET_E_REDIRECTING == hr) {
    ProtocolSink_TRACE(L"==> redirect %s ==>", szURL);
    return hr;
  }
  if (FAILED(hr)) {
    return hr;
  }

  ProtocolSink_TRACE(L"fire_onBeforeSendHeaders %s", szURL);
  // then fire onBeforeSendHeaders
  hr = mRequestRecord.fire_onBeforeSendHeaders(m_bindVerb, sHdrs);
  if (FAILED(hr)) {
    return hr;
  }

  ProtocolSink_TRACE(L"BeginningTransaction on original %s", szURL);
  LPWSTR additionalHeaders = NULL;
  hr = (spHttpNegotiate)
      ? spHttpNegotiate->BeginningTransaction(szURL, sHdrs, dwReserved, &additionalHeaders)
      : S_OK;

  if (additionalHeaders) {
    sHdrs += additionalHeaders;
    ::CoTaskMemFree(additionalHeaders);
  }
  LPWSTR wszAdditionalHeaders = (LPWSTR) CoTaskMemAlloc((sHdrs.GetLength()+1)*sizeof(WCHAR));
  if (!wszAdditionalHeaders) {
    return E_OUTOFMEMORY;
  }

  wcscpy_s(wszAdditionalHeaders, sHdrs.GetLength()+1, sHdrs);
  if (*pszAdditionalHeaders) {
    ::CoTaskMemFree(*pszAdditionalHeaders);
  }
  (*pszAdditionalHeaders) = wszAdditionalHeaders;

  return hr;
}

//----------------------------------------------------------------------------
//  OnResponse
STDMETHODIMP ProtocolSink::OnResponse(
  /* [in] */ DWORD dwResponseCode,
  /* [in] */ LPCWSTR szResponseHeaders,
  /* [in] */ LPCWSTR szRequestHeaders,
  /* [out] */ LPWSTR *pszAdditionalRequestHeaders)
{
  if (pszAdditionalRequestHeaders) {
    (*pszAdditionalRequestHeaders) = 0;
  }

  CComPtr<IHttpNegotiate> spHttpNegotiate;
  QueryServiceFromClient(&spHttpNegotiate);
  HRESULT hr = (spHttpNegotiate)
      ? spHttpNegotiate->OnResponse(dwResponseCode, szResponseHeaders, szRequestHeaders, pszAdditionalRequestHeaders)
      : S_OK;
  hr = (E_ABORT == mRequestRecord.fire_onHeadersReceived(dwResponseCode, szResponseHeaders, hr)) ? E_ABORT : hr;
  return hr;
}

//----------------------------------------------------------------------------
//  ReportProgress
STDMETHODIMP ProtocolSink::ReportProgress(
  /* [in] */ ULONG ulStatusCode,
  /* [in] */ LPCWSTR szStatusText)
{
  if (ulStatusCode == BINDSTATUS_REDIRECTING) {
    ProtocolSink_TRACE(L"==> redirecting ==> %s", szStatusText);
    HRESULT hr = mRequestRecord.fire_onBeforeRedirect(szStatusText);
    if (E_ABORT == hr) {
      return hr;
    }
  }

  ATLASSERT(m_spInternetProtocolSink != 0);
  HRESULT hr = (m_spInternetProtocolSink)
      ? m_spInternetProtocolSink->ReportProgress(ulStatusCode, szStatusText)
      : S_OK;
  return hr;
}

//----------------------------------------------------------------------------
//  ReportData
STDMETHODIMP ProtocolSink::ReportData(
  /* [in] */ DWORD grfBSCF,
  /* [in] */ ULONG ulProgress,
  /* [in] */ ULONG ulProgressMax)
{
  ATLASSERT(m_spInternetProtocolSink != 0);
  return (m_spInternetProtocolSink)
      ? m_spInternetProtocolSink->ReportData(grfBSCF, ulProgress, ulProgressMax)
      : S_OK;
}

//----------------------------------------------------------------------------
//  ReportResult
STDMETHODIMP ProtocolSink::ReportResult(
  /* [in] */ HRESULT hrResult,
  /* [in] */ DWORD dwError,
  /* [in] */ LPCWSTR szResult)
{
  ATLASSERT(m_spInternetProtocolSink != 0);
  HRESULT hr = (m_spInternetProtocolSink)
      ? m_spInternetProtocolSink->ReportResult(hrResult, dwError, szResult)
      : S_OK;

  ProtocolSink_TRACE(L"0x%08x - %i - \"%s\"", hrResult, dwError, szResult);
  if (hrResult == 0) {
    // NOTE: We are already on the document thread here, but we still Switch()
    // because we are not able to get a html document at this state of the request.
    // So handle this in the next Continue() call.
    mReportResultParams.dwError = dwError;
    mReportResultParams.hrResult = hrResult;
    mReportResultParams.szResult = szResult;
    SwitchReportResult();
  }
  return hr;
}

//----------------------------------------------------------------------------
// GetBindInfo
STDMETHODIMP ProtocolSink::GetBindInfoEx(
  /* [out] */ DWORD *grfBINDF,
  /* [in, out] */ BINDINFO *pbindinfo,
  /* [out] */ DWORD *grfBINDF2,
  /* [out] */ DWORD* pdwReserved)
{
  CComQIPtr<IInternetBindInfoEx> pBindInfo(m_spInternetProtocolSink);
  if (!pBindInfo) {
    return E_NOINTERFACE;
  }
  HRESULT hr = pBindInfo->GetBindInfoEx(grfBINDF, pbindinfo, grfBINDF2, pdwReserved);
  if (SUCCEEDED(hr)) {
    m_bindVerb = pbindinfo->dwBindVerb;
  }
  return hr;
}

/*============================================================================
 * class ProtocolSink::DocumentSink
 */

//----------------------------------------------------------------------------
//  prepareDocumentNotification
HRESULT ProtocolSink::DocumentSink::prepareDocumentNotification(RequestRecord & aRecord, IHTMLDocument2 * aHTMLDoc)
{
  ATLASSERT(aHTMLDoc);
  CComBSTR readyState;
  aHTMLDoc->get_readyState(&readyState);

  aRecord.fire_onDocumentReadyState(readyState);
  if (RequestRecord::DOCUMENTCOMPLETE == aRecord.state) {
    // complete state already reached
    return S_OK;
  }

  // Document not ready yet: Create instance and let it do the job when ready
  CComObject<DocumentSink> * sinkObject = NULL;
  HRESULT hr = CComObject<DocumentSink>::CreateInstance(&sinkObject);
  if (FAILED(hr)) {
    return hr;
  }
  CComPtr<IUnknown> tmp(sinkObject);
  // init will AddRef the object, it will stay alive until the job is done
  return sinkObject->init(aRecord, aHTMLDoc);
}

//----------------------------------------------------------------------------
//  CTOR
ProtocolSink::DocumentSink::DocumentSink()
{
}

//----------------------------------------------------------------------------
//  DTOR
ProtocolSink::DocumentSink::~DocumentSink()
{
  if (mHTMLDocument) {
    DispEventUnadvise(mHTMLDocument);
    mHTMLDocument.Release();
  }
}

//----------------------------------------------------------------------------
//  init
HRESULT ProtocolSink::DocumentSink::init(RequestRecord & aRecord, IHTMLDocument2 * aHTMLDoc)
{
  mRecord = aRecord;
  mHTMLDocument = aHTMLDoc;
  HRESULT hr = (mHTMLDocument) ? S_OK : E_INVALIDARG;
  if (FAILED(hr)) {
    return hr;
  }
  // this will AddRef and keeps us alive
  return DispEventAdvise(mHTMLDocument);
}

//----------------------------------------------------------------------------
//  OnReadyStateChange
STDMETHODIMP_(void) ProtocolSink::DocumentSink::OnReadyStateChange(IHTMLEventObj* ev)
{
  ATLASSERT(mHTMLDocument);
  CComBSTR readyState;
  mHTMLDocument->get_readyState(&readyState);

  mRecord.fire_onDocumentReadyState(readyState);
  if (RequestRecord::DOCUMENTCOMPLETE == mRecord.state) {
    // tmp for DispEventUnadvise
    CComPtr<IHTMLDocument2> tmp(mHTMLDocument);
    mHTMLDocument.Release();

    // NOTE: this might destroy us, so no more member access beyond this point!
    DispEventUnadvise(tmp);
  }
}

} // namespace protocolpatchLib
