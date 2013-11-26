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

// Response

class ATL_NO_VTABLE Response :
	public CComObjectRootEx<CComSingleThreadModel>,
	public CComCoClass<Response, &CLSID_Response>,
	public IDispatchImpl<IResponse, &IID_IResponse, &LIBID_protocolpatchLib, 0xffff, 0xffff>
{
public:
  typedef CComObject< Response > _ComObject;

  //--------------------------------------------------------------------------
  // static creator
  static HRESULT createInstance(CComPtr<IResponse> & aRetVal);

  Response() : mRequestId(0), mResponseCode(0)
	  {}

  //DECLARE_REGISTRY_RESOURCEID(IDR_RESPONSE)
  DECLARE_NO_REGISTRY()
  DECLARE_NOT_AGGREGATABLE(Response)
	DECLARE_PROTECT_FINAL_CONSTRUCT()

  BEGIN_COM_MAP(Response)
	  COM_INTERFACE_ENTRY(IResponse)
	  COM_INTERFACE_ENTRY(IDispatch)
  END_COM_MAP()

	HRESULT FinalConstruct()
    { return S_OK; }

	void FinalRelease()
    { }

  HRESULT initResponse(RequestRecord & aRecord, LPCWSTR aHeaders, DWORD aResponseCode, HRESULT aOnResponseResult);

  STDMETHOD(get_result)(ULONG * aRetVal);
  STDMETHOD(get_headers)(IDispatchEx ** aRetVal);

private:
  LONG mRequestId;
  DWORD mResponseCode;
  CComPtr<IDispatchEx> mHeaders;
};

OBJECT_ENTRY_NON_CREATEABLE_EX_AUTO(CLSID_Response, Response)


} // namespace protocolpatchLib
