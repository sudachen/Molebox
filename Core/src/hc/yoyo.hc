
/*

Copyright © 2010-2011, Alexéy Sudachén, alexey@sudachen.name, Chile

In USA, UK, Japan and other countries allowing software patents:

    This program is free software: you can redistribute it and/or modify
    it under the terms of the GNU General Public License as published by
    the Free Software Foundation, either version 3 of the License, or
    (at your option) any later version.

    This program is distributed in the hope that it will be useful,
    but WITHOUT ANY WARRANTY; without even the implied warranty of
    MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
    GNU General Public License for more details.

    http://www.gnu.org/licenses/

Otherwise:

Permission is hereby granted, free of charge, to any person obtaining a copy
of this software and associated documentation files (the "Software"), to deal
in the Software without restriction, including without limitation the rights
to use, copy, modify, merge, publish, distribute, sublicense, and/or sell
copies of the Software, and to permit persons to whom the Software is
furnished to do so, subject to the following conditions:

The above copyright notice and this permission notice shall be included in
all copies or substantial portions of the Software.

THE SOFTWARE IS PROVIDED "AS IS", WITHOUT WARRANTY OF ANY KIND, EXPRESS OR
IMPLIED, INCLUDING BUT NOT LIMITED TO THE WARRANTIES OF MERCHANTABILITY,
FITNESS FOR A PARTICULAR PURPOSE AND NONINFRINGEMENT. IN NO EVENT SHALL THE
AUTHORS OR COPYRIGHT HOLDERS BE LIABLE FOR ANY CLAIM, DAMAGES OR OTHER
LIABILITY, WHETHER IN AN ACTION OF CONTRACT, TORT OR OTHERWISE, ARISING FROM,
OUT OF OR IN CONNECTION WITH THE SOFTWARE OR THE USE OR OTHER DEALINGS IN
THE SOFTWARE.

Except as contained in this notice, the name of a copyright holder shall not
be used in advertising or otherwise to promote the sale, use or other dealings
in this Software without prior written authorization of the copyright holder.

*/

/*

  If using GNU bintools:

    Don't foget to use -rdynamic to see symbols in backtrace!

*/

#ifndef C_once_6973F3BA_26FA_434D_9ED9_FF5389CE421C
#define C_once_6973F3BA_26FA_434D_9ED9_FF5389CE421C
#define YO_CORE_VERSION 1000

#ifdef _LIBYOYO
#define _YO_CORE_BUILTIN
#endif

#if defined _MSC_VER && _MSC_VER > 1400
#pragma warning(disable:4996) /*The POSIX name for this item is deprecated*/
# ifndef _CRT_SECURE_NO_WARNINGS
#   define _CRT_SECURE_NO_WARNINGS
# endif
#endif

/* markers */
#define __Acquire /* a function acquires the ownership of argument */

#if defined _MSC_VER
#define __No_Return __declspec(noreturn)
#elif defined __GNUC__
#define __No_Return __attribute__((noreturn))
#else
#define __No_Return
#endif

#ifndef __yoTa
# define __yoTa(Text,NumId) Text
#endif

#define __FOUR_CHARS(C1,C2,C3,C4) ((uint_t)(C4)<<24)|((uint_t)(C3)<<16)|((uint_t)(C2)<<8)|((uint_t)(C1))

#if defined __linux__
#define _GNU_SOURCE
#elif defined __NetBSD__
#define _NETBSD_SOURCE
#elif defined __FreeBSD__
/* __BSD_VISIBLE defined by default! */
#endif

#ifdef _TREADS
#define _REENTRANT
#endif

#include <stdlib.h>
#include <stdio.h>
#include <setjmp.h>
#include <time.h>
#include <string.h>
#include <stdarg.h>
#include <ctype.h>
#include <limits.h>
#include <wctype.h>
#include <wchar.h>
#include <sys/types.h>
#include <sys/stat.h>
#include <errno.h>
#include <fcntl.h>
#include <math.h>
#include <locale.h>

#if defined WIN32 || defined _WIN32 || defined _MSC_VER || defined __MINGW32_VERSION
# define __windoze
# if !defined __i386 && !defined __x86_64
#  ifdef _M_IX86
#   define __i386
#  elif defined _M_AMD64
#   define __x86_64
#  else
#   error "unknown processor"
#  endif
# endif
# if !defined _WINDOWS_
#  if !defined _X86_ && defined __i386
#   define _X86_ 1
#  endif
# endif
# if !defined WINVER
#  define WINVER 0x600
# endif
# if !defined _WIN32_WINNT
#  define _WIN32_WINNT 0x600
# endif
# define WIN32_LEAN_AND_MEAN
# include <windef.h>
# include <winbase.h>
# include <excpt.h>
# include <objbase.h>
# include <io.h>
# include <process.h>
# include <malloc.h> /* alloca */
# if defined _MSC_VER && _MSC_VER >= 1600
#  include <intrin.h>
#  pragma intrinsic(_ReadWriteBarrier)
# elif defined _MSC_VER && _MSC_VER >= 1300
   extern void _ReadWriteBarrier();
#  pragma intrinsic(_ReadWriteBarrier)
# else
#  define _ReadWriteBarrier() ((void)0)
# endif
# ifdef _PTHREADS 
#   define _XTHREADS
#   ifdef __GNUC__
#     include <pthread.h>
#   else
#     include "otros/winpthreads.hc" /* testing purposes only! */ 
#   endif
# endif
#else
# include <sys/time.h>
# include <unistd.h>
# include <dlfcn.h>
# include <pthread.h>
# if defined __APPLE__
#   include <malloc/malloc.h> /* malloc_size */
# elif defined __NetBSD__
#   ifndef _NBSDMUTE
#     warning {! NetBSD has malloc_usable_size commented !}
#   endif
#   define malloc_size(Ptr) (0)
# else
#   define malloc_size(Ptr) malloc_usable_size(Ptr)
# endif
#endif

#if defined __APPLE__ || defined __linux__
# include <execinfo.h> /* backtrace */
#elif defined __windoze
# include <imagehlp.h>
# define snprintf _snprintf
  int backtrace( void **cbk, int count );
#elif defined __GNUC__
# include <unwind.h>
  int backtrace( void **cbk, int count );
#else
# define backtrace(Cbk,Count) (0)
#endif

/* Only if x is one-byte symbol! */
#define Isspace(x)   isspace((byte_t)(x))
#define Isalpha(x)   isalpha((byte_t)(x))
#define Isalnum(x)   isalnum((byte_t)(x))
#define Isdigit(x)   isdigit((byte_t)(x))
#define Isxdigit(x)  isxdigit((byte_t)(x))
#define Toupper(x)   toupper((byte_t)(x))
#define Tolower(x)   tolower((byte_t)(x))

/* ATTENTION! int is always less then size_t! use it carefully */
#define iszof(x)     ((int)sizeof(x))
#define iszof_double ((int)sizeof(double))
#define iszof_long   ((int)sizeof(long))
#define iszof_wchar  ((int)sizeof(wchar_t))
#define iszof_arr(x) ((int)(sizeof(x)/sizeof(*x)))

#define __Offset_Of(T,Memb) ((longptr_t)(&((T*)0)->Memb))
#define __To_Ptr(Val) ((void*)((longptr_t)(Val)))

typedef signed   char  ioct_t;
typedef unsigned char  byte_t;

typedef unsigned short ushort_t;
typedef unsigned short uhalf_t; /* 16 bit unsigned ( half word )*/
typedef short          half_t;  /* 16 bit signed ( half word )  */

typedef unsigned int   uint_t;
typedef unsigned int   udword_t; /* 32 bit unsigned ( i386 word ) */
typedef int            dword_t;  /* 32 bit signed ( i386 word )   */

typedef unsigned long  ulong_t;

#ifndef __windoze
  typedef unsigned long long  uquad_t; /* 64-bit unsigned ( double word ) historically named as quad word */
# if !defined __APPLE__ && !defined __linux__
    typedef long long  quad_t; /* 64-bit signed word ( double word ) */
# endif
#else
  typedef unsigned __int64  uquad_t;
  typedef __int64  quad_t;
#endif

/* halflong_t is half of unsigned long value can be 16 or 32 bit, depends on platform */
#if defined __x86_64 && !defined __windoze
  typedef uint_t   halflong_t;  /* windows has 32-bit long always */
#else
  typedef ushort_t halflong_t;
#endif

/* maxint_t is max integer value supporting natively by CPU, depends on platform */
#ifdef __x86_64
  typedef uquad_t   umaxint_t;
  typedef uint_t    halfumi_t;
  typedef quad_t    maxint_t;
  typedef int       halfmi_t;
#else
  typedef ulong_t   umaxint_t;
  typedef ushort_t  halfumi_t;
  typedef int       maxint_t;
  typedef short     halfmi_t;
#endif

/* longptr_t is unsigned integer value enough to store pointer value */
#ifdef __x86_64
  typedef uquad_t  longptr_t; /* windows has 32-bit long always */
#else
  typedef ulong_t  longptr_t;
#endif

/* compatibility with legacy TeggoSoft/MoleStudio code */
typedef uhalf_t  u16_t;
typedef udword_t u32_t;
typedef uquad_t  u64_t;
typedef half_t   i16_t;
typedef dword_t  i32_t;
typedef quad_t   i64_t;

#ifdef __windoze
  #define _WINPOSIX(W,F) W
#else
  #define _WINPOSIX(W,F) F
#endif

#ifndef _NO__FILE__
# define __Yo_FILE__ __FILE__
# define __Yo_Expr__(Expr) #Expr
# define Yo_Raise(Error,Msg,File,Line) _Yo_Raise(Error,Msg,File,Line)
# define Yo_Fatal(Error,Msg,File,Line) _Yo_Fatal(Error,Msg,File,Line)
#else
# define __Yo_FILE__ 0
# define __Yo_Expr__(Expr) 0
# define Yo_Raise(Error,Msg,File,Line) _Yo_Raise(Error,Msg,0,0)
# define Yo_Fatal(Error,Msg,File,Line) _Yo_Fatal(Error,Msg,0,0)
#endif

#define YO_COMPOSE2(a,b) a##b
#define YO_COMPOSE3(a,b,c) a##b##_##c
#define YO_ID(Name,Line) YO_COMPOSE3(_YoYo_Label_,Name,Line)
#define YO_LOCAL_ID(Name) YO_ID(Name,__LINE__)

#ifdef _YO_CORE_BUILTIN
# define _YO_CORE_BUILTIN_CODE(Code) Code
# define _YO_CORE_EXTERN
#else
# define _YO_CORE_BUILTIN_CODE(Code)
# define _YO_CORE_EXTERN extern
#endif

#define Yo_MIN(a,b) ( (a) < (b) ? (a) : (b) )
#define Yo_MAX(a,b) ( (a) > (b) ? (a) : (b) )
#define Yo_ABS(a) ( (a) > 0 ? (a) : -(a) ) /* a > 0  does not produce warning on unsigned types */
#define Yo_ALIGNU(a,n) ( ((a) + ((n) - 1))&~((n) - 1) )

#define YO_REPN_2(Val)   Val,Val
#define YO_REPN_4(Val)   YO_REPN_2(Val),YO_REPN_2(Val)
#define YO_REPN_8(Val)   YO_REPN_4(Val),YO_REPN_4(Val)
#define YO_REPN_16(Val)  YO_REPN_8(Val),YO_REPN_8(Val)
#define YO_REPN_32(Val)  YO_REPN_16(Val),YO_REPN_16(Val)
#define YO_REPN_64(Val)  YO_REPN_32(Val),YO_REPN_32(Val)
#define YO_REPN_128(Val) YO_REPN_64(Val),YO_REPN_64(Val)
#define C32_BIT(No)        (1U<<No)
#define C64_BIT(No)        (1ULL<<No)

