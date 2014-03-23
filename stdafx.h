// stdafx.h : include file for standard system include files,
// or project specific include files that are used frequently,
// but are changed infrequently

#pragma once

#ifndef STRICT
#define STRICT
#endif

#include "targetver.h"

#define _ATL_SINGLE_THREADED
#define _ATL_NO_AUTOMATIC_NAMESPACE
#define _ATL_CSTRING_EXPLICIT_CONSTRUCTORS  // some CString constructors will be explicit
#define ATL_NO_ASSERT_ON_DESTROY_NONEXISTENT_WINDOW

#include "resource.h"
#include <atlbase.h>
#include <atlstr.h>
#include <atlcom.h>
#include <atlctl.h>
#include <wininet.h>
#include <Exdisp.h>
#include <Exdispid.h>
#include <SHLGUID.h>

using namespace ATL;

#include "Buffer.h"

#ifndef IF_FAILED_RET
#define IF_FAILED_RET(_hr) \
  do \
  { \
    HRESULT _hr__ = _hr; \
    if (FAILED(_hr__)) \
    { \
      return _hr__; \
    } \
  } \
  while(0)
#endif

#define DECLARE_GETTER(_type, _name) \
  __declspec(property(get = get_##_name)) _type _name; \
  _type get_##_name()

#ifndef FORBID_COPY_CONSTRUCTOR
#define FORBID_COPY_CONSTRUCTOR(_cls) \
  private: \
    _cls(const _cls &); \
    _cls & operator = (const _cls &);
#endif // ndef FORBID_COPY_CONSTRUCTOR

class CritSectLock
{
public:
  CritSectLock(CRITICAL_SECTION & crit) : m_crit(crit)
  {
    EnterCriticalSection(&m_crit);
  }

  ~CritSectLock()
  {
    LeaveCriticalSection(&m_crit);
  }
private:
  CRITICAL_SECTION & m_crit;
};

namespace protocolpatchLib
{
class MembersLockable
{
public:
  MembersLockable()
  {
    ::InitializeCriticalSection(&mCriticalSection);
  }

  virtual ~MembersLockable()
  {
    ::DeleteCriticalSection(&mCriticalSection);
  }

protected:
  class MembersLock
  {
  public:
    MembersLock(CRITICAL_SECTION & aCriticalSection) :
        mCriticalSection(aCriticalSection)
    {
      ::EnterCriticalSection(&mCriticalSection);
    }

    ~MembersLock()
    {
      ::LeaveCriticalSection(&mCriticalSection);
    }
  private:
    CRITICAL_SECTION & mCriticalSection;
  };

protected:
  CRITICAL_SECTION mCriticalSection;
};

#define LOCKED_BLOCK() MembersLock lock(mCriticalSection)

} // namespace protocolpatchLib

/*
#import "U:\\Users\\Hans\\Documents\\Visual Studio 2010\\Projects\\LogServer\\LogServer\\Debug_Win32\\LogServer.tlb" named_guids raw_interfaces_only raw_native_types no_smart_pointers exclude("tagSAFEARRAYBOUND")
using namespace LogServerLib;
*/

class ApartmentInfo
{
private:
  BOOL mIsValid;
  APTTYPE mApartmentType;
  GUID mThreadGUID;
  THDTYPE mThreadType;

public:
  ApartmentInfo() :
    mIsValid(FALSE),
    mApartmentType(APTTYPE_CURRENT),
    mThreadGUID(GUID_NULL),
    mThreadType(THDTYPE_BLOCKMESSAGES)
  {
    CComPtr<IComThreadingInfo> info;
    CoGetObjectContext(__uuidof(IComThreadingInfo), (void**)&info.p);
    if (info) {
      info->GetCurrentApartmentType(&mApartmentType);
      info->GetCurrentLogicalThreadId(&mThreadGUID);
      info->GetCurrentThreadType(&mThreadType);
      mIsValid = TRUE;
    }
  }

  DECLARE_GETTER(BOOL, valid) { return mIsValid; }
  DECLARE_GETTER(APTTYPE, apartmentType) { return mApartmentType; }
  DECLARE_GETTER(GUID, threadGUID) { return mThreadGUID; }
  DECLARE_GETTER(THDTYPE, threadType) { return mThreadType; }

  BOOL operator == (const ApartmentInfo & other)
  {
    return (other.mThreadGUID == mThreadGUID);
  }

  BOOL operator != (const ApartmentInfo & other)
  {
    return (other.mThreadGUID != mThreadGUID);
  }

  CString toString()
  {
    CString s;
    OLECHAR* sGUID;
    StringFromCLSID(mThreadGUID, &sGUID);
    s.Format(_T("GUID: %s Type: "), sGUID);
    ::CoTaskMemFree(sGUID);
    switch(mApartmentType) {
      case APTTYPE_CURRENT:
        s += _T("CURRENT");break;
      case APTTYPE_STA:
        s += _T("STA");break;
      case APTTYPE_MTA:
        s += _T("MTA");break;
      case APTTYPE_NA:
        s += _T("NA");break;
      case APTTYPE_MAINSTA:
        s += _T("MAINSTA");break;
    }
    s += _T(" ThreadType: ");
    switch(mThreadType) {
      case THDTYPE_BLOCKMESSAGES:
        s += _T("BLOCKMESSAGES");break;
      case THDTYPE_PROCESSMESSAGES:
        s += _T("PROCESSMESSAGES");break;
    }
    return s;
  }

};

