
/*

  Copyright (c) 2005-2009, Alexey Sudachen, alexey@sudachen.name

*/


#ifndef ___695d6f83_b274_4960_beaf_d3d70cfe00e6___
#define ___695d6f83_b274_4960_beaf_d3d70cfe00e6___

#include "_specific.h"
#include "array.h"
#include "hinstance.h"
#include "x86code.h"
#include "threads.h"

namespace teggo
{

    struct ROUTINE_SPLICE_RECORD
    {
        byte_t code[28];
        HANDLE heap;
        void Dispose() { HeapFree(heap,0,this); }
    };

    struct ROUTINE_SPLICE_RECORD_CRC
    {
        byte_t code[16+0x30];
        u32_t  xrc;
        HANDLE heap;
        void Dispose() { HeapFree(heap,0,this); }
    };

    _TEGGO_EXPORTABLE ROUTINE_SPLICE_RECORD* RoutineSplice( void* fn, HANDLE heap = 0, void* fnX = 0);
    _TEGGO_EXPORTABLE ROUTINE_SPLICE_RECORD* RoutineSpliceIndirect( ROUTINE_SPLICE_RECORD* rec, void* fn, void* fnX = 0);
    _TEGGO_EXPORTABLE ROUTINE_SPLICE_RECORD_CRC* RoutineSpliceIndirectCRC( ROUTINE_SPLICE_RECORD_CRC* rec, void* fn, void* fnX = 0);

    struct ROUTINE_HOOK_RECORD
    {
        int     orign_len;
        HANDLE  heap;
        void*   fn;
        void*   handler;
        void*   cookie;
        byte_t code[/*regs*/24+/*regs save/stor*/48+/*call*/16+/*orign*/32+/*jump*/8];
    };

    _TEGGO_EXPORTABLE ROUTINE_HOOK_RECORD* RoutineHook( void* fn, void* handler, void* cookie, HANDLE heap = 0 );
    _TEGGO_EXPORTABLE void  RoutineUnhook( ROUTINE_HOOK_RECORD* hook );

    struct _HookManager
    {

        struct P_HOOK_RECORD
        {
            P_HOOK_RECORD(ROUTINE_HOOK_RECORD* _p) : p(_p) {}
            ROUTINE_HOOK_RECORD* p;
            ROUTINE_HOOK_RECORD* operator->() const { return p; }
            ROUTINE_HOOK_RECORD* operator+() { return p; }
            friend bool operator < ( void const* fn, P_HOOK_RECORD const& h ) { return operator <(h,fn);}
            friend bool operator < ( P_HOOK_RECORD const& h, void const* fn ) { return h->fn < fn; }
            friend bool operator < ( P_HOOK_RECORD const& h0, P_HOOK_RECORD const& h1 ) { return h0->fn < h1->fn; }
        };

        _TEGGO_EXPORTABLE _HookManager();
        _TEGGO_EXPORTABLE ~_HookManager();

        //_TEGGO_EXPORTABLE void HookFn(void *fn, void *cdecl_handler,void *cookie);
        _TEGGO_EXPORTABLE void UnhookFn(void* fn);
        _TEGGO_EXPORTABLE void UnhookAll();

        HANDLE Heap() { return heap_; }

        HANDLE heap_;
        Tlocker locker_;
        BufferT<P_HOOK_RECORD> hooks_;
    };

    struct HookManagerInstance
    {
        HookManagerInstance() {}
        _TEGGO_EXPORTABLE static _HookManager* __Instance();
        _HookManager* operator ->() const { return __Instance(); }
    };

    const HookManagerInstance HookManager = HookManagerInstance();

    #if defined _TEGGOINLINE || defined _TEGGO_HOOKMGR_HERE

    #if defined _TEGGOINLINE
# define _TEGGO_HOOKMGR_FAKE_INLINE CXX_FAKE_INLINE
    #else
# define _TEGGO_HOOKMGR_FAKE_INLINE _TEGGO_EXPORTABLE
    #endif

    _TEGGO_HOOKMGR_FAKE_INLINE
    ROUTINE_SPLICE_RECORD* RoutineSplice( void* fn, HANDLE heap, void* fnX )
    {
        if ( !heap ) heap = HookManager->Heap();
        ROUTINE_SPLICE_RECORD* rec = (ROUTINE_SPLICE_RECORD*)HeapAlloc(heap,0,sizeof(ROUTINE_SPLICE_RECORD));
        rec->heap = heap;
        return RoutineSpliceIndirect( rec, fn, fnX );
    }


