
/*

  Copyright (c) 2005-2009, Alexey Sudachen, alexey@sudachen.name

*/

#if !defined __9682C09E_C256_4e85_8872_AD7597F6BFAC__
#define __9682C09E_C256_4e85_8872_AD7597F6BFAC__

#include "_specific.h"
#include "genericio.h"
#include "string.h"
#include "array.h"

namespace teggo {

  template < class tTc > struct FormatT;

/*  _TEGGO_EXPORTABLE BaseStringT<wchar_t>& BaseStringT<wchar_t>::operator =(FormatT<wchar_t> const&f);
  _TEGGO_EXPORTABLE BaseStringT<char>& BaseStringT<char>::operator =(FormatT<char> const&f);
  _TEGGO_EXPORTABLE BaseStringT<wchar_t>& BaseStringT<wchar_t>::operator =(FormatT<char> const&f);
  _TEGGO_EXPORTABLE BaseStringT<char>& BaseStringT<char>::operator =(FormatT<wchar_t> const&f); */

  _TEGGO_EXPORTABLE void CXX_STDCALL FormatT_WriteFormat( FormatT<char> const *fmt, GenericWriterT<char> &wr );
  _TEGGO_EXPORTABLE void CXX_STDCALL FormatT_WriteFormat( FormatT<char> const *fmt, GenericWriterT<wchar_t> &wr );
  _TEGGO_EXPORTABLE void CXX_STDCALL _FmtOutValue( _XsprintAjuster<char> &xa, unsigned perc, char fmt, FormatT<char> const *t );
  _TEGGO_EXPORTABLE void CXX_STDCALL _FmtOutValue( _XsprintAjuster<wchar_t> &xa, unsigned perc, wchar_t fmt, FormatT<char> const *t );

  template < class tTc >
    struct FormatT_
      {
        StringT<tTc> s_;
        tTc const* Str() const { return s_.Str(); }
        CXX_NO_INLINE FormatT_(FormatT<tTc> const &ft) : s_(ft) {}
      };

  template < class tTc >
    struct FormatT
      {
        FormatT const *ft_;
        enum CONTENT_TYPE { xT, xS, xW, xI, xU, xF, xP, xJ } t_;
        union
          {
            pchar_t   sval_;
            pwide_t   wval_;
            int       ival_;
            unsigned  uval_;
            double    fval_;
            void const *pval_;
            GUID const *gval_;
          };

        explicit FormatT( tTc const *fmt ) : ft_(0), t_(xT) { sval_ = (pchar_t)fmt; }

        template < class tTx >
        void operator ,(tTx const &x) { x.__there_is_invalid_operation_comma__(); }

        explicit FormatT( FormatT const *ft, pchar_t s ) : ft_(ft), t_(xS) { sval_ = s; }
        explicit FormatT( FormatT const *ft, pwide_t w ) : ft_(ft), t_(xW) { wval_ = w; }
        explicit FormatT( FormatT const *ft, BaseStringT<char> const &s ) : ft_(ft), t_(xS) { sval_ = s.Str(); }
        explicit FormatT( FormatT const *ft, BaseStringT<wchar_t> const &w ) : ft_(ft), t_(xW) { wval_ = w.Str(); }
        explicit FormatT( FormatT const *ft, int l ) : ft_(ft), t_(xI) { ival_ = l; }
        explicit FormatT( FormatT const *ft, long u ) : ft_(ft), t_(xI) { ival_ = u; }
        explicit FormatT( FormatT const *ft, unsigned u ) : ft_(ft), t_(xU) { uval_ = u; }
        explicit FormatT( FormatT const *ft, unsigned long u ) : ft_(ft), t_(xU) { uval_ = u; }
        explicit FormatT( FormatT const *ft, float f ) : ft_(ft), t_(xF) { fval_ = f; }
        explicit FormatT( FormatT const *ft, double f ) : ft_(ft), t_(xF) { fval_ = f; }
        explicit FormatT( FormatT const *ft, FormatT_<char> const &s, int ) : ft_(ft), t_(xS) {sval_ = s.Str();}
        explicit FormatT( FormatT const *ft, FormatT_<wchar_t> const &s, int ) : ft_(ft), t_(xW) {wval_ = s.Str();}
        explicit FormatT( FormatT const *ft, GUID const &s ) : ft_(ft), t_(xJ) {gval_ = &s;}
        explicit FormatT( FormatT const *ft, GUID const *s ) : ft_(ft), t_(xJ) {gval_ = s;}

