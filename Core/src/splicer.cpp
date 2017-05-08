
/*

  Copyright (c) 2008-2009, Alexey Sudachen, alexey@sudachen.name

*/

#include "_common.h"
#include "apif.h"
#include "splicer.h"
#include "logger.h"
#include "splicer.h"
#include "import.h"
#include "XorS.h"

enum { MAX_THU_HEAP_COUNT = 3 };
enum { THU_BLOCK_SIZE = 32 };

HANDLE g_heap = 0;
static HANDLE thu_heap[MAX_THU_HEAP_COUNT] = {0};
static teggo::randizer_t thu_rnd = 0;

void InitHeapAndSplicer(STB_INFO* info, void* base)
{
    for ( int i = 0; i < MAX_THU_HEAP_COUNT; ++i )
    {
        thu_heap[i] = HeapCreate(HEAP_CREATE_ENABLE_EXECUTE,4*1024,0);
        thu_rnd ^= (teggo::randizer_t)thu_heap[i];
    }
    g_heap = HeapCreate(HEAP_CREATE_ENABLE_EXECUTE,512*1024,0);
}

int StealFiveBytes(byte_t* code, byte_t* f)
{
    int orign_len = 0;

    while ( orign_len < 5 )
    {
        teggo::x86copInfo ifn = {0};
        int l = teggo::x86Length(f+orign_len,&ifn);
        if ( l == 0 ) return 0; // oops, is there unknown instruction ?

        if ( ifn.is_rjmp && ifn.rOffs_size == 1 && !orign_len)
            f += ifn.rOffs_rel;
        else
        {
            memcopy(code+orign_len,f+orign_len,l);

            if ( ifn.is_rjmp )
            {
                if ( ifn.rOffs_size == 4 )
                {
                    byte_t* offs = (code+orign_len+ifn.rOffs_offset);
                    *((u32_t*)offs) += (f-code);
                }
                else
                    return 0;
            }
            else if ( ifn.is_rcall )
            {
                if ( ifn.rOffs_size == 4 )
                {
                    byte_t* offs = (code+orign_len+ifn.rOffs_offset);
                    *((u32_t*)offs) += (f-code);
                }
                else
                    return 0;
            }
            else if ( ifn.is_rjc )
                return 0;

            orign_len += l;
        }
    }

    return orign_len;
}

void* Splice5(void* f)
{
    byte_t* fx = (byte_t*)HeapAlloc(thu_heap[teggo::modulate_random(&thu_rnd,MAX_THU_HEAP_COUNT)],0,THU_BLOCK_SIZE);
    for ( int i = 0; i < THU_BLOCK_SIZE/4; ++i )
        ((unsigned*)fx)[i] = teggo::unsigned_random(&thu_rnd);

    if ( int l = StealFiveBytes( fx, (byte_t*)f ) )
    {
        fx[l] = 0x68; // push
        *(void**)(fx+l+1) = (byte_t*)f+l;
        fx[l+5] = 0xc3; // ret
        f = fx;
    }
    else
        __asm int 3
        ;
    return f;
}

extern "C" void* __stdcall AutoIatSpliceProcAddress(char* modname, void** modhandle, unsigned char* xxored, unsigned xno)
{
    //__asm int 3
    ;

    unsigned char* xored = xxored;
    char AutoIatSpliceProcAddress_b[128] = {0};
    void* f = 0;
    unsigned char* bp = (unsigned char*)AutoIatSpliceProcAddress_b;
    unsigned long sid = xno;
    int no = 0;

    sid += 0xabecaffe; // 0xfabecaffe; ???
    do
    {
        sid = 1664525*sid + 1013904223;
        *bp = (sid >> ( no++ % 8 )) ^ *xored++;
    }
    while ( *bp++ && no < 127 );

    if ( !*modhandle )
        if ( !(*modhandle = get_predefined_module(modname)) )
            if ( !(*modhandle = LoadLibraryA(modname)) )
            {
                __asm mov ecx,__LINE__;
                __asm int 3;
                ;
            }

    if ( !(f = GetProcAddressIndirect(*modhandle,AutoIatSpliceProcAddress_b,0)) )
    {
        if ( !strcmp(AutoIatSpliceProcAddress_b,_XOr("GetModuleBaseNameA",19,420444935)) )
        {
            void* mdh = 0;
            return AutoIatSpliceProcAddress(_XOr("psapi.dll",10,426146622),&mdh,xxored,xno);
        }
        else if ( !strcmp(AutoIatSpliceProcAddress_b,_XOr("GetModuleFileNameW",19,427195182)) )
        {
            void* mdh = 0;
            return AutoIatSpliceProcAddress(_XOr("psapi.dll",10,424639303),&mdh,xxored,xno);
        }
        __asm mov ecx,__LINE__;
        __asm int 3;
    }
    if ( *modhandle == _NTDLL )
    {
        f = Splice5(f);
    }

    memsetS(AutoIatSpliceProcAddress_b,0xcc,sizeof(AutoIatSpliceProcAddress_b));
    return f;
}

