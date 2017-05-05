
/*

  Copyright (c) 2005-2009, Alexey Sudachen, alexey@sudachen.name

*/

#if !defined __68A0C924_59E0_467A_B329_BA4B1809C5BC__
#define __68A0C924_59E0_467A_B329_BA4B1809C5BC__

#if !defined CXX_NO_MSCOM_SUPPORT

#include "_specific.h"
#include "hinstance.h"
#include "com_generic.h"
#include "flatmap.h"

struct IConnectionPoint;
struct IConnectionPointContainer;
struct IProvideClassInfo;
struct IEnumConnections;
struct IEnumConnectionPoints;

TEGGO_DECLARE_GUIDOF(IConnectionPoint, 0xB196B286,0xBAB4,0x101A,0xB6,0x9C,0x00,0xAA,0x00,0x34,0x1D,0x07);
TEGGO_DECLARE_GUIDOF(IConnectionPointContainer, 0xB196B284,0xBAB4,0x101A,0xB6,0x9C,0x00,0xAA,0x00,0x34,0x1D,0x07);
TEGGO_DECLARE_GUIDOF(IProvideClassInfo, 0xB196B283,0xBAB4,0x101A,0xB6,0x9C,0x00,0xAA,0x00,0x34,0x1D,0x07);

namespace teggo
{

  struct DispResult
    {

      HRESULT hResult_;
      VARIANT result_;

      DispResult(VARIANT const &v,HRESULT h) : hResult_(h), result_(v)
        {
        }

      DispResult(HRESULT h) : hResult_(h)
        {
          VariantClear( &result_ );
        }

      ~DispResult()
        {
          VariantClear( &result_ );
        }

      DispResult(DispResult const &r)
        {
          hResult_ = r.hResult_;
          VariantInit(&result_);
          VariantCopyInd(&result_,(VARIANT*)&r.result_);
        }

      const DispResult& operator = (DispResult const &r)
        {
          hResult_ = r.hResult_;
          VariantCopyInd(&result_,(VARIANT*)&r.result_);
          return *this;
        }

      bool IsSuccess()
        {
          return SUCCEEDED(hResult_);
        }

      bool HasResult()
        {
          return result_.vt != VT_EMPTY;
        }

      long ToLong()
        {
          if ( result_.vt == VT_I2 )
            return result_.iVal;
          if ( result_.vt == VT_I4 )
            return result_.lVal;
          if ( result_.vt == VT_R4 )
            return long(result_.fltVal);
          if ( result_.vt == VT_UI2 )
            return result_.uiVal;
          if ( result_.vt == VT_UI4 )
            return result_.ulVal;
          else
            {
              VARIANT v;
              VariantInit(&v);
              VariantChangeType(&v,&result_,0,VT_I4);
              return v.lVal;
            }
        }
    };


  template < class tDi >
    DispResult CXX_STDCALL Disp_Invoker_Proceed_( tDi const* );

  struct Disp_Invoker_Base_
    {
      union
        {
          VARIANT value_;
          struct
            {
              u32_t dispid_or_hresult;
              IDispatch *pdisp;
            }
            invoker_;
        } _;

      Disp_Invoker_Base_ const *next_;
    };

  template < unsigned tAno >
    struct Disp_Invoker_ : Disp_Invoker_Base_
      {
        typedef Disp_Invoker_<tAno+1> Disp_Invoker_1;

        enum { ArgCnt = tAno };

        CXX_NO_INLINE
        Disp_Invoker_( IDispatch *disp, pwide_t method )
          {
            next_ = 0;
            _.invoker_.dispid_or_hresult = E_FAIL;
            _.invoker_.pdisp = disp;

            HRESULT hr =
              disp->GetIDsOfNames(
                IID_NULL,
                (LPOLESTR*)&method,1,LOCALE_SYSTEM_DEFAULT,
                (DISPID*)&_.invoker_.dispid_or_hresult);

            if ( FAILED(hr) )
              {
                _.invoker_.dispid_or_hresult = hr;
                _.invoker_.pdisp = 0;
              }
          }

        Disp_Invoker_( IDispatch *disp, u32_t dispid )
          {
            next_ = 0;
            _.invoker_.dispid_or_hresult = dispid;
            _.invoker_.pdisp = disp;
          }

        ~Disp_Invoker_()
          {
          }

        CXX_NO_INLINE
        DispResult Invoke()
          {
            return Disp_Invoker_Proceed_(this);
          }

        DispResult operator*()
          {
            return Invoke();
          }

        Disp_Invoker_( Disp_Invoker_Base_ const *di, bool val )
          {
            _.value_.vt = VT_BOOL;
            _.value_.boolVal = val ? VARIANT_TRUE : VARIANT_FALSE;
            next_ = di;
          }

        Disp_Invoker_( Disp_Invoker_Base_ const *di, long val )
          {
            _.value_.vt = VT_I4;
            _.value_.lVal = val;
            next_ = di;
          }