        explicit FormatT( FormatT const *ft, void const*p, bool _) : ft_(ft), t_(xP) { pval_ = p; }
        FormatT operator % (void const *val) const { return FormatT(this,val,false); }

        //template < class tTy >
        //  FormatT operator % (tTy const &val) const { return FormatT(this,val); }

        FormatT operator % (float val) const { return FormatT(this,val); }
        FormatT operator % (double val) const { return FormatT(this,val); }
        FormatT operator % (int val) const { return FormatT(this,val); }
        FormatT operator % (long val) const { return FormatT(this,val); }
        FormatT operator % (unsigned int val) const { return FormatT(this,val); }
        FormatT operator % (unsigned long val) const { return FormatT(this,val); }
        FormatT operator % (BaseStringT<char> const &val) const { return FormatT(this,val); }
        FormatT operator % (BaseStringT<wchar_t> const &val) const { return FormatT(this,val); }
        FormatT operator % (GUID const &val) const { return FormatT(this,val); }
        FormatT operator % (GUID const *val) const { return FormatT(this,val); }

        FormatT operator % (char val[]) const { return FormatT(this,(char const*)val); }
        FormatT operator % (wchar_t val[]) const { return FormatT(this,(wchar_t const*)val); }
        FormatT operator % (char const val[]) const { return FormatT(this,(char const*)val); }
        FormatT operator % (wchar_t const val[]) const { return FormatT(this,(wchar_t const*)val); }

        FormatT operator % (BufferT<char> &b) const { return FormatT(this,+b); }
        FormatT operator % (BufferT<wchar_t> &b) const { return FormatT(this,+b); }

        FormatT operator %(FormatT_<char> const &val)
          {
            return FormatT(this,val,0);
          }
        FormatT operator %(FormatT_<wchar_t> const &val)
          {
            return FormatT(this,val,0);
          }

        template < class tTc0 > CXX_NO_INLINE
          void _OutValue( _XsprintAjuster<tTc0> &xa, unsigned perc, tTc0 fmt, ... ) const
            {
              _FmtOutValue(xa,perc,fmt,(FormatT<char> const*)this);
            }

        CXX_NO_INLINE tTc const *Write( BaseStringT<tTc> &buf, ... ) const
          {
            StringTwriterT<tTc> wr(buf);
            FormatT_WriteFormat( (FormatT<char> const *)this, wr );
            return buf.FixCstr().Str();
          }

        void Write( GenericWriterT<tTc> &wr ) const
          {
            FormatT_WriteFormat( (FormatT<char> const *)this, wr );
          }

        tTc const *Str( mutable_t< StringT<tTc> > const &tmp = mutable_t< StringT<tTc> >(), ... ) const
          {
            StringTwriterT<tTc> wr(tmp.value);
            FormatT_WriteFormat( (FormatT<char> const *)this, wr );
            return tmp.value.FixCstr().Str();
          }

        StringT<tTc> operator +() const { return StringT<tTc>(*this); }
        StringT<tTc> operator ~() const { return operator+(); }
      };

//     inline FormatT<char> operator %(FormatT<char> const &p, BaseStringT<char> const &val) { return FormatT<char>(&p,val); }
//     inline FormatT<wchar_t> operator %(FormatT<wchar_t> const &p, BaseStringT<char> const &val) { return FormatT<wchar_t>(&p,val); }
//     inline FormatT<char> operator %(FormatT<char> const &p, BaseStringT<wchar_t> const &val) { return FormatT<char>(&p,val); }
//     inline FormatT<wchar_t> operator %(FormatT<wchar_t> const &p, BaseStringT<wchar_t> const &val) { return FormatT<wchar_t>(&p,val); }

