/****************************************************************************
 * ProtocolHandlerRegistrar.cpp : Implementation of CProtocolHandlerRegistrar
 * Copyright 2013 Salsita Software (http://www.salsitasoft.com).
 * Author: Arne Seib <arne@salsitasoft.com>
 ****************************************************************************/

#include "StdAfx.h"
#include "TemporaryProtocolFolderHandler.h"
#include "TemporaryProtocolFolderHandlerClassFactory.h"
#include "TemporaryProtocolResourceHandler.h"
#include "TemporaryProtocolResourceHandlerClassFactory.h"
#include "ProtocolHandlerRegistrar.h"

/*****************************************************************************
 * class Dummy
 *  Just a dummy to satisfy template<class CF> HRESULT RegisterTemporaryHandler()
 *  and template<class CF> HRESULT UnregisterTemporaryHandler()
 *****************************************************************************/
class Dummy
{
};

/*****************************************************************************
 * class CProtocolHandlerRegistrar
 *****************************************************************************/
CProtocolHandlerRegistrar & CProtocolHandlerRegistrar::GetInstance()
{
  static CProtocolHandlerRegistrar instance;
  return instance;
}

//---------------------------------------------------------------------------
// ctor
CProtocolHandlerRegistrar::CProtocolHandlerRegistrar(void)
{
  InitializeCriticalSection(&m_CriticalSection);
}

//---------------------------------------------------------------------------
// dtor
CProtocolHandlerRegistrar::~CProtocolHandlerRegistrar(void)
{
  DeleteCriticalSection(&m_CriticalSection);
}

//---------------------------------------------------------------------------
// RegisterTemporaryFolderHandler
HRESULT CProtocolHandlerRegistrar::RegisterTemporaryFolderHandler(
  LPCWSTR lpszScheme,
  LPCWSTR lpszHost,
  LPCWSTR lpszFolder)
{
  return GetInstance().
    RegisterTemporaryHandler<CTemporaryProtocolFolderHandlerClassFactory>
    (lpszScheme, lpszHost, CComVariant(lpszFolder));
}

//---------------------------------------------------------------------------
// RegisterTemporaryResourceHandler
HRESULT CProtocolHandlerRegistrar::RegisterTemporaryResourceHandler(
  LPCWSTR lpszScheme,
  LPCWSTR lpszHost,
  LPCWSTR lpszResourceFileName)
{
  return GetInstance().
    RegisterTemporaryHandler<CTemporaryProtocolResourceHandlerClassFactory>
    (lpszScheme, lpszHost, CComVariant(lpszResourceFileName));
}

//---------------------------------------------------------------------------
// RegisterTemporaryResourceHandler
HRESULT CProtocolHandlerRegistrar::RegisterTemporaryResourceHandler(
  LPCWSTR   lpszScheme,
  LPCWSTR   lpszHost,
  HINSTANCE hInstResources)
{
  VARIANT vt;
  vt.vt = VT_BYREF;
  vt.byref = reinterpret_cast<PVOID>(hInstResources);
  return GetInstance().
    RegisterTemporaryHandler<CTemporaryProtocolResourceHandlerClassFactory>
    (lpszScheme, lpszHost, vt);
}

//---------------------------------------------------------------------------
// RegisterTemporaryCustomHandler
HRESULT CProtocolHandlerRegistrar::RegisterTemporaryCustomHandler(
  LPCWSTR   lpszScheme,
  LPCWSTR   lpszHost,
  VARIANT   aResourceId,
  IProtocolClassFactory * aClassFactory)
{
  return GetInstance().
    RegisterTemporaryHandler<Dummy>
    (lpszScheme, lpszHost, aResourceId, aClassFactory);
}

//---------------------------------------------------------------------------
// UnregisterTemporaryFolderHandler
HRESULT CProtocolHandlerRegistrar::UnregisterTemporaryFolderHandler(
  LPCWSTR lpszScheme,
  LPCWSTR lpszHost)
{
  return GetInstance().
    UnregisterTemporaryHandler<CTemporaryProtocolFolderHandlerClassFactory>(lpszScheme, lpszHost);
}

//---------------------------------------------------------------------------
// UnregisterTemporaryResourceHandler
HRESULT CProtocolHandlerRegistrar::UnregisterTemporaryResourceHandler(
  LPCWSTR lpszScheme,
  LPCWSTR lpszHost)
{
  return GetInstance().
    UnregisterTemporaryHandler<CTemporaryProtocolResourceHandlerClassFactory>(lpszScheme, lpszHost);
}

//---------------------------------------------------------------------------
// UnregisterTemporaryCustomHandler
HRESULT CProtocolHandlerRegistrar::UnregisterTemporaryCustomHandler(
  LPCWSTR lpszScheme,
  LPCWSTR   lpszHost)
{
  return GetInstance().
    UnregisterTemporaryHandler<Dummy>(lpszScheme, lpszHost);
}

