/****************************************************************************
 * Headers.h : Declaration of Headers
 * Copyright 2013 Salsita (http://www.salsitasoft.com).
 * Author: Arne Seib <arne@salsitasoft.com>
 ****************************************************************************/
#pragma once

#include "resource.h"       // main symbols
#include <string>
#include <map>
#include <vector>
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
class ATL_NO_VTABLE KVPair :
	public CComObjectRootEx<CComSingleThreadModel>,
	public IDispatchImpl<IKVPair, &IID_IKVPair, &LIBID_protocolpatchLib, 0xffff, 0xffff>
{
public:
  typedef CComObject< KVPair > _ComObject;

  //--------------------------------------------------------------------------
  // static creator
  static CComPtr<IKVPair> createInstance(LPCWSTR aKey, LPCWSTR aValue, HRESULT & hr)
  {
    _ComObject * newInstance = NULL;
    hr = _ComObject::CreateInstance(&newInstance);
    if (SUCCEEDED(hr)) {
      CComPtr<IKVPair> owner(newInstance);
      newInstance->mKey = aKey;
      newInstance->mVal = aValue;
      return owner;
    }
    return NULL;
  }

  //--------------------------------------------------------------------------
  // usual stuff
	KVPair()
	{
	}

  DECLARE_NO_REGISTRY()
	DECLARE_PROTECT_FINAL_CONSTRUCT()

  BEGIN_COM_MAP(KVPair)
	  COM_INTERFACE_ENTRY(IKVPair)
	  COM_INTERFACE_ENTRY(IDispatch)
  END_COM_MAP()

  HRESULT FinalConstruct()
	{
		return S_OK;
	}

	void FinalRelease()
	{
	}

public:
  //--------------------------------------------------------------------------
  // IKVPair methods
  STDMETHOD(get_key)(BSTR * aRetVal)
  {
    if (!aRetVal) {
      return E_POINTER;
    }
    (*aRetVal) = mKey.AllocSysString();
    return S_OK;
  }

  STDMETHOD(put_key)(BSTR aVal)
  {
    mKey = aVal;
    return S_OK;
  }

  STDMETHOD(get_val)(BSTR * aRetVal)
  {
    if (!aRetVal) {
      return E_POINTER;
    }
    (*aRetVal) = mVal.AllocSysString();
    return S_OK;
  }

  STDMETHOD(put_val)(BSTR aVal)
  {
    mVal = aVal;
    return S_OK;
  }

  STDMETHOD(set)(BSTR aKey, BSTR aVal)
  {
    mKey = aKey;
    mVal = aVal;
    return S_OK;
  }

  STDMETHOD(get)(BSTR * aKeyOut, BSTR * aValOut)
  {
    if (!aKeyOut || !aValOut) {
      return E_POINTER;
    }
    (*aKeyOut) = mKey.AllocSysString();
    (*aValOut) = mVal.AllocSysString();
    return S_OK;
  }

private:
  //--------------------------------------------------------------------------
  // private members
  CStringW mKey;
  CStringW mVal;
};






/*============================================================================
 * class Headers
 *  Dictionary object, containing HTTP headers
 */
class ATL_NO_VTABLE Headers :
	public CComObjectRootEx<CComSingleThreadModel>,
	public IDispatchImpl<IHeaders, &IID_IHeaders, &LIBID_protocolpatchLib, 0xffff, 0xffff>
{
public:
  typedef CComObject< Headers > _ComObject;

  //--------------------------------------------------------------------------
  // static creator
  static CComPtr<IHeaders> createInstance(
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
  END_COM_MAP()

  HRESULT FinalConstruct()
	{
		return S_OK;
	}

	void FinalRelease()
	{
	}

public:
  //--------------------------------------------------------------------------
  // public methods
  HRESULT getHeaders(CStringW & aHeaders);

  STDMETHOD(get_length)(ULONG * aRetVal);
  //STDMETHOD(item)(ULONG aIndex);
  STDMETHOD(forEach)(IDispatch * aCallback, VARIANT aThis);

private:
  //--------------------------------------------------------------------------
  // private types
  typedef std::vector< CComPtr<IKVPair> >  KVPairVector;

private:
  //--------------------------------------------------------------------------
  // private methods
  static int onHeaderField(http_parser * aParser, const char *aField, size_t aLength);
  static int onHeaderValue(http_parser * aParser, const char *aField, size_t aLength);
  HRESULT parse(LPCWSTR aHeaders, BOOL aIsResponse);

private:
  //--------------------------------------------------------------------------
  // private members

  // for parsing
  struct {
    CStringW  key;
    CStringW  value;
  } mKv;

  KVPairVector  mValues;
};

} // namespace protocolpatchLib
