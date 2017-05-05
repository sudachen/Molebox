
/*

  Copyright (c) 2008-2009, Alexey Sudachen, alexey@sudachen.name

*/

#include "myafx.h"
#include "stbinfo.h"
#include "teggovbox.h"

extern "C" unsigned char core0S[];
extern "C" unsigned core0S_size;

typedef TEGGOVBOX_FUNCTIONS_TABLE *(__stdcall *T_Install)(unsigned flags, void *ntdll, void *kern32, void *base);
TEGGOVBOX_FUNCTIONS_TABLE *g_vbox_funcs = 0;

void Relocate(CORE0_HEADER *eh)
  {
    unsigned char *base = (unsigned char*)eh;
    unsigned offs = (unsigned)eh - eh->image_base;
    const unsigned char* reloc  = (unsigned char*)eh+eh->relocs;
    const unsigned char* relocE = reloc+eh->relocs_size;
    while ( reloc != relocE )
      {
        int rva = *(int*)reloc; reloc += 4;
        unsigned bsize = *(unsigned*)reloc-8; reloc += 4;
        unsigned char const *e = reloc+bsize;
        while ( reloc != e )
          {
            switch ( *(unsigned short*)reloc >> 12 )
              {
                default:
                  __asm int 3
                  ;
                case 3:
                  {
                    int fixup = *(unsigned short*)reloc & 0x0fff;
                    *(unsigned*)(base+rva+fixup) += offs;
                  }
                case 0:
                  ;
              }
            reloc+=2;
          }
      }
  }

void Import(CORE0_HEADER *eh)
  {
    unsigned char *base = (unsigned char*)eh;
    if ( eh->imports != 0 && eh->imports_size != 0 )
      {
        IMAGE_IMPORT_DESCRIPTOR *import = (IMAGE_IMPORT_DESCRIPTOR *)(base + eh->imports);
        for ( ;import->Name; ++import )
          {
            int i,*offs = (int *)(base+import->OriginalFirstThunk);
            void **func = (void **)(base+import->FirstThunk);
            HMODULE m = 0;
            char *modname;
            if ( (unsigned)offs == (unsigned)base )
              offs = (int*)func;
            modname = (char*)base+import->Name;
            m = LoadLibraryA(modname);
            if ( !m ) 
              __asm int 3
              ;
            for ( ; *offs ; ++func, ++offs )
              {
                if ( *offs & 0x80000000 ) // by ordinal
                  {
                    *func = GetProcAddress(m,(char*)(*offs&0x0ffff));
                  }
                else
                  {
                    *func = GetProcAddress(m,(char*)base+(*offs+2));
                  }
                if ( !*func ) 
                  __asm int 3
                  ;
              }
          }
      }
  }

CORE0_HEADER *load_CORE()
  {
    void *unpacked;
    CORE0_HEADER *eh;

    unsigned l = *(unsigned*)core0S;
    unpacked = VirtualAlloc(0,l,MEM_COMMIT,PAGE_EXECUTE_READWRITE);
    if (!unpacked) unpacked = VirtualAlloc(0,l,MEM_COMMIT,PAGE_READWRITE);
    if (!unpacked)
      __asm int 3
      ;
      
    lz77ss_decompress(core0S+4,core0S_size-4,unpacked,l);

    eh = (CORE0_HEADER*)unpacked;
    Relocate(eh);
    Import(eh);

    return eh;
  }

bool InstallVBOX(unsigned flags)
  {
    HMODULE ntdll = GetModuleHandle("ntdll.dll");
    HMODULE kern32 = GetModuleHandle("kernel32.dll");
    T_Install _Install =0;
    void *base;
#ifdef _DEBUG
    HMODULE hm = LoadLibrary("cor1dbg.dll");
    _Install = (T_Install)GetProcAddress(hm,"_Install@16");
    base = hm;
#else
    //__asm int 3;
    if ( CORE0_HEADER *c0 = load_CORE() )
      {
        *(void**)&_Install = (char*)c0+c0->install;
        base = c0;
      }
#endif
    if ( _Install )
      return !!(g_vbox_funcs = _Install(flags,ntdll,kern32,base));
    else
      return false;
  }

/*bool InstallVBOX(unsigned flags)
  {
#ifndef _RELEASE_BUILD
    HMODULE hm = LoadLibrary("cor1dbg.dll");
    T_Install _Install = (T_Install)GetProcAddress(hm,"_Install@16");
    HMODULE ntdll = GetModuleHandle("ntdll.dll");
    HMODULE kern32 = GetModuleHandle("kernel32.dll");
    return !!(g_vbox_funcs = _Install(flags,ntdll,kern32,hm));
#else
    return false;
#endif
  }*/

bool GetVboxRegistryText(char **p,int *len)
  {
    if ( g_vbox_funcs )
      return 0 == g_vbox_funcs->SerilizeVirtualRegistryAsText(0,0,p,len);
    else
      return false;
  }

bool ClearVboxRegistry()
  {
    return true;
  }

bool LogVboxState()
  {
    if ( g_vbox_funcs )
      return 0 == g_vbox_funcs->LogState();
    else
      return false;
  }