_YO_CORE_EXTERN char Oj_Destruct_OjMID[] _YO_CORE_BUILTIN_CODE ( = "~/@" );
_YO_CORE_EXTERN char Oj_Destruct_Element_OjMID[] _YO_CORE_BUILTIN_CODE ( = "~1/@" );
_YO_CORE_EXTERN char Oj_Compare_Elements_OjMID[] _YO_CORE_BUILTIN_CODE ( = "?2/**" );
_YO_CORE_EXTERN char Oj_Compare_Keys_OjMID[] _YO_CORE_BUILTIN_CODE ( = "?3/**" );
_YO_CORE_EXTERN char Oj_Clone_OjMID[] _YO_CORE_BUILTIN_CODE ( = "$=/@" );
_YO_CORE_EXTERN char Oj_Count_OjMID[] _YO_CORE_BUILTIN_CODE ( = "$#/@" );
_YO_CORE_EXTERN char Oj_Set_Key_OjMID[] _YO_CORE_BUILTIN_CODE ( = ">+S>/@*" );
_YO_CORE_EXTERN char Oj_Find_Key_OjMID[] _YO_CORE_BUILTIN_CODE ( = ">?S>/@*" );
_YO_CORE_EXTERN char Oj_Take_Key_OjMID[] _YO_CORE_BUILTIN_CODE ( = ">-S>/@*" );
_YO_CORE_EXTERN char Oj_Del_Key_OjMID[] _YO_CORE_BUILTIN_CODE ( = ">~S>/@*" );
_YO_CORE_EXTERN char Oj_Set_Lkey_OjMID[] _YO_CORE_BUILTIN_CODE ( = ">+L>/@L" );
_YO_CORE_EXTERN char Oj_Find_Lkey_OjMID[] _YO_CORE_BUILTIN_CODE ( = ">?L>/@L" );
_YO_CORE_EXTERN char Oj_Take_Lkey_OjMID[] _YO_CORE_BUILTIN_CODE ( = ">-L>/@L" );
_YO_CORE_EXTERN char Oj_Del_Lkey_OjMID[] _YO_CORE_BUILTIN_CODE ( = ">~L>/@L" );

enum
  {
    KILOBYTE = 1024,
    MEGABYTE = 1024*KILOBYTE,
    GIGABYTE = 1024*MEGABYTE,
  };

enum _YO_ERRORS
  {
    YO_FATAL_ERROR_GROUP          = 0x70000000,
    YO_USER_ERROR_GROUP           = 0x00010000,
    YO_IO_ERROR_GROUP             = 0x00020000,
    //YO_TCPIP_ERROR_GROUP          = 0x00040000,
    YO_RUNTIME_ERROR_GROUP        = 0x00080000,
    YO_SELFCHECK_ERROR_GROUP      = 0x00100000,
    YO_ENCODING_ERROR_GROUP       = 0x00200000,
    YO_ILLFORMED_ERROR_GROUP      = 0x00400000,
    YO_RANGE_ERROR_GROUP          = 0x00800000,
    YO_CORRUPTED_ERROR_GROUP      = 0x01000000,
    YO_STORAGE_ERROR_GROUP        = 0x02000000,
    //YO_SYSTEM_ERROR_GROUP         = 0x04000000,

    YO_XXXX_ERROR_GROUP           = 0x7fff0000,
    YO_RERAISE_CURRENT_ERROR      = 0x7fff7fff,

    YO_TRACED_ERROR_GROUP         = YO_FATAL_ERROR_GROUP
                                   |YO_RANGE_ERROR_GROUP
                                   |YO_SELFCHECK_ERROR_GROUP,

    YO_ERROR_BASE                 = 0x00008000,
    YO_ERROR_USER                 = YO_USER_ERROR_GROUP|0,

    YO_ERROR_OUT_OF_MEMORY    = YO_FATAL_ERROR_GROUP|(YO_ERROR_BASE+1),
    YO_FATAL_ERROR            = YO_FATAL_ERROR_GROUP|(YO_ERROR_BASE+2),
    YO_ERROR_DYNCO_CORRUPTED  = YO_FATAL_ERROR_GROUP|(YO_ERROR_BASE+3),
    YO_ERROR_METHOD_NOT_FOUND = YO_RUNTIME_ERROR_GROUP|(YO_ERROR_BASE+4),
    YO_ERROR_REQUIRE_FAILED   = YO_FATAL_ERROR_GROUP|(YO_ERROR_BASE+5),
    YO_ERROR_ILLFORMED        = YO_ILLFORMED_ERROR_GROUP|(YO_ERROR_BASE+6),
    YO_ERROR_OUT_OF_POOL      = YO_FATAL_ERROR_GROUP|(YO_ERROR_BASE+7),
    YO_ERROR_UNEXPECTED       = YO_FATAL_ERROR_GROUP|(YO_ERROR_BASE+8),
    YO_ERROR_OUT_OF_RANGE     = YO_RANGE_ERROR_GROUP|(YO_ERROR_BASE+9),
    YO_ERROR_NULL_PTR         = YO_FATAL_ERROR_GROUP|(YO_ERROR_BASE+10),
    YO_ERROR_CORRUPTED        = YO_CORRUPTED_ERROR_GROUP|(YO_ERROR_BASE+11),
    YO_ERROR_IO               = YO_IO_ERROR_GROUP|(YO_ERROR_BASE+12),
    YO_ERROR_UNSORTABLE       = YO_RUNTIME_ERROR_GROUP|(YO_ERROR_BASE+13),
    YO_ERROR_DOESNT_EXIST     = YO_IO_ERROR_GROUP|(YO_ERROR_BASE+14),
    YO_ERROR_DSNT_EXIST       = YO_ERROR_DOESNT_EXIST,
    YO_ERROR_ACCESS_DENAIED   = YO_IO_ERROR_GROUP|(YO_ERROR_BASE+15),
    YO_ERROR_NO_ENOUGH        = YO_ILLFORMED_ERROR_GROUP|(YO_ERROR_BASE+16),
    YO_ERROR_UNALIGNED        = YO_ILLFORMED_ERROR_GROUP|(YO_ERROR_BASE+17),
    YO_ERROR_COMPRESS_DATA    = YO_ENCODING_ERROR_GROUP|(YO_ERROR_BASE+18),
    YO_ERROR_ENCRYPT_DATA     = YO_ENCODING_ERROR_GROUP|(YO_ERROR_BASE+19),
    YO_ERROR_DECOMPRESS_DATA  = YO_ENCODING_ERROR_GROUP|(YO_ERROR_BASE+20),
    YO_ERROR_DECRYPT_DATA     = YO_ENCODING_ERROR_GROUP|(YO_ERROR_BASE+21),
    YO_ERROR_INVALID_PARAM    = YO_RUNTIME_ERROR_GROUP|(YO_ERROR_BASE+22),
    YO_ERROR_UNEXPECTED_VALUE = YO_FATAL_ERROR_GROUP|(YO_ERROR_BASE+23),
    YO_ERROR_ALREADY_EXISTS   = YO_IO_ERROR_GROUP|(YO_ERROR_BASE+24),
    YO_ERROR_INCONSISTENT     = YO_STORAGE_ERROR_GROUP|(YO_ERROR_BASE+25),
    YO_ERROR_TO_BIG           = YO_STORAGE_ERROR_GROUP|(YO_ERROR_BASE+26),
    YO_ERROR_ZERODIVIDE       = YO_FATAL_ERROR_GROUP|(YO_ERROR_BASE+27),
    YO_ERROR_LIMIT_REACHED    = YO_RUNTIME_ERROR_GROUP|(YO_ERROR_BASE+28),
    YO_ERROR_UNSUPPORTED      = YO_RUNTIME_ERROR_GROUP|(YO_ERROR_BASE+29),
    YO_ERROR_IO_EOF           = YO_IO_ERROR_GROUP|(YO_ERROR_BASE+30),
    YO_ERROR_DNS              = YO_IO_ERROR_GROUP|(YO_ERROR_BASE+31),
    YO_ERROR_SUBSYSTEM_INIT   = YO_RUNTIME_ERROR_GROUP|(YO_ERROR_BASE+32),
    YO_ERROR_SYSTEM           = YO_RUNTIME_ERROR_GROUP|(YO_ERROR_BASE+33),
    YO_ERROR_SYNTAX           = YO_ILLFORMED_ERROR_GROUP|(YO_ERROR_BASE+34),
    YO_ERROR_TESUITE_FAIL     = YO_SELFCHECK_ERROR_GROUP|(YO_ERROR_BASE+35),
    YO_ERROR_ASSERT_FAIL      = YO_SELFCHECK_ERROR_GROUP|(YO_ERROR_BASE+36),
    YO_ERROR_CONSTANT_NOT_FOUND = YO_RUNTIME_ERROR_GROUP|(YO_ERROR_BASE+37),
  };

#define YO_ERROR_IS_USER_ERROR(err) !(err&YO_XXXX_ERROR_GROUP)

enum _YO_FLAGS
  {
    YO_RAISE_ERROR            = 0x70000000,
    YO_PRINT_FLUSH            = 1,
    YO_PRINT_NEWLINE          = 2,
  };

__No_Return void _Yo_Fatal(int err,void *ctx,char *filename,int lineno);
__No_Return void _Yo_Raise(int err,char *msg,char *filename,int lineno);

#ifdef __windoze
#define Switch_to_Thread() SwitchToThread()
#else
#define Switch_to_Thread() pthread_yield()
#define Sleep(Ms) usleep((Ms)*1000)
#endif

#if defined __GNUC__
# define __RwBarrier() __asm__ __volatile__ ("" ::: "memory")
# define Atomic_Increment(Ptr) __sync_sub_and_fetch((i32_t volatile *)Ptr,1)
# define Atomic_Decrement(Ptr) __sync_sub_and_fetch((i32_t volatile *)Ptr,1)
# define Atomic_CmpXchg(Ptr,Val,Comp) __sync_bool_compare_and_swap((u32_t *volatile)Ptr,(u32_t)Comp,(u32_t)Val)
# define Atomic_CmpXchg_Ptr(Ptr,Val,Comp) __sync_bool_compare_and_swap((void *volatile*)Ptr,(void*)Comp,(void*)Val)
#elif defined _MSC_VER 
# define __RwBarrier() _ReadWriteBarrier()
# define Atomic_Increment(Ptr) InterlockedIncrement(Ptr)
# define Atomic_Decrement(Ptr) InterlockedDecrement(Ptr)
# define Atomic_CmpXchg(Ptr,Val,Comp) (InterlockedCompareExchange(Ptr,Val,Comp) == (Comp))
# define Atomic_CmpXchg_Ptr(Ptr,Val,Comp) (InterlockedCompareExchangePointer(Ptr,Val,Comp) == (Comp))
#endif 

typedef void (*Yo_JMPBUF_Unlock)(void *);
void Yo_JmpBuf_Push_Cs(void *cs,Yo_JMPBUF_Unlock unlock);
void Yo_JmpBuf_Pop_Cs(void *cs);

#define __Interlock_Opt(Decl,Lx,Lock,Unlock,Unlock_Proc) \
  switch ( 0 ) while ( 1 ) \
    if ( 1 ) \
      goto YO_LOCAL_ID(Do_Unlock); \
    else if ( 1 ) \
      case 0: \
        { \
          Decl;\
          Lock(Lx); \
          Yo_JmpBuf_Push_Cs(Lx,(Yo_JMPBUF_Unlock)Unlock_Proc); \
          goto YO_LOCAL_ID(Do_Code); \
        YO_LOCAL_ID(Do_Unlock): \
          Yo_JmpBuf_Pop_Cs(Lx); \
          Unlock(Lx); \
          break; \
        } \
    else \
      YO_LOCAL_ID(Do_Code):


#ifndef _THREADS

# define __Atomic_Increment(Ptr) (++*(Ptr))
# define __Atomic_Decrement(Ptr) (--*(Ptr))
# define __Atomic_CmpXchg(Ptr,Val,Comp) ( *(Ptr) == (Comp) ? (*(Ptr) = (Val), 1) : 0 )
# define __Atomic_CmpXchg_Ptr(Ptr,Val,Comp) ( *(Ptr) == (Comp) ? (*(Ptr) = (Val), 1) : 0 )

# define __Tls_Define(Name)  void *Name = 0
# define __Tls_Declare(Name) extern void *Name
# define __Tls_Set(Name,Val) ((Name) = (Val))
# define __Tls_Get(Name)     (Name)

# define YO_MULTITHREADED(Expr)

# define __Xchg_Interlock if (0) {;} else
# define __Xchg_Sync(Lx)  if (0) {;} else
# define Yo_Wait_Xchg_Lock(Ptr)
# define Yo_Xchg_Unlock(Ptr)

#else /* -D _THREADS */