    _TEGGO_HOOKMGR_FAKE_INLINE
    int _StealFiveBytes(byte_t*& fnX, int& orign_len, int& redirected, byte_t* code)
    {
        orign_len = 0;
        redirected = 0;

        while ( orign_len < 5 )
        {
            x86copInfo ifn = {0};
            int l = x86Length((byte_t*)fnX+orign_len,&ifn);
            if ( l == 0 ) return 0;

            memcpy(code+orign_len,(byte_t*)fnX+orign_len,l);

            if ( ifn.is_rjmp )
                /*if ( !orign_len )
                  {
                    fnX = (byte_t*)fnX + (ifn.rOffs_rel+l);
                    redirected += l;
                    continue;
                  }
                else*/
            {
                if ( ifn.rOffs_size == 4 )
                {
                    byte_t* offs = (code+orign_len+ifn.rOffs_offset);
                    *((u32_t*)offs) += (fnX-code);
                }
                else if ( ifn.rOffs_size != 4 )
                    break;
            }
            else if ( ifn.is_rcall )
            {
                if ( ifn.rOffs_size == 4 )
                {
                    byte_t* offs = (code+orign_len+ifn.rOffs_offset);
                    *((u32_t*)offs) += (fnX-code);
                }
                else if ( ifn.rOffs_size != 4 )
                    break;
            }
            else if ( ifn.is_rjc )
                break;

            orign_len += l;
        }
        return orign_len >= 5;
    }

    _TEGGO_HOOKMGR_FAKE_INLINE
    ROUTINE_SPLICE_RECORD_CRC* RoutineSpliceIndirectCRC( ROUTINE_SPLICE_RECORD_CRC* rec, void* fn, void* fnX )
    {
        int orign_len, redirected;
        memcpy(rec->code,&RoutineSpliceIndirect,sizeof(rec->code));

        if ( !fnX ) fnX = fn;
        byte_t* fn_p = (byte_t*)fnX;

        if (!_StealFiveBytes(fn_p,orign_len,redirected,rec->code) )
            return 0;

        u32_t fn_jmp_address = (u32_t)(fn_p+orign_len);
        while ( (orign_len&3) != 3 ) *(rec->code+(orign_len++)) = 0x90;
        memcpy(rec->code+orign_len,
               "\x68\0\0\0\0"          // +0  push $xoredjmp
               "\x16"                  // +5  push ss
               "\x87\x44\x24\x04"      // +6  xchg eax,[esp+4]
               "\x57"                  // +a  push edi
               "\x66\x8e\x54\x24\x04"  // +b  mov  ss,word ptr [esp+4]
               "\x6a\x10"              // +10 push $len
               "\xbf\0\0\0\0"          // +12 mov  edi,$recaddr
               "\x87\x4c\x24\x00"      // +17 xchg ecx,[esp+0]
               "\x9c"                  // pushf
               "\x31\xc0"              // +1b xor  eax,eax
               "\x33\x04\x8f"          // +1d xor  eax,[edi+ecx*4]
               "\x49"                  // +20 dec  ecx
               "\x75\xfa"              // +21 jne  -4
               "\x9d"                  // popf
               "\x59"                  // +23 pop  ecx
               "\x5f"                  // +24 pop  edi
               "\x87\x44\x24\x04"      // +25 xchg eax,[esp+4]
               "\x17"                  // +29 pop  ss
               "\xc3"                  // +2a ret
               ,0x2b+2);
        *(void**)(rec->code+(orign_len+0x13)) = (rec->code-4);
        u32_t xrc = 0;
        for ( byte_t* k = rec->code; k != (rec->code + 0x40); k+=4 )
            xrc ^= *(u32_t*)k;
        *(u32_t*)(rec->code+(orign_len+1)) = xrc ^ fn_jmp_address;
        rec->xrc = xrc;
        return rec;
    }

    _TEGGO_HOOKMGR_FAKE_INLINE
    ROUTINE_SPLICE_RECORD* RoutineSpliceIndirect( ROUTINE_SPLICE_RECORD* rec, void* fn, void* fnX )
    {
        if ( !fnX ) fnX = fn;
        int orign_len = 0;
        int redirected = 0;
        byte_t* fn_p = (byte_t*)fnX;

        memset(rec->code,0xcc,sizeof(rec->code));
        if ( !_StealFiveBytes(fn_p,orign_len,redirected,rec->code) )
            return 0;

        u32_t fn_jmp_address = (fn_p+orign_len) - (rec->code+(orign_len+5));
        rec->code[orign_len] = 0xe9;
        *(u32_t*)(rec->code+(orign_len+1)) = fn_jmp_address;
        return rec;
    }

