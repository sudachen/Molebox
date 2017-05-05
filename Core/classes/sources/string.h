
/*

  Copyright (c) 2005-2009, Alexey Sudachen, alexey@sudachen.name

*/

#if !defined ___F176981E_A85C_470b_AAC5_D91BACD77192___
#define ___F176981E_A85C_470b_AAC5_D91BACD77192___

#include "_specific.h"

#if 0
#define TEGGO_USING_EXPORTABLE_DELETER
#endif

namespace teggo
{

#if !SYSTEM_IS_WINDOWS
  _TEGGO_EXPORTABLE int wcsnicmp (const wchar_t *cs, const wchar_t *ct, size_t count);
  _TEGGO_EXPORTABLE int stricmp(const char* cs,const char * ct);  
  _TEGGO_EXPORTABLE int wcsicmp(const wchar_t* cs,const wchar_t * ct);  
  _TEGGO_EXPORTABLE int strnicmp (const char *cs, const char *ct, size_t count);
#endif

  template <class tTchr> struct FormatT;

  inline void CXX_STDCALL ChrSet(wchar_t *p, wchar_t v) { *p = v; }
  inline void CXX_STDCALL ChrSet(wchar_t *p, char v) { *p = ucs2_btowc(v); }
  inline void CXX_STDCALL ChrSet(char *p, wchar_t v) { *p = ucs2_wctob(v); }
  inline void CXX_STDCALL ChrSet(char *p, char v) { *p = v; }

  template < class tTx, class tTy >
    inline int CXX_STDCALL StrCopyN_(tTx *to,tTy const *from,int len)
      {
        int i = 0;
        for ( ; i < len && *from; ++i, ++from ) ChrSet(to+i,*from);
        return i;
      }

  template < class tTx, class tTy >
    inline int CXX_STDCALL StrCopyN(tTx *to,tTy const *from,int len)
      {
        int i = StrCopyN_(to,from,len);
        to[i] = 0;
        return i;
      }

  inline wchar_t CXX_STDCALL ChrLower(wchar_t a) { return towlower(a); }
  inline char    CXX_STDCALL ChrLower(char a) { return tolower(a); }
  inline wchar_t CXX_STDCALL ChrUpper(wchar_t a) { return towupper(a); }
  inline char    CXX_STDCALL ChrUpper(char a) { return toupper(a); }
  inline bool    CXX_STDCALL ChrIsAlpha(char c)  { return 0 != isalpha(c); }
  inline bool    CXX_STDCALL ChrIsAlnum(char c)  { return 0 != isalnum(c); }
  inline bool    CXX_STDCALL ChrIsDigit(char c)  { return 0 != isdigit(c); }
  inline bool    CXX_STDCALL ChrIsXdigit(char c) { return 0 != isxdigit(c); }
  inline bool    CXX_STDCALL ChrIsUpper(char c) { return 0 != isupper(c); }
  inline bool    CXX_STDCALL ChrIsLower(char c) { return 0 != islower(c); }
  inline bool    CXX_STDCALL ChrIsAlpha(wchar_t c)  { return 0 != iswalpha(c); }
  inline bool    CXX_STDCALL ChrIsAlnum(wchar_t c)  { return 0 != iswalnum(c); }
  inline bool    CXX_STDCALL ChrIsDigit(wchar_t c)  { return 0 != iswdigit(c); }
  inline bool    CXX_STDCALL ChrIsXdigit(wchar_t c) { return 0 != iswxdigit(c); }
  inline bool    CXX_STDCALL ChrIsUpper(wchar_t c) { return 0 != iswupper(c); }
  inline bool    CXX_STDCALL ChrIsLower(wchar_t c) { return 0 != iswlower(c); }
  inline long    CXX_STDCALL StrAsLong(pwide_t p,unsigned radix = 10) { wchar_t *f; return wcstol(p,&f,radix); }
  inline long    CXX_STDCALL StrAsLong(pchar_t p,unsigned radix = 10) { char *f; return strtol(p,&f,radix); }
  inline double  CXX_STDCALL StrAsDouble(pwide_t p) { wchar_t *f; return wcstod(p,&f); }
  inline double  CXX_STDCALL StrAsDouble(pchar_t p) { char *f; return strtod(p,&f); }

  inline u32_t CXX_STDCALL StrLen(pchar_t val)
    {
      STRICT_REQUIRE( val != 0  );
      return (u32_t)strlen(val);
    }
  inline u32_t CXX_STDCALL StrLen(pwide_t val)
    {
      STRICT_REQUIRE( val != 0  );
      return (u32_t)wcslen(val);
    }
  inline int CXX_STDCALL StrCmpN(pwide_t a,pwide_t b, unsigned cnt)
    {
      STRICT_REQUIRE( a != 0 && b != 0 );
      return wcsncmp(a,b,cnt);
    }
  inline int CXX_STDCALL StrCmpN(pchar_t a,pchar_t b, unsigned cnt)
    {
      STRICT_REQUIRE( a != 0 && b != 0 );
      return strncmp(a,b,cnt);
    }
  inline int CXX_STDCALL StrCmpNi(pwide_t a,pwide_t b, unsigned cnt)
    {
      STRICT_REQUIRE( a != 0 && b != 0 );
      return wcsnicmp(a,b,cnt);
    }
  inline int CXX_STDCALL StrCmpNi(pchar_t a,pchar_t b, unsigned cnt)
    {
      STRICT_REQUIRE( a != 0 && b != 0 );
      return strnicmp(a,b,cnt);
    }
  inline int CXX_STDCALL StrCompare(pwide_t a,pwide_t b)
    {
      STRICT_REQUIRE( a != 0 && b != 0 );
      return wcscmp(a,b);
    }
  inline int CXX_STDCALL StrCompare(pchar_t a,pchar_t b)
    {
      STRICT_REQUIRE( a != 0 && b != 0 );
      return strcmp(a,b);
    }
  inline int CXX_STDCALL StrCompareI(pwide_t a,pwide_t b)
    {
      STRICT_REQUIRE( a != 0 && b != 0 );
      return wcsicmp(a,b);
    }
  inline int CXX_STDCALL StrCompareI(pchar_t a,pchar_t b)
    {
      STRICT_REQUIRE( a != 0 && b != 0 );
      return stricmp(a,b);
    }

