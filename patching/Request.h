/****************************************************************************
 * Response.h : Declaration of Response
 * Copyright 2013 Salsita (http://www.salsitasoft.com).
 * Author: Arne Seib <arne@salsitasoft.com>
 ****************************************************************************/
#pragma once

#include "resource.h"       // main symbols
#include "ProtocolLibrary_i.h"

namespace protocolpatchLib
{

class RequestRecord;

// Request

class ATL_NO_VTABLE Request :
  public CComObjectRootEx<CComObjectThreadModel>,
  public CComCoClass<Request, &CLSID_Request>,
  public IDispatchImpl<IRequest, &IID_IRequest, &LIBID_protocolpatchLib, 0xffff, 0xffff>
{
public:
  typedef CComObject< Request > _ComObject;

  //--------------------------------------------------------------------------
  // static creator: can be replaced with own implmentation.
  static HRESULT createInstance(CComPtr<IRequest> & aRetVal);

  Request() : mRequestId(0), mIsDocument(FALSE), mIsTopLevel(FALSE), mCanceled(FALSE)
    {}

  //DECLARE_REGISTRY_RESOURCEID(IDR_REQUEST)
  DECLARE_NO_REGISTRY()
  DECLARE_NOT_AGGREGATABLE(Request)
  DECLARE_PROTECT_FINAL_CONSTRUCT()

  BEGIN_COM_MAP(Request)
    COM_INTERFACE_ENTRY(IRequest)
    COM_INTERFACE_ENTRY(IDispatch)
  END_COM_MAP()

  HRESULT FinalConstruct()
    { return S_OK; }

  void FinalRelease()
    { }


  HRESULT initRequest(RequestRecord & aRecord);
  HRESULT beforeSendingRequest(RequestRecord & aRecord, DWORD aBindVerb, LPCWSTR aHeaders);
  HRESULT redirectRequest(IUri * aNewUri);
  HRESULT initResponse(RequestRecord & aRecord, LPCWSTR aHeaders, DWORD aResponseCode, HRESULT aOnResponseResult);
  HRESULT getHeaders(CStringW & aHeaders);
  HRESULT setType(LPCWSTR aRequestType);

  STDMETHOD(get_requestId)(LONG * aRetVal);
  STDMETHOD(get_currentBrowser)(IWebBrowser2 ** aRetVal);
  STDMETHOD(get_headers)(IHeaders ** aRetVal);
  STDMETHOD(get_uri)(BSTR * aRetVal);
  STDMETHOD(get_verb)(BSTR * aRetVal);
  STDMETHOD(get_response)(IResponse ** aRetVal);
  STDMETHOD(get_requestType)(BSTR * aRetVal);
  STDMETHOD(isDocumentRequest)();
  STDMETHOD(isSubFrame)();
  STDMETHOD(redirect)(BSTR aNewUri);
  STDMETHOD(cancel)();
  STDMETHOD(getIUri)(IUri ** aRetVal);
  STDMETHOD(getRedirectUri)(IUri ** aRetVal);
  STDMETHOD(isCanceled)();

private:
  LONG mRequestId;
  BOOL mIsDocument;
  BOOL mIsTopLevel;
  BOOL mCanceled;
  CStringW  mVerb;
  CStringW  mType;
  CComPtr<IUri> mUri;
  CComPtr<IUri> mRedirectUri;
  CComPtr<IWebBrowser2> mCurrentBrowser;
  CComPtr<IHeaders> mHeaders;
  CComPtr<IResponse> mResponse;
};

OBJECT_ENTRY_NON_CREATEABLE_EX_AUTO(CLSID_Request, Request)


} // namespace protocolpatchLib
