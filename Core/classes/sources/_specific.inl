
/*

  Copyright (c) 2005-2009, Alexey Sudachen, alexey@sudachen.name

*/


#if !defined ___BE1F879C_7BF4_4220_B0CA_C5385185DE0C___
#define ___BE1F879C_7BF4_4220_B0CA_C5385185DE0C___

#include "_specific.h"

#if defined _TEGGO_SPECIFIC_HERE
# define _TEGGO_SPECIFIC_FAKE_INLINE _TEGGO_EXPORTABLE
#else
# define _TEGGO_SPECIFIC_FAKE_INLINE CXX_FAKE_INLINE
#endif

_TEGGO_SPECIFIC_FAKE_INLINE
pchar_t CXX_STDCALL TeggoErrorText(TEGGOt_E e)
  {
    switch ( e )
      {
        case TEGGO_OK:        return "succeeded";
        case TEGGO_FAIL:      return "failed with unknown reason";
        case TEGGO_NOFILE:    return "no file";
        case TEGGO_BADARG:    return "bad argument provided";
        case TEGGO_ASSERT:    return "expression asserted";
        case TEGGO_NOACCESS:  return "access violation";
        case TEGGO_NODATA:    return "there is no data";
        case TEGGO_IOFAIL:    return "input/output failed";
        case TEGGO_UNSUPPORTED: return "routine unsupported";
        default:
          return "unknown error";
      }
  }

#if !defined _TEGGOINLINE
  template<>
  _TEGGO_EXPORTABLE_DATA wchar_t Teggo_LastError_<int>::reason[128] = {0};
  template<>
  _TEGGO_EXPORTABLE_DATA u32_t Teggo_LastError_<int>::value = 0;
#endif

namespace teggo {

  _TEGGO_SPECIFIC_FAKE_INLINE
  wchar_t CXX_STDCALL ucs2_btowc(char c)
    {
#if SYSTEM_IS_WINDOWS      
      wchar_t wc[2] = {0};
      //mbtowc(&wc,&c,1);
      MultiByteToWideChar(CP_ACP,0,&c,1,&wc[0],1);
      return *wc;
#elif SYSTEM_IS_MACOSX
      return c;
#else
      return c;
#endif
    }

  _TEGGO_SPECIFIC_FAKE_INLINE
  char CXX_STDCALL ucs2_wctob(wchar_t wc)
    {
#if SYSTEM_IS_WINDOWS      
      char c[16] = { 0 };
      //wctomb(c,wc);
      WideCharToMultiByte(CP_ACP,0,&wc,1,&c[0],1,0,0);
      return c[0];
#elif SYSTEM_IS_MACOSX
      return (char)wc;
#else
      return (char)wc;
#endif
    }

  _TEGGO_SPECIFIC_FAKE_INLINE
  void *DefaultAllocator::Allocate( u32_t sz )
    {
      void *p = _MALLOC((size_t)sz); //operator new ((size_t)sz,nothrow);
      VIOLATION( p == 0 , L"OutOfMemory" );
      return p;
    }

  _TEGGO_SPECIFIC_FAKE_INLINE
  void DefaultAllocator::Deallocate( void* p )
    {
      //operator delete ( p );
      if ( p ) _FREE(p);
    }

}

#undef _TEGGO_SPECIFIC_FAKE_INLINE

#endif // ___BE1F879C_7BF4_4220_B0CA_C5385185DE0C___