    /*  _TEGGO_HOOKMGR_FAKE_INLINE
        ROUTINE_HOOK_RECORD *RoutineHook( void *fn, void *handler, void *cookie, HANDLE heap )
          {
            if ( !heap ) heap = HookManager->Heap();
            ROUTINE_HOOK_RECORD *hook = (ROUTINE_HOOK_RECORD*)HeapAlloc(heap,0,sizeof(ROUTINE_HOOK_RECORD));
            hook->fn = fn;
            hook->handler = handler;
            hook->cookie  = cookie;
            memset(hook->code,0xcc,sizeof(hook->code));

            memcpy(hook->code+24,
              "\xb8\0\0\0\0" // +0   mov eax, <hook->code>
              "\x89\x18"     // +5   mov [eax]   ,ebx
              "\x89\x68\x04" // +7   mov [eax+4] ,ebp
              "\x89\x48\x08" // +a   mov [eax+8] ,ecx
              "\x89\x78\x0c" // +d   mov [eax+c] ,edi
              "\x89\x70\x10" // +10  mov [eax+10],esi
              "\xff\x70\xfc" // +13  push [eax-4] ; cookie
              "\xff\x50\xf8" // +16  call [eax-8] ; handler
              "\x59"         // +19  pop ecx      ; cookie
              "\xb8\0\0\0\0" // +1a  mov eax, <hook->code>
              "\x8b\x70\x10" // +1f  mov esi,[eax+10]
              "\x8b\x78\x0c" // +22  mov edi,[eax+c]
              "\x8b\x48\x08" // +25  mov ecx,[eax+8]
              "\x8b\x68\x04" // +28  mov ebp,[eax+4]
              "\x8b\x18"     // +2b  mov ebx,[eax]
              // +2d
              ,0x2d);

            *(u32_t*)(hook->code+24+1) = (u32_t)hook->code;
            *(u32_t*)(hook->code+24+0x1b) = (u32_t)hook->code;
            int orign_base = 0x2d+24;
            int orign_len  = 0;
            int redirected = 0;
            byte_t *fn_p = (byte_t*)fn;
            if ( !_StealFiveBytes(fn_p,orign_len,redirected,hook->code+orign_base) )
              return 0;
            u32_t fn_jmp_address = (fn_p+orign_len) - (hook->code+(orign_base+orign_len+5));
            hook->code[orign_base+orign_len] = 0xe9;
            *(u32_t*)(hook->code+(orign_base+orign_len+1)) = fn_jmp_address;//((u32_t)fn+orign_len+redirected)-((u32_t)hook->code+orign_base+orign_len+5);
            u32_t foo;
            VirtualProtect(fn,1024,PAGE_EXECUTE_WRITECOPY,&foo);
            *(byte_t*)fn = 0xe9;
            *(u32_t*)((byte_t*)fn+1) = (u32_t)hook->code-(u32_t)((byte_t*)fn+5)+24;
            VirtualProtect(fn,1024,foo,&foo);
            hook->orign_len = orign_len;
            return hook;
          } */

    _TEGGO_HOOKMGR_FAKE_INLINE
    void  RoutineUnhook( ROUTINE_HOOK_RECORD* hook )
    {
        u32_t foo;
        VirtualProtect(hook->fn,1024,PAGE_EXECUTE_WRITECOPY,&foo);
        memcpy(hook->fn,hook->code+0x2d+24,hook->orign_len);
        VirtualProtect(hook->fn,1024,foo,&foo);
        HeapFree(hook->heap,0,hook);
    }

    _TEGGO_HOOKMGR_FAKE_INLINE
    _HookManager* HookManagerInstance::__Instance()
    {
        static TsafeHolder<_HookManager> hmag = {0};
        return hmag.Get();
    }

    /*
      _TEGGO_HOOKMGR_FAKE_INLINE
        void _HookManager::HookFn(void *fn, void *cdecl_handler,void *cookie)
          {
            __lockon__(locker_)
              {
                if ( hooks_.Bfind(fn) ) return;
                ROUTINE_HOOK_RECORD *r = RoutineHook(fn, cdecl_handler, cookie, heap_);
                hooks_.Binsert(P_HOOK_RECORD(r));
              }
          }
    */

    _TEGGO_HOOKMGR_FAKE_INLINE
    void _HookManager::UnhookFn(void* fn)
    {
        __lockon__(locker_)
        {
            if ( P_HOOK_RECORD* p = hooks_.Bfind(fn) )
            {
                RoutineUnhook(+*p);
                hooks_.Erase(p);
            }
        }
    }

    _TEGGO_HOOKMGR_FAKE_INLINE
    void _HookManager::UnhookAll()
    {
        __lockon__(locker_)
        {
            for ( P_HOOK_RECORD* p = hooks_.Begin(), *pE = hooks_.End(); p != pE; ++p )
                RoutineUnhook(+*p);
            hooks_.Clear();
        }
    }

    _TEGGO_HOOKMGR_FAKE_INLINE
    _HookManager::_HookManager()
    {
        this->heap_ = HeapCreate(
                          (Hinstance->IsWi9x()?/*HEAP_CREATE_ENABLE_EXECUTE*/0x00040000:0)|HEAP_GENERATE_EXCEPTIONS,
                          16*1024,
                          0);
    }

    _TEGGO_HOOKMGR_FAKE_INLINE
    _HookManager::~_HookManager()
    {
        this->UnhookAll();
    }

    #endif
}

#endif /*___695d6f83_b274_4960_beaf_d3d70cfe00e6___*/
