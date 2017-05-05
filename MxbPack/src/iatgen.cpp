
#include "myafx.h"
#include <time.h>

struct IAT_func_t
  {
    char * n; int hint; unsigned long m;
  };

struct IAT_space_t
  {
    char *n;
    int l,ml;
    IAT_func_t *mand;
    IAT_func_t *opt;
  };

int GenerateIAT(byte_t *section,unsigned section_rva,unsigned section_size,IMAGE_DATA_DIRECTORY *dict, unsigned reserve, EhFilter ehf)
  {
    static IAT_func_t IAT_k32_mand[] =
      {{"GetModuleHandleA",375},
       {"VirtualProtect",884},
       {"GetProcAddress",393},
       {"HeapCreate",497},
       {"HeapDestroy",499},
       {"HeapFree",501},
       {"HeapAlloc",495},
       {"RtlUnwind",689},
        0};
    static IAT_func_t IAT_k32_opt[] =
      {{"ExitProcess",182},
       {"GetCurrentProcess",314},
       {"CloseHandle",46},
       {"CreateFileA",77},
       {"LoadLibraryA",577},
       {"LCMapStringA",544},
       0};
    static IAT_func_t IAT_u32_opt[] =
      {{"wsprintfA",728},
       {"SetWindowLongA",640},
       {"CharLowerBuffA",39},
       {"CloseWindow",68},
       {"CreateWindowExA",96},
       0};
    static IAT_func_t IAT_a32_opt[] =
      {{"RegCloseKey",458},
       {"RegCreateKeyA",461},
       {"RegDeleteKeyA",465},
       {"RegDeleteValueA",467},
       {"RegEnumKeyA",470},
       {"RegEnumValueA",474},
       {"RegOpenKeyA",483},
       {"RegQueryValueA",492},
       {"RegSetValueA",505},
       0};
    static IAT_func_t IAT_o32_opt[] =
      {{"OleRun",267},
       {"CoInitialize",58},
       {"CoInitializeEx",59},
       {"CoMarshalInterface",70},
       {"CoRegisterClassObject",79},
       {"CoUninitialize",104},
       {"CoCreateInstance",16},
       {"CoCreateInstanceEx",17},
       {"CoGetClassObject",34},
       0};
    static IAT_func_t IAT_vcr_opt[] =
      {{"_onexit",0},
       {"_exit",0},
       {"_stricmp",0},
       {"sprintf",0},
       {"printf",0},
       {"memcpy",0},
       {"memmove",0},
       {"strcpy",0},
       {"memset",0},
       {"strlen",0},
       {"abort",0},
       {"fopen",0},
       {"fread",0},
       {"fseek",0},
       {"fclose",0},
       {"_initterm",0},
       0};
    static IAT_space_t IAT_space[] =
      {
        {"kernel32.dll",0,sizeof(IAT_k32_opt)/sizeof(IAT_func_t) -1,IAT_k32_mand,IAT_k32_opt},
        {"user32.dll",0,sizeof(IAT_u32_opt)/sizeof(IAT_func_t) -1,0,IAT_u32_opt},
        {"advapi32.dll",0,sizeof(IAT_a32_opt)/sizeof(IAT_func_t) -1,0,IAT_a32_opt},
        //{"ole32.dll",0,sizeof(IAT_o32_opt)/sizeof(IAT_func_t) -1,0,IAT_o32_opt},
        //{"msvcrt.dll",0,sizeof(IAT_vcr_opt)/sizeof(IAT_func_t) -1,0,IAT_vcr_opt},
        {0}
      };

    u32_t seed = time(0);

    ArrayT< BufferT<IAT_func_t*> > functions;
    unsigned long IAT_m = (unsigned long)time(0);
    unsigned IAT_limit = section_size-reserve;

    IAT_limit = IAT_limit&~3;
    unsigned char *iat_pS = (unsigned char*)section;
    unsigned char *iat_p = iat_pS;

    for ( IAT_space_t *p = IAT_space; p->n ; ++p )
      IAT_limit -= 5*4; // import record
    IAT_limit -= 5*4; // imports terminator
    IAT_limit &= ~3;

    unsigned char *iat_pE = iat_pS + IAT_limit;

    for ( IAT_space_t *p = IAT_space; p->n ; ++p )
      {
        functions.Push(BufferT<IAT_func_t*>());
        p->l = p->ml;
        if (p->mand)
          for ( IAT_func_t *j = p->mand; j->n; ++j )
            {
              functions.Last().Push(j);
              IAT_limit -= ((strlen(j->n) + 2) & ~1) + 2 + 4;
              // len aligned per 2, hint, thunk
            }
        IAT_limit -= ((strlen(p->n) + 2) & ~1) + 4;
        // len aligned per 2, import record, thunk terminator
      }

    memset(iat_pS,0,IAT_limit);
    unsigned imports_rva = section_rva + section_size - 5*4*(functions.Count()+1);
    memset(iat_pS+section_size - 5*4*(functions.Count()+1),0,5*4*(functions.Count()+1));

    bool f = true;
    while ( IAT_limit > 0 && f )
      {
        f = false;
        for ( int i = 0; IAT_space[i].n ; ++i )
          {
            IAT_space_t *p = IAT_space+i;
            if ( p->opt && p->l)
              {
                int x = teggo::unsigned_random(&seed)%p->l;
                int j = 0;
                for ( ; j < p->ml; ++j )
                  {
                    if ( p->opt[j].m != IAT_m )
                      if ( !x-- )
                        break;
                  }
                if ( j < p->ml )
                  {
                    if ( IAT_limit < ((strlen(p->opt[j].n) + 2) & ~1) + 2 + 4 )
                      break;
                    f = true;
                    p->opt[j].m = IAT_m;
                    functions[i].Push(p->opt+j);
                    IAT_limit -= ((strlen(p->opt[j].n) + 2) & ~1) + 2 + 4;
                    --p->l;
                  }
              }
          }
      }

    int thunks_count = 0;
    int thunks_count_i = 0;

    {for ( int i = 0; i < functions.Count(); ++i )
      {
        thunks_count += 1 + functions[i].Count();
      }}

    IMAGE_IMPORT_DESCRIPTOR *imports = (IMAGE_IMPORT_DESCRIPTOR*)(iat_p+imports_rva-section_rva);
    unsigned int *thunks    = (unsigned int *)iat_p;
    unsigned int iat_rva    = section_rva;
    unsigned int thunks_rva = iat_rva;
    unsigned int names_rva  = section_rva + 4*thunks_count;
    iat_p += 4*thunks_count;

    {for ( int i = 0; i < functions.Count(); ++i )
      {
        int l;
        //iat_p = (byte_t*)(((unsigned)iat_p+1)&~1);
        //names_rva = ((names_rva+1)&~1);
        imports[i].OriginalFirstThunk = imports[i].FirstThunk = thunks_rva;
        imports[i].Name = names_rva;
        strcpy((char*)iat_p,IAT_space[i].n);
        l = ( strlen(IAT_space[i].n) + 2 ) & ~1;
        names_rva += l; iat_p += l;
        for ( int j = 0; j < functions[i].Count(); ++j )
          {
            /*if ( !strcmp("GetModuleHandleA",functions[i][j]->n) )
              {
                *(unsigned int*)(destbase+box_code->PointerToRawData+0x2000) =
                  thunks_rva + NT_hdr->OptionalHeader.ImageBase;
              }*/
            *thunks++ = names_rva;
            thunks_rva+=4;
            *(unsigned short*)iat_p = functions[i][j]->hint;
            l = 2;
            strcpy((char*)iat_p+2,functions[i][j]->n);
            l += strlen(functions[i][j]->n) + 1;
            *(iat_p+l) = 0;
            l = (l+1) & ~1;
            iat_p += l;
            iat_p[-1] = 0;
            names_rva += l;
          }
        *thunks++ = 0;
        thunks_rva += 4;
        REQUIRE(iat_p <= iat_pE);
      }}

    u32_t sr = time(0);
    iat_p = (byte_t*)(((unsigned)iat_p+3)&~3);
    for ( ;iat_p != iat_pE; iat_p +=4 ) *(unsigned*)iat_p = teggo::unsigned_random(&sr);

    dict[IMAGE_DIRECTORY_ENTRY_IMPORT].Size = 5*4*(functions.Count());
    dict[IMAGE_DIRECTORY_ENTRY_IMPORT].VirtualAddress = imports_rva;
    dict[IMAGE_DIRECTORY_ENTRY_IAT].Size = 0;//4*(thunks_count);
    dict[IMAGE_DIRECTORY_ENTRY_IAT].VirtualAddress = 0;//iat_rva;

    return iat_pE-iat_pS;
  }
