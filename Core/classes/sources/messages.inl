
/*

  Copyright (c) 2005-2009, Alexey Sudachen, alexey@sudachen.name

*/

#ifndef ___e1412894_dc33_4b47_b903_ec271d1c74ba___
#define ___e1412894_dc33_4b47_b903_ec271d1c74ba___

#include "messages.h"
#include "splitter.h"
#include "format.h"

#if defined _TEGGO_MESSAGES_HERE
# define _TEGGO_MESSAGES_FAKE_INLINE _TEGGO_EXPORTABLE
#else
# define _TEGGO_MESSAGES_FAKE_INLINE CXX_FAKE_INLINE
#endif

namespace teggo
{

    _TEGGO_MESSAGES_FAKE_INLINE
    void MessageHub::Send(pwide_t args)
    {
        if ( widesplitter splitter = widesplitter(+StringT<wchar_t,128>(args).RemoveExtraWS(),' ') )
        {
            Message_ * dva =  (Message_*)alloca(sizeof(Message_)*(splitter.Count()-1));
            dva = new (dva) Message_(splitter.Get(0));
            for ( unsigned i = 1; i < splitter.Count(); ++i )
                dva = new (dva+1) Message_(dva,splitter.Get(i));
            Send(*dva);
        }
    }

    _TEGGO_MESSAGES_FAKE_INLINE
    int MessageHub::Send(Message_ const& args)
    {
        MessageArgs_ a; a.count_ = 0;
        Message_ const* m = &args;
        for ( ; m->ft_ ; m = m->ft_ ) ++a.count_;
        pwide_t msgid = m->wval_;
        a.params_ = (Message_ const**)alloca(sizeof(Message_*)*a.count_);
        m = &args;
        for ( int i = a.count_-1; m->ft_; --i, m = m->ft_ ) a.params_[i] = m;

        if ( pwide_t sid = stable_->FindSymbol(msgid) )
            if ( listeners_ )
                return listeners_->Process(sid,a);
        return 0;
    }

    _TEGGO_MESSAGES_FAKE_INLINE
    int MessageConnList_::Process_(pwide_t msgid, MessageArgs args)
    {
        int counter = 0;
        for ( MessageConnector_ *c = lst_, *c2 = 0; c != 0 && (c2 = c->next_, true); c = c2 )
            (*c)(msgid,args), ++counter;
        return counter;
    }

    _TEGGO_MESSAGES_FAKE_INLINE
    int MessageConnList_::Process(pwide_t msgid, MessageArgs args)
    {
        int n = 0;

        MessageConnList_ *l = this;
        do
        {
            if ( l->msgid_ == msgid_ )
            {
                n = l->Process_(msgid,args);
                break;
            }
            l = l->next_;
        }
        while ( l );

        if ( *msgid_ && !*this->msgid_ )
            n += this->Process_(msgid,args);

        return n;
    }

    _TEGGO_MESSAGES_FAKE_INLINE
    MessageConnector_* MessageConnList_::Insert_(MessageConnector_* sigconn)
    {
        sigconn->next_ = lst_;
        lst_ = sigconn;
        sigconn->lst_ = this;
        return sigconn;
    }

    _TEGGO_MESSAGES_FAKE_INLINE
    MessageConnector_* MessageConnList_::Insert(pwide_t msgid, MessageConnector_* sigconn)
    {
        MessageConnList_ *ll = this;
        MessageConnList_** l = &ll;

        do
        {
            if ( (*l)->msgid_ == msgid_ )
                return (*l)->Insert_(sigconn);
            l = &((*l)->next_);
        }
        while ( *l );

        if ( !*msgid )
        {
            ll = new MessageConnList_(msgid);
            ll->next_ = ll;
            _swab((char*)ll,(char*)this,sizeof(MessageConnList_));
            ll = this;
            l = &ll;
        }
        else
            *l = new MessageConnList_(msgid);

        return (*l)->Insert_(sigconn);
    }

    _TEGGO_MESSAGES_FAKE_INLINE
    void MessageConnList_::Erase_(MessageConnector_* sigconn)
    {
        MessageConnector_** c = &lst_;

        for ( ; *c && *c != sigconn; c = &(*c)->next_ ) {}

        if ( *c == sigconn )
        {
            *c = (*c)->next_;
            sigconn->lst_ = 0;
            sigconn->next_ = 0;
            delete sigconn;
        }
    }

    _TEGGO_MESSAGES_FAKE_INLINE
    MessageConnList_::~MessageConnList_()
    {
        for ( MessageConnector_* c = 0; 0 != (c = lst_); )
        {
            lst_ = c->next_;
            delete c;
        }
    }

    _TEGGO_MESSAGES_FAKE_INLINE
    MessageHub::MessageHub( SymbolTable* stable ) : listeners_(0), stable_(Refe(stable))
    {
        if ( !stable_ )
            stable_ = RccPtr(new SymbolTable());
    }

    _TEGGO_MESSAGES_FAKE_INLINE
    MessageHub::~MessageHub()
    {
        for ( MessageConnList_* l = 0; 0 != (l = listeners_); )
        {
            listeners_ = l->next_;
            delete l;
        }
    }

    _TEGGO_MESSAGES_FAKE_INLINE
    MessageConnector_* MessageHub::Connect_(pwide_t msgid,MessageConnector_ *conn)
    {
        pwide_t sid = stable_->GetSymbol(msgid);
        if ( !listeners_ )
            listeners_ = new MessageConnList_(sid);
        return listeners_->Insert(sid,conn);
    }
///

    _TEGGO_MESSAGES_FAKE_INLINE
    long Message_::ToLong() const
    {
        switch ( t_ )
        {
            case xF: return long(fval_);
            case xW: return teggo::ToLong(wval_);
            case xS: return teggo::ToLong(sval_);
            default :  return long(0);
        }
    }

    _TEGGO_MESSAGES_FAKE_INLINE
    double Message_::ToFloat() const
    {
        switch ( t_ )
        {
            case xL: return float(lval_);
            case xW: return teggo::ToFloat(wval_);
            case xS: return teggo::ToFloat(sval_);
            default :  return float(0);
        }
    }

    _TEGGO_MESSAGES_FAKE_INLINE
    pwide_t Message_::Str() const
    {
        typedef StringT<wchar_t,32> S;

        if ( tempo_ )
            return tempo_;

        switch ( t_ )
        {
            case xL:
                return tempo_ = widestrdup(+S(_S*L"%ld" %lval_));
            case xF:
                return tempo_ = widestrdup(+S(_S*L"%.3f" %fval_));
            case xP:
            case xR:
                return tempo_ = widestrdup(+S(_S*L"object:%08x" %pval_));
            case xJ:
                return tempo_ = widestrdup(+S(_S*L"%J" %gval_));
            case xW:
                return wval_?wval_:L"(null)";
            case xS:
                return tempo_ = widestrdup(sval_);
            default :
                return L"(null)";
        }
    }

    _TEGGO_MESSAGES_FAKE_INLINE
    bool Message_::IsNull() const
    {
        return  ( t_ == xW || t_ == xS || t_ == xP || t_ == xR ) ? 0 == pval_ : false;
    }

} // namespace teggo

#endif //___e1412894_dc33_4b47_b903_ec271d1c74ba___
