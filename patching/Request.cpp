/****************************************************************************
 * Request.h : Declaration of Request
 * Copyright 2013 Salsita Software (http://www.salsitasoft.com).
 * Author: Arne Seib <arne@salsitasoft.com>
 ****************************************************************************/

#include "stdafx.h"
#include "Headers.h"
#include "RequestRecord.h"

// Request

namespace protocolpatchLib
{

HRESULT Request::createInstance(CComPtr<IRequest> & aRetVal)
{
  _ComObject * newInstance = NULL;
  HRESULT hr = _ComObject::CreateInstance(&newInstance);
  if (FAILED(hr)) {
    return hr;
  }
  aRetVal = newInstance;

  if (FAILED(hr)) {
    aRetVal.Release();
  }
  return hr;
}

HRESULT Request::initRequest(RequestRecord & aRecord)
{
  mUri.Release();
  mRedirectUri.Release();
  mCurrentBrowser.Release();
  mHeaders.Release();
  mResponse.Release();

  mIsDocument = mIsTopLevel = mCanceled = FALSE;

  mRequestId = aRecord.getId();
  aRecord.getUri(&mUri.p);

  CComPtr<IFrameRecord> frameRecord = aRecord.getFrameRecord();
  if (frameRecord) {
    frameRecord->getBrowser(&mCurrentBrowser.p);
    mIsDocument = (S_OK == frameRecord->isEqualUri(mUri));
    mIsTopLevel = (S_OK == frameRecord->isTopLevel());
  }

  // request type might change later as soon as we have some
  // request headers
  mType = (mIsTopLevel) ? L"main_frame" : L"sub_frame";

  return S_OK;
}

HRESULT Request::beforeSendingRequest(RequestRecord & aRecord, DWORD aBindVerb, LPCWSTR aHeaders)
{
  switch (aBindVerb) {
    case BINDVERB_POST:
      mVerb = L"POST";
      break;
    case BINDVERB_PUT:
      mVerb = L"PUT";
      break;
    case BINDVERB_GET:
    default:
      mVerb = L"GET";
      break;
  };
  mResponse.Release();
  HRESULT hr = Response::createInstance(mResponse);
  if (FAILED(hr)) {
    return hr;
  }

  mHeaders.Release();
  mHeaders = Headers::createInstance(aHeaders, FALSE, hr);
  if (!mHeaders) {
    return hr;
  }

  return S_OK;
}

HRESULT Request::redirectRequest(IUri * aNewUri)
{
  mUri = aNewUri;
CComBSTR bs;
mUri->GetAbsoluteUri(&bs);
  return S_OK;
}

HRESULT Request::initResponse(RequestRecord & aRecord, LPCWSTR aHeaders, DWORD aResponseCode, HRESULT aOnResponseResult)
{
  ATLASSERT(mResponse);
  return static_cast<Response*>(mResponse.p)->initResponse(aRecord, aHeaders, aResponseCode, aOnResponseResult);
}

HRESULT Request::getHeaders(CStringW & aHeaders)
{
  ATLASSERT(mHeaders);
  return static_cast<Headers*>(mHeaders.p)->getHeaders(aHeaders);
}

HRESULT Request::setType(LPCWSTR aRequestType)
{
  // aRequestType can be NULL, in this case we use the type
  // we have already
  if (aRequestType) {
    mType = aRequestType;
  }
  return S_OK;
}

STDMETHODIMP Request::get_requestId(LONG * aRetVal)
{
  if (!aRetVal) {
    return E_POINTER;
  }
  (*aRetVal) = mRequestId;
  return S_OK;
}

STDMETHODIMP Request::get_currentBrowser(IWebBrowser2 ** aRetVal)
{
  if (!aRetVal) {
    return E_POINTER;
  }
  if (!mCurrentBrowser) {
    return E_UNEXPECTED;
  }
  return mCurrentBrowser.CopyTo(aRetVal);
}

STDMETHODIMP Request::get_uri(BSTR * aRetVal)
{
  if (!aRetVal) {
    return E_POINTER;
  }
  if (!mUri) {
    return E_UNEXPECTED;
  }
  return mUri->GetAbsoluteUri(aRetVal);
}

STDMETHODIMP Request::get_verb(BSTR * aRetVal)
{
  if (!aRetVal) {
    return E_POINTER;
  }
  (*aRetVal) = mVerb.AllocSysString();
  return S_OK;
}

STDMETHODIMP Request::get_headers(IHeaders ** aRetVal)
{
  if (!aRetVal) {
    return E_POINTER;
  }
  if (!mHeaders) {
    return E_UNEXPECTED;
  }
  return mHeaders.CopyTo(aRetVal);
}

STDMETHODIMP Request::get_response(IResponse ** aRetVal)
{
  if (!aRetVal) {
    return E_POINTER;
  }
  if (!mResponse) {
    return E_UNEXPECTED;
  }
  return mResponse.CopyTo(aRetVal);
}

STDMETHODIMP Request::get_requestType(BSTR * aRetVal)
{
  if (!aRetVal) {
    return E_POINTER;
  }
  (*aRetVal) = mType.AllocSysString();
  return S_OK;
}

STDMETHODIMP Request::isDocumentRequest()
{
  return (mIsDocument)
      ? S_OK
      : S_FALSE;
}

STDMETHODIMP Request::isSubFrame()
{
  return (mIsTopLevel)
      ? S_FALSE
      : S_OK;
}

STDMETHODIMP Request::redirect(BSTR aNewUri)
{
  mRedirectUri.Release();
  return ::CreateUri(aNewUri, Uri_CREATE_CANONICALIZE | Uri_CREATE_NO_DECODE_EXTRA_INFO, 0, &mRedirectUri);
}

STDMETHODIMP Request::cancel()
{
  mCanceled = TRUE;
  return S_OK;
}

STDMETHODIMP Request::getIUri(IUri ** aRetVal)
{
  if (!aRetVal) {
    return E_POINTER;
  }
  if (!mUri) {
    return E_UNEXPECTED;
  }
  return mUri.CopyTo(aRetVal);
}

STDMETHODIMP Request::getRedirectUri(IUri ** aRetVal)
{
  if (!aRetVal) {
    return E_POINTER;
  }
  if (!mRedirectUri) {
    return E_FAIL;
  }
  return mRedirectUri.CopyTo(aRetVal);
}

STDMETHODIMP Request::isCanceled()
{
  return (mCanceled) ? S_OK : S_FALSE;
}

} // namespace protocolpatchLib
