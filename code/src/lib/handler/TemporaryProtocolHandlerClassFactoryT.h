#pragma once

#include <atlcoll.h>

#include "interfaces.h"

/*****************************************************************************
 * class CTemporaryProtocolHandlerClassFactoryT
 *  Implements IProtocolClassFactory.
 *  Creates instances of CTemporaryProtocolFolderHandler.
 *****************************************************************************/
template<class T, class H, class HI>
  class ATL_NO_VTABLE CTemporaryProtocolHandlerClassFactoryT :
    public IProtocolClassFactory
{
public:
  friend class CProtocolHandlerRegistrar;

  static HRESULT CreateCFInstance(CComPtr<IProtocolClassFactory> & aRetVal)
  {
    T::_ComObject * newInstance = nullptr;
    HRESULT hr = T::_ComObject::CreateInstance(&newInstance);
    if (FAILED(hr)) {
      return hr;
    }
    aRetVal = newInstance;
    return S_OK;
  }

  //----------------------------------------------------------------------------
  // CTOR / DTOR
  CTemporaryProtocolHandlerClassFactoryT()
  {
    InitializeCriticalSection(&m_CriticalSection);
  }

  virtual ~CTemporaryProtocolHandlerClassFactoryT()
  {
    DeleteCriticalSection(&m_CriticalSection);
  }

  //-------------------------------------------------------------------------
  // called from handlers to check if this is our scheme
  BOOL CheckScheme(
    BSTR bsScheme)
  {
    return (m_sScheme == bsScheme);
  }

  //-------------------------------------------------------------------------
  // called from handlers to get the resource (host) info for a request
  BOOL GetResourceInfo(
    LPCWSTR lpszHost, HI & hostInfo)
  {
    CritSectLock lock(m_CriticalSection);
    return m_HostInfos.Lookup(lpszHost, hostInfo);
  }

public:
  //----------------------------------------------------------------------------
  // IClassFactory implementation
  STDMETHOD(CreateInstance)(
    IUnknown *pUnkOuter,
    REFIID riid, void **ppvObject)
  {
    if (pUnkOuter) {
      // we don't support aggregation
      return CLASS_E_NOAGGREGATION;
    }

    // the handler instance
    CComObject<H> *pHandler = NULL;

    // create the handler instance
    IF_FAILED_RET(CComObject<H>::CreateInstance(&pHandler));

    // make lifetime management safe
    CComPtr<IInternetProtocol> pInternetProtocol = pHandler;

    // and init the handler
    IF_FAILED_RET(static_cast<T*>(this)->InitHandler(pHandler));

    // return whatever interface caller wants or E_NOINTERFACE
    return pHandler->QueryInterface(riid, ppvObject);
  }

  STDMETHOD(LockServer)(
    BOOL fLock)
  {
    // this method is not used
    return S_OK;
  }

  //----------------------------------------------------------------------------
  // IProtocolClassFactory implementation
  STDMETHOD(Init)(
    LPCWSTR aScheme)
  {
    if (!wcslen(aScheme)) {
      return E_INVALIDARG;
    }
    m_sScheme = aScheme;
    return S_OK;
  }

  STDMETHOD_(size_t, RemoveHost)(
    LPCWSTR aHostname)
  {
    CritSectLock lock(m_CriticalSection);
    m_HostInfos.RemoveKey(aHostname);
    return m_HostInfos.GetCount();
  }

protected:
  //----------------------------------------------------------------------------

  // lookup a host
  BOOL LookupHostInfo(CStringW key, HI & val)
  {
    CritSectLock lock(m_CriticalSection);
    return m_HostInfos.Lookup(key, val);
  }

  // check if a host exists
  BOOL HaveHostInfo(CStringW key)
  {
    CritSectLock lock(m_CriticalSection);
    return (m_HostInfos.Lookup(key) != nullptr);
  }

  // set a host
  void SetHostInfo(CStringW key, HI & val)
  {
    CritSectLock lock(m_CriticalSection);
    m_HostInfos[key] = val;
  }

protected:
  //----------------------------------------------------------------------------
  // protected data members

  // critical section protecting hosts map
  CRITICAL_SECTION      m_CriticalSection;

private:
  //----------------------------------------------------------------------------
  // private data members

  // the scheme. See ProtocolHandlerRegistrar.h
  CStringW              m_sScheme;

  // map for registered hosts: maps a host name to a HostInfo
  CAtlMap<CStringW, HI> m_HostInfos;
};
