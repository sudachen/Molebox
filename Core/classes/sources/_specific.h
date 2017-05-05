
/*

  Copyright (c) 2005-2009, Alexey Sudachen, alexey@sudachen.name

*/


#if !defined __8903A158_94C4_4670_84F8_EEE59F0625DE__
#define __8903A158_94C4_4670_84F8_EEE59F0625DE__

#define TEGGO_CLASSES_LIBRARY
#include "detect_compiler.h"

#if defined _WIN32 || defined WIN32
#define SYSTEM_IS_WINDOWS 1
#else
#define SYSTEM_IS_WINDOWS 0 
#endif

#if defined _MACOSX
#define SYSTEM_IS_MACOSX 1
#else
#define SYSTEM_IS_MACOSX 0
#endif

#if CXX_COMPILER_IS_MSVC_COMPATIBLE
#pragma warning(disable:4786)
#endif

#if CXX_COMPILER_IS_MSVC_BUNNY
# undef _CRT_NONSTDC_NO_DEPRECATE
# define _CRT_NONSTDC_NO_DEPRECATE 1
# undef _CRT_SECURE_NO_DEPRECATE
# define _CRT_SECURE_NO_DEPRECATE 1
#endif

#if !defined _TEGGOINLINE
#  if defined _TEGGODYNAMIC_EXPORT
#    define _TEGGO_EXPORTABLE __declspec(dllexport)
#    define _TEGGO_EXPORTABLE_DATA __declspec(dllexport)
#  else
#    if !defined _TEGGOSTATIC
#      define _TEGGODYNAMIC_IMPORT
#    endif
#    define _TEGGO_EXPORTABLE
#    if defined _TEGGODYNAMIC_IMPORT
#      define _TEGGO_EXPORTABLE_DATA __declspec(dllimport)
#    else
#      define _TEGGO_EXPORTABLE_DATA
#    endif
#  endif
#else
# define _TEGGO_EXPORTABLE
# define _TEGGO_EXPORTABLE_DATA
#endif

#if !SYSTEM_IS_WINDOWS
#define __declspec(x)
#define __stdcall
#define __cdecl
#define stdcall
#define cdecl
#endif

#define TEGGO_BIT(x) CXX_BIT(x)
#define TEGGO_CALL   CXX_STDCALL
#define TEGGO_LABEL_(a) _teggo_label_L##a
#define TEGGO_LABEL(a) TEGGO_LABEL_(a)

#if !(CXX_COMPILER_IS_GXX)
#include <malloc.h>
#endif

#include <string.h>
#include <stdio.h>
#include <stdlib.h>
#include <ctype.h>
#include <wchar.h>
#include <wctype.h>
#include <math.h>
#include <stdarg.h>
#include <limits.h>

enum TEGGOt_E
  {
    TEGGO_OK            = 0,
    TEGGO_FAIL          = 1,
    TEGGO_NOFILE        = 2,
    TEGGO_BADARG        = 3,
    TEGGO_ASSERT        = 4,
    TEGGO_NOACCESS      = 5,
    TEGGO_NODATA        = 6,
    TEGGO_IOFAIL        = 7,
    TEGGO_CLOSED        = 8,
    TEGGO_CORRUPTED     = 9,
    TEGGO_UNSUPPORTED   = 10,
    TEGGO_OVERFLOW      = 11,
  };

_TEGGO_EXPORTABLE pchar_t CXX_STDCALL TeggoErrorText(TEGGOt_E);

namespace teggo
  {
    template<class tTc> struct FormatT;
    struct StringParam_;
    void CXX_STDCALL StringParamCopy(StringParam_ const& _,wchar_t * b,int l); // stuppid paranoic GCC
  }

