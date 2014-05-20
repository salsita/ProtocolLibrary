#pragma once

/*****************************************************************************
 * Interface IProtocolClassFactory
 *  Protocol classfactories have to implement this interface.
 *****************************************************************************/
MIDL_INTERFACE("72FF577B-A2DD-41C8-9FB0-F2977328BC32")
IProtocolClassFactory :
  public IClassFactory
{
public:
  STDMETHOD(Init)(LPCWSTR aScheme) PURE;
  STDMETHOD(AddHost)(LPCWSTR aHostname, VARIANT vtValue) PURE;
  STDMETHOD_(size_t, RemoveHost)(LPCWSTR aHostname) PURE;
};

/*****************************************************************************
 * Interface IProtocolMemoryResource
 *  Adds support for special URLs - URLs that don't exist pysically.
 *  The data for these resources comes directly from a memory buffer.
 *****************************************************************************/
MIDL_INTERFACE("D3C4D0CB-DEC9-4529-8DDD-8F441E76D584")
IProtocolMemoryResource : public IUnknown
{
public:
    STDMETHOD(AddResource)(
        IUri * aURI,
        LPCVOID lpData,
        DWORD dwLength,
        LPCWSTR lpszMimeType) PURE;

    STDMETHOD(GetResource)(
        IUri * aUri,
        URLMemoryResource & aRetBuffer) PURE;

};