#if defined _MSC_VER && !defined _PTHREADS && !defined _RTTLS
# define __Tls_Define(Name)  __declspec(thread) void *Name = 0
# define __Tls_Declare(Name) extern __declspec(thread) void *Name
# define __Tls_Set(Name,Val) ((Name) = (Val))
# define __Tls_Get(Name)     (Name)
#else
# define __Tls_Set(Name,Val) Tls_Set(&Name,Val)
# define __Tls_Get(Name)     Tls_Get(Name)
# if defined __windoze && !defined _PTHREADS
#   define INVALID_TLS_VALUE TLS_OUT_OF_INDEXES
#   define __Tls_Define(Name)  ulong_t volatile Name = INVALID_TLS_VALUE;
#   define __Tls_Declare(Name) extern ulong_t volatile Name;

void Tls_Set(ulong_t volatile *name, void *val)
  {
    if ( *name == INVALID_TLS_VALUE ) 
      {
        ulong_t tmp = TlsAlloc();
        if ( InterlockedCompareExchange(name,tmp,INVALID_TLS_VALUE) != INVALID_TLS_VALUE )
          TlsFree(tmp);
      }
    TlsSetValue(*name,val);
  }

void *Tls_Get(ulong_t name)
  {
    if ( name == INVALID_TLS_VALUE ) 
      return 0;
    return TlsGetValue(name);
  }

# else // posix threads
#   define INVALID_TLS_VALUE ((pthread_key_t)-1) 
#   define __Tls_Define(Name)  pthread_key_t Name = INVALID_TLS_VALUE; /* pthread_once way is madness! */
#   define __Tls_Declare(Name) extern pthread_key_t Name;
# endif

void Tls_Set(pthread_key_t volatile *name, void *val)
  {
    if ( *name == INVALID_TLS_VALUE ) 
      {
        pthread_key_t tmp;
        pthread_key_create(&tmp,0);
        if ( !Atomic_CmpXchg(name,tmp,INVALID_TLS_VALUE) )
          pthread_key_delete(tmp);
      }
    pthread_setspecific(*name,val);
  }

void *Tls_Get(pthread_key_t name)
  {
    if ( name == INVALID_TLS_VALUE ) 
      return 0;
    return pthread_getspecific(name);
  }

#endif 

# define __Atomic_Increment(Ptr) Atomic_Increment(Ptr)
# define __Atomic_Decrement(Ptr) Atomic_Decrement(Ptr)
# define __Atomic_CmpXchg(Ptr,Val,Comp) Atomic_CmpXchg(Ptr,Val,Comp)
# define __Atomic_CmpXchg_Ptr(Ptr,Val,Comp) Atomic_CmpXchg_Ptr(Ptr,Val,Comp)

# define YO_MULTITHREADED(Expr) Expr

# define _xchg_YO_LOCAL_LX static int YO_LOCAL_ID(lx)
# define _xchg_YO_LOCAL_ID_REF &YO_LOCAL_ID(lx)
# define __Xchg_Interlock \
              __Interlock_Opt( _xchg_YO_LOCAL_LX, _xchg_YO_LOCAL_ID_REF, \
                  Yo_Wait_Xchg_Lock,Yo_Xchg_Unlock,Yo_Xchg_Unlock_Proc)

# define __Xchg_Sync(Lx) \
              __Interlock_Opt(((void)0),Lx, \
                  Yo_Wait_Xchg_Lock,Yo_Xchg_Unlock,Yo_Xchg_Unlock_Proc)

# define Yo_Wait_Xchg_Lock(Ptr) while ( !__Atomic_CmpXchg(Ptr,1,0) ) Switch_to_Thread()
# define Yo_Xchg_Unlock(Ptr) __Atomic_CmpXchg(Ptr,0,1)
void Yo_Xchg_Unlock_Proc(int volatile *p) _YO_CORE_BUILTIN_CODE({__Atomic_CmpXchg(p,0,1);});

#endif
  
#if defined __GNUC__ && ( __GNUC__ > 4 || (__GNUC__ == 4 && __GNUC_MINOR__>=6) )
  #define __Static_Assert_S(expr,S) _Static_assert(expr,S)
#elif !defined _MSC_VER || _MSC_VER < 1600 || defined __ICL
  #define __Static_Assert_S(expr,S) \
    extern char YO_LOCAL_ID(__assert__)[(expr)?1:-1]
#else
  #define __Static_Assert_S(expr,S) static_assert(expr,S)
#endif

#define __Static_Assert(expr) __Static_Assert_S(expr,#expr)

#define __REQUIRE_FATAL(Expr) Yo_Fatal(YO_ERROR_REQUIRE_FAILED,__Yo_Expr__(Expr),__Yo_FILE__,__LINE__)
#define PANICA(msg) Yo_Fatal(YO_FATAL_ERROR,msg,__Yo_FILE__,__LINE__)
#define UNREACHABLE PANICA("unreachable code")

#if defined __GNUC__ && ( __GNUC__ > 4 || (__GNUC__ == 4 && __GNUC_MINOR__>=5) )
#define ASSUME(Expr)  if (Expr); else __builtin_unreachable();
#define REQUIRE(Expr) if (Expr); else { __REQUIRE_FATAL(Expr); __builtin_unreachable(); }
#elif defined _MSC_VER
#define ASSUME(expr)  __assume(expr)
#define REQUIRE(Expr) if (Expr); else { __REQUIRE_FATAL(Expr); __assume(0); }
#else 
#define ASSUME(expr)  ((void)0)
#define REQUIRE(Expr) if (Expr); else __REQUIRE_FATAL(Expr)
#endif

#ifdef _STRICT
# define STRICT_REQUIRE(Expr) REQUIRE(Expr)
# define STRICT_CHECK(Expr) (Expr)
# define STRICT_UNREACHABLE UNREACHABLE
# define STRICT_ASSUME(Expr) REQUIRE(Expr)
#else
# define STRICT_REQUIRE(Expr) ((void)0)
# define STRICT_CHECK(Expr) (1)
# define STRICT_UNREACHABLE ASSUME(0);
# define STRICT_ASSUME(Expr) ASSUME(Expr)
#endif /* _STRICT */

enum 
  { 
    YO_OBJECT_SIGNATURE_PFX  =  0x00594f59, /*'YOY'*/  
    YO_OBJECT_SIGNATURE_HEAP =  0x4f594f59, /*'YOYO'*/  
    YO_MEMPOOL_PIECE_MAXSIZE = 1*KILOBYTE,
    YO_MEMPOOL_PIECE_ON_BLOCK= 16,
    YO_MEMPOOL_PIECE_STEP    = 64,
    YO_MEMPOOL_SLOTS_COUNT   = YO_MEMPOOL_PIECE_MAXSIZE/YO_MEMPOOL_PIECE_STEP,
  };

__Static_Assert(YO_MEMPOOL_SLOTS_COUNT <= 'O');

#ifdef _YO_CORE_BUILTIN
void **Yo_Mempool_Blocks = 0;
void *Yo_Mempool_Slots[YO_MEMPOOL_SLOTS_COUNT] = {0,};
int Yo_Mempool_Counts[YO_MEMPOOL_SLOTS_COUNT] = {0,};
#endif

#define Slist_Remove_By(ListPtr,Next,Val) Yo_Slist_Remove((void**)ListPtr,(int)((size_t)(&(*ListPtr)->Next)-(size_t)(*ListPtr)),Val)
#define Slist_Remove(ListPtr,Val) Slist_Remove_By(ListPtr,next,Val)

void Yo_Slist_Remove(void **p, int offs_of_next, void *val)
#ifdef _YO_CORE_BUILTIN
  {
    if ( p ) 
      {
        while ( *p )
          {
            if ( *p == val )
              {
                void *r = *p;
                *p = *(void**)((byte_t*)r + offs_of_next);
                *(void**)((byte_t*)r + offs_of_next) = 0;
                break;
              }
            else
              p =  (void**)((byte_t*)*p + offs_of_next);
          }
      }
  }
#endif
  ;

#define Slist_Push_By(ListPtr,Next,Val) Yo_Slist_Push((void**)ListPtr,(int)((size_t)(&(*ListPtr)->Next)-(size_t)(*ListPtr)),Val)
#define Slist_Push(ListPtr,Val) Slist_Push_By(ListPtr,next,Val)

void Yo_Slist_Push(void **p, int offs_of_next, void *val)
#ifdef _YO_CORE_BUILTIN
  {
    if ( p ) 
      {
        while ( *p )
          {
            p =  (void**)((byte_t*)*p + offs_of_next);
          }
        *p = val;
        *(void**)((byte_t*)*p + offs_of_next) = 0;
      }
  }
#endif
  ;
  
#define Slist_Pop_By(ListPtr,Next) Yo_Slist_Pop((void**)ListPtr,(int)((size_t)(&(*ListPtr)->Next)-(size_t)(*ListPtr)))
#define Slist_Pop(ListPtr) Slist_Pop_By(ListPtr,next)

void *Yo_Slist_Pop(void **p, int offs_of_next)
#ifdef _YO_CORE_BUILTIN
  {
    void *r = 0;
    
    if ( p )
      {
        r = *p;
        if ( r ) 
          {
            *p = *(void**)((byte_t*)r + offs_of_next);
            *(void**)((byte_t*)r + offs_of_next) = 0;
          }
      }
      
    return r;
  }
#endif
  ;

#if defined __i386 || defined __x86_64
#define Eight_To_Quad(Eight)  (*(quad_t*)(Eight))
#else
quad_t Eight_To_Quad(void *b)
#ifdef _YO_CORE_BUILTIN
  {
    uint_t q0,q1;
    q0 =   (unsigned int)((unsigned char*)b)[0]
              | ((unsigned int)((unsigned char*)b)[1] << 8)
              | ((unsigned int)((unsigned char*)b)[2] << 16)
              | ((unsigned int)((unsigned char*)b)[3] << 24);
    b = (char*b)+4;
    q1 =   (unsigned int)((unsigned char*)b)[0]
              | ((unsigned int)((unsigned char*)b)[1] << 8)
              | ((unsigned int)((unsigned char*)b)[2] << 16)
              | ((unsigned int)((unsigned char*)b)[3] << 24);
    return (quad_t)q0 | ((quad_t)q1 << 32);
  }
#endif
  ;
#endif

#if defined __i386 || defined __x86_64
#define Quad_To_Eight(Q,Eight) ((*(quad_t*)(Eight)) = (Q))
#else
void Quad_To_Eight(quad_t q, void *b)
#ifdef _YO_CORE_BUILTIN
  {
    byte_t *p = b;
    p[0] = (byte_t)q;
    p[1] = (byte_t)(q>>8);
    p[2] = (byte_t)(q>>16);
    p[3] = (byte_t)(q>>24);
    p[4] = (byte_t)(q>>32);
    p[5] = (byte_t)(q>>40);
    p[6] = (byte_t)(q>>48);
    p[7] = (byte_t)(q>>56);
  }
#endif
  ;
#endif

#if defined __i386 || defined __x86_64
#define Four_To_Unsigned(Four)  (*(uint_t*)(Four))
#else
uint_t Four_To_Unsigned(void *b)
#ifdef _YO_CORE_BUILTIN
  {
    byte_t *p = b;
    uint_t q =   p[0]
              |  (p[1] << 8)
              |  (p[2] << 16)
              |  (p[3] << 24);
    return q;
  }
#endif
  ;
#endif

#if defined __i386 || defined __x86_64
#define Unsigned_To_Four(Uval,Four) ((*(uint_t*)(Four)) = (Uval))
#else
void Unsigned_To_Four(uint_t q, void *b)
#ifdef _YO_CORE_BUILTIN
  {
    byte_t *p = b;
    p[0] = (byte_t)q;
    p[1] = (byte_t)(q>>8);
    p[2] = (byte_t)(q>>16);
    p[3] = (byte_t)(q>>24);
  }
#endif
  ;
#endif

uint_t Four_To_Unsigned_BE(void *b)
#ifdef _YO_CORE_BUILTIN
  {
    byte_t *p = b;
    uint_t q =   p[3]
              |  (p[2] << 8)
              |  (p[1] << 16)
              |  (p[0] << 24);
    return q;
  }
#endif
  ;

void Unsigned_To_Four_BE(uint_t q, void *b)
#ifdef _YO_CORE_BUILTIN
  {
    byte_t *p = b;
    p[3] = (byte_t)q;
    p[2] = (byte_t)(q>>8);
    p[1] = (byte_t)(q>>16);
    p[0] = (byte_t)(q>>24);
  }