template<class tFake> struct Teggo_LastError_
  {
    _TEGGO_EXPORTABLE_DATA static wchar_t reason[128];
    _TEGGO_EXPORTABLE_DATA static u32_t value;
    template < class tTe >
      static tTe CXX_STDCALL Success(tTe e)
        {
          return value = TEGGO_OK, reason[0] = 0, e;
        }
    static TEGGOt_E CXX_STDCALL Success()
      {
        return value = TEGGO_OK, reason[0] = 0, TEGGO_OK;
      }
    void Clear() { Success(); }
    template < class tTe >
      static tTe CXX_STDCALL Fail(tTe e,teggo::StringParam_ const &error_string)
        {
          return Fail(error_string), e;
        }
    _TEGGO_EXPORTABLE static TEGGOt_E CXX_STDCALL Fail(teggo::StringParam_ const &error_string)
      {
        value = TEGGO_FAIL;
        return teggo::StringParamCopy(error_string,reason,127), TEGGO_FAIL;
      }
    _TEGGO_EXPORTABLE static TEGGOt_E CXX_STDCALL Fail(TEGGOt_E e,teggo::StringParam_ const &error_string)
      {
        value = e;
        return teggo::StringParamCopy(error_string,reason,127), e;
      }
    _TEGGO_EXPORTABLE static TEGGOt_E CXX_STDCALL TEGGOt() { return (TEGGOt_E)value; }
    _TEGGO_EXPORTABLE static pwide_t  CXX_STDCALL Reason() { return reason; }
    Teggo_LastError_ *operator ->() const { return 0; }    
  };

#if defined _TEGGOINLINE
  template <class tFake> wchar_t Teggo_LastError_<tFake>::reason[128] = {0};
  template <class tFake> u32_t Teggo_LastError_<tFake>::value = 0;
#endif

static const Teggo_LastError_<int> _E = Teggo_LastError_<int>();

namespace teggo
{
  struct Uncopyable
    {
      Uncopyable() {}
    private:
      Uncopyable(Uncopyable const&);
      Uncopyable& operator=(Uncopyable const&);
    };

  struct EmptyStruct
    {
    };
}

//#undef min
//#undef max

template <class tTx> inline
  tTx const& CXX_STDCALL teggo_min(tTx const &a,tTx const &b)
    {
      return a < b ? a : b;
    }

template <class tTx> inline
  tTx const& CXX_STDCALL teggo_max(tTx const &a,tTx const &b)
    {
      return a > b ? a : b;
    }

template <class tTx> inline
  tTx const CXX_STDCALL teggo_abs(tTx const a)
    {
      return (signed)a < 0 ? -(signed)a : a;
    }

template <class tTx> inline
  void CXX_STDCALL teggo_swap(tTx &a,tTx &b)
    {
      tTx temp(a);
      a = b;
      b = temp;
    }

#if (defined _DEBUG || defined DEBUG) && !defined _NOSTRICT
#define _STRICTCODECOP
#endif

#if (defined _DEBUG || defined DEBUG) && !defined _NODBGCODECOP
#define _DBGCODECOP
#endif

#if SYSTEM_IS_WINDOWS && !defined _WINDOWS_
# if !defined _X86_
#   define _X86_ 1
# endif
# if !defined WINVER
#   define WINVER 0x400
# endif
#include <windef.h>
#include <winbase.h>
#include <excpt.h>
#endif

#include "_codecop.h"
#include <new>

#if !defined EXTERN_C
#define EXTERN_C CXX_EXTERNC
#endif

template <class tTx> inline
  void CXX_STDCALL teggo_new_and_move_data (void *to, tTx* from )
    {
      new (to) tTx(*from);
    }

#include "_strategy_copy.h"
#include "_algo.h"
#include "_qsort.h"
#include "_shash.h"
#include "_crc32.h"
#include "_adler32.h"

#if !defined CXX_NO_MSCOM_SUPPORT
# if SYSTEM_IS_WINDOWS
# include <objbase.h>
# else
    typedef struct GUID 
      {
        u32_t   Data1;
        u16_t   Data2;
        u16_t   Data3;
        byte_t  Data4[8];
      } GUID;
# endif
#else
#define CXX_NO_GUID_SUPPORT
#endif

#if !defined _MALLOC
#define _MALLOC malloc
#endif

#if !defined _FREE
#define _FREE free
#endif

