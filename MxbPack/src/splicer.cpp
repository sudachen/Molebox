
/*

  Copyright (c) 2008-2009, Alexey Sudachen, alexey@sudachen.name

*/

#include <classes.h>
#include "import.h"
#include "splicer.h"

enum { MAX_THU_HEAP_COUNT = 3 };
enum { THU_BLOCK_SIZE = 32 };

static HANDLE thu_heap[MAX_THU_HEAP_COUNT] = {0};
static teggo::randizer_t thu_rnd = 0;

void InitHeapAndSplicer()
{
    for ( int i = 0; i < MAX_THU_HEAP_COUNT; ++i )
    {
        thu_heap[i] = HeapCreate(HEAP_CREATE_ENABLE_EXECUTE,4*1024,0);
        thu_rnd ^= (teggo::randizer_t)thu_heap[i];
    }
}

/*
int StealFiveBytes(byte_t *code, byte_t *f)
  {
    int orign_len = 0;

    while ( orign_len < 5 )
      {
        teggo::x86copInfo ifn = {0};
        int l = teggo::x86Length(f+orign_len,&ifn);
        if ( l == 0 ) return 0; // oops, is there unknown instruction ?

        //if ( ifn.is_rjmp && !orign_len)
        //  f += ifn.rOffs_rel;
        //else
          {
            memcpy(code+orign_len,f+orign_len,l);

            if ( ifn.is_rjmp )
              {
                if ( ifn.rOffs_size == 4 )
                  {
                    byte_t *offs = (code+orign_len+ifn.rOffs_offset);
                    *((u32_t*)offs) += (f-code);
                  }
                else
                  return false;
              }
            else if ( ifn.is_rcall )
              {
                if ( ifn.rOffs_size == 4 )
                  {
                    byte_t *offs = (code+orign_len+ifn.rOffs_offset);
                    *((u32_t*)offs) += (f-code);
                  }
                else
                  return false;
              }
            else if ( ifn.is_rjc )
              return false;

            orign_len += l;
          }
      }

    return orign_len;
  }

void *Splice5(void *f)
  {
    byte_t *fx = (byte_t*)HeapAlloc(thu_heap[teggo::modulate_random(&thu_rnd,MAX_THU_HEAP_COUNT)],0,THU_BLOCK_SIZE);
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
*/

extern "C" void* __stdcall AutoIatSpliceProcAddress(char* modname, void** modhandle, unsigned char* xored, unsigned no)
{
    //__asm int 3
    ;

    char AutoIatSpliceProcAddress_b[128] = {0};
    void* f = 0;
    unsigned char* bp = (unsigned char*)AutoIatSpliceProcAddress_b;
    unsigned long sid = no;
    no = 0;

    sid += 0xabecaffe; // 0xfabecaffe; ???
    do
    {
        sid = 1664525*sid + 1013904223;
        *bp = (sid >> ( no++ % 8 )) ^ *xored++;
    }
    while ( *bp++ && no < 127 );

    if ( !*modhandle )
        //if ( !(*modhandle = get_predefined_module(modname)) )
        if ( !(*modhandle = GetModuleHandleA(modname)) )
            if ( !(*modhandle = LoadLibraryA(modname)) )
                __asm int 3
                ;

    if ( !(f = GetProcAddressIndirect(*modhandle,AutoIatSpliceProcAddress_b,0)) )
        __asm int 3
        ;

    /*if ( *modhandle == _NTDLL )
      {
        f = Splice5(f);
      }*/

    memset(AutoIatSpliceProcAddress_b,0xcc,sizeof(AutoIatSpliceProcAddress_b));
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
