/****************************************************************************
 * VTablePatch.h : Patching virtual function tables.
 * Copyright 2013 Salsita (http://www.salsitasoft.com).
 * Author: Arne Seib <arne@salsitasoft.com>
 ****************************************************************************/

#pragma once

#include "GlobalMap.h"

namespace protocolpatchLib
{
/*============================================================================
 * types
 */

// VTABLE type: a pointer to a vtable (array of PROC)
typedef PROC* VTABLE;


// A global map with all patched vtables. We provide this map to have a way
// to check if a certain vtable is already patched.
// Value is an integer, can be used as an ID or simply as a flag.
typedef GlobalMapAccessor< std::unordered_map<VTABLE, int> > GlobalVtableMap;

/*============================================================================
 * class VTablePatch
 *  This class provides a typesafe patch for one single method in virtual
 *  function table.
 *  Template argument is the type of the method being patched.
 *  For IUnknown::QueryInterface it would be:
 *      HRESULT (STDMETHODCALLTYPE *)(IUnknown *, REFIID, void**)
 *  NOTE: The first argument is always the "this" pointer of the current
 *  instance.
 */

template<typename Tfn>
class VTablePatch
{
public:
  VTablePatch() :
    mTargetAddress(NULL), mOriginalFn(NULL)
  { }

  ~VTablePatch() {
    restore();
  }

  // restore: Remove the hook previously set.
  HRESULT restore() {
    if (!mTargetAddress || !mOriginalFn) {
      // we are already restored
      return S_FALSE;
    }
    DWORD dwOldProt = 0;
    if( !VirtualProtect(mTargetAddress, sizeof(PROC), PAGE_EXECUTE_READWRITE, &dwOldProt) ) {
      return AtlHresultFromLastError();
    }
    (*mTargetAddress) = mOriginalFn;
    VirtualProtect(mTargetAddress, sizeof(PROC), dwOldProt, &dwOldProt);
    mOriginalFn = NULL;
    mTargetAddress = NULL;
    return S_OK;
  }

  // patch: Set the patch and remember original method.
  HRESULT patch(VTABLE aVtable, size_t aMethodIndex, Tfn aFn) {
    if (mTargetAddress || isHooked(aVtable, aMethodIndex, aFn)) {
      // we are already hooked
      return S_FALSE;
    }
    DWORD dwOldProt = 0;
    if( !VirtualProtect(&aVtable[aMethodIndex], sizeof(PROC), PAGE_EXECUTE_READWRITE, &dwOldProt) ) {
      return AtlHresultFromLastError();
    }
    mOriginalFn = reinterpret_cast<Tfn>(aVtable[aMethodIndex]);
    aVtable[aMethodIndex] = reinterpret_cast<PROC>(aFn);
    VirtualProtect(&aVtable[aMethodIndex], sizeof(PROC), dwOldProt, &dwOldProt);
    // remember address for restoring
    mTargetAddress = reinterpret_cast<Tfn*>(&aVtable[aMethodIndex]);
    return S_OK;
  }

  // isHooked: Check if we have already a hook on this method.
  static BOOL isHooked(VTABLE aVtable, size_t aMethodIndex, Tfn aFn) {
    return (aVtable[aMethodIndex] == (PROC)aFn);
  }

  // This acts as a (...) operator for calling the original method.
  // So if you have a patch:
  //  VTablePatch<HRESULT (STDMETHODCALLTYPE *)(IUnknown *, REFIID, void**)>
  //     mMyQueryInterface;
  // you can call the original method as:
  //  mMyQueryInterface(aTargetUnknown, aRefIID, aVoidPtrPtr);
  operator Tfn () {
    ATLASSUME(NULL != mOriginalFn);
    return mOriginalFn;
  }

private:
  // address of the vtable entry being replaced
  Tfn * mTargetAddress;
  // original function
  Tfn mOriginalFn;

  FORBID_COPY_CONSTRUCTOR(VTablePatch);
};

/*============================================================================
 * class IUnknownPatch
 *  Enumerates the basic IUnknown methods.
 *  Acts as a base class for COM object patches.
 */
class IUnknownPatch
{
protected:
  //--------------------------------------------------------------------------
  // vtable indices for IUnknown
  enum TVtableIndex {
    VTI_QueryInterface = 0,
    VTI_AddRef,
    VTI_Release,
    VTI_NEXT__
  };
};






/*============================================================================
 * Macros for implementing urlmon patches
 */

// The DECLARE_VTABLE_PATCH??(type, MethodName) macros create:
//  - a type "VFTablePatch_MethodName": VTablePatch<function_pointer_type>
//  - an instance of VFTablePatch_MethodName: "do_MethodName"
//  - a patch function: "HRESULT _patch_MethodName(VTABLE aVtable)"
//  - a restore function: "HRESULT _restore_MethodName()"
//  - a static method "MethodName_Hook" of type function_pointer_type
#define DECLARE_VTABLE_PATCH0_(type, name) \
  typedef VTablePatch<type (STDMETHODCALLTYPE *)(IUnknown * aInstance)> VFTablePatch_##name; \
  VFTablePatch_##name do_##name; \
  HRESULT _patch_##name(VTABLE aVtable) { \
    return do_##name.patch(aVtable, VTI_##name, name##_Hook); \
  } \
  HRESULT _restore_##name() { \
    return do_##name.restore(); \
  } \
  static type STDMETHODCALLTYPE name##_Hook(IUnknown * aInstance)