  template < class tTchr > _TEGGO_EXPORTABLE int CXX_STDCALL StrFindChar(tTchr const *text,tTchr c,int from=0,int length=-1);
  template < class tTchr > _TEGGO_EXPORTABLE int CXX_STDCALL StrFindRChar(tTchr const *text, tTchr c,int from=-1,int length=-1);
  //template < class tTchr > _TEGGO_EXPORTABLE int CXX_STDCALL StrFind(tTchr const *text, tTchr const *s, int from=0);

  template < class tTx > inline
    bool CXX_STDCALL StrSafeEqual(tTx const *a,tTx const *b)
      { return a && b ? StrCompare(a,b) == 0 : false; }

  template < class tTx > inline
    bool CXX_STDCALL StrSafeEqualI(tTx const *a,tTx const *b)
      { return a && b ? StrCompare(a,b) == 0 : false; }

  template < class tTx > inline
    bool CXX_STDCALL StrSafeLess(tTx const *a,tTx const *b)
      { return a && b ? StrCompare(a,b) < 0 : a < b ; }

  template < class tTx > inline
    bool CXX_STDCALL StrSafeLessI(tTx const *a,tTx const *b)
      { return a && b ? StrCompareI(a,b) < 0 : a < b ; }

  template < class tTx > inline
    bool CXX_STDCALL StrSafeGreater(tTx const *a,tTx const *b)
      { return a && b ? StrCompare(a,b) > 0 : a > b ; }

  template < class tTx > inline
    bool CXX_STDCALL StrSafeGreaterI(tTx const *a,tTx const *b)
      { return a && b ? StrCompareI(a,b) > 0 : a > b ; }

  template < class tTx > inline
    bool CXX_STDCALL StrStartWith( tTx const *the, tTx const *what )
      {
        unsigned wl = StrLen( what );
        return ( wl > StrLen(the ) ) ? false : StrCmpN( the, what, wl ) == 0;
      }

  template < class tTx > inline
    bool CXX_STDCALL StrEndWith( tTx const *the, tTx const *what )
      {
        unsigned tl = StrLen(the);
        unsigned wl = StrLen( what );
        return ( wl > tl ) ? false : StrCmpN( the + (tl-wl), what, wl ) == 0;
      }

  template < class tTx > inline
    bool CXX_STDCALL StrStartWithI( tTx const *the, tTx const *what )
      {
        unsigned wl = StrLen( what );
        return ( wl > StrLen(the ) ) ? false : StrCmpNi( the, what, wl ) == 0;
      }

  template < class tTx > inline
    bool CXX_STDCALL StrEndWithI( tTx const *the, tTx const *what )
      {
        unsigned tl = StrLen(the);
        unsigned wl = StrLen( what );
        return ( wl > tl ) ? false : StrCmpNi( the + (tl-wl), what, wl ) == 0;
      }
      
  template<class tTchr>
  struct BaseStringT
  {
    typedef tTchr const *Tstr;
    typedef tTchr Tchar;
    static tTchr const* CXX_STDCALL SelectTstr(pchar_t aval,pwide_t wval);

    static bool CXX_STDCALL StartWith_( Tstr the, Tstr what ) { return StrStartWith(the,what); }
    static bool CXX_STDCALL EndsWith_( Tstr the, Tstr what ) { return StrEndWith(the,what); }
    static bool CXX_STDCALL StartWithI_( Tstr the, Tstr what ) { return StrStartWithI(the,what); }
    static bool CXX_STDCALL EndsWithI_( Tstr the, Tstr what ) { return StrEndWithI(the,what); }

    static u32_t MEM_BLOCK_ROUND(u32_t val) { return ( val + 4 - 1 ) & ~0x03UL; }

    #if defined TEGGO_USING_EXPORTABLE_DELETER
    typedef void (CXX_STDCALL *widestring_deleter_t)(byte_t *p);
    static void CXX_STDCALL widestring_deleter(byte_t *p) { delete[] p; }
    #endif

    static inline void CXX_STDCALL widestring_free(void *p)
      {
        #if defined TEGGO_USING_EXPORTABLE_DELETER
        if (p) (*((widestring_deleter_t*)p-1))((byte_t*)((widestring_deleter_t*)p-1));
        #else
        if ( p ) delete[] (byte_t*) p;
        #endif
      }

    static inline void* CXX_STDCALL widestring_malloc(unsigned sz)
      {
        #if defined TEGGO_USING_EXPORTABLE_DELETER
        byte_t *p = new byte_t[sz+sizeof(widestring_deleter_t)];
        *(widestring_deleter_t*)p = &widestring_deleter;
        return (widestring_deleter_t*)p+1;
        #else
        return new byte_t[sz];
        #endif
      }

    enum
      {
        char_tab_         = '\t',
        char_ln_          = '\n',
        char_cr_          = '\r',
        char_space_       = ' ',
        char_back_slash_  = '\\',
        char_slash_       = '/'
      };

