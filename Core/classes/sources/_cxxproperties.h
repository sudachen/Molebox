
/*

  Copyright (c) 2005-2009, Alexey Sudachen, alexey@sudachen.name

*/


#if !defined __355016D5_F220_4D95_A6AA_E1CFA9B538F2__
#define __355016D5_F220_4D95_A6AA_E1CFA9B538F2__

#include "_specific.h"
#include "string.h"

namespace teggo
{
  template < class tTchr >
    struct Property_String : StringT<tTchr>
      {

        typedef tTchr const CChar;
        Property_String() {}
        Property_String(pwide_t s) : StringT<tTchr>(s) {}

      private:
        operator CChar*();
        operator tTchr*();
      };
}

#define TEGGO_RESOLVE_PTR(T,__name) \
  template <class fake> static inline T* resolve_(const void* p,fake) {\
  /*static inline T* resolve_(const void* p,unsigned) {*/\
    T *_ = 0; \
    return (T*)( (char*)p - ( (char*)(&_->__name) - (char*)(_) ) ); \
  }

#define TEGGO_PROPERTY(T,__type,__set,__get,__name) \
  \
  __type value() const { return resolve_(this,1)->__get(); } \
  \
  __type operator+() const { return value(); } \
  \
  operator __type () const { return value(); } \
  \
  void \
    operator = (__type const& a) { resolve_(this,1)->__set(a); } \
  \
  template <class D> void \
    operator += (D dif) { resolve_(this,1)->__set(value() + dif);} \
  \
  template <class D> void \
    operator -= (D dif) { resolve_(this,1)->__set(value() - dif);} \
  \
  TEGGO_RESOLVE_PTR(T,__name)

#define TEGGO_PROPERTY_ELM(T,__type,__get,__name) \
  const Property_##__name* arr_; \
  int i_; \
  \
  elm_t(const Property_##__name* a,int i) : arr_(a), i_(i) {} \
  \
  __type value() const { return resolve_(arr_,1)->__get(i_); } \
  \
  operator __type () const { return value(); } \
  \
  TEGGO_RESOLVE_PTR(T,__name)


#define TEGGO_PROPERTY_ARR(T,__type,__set,__get,__name) \
  struct elm_t { \
    TEGGO_PROPERTY_ELM(T,__type,__get,__name) \
    void \
      operator = (__type const& a) { resolve_(arr_,1)->__set(i_,a); } \
    \
    template <class D> void \
      operator += (D dif) { resolve_(arr_,1)->__set(i_,value() + dif);} \
    \
    template <class D> void \
      operator -= (D dif) { resolve_(arr_,1)->__set(i_,value() - dif);} \
  }; \
  elm_t operator[](int i) const { return elm_t(this,i); }

#define TEGGO_PROPERTY_ARROW(T,__type,__set,__get,__name) \
  TEGGO_PROPERTY(T,__type,__set,__get,__name) \
  __type operator->() const { return value(); }

#define TEGGO_R_PROPERTY(T,__type,__get,__name) \
  __type value() const { return resolve_(this,1)->__get(); } \
  operator __type () const { return value(); } \
  \
  TEGGO_RESOLVE_PTR(T,__name)

#define TEGGO_R_PROPERTY_ARROW(T,__type,__get,__name) \
  TEGGO_R_PROPERTY(T,__type,__get,__name) \
  __type \
    operator->() const { return value(); }


#define TEGGO_W_PROPERTY(T,__type,__set,__name) \
  void \
    operator = (__type const &a ) { resolve_(this,1)->__set(a); } \
  \
  TEGGO_RESOLVE_PTR(T,__name)

#define TEGGO_S_PROPERTY(T,__set,__get,__name) \
  ::teggo::Property_String<wchar_t> value() const { return resolve_(this,1)->__get(); } \
  operator ::teggo::Property_String<wchar_t> () const { return resolve_(this,1)->__get(); } \
  void \
    operator = (pwide_t a) { resolve_(this,1)->__set(a); } \
  \
  void \
    operator = (::teggo::BaseStringT<wchar_t> const &a) { resolve_(this,1)->__set(a.Str()); } \
  \
  pwide_t Str() const { return value().Str(); } \
  pwide_t c_str() const { return Str(); }  \
  \
  TEGGO_RESOLVE_PTR(T,__name)

#endif // __355016D5_F220_4D95_A6AA_E1CFA9B538F2__
