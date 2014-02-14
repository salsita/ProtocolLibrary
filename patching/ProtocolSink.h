/****************************************************************************
 * ProtocolSink.h : Declaration of ProtocolSink
 * Copyright 2013 Salsita Software (http://www.salsitasoft.com).
 * Author: Arne Seib <arne@salsitasoft.com>
 ****************************************************************************/
#pragma once

#include "papp/ProtocolImpl.h"
#include "RequestRecord.h"

namespace protocolpatchLib
{

/*============================================================================
 * Types
 */

/*============================================================================
 * class ProtocolSink
 * TODO: what is the purpose of this class?
 */
class ProtocolSink :
  public PassthroughAPP::CInternetProtocolSinkWithSP<ProtocolSink, CComObjectThreadModel>,
  public IHttpNegotiate
{
// ---------------------------------------------------------------------------
private:  // types
  typedef PassthroughAPP::CInternetProtocolSinkWithSP<ProtocolSink, CComObjectThreadModel> BaseClass;

// ---------------------------------------------------------------------------
public:  // types
  enum ESwitchStates {
    SWITCH_BASE = 50000,
    SWITCH_START,
    SWITCH_START_EX,
    SWITCH_REPORT_RESULT,
    SWITCH_MAX
  };

public: // members
  struct {
    CComPtr<IUri> pUri;
    CString sUri;
    CComPtr<IInternetProtocolSink> pOIProtSink;
    CComPtr<IInternetBindInfo> pOIBindInfo;
    DWORD grfPI;
    HANDLE_PTR dwReserved;
    CComPtr<IInternetProtocol> pTargetProtocol;
  } mStartParams;

  struct {
    HRESULT  hrResult;
    DWORD    dwError;
    CStringW szResult;
  } mReportResultParams;

// ---------------------------------------------------------------------------
public: // methods
  // -------------------------------------------------------------------------
  // Constructor
  ProtocolSink();

  HRESULT OnStart(LPCWSTR szUrl, IInternetProtocolSink *pOIProtSink,
                  IInternetBindInfo *pOIBindInfo, DWORD grfPI, HANDLE_PTR dwReserved,
                  IInternetProtocol* pTargetProtocol);

  HRESULT OnStartEx(IUri* pUri, IInternetProtocolSink *pOIProtSink,
                    IInternetBindInfo *pOIBindInfo,  DWORD grfPI, HANDLE_PTR dwReserved,
                    IInternetProtocolEx* pTargetProtocol);

  // -------------------------------------------------------------------------
  // COM interface map
  BEGIN_COM_MAP(ProtocolSink)
    COM_INTERFACE_ENTRY(IHttpNegotiate)
    COM_INTERFACE_ENTRY_CHAIN(BaseClass)
  END_COM_MAP()

  BEGIN_SERVICE_MAP(ProtocolSink)
    SERVICE_ENTRY(IID_IHttpNegotiate)
  END_SERVICE_MAP()

  // -------------------------------------------------------------------------
  // IHttpNegotiate
  STDMETHOD(BeginningTransaction)(
    /* [in] */  LPCWSTR   szURL,
    /* [in] */  LPCWSTR   szHeaders,
    /* [in] */  DWORD     dwReserved,
    /* [out] */ LPWSTR  * pszAdditionalHeaders);

  STDMETHOD(OnResponse)(
    /* [in] */  DWORD     dwResponseCode,
    /* [in] */  LPCWSTR   szResponseHeaders,
    /* [in] */  LPCWSTR   szRequestHeaders,
    /* [out] */ LPWSTR  * pszAdditionalRequestHeaders);

  STDMETHOD(ReportProgress)(
    /* [in] */ ULONG    ulStatusCode,
    /* [in] */ LPCWSTR  szStatusText);

  STDMETHOD(ReportData)(
    /* [in] */ DWORD  grfBSCF,
    /* [in] */ ULONG  ulProgress,
    /* [in] */ ULONG  ulProgressMax);

  STDMETHOD(ReportResult)(
    /* [in] */ HRESULT  hrResult,
    /* [in] */ DWORD    dwError,
    /* [in] */ LPCWSTR  szResult);

  // IInternetBindInfo
  STDMETHODIMP GetBindInfoEx(
    /* [out] */     DWORD     * grfBINDF,
    /* [in, out] */ BINDINFO  * pbindinfo,
    /* [out] */     DWORD     * grfBINDF2,
    /* [in] */      DWORD     * pdwReserved);

  // -------------------------------------------------------------------------

  HRESULT initRequest(IFrameRecord * aFrameRecord, IUri * aUri, DWORD aDocumentThreadId);

  // Returns the current bind verb.
  DWORD GetBindVerb() { return m_bindVerb; }

  HRESULT SwitchStart();
  HRESULT ContinueStart();

  HRESULT SwitchStartEx();
  HRESULT ContinueStartEx();

  HRESULT SwitchReportResult();
  HRESULT ContinueReportResult();

  RequestRecord mRequestRecord;

// ---------------------------------------------------------------------------
private:  // types
  /*==========================================================================
   * class DocumentSink
   * Listens to the readystatechange event of HTMLDocumentEvents2 and notifies
   */
  class DocumentSink :
    public CComObjectRootEx<CComSingleThreadModel>,
    public IUnknown,
    public IDispEventImpl<1, DocumentSink, &DIID_HTMLDocumentEvents2, &LIBID_MSHTML, 4, 0>
  {
  // -------------------------------------------------------------------------
  public: // types
    friend CComObject<DocumentSink>;

  // -------------------------------------------------------------------------
  public: // methods and functions
    // Static function to notify aBrowserEvents that a frame is complete.
    // If the associated document is already in "complete" state, call
    // DAnchoBrowserEvents::OnEndFrame directly. Otherwise create an
    // instance and attach to the document's ready state change event.
    static HRESULT prepareDocumentNotification(RequestRecord & aRecord, IHTMLDocument2 * aHTMLDoc);

    // DTOR
    ~DocumentSink();

    BEGIN_COM_MAP(DocumentSink)
      COM_INTERFACE_ENTRY(IUnknown)
    END_COM_MAP()

    // -------------------------------------------------------------------------
    // Event map
    BEGIN_SINK_MAP(DocumentSink)
      SINK_ENTRY_EX(
          1,
          DIID_HTMLDocumentEvents2,
          DISPID_READYSTATECHANGE,
          OnReadyStateChange)
    END_SINK_MAP()

    // HTMLDocumentEvents2
    STDMETHOD_(void, OnReadyStateChange)(IHTMLEventObj* ev);

  // -------------------------------------------------------------------------
  private:  // methods
    // CTOR
    DocumentSink();

    HRESULT init(RequestRecord & aRecord, IHTMLDocument2 * aHTMLDoc);

  // -------------------------------------------------------------------------
  private:  // members
    RequestRecord           mRecord;
    CComPtr<IHTMLDocument2> mHTMLDocument;
  };

// ---------------------------------------------------------------------------
private:  // methods
  HRESULT beforeRequest(BOOL & aRedirected);

// ---------------------------------------------------------------------------
private:  // members
  static PROTOCOLDATA   sProtocolData[3];
  static PROTOCOLDATA * sProtocolDataStart;
  static PROTOCOLDATA * sProtocolDataStartEx;
  static PROTOCOLDATA * sProtocolDataReportResult;
  DWORD m_bindVerb;
  DWORD mDocumentThreadId;

//CComQIPtr<ILogger2> mLogger;

};

} // namespace protocolpatchLib
