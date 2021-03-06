/****************************************************************************
 * Application.cpp : Implementation of Application
 * Copyright 2013 Salsita Software (http://www.salsitasoft.com).
 * Author: Arne Seib <arne@salsitasoft.com>
 ****************************************************************************/

#include "stdafx.h"
#include "IInternetProtocolCFPatch.h"
#include "handler/ProtocolHandlerRegistrar.h"

#include "ThreadRecord.h"
#include "Application.h"

namespace protocolpatchLib
{

typedef PassthroughAPP::CMetaFactory<PassthroughAPP::CComClassFactoryProtocol,
  Protocol> MetaFactoryProtocol;

//--------------------------------------------------------------------------
// getInstance
//  Singleton: Handled by COM.
CComPtr<IProtPatchApplication> Application::getInstance()
{
  CComPtr<IProtPatchApplication> protocolPatchApp;
  // get class factory
  CComPtr<IClassFactory> classFactory;
  if (SUCCEEDED(DllGetClassObject(CLSID_ProtPatchApplication, IID_IClassFactory, (LPVOID*)&classFactory.p))) {
    classFactory->CreateInstance(NULL, IID_IProtPatchApplication, (void**)&protocolPatchApp.p);
  }
  return protocolPatchApp;
}

//--------------------------------------------------------------------------
// patchProtokol
//  Patches protocol aScheme. Call as soon as possible.
STDMETHODIMP Application::patchProtokol(Scheme aScheme, BOOL aUseVTablePatching)
{
  THandlerType type = (aUseVTablePatching) ? PATCH : PAPP;
  if (NONE != mHandlerType && type != mHandlerType) {
    return E_UNEXPECTED;  // already patched
  }
  mHandlerType = type;
  switch(mHandlerType) {
    case PAPP:
      return registerProtocolInternal(aScheme);
    case PATCH:
      return patchProtocolInternal(aScheme);

  }
  return E_INVALIDARG;  // not reached
}

//--------------------------------------------------------------------------
// unpatchProtokol
//  Unpatches protocol aScheme. Probably don't call at all, it's just there
//  for completeness.
STDMETHODIMP Application::unpatchProtokol(Scheme aScheme)
{
  switch(mHandlerType) {
    case PAPP:
      return unregisterProtocolInternal(aScheme);
    case PATCH:
      return unpatchProtocolInternal(aScheme);
  }
  return E_UNEXPECTED;  // if not patched
}

//--------------------------------------------------------------------------
// enableProtokol
//  Enables / disables protocol aScheme.
STDMETHODIMP Application::enableProtokol(Scheme aScheme, BOOL aEnable)
{
  if (PATCH != mHandlerType) {
    return E_UNEXPECTED;  // already patched
  }
  switch(aScheme) {
    case SCHEME_HTTP:
      IInternetProtocolCFPatchHTTP::enable(aEnable);
      return S_OK;
    case SCHEME_HTTP_S:
      IInternetProtocolCFPatchHTTP_S::enable(aEnable);
      return S_OK;
  }
  return E_INVALIDARG;
}

//-------------------------------------------------------------------------
// registers a temporary file protocol of the form
// lpszScheme://lpszHost/
// where lpszFolder points to the root folder of this "server"
STDMETHODIMP Application::RegisterTemporaryFolderHandler(
  LPCOLESTR lpszScheme,
  LPCOLESTR lpszHost,
  LPCOLESTR lpszFolder)
{
  return CProtocolHandlerRegistrar::
      RegisterTemporaryFolderHandler(lpszScheme, lpszHost, lpszFolder);
}

//-------------------------------------------------------------------------
// registers a temporary resource protocol of the form
// lpszScheme://lpszHost/
// where lpszResourceFileName points to the file (DLL or exe) holding the
// resources
STDMETHODIMP Application::RegisterTemporaryResourceHandlerFile(
  LPCOLESTR lpszScheme,
  LPCOLESTR lpszHost,
  LPCOLESTR lpszResourceFileName)
{
  return CProtocolHandlerRegistrar::
      RegisterTemporaryResourceHandler(lpszScheme, lpszHost, lpszResourceFileName);
}

//-------------------------------------------------------------------------
// registers a temporary resource protocol of the form
// lpszScheme://lpszHost/
// where hInstResources is a module handle of the file (DLL or exe) holding
// the resources
STDMETHODIMP Application::RegisterTemporaryResourceHandlerInstance(
  LPCOLESTR   lpszScheme,
  LPCOLESTR   lpszHost,
  HINSTANCE hInstResources)
{
  return CProtocolHandlerRegistrar::
      RegisterTemporaryResourceHandler(lpszScheme, lpszHost, hInstResources);
}

//-------------------------------------------------------------------------
// unregisters a file protocol previously registered with
// one of the RegisterTemporaryXXXHandler methods
STDMETHODIMP Application::UnregisterTemporaryFolderHandler(
  LPCOLESTR lpszScheme,
  LPCOLESTR lpszHost)
{
  return CProtocolHandlerRegistrar::
      UnregisterTemporaryFolderHandler(lpszScheme, lpszHost);
}

//-------------------------------------------------------------------------
// unregisters a resource protocol previously registered with
// one of the RegisterTemporaryXXXHandler methods
STDMETHODIMP Application::UnregisterTemporaryResourceHandler(
  LPCOLESTR lpszScheme,
  LPCOLESTR lpszHost)
{
  return CProtocolHandlerRegistrar::
      UnregisterTemporaryResourceHandler(lpszScheme, lpszHost);
}

//-------------------------------------------------------------------------
// adds a URL where the content resides in memory.
STDMETHODIMP Application::AddResource(
  LPCOLESTR lpszURL,
  const void * lpData,
  DWORD dwLength,
  LPCOLESTR lpszMimeType)
{
  return CProtocolHandlerRegistrar::
      AddResource(lpszURL, lpData, dwLength, lpszMimeType);
}

//--------------------------------------------------------------------------
// watchBrowser
//  Add an event handler for a certain top level browser.
//  This is meant to be called as soon as a browser is available,
//  so in a BHO in SetSite().
STDMETHODIMP Application::watchBrowser(IWebBrowser2* aWebBrowser, IWebRequestEvents * aEvents)
{
  // get or create a record for the current thread
  CComPtr<IThreadRecord> threadRecord = ThreadRecord::get();
  if (!threadRecord) {
    threadRecord = ThreadRecord::create();
  }
  if (!threadRecord) {
    return E_FAIL;
  }
  return threadRecord->watchBrowser(aWebBrowser, aEvents);
}

//--------------------------------------------------------------------------
// unwatchBrowser
//  Removes the event handler, and, if the last one, also the browser record
//  from the global list.
STDMETHODIMP Application::unwatchBrowser(IWebBrowser2* aWebBrowser, IWebRequestEvents * aEvents)
{
  // get the record for the current thread
  CComPtr<IThreadRecord> threadRecord = ThreadRecord::get();
  if (!threadRecord) {
    return E_FAIL;
  }
  // remove the handler
  HRESULT hr = threadRecord->unwatchBrowser(aWebBrowser, aEvents);
  if (S_OK == hr) {
  // and if this was the last, remove the thread record
    return ThreadRecord::remove();
  }
  return S_OK;
}

//--------------------------------------------------------------------------
// watchAll
//  Add an event handler for a certain top level browser.
//  This is meant to be called as soon as a browser is available,
//  so in a BHO in SetSite().
STDMETHODIMP Application::watchAll(IWebRequestEvents * aEvents)
{
  // get or create a record for the current thread
  CComPtr<IThreadRecord> threadRecord = ThreadRecord::get();
  if (!threadRecord) {
    threadRecord = ThreadRecord::create();
  }
  if (!threadRecord) {
    return E_FAIL;
  }
  return threadRecord->watchAll(aEvents);
}

//--------------------------------------------------------------------------
// unwatchAll
//  Removes the event handler, and, if the last one, also the browser record
//  from the global list.
STDMETHODIMP Application::unwatchAll(IWebRequestEvents * aEvents)
{
  // get the record for the current thread
  CComPtr<IThreadRecord> threadRecord = ThreadRecord::get();
  if (!threadRecord) {
    return E_FAIL;
  }
  // remove the handler
  HRESULT hr = threadRecord->unwatchAll(aEvents);
  if (S_OK == hr) {
  // and if this was the last, remove the thread record
    return ThreadRecord::remove();
  }
  return S_OK;
}

//--------------------------------------------------------------------------
// patchProtocolInternal
HRESULT Application::patchProtocolInternal(Scheme aScheme)
{
  switch(aScheme) {
    case SCHEME_HTTP:
      return IInternetProtocolCFPatchHTTP::patch();
    case SCHEME_HTTP_S:
      return IInternetProtocolCFPatchHTTP_S::patch();
  }
  return E_INVALIDARG;
}

//--------------------------------------------------------------------------
// unpatchProtocolInternal
HRESULT Application::unpatchProtocolInternal(Scheme aScheme)
{
  if (PATCH != mHandlerType) {
    return E_UNEXPECTED;
  }
  switch(aScheme) {
    case SCHEME_HTTP:
      return IInternetProtocolCFPatchHTTP::restore();
    case SCHEME_HTTP_S:
      return IInternetProtocolCFPatchHTTP_S::restore();
  }
  return E_INVALIDARG;
}

//--------------------------------------------------------------------------
// registerProtocolInternal
HRESULT Application::registerProtocolInternal(Scheme aScheme)
{
  CComPtr<IInternetSession> internetSession;
  HRESULT hr = CoInternetGetSession(0, &internetSession, 0);
  if (FAILED(hr)) {
    return hr;
  }

  switch(aScheme) {
    case SCHEME_HTTP:
      hr = MetaFactoryProtocol::CreateInstance(
          SchemeTraits<SCHEME_HTTP>::getCLSID(),
          &mClassFactoryHTTP);

      if (SUCCEEDED(hr)) {
        hr = internetSession->RegisterNameSpace(
          mClassFactoryHTTP,
          CLSID_NULL,
          SchemeTraits<SCHEME_HTTP>::getSchemeName(),
          0, 0, 0);
      }
      break;
    case SCHEME_HTTP_S:
      hr = MetaFactoryProtocol::CreateInstance(
          SchemeTraits<SCHEME_HTTP_S>::getCLSID(),
          &mClassFactoryHTTPS);

      if (SUCCEEDED(hr)) {
        hr = internetSession->RegisterNameSpace(
          mClassFactoryHTTPS,
          CLSID_NULL,
          SchemeTraits<SCHEME_HTTP_S>::getSchemeName(),
          0, 0, 0);
      }
      break;
    default:
      return E_INVALIDARG;
  }
  if (FAILED(hr)) {
    return hr;
  }
  return S_OK;
}

//--------------------------------------------------------------------------
// unregisterProtocolInternal
HRESULT Application::unregisterProtocolInternal(Scheme aScheme)
{
  if (PAPP != mHandlerType) {
    return E_UNEXPECTED;
  }
  CComPtr<IInternetSession> internetSession;
  HRESULT hr = CoInternetGetSession(0, &internetSession, 0);
  if (FAILED(hr)) {
    return hr;
  }

  switch(aScheme) {
    case SCHEME_HTTP:
      internetSession->UnregisterNameSpace(mClassFactoryHTTP, SchemeTraits<SCHEME_HTTP>::getSchemeName());
      mClassFactoryHTTP.Release();
      return S_OK;
    case SCHEME_HTTP_S:
      internetSession->UnregisterNameSpace(mClassFactoryHTTPS, SchemeTraits<SCHEME_HTTP_S>::getSchemeName());
      mClassFactoryHTTPS.Release();
      return S_OK;
  }
  return E_INVALIDARG;
}

} // namespace protocolpatchLib