    static bool IsWS_(tTchr c)
      {
        switch ( c )
          {
          case tTchr(char_space_):
          case tTchr(char_tab_):
          case tTchr(char_ln_):
          case tTchr(char_cr_):
            return true;
          default:
            return false;
          }
      }

    enum
      {
        SIZE_BITS_MASK      = 0xfffc,
        OWNER_BIT_MASK      = 0x0002,
        FIXED_BIT_MASK      = 0x0001
      };

    tTchr *s_;
    u16_t size_bits_length_[2];

    unsigned Size_() const    { return size_bits_length_[1] & SIZE_BITS_MASK; }
    unsigned Length() const   { return size_bits_length_[0]; }
    void SetSize_(u32_t s)  { size_bits_length_[1] = u16_t((size_bits_length_[1]&~SIZE_BITS_MASK) | (s & SIZE_BITS_MASK)); }
    void SetLength_(u32_t l){ size_bits_length_[0] = u16_t(l); }
    bool IsOwner_()           { return ( size_bits_length_[1] & OWNER_BIT_MASK) != 0; }

    _TEGGO_EXPORTABLE BaseStringT<tTchr> &FixCstr();
    _TEGGO_EXPORTABLE unsigned Resize(unsigned newlen);
    _TEGGO_EXPORTABLE unsigned ResizeBuffer(unsigned newlen);
    _TEGGO_EXPORTABLE bool Expand2();
    _TEGGO_EXPORTABLE BaseStringT& Expand(unsigned extent);
    unsigned BufferSize() { return Size_(); }

    //_TEGGO_EXPORTABLE 
    BaseStringT<tTchr>& EscMask()
      {
        for ( int i = 0; i < Length(); ++i )
          {
            if ( s_[i] == '\\' || s_[i] == '"' ) 
              {
                InsertChr(i,'\\'); ++i;
              }
            else if ( s_[i] == '\n' )
              {
                InsertChr(i,'\\'); ++i;
                s_[i] = 'n';
              }
          }
        return *this;
      }
    
    //operator Tstr() const { return Str(); }
    operator bool() const { return Length() != 0; }
    bool operator !() const { return Length() == 0; }

    _TEGGO_EXPORTABLE Tstr SetRange(wchar_t const *txt,int l);
    _TEGGO_EXPORTABLE Tstr SetRange(char const *txt,int l);

    template<class tTu>
      Tstr SetRange(tTu const *iS, tTu const *iE)
        { return SetRange(iS,iE-iS); }

    _TEGGO_EXPORTABLE BaseStringT<tTchr>& AppendRange(wchar_t const *val,int l = -1);
    _TEGGO_EXPORTABLE BaseStringT<tTchr>& AppendRange(char const *val,int l = -1);

    template<class tTu>
      BaseStringT<tTchr>& AppendRange(tTu const *iS, tTu const *iE)
        { return AppendRange(iS,iE-iS); }

    template<class tTu>
      Tstr Set(tTu const *s)
        { return SetRange(s,-1); }

    Tstr Set(FormatT<tTchr> const &s)
      {
        //s_[0] = 0;
        //SetLength_(0);
        return s.Write(*this);
      }

    template<class tTu>
      BaseStringT& Sset(tTu const *s)
        { Set(s); return *this; }

    template<class tTu>
      Tstr Set(BaseStringT<tTu> const &x)
        { return SetRange( x.s_, x.Length() ); }

    template<class tTu>
      BaseStringT&  Sset(BaseStringT<tTu> const &x) { Sset(x); return *this; }

    bool StartWith( Tstr what ) const { return StartWith_( Str(), what ); }
    bool StartsWith( Tstr what ) const { return StartWith_( Str(), what ); }
    bool EndsWith( Tstr what ) const { return EndsWith_( Str(), what ); }
    bool StartWithI( Tstr what ) const { return StartWithI_( Str(), what ); }
    bool StartsWithI( Tstr what ) const { return StartWithI_( Str(), what ); }
    bool EndsWithI( Tstr what ) const { return EndsWithI_( Str(), what ); }

    static int CXX_STDCALL StripCR2( tTchr *text, int len = -1 );

    BaseStringT& StripCR()
      {
        int newlen = StripCR2( s_, Length() );
        Resize( newlen );
        return *this;
      }

    _TEGGO_EXPORTABLE BaseStringT& RestoreCRLF();

    template<class tTu>
      BaseStringT& Append(tTu const *pS, tTu const *pE)
        { return AppendRange(pS,pE-pS); }

    BaseStringT& Append(pchar_t txt) { return AppendRange(txt,-1); }
    BaseStringT& Append(pwide_t txt) { return AppendRange(txt,-1); }
    BaseStringT& Append(char const c) { return AppendRange(&c,1); }
    BaseStringT& Append(wchar_t const c) { return AppendRange(&c,1); }

    template<class tTu>
      BaseStringT& Append(BaseStringT<tTu> const &txt)
        { return AppendRange(txt.Str(),txt.Length()); }

    BaseStringT& operator +=(pchar_t txt) { return Append(txt); }
    BaseStringT& operator +=(pwide_t txt) { return Append(txt); }
    BaseStringT& operator +=(FormatT<char> const &f);
    BaseStringT& operator +=(FormatT<wchar_t> const &f);
    _TEGGO_EXPORTABLE BaseStringT& operator =(FormatT<char> const &f);
    _TEGGO_EXPORTABLE BaseStringT& operator =(FormatT<wchar_t> const &f);
    BaseStringT& operator =(BaseStringT<char> const &a) { Set(+a); return *this; }
    BaseStringT& operator =(BaseStringT<wchar_t> const &a) { Set(+a); return *this; }