#endif
  ;

uint_t Two_To_Unsigned(void *b)
#ifdef _YO_CORE_BUILTIN
  {
    uint_t q =   (unsigned int)((unsigned char*)b)[0]
              | ((unsigned int)((unsigned char*)b)[1] << 8);
    return q;
  }
#endif
  ;

void Unsigned_To_Two(uint_t q, void *b)
#ifdef _YO_CORE_BUILTIN
  {
    byte_t *p = b;
    p[0] = (byte_t)q;
    p[1] = (byte_t)(q>>8);
  }
#endif
  ;

_YO_CORE_EXTERN byte_t Bitcount_8_Q[]
#ifdef _YO_CORE_BUILTIN
= {0, 1, 2, 2, 3, 3, 3, 3, 4, 4, 4, 4, 4, 4, 4, 4,
   5, 5, 5, 5, 5, 5, 5, 5, 5, 5, 5, 5, 5, 5, 5, 5,
   6, 6, 6, 6, 6, 6, 6, 6, 6, 6, 6, 6, 6, 6, 6, 6,
   6, 6, 6, 6, 6, 6, 6, 6, 6, 6, 6, 6, 6, 6, 6, 6,
   7, 7, 7, 7, 7, 7, 7, 7, 7, 7, 7, 7, 7, 7, 7, 7,
   7, 7, 7, 7, 7, 7, 7, 7, 7, 7, 7, 7, 7, 7, 7, 7,
   7, 7, 7, 7, 7, 7, 7, 7, 7, 7, 7, 7, 7, 7, 7, 7,
   7, 7, 7, 7, 7, 7, 7, 7, 7, 7, 7, 7, 7, 7, 7, 7,
   8, 8, 8, 8, 8, 8, 8, 8, 8, 8, 8, 8, 8, 8, 8, 8,
   8, 8, 8, 8, 8, 8, 8, 8, 8, 8, 8, 8, 8, 8, 8, 8,
   8, 8, 8, 8, 8, 8, 8, 8, 8, 8, 8, 8, 8, 8, 8, 8,
   8, 8, 8, 8, 8, 8, 8, 8, 8, 8, 8, 8, 8, 8, 8, 8,
   8, 8, 8, 8, 8, 8, 8, 8, 8, 8, 8, 8, 8, 8, 8, 8,
   8, 8, 8, 8, 8, 8, 8, 8, 8, 8, 8, 8, 8, 8, 8, 8,
   8, 8, 8, 8, 8, 8, 8, 8, 8, 8, 8, 8, 8, 8, 8, 8,
   8, 8, 8, 8, 8, 8, 8, 8, 8, 8, 8, 8, 8, 8, 8, 8}
#endif
 ;

#define Bitcount_8(q) (Bitcount_8_Q[(q)&0x0ff])
uint_t Bitcount_Of( uint_t u )
#ifdef _YO_CORE_BUILTIN
    {
      int i;
      uint_t q;
      if ( u )
        for ( i = sizeof(u)*8-8; i >= 0; i-=8 )
          if ( !!(q = Bitcount_8(u>>i)) )
            return q+i;
      return 0;
    }
#endif
  ;

uint_t Min_Pow2(uint_t a)
#ifdef _YO_CORE_BUILTIN
  {
    if ( a ) --a;
    return 1<<Bitcount_Of(a);
  }
#endif
  ;

int Yo_Mini(int a, int b) _YO_CORE_BUILTIN_CODE({ return Yo_MIN(a,b); });
int Yo_Maxi(int a, int b) _YO_CORE_BUILTIN_CODE({ return Yo_MAX(a,b); });
uint_t Yo_Minu(uint_t a, uint_t b) _YO_CORE_BUILTIN_CODE({ return Yo_MIN(a,b); });
uint_t Yo_Maxu(uint_t a, uint_t b) _YO_CORE_BUILTIN_CODE({ return Yo_MAX(a,b); });
uint_t Yo_Absi(int a) _YO_CORE_BUILTIN_CODE({ return Yo_ABS(a); });

uint_t Align_To_Pow2(uint_t a, uint_t mod)
#ifdef _YO_CORE_BUILTIN
  {
    uint_t Q;
    if ( !mod ) mod = 1;
    Q = Min_Pow2(mod) - 1;
    return (a+Q)&~Q;
  }
#endif
  ;

int Compare_u32(void const *a, void const *b)
#ifdef _YO_CORE_BUILTIN
  {
    return *(u32_t*)a - *(u32_t*)b;
  }
#endif
  ;

int Compare_Int(void const *a, void const *b)
#ifdef _YO_CORE_BUILTIN
  {
    return *(int*)a - *(int*)b;
  }
#endif
  ;


#ifdef __windoze
size_t malloc_size(void *p) _YO_CORE_BUILTIN_CODE({return _msize(p);});
#endif /* __windoze */

void *Yo_Malloc_Npl(int size)
#ifdef _YO_CORE_BUILTIN
  {
    void *p;
    STRICT_REQUIRE(size >= 0);
    p = malloc(size);
    if ( !p )
      Yo_Fatal(YO_ERROR_OUT_OF_MEMORY,0,0,0);
    return p;
  }
#endif
  ;

void *Yo_Realloc_Npl(void *p,int size)
#ifdef _YO_CORE_BUILTIN
  {
    STRICT_REQUIRE(size >= 0);
    p = realloc(p,size);
    if ( !p )
      Yo_Fatal(YO_ERROR_OUT_OF_MEMORY,0,0,0);
    return p;
  }
#endif
  ;

void *Yo_Zero_Malloc_Npl(int size)
#ifdef _YO_CORE_BUILTIN
  {
    void *p = Yo_Malloc_Npl(size);
    memset(p,0,size);
    return p;
  }
#endif
  ;

void *Yo_Resize_Npl(void *p,int size,int granularity)
#ifdef _YO_CORE_BUILTIN
  {
    int capacity = p?malloc_size(p):0;
    STRICT_REQUIRE(size >= 0);
    if ( !p || capacity < size )
      {
        if ( !granularity )
          capacity = Min_Pow2(size);
        else if ( granularity > 1 )
          {
            capacity = size+granularity-1;
            capacity -= capacity % granularity;
          }
        else
          capacity = size;
        p = realloc(p,capacity);
        if ( !p )
          Yo_Fatal(YO_ERROR_OUT_OF_MEMORY,0,0,0);
      }
    return p;
  }
#endif
  ;

void *Yo_Memcopy_Npl(void *src,int size)
#ifdef _YO_CORE_BUILTIN
  {
    void *p;
    STRICT_REQUIRE(size >= 0);
    p = malloc(size);
    if ( !p )
      Yo_Fatal(YO_ERROR_OUT_OF_MEMORY,0,0,0);
    memcpy(p,src,size);
    return p;
  }
#endif
  ;

typedef void (*pool_cleanup_t)(void*); 
typedef struct _YO_AUTORELEASE
  {
    void *ptr;
    pool_cleanup_t cleanup;
  }
  YO_AUTORELEASE;

enum { YO_MAX_ERROR_BTRACE = 25 };

typedef struct _YO_ERROR_INFO
  {
    char *msg;
    char *filename;
    int  code;
    int  lineno;
    int  bt_count;
    void *bt_cbk[YO_MAX_ERROR_BTRACE];
  } YO_ERROR_INFO;

enum { YO_MAX_CS_COUNT = 7 };
enum { YO_INI_JB_COUNT = 5 };
enum { YO_EXT_JB_COUNT = 3 };
enum { YO_INI_POOL_COUNT = 256 };
enum { YO_EXT_POOL_COUNT = 128 };

typedef struct _YO_JMPBUF_LOCK
  {
    void *cs;
    Yo_JMPBUF_Unlock unlock;
  } YO_JMPBUF_LOCK;

typedef struct _YO_JMPBUF
  {
    jmp_buf b;
    YO_JMPBUF_LOCK locks[YO_MAX_CS_COUNT];
    int auto_top;
  } YO_JMPBUF;

typedef struct _YO_C_SUPPORT_INFO
  {
    int auto_count;
    int auto_top;
    int jb_count;
    int jb_top;
    struct
      {
        unsigned unwinding: 1;
      } stats;
    YO_ERROR_INFO err;
    YO_AUTORELEASE *auto_pool;
    YO_JMPBUF jb[YO_INI_JB_COUNT];
  } YO_C_SUPPORT_INFO;

#ifdef _YO_CORE_BUILTIN
__Tls_Define(Yo_Csup_Nfo_Tls);
#else
__Tls_Declare(Yo_Csup_Nfo_Tls);
#endif

YO_C_SUPPORT_INFO *Yo_Acquire_Csup_Nfo()
#ifdef _YO_CORE_BUILTIN
  {
    YO_C_SUPPORT_INFO *nfo = Yo_Malloc_Npl(sizeof(YO_C_SUPPORT_INFO));
    memset(nfo,0,sizeof(*nfo));
    nfo->jb_count = sizeof(nfo->jb)/sizeof(nfo->jb[0]);
    nfo->jb_top = -1;
    nfo->auto_pool = Yo_Malloc_Npl(sizeof(*nfo->auto_pool)*YO_INI_POOL_COUNT);
    nfo->auto_count = YO_INI_POOL_COUNT;
    nfo->auto_top = -1;
    __Tls_Set(Yo_Csup_Nfo_Tls,nfo);
    return nfo;
  }
#endif
  ;

YO_C_SUPPORT_INFO *Yo_C_Support_Nfo()
#ifdef _YO_CORE_BUILTIN
  {
    YO_C_SUPPORT_INFO *nfo = __Tls_Get(Yo_Csup_Nfo_Tls);
    if ( !nfo ) nfo = Yo_Acquire_Csup_Nfo();
    return nfo;
  }
#endif
  ;

YO_C_SUPPORT_INFO *Yo_Extend_Csup_JmpBuf()
#ifdef _YO_CORE_BUILTIN
  {
    YO_C_SUPPORT_INFO *nfo = __Tls_Get(Yo_Csup_Nfo_Tls);
    nfo = Yo_Realloc_Npl(nfo,sizeof(YO_C_SUPPORT_INFO)
              + (nfo->jb_count - YO_INI_JB_COUNT + YO_EXT_JB_COUNT)*sizeof(YO_JMPBUF));
    nfo->jb_count += YO_EXT_JB_COUNT;
    __Tls_Set(Yo_Csup_Nfo_Tls,nfo);
    return nfo;
  }
#endif
  ;

void Yo_Extend_Csup_Autopool()
#ifdef _YO_CORE_BUILTIN
  {
    YO_C_SUPPORT_INFO *nfo = __Tls_Get(Yo_Csup_Nfo_Tls);
    uint_t ncount = nfo->auto_count + YO_EXT_POOL_COUNT;
    nfo->auto_pool = Yo_Realloc_Npl(nfo->auto_pool,sizeof(*nfo->auto_pool)*ncount);
    nfo->auto_count = ncount;
    //nfo->auto_top = ncount;
  }
#endif
  ;

void Yo_Pool_Marker_Tag(void *o) _YO_CORE_BUILTIN_CODE({});


YO_AUTORELEASE *Yo_Find_Ptr_In_Pool(YO_C_SUPPORT_INFO *nfo, void *p)
#ifdef _YO_CORE_BUILTIN
  {
    int n = nfo->auto_top;
    while ( n >= 0 )
      {
        if ( nfo->auto_pool[n].ptr == p )
          return &nfo->auto_pool[n];
        --n;
      }
    return 0;
  }
#endif
  ;

#define Yo_Push_Scope() Yo_Pool_Ptr(0,Yo_Pool_Marker_Tag)
#define Yo_Pool(Ptr) Yo_Pool_Ptr(Ptr,0)

void *Yo_Unrefe(void *p);
void *Yo_Pool_Ptr(void *ptr,void *cleanup)
#ifdef _YO_CORE_BUILTIN
  {
    if ( ptr || cleanup == Yo_Pool_Marker_Tag )
      {
        YO_C_SUPPORT_INFO *nfo = __Tls_Get(Yo_Csup_Nfo_Tls);
        if ( !nfo ) nfo = Yo_Acquire_Csup_Nfo();
        STRICT_REQUIRE( (cleanup == Yo_Pool_Marker_Tag)
                      ||(cleanup == Yo_Unrefe)
                      ||!Yo_Find_Ptr_In_Pool(nfo,ptr) );

        ++nfo->auto_top;
        STRICT_REQUIRE(nfo->auto_top <= nfo->auto_count);

        if ( nfo->auto_top == nfo->auto_count )
          Yo_Extend_Csup_Autopool();
        nfo->auto_pool[nfo->auto_top].ptr = ptr;
        nfo->auto_pool[nfo->auto_top].cleanup = cleanup?cleanup:(void*)free;
      }
    return ptr;
  }
