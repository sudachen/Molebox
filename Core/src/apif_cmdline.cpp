
/*

  Copyright (c) 2008-2009, Alexey Sudachen, alexey@sudachen.name

*/

#include "_common.h"
#include "apif.h"
#include "logger.h"
#include "XorS.h"
#include "xnt.h"

extern "C" void *_PEB();

struct CmdlineFilter : APIfilter
  {

    wchar_t *cmdlineW;
    char    *cmdlineA;
    wchar_t *peb_cmdl;
    
    wchar_t *GetPebCmdline()
      {
        _XPEB *peb = (_XPEB *)_PEB();
        MY_RTL_USER_PROCESS_PARAMETERS *para = peb->ProcessParameters;
        return para->CommandLine.Buffer;
      }

    void UpdateCmdline()
      {
        wchar_t *q = GetPebCmdline();
        if ( peb_cmdl != q )
          {
            wcsncpy(cmdlineW,q,4095);
            WideCharToMultiByte(CP_ACP,0,cmdlineW,-1,cmdlineA,4095,0,0);
            peb_cmdl = q;
          }
      }

    CmdlineFilter() : cmdlineW(0), cmdlineA(0), peb_cmdl(0)
      {
        byte_t *q = (byte_t*)VirtualAlloc(0,4096*3,MEM_COMMIT,PAGE_READWRITE);
        cmdlineW = (wchar_t*)q;
        cmdlineA = (char*)q+4096*2;
      }

    APIF_STATUS DoGetCmdlA(void *args, longptr_t *result)
      {
        UpdateCmdline();
        *result = (longptr_t)cmdlineA;
        return APIF_RETURN;
      }
 
    APIF_STATUS DoGetCmdlW(void *args, longptr_t *result)
      {
        UpdateCmdline();
        *result = (longptr_t)cmdlineW;
        return APIF_RETURN;
      }

    virtual APIF_STATUS DoCall(int apif_id, void *args, longptr_t *result)
      {
        switch(apif_id)
          {
            case APIF_CMDLINEA:
              return DoGetCmdlA(args,result);
            case APIF_CMDLINEW:
              return DoGetCmdlW(args,result);
          }
        return APIF_CONTINUE;
      }
  };
  
CmdlineFilter apif_cmdlfilt = CmdlineFilter();

void APIF_RegisterCmdlHooks()
  {
    APIF->Push(&apif_cmdlfilt);
  };
