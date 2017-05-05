
/*

  Copyright (c) 2005-2009, Alexey Sudachen, alexey@sudachen.name

*/

#if !defined __E3EE9CE8_7404_4A0F_AD93_E28AC2A4E9C2__
#define __E3EE9CE8_7404_4A0F_AD93_E28AC2A4E9C2__


#if !defined CXX_NO_MSCOM_SUPPORT

#include "_specific.h"
#include "string.h"
#include "wexcept.h"
#include "guid.h"
#include "ptr_rcc.h"
#include "ptr_mem.h"

TEGGO_DECLARE_GUIDOF_(IUnknown,0x00000000,0x0000,0x0000,0xC0,0x00,0x00,0x00,0x00,0x00,0x00,0x46);
TEGGO_DECLARE_GUIDOF_(IDispatch,0x00020400,0x0000,0x0000,0xC0,0x00,0x00,0x00,0x00,0x00,0x00,0x46);
TEGGO_DECLARE_GUIDOF_(IClassFactory,0x00000001,0x0000,0x0000,0xC0,0x00,0x00,0x00,0x00,0x00,0x00,0x46);
TEGGO_DECLARE_GUIDOF_(ITypeLib,0x00020402,0x0000,0x0000,0xC0,0x00,0x00,0x00,0x00,0x00,0x00,0x46);
TEGGO_DECLARE_GUIDOF_(ITypeInfo,0x00020401,0x0000,0x0000,0xC0,0x00,0x00,0x00,0x00,0x00,0x00,0x46);
TEGGO_DECLARE_GUIDOF_(IErrorInfo,0x1CF2B120,0x547D,0x101B,0x8E,0x65,0x08,0x00,0x2B,0x2B,0xD1,0x19);
TEGGO_DECLARE_GUIDOF_(ISupportErrorInfo,0xDF0B3D60,0x548F,0x101B,0x8E,0x65,0x08,0x00,0x2B,0x2B,0xD1,0x19);
TEGGO_DECLARE_GUIDOF_(ITypeLib2,0x00020411,0x0000,0x0000,0xC0,0x00,0x00,0x00,0x00,0x00,0x00,0x46);
TEGGO_DECLARE_GUIDOF_(ITypeInfo2,0x00020412,0x0000,0x0000,0xC0,0x00,0x00,0x00,0x00,0x00,0x00,0x46);
TEGGO_DECLARE_GUIDOF_(IEnumVARIANT,0x00020404,0x0000,0x0000,0xC0,0x00,0x00,0x00,0x00,0x00,0x00,0x46);
TEGGO_DECLARE_GUIDOF_(IStream,0x0000000c,0x0000,0x0000,0xC0,0x00,0x00,0x00,0x00,0x00,0x00,0x46);
TEGGO_DECLARE_GUIDOF_(ISequentialStream,0x0c733a30,0x2a1c,0x11ce,0xad,0xe5,0x00,0xaa,0x00,0x44,0x77,0x3d);

namespace teggo
{
  typedef rcc_ptr<IDispatch> IDispatchPtr;
  typedef rcc_ptr<IUnknown>  IUnknownPtr;
  typedef rcc_ptr<IStream>   IStreamPtr;
  typedef rcc_ptr<ISequentialStream> ISeqStreamPtr;

  struct Delete_Bstr_
    {
      static void Delete(OLECHAR *p) { if ( p != 0 ) ::SysFreeString(p); }
      void operator ()(OLECHAR *&p) const { Delete(p); p = 0; }
    };

  struct bstr_ptr : Auto_Ptr<OLECHAR,Delete_Bstr_>
    {
      explicit bstr_ptr(OLECHAR *p = 0) : Auto_Ptr<OLECHAR,Delete_Bstr_> (p)
        {
        }
    };

  template <class tTx,class tTy> inline
    rcc_ptr<tTx> ifs_cast(tTy *u,tTx * = 0)
      {
        tTx* p = 0;
        if ( u ) u->QueryInterface(TEGGO_GUIDOF(tTx),(void**)&p);
        return rcc_ptr<tTx>(p);
      }

  template <class tTx,class tTy> inline
    rcc_ptr<tTx> ifs_cast(rcc_ptr<tTy> const &u, tTx * = 0)
      {
        return ifs_cast<tTx,tTy>(+u);
      }

}

