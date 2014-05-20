/****************************************************************************
 * TemporaryProtocolResourceHandlerClassFactory.h : Declaration of
 * CTemporaryProtocolResourceHandlerClassFactory
 * Copyright 2013 Salsita Software (http://www.salsitasoft.com).
 * Author: Arne Seib <arne@salsitasoft.com>
 ****************************************************************************/

#pragma once

#include "TemporaryProtocolHandlerClassFactoryT.h"
#include "TemporaryProtocolResourceHandler.h"

/*****************************************************************************
 * class CTemporaryProtocolResourceHandlerClassFactory
 *  Implements IClassFactory.
 *  Creates instances of CTemporaryProtocolFolderHandler.
 *****************************************************************************/
class ATL_NO_VTABLE CTemporaryProtocolResourceHandlerClassFactory :
  public CTemporaryProtocolHandlerClassFactoryT
        <CTemporaryProtocolResourceHandlerClassFactory,
         CTemporaryProtocolResourceHandler,
         ResourceHandlerHostInfo>,
  public CComObjectRootEx<CComSingleThreadModel>
{
public:
  friend class CProtocolHandlerRegistrar;
  typedef CComObject<CTemporaryProtocolResourceHandlerClassFactory> _ComObject;

  //----------------------------------------------------------------------------
  // com stuff
  DECLARE_NO_REGISTRY()
  DECLARE_PROTECT_FINAL_CONSTRUCT()

  BEGIN_COM_MAP(CTemporaryProtocolResourceHandlerClassFactory)
    COM_INTERFACE_ENTRY(IClassFactory)
  END_COM_MAP()

  HRESULT FinalConstruct();
  void FinalRelease();

  //----------------------------------------------------------------------------
  // IProtocolClassFactory implementation
  STDMETHOD(AddHost)(
    LPCWSTR aHostname,
    VARIANT vtValue);

public:
  //-------------------------------------------------------------------------
  // called from CTemporaryProtocolHandlerClassFactoryT when creating a new
  // protocol handler instance to allow initialization
  HRESULT InitHandler(CTemporaryProtocolResourceHandler * pHandler);

private:
  HRESULT AddHostResource(
    LPCWSTR   lpszHost,
    HINSTANCE hInstResources);

};
