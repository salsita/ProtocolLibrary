/****************************************************************************
 * Response.h : Declaration of Response
 * Copyright 2013 Salsita Software (http://www.salsitasoft.com).
 * Author: Arne Seib <arne@salsitasoft.com>
 ****************************************************************************/

#include "stdafx.h"
#include "Headers.h"
#include "RequestRecord.h"

// Response

namespace protocolpatchLib
{

HRESULT Response::createInstance(CComPtr<IResponse> & aRetVal)
{
  _ComObject * newInstance = NULL;
  HRESULT hr = _ComObject::CreateInstance(&newInstance);
  if (FAILED(hr)) {
    return hr;
  }
  aRetVal = newInstance;
  if (FAILED(hr)) {
    aRetVal.Release();
  }
  return hr;
}

HRESULT Response::initResponse(RequestRecord & aRecord, LPCWSTR aHeaders, DWORD aResponseCode, HRESULT aOnResponseResult)
{
  mRequestId = aRecord.id;
  mResponseCode = aResponseCode;
  mHeaders.Release();
  HRESULT hr = S_OK;
  mHeaders = Headers::createInstance(aHeaders, TRUE, hr);
  return hr;
}

STDMETHODIMP Response::get_result(ULONG * aRetVal)
{
  if (!aRetVal) {
    return E_POINTER;
  }
  (*aRetVal) = mResponseCode;
  return S_OK;
}

STDMETHODIMP Response::get_headers(IHeaders ** aRetVal)
{
  if (!aRetVal) {
    return E_POINTER;
  }
  if (!mHeaders) {
    return E_UNEXPECTED;
  }
  return mHeaders.CopyTo(aRetVal);
}

} // namespace protocolpatchLib