#endif
  ;

#define Yo_Release(Pooled) ((void)Yo_Unpool((Pooled),1))
#define Yo_Retain(Pooled) Yo_Unpool((Pooled),0)

void *Yo_Unpool(void *pooled,int do_cleanup)
#ifdef _YO_CORE_BUILTIN
  {
    YO_C_SUPPORT_INFO *nfo = __Tls_Get(Yo_Csup_Nfo_Tls);
    if ( nfo && pooled )
      {
        int n = nfo->auto_top;
        while ( n >= 0 )
          {
            if ( nfo->auto_pool[n].ptr == pooled )
              {
                YO_AUTORELEASE *q = &nfo->auto_pool[n];
                if ( do_cleanup && q->ptr ) q->cleanup(q->ptr);

                if ( nfo->jb_top < 0 || nfo->jb[nfo->jb_top].auto_top < n )
                  {
                    if ( nfo->auto_top != n )
                        memmove(nfo->auto_pool+n,nfo->auto_pool+n+1,(nfo->auto_top-n)*sizeof(*q));
                    --nfo->auto_top;
                  }
                else
                  {
                    q->ptr = 0;
                    q->cleanup = 0;
                  }

                break; // while
              }
            --n;
          }
      }
    return pooled;
  }
#endif
  ;

void *Yo_Unwind_Scope(void *pooled,int min_top,void *mark)
#ifdef _YO_CORE_BUILTIN
  {
    YO_C_SUPPORT_INFO *nfo = __Tls_Get(Yo_Csup_Nfo_Tls);
    int L = min_top>=0?min_top:0;
    if ( nfo )
      {
        YO_AUTORELEASE *q_p = 0;
        nfo->stats.unwinding = 1;
        while ( nfo->auto_top >= L )
          {
            YO_AUTORELEASE *q = &nfo->auto_pool[nfo->auto_top];
            STRICT_REQUIRE(nfo->auto_top <= nfo->auto_count);
            if ( q->ptr && (q->cleanup != Yo_Pool_Marker_Tag) )
              {
                if ( !pooled || q->ptr != pooled )
                  {
                    q->cleanup(q->ptr);
                  }
                else
                  q_p = q;
              }
            --nfo->auto_top;
            if ( q->cleanup == Yo_Pool_Marker_Tag && !min_top && mark == q->ptr )
              break;
          }
        REQUIRE(nfo->auto_top < nfo->auto_count);
        if ( q_p )
          {
            ++nfo->auto_top;
            nfo->auto_pool[nfo->auto_top] = *q_p;
          }
        nfo->stats.unwinding = 0;
      }
    return pooled;
  }
#endif
  ;

#define Yo_Refresh(Old,New) Yo_Refresh_Ptr(Old,New,0)
void *Yo_Refresh_Ptr(void *old,void *new,void *cleaner)
#ifdef _YO_CORE_BUILTIN
  {
    YO_C_SUPPORT_INFO *nfo;
    REQUIRE( new != 0 );
    if ( old && !!(nfo = __Tls_Get(Yo_Csup_Nfo_Tls)) )
      {
        YO_AUTORELEASE *p = Yo_Find_Ptr_In_Pool(nfo,old);
        if ( !p ) Yo_Fatal(YO_ERROR_OUT_OF_POOL,old,0,0);
        p->ptr = new;
      }
    else
      Yo_Pool_Ptr(new,cleaner);
    return new;
  }
#endif
  ;

enum { YO_DEFAULT_PURGE_CAP = 5 };

int Yo_Pool_Purge(int *thold, int cap)
#ifdef _YO_CORE_BUILTIN
  {
    return 1;
  }
#endif
  ;

void Yo_Thread_Cleanup()
#ifdef _YO_CORE_BUILTIN
  {
    YO_C_SUPPORT_INFO *nfo;
    Yo_Unwind_Scope(0,-1,0);
    if ( !!(nfo = __Tls_Get(Yo_Csup_Nfo_Tls)) )
      {
        free(nfo->err.msg);
        free(nfo->auto_pool);
        free(nfo);
        __Tls_Set(Yo_Csup_Nfo_Tls,0);
      }
  }
#endif
  ;

void Yo_Global_Cleanup()
#ifdef _YO_CORE_BUILTIN
  {
    while ( Yo_Mempool_Blocks )
      {
        void **Q = Yo_Mempool_Blocks;
        Yo_Mempool_Blocks = *Q;
        free(Q[1]);
        free(Q);
      }
  }
#endif
  ;

void *Yo_Malloc(unsigned size)
  _YO_CORE_BUILTIN_CODE({return Yo_Pool(Yo_Malloc_Npl(size));});
void *Yo_Realloc(void *p,unsigned size)
  _YO_CORE_BUILTIN_CODE({return Yo_Refresh(p,Yo_Realloc_Npl(p,size));});
void *Yo_Memcopy(void *p,unsigned size)
  _YO_CORE_BUILTIN_CODE({return Yo_Pool(Yo_Memcopy_Npl(p,size));});

void *Yo_Resize(void *p,unsigned size,int granularity)
#ifdef _YO_CORE_BUILTIN
  {
    void *q = Yo_Resize_Npl(p,size,granularity);
    if ( p && q != p )
      Yo_Refresh(p,q);
    else if ( !p )
      Yo_Pool(q);
    return q;
  }
#endif
  ;

#if defined _MSC_VER && _MSC_VER < 1300
#  define __VSCPRINTF
#endif

#if defined _YO_CORE_BUILTIN && defined __windoze && defined __VSCPRINTF
int _vscprintf(char *fmt,va_list va)
  {
    static char simulate[4096*4] = {0};
    return vsprintf(simulate,fmt,va);
  }
#endif

int Yo_Detect_Required_Buffer_Size(char *fmt,va_list va)
#ifdef _YO_CORE_BUILTIN
  {
  #ifdef __windoze
    return _vscprintf(fmt,va)+1;
  #else
    va_list qva;
    va_copy(qva,va);
    return vsnprintf(0,0,fmt,qva)+1;
  #endif
  }
#endif
  ;

char *Yo_Format_(char *fmt,va_list va)
#ifdef _YO_CORE_BUILTIN
  {
    int rq_len = Yo_Detect_Required_Buffer_Size(fmt,va)+1;
    char *b = Yo_Malloc_Npl(rq_len);
  #ifdef __windoze
    vsprintf(b,fmt,va);
  #else
    vsnprintf(b,rq_len,fmt,va);
  #endif
    return b;
  }
#endif
  ;

char *Yo_Format_Npl(char *fmt,...)
  _YO_CORE_BUILTIN_CODE({va_list va;char *t; va_start(va,fmt); t = Yo_Format_(fmt,va);va_end(va); return t;});

char *Yo_Format(char *fmt,...)
  _YO_CORE_BUILTIN_CODE({va_list va;char *t; va_start(va,fmt); t = Yo_Pool(Yo_Format_(fmt,va));va_end(va); return t;});

#define StdOut_Print(Text) Yo_Print_FILE(stdout,Text,YO_PRINT_FLUSH)
#define StdOut_Print_Nl(Text) Yo_Print_FILE(stdout,Text,YO_PRINT_FLUSH|YO_PRINT_NEWLINE)
#define StdErr_Print(Text) Yo_Print_FILE(stderr,Text,YO_PRINT_FLUSH)
#define StdErr_Print_Nl(Text) Yo_Print_FILE(stderr,Text,YO_PRINT_FLUSH|YO_PRINT_NEWLINE)

typedef struct _YO_FUNCTABLE
  {
    char *name;
    void *func;
  } YO_FUNCTABLE;

typedef struct _YO_CORE_DYNAMIC
  {
    longptr_t contsig;
    longptr_t typeid;
    YO_FUNCTABLE funcs[1];
  }
  YO_DYNAMIC;

typedef struct _YO_CORE_OBJECT
  {
    uint_t signature; /* YO_OBJECT_SIGNATURE */
    uint_t rc;
    YO_DYNAMIC *dynamic;
  }
  YO_OBJECT;

#define YO_BASE(Ptr)          ((YO_OBJECT*)(Ptr) - 1)
#define YO_RC(Ptr)            (YO_BASE(Ptr)->rc)
#define YO_SIGNAT(Ptr)        (YO_BASE(Ptr)->signature)
#define YO_SIGNAT_IS_OK(Ptr)  ((YO_BASE(Ptr)->signature&0x00ffffff) == YO_OBJECT_SIGNATURE_PFX)

void *Yo_Unrefe(void *p);

#ifdef _YO_CORE_BUILTIN
uint_t Yo_Typeid_Counter = 0;
#endif

enum { YO_DYNCO_NYD = 0x4e5944/*'NYD'*/, YO_DYNCO_ATS = 0x415453/*'ATS'*/ };

void *Yo_Clone_Dynamic( YO_DYNAMIC *dynco, int extra )
#ifdef _YO_CORE_BUILTIN
  {
    int count = dynco->contsig&0x0ff;
    int fc = count?count-1:0;
    int fcc = (count+extra)?count+extra-1:0;
    YO_DYNAMIC *d = Yo_Malloc_Npl(sizeof(YO_DYNAMIC)+sizeof(YO_FUNCTABLE)*fcc);
    *d = *dynco;
    if ( fc )
      memcpy(d->funcs+1,dynco->funcs+1,sizeof(YO_FUNCTABLE)*fc);
    d->contsig = (YO_DYNCO_NYD<<8)|count;
    return d;
  }
#endif
  ;

void *Yo_Extend_Dynamic( YO_DYNAMIC *dynco, int extra )
#ifdef _YO_CORE_BUILTIN
  {
    int count = dynco->contsig&0x0ff;
    int fcc = (count+extra)?count+extra-1:0;
    YO_DYNAMIC *d = Yo_Realloc_Npl(dynco,sizeof(YO_DYNAMIC)+sizeof(YO_FUNCTABLE)*fcc);
    return d;
  }
#endif
  ;

void *Yo_Object_Extend( void *o, char *func_name, void *func )
#ifdef _YO_CORE_BUILTIN
  {
    YO_OBJECT *T = YO_BASE(o);
    YO_FUNCTABLE *f;
    if ( !T )
      Yo_Raise(YO_ERROR_NULL_PTR,__yoTa("failed to extend nullptr",0),__Yo_FILE__,__LINE__);

    if ( T->dynamic )
      {
        if ( (T->dynamic->contsig >> 8) == YO_DYNCO_ATS )
          T->dynamic = Yo_Clone_Dynamic(T->dynamic,1);
        else
          T->dynamic = Yo_Extend_Dynamic(T->dynamic,1);
      }
    else
      {
        T->dynamic = Yo_Malloc_Npl(sizeof(YO_DYNAMIC));
        T->dynamic->contsig = YO_DYNCO_NYD<<8;
      }

    T->dynamic->typeid = __Atomic_Increment(&Yo_Typeid_Counter);
    f = T->dynamic->funcs+(T->dynamic->contsig&0x0ff);
    ++T->dynamic->contsig;
    f->name = func_name;
    f->func = func;

    return o;
  }
#endif
  ;

void Yo_Mempool_Free(void *p,int plidx)
#ifdef _YO_CORE_BUILTIN
  {
    void **Q = p;
    STRICT_REQUIRE(plidx >= 0 && plidx <YO_MEMPOOL_SLOTS_COUNT); 
    for (;;)
      {
        void * volatile *S = Yo_Mempool_Slots+plidx;
        *Q = *S;
        if ( __Atomic_CmpXchg_Ptr(S,Q,*Q) ) break;
      }
  }
#endif
  ;
  
