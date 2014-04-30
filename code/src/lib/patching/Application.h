/****************************************************************************
 * Application.h : Declaration of Application
 * Copyright 2013 Salsita Software (http://www.salsitasoft.com).
 * Author: Arne Seib <arne@salsitasoft.com>
 ****************************************************************************/
#pragma once

#include "../dll/resource.h"       // main symbols
#include "ProtocolLibrary_i.h"
#include "ClassFactories.h"
#include <map>

namespace protocolpatchLib
{

/*============================================================================
 * class Application
 *  The main protocol patch application object. Patches / unpatches HTTP and
 *  HTTPS protocols, manages browser instances.
 */
class ATL_NO_VTABLE Application :
  public CComObjectRootEx<CComSingleThreadModel>,
  public CComCoClass<Application, &CLSID_ProtPatchApplication>,
  public IProtPatchApplication
{
public:
  //--------------------------------------------------------------------------
  // static methods
  static CComPtr<IProtPatchApplication> getInstance();

  //--------------------------------------------------------------------------
  // usual stuff
  Application() : mHandlerType(NONE)
  {
  }

  DECLARE_CLASSFACTORY_SINGLETON(Application)
  DECLARE_NO_REGISTRY()
  DECLARE_NOT_AGGREGATABLE(Application)

  BEGIN_COM_MAP(Application)
    COM_INTERFACE_ENTRY(IProtPatchApplication)
  END_COM_MAP()

  DECLARE_PROTECT_FINAL_CONSTRUCT()

  HRESULT FinalConstruct()
  {
    return S_OK;
  }

  void FinalRelease()
  {
  }

public:
  //--------------------------------------------------------------------------
  // IProtPatchApplication
  STDMETHOD(patchProtokol)(Scheme aScheme, BOOL aUseVTablePatching);
  STDMETHOD(unpatchProtokol)(Scheme aScheme);
  STDMETHOD(enableProtokol)(Scheme aScheme, BOOL aEnable);

  //-------------------------------------------------------------------------
  // registers a temporary file protocol of the form
  // lpszScheme://lpszHost/
  // where lpszFolder points to the root folder of this "server"
  STDMETHOD(RegisterTemporaryFolderHandler)(
    LPCOLESTR lpszScheme,
    LPCOLESTR lpszHost,
    LPCOLESTR lpszFolder);

  //-------------------------------------------------------------------------
  // registers a temporary resource protocol of the form
  // lpszScheme://lpszHost/
  // where lpszResourceFileName points to the file (DLL or exe) holding the
  // resources
  STDMETHOD(RegisterTemporaryResourceHandlerFile)(
    LPCOLESTR lpszScheme,
    LPCOLESTR lpszHost,
    LPCOLESTR lpszResourceFileName);

  //-------------------------------------------------------------------------
  // registers a temporary resource protocol of the form
  // lpszScheme://lpszHost/
  // where hInstResources is a module handle of the file (DLL or exe) holding
  // the resources
  STDMETHOD(RegisterTemporaryResourceHandlerInstance)(
    LPCOLESTR   lpszScheme,
    LPCOLESTR   lpszHost,
    HINSTANCE hInstResources);

  //-------------------------------------------------------------------------
  // unregisters a file protocol previously registered with
  // one of the RegisterTemporaryXXXHandler methods
  STDMETHOD(UnregisterTemporaryFolderHandler)(
    LPCOLESTR lpszScheme,
    LPCOLESTR lpszHost);

  //-------------------------------------------------------------------------
  // unregisters a resource protocol previously registered with
  // one of the RegisterTemporaryXXXHandler methods
  STDMETHOD(UnregisterTemporaryResourceHandler)(
    LPCOLESTR lpszScheme,
    LPCOLESTR lpszHost);

  //-------------------------------------------------------------------------
  // adds a URL where the content resides in memory.
  STDMETHOD(AddResource)(
    LPCOLESTR lpszURL,
    const void * lpData,
    DWORD dwLength,
    LPCOLESTR lpszMimeType);

  STDMETHOD(watchBrowser)(IWebBrowser2* aWebBrowser, IWebRequestEvents * aEvents);
  STDMETHOD(unwatchBrowser)(IWebBrowser2* aWebBrowser, IWebRequestEvents * aEvents);
  STDMETHOD(watchAll)(IWebRequestEvents * aEvents);
  STDMETHOD(unwatchAll)(IWebRequestEvents * aEvents);

private:  // methods
  //--------------------------------------------------------------------------
  HRESULT patchProtocolInternal(Scheme aScheme);
  HRESULT unpatchProtocolInternal(Scheme aScheme);
  HRESULT registerProtocolInternal(Scheme aScheme);
  HRESULT unregisterProtocolInternal(Scheme aScheme);

private:  // members
  //--------------------------------------------------------------------------
  enum THandlerType {
    NONE,
    PAPP,   // conventional pluggable protocol based on passthru-app
    PATCH   // vtable patch based protocol handler
  };

  THandlerType mHandlerType;
  CComPtr<IClassFactory> mClassFactoryHTTP;
  CComPtr<IClassFactory> mClassFactoryHTTPS;
};

OBJECT_ENTRY_AUTO(CLSID_ProtPatchApplication, Application)

} // namespace protocolpatchLib