    template < class tTx >
      BaseStringT& operator +=(BaseStringT<tTx> const &p) { return Append(p); }

    Tstr Str() const { return s_; }
    Tstr operator +() const { return s_; }

    long ToLong(unsigned base = 10) { return StrAsLong(s_,base); }
    double ToDouble() { return StrAsDouble(s_); }

    _TEGGO_EXPORTABLE int FindChar(tTchr c,int from=0) const;
    _TEGGO_EXPORTABLE int FindRChar(tTchr c,int from=-1) const;
    //_TEGGO_EXPORTABLE int Find(tTchr const *s, int from=0) const;

    _TEGGO_EXPORTABLE void RemoveReps(tTchr pat);

    tTchr& operator[](int idx)
      {
        STRICT_VIOLATION( idx < 0 || unsigned(idx) >= Length(),
          L"'StringT<tTchr>::operator[](idx)' error: out of range" );
        return s_[idx];
      }

    tTchr const& operator[](int idx) const
      {
        STRICT_VIOLATION( idx < 0 || unsigned(idx) >= Length(),
          L"'StringT<tTchr>::operator[](idx) const' error: out of range" );
        return s_[idx];
      }

    int Cmp( Tstr s ) { return StrCompare(s_,s); }

    bool Equal(Tstr text) const { return StrCompare(s_,text) == 0; }
    bool EqualI(Tstr text) const { return StrCompareI(s_,text) == 0; }

    //struct OptimalLessP : std::binary_function<BaseStringT<tTchr>,BaseStringT<tTchr>,bool>
    bool OptimalLess(const BaseStringT &a) const
      {
        const unsigned len = Length();
        const unsigned alen = a.Length();
        return len < alen || ( len == alen && memcmp(Str(),a.Str(),len*sizeof(tTchr)) < 0 );
      }

    struct OptimalLessP
      {
        bool operator() (const BaseStringT<tTchr> &a,const BaseStringT<tTchr> &b) const
          { return a.OptimalLess(b); }
      };

    _TEGGO_EXPORTABLE BaseStringT& ToLower();
    _TEGGO_EXPORTABLE BaseStringT& ToUpper();
    _TEGGO_EXPORTABLE BaseStringT& LeftTrim();
    _TEGGO_EXPORTABLE BaseStringT& RightTrim();
    BaseStringT& Trim() { return RightTrim().LeftTrim(); }
    _TEGGO_EXPORTABLE BaseStringT& Chomp(int count = 1);
    _TEGGO_EXPORTABLE BaseStringT& RemoveExtraWS();
    _TEGGO_EXPORTABLE BaseStringT& Replace(Tstr from,tTchr to);
    _TEGGO_EXPORTABLE BaseStringT& PosixPath();
    _TEGGO_EXPORTABLE BaseStringT& ReplaceAfter(tTchr after, Tstr data);
    _TEGGO_EXPORTABLE BaseStringT& ReplaceRight(unsigned after, Tstr data);
    _TEGGO_EXPORTABLE BaseStringT& Insert(long idx, Tstr data, int count = -1);
    _TEGGO_EXPORTABLE BaseStringT& InsertChr(long idx, Tchar data);
    _TEGGO_EXPORTABLE BaseStringT& Remove(long idx, long count = -1);

    _TEGGO_EXPORTABLE Tstr Left(int count,BaseStringT& result) const;
    _TEGGO_EXPORTABLE Tstr Right(int count,BaseStringT& result) const;
    _TEGGO_EXPORTABLE Tstr Substr(int first,int count,BaseStringT& result) const;

    tTchr *Begin() const { return s_; }
    tTchr *End()   const
      {
        return s_+Length();
      }
    unsigned Count() const { return Length(); }
    tTchr Last() const { return Length()?s_[Length()-1]:tTchr(0); }
    bool  Chop(tTchr c = tTchr(0))
      {
        if ( Length() && ( !c || c == s_[Length()-1] ))
          { Resize(Length()-1); return true; }
        return false;
      }

    // std::string compatibility
    //Tstr c_str() const { return Str(); }
    //tTchr* begin() const { return s_; }
    //tTchr* end()   const { return s_+Length(); }
    //tTchr& front() const { return *begin(); }
    //tTchr& back()  const { return *end(); }
    //unsigned size() const { return Length(); }
    // ----------------

    int Search( int idx, tTchr const *patt, int pattL, bool casesense = true ) const
      {
        if ( idx < 0 || idx >= Length() ) return -1;
        if ( !patt ) return -1;
        if ( pattL < 0 ) pattL = StrLen(patt);
        if ( !pattL ) return -1;
        if ( idx+pattL > Length() ) return -1;
        
        for ( tTchr *p = s_+idx, *pE = s_+Length()-pattL+1; p<pE; ++p )
          if ( *p == *patt )
            {
              int i = 0;
              for ( ; i < pattL; ++i )
                if ( casesense )
                  { if ( patt[i] != p[i] ) break; }
                else
                  { if ( ChrUpper(patt[i]) != ChrUpper(p[i]) ) break; }
              if (i == pattL) return p-s_;
            }
        return -1;
      }

    int Search( tTchr const *patt, bool casesense = true ) const{ return Search(0,patt,-1,true); }
    int SearchS( tTchr const *patt, int pattL = -1 ) const { return Search(0,patt,pattL,true); }
    int SearchI( tTchr const *patt, int pattL = -1 ) const { return Search(0,patt,pattL,false); }
        