#define DECLARE_VTABLE_PATCH_(type, name, ...) \
  typedef VTablePatch<type (STDMETHODCALLTYPE *)(IUnknown * aInstance, __VA_ARGS__)> name##_t; \
  name##_t do_##name; \
  HRESULT _patch_##name(VTABLE aVtable) { \
    return do_##name.patch(aVtable, VTI_##name, name##_Hook); \
  } \
  HRESULT _restore_##name() { \
    return do_##name.restore(); \
  } \
  static type STDMETHODCALLTYPE name##_Hook(IUnknown * aInstance, __VA_ARGS__)

#define DECLARE_VTABLE_PATCH0(name) DECLARE_VTABLE_PATCH0_(HRESULT, name)

#define DECLARE_VTABLE_PATCH(name, ...) DECLARE_VTABLE_PATCH_(HRESULT, name, __VA_ARGS__)

// BEGIN_PATCH_MAP(cls) macro delcares:
//  - a type "_PatchMapClass": of type cls
//  - a type "_patchFn": for a _patch_MethodName method
//  - a type "_restoreFn": for a _restore_MethodName method
//  - a struct _PatchMapEntry containining a _patchFn / _restoreFn pair
//  - a static method _getPatchMap that returns the patchmap
#define BEGIN_PATCH_MAP(cls) \
	typedef cls _PatchMapClass; \
  typedef HRESULT (_PatchMapClass::*_patchFn)(VTABLE); \
  typedef HRESULT (_PatchMapClass::*_restoreFn)(); \
  struct _PatchMapEntry { \
    _patchFn patch; \
    _restoreFn restore; \
  }; \
  static _PatchMapEntry * _getPatchMap() { \
    static _PatchMapEntry map[] = {

// PATCH_MAP_ENTRY(name) macro adds a _PatchMapEntry containining a
// _patchFn / _restoreFn pair to the patchmap.
#define PATCH_MAP_ENTRY(name) {&_patch_##name, &_restore_##name},

// END_PATCH_MAP() macro delcares:
//  - a terminating _PatchMapEntry containining NULL / NULL
//  - a method _patch(VTABLE aVtable)
//  - a method _restore()
#define END_PATCH_MAP() \
      {NULL, NULL} \
    }; \
    return map; \
  } \
\
  HRESULT _patch(VTABLE aVtable, int aId = 0) { \
    if (mVTable) { \
      return (mVTable == aVtable) ? S_FALSE : E_UNEXPECTED; \
    } \
    GlobalVtableMap map; \
    if (map.exists(aVtable)) { \
      return S_FALSE; \
    } \
    mVTable = aVtable; \
    for (_PatchMapEntry * entry = _getPatchMap(); entry->patch; entry++) { \
      HRESULT hr = ((*this).*entry->patch)(mVTable); \
      ATLASSERT(SUCCEEDED(hr)); \
      if (FAILED(hr)) { \
        return hr; \
      } \
    } \
    map[mVTable] = aId; \
    return S_OK; \
  } \
\
  HRESULT _restore() { \
    if (!mVTable) { \
      return S_FALSE; \
    } \
    for (_PatchMapEntry * entry = _getPatchMap(); entry->restore; entry++) { \
      ATLVERIFY(SUCCEEDED(((*this).*entry->restore)())); \
    } \
    GlobalVtableMap map; \
    map.erase(mVTable); \
    mVTable = NULL; \
    return S_OK; \
  }

#define IMPLEMENT_ADAPTER_CALL0(_name)  \
    if (!getInst().isEnabled()) \
        { return getInst().do_##_name(aInstance); } \
    CComPtr<IInternetProtocolEx> item; \
    { \
      GlobalProtocolAdapterMap map; \
      GlobalProtocolAdapterMap::iterator it = map.find(aInstance); \
      if (it != map.end()) { \
        item = it->second; \
      } \
    } \
    if (item) { \
      return item.p->_name(); \
    } \
    return getInst().do_##_name(aInstance); 

#define IMPLEMENT_ADAPTER_CALL(_name, ...)  \
    if (!getInst().isEnabled()) \
        { return getInst().do_##_name(aInstance, __VA_ARGS__); } \
    CComPtr<IInternetProtocolEx> item; \
    { \
      GlobalProtocolAdapterMap map; \
      GlobalProtocolAdapterMap::iterator it = map.find(aInstance); \
      if (it != map.end()) { \
        item = it->second; \
      } \
    } \
    if (item) { \
      return item.p->_name(__VA_ARGS__); \
    } \
    return getInst().do_##_name(aInstance, __VA_ARGS__);

// get VTABLE from IUnknown
#define UNK_VTABLE(_unk)    (*reinterpret_cast<VTABLE*>((IUnknown*)(_unk)))












} // namespace protocolpatchLib
