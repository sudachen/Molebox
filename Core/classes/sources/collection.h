
/*

  Copyright (c) 2005-2009, Alexey Sudachen, alexey@sudachen.name

*/

#if !defined ___CD4E5B55_0248_456a_A23F_F3B50A68DEFE___
#define ___CD4E5B55_0248_456a_A23F_F3B50A68DEFE___

#include "_specific.h"
#include "_crc32.h"
#include "string.h"

namespace teggo
{

  struct CollectionT_Record
    {
      unsigned hascode;
      CollectionT_Record *next;
    };

  struct CollectionT_IRecord
    {
      CollectionT_Record **row;
      CollectionT_Record *col;
    };

  template <class Tchr=wchar_t>
    struct CollectionBasicT
      {
        typedef Tchr const *key_t;
        enum { PRECLEANED = 0x7fffffff };
        unsigned sizeOfRec_;
        unsigned count_;
        unsigned mod_;
        CollectionT_Record** table_;
        _TEGGO_EXPORTABLE void* Get(key_t) const;
        _TEGGO_EXPORTABLE void* Erase(key_t);
        _TEGGO_EXPORTABLE void  Erase_(void*);
        _TEGGO_EXPORTABLE bool EnsurePresent(key_t key,void** mem);
        _TEGGO_EXPORTABLE void DisposeValue(void*);
        _TEGGO_EXPORTABLE void* NextValue(CollectionT_IRecord*) const;
        _TEGGO_EXPORTABLE key_t NextKey(CollectionT_IRecord*) const;
        //_TEGGO_EXPORTABLE bool IncrementIter(CollectionT_IRecord*) const;
        _TEGGO_EXPORTABLE void Clear();
        _TEGGO_EXPORTABLE void Rehash(unsigned newmod);
        _TEGGO_EXPORTABLE CollectionBasicT(unsigned preserve,unsigned sizeOfT);
        _TEGGO_EXPORTABLE ~CollectionBasicT();
        unsigned Count() const { return count_; }

        static bool CXX_STDCALL KeyEqual(key_t keyA,key_t keyB) { return StrSafeEqual(keyA,keyB); }
        static unsigned CXX_STDCALL HashCode(key_t key) { return Crc32(0,(const byte_t*)key,StrLen(key)*sizeof(Tchr)); }

        void Swap(CollectionBasicT &c)
          {
            teggo_swap(sizeOfRec_,c.sizeOfRec_);
            teggo_swap(count_,c.count_);
            teggo_swap(mod_,c.mod_);
            teggo_swap(table_,c.table_);
          }

      };

  template <class Tchr>
    CollectionBasicT<Tchr>::CollectionBasicT(unsigned preserve,unsigned sizeOfT)
      {
        sizeOfRec_ = sizeOfT+sizeof(CollectionT_Record);
        count_ = 0;
        table_ = 0;
        mod_   = 0;
        Rehash(preserve?preserve:13);
      }

  template <class Tchr>
    CollectionBasicT<Tchr>::~CollectionBasicT()
      {
        delete[] table_;
      }

  template <class tTx,class Tchr=wchar_t>
    struct CollectionIteratorT
      {
        typedef Tchr const *key_t;
        CollectionBasicT<Tchr> const *db_;
        CollectionT_IRecord iter;
        key_t Key() const;
        tTx* val_;

        tTx& Value()
          {
            return *val_;
          }

        tTx* Next()
          {
            return db_ ? (val_ = (tTx*)db_->NextValue(&iter)) : 0;
          }

        bool Reset()
          {
            iter.row = 0;
            val_ = 0;
            return db_ ? db_->Count() != 0 : false;
          }

        CollectionIteratorT(CollectionBasicT<Tchr> *db) : db_(db)
          {
            iter.row = 0;
            val_ = 0;
          }

        tTx& operator*()
          {
            return *val_;
          }

        tTx* operator->()
          {
            return val_;
          }

        operator bool () const
          {
            return db_ ? db_->Count() != 0 : false;
          }
      };

  template<class tTx,class Tchr=wchar_t>
    struct CollectionT : private CollectionBasicT<Tchr>
      {
        typedef CollectionIteratorT<tTx,Tchr> Iterator;
        typedef tTx Value;
        typedef Tchr const *key_t;
        typedef CollectionBasicT<Tchr> Basic;
        
        using Basic::count_;
        using Basic::PRECLEANED;

        CollectionT(int preserve=13);
        ~CollectionT();