    void ReplaceEx( int idx, tTchr const *patt, int pattL, tTchr const *val, int valL, bool casesense)
      {
        if ( pattL < 0 )
          pattL = StrLen(patt);
        if ( valL < 0 )
          valL = StrLen(val);
        for ( int p = Search(idx,patt,pattL,casesense); p >= 0; p = Search(p+valL,patt,pattL,casesense) )
          {
            Remove(p,pattL);
            Insert(p,val,valL);
          }
      }

    void ReplaceEx( tTchr const *patt, tTchr const *val, bool casesense )  { ReplaceEx(0, patt, -1, val, -1, casesense); }    
    void ReplaceS( tTchr const *patt, tTchr const *val,int idx = 0)  { ReplaceEx(idx, patt, -1, val, -1, true); }    
    void ReplaceI( tTchr const *patt, tTchr const *val,int idx = 0)  { ReplaceEx(idx, patt, -1, val, -1, false); }
      
  //protected:
    _TEGGO_EXPORTABLE void InitValue(unsigned R,resize_policy_t rpt,char const *value,int len);
    _TEGGO_EXPORTABLE void InitValue(unsigned R,resize_policy_t rpt,wchar_t const *value,int len);
    _TEGGO_EXPORTABLE void InitReuse(unsigned BufSz,resize_policy_t,void *buf);
    _TEGGO_EXPORTABLE void Init(unsigned R,resize_policy_t);
    _TEGGO_EXPORTABLE void Term();
  protected:
    ~BaseStringT() {}
    BaseStringT() {}
    _TEGGO_EXPORTABLE BaseStringT(const BaseStringT&);
//     void operator =(const BaseStringT&);
  };

  template<> inline pwide_t CXX_STDCALL BaseStringT<wchar_t>::SelectTstr(pchar_t,pwide_t wval) { return wval; }
  template<> inline pchar_t CXX_STDCALL BaseStringT<char>::SelectTstr(pchar_t aval,pwide_t) { return aval; }

  typedef BaseStringT<wchar_t> widestring_basic;
  typedef BaseStringT<char>    asciistring_basic;

  template<class tTchr> inline
    bool CXX_STDCALL operator ==(BaseStringT<tTchr> const &a,BaseStringT<tTchr> const &b)
      { return a.Length() == b.Length() && StrCompare(a.Str(),b.Str()) == 0; }

  template<class tTchr> inline
    bool CXX_STDCALL operator ==(BaseStringT<tTchr> const &a,tTchr const *b)
      { return StrCompare(a.Str(),b) == 0; }

  template<class tTchr> inline
    bool CXX_STDCALL operator ==(tTchr const *a, BaseStringT<tTchr> const &b)
      { return StrCompare(a,b.Str()) == 0; }

  template<class tTchr> inline
    bool CXX_STDCALL operator !=(BaseStringT<tTchr> const &a, BaseStringT<tTchr> const &b)
      { return !operator ==(a,b); }

  template<class tTchr> inline
    bool CXX_STDCALL operator !=(BaseStringT<tTchr> const &a,tTchr const *b)
      { return !operator ==(a,b); }

  template<class tTchr> inline
    bool CXX_STDCALL operator !=(tTchr const *a, BaseStringT<tTchr> const &b)
      { return !operator ==(a,b); }

  template<class tTchr> inline
    bool CXX_STDCALL operator<(BaseStringT<tTchr> const &a,BaseStringT<tTchr> const &b)
      { return StrCompare(a.Str(),b.Str()) < 0; }

  template<class tTchr> inline
    bool CXX_STDCALL operator <(BaseStringT<tTchr> const &a,tTchr const *b)
      { return StrCompare(a.Str(),b) < 0; }

  template<class tTchr> inline
    bool CXX_STDCALL operator <(tTchr const *a, BaseStringT<tTchr> const &b)
      { return StrCompare(a,b.Str()) < 0; }

  template<class tTchr> inline
    bool CXX_STDCALL operator <=(BaseStringT<tTchr> const &a,BaseStringT<tTchr> const &b)
      { return StrCompare(a.Str(),b.Str()) <= 0; }

  template<class tTchr> inline
    bool operator <=(BaseStringT<tTchr> const &a,tTchr const *b)
      { return StrCompare(a.Str(),b) <= 0; }

  template<class tTchr> inline
    bool CXX_STDCALL operator <=(tTchr const *a, BaseStringT<tTchr> const &b)
      { return StrCompare(a,b.Str()) <= 0; }

  template<class tTchr> inline
    bool CXX_STDCALL operator >(BaseStringT<tTchr> const &a,BaseStringT<tTchr> const &b)
      { return !operator <=(a,b); }

  template<class tTchr> inline
    bool CXX_STDCALL operator >(BaseStringT<tTchr> const &a,tTchr const *b)
      { return !operator <=(a,b); }

  template<class tTchr> inline
    bool CXX_STDCALL operator >(tTchr const *a, BaseStringT<tTchr> const &b)
      { return !operator <=(a,b); }

  template<class tTchr> inline
    bool CXX_STDCALL operator >=(BaseStringT<tTchr> const &a,BaseStringT<tTchr> const &b)
      { return !operator <(a,b); }

  template<class tTchr> inline
    bool CXX_STDCALL operator >=(BaseStringT<tTchr> const &a,tTchr const *b)
      { return !operator <(a,b); }

  template<class tTchr> inline
    bool CXX_STDCALL operator >=(tTchr const *a, BaseStringT<tTchr> const &b)
      { return !operator <(a,b); }

