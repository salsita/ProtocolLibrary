/****************************************************************************
 * TemporaryProtocolResourceHandler.cpp : Implementation of
 * CTemporaryProtocolResourceHandler
 * Copyright 2013 Salsita Software (http://www.salsitasoft.com).
 * Author: Arne Seib <arne@salsitasoft.com>
 ****************************************************************************/

#include "StdAfx.h"
#include "TemporaryProtocolResourceHandler.h"
#include "TemporaryProtocolResourceHandlerClassFactory.h"

// CTOR ResourceHandlerHostInfo
ResourceHandlerHostInfo::ResourceHandlerHostInfo() :
  hResourceInstance(NULL)
{
}

/*****************************************************************************
 * class CTemporaryProtocolResourceHandler
 *****************************************************************************/
//---------------------------------------------------------------------------
// the CLSID for this object
// {5902FEC0-9341-47cd-AAD3-A63FD45E01D4}
const GUID CTemporaryProtocolResourceHandler::CLSID =
  {0x5902fec0, 0x9341, 0x47cd, {0xaa, 0xd3, 0xa6, 0x3f, 0xd4, 0x5e, 0x1, 0xd4}};

//---------------------------------------------------------------------------
// FreeResources
void CTemporaryProtocolResourceHandler::FreeResources()
{
  m_SpecialURLResource.clear();
  ReleaseResource();
}

//---------------------------------------------------------------------------
// FinalConstruct
HRESULT CTemporaryProtocolResourceHandler::FinalConstruct()
{
  return S_OK;
}

//---------------------------------------------------------------------------
// FinalRelease
void CTemporaryProtocolResourceHandler::FinalRelease()
{
  FreeResources();
}

//---------------------------------------------------------------------------
// InitializeRequest
HRESULT CTemporaryProtocolResourceHandler::InitializeRequest(
  LPCWSTR lpszPath, DWORD & dwSize, CStringW & aMimeType)
{
  // strip leading '/'
  CStringW sPath(lpszPath+1);

  // And adjust to resource name scheme.
  // In a resource name no slashes or backslashes are allowed. That's why we
  // use '|' as a path separator.
  sPath.Replace(_T('/'), _T('|'));

  if (FAILED(SetResource(m_HostInfo.hResourceInstance, sPath))) {
    return INET_E_OBJECT_NOT_FOUND;
  }

  dwSize = mLength;
  return S_OK;
}

//---------------------------------------------------------------------------
// Read
STDMETHODIMP CTemporaryProtocolResourceHandler::Read(
  void *pv, ULONG cb, ULONG *pcbRead)
{
  if (m_SpecialURLResource.mData) {
    // have a special URL
    return m_SpecialURLResource.read(pv, cb, pcbRead);
  }
  return read(pv, cb, pcbRead);
}

//---------------------------------------------------------------------------
// UnlockRequest
STDMETHODIMP CTemporaryProtocolResourceHandler::UnlockRequest()
{
  FreeResources();
  return S_OK;
}

//---------------------------------------------------------------------------
// Seek
STDMETHODIMP CTemporaryProtocolResourceHandler::Seek(
  LARGE_INTEGER dlibMove, DWORD dwOrigin, ULARGE_INTEGER *plibNewPosition)
{
  if (m_SpecialURLResource.mData) {
    // have a special URL
    return m_SpecialURLResource.seek(dlibMove, dwOrigin, plibNewPosition);
  }
  return seek(dlibMove, dwOrigin, plibNewPosition);
}
