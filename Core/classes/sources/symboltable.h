
/*

  Copyright (c) 2005-2009, Alexey Sudachen, alexey@sudachen.name

*/

#ifndef ___9edf61b2_cac8_4452_b9f5_62d63e759207___
#define ___9edf61b2_cac8_4452_b9f5_62d63e759207___

#include "_specific.h"
#include "string.h"
#include "flatset.h"
#include "refcounted.h"

namespace teggo {

  struct SymbolTable : Refcounted
    {
      _TEGGO_EXPORTABLE pwide_t FindSymbol(pwide_t);
      _TEGGO_EXPORTABLE pwide_t GetSymbol(pwide_t);
      _TEGGO_EXPORTABLE SymbolTable();
      _TEGGO_EXPORTABLE ~SymbolTable();
    private:
      struct PWIDE { unsigned len; pwide_t text; };
      struct PWideLess
        {
          bool operator() (const PWIDE& a,const PWIDE& b) const
            {
              return a.len < b.len || (a.len == b.len && wcsicmp(a.text,b.text) < 0);
            }
        };
      FlatsetT<PWIDE,PWideLess> table_;
    };
}

#if defined _TEGGOINLINE
#include "symboltable.inl"
#endif

#endif //___9edf61b2_cac8_4452_b9f5_62d63e759207___