  template < class tTc >
    struct FormatT128_
      {
        StringT<tTc,128> s_;
        tTc const* Str() const { return s_.Str(); }
        CXX_NO_INLINE FormatT128_(FormatT<tTc> const &ft) : s_(ft) {}
      };

  template < class tTc >
    struct FormatT128x_
      {
        StringT<tTc,128> s_;
        tTc const* Str() const { return s_.Str(); }
        CXX_NO_INLINE FormatT128x_(FormatT<char> const &ft) : s_(StringT<char,128>(ft)) {}
        CXX_NO_INLINE FormatT128x_(FormatT<wchar_t> const &ft) : s_(StringT<wchar_t,128>(ft))  {}
      };

  template <class tTchr,unsigned PRESERVE, resize_policy_t RESIZE_POLICY>
    StringT<tTchr,PRESERVE,RESIZE_POLICY>::StringT( FormatT<tTchr> const &fmt )
      {
        BaseStringT<tTchr>::InitValue(R,RESIZE_POLICY,(tTchr const*)0,0);
        fmt.Write(*this);
      }

#if TL_COMPILER_IS_MSVC_COMPATIBLE
#pragma warning(disable:4355)
#endif

  _TEGGO_EXPORTABLE BaseStringT<char> & CXX_STDCALL operator <<(BaseStringT<char> &s, FormatT<char> v);
  _TEGGO_EXPORTABLE BaseStringT<wchar_t> & CXX_STDCALL operator <<(BaseStringT<wchar_t> &s, FormatT<wchar_t> v);
  _TEGGO_EXPORTABLE BaseStringT<char> & CXX_STDCALL operator <<(BaseStringT<char> &s, FormatT<wchar_t> v);
  _TEGGO_EXPORTABLE BaseStringT<wchar_t> & CXX_STDCALL operator <<(BaseStringT<wchar_t> &s, FormatT<char> v);
  _TEGGO_EXPORTABLE BufferT<char> & CXX_STDCALL operator <<(BufferT<char> &b, FormatT<char> v);
  _TEGGO_EXPORTABLE BufferT<wchar_t> & CXX_STDCALL operator <<(BufferT<wchar_t> &b, FormatT<wchar_t> v);

#if defined _TEGGOINLINE || defined _TEGGO_FORMAT_HERE

#if !defined _TEGGO_FORMAT_HERE
# define _TEGGO_FORMAT_FAKE_INLINE  CXX_FAKE_INLINE
#else
# define _TEGGO_FORMAT_FAKE_INLINE
#endif

  template < class tXa >
    void CXX_STDCALL _FmtInt( tXa &xa, int val, char fmt, unsigned perc )
      {
        switch ( fmt )
          {
            case 'x': SprintXlower(xa,val); break;
            case 'X': SprintXupper(xa,val); break;
            case 'b': SprintBinary(xa,val); break;
            case 'o': SprintOctet(xa,val); break;
            case 'f': case 'g': SprintFloat(xa,val,perc); break;
            case 'u': SprintUnsigned(xa,val); break;
            case 'c': xa->Push((typename tXa::tChar)val); break;
            //case (tTc)'d':
            default:
              SprintSigned(xa,val);
          }
      }

  template < class tXa >
    void CXX_STDCALL _FmtUint( tXa &xa, unsigned val, char fmt, unsigned perc )
      {
        switch ( fmt )
          {
            case 'x': SprintXlower(xa,val); break;
            case 'X': SprintXupper(xa,val); break;
            case 'b': SprintBinary(xa,val); break;
            case 'o': SprintOctet(xa,val); break;
            case 'f':case 'g': SprintFloat(xa,val,perc); break;
            case 'd': SprintSigned(xa,val); break;
            case 'c': xa->Push((typename tXa::tChar)val); break;
            //case 'u':
            default:
              SprintUnsigned(xa,val);
          }
      }

