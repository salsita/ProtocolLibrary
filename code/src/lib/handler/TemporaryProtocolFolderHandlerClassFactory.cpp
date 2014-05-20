/****************************************************************************
 * TemporaryProtocolFolderHandlerClassFactory.cpp : Implementation of
 * CTemporaryProtocolFolderHandlerClassFactory
 * Copyright 2013 Salsita Software (http://www.salsitasoft.com).
 * Author: Arne Seib <arne@salsitasoft.com>
 ****************************************************************************/

#include "StdAfx.h"

#include "TemporaryProtocolFolderHandlerClassFactory.h"

/*****************************************************************************
 * class CTemporaryProtocolFolderHandler
 *****************************************************************************/

//---------------------------------------------------------------------------
// FinalConstruct
HRESULT CTemporaryProtocolFolderHandlerClassFactory::FinalConstruct()
{
  return S_OK;
}

//---------------------------------------------------------------------------
// FinalRelease
void CTemporaryProtocolFolderHandlerClassFactory::FinalRelease()
{
}

//-------------------------------------------------------------------------
// AddHost
STDMETHODIMP CTemporaryProtocolFolderHandlerClassFactory::AddHost(
  LPCWSTR aHostname,
  VARIANT vtValue)
{
  // check arguments
  if (!wcslen(aHostname) || !wcslen(aHostname)) {
    return E_INVALIDARG;
  }
  if (VT_BSTR != vtValue.vt) {
    return E_INVALIDARG;
  }

  CStringW folderName = vtValue.bstrVal;
  CritSectLock lock(m_CriticalSection);

  // lookup host if we have already
  FolderHandlerHostInfo hostInfo;
  if (LookupHostInfo(aHostname, hostInfo)) {
    // found
    return S_FALSE;
  }

  hostInfo.hostName = aHostname;
  hostInfo.folderName = folderName;

  // append backslash to folder
  PathAddBackslash(hostInfo.folderName.GetBuffer(MAX_PATH));
  hostInfo.folderName.ReleaseBuffer();

  // add to map
  SetHostInfo(aHostname, hostInfo);

  return S_OK;
}

//---------------------------------------------------------------------------
// AddResource
STDMETHODIMP CTemporaryProtocolFolderHandlerClassFactory::AddResource(
  IUri * aURI,
  LPCVOID lpData,
  DWORD dwLength,
  LPCWSTR lpszMimeType)
{
  CritSectLock lock(m_CriticalSection);
  if (!aURI) {
    return E_INVALIDARG;
  }
  CComBSTR url;
  IF_FAILED_RET(aURI->GetAbsoluteUri(&url));
  mSpecialURLs[url].setData(lpData, dwLength, lpszMimeType);
  return S_OK;
}

//---------------------------------------------------------------------------
// GetResource
STDMETHODIMP CTemporaryProtocolFolderHandlerClassFactory::GetResource(IUri * aURI, URLMemoryResource & aRetBuffer)
{
  CritSectLock lock(m_CriticalSection);
  if (!aURI) {
    return E_INVALIDARG;
  }
  CComBSTR url;
  IF_FAILED_RET(aURI->GetAbsoluteUri(&url));
  CAtlMap<CStringW, URLMemoryResource>::CPair * entry = mSpecialURLs.Lookup(url);
  if (!entry) {
    return E_FAIL;
  }
  aRetBuffer.copyFrom(entry->m_value);

  return S_OK;
}

//---------------------------------------------------------------------------
// InitHandler
HRESULT CTemporaryProtocolFolderHandlerClassFactory::InitHandler(
  CTemporaryProtocolFolderHandler * pHandler)
{
  return pHandler->Init(this);
}
