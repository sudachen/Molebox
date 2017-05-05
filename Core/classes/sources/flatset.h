
/*

  Copyright (c) 2005-2009, Alexey Sudachen, alexey@sudachen.name

*/

#if !defined ___5CFD68C2_325C_4F82_8B6B_A51E1FD6349E___
#define ___5CFD68C2_325C_4F82_8B6B_A51E1FD6349E___

#include "_specific.h"
#include "array.h"

namespace teggo
{

  template < class tTset >
    struct FlatsetIteratorT;
  template < class tTset >
    FlatsetIteratorT<tTset> CXX_STDCALL FlatsetIteratorT_IterateFlatset(tTset const&);
  template < class tTset >
    FlatsetIteratorT<tTset> CXX_STDCALL FlatsetIteratorT_IterateFlatset(tTset &);

  template < class tValue, class tTcmp = Less >
    struct FlatsetT
      {
        typedef tTcmp   ValueComparer;
        typedef tValue  ValueType;

        typedef ArrayT<ValueType> Table;
        Table table_;

        void Put(ValueType const &k)
          {
            table_.BinsertOne( k, ValueComparer() );
          }

        ValueType* Get(ValueType const &k) const
          {
            return (ValueType*)table_.Bfind( k, ValueComparer() );
          }

        void Erase(ValueType const &k)
          {
            table_.Berase( k, ValueComparer() );
          }

        void Reserve( unsigned count )
          {
            table_.Reserve(count);
          }

        typedef FlatsetIteratorT< FlatsetT<tValue,tTcmp> > Iterator;

        Iterator Iterate() const
          {
            return FlatsetIteratorT_IterateFlatset(*this);
          }

        Iterator Iterate()
          {
            return FlatsetIteratorT_IterateFlatset(*this);
          }

        unsigned Count() const
          {
            return table_.Count();
          }

        unsigned Empty() const
          {
            return Count() == 0;
          }

        void Swap( FlatsetT &s )
          {
            table_.Swap(s.table_);
          }

        void Clear()
          {
            table_.Clear();
          }

        void Purge()
          {
            table_.Purge();
          }

        FlatsetT() {}

      private:

        FlatsetT(FlatsetT const&);
        FlatsetT& operator =(FlatsetT const&);

      };

  template < class tTset >
    struct FlatsetIteratorT
      {
        int i_;
        tTset& map_;

        bool Next()
          {
            if ( i_+1 >= signed( map_.Count() ) )
              return false;
            else
              return ++i_, true;
          }

        typename tTset::ValueType& Value() const
          {
            return map_.table_[i_];
          }

        typename tTset::ValueType& operator*() const
          {
            return Value();
          }

        typename tTset::ValueType* operator->() const
          {
            return &Value();
          }

        FlatsetIteratorT(tTset &m) : i_(-1), map_(m)
          {
          }
      };

  template < class tTset >
    FlatsetIteratorT<tTset> CXX_STDCALL FlatsetIteratorT_IterateFlatset(tTset const& m)
      {
        return FlatsetIteratorT<tTset>(m);
      }

  template < class tTset >
    FlatsetIteratorT<tTset> CXX_STDCALL FlatsetIteratorT_IterateFlatset(tTset &m)
      {
        return FlatsetIteratorT<tTset>(m);
      }
}

template < class tTx, class tTy > inline
  void CXX_STDCALL teggo_swap(teggo::FlatsetT<tTx,tTy> &s0, teggo::FlatsetT<tTx,tTy> &s1)
    {
      s0.Swap(s1);
    }

#endif // ___5CFD68C2_325C_4F82_8B6B_A51E1FD6349E___
