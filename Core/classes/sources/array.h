
/*

  Copyright (c) 2005-2009, Alexey Sudachen, alexey@sudachen.name

*/

#if !defined ___5DE79194_FAF5_4A63_A20E_C2C697196D32___
#define ___5DE79194_FAF5_4A63_A20E_C2C697196D32___

#include "_specific.h"
#include "ptr_mem.h"
#include "genericio.h"

namespace teggo
{
    template <class tTx, class tCpm = NonPodCopyStrategy, class tAlloc = DefaultAllocator>
    struct ArrayT : private tAlloc
    {

        ArrayT( tAlloc const& a = tAlloc() )
            : tAlloc(a)
        {
            buffer_end_ = end_ = begin_ = 0;
        }

        ArrayT( u32_t count, tTx const& filler = tTx(), tAlloc const& a = tAlloc() )
            : tAlloc(a)
        {
            buffer_end_ = end_ = begin_ = 0;
            Insert( end_, count, filler );
        }

        ArrayT( tTx const* iS, tTx const* iE, tAlloc const& a = tAlloc() )
            : tAlloc(a)
        {
            buffer_end_ = end_ = begin_ = 0;
            Insert( end_, iS, iE );
        }

        ArrayT( ArrayT<tTx,tCpm> const& a )
            : tAlloc(a)
        {
            buffer_end_ = end_ = begin_ = 0;
            Insert( end_, a.begin_, a.end_ );
        }

        ArrayT( ArrayT<tTx,tCpm>& a, swap_policy_t )
            : tAlloc(a)
        {
            buffer_end_ = end_ = begin_ = 0;
            a.Swap(*this);
        }

        ArrayT& operator = ( ArrayT<tTx,tCpm> const& a )
        {
            Purge();
            ArrayT<tTx,tCpm> temp( a );
            Swap(temp);
        }

        ~ArrayT()
        {
            Purge();
        }

        tTx* Begin()
        {
            return begin_;
        }

        tTx* End()
        {
            return end_;
        }

        tTx const* Begin() const
        {
            return begin_;
        }

        tTx const* End() const
        {
            return end_;
        }

        tTx& First()
        {
            return *begin_;
        }

        tTx& Last()
        {
            return *(end_-1);
        }

        tTx const& First() const
        {
            return *begin_;
        }

        tTx const& Last() const
        {
            return *(end_-1);
        }

        CXX_NO_INLINE
        void Resize( u32_t newCount, tTx const& filler = tTx(), ...)
        {
            u32_t count = Count();
            if ( newCount > count )
                Insert( end_, newCount - count, filler );
            else if ( newCount < count )
                Erase( begin_+newCount, end_ );
            else if ( !begin_ && !count )
                Reserve(1);
        }

        CXX_NO_INLINE
        void Reserve_( u32_t newCount, ... )
        {
            u32_t count = Count();
            if ( count )
            {
                ArrayT<tTx , tCpm> newBuffer;
                newBuffer.Reserve( newCount );

                //newBuffer.Insert( newBuffer.Begin(), begin_, end_ );
                for ( tTx* p = begin_; p != end_ ; ++p, ++newBuffer.end_ )
                    teggo_new_and_move_data(newBuffer.end_,p);

                Swap(newBuffer);
            }
            else
            {
                Purge();
                end_ = begin_ = (tTx*)tAlloc::Allocate((newCount+1) * sizeof(tTx));
                VIOLATION ( !begin_, L"'ArrayT<T>.Reserve_(newCount)' error: out of range" );
                buffer_end_ = begin_ + newCount;
            }
        }

        void Reserve( u32_t newCount )
        {
            if ( u32_t(buffer_end_-begin_) <= newCount )
                Reserve_ ( newCount );
            else if ( !begin_ && !newCount )
                Reserve_ ( 1 );
        }

        void Grow2_( u32_t gap, tTx*& pos )
        {
            if ( u32_t(buffer_end_-end_) < gap )
            {
                u32_t distance = pos - begin_;
                Reserve_( teggo_max<u32_t>(Count() + gap, Count() * 2 ) );
                pos = begin_ + distance;
            }
        }

        u32_t Capacity() const
        {
            return buffer_end_ - begin_;
        }

        u32_t Count() const
        {
            return end_ - begin_;
        }