void Yo_Mempool_Extend(int plidx)
#ifdef _YO_CORE_BUILTIN
  {
    __Xchg_Interlock
      {
        void * volatile *S = Yo_Mempool_Slots+plidx;
        if ( !*S )
          {
            int i,piece = (plidx+1)*YO_MEMPOOL_PIECE_STEP; 
            void **Q = Yo_Malloc_Npl(piece*YO_MEMPOOL_PIECE_ON_BLOCK);
            void **R = Yo_Malloc_Npl(sizeof(void*)*2);
            R[1] = Q;
            R[0] = Yo_Mempool_Blocks;
            Yo_Mempool_Blocks = R;
            Yo_Mempool_Counts[plidx] += YO_MEMPOOL_PIECE_ON_BLOCK;
            for ( i=0; i < YO_MEMPOOL_PIECE_ON_BLOCK; ++i, Q = (void**)((byte_t*)Q + piece) )
              for (;;)
                {
                  *Q = *S;
                  if ( __Atomic_CmpXchg_Ptr(S,Q,*Q) ) break;
                }
          }
      }
  }
#endif
  ;
  
void *Yo_Mempool_Zalloc(int plidx)
#ifdef _YO_CORE_BUILTIN
  {
    void **Q;
    STRICT_REQUIRE(plidx >= 0 && plidx <YO_MEMPOOL_SLOTS_COUNT); 
    for (;;)
      {
        void * volatile *S = Yo_Mempool_Slots+plidx;
        Q = *S;
        if ( !Q ) 
          Yo_Mempool_Extend(plidx);
        else
          if ( __Atomic_CmpXchg_Ptr(S,*Q,Q) ) break;
      }
    memset(Q,0,(plidx+1)*YO_MEMPOOL_PIECE_STEP);
    return Q;              
  }
#endif
  ;

YO_OBJECT *Yo_Object_Alloc(int size)
#ifdef _YO_CORE_BUILTIN
  {
    YO_OBJECT *o;
    if ( size + sizeof(YO_OBJECT) > YO_MEMPOOL_PIECE_MAXSIZE )
      {
        o = Yo_Zero_Malloc_Npl(sizeof(YO_OBJECT)+size);
        o->signature = YO_OBJECT_SIGNATURE_HEAP;
      }
    else
      {
        int plidx = (sizeof(YO_OBJECT)+size-1)/YO_MEMPOOL_PIECE_STEP;
        o = Yo_Mempool_Zalloc(plidx);
        o->signature = YO_OBJECT_SIGNATURE_PFX + (plidx<<24);
      }
    return o;
  }
#endif
  ;

void Yo_Object_Free(YO_OBJECT *o)
#ifdef _YO_CORE_BUILTIN
  {
    int plidx = o->signature >> 24;
    if ( plidx >= YO_MEMPOOL_SLOTS_COUNT ) 
      free(o);
    else
      Yo_Mempool_Free(o,plidx);
  }
#endif
  ;

void *Yo_Object_Clone(int size, void *orign)
#ifdef _YO_CORE_BUILTIN
  {
    YO_OBJECT *o;
    YO_OBJECT *T = YO_BASE(orign);
    if ( !T )
      Yo_Raise(YO_ERROR_NULL_PTR,__yoTa("failed to clone nullptr",0),__Yo_FILE__,__LINE__);

    o = Yo_Object_Alloc(size);
    o->rc = 1;
    memcpy(o+1,orign,size);

    if ( T->dynamic )
      {
        if ( (T->dynamic->contsig>>8) == YO_DYNCO_ATS )
          o->dynamic = T->dynamic;
        else
          {
            STRICT_REQUIRE( (T->dynamic->contsig>>8) == YO_DYNCO_NYD );
            o->dynamic = Yo_Clone_Dynamic(T->dynamic,0);
          }
      }
    else
      o->dynamic = 0;

    return Yo_Pool_Ptr(o+1,Yo_Unrefe);
  }
#endif
  ;

void *Yo_Object(int size,YO_FUNCTABLE *tbl)
#ifdef _YO_CORE_BUILTIN
  {
    YO_OBJECT *o = Yo_Object_Alloc(size);
    o->rc = 1;
    o->dynamic = (YO_DYNAMIC*)tbl;

    if ( tbl )
      {__Xchg_Interlock
        {
          YO_DYNAMIC *dynco = (YO_DYNAMIC*)tbl;
          if ( !dynco->contsig )
            {
              int count;
              for ( count = 0; tbl[count+1].name; ) { ++count; }
              dynco->contsig = (YO_DYNCO_ATS<<8)|count;
              dynco->typeid = __Atomic_Increment(&Yo_Typeid_Counter);
            }
        }}

    return Yo_Pool_Ptr(o+1,Yo_Unrefe);
  }
#endif
  ;

ulong_t Yo_Align(ulong_t val)
#ifdef _YO_CORE_BUILTIN
  {
    return (val + 7)&~7;
  }
#endif
  ;

void *Yo_Object_Dtor(int size,void *dtor)
#ifdef _YO_CORE_BUILTIN
  {
    int Sz = Yo_Align(sizeof(YO_OBJECT)+size);
    YO_OBJECT *o = Yo_Object_Alloc(Sz+sizeof(YO_DYNAMIC));
    o->rc = 1;
    o->dynamic = (YO_DYNAMIC*)((char*)o + Sz);
    o->dynamic->contsig = (YO_DYNCO_ATS<<8)|1;
    o->dynamic->funcs[0].name = Oj_Destruct_OjMID;
    o->dynamic->funcs[0].func = dtor;
    o->dynamic->typeid = __Atomic_Increment(&Yo_Typeid_Counter);
    return Yo_Pool_Ptr(o+1,Yo_Unrefe);
  }
#endif
  ;

void Yo_Object_Destruct(void *ptr)
#ifdef _YO_CORE_BUILTIN
  {
    if ( ptr )
      {
        YO_OBJECT *o = (YO_OBJECT *)ptr - 1;
        if ( o->dynamic && (o->dynamic->contsig>>8) == YO_DYNCO_NYD )
          free(o->dynamic);
        o->dynamic = 0;
        Yo_Object_Free(o);
      }
  }
#endif
  ;

void *Yo_Find_Method_In_Table(char *name, YO_FUNCTABLE *tbl, int count, int flags)
#ifdef _YO_CORE_BUILTIN
  {
    int i;
    for ( i = 0; i < count; ++i )
      if ( strcmp(tbl[i].name,name) == 0 )
        return tbl[i].func;
    return 0;
  }
#endif
  ;

void *Yo_Find_Method_Of(void *self /* VOID**  */, char *name, unsigned flags)
#ifdef _YO_CORE_BUILTIN
  {
    void *o = *(void**)self;
    if ( o && STRICT_CHECK(YO_SIGNAT_IS_OK(o)) )
      {
        YO_DYNAMIC *dynco = YO_BASE(o)->dynamic;
        if ( dynco )
          {
            if ( 1 && STRICT_CHECK((dynco->contsig>>8) == YO_DYNCO_ATS || (dynco->contsig>>8) == YO_DYNCO_NYD) )
              {
                void *f = Yo_Find_Method_In_Table(name,dynco->funcs,(dynco->contsig&0x0ff),flags);
                if ( !f && (flags & YO_RAISE_ERROR) )
                  Yo_Raise(YO_ERROR_METHOD_NOT_FOUND,name,__Yo_FILE__,__LINE__);
                return f;
              }
            else
              Yo_Fatal(YO_ERROR_DYNCO_CORRUPTED,o,__Yo_FILE__,__LINE__);
          }
        else if (flags & YO_RAISE_ERROR)
          Yo_Raise(YO_ERROR_METHOD_NOT_FOUND,name,__Yo_FILE__,__LINE__);
      }
    else if (flags & YO_RAISE_ERROR)
      Yo_Raise(YO_ERROR_METHOD_NOT_FOUND,name,__Yo_FILE__,__LINE__);
    return 0;
  }
#endif
  ;

longptr_t Yo_Find_Constant_Of(void *o, char *name, unsigned flags, longptr_t dflt)
#ifdef _YO_CORE_BUILTIN
  {
    if ( o && STRICT_CHECK(YO_SIGNAT_IS_OK(o)) )
      {
        YO_DYNAMIC *dynco = YO_BASE(o)->dynamic;
        if ( dynco )
          {
            if ( 1 && STRICT_CHECK((dynco->contsig>>8) == YO_DYNCO_ATS || (dynco->contsig>>8) == YO_DYNCO_NYD) )
              {
                void *f = Yo_Find_Method_In_Table(name,dynco->funcs,(dynco->contsig&0x0ff),flags);
                if ( !f && (flags & YO_RAISE_ERROR) )
                  Yo_Raise(YO_ERROR_CONSTANT_NOT_FOUND,name,__Yo_FILE__,__LINE__);
                return (longptr_t)f;
              }
            else
              Yo_Fatal(YO_ERROR_DYNCO_CORRUPTED,o,__Yo_FILE__,__LINE__);
          }
        else if (flags & YO_RAISE_ERROR)
          Yo_Raise(YO_ERROR_CONSTANT_NOT_FOUND,name,__Yo_FILE__,__LINE__);
      }
    else if (flags & YO_RAISE_ERROR)
      Yo_Raise(YO_ERROR_CONSTANT_NOT_FOUND,name,__Yo_FILE__,__LINE__);
    return dflt;
  }
#endif
  ;
    
void *Yo_Refe(void *p)
#ifdef _YO_CORE_BUILTIN
  {
    if ( p && STRICT_CHECK(YO_SIGNAT_IS_OK(p)) )
      __Atomic_Increment(&YO_RC(p));
    return p;
  }
#endif
  ;

#ifdef _YO_CORE_BUILTIN
void *Yo_Unrefe(void *p)
  {
    if ( p && STRICT_CHECK(YO_SIGNAT_IS_OK(p))
           && !(__Atomic_Decrement(&YO_RC(p))&0x7fffff) )
      {
        void (*destruct)(void *) = Yo_Find_Method_Of(&p,Oj_Destruct_OjMID,0);
        if ( !destruct )
          Yo_Object_Destruct(p);
        else
          destruct(p);
        return 0;
      }
    return p;
  }
#endif
  ;

void *Oj_Clone(void *p)
#ifdef _YO_CORE_BUILTIN
  {
    if ( p )
      {
        void *(*clone)(void *) = Yo_Find_Method_Of(&p,Oj_Clone_OjMID,YO_RAISE_ERROR);
        return clone(p);
      }
    return p;
  }
#endif
  ;

int Oj_Count(void *self)
#ifdef _YO_CORE_BUILTIN
  {
    int (*count)(void *) = Yo_Find_Method_Of(&self,Oj_Count_OjMID,YO_RAISE_ERROR);
    return count(self);
  }
#endif
  ;


#define __Try __Try_Specific(default: Yo_Raise_Occured())
#define __Try_Abort __Try_Specific(default: Error_Abort())
#define __Try_Exit(pfx) __Try_Specific(default: Error_Exit(pfx))
#define __Try_Except __Try_Specific((void)0)

#define __Try_Specific(What)  \
  switch ( setjmp(Yo_Push_JmpBuf()->b) ) \
    if (1) /* guards exception way */ while (1) \
      if (1) /* on second while's step if executed without errors */ \
        { Yo_Pop_JmpBuf(); break; } \
      else if (0) /* if unexpected */ \
        /* default: */ What; \
      else /* there is protected code */ \
        /* switch jumps to here */ \
        case 0:

#define __Catch(Code) \
    else if (0) /* else branch of guards if */ \
      case (Code):

#define __Except /* using with __Try_Except */ \
    else /* else branch of guards if */ \
      default:

#define __Enter_Once(Flag) if ( Flag ); else __Safe_Prolog_Epilog(Flag=1,Flag=0)

#define __Safe_Prolog_Epilog(Prolog,Epilog)  \
  switch ( setjmp(Yo_Push_JmpBuf()->b) ) \
    if (1) /* guards exception way */ while (1) \
      if (1) /* on second while's step if executed without errors */ \
        { Epilog; Yo_Pop_JmpBuf(); break; } \
      else if (0) /* if unexpected */ \
        { Epilog; Yo_Raise_Occured(); } \
      else /* there is protected code */ \
        /* switch jumps to here */ \
        case 0: if (1) { Prolog; goto YO_LOCAL_ID(ap_Body); } else \
           YO_LOCAL_ID(ap_Body):

YO_ERROR_INFO *Error_Info()
#ifdef _YO_CORE_BUILTIN
  {
    YO_C_SUPPORT_INFO *nfo = __Tls_Get(Yo_Csup_Nfo_Tls);
    if ( nfo && nfo->err.code )
      return &nfo->err;
    else
      return 0;
  }
