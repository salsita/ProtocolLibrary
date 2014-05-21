/****************************************************************************
 * Buffer.h : Declaration of Buffer and URLMemoryResource
 * Copyright 2013 Salsita Software (http://www.salsitasoft.com).
 * Author: Arne Seib <arne@salsitasoft.com>
 ****************************************************************************/

#pragma once

#include <Buffer.h>

/*****************************************************************************
 * class ResourceReader
 *****************************************************************************/
class ResourceReader :
  public SeekableBuffer
{
public:
  ResourceReader() :
    m_hrc(0), m_hGlobalResource(0)
  {
  }

  virtual ~ResourceReader()
  {
  }

  HRESULT SetResource(HINSTANCE aResourceHandle, LPCWSTR lpszPath)
  {
    m_hrc = FindResource(aResourceHandle, lpszPath, RT_HTML);
    if (nullptr == m_hrc) {
      return E_FAIL;
    }

    m_hGlobalResource = LoadResource(aResourceHandle, m_hrc);
    if (!m_hGlobalResource) {
      return E_FAIL;
    }

    mPos = 0;
    mLength = SizeofResource(aResourceHandle, m_hrc);
    mData = (LPBYTE)LockResource(m_hGlobalResource);

    return S_OK;
  }

  void ReleaseResource()
  {
    m_hrc = nullptr;
    m_hGlobalResource = nullptr;
    mData = nullptr;
    mPos = mLength = 0;
  }

protected:
  HRSRC     m_hrc;
  HGLOBAL   m_hGlobalResource;
};

