
/*

  Copyright (c) 2005-2009, Alexey Sudachen, alexey@sudachen.name

*/

#ifndef ___66e66e00_3224_4300_a779_4b8e401dadb7___
#define ___66e66e00_3224_4300_a779_4b8e401dadb7___

#include "symboltable.h"

#if defined _TEGGO_SYMTABLE_HERE
# define _TEGGO_SYMTABLE_FAKE_INLINE _TEGGO_EXPORTABLE
#else
# define _TEGGO_SYMTABLE_FAKE_INLINE CXX_FAKE_INLINE
#endif

namespace teggo {

  _TEGGO_SYMTABLE_FAKE_INLINE
    pwide_t SymbolTable::FindSymbol(pwide_t val)
      {
        if ( !val ) return 0;
        PWIDE pw;
        pw.text = val;
        pw.len  = wcslen(val);
        if ( PWIDE *ppw = table_.Get(pw) )
          return ppw->text;
        return 0;
      }

  _TEGGO_SYMTABLE_FAKE_INLINE
    pwide_t SymbolTable::GetSymbol(pwide_t val)
      {
        PWIDE pw;
        pw.text = val;
        pw.len  = wcslen(val);
        if ( PWIDE *ppw = table_.Get(pw) )
          return ppw->text;
        else
          {
            pw.text = new wchar_t[pw.len+1];
            for ( unsigned i = 0; i < pw.len; ++i ) ((wchar_t*)pw.text)[i] = towlower(val[i]);
            ((wchar_t*)pw.text)[pw.len] = 0;
            table_.Put(pw);
            return pw.text;
          }
      }

  _TEGGO_SYMTABLE_FAKE_INLINE
    SymbolTable::SymbolTable()
      {
      }

  _TEGGO_SYMTABLE_FAKE_INLINE
    SymbolTable::~SymbolTable()
      {
        for ( FlatsetT<PWIDE,PWideLess>::Iterator i = table_.Iterate(); i.Next(); )
          delete[] (wchar_t*)i->text;
      }
} // namespace teggo

#endif //___66e66e00_3224_4300_a779_4b8e401dadb7___
