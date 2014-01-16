/****************************************************************************
 * Headers.h : Declaration of Headers
 * Copyright 2013 Salsita Software (http://www.salsitasoft.com).
 * Author: Arne Seib <arne@salsitasoft.com>
 ****************************************************************************/
#pragma once

#include "resource.h"       // main symbols
#include <string>
#include <set>
#include <vector>
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
public: // types
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

public: // methods
  //--------------------------------------------------------------------------
  // get_key
  STDMETHOD(get_key)(BSTR * aRetVal)
  {
    if (!aRetVal) {
      return E_POINTER;
    }
    (*aRetVal) = mKey.AllocSysString();
    return S_OK;
  }

  //--------------------------------------------------------------------------
  // put_key
  STDMETHOD(put_key)(BSTR aVal)
  {
    mKey = aVal;
    return S_OK;
  }

  //--------------------------------------------------------------------------
  // get_val
  STDMETHOD(get_val)(BSTR * aRetVal)
  {
    if (!aRetVal) {
      return E_POINTER;
    }
    (*aRetVal) = mVal.AllocSysString();
    return S_OK;
  }

  //--------------------------------------------------------------------------
  // put_val
  STDMETHOD(put_val)(BSTR aVal)
  {
    mVal = aVal;
    return S_OK;
  }

  //--------------------------------------------------------------------------
  // set
  STDMETHOD(set)(BSTR aKey, BSTR aVal)
  {
    mKey = aKey;
    mVal = aVal;
    return S_OK;
  }

  //--------------------------------------------------------------------------
  // get
  STDMETHOD(get)(BSTR * aKeyOut, BSTR * aValOut)
  {
    if (!aKeyOut || !aValOut) {
      return E_POINTER;
    }
    (*aKeyOut) = mKey.AllocSysString();
    (*aValOut) = mVal.AllocSysString();
    return S_OK;
  }

private:  // members
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
public: // types
  typedef CComObject< Headers > _ComObject;
  typedef IDispatchImpl<IHeaders, &IID_IHeaders, &LIBID_protocolpatchLib, 0xffff, 0xffff>  _DispImpl;

public: // methods
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

public: // methods

  //--------------------------------------------------------------------------
  // getHeaders: return all headers concatenated with CRLF
  HRESULT getHeaders(CStringW & aHeaders);

  //--------------------------------------------------------------------------
  // IDispatch::GetIDsOfNames
  STDMETHOD(GetIDsOfNames)(
    _In_ REFIID riid,
    _In_count_(cNames) _Deref_pre_z_ LPOLESTR* rgszNames,
    _In_ UINT cNames,
    _In_ LCID lcid,
    _Out_ DISPID* rgdispid);

  //--------------------------------------------------------------------------
  // IDispatch::Invoke
  STDMETHOD(Invoke)(
    _In_ DISPID dispidMember,
    _In_ REFIID riid,
    _In_ LCID lcid,
    _In_ WORD wFlags,
    _In_ DISPPARAMS* pdispparams,
    _Out_opt_ VARIANT* pvarResult,
    _Out_opt_ EXCEPINFO* pexcepinfo,
    _Out_opt_ UINT* puArgErr);

  //--------------------------------------------------------------------------
  // GetIDsOfNames
  STDMETHOD(get_length)(ULONG * aRetVal);

  //--------------------------------------------------------------------------
  // forEach
  STDMETHOD(forEach)(IDispatch * aCallback, VARIANT aThis);

  //--------------------------------------------------------------------------
  // remove
  STDMETHOD(remove)(ULONG aIndex);

  //--------------------------------------------------------------------------
  // push
  STDMETHOD(push)(BSTR aKey, BSTR aVal);

  //--------------------------------------------------------------------------
  // clear
  STDMETHOD(clear)();

  //--------------------------------------------------------------------------
  // getKV
  STDMETHOD(getKV)(ULONG aIndex, BSTR * aRetKey, BSTR * aRetVal);

  //--------------------------------------------------------------------------
  // get
  STDMETHOD(get)(ULONG aIndex, IKVPair ** aRetVal);

private:  // types
  typedef std::vector< CComPtr<IKVPair> >  KVPairVector;
  typedef std::set< IKVPair* > KVPairSet;

private:  // methods

  //--------------------------------------------------------------------------
  // onHeaderField: parser callback
  static int onHeaderField(http_parser * aParser, const char *aField, size_t aLength);

  //--------------------------------------------------------------------------
  // onHeaderValue: parser callback
  static int onHeaderValue(http_parser * aParser, const char *aField, size_t aLength);

  //--------------------------------------------------------------------------
  // parse: parse headers
  HRESULT parse(LPCWSTR aHeaders, BOOL aIsResponse);

private:  // members
  // temp var for parsing
  CStringW  mCurrentHeaderKey;
  // storage
  KVPairVector  mValues;
};

} // namespace protocolpatchLib