        bool Empty() const
        {
            return 0 == Count();
        }

        operator bool() const
        {
            return !Empty();
        }

        tTx const* operator +() const
        {
            return Begin();
        }

        tTx const* operator ~() const { return operator+(); }

        tTx* operator +()
        {
            return Begin();
        }

        tTx* operator ~() { return operator+(); }

        tTx& operator [](long idx)
        {
            STRICT_VIOLATION(
                idx < 0 || idx >= end_-begin_,
                L"'ArrayT<T>.operator[](idx)' error: out of range" );
            return *(begin_ + idx);
        }

        tTx const& operator [](long idx) const
        {
            STRICT_VIOLATION(
                idx < 0 || idx >= end_-begin_,
                L"'ArrayT<T>.operator[](idx) const' error: out of range" );
            return *(begin_ + idx);
        }

        tTx const* operator ->() const
        {
            VIOLATION(
                end_-begin_ == 0,
                L"'ArrayT<T>.operator->() const' error: array empty" );
            return end_-1;
        }

        tTx* operator ->()
        {
            VIOLATION(
                end_-begin_ == 0,
                L"'ArrayT<T>.operator->()' error: array empty" );
            return end_-1;
        }

        CXX_NO_INLINE
        tTx& At(long idx)
        {
            VIOLATION(
                idx < 0 || idx >= end_-begin_,
                L"'ArrayT<T>.At(idx)' error: out of range" );
            return *(begin_ + idx);
        }

        CXX_NO_INLINE
        tTx const& At(long idx) const
        {
            VIOLATION(
                idx < 0 || idx >= end_-begin_,
                L"'ArrayT<T>.At(idx) const' error: out of range" );
            return *(begin_ + idx);
        }

        void Clear()
        {
            Erase( begin_, end_ );
        }

        void Purge()
        {
            Clear();
            if ( begin_ )
            {
                tAlloc::Deallocate(begin_);
                begin_ = end_ = buffer_end_ = 0;
            }
        }

        void Swap(ArrayT<tTx,tCpm>& a)
        {
            //teggo_swap( (tAlloc&)*this, (tAlloc&)a ); ???
            teggo_swap( begin_, a.begin_ );
            teggo_swap( end_, a.end_ );
            teggo_swap( buffer_end_, a.buffer_end_ );
        }

        CXX_NO_INLINE
        tTx* Insert( tTx* pos, tTx const& val = tTx() )
        {
            if ( !pos ) pos = Begin();
            Grow2_( 1, pos );
            STRICT_VIOLATION(
                pos > buffer_end_ || pos < begin_,
                L"'ArrayT<T>.Insert(*pos,&val)' error: out of range" );
            STRICT_VIOLATION(
                &val < buffer_end_ && &val >= begin_,
                L"'ArrayT<T>.Insert(*pos,&val)' error: inserting from self" );
            tCpm::MoveRight( pos, end_, 1, val );
            return pos;
        }

        CXX_NO_INLINE
        tTx* Insert( tTx* pos, u32_t n, tTx const& val, ... )
        {
            if ( !pos ) pos = Begin();
            Grow2_( n , pos );
            STRICT_VIOLATION(
                pos+n > buffer_end_ || pos < begin_,
                L"'ArrayT<T>.Insert(*pos,n,&val)' error: out of range" );
            STRICT_VIOLATION(
                &val < buffer_end_ && &val >= begin_,
                L"'ArrayT<T>.Insert(*pos,&val)' error: inserting from self" );
            tCpm::MoveRight( pos, end_, n, val );
            return pos;
        }

        CXX_NO_INLINE
        tTx* Insert( tTx* pos, tTx const* iS, tTx const* iE, ... )
        {
            if ( !pos ) pos = Begin();
            Grow2_( iE - iS, pos );
            STRICT_VIOLATION(
                iE < iS || pos+(iE-iS) > buffer_end_ || pos < begin_ ,
                L"'ArrayT<T>.Insert(*pos,*iS,*iE)' error: out of range" );
            STRICT_VIOLATION(
                iS < buffer_end_ && iS >= begin_,
                L"'ArrayT<T>.Insert(*pos,*iS,*iE)' error: inserting from self" );
            tCpm::MoveRight( pos, end_, (iE - iS), iS );
            return pos;
        }

