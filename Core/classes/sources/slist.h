
/*

  Copyright (c) 2005-2009, Alexey Sudachen, alexey@sudachen.name

*/

#if !defined ___4EED72FC_07F5_4C06_8DF5_C705EC903950___
#define ___4EED72FC_07F5_4C06_8DF5_C705EC903950___

#include "_specific.h"
#include "ptr_mem.h"

namespace teggo
{

  template < unsigned tFake >
    struct SlistT_Position_End
      {
        static void const * const lstEnd_;
      };

  template < unsigned tFake >
    void const * const SlistT_Position_End<tFake>::lstEnd_ = 0;

  template < class tPtr >
    struct SlistT_Position : SlistT_Position_End<0>
      {
        explicit SlistT_Position(tPtr e) : elm_ (e)
          {
            STRICT_REQUIRE( elm_ != 0 );
          }

        SlistT_Position operator + ( int distance );

        SlistT_Position operator ++ ()
          {
            *this = *this + 1;
            return *this;
          }

        SlistT_Position operator ++ (int)
          {
            SlistT_Position p = *this;
            *this = *this + 1;
            return p;
          }

        bool SafeNext()
          {
            if ( IsValid() )
              {
                operator ++();
                return IsValid();
              }
            else
              return false;
          }

        bool operator == ( const SlistT_Position& i ) const
          {
            return *elm_ == *i.elm_ ;
          }

        bool operator != ( const SlistT_Position& i ) const
          {
            return !operator==(i);
          }

        bool IsValid() const
          {
            return *elm_ != 0;
          }

        operator bool() const
          {
            return IsValid();
          }

        tPtr operator +() { return elm_; }
        tPtr elm_;
      };

  template < class tPtr > CXX_NO_INLINE
    SlistT_Position<tPtr> SlistT_Position<tPtr>::operator + ( int distance )
      {
        STRICT_REQUIRE( distance >= 0 );
        STRICT_REQUIRE( distance > 0 || elm_ != 0 );
        tPtr e = elm_;

        while ( distance-- )
          {
            STRICT_REQUIRE( e != 0  );
            e = &(*e)->next;
            if ( !(*e) )
            {
              e = (tPtr)&lstEnd_;
              break; // end reachid
            }
          }

        return SlistT_Position<tPtr>(e);
      }

  template < class tTx >
    struct SlistT_Element
      {
        SlistT_Element *next;
        tTx value;
      };

  template < class tTx >
    struct SlistT_ConstPosition : SlistT_Position<SlistT_Element<tTx> const* const*>
      {
        typedef SlistT_Position<SlistT_Element<tTx> const* const*> _Super;
        typedef SlistT_Element<tTx> const* const* tPtr;

        SlistT_ConstPosition(tPtr p) : SlistT_Position<tPtr>(p)
          {
          }

        SlistT_ConstPosition()
          : SlistT_Position<tPtr>((tPtr)&SlistT_Position_End<0>::lstEnd_)
          {
          }

        tTx const* operator->() const
          {
            STRICT_REQUIRE( _Super::elm_ != 0 && *_Super::elm_ != 0 );
            return &(*_Super::elm_)->value;
          }

        tTx const& operator *() const
          {
            return * operator->();
          }
      };


  template < class tTx >
    struct SlistT_NoconstPosition : SlistT_Position<SlistT_Element<tTx>**>
      {
        typedef SlistT_Position<SlistT_Element<tTx>**> _Super;
        typedef SlistT_Element<tTx> **tPtr;

        SlistT_NoconstPosition(tPtr p)
          : SlistT_Position<tPtr>(p)
          {
          }

        SlistT_NoconstPosition()
          : SlistT_Position<tPtr>((tPtr)&SlistT_Position_End<0>::lstEnd_)
          {
          }

        tTx* operator->() const
          {
            STRICT_REQUIRE( _Super::elm_ != 0 && *_Super::elm_ != 0 );
            return &(*_Super::elm_)->value;
          }

        tTx& operator *() const
          {
            return * operator->();
          }
      };

  template
    <
      class tTx ,
      class tAlloc = DefaultAllocator
    >
    struct SlistT : private tAlloc
      {
        typedef SlistT<tTx,tAlloc> Self;

        typedef SlistT_Element<tTx> Element;
        typedef SlistT_ConstPosition<tTx> ConstPosition;
        typedef SlistT_NoconstPosition<tTx> Position;

        SlistT( tAlloc const &a = tAlloc() )
          : tAlloc(a), begin_(0), pend_(&begin_), count_(0)
          {
          };

        SlistT( SlistT const &s)
          : tAlloc(s), begin_(0), pend_(&begin_), count_(0)
          {
            ConstPosition p = s.Begin();
            while ( p ) Append( *p );
          };

        SlistT& operator = ( SlistT const &s )
          {
            SlistT temp ( s );
            Swap( temp );
          }

        ~SlistT()
          {
            Purge();
          };

        u32_t Count() const
          {
            return count_;
          }

        bool Empty() const
          {
            return 0 == Count();
          }

        operator bool() const
          {
            return !Empty();
          }