template < class tBaseException >
  struct Teggo_Com_Exception : public tBaseException
    {
      Teggo_Com_Exception()
        {
          // G++ hack
          GetComError(&((teggo::widestring&)tBaseException::what_));
        }

      virtual ~Teggo_Com_Exception()
        {
        }

      static void GetComError(teggo::widestring*);
    };

typedef Teggo_Com_Exception<teggo::WideException> TeggoComException;

template < class tBaseException > CXX_NO_INLINE
  void Teggo_Com_Exception<tBaseException>::GetComError(teggo::widestring* wc)
    {
      teggo::rcc_ptr<IErrorInfo> ierr;
      *wc = L"unknown com error";
      if ( SUCCEEDED(GetErrorInfo(0,&+ierr)) && ierr )
        {
          BSTR b = 0;
          if ( SUCCEEDED(ierr->GetDescription(&b)) && b)
            {
              *wc = b;
              SysFreeString(b);
            }
        }
    }

template < class tTx >
  struct CXX_NO_VTABLE TeggoIfs0 : public tTx
    {
      HRESULT InternalQueryInterface(REFIID riid,void **pI)
        {
          if ( IsEqualGUID(riid,TEGGO_GUIDOF(tTx)) )
            { *pI = teggo::Refe((tTx*)this); return S_OK; }
          return E_NOINTERFACE;
        }
      IUnknown* __iunknown()
        {
          return (IUnknown*)this;
        }
    };

typedef TeggoIfs0< IUnknown > TeggoIfsUnknown;

template < class tTx >
  struct CXX_NO_VTABLE TeggoIfs0_Private : private tTx
    {
      HRESULT InternalQueryInterface(REFIID riid,void **pI)
        {
          if ( IsEqualGUID(riid,TEGGO_GUIDOF(tTx)) )
            {
              *pI = teggo::Refe((tTx*)this);
              return S_OK;
            }
          return E_NOINTERFACE;
        }
      IUnknown* __iunknown()
        {
          return (IUnknown*)this;
        }
    };

template < class tTx, class tTu = TeggoIfsUnknown >
  struct CXX_NO_VTABLE TeggoIfs : tTu, tTx
    {
      HRESULT InternalQueryInterface(REFIID riid,void **pI)
        {
          if ( IsEqualGUID(riid,TEGGO_GUIDOF(tTx)) )
            {
              *pI = teggo::Refe((tTx*)this);
              return S_OK;
            }
          return tTu::InternalQueryInterface(riid,pI);
        }
      using tTu::__iunknown;
    };

typedef TeggoIfs< ISupportErrorInfo > TeggoISupportErrorInfo;

template < class tTx >
  struct CXX_NO_VTABLE TeggoSupportEInfo : TeggoISupportErrorInfo
    {
      HRESULT __stdcall InterfaceSupportsErrorInfo(REFIID riid)
        {
          return IsEqualGUID(TEGGO_GUIDOF(tTx),riid)?S_OK:S_FALSE;
        }
        
      HRESULT RaiseComError(pwide_t msg)
        {
          ICreateErrorInfo* pICE = 0;
          if ( FAILED( ::CreateErrorInfo(&pICE) ) ) return E_FAIL;
          pICE->SetSource((BSTR)L"");
          pICE->SetDescription((BSTR)msg);
          IErrorInfo* pIEI = 0;
          if ( SUCCEEDED( pICE->QueryInterface(TEGGO_GUIDOF(IErrorInfo),(void**)&pIEI) ) ) 
            {
              ::SetErrorInfo(0L,pIEI);
              pIEI->Release();
            }
          pICE->Release();
          return E_FAIL;
        }
    };

template < class tTx, class tTu = TeggoIfsUnknown >
  struct CXX_NO_VTABLE TeggoCppIfs : tTu, tTx
    {
      using tTu::InternalQueryInterface;
      using tTu::__iunknown;
    };

template < class tTx, class tTu = TeggoIfsUnknown >
  struct CXX_NO_VTABLE Teggo_Ifl_ : tTu, tTx
    {
      HRESULT InternalQueryInterface(REFIID riid,void ** pI)
        {
          if ( FAILED( tTx::InternalQueryInterface(riid,pI)) )
            return tTu::InternalQueryInterface(riid,pI);
          return S_OK;
        }
      using tTu::__iunknown;
    };

