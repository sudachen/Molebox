
/*

  Copyright (c) 2008-2009, Alexey Sudachen, alexey@sudachen.name

*/

#define _TEGGOSTATIC
#define _TEGGOSTRIP__EXPR__
#define _TEGGOSTRIP__FILE__
#define _TEGGO_WITHOUT_SHL
#define __BUILD_CORE__

#define _DISABLE_DBGOUTPUT

#if defined _WIN32 && !defined _WINDOWS_
# if !defined _X86_
#   define _X86_ 1
# endif
# if !defined WINVER
#   define WINVER 0x600
# endif
# if !defined _WIN32_WINNT
#   define _WIN32_WINNT 0x0600
# endif
#endif

#include <stdint.h>

typedef __int64 INT64T;

#include <WinSock2.h>
#include <windows.h>
#include <winternl.h>
#include "xnt.h"
#include "../Xi/ntdll.h"
//#include "Xi/Xi.h"

#undef EXTERN_C

#define _FREE   Lfree
#define _MALLOC Lalloc
#define _TEGGONEW 1

#if defined __cplusplus
extern "C" void* Lalloc(unsigned sz);
extern "C" void Lfree(void* pp);
#include "../Classes/classes.h"
#else
void* Lalloc(unsigned sz);
void Lfree(void* pp);
#define EXTERN_C
#endif // __cplusplus

#include "stbinfo.h"

EXTERN_C  int _MAJOR_VERSION;
EXTERN_C  int _BUILD_NUMBER;
EXTERN_C  void memsetS(void* b, int val, unsigned n);
EXTERN_C  void memcopy(void* dst, void* src, unsigned n);
EXTERN_C  char* __cdecl _UnXOr(int source, int count, char* buffer);
//EXTERN_C void  __cdecl _UnXOrFree(void* );

EXTERN_C  void StartDebugger();
EXTERN_C  int AuxReadFile(HANDLE f, void* ptr, unsigned len);
EXTERN_C  void* AuxAllocExec(int sz);
EXTERN_C  void AuxFree(void*);

EXTERN_C  unsigned* _LASTSTATUS();
EXTERN_C  HANDLE _CURDIRHANDLE();

EXTERN_C  unsigned isNestedProcess;

typedef struct _CRPROC_INFO
{
    wchar_t fullExePath[1024];
    wchar_t internalName[256];
    unsigned char internalSign[16];
    struct
    {
        unsigned injected   : 1;
        unsigned suspended  : 1;
    } f;
} CRPROC_INFO;

EXTERN_C void* _TIB();