  template < class _Tchar, bool __nocsence = false, bool __safe = true >
    struct TstrComparer
      {
        bool operator() ( _Tchar const *a, _Tchar const *b) const
          {
            if ( __safe )
              if ( __nocsence )
                return StrSafeLessI(a,b);
              else
                return StrSafeLess(a,b);
            else
              if ( __nocsence )
                return StrCompareI(a,b) < 0;
              else
                return StrCompare(a,b) < 0;
          }
      };

  template < class _Tchar > struct TstrComparerI : TstrComparer< _Tchar, true > {};

  enum lowercopy_tag_t { LOWERCOPY };
  enum trimlower_tag_t { LOWERTRIMMED };
  enum trimmed_tag_t { TRIMMED };
  enum concat_tag_t { CONCATSTRINGS };

  #if !defined __MICROSTRING_PRESERVE
  #define __MICROSTRING_PRESERVE 4
  #endif

  template<class tTchr,unsigned tPRESERVE>
  struct StringHeadT : BaseStringT<tTchr>
    {
      enum
        {
          R = tPRESERVE>__MICROSTRING_PRESERVE
          ?(tPRESERVE<16*1024?tPRESERVE:16*1024)
          :__MICROSTRING_PRESERVE
        };
      tTchr static_buffer_[R+1];
    };
  template<> struct StringHeadT<char,0>    : BaseStringT<char>{ enum { R = 0 }; };
  template<> struct StringHeadT<wchar_t,0> : BaseStringT<wchar_t>{ enum { R = 0 }; };
  
  struct StringParam_;

  template <class tTchr,unsigned tPRESERVE=0, resize_policy_t tRESIZE_POLICY=RESIZABLE>
  struct StringT : StringHeadT<tTchr,tPRESERVE>
  {
    enum { R = StringHeadT<tTchr,tPRESERVE>::R };

    //using StringHeadT<tTchr,tPRESERVE>::s_;

    //unsigned Length() { return BaseStringT<tTchr>::Length(); }
    using BaseStringT<tTchr>::Length;

    StringT() { BaseStringT<tTchr>::InitValue(R,tRESIZE_POLICY,(tTchr const*)0,0);  }
    template <class tU>
		StringT(const tU *value) { BaseStringT<tTchr>::InitValue(R,tRESIZE_POLICY,value,-1); }
    StringT(const tTchr *value) { BaseStringT<tTchr>::InitValue(R,tRESIZE_POLICY,value,-1); }
    StringT(reuse_policy_t,void *buf,unsigned bufsz) { BaseStringT<tTchr>::InitReuse(bufsz, tRESIZE_POLICY, buf); }
    ~StringT(){ BaseStringT<tTchr>::Term(); }

    explicit StringT(int length)
      { BaseStringT<tTchr>::InitValue(R,tRESIZE_POLICY,(tTchr const*)0,length); }

    template<class tTu>
      /*explicit*/ StringT(BaseStringT<tTu> const &value)
        { BaseStringT<tTchr>::InitValue(R,tRESIZE_POLICY,value.Str(),value.Length()); }

	  template<class tTu>
      StringT& operator=(tTu const *val)
        { BaseStringT<tTchr>::Set(val); return *this; }

    //  StringT& operator=(char const *val)
    //    { BaseStringT<tTchr>::Set(val); return *this; }
      StringT& operator=(tTchr const *val)
        { BaseStringT<tTchr>::Set(val); return *this; }

	template<class tTu>
      StringT& operator=(BaseStringT<tTu> const &val)
        { BaseStringT<tTchr>::Set(val.Str()); return *this; }

    StringT<tTchr,0> Substr(int first,int len) const
      { StringT<tTchr,0> result; BaseStringT<tTchr>::Substr(first,len,result); return result; }

    StringT<tTchr,0> Left(int count) const
      { StringT<tTchr,0> result; BaseStringT<tTchr>::Left(count,result); return result; }

    StringT<tTchr,0> Right(int count) const
      { StringT<tTchr,0> result; BaseStringT<tTchr>::Right(count,result); return result; }

    StringT<tTchr,0> CXX_FAKE_INLINE Till(int first,tTchr c) const
      { 
        if ( first < Length() )
          {
            StringT<tTchr,0> result; 
            int l = BaseStringT<tTchr>::FindChar(c,first);
            if ( l < 0 ) 
              l = Length();
            if ( l )
              BaseStringT<tTchr>::Substr(first,l-first,result); 
            return result; 
          }
        return StringT<tTchr,0>();
      }

    StringT<tTchr,0> CXX_FAKE_INLINE TillAndTrim(tTchr c,bool withmark) 
      { 
        StringT<tTchr,0> result; 
        int l = BaseStringT<tTchr>::FindChar(c);
        if ( l < 0 ) 
          l = Length();
        if ( l )
            BaseStringT<tTchr>::Substr(0,l,result); 
        if ( l || withmark )
            BaseStringT<tTchr>::Remove(0,l+((withmark&&l<Length())?1:0));
        return result; 
      }

	const StringT& operator=(StringParam_ const &val);

    const StringT& operator=(StringT<tTchr> const &val)
      { BaseStringT<tTchr>::Set(val.Str()); return *this; }

    const StringT& operator=(FormatT<char> const &val)
      { BaseStringT<tTchr>::operator=(val); return *this; }

    const StringT& operator=(FormatT<wchar_t> const &val)
      { BaseStringT<tTchr>::operator=(val); return *this; }

