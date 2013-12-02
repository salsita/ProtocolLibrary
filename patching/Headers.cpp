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
  strcpy_s(buffer, aLength, aField);
  name.ReleaseBuffer(aLength);
  static_cast<Headers*>(aParser->data)->mKv.key = CA2W(name);
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
  strcpy_s(buffer, aLength, aField);
  value.ReleaseBuffer(aLength);

  Headers * pThis = static_cast<Headers*>(aParser->data);
  HRESULT hr = S_OK;
  CComPtr<IKVPair> pair = KVPair::createInstance(pThis->mKv.key, CA2W(value), hr);
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

  CStringA headers;
  if (!aIsResponse) {
    // In case of a request the parser needs a proper request starting
    // with an HTTP verb / result. We have only raw headers,
    // so fake this.
    headers = ("GET / HTTP/1.1\r\n");
  }
  headers += CW2A(aHeaders);

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
// get_length
STDMETHODIMP Headers::get_length(ULONG * aRetVal)
{
  if (!aRetVal) {
    return E_POINTER;
  }
  (*aRetVal) = (ULONG)mValues.size();
  return S_OK;
}

/*
//--------------------------------------------------------------------------
// item
STDMETHODIMP Headers::item(ULONG aIndex)
{
  if (aIndex >= (ULONG)mValues.size()) {
    return DISP_E_BADINDEX;
  }
  return S_OK;
}
*/

//--------------------------------------------------------------------------
// forEach
STDMETHODIMP Headers::forEach(IDispatch * aCallback, VARIANT aThis)
{
  if (!aCallback) {
    return E_INVALIDARG;
  }
  KVPairVector tmp = mValues;

  //                    this,  array, index,    value
  CComVariant args[] = {aThis, this,  (ULONG)0, NULL};
  DISPID namedArgs[] = {DISPID_THIS};
  DISPPARAMS params  = {&args[1], NULL, _countof(args)-1, 0};

  if (VT_DISPATCH == aThis.vt) {
    params.rgvarg = args;
    params.rgdispidNamedArgs = namedArgs;
    params.cNamedArgs = 1;
    params.cArgs++;
  }

  for (auto it = tmp.begin(); it != tmp.end(); ++it) {
    args[2].ulVal++;
    args[3] = (*it);
    CComVariant vtResult;
    aCallback->Invoke(DISPID_VALUE, IID_NULL, LOCALE_USER_DEFAULT, DISPATCH_METHOD, &params, &vtResult, NULL, NULL);
  }
  return S_OK;
}

} // namespace protocolpatchLib
