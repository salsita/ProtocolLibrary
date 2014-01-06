/****************************************************************************
 * Protocol.h : Declaration of Protocol
 * Copyright 2013 Salsita (http://www.salsitasoft.com).
 * Author: Arne Seib <arne@salsitasoft.com>
 ****************************************************************************/
#pragma once

#include "papp/ProtocolImpl.h"
#include "ThreadRecord.h"
#include "ProtocolSink.h"

namespace protocolpatchLib
{

/*============================================================================
 * Types
 */

class Protocol;

/*============================================================================
 * class ProtocolStartPolicy
 * This class is responsible for initializing the Protocol, the ProtoclSink
 * and for starting the actual request on the target protocol.
 *
 * The programflow is as follows (and accordingly for Start()):
 * 1) urlmon calls Protocol::StartEx().
 * 2) the protocol baseclass calls StartPolicy::OnStartEx()
 * 3) the StartPolicy initializes the sink
 * 4) and calls SwitchStartEx() on the sink
 * 5) Continue() is called on the protocol class.
 * 6) the protocol initializes itself
 * 7) ContinueStartEx() on the sink is called
 * 8) the sink calls StartEx() on the target protocol
 */
class ProtocolStartPolicy :
  public PassthroughAPP::CustomSinkStartPolicy<Protocol, ProtocolSink>
{
// ---------------------------------------------------------------------------
public: // methods
  HRESULT OnStart(
      LPCWSTR                 szUrl,
      IInternetProtocolSink * pOIProtSink,
      IInternetBindInfo     * pOIBindInfo,
      DWORD                   grfPI,
      HANDLE_PTR              dwReserved,
      IInternetProtocol     * pTargetProtocol);

  HRESULT OnStartEx(
      IUri                  * pUri,
      IInternetProtocolSink * pOIProtSink,
      IInternetBindInfo     * pOIBindInfo,
      DWORD                   grfPI,
      HANDLE_PTR              dwReserved,
      IInternetProtocolEx   * pTargetProtocol);
};



/*============================================================================
 * class Protocol
 * TODO: what is the purpose of this class?
 */
class Protocol :
  public PassthroughAPP::CInternetProtocol<ProtocolStartPolicy, CComObjectThreadModel>
{
public: // types
  typedef CComObject<Protocol> _ComObject;

public: // static
  //--------------------------------------------------------------------------
  // creator
  static CComPtr<IInternetProtocolEx>
    createInstance(
        IUnknown * aUnkProtocol,
        IInternetProtocolEx * aPassthru,
        HRESULT * aHresult = NULL);

// ---------------------------------------------------------------------------
public: // methods
  Protocol()
  {
    int asd = 0;
  }

  // -------------------------------------------------------------------------
  // IInternetProtocolRoot
  STDMETHOD(Continue)(PROTOCOLDATA *pProtocolData);

  // Initializes members from the protocol sink.
  HRESULT initFromSink(ProtocolSink * aProtocolSink);
  // Clear members, release objects.
  void reset();
  // Fire OnFrameStart event
  HRESULT fireOnFrameStart(CComBSTR & aCurrentURL);
  // Fire OnFrameRedirect event
  HRESULT fireOnFrameRedirect(CComBSTR & aCurrentURL, CComBSTR & aAdditionalData);
  // Fire OnBeforeHeaders event
  HRESULT fireOnBeforeHeaders();
  // Fire OnFrameEnd event
  HRESULT fireOnFrameEnd(CComBSTR aUrl);

// ---------------------------------------------------------------------------
private:  // types
  typedef std::vector<std::pair<CString, CString> > RedirectList;

  //==========================================================================

private:  // methods
  //--------------------------------------------------------------------------
  // initRequest method: called from Start or StartEx, initializes a new
  // request.
  HRESULT initRequest(IUri *pUri,
      IInternetProtocolSink *pOIProtSink,
      IInternetBindInfo *pOIBindInfo);

// ---------------------------------------------------------------------------
private:  // members
  CComPtr<IFrameRecord> mFrameRecord;
};


} // namespace protocolpatchLib