        Disp_Invoker_( Disp_Invoker_Base_ const *di, float val )
          {
            _.value_.vt = VT_R4;
            _.value_.fltVal = val;
            next_ = di;
          }

        Disp_Invoker_( Disp_Invoker_Base_ const *di, IDispatch *val )
          {
            _.value_.vt = VT_DISPATCH;
            _.value_.pdispVal = val;
            next_ = di;
          }

        Disp_Invoker_( Disp_Invoker_Base_ const *di, IUnknown *val )
          {
            _.value_.vt = VT_UNKNOWN;
            _.value_.punkVal = val;
            next_ = di;
          }

        Disp_Invoker_1 operator %(long val) const
          {
            return Disp_Invoker_1(this,val);
          }

        Disp_Invoker_1 operator %(bool val) const
          {
            return Disp_Invoker_1(this,val);
          }

        Disp_Invoker_1 operator %(float val) const
          {
            return Disp_Invoker_1(this,val);
          }

        Disp_Invoker_1 operator %(IDispatch *val) const
          {
            return Disp_Invoker_1(this,val);
          }

        Disp_Invoker_1 operator %(IUnknown *val) const
          {
            return Disp_Invoker_1(this,val);
          }
      };

  typedef Disp_Invoker_<0> DispInvoker;

  _TEGGO_EXPORTABLE DispResult CXX_STDCALL Disp_Invoker_Proceed_Common_(
    Disp_Invoker_Base_ const *di, int argCnt, VARIANT *arguments );

  template < class tDi >
    DispResult CXX_STDCALL Disp_Invoker_Proceed_( tDi const* di )
      {
        enum { argCnt = tDi::ArgCnt };
        VARIANT arguments[argCnt];
        return Disp_Invoker_Proceed_Common_( di, argCnt, arguments );
      }

  template <> CXX_FAKE_INLINE
    DispResult CXX_STDCALL Disp_Invoker_Proceed_( Disp_Invoker_<0> const *di )
      {
        return Disp_Invoker_Proceed_Common_( di, 0, 0 );
      }

  struct TypeInfo_
    {
      u32_t major_;
      u32_t minor_;
      GUID const *iid_;
      rcc_ptr<ITypeInfo> info_;

      struct DISPID__
        {
          DISPID id;
          LPWSTR name;

          int Dispose()
            {
              if ( name )
                SysFreeString((BSTR)name);
              name = 0;
              return 0;
            }

          bool operator < (const DISPID__& b) const
            {
              if ( !b.name ) return false;
              if ( !name ) return true;
              return wcscmp(name,b.name) < 0;
            }

          _TEGGO_EXPORTABLE static int CXX_CDECL Qscmp(void const* aa,void const* bb);
        };

      ArrayT<DISPID__> dispids_;

      _TEGGO_EXPORTABLE TypeInfo_(const GUID* iid);
      _TEGGO_EXPORTABLE ~TypeInfo_();
      _TEGGO_EXPORTABLE HRESULT GetTI(LCID lcid, ITypeInfo** info, HinstanceType &hinstance);
      _TEGGO_EXPORTABLE HRESULT GetTI(LCID lcid, HinstanceType &hinstance);
      _TEGGO_EXPORTABLE HRESULT EnsureTI(LCID lcid, HinstanceType &hinstance);
      _TEGGO_EXPORTABLE HRESULT GetTypeInfo(UINT , LCID lcid, ITypeInfo** info, HinstanceType &hinstance);
      _TEGGO_EXPORTABLE HRESULT GetIDsOfNames(REFIID, LPOLESTR* names, UINT count, LCID lcid, DISPID* dispid, HinstanceType &hinstance);
      _TEGGO_EXPORTABLE HRESULT Invoke(IDispatch* p, DISPID dispidMember, REFIID /* riid */,
        LCID lcid, WORD wFlags, DISPPARAMS* pdispparams, VARIANT* pvarResult,
        EXCEPINFO* pexcepinfo, UINT* puArgErr, HinstanceType &hinstance = Hinstance);
      _TEGGO_EXPORTABLE HRESULT LoadNameCache(ITypeInfo* pTypeInfo);
    };

  struct TypeInfo_Factory_
    {
      typedef FlatmapT<GUID,TypeInfo_*,GUIDless> TLB_MAP;
      TLB_MAP* map_;
      _TEGGO_EXPORTABLE TypeInfo_* GetTypeinfo(const GUID* iidGUID);
      _TEGGO_EXPORTABLE void Term();
      _TEGGO_EXPORTABLE ~TypeInfo_Factory_();
    protected:
      _TEGGO_EXPORTABLE TypeInfo_Factory_();
      _TEGGO_EXPORTABLE friend TypeInfo_Factory_& TypeInfoFactory();
    };

  _TEGGO_EXPORTABLE TypeInfo_Factory_& TypeInfoFactory();
  _TEGGO_EXPORTABLE void InterlockedInitHolder(TypeInfo_ **holder,GUID const *guid);