    StringT(const StringT<tTchr,tPRESERVE,tRESIZE_POLICY> &value)
      { BaseStringT<tTchr>::InitValue(R,tRESIZE_POLICY,value.Str(),value.Length()); }

    StringT(StringParam_ const &);

	CXX_NO_INLINE
    StringT(trimlower_tag_t, tTchr const *value)
      {
        BaseStringT<tTchr>::InitValue(R,tRESIZE_POLICY,value,-1);
        BaseStringT<tTchr>::ToLower().Trim();
      }

    CXX_NO_INLINE
    StringT(lowercopy_tag_t, tTchr const *value)
      {
        BaseStringT<tTchr>::InitValue(R,tRESIZE_POLICY,value,-1);
        BaseStringT<tTchr>::ToLower();
      }

    CXX_NO_INLINE
    StringT(trimmed_tag_t, tTchr const *value)
      {
        BaseStringT<tTchr>::InitValue(R,tRESIZE_POLICY,value,-1);
        BaseStringT<tTchr>::Trim();
      }

    CXX_NO_INLINE
    StringT(concat_tag_t, tTchr const *value1,tTchr const *value2)
      {
        unsigned len1 = StrLen(value1);
        unsigned len2 = StrLen(value2);
        BaseStringT<tTchr>::InitValue(R,tRESIZE_POLICY,(tTchr*)0,len1+len2);
        memcpy(BaseStringT<tTchr>::s_,value1,sizeof(tTchr)*len1);
        memcpy(BaseStringT<tTchr>::s_+len1,value2,sizeof(tTchr)*len2);
      }

    CXX_NO_INLINE
    StringT(concat_tag_t, wchar_t const *value1, char const *value2)
      {
        BaseStringT<tTchr>::InitValue(R,tRESIZE_POLICY,value1,-1);
        *this += value2;
      }
      
    CXX_NO_INLINE
    StringT(concat_tag_t, char const *value1, wchar_t const *value2)
      {
        BaseStringT<tTchr>::InitValue(R,tRESIZE_POLICY,value1,-1);
        *this += value2;
      }

    StringT(pchar_t pS, pchar_t pE)
      {
        BaseStringT<tTchr>::InitValue(R,tRESIZE_POLICY,pS,pE-pS);
      }

    StringT(pwide_t pS, pwide_t pE)
      {
        BaseStringT<tTchr>::InitValue(R,tRESIZE_POLICY,pS,pE-pS);
      }

    StringT(pchar_t pS, int l)
      {
        BaseStringT<tTchr>::InitValue(R,tRESIZE_POLICY,pS,l);
      }

    StringT(pwide_t pS, int l)
      {
        BaseStringT<tTchr>::InitValue(R,tRESIZE_POLICY,pS,l);
      }

    StringT( FormatT<tTchr> const& );

  };

  typedef StringT<char> StringA;
  typedef StringT<wchar_t> StringW;

  template < class tTchr >
    StringT<tTchr> CXX_STDCALL operator +( BaseStringT<tTchr> const &a, wchar_t const *b )
      {
        return StringT<tTchr>(CONCATSTRINGS,+a,b);
      }

  template < class tTchr >
    StringT<tTchr> CXX_STDCALL operator +( BaseStringT<tTchr> const &a, char const *b )
      {
        return StringT<tTchr>(CONCATSTRINGS,+a,b);
      }

  template < class tTchr >
    StringT<tTchr> CXX_STDCALL operator +( tTchr const *a, BaseStringT<tTchr> const  &b)
      {
        return StringT<tTchr>(CONCATSTRINGS,a,+b);
      }

  template < class tTchr >
    StringT<tTchr> CXX_STDCALL operator +( BaseStringT<tTchr> const &a, BaseStringT<tTchr> const &b )
      {
        return StringT<tTchr>(CONCATSTRINGS,+a,+b);
      }

  typedef StringT<wchar_t,0,RESIZABLE> widestring;
  typedef StringT<char   ,0,RESIZABLE> asciistring;

  template <class tTchr>
    StringT<tTchr> CXX_STDCALL StrLower(tTchr const *str)
      {
        return StringT<tTchr>(LOWERCOPY,str);
      }

  template <class tTchr>
    StringT<tTchr> CXX_STDCALL StrTrimLower(tTchr const *str)
      {
        return StringT<tTchr>(LOWERTRIMMED,str);
      }

  template <class tTchr>
    StringT<tTchr> CXX_STDCALL StrTrimLower(BaseStringT<tTchr> const &str)
      {
        return StringT<tTchr>(LOWERTRIMMED,str);
      }

  template <class tTchr>
    StringT<tTchr> CXX_STDCALL StrTrim(tTchr const *str)
      {
        return StringT<tTchr>(TRIMMED,str);
      }

  template <class tTchr>
    StringT<tTchr> CXX_STDCALL StrTrim(BaseStringT<tTchr> const &str)
      {
        return StringT<tTchr>(TRIMMED,str);
      }

  template <class tTchr> inline
    StringT<tTchr> & CXX_STDCALL operator <<(StringT<tTchr> &s, pwide_t v) {s += v;return s;}
  template <class tTchr> inline
    StringT<tTchr> & CXX_STDCALL operator <<(StringT<tTchr> &s, pchar_t v) {s += v;return s;}
  template <class tTchr> inline
    StringT<tTchr> & CXX_STDCALL operator <<(StringT<tTchr> &s, char v) {s.Append(v);return s;}
  template <class tTchr> inline
    StringT<tTchr> & CXX_STDCALL operator <<(StringT<tTchr> &s, wchar_t v) {s.Append(v);return s;}