        void Clear()
          {
            while ( !Empty() )
              Pop();
          }

        void Purge()
          {
            Clear();
          }

        CXX_NO_INLINE
        void* RawInsert( Position pos)
          {
            if ( ! *+pos  )
              pos = Position(pend_);

            Element *e = (Element*)tAlloc::Allocate(sizeof(Element));
            e->next = *+pos;
            ++count_;
            *+pos = e;

            if ( !e->next )
              {
                STRICT_REQUIRE( +pos == pend_ );
                pend_ = &e->next;
              }

            return &e->value;
          }

        tTx* Insert( Position pos, tTx const &val = tTx() )
          {
            void* rawval = RawInsert(pos);
            CXX_TRY
              {
                return new (rawval) tTx(val);
              }
            CXX_CATCH_ALL
              {
                RawErase(pos);
                CXX_RETHROW;
              }
          }

        CXX_NO_INLINE
        void RawErase(Position pos)
          {
            STRICT_REQUIRE( count_ > 0 );

            Element *e = *+pos;
            *+pos = e->next;
            --count_;
            if ( !e->next )
              {
                STRICT_REQUIRE( &e->next == pend_ );
                pend_ = +pos;
              }
            tAlloc::Deallocate(e);
          }

        void Erase(Position pos)
          {
            STRICT_REQUIRE ( *+pos );
            (*+pos)->value.~tTx();
            RawErase(pos);
          }

        tTx const& Last() const
          {
            STRICT_REQUIRE(!Empty());
            return ((Element*)pend_)->value;
          }

        tTx& Last()
          {
            return (tTx&)((Self const*)this)->Last();
          }

        tTx const& First() const
          {
            STRICT_REQUIRE(!Empty());
            return begin_->value;
          }

        tTx& First()
          {
            return (tTx&)((Self const*)this)->First();
          }

        template < class tTcmp > CXX_NO_INLINE
          ConstPosition Find( tTx const &val, tTcmp cmp ) const
            {
              ConstPosition p = Begin();
              for ( ;p ; ++p )
                if ( cmp(*p,val) )
                  return p;
              return ConstPosition();
            }

        ConstPosition Find( tTx const &val ) const
          {
            return Find( val, Equal() );
          }

        template < class tTcmp >
          Position Find( tTx const &val, tTcmp cmp )
            {
              return Position( (Element**) +((Self const*)this)->Find(val,cmp) );
            }

        Position Find( tTx const &val )
          {
            return Find( val, Equal() );
          }

        template < class tTpred > CXX_NO_INLINE
          ConstPosition FindIf( tTpred pred ) const
            {
              ConstPosition p = Begin();
              for ( ; p; ++p )
                if ( pred(*p) )
                  return p;
              return ConstPosition();
            }

        template < class tTpred >
          Position FindIf( tTpred pred )
            {
              return Position( (Element**) +((Self const*)this)->FindIf(pred) );
            }

        ConstPosition Begin() const
          {
            if ( !Empty() )
              return ConstPosition(&begin_);
            else
              return End();
          }

        Position Begin()
          {
            return Position( (Element**) +((Self const*)this)->Begin() );
          }

        ConstPosition End() const
          {
            return ConstPosition();
          }

        Position End()
          {
            return Position( (Element**) +((Self const*)this)->End() );
          }

        tTx* Append(tTx const &val = tTx() )
          {
            return Insert(End(),val);
          }

        tTx* Push(tTx const &val = tTx() )
          {
            return Insert(Begin(),val);
          }

        void Pop()
          {
            STRICT_REQUIRE(!Empty());
            Erase(Begin());
          }

        tTx& Top()
          {
            STRICT_REQUIRE(!Empty());
            return First();
          }

        tTx const& Top() const
          {
            STRICT_REQUIRE(!Empty());
            return First();
          }

        void Swap( SlistT &s )
          {
            if ( s.pend_ == &s.begin_ )
              s.pend_ = &begin_;
            if ( pend_ == &begin_ )
              pend_ = &s.begin_;
            teggo_swap(pend_,s.pend_);
            teggo_swap(begin_,s.begin_);
            teggo_swap(count_,s.count_);
          }

        typedef tTx ValueType;
        typedef tTx &ValueRef;
        typedef tTx *ValuePtr;
        typedef tTx const &ValueCref;
        typedef tTx const *ValueCptr;

      private:

        Element *begin_;
        Element **pend_;
        u32_t  count_;

      };

} // namespace

template < class tTx, class tAlloc > inline
  void CXX_STDCALL teggo_new_and_move_data (
    teggo::SlistT<tTx,tAlloc> *to, teggo::SlistT<tTx,tAlloc> *from )
    {
      (new (to) teggo::SlistT<tTx,tAlloc>())->Swap(*from);
    }

template < class tTx > inline
  void CXX_STDCALL teggo_swap ( teggo::SlistT<tTx> &to, teggo::SlistT<tTx> &from )
    {
      to.Swap(from);
    }

#endif // ___4EED72FC_07F5_4C06_8DF5_C705EC903950___
