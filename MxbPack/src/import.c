
/*

  Copyright (c) 2008-2009, Alexey Sudachen, alexey@sudachen.name

*/

#if defined _WIN32 && !defined _WINDOWS_
# if !defined _X86_
#   define _X86_ 1
# endif
# if !defined WINVER
#   define WINVER 0x500
# endif
# if !defined _WIN32_WINNT
#   define _WIN32_WINNT 0x0500
# endif
#endif

#include <windows.h>
#include <winternl.h>

#include "import.h"

#define _XOrX(a,M,S) S

void *_NTDLL = 0;
void *_KERN32 = 0;

void *(__stdcall *_get_module_hook)(char *modname) = 0;
void *(__stdcall *_load_module_hook)(char *modname) = 0;

void *GetProcAddressIndirect(void *module,char *procname,int hint);

void *get_predefined_module(char *modname)
  {
    struct _predefined { void **hm; char *modname; };
    static struct _predefined predefined[] =
      {{&_KERN32,_XOrX("kernel32",85,"\x3e\x30\x27\x3b\x30\x39\x66\x67")},
       {&_NTDLL,_XOrX("ntdll",85,"\x3b\x21\x31\x39\x39")}};

    char *x = modname;
    int i = 0;
    for ( ; i < 2; ++i )
      {
        char *k = predefined[i].modname;
        while ( *k )
          if ( *x
               && (( *x <= 'Z' && *x >= 'A' && *k == ((*x-'A'+'a')^'\x55') )
                   || *k == (*x^'\x55')) )
            ++k,++x;
          else
            break;
        if ( !*k )
          return *predefined[i].hm;
      }
    return 0;
  }

int xstrncmp(char *a, char *b, int l)
  {
    for ( ;l && *a && *a == *b; --l, ++a, ++b ) (void)0;
    return l ? (int)*(unsigned char*)a-(int)*(unsigned char*)b : 0;
  }

void *function_bsearch(unsigned char *base,IMAGE_EXPORT_DIRECTORY *dir,char *name,int hint)
  {
    int k = 0;
    int L = 0;
    int R = 0;

    if ( hint >= 0 && hint < dir->NumberOfNames )
      if ( 0 == strcmp(name,(char*)base+((unsigned int*)(base+dir->AddressOfNames))[hint]) )
        {
          k = hint; goto l_hinted;
        }

    L = 0;
    R = dir->NumberOfNames-1;
    while ( L<=R )
      {
        unsigned char *r_name;
        int cmp;
        k = (L+R)/2;
        r_name = base+((unsigned int*)(base+dir->AddressOfNames))[k];
        if ( hint < 0 )
          cmp = xstrncmp(name,(char*)r_name,127);
        else
          cmp = strncmp(name,(char*)r_name,127);
        if ( !cmp )
          {
            unsigned fndx;
            void *proc;
          l_hinted:
            fndx = ((unsigned short*)(base+dir->AddressOfNameOrdinals))[k];
            proc =
              (base + ((unsigned int*)(base+dir->AddressOfFunctions))[fndx/*-dir->Base*/]);
            return  proc;
          }
        else
          if ( cmp > 0 ) L = k+1;
          else R = k-1;
      }
    return 0;
  }

void *GetProcAddressIndirect(void *module,char *procname,int hint)
  {
  repeat:
    if ( (unsigned int)module > 64*1024 && (unsigned int)module < 0xffff0000 && !((unsigned int)module&3) )
    {
      unsigned char *base = (unsigned char*)module;
      IMAGE_NT_HEADERS const *_nth = (IMAGE_NT_HEADERS const*)(base+((IMAGE_DOS_HEADER const*)base)->e_lfanew);
      IMAGE_DATA_DIRECTORY expo = _nth->OptionalHeader.DataDirectory[IMAGE_DIRECTORY_ENTRY_EXPORT];
      IMAGE_EXPORT_DIRECTORY *edir = (IMAGE_EXPORT_DIRECTORY*)(base+expo.VirtualAddress);
      unsigned char *p = 0;

      if ( !expo.Size || !expo.VirtualAddress )
        return 0;

      if ( ((unsigned int)procname >> 16) == 0 )
        p = (base +
             ((unsigned int*)(base+edir->AddressOfFunctions))
             [(unsigned int)procname-edir->Base]);
      else
        p = (unsigned char *)function_bsearch(base,edir,procname,hint);

      if ( base+expo.VirtualAddress < p && p < base+expo.VirtualAddress+expo.Size )
        {
          char modname[128] = {0};
          char *m = modname;
          while ( *p && *p != '!' && *p != '.' ) *m++ = *p++;
          procname = (char *)(p+1);

          //if ( !(module = get_predefined_module(modname)) )
            if ( !(module = GetModuleHandleA(modname) ) )
              module = LoadLibraryA(modname);
            //if ( _get_module_hook )
            //  if ( !(module = _get_module_hook(modname)) )
            //    if ( _load_module_hook )
            //      module = _load_module_hook(modname);

          if ( module )
            goto repeat;
          else
            return 0;
        }
      else
        return p;
    }
    return 0;
  }

void Import1(IMAGE_IMPORT_DESCRIPTOR *import,unsigned char *base, loadlibrary_t l)
  {
    int i,*offs = (int *)(base+import->OriginalFirstThunk);
    void *m = 0, **func = (void **)(base+import->FirstThunk);
    char *modname;
    if ( (unsigned)offs == (unsigned)base )
      offs = (int*)func;
    modname = (char*)base+import->Name;
    if ( l )  m = l(modname);
    if ( !m ) m = LoadLibraryA(modname);
    if ( !m )
      __asm int 3
      ;
    for ( ; *offs ; ++func, ++offs )
      {
        char *n;
        if ( *offs & 0x80000000 ) // by ordinal
          n = (char*)(*offs&0x0ffff);
        else
          n = (char*)base+(*offs+2);
        *(void**)func = GetProcAddressIndirect(m,n,0);
        if ( !*func )
          __asm int 3
          ;
      }
  }

void Import(unsigned char *base, IMAGE_NT_HEADERS const *nth, loadlibrary_t l)
  {
    IMAGE_DATA_DIRECTORY const d = nth->OptionalHeader.DataDirectory[IMAGE_DIRECTORY_ENTRY_IMPORT];
    if ( d.VirtualAddress != 0 && d.Size != 0 )
      {
        IMAGE_IMPORT_DESCRIPTOR *import = (IMAGE_IMPORT_DESCRIPTOR *)(base + d.VirtualAddress);
        for ( ;import->Name; ++import )
          Import1(import,base,l);
      }
  }

void Relocate(unsigned char *base, IMAGE_NT_HEADERS const*nth)
  {
    unsigned offs = (unsigned)base - nth->OptionalHeader.ImageBase;
    const unsigned char* reloc  = base+nth->OptionalHeader.DataDirectory[IMAGE_DIRECTORY_ENTRY_BASERELOC].VirtualAddress;
    const unsigned char* relocE = reloc+nth->OptionalHeader.DataDirectory[IMAGE_DIRECTORY_ENTRY_BASERELOC].Size;
    while ( reloc != relocE )
      {
        unsigned bsize; int rva; unsigned char const *e;
        rva = *(int*)reloc; reloc += 4;
        bsize = *(unsigned*)reloc-8; reloc += 4;
        e = reloc+bsize;
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
