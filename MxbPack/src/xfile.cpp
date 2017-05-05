
/*

  Copyright (c) 2008-2009, Alexey Sudachen, alexey@sudachen.name

*/

#include "myafx.h"
#include <time.h>
#include "peover.h"
#include "teggovbox.h"

unsigned QuerySystemHWID()
  {
    wchar_t p[256] = {0};
    wchar_t n[256] = {0};
    unsigned long serial = 0;
    unsigned long foo;
    u32_t crc = 0;
    int l = GetWindowsDirectoryW(p,255);
    p[StrFindChar(p,L'\\',0)+1] = 0;
    if ( GetVolumeInformationW(p,n,255,&serial,&foo,&foo,n,255) )
      crc = Crc32(0,&serial,4);
    CPUINFO cpui;
    GetCPUInfo(&cpui);
    crc = Crc32(crc,&cpui,sizeof(cpui));
    return crc;
  }

bool ListImports(StringParam file, CollectionT<ArrayT<StringW> > &dlls, ArrayT<StringW> &l, EhFilter ehf)
  {
    if ( byte_t *mapped = (byte_t*)SysMmapFile(+file,0,0,false,false,EMPTYEF) )
      {
        IMAGE_NT_HEADERS *nth = NT_HEADERS(mapped);
        IMAGE_SECTION_HEADER *S = IMAGE_FIRST_SECTION(nth);
        IMAGE_SECTION_HEADER *Se = S+nth->FileHeader.NumberOfSections;
        unsigned import_offs = nth->OptionalHeader.DataDirectory[IMAGE_DIRECTORY_ENTRY_IMPORT].VirtualAddress;
        unsigned import_count = nth->OptionalHeader.DataDirectory[IMAGE_DIRECTORY_ENTRY_IMPORT].Size;
        IMAGE_IMPORT_DESCRIPTOR *d = 0;
        for ( ; S != Se; ++S )
          if ( S->VirtualAddress <= import_offs && S->VirtualAddress + S->SizeOfRawData > import_offs )
            d = (IMAGE_IMPORT_DESCRIPTOR *)(mapped+S->PointerToRawData+import_offs-S->VirtualAddress);
        for ( ; d->Name; ++d )
          {
            StringW dn = StringW((pchar_t)(mapped+RVAtoOffs(mapped,d->Name))).ToLower();
            if (!dlls.Get(+dn))
              ListImports(+dn,dlls,dlls.Rec(+dn),ehf);
            l.Push(+dn);
          }
        SysUnmmapFile(mapped);
     }
     
    return true;
  }

unsigned RVAtoOffs(byte_t *mapped,unsigned rva)
  {
    IMAGE_NT_HEADERS *nth = NT_HEADERS(mapped);
    IMAGE_SECTION_HEADER *S = IMAGE_FIRST_SECTION(nth);
    IMAGE_SECTION_HEADER *Se = S+nth->FileHeader.NumberOfSections;
    for ( ; S != Se; ++S )
      if ( S->VirtualAddress <= rva && S->VirtualAddress + S->SizeOfRawData > rva )
        return S->PointerToRawData+rva-S->VirtualAddress;
    return 0;
  }

void SysTruncateFile(StringParam fname,unsigned len)
  {
    HANDLE hf = ::CreateFileW(+fname,GENERIC_READ|GENERIC_WRITE,0,0,OPEN_EXISTING,0,0);
    if ( hf && hf != INVALID_HANDLE_VALUE )
      {
        ::SetFilePointer(hf,len,0,FILE_BEGIN);
        ::SetEndOfFile(hf);
        ::CloseHandle(hf);
      }
  }

