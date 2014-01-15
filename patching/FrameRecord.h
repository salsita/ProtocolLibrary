/****************************************************************************
 * FrameRecord.h : Declaration of FrameRecord
 * Copyright 2013 Salsita (http://www.salsitasoft.com).
 * Author: Arne Seib <arne@salsitasoft.com>
 ****************************************************************************/
#pragma once

#include "GlobalMap.h"
#include "Request.h"
#include "Response.h"
#include "RequestRecord.h"
#include "Application.h"

namespace protocolpatchLib
{

/*============================================================================
 * class FrameRecord
 *  Contains information about a frame like browser, current URI and event
 *  handler.
 *  For a top level frame it handles DWebBrowserEvents2::OnBeforeNavigate2
 *  to update the FrameRecord for top level and current request.
 */

class FrameRecord;

//--------------------------------------------------------------------------
// WebBrowser-Event-Implementation
typedef IDispEventImpl<1, FrameRecord, &DIID_DWebBrowserEvents2, &LIBID_SHDocVw, 1, 0> TWebBrowserEvents;

//--------------------------------------------------------------------------
// FrameRecord
class ATL_NO_VTABLE FrameRecord :
  public CComObjectRootEx<CComObjectThreadModel>,
  public IFrameRecord,
  public TWebBrowserEvents
{
public:
  typedef CComObject< FrameRecord > _ComObject;

  //--------------------------------------------------------------------------
  // static methods
  static CComPtr<IFrameRecord> createInstance(
      IWebRequestEvents * aEventSink,
      IWebBrowser2 * aBrowser,
      BOOL aIsTopLevel);

  static HRESULT createUriNoFragment(IUri * aUri, IUri ** aRetVal);

  //--------------------------------------------------------------------------
  // usual stuff
  FrameRecord() : mIsTopLevel(FALSE)
  {
  }

  DECLARE_PROTECT_FINAL_CONSTRUCT()

  BEGIN_COM_MAP(FrameRecord)
    COM_INTERFACE_ENTRY(IFrameRecord)
  END_COM_MAP()

  BEGIN_SINK_MAP(FrameRecord)
    SINK_ENTRY_EX(1, DIID_DWebBrowserEvents2, DISPID_BEFORENAVIGATE2, OnBeforeNavigate2)
  END_SINK_MAP()

  HRESULT FinalConstruct()
  {
    return S_OK;
  }

  void FinalRelease()
  {
  }

public:
  // -------------------------------------------------------------------------
  // IFrameRecord methods
  STDMETHOD(cleanup)();
  STDMETHOD(beforeNavigate)(LPCWSTR aUrl, IWebBrowser2 * aBrowser);
  STDMETHOD(setUri)(LPCWSTR aNewUri);
  STDMETHOD(getBrowser)(IWebBrowser2 ** aRetVal);
  STDMETHOD(getUri)(IUri ** aRetVal);
  STDMETHOD(getSink)(IWebRequestEvents ** aRetVal);
  STDMETHOD(isTopLevel)();
  STDMETHOD(isEqualUri)(IUri * aUri);

  // -------------------------------------------------------------------------
  // _DWebBrowserEvents2 methods
  STDMETHOD_(void, OnBeforeNavigate2)(LPDISPATCH pDisp, VARIANT *pURL, VARIANT *Flags,
    VARIANT *TargetFrameName, VARIANT *PostData, VARIANT *Headers, BOOL *Cancel);

private:
  // -------------------------------------------------------------------------
  // private methods
  HRESULT init(IWebRequestEvents * aEventSink, IWebBrowser2 * aBrowser, BOOL aIsTopLevel);

private:
#ifdef _DEBUG
  CString mUrlString;
#endif
  CComPtr<IWebBrowser2>   mBrowser;
  CComPtr<IUri>           mUri;
  CComPtr<IWebRequestEvents> mEvents;
  BOOL                    mIsTopLevel;
};

} // namespace protocolpatchLib
