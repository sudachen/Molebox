
/*

  Copyright (c) 2005-2009, Alexey Sudachen, alexey@sudachen.name

*/

#if !defined ___ff69e169_b893_4497_a3f2_a62bfa3a798c___
#define ___ff69e169_b893_4497_a3f2_a62bfa3a798c___

#include "_specific.h"
#include "string.h"

namespace teggo {

  template < class tTc >
    struct GenericWriterT
      {
        typedef tTc tChar;

        void Setup(tTc *S,tTc *I, tTc *E) { outS = S; outI = I, outE = E; }
        bool extendable_;
        unsigned count_;
        tTc *outS;
        tTc *outI;
        tTc *outE;

        GenericWriterT(bool extendable = false);
        GenericWriterT(tTc *S,tTc *I, tTc *E);
        virtual ~GenericWriterT();

        template < class tTc0 > CXX_NO_INLINE
          void Fill_(unsigned cnt,tTc0 c)
            {
              tTc cc;
              teggo::ChrSet(&cc,c);
              if ( extendable_ && outI + cnt > outE )
                Require(cnt);
              for ( unsigned iE = cnt+count_ ; count_ < iE && outI != outE; ++outI, ++count_ )
                *outI = cc;
            };

        template < class tTc0 > void Fill(unsigned cnt,tTc0 c) { Fill_(cnt,c); }
        void Fill(unsigned cnt,char c) { Fill_(cnt,c); }
        void Fill(unsigned cnt,wchar_t c) { Fill_(cnt,c); }

        template < class tTc0 > CXX_NO_INLINE
          void Copy_(unsigned cnt,tTc0 const *c)
            {
              if ( extendable_ && outI + cnt > outE )
                Require(cnt);
              for ( unsigned iE = cnt+count_; count_ < iE && outI != outE; ++outI, ++count_ )
                teggo::ChrSet(outI,*c++);
            }

        template < class tTc0 > void Copy(unsigned cnt,tTc0 const *c) { Copy_(cnt,c); }
        void Copy(unsigned cnt,char const *c) { Copy_(cnt,c); }
        void Copy(unsigned cnt,wchar_t const *c) { Copy_(cnt,c); }

        template < class tTc0 > CXX_NO_INLINE
          void Push(tTc0 c)
            {
              if ( outI != outE || (extendable_ && Require(1)) )
                {
                  teggo::ChrSet(outI++,c);
                  ++count_;
                }
            }

        bool Overflow() { return !extendable_ && outI == outE; }
        int Count() { return count_; }

        virtual bool Require(unsigned) { return false; }
      };

  template < class tTc > CXX_NO_INLINE
    GenericWriterT<tTc>::GenericWriterT(bool extendable)
    : extendable_(extendable) , count_(0), outS(0), outI(0), outE(0)
      {}

  template < class tTc > CXX_NO_INLINE
    GenericWriterT<tTc>::GenericWriterT(tTc *S, tTc *I, tTc *E)
    : extendable_(false) , count_(0), outS(S), outI(I), outE(E)
      {}

  template < class tTc > CXX_NO_INLINE
    GenericWriterT<tTc>::~GenericWriterT() {}

  _TEGGO_EXPORTABLE pchar_t CXX_STDCALL Itoa(int a,int radix=10);
  _TEGGO_EXPORTABLE pchar_t CXX_STDCALL Utoa(unsigned int a,int radix=10);
  _TEGGO_EXPORTABLE pchar_t CXX_STDCALL Ftoa(double a,int perc=3);
  _TEGGO_EXPORTABLE pchar_t CXX_STDCALL Ptoa(void *a);
  _TEGGO_EXPORTABLE pwide_t CXX_STDCALL Itow(int a,int radix=10);
  _TEGGO_EXPORTABLE pwide_t CXX_STDCALL Utow(unsigned int a,int radix=10);
  _TEGGO_EXPORTABLE pwide_t CXX_STDCALL Ftow(double a,int perc=3);
  _TEGGO_EXPORTABLE pwide_t CXX_STDCALL Ptow(void *a);