//---------------------------------------------------------------------------
// AddResource
HRESULT CProtocolHandlerRegistrar::AddResource(
  LPCWSTR lpszURL,
  LPCVOID lpData,
  DWORD dwLength,
  LPCWSTR lpszMimeType)
{
  return GetInstance().
    InternalAddResource(lpszURL, lpData, dwLength, lpszMimeType);
}

//---------------------------------------------------------------------------
// InternalAddURL
HRESULT CProtocolHandlerRegistrar::InternalAddResource(
  LPCWSTR lpszURL,
  LPCVOID lpData,
  DWORD dwLength,
  LPCWSTR lpszMimeType)
{
  // TODO: implement
  CComPtr<IUri> pURI;
  IF_FAILED_RET(::CreateUri(lpszURL, Uri_CREATE_CANONICALIZE, 0, &pURI));

  CritSectLock lock(m_CriticalSection);

  CComPtr<IProtocolClassFactory> pClassFactory;

  CComBSTR scheme, host;
  IF_FAILED_RET(pURI->GetSchemeName(&scheme));
  IF_FAILED_RET(pURI->GetHost(&host));

  // lookup class factory for lpszScheme
  if (!m_ClassFactories.Lookup(scheme, pClassFactory)) {
    // a protocol handler for this scheme has to be registered first!
    return E_UNEXPECTED;
  }

  CComQIPtr<IProtocolMemoryResource> memoryResource(pClassFactory);
  if (!memoryResource) {
    // Class factory for this handler does not support memory based resources
    return E_NOINTERFACE;
  }

  // add the URL
  return memoryResource->AddResource(pURI, lpData, dwLength, lpszMimeType);
}

//---------------------------------------------------------------------------
// RegisterTemporaryHandler
template<class CF>
HRESULT CProtocolHandlerRegistrar::RegisterTemporaryHandler(
  LPCWSTR lpszScheme,
  LPCWSTR lpszHost,
  VARIANT & aResourceId,
  IProtocolClassFactory * aClassFactory)
{
  CritSectLock lock(m_CriticalSection);
  BOOL registered = FALSE;

  CComQIPtr<IProtocolClassFactory> classFactory(aClassFactory);

  CComPtr<IProtocolClassFactory> classFactoryFound;
  registered = m_ClassFactories.Lookup(lpszScheme, classFactoryFound);
  if (!registered) {
    // classfactory for scheme is not registered yet
    if (nullptr == classFactory) {
      // no classfactory given, create one
      __if_exists (CF::CreateCFInstance) {
        IF_FAILED_RET(CF::CreateCFInstance(classFactory));
      }
    }
    IF_FAILED_RET(classFactory->Init(lpszScheme));
  }
  else {
    // already registered:
    if (classFactory && !classFactory.IsEqualObject(classFactoryFound)) {
      // not allowed to replace an existing class factory!
      return E_INVALIDARG;
    }
    classFactory = classFactoryFound;
  }


  // now we should have a classfactory in any case
  if (!classFactory) {
    return E_INVALIDARG;
  }

  // add the host
  IF_FAILED_RET(classFactory->AddHost(lpszHost, aResourceId));

  // register protocol handler
  if (!registered) {
    // get IInternetSession
    CComPtr<IInternetSession> pInternetSession;
    IF_FAILED_RET(CoInternetGetSession(0, &pInternetSession, 0));

    IF_FAILED_RET(pInternetSession->RegisterNameSpace(
      classFactory,
      CTemporaryProtocolFolderHandler::CLSID,
      lpszScheme,
      0, NULL, 0));

    // store classfactory object
    m_ClassFactories[lpszScheme] = classFactory;
  }

  return S_OK;
}

//---------------------------------------------------------------------------
// UnregisterTemporaryHandler
template<class CF>
HRESULT CProtocolHandlerRegistrar::UnregisterTemporaryHandler(
  LPCWSTR lpszScheme,
  LPCWSTR lpszHost)
{
  HRESULT found = S_FALSE;  // not found
  CritSectLock lock(m_CriticalSection);
  // lookup classfactory object for lpszHost
  CComPtr<IProtocolClassFactory> pClassFactory;

  if ( m_ClassFactories.Lookup(lpszScheme, pClassFactory) ) {
    found = S_OK;
    // unregister host
    size_t registeredHosts = pClassFactory->RemoveHost(lpszHost);
    if (registeredHosts > 0) {
      // the handler has still registered hosts
      return found;
    }
  }

  // there are no more registered hosts, so remove the registration

  // get IInternetSession
  CComPtr<IInternetSession> pInternetSession;
  IF_FAILED_RET(CoInternetGetSession(0, &pInternetSession, 0));

  // unregister
  pInternetSession->UnregisterNameSpace(pClassFactory, lpszScheme);

  // and remove from map
  m_ClassFactories.RemoveKey(lpszScheme);

  return found;
}
