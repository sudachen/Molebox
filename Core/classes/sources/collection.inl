
/*

  Copyright (c) 2005-2009, Alexey Sudachen, alexey@sudachen.name

*/

#if !defined ___8f601429_033d_46af_b9e3_f2989df017d8___
#define ___8f601429_033d_46af_b9e3_f2989df017d8___

#include "collection.h"

#if defined _TEGGO_COLLECTION_HERE
# define _TEGGO_COLLECTION_FAKE_INLINE _TEGGO_EXPORTABLE
#else
# define _TEGGO_COLLECTION_FAKE_INLINE CXX_FAKE_INLINE
#endif

namespace teggo
{

    template <class Tchr>
    _TEGGO_COLLECTION_FAKE_INLINE
    void* CollectionBasicT<Tchr>::Get(key_t key) const
    {
        if ( !key ) return 0;
        unsigned hascode = HashCode(key);
        CollectionT_Record* rec = table_[hascode%mod_];
        while ( rec )
        {
            if ( hascode == rec->hascode && KeyEqual(key_t((char*)rec+sizeOfRec_),key) )
                return rec+1;
            rec = rec->next;
        }
        return 0;
    }

    template <class Tchr>
    _TEGGO_COLLECTION_FAKE_INLINE
    void* CollectionBasicT<Tchr>::Erase(key_t key)
    {
        if ( !key ) return 0;
        unsigned hascode = HashCode(key);
        CollectionT_Record** rec = &table_[hascode%mod_];
        while ( *rec )
        {
            if ( hascode == (*rec)->hascode && KeyEqual(key_t((char*)*rec+sizeOfRec_),key) )
            {
                CollectionT_Record* r = *rec;
                *rec = r->next;
                r->next = 0;
                --count_;
                return r+1;
            }
            rec = &(*rec)->next;
        }
        return 0;
    }

    template <class Tchr>
    _TEGGO_COLLECTION_FAKE_INLINE
    void CollectionBasicT<Tchr>::Erase_(void* mem)
    {
        CollectionT_Record*  memrec = (CollectionT_Record*)mem-1;
        CollectionT_Record** rec = &table_[memrec->hascode%mod_];
        while ( *rec )
        {
            if ( *rec == memrec )
            {
                *rec = memrec->next;
                memrec->next = 0;
                --count_;
                return;
            }
            rec = &(*rec)->next;
        }
        REQUIRE(!"element is not in hashmap");
    }

    template <class Tchr>
    _TEGGO_COLLECTION_FAKE_INLINE
    bool CollectionBasicT<Tchr>::EnsurePresent(key_t key,void** mem)
    {
        unsigned hascode = HashCode(key);
        CollectionT_Record** nrec = &table_[hascode%mod_];
        CollectionT_Record*  rec = *nrec;

        while ( rec )
        {
            if ( hascode == rec->hascode && KeyEqual(key_t((char*)rec+sizeOfRec_),key) )
            {
                *mem = rec+1;
                return true;
            }
            rec = rec->next;
        }

        if ( count_*100 >= mod_*90 )
        {
            unsigned newmod = mod_*2 + 1;
            while ( newmod < count_*100/90 ) newmod = newmod*2 + 1;
            Rehash(newmod);
            nrec = &table_[hascode%mod_];
            rec = *nrec;
        }

        unsigned slen = StrLen(key);
        rec = (CollectionT_Record*)new byte_t[sizeOfRec_+(slen+1)*sizeof(Tchr)];

        rec->hascode = hascode;
        rec->next = *nrec;
        memcpy((byte_t*)rec+sizeOfRec_,key,(slen+1)*sizeof(Tchr));
        *nrec = rec;
        *mem = rec+1;
        ++count_;

        return false;
    }

    template <class Tchr>
    _TEGGO_COLLECTION_FAKE_INLINE
    void CollectionBasicT<Tchr>::DisposeValue(void* mem)
    {
        if ( mem )
        {
            delete[] (byte_t*)((CollectionT_Record*)mem - 1);
        }
    }

