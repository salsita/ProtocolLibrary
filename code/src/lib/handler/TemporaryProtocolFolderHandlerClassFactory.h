/****************************************************************************
 * TemporaryProtocolFolderHandlerClassFactory.h : Declaration of
 * CTemporaryProtocolFolderHandlerClassFactory
 * Copyright 2013 Salsita Software (http://www.salsitasoft.com).
 * Author: Arne Seib <arne@salsitasoft.com>
 ****************************************************************************/

#pragma once

#include "TemporaryProtocolHandlerClassFactoryT.h"
#include "TemporaryProtocolFolderHandler.h"

/*****************************************************************************
 * class CTemporaryProtocolFolderHandlerClassFactory
 *  Implements IClassFactory.
 *  Creates instances of CTemporaryProtocolFolderHandler.
 *****************************************************************************/
class ATL_NO_VTABLE CTemporaryProtocolFolderHandlerClassFactory :
  public CTemporaryProtocolHandlerClassFactoryT
        <CTemporaryProtocolFolderHandlerClassFactory,
         CTemporaryProtocolFolderHandler,
          FolderHandlerHostInfo>,
  public CComObjectRootEx<CComSingleThreadModel>,
  public IProtocolMemoryResource
{
public:
  friend class CProtocolHandlerRegistrar;

  //----------------------------------------------------------------------------
  // com stuff
  DECLARE_NO_REGISTRY()
  DECLARE_PROTECT_FINAL_CONSTRUCT()

  BEGIN_COM_MAP(CTemporaryProtocolFolderHandlerClassFactory)
    COM_INTERFACE_ENTRY(IClassFactory)
    COM_INTERFACE_ENTRY(IProtocolMemoryResource)
  END_COM_MAP()

  HRESULT FinalConstruct();
  void FinalRelease();

public:
  //-------------------------------------------------------------------------
  // called from CTemporaryProtocolHandlerClassFactoryT when creating a new
  // protocol handler instance to allow initialization
  HRESULT InitHandler(CTemporaryProtocolFolderHandler * pHandler);

  // IProtocolMemoryResource
  STDMETHOD(AddResource)(
      IUri * aURI,
      LPCVOID lpData,
      DWORD dwLength,
      LPCWSTR lpszMimeType);

  STDMETHOD(GetResource)(
      IUri * aUri,
      URLMemoryResource & aRetBuffer);

protected:
  // called from CProtocolHandlerRegistrar
  // adds a host with the host name and a folder name to load the resources
  // from
  HRESULT AddHost(
    LPCWSTR   lpszHost,
    LPCWSTR   lpszFolderName);

private:
  //----------------------------------------------------------------------------
  // private data members

  // map for special URLs
  CAtlMap<CStringW, URLMemoryResource> mSpecialURLs;

};
