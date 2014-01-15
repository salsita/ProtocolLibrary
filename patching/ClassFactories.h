/****************************************************************************
 * ClassFactories.h : 
 * Copyright 2013 Salsita (http://www.salsitasoft.com).
 * Author: Arne Seib <arne@salsitasoft.com>
 ****************************************************************************/
#pragma once
#include "papp/ProtocolCF.h"

namespace protocolpatchLib
{

/*============================================================================
 * class CComClassFactoryPatch
 *  A copy of papp's CComClassFactoryProtocol but without the target-creating
 *  functionality. Used for vtable-patch handlers.
 */

class ATL_NO_VTABLE CComClassFactoryPatch :
  public ATL::CComClassFactory
{
  typedef ATL::CComClassFactory BaseClass;
public:
  STDMETHODIMP CreateInstance(IUnknown* punkOuter, REFIID riid,
    void** ppvObj)
  {
    ATLASSERT(ppvObj != 0);
    if (!ppvObj)
    {
      return E_POINTER;
    }
    *ppvObj = 0;
    CComPtr<IUnknown> spUnkObject;
    HRESULT hr = BaseClass::CreateInstance(punkOuter, riid,
      reinterpret_cast<void**>(&spUnkObject));
    ATLASSERT(SUCCEEDED(hr) && spUnkObject != 0);
    if (SUCCEEDED(hr))
    {
      *ppvObj = spUnkObject.Detach();
    }
    return hr;
  }

  HRESULT GetTargetClassFactory(IClassFactory** ppCF)
  {
    ObjectLock lock(this);
    return m_spTargetCF.CopyTo(ppCF);
  }

  HRESULT SetTargetClassFactory(IClassFactory* pCF)
  {
    HRESULT hr = (pCF ? pCF->LockServer(TRUE) : S_OK);
    if (SUCCEEDED(hr))
    {
      ObjectLock lock(this);
      if (m_spTargetCF)
      {
        // LockServer(FALSE) is assumed to always succeed. Otherwise,
        // it is impossible to implement correct semantics
        HRESULT hr1 = m_spTargetCF->LockServer(FALSE);
        hr1;
        ATLASSERT(SUCCEEDED(hr1));
      }
      m_spTargetCF = pCF;
    }
    return hr;
  }
  
  HRESULT SetTargetCLSID(REFCLSID clsid, DWORD clsContext = CLSCTX_ALL)
  {
    CComPtr<IClassFactory> spTargetCF;
    HRESULT hr = CoGetClassObject(clsid, clsContext, 0, IID_IClassFactory,
      reinterpret_cast<void**>(&spTargetCF));
    ATLASSERT(SUCCEEDED(hr) && spTargetCF != 0);
    if (SUCCEEDED(hr))
    {
      hr = SetTargetClassFactory(spTargetCF);
      ATLASSERT(SUCCEEDED(hr));
    }
    return hr;
  }

  void FinalRelease()
  {
    // No need to be thread safe here
    if (m_spTargetCF)
    {
      // LockServer(FALSE) is assumed to always succeed.
      HRESULT hr = m_spTargetCF->LockServer(FALSE);
      hr;
      ATLASSERT(SUCCEEDED(hr));

      m_spTargetCF.Release();
    }
  }

private:
  CComPtr<IClassFactory> m_spTargetCF;
};



} // namespace protocolpatchLib
