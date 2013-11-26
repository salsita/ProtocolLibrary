/****************************************************************************
 * IInternetProtocolCFPatch.h : Declaration of IInternetProtocolCFPatch.
 * Copyright 2013 Salsita (http://www.salsitasoft.com).
 * Author: Arne Seib <arne@salsitasoft.com>
 ****************************************************************************/
#pragma once

#include "VTablePatch.h"
#include "Scheme.h"
#include "Protocol.h"
#include "papp/ProtocolCF.h"
#include "ThreadRecord.h"

namespace protocolpatchLib
{

typedef PassthroughAPP::CMetaFactory<PassthroughAPP::CComClassFactoryProtocol,
  Protocol> MetaFactory;


/*============================================================================
 * class IInternetProtocolCFPatch
 *  IClassFactory -> IUnknown
 */
template<class TSchemeTraits> class IInternetProtocolCFPatch :
    public IUnknownPatch
{
public:
  typedef IInternetProtocolCFPatch<TSchemeTraits> _PatchClass;

  //--------------------------------------------------------------------------
  // the one and only instance
  static _PatchClass & getInst()
  {
    static _PatchClass sInstance;
    return sInstance;
  }

  //--------------------------------------------------------------------------
  // static patch method
  static HRESULT patch()
  {
    // load urlmon
    HMODULE module = ::GetModuleHandle(_T("urlmon.dll"));
    ATLASSERT(module);
    if (!module) {
      return AtlHresultFromLastError();
    }

    // get DllGetClassObject export
    typedef HRESULT (WINAPI* DllGetClassObjectFn)(REFCLSID, REFIID, LPVOID*);
    DllGetClassObjectFn DllGetClassObjectF = (DllGetClassObjectFn)::GetProcAddress(module, "DllGetClassObject");
    ATLASSERT(DllGetClassObjectF);
    if (!DllGetClassObjectF) {
      return AtlHresultFromLastError();
    }

    // get class factory for protocol implementation
    CComPtr<IClassFactory> classFactory;
    HRESULT hr = DllGetClassObjectF(TSchemeTraits::getCLSID(), IID_IClassFactory, (void**)&classFactory.p);
    ATLASSERT(SUCCEEDED(hr));
    if (FAILED(hr)) {
      return hr;
    }

    // patch
    return getInst()._patch(UNK_VTABLE(classFactory), TSchemeTraits::getScheme());
  }

  //--------------------------------------------------------------------------
  // static restore method
  static HRESULT restore()
      { return getInst()._restore(); }

  //--------------------------------------------------------------------------
  // enable / disable patches for this vtable
  static void enable(BOOL aEnable = TRUE)
      { getInst().mEnabled = aEnable; }

  //--------------------------------------------------------------------------
  // isEnabled
  static BOOL isEnabled()
      { return getInst().mEnabled; }

  //--------------------------------------------------------------------------
  // public default CTOR
  IInternetProtocolCFPatch() : mVTable(NULL), mEnabled(FALSE)
      { }

  DECLARE_VTABLE_PATCH(CreateInstance,
      /* [unique][in] */ IUnknown *pUnkOuter,
      /* [in] */ REFIID riid,
      /* [iid_is][out] */ void **ppvObject)
  {
    if (!ppvObject) {
      return E_POINTER;
    }
    _PatchClass & instance = getInst();
    CComPtr<IClassFactory> classFactory;
    HRESULT hr = instance.getClassFactory(&classFactory.p);
    if (FAILED(hr)) {
      return hr;
    }

    // create native protocol instance
    CComPtr<IUnknown> nativeProtocol;
    hr = instance.do_CreateInstance(aInstance, 0, IID_IUnknown, (void**)&nativeProtocol.p);
    if (FAILED(hr)) {
      return hr;
    }
    // uh?? sometimes this class factory does not create a IInternetProtocol - well, however,
    // in this case return just the original instance
    CComQIPtr<IInternetProtocol> testForIInternetProtocol(nativeProtocol);
    if (!testForIInternetProtocol) {
      return nativeProtocol->QueryInterface(riid, ppvObject);
    }

    // create our own instance
    CComPtr<IUnknown> ourProtocol;
    hr = classFactory->CreateInstance(pUnkOuter, IID_IUnknown, (void**)&ourProtocol.p);
    if (FAILED(hr)) {
      return hr;
    }

		CComQIPtr<IPassthroughObject> spPassthroughObj(ourProtocol);
    ATLASSERT(spPassthroughObj);
		hr = spPassthroughObj->SetTargetUnknown(nativeProtocol);
    ATLASSERT(SUCCEEDED(hr));

    return ourProtocol->QueryInterface(riid, ppvObject);
  }

protected:
  //--------------------------------------------------------------------------
  // vtable indices for IClassFactory
  enum VtableIndex {
    VTI_CreateInstance = __super::VTI_NEXT__,
    VTI_LockServer,
    VTI_NEXT__
  };

private:
  HRESULT getClassFactory(IClassFactory ** aRetVal)
  {
    if (!mClassFactory) {
      HRESULT hr = MetaFactory::CreateInstance(TSchemeTraits::getCLSID(), &mClassFactory.p);
      if (FAILED(hr)) {
        return hr;
      }
    }
    return mClassFactory.CopyTo(aRetVal);
  }
  // our vtable
  VTABLE  mVTable;
  // flag: patch enabled or not
  BOOL    mEnabled;
  CComPtr<IClassFactory> mClassFactory;

  BEGIN_PATCH_MAP(_PatchClass)
    // IClassFactory
    PATCH_MAP_ENTRY(CreateInstance)
  END_PATCH_MAP()

  FORBID_COPY_CONSTRUCTOR(IInternetProtocolCFPatch);
};

/*============================================================================
 * Types
 */
typedef IInternetProtocolCFPatch< HTTP_Traits >
          IInternetProtocolCFPatchHTTP;

typedef IInternetProtocolCFPatch< HTTP_S_Traits >
          IInternetProtocolCFPatchHTTP_S;

} // namespace protocolpatchLib
