/****************************************************************************
 * IDispatchExImpl.h : Template IDispatchExImpl
 * Copyright 2013 Salsita (http://www.salsitasoft.com).
 * Author: Arne Seib <arne@salsitasoft.com>
 ****************************************************************************/
#pragma once

#include <string>
#include <map>
#include <unordered_map>

namespace protocolpatchLib
{

//--------------------------------------------------------------------------
// types
typedef std::unordered_map<std::wstring, DISPID> MapName2DispId;
typedef std::unordered_map<DISPID, CComVariant> MapDispId2Variant;
typedef std::map<DISPID, std::wstring> MapDispId2StringOrdered;

/*============================================================================
 * class IDispatchExImpl
 *  A simple dictionary object, expandable, threadsafe
 */
template <class TImpl, const GUID* plibid = &CAtlModule::m_libid, WORD wMajor = 1,
          WORD wMinor = 0, class tihclass = CComTypeInfoHolder>
  class IDispatchExImpl :
      public MembersLockable,
      public IDispatchImpl<IDispatchEx, &IID_IDispatchEx, plibid,
                      wMajor, wMinor>
{
public:
  IDispatchExImpl() : mNextDispID(1), mModified(FALSE)
      {}


  //----------------------------------------------------------------------------
  // GetDispID
  STDMETHOD(GetDispID)(
    BSTR      bstrName,
    DWORD     grfdex,
    DISPID  * pid)
  {
    LOCKED_BLOCK();
    return static_cast<TImpl*>(this)->getDispID(bstrName, pid, (fdexNameEnsure & grfdex));
  }

  //----------------------------------------------------------------------------
  // InvokeEx
  STDMETHOD(InvokeEx)(
    DISPID              id,
    LCID                lcid,
    WORD                wFlags,
    DISPPARAMS        * pdp,
    VARIANT           * pvarRes,
    EXCEPINFO         * pei,
    IServiceProvider  * pspCaller)
  {
    LOCKED_BLOCK();
    if (wFlags & DISPATCH_METHOD && !(wFlags & DISPATCH_PROPERTYGET)) {
      return static_cast<TImpl*>(this)->callMethod(id, pdp, pvarRes);
    }

    MapDispId2StringOrdered::iterator it = mNames.find(id);
    if (it == mNames.end()) {
      return DISP_E_MEMBERNOTFOUND;
    }

    // set value
    if (wFlags & DISPATCH_PROPERTYPUT)
    {
      // need the property to be set
      if (!pdp || (pdp->cArgs < 1)) {
        return DISP_E_BADPARAMCOUNT;
      }
      return static_cast<TImpl*>(this)->putValue(id, pdp->rgvarg + pdp->cArgs - 1);
    }
    if (!pvarRes) {
      return E_POINTER;
    }
    return static_cast<TImpl*>(this)->getValue(id, pvarRes);
  }

  //----------------------------------------------------------------------------
  // DeleteMemberByName
  STDMETHOD(DeleteMemberByName)(
    BSTR  bstrName,
    DWORD grfdex)
  {
    LOCKED_BLOCK();
    return static_cast<TImpl*>(this)->remove(bstrName);
  }

  //----------------------------------------------------------------------------
  // DeleteMemberByDispID
  STDMETHOD(DeleteMemberByDispID)(
    DISPID id)
  {
    LOCKED_BLOCK();
    return static_cast<TImpl*>(this)->remove(id);
  }

  //----------------------------------------------------------------------------
  // GetMemberProperties
  STDMETHOD(GetMemberProperties)(
    DISPID  id,
    DWORD   grfdexFetch,
    DWORD * pgrfdex)
  {
    return E_NOTIMPL;
  }

  //----------------------------------------------------------------------------
  // GetMemberName
  STDMETHOD(GetMemberName)(
    DISPID  id,
    BSTR  * pbstrName)
  {
    LOCKED_BLOCK();
    return static_cast<TImpl*>(this)->getName(id, pbstrName);
  }

  //----------------------------------------------------------------------------
  // GetNextDispID
  STDMETHOD(GetNextDispID)(
    DWORD     grfdex,
    DISPID    id,
    DISPID  * pid)
  {
    LOCKED_BLOCK();
    if (!pid) {
      return E_POINTER;
    }

    MapDispId2StringOrdered::iterator it;
    if (id == DISPID_STARTENUM) {
      // get first entry
      it = mNames.begin();
    }
    else {
      // get current entry
      it = mNames.find(id);
      if (it != mNames.end()) {
        // get next entry
        ++it;
      }
    }

    // as long as we have a valid pos...
    while(it != mNames.end())
    {
      // ...get the entry at pos...
      MapDispId2Variant::iterator itName = mValues.find(it->first);
      if (itName != mValues.end()) {
        // gotcha
        (*pid) = itName->first;
        return S_OK;
      }
      ++it;
    }
    // sorry, no more data
    return S_FALSE;
  }

  //----------------------------------------------------------------------------
  // GetNameSpaceParent
  STDMETHOD(GetNameSpaceParent)(
    IUnknown  **  ppunk)
  {
    return E_NOTIMPL;
  }

protected:
  //--------------------------------------------------------------------------
  // protected methods
  HRESULT callMethod(
    DISPID        aId,
    DISPPARAMS  * aParams,
    VARIANT     * aRetVal)
  {
    // by default have no methods, only values
    return DISP_E_MEMBERNOTFOUND;
  }

  HRESULT putValue(
    DISPID    aId,
    VARIANT * aProperty)
  {
    if (aProperty != NULL) {
      mValues[aId].Copy(aProperty);
      mModified = TRUE;
    }
    return S_OK;
  }

  HRESULT getValue(
    DISPID    aId,
    VARIANT * aRetVal)
  {
    if (!aRetVal) {
      return E_POINTER;
    }
    MapDispId2Variant::iterator it = mValues.find(aId);
    if (it == mValues.end()) {
      return DISP_E_MEMBERNOTFOUND;
    }
    return ::VariantCopy(aRetVal, &it->second);
  }

  HRESULT getName(
    DISPID    aId,
    BSTR    * aRetVal)
  {
    if (!aRetVal) {
      return E_POINTER;
    }
    MapDispId2StringOrdered::iterator it = mNames.find(aId);
    if (it == mNames.end()) {
      return DISP_E_MEMBERNOTFOUND;
    }
    (*aRetVal) = ::SysAllocString(it->second.c_str());
    return S_OK;
  }

  HRESULT getDispID(
    BSTR      aName,
    DISPID  * aRetVal,
    BOOL      aCreate = FALSE)
  {
    if (!aRetVal) {
      return E_POINTER;
    }
    (*aRetVal) = DISPID_UNKNOWN;

    MapName2DispId::iterator it = mNameIDs.find(aName);
    if (it != mNameIDs.end()) {
      MapDispId2Variant::iterator it2 = mValues.find(it->second);
      if (it2 == mValues.end() && !aCreate) {
        return DISP_E_UNKNOWNNAME;
      }
      (*aRetVal) = it->second;
      return S_OK;
    }
    if (!aCreate) {
      return DISP_E_UNKNOWNNAME;
    }

    mNameIDs[aName] = mNextDispID;
    mNames[mNextDispID] = aName;
    (*aRetVal) = mNextDispID;
    ++mNextDispID;

    return S_OK;
  }

  HRESULT remove(DISPID aId)
  {
    MapDispId2Variant::iterator it = mValues.find(aId);
    if (it == mValues.end()) {
      return DISP_E_UNKNOWNNAME;
    }
    // Note: We keep the entry in mNames and of course in
    // mNameIDs. IDispatchEx requires, that the name/DISPID
    // combination stays valid for the whole lifetime of the object.
    mValues.erase(it);
    return S_OK;
  }

  HRESULT remove(LPCWSTR aName)
  {
    MapName2DispId::iterator it = mNameIDs.find(aName);
    if (it == mNameIDs.end()) {
      return DISP_E_UNKNOWNNAME;
    }
    return static_cast<TImpl*>(this)->remove(it->second);
  }

protected:
  //--------------------------------------------------------------------------
  // protected attributes
  MapDispId2StringOrdered
                    mNames;   // key: DISPID, value: header name
                                    // NOTE: This is the map we use for
                                    // iteration, so it is ordered.
  MapDispId2Variant mValues;  // key: DISPID, value: header value
  MapName2DispId    mNameIDs; // key: header name, value: DISPID

  DISPID mNextDispID;
  BOOL   mModified;
};



} // namespace protocolpatchLib
