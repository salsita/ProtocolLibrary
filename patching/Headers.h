/****************************************************************************
 * Headers.h : Declaration of Headers
 * Copyright 2013 Salsita (http://www.salsitasoft.com).
 * Author: Arne Seib <arne@salsitasoft.com>
 ****************************************************************************/
#pragma once

#include "resource.h"       // main symbols
#include <string>
#include <map>
#include <unordered_map>
#include "IDispatchExImpl.h"
#include "ProtocolLibrary_i.h"
#include "http_parser.h"

namespace protocolpatchLib
{

/*============================================================================
 * class Headers
 *  Dictionary object, containing HTTP headers
 */
class ATL_NO_VTABLE Headers :
	public CComObjectRootEx<CComSingleThreadModel>,
  public IDispatchExImpl<Headers>,
  public IHeaders
{
public:
  friend IDispatchExImpl<Headers>;
  typedef CComObject< Headers > _ComObject;

  //--------------------------------------------------------------------------
  // static creator
  static CComPtr<IDispatchEx> createInstance(
      LPCWSTR aHeaders,
      BOOL aIsResponse,
      HRESULT & hr);

  //--------------------------------------------------------------------------
  // usual stuff
	Headers()
	{
	}

  DECLARE_NO_REGISTRY()
  DECLARE_NOT_AGGREGATABLE(Headers)
	DECLARE_PROTECT_FINAL_CONSTRUCT()

  BEGIN_COM_MAP(Headers)
	  COM_INTERFACE_ENTRY(IHeaders)
	  COM_INTERFACE_ENTRY(IDispatch)
	  COM_INTERFACE_ENTRY(IDispatchEx)
  END_COM_MAP()

  HRESULT FinalConstruct()
	{
		return S_OK;
	}

	void FinalRelease()
	{
    LOCKED_BLOCK();
    mNames.clear();
    mValues.clear();
    mNameIDs.clear();
	}

public:
  //--------------------------------------------------------------------------
  // public methods
  HRESULT getHeaders(CStringW & aHeaders);

  STDMETHOD(get)(BSTR aName, BSTR * aRetVal);
  STDMETHOD(set)(BSTR aName, BSTR aValue);

protected:
  //--------------------------------------------------------------------------
  // protected methods
  HRESULT putValue(
    DISPID    aId,
    VARIANT * aProperty)
  {
    CComVariant vt;
    vt.ChangeType(VT_BSTR, aProperty);
    if (VT_BSTR != vt.vt) {
      // accept only strings
      return DISP_E_TYPEMISMATCH;
    }
    return IDispatchExImpl<Headers>::putValue(aId, &vt);
  }

private:
  //--------------------------------------------------------------------------
  // private methods
  static int onHeaderField(http_parser * aParser, const char *aField, size_t aLength);
  static int onHeaderValue(http_parser * aParser, const char *aField, size_t aLength);
  HRESULT parse(LPCWSTR aHeaders, BOOL aIsResponse);

};

} // namespace protocolpatchLib
