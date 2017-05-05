
/*

  Copyright (c) 2005-2009, Alexey Sudachen, alexey@sudachen.name

*/

#if !defined ___0db6185d_0223_4cd6_b32b_568242e1c30e___
#define ___0db6185d_0223_4cd6_b32b_568242e1c30e___

#include "genericio.h"

#if !defined _TEGGO_GENIO_HERE
# define _TEGGO_GENIO_FAKE_INLINE  CXX_FAKE_INLINE
#else
# define _TEGGO_GENIO_FAKE_INLINE  _TEGGO_EXPORTABLE
#endif

namespace teggo {

  template < class tTc, class tTc0 > CXX_NO_INLINE
    void CXX_STDCALL SprintAjust_(_XsprintAjuster<tTc> &_,tTc0 const text[],int ln)
      {
        tTc0 const *p = text;
        if ( _.width < 0 )
          {
            _->Copy(ln,p);
            if ( -_.width > ln )
              _->Fill(-_.width - ln,' ');
          }
        else
          {
            if ( _.width > ln )
              _->Fill( _.width - ln, _.filler);
            _->Copy(ln,p);
          }
      }
  _TEGGO_GENIO_FAKE_INLINE void CXX_STDCALL SprintAjust(_XsprintAjuster<char> &_,char const text[],int ln) { SprintAjust_(_,text,ln); }
  _TEGGO_GENIO_FAKE_INLINE void CXX_STDCALL SprintAjust(_XsprintAjuster<wchar_t> &_,char const text[],int ln) { SprintAjust_(_,text,ln); }
  _TEGGO_GENIO_FAKE_INLINE void CXX_STDCALL SprintAjust(_XsprintAjuster<char> &_,wchar_t const text[],int ln) { SprintAjust_(_,text,ln); }
  _TEGGO_GENIO_FAKE_INLINE void CXX_STDCALL SprintAjust(_XsprintAjuster<wchar_t> &_,wchar_t const text[],int ln) { SprintAjust_(_,text,ln); }

  template < class tTc > CXX_NO_INLINE
    void CXX_STDCALL SprintXnumber_(_XsprintAjuster<tTc> &_, unsigned int val, char const symbols[],unsigned bno)
      {
        char foo[16];
        unsigned mask = 0x0ff >> (8-bno);
        int i = sizeof(foo);

        if ( val )
          {
            for ( unsigned int v = val; v > 0; )
              {
                STRICT_REQUIRE( i > 1 );
                foo[--i] = symbols[ v & mask ];
                v >>= bno;
              }
          }
        else
          foo[--i] = '0';

        SprintAjust(_,foo+i,sizeof(foo)-i);
      }

  _TEGGO_GENIO_FAKE_INLINE void CXX_STDCALL SprintXnumber(_XsprintAjuster<char> &_, unsigned int val, char const symbols[],unsigned bno)
    { SprintXnumber_(_,val,symbols,bno); }
  _TEGGO_GENIO_FAKE_INLINE void CXX_STDCALL SprintXnumber(_XsprintAjuster<wchar_t> &_, unsigned int val, char const symbols[],unsigned bno)
    { SprintXnumber_(_,val,symbols,bno); }

  template < class tTc > CXX_NO_INLINE
    void CXX_STDCALL SprintPointer_(_XsprintAjuster<tTc> &_, void const*p)
      {
        char foo[8+2+1];
        static char const symbols[] = { '0','1','2','3','4','5','6','7','8','9','a','b','c','d','e','f' };
        memset(foo,'0',sizeof(foo));
        foo[0] = '#';
        foo[1] = 'x';
        foo[8+2] = 0;
        for ( unsigned int v = (u32_t&)p, i = 8+2; v > 0; )
          {
            STRICT_REQUIRE( i > 1 );
            foo[--i] = symbols[ v & 0x0f ];
            v >>= 4;
          }
        SprintAjust(_,foo,8+2);
      }
  _TEGGO_GENIO_FAKE_INLINE void CXX_STDCALL SprintPointer(_XsprintAjuster<char> &_, void const*p) { SprintPointer_(_,p); }
  _TEGGO_GENIO_FAKE_INLINE void CXX_STDCALL SprintPointer(_XsprintAjuster<wchar_t> &_, void const*p) { SprintPointer_(_,p); }


