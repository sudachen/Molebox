
/*

  Copyright (c) 2005-2009, Alexey Sudachen, alexey@sudachen.name

*/

#if !defined ___aa7f0f01_6ce7_451c_964a_c4d68c7902bc___
#define ___aa7f0f01_6ce7_451c_964a_c4d68c7902bc___

#include "keylist.h"

namespace teggo
{

#if !defined _TEGGO_KEYLIST_HERE
# define _TEGGO_KEYLIST_FAKE_INLINE  CXX_FAKE_INLINE
#else
# define _TEGGO_KEYLIST_FAKE_INLINE  _TEGGO_EXPORTABLE
#endif

  _TEGGO_KEYLIST_FAKE_INLINE
  KeyListRecord * CXX_STDCALL KeyListRecord::Insert(pwide_t key,KeyListRecord** rec,unsigned sizeOfT)
    {
      long key_len = key?wcslen(key):0;

      if ( !key )
        {
          while ( *rec )
            {
              if ( *(wchar_t*)((char*)(*rec + 1) + sizeOfT) != 0 )
                break;
              rec = &(*rec)->next;
            }
        }

      KeyListRecord* rec0 =
          (KeyListRecord*)new byte_t[sizeof(KeyListRecord)+sizeOfT+(key_len+1)*sizeof(wchar_t)];

      rec0->next = *rec;
      *rec = rec0;

      if ( key )
        memcpy( ((byte_t*)(rec0+1) + sizeOfT), key, (key_len+1)*sizeof(wchar_t) );

      return rec0;
    }

  _TEGGO_KEYLIST_FAKE_INLINE
  KeyListRecord** CXX_STDCALL KeyListRecord::FindRecordByKey(pwide_t key,KeyListRecord** rec,unsigned sizeOfT)
    {
      while ( *rec )
        {
          pwide_t k = pwide_t((byte_t*)(*rec+1)+sizeOfT);
          if ( *k == 0 )
            return 0;
          if ( wcscmp(k,key) == 0 )
            return rec;
          rec = &(*rec)->next;
        }

      return 0;
    }

  _TEGGO_KEYLIST_FAKE_INLINE
  KeyListRecord** CXX_STDCALL KeyListRecord::FindRecordByKeyI(pwide_t key,KeyListRecord** rec,unsigned sizeOfT)
    {
      while ( *rec )
        {
          pwide_t k = pwide_t((byte_t*)(*rec+1)+sizeOfT);

          if ( *k == 0 )
            return 0;

          if ( wcsicmp(k,key) == 0 )
            return rec;

          rec = &(*rec)->next;
        }

      return 0;
    }

} // namespace

#endif // #define ___aa7f0f01_6ce7_451c_964a_c4d68c7902bc___
