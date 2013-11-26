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
  Headers * _this = static_cast<Headers*>(aParser->data);
  if (aLength > MAXINT)
      { return -1; }
  std::string value;
  value.append(aField, aLength);
  std::wstring valueW = CA2W(value.c_str());
  _this->mNameIDs[valueW] = _this->mNextDispID;
  _this->mNames[_this->mNextDispID] = valueW;
  return 0;
}

//--------------------------------------------------------------------------
// onHeaderValue
//  Static method, callback from parser, adds a new header value.
int Headers::onHeaderValue(http_parser * aParser, const char *aField, size_t aLength)
{
  Headers * _this = static_cast<Headers*>(aParser->data);
  if (aLength > MAXINT)
      { return -1; }
  std::string value;
  value.append(aField, aLength);
  _this->mValues[_this->mNextDispID] = CA2W(value.c_str());
  _this->mNextDispID++;
  return 0;
}

//--------------------------------------------------------------------------
// createInstance
//  Static creator
CComPtr<IDispatchEx> Headers::createInstance(LPCWSTR aHeaders, BOOL aIsResponse, HRESULT & hr)
{
  _ComObject * newInstance = NULL;
  hr = _ComObject::CreateInstance(&newInstance);
  if (SUCCEEDED(hr)) {
    CComPtr<IDispatchEx> owner(newInstance);
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
  mModified = FALSE;
  mValues.clear();
  mNames.clear();
  mNameIDs.clear();

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

STDMETHODIMP Headers::get(BSTR aName, BSTR * aRetVal)
{
  DISPID did = 0;
  HRESULT hr = getDispID(aName, &did);
  if (FAILED(hr)) {
    return hr;
  }
  CComVariant vt;
  hr = getValue(did, &vt);
  if (FAILED(hr)) {
    return hr;
  }
  vt.ChangeType(VT_BSTR);
  if (VT_BSTR != vt.vt) {
    return DISP_E_TYPEMISMATCH;
  }
  return vt.CopyTo(aRetVal);
}

STDMETHODIMP Headers::set(BSTR aName, BSTR aValue)
{
  DISPID did = 0;
  HRESULT hr = getDispID(aName, &did, TRUE);
  if (FAILED(hr)) {
    return hr;
  }
  CComVariant vt(aValue);
  return putValue(did, &vt);
}

//--------------------------------------------------------------------------
// getHeadersIfModified
//  If the we were modified, replace the headers in pszAdditionalHeaders.
//  If not, return S_FALSE and leave pszAdditionalHeaders unchanged.
HRESULT Headers::getHeaders(CStringW & aHeaders)
{
  LOCKED_BLOCK();
  if (!mModified) {
    return S_FALSE;
  }
  aHeaders.Empty();

  for (MapName2DispId::iterator it = mNameIDs.begin();
      it != mNameIDs.end(); ++it) {
    MapDispId2Variant::iterator itValue = mValues.find(it->second);
    if (itValue != mValues.end() && (VT_BSTR == itValue->second.vt)) {
      aHeaders += it->first.c_str();
      aHeaders += L": ";
      aHeaders += itValue->second.bstrVal;
      aHeaders += L"\r\n";
    }
  }
  return S_OK;
}

} // namespace protocolpatchLib
