/****************************************************************************
 * Scheme.h : Scheme enum and traits.
 * Copyright 2013 Salsita Software (http://www.salsitasoft.com).
 * Author: Arne Seib <arne@salsitasoft.com>
 ****************************************************************************/
#pragma once

#include "ProtocolLibrary_i.h"

namespace protocolpatchLib
{

//--------------------------------------------------------------------------
// Scheme traits: Scheme enum and CLSID.
template <Scheme S, class TImpl> class SchemeTraitsBase
{
public:
  static Scheme getScheme()
      { return S; }
};

//--------------------------------------------------------------------------
// Default SchemeTraits - not compilable. Needs specialization.
template <Scheme S>
    class SchemeTraits :
    public SchemeTraitsBase<S, SchemeTraits<S> >
{
public:
  // not implemented
  static const IID & getCLSID();
  static LPCWSTR getSchemeName();
};

//--------------------------------------------------------------------------
// Specialization HTTP
template<>
    class SchemeTraits<SCHEME_HTTP> :
    public SchemeTraitsBase< SCHEME_HTTP, SchemeTraits<SCHEME_HTTP> >
{
public:
  static const IID & getCLSID()
      { return CLSID_HttpProtocol; }
  static LPCWSTR getSchemeName()
      { return L"http"; }
};
typedef SchemeTraits<SCHEME_HTTP> HTTP_Traits;

//--------------------------------------------------------------------------
// Specialization HTTPS
template<>
    class SchemeTraits<SCHEME_HTTP_S> :
    public SchemeTraitsBase< SCHEME_HTTP_S, SchemeTraits<SCHEME_HTTP_S> >
{
public:
  static const IID & getCLSID()
      { return CLSID_HttpSProtocol; }
  static LPCWSTR getSchemeName()
      { return L"https"; }
};
typedef SchemeTraits<SCHEME_HTTP_S> HTTP_S_Traits;

} // namespace protocolpatchLib