#if defined _TEGGONEW
inline void *CXX_CDECL operator new(unsigned size) { return _MALLOC(size); }
inline void *CXX_CDECL operator new[](unsigned size) { return _MALLOC(size); }
inline void CXX_CDECL operator delete(void *p) { if(p) _FREE(p); }
inline void CXX_CDECL operator delete[](void *p) { if(p) _FREE(p); }
#endif

namespace teggo
{

  enum resize_policy_t { FIXEDSIZE, RESIZABLE };
  enum reuse_policy_t { REUSE };
  enum swap_policy_t { SWAP_CONTENT };

  _TEGGO_EXPORTABLE wchar_t CXX_STDCALL ucs2_btowc(char c);
  _TEGGO_EXPORTABLE char CXX_STDCALL ucs2_wctob(wchar_t wc);

#if defined _TEGGOINLINE
# define _TEGGO_FAKE_STATIC static
# define _TEGGO_SPECIFIC_INIT(x) = x
#else
// # if CXX_COMPILER_IS_MSVC_COMPATIBLE
// #  if defined _DEBUG
// #    pragma comment(lib,"tffcls_d.lib")
// #  else
// #    pragma comment(lib,"tffcls.lib")
// #  endif
// # endif
# if defined _TEGGO_SPECIFIC_HERE
#  define _TEGGO_FAKE_STATIC
# else
#  define _TEGGO_FAKE_STATIC extern
# endif
#endif

#if !defined __DMC__
  _TEGGO_FAKE_STATIC std::nothrow_t nothrow;
#else
  struct nothrow_t {};
  _TEGGO_FAKE_STATIC nothrow_t nothrow;
  inline void* operator new (size_t sz,nothrow_t const&)
    {
      return ::operator new(sz);
    }
#endif

  struct DefaultAllocator
    {
      _TEGGO_EXPORTABLE void* Allocate( u32_t sz );
      _TEGGO_EXPORTABLE void Deallocate( void* p );
    };

  template < class tTx >
    struct mutable_t
      {
        mutable tTx value;
        typedef mutable_t<tTx> const &T;
        mutable_t () {}
      };

  template < bool tTrue = false >
    struct static_check
      {
        enum isFalse { value = 0 };
      };

  template <>
    struct static_check<true>
      {
        enum isTrue { value = 1 };
      };
} //namespace

template < class tTc > struct teggo_if_char_else;

template<> struct teggo_if_char_else<wchar_t>
  {
//     template < class tTx, class tTy >
//       static tTy Select( tTx &, tTy &a) { return a; }
    template < class tTx, class tTy >
      static tTy Select( tTx, tTy a) { return a; }
  };

template<> struct teggo_if_char_else<char>
  {
//     template < class tTx, class tTy >
//       static tTy Select( tTx &a, tTy &) { return a; }
    template < class tTx, class tTy >
      static tTy Select( tTx a, tTy) { return a; }
  };

#define TEGGO_STATIC_CHECK(expr) \
  void teggo_static_check___( teggo::static_check<expr>::isTrue );
#define DO_NOTHING ((void)0)

#if !SYSTEM_IS_WINDOWS 
  typedef void* HANDLE;

  _TEGGO_EXPORTABLE long InterlockedDecrement(long*);
  _TEGGO_EXPORTABLE long InterlockedIncrement(long*);
#endif

#if defined _TEGGOINLINE
# include "_specific.inl"
#else
# if 0
# if CXX_COMPILER_IS_MSVC_COMPATIBLE
#  if !defined _MSC_VER
#   define _TFFCLS_RTVER
#  elif _MSC_VER <= 1200
#   define _TFFCLS_RTVER
#  elif _MSC_VER < 1400
#   define _TFFCLS_RTVER "7"
#  elif _MSC_VER == 1400
#   define _TFFCLS_RTVER "8"
#  else
#   define _TFFCLS_RTVER "0"
#  endif
#  if defined _TEGGOTFFCLSLIB
#   ifdef _DEBUG
#    pragma comment(lib,"tffcls"_TFFCLS_RTVER"_d.lib")
#   else
#    pragma comment(lib,"tffcls"_TFFCLS_RTVER".lib")
#   endif
#  endif
# endif
# endif
#endif

#endif // __8903A158_94C4_4670_84F8_EEE59F0625DE__