void* AllocExecutableBlock(int sz)
{
    sz = cxx_max<unsigned>(sz+sizeof(HANDLE),THU_BLOCK_SIZE);
    HANDLE h = thu_heap[teggo::modulate_random(&thu_rnd,MAX_THU_HEAP_COUNT)];
    void* f = HeapAlloc(h,0,sz);
    for ( int i = 0; i < sz/4; ++i )
        ((unsigned*)f)[i] = teggo::unsigned_random(&thu_rnd);
    *(HANDLE*)f = h;
    return (char*)f + sizeof(HANDLE);
}

void FreeExecutableBlock(void* f)
{
    HANDLE* h = (HANDLE*)((char*)f - sizeof(HANDLE));
    HeapFree(*h,0,h);
}

enum { JX_THUNK_LEN = 44, NONSEH_JXTHUNKS = 4, THUNK_NONE = 0};
struct Type_Tunk_Data
{
    u32_t ofs_addr;
    u32_t ofs_xor;
    u32_t ofs_fn;
    u32_t ofs_av_xor;
    u32_t ofs_av_ret;
    u32_t len;
    char const* data;
};

static const Type_Tunk_Data thunk_data[] =
{
    // 0
    {
        THUNK_NONE, THUNK_NONE, THUNK_NONE, 1, 6, 10,
        //BEGIN XORED DATA,0//
        "\x37\xaf\xd7\x6b\x35"  // "\x68\x00\x00\x00\x00"  +0  push xxxx
        "\x56\xcd\xe6\xf3\x79"  // "\xcc\x00\x00\x00\x00"  +5  int3 xxxx
        //END XORED DATA,0//
    },
    // 1
    {
        THUNK_NONE, THUNK_NONE, THUNK_NONE, 1, 6, 10,
        //BEGIN XORED DATA,1//
        "\x04\xb6\x5b\x2d\x96"  // "\x68\x00\x00\x00\x00"  +0  push xxxx
        "\x07\x65\xb2\x59\x2c"  // "\xcc\x00\x00\x00\x00"  +5  int3 xxxx
        //END XORED DATA,1//
    },
    // 2
    // ---------------------------
    {
        THUNK_NONE, THUNK_NONE, THUNK_NONE, 1, 6, 11,
        //BEGIN XORED DATA,2//
        "\x11\xbc\xde\xef\xf7"  // "\x68\x00\x00\x00\x00"  +0  push xxxx
        "\x93\xfd\x7e\xbf\xdf"  // "\x68\x00\x00\x00\x00"  +5  push xxxx
        "\xac"                  // "\xc3"                  +a  ret
        //END XORED DATA,2//
    },

    // 3
    {
        1, 0x10, THUNK_NONE, THUNK_NONE, 0x15, 0x1a,
        //BEGIN XORED DATA,3//
        "\xee\xc3\x61\xb0\x58"  // "\x68\x00\x00\x00\x00"  +0  push xxxx
        "\xab\x92\x6f"          // "\x87\x04\x24"          +5  xchg %eax,(%esp)
        "\x16\x92"              // "\x33\x00"              +8  xor  %(eax),%eax
        "\x4e\x60\x96"          // "\x87\x04\x24"          +a  xchg %eax,(%esp)
        "\x58\xd8\x52\xbb\x5d\x2e\x97"// "\x81\x34\x24\x00\x00\x00\x00" +d  xor  $xxxx,(%esp)
        "\xa3\xe5\xf2\x79\x3c"  // "\x68\x00\x00\x00\x00"  +14 push xxxx
        "\xdd"                  // "\xc3"                  +19 ret
        //END XORED DATA,3//
    },
    // 4
    // ---------------------------
    {
        THUNK_NONE, THUNK_NONE, 1, THUNK_NONE, THUNK_NONE, 6,
        //BEGIN XORED DATA,4//
        "\xfb\xc9\xe4\x72\xb9"  // "\x68\x00\x00\x00\x00"  +0  push xxxx
        "\x9f"                  // "\xc3"                  +5  ret
        //END XORED DATA,4//
    },

    // 5
    {
        THUNK_NONE, THUNK_NONE, 1, THUNK_NONE, THUNK_NONE, 12,
        //BEGIN XORED DATA,5//
        "\xc8\xd0\x68\x34\x1a"  // "\x68\x00\x00\x00\x00"  +0  push xxxx
        "\x0e\x02\xe7"          // "\x83\xc4\x04"          +5  add $4,%esp
        "\x0e\x9c\x58\x42"      // "\xff\x64\x24\xfc"      +8  jmp (%esp-4)
        //END XORED DATA,5//
    },

    // 6
    {
        THUNK_NONE, THUNK_NONE, 1, THUNK_NONE, THUNK_NONE, 13,
        //BEGIN XORED DATA,6//
        "\xc5\xd6\xeb\xf5\x7a"  // "\x68\x00\x00\x00\x00"  +0  push xxxx
        "\x2d\xce\x3f"          // "\x90\x90\x90"              nop
        "\x02"                  // "\x55"                  +8  push %ebp
        "\x20\x39"              // "\x8b\xec"              +9  mov %esp,%ebp
        "\x23"                  // "\xc9"                  +b  leave
        "\xb6"                  // "\xc3"                  +c  ret
        //END XORED DATA,6//
    },

    // 7
    {
        1, 0x10, THUNK_NONE, THUNK_NONE, THUNK_NONE, 0x15,
        //BEGIN XORED DATA,7//
        "\xd2\xdd\x6e\xb7\xdb"  // "\x68\x00\x00\x00\x00"  +0  push xxxx
        "\x6a\xf2\x5f"          // "\x87\x04\x24"          +5  xchg %eax,(%esp)
        "\x8e\x5e"              // "\x33\x00"              +8  xor  %(eax),%eax
        "\xa8\x13\x2f"          // "\x87\x04\x24"          +a  xchg %eax,(%esp)
        "\x84\xb6\x65\x20\x90\x48\xa4"// "\x81\x34\x24\x00\x00\x00\x00" +d  xor  $xxxx,(%esp)
        "\x11"                  // "\xc3"                  +14 ret
        //END XORED DATA,7//
    },
};

