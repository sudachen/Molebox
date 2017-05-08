
/*

  Copyright (c) 2005-2009, Alexey Sudachen, alexey@sudachen.name

*/


#if !defined ___F32DDF71_33DD_47F8_ABEC_38C3D9052D8C___
#define ___F32DDF71_33DD_47F8_ABEC_38C3D9052D8C___

#include "_specific.h"
#include "array.h"

namespace teggo
{

    template <class tTmap>
    struct FlatmapIteratorT;

    template <class tTmap>
    FlatmapIteratorT<tTmap> CXX_STDCALL FlatmapIteratorT_IterateFlatmap(tTmap const&);

    template <class tTmap>
    FlatmapIteratorT<tTmap> CXX_STDCALL FlatmapIteratorT_IterateFlatmap(tTmap&);

    template <class tKey, class tValue, class tTcmp = Less>
    struct FlatmapT
    {
        typedef tTcmp   KeyComparer;
        typedef tKey    KeyType;
        typedef tValue  ValueType;

        struct Record
        {
            KeyType key;
            ValueType value;
            Record(KeyType k, ValueType v) : key(k),value(v) {}
        };

        struct Comparer
        {
            bool operator ()( Record const& a, KeyType const& b ) const
            {
                return KeyComparer()(a.key,b);
            }
            bool operator ()( KeyType const& b, Record const& a ) const
            {
                return KeyComparer()(b,a.key);
            }
        };

        typedef ArrayT<Record,NonPodCopyStrategy> Table;
        Table table_;

        void Put(KeyType const& k, ValueType const& v)
        {
            Record* i = teggo::binary_lower( table_.Begin(), table_.End(), k, Comparer() );
            if ( i != table_.End() && !KeyComparer()(k,i->key) )
                i->value = v;
            else
                table_.Insert( i, Record(k,v) );
        }

        ValueType* Get(KeyType const& k) const
        {
            Record const* i = table_.Bfind( k, Comparer() );
            return ( i ) ? (ValueType*)&i->value : 0;
        }

        void Erase(KeyType const& k)
        {
            table_.Berase( k, Comparer() );
        }

        void Reserve( unsigned count )
        {
            table_.Reserve(count);
        }

        typedef FlatmapIteratorT<FlatmapT<tKey,tValue,tTcmp>> Iterator;

        Iterator Iterate() const
        {
            return FlatmapIteratorT_IterateFlatmap(*this);
        }

        Iterator Iterate()
        {
            return FlatmapIteratorT_IterateFlatmap(*this);
        }

        unsigned Count() const
        {
            return table_.Count();
        }

        unsigned Empty() const
        {
            return Count() == 0;
        }

        void Swap(FlatmapT& m)
        {
            table_.Swap(m.table_);
        }

        void Clear()
        {
            table_.Clear();
        }

        void Purge()
        {
            table_.Purge();
        }

        FlatmapT()
        {
        }

    private:
        FlatmapT(FlatmapT const&);
        FlatmapT& operator =(FlatmapT const&);

    };

    template <class tTmap>
    struct FlatmapIteratorT
    {
        int i_;
        tTmap& map_;

        bool Next()
        {
            if ( i_+1 >= signed( map_.Count() ) )
                return false;
            else return ++i_, true;
        }

        typename tTmap::KeyType& Key() const
        {
            return map_.table_[i_].key;
        }

        typename tTmap::ValueType& Value() const
        {
            return map_.table_[i_].value;
        }

        typename tTmap::ValueType& operator*() const
        {
            return Value();
        }

        typename tTmap::ValueType* operator->() const
        {
            return &Value();
        }

        FlatmapIteratorT(tTmap& m) : i_(-1), map_(m)
        {
        }

    };

    template <class tTmap>
    FlatmapIteratorT<tTmap> CXX_STDCALL FlatmapIteratorT_IterateFlatmap(tTmap const& m)
    {
        return FlatmapIteratorT<tTmap>(m);
    }

    template <class tTmap>
    FlatmapIteratorT<tTmap> CXX_STDCALL FlatmapIteratorT_IterateFlatmap(tTmap& m)
    {
        return FlatmapIteratorT<tTmap>(m);
    }
}

template <class tTx, class tTy, class tTz> inline
void CXX_STDCALL teggo_swap(teggo::FlatmapT<tTx,tTy,tTz>& m0, teggo::FlatmapT<tTx,tTy,tTz>& m1)
{
    m0.Swap(m1);
}

#endif // __F32DDF71_33DD_47F8_ABEC_38C3D9052D8C__
