/****************************************************************************
 * Protocol.cpp : Implementation of Protocol
 * Copyright 2013 Salsita Software (http://www.salsitasoft.com).
 * Author: Arne Seib <arne@salsitasoft.com>
 ****************************************************************************/

#include "stdafx.h"
#include "Protocol.h"

//#define PAPPTRACE
#define PAPPTRACE(...)   ATLTRACE(__FUNCTION__);ATLTRACE(_T(": "));ATLTRACE(__VA_ARGS__)

namespace protocolpatchLib
{

HRESULT ProtocolStartPolicy::OnStart(
  LPCWSTR szUrl, IInternetProtocolSink *pOIProtSink,
  IInternetBindInfo *pOIBindInfo,  DWORD grfPI, HANDLE_PTR dwReserved,
  IInternetProtocol* pTargetProtocol)
{
  // Initialize the sink. This has to happen first to be able to Switch().
  Protocol * protocol = static_cast<Protocol*>(this);
  ProtocolSink* sink = GetSink(protocol);
  HRESULT hr = sink->OnStart(szUrl, pOIProtSink, pOIBindInfo, grfPI, dwReserved, pTargetProtocol);
  if (FAILED(hr)) {
    return hr;
  }

  // And Switch() for Start() to the main thread.
  return sink->SwitchStart();
}

//----------------------------------------------------------------------------
//  OnStartEx
HRESULT ProtocolStartPolicy::OnStartEx(
  IUri* pUri, IInternetProtocolSink *pOIProtSink,
  IInternetBindInfo *pOIBindInfo,  DWORD grfPI, HANDLE_PTR dwReserved,
  IInternetProtocolEx* pTargetProtocol)
{
  // Initialize the sink. This has to happen first to be able to Switch().
  Protocol * protocol = static_cast<Protocol*>(this);
  ProtocolSink* sink = GetSink(protocol);
  HRESULT hr = sink->OnStartEx(pUri, pOIProtSink, pOIBindInfo, grfPI, dwReserved, pTargetProtocol);
  if (FAILED(hr)) {
    return hr;
  }

  // And Switch() for StartEx() to the main thread.
  return sink->SwitchStartEx();
}

/*============================================================================
 * class Protocol
 */

//----------------------------------------------------------------------------
//  Continue
STDMETHODIMP Protocol::Continue(PROTOCOLDATA* pProtocolData)
{
  // If this is not an ancho related state...
  if (pProtocolData->dwState < ProtocolSink::SWITCH_BASE || pProtocolData->dwState >= ProtocolSink::SWITCH_MAX) {
    // ... just call super
    return __super::Continue(pProtocolData);
  }

  ProtocolSink * sink = GetSink();

  // for safe releasing the AddRef from SwitchXXX() calls
  CComPtr<IInternetProtocolSink> guard;
  guard.Attach(sink);

  HRESULT hrRet = S_OK;
  switch(pProtocolData->dwState) {

    case ProtocolSink::SWITCH_START: {
      hrRet = initRequest(sink->mStartParams.pUri, sink->mStartParams.pOIProtSink, sink->mStartParams.pOIBindInfo);
      if (SUCCEEDED(hrRet)) {
        // call Start on native protocol
        return sink->ContinueStart();
      }
    } break;

    case ProtocolSink::SWITCH_START_EX: {
      hrRet = initRequest(sink->mStartParams.pUri, sink->mStartParams.pOIProtSink, sink->mStartParams.pOIBindInfo);
      if (SUCCEEDED(hrRet)) {
        // call StartEx on native protocol
        return sink->ContinueStartEx();
      }
    } break;

    case ProtocolSink::SWITCH_REPORT_RESULT: {
      return sink->ContinueReportResult();
    } break;
  }

  return hrRet;
}

//--------------------------------------------------------------------------
// initRequest method: called from Start or StartEx, initializes a new
// request.
HRESULT Protocol::initRequest(IUri *pUri,
    IInternetProtocolSink *pOIProtSink,
    IInternetBindInfo *pOIBindInfo)
{
  ATLASSERT(pUri);
  ATLASSERT(pOIProtSink);
  ATLASSERT(pOIBindInfo);

  mFrameRecord.Release();

  // The logic here is as follows:
  // - if we have no ThreadRecord for this thread this is for sure
  //   a resource request
  // - if we HAVE a ThreadRecord:
  //   - get the FrameRecord by the current URL
  //     This will return whether the top level request or the latest
  //     request on this thread.
  //     - if we have no FrameRecord this is a resource request
  //     - if we HAVE a FrameRecord this is a document request
  // So in the end having a FrameRecord means this is a document request.

  CComPtr<IThreadRecord> threadRecord = ThreadRecord::get();
  if (threadRecord) {
    // now get the FrameRecord
    CComPtr<IFrameRecord> frameRecord;
    threadRecord->getForUri(pUri, &frameRecord.p);
    if (!frameRecord) {
      // try current - should be a resource request
      threadRecord->getCurrent(&frameRecord.p);
    }
    if (frameRecord) {
      mFrameRecord = frameRecord;
      // NOTE: In case of a refresh for a subframe we might have the
      // wrong browser. I didn't find a way to solve this problem.
    }
  }

  return GetSink()->initRequest(mFrameRecord, pUri, (threadRecord != NULL));
}

} // namespace protocolpatchLib