    template <class Tchr>
    _TEGGO_COLLECTION_FAKE_INLINE
    void* CollectionBasicT<Tchr>::NextValue(CollectionT_IRecord* iter) const
    {
        if ( !table_ ) return 0;

        if ( iter->row == table_ + mod_ ) return 0;

        if ( !iter->row )
        {
            iter->row = table_;
            iter->col = *iter->row;
        lb_find_row:
            while ( !iter->col )
            {
                ++iter->row;
                if ( iter->row == table_ + mod_ ) return 0;
                iter->col = *iter->row;
            }
        }
        else
        {
            if ( iter->col )
                iter->col = iter->col->next;
            else
                iter->col = *iter->row;
            goto lb_find_row; // find next row if !iter->col
        }

        return iter->col+1;
    }

    template <class Tchr>
    _TEGGO_COLLECTION_FAKE_INLINE
    Tchr const* CollectionBasicT<Tchr>::NextKey(CollectionT_IRecord* iter) const
    {
        void* val = NextValue(iter);
        if ( val )
            return key_t((char*)((CollectionT_Record*)val-1)+sizeOfRec_);
        return 0;
    }

    template <class Tchr>
    _TEGGO_COLLECTION_FAKE_INLINE
    void CollectionBasicT<Tchr>::Clear()
    {
        for ( unsigned i = 0; i < mod_; ++i )
        {
            CollectionT_Record* rec = table_[i];
            while ( rec )
            {
                CollectionT_Record* r = rec->next;
                delete[] (byte_t*)rec;
                rec = r;
            }
        }
        memset(table_,0,sizeof(CollectionT_Record*)*mod_);
        count_ = 0;
    }

    template <class Tchr>
    _TEGGO_COLLECTION_FAKE_INLINE
    void CollectionBasicT<Tchr>::Rehash(unsigned newmod)
    {
        CollectionT_Record** table = new CollectionT_Record*[newmod];
        memset(table,0,newmod*sizeof(CollectionT_Record*));
        if ( table_ )
        {
            for ( unsigned i = 0; i < mod_; ++i )
            {
                CollectionT_Record* rec = table_[i];
                while ( rec )
                {
                    CollectionT_Record* r= rec->next;
                    unsigned j = rec->hascode % newmod;
                    rec->next = table[j];
                    table[j] = rec;
                    rec = r;
                }
            }
            delete[] table_;
        }
        table_ = table;
        mod_ = newmod;
    }

    #if defined _TEGGO_COLLECTION_HERE
    void __export_collection_functions()
    {
        typedef void (CollectionBasicT<char>::*t0)();
        typedef void (CollectionBasicT<wchar_t>::*t1)();
        volatile t0* _t0 = 0;
        volatile t1* _t1 = 0;

        *_t0 = (t0)&CollectionBasicT<char>::Get;
        *_t1 = (t1)&CollectionBasicT<wchar_t>::Get;
        *_t0 = (t0)&CollectionBasicT<char>::Erase;
        *_t1 = (t1)&CollectionBasicT<wchar_t>::Erase;
        *_t0 = (t0)&CollectionBasicT<char>::Erase_;
        *_t1 = (t1)&CollectionBasicT<wchar_t>::Erase_;
        *_t0 = (t0)&CollectionBasicT<char>::EnsurePresent;
        *_t1 = (t1)&CollectionBasicT<wchar_t>::EnsurePresent;
        *_t0 = (t0)&CollectionBasicT<char>::DisposeValue;
        *_t1 = (t1)&CollectionBasicT<wchar_t>::DisposeValue;
        *_t0 = (t0)&CollectionBasicT<char>::NextValue;
        *_t1 = (t1)&CollectionBasicT<wchar_t>::NextValue;
        *_t0 = (t0)&CollectionBasicT<char>::NextKey;
        *_t1 = (t1)&CollectionBasicT<wchar_t>::NextKey;
        //*_t0 = (t0)&CollectionBasicT<char>::IncrementIter;
        //*_t1 = (t1)&CollectionBasicT<wchar_t>::IncrementIter;
        *_t0 = (t0)&CollectionBasicT<char>::Clear;
        *_t1 = (t1)&CollectionBasicT<wchar_t>::Clear;
        *_t0 = (t0)&CollectionBasicT<char>::Rehash;
        *_t1 = (t1)&CollectionBasicT<wchar_t>::Rehash;
    }

    #endif

} // namespace

#undef _TEGGO_COLLECTION_FAKE_INLINE

#endif // ___8f601429_033d_46af_b9e3_f2989df017d8___
