
/*

  Copyright (c) 2008-2009, Alexey Sudachen, alexey@sudachen.name

*/


#define _FREE   Lfree
#define _MALLOC Lalloc
#define _TEGGONEW 1

#define _TEGGOSTATIC
#define _TEGGOSTRIP__EXPR__
#define _TEGGOSTRIP__FILE__

#if defined _WIN32 && !defined _WINDOWS_
# if !defined _X86_
#   define _X86_ 1
# endif
# if !defined WINVER
#   define WINVER 0x400
# endif
#endif

#include <windows.h>
                
extern "C" void Lfree(void *pp)
  { 
      HeapFree(GetProcessHeap(),0,pp);
  }
  
extern "C" void *Lalloc(unsigned sz)
  { 
      return HeapAlloc(GetProcessHeap(),HEAP_GENERATE_EXCEPTIONS|HEAP_ZERO_MEMORY,sz);
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
#define _TEGGO_COLLECTION_HERE
#define _TEGGO_XDOM_HERE
#define _TEGGO_SAXPARS_HERE
#define _TEGGO_LOGGER_HERE
#define _TEGGO_SYMTABLE_HERE
#define _TEGGO_MESSAGES_HERE
#define _TEGGO_COMREGISTER_HERE
#define _TEGGO_SYSUTIL_HERE
#define _TEGGO_LZ77SS_HERE

#include "../Classes/Sources/_specific.h"
#include "../Classes/Sources/_adler32.h"
#include "../Classes/Sources/_codecop.inl"
#include "../Classes/Sources/_crc32.h"
#include "../Classes/Sources/_specific.inl"
#include "../Classes/Sources/format.h"
#include "../Classes/Sources/genericio.inl"
#include "../Classes/Sources/hinstance.h"
#include "../Classes/Sources/lz+decoder.inl"
#include "../Classes/Sources/string.inl"
#include "../Classes/Sources/sysutil.h"
#include "../Classes/Sources/newdes.inl"
#include "../Classes/Sources/x86code.h"
#include "../Classes/Sources/threads.h"
#include "../Classes/Sources/streams.inl"
#include "../Classes/Sources/zlib.inl"
#include "../Classes/Sources/collection.inl"
#include "../Classes/Sources/xdom.inl"
#include "../Classes/Sources/saxparser.inl"
#include "../Classes/Sources/symboltable.inl"
#include "../Classes/Sources/logger.inl"
#include "../Classes/Sources/messages.inl" 
#include "../Classes/Sources/com_register.inl"
#include "../Classes/Sources/sysutil.h"
#include "../Classes/Sources/lz77ss.inl"
