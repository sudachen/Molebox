
/*

  Copyright (c) 2005-2009, Alexey Sudachen, alexey@sudachen.name

*/

#ifndef ___86151b2f_4259_42d8_8d23_22c08abf37e5___
#define ___86151b2f_4259_42d8_8d23_22c08abf37e5___

#include "_specific.h"
#include "format.h"

namespace teggo {

  struct ErrorFilter
    {
      bool operator() (const char *m,int e= -1) const { return Ascii(m,e); }
      bool operator() (const wchar_t *m, int e = -1) const { return Unicode(m,e); }

      template <class Tchr>
        bool operator() (FormatT<Tchr> const &f, int e=-1) const { return operator() (f.Str(),e); }

    protected:
      ErrorFilter() {}

      virtual bool Ascii(const char * message, int e) const = 0;
        //{
        //  return Unicode(StringW(message).Str(),e);
        //}
      virtual bool Unicode(const wchar_t * message, int e) const
        {
          return Ascii(StringA(message).Str(),e);
        }
    };

  typedef ErrorFilter const &EhFilter;

  template < class Tchr >
    struct SbufOutEfT : ErrorFilter
      {
        virtual bool Ascii(const char * message, int e) const
          {
            b_.Set(message);
            return !e;
          }
        virtual bool Unicode(const wchar_t * message, int e) const
          {
            b_.Set(message);
            return !e;
          }
        BaseStringT<Tchr> &b_;
        SbufOutEfT(BaseStringT<Tchr> &b) : b_(b) {}
      };

  template < class Tchr >
    struct StringEfT : SbufOutEfT<Tchr>
      {
        StringT<Tchr> b_;
        StringEfT() : SbufOutEfT<Tchr>(b_) {}
        Tchr const *Str() { return b_.Str(); }
        Tchr const *operator +() { return b_.Str(); }
      };

  typedef StringEfT<wchar_t> StringEfW;
  typedef StringEfT<char> StringEfA;
  typedef SbufOutEfT<wchar_t> SbufOutEfW;
  typedef SbufOutEfT<char> SbufOutEfA;

  enum XFILE { xout, xerr, xoutln = xout|4, xerrln, 
                xout8 = xout|8, 
                xerr8 = xerr|8, 
                xout8ln = xoutln|8, 
                xerr8ln = xerrln|8, 
                };

  //void Print(XFILE f,char const *s);
  //void Print(XFILE f,wchar_t const *s);

  CXX_FAKE_INLINE void Print(XFILE f,char const *s)
    {
      FILE *fd; switch (f&3) { case xout: fd=stdout; break; case xerr: fd=stderr; }
      if ( f&8 )
        fputs(+Utf8String(s),fd);
      else      
        fputs(s,fd);
      if ( f&4 ) fputc('\n',fd);
    }

  CXX_FAKE_INLINE void Print(XFILE f,wchar_t const *w)
    {
      FILE *fd; switch (f&3) { case xout: fd=stdout; break; case xerr: fd=stderr; }
      if ( f&8 )
        fputs(+Utf8String(w),fd);
      else      
        fputws(w,fd);
      if ( f&4 ) fputc('\n',fd);
    }

  template < class Tchr>
    void operator |(XFILE f,FormatT<Tchr> const &b)
      {
        Print(f,b.Str());
      }

  template < class Tchr>
    void operator |(XFILE f, Tchr const * s)
      {
        Print(f,s);
      }

  template < class Tchr>
    void operator |(XFILE f,BaseStringT<Tchr> const &b)
      {
        Print(f,b.Str());
      }

  struct EhPrintOut : ErrorFilter
    {
      virtual bool Ascii(const char * message, int e) const
        {
          if ( e )
            xerr|_S*"error: %s\n" % message;
          else
            xerr|_S*"warning: %s\n" % message;
          return !e;
        }
    };

  struct Ehxoutln : ErrorFilter
    {
      virtual bool Ascii(const char * message, int e) const
        {
          xerrln|message;
          return !e;
        }
    };

  typedef Ehxoutln PrintEf;
  PrintEf const PRINTEF = PrintEf();

  struct EmptyEf : ErrorFilter
    {
      virtual bool Ascii(const char * message, int e) const
        {
          return !e;
        }
    };

  EmptyEf const EMPTYEF = EmptyEf();

} // namespace

#endif /*___86151b2f_4259_42d8_8d23_22c08abf37e5___*/