  template < class tTc >
    struct _XsprintAjuster
      {
        typedef tTc tChar;

        int width;
        tTc filler;
        GenericWriterT<tTc> &gw_;
        _XsprintAjuster(GenericWriterT<tTc> &gw);
        ~_XsprintAjuster();
        GenericWriterT<tTc>* operator->() { return &gw_; }
      };

  template < class tTc > CXX_NO_INLINE
    _XsprintAjuster<tTc>::_XsprintAjuster(GenericWriterT<tTc> &gw) : gw_(gw) {}

  template < class tTc > CXX_NO_INLINE
    _XsprintAjuster<tTc>::~_XsprintAjuster() {}

  template < class tTc > void CXX_STDCALL SprintBinary(_XsprintAjuster<tTc> &_, unsigned int val);
  template < class tTc > void CXX_STDCALL SprintXlower(_XsprintAjuster<tTc> &_, unsigned int val);
  template < class tTc > void CXX_STDCALL SprintXupper(_XsprintAjuster<tTc> &_, unsigned int val);
  template < class tTc > void CXX_STDCALL SprintOctet(_XsprintAjuster<tTc> &_, unsigned int val);
  template < class tTc > void CXX_STDCALL SprintSigned(_XsprintAjuster<tTc> &_, int val);
  template < class tTc > void CXX_STDCALL SprintUnsigned(_XsprintAjuster<tTc> &_, unsigned int val);
  template < class tTc > void CXX_STDCALL SprintChar(_XsprintAjuster<tTc> &_, tTc val);
//  template < class tTc > void CXX_STDCALL SprintStringC(_XsprintAjuster<tTc> &_, char const *val,int = 0);
//  template < class tTc > void CXX_STDCALL SprintStringW(_XsprintAjuster<tTc> &_, wchar_t const *val,int = 0);
  template < class tTc > void CXX_STDCALL SprintGUID(_XsprintAjuster<tTc> &_, GUID *val);

  template < class tTc >
    struct CXX_NO_VTABLE _XsprintArglist
      {
        virtual void OutValue( _XsprintAjuster<tTc> &xa, unsigned perc, tTc fmt, bool long_fmt = false ) const = 0;
      };

  _TEGGO_EXPORTABLE int CXX_STDCALL VsprintF(GenericWriterT<char> &gw, char const *fmt, _XsprintArglist<char> const &xa);
  _TEGGO_EXPORTABLE int CXX_STDCALL VsprintF(GenericWriterT<wchar_t> &gw, wchar_t const *fmt, _XsprintArglist<wchar_t> const &xa);

  template < class tTc >
    struct _XsprintArglistVa : _XsprintArglist<tTc>
      {
#if CXX_COMPILER_IS_GXX
        mutable va_list &va_;
#else 
        mutable va_list va_;
#endif
        virtual void OutValue( _XsprintAjuster<tTc> &o, unsigned perc, tTc fmt, bool long_fmt = false ) const
          {
            switch ( fmt )
              {
                case 'x': SprintXlower(o,va_arg(va_,unsigned)); break;
                case 'X': SprintXupper(o,va_arg(va_,unsigned)); break;
                case 'b': SprintBinary(o,va_arg(va_,unsigned)); break;
                case 'i': case 'd': SprintSigned(o,va_arg(va_,int)); break;
                case 'u': SprintUnsigned(o,va_arg(va_,unsigned)); break;
                case 'o': SprintOctet(o,va_arg(va_,unsigned)); break;
                case 'c': SprintChar(o,(tTc)va_arg(va_,unsigned)); break;
                case 's':
                  if (!long_fmt) {SprintStringC(o,va_arg(va_,pchar_t),perc); break;}
                case 'S': // else
                  SprintStringW(o,va_arg(va_,pwide_t),perc); break;
                case 'f': case 'g': SprintFloat(o,va_arg(va_,double),perc); break;
                case 'p': SprintPointer(o,va_arg(va_,void*)); break;
                case 'j': SprintGUID(o,va_arg(va_,GUID*),false); break;
                case 'J': SprintGUID(o,va_arg(va_,GUID*),true); break;
                default:
                  SprintStringC(o,"(badformat)",0);
              }
          }
#if CXX_COMPILER_IS_GXX
        _XsprintArglistVa(va_list va) : va_((va_list&)va) {}
#else
        _XsprintArglistVa(va_list va) : va_(va) {}
#endif
        ~_XsprintArglistVa() {}
      };

