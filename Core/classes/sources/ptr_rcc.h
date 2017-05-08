
/*

  Copyright (c) 2005-2009, Alexey Sudachen, alexey@sudachen.name

*/

#if !defined __A53291A1_6C0C_4042_99CA_7B59EAC9C547__
#define __A53291A1_6C0C_4042_99CA_7B59EAC9C547__

#include "_specific.h"
#include "refcounted.h"

template<class tTx> struct TeggoRccTaits
{
    struct Refcounter
    {
        static void AddRef(tTx* p) { if (p) p->AddRef(); }
        static void Release(tTx* p) { if (p) p->Release(); }
    };
};

namespace teggo
{

    template <class tTx> inline
    tTx* CXX_STDCALL Refe(tTx* ref)
    { TeggoRccTaits<tTx>::Refcounter::AddRef(ref); return ref; }
    template <class tTx> inline
    void CXX_STDCALL Unrefe(tTx*& ref)
    { TeggoRccTaits<tTx>::Refcounter::Release(ref); ref = 0; }

    template<class tTx = Ireferred>
    struct rcc_ptr
    {
        mutable tTx* ref_;

        typedef rcc_ptr<tTx> const& Ref;

        explicit rcc_ptr(tTx* t=0) : ref_(t)
        {
        }

        explicit rcc_ptr(tTx* t,bool addref) : ref_(t)
        {
            if (addref)
                Refe(ref_);
        }

        rcc_ptr(const rcc_ptr<tTx>& a)
        {
            ref_ = Refe(a.ref_);
        }

        ~rcc_ptr()
        {
            Unrefe(ref_);
        }

        bool operator !() const
        {
            return !ref_;
        }

        operator bool() const
        {
            return ref_!=0;
        }

        tTx& operator *() const
        {
            return *ref_;
        }

        tTx* operator ->() const
        {
            return ref_;
        }

        const rcc_ptr& operator=(const rcc_ptr& a)
        {
            reset_(Refe(a.ref_));
            return *this;
        }

        bool operator == ( const rcc_ptr& a ) const
        {
            return ref_ == a.ref_;
        }

        bool operator != ( const rcc_ptr& a ) const
        {
            return ref_ != a.ref_;
        }

        bool operator < ( const rcc_ptr& a ) const
        {
            return ref_ < a.ref_;
        }

        bool operator > ( const rcc_ptr& a ) const
        {
            return ref_ > a.ref_;
        }

        tTx*& operator + () const
        {
            return  ref_;
        }

        void reset_(tTx* t)
        {
            Unrefe(ref_);
            ref_ = t;
        }

        tTx* forget_()
        {
            tTx* t=ref_;
            ref_ = 0;
            return t;
        }

        void Swap( rcc_ptr& p )
        {
            teggo_swap( p.ref_, ref_ );
        }

    private:
        void operator=(const tTx*);
    };

    template <class tTx> inline
    tTx* CXX_STDCALL Refe(const rcc_ptr<tTx>& ref)
    {
        TeggoRccTaits<tTx>::Refcounter::AddRef(ref.ref_);
        return ref.ref_;
    }

    template <class tTx> inline
    void CXX_STDCALL Unrefe(rcc_ptr<tTx>& ref) { ref.reset_((tTx*)0); }
    template <class tTx> inline
    tTx* CXX_STDCALL Forget(rcc_ptr<tTx>& ref) { return ref.forget_(); }
    template <class tTx> inline
    void CXX_STDCALL Reset(rcc_ptr<tTx>& ref,tTx* p) { ref.reset_(p); }
    template <class tTx> inline
    rcc_ptr<tTx> CXX_STDCALL RccPtr(tTx* ref) { return rcc_ptr<tTx>(ref); }
    template <class tTx> inline
    rcc_ptr<tTx> CXX_STDCALL RccRefe(tTx* ref) { return rcc_ptr<tTx>(Refe(ref)); }

} // namespace

template <class tTx> inline
void CXX_STDCALL teggo_new_and_move_data (
    teggo::rcc_ptr<tTx>* to, teggo::rcc_ptr<tTx>* from )
{
    (new (to) teggo::rcc_ptr<tTx>())->Swap(*from);
}

template <class tTx> inline
void CXX_STDCALL teggo_swap ( teggo::rcc_ptr<tTx>& to, teggo::rcc_ptr<tTx>& from )
{
    to.Swap(from);
}

template <class tTx>
teggo::rcc_ptr<tTx> rcc_cast( teggo::rcc_ptr<> const& p )
{
    return p
           ? teggo::RccRefe( (tTx*) p->QueryInterface( *teggo_guidof((tTx*)0) ) )
           : teggo::rcc_ptr<tTx>(0);
}

#endif // __A53291A1_6C0C_4042_99CA_7B59EAC9C547__
