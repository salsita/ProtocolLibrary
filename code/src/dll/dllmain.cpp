/****************************************************************************
 * dllmain.cpp : Implementation of DllMain.
 * Copyright 2013 Salsita Software (http://www.salsitasoft.com).
 * Author: Arne Seib <arne@salsitasoft.com>
 ****************************************************************************/

#include "stdafx.h"
#include "resource.h"
#include "ProtocolLibrary_i.h"
#include "patching/Application.h"
#include "dllmain.h"

ProtocolLibraryModule _AtlModule;

// DLL Entry Point
extern "C" BOOL WINAPI DllMain(HINSTANCE hInstance, DWORD dwReason, LPVOID lpReserved)
{
  hInstance;
  return _AtlModule.DllMain(dwReason, lpReserved);
}