        CXX_NO_INLINE
        tTx* Insert( tTx* pos, tTx const* iS, int n, ... )
        {
            return Insert( pos, iS, iS+n );
        }

        CXX_NO_INLINE
        tTx* Erase( tTx* pos )
        {
            if ( !pos ) pos = Begin();
            STRICT_VIOLATION(
                pos >= end_ || pos < begin_ ,
                L"'ArrayT<T>.Erase(*pos)' error: out of range" );
            tCpm::MoveLeft( pos, end_, 1 );
            return pos;
        }

        CXX_NO_INLINE
        tTx* Erase( tTx* iS, tTx* iE, ... )
        {
            STRICT_VIOLATION(
                iE < iS || iE > end_ || iS < begin_ ,
                L"'ArrayT<T>.Erase(*iS,*iE)' error: out of range" );
            tCpm::MoveLeft( iS, end_, (iE-iS) );
            return iS;
        }

        CXX_NO_INLINE
        tTx* Erase( tTx* pos, int n, ... )
        {
            if ( !pos ) pos = Begin();
            return Erase( pos, pos+n );
        }

        tTx* Push( tTx const& val = tTx(), ... )
        {
            return Insert( end_, val );
        }

        tTx* Append( tTx const& val = tTx(), ... )
        {
            return Insert( end_, val );
        }

        void Pop()
        {
            Erase( end_-1 );
        }

        tTx& Top()
        {
            return Last();
        }

        tTx const& Top() const
        {
            return Last();
        }

        tTx* PushFront( tTx const& val = tTx() )
        {
            return Insert( begin_, val );
        }

        void PopFront()
        {
            Erase( begin_ );
        }

        /*
          linear find
        */

        template <class tTr, class tTy, class tTequal>
        tTr Find( tTy const& key, tTequal eq, tTr ) const
        {
            tTr p = linear_find(begin_,end_,key,eq);
            return p != end_ ? (tTr)p : (tTr)0;
        }

        template <class tTy, class tTequal>
        tTx* Find( tTy const& key, tTequal eq )
        {
            return Find( key, eq, (tTx*)0 );
        }

        template <class tTy, class tTequal>
        tTx const* Find( tTy const& key, tTequal eq ) const
        {
            return Find( key, eq, (tTx const*)0 );
        }

        template <class tTy>
        tTx* Find( tTy const& key )
        {
            return Find( key, Equal() );
        }

        template <class tTy>
        tTx const* Find( tTy const& key ) const
        {
            return Find( key, Equal() );
        }

        /*
          binary lower
        */

        template <class tTy, class tTless>
        tTx const* Blower( tTy const& key, tTless lt ) const
        {
            return binary_lower(begin_,end_,key,lt);
        }

        template <class tTy>
        tTx const* Blower( tTy const& key ) const
        {
            return Blower(key, Less());
        }

        template <class tTy, class tTless>
        tTx* Blower( tTy const& key, tTless lt )
        {
            return binary_lower(begin_,end_,key,lt);
        }

        template <class tTy>
        tTx* Blower( tTy const& key )
        {
            return Blower(key,Less());
        }

        /*
          binary upper
        */

        template <class tTy, class tTless>
        tTx const* Bupper( tTy const& key, tTless lt ) const
        {
            return binary_upper(begin_,end_,key,lt);
        }

        template <class tTy>
        tTx const* Bupper( tTy const& key ) const
        {
            return Bupper(key, Less());
        }

        template <class tTy, class tTless>
        tTx* Bupper( tTy const& key, tTless lt )
        {
            return binary_upper(begin_,end_,key,lt);
        }

        template <class tTy>
        tTx* Bupper( tTy const& key )
        {
            return Bupper(key,Less());
        }

        /*
          binary find
        */

        template <class tTy, class tTless>
        tTx const* Bfind_( tTy const& key, tTless lt ) const
        {
            tTx const* p = binary_lower(begin_,end_,key,lt);
            return ( p != end_ && !lt(key,*p) ) ? p : 0;
        }

        template <class tTy, class tTless>
        tTx* Bfind( tTy const& key, tTless lt )
        {
            return const_cast<tTx*>(Bfind_( key,lt));
        }

