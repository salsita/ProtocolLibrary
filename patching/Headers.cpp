/****************************************************************************
 * Headers.cpp : Implementation of Headers
 * Copyright 2013 Salsita (http://www.salsitasoft.com).
 * Author: Arne Seib <arne@salsitasoft.com>
 ****************************************************************************/

#include "stdafx.h"
#include "Headers.h"

// Headers

namespace protocolpatchLib
{

//--------------------------------------------------------------------------
// onHeaderField
//  Static method, callback from parser, adds a new header field (name).
int Headers::onHeaderField(http_parser * aParser, const char *aField, size_t aLength)
{
  if (aLength > MAXINT)
      { return -1; }
  CStringA name;
  LPSTR buffer = name.GetBuffer(aLength);
  memcpy(buffer, aField, aLength);
  name.ReleaseBuffer(aLength);
  static_cast<Headers*>(aParser->data)->mCurrentHeaderKey = CA2W(name);
  return 0;
}

//--------------------------------------------------------------------------
// onHeaderValue
//  Static method, callback from parser, adds a new header value.
int Headers::onHeaderValue(http_parser * aParser, const char *aField, size_t aLength)
{
  if (aLength > MAXINT)
      { return -1; }
  CStringA value;
  LPSTR buffer = value.GetBuffer(aLength);
  memcpy(buffer, aField, aLength);
  value.ReleaseBuffer(aLength);

  Headers * pThis = static_cast<Headers*>(aParser->data);
  HRESULT hr = S_OK;
  CComPtr<IKVPair> pair = KVPair::createInstance(pThis->mCurrentHeaderKey, CA2W(value), hr);
  pThis->mValues.push_back(pair);
  return 0;
}

//--------------------------------------------------------------------------
// createInstance
//  Static creator
CComPtr<IHeaders> Headers::createInstance(LPCWSTR aHeaders, BOOL aIsResponse, HRESULT & hr)
{
  _ComObject * newInstance = NULL;
  hr = _ComObject::CreateInstance(&newInstance);
  if (SUCCEEDED(hr)) {
    CComPtr<IHeaders> owner(newInstance);
    hr = newInstance->parse(aHeaders, aIsResponse);
    if (SUCCEEDED(hr)) {
      return owner;
    }
  }
  return NULL;
}

//--------------------------------------------------------------------------
// parse
//  Parses the headers in aHeaders and remembers them.
//  Called by createInstance only.
HRESULT Headers::parse(LPCWSTR aHeaders, BOOL aIsResponse)
{
  mValues.clear();

  CStringA headers = CW2A(aHeaders);

  http_parser parser;
  http_parser_init(&parser, (aIsResponse) ? HTTP_RESPONSE : HTTP_REQUEST);
  parser.data = this;

  http_parser_settings settings = {0};
  settings.on_header_field = onHeaderField;
  settings.on_header_value = onHeaderValue;

  http_parser_execute(&parser, &settings, headers, headers.GetLength());

  return (HPE_OK == HTTP_PARSER_ERRNO(&parser)) ? S_OK : E_FAIL;
}

//--------------------------------------------------------------------------
// getHeaders
HRESULT Headers::getHeaders(CStringW & aHeaders)
{
  aHeaders.Empty();

  for (auto it = mValues.begin(); it != mValues.end(); ++it) {
    CComBSTR key, value;
    (*it)->get(&key, &value);
    if (key.Length() && value.Length()) {
      aHeaders += key;
      aHeaders += L": ";
      aHeaders += value;
      aHeaders += L"\r\n";
    }
  }
  return S_OK;
}

//--------------------------------------------------------------------------
// GetIDsOfNames
STDMETHODIMP Headers::GetIDsOfNames(
  _In_ REFIID riid, 
  _In_count_(cNames) _Deref_pre_z_ LPOLESTR* rgszNames, 
  _In_ UINT cNames,
  _In_ LCID lcid, 
  _Out_ DISPID* rgdispid)
{
  if (!rgdispid) {
    return E_POINTER;
  }

  HRESULT hr = _DispImpl::GetIDsOfNames(riid, rgszNames, cNames, lcid, rgdispid);
  if (SUCCEEDED(hr)) {
    return hr;
  }
  HRESULT hrRet = hr;

  CComVariant vtIndex(rgszNames[0]);
  hr = vtIndex.ChangeType(VT_I4);
  if (FAILED(hr)) {
    return hrRet;
  }

  ULONG index = vtIndex.ulVal;
  if ((index >= (ULONG)mValues.size()) || (index >= DISPID_FIRST_HEADER_NATIVE)) {
    return hrRet;
  }
  (*rgdispid) = index;

  return S_OK;
}

//--------------------------------------------------------------------------
// Invoke
STDMETHODIMP Headers::Invoke(
  _In_ DISPID dispidMember, 
  _In_ REFIID riid,
  _In_ LCID lcid, 
  _In_ WORD wFlags, 
  _In_ DISPPARAMS* pdispparams, 
  _Out_opt_ VARIANT* pvarResult,
  _Out_opt_ EXCEPINFO* pexcepinfo, 
  _Out_opt_ UINT* puArgErr)
{
  HRESULT hr = _DispImpl::Invoke(dispidMember, riid, lcid, wFlags, pdispparams, pvarResult, pexcepinfo, puArgErr);
  if (SUCCEEDED(hr)) {
    return hr;
  }
  if (!(DISPATCH_PROPERTYGET & wFlags)) {
    return hr;
  }

  if (!pvarResult) {
    return E_POINTER;
  }

  ULONG index = (ULONG)dispidMember;
  if ((dispidMember < 0) || (index >= (ULONG)mValues.size()) || (index >= DISPID_FIRST_HEADER_NATIVE)) {
    pvarResult->vt = VT_EMPTY;
    return S_OK;
  }

  CComVariant vt(mValues[index]);
  return vt.Detach(pvarResult);
}

//--------------------------------------------------------------------------
// get_length
STDMETHODIMP Headers::get_length(ULONG * aRetVal)
{
  if (!aRetVal) {
    return E_POINTER;
  }
  (*aRetVal) = (ULONG)mValues.size();
  return S_OK;
}

//--------------------------------------------------------------------------
// forEach
STDMETHODIMP Headers::forEach(IDispatch * aCallback, VARIANT aThis)
{
  if (!aCallback) {
    return E_INVALIDARG;
  }
  KVPairVector tmp = mValues;
  KVPairSet removed;

  // forEach callback args:  this,  array, index,    value
  // in reversed order
  CComVariant args[]      = {aThis, this,  (ULONG)0, NULL};
  DISPID namedArgs[] = {DISPID_THIS};
  DISPPARAMS params = {&args[1], NULL, _countof(args)-1, 0};

  if (VT_DISPATCH == aThis.vt) {
    params.rgvarg = args;
    params.rgdispidNamedArgs = namedArgs;
    params.cNamedArgs = 1;
    params.cArgs++;
  }

  for (auto it = tmp.begin(); it != tmp.end(); ++it, args[2].ulVal++) {
    args[3] = (*it);
    CComVariant vtResult;
    aCallback->Invoke(DISPID_VALUE, IID_NULL, LOCALE_USER_DEFAULT, DISPATCH_METHOD, &params, &vtResult, NULL, NULL);
    if ((VT_BOOL == vtResult.vt) && (VARIANT_TRUE == vtResult.boolVal)) {
      removed.insert((*it));
    }
  }

  // remove removed values
  for (auto it = mValues.begin(); it != mValues.end(); ++it) {
    if (removed.end() != removed.find((*it))) {
      mValues.erase(it--);
    }
  }
  return S_OK;
}

//--------------------------------------------------------------------------
// remove
STDMETHODIMP Headers::remove(ULONG aIndex)
{
  if (aIndex < (ULONG)mValues.size()) {
    mValues.erase(mValues.begin() + aIndex);
  }
  return S_OK;
}

//--------------------------------------------------------------------------
// push
STDMETHODIMP Headers::push(BSTR aKey, BSTR aVal)
{
  HRESULT hr = S_OK;
  CComPtr<IKVPair> entry = KVPair::createInstance(aKey, aVal, hr);
  if (FAILED(hr)) {
    return hr;
  }
  mValues.push_back(entry);
  return hr;
}

//--------------------------------------------------------------------------
// clear
STDMETHODIMP Headers::clear()
{
  mValues.clear();
  return S_OK;
}

//--------------------------------------------------------------------------
// getKV
STDMETHODIMP Headers::getKV(ULONG aIndex, BSTR * aRetKey, BSTR * aRetVal)
{
  CComPtr<IKVPair> pair;
  HRESULT hr = get(aIndex, &pair);
  if (FAILED(hr)) {
    return hr;
  }
  return pair->get(aRetKey, aRetVal);
}

//--------------------------------------------------------------------------
// get
STDMETHODIMP Headers::get(ULONG aIndex, IKVPair ** aRetVal)
{
  if (aIndex >= (ULONG)mValues.size()) {
    return DISP_E_BADINDEX;
  }
  return mValues[aIndex].CopyTo(aRetVal);
}

} // namespace protocolpatchLib