unsigned FindFunction(byte_t *mapped,IMAGE_NT_HEADERS *nth,IMAGE_SECTION_HEADER *S,char *fn)
  {
    unsigned foo = nth->OptionalHeader.DataDirectory[IMAGE_DIRECTORY_ENTRY_IMPORT].VirtualAddress - S->VirtualAddress;
    IMAGE_IMPORT_DESCRIPTOR *imports = (IMAGE_IMPORT_DESCRIPTOR*)(mapped + (S->PointerToRawData + foo));
    byte_t *a = mapped + ( (int)S->PointerToRawData - (int)S->VirtualAddress );
    //__asm int 3;
    for ( ;imports->Name; ++imports )
      {
        unsigned *j = (unsigned*)(a + imports->FirstThunk);
        for ( int i = 0; j[i]; ++i )
          if ( strcmp( (char*)(a + (j[i]+2)),  fn ) == 0 )
            return nth->OptionalHeader.ImageBase + imports->FirstThunk+i*4;
      }
    EMERGENCY(L"failed to find required API");
    return 0;
  }
  
void ReorderImports1(byte_t *mapped, IMAGE_IMPORT_DESCRIPTOR *d, FlatmapT<unsigned,unsigned> &imports, byte_t *xkey, EhFilter ehf)
  {
    int n = 0;
    u32_t *p = d->OriginalFirstThunk
      ?(u32_t*)(mapped+RVAtoOffs(mapped,d->OriginalFirstThunk))
      :(u32_t*)(mapped+RVAtoOffs(mapped,d->FirstThunk));
    while ( p[n] ) ++n;
    if ( n > 1 )
      {
        BufferT<u32_t> reord(n);
        for ( int i = 0; i < n; ++i )
          reord[i] = i;
        u32_t sid = *(u32_t*)xkey;
        for ( int i = 0; i < n; ++i )
          {
            u32_t q = teggo::unsigned_random(&sid)%n;
            if ( i != q )
              {
                u32_t t = reord[i]; reord[i] = reord[q]; reord[q] = t;
              }
          }
        for ( int i = 0; i < n; ++i )
          {
            int q = reord[i];
            imports.Put(d->FirstThunk+i,d->FirstThunk+i);
          }
        for ( int i = 0; i+1 < n; i+=2 )
          {
            cxx_swap(p[reord[i]],p[reord[i+1]]);
          }
      }
  }
  