#endif
  ;

char *Error_Message(void)
#ifdef _YO_CORE_BUILTIN
  {
    YO_ERROR_INFO *info = Error_Info();
    if ( info && info->msg )
      return info->msg;
    return "";
  }
#endif
  ;

int Error_Code(void)
#ifdef _YO_CORE_BUILTIN
  {
    YO_ERROR_INFO *info = Error_Info();
    if ( info )
      return info->code;
    return 0;
  }
#endif
  ;

char *Error_File(void)
#ifdef _YO_CORE_BUILTIN
  {
    YO_ERROR_INFO *info = Error_Info();
    if ( info && info->filename )
      return info->filename;
    return __yoTa("<file>",0);
  }
#endif
  ;

int Error_Line(void)
#ifdef _YO_CORE_BUILTIN
  {
    YO_ERROR_INFO *info = Error_Info();
    if ( info )
      return info->lineno;
    return 0;
  }
#endif
  ;

void Yo_Print_FILE(FILE *st, char *text, unsigned flags)
#ifdef _YO_CORE_BUILTIN
  {
    __Xchg_Interlock
      {
        fputs(text,st);
        if ( flags & YO_PRINT_NEWLINE ) fputc('\n',st);
        if ( flags & YO_PRINT_FLUSH ) fflush(st);
      }
  }
#endif
  ;

char *Error_Print_N_Exit(char *prefix, int code)
#ifdef _YO_CORE_BUILTIN
  {
    StdErr_Print_Nl(Yo_Format(__yoTa("%s: %s",0),prefix,Error_Message()));
    exit(code);
  }
#endif
  ;

#define Yo_Pop_JmpBuf() \
   (--((YO_C_SUPPORT_INFO *)__Tls_Get(Yo_Csup_Nfo_Tls))->jb_top)

YO_JMPBUF *Yo_Push_JmpBuf(void)
#ifdef _YO_CORE_BUILTIN
  {
    YO_C_SUPPORT_INFO *nfo = Yo_C_Support_Nfo();
    YO_JMPBUF *jb;

    STRICT_REQUIRE(nfo->jb_top < nfo->jb_count);
    STRICT_REQUIRE(nfo->jb_top >= -1);

    if ( nfo->jb_top == nfo->jb_count-1 )
      nfo = Yo_Extend_Csup_JmpBuf();
    ++nfo->jb_top;

    jb = &nfo->jb[nfo->jb_top];
    memset(jb->locks,0,sizeof(jb->locks));
    jb->auto_top = nfo->auto_top+1;

    return jb;
  }
#endif
  ;

void Yo_JmpBuf_Push_Cs(void *cs,Yo_JMPBUF_Unlock unlock)
#ifdef _YO_CORE_BUILTIN
  {
    YO_C_SUPPORT_INFO *nfo = __Tls_Get(Yo_Csup_Nfo_Tls);
    STRICT_REQUIRE ( cs );
    if ( nfo && cs )
      {
        STRICT_REQUIRE(nfo->jb_top < nfo->jb_count);
        STRICT_REQUIRE(nfo->jb_top >= -1);
        if ( nfo->jb_top > -1 && !nfo->stats.unwinding )
          {
            int i;
            YO_JMPBUF_LOCK *locks = nfo->jb[nfo->jb_top].locks;
            for ( i = YO_MAX_CS_COUNT-1; i >= 0; --i )
              if ( !locks[i].cs )
                {
                  locks[i].cs = cs;
                  locks[i].unlock = unlock;
                  return;
                }
            Yo_Fatal(YO_FATAL_ERROR,__yoTa("no enough lock space",0),__Yo_FILE__,__LINE__);
          }
      }
  }
#endif
  ;


void Yo_JmpBuf_Pop_Cs(void *cs)
#ifdef _YO_CORE_BUILTIN
  {
    YO_C_SUPPORT_INFO *nfo = __Tls_Get(Yo_Csup_Nfo_Tls);
    if ( nfo && cs )
      {
        STRICT_REQUIRE(nfo->jb_top < nfo->jb_count);
        STRICT_REQUIRE(nfo->jb_top >= -1);
        if ( nfo->jb_top > -1 && !nfo->stats.unwinding )
          {
            int i;
            YO_JMPBUF_LOCK *locks = nfo->jb[nfo->jb_top].locks;
            for ( i = YO_MAX_CS_COUNT-1; i >= 0; --i )
              if ( locks[i].cs == cs )
                {
                  memset(&locks[i],0,sizeof(locks[i]));
                  return;
                }
            Yo_Fatal(YO_FATAL_ERROR,__yoTa("trying to pop unexistent lock",0),__Yo_FILE__,__LINE__);
          }
      }
  }
#endif
  ;

char *Yo_Btrace(void);

#ifdef _YO_CORE_BUILTIN
__No_Return void _Yo_Raise(int err,char *msg,char *filename,int lineno)
  {
    YO_C_SUPPORT_INFO *nfo = __Tls_Get(Yo_Csup_Nfo_Tls);
    STRICT_REQUIRE( !nfo || nfo->jb_top < nfo->jb_count );

    //printf(__yoTa("err: %d, msg: %s, filename: %s, lineno: %d\n",0),err,msg,filename,lineno);
  
  #if defined _DEBUG && defined __windoze
    if ( IsDebuggerPresent() ) __debugbreak();
  #endif  
  
    if ( err == YO_RERAISE_CURRENT_ERROR && (!nfo || !nfo->err.code) )
      Yo_Fatal(YO_ERROR_UNEXPECTED,__yoTa("no errors occured yet",0),filename,lineno);

    if ( nfo && nfo->jb_top >= 0 && !nfo->stats.unwinding )
      {
        int i;
        char *old_msg = nfo->err.msg;
        YO_JMPBUF_LOCK *locks = nfo->jb[nfo->jb_top].locks;

        if ( err != YO_RERAISE_CURRENT_ERROR )
          {
            nfo->err.msg = msg ? strdup(msg) : 0;
            nfo->err.code = err?err:-1;
            nfo->err.filename = filename;
            nfo->err.lineno = lineno;
            nfo->err.bt_count = backtrace(nfo->err.bt_cbk,YO_MAX_ERROR_BTRACE);
            free( old_msg );
          }

        for ( i = YO_MAX_CS_COUNT-1; i >= 0; --i )
          if (  locks[i].cs )
            locks[i].unlock(locks[i].cs);

        Yo_Unwind_Scope(0,nfo->jb[nfo->jb_top].auto_top,0);

        --nfo->jb_top;
        STRICT_REQUIRE(nfo->jb_top >= -1);

      #ifdef _TRACEXPT
        StdErr_Print_Nl(Yo_Btrace());
      #endif
        longjmp(nfo->jb[nfo->jb_top+1].b,err?err:-1);
      }
    else
      {
        if ( err != YO_RERAISE_CURRENT_ERROR )
          Yo_Fatal(err,msg,filename,lineno);
        else
          Yo_Fatal(nfo->err.code,nfo->err.msg,nfo->err.filename,nfo->err.lineno);
      }
  }
#endif

#define Yo_Raise_Occured() _Yo_Raise(YO_RERAISE_CURRENT_ERROR,0,0,0)

__No_Return void Yo_Abort(char *msg)
#ifdef _YO_CORE_BUILTIN
  {
    StdErr_Print_Nl(msg);
    abort();
  }
#endif
  ;

char *Yo__basename(char *S)
#ifdef _YO_CORE_BUILTIN
  {
    if ( S )
      {
        char *a = strrchr(S,'/');
        char *b = strrchr(S,'\\');
        if ( b > a ) a = b;
        return a ? a+1 : S;
      }
    return 0;
  }
#endif
  ;

char *Yo_Btrace_Format(int frames, void **cbk)
#ifdef _YO_CORE_BUILTIN
  {
    int  max_bt = 4096;
    char *bt = Yo_Malloc(max_bt);
    char *bt_p = bt;
    int i;

    i = snprintf(bt_p,max_bt,__yoTa("--backtrace--",0));
    bt_p+=i;
    max_bt-=i;
    memset(bt_p,0,max_bt--);

    for ( i = 0; i < frames; ++i )
      {
      #if defined __windoze || defined __CYGWIN__
        int dif = 0;
        char c = '+';
        int l = snprintf(bt_p,max_bt,__yoTa("\n %-2d=> %s %c%x (%p at %s)",0),
           i,
           "",
           c,
           dif>0?dif:-dif,
           cbk[i],
           Yo__basename("basename"));

        if ( l > 0 )
          {
            max_bt -= l;
            bt_p += l;
          }

      #else
        Dl_info dlinfo = {0};
        if ( dladdr(cbk[i], &dlinfo) )
          {
            int dif = (char*)cbk[i]-(char*)dlinfo.dli_saddr;
            char c = dif > 0?'+':'-';
            int l = snprintf(bt_p,max_bt,__yoTa("\n %-2d=> %s %c%x (%p at %s)",0),
               i,
               dlinfo.dli_sname,
               c,
               dif>0?dif:-dif,
               cbk[i],
               Yo__basename((char*)dlinfo.dli_fname));
            if ( l > 0 )
              {
                max_bt -= l;
                bt_p += l;
              }
          }
      #endif
      }

    return bt;
  }
#endif
  ;

char *Yo_Btrace(void)
#ifdef _YO_CORE_BUILTIN
  {
    void *cbk[128] = {0};
    int frames = backtrace(cbk,127);
    return Yo_Btrace_Format(frames,cbk);
  }
#endif
  ;

#if defined __windoze  && defined _YO_CORE_BUILTIN

int backtrace( void **cbk, int count )
  {
    return 0;
  }

#elif defined __GNUC__ && defined _YO_CORE_BUILTIN \
    && !(defined __APPLE__ || defined __linux__)

typedef struct _YO_BACKTRACE
  {
    void **cbk;
    int count;
  } YO_BACKTRACE;

_Unwind_Reason_Code backtrace_Helper(struct _Unwind_Context* ctx, YO_BACKTRACE *bt)
  {
    if ( bt->count )
      {
        void *eip = (void*)_Unwind_GetIP(ctx);
        if ( eip )
          {
            *bt->cbk++ = eip;
            --bt->count;
            return _URC_NO_REASON;
          }
      }
    return _URC_NORMAL_STOP;
  }

int backtrace( void **cbk, int count )
  {
    YO_BACKTRACE T = { cbk, count };
    _Unwind_Backtrace((_Unwind_Trace_Fn)&backtrace_Helper, &T);
    return count-T.count;
  }

#endif /* __GNUC__  && _YO_CORE_BUILTIN */

char *Yo_Error_Format_Btrace(void)
#ifdef _YO_CORE_BUILTIN
  {
    YO_ERROR_INFO *info = Error_Info();
    if ( info && info->bt_count )
      {
        return Yo_Btrace_Format(info->bt_count,info->bt_cbk);
      }
    return __yoTa("--backtrace--\n   unavailable",0);
  }
#endif
  ;

__No_Return void Yo_Btrace_N_Abort(char *prefix, char *msg, char *filename, int lineno)
#ifdef _YO_CORE_BUILTIN
  {
    char *at = filename?Yo_Format_Npl(__yoTa(" [%s(%d)]",0),Yo__basename(filename),lineno):"";
    char *pfx = prefix?Yo_Format_Npl(__yoTa("%s: ",0),prefix):"";
    StdErr_Print_Nl(Yo_Btrace());
    Yo_Abort(Yo_Format_Npl(__yoTa("%s%s%s",0),pfx,msg,at));
  }
#endif
  ;

#ifdef _YO_CORE_BUILTIN
__No_Return void _Yo_Fatal(int err,void *ctx,char *filename,int lineno)
  {
    switch (err)
      {
        case YO_ERROR_OUT_OF_MEMORY:
          Yo_Abort(__yoTa("out of memory",0));
        case YO_ERROR_REQUIRE_FAILED:
          Yo_Btrace_N_Abort(__yoTa("require",0),ctx,filename,lineno);
        case YO_FATAL_ERROR:
          Yo_Btrace_N_Abort(__yoTa("fatal",0),ctx,filename,lineno);
        case YO_ERROR_DYNCO_CORRUPTED:
          Yo_Btrace_N_Abort(__yoTa("fatal",0),
            Yo_Format_Npl(__yoTa("corrupted dynco (%p)",0),ctx),filename,lineno);
        default:
          {
            char err_pfx[60];
            sprintf(err_pfx,__yoTa("unexpected(%08x)",0),err);
            Yo_Btrace_N_Abort(err_pfx,ctx,filename,lineno);
          }
      }
  }