        template <class tTy, class tTless>
        tTx const* Bfind( tTy const& key, tTless lt ) const
        {
            return Bfind_( key,lt);
        }

        template <class tTy>
        tTx* Bfind( tTy const& key )
        {
            return Bfind( key, Less() );
        }

        template <class tTy>
        tTx const* Bfind( tTy const& key ) const
        {
            return Bfind( key, Less() );
        }

        /*
          binary insert
        */

        template <class tTless>
        tTx* Binsert( tTx const& val, tTless lt )
        {
            return Insert(binary_lower( begin_, end_, val, lt ),val);
        }

        tTx* Binsert( tTx const& val ) { return Binsert( val, Less() ); }

        template <class tTless>
        tTx* BinsertOne( tTx const& val, tTless lt, bool _Replace = true )
        {
            tTx* p = binary_lower( begin_, end_, val, lt );
            if ( p != end_ && !lt(val,*p) )
                if ( _Replace )
                    *p = val;
                else
                    return 0;
            else
                p = Insert(p,val);
            return p;
        }

        tTx* BinsertOne( tTx const& val, bool _Replace = true )
        {
            return BinsertOne( val, Less(), _Replace );
        }

        template <class tTless>
        tTx* BinsertOneNoReplace( tTx const& val, tTless lt )
        {
            return BinsertOne( val, lt, false );
        }

        tTx* BinsertOneNoReplace ( tTx const& val )
        {
            return BinsertOneNoReplace( val, Less() );
        }

        /*
          binary erase
        */

        template <class tTy, class tTless>
        bool Berase( tTy key, tTless lt )
        {
            if ( tTx* p = Bfind(key, lt) )
                return Erase( p ), true;
            else
                return false;
        }

        bool Berase( tTx const& val )
        {
            return Berase( val, Less() );
        }

        typedef tTx ValueType;
        typedef tTx& ValueRef;
        typedef tTx* ValuePtr;
        typedef tTx const& ValueCref;
        typedef tTx const* ValueCptr;

        // STL compatibility

        typedef tTx* iterator;
        typedef tTx const* const_iterator;

        tTx* begin() { return Begin(); }
        tTx* end() { return End(); }
        tTx& front() { return First(); }
        tTx& back() { return Last(); }
        tTx const* begin() const { return Begin(); }
        tTx const* end() const { return End(); }
        tTx const& front() const { return First(); }
        tTx const& back() const { return Last(); }
        unsigned size() const { return Count(); }
        tTx* push_back( tTx const& tx ) { return Push(tx); }
        void pop_back() { Pop(); }
        void swap( ArrayT& a ) { Swap(a); }
        void resize(unsigned sz, tTx const& tx = tTx()) { Resize(sz,tx); }
        void reserve(unsigned sz) { Reserve(sz); }
        void clear() { Clear(); }
        bool empty() const { return Empty(); }
        tTx* insert( tTx* x, tTx const& tx = tTx() ) { return Insert(x,tx); }
        tTx* insert( tTx* x, tTx const* b, tTx const* e) { return Insert(x,b,e); }
        void erase( tTx* x ) { Erase(x); }

        struct Untransfomed  { tTx operator() (tTx a) const { return a; } };
        struct E {};

        template <class tTransform>
        tTx const* Search( tTx const* patt, unsigned pattL, tTx const* p, tTransform transformation, E* = 0 ) const
        {
            if ( !p ) p = Begin();
            STRICT_VIOLATION(
                p > buffer_end_ || p < begin_,
                L"'ArrayT<T>.Search(patt,pattL,*P)' error: P is out of range" );
            for ( ; unsigned(end_-p) >= pattL; ++p )
                if ( transformation(*p) == transformation(*patt) )
                {
                    int i = 0;
                    for ( ; i < pattL; ++i )
                        if ( transformation(patt[i]) != transformation(p[i]) )
                            break;
                    if (i == pattL) return p;
                }
            return 0;
        }

        template <class tTransform>
        tTx* Search( tTx const* patt, unsigned pattL, tTx* p, tTransform transformation )
        {
            return const_cast<tTx*>(Search(patt,pattL,p,transformation,0));
        }

        template <class tTransform>
        tTx const* Search( tTx const* patt, unsigned pattL, tTransform transformation, E* = 0 ) const
        {
            return Search(patt, pattL, Begin(), transformation);
        }

