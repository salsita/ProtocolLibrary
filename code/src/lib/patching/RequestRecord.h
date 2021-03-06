/****************************************************************************
 * Request.h : Declaration of Request
 * Copyright 2013 Salsita Software (http://www.salsitasoft.com).
 * Author: Arne Seib <arne@salsitasoft.com>
 ****************************************************************************/

#pragma once

#include "GlobalMap.h"
#include "Request.h"
#include "Response.h"
#include "FrameRecord.h"
#include "Application.h"
#include <memory>

namespace protocolpatchLib
{

  /*============================================================================
 * class RequestRecord
 *  A helper class for RequestRecordMap containing the browser and the
 *  URL the browser is initially navigating to.
 */
class RequestRecord
{
public:
  enum EState {
    FRESH,
    STARTED,
    SENDING,
    RECEIVED,
    DOCUMENTINTERACTIVE,
    DOCUMENTCOMPLETE
  };

  RequestRecord();
  //virtual ~RequestRecord();

  // events
  HRESULT fire_onBeforeRequest(LPCWSTR aRequestType, IUri ** aRedirectUri);
  HRESULT fire_onBeforeSendHeaders(DWORD aBindVerb, CStringW & aHeaders);
  HRESULT fire_onBeforeRedirect(LPCWSTR aNewUrl);
  HRESULT fire_onHeadersReceived(DWORD aResponseCode, LPCWSTR aHeaders, HRESULT aOnResponseResult);
  HRESULT fire_onInteractive();
  HRESULT fire_onCompleted();

  HRESULT fire_onDocumentReadyState(LPCWSTR aReadyState);

  //HRESULT reset();

  //HRESULT prepareRequest(IWebBrowser2* aTopLevelBrowser, IWebBrowser2* aCurrentBrowser, IUri * aUri, IWebRequestEvents * aBrowserEvents);
  HRESULT initRequest(IFrameRecord * aFrameRecord, IUri * aUri);

  // getters / setters
  DECLARE_GETTER(LONG, id) { return mRequestId; }
  DECLARE_GETTER(EState, state) { return mState; }
  DECLARE_GETTER(CComPtr<IFrameRecord>, frameRecord) { return mFrameRecord; }
  DECLARE_GETTER(CComPtr<IRequest>, request) { return mRequest; }

  HRESULT getUri(IUri ** aUriRet);
  CComPtr<IWebRequestEvents> getSink(HRESULT & hr);  // can return a global sink object, see implementation

private:
  BOOL shouldNotify();
  HRESULT setCurrentUri(IUri * aUri);
  static LONG sRequestId;

  LONG    mRequestId;
  EState  mState;

  CComPtr<IRequest>       mRequest;
  CComPtr<IFrameRecord>   mFrameRecord;
  CComPtr<IUri>           mUri;
#ifdef _DEBUG
public:
  CString mUrlString;
#endif
};

typedef std::shared_ptr<RequestRecord> RequestRecordPtr;
typedef std::unordered_map<DWORD, RequestRecordPtr> RequestRecordPtrMap;

// The global map storing browser objects by thread ID
typedef GlobalMapAccessor< RequestRecordPtrMap >
      RequestRecordMap;

} // namespace protocolpatchLib
