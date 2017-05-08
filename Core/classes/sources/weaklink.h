
/*

  Copyright (c) 2005-2009, Alexey Sudachen, alexey@sudachen.name

*/

#ifndef ___4222b811_f8b1_4b66_8347_450be61c74a9___
#define ___4222b811_f8b1_4b66_8347_450be61c74a9___

#include "_specific.h"

namespace teggo
{

    template <class tSelf, class tBase = teggo::EmptyStruct>
    struct InternList : tBase
    {
        tSelf*  next;
        tSelf** pnext;
        InternList() : next(0), pnext(0) {}
        ~InternList() { LinkOut(); }
        void LinkIn(tSelf** l)
        {
            next = *l;
            if ( next )
                next->pnext = &next;
            pnext = l;
            *l = (tSelf*)this;
        }
        void LinkOut()
        {
            if ( next ) next->pnext = pnext;
            if ( pnext ) *pnext = next;
            pnext = 0;
            next = 0;
        }

        static void UnlinkAll(tSelf** l)
        {
            while ( *l )
                (*l)->LinkOut();
        }

    private:
        InternList(InternList const&);
        InternList& operator=(InternList const&);
    };

    template <class tTx>
    struct WeakLink : InternList<WeakLink<tTx>>
    {
        tTx obj;

        void LinkIn(WeakLink** l, tTx o)
        {
            obj = o;
            InternList<WeakLink<tTx>>::LinkIn(l);
        }

        void LinkOut()
        {
            obj = 0;
            InternList<WeakLink<tTx>>::LinkOut();
        }

        tTx operator->() const { return obj; }
        operator bool() const { return !!obj; }
        bool operator!() const { return !obj; }
        tTx& operator+() { return obj; }

        WeakLink() : obj(0) {}
        ~WeakLink() { obj = 0; }

        void Swap( WeakLink& p )
        {
            WeakLink* self_next = this->next;
            WeakLink* other_next = p.next;
            WeakLink** self_pnext = this->pnext;
            WeakLink** other_pnext = p.pnext;

            if ( !!(p.next = self_next) )
                self_next->pnext = &p.next;
            if ( !!(p.pnext = self_pnext) )
                *self_pnext = &p;

            if ( !!(this->next = other_next) )
                other_next->pnext = &this->next;
            if ( !!(this->pnext = other_pnext) )
                *other_pnext = this;

            teggo_swap( p.obj, this->obj );
        }
    private:
        WeakLink(WeakLink& a);
        WeakLink& operator=(WeakLink const& a);
    };

    template <class tTx>
    struct WeakLinkHolder
    {
        WeakLink<tTx>* links;
        WeakLinkHolder()
            : links(0)
        {
        }
        ~WeakLinkHolder()
        {
            UnlinkAll();
        }
        void Link( WeakLink<tTx>* l, tTx o )
        {
            l->LinkIn(&links,o);
        }
        void UnlinkAll()
        {
            WeakLink<tTx>::UnlinkAll(&links);
        }
    private:
        WeakLinkHolder(WeakLinkHolder const&);
        WeakLinkHolder& operator=(WeakLinkHolder const&);
    };

} // end namespace

template <class tTx> inline
void CXX_STDCALL teggo_new_and_move_data (
    teggo::WeakLink<tTx>* to, teggo::WeakLink<tTx>* from )
{
    (new (to) teggo::WeakLink<tTx>())->Swap(*from);
}

template <class tTx> inline
void CXX_STDCALL teggo_swap ( teggo::WeakLink<tTx>& to, teggo::WeakLink<tTx>& from )
{
    to.Swap(from);
}

#endif //___4222b811_f8b1_4b66_8347_450be61c74a9___
