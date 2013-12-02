#include "StdAfx.h"
#include "ThreadRecord.h"

namespace protocolpatchLib
{

CComPtr<IThreadRecord> ThreadRecord::createInstance()
{
  _ComObject * newInstance = NULL;
  HRESULT hr = _ComObject::CreateInstance(&newInstance);

  if (SUCCEEDED(hr)) {
    return newInstance;
  }
  return NULL;
}

CComPtr<IThreadRecord> ThreadRecord::get()
{
  GlobalThreadRecordMap map;
  GlobalThreadRecordMap::iterator it = map.find(::GetCurrentThreadId());
  return (it != map.end()) ? it->second : NULL;
}

CComPtr<IThreadRecord> ThreadRecord::create()
{
  GlobalThreadRecordMap map;
  CComPtr<IThreadRecord> record = get();
  if (record) {
    return record;
  }
  record = createInstance();
  map[::GetCurrentThreadId()] = record;
  return record;
}

HRESULT ThreadRecord::remove()
{
  GlobalThreadRecordMap map;
  GlobalThreadRecordMap::iterator it = map.find(::GetCurrentThreadId());
  if  (it == map.end()) {
    return S_OK;
  }
  it->second->cleanup();
  map.erase(it);
  return S_OK;
}

STDMETHODIMP ThreadRecord::cleanup()
{
  CComPtr<IThreadRecord> lifeGuard(this);
  if (mTopLevelFrame) {
    mTopLevelFrame->cleanup();
    mTopLevelFrame.Release();
  }
  if (mCurrentFrame) {
    mCurrentFrame->cleanup();
    mCurrentFrame.Release();
  }
  mEvents.clear();
  return S_OK;
}

STDMETHODIMP ThreadRecord::getToplevel(IFrameRecord ** aRetVal)
{
  return mTopLevelFrame.CopyTo(aRetVal);
}

STDMETHODIMP ThreadRecord::getCurrent(IFrameRecord ** aRetVal)
{
  return mCurrentFrame.CopyTo(aRetVal);
}

STDMETHODIMP ThreadRecord::getForUri(IUri * aUri, IFrameRecord ** aRetVal)
{
  if (mTopLevelFrame && (S_OK == mTopLevelFrame->isEqualUri(aUri))) {
    return mTopLevelFrame.CopyTo(aRetVal);
  }
  if (mCurrentFrame && (S_OK == mCurrentFrame->isEqualUri(aUri))) {
    return mCurrentFrame.CopyTo(aRetVal);
  }
  return E_FAIL;
}

STDMETHODIMP ThreadRecord::watchBrowser(IWebBrowser2 * aBrowser, IWebRequestEvents * aEvents)
{
  if (!aEvents) {
    return E_INVALIDARG;
  }
  if (!aBrowser) {
    return E_INVALIDARG;
  }
  mTopLevelFrame = FrameRecord::createInstance(this, aBrowser, TRUE);
  mCurrentFrame = FrameRecord::createInstance(this, aBrowser, FALSE);
  if (!mTopLevelFrame || !mCurrentFrame) {
    return E_FAIL;
  }

  return watchAll(aEvents);
}

STDMETHODIMP ThreadRecord::unwatchBrowser(IWebBrowser2 * aBrowser, IWebRequestEvents * aEvents)
{
  mTopLevelFrame.Release();
  mCurrentFrame.Release();
  return unwatchAll(aEvents);
}

STDMETHODIMP ThreadRecord::watchAll(IWebRequestEvents * aEvents)
{
  if (!aEvents) {
    return E_INVALIDARG;
  }
  mEvents[(DWORD_PTR)aEvents] = aEvents;
  return S_OK;
}

STDMETHODIMP ThreadRecord::unwatchAll(IWebRequestEvents * aEvents)
{
  mEvents.erase((DWORD_PTR)aEvents);
  return S_OK;
}

STDMETHODIMP ThreadRecord::onBeforeRequest(IRequest * aRequest)
{
  HRESULT hrRet = S_OK;
  for (auto it = mEvents.begin(); it != mEvents.end(); ++it) {
    HRESULT hr = it->second->onBeforeRequest(aRequest);
    hrRet = (FAILED(hr)) ? hr : hrRet;
  }
  return hrRet;
}

STDMETHODIMP ThreadRecord::onBeforeSendHeaders(IRequest * aRequest)
{
  HRESULT hrRet = S_OK;
  for (auto it = mEvents.begin(); it != mEvents.end(); ++it) {
    HRESULT hr = it->second->onBeforeSendHeaders(aRequest);
    hrRet = (FAILED(hr)) ? hr : hrRet;
  }
  return hrRet;
}

STDMETHODIMP ThreadRecord::onBeforeRedirect(IRequest * aRequest)
{
  HRESULT hrRet = S_OK;
  for (auto it = mEvents.begin(); it != mEvents.end(); ++it) {
    HRESULT hr = it->second->onBeforeRedirect(aRequest);
    hrRet = (FAILED(hr)) ? hr : hrRet;
  }
  return hrRet;
}

STDMETHODIMP ThreadRecord::onHeadersReceived(IRequest * aRequest)
{
  HRESULT hrRet = S_OK;
  for (auto it = mEvents.begin(); it != mEvents.end(); ++it) {
    HRESULT hr = it->second->onHeadersReceived(aRequest);
    hrRet = (FAILED(hr)) ? hr : hrRet;
  }
  return hrRet;
}

STDMETHODIMP ThreadRecord::onInteractive(IRequest * aRequest)
{
  HRESULT hrRet = S_OK;
  for (auto it = mEvents.begin(); it != mEvents.end(); ++it) {
    HRESULT hr = it->second->onInteractive(aRequest);
    hrRet = (FAILED(hr)) ? hr : hrRet;
  }
  return hrRet;
}

STDMETHODIMP ThreadRecord::onCompleted(IRequest * aRequest)
{
  HRESULT hrRet = S_OK;
  for (auto it = mEvents.begin(); it != mEvents.end(); ++it) {
    HRESULT hr = it->second->onCompleted(aRequest);
    hrRet = (FAILED(hr)) ? hr : hrRet;
  }
  return hrRet;
}

} // namespace protocolpatchLib