void _UnpackXoredData( byte_t* to, char const* from, u32_t len, u32_t init_val )
{
    u32_t foo = teggo::unsigned_random(&init_val);
    __asm mov edi,to
    __asm mov esi,from
    __asm mov ecx,len
    __asm mov edx,init_val
l:
    __asm mov al,dl
    __asm xor al,[esi]
    __asm mov [edi],al
    __asm inc esi
    __asm inc edi
    __asm ror edx,1
    __asm loop l
}

static u32_t thunk_rnd = 0;

void* WrapWithJxThunk(void* fn)
{
    byte_t* thunk = (byte_t*)AllocExecutableBlock(JX_THUNK_LEN);
    u32_t r = GetTickCount();
    u32_t av_r = 0;//teggo::unsigned_random(&r) % (sizeof(Thunk_AV_XOR)/sizeof(Thunk_AV_XOR[0]));
    u32_t thunk_i = NONSEH_JXTHUNKS;
    u32_t thunk_I = sizeof(thunk_data)/sizeof(thunk_data[0]);

    if ( thunk_I - thunk_i > 1 )
    {
        thunk_I = (teggo::unsigned_random(&thunk_rnd))%(thunk_I-thunk_i);
        thunk_i += thunk_I;
    }

    for ( int i = thunk_data[thunk_i].len/4; i < JX_THUNK_LEN; ++i )
        thunk[i] = teggo::unsigned_random(&r);
    _UnpackXoredData(thunk,thunk_data[thunk_i].data,thunk_data[thunk_i].len,thunk_i);

    if ( thunk_data[thunk_i].ofs_addr )
        *(u32_t*)(thunk+thunk_data[thunk_i].ofs_addr) = (u32_t)thunk;
    if ( thunk_data[thunk_i].ofs_fn )
        *(u32_t*)(thunk+thunk_data[thunk_i].ofs_fn) = (u32_t)fn;
    //if ( thunk_data[thunk_i].ofs_av_xor )
    //  *(u32_t*)(thunk+thunk_data[thunk_i].ofs_av_xor) = Thunk_AV_XOR[av_r]^(u32_t)fn;
    //if ( thunk_data[thunk_i].ofs_av_ret )
    //  *(u32_t*)(thunk+thunk_data[thunk_i].ofs_av_ret) = av_r + Thunk_AV_Base;
    if ( thunk_data[thunk_i].ofs_xor )
    {
        u32_t C = *(u32_t*)thunk ^ (u32_t)thunk;
        C ^= (u32_t)fn;
        //if (thunk_data[thunk_i].ofs_av_ret)
        //  C ^= Thunk_AV_XOR[av_r];
        *(u32_t*)(thunk+thunk_data[thunk_i].ofs_xor) = C;
    }

    return thunk;
}
