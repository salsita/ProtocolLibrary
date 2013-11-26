#include "StdAfx.h"
#include "ThreadRecord.h"

//#define REQUEST_EVENT_SRC_USE_DEFAULT_SINK

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
  mEvents.Release();
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

  mEvents = aEvents;
  return S_OK;
}

STDMETHODIMP ThreadRecord::unwatchBrowser(IWebBrowser2 * aBrowser, IWebRequestEvents * aEvents)
{
  mTopLevelFrame.Release();
  mCurrentFrame.Release();
  mEvents.Release();
  return S_OK;
}

STDMETHODIMP ThreadRecord::onBeforeRequest(IRequest * aRequest)
{
  return (mEvents) ? mEvents->onBeforeRequest(aRequest) : S_OK;
}

STDMETHODIMP ThreadRecord::onBeforeSendHeaders(IRequest * aRequest)
{
  return (mEvents) ? mEvents->onBeforeSendHeaders(aRequest) : S_OK;
}

STDMETHODIMP ThreadRecord::onBeforeRedirect(IRequest * aRequest)
{
  HRESULT hr = (mEvents) ? mEvents->onBeforeRedirect(aRequest) : S_OK;
  return hr;
}

STDMETHODIMP ThreadRecord::onHeadersReceived(IRequest * aRequest)
{
  return (mEvents) ? mEvents->onHeadersReceived(aRequest) : S_OK;
}

STDMETHODIMP ThreadRecord::onInteractive(IRequest * aRequest)
{
  return (mEvents) ? mEvents->onInteractive(aRequest) : S_OK;
}

STDMETHODIMP ThreadRecord::onCompleted(IRequest * aRequest)
{
  return (mEvents) ? mEvents->onCompleted(aRequest) : S_OK;
}

} // namespace protocolpatchLib
