
/*

  Copyright (c) 2005-2009, Alexey Sudachen, alexey@sudachen.name

*/

#if !defined ___84F67621_B2EE_41bf_A0E0_38456AC55197___
#define ___84F67621_B2EE_41bf_A0E0_38456AC55197___

#include "_specific.h"

namespace teggo
{

  struct KeyListRecord
    {
      KeyListRecord* next;
      _TEGGO_EXPORTABLE static KeyListRecord*  CXX_STDCALL Insert(pwide_t key,KeyListRecord** rec,unsigned sizeOfT);
      _TEGGO_EXPORTABLE static KeyListRecord** CXX_STDCALL FindRecordByKey(pwide_t key,KeyListRecord** rec,unsigned sizeOfT);
      _TEGGO_EXPORTABLE static KeyListRecord** CXX_STDCALL FindRecordByKeyI(pwide_t key,KeyListRecord** rec,unsigned sizeOfT);
    };

  template<class tTx>
    struct KeyListT
      {
        KeyListRecord* list_;

        static pwide_t KeyOf(const tTx* t)
          {
            return pwide_t((char*)t + sizeof(tTx));
          }

        pwide_t FindKeyOf(const tTx& t)
          {
            KeyListRecord** rec = FindRecordByValue(t);
            if ( rec ) return pwide_t((char*)(*rec + 1) + sizeof(tTx));
            return 0;
          }

        tTx* FindValueOf(pwide_t key)
          {
            KeyListRecord** rec = KeyListRecord::FindRecordByKey(key,&list_,sizeof(tTx));
            if ( rec ) return (tTx*)(*rec + 1);
            return 0;
          }

        tTx* FindValueOfI(pwide_t key)
          {
            KeyListRecord** rec = KeyListRecord::FindRecordByKeyI(key,&list_,sizeof(tTx));
            if ( rec ) return (tTx*)(*rec + 1);
            return 0;
          }

        tTx& Insert(pwide_t key)
          {
            KeyListRecord** rec = &list_;
            return *( new((tTx*)(KeyListRecord::Insert(key,&list_,sizeof(tTx))+1)) tTx );
          }

        tTx& Insert(pwide_t key,const tTx& value)
          {
            KeyListRecord** rec = &list_;
            return *( new((tTx*)(KeyListRecord::Insert(key,&list_,sizeof(tTx))+1)) tTx(value) );
          }

        void Erase(const tTx& t)
          {
            KeyListRecord** rec = FindRecordByValue(t);
            if ( rec )
              {
                KeyListRecord* k = *rec;
                *rec = k->next;
                ((tTx*)(k + 1))->~tTx();
                delete[] (byte_t*)k;
              }
          }

        tTx* First()
          {
            if ( list_ )
              return (tTx*)(list_+1);
            else
              return 0;
          }

        void EraseFirst()
          {
            if ( list_ )
              {
                KeyListRecord* k = list_;
                list_ = k->next;
                ((tTx*)(k+1))->~tTx();
                delete[] (byte_t*)k;
              }
          }

        KeyListT()
          : list_(0)
          {
          }

        ~KeyListT()
          {
            while ( First() )
              EraseFirst();
          }

      private:

        KeyListT(const KeyListT&);
        void operator =(const KeyListT&);

        KeyListRecord** FindRecordByValue(const tTx &t)
          {
            KeyListRecord **rec = &list_;
            while ( *rec )
              {
                if ( *(const tTx*)(*rec+1) == t )
                  return rec;
                rec = &(*rec)->next;
              }
            return 0;
          }
      };

} // namespace

#if defined _TEGGOINLINE
#include "keylist.inl"
#endif

#endif // ___84F67621_B2EE_41bf_A0E0_38456AC55197___