  template < class tXa >
    void CXX_STDCALL _FmtFloat( tXa &xa, double val, char fmt, unsigned perc )
      {
        switch ( fmt )
          {
            case 'x': SprintXlower(xa,(unsigned int)val); break;
            case 'X': SprintXupper(xa,(unsigned int)val); break;
            case 'b': SprintBinary(xa,(unsigned int)val); break;
            case 'o': SprintOctet (xa,(unsigned int)val); break;
            case 'd': SprintSigned(xa,(int)val); break;
            //case 'f':
            default:
              SprintFloat(xa,val,perc);
          }
      }

  template < class tXa >
    void CXX_STDCALL _FmtPtr( tXa &xa, void const *val, char fmt )
      {
        switch ( fmt )
          {
            case 'x': SprintXlower(xa,(unsigned long)(val)); break;
            case 'X': SprintXupper(xa,(unsigned long)val); break;
            case 'b': SprintBinary(xa,(unsigned long)val); break;
            case 'o': SprintOctet(xa, (unsigned long)val); break;
            case 'd': SprintSigned(xa,(long)val); break;
            case 'u': SprintUnsigned(xa,(unsigned long)val); break;
            //case 'p':
            default:
              SprintPointer(xa,val);
          }
      }

  template < class tXa >
    void CXX_STDCALL _FmtGUID( tXa &xa, GUID const *val, char fmt )
      {
        switch ( fmt )
          {
            case 'J':
              SprintGUID(xa,val,true); break;
            case 'j':
            case 's':
            default:
              SprintGUID(xa,val,false);
          }
      }

  template < class tXa >
    void CXX_STDCALL _FmtStringA( tXa &xa, char const *val, char fmt, int limit )
      {
        if ( fmt == 's' || fmt == '?' )
          SprintStringC(xa,val,limit);
        else
          _FmtPtr(xa,val,fmt);
      }

  template < class tXa >
    void CXX_STDCALL _FmtStringW( tXa &xa, wchar_t const *val, char fmt, int limit )
      {
        if ( fmt == 's' || fmt == '?' )
          SprintStringW(xa,val,limit);
        else
          _FmtPtr(xa,val,fmt);
      }

  template <class tXa> inline
    void CXX_STDCALL _FmtOutValue__( tXa &xa, unsigned perc, char fmt, FormatT<char> const *t )
      {
        switch ( t->t_ )
          {
            case FormatT<char>::xT: break;
            case FormatT<char>::xS: _FmtStringA(xa,t->sval_,fmt,perc); break;
            case FormatT<char>::xW: _FmtStringW(xa,t->wval_,fmt,perc); break;
            case FormatT<char>::xI: _FmtInt(xa,t->ival_,fmt,perc); break;
            case FormatT<char>::xU: _FmtUint(xa,t->uval_,fmt,perc); break;
            case FormatT<char>::xF: _FmtFloat(xa,t->fval_,fmt,perc); break;
            case FormatT<char>::xP: _FmtPtr(xa,t->pval_,fmt); break;
            case FormatT<char>::xJ: _FmtGUID(xa,t->gval_,fmt); break;
          }
      }

  _TEGGO_FORMAT_FAKE_INLINE
  void CXX_STDCALL _FmtOutValue( _XsprintAjuster<char> &xa, unsigned perc, char fmt, FormatT<char> const *t )
    {
      _FmtOutValue__(xa,perc,fmt,t);
    }

  _TEGGO_FORMAT_FAKE_INLINE
  void CXX_STDCALL _FmtOutValue( _XsprintAjuster<wchar_t> &xa, unsigned perc, wchar_t fmt, FormatT<char> const *t )
    {
      _FmtOutValue__(xa,perc,(char)fmt,t);
    }