  template < class tTc > CXX_NO_INLINE
    void CXX_STDCALL SprintFloat_(_XsprintAjuster<tTc> &_, double val, int perc)
      {
        static int perc_[] = { 0, 10, 100, 1000, 10000, 100000, 1000000 };
        if ( perc > 6 ) perc = 6;
        tTc foo[64];
        unsigned i = sizeof(foo)/sizeof(tTc);
        if ( perc )
          {
            //unsigned val0 = teggo_abs(int(val * perc)) % perc + perc_[perc];
            unsigned val0 = teggo_abs(int(val * perc_[perc])) % perc_[perc] + perc_[perc];
            _XsprintPrepareDigits(foo,i,val0);
            foo[i] = '.';
          }
        int val1 = int(val);
        //while ( i > (sizeof(foo)-perc) ) foo[i--] = '0';
        _XsprintPrepareDigits(foo,i,val1);
        SprintAjust(_,foo+i,sizeof(foo)/sizeof(tTc)-i);
      }


  _TEGGO_GENIO_FAKE_INLINE void CXX_STDCALL SprintFloat(_XsprintAjuster<char> &_, double val, int perc) {SprintFloat_(_,val,perc);}
  _TEGGO_GENIO_FAKE_INLINE void CXX_STDCALL SprintFloat(_XsprintAjuster<wchar_t> &_, double val, int perc) {SprintFloat_(_,val,perc);}

  _TEGGO_GENIO_FAKE_INLINE
  void CXX_STDCALL SprintGUID(_XsprintAjuster<char> &_, GUID const *guid, bool ucase)
    {
      char out[48];
      SprintF(out,48,
          ((!ucase) ?
            "%08x-%04x-%04x-%02x%02x-%02x%02x%02x%02x%02x%02x" :
            "%08X-%04X-%04X-%02X%02X-%02X%02X%02X%02X%02X%02X" ),
          guid->Data1,guid->Data2,guid->Data3,
          guid->Data4[0],guid->Data4[1],guid->Data4[2],guid->Data4[3],
          guid->Data4[4],guid->Data4[5],guid->Data4[6],guid->Data4[7]);
      SprintStringC(_,out);
    }

  _TEGGO_GENIO_FAKE_INLINE
  void CXX_STDCALL SprintGUID(_XsprintAjuster<wchar_t> &_, GUID const *guid, bool ucase)
    {
      wchar_t out[48];
      SprintF(out,48,
          ((!ucase) ?
            L"%08x-%04x-%04x-%02x%02x-%02x%02x%02x%02x%02x%02x" :
            L"%08X-%04X-%04X-%02X%02X-%02X%02X%02X%02X%02X%02X" ),
          guid->Data1,guid->Data2,guid->Data3,
          guid->Data4[0],guid->Data4[1],guid->Data4[2],guid->Data4[3],
          guid->Data4[4],guid->Data4[5],guid->Data4[6],guid->Data4[7]);
      SprintStringW(_,out);
    }

  template <class tTc>
    GenericWriterT<tTc> CXX_STDCALL _X_xToa_writer(tTc*)
      {
        static tTc buf[64];
        return GenericWriterT<tTc>(buf,buf,buf+sizeof(buf)/sizeof(buf[0]));
      }

  template <class tTc>
    tTc const * CXX_STDCALL Utox_(unsigned a,int radix,tTc*)
      {
        GenericWriterT<tTc> gw = _X_xToa_writer((tTc*)0);
        _XsprintAjuster<tTc> o(gw);
        o.width = 0;
        o.filler = (tTc)'0';

        switch( radix )
          {
            case 2:
              o.width = 32;
              SprintBinary(o,a);
              break;
            case 8:
              SprintOctet(o,a);
              break;
            case 10:
              SprintUnsigned(o,a);
              break;
            case 16:
              SprintXlower(o,a);
              break;
            default:
              StrCopyN(gw.outS,BaseStringT<tTc>::SelectTstr("(badradix)",L"(badradix)"),10);
          }

        gw.Push((tTc)'\0');
        return gw.outS;
      }

  _TEGGO_GENIO_FAKE_INLINE pchar_t CXX_STDCALL Utoa(unsigned a,int radix) { return Utox_(a,radix,(char*)0); }
  _TEGGO_GENIO_FAKE_INLINE pwide_t CXX_STDCALL Utow(unsigned a,int radix) { return Utox_(a,radix,(wchar_t*)0); }

  template < class tTc >
    tTc const * CXX_STDCALL Itox_(int a, int radix, tTc* )
      {
        if ( radix == 10 )
          {
            GenericWriterT<tTc> gw = _X_xToa_writer((tTc*)0);
            _XsprintAjuster<tTc> o(gw);
            o.width = 0;
            o.filler = (tTc)'0';
            SprintSigned(o,a);
            gw.Push((tTc)'\0');
            return gw.outS;
          }
        else
          return Utox_(a,radix,(tTc*)0);
      }