void ReorderImports(byte_t *mapped, BufferT<SQUOTED_IMPORT> &squoted, byte_t *xkey, EhFilter ehf)
  {
    IMAGE_NT_HEADERS *nth = NT_HEADERS(mapped);
    IMAGE_SECTION_HEADER *S = IMAGE_FIRST_SECTION(nth);
    IMAGE_SECTION_HEADER *Se = S+nth->FileHeader.NumberOfSections;
    unsigned import_offs = nth->OptionalHeader.DataDirectory[IMAGE_DIRECTORY_ENTRY_IMPORT].VirtualAddress;
    unsigned import_count = nth->OptionalHeader.DataDirectory[IMAGE_DIRECTORY_ENTRY_IMPORT].Size;
    
    if ( !import_offs || !import_count ) return;
    
    IMAGE_IMPORT_DESCRIPTOR *d = 0;
    for ( ; S != Se; ++S )
      if ( S->VirtualAddress <= import_offs && S->VirtualAddress + S->SizeOfRawData > import_offs )
        d = (IMAGE_IMPORT_DESCRIPTOR *)(mapped+S->PointerToRawData+import_offs-S->VirtualAddress);

    if ( !d ) return;
        
    FlatmapT<unsigned,unsigned> imports; //rva, imprec
        
    for ( ; d->Name; ++d )
      {
        StringA dn = StringA((pchar_t)(mapped+RVAtoOffs(mapped,d->Name))).ToLower();
        if ( dn.StartWith("user32") || dn.StartWith("gdi32") )
          continue;
        //Xoln|+dn;
        ReorderImports1(mapped,d,imports,xkey,ehf);
      }
    
    if ( nth->OptionalHeader.DataDirectory[IMAGE_DIRECTORY_ENTRY_BASERELOC].VirtualAddress )
      {
        //__asm int 3;
        
        byte_t* reloc  = mapped+RVAtoOffs(mapped,nth->OptionalHeader.DataDirectory[IMAGE_DIRECTORY_ENTRY_BASERELOC].VirtualAddress);
        byte_t* relocE = reloc+nth->OptionalHeader.DataDirectory[IMAGE_DIRECTORY_ENTRY_BASERELOC].Size;

        while ( reloc < relocE )
          {
            u32_t seed = GetMillis();
            int rva; byte_t *e;
            rva = *(int*)reloc; reloc += 4;
            unsigned &bsize = *(unsigned*)reloc; reloc += 4;
            e = reloc+bsize-8;
            while ( reloc != e )
              {
                switch ( *(u16_t*)reloc >> 12 )
                  {
                    case 3:
                      {
                        unsigned fixup = *(u16_t*)reloc & 0x0fff;
                        unsigned q = *(unsigned*)(mapped+RVAtoOffs(mapped,rva+fixup));
                        if ( unsigned *p = imports.Get(q-nth->OptionalHeader.ImageBase) )
                          {
                            //__asm int 3;
                            *(unsigned*)(mapped+RVAtoOffs(mapped,rva+fixup)) = 
                              nth->OptionalHeader.ImageBase + teggo::unsigned_random(&seed)%nth->OptionalHeader.SizeOfImage;//0x0badf00d;
                            SQUOTED_IMPORT &i = *squoted.Insert(squoted.End());
                            i.rva = rva+fixup;
                            i.thunk = *p;
                            *(u16_t*)reloc = ((u16_t)teggo::unsigned_random(&seed))&0x0fff;
                            //memmove(reloc,reloc+2,relocE-reloc-2);
                            //reloc-=2;
                            //e-=2;
                            //relocE-=2;
                            //bsize-=2;
                          }
                      }
                  }
                reloc+=2;
              }
          }
        
        //byte_t* reloc1 = mapped+RVAtoOffs(mapped,nth->OptionalHeader.DataDirectory[IMAGE_DIRECTORY_ENTRY_BASERELOC].VirtualAddress);
        //byte_t* relocE1 = reloc1+nth->OptionalHeader.DataDirectory[IMAGE_DIRECTORY_ENTRY_BASERELOC].Size;
        //memset(relocE,0,relocE1-relocE);
        //nth->OptionalHeader.DataDirectory[IMAGE_DIRECTORY_ENTRY_BASERELOC].Size = relocE-reloc1;
      }
  }

bool IsSystemDLL(pwide_t path)
  {
    wchar_t buffer[512];
    if ( GetWindowsDirectoryW(buffer,sizeof(buffer)/2) )
      return teggo::StrLower(path).StartWith(+teggo::StrLower(buffer));
    return false;
  }
  
bool SearchFullPath(pwide_t name, StringW &fullpath)
  {
    wchar_t buffer[512] = {0};
    wchar_t *foo;
    if ( int l = SearchPathW(0,name,0,sizeof(buffer)/2,buffer,&foo) )
      {
        fullpath.SetRange(buffer,l);
        fullpath.ToLower();
        return true;
      }
    return false;
  }
  