        template <class tTransform>
        tTx* Search( tTx const* patt, unsigned pattL, tTransform transformation )
        {
            return const_cast<tTx*>(Search(patt,pattL,transformation,0));
        }

        template <class tTransform>
        void Replace( tTx const* patt, unsigned pattL, tTx const* val, unsigned valL, tTransform transformation )
        {
            for ( tTx* p = Search(patt,pattL,transformation); p; p = Search(patt,pattL,p,transformation) )
            {
                p = Erase(p,pattL);
                p = Insert(p,val,valL);
            }
        }

        void Replace( tTx const* patt, unsigned pattL, tTx const* val, unsigned valL)
        { return Replace(patt,pattL,val,valL,Untransfomed()); }

        bool Contains( tTx const& val ) const
        {
            return Find(val)!=0;
        }

        //private:

        tTx* begin_;
        tTx* end_;
        tTx* buffer_end_;
    };


    template <class tTx>
    struct BufferT : ArrayT <tTx, PodCopyStrategy, DefaultAllocator>
    {
        typedef ArrayT <tTx, PodCopyStrategy, teggo::DefaultAllocator> Super;

        BufferT()
        {
        }

        BufferT( u32_t count, tTx const& filler = tTx() )
            : Super( count, filler )
        {
        }

        BufferT( tTx const* iS, tTx const* iE )
            : Super( iS, iE )
        {
        }

        typedef tTx ValueType;
        typedef tTx& ValueRef;
        typedef tTx* ValuePtr;
        typedef tTx const& ValueCref;
        typedef tTx const* ValueCptr;

        void Truncate() { Super::end_ = Super::begin_; }
        void Clear() { Truncate(); }
    };

    typedef BufferT<char> Cbuffer;
    typedef BufferT<wchar_t> Wbuffer;


    template <class tTc>
    struct BufferTwriterT : GenericWriterT<tTc>
    {
        typedef GenericWriterT<tTc> B_;
        BufferT<tTc>& out_;
        BufferTwriterT(BufferT<tTc>& out) : GenericWriterT<tTc>(true), out_(out)
        {
            Setup(out_.Begin(),out_.End(),out_.End());
        }
        /*override*/ bool Require(unsigned cnt)
        {
            STRICT_REQUIRE( B_::outE == out_.end_ );
            STRICT_REQUIRE( B_::outS == out_.begin_ );
            STRICT_REQUIRE( B_::outI <= out_.end_ );
            STRICT_REQUIRE( B_::outI >= out_.begin_ );

            if ( B_::outI + cnt > B_::outE )
            {
                if ( out_.buffer_end_ >= B_::outI+cnt )
                    B_::outE = out_.end_ = B_::outI+cnt;
                else
                {
                    out_.Resize( (B_::outI-B_::outS) + (cnt - (B_::outE-B_::outI)) );
                    B_::outI = out_.Begin() + (B_::outI-B_::outS);
                    B_::outS = out_.Begin();
                    B_::outE = out_.End();
                }
            }
            return true;
        }
    };

}

template <class tTx, class tCmp, class tAlloc> inline
void CXX_STDCALL teggo_new_and_move_data (
    teggo::ArrayT<tTx,tCmp,tAlloc>* to, teggo::ArrayT<tTx,tCmp,tAlloc>* from )
{
    (new (to) teggo::ArrayT<tTx,tCmp,tAlloc>())->Swap(*from);
}

template <class tTx, class tCmp, class tAlloc> inline
void CXX_STDCALL teggo_swap (
    teggo::ArrayT<tTx,tCmp,tAlloc>& to, teggo::ArrayT<tTx,tCmp,tAlloc>& from )
{
    to.Swap(from);
}

template <class tTx> inline
void CXX_STDCALL teggo_new_and_move_data (
    teggo::BufferT<tTx>* to, teggo::BufferT<tTx>* from )
{
    (new (to) teggo::BufferT<tTx>())->Swap(*from);
}
template <class tTx> inline
void CXX_STDCALL teggo_swap ( teggo::BufferT<tTx>& to, teggo::BufferT<tTx>& from )
{
    to.Swap(from);
}

#endif //___5DE79194_FAF5_4A63_A20E_C2C697196D32___