  _TEGGO_GENIO_FAKE_INLINE pchar_t CXX_STDCALL Itoa(int a,int radix) { return Itox_(a,radix,(char*)0); }
  _TEGGO_GENIO_FAKE_INLINE pwide_t CXX_STDCALL Itow(int a,int radix) { return Itox_(a,radix,(wchar_t*)0); }

  template < class tTc >
    tTc const * CXX_STDCALL Ptox_(void *a,tTc*)
      {
        GenericWriterT<tTc> gw = _X_xToa_writer((tTc*)0);
        _XsprintAjuster<tTc> o(gw);
        o->Push((tTc)'0');
        o->Push((tTc)'x');
        o.width = 8;
        o.filler = '0';
        SprintXupper(o,(u32_t&)a);
        gw.Push((tTc)'\0');
        return gw.outS;
      }

  _TEGGO_GENIO_FAKE_INLINE pchar_t CXX_STDCALL Ptoa(void *a) { return Ptox_(a,(char*)0); }
  _TEGGO_GENIO_FAKE_INLINE pwide_t CXX_STDCALL Ptow(void *a) { return Ptox_(a,(wchar_t*)0); }

  template < class tTc >
    tTc const * CXX_STDCALL Ftox_(double a,int perc,tTc*)
      {
        GenericWriterT<tTc> gw = _X_xToa_writer((tTc*)0);
        _XsprintAjuster<tTc> o(gw);
        o.width = 0;
        o.filler = (tTc)'0';
        SprintFloat(o,a,perc);
        gw.Push((tTc)'\0');
        return gw.outS;
      }

  _TEGGO_GENIO_FAKE_INLINE pchar_t CXX_STDCALL Ftoa(double a,int perc) { return Ftox_(a,perc,(char*)0); }
  _TEGGO_GENIO_FAKE_INLINE pwide_t CXX_STDCALL Ftow(double a,int perc) { return Ftox_(a,perc,(wchar_t*)0); }

  template < class tTc >
    int CXX_STDCALL VsprintF_( GenericWriterT<tTc> &gw, tTc const *fmt, _XsprintArglist<tTc> const &xa)
      {
        _XsprintAjuster<tTc> o(gw);

        int nfld = 0;
        unsigned perc;
        tTc const *fmtI = fmt;

        do
          {
            while ( *fmtI && *fmtI != '%' && !o->Overflow() ) o->Push(*fmtI++);
            while ( *fmtI == '%' )
              {
                ++fmtI;
                if ( *fmtI == '%' )
                  {
                    if ( o->Overflow() ) goto lb_end;
                    o->Push('%');
                    ++fmtI;
                    continue;
                  }

                bool long_fmt = false;

                o.filler = ' ';
                o.width = 0;
                perc = 0;

                {
                  int sign = 1;
                  if ( *fmtI == '-' ) { sign = -1; ++fmtI; }
                  if ( *fmtI == '0' ) o.filler = '0';
                  for ( ; isdigit(*fmtI) ; fmtI++ )
                    o.width = o.width*10 + (*fmtI-'0');
                  o.width *= sign;
                  if ( *fmtI == '.' )
                    {
                      ++fmtI;
                      for ( ; isdigit(*fmtI) ; fmtI++ )
                        perc = perc*10 + (*fmtI-'0');
                    }
                }

                if ( *fmtI == 'l' )
                  { long_fmt = true; ++fmtI; }

                xa.OutValue( o, perc, *fmtI++, long_fmt );
              }
          }
        while ( *fmtI && !o->Overflow() );

      lb_end:
        if ( !o->Overflow() )
          {
            o->Push('\0');
            return o->Count()-1;
          }
        else
          return -o->Count();
      }

  _TEGGO_GENIO_FAKE_INLINE
  int CXX_STDCALL VsprintF( GenericWriterT<wchar_t> &gw, wchar_t const *fmt, _XsprintArglist<wchar_t> const &xa)
    {
      return VsprintF_( gw, fmt, xa );
    }

  _TEGGO_GENIO_FAKE_INLINE
  int CXX_STDCALL VsprintF( GenericWriterT<char> &gw, char const *fmt, _XsprintArglist<char> const &xa)
    {
      return VsprintF_( gw, fmt, xa );
    }

} //namespace

#endif  //___0db6185d_0223_4cd6_b32b_568242e1c30e___
