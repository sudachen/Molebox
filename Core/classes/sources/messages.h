
/*

  Copyright (c) 2005-2009, Alexey Sudachen, alexey@sudachen.name

*/

#ifndef ___9e2a6956_c0dc_46ad_b507_bc3518d92fe9___
#define ___9e2a6956_c0dc_46ad_b507_bc3518d92fe9___

#include "_specific.h"
#include "string.h"
#include "symboltable.h"
#include "ptr_rcc.h"

enum _M_message_constuctor { _M };

namespace teggo {

  struct Message_
    {
      Message_ const *ft_;
      enum CONTENT_TYPE { xS, xW, xL, xF, xP, xJ, xR, xMSG } t_;
      union
        {
          pchar_t     sval_;
          pwide_t     wval_;
          long        lval_;
          double      fval_;
          void const *pval_;
          GUID const *gval_;
          Ireferred  *rval_;
        };
      mutable wchar_t *tempo_;

      explicit Message_(pwide_t msg) : ft_(0), t_(xMSG) { wval_ = msg; }
      explicit Message_(pchar_t msg) : ft_(0), t_(xMSG) { wval_ = tempo_ = widestrdup(msg); }
      ~Message_() { if ( tempo_ ) delete[] tempo_; }

      template < class tTx >
      void operator ,(tTx const &x) { x.__there_is_invalid_operation_comma__(); }

      explicit Message_( Message_ const *ft, pchar_t s ) : ft_(ft), t_(xS) { sval_ = s; }
      explicit Message_( Message_ const *ft, pwide_t w ) : ft_(ft), t_(xW) { wval_ = w; }
      explicit Message_( Message_ const *ft, BaseStringT<char> const &s ) : ft_(ft), t_(xS) { sval_ = s.Str(); }
      explicit Message_( Message_ const *ft, BaseStringT<wchar_t> const &w ) : ft_(ft), t_(xW) { wval_ = w.Str(); }
      explicit Message_( Message_ const *ft, Ireferred *p ) : ft_(ft), t_(xR) { rval_ = p; }
      explicit Message_( Message_ const *ft, void const*p ) : ft_(ft), t_(xP) { pval_ = p; }
      explicit Message_( Message_ const *ft, int l ) : ft_(ft), t_(xL) { lval_ = l; }
      explicit Message_( Message_ const *ft, long u ) : ft_(ft), t_(xL) { lval_ = u; }
      explicit Message_( Message_ const *ft, unsigned u ) : ft_(ft), t_(xL) { lval_ = long(u); }
      explicit Message_( Message_ const *ft, unsigned long u ) : ft_(ft), t_(xL) { lval_ = long(u); }
      explicit Message_( Message_ const *ft, float f ) : ft_(ft), t_(xF) { fval_ = f; }
      explicit Message_( Message_ const *ft, double f ) : ft_(ft), t_(xF) { fval_ = f; }
      explicit Message_( Message_ const *ft, GUID const &s ) : ft_(ft), t_(xJ) {gval_ = &s;}
      explicit Message_( Message_ const *ft, GUID const *s ) : ft_(ft), t_(xJ) {gval_ = s;}

      template < class tTy >
        Message_ operator % (tTy const &val) const { return Message_(this,val); }

      _TEGGO_EXPORTABLE long ToLong() const;
      _TEGGO_EXPORTABLE double ToFloat() const;
      _TEGGO_EXPORTABLE pwide_t Str() const;
      _TEGGO_EXPORTABLE bool IsNull() const;

      long   Long()  const   { return ( t_ == xL ) ? lval_ : ToLong(); }
      double Float() const   { return ( t_ == xF ) ? fval_ : ToFloat(); }
      void const * Uptr()  const   { return ( t_ == xP || t_ == xR ) ? pval_ : 0; }
      rcc_ptr<Ireferred> Referred()  const { return ( t_ == xR ) ? RccRefe(rval_) : rcc_ptr<Ireferred>(0); }
      template<class tTx> tTx *Cast(tTx* = 0) const { return t_ == xR ? dynamic_cast<tTx*>(rval_) : 0; }
    };

  inline Message_ operator *(_M_message_constuctor,pwide_t msg) { return Message_(msg); }
  inline Message_ operator *(_M_message_constuctor,pchar_t msg) { return Message_(msg); }

  struct MessageConnector_;

  struct MessageArgs_
    {
      Message_ const **params_;
      int count_;

      int Count() const { return count_; }
      Message_ const &operator[](int no) const
        {
          REQUIRE( no >= 0 && no < count_ );
          return *params_[no];
        }
    };
  typedef MessageArgs_ const &MessageArgs;

  struct MessageConnList_
    {
      MessageConnector_ *lst_;
      MessageConnList_  *next_;
      pwide_t           msgid_;
      MessageConnList_(pwide_t msgid) : lst_(0), next_(0), msgid_(msgid) {}
      _TEGGO_EXPORTABLE ~MessageConnList_();
      _TEGGO_EXPORTABLE int Process_(pwide_t msgid,MessageArgs args);
      _TEGGO_EXPORTABLE int Process(pwide_t msgid,MessageArgs args);
      _TEGGO_EXPORTABLE void Erase_(MessageConnector_ *);
      _TEGGO_EXPORTABLE MessageConnector_ *Insert_(MessageConnector_ *);
      _TEGGO_EXPORTABLE MessageConnector_ *Insert(pwide_t msgid, MessageConnector_ *);
    };

