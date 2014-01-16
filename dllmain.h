/****************************************************************************
 * dllmain.h : Declaration of module class.
 * Copyright 2013 Salsita Software (http://www.salsitasoft.com).
 * Author: Arne Seib <arne@salsitasoft.com>
 ****************************************************************************/

// We don't want this DLL to be registerable. Clients have to use
// registry-free loading. So our UpdateRegistryAppId() method
// does simply nothing.

#define DECLARE_REGISTRY_APPID_NORESOURCE(appid) \
  static LPCOLESTR GetAppId() throw() \
  { \
    return OLESTR(appid); \
  } \
  static TCHAR* GetAppIdT() throw() \
  { \
    return _T(appid); \
  } \
  static HRESULT WINAPI UpdateRegistryAppId(_In_ BOOL bRegister) throw() \
  { \
    return S_OK; \
  }

class ProtocolLibraryModule : public ATL::CAtlDllModuleT< ProtocolLibraryModule >
{
public :
  DECLARE_LIBID(LIBID_protocolpatchLib)
  DECLARE_REGISTRY_APPID_NORESOURCE("{4DE9C92C-E98F-41CB-B7CE-68B546DD53AC}")
};

extern class ProtocolLibraryModule _AtlModule;
