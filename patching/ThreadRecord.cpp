#include "StdAfx.h"
#include "ThreadRecord.h"

namespace protocolpatchLib
{

/*============================================================================
 * struct ComCallDataWebRequestEvents
 * This struct derives from ComCallData and contains additional members for
 * passing information to a IContextCallback::ContextCallback call.
 * This includes the current CAnchoRuntime instance as a IWebRequestEvents
 * and the current IRequest.
 * It also contains an enum for specifying the IWebRequestEvents method to.
 */
struct ComCallDataWebRequestEvents : public ComCallData
{
  enum {
    OnBeforeRequest = 1,
    OnBeforeSendHeaders,
    OnBeforeRedirect,
    OnHeadersReceived,
    OnInteractive,
    OnCompleted
  };
  ComCallDataWebRequestEvents(DWORD aDispId, IWebRequestEvents * aEvents, IRequest * aRequest) :
    mEvents(aEvents), mRequest(aRequest)
  {
    dwDispid = aDispId;
    dwReserved = 0;
    pUserDefined = NULL;
  }
  CComPtr<IWebRequestEvents> mEvents;
  CComPtr<IRequest> mRequest;
};

//----------------------------------------------------------------------------
// contextCall
// The function passed to IContextCallback::ContextCallback for IWebRequestEvents
// calls that don't happen on the document thread.
// It extracts information about the call from pParam and calls the method again,
// this time on the correct thread.
HRESULT _stdcall contextCall(ComCallData *pParam)
{
  ComCallDataWebRequestEvents * callback = (ComCallDataWebRequestEvents*) pParam;
  switch(callback->dwDispid) {
    case ComCallDataWebRequestEvents::OnBeforeRequest:
      return callback->mEvents->onBeforeRequest(callback->mRequest);
    case ComCallDataWebRequestEvents::OnBeforeSendHeaders:
      return callback->mEvents->onBeforeSendHeaders(callback->mRequest);
    case ComCallDataWebRequestEvents::OnBeforeRedirect:
      return callback->mEvents->onBeforeRedirect(callback->mRequest);
    case ComCallDataWebRequestEvents::OnHeadersReceived:
      return callback->mEvents->onHeadersReceived(callback->mRequest);
    case ComCallDataWebRequestEvents::OnInteractive:
      return callback->mEvents->onInteractive(callback->mRequest);
    case ComCallDataWebRequestEvents::OnCompleted:
      return callback->mEvents->onCompleted(callback->mRequest);
  }
  return S_OK;
}

// macro for marshalling a call back to the document thread. Used
// by the IWebRequestEvents implementaton of CAnchoRuntime.
// If the call does NOT happen on the document thread it invokes
// contextCall which in turn will call back on the document thread.
#define MARSHALL_CALL(callId) \
  if (mDocThreadId != ::GetCurrentThreadId()) { \
    ComCallDataWebRequestEvents cd(ComCallDataWebRequestEvents::callId, this, aRequest); \
    return mContextCallback->ContextCallback(&contextCall, &cd, IID_NULL, 0, NULL); \
  }

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
  map[record->getThreadId()] = record;
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

STDMETHODIMP_(ULONG) ThreadRecord::getThreadId()
{
  return mDocThreadId;
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
  return (0 == mEvents.size()) ? S_OK : S_FALSE;
}

STDMETHODIMP ThreadRecord::onBeforeRequest(IRequest * aRequest)
{
  MARSHALL_CALL(OnBeforeRequest);
  HRESULT hrRet = S_OK;
  for (auto it = mEvents.begin(); it != mEvents.end(); ++it) {
    HRESULT hr = it->second->onBeforeRequest(aRequest);
    hrRet = (FAILED(hr)) ? hr : hrRet;
  }
  return hrRet;
}

STDMETHODIMP ThreadRecord::onBeforeSendHeaders(IRequest * aRequest)
{
  MARSHALL_CALL(OnBeforeSendHeaders);
  HRESULT hrRet = S_OK;
  for (auto it = mEvents.begin(); it != mEvents.end(); ++it) {
    HRESULT hr = it->second->onBeforeSendHeaders(aRequest);
    hrRet = (FAILED(hr)) ? hr : hrRet;
  }
  return hrRet;
}

STDMETHODIMP ThreadRecord::onBeforeRedirect(IRequest * aRequest)
{
  MARSHALL_CALL(OnBeforeRedirect);
  HRESULT hrRet = S_OK;
  for (auto it = mEvents.begin(); it != mEvents.end(); ++it) {
    HRESULT hr = it->second->onBeforeRedirect(aRequest);
    hrRet = (FAILED(hr)) ? hr : hrRet;
  }
  return hrRet;
}

STDMETHODIMP ThreadRecord::onHeadersReceived(IRequest * aRequest)
{
  MARSHALL_CALL(OnHeadersReceived);
  HRESULT hrRet = S_OK;
  for (auto it = mEvents.begin(); it != mEvents.end(); ++it) {
    HRESULT hr = it->second->onHeadersReceived(aRequest);
    hrRet = (FAILED(hr)) ? hr : hrRet;
  }
  return hrRet;
}

STDMETHODIMP ThreadRecord::onInteractive(IRequest * aRequest)
{
  MARSHALL_CALL(OnInteractive);
  HRESULT hrRet = S_OK;
  for (auto it = mEvents.begin(); it != mEvents.end(); ++it) {
    HRESULT hr = it->second->onInteractive(aRequest);
    hrRet = (FAILED(hr)) ? hr : hrRet;
  }
  return hrRet;
}

STDMETHODIMP ThreadRecord::onCompleted(IRequest * aRequest)
{
  MARSHALL_CALL(OnCompleted);
  HRESULT hrRet = S_OK;
  for (auto it = mEvents.begin(); it != mEvents.end(); ++it) {
    HRESULT hr = it->second->onCompleted(aRequest);
    hrRet = (FAILED(hr)) ? hr : hrRet;
  }
  return hrRet;
}

} // namespace protocolpatchLib