  template < class tTc > CXX_NO_INLINE
    int CXX_STDCALL VsprintF(tTc *out, unsigned sz, tTc const *fmt, va_list va)
      {
        GenericWriterT<tTc> gw;
        gw.Setup(out,out,out+sz);
        return VsprintF(gw,fmt,_XsprintArglistVa<tTc>(va));
      }


  template < class tTc > CXX_NO_INLINE
    int CXX_CDECL SprintF(tTc *out, unsigned sz, tTc const *fmt, ...)
      {
        va_list va;
        va_start(va,fmt);
        int retval = VsprintF(out,sz,fmt,va);
        va_end(va);
        return retval;
      }


  _TEGGO_EXPORTABLE void CXX_STDCALL SprintAjust(_XsprintAjuster<char> &_,char const text[],int ln);
  _TEGGO_EXPORTABLE void CXX_STDCALL SprintAjust(_XsprintAjuster<wchar_t> &_,char const text[],int ln);
  _TEGGO_EXPORTABLE void CXX_STDCALL SprintAjust(_XsprintAjuster<char> &_,wchar_t const text[],int ln);
  _TEGGO_EXPORTABLE void CXX_STDCALL SprintAjust(_XsprintAjuster<wchar_t> &_,wchar_t const text[],int ln);
  _TEGGO_EXPORTABLE void CXX_STDCALL SprintXnumber(_XsprintAjuster<char> &_, unsigned int val, char const symbols[],unsigned bno);
  _TEGGO_EXPORTABLE void CXX_STDCALL SprintXnumber(_XsprintAjuster<wchar_t> &_, unsigned int val, char const symbols[],unsigned bno);

  template < class tTc > CXX_NO_INLINE
    void CXX_STDCALL SprintBinary(_XsprintAjuster<tTc> &_, unsigned int val)
      {
        static char const symbols[] = { '0','1' };
        SprintXnumber(_,val,symbols,1);
      }

  template < class tTc > CXX_NO_INLINE
    void CXX_STDCALL SprintXlower(_XsprintAjuster<tTc> &_, unsigned int val)
      {
        static char const symbols[] = { '0','1','2','3','4','5','6','7','8','9','a','b','c','d','e','f' };
        SprintXnumber(_,val,symbols,4);
      }

  template < class tTc > CXX_NO_INLINE
    void CXX_STDCALL SprintXupper(_XsprintAjuster<tTc> &_, unsigned int val)
      {
        static char const symbols[] = { '0','1','2','3','4','5','6','7','8','9','A','B','C','D','E','F' };
        SprintXnumber(_,val,symbols,4);
      }

  template < class tTc > CXX_NO_INLINE
    void CXX_STDCALL SprintOctet(_XsprintAjuster<tTc> &_, unsigned int val)
      {
        static char const symbols[] = { '0','1','2','3','4','5','6','7' };
        SprintXnumber(_,val,symbols,3);
      }

  template < class tTc > CXX_NO_INLINE
    void CXX_STDCALL _XsprintPrepareDigits( tTc *p, unsigned &i, unsigned int v )
      {
        if ( v )
          {
            while (  v > 0 )
              {
                STRICT_REQUIRE( i > 1 );
                p[--i] = '0'+(v % 10);
                v /= 10;
              }
          }
        else
          p[--i] = '0';
      }