bool PEoverFilterBounds(pwide_t pefile, ArrayT<StringW> &a, bool autobound, EhFilter ehf, bool silance)
  {
    CollectionT<ArrayT<StringW> > dlls;
    ArrayT<StringW> l;
    ListImports(pefile,dlls,l,ehf);
    
    CollectionT<unsigned> bundles;
    StringW fullpath;
    
    for ( int i=0; i < a.Count(); ++i )
      {
        bool f = false;
        if ( ArrayT<StringW> *p = dlls.GetLower(+a[i]) ) 
          {
            if ( SearchFullPath(dlls.KeyOf(p),fullpath) )
              bundles[+fullpath] = 1, f = true;
          }
        else if ( ArrayT<StringW> *p = dlls.GetLower(+(a[i]+L".dll")) ) 
          {
            if ( SearchFullPath(dlls.KeyOf(p),fullpath) )
              bundles[+fullpath] = 1, f = true;
          }
          
        if ( !f && !silance )
          Xoln| _S*"(!) bundling request for '%s' was rejected" % a[i];
      }      
    
    if ( autobound )
      for ( CollectionIteratorT<ArrayT<StringW> > i = dlls.Iterate(); i.Next(); )
        {
          if ( SearchFullPath(i.Key(),fullpath) )
            if ( !IsSystemDLL(+fullpath) )
              bundles[+fullpath] = 1;
        }
  
    a.Clear();
    for ( CollectionIteratorT<unsigned> i = bundles.Iterate(); i.Next(); )
      a.Push(i.Key());
      
    return true;
  }

struct STORAGESIGNATURE
  {
    u32_t signature;
    u16_t major;
    u16_t minor;
    u32_t external;
    u32_t version_len;
    char  version[0];
  };
  
bool PeIsDotNet(pwide_t file, unsigned ver[3], EhFilter ehf)
  {
    bool found = false;
    if ( byte_t *mapped = (byte_t*)SysMmapReadonlyFile(file,ehf) )
      {
        IMAGE_NT_HEADERS *nth = NT_HEADERS(mapped);
        IMAGE_SECTION_HEADER *S = IMAGE_FIRST_SECTION(nth);
        IMAGE_SECTION_HEADER *Se = S+nth->FileHeader.NumberOfSections;
        if ( unsigned cor20_offs = nth->OptionalHeader.DataDirectory[IMAGE_DIRECTORY_ENTRY_COM_DESCRIPTOR].VirtualAddress )
          if ( IMAGE_COR20_HEADER *cor20 = (IMAGE_COR20_HEADER *)(mapped+RVAtoOffs(mapped,cor20_offs)) )
            {
              STORAGESIGNATURE* ss = (STORAGESIGNATURE*)(mapped+RVAtoOffs(mapped,cor20->MetaData.VirtualAddress));
              sscanf(ss->version,"v%d.%d.%d",ver+0,ver+1,ver+2);
              found = true;
              //version = ((unsigned int)cor20->MajorRuntimeVersion<<16)|cor20->MinorRuntimeVersion;            
            }
        SysUnmmapFile(mapped);
      }
    return found;
  }
   
bool ReadImageNtHeaders(DataStreamPtr ds,IMAGE_NT_HEADERS *h)
  {
    IMAGE_DOS_HEADER dos = {0};    

    ds->Seek(0);
    if ( sizeof(dos) == ds->Read(&dos,sizeof(dos)) )
      if ( dos.e_magic == IMAGE_DOS_SIGNATURE ) 
        if ( TEGGO_OK == ds->Seek(dos.e_lfanew) )
          if ( sizeof(*h) == ds->Read(h,sizeof(*h)) )
            if ( h->Signature == IMAGE_NT_SIGNATURE )
              return true;
    return false;
  }
          
bool PeGetParams(DataStreamPtr ds, u32_t *size, u32_t *tls, u32_t *base, bool *relocatable)
  {      
    IMAGE_NT_HEADERS nth;
    if ( ReadImageNtHeaders(ds,&nth) && (nth.FileHeader.Characteristics&(IMAGE_FILE_DLL|IMAGE_FILE_EXECUTABLE_IMAGE)) == IMAGE_FILE_EXECUTABLE_IMAGE )   
      {
        *size = nth.OptionalHeader.SizeOfImage;
        *tls  = 0;
        *base = nth.OptionalHeader.ImageBase;            
        *relocatable = !!nth.OptionalHeader.DataDirectory[IMAGE_DIRECTORY_ENTRY_BASERELOC].VirtualAddress;
        return true;
      } 
    return false;
  }

