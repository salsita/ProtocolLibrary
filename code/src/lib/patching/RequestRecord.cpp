#include "StdAfx.h"
#include "RequestRecord.h"
#include "ThreadRecord.h"

//#define REQUEST_EVENT_SRC_USE_DEFAULT_SINK

namespace protocolpatchLib
{

LONG RequestRecord::sRequestId = 0;

RequestRecord::RequestRecord() :
    mRequestId(0), mState(FRESH)
{
}

HRESULT RequestRecord::initRequest(IFrameRecord * aFrameRecord, IUri * aUri)
{
  mRequestId = ::InterlockedIncrement(&sRequestId);
  mState = FRESH;
  mRequest.Release();
  mFrameRecord.Release();

  //mIsDocument = FALSE;
  setCurrentUri(aUri);

  HRESULT hr = Request::createInstance(mRequest);
  if (FAILED(hr)) {
    return hr;
  }
  if (aFrameRecord) {
    // If we have a frame record this is a request for a document.
    // Otherwise this is a resource request and the current frame record
    // is NULL.
    mFrameRecord = aFrameRecord;
  }
  Request* request = static_cast<Request*>(mRequest.p);
  return request->initRequest(*this);
}

HRESULT RequestRecord::fire_onBeforeRequest(LPCWSTR aRequestType, IUri ** aRedirectUri)
{
  if (!mRequest)
    { return E_UNEXPECTED; }

  static_cast<Request*>(mRequest.p)->setType(aRequestType);

  HRESULT hr = S_OK;
  CComPtr<IWebRequestEvents> sink(getSink(hr));
  if (sink && shouldNotify()) {
    hr = sink->onBeforeRequest(mRequest);
    if (FAILED(hr))
      { return hr; }

    // canceled?
    if (S_OK == mRequest->isCanceled())
      { return E_ABORT; }

    // a redirect?
    mRequest->getRedirectUri(aRedirectUri);
  }
  mState = STARTED;
  return hr;
}

HRESULT RequestRecord::fire_onBeforeSendHeaders(DWORD aBindVerb, CStringW & aHeaders)
{
  if (!mRequest)
    { return E_UNEXPECTED; }

  Request* request = static_cast<Request*>(mRequest.p);
  HRESULT hr = request->beforeSendingRequest(*this, aBindVerb, aHeaders);
  if (FAILED(hr))
    { return hr; }

  CComPtr<IWebRequestEvents> sink(getSink(hr));
  if (sink && shouldNotify()) {
    hr = sink->onBeforeSendHeaders(mRequest);
  }
  request->getHeaders(aHeaders);
  mState = SENDING;
  return hr;
}

HRESULT RequestRecord::fire_onBeforeRedirect(LPCWSTR aNewUrl)
{
  CComPtr<IUri> uri;
  HRESULT hr = ::CreateUri(aNewUrl, Uri_CREATE_CANONICALIZE | Uri_CREATE_NO_DECODE_EXTRA_INFO, 0, &uri);
  setCurrentUri(uri);
  if (mRequest) {
    static_cast<Request*>(mRequest.p)
      ->redirectRequest(uri);
  }
  CComPtr<IWebRequestEvents> sink(getSink(hr));
  if (sink && shouldNotify())
    { hr = sink->onBeforeRedirect(mRequest); }
  mState = SENDING;
  return hr;
}

HRESULT RequestRecord::fire_onHeadersReceived(DWORD aResponseCode, LPCWSTR aHeaders, HRESULT aOnResponseResult)
{
  if (RECEIVED == mState)
    { return S_OK; }
  if (!mRequest)
    { return E_UNEXPECTED; }
  HRESULT hr = static_cast<Request*>(mRequest.p)
    ->initResponse((*this), aHeaders, aResponseCode, aOnResponseResult);
  if (FAILED(hr))
    { return hr; }
  CComPtr<IWebRequestEvents> sink(getSink(hr));
  if (sink && shouldNotify())
    { hr = sink->onHeadersReceived(mRequest); }
  mState = RECEIVED;
  return hr;
}

HRESULT RequestRecord::fire_onInteractive()
{
  if (DOCUMENTINTERACTIVE == mState)
    { return S_OK; }
  if (!mRequest)
    { return E_UNEXPECTED; }

  HRESULT hr = S_OK;
  CComPtr<IWebRequestEvents> sink(getSink(hr));
  if (sink && shouldNotify())
    { hr = sink->onInteractive(mRequest); }
  mState = DOCUMENTINTERACTIVE;
  return hr;
}

HRESULT RequestRecord::fire_onCompleted()
{
  if (DOCUMENTCOMPLETE == mState)
    { return S_OK; }
  if (!mRequest)
    { return E_UNEXPECTED; }

  HRESULT hr = S_OK;
  CComPtr<IWebRequestEvents> sink(getSink(hr));
  if (sink && shouldNotify())
    { hr = sink->onCompleted(mRequest); }
  mState = DOCUMENTCOMPLETE;
  return hr;
}

HRESULT RequestRecord::fire_onDocumentReadyState(LPCWSTR aReadyState)
{
  CStringW state(aReadyState);
  if (state == L"interactive" && mState < DOCUMENTINTERACTIVE) {
    return fire_onInteractive();
  }
  else if (state == L"complete" && mState < DOCUMENTCOMPLETE) {
    if (mState < DOCUMENTINTERACTIVE) {
      fire_onInteractive();
    }
    return fire_onCompleted();
  }
  return S_FALSE; // non-critical
}

CComPtr<IWebRequestEvents> RequestRecord::getSink(HRESULT & hr)
{
  if (mFrameRecord) {
    CComPtr<IWebRequestEvents> sink;
    hr = mFrameRecord->getSink(&sink.p);
    ATLASSERT(sink);
    return sink;
  }
  CComPtr<IThreadRecord> threadRecord = ThreadRecord::get();
  if (threadRecord) {
    CComQIPtr<IWebRequestEvents> sink(threadRecord);
    if (!sink) {
      hr = E_NOINTERFACE;
    }
    return sink;
  }
  return NULL;
}

HRESULT RequestRecord::getUri(IUri ** aUriRet)
{
  if (!aUriRet) {
    return E_POINTER;
  }
  CComPtr<IUriBuilder> uriBuilder;
  HRESULT hr = CreateIUriBuilder(mUri, 0, 0, &uriBuilder.p);
  if (FAILED(hr)) {
    return hr;
  }
  return uriBuilder->CreateUri(Uri_CREATE_CANONICALIZE | Uri_CREATE_NO_DECODE_EXTRA_INFO, 0, 0, aUriRet);
}

HRESULT RequestRecord::setCurrentUri(IUri * aUri)
{
  mUri = aUri;
  CComBSTR bs;
  mUri->GetAbsoluteUri(&bs);
#ifdef _DEBUG
  mUrlString = bs;
#endif
  if (mFrameRecord) {
    mFrameRecord->setUri(bs);
  }
  return S_OK;
}

BOOL RequestRecord::shouldNotify()
{
  return TRUE;
}

} // namespace protocolpatchLib
