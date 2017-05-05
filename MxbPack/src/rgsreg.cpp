
/*

  Copyright (c) 2008-2009, Alexey Sudachen, alexey@sudachen.name

*/

#define UNICODE

#include "myafx.h"
#include <atlbase.h>
//#include <atlcom.h>
#include <statreg.h>

bool RegisterRcScript(pchar_t text,unsigned len,pwide_t modname)
  {
    CRegObject cro;
    cro.FinalConstruct();
    cro.AddReplacement(L"MODULE",modname);
    CRegParser crp(&cro);
    return SUCCEEDED(crp.RegisterBuffer((wchar_t*)text,TRUE));
  }
  