  struct MessageConnector_
    {
      MessageConnector_ *next_;
      MessageConnList_ *lst_;
      MessageConnector_() : next_(0), lst_(0) {};
      virtual ~MessageConnector_() {}
      virtual void operator() (pwide_t msgid, MessageArgs args) const = 0;
    };

  template<class tTx>
    struct MesgObjeConnT : MessageConnector_
      {
        tTx* t_;
        void (tTx::*f_)(pwide_t,MessageArgs);

        void operator() (pwide_t msgid, MessageArgs args) const
          {
            (t_->*f_)(msgid, args);
          }

        MesgObjeConnT(tTx* t,void (tTx::*f)(pwide_t,MessageArgs)) : t_(t),f_(f) {}
      };

  struct MesgFuncConnT : MessageConnector_
    {
      void (CXX_STDCALL *f_)(pwide_t,MessageArgs);

      void operator() (pwide_t msgid, MessageArgs args) const
        {
          f_(msgid, args);
        }

      MesgFuncConnT(void (CXX_STDCALL *f)(pwide_t,MessageArgs)) : f_(f) {}
    };

  struct MesgObject_
    {
      pwide_t msgid_;
      MessageConnector_ *conn_;
      MesgObject_( pwide_t msgid, MessageConnector_ *conn ) : msgid_(msgid), conn_(conn) {}
    };

  struct MesgObject2_
    {
      MessageConnector_ *conn_;
      MesgObject2_( MessageConnector_ *conn ) : conn_(conn) {}
      MesgObject_ operator &(pwide_t msgid) { return MesgObject_(msgid,conn_); }
    };

  template <class tTx>
    struct MesgObject1_
      {
        void (tTx::*f_)(pwide_t,MessageArgs);
        MesgObject1_( void (tTx::*f)(pwide_t,MessageArgs) ) : f_(f) {}
        MesgObject2_ operator &( tTx *self )
          {
            return MesgObject2_( new MesgObjeConnT<tTx>(self,f_) );
          }
      };

  template < class tTx >
    MesgObject1_<tTx> operator *(_M_message_constuctor, void (tTx::*f)(pwide_t,MessageArgs) )
      { return MesgObject1_<tTx>( f ); }

  inline
    MesgObject2_ operator *( _M_message_constuctor, void (CXX_STDCALL *f)(pwide_t,MessageArgs) )
      { return MesgObject2_( new MesgFuncConnT(f) ); }

  struct MessageHub
    {
      MessageConnList_ *listeners_;
      rcc_ptr<SymbolTable> stable_;

      _TEGGO_EXPORTABLE void Send(pwide_t args);
      _TEGGO_EXPORTABLE int  Send(Message_ const &args);
      _TEGGO_EXPORTABLE MessageConnector_ *Connect_(pwide_t msgid,MessageConnector_ *conn);
      _TEGGO_EXPORTABLE void operator << ( Message_ const &args ) { Send( args); }

      void  Disconnect(void* conn)
        {
          if ( conn )
            ((MessageConnector_*)conn)->lst_->Erase_( (MessageConnector_*)conn );
        }

      template<class tTx,class tTf>
        void *Connect(pwide_t msgid,tTx* self,tTf fn)
          {
            return Connect_(msgid,new MesgObjeConnT<tTx>(self,fn));
          }

      _TEGGO_EXPORTABLE void *Connect(MesgObject_ const&);
      _TEGGO_EXPORTABLE void *Disconnect(MesgObject_ const&);
      void operator += ( MesgObject_ const& m ) { Connect(m); }
      void operator -= ( MesgObject_ const& m ) { Disconnect(m); }

      _TEGGO_EXPORTABLE MessageHub( SymbolTable *stable = 0 );
      _TEGGO_EXPORTABLE ~MessageHub();
    };

  struct MesgTarget
    {
      template<class tTx,class tTf>
        struct Connector : MessageConnector_
          {
            tTx* t_; tTf f_; MesgTarget& target_;
            void operator() (pwide_t msgid,MessageArgs args) const
              {
                (t_->*f_)(msgid,args);
              }
            Connector(tTx* t,tTf f,MesgTarget& s) : t_(t),f_(f), target_(s) {}
            virtual ~Connector() { target_.connector_ = 0; }
          };

      MesgTarget() : connector_(0) {}
      ~MesgTarget() { Disconnect(); }

      void Disconnect()
        {
          if ( connector_ )
            connector_->lst_->Erase_(connector_);
        }

      template<class tTx,class tTf>
        void Connect(MessageHub *scom, pwide_t msgid, tTx* self, tTf fn)
          {
            Disconnect();
            if ( scom )
              connector_ = scom->Connect_(
                              msgid,
                              new Connector<tTx,tTf>(self,fn,*this));
          }

      MessageConnector_* connector_;

    private:
      MesgTarget(const MesgTarget&);
      void operator = (const MesgTarget&);

    };

} // namespace teggo

#ifdef _TEGGOINLINE
#include "./messages.inl"
#endif

#endif // ___9e2a6956_c0dc_46ad_b507_bc3518d92fe9___