  template < class tTc >
    struct _XsprintArglistTno : _XsprintArglist<tTc>
      {
        mutable FormatT<char> const **a_;
        virtual void OutValue( _XsprintAjuster<tTc> &xa, unsigned perc, tTc fmt, bool long_fmt = false ) const
          { if ( *a_ ) (*a_++)->_OutValue(xa,perc,fmt); }
        _XsprintArglistTno(FormatT<char> const **a) : a_(a) {}
        ~_XsprintArglistTno() {}
      };

  template < class tTc >
    void CXX_STDCALL FormatT_WriteFormat_( FormatT<char> const *fmt, GenericWriterT<tTc> &wr )
      {
        unsigned no = 0;
        FormatT<char> const *f = fmt;
        for ( ; f->ft_ ; f = f->ft_ ) ++no;
        FormatT<char> const **a = (FormatT<char> const **)alloca((no+1)*sizeof(FormatT<char>*));
        for ( unsigned i = 1; i <= no; ++i, fmt = fmt->ft_ ) a[no-i] = fmt;
        a[no] = 0;
        STRICT_REQUIRE(f->t_ == FormatT<char>::xT);
        teggo::VsprintF(wr,(tTc const*)f->sval_,_XsprintArglistTno<tTc>(a));
      }

  _TEGGO_FORMAT_FAKE_INLINE
  void CXX_STDCALL FormatT_WriteFormat( FormatT<char> const *fmt, GenericWriterT<char> &wr )
    {
      FormatT_WriteFormat_(fmt,wr);
    }

  _TEGGO_FORMAT_FAKE_INLINE
  void CXX_STDCALL FormatT_WriteFormat( FormatT<char> const *fmt, GenericWriterT<wchar_t> &wr )
    {
      FormatT_WriteFormat_(fmt,wr);
    }

  _TEGGO_FORMAT_FAKE_INLINE
  BaseStringT<char> & CXX_STDCALL operator <<(BaseStringT<char> &s, FormatT<char> v)
    { v.Write(s); return s; }

  _TEGGO_FORMAT_FAKE_INLINE
  BaseStringT<wchar_t> & CXX_STDCALL operator <<(BaseStringT<wchar_t> &s, FormatT<wchar_t> v)
    { v.Write(s); return s; }

  _TEGGO_FORMAT_FAKE_INLINE
  BaseStringT<char> & CXX_STDCALL operator <<(BaseStringT<char> &s, FormatT<wchar_t> v)
    { s.Append(v.Str()); return s; }

  _TEGGO_FORMAT_FAKE_INLINE
  BaseStringT<wchar_t> & CXX_STDCALL operator <<(BaseStringT<wchar_t> &s, FormatT<char> v)
    { s.Append(v.Str()); return s; }

  template<> _TEGGO_FORMAT_FAKE_INLINE
  BaseStringT<char> & BaseStringT<char>::operator +=(FormatT<char> const &v)
    { Append(v.Str()); return *this; }

  template<> _TEGGO_FORMAT_FAKE_INLINE
  BaseStringT<wchar_t> & BaseStringT<wchar_t>::operator +=(FormatT<wchar_t> const &v)
    { Append(v.Str()); return *this; }

  template<> _TEGGO_FORMAT_FAKE_INLINE
  BaseStringT<char> & BaseStringT<char>::operator +=(FormatT<wchar_t> const &v)
    { Append(v.Str()); return *this; }

  template<> _TEGGO_FORMAT_FAKE_INLINE
  BaseStringT<wchar_t> & BaseStringT<wchar_t>::operator +=(FormatT<char> const &v)
    { Append(v.Str()); return *this; }

  _TEGGO_FORMAT_FAKE_INLINE
  BufferT<char> & CXX_STDCALL operator <<(BufferT<char> &b, FormatT<char> v)
    { BufferTwriterT<char> wr(b); v.Write(wr); return b; }

  _TEGGO_FORMAT_FAKE_INLINE
  BufferT<wchar_t> & CXX_STDCALL operator <<(BufferT<wchar_t> &b, FormatT<wchar_t> v)
    { BufferTwriterT<wchar_t> wr(b); v.Write(wr); return b; }

