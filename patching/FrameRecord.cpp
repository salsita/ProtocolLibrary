/****************************************************************************
 * FrameRecord.cpp : Implementation of FrameRecord
 * Copyright 2013 Salsita Software (http://www.salsitasoft.com).
 * Author: Arne Seib <arne@salsitasoft.com>
 ****************************************************************************/

#include "StdAfx.h"
#include "RequestRecord.h"
#include "FrameRecord.h"
#include "ThreadRecord.h"

namespace protocolpatchLib
{

#define FrameRecord_TRACE(...) \
  ATLTRACE(__FUNCTION__); \
  ATLTRACE(_T(": ")); \
  ATLTRACE(__VA_ARGS__); \
  ATLTRACE(_T("\n"));

//--------------------------------------------------------------------------
// createInstance
//  static creator method
CComPtr<IFrameRecord> FrameRecord::createInstance(
    IWebRequestEvents * aEventSink,
      IWebBrowser2 * aBrowser,
      BOOL aIsTopLevel)
{
  _ComObject * newInstance = NULL;
  if (SUCCEEDED(_ComObject::CreateInstance(&newInstance))) {
    CComPtr<IFrameRecord> owner(newInstance);
    if (SUCCEEDED(newInstance->init(aEventSink, aBrowser, aIsTopLevel))) {
      return owner;
    }
    newInstance->cleanup();
  }
  return NULL;
}

//--------------------------------------------------------------------------
// createUriNoFragment
//  Create a URI, but remove fragment.
HRESULT FrameRecord::createUriNoFragment(IUri * aUri, IUri ** aRetVal)
{
  CComPtr<IUriBuilder> uriBuilder;
  HRESULT hr = CreateIUriBuilder(aUri, 0, 0, &uriBuilder.p);
  if (FAILED(hr)) {
    return hr;
  }
  uriBuilder->SetFragment(NULL);
  return uriBuilder->CreateUri(Uri_CREATE_CANONICALIZE, 0, 0, aRetVal);
}

//--------------------------------------------------------------------------
// init
//  Called after creation, usually from ThreadRecord
HRESULT FrameRecord::init(IWebRequestEvents * aEventSink, IWebBrowser2 * aBrowser, BOOL aIsTopLevel)
{
  if (!aBrowser) {
    return E_INVALIDARG;
  }
  cleanup();
  FrameRecord_TRACE(_T("Top-Level: %i"), aIsTopLevel)

  mEvents = aEventSink;
  mBrowser = aBrowser;
  mIsTopLevel = aIsTopLevel;

  if (!aIsTopLevel) {
    return S_OK;
  }
  return DispEventAdvise(mBrowser);
}

//--------------------------------------------------------------------------
// cleanup
//  Important method: Since we have a lot of circular references
//  this method (and its equivalents in ThreadRecord etc) releases them
//  properly.
STDMETHODIMP FrameRecord::cleanup()
{
  FrameRecord_TRACE(_T("Top-Level: %i"), mIsTopLevel)
  // keep us alive while we work on members
  CComPtr<IFrameRecord> lifeGuard(this);

  if (mBrowser && mIsTopLevel) {
    DispEventUnadvise(mBrowser);
  }

  mBrowser.Release();
  mUri.Release();
  mEvents.Release();
  mIsTopLevel = FALSE;

  return S_OK;
}

//--------------------------------------------------------------------------
// beforeNavigate
//  Called from within OnBeforeNavigate2() to update our internal data
STDMETHODIMP FrameRecord::beforeNavigate(LPCWSTR aUrl, IWebBrowser2 * aBrowser)
{
  if (aBrowser) {
    mBrowser = aBrowser;
  }
  return setUri(aUrl);
}

//--------------------------------------------------------------------------
// setUri
STDMETHODIMP FrameRecord::setUri(LPCWSTR aNewUri)
{
  CComPtr<IUri> uri;
  HRESULT hr = ::CreateUri(aNewUri, Uri_CREATE_CANONICALIZE, 0, &uri);
  if (FAILED(hr)) {
    // NOTE: A failure might happen here. File URLs from
    // OnBrowserBeforeNavigate2 for example are passed
    // WITHOUT scheme, so the raw file path arrives here.
    // In this case CreateUri returns E_INVALIDARG. That's ok,
    // because in file urls we are anyway not interested.
    // If you plan to implement file:// URLs or any other URLs that
    // will fail parsing you have to change this behaviour.
    return hr;
  }
  hr = createUriNoFragment(uri, &mUri.p);
  if (FAILED(hr)) {
    return hr;
  }
#ifdef _DEBUG
  CComBSTR bs;
  mUri->GetAbsoluteUri(&bs);
  mUrlString = bs;
  FrameRecord_TRACE(_T("Top-Level: %i, URL: %s"), mIsTopLevel, mUrlString)
#endif
  return S_OK;
}

//--------------------------------------------------------------------------
// getBrowser
//  Returns the current IWebBrowser2 object.
STDMETHODIMP FrameRecord::getBrowser(IWebBrowser2 ** aRetVal)
{
  return mBrowser.CopyTo(aRetVal);
}

//--------------------------------------------------------------------------
// getUri
//  Returns the current IUri object.
STDMETHODIMP FrameRecord::getUri(IUri ** aRetVal)
{
  return mUri.CopyTo(aRetVal);
}

//--------------------------------------------------------------------------
// getSink
//  Returns the current IWebRequestEvents object.
STDMETHODIMP FrameRecord::getSink(IWebRequestEvents ** aRetVal)
{
  return mEvents.CopyTo(aRetVal);
}

//--------------------------------------------------------------------------
// isTopLevel
//  Returns S_OK if this is a top level frame, S_FALSE otherwise.
STDMETHODIMP FrameRecord::isTopLevel()
{
  return (mIsTopLevel) ? S_OK : S_FALSE;
}

//--------------------------------------------------------------------------
// isEqualUri
//  Checks whether aUri is equal to our URI - ignores the fragment.
//  Used for detection of a refresh.
STDMETHODIMP FrameRecord::isEqualUri(IUri * aUri)
{
  CComPtr<IUri> uri;
  createUriNoFragment(aUri, &uri.p);
  BOOL isEqual = FALSE;
  uri->IsEqual(mUri, &isEqual);
  return (isEqual) ? S_OK : S_FALSE;
}

//--------------------------------------------------------------------------
// OnBeforeNavigate2
//  DWebBrowserEvents2::OnBeforeNavigate2 handler.
STDMETHODIMP_(void) FrameRecord::OnBeforeNavigate2(LPDISPATCH pDisp, VARIANT *pURL, VARIANT *Flags,
  VARIANT *TargetFrameName, VARIANT *PostData, VARIANT *Headers, BOOL *Cancel)
{
  // sanity
  CComQIPtr<IWebBrowser2> browser(pDisp);
  ATLASSERT(browser);
  ATLASSERT(pURL);
  if ( !browser || !pURL || (VT_BSTR != pURL->vt) ) {
    return;
  }
  FrameRecord_TRACE(_T("URL: %s"), pURL->bstrVal)

  if (mBrowser.IsEqualObject(browser)) {
    // There is only one FrameRecord attached to the browser events:
    // The record for the top level browser.
    // Means, if we receive this event and our browser is the same
    // like the browser passed here, this is a request for the
    // top level frame - means, for us. So we update our data as well.
    beforeNavigate(pURL->bstrVal, NULL);
    // And fall through to initialize also
    // the current frame.
  }

  // get or create the ThreadRecord for the current
  // thread
  CComPtr<IThreadRecord> threadRecord = ThreadRecord::get();
  ATLASSERT(threadRecord);
  if (!threadRecord) {
    return;
  }

  // now get the FrameRecord for the CURRENT frame
  CComPtr<IFrameRecord> frameRecord;
  threadRecord->getCurrent(&frameRecord.p);
  if (frameRecord) {
    // and update the FrameRecord
    frameRecord->beforeNavigate(pURL->bstrVal, browser);
  }
}

} // namespace protocolpatchLib