#endif
  ;

__No_Return void Error_Abort()
#ifdef _YO_CORE_BUILTIN
  {
    Yo_Btrace_N_Abort(
      Yo_Format_Npl(__yoTa("\ncaught(0x%08x)",0),Error_Code()),
      Error_Message(),Error_File(),Error_Line());
  }
#endif
  ;

char *Yo_Error_Format()
#ifdef _YO_CORE_BUILTIN
  {
    int code = Error_Code();
    char *msg = Error_Message();

    if ( YO_ERROR_IS_USER_ERROR(code) )
      return Yo_Format(__yoTa("error(%d): %s",0),code,msg);
    else
      return Yo_Format(__yoTa("error(%08x): %s",0),code,msg);
  }
#endif
  ;

__No_Return void Error_Exit(char *pfx)
#ifdef _YO_CORE_BUILTIN
  {
    int code = Error_Code();
    char *msg = Error_Message();

  #ifndef _BACKTRACE
    if ( (code & YO_TRACED_ERROR_GROUP) || !Error_Info()->msg )
  #endif
      StdErr_Print_Nl(Yo_Error_Format_Btrace());

    if ( code == YO_ERROR_USER )
      StdErr_Print_Nl(Yo_Format(__yoTa("\n%s: %s",0),(pfx?pfx:__yoTa("error",0)),msg));
    else if ( YO_ERROR_IS_USER_ERROR(code) )
      StdErr_Print_Nl(Yo_Format(__yoTa("\n%s(%d): %s",0),(pfx?pfx:__yoTa("error",0)),code,msg));
    else
      StdErr_Print_Nl(Yo_Format(__yoTa("\n%s(%08x): %s",0),(pfx?pfx:__yoTa("error",0)),code,msg));
    if ( code & YO_FATAL_ERROR_GROUP )
      abort();
    Yo_Unwind_Scope(0,-1,0);
    exit(code);
  }
#endif
  ;

char *Yo_Format_System_Error()
#ifdef _YO_CORE_BUILTIN
  {
  #ifdef __windoze
    int err = GetLastError();
    char *msg = Yo_Malloc(1024);
    FormatMessageA(FORMAT_MESSAGE_FROM_SYSTEM | FORMAT_MESSAGE_IGNORE_INSERTS |
                  FORMAT_MESSAGE_MAX_WIDTH_MASK, NULL, err,
                  MAKELANGID(LANG_NEUTRAL, SUBLANG_DEFAULT),
                  (LPSTR)msg, 1024,0);
    return msg;  
  #else
    return strerror(errno);
  #endif
  }
#endif
  ;

#define __Pool(Ptr)                     Yo_Pool_Ptr(Ptr,0)
#define __Pool_Ptr(Ptr,Free)            Yo_Pool_Ptr(Ptr,Free)
#define __Pool_RefPtr(Ptr)              Yo_Pool_Ptr(Ptr,Yo_Unrefe)
#define __Release(Pooled)               Yo_Release(Pooled)
#define __Retain(Pooled)                Yo_Retain(Pooled)
#define __Purge(TholdPtr)               Yo_Pool_Purge(TholdPtr,YO_DEFAULT_PURGE_CAP)
#define __Refe(Ptr)                     Yo_Refe(Ptr)
#define __Unrefe(Ptr)                   Yo_Unrefe(Ptr)
#define __Raise(Err,Msg)                Yo_Raise(Err,Msg,__Yo_FILE__,__LINE__)
#define __Raise_User_Error(Msg_)        Yo_Raise(YO_ERROR_USER,Msg_,__Yo_FILE__,__LINE__)
#define __Raise_Format(Err,Fmt)         Yo_Raise(Err,(Yo_Format Fmt),__Yo_FILE__,__LINE__)
#define __Raise_Occured()               Yo_Raise_Occured()
#define __Raise_System_Error()          Yo_Raise(YO_ERROR_SYSTEM,Yo_Format_System_Error(),__Yo_FILE__,__LINE__)
#define __Fatal(Ctx)                    Yo_Fatal(YO_FATAL_ERROR,Ctx,__Yo_FILE__,__LINE__)
#define __Fatal_Format(x)               Yo_Fatal(YO_FATAL_ERROR,(Yo_Format_Npl x),__Yo_FILE__,__LINE__)
#define __Format                        Yo_Format
#define __Format_Npl                    Yo_Format_Npl
#define __Format_Error()                Yo_Error_Format()
#define __Format_Btrace()               Yo_Btrace()
#define __Format_Error_Btrace()         Yo_Error_Format_Btrace()
#define __Format_System_Error()         Yo_Format_System_Error()

#define __Malloc(Size)                  Yo_Malloc(Size)
#define __Malloc_Npl(Size)              Yo_Malloc_Npl(Size)
#define __Zero_Malloc(Size)             __Pool(Yo_Zero_Malloc_Npl(Size))
#define __Zero_Malloc_Npl(Size)         Yo_Zero_Malloc_Npl(Size)
#define __Memcopy(Ptr,Size)             Yo_Memcopy(Ptr,Size)
#define __Memcopy_Npl(Ptr,Size)         Yo_Memcopy_Npl(Ptr,Size)
#define __Realloc(Ptr,Size)             Yo_Realloc(Ptr,Size)
#define __Realloc_Npl(Ptr,Size)         Yo_Realloc_Npl(Ptr,Size)
#define __Resize(Ptr,Size,Gran)         Yo_Resize(Ptr,Size,Gran)
#define __Resize_Npl(Ptr,Size,Gran)     Yo_Resize_Npl(Ptr,Size,Gran)

#define __Object(Size,Funcs)            Yo_Object(Size,Funcs)
#define __Object_Dtor(Size,Dtor)        Yo_Object_Dtor(Size,Dtor)
#define __Object_Extend(Obj,Name,Func)  Yo_Object_Extend(Obj,Name,Func)
#define __Destruct(Ptr)                 Yo_Object_Destruct(Ptr)
#define __Clone(Size,Ptr)               Yo_Object_Clone(Size,Ptr)

#define __Free(P)                       free(P)
#define __Ref_Set(PP,R)                 if (0); else { __Unrefe(PP); PP = __Refe(R); }   
#define __Ptr_Set(PP,R)                 if (0); else { free(PP); PP = R; }   

#ifdef _STRONGPOOL

#define __Auto_Release \
  switch ( 0 ) while ( 1 ) \
    if ( 1 ) \
      { \
        Yo_Unwind_Scope(0,0,0); \
        break; \
      case 0: Yo_Push_Scope(); \
        goto YO_LOCAL_ID(ar_Body);\
      } \
    else \
      YO_LOCAL_ID(ar_Body):

#define __Auto_Ptr(Ptr) \
  switch ( 0 ) while ( 1 ) \
    if ( 1 ) \
      { \
        Yo_Unwind_Scope(Ptr,0,0); \
        break; \
      case 0: Yo_Push_Scope(); \
        goto YO_LOCAL_ID(ap_Body);\
      } \
    else \
      YO_LOCAL_ID(ap_Body):

#else

#define __Auto_Release \
  switch ( 0 ) while ( 1 ) \
    if ( 1 ) \
      { \
        int YO_LOCAL_ID(ar_Mark); \
        Yo_Unwind_Scope(0,0,&YO_LOCAL_ID(ar_Mark)); \
        break; \
      case 0: Yo_Pool_Ptr(&YO_LOCAL_ID(ar_Mark),Yo_Pool_Marker_Tag); \
        goto YO_LOCAL_ID(ar_Body);\
      } \
    else \
      YO_LOCAL_ID(ar_Body):

#define __Auto_Ptr(Ptr) \
  switch ( 0 ) while ( 1 ) \
    if ( 1 ) \
      { \
        int YO_LOCAL_ID(ap_Mark); \
        Yo_Unwind_Scope(Ptr,0,&YO_LOCAL_ID(ap_Mark)); \
        break; \
      case 0: Yo_Pool_Ptr(&YO_LOCAL_ID(ap_Mark),Yo_Pool_Marker_Tag); \
        goto YO_LOCAL_ID(ap_Body);\
      } \
    else \
      YO_LOCAL_ID(ap_Body):

#endif /* _STRONGPOOL */

#define __Gogo \
  if ( 1 ) goto YO_LOCAL_ID(__gogo); \
  else YO_LOCAL_ID(__gogo):

#define __Elm_Resize_Npl(MemPptr,L,Width,CpsPtr) Yo_Elm_Resize_Npl((void**)MemPptr,L,Width,CpsPtr)
void Yo_Elm_Resize_Npl(void **inout, int L, int type_width, int *capacity_ptr)
#ifdef _YO_CORE_BUILTIN
  {
    int requires = 0;
    int capacity = capacity_ptr?*capacity_ptr:0;

    if ( L )
      {
        requires = (L+1)*type_width;

        if ( *inout )
          {
            if ( !capacity )
              capacity = malloc_size(*inout);
            if ( capacity < requires )
              {
                capacity = Min_Pow2(requires);
                *inout = Yo_Realloc_Npl(*inout,capacity);
              }
          }
        else
          {
            if ( capacity < requires )
              capacity = Min_Pow2(requires);

            *inout = Yo_Malloc_Npl(capacity);
          }
      }

    if ( capacity_ptr ) *capacity_ptr = capacity;
  }
#endif
  ;

#define __Elm_Insert_Npl(MemPptr,Pos,Count,S,L,Width,CpsPtr) Yo_Elm_Insert_Npl((void**)MemPptr,Pos,Count,S,L,Width,CpsPtr)
int Yo_Elm_Insert_Npl(void **inout, int pos, int count, void *S, int L, int type_width, int *capacity_ptr)
#ifdef _YO_CORE_BUILTIN
  {
    STRICT_REQUIRE(pos <= count);

    if ( L < 0 ) /* inserting Z-string */
      switch ( type_width )
        {
          case sizeof(wchar_t): L = wcslen(S); break;
          case 1: L = strlen(S); break;
          default: PANICA(__yoTa("invalid size of string element",0));
        }

    if ( L )
      {
        Yo_Elm_Resize_Npl(inout,count+L,type_width,capacity_ptr);

        if ( pos < count )
          memmove((byte_t*)*inout+(pos+L)*type_width,(byte_t*)*inout+pos*type_width,(count-pos)*type_width);
        memcpy((byte_t*)*inout+pos*type_width, S, L*type_width);
        count += L;
        memset((byte_t*)*inout+count*type_width, 0, type_width);
      }

    return L;
  }
#endif
  ;

#define __Elm_Insert(MemPptr,Pos,Count,S,L,Width,CpsPtr) Yo_Elm_Insert((void**)MemPptr,Pos,Count,S,L,Width,CpsPtr)
int Yo_Elm_Insert(void **inout, int pos, int count, void *S, int L, int type_width, int *capacity_ptr)
#ifdef _YO_CORE_BUILTIN
  {
    void *old = *inout;
    int r = Yo_Elm_Insert_Npl(inout,pos,count,S,L,type_width,capacity_ptr);
    if ( *inout != old )
      {
        if ( old )
          Yo_Refresh_Ptr(old,*inout,0);
        else
          Yo_Pool_Ptr(*inout,0);
      }
    return r;
  }
#endif
  ;

#define Yo_Elm_Append(Mem,Count,S,L,Width,CpsPtr) Yo_Elm_Insert(Mem,Count,Count,S,L,Width,CpsPtr)
#define Yo_Elm_Append_Npl(Mem,Count,S,L,Width,CpsPtr) Yo_Elm_Insert_Npl(Mem,Count,Count,S,L,Width,CpsPtr)
#define __Vector_Append(Mem,Count,Capacity,S,L) (void)(*Count += Yo_Elm_Append((void**)Mem,*Count,S,L,1,Capacity))
#define __Elm_Append(Mem,Count,S,L,Width,CpsPtr) Yo_Elm_Append((void**)Mem,Count,S,L,Width,CpsPtr)
#define __Elm_Append_Npl(Mem,Count,S,L,Width,CpsPtr) Yo_Elm_Append_Npl((void**)Mem,Count,S,L,Width,CpsPtr)

#endif /* C_once_6973F3BA_26FA_434D_9ED9_FF5389CE421C */