  template<> _TEGGO_FORMAT_FAKE_INLINE
  BaseStringT<wchar_t>& BaseStringT<wchar_t>::operator =(FormatT<wchar_t> const&f)
//     { this->Resize(0); f.Write(*this); return *this; }
    { Set(f.Str()); return *this; }

  template<> _TEGGO_FORMAT_FAKE_INLINE
  BaseStringT<char>& BaseStringT<char>::operator =(FormatT<char> const&f)
//     { this->Resize(0); f.Write(*this); return *this; }
    { Set(f.Str()); return *this; }

  template<> _TEGGO_FORMAT_FAKE_INLINE
  BaseStringT<wchar_t>& BaseStringT<wchar_t>::operator =(FormatT<char> const&f)
    { Set(f.Str()); return *this; }

  template<> _TEGGO_FORMAT_FAKE_INLINE
  BaseStringT<char>& BaseStringT<char>::operator =(FormatT<wchar_t> const&f)
    { Set(f.Str()); return *this; }

} // namespace

  _TEGGO_FORMAT_FAKE_INLINE
  Teggo_LastError_<int> CXX_STDCALL operator *( Teggo_LastError_<int> const &, teggo::FormatT<char> const &f )
    {
      teggo::StringT<char> c;
      f.Write( c );
      teggo::StrCopyN(_E.reason,+c,sizeof(_E.reason)/sizeof(_E.reason[0])-1);
      return _E;
    }

  _TEGGO_FORMAT_FAKE_INLINE
  Teggo_LastError_<int> CXX_STDCALL operator *( Teggo_LastError_<int> const &, teggo::FormatT<wchar_t> const &f )
    {
      teggo::GenericWriterT<wchar_t> gw(_E.reason,_E.reason,_E.reason+sizeof(_E.reason)/sizeof(_E.reason[0])-1);
      f.Write( gw );
      return _E;
    }

namespace teggo {

  _TEGGO_FORMAT_FAKE_INLINE
    StringParam_::StringParam_(FormatT<wchar_t> const &f) : buffer_(new StringT<wchar_t>(f))
      {
        value_ = +*buffer_;
      }

  _TEGGO_FORMAT_FAKE_INLINE
    StringParam_::StringParam_(FormatT<char> const& f) : buffer_(new StringT<wchar_t>(f.Str()))
      {
        value_ = +*buffer_;
      }

#undef _TEGGO_FORMAT_FAKE_INLINE

#endif

} // namespace

enum _S_formatter_W { WCS };
enum _S_formatter_A { ASC };
enum _S_formatter { _S };

template<class tTchr> inline
  teggo::FormatT<tTchr> CXX_STDCALL operator *( _S_formatter, tTchr const* fmt)
    {return teggo::FormatT<tTchr>(fmt);}

inline teggo::FormatT<char> CXX_STDCALL operator *( _S_formatter, teggo::FormatT_<char> const &fmt)
  {return teggo::FormatT<char>(fmt.Str());}

inline teggo::FormatT<wchar_t> CXX_STDCALL operator *( _S_formatter, teggo::FormatT_<wchar_t> const &fmt)
  {return teggo::FormatT<wchar_t>(fmt.Str());}

inline pwide_t CXX_STDCALL operator |( int, teggo::FormatT128_<wchar_t> const &f128 ) { return f128.Str(); }
inline pwide_t CXX_STDCALL operator |( _S_formatter_W, teggo::FormatT128x_<wchar_t> const &f128 ) { return f128.Str(); }
inline pchar_t CXX_STDCALL operator |( int, teggo::FormatT128_<char> const &f128 ) { return f128.Str(); }
inline pchar_t CXX_STDCALL operator |( _S_formatter_A, teggo::FormatT128x_<char> const &f128 ) { return f128.Str(); }

#endif // __9682C09E_C256_4e85_8872_AD7597F6BFAC__
