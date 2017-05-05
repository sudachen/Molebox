
/*

  Copyright (c) 2008-2009, Alexey Sudachen, alexey@sudachen.name

*/


#define _FREE   Lfree
#define _MALLOC Lalloc
#define _TEGGONEW 1

#define _TEGGOSTATIC
#define _TEGGOSTRIP__EXPR__
#define _TEGGOSTRIP__FILE__
#define _TEGGO_WITHOUT_SHL
#define __BUILD_CORE__

#if defined _WIN32 && !defined _WINDOWS_
# if !defined _X86_
#   define _X86_ 1
# endif
# if !defined WINVER
#   define WINVER 0x400
# endif
#endif

#include <windows.h>
#include <winternl.h>
#include "xnt.h"
#include "sources/detect_compiler.h"
#include "stbinfo.h"
                
extern HANDLE g_heap;
extern STB_INFO2_FLAGS g_stbi2f;

extern "C" void Lfree(void *pp)
  { 
    if ( !pp ) return;
    if ( g_stbi2f.insinity )
      {
        byte_t *p = (byte_t*)pp - 12; 
        if ( *(unsigned*)p != 'AAAA' || *(unsigned*)(p+8) != 'BBBB' || *(unsigned*)(*(unsigned*)(p+4) + (p + 12)) != 'CCCC' )
          __asm int 3;
        if ( !HeapValidate(g_heap,0,p) )
          __asm int 3;
        HeapFree(g_heap,0,p);
      }
    else
      HeapFree(g_heap,0,pp);
  }
  
extern "C" void *Lalloc(unsigned sz)
  { 
    if ( g_stbi2f.insinity )
      {
        if ( !HeapValidate(g_heap,0,0) ) 
          __asm int 3;
        //if ( !HeapValidate(GetProcessHeap(),0,0) ) 
        //  __asm int 3;
        byte_t *p = (byte_t*)HeapAlloc(g_heap,HEAP_GENERATE_EXCEPTIONS|HEAP_ZERO_MEMORY,sz+16);
        memset(p,'A',4);
        *(unsigned*)(p+4) = sz;
        memset(p+8,'B',4);
        memset(p+(12+sz),'C',4);
        return p+12;
      }
    else                                                                                       
      return HeapAlloc(g_heap,HEAP_GENERATE_EXCEPTIONS|HEAP_ZERO_MEMORY,sz);
  }


#define _X86_ASSEMBLER

/*
namespace aux {
  CXX_EXTERNC void *CXX_STDCALL Lalloc(u32_t sz);
  CXX_EXTERNC void *CXX_STDCALL LallocZ(u32_t sz);
  CXX_EXTERNC void  CXX_STDCALL Lfree(void *p);
  CXX_EXTERNC void  CXX_STDCALL LfreePA(void *p, u32_t cnt);
}
*/

#define _TEGGO_ADLER_HERE
#define _TEGGO_CODECOP_HERE
#define _TEGGO_CRC32_HERE
#define _TEGGO_SPECIFIC_HERE
#define _TEGGO_FORMAT_HERE
#define _TEGGO_GENIO_HERE
#define _TEGGO_INSTANCE_HERE
#define _TEGGO_LZPLUS_DECODER_HERE
#define _TEGGO_STRING_HERE
#define _TEGGO_SYSUTIL_HERE
#define _TEGGO_XHASH_HERE
#define _TEGGO_NEWDES_HERE
#define _TEGGO_HOOKMGR_HERE
#define _TEGGO_X86CODE_HERE
#define _TEGGO_THREADS_HERE
#define _TEGGO_DISABLE_DBGOUTPUT
#define _TEGGO_THREADS_HERE
#define _TEGGO_STREAMS_HERE
#define _TEGGO_ZLIB_HERE

#include "../Classes/Sources/_specific.h"
#include "../Classes/Sources/_adler32.h"
#include "../Classes/Sources/_codecop.inl"
#include "../Classes/Sources/_crc32.h"
#include "../Classes/Sources/_specific.inl"
#include "../Classes/Sources/format.h"
#include "../Classes/Sources/genericio.inl"
#include "../Classes/Sources/hinstance.h"
//#include "../Classes/Sources/lz+decoder.inl"
#include "../Classes/Sources/string.inl"
#include "../Classes/Sources/sysutil.h"
#include "../Classes/Sources/newdes.inl"
#include "../Classes/Sources/x86code.h"
#include "../Classes/Sources/threads.h"
#include "../Classes/Sources/streams.inl"
#include "../Classes/Sources/zlib.inl"