  template < class _Tstr > inline
    long CXX_STDCALL ToLong(_Tstr p, unsigned base = 10)
      {
        return ( !p ) ? 0 : StrAsLong(p,base);
      }

  template < class _Tstr > inline
    long CXX_STDCALL ToLong(BaseStringT<_Tstr> const &p, unsigned base = 10)
      {
        return StrAsLong(p.Str(),base);
      }

  template < class _Tstr > inline
    float CXX_STDCALL ToFloat(_Tstr p)
      {
        return ( !p ) ? 0.0f : float(StrAsDouble(p));
      }

  template < class _Tstr > inline
    float CXX_STDCALL ToFloat(BaseStringT<_Tstr> const &p, unsigned base = 10)
      {
        return float(StrAsDouble(p.Str(),base));
      }

  _TEGGO_EXPORTABLE bool CXX_STDCALL ToBool__(BaseStringT<wchar_t> const &);

  template < class _Tstr >
    bool CXX_STDCALL ToBool(_Tstr pp)
      {
        return ToBool__(StringT<wchar_t,32>(LOWERTRIMMED,pp));
      }

  template < class _Tstr >
    bool CXX_STDCALL ToBool(BaseStringT<_Tstr> const &pp)
      {
        return ToBool__(StringT<wchar_t,32>(LOWERTRIMMED,pp.Str()));
      }

  _TEGGO_EXPORTABLE void CXX_STDCALL InplaceWideToChar(void* wp,int charcount = -1,bool zterminate = true);
  _TEGGO_EXPORTABLE void CXX_STDCALL InplaceCharToWide(void* wp,int charcount = -1,bool zterminate = true);
  _TEGGO_EXPORTABLE wchar_t* CXX_STDCALL widestrdup(pwide_t val);
  _TEGGO_EXPORTABLE wchar_t* CXX_STDCALL widestrdup(pchar_t val);

  template < class tTc > inline
    tTc *ChrFind(tTc *i, int c)
      {
        while ( *i && *i != tTc(c) ) ++i;
        return i;
      }

  template < class tTc > inline
    tTc *ChrFind(tTc *i, int c, int O)
      {
        while ( *i && *i != O && *i != tTc(c) ) ++i;
        return i;
      }

  struct StringParam_
    {
      pwide_t value_;
      StringT<wchar_t> *buffer_;
      StringParam_(pwide_t f) : value_(f), buffer_(0) {}
      StringParam_(BaseStringT<wchar_t> const &f) : value_(+f), buffer_(0) {}
      _TEGGO_EXPORTABLE StringParam_(pchar_t f);
      _TEGGO_EXPORTABLE StringParam_(BaseStringT<char> const &f);
      _TEGGO_EXPORTABLE StringParam_(FormatT<wchar_t> const &f);
      _TEGGO_EXPORTABLE StringParam_(FormatT<char> const &f);
      ~StringParam_() { if (buffer_) _DeletBuffer(buffer_); }
      pwide_t operator +() const { return value_; }
      pwide_t Str() const { return value_; }
      operator pwide_t() const { return value_; }
      operator StringT<wchar_t>() const { return StringT<wchar_t>(value_); }
      void CopyTo(wchar_t *p, int count) const { StrCopyN(p,value_,count); }
      void CopyTo(char    *p, int count) const { StrCopyN(p,value_,count); }
      unsigned Length() const { return wcslen(value_); }  
      operator bool() { return value_ && Length(); }
      bool operator !() { return !(value_ && Length()); }
    private:
      _TEGGO_EXPORTABLE static void CXX_STDCALL _DeletBuffer(StringT<wchar_t> *);
      StringParam_(StringParam_ const&);
      StringParam_& operator=(StringParam_ const&);
    };

  template <class tTchr,unsigned tPRESERVE, resize_policy_t tRESIZE_POLICY> 
	const StringT<tTchr,tPRESERVE,tRESIZE_POLICY>& StringT<tTchr,tPRESERVE,tRESIZE_POLICY>
	::operator=(StringParam_ const &val)
	{
		BaseStringT<tTchr>::Set(val.Str()); return *this;
	}

  template <class tTchr,unsigned tPRESERVE, resize_policy_t tRESIZE_POLICY> 
    StringT<tTchr,tPRESERVE,tRESIZE_POLICY>::StringT(StringParam_ const &value)
      { BaseStringT<tTchr>::InitValue(R,tRESIZE_POLICY,value.Str(),value.Length()); }

  typedef StringParam_ const &StringParam;
  inline void CXX_STDCALL StringParamCopy(StringParam_ const& _,wchar_t * b,int l) { _.CopyTo(b,l); }

  _TEGGO_EXPORTABLE int CXX_STDCALL UtfGetCodeLength(int c);
  _TEGGO_EXPORTABLE char * CXX_STDCALL UtfEncode( char* bf, wchar_t c, int *cnt );
  _TEGGO_EXPORTABLE StringA CXX_STDCALL Utf8StringW(pwide_t p,int L=-1);
  _TEGGO_EXPORTABLE StringA CXX_STDCALL Utf8String(StringParam p);
  _TEGGO_EXPORTABLE StringW WideStringFromUtf8(pchar_t text,int strl=-1);
  
  template<class tTchr> tTchr const *StrSelect(tTchr *, pchar_t aval, pwide_t wval) { BaseStringT<tTchr>::SelectTstr(aval,wval); }
      
} // namespace teggo

#if defined _TEGGOINLINE
#include "string.inl"
#endif

#endif //  ___F176981E_A85C_470b_AAC5_D91BACD77192___