        void Swap(CollectionT &c)
          {
            Basic::Swap(c);
          }

        tTx* Get(key_t key) const
          {
            return (tTx*)Basic::Get(key);
          }

        tTx* Get(BaseStringT<Tchr> const &k) const
          {                               
            return Get(+k);
          }
           
        tTx* GetLower(key_t key) const
          {
            return Get(StringT<Tchr,32>(key).ToLower().Str());
          }

        tTx* GetLower(BaseStringT<Tchr> const &k) const
          {                               
            return GetLower(+k);
          }    
          
        tTx& Put(key_t key,const tTx& value)
          {
            REQUIRE(count_ != PRECLEANED);
            tTx* t;
            if ( Basic::EnsurePresent(key,(void**)&t) )
              {
                if ( t == &value ) return *t;
                t->~tTx();
              }
            return *(new(t) tTx(value));
          }

        tTx& Put(BaseStringT<Tchr> const &k,const tTx& value)
          {
            return Put(+k,value);
          }
          
        tTx& PutLower(key_t key,const tTx& value)
          {
            return Put(StringT<Tchr,32>(key).ToLower().Str(),value);
          }

        tTx& PutLower(BaseStringT<Tchr> const &k,const tTx& value)
          {
            return PutLower(+k,value);
          }
          
        tTx& Rec(key_t key)
          {
            REQUIRE(count_ != PRECLEANED);
            tTx* t;
            if ( !Basic::EnsurePresent(key,(void**)&t) )
              new(t) tTx;
            return *t;
          }

        tTx& Rec(BaseStringT<Tchr> const &k)
          {
            return Rec(+k);
          }
          
        void Erase(key_t key)
          {
            if ( count_ == PRECLEANED ) return;

            tTx* t = (tTx*) Basic::Erase(key);
            if ( t )
              {
                t->~tTx();
                Basic::DisposeValue(t);
              }
          }

        void Erase(BaseStringT<Tchr> const &k)
          {
            return Erase(+k);
          }
          
        void EraseLower(key_t key)
          {
            Erase(StringT<Tchr,32>(key).ToLower().Str());
          }

        void EraseLower(BaseStringT<Tchr> const &k)
          {
            return EraseLower(+k);
          }

        void Erase(tTx* t)
          {
            if ( count_ == PRECLEANED ) return;

            Basic::Erase_((void*)t);
            t->~tTx();
            Basic::DisposeValue(t);
          }

        tTx &operator[] (key_t key)
          {
            return Rec(key);
          }
              
        tTx &operator[] (BaseStringT<Tchr> const &k)
          {
            return operator[](+k);
          }

        void Clear() ;

        Iterator Iterate() const
          {
            return Iterator((Basic*)this);
          }

        static key_t CXX_STDCALL KeyOf(const tTx* t)
          {
            return key_t((char*)((CollectionT_Record*)t-1)+sizeof(CollectionT_Record)+sizeof(tTx));
          }

        using Basic::Count;

      private:

        CollectionT(CollectionT const&);
        CollectionT& operator =(CollectionT const&);

      };

  template <class tTx,class Tchr>
    CollectionT<tTx,Tchr>::CollectionT(int preserve) : CollectionBasicT<Tchr>(preserve,sizeof(tTx))
      {
      }

  template <class tTx,class Tchr>
    CollectionT<tTx,Tchr>::~CollectionT()
      {
        Clear();
      }

  template <class tTx,class Tchr>
    void CollectionT<tTx,Tchr>::Clear()
      {
        Basic::count_ = PRECLEANED;
        tTx* t;
        CollectionT_IRecord iter = { 0,0 };
        while ( (t=(tTx*)Basic::NextValue(&iter)) != 0 )
          t->~tTx();
        Basic::count_ = 0;
        Basic::Clear();
      }

  template<class tTx,class Tchr>
    Tchr const * CollectionIteratorT<tTx,Tchr>::Key() const
      {
        return CollectionT<tTx,Tchr>::KeyOf(val_);
      }

} // namespace teggo

template <class tTx,class Tchr> inline
  void CXX_STDCALL teggo_swap(teggo::CollectionT<tTx,Tchr> &s0, teggo::CollectionT<tTx,Tchr> &s1)
    {
      s0.Swap(s1);
    }

#if defined _TEGGOINLINE
#include "collection.inl"
#endif

#endif // ___CD4E5B55_0248_456a_A23F_F3B50A68DEFE___
