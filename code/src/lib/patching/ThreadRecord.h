/****************************************************************************
 * ThreadRecord.h : Declaration of ThreadRecord
 * Copyright 2013 Salsita Software (http://www.salsitasoft.com).
 * Author: Arne Seib <arne@salsitasoft.com>
 ****************************************************************************/

#pragma once

#include "GlobalMap.h"
#include "FrameRecord.h"
#include <map>
#include <libbhohelper.h>

namespace protocolpatchLib
{

class ThreadRecord;

typedef std::unordered_map< DWORD, CComPtr<IThreadRecord> > ThreadRecordMap;

// The global map storing browser objects by thread ID
typedef GlobalMapAccessor< ThreadRecordMap >
      GlobalThreadRecordMap;

/*============================================================================
 * class ThreadRecord
 *  A helper class for RequestRecordMap containing the browser and the
 *  URL the browser is initially navigating to.
 */
class ATL_NO_VTABLE ThreadRecord :
  public LIB_BhoHelper::ApartmentContext,
  public CComObjectRootEx<CComObjectThreadModel>,
  public IWebRequestEvents,
  public IThreadRecord
{
public:
  typedef CComObject< ThreadRecord > _ComObject;

  //--------------------------------------------------------------------------
  // static get/set methods
  static CComPtr<IThreadRecord> get();
  static CComPtr<IThreadRecord> create();
  static HRESULT remove();

  ThreadRecord() : mDocThreadId(0)
  {
    mDocThreadId = ::GetCurrentThreadId();
  }

  DECLARE_PROTECT_FINAL_CONSTRUCT()

  BEGIN_COM_MAP(ThreadRecord)
    COM_INTERFACE_ENTRY(IThreadRecord)
    COM_INTERFACE_ENTRY(IWebRequestEvents)
  END_COM_MAP()

  HRESULT FinalConstruct()
  {
    return S_OK;
  }

  void FinalRelease()
  {
  }

public:
  // -------------------------------------------------------------------------
  // IThreadRecord methods
  STDMETHOD(cleanup)();
  STDMETHOD(getToplevel)(IFrameRecord ** aRetVal);
  STDMETHOD(getCurrent)(IFrameRecord ** aRetVal);
  STDMETHOD_(ULONG, getThreadId)();
  STDMETHOD(getForUri)(IUri * aUri, IFrameRecord ** aRetVal);
  STDMETHOD(watchBrowser)(IWebBrowser2 * aBrowser, IWebRequestEvents * aEvents);
  STDMETHOD(unwatchBrowser)(IWebBrowser2 * aBrowser, IWebRequestEvents * aEvents);
  STDMETHOD(watchAll)(IWebRequestEvents * aEvents);
  STDMETHOD(unwatchAll)(IWebRequestEvents * aEvents);

  // -------------------------------------------------------------------------
  // IWebRequestEvents methods
  STDMETHOD(onBeforeRequest)(IRequest * aRequest);
  STDMETHOD(onBeforeSendHeaders)(IRequest * aRequest);
  STDMETHOD(onBeforeRedirect)(IRequest * aRequest);
  STDMETHOD(onHeadersReceived)(IRequest * aRequest);
  STDMETHOD(onInteractive)(IRequest * aRequest);
  STDMETHOD(onCompleted)(IRequest * aRequest);

private:
  typedef std::map<DWORD_PTR, CComPtr<IWebRequestEvents> > WebRequestEventsMap;

private:
  static CComPtr<IThreadRecord> createInstance();
  HRESULT init(IWebBrowser2 * aBrowser);

  DWORD mDocThreadId;
  CComPtr<IFrameRecord>       mTopLevelFrame;
  CComPtr<IFrameRecord>       mCurrentFrame;
  WebRequestEventsMap         mEvents;
  //CComPtr<IWebRequestEvents>  mEvents;
};

} // namespace protocolpatchLib