  template < class tTc > CXX_NO_INLINE
    void CXX_STDCALL SprintSigned(_XsprintAjuster<tTc> &_, int val)
      {
        tTc foo[16];
        unsigned i = sizeof(foo)/sizeof(tTc);
        _XsprintPrepareDigits(foo,i,(val<0?-val:val));
        if ( val < 0 ) foo[--i] = '-';
        SprintAjust(_,foo+i,sizeof(foo)/sizeof(tTc)-i);
      }

  template < class tTc > CXX_NO_INLINE
    void CXX_STDCALL SprintUnsigned(_XsprintAjuster<tTc> &_, unsigned int val)
      {
        tTc foo[16];
        unsigned i = sizeof(foo)/sizeof(tTc);
        _XsprintPrepareDigits(foo,i,val);
        SprintAjust(_,foo+i,sizeof(foo)/sizeof(tTc)-i);
      }

  template < class tTc > CXX_NO_INLINE
    void CXX_STDCALL SprintChar(_XsprintAjuster<tTc> &_, tTc val)
      {
        SprintAjust(_,&val,1);
      }

  template < class tTc > CXX_NO_INLINE
    void CXX_STDCALL SprintStringC(_XsprintAjuster<tTc> &_, char const *val, int limit=0)
      {
        if ( !val ) val = "(null)";
        u32_t l = StrLen(val);
        if ( limit && limit < l ) l = limit;
        SprintAjust(_,val,l);
      }

  template < class tTc > CXX_NO_INLINE
    void CXX_STDCALL SprintStringW(_XsprintAjuster<tTc> &_, wchar_t const *val, int limit=0)
      {
        if ( !val ) val = L"(null)";
        u32_t l = StrLen(val);
        if ( limit && limit < l ) l = limit;
        SprintAjust(_,val,l);
      }

  _TEGGO_EXPORTABLE void CXX_STDCALL SprintPointer(_XsprintAjuster<char> &_, void const*p);
  _TEGGO_EXPORTABLE void CXX_STDCALL SprintPointer(_XsprintAjuster<wchar_t> &_, void const*p);
  _TEGGO_EXPORTABLE void CXX_STDCALL SprintFloat(_XsprintAjuster<char> &_, double val, int perc);
  _TEGGO_EXPORTABLE void CXX_STDCALL SprintFloat(_XsprintAjuster<wchar_t> &_, double val, int perc);

  template < class tTc >
    struct StringTwriterT : GenericWriterT<tTc>
      {
        typedef GenericWriterT<tTc> B_;
        BaseStringT<tTc> &out_;
        StringTwriterT(BaseStringT<tTc> &out) : GenericWriterT<tTc>(true), out_(out)
          {
            Setup(out_.Begin(),out_.End(),out_.End());
          }
        /*override*/ bool Require(unsigned cnt)
          {
            if ( B_::outE-B_::outI < signed(cnt) )
              {
                out_.Resize( (B_::outI-B_::outS) + (cnt - (B_::outE-B_::outI)) );
                B_::outI = out_.Begin() + (B_::outI-B_::outS);
                B_::outS = out_.Begin();
                B_::outE = out_.End();
              }
            return true;
          }
      };

  template < class tTc >
    int CXX_STDCALL VsprintF(BaseStringT<tTc> &b, tTc const *f, va_list va)
      {
        StringTwriterT<tTc> wr(b);
        return VsprintF(wr,f,_XsprintArglistVa<tTc>(va));
      }

  _TEGGO_EXPORTABLE void CXX_STDCALL SprintGUID(_XsprintAjuster<char> &_, GUID const *guid, bool ucase);
  _TEGGO_EXPORTABLE void CXX_STDCALL SprintGUID(_XsprintAjuster<wchar_t> &_, GUID const *guid, bool ucase);

} // end namespace

#if defined _TEGGOINLINE
#include "GenericIO.inl"
#endif

#endif // ___ff69e169_b893_4497_a3f2_a62bfa3a798c___