  _TEGGO_EXPORTABLE StringW GenGUIDW();
  _TEGGO_EXPORTABLE StringA GenGUIDA();

} // namespace

template < const GUID *tConIID,const GUID *tClsidCo, class tOther >
  struct CXX_NO_VTABLE TeggoCopntImpl :
    TeggoIfs< IConnectionPoint,
      TeggoIfs< IConnectionPointContainer,
        TeggoIfs< IProvideClassInfo, tOther > > >
    {
      virtual HRESULT InternalAdvise(IUnknown* ifs,u32_t* cookie) = 0;
      virtual HRESULT InternalUnadvise(u32_t cookie) = 0;

      virtual HRESULT __stdcall GetConnectionInterface(IID *pIID)
        {
          *pIID = *tConIID;
          return S_OK;
        }

      virtual HRESULT __stdcall Advise(IUnknown* pUnkSink,DWORD* pdwCookie)
        {
          teggo::rcc_ptr<IUnknown> ifs;
          pUnkSink->QueryInterface(*tConIID,(void**)&+ifs);
          return InternalAdvise(+ifs,pdwCookie);
        }

      virtual HRESULT __stdcall Unadvise(DWORD dwCookie)
        {
          return InternalUnadvise(dwCookie);
        }

      virtual HRESULT __stdcall FindConnectionPoint(REFIID riid,IConnectionPoint **ppCP)
        {
          if ( IsEqualGUID(riid,*tConIID) )
            return this->InternalQueryInterface(TEGGO_GUIDOF(IConnectionPoint),(void**)ppCP);
          else
            return E_NOINTERFACE;
        }

      virtual HRESULT __stdcall GetClassInfo(ITypeInfo** ppTI)
        {
          return teggo::TypeInfoFactory().GetTypeinfo(tClsidCo)->GetTypeInfo(0,0,ppTI,Hinstance);
        }

      virtual HRESULT __stdcall EnumConnections(IEnumConnections **ppEnum)
        {
          return E_UNEXPECTED;
        }

      virtual HRESULT __stdcall EnumConnectionPoints(IEnumConnectionPoints **ppEnum)
        {
          return E_UNEXPECTED;
        }

      virtual HRESULT __stdcall GetConnectionPointContainer(IConnectionPointContainer **ppCPC)
        {
          return E_UNEXPECTED;
        }
    };

template< class tDifs, class tOther = TeggoIfsUnknown >
  struct CXX_NO_VTABLE TeggoDispImpl : tDifs, tOther
    {

      static teggo::TypeInfo_& GetHolder()
        {
          static teggo::TypeInfo_* holder = 0;
          if ( !holder )
            teggo::InterlockedInitHolder(&holder,&TEGGO_GUIDOF(tDifs));
          return *holder;
        }

      virtual HRESULT __stdcall GetTypeInfoCount(UINT* pctinfo)
        {
          *pctinfo = 1;
          return S_OK;
        }

      virtual HRESULT __stdcall GetTypeInfo(UINT itinfo, LCID lcid, ITypeInfo** pptinfo)
        {
          return GetHolder().GetTypeInfo(itinfo, lcid, pptinfo, Hinstance);
        }

      virtual HRESULT __stdcall GetIDsOfNames(REFIID riid, LPOLESTR* rgszNames, UINT cNames,
        LCID lcid, DISPID* rgdispid)
        {
          return GetHolder().GetIDsOfNames(riid, rgszNames, cNames, lcid, rgdispid, Hinstance);
        }

      virtual HRESULT __stdcall Invoke(
        DISPID dispidMember, REFIID riid,
        LCID lcid, WORD wFlags, DISPPARAMS* pdispparams, VARIANT* pvarResult,
        EXCEPINFO* pexcepinfo, UINT* puArgErr)
        {
          return GetHolder().Invoke(
            (tDifs*)this, dispidMember, riid, lcid,
            wFlags, pdispparams, pvarResult, pexcepinfo, puArgErr);
        }

      virtual HRESULT InternalQueryInterface(REFIID riid ,void ** pI)
        {
          if ( IsEqualIID(riid,TEGGO_GUIDOF(tDifs)) )
            {
              *(tDifs**)pI = this;
              this->AddRef();
              return S_OK;
            }
          if ( IsEqualIID(riid,TEGGO_GUIDOF(IDispatch)) )
            {
              *(IDispatch**)pI = (tDifs*)this;
              this->AddRef();
              return S_OK;
            }
          return tOther::InternalQueryInterface(riid,pI);
        }

      virtual u32_t __stdcall AddRef() = 0;
    };

#if defined _TEGGOINLINE
#include "com_dynamic.inl"
#endif

#endif // !defined CXX_NO_MSCOM_SUPPORT

#endif // __68A0C924_59E0_467A_B329_BA4B1809C5BC__