template < unsigned tFno >
struct Teggo_Fifs_
  {
    static HRESULT InternalQueryInterface(REFIID riid,void ** pI)
      {
        return E_NOINTERFACE;
      }
  };

template <
    class tIx0 = Teggo_Fifs_<0>,
    class tIx1 = Teggo_Fifs_<1>,
    class tIx2 = Teggo_Fifs_<2>,
    class tIx3 = Teggo_Fifs_<3>,
    class tIx4 = Teggo_Fifs_<4>,
    class tIx5 = Teggo_Fifs_<5>,
    class tIx6 = Teggo_Fifs_<6>,
    class tIx7 = Teggo_Fifs_<7>,
    class tIx8 = Teggo_Fifs_<8>,
    class tIx9 = Teggo_Fifs_<9>
  >
  struct TeggoIfsList :
    Teggo_Ifl_< tIx0,
    Teggo_Ifl_< tIx1,
    Teggo_Ifl_< tIx2,
    Teggo_Ifl_< tIx3,
    Teggo_Ifl_< tIx4,
    Teggo_Ifl_< tIx5,
    Teggo_Ifl_< tIx6,
    Teggo_Ifl_< tIx7,
    Teggo_Ifl_< tIx8,
    Teggo_Ifl_< tIx9
      > > > > > > > > > >
    {
    };

template < class tTx = TeggoIfsUnknown >
  struct TeggoObject : tTx
    {
      virtual u32_t __stdcall Release()
        {
          long refcount = InterlockedDecrement(&refcount_);
          if ( refcount == 0 )
            {
              TeggoObject_Finalize();
              TeggoObject_Dispose();
            }
          return refcount;
        }

      virtual u32_t __stdcall AddRef()
        {
          long refcount = InterlockedIncrement(&refcount_);
          return refcount;
        }

      virtual HRESULT __stdcall QueryInterface(REFIID riid,void ** pI)
        {
          return tTx::InternalQueryInterface(riid,pI);
        }

      virtual void TeggoObject_Dispose()
        {
          delete this;
        }

      virtual void TeggoObject_Finalize()
        {
          // finalization hook
        }

      virtual ~TeggoObject()
        {
        }

      TeggoObject() : refcount_(1)
        {
        }

      u32_t GetRefCount__()
        {
          return refcount_;
        }

      teggo::IUnknownPtr __iunknown_ptr()
        {
          return teggo::RccRefe(tTx::__iunknown());
        }

      typedef teggo::rcc_ptr< TeggoObject<tTx> > Ptr;

    private:
      long refcount_;
    };

template <
    class tIx0 = Teggo_Fifs_<0>,
    class tIx1 = Teggo_Fifs_<1>,
    class tIx2 = Teggo_Fifs_<2>,
    class tIx3 = Teggo_Fifs_<3>,
    class tIx4 = Teggo_Fifs_<4>,
    class tIx5 = Teggo_Fifs_<5>,
    class tIx6 = Teggo_Fifs_<6>,
    class tIx7 = Teggo_Fifs_<7>,
    class tIx8 = Teggo_Fifs_<8>,
    class tIx9 = Teggo_Fifs_<9>
  >
  struct TeggoObjectEx : TeggoObject
    <
      Teggo_Ifl_< tIx0,
      Teggo_Ifl_< tIx1,
      Teggo_Ifl_< tIx2,
      Teggo_Ifl_< tIx3,
      Teggo_Ifl_< tIx4,
      Teggo_Ifl_< tIx5,
      Teggo_Ifl_< tIx6,
      Teggo_Ifl_< tIx7,
      Teggo_Ifl_< tIx8,
      Teggo_Ifl_< tIx9
        > > > > > > > > > >
    >
    {
    };

template < class tTx >
  struct CXX_NO_VTABLE TeggoInterface : tTx, teggo::Uncopyable
    {
    };

#define REQ_S_OK(expr) REQUIRE(SUCCEEDED(expr))

#endif //!defined CXX_NO_MSCOM_SUPPORT

#endif // __E3EE9CE8_7404_4A0F_AD93_E28AC2A4E9C2__
