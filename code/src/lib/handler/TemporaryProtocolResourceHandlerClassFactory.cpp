/****************************************************************************
 * TemporaryProtocolResourceHandlerClassFactory.cpp : Implementation of
 * CTemporaryProtocolResourceHandlerClassFactory
 * Copyright 2013 Salsita Software (http://www.salsitasoft.com).
 * Author: Arne Seib <arne@salsitasoft.com>
 ****************************************************************************/

#include "StdAfx.h"

#include "TemporaryProtocolResourceHandlerClassFactory.h"

/*****************************************************************************
 * class CTemporaryProtocolResourceHandlerClassFactory
 *****************************************************************************/

//---------------------------------------------------------------------------
// FinalConstruct
HRESULT CTemporaryProtocolResourceHandlerClassFactory::FinalConstruct()
{
  return S_OK;
}

//---------------------------------------------------------------------------
// FinalRelease
void CTemporaryProtocolResourceHandlerClassFactory::FinalRelease()
{
}

//-------------------------------------------------------------------------
// AddHost
STDMETHODIMP CTemporaryProtocolResourceHandlerClassFactory::AddHost(
  LPCWSTR aHostname,
  VARIANT vtValue)
{
  // check arguments
  if (!wcslen(aHostname) || !wcslen(aHostname)) {
    return E_INVALIDARG;
  }

  if (VT_BYREF == vtValue.vt) {
    // a resource handle
    return AddHostResource(aHostname, reinterpret_cast<HINSTANCE>(vtValue.byref));
  }
  if (VT_BSTR == vtValue.vt) {
    // a filename
    // load resource file
    // note that the library stays loaded until the process terminates
    HINSTANCE hInstResources =
        ::LoadLibraryExW(vtValue.bstrVal, NULL, LOAD_LIBRARY_AS_DATAFILE);
    if (!hInstResources) {
      DWORD dw = GetLastError();
      return HRESULT_FROM_WIN32(dw);
    }

    HRESULT hr = AddHostResource(aHostname, hInstResources);
    if (FAILED(hr)) {
      ::FreeLibrary(hInstResources);
    }
    return hr;
  }
  return E_INVALIDARG;
}

//---------------------------------------------------------------------------
// AddHost
HRESULT CTemporaryProtocolResourceHandlerClassFactory::AddHostResource(
  LPCWSTR   lpszHost,
  HINSTANCE hInstResources)
{
  CritSectLock lock(m_CriticalSection);

  // check arguments
  if (!wcslen(lpszHost) || !hInstResources) {
    return E_INVALIDARG;
  }

  // lookup host if we have already
  ResourceHandlerHostInfo hostInfo;
  if (LookupHostInfo(lpszHost, hostInfo)) {
    // found
    return S_FALSE;
  }

  hostInfo.hostName = lpszHost;
  hostInfo.hResourceInstance = hInstResources;

  // add to map
  SetHostInfo(lpszHost, hostInfo);

  return S_OK;
}

//---------------------------------------------------------------------------
// InitHandler
HRESULT CTemporaryProtocolResourceHandlerClassFactory::InitHandler(
  CTemporaryProtocolResourceHandler * pHandler)
{
  return pHandler->Init(this);
}
