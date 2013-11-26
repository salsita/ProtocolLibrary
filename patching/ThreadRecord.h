/****************************************************************************
 * ThreadRecord.h : Declaration of ThreadRecord
 * Copyright 2013 Salsita (http://www.salsitasoft.com).
 * Author: Arne Seib <arne@salsitasoft.com>
 ****************************************************************************/

#pragma once

#include "GlobalMap.h"
#include "FrameRecord.h"

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

  ThreadRecord()
  {
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
    int asd = 0;
	}

public:
  // -------------------------------------------------------------------------
  // IThreadRecord methods
  STDMETHOD(cleanup)();
  STDMETHOD(getToplevel)(IFrameRecord ** aRetVal);
  STDMETHOD(getCurrent)(IFrameRecord ** aRetVal);
  STDMETHOD(getForUri)(IUri * aUri, IFrameRecord ** aRetVal);
  STDMETHOD(watchBrowser)(IWebBrowser2 * aBrowser, IWebRequestEvents * aEvents);
  STDMETHOD(unwatchBrowser)(IWebBrowser2 * aBrowser, IWebRequestEvents * aEvents);

  // -------------------------------------------------------------------------
  // IWebRequestEvents methods
  STDMETHOD(onBeforeRequest)(IRequest * aRequest);
  STDMETHOD(onBeforeSendHeaders)(IRequest * aRequest);
  STDMETHOD(onBeforeRedirect)(IRequest * aRequest);
  STDMETHOD(onHeadersReceived)(IRequest * aRequest);
  STDMETHOD(onInteractive)(IRequest * aRequest);
  STDMETHOD(onCompleted)(IRequest * aRequest);

private:
  static CComPtr<IThreadRecord> createInstance();
  HRESULT init(IWebBrowser2 * aBrowser);

  CComPtr<IFrameRecord>   mTopLevelFrame;
  CComPtr<IFrameRecord>   mCurrentFrame;
  CComPtr<IWebRequestEvents> mEvents;
};

} // namespace protocolpatchLib
