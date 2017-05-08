
/*

  Copyright (c) 2008-2009, Alexey Sudachen, alexey@sudachen.name

*/

#include "myafx.h"
#include <time.h>
#include "peover.h"
#include "../Chain0/stbinfo.h"
#include "../Core/teggovbox.h"

extern "C" unsigned char code_bytes[];
extern "C" unsigned code_bytes_size;
//extern "C" unsigned char chain0S[];
//extern "C" unsigned chain0S_size;
extern "C" unsigned char core0S_demo[];
extern "C" unsigned core0S_demo_size;
extern "C" unsigned char core0S[];
extern "C" unsigned core0S_size;
extern "C" unsigned char mirage0S[];
extern "C" unsigned mirage0S_size;
extern "C" unsigned mirage0S_ep;
extern "C" unsigned mirage0S_relocs[];
enum { POLYMORF_STUFF = 768 };

extern void ConfigureMirage(unsigned* relocs, byte_t* code, unsigned section_va, unsigned plymorph_va,unsigned getmodhand_va);
extern "C" void polygen_rand_xor(void* pS, void* pE, unsigned seed);
extern "C" int poligen_entrypoint(unsigned address, unsigned target, unsigned count, unsigned pVa, void* pS, void* pE,
                                  unsigned seed, unsigned vprotect);

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

unsigned RVAtoOffs(byte_t* mapped,unsigned rva)
{
    IMAGE_NT_HEADERS* nth = NT_HEADERS(mapped);
    IMAGE_SECTION_HEADER* S = IMAGE_FIRST_SECTION(nth);
    IMAGE_SECTION_HEADER* Se = S+nth->FileHeader.NumberOfSections;
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

namespace
{
    StringW  g_outfile;
    unsigned g_wfile_flags = 0;
    HANDLE   g_temporal_data = 0;
    enum { WRAPPING_EXTENSION = 256*1024 };
    byte_t   g_xcrypt_key[16] = {0};
    unsigned g_stbinfo_offset = 0;
    unsigned g_stbinfo_max = 0;
    byte_t*  g_bundles_area = 0;
    unsigned g_bundles_area_length = 0;
    byte_t*  g_zipped_area = 0;
    unsigned g_zipped_area_length = 0;
    unsigned g_tls_index = 0;
    unsigned g_tls_zero_count = 0;
    unsigned g_tls_callback = 0;
    BufferT<byte_t> g_tls;
    BufferT<byte_t> g_rsc;
    BufferT<BUNDLED_IMPORT> g_bundles;
    BufferT<SQUOTED_IMPORT> g_squoted;
    BufferT<SQUOTED_IMPORT> g_squoted_act;
}

int InitXdataSection(byte_t* mapped,IMAGE_SECTION_HEADER* S,IMAGE_NT_HEADERS* nth,unsigned* file_length,EhFilter ehf);
/*
  {
    memset(S,0,sizeof(*S));
    memcpy(S->Name,".data\0",8);
    *file_length = cxx_alignu(*file_length,nth->OptionalHeader.FileAlignment);
    S->PointerToRawData = *file_length;
    enum { SPECIAL_DATA = 32+8 };
    unsigned starter_sz  = ((chain0S_size+15)&~15);
    unsigned reserve = starter_sz + SPECIAL_DATA;
    S->Misc.VirtualSize   = 1200+reserve;
    S->SizeOfRawData = cxx_alignu(S->Misc.VirtualSize,nth->OptionalHeader.FileAlignment);
    S->VirtualAddress = nth->OptionalHeader.SizeOfImage;
    S->Characteristics = IMAGE_SCN_MEM_READ|IMAGE_SCN_MEM_WRITE|IMAGE_SCN_CNT_INITIALIZED_DATA;
    nth->OptionalHeader.SizeOfImage = S->VirtualAddress + cxx_alignu(S->Misc.VirtualSize,nth->OptionalHeader.SectionAlignment);
    nth->OptionalHeader.SizeOfInitializedData += S->SizeOfRawData;
    *file_length += S->SizeOfRawData;
    nth->FileHeader.NumberOfSections += 1;
    int o = GenerateIAT(mapped+S->PointerToRawData,S->VirtualAddress,S->SizeOfRawData,nth->OptionalHeader.DataDirectory,reserve,ehf);
    memcpy(mapped+S->PointerToRawData+o,chain0S,chain0S_size);
    memset(mapped+S->PointerToRawData+o+starter_sz,0,SPECIAL_DATA);
    return o;
  }
*/

bool InitTlsDataSection(byte_t* mapped,IMAGE_SECTION_HEADER* S,IMAGE_NT_HEADERS* nth,unsigned* file_length,unsigned reserved,
                        unsigned inited, EhFilter ehf)
{
    memset(S,0,sizeof(*S));
    memcpy(S->Name,".tls\0\0",8);
    *file_length = (*file_length + nth->OptionalHeader.FileAlignment-1)&~(nth->OptionalHeader.FileAlignment-1);
    unsigned aligned_size = (reserved + nth->OptionalHeader.SectionAlignment-1)&~(nth->OptionalHeader.SectionAlignment-1);
    if ( inited )
    {
        S->SizeOfRawData    = (inited+nth->OptionalHeader.FileAlignment-1)&~(nth->OptionalHeader.FileAlignment-1);
        S->PointerToRawData = *file_length;
        S->Characteristics = IMAGE_SCN_MEM_READ|/*IMAGE_SCN_MEM_WRITE|*/IMAGE_SCN_CNT_INITIALIZED_DATA;
        nth->OptionalHeader.SizeOfInitializedData += aligned_size;
    }
    else
    {
        S->SizeOfRawData    = 0;
        S->PointerToRawData = 0;
        S->Characteristics = IMAGE_SCN_MEM_READ|/*IMAGE_SCN_MEM_WRITE|*/IMAGE_SCN_CNT_UNINITIALIZED_DATA;
        nth->OptionalHeader.SizeOfUninitializedData += aligned_size;
    }
    S->Misc.VirtualSize = reserved+1; //aligned_size;
    S->VirtualAddress = S[-1].VirtualAddress
                        + ( cxx_max(S[-1].SizeOfRawData,S[-1].Misc.VirtualSize)
                            + nth->OptionalHeader.SectionAlignment-1)
                        &~(nth->OptionalHeader.SectionAlignment-1);
    nth->OptionalHeader.SizeOfImage = S->VirtualAddress + aligned_size;
    *file_length += S->SizeOfRawData;
    nth->FileHeader.NumberOfSections += 1;
    unsigned aligned = (S->SizeOfRawData + nth->OptionalHeader.FileAlignment-1)&~(nth->OptionalHeader.FileAlignment-1);
    memset(mapped+S->PointerToRawData,0,aligned);
    return true;
}

bool InitRSRCSection(byte_t* mapped,IMAGE_SECTION_HEADER* S,IMAGE_NT_HEADERS* nth,unsigned* file_length,EhFilter ehf)
{
    memcpy(S->Name,".rsrc\0\0",8);
    *file_length = cxx_alignu(*file_length,nth->OptionalHeader.FileAlignment);
    S->PointerToRawData = *file_length;
    S->Misc.VirtualSize = g_rsc.Count();
    S->SizeOfRawData = cxx_alignu(g_rsc.Count(),nth->OptionalHeader.FileAlignment);
    //S->SizeOfRawData = S->Misc.VirtualSize = (g_rsc.Count() + nth->OptionalHeader.FileAlignment-1)&~(nth->OptionalHeader.FileAlignment-1);
    S->VirtualAddress = S[-1].VirtualAddress
                        + cxx_alignu( cxx_max(S[-1].SizeOfRawData,S[-1].Misc.VirtualSize),nth->OptionalHeader.SectionAlignment);
    S->Characteristics = IMAGE_SCN_MEM_READ|IMAGE_SCN_CNT_INITIALIZED_DATA;;
    nth->OptionalHeader.SizeOfImage = S->VirtualAddress + cxx_alignu(S->Misc.VirtualSize,nth->OptionalHeader.SectionAlignment);
    nth->OptionalHeader.SizeOfInitializedData += S->SizeOfRawData;
    memcpy(mapped+*file_length,+g_rsc,g_rsc.Count());
    RscFixup(mapped+*file_length,S->VirtualAddress);
    *file_length = cxx_alignu(*file_length + g_rsc.Count(),nth->OptionalHeader.FileAlignment);
    nth->FileHeader.NumberOfSections += 1;
    nth->OptionalHeader.DataDirectory[IMAGE_DIRECTORY_ENTRY_RESOURCE].VirtualAddress = S->VirtualAddress;
    nth->OptionalHeader.DataDirectory[IMAGE_DIRECTORY_ENTRY_RESOURCE].Size = g_rsc.Count();
    return true;
}

unsigned FindFunction(byte_t* mapped,IMAGE_NT_HEADERS* nth,IMAGE_SECTION_HEADER* S,char* fn)
{
    unsigned foo = nth->OptionalHeader.DataDirectory[IMAGE_DIRECTORY_ENTRY_IMPORT].VirtualAddress - S->VirtualAddress;
    IMAGE_IMPORT_DESCRIPTOR* imports = (IMAGE_IMPORT_DESCRIPTOR*)(mapped + (S->PointerToRawData + foo));
    byte_t* a = mapped + ( (int)S->PointerToRawData - (int)S->VirtualAddress );
    //__asm int 3;
    for ( ; imports->Name; ++imports )
    {
        unsigned* j = (unsigned*)(a + imports->FirstThunk);
        for ( int i = 0; j[i]; ++i )
            if ( strcmp( (char*)(a + (j[i]+2)),  fn ) == 0 )
                return nth->OptionalHeader.ImageBase + imports->FirstThunk+i*4;
    }
    EMERGENCY(L"failed to find required API");
    return 0;
}

bool PlaceChain0(byte_t* mapped,unsigned* file_length, STB_INFO* info, u32_t minsize, u32_t mintls, EhFilter ehf);
/*  {
    IMAGE_NT_HEADERS *nth = NT_HEADERS(mapped);
    IMAGE_SECTION_HEADER *S = IMAGE_FIRST_SECTION(nth);
    IMAGE_SECTION_HEADER *St = S;

    u32_t seed = time(0);
    unsigned gap = teggo::unsigned_random(&seed);
    gap = ((gap%251)+16)&~15;

    *file_length = (*file_length + nth->OptionalHeader.FileAlignment-1)&~(nth->OptionalHeader.FileAlignment-1);
    if ( !(g_wfile_flags & PEWRAP_RELINK) )
      {
        bool relocs_attached = false;
        IMAGE_SECTION_HEADER  Sx = S[nth->FileHeader.NumberOfSections-1];
        if ( 0 == memcmp(Sx.Name,".reloc",7) && (byte_t*)(S+(nth->FileHeader.NumberOfSections+1)) >= mapped+S[0].PointerToRawData )
          {
            relocs_attached = true;
            --nth->FileHeader.NumberOfSections;
          }
        S += nth->FileHeader.NumberOfSections;
        memset(S,0,sizeof(*S));
        memcpy(S->Name,".VS\0\0",8);
        S->PointerToRawData = *file_length;
        S->SizeOfRawData  = chain0S_size+2084;
        S->VirtualAddress = Sx.VirtualAddress
                + ( cxx_max(Sx.SizeOfRawData,Sx.Misc.VirtualSize)
                  + nth->OptionalHeader.SectionAlignment-1)
                  &~(nth->OptionalHeader.SectionAlignment-1);
        S->Characteristics = IMAGE_SCN_CNT_CODE|IMAGE_SCN_MEM_EXECUTE|IMAGE_SCN_MEM_READ;
        nth->FileHeader.NumberOfSections += 1;
        for ( int i = 0; i < gap/4; ++i ) *(unsigned*)(mapped+S->PointerToRawData+i*4) = teggo::unsigned_random(&seed);
        memcpy(mapped+S->PointerToRawData+gap,chain0S,chain0S_size);
        S->SizeOfRawData = (S->SizeOfRawData + nth->OptionalHeader.FileAlignment-1)&~(nth->OptionalHeader.FileAlignment-1);
        *file_length += S->SizeOfRawData;
        *file_length = (*file_length + nth->OptionalHeader.FileAlignment-1)&~(nth->OptionalHeader.FileAlignment-1);
        if ( relocs_attached )
          {
            //memcpy(S->Name,".reloc\0",8);
            S->VirtualAddress   = Sx.VirtualAddress;
            nth->OptionalHeader.SizeOfInitializedData -= Sx.SizeOfRawData;
            gap += S->PointerToRawData - Sx.PointerToRawData;
            S->SizeOfRawData   += S->PointerToRawData - Sx.PointerToRawData;
            S->PointerToRawData = Sx.PointerToRawData;
          }
        S->Misc.VirtualSize = S->SizeOfRawData; //(S->SizeOfRawData + nth->OptionalHeader.SectionAlignment-1)&~(nth->OptionalHeader.SectionAlignment-1);
        nth->OptionalHeader.AddressOfEntryPoint = S->VirtualAddress + gap;
        nth->OptionalHeader.SizeOfImage = S->VirtualAddress + cxx_alignu(S->Misc.VirtualSize,nth->OptionalHeader.SectionAlignment);
        nth->OptionalHeader.SizeOfCode += S->SizeOfRawData;
        if ( nth->OptionalHeader.SizeOfImage < minsize )
          {
            nth->OptionalHeader.SizeOfImage = cxx_alignu(cxx_max<u32_t>(nth->OptionalHeader.SizeOfImage,minsize),nth->OptionalHeader.SectionAlignment);
            S->Misc.VirtualSize = nth->OptionalHeader.SizeOfImage-S->VirtualAddress;
          }
      }

    if ( g_wfile_flags & PEWRAP_RELINK )
      {
        IMAGE_SECTION_HEADER *S1 = S+nth->FileHeader.NumberOfSections;
        int starter = InitXdataSection(mapped,S1,nth,file_length,ehf);
        unsigned vprotect = FindFunction(mapped,nth,S1,"VirtualProtect");
        unsigned gemodhand = FindFunction(mapped,nth,S1,"GetModuleHandleA");
        unsigned long seed = time(0);
        byte_t *p = mapped+S1->PointerToRawData;
        polygen_rand_xor(p+starter,p+starter+chain0S_size,seed);
        unsigned sz = (chain0S_size +3 & ~3);
        unsigned va_addr = S1->VirtualAddress+nth->OptionalHeader.ImageBase+starter;
        unsigned va_targ = St->VirtualAddress+nth->OptionalHeader.ImageBase+mirage0S_size+POLYMORF_STUFF;
        unsigned ep = (mirage0S_size+3)&~3;
        unsigned va_pva  = St->VirtualAddress+nth->OptionalHeader.ImageBase+ep;
        p = mapped+St->PointerToRawData+mirage0S_size;
        ep += poligen_entrypoint(va_addr,va_targ,sz,va_pva,p,p+POLYMORF_STUFF,seed,vprotect);
        memcpy(mapped+St->PointerToRawData,mirage0S,mirage0S_size);
        //memcpy(mapped+St->PointerToRawData,mirage0S,0x19);//mirage0S_size);
        //memcpy(mapped+St->PointerToRawData+0x284,mirage0S+0x284,0x78);//mirage0S_size);
        //*(byte_t*)(mapped+St->PointerToRawData+0x19) = 0xc3;

        if (1)
          ConfigureMirage(
            mirage0S_relocs,
            mapped+St->PointerToRawData,St->VirtualAddress+nth->OptionalHeader.ImageBase,
            St->VirtualAddress+nth->OptionalHeader.ImageBase+ep,
            gemodhand);
        nth->OptionalHeader.AddressOfEntryPoint = St->VirtualAddress+mirage0S_ep;

        bool zero_tls_data = false;
        //g_tls_index = 0;
        IMAGE_SECTION_HEADER *S2 = S1+1;
        if ( g_tls_index )
          {
            //__asm int 3;
            InitTlsDataSection(mapped,S2,nth,file_length,g_tls.Count()+g_tls_zero_count,(zero_tls_data?0:(g_tls.Count())),ehf);
            unsigned va_off = starter+((chain0S_size+15)&~15);
            unsigned tls_off = S1->PointerToRawData+va_off;
            IMAGE_TLS_DIRECTORY *tls   = (IMAGE_TLS_DIRECTORY*)(mapped+tls_off);
            memset(tls,0,sizeof(*tls));
            tls->StartAddressOfRawData = S2->VirtualAddress + nth->OptionalHeader.ImageBase;
            tls->EndAddressOfRawData   = tls->StartAddressOfRawData + g_tls.Count();
            tls->AddressOfIndex        = S1->VirtualAddress + nth->OptionalHeader.ImageBase + va_off + 36;
            tls->AddressOfCallBacks    = S1->VirtualAddress+va_off+sizeof(IMAGE_TLS_DIRECTORY) + nth->OptionalHeader.ImageBase;
            tls->SizeOfZeroFill        = g_tls_zero_count;
            tls->Characteristics       = 0;

            if ( !zero_tls_data )
              memcpy(mapped+S2->PointerToRawData,+g_tls,g_tls.Count());
            nth->OptionalHeader.DataDirectory[IMAGE_DIRECTORY_ENTRY_TLS].VirtualAddress = S1->VirtualAddress+va_off;
            nth->OptionalHeader.DataDirectory[IMAGE_DIRECTORY_ENTRY_TLS].Size = sizeof(IMAGE_TLS_DIRECTORY);//+4;
            ++S2;
          }
        if ( g_rsc.Count() )
          {
            InitRSRCSection(mapped,S2,nth,file_length,ehf);
            ++S2;
          }

        if ( info->zip.offset )
          {
            u32_t crcX = Crc32(nth->OptionalHeader.AddressOfEntryPoint,(byte_t*)S,info->zip.offset-((byte_t*)S-mapped));
            byte_t *zipped = mapped + info->zip.offset;
            ZIPPED_HEADER *zh = (ZIPPED_HEADER*)zipped;
            for ( int i = 0; i < zh->number; ++i )
              if ( zh->section[i].count )
                for ( u32_t r=crcX, j=0; j+4 < zh->section[i].count; j+=4 )
                  *(u32_t*)(zipped+zh->section[i].offset+j) ^= teggo::unsigned_random(&r);
            info->zip.crc = Crc32(0,zipped,info->zip.count);
          }
      }

    *file_length = (*file_length + nth->OptionalHeader.FileAlignment-1)&~(nth->OptionalHeader.FileAlignment-1);
    return true;
  }
*/

unsigned Core0sLength()
{
    if ( g_wfile_flags&PEWRAP_DEMO )
        return  core0S_demo_size;
    else
        return core0S_size;
}

byte_t const* Core0S()
{
    if ( g_wfile_flags&PEWRAP_DEMO )
        return  core0S_demo;
    else
        return core0S;
}

bool PlaceCore0(byte_t* mapped, unsigned* file_length, STB_INFO* info,EhFilter ehf)
{
    if ( g_wfile_flags & PEWRAP_DEBUGCORE )
    {
        info->flags |= STBF_EXTERNAL_CORE;
    }
    else
    {
        info->core.offset = *file_length;
        info->core.count  = Core0sLength();
        info->flags |= STBF_CORE_ZIPPED|STBF_CORE_XCRYPED;
        memcpy(mapped+*file_length,Core0S(),info->core.count);
        NEWDES_Cipher cipher;
        cipher.SetupEncipher(g_xcrypt_key);
        cipher.DoCipher(mapped+*file_length,info->core.count/8);
        info->core.crc = Crc32(0,mapped+*file_length,info->core.count);
        *file_length += info->core.count;
    }
    return true;
}

bool AttachBundles(EhFilter ehf)
{
    if ( g_bundles.Count() )
    {
        BUNDLE_HEADER hdr = {0};
        hdr.number = g_bundles.Count();
        DataStreamPtr ds = OpenHandleStream(g_outfile,STREAM_WRITE|STREAM_NONINHERITABLE);
        ds->Seek(0,SEEK_END);
        unsigned offset = ds->Tell();
        ds->Write(&hdr,sizeof(hdr));
        Qsort(g_bundles.Begin(),g_bundles.End(),LessBundle());
        BufferT<byte_t> edata(g_bundles.Count()*sizeof(g_bundles[0]));
        memcpy(+edata,+g_bundles,edata.Count());
        //if ( !(g_wfile_flags&PEWRAP_DEMO) )
        //  {
        NEWDES_Cipher cipher;
        cipher.SetupEncipher(g_xcrypt_key);
        cipher.DoCipher(+edata,edata.Count()/8);
        //  }
        //else
        //  {
        //    u32_t r = *(unsigned*)g_xcrypt_key;
        //    for ( int i = 0; i < edata.Count()/4; ++i )
        //      {
        //        ((unsigned*)+edata)[i] |= teggo::unsigned_random(&r);
        //      }
        //  }
        ds->Write(+edata,edata.Count());
        Unrefe(ds);

        if ( byte_t* mapped = (byte_t*)SysMmapFile(+g_outfile,0,0,true,false,ehf) )
        {
            STB_INFO* info = (STB_INFO*)(mapped+g_stbinfo_offset);
            info->bundle.offset = offset;
            info->bundle.count  = g_bundles.Count()*sizeof(g_bundles[0])+sizeof(hdr);
            info->bundle.crc    = Crc32(0,(byte_t*)&hdr,sizeof(hdr));
            info->bundle.crc    = Crc32(info->bundle.crc,(byte_t*)+g_bundles,g_bundles.Count()*sizeof(g_bundles[0]));
            SysUnmmapFile(mapped);
        }
        else
        { ehf("failed to write bundles info"); return false; }
    }
    return true;
}

bool AttachSquoted(EhFilter ehf)
{
    SQUOTED_HEADER hdr = {0};
    SQUOTED_HEADER hdr_act = {0};
    unsigned offset = 0, offset_act = 0;

    if ( g_squoted.Count() )
    {
        hdr.number = g_squoted.Count();
        DataStreamPtr ds = OpenHandleStream(g_outfile,STREAM_WRITE|STREAM_NONINHERITABLE);
        ds->Seek(0,SEEK_END);
        offset = ds->Tell();
        ds->Write(&hdr,sizeof(hdr));
        BufferT<byte_t> edata(g_squoted.Count()*sizeof(g_squoted[0]));
        memcpy(+edata,+g_squoted,edata.Count());
        NEWDES_Cipher cipher;
        cipher.SetupEncipher(g_xcrypt_key);
        cipher.DoCipher(+edata,edata.Count()/8);
        ds->Write(+edata,edata.Count());
        Unrefe(ds);
    }

    if ( g_squoted_act.Count() )
    {
        hdr.number = g_squoted_act.Count();
        DataStreamPtr ds = OpenHandleStream(g_outfile,STREAM_WRITE|STREAM_NONINHERITABLE);
        ds->Seek(0,SEEK_END);
        offset_act = ds->Tell();
        ds->Write(&hdr,sizeof(hdr));
        BufferT<byte_t> edata(g_squoted_act.Count()*sizeof(g_squoted[0]));
        memcpy(+edata,+g_squoted_act,edata.Count());
        NEWDES_Cipher cipher;
        cipher.SetupEncipher(g_xcrypt_key);
        cipher.DoCipher(+edata,edata.Count()/8);
        ds->Write(+edata,edata.Count());
        Unrefe(ds);
    }

    if ( g_squoted.Count() || g_squoted_act.Count() )
    {
        if ( byte_t* mapped = (byte_t*)SysMmapFile(+g_outfile,0,0,true,false,ehf) )
        {
            STB_INFO* info = (STB_INFO*)(mapped+g_stbinfo_offset);
            info->squoted.offset = offset;
            info->squoted.count  = g_squoted.Count()*sizeof(g_squoted[0])+sizeof(hdr);
            info->squoted.crc    = Crc32(0,(byte_t*)&hdr,sizeof(hdr));
            info->squoted.crc    = Crc32(info->squoted.crc,(byte_t*)+g_squoted,g_squoted.Count()*sizeof(g_squoted[0]));
            info->squoted_act.offset = offset_act;
            info->squoted_act.count  = g_squoted.Count()*sizeof(g_squoted_act[0])+sizeof(hdr_act);
            info->squoted_act.crc    = Crc32(0,(byte_t*)&hdr_act,sizeof(hdr_act));
            info->squoted_act.crc    = Crc32(info->squoted_act.crc,(byte_t*)+g_squoted_act,g_squoted_act.Count()*sizeof(g_squoted_act[0]));
            SysUnmmapFile(mapped);
        }
        else
        { ehf("failed to write squoted info"); return false; }
    }
    return true;
}

void ReorderImports1(byte_t* mapped, IMAGE_IMPORT_DESCRIPTOR* d, FlatmapT<unsigned,unsigned>& imports, byte_t* xkey,
                     EhFilter ehf)
{
    int n = 0;
    u32_t* p = d->OriginalFirstThunk
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

void ReorderImports(byte_t* mapped, BufferT<SQUOTED_IMPORT>& squoted, byte_t* xkey, EhFilter ehf)
{
    IMAGE_NT_HEADERS* nth = NT_HEADERS(mapped);
    IMAGE_SECTION_HEADER* S = IMAGE_FIRST_SECTION(nth);
    IMAGE_SECTION_HEADER* Se = S+nth->FileHeader.NumberOfSections;
    unsigned import_offs = nth->OptionalHeader.DataDirectory[IMAGE_DIRECTORY_ENTRY_IMPORT].VirtualAddress;
    unsigned import_count = nth->OptionalHeader.DataDirectory[IMAGE_DIRECTORY_ENTRY_IMPORT].Size;

    if ( !import_offs || !import_count ) return;

    IMAGE_IMPORT_DESCRIPTOR* d = 0;
    for ( ; S != Se; ++S )
        if ( S->VirtualAddress <= import_offs && S->VirtualAddress + S->SizeOfRawData > import_offs )
            d = (IMAGE_IMPORT_DESCRIPTOR*)(mapped+S->PointerToRawData+import_offs-S->VirtualAddress);

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
            int rva; byte_t* e;
            rva = *(int*)reloc; reloc += 4;
            unsigned& bsize = *(unsigned*)reloc; reloc += 4;
            e = reloc+bsize-8;
            while ( reloc != e )
            {
                switch ( *(u16_t*)reloc >> 12 )
                {
                    case 3:
                    {
                        unsigned fixup = *(u16_t*)reloc & 0x0fff;
                        unsigned q = *(unsigned*)(mapped+RVAtoOffs(mapped,rva+fixup));
                        if ( unsigned* p = imports.Get(q-nth->OptionalHeader.ImageBase) )
                        {
                            //__asm int 3;
                            *(unsigned*)(mapped+RVAtoOffs(mapped,rva+fixup)) =
                                nth->OptionalHeader.ImageBase + teggo::unsigned_random(&seed)%nth->OptionalHeader.SizeOfImage;//0x0badf00d;
                            SQUOTED_IMPORT& i = *squoted.Insert(squoted.End());
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

bool FixStub(byte_t* mapped, unsigned* file_length, unsigned flags, pwide_t stub, EhFilter ehf)
{
    return true;
}

void RelinkHeaders(byte_t* mapped)
{
    static unsigned common_DOSHDR[] =
    {
        0x00905a4d, 0x00000003, 0x00000004, 0x0000ffff,
        0x000000b8, 0x00000000, 0x00000040, 0x00000000,
        0x00000000, 0x00000000, 0x00000000, 0x00000000,
        0x00000000, 0x00000000, 0x00000000, 0x000000f0,
        0x0eba1f0e, 0xcd09b400, 0x4c01b821, 0x685421cd,
        0x70207369, 0x72676f72, 0x63206d61, 0x6f6e6e61,
        0x65622074, 0x6e757220, 0x206e6920, 0x20534f44,
        0x65646f6d, 0x0a0d0d2e, 0x00000024, 0x00000000,
    };

    IMAGE_DOS_HEADER* dsh = (IMAGE_DOS_HEADER*)mapped;
    if ( dsh->e_lfanew >= 0xC0 )
    {
        IMAGE_NT_HEADERS* nth = NT_HEADERS(mapped);
        unsigned sz = sizeof(IMAGE_SECTION_HEADER)*nth->FileHeader.NumberOfSections
                      + FIELD_OFFSET( IMAGE_NT_HEADERS, OptionalHeader ) + nth->FileHeader.SizeOfOptionalHeader;
        unsigned hdrsz = nth->OptionalHeader.SizeOfHeaders-0xC0-sz;
        memmove(mapped+0xC0,nth,sz);
        memset(mapped+0xC0+sz,0,hdrsz);
        memcpy(dsh,common_DOSHDR,sizeof(common_DOSHDR));
        unsigned mask = 0x8cabe24d;//(unsigned)time(0);
        //unsigned compid = 0x005a0c05;//0x0badcafe;
        unsigned* p = (unsigned*)((char*)dsh + 0x80);
        p[0] = 'SnaD' ^ mask; p[1] = 0 ^ mask; p[2] = 0 ^ mask; p[3] = 0 ^ mask;
        //p[4] = compid ^ mask; p[5] = 1 ^ mask; p[6] = 'hciR';   p[7] = mask;
        p[4] = 0x005d0fc3 ^ mask,  p[5] = 0x00000003 ^ mask,
                                          p[6] = 0x00010000 ^ mask,  p[7] = 0x00000001 ^ mask,
                                                  p[8] = 0x005f0c05 ^ mask,  p[9] = 0x00000001 ^ mask,
                                                          p[10] = 0x005a0c05 ^mask, p[11] = 0x00000001 ^ mask,
                                                                  p[12] = 'hciR', p[13] = mask, p[14] = 0, p[15] = 0;
        dsh->e_lfanew = 0xC0;
    }
}

bool Relink(byte_t* mapped, unsigned* file_length, unsigned flags, STB_INFO*& info, u32_t minsize, u32_t tlssize, EhFilter ehf)
{
    // creates two areas - g_zipped_area & g_newrsrc
    RelinkHeaders(mapped);
    IMAGE_NT_HEADERS* nth = NT_HEADERS(mapped);
    ZIPPED_HEADER* zh = 0;
    unsigned HEADERSIZE = sizeof(ZIPPED_HEADER) + (nth->FileHeader.NumberOfSections-1)*sizeof(ZIPPED_SECTION);
    unsigned zhl = HEADERSIZE;
    g_zipped_area_length = zhl;

    NEWDES_Cipher cipher;
    cipher.SetupEncipher(g_xcrypt_key);

    // pack all original sections into one
    IMAGE_SECTION_HEADER* S = IMAGE_FIRST_SECTION(nth);
    IMAGE_SECTION_HEADER* Se = S+nth->FileHeader.NumberOfSections;

    for ( int i = 0; i < nth->FileHeader.NumberOfSections; ++i )
    {
        zhl += S[i].SizeOfRawData;
    }

    if ( unsigned va = nth->OptionalHeader.DataDirectory[IMAGE_DIRECTORY_ENTRY_TLS].VirtualAddress )
    {
        unsigned tls_off = 0;
        for ( int i = 0; i < nth->FileHeader.NumberOfSections; ++i )
            if ( S[i].VirtualAddress <= va && va
                 < S[i].VirtualAddress + cxx_max(nth->OptionalHeader.SectionAlignment,
                                                 cxx_align(S[i].SizeOfRawData,nth->OptionalHeader.SectionAlignment)) )
            {
                tls_off = S[i].PointerToRawData+va-S[i].VirtualAddress;
                break;
            }
        IMAGE_TLS_DIRECTORY* tls = (IMAGE_TLS_DIRECTORY*)(mapped+tls_off);
        g_tls_index = tls->AddressOfIndex;
        g_tls_zero_count = tls->SizeOfZeroFill;
        byte_t* p = 0;
        unsigned tls_va = tls->StartAddressOfRawData - nth->OptionalHeader.ImageBase;
        for ( int i = 0; i < nth->FileHeader.NumberOfSections; ++i )
            if ( S[i].VirtualAddress <= tls_va && tls_va < S[i].VirtualAddress + S[i].SizeOfRawData )
            {
                p = mapped + (S[i].PointerToRawData+tls_va-S[i].VirtualAddress);
                break;
            }
        if ( p )
            g_tls.Insert(g_tls.End(),p,
                         p+(tls->EndAddressOfRawData-tls->StartAddressOfRawData));
        else
            g_tls.Insert(g_tls.End(),(tls->EndAddressOfRawData-tls->StartAddressOfRawData),0);
        g_tls_callback = 0;
        if ( tls->AddressOfCallBacks )
        {
            tls_va = tls->AddressOfCallBacks - nth->OptionalHeader.ImageBase;
            for ( int i = 0; i < nth->FileHeader.NumberOfSections; ++i )
                if ( S[i].VirtualAddress <= tls_va && tls_va
                     < S[i].VirtualAddress + cxx_max(nth->OptionalHeader.SectionAlignment,cxx_align(S[i].SizeOfRawData,
                                                     nth->OptionalHeader.SectionAlignment)) )
                {
                    p = mapped + (S[i].PointerToRawData+tls_va-S[i].VirtualAddress);
                    break;
                }
            g_tls_callback = *(unsigned*)p;
        }
        //g_tls.Insert(g_tls.End(),8,0);
    }
    else
    {
        g_tls_index = 0;
        g_tls_zero_count = 0;
        g_tls_callback = 0;
        g_tls.Clear();
    }

    if ( flags & PEWRAP_SELFCONTAINS )
    {
        zh = (ZIPPED_HEADER*)(g_zipped_area = 0);
        g_zipped_area_length = 0;
    }
    else
    {
        zh = (ZIPPED_HEADER*)(g_zipped_area = (byte_t*)SysMmapFileByHandle(0,zhl,0,true,ehf));
        zh->file_alignment = nth->OptionalHeader.FileAlignment;
        zh->section_alignment = nth->OptionalHeader.SectionAlignment;
    }
    //nth->OptionalHeader.SectionAlignment = 0x1000;
    nth->OptionalHeader.FileAlignment = 0x1000;
    nth->OptionalHeader.SizeOfHeaders = cxx_align(nth->OptionalHeader.SizeOfHeaders,nth->OptionalHeader.FileAlignment);

    g_rsc.Clear();
    GetRequiredResources(mapped,nth, g_rsc);

    for ( int i = 0; i < 16; ++i )
    {
        if ( zh ) zh->orgdir[i] = nth->OptionalHeader.DataDirectory[i];
        nth->OptionalHeader.DataDirectory[i].VirtualAddress = 0;
        nth->OptionalHeader.DataDirectory[i].Size = 0;
    }

    if ( zh ) cipher.DoCipher(zh->orgdir,16);
    unsigned executable_barier = 0;

    if ( zh )
    {
        zh->number = nth->FileHeader.NumberOfSections;
        for ( int i = 0; i < nth->FileHeader.NumberOfSections; ++i )
        {
            memset(zh->section+i,0,sizeof(zh->section[i]));
            memcpy(zh->section[i].name,S[i].Name,8);
            cipher.DoCipher(zh->section[i].name,1);
            zh->section[i].access = S[i].Characteristics;
            zh->section[i].rva    = S[i].VirtualAddress;
            zh->section[i].vasize = S[i].Misc.VirtualSize;
            zh->section[i].rawsize= S[i].SizeOfRawData;
            zh->section[i].offset = g_zipped_area_length;
            zh->section[i].crc    = Crc32(0,mapped+S[i].PointerToRawData,S[i].SizeOfRawData);

            //if ( S[i].Characteristics & IMAGE_SCN_MEM_EXECUTE )
            //  executable_barier = S[i].VirtualAddress + S[i].SizeOfRawData;

            if ( S[i].SizeOfRawData )
            {
                int l = zlib_compress_l(
                            mapped+S[i].PointerToRawData,
                            S[i].SizeOfRawData,
                            g_zipped_area+g_zipped_area_length,
                            zhl-g_zipped_area_length,
                            9);
                if ( l > 0 && l < S[i].SizeOfRawData )
                {
                    zh->section[i].zipped = 1;
                    zh->section[i].count = l;
                }
                else
                {
                    memcpy(g_zipped_area+g_zipped_area_length,mapped+S[i].PointerToRawData,S[i].SizeOfRawData);
                    zh->section[i].count = S[i].SizeOfRawData;
                }
                cipher.DoCipher(g_zipped_area+zh->section[i].offset,zh->section[i].count/8);
                zh->section[i].xcrypt = 1;
            }

            g_zipped_area_length += zh->section[i].count;
        }
    }

    u32_t start = 0;
    u32_t finish = 0;
    for ( int i = 0; i < nth->FileHeader.NumberOfSections; ++i )
        if ( S[i].PointerToRawData ) { start = S[i].PointerToRawData; break; }
    for ( int i = 0; i < nth->FileHeader.NumberOfSections; ++i )
        if ( S[i].PointerToRawData ) { finish = S[i].PointerToRawData+S[i].SizeOfRawData; }

    *file_length = cxx_align<u32_t>(start,nth->OptionalHeader.FileAlignment);
    memset(mapped+start,0,*file_length-start);
    S->PointerToRawData = *file_length;
    if ( finish > S->PointerToRawData+4*1024 )
        memset(mapped+S->PointerToRawData+4*1024,0,finish-S->PointerToRawData);

    nth->OptionalHeader.SizeOfCode = 0;
    nth->OptionalHeader.BaseOfData = 0;
    nth->OptionalHeader.SizeOfImage = cxx_alignu(cxx_max<u32_t>(nth->OptionalHeader.SizeOfImage,minsize),
                                      nth->OptionalHeader.SectionAlignment);

    memcpy(S->Name,".text\0\0\0",8);
    S->Characteristics = IMAGE_SCN_CNT_CODE|IMAGE_SCN_MEM_EXECUTE|IMAGE_SCN_MEM_READ;//|IMAGE_SCN_MEM_WRITE;
    nth->OptionalHeader.SizeOfInitializedData = 0;
    nth->OptionalHeader.SizeOfUninitializedData = 0;

    unsigned starter_gap = mirage0S_size+POLYMORF_STUFF;
    unsigned cor_space = Core0sLength()+8;
    unsigned required_size = starter_gap + g_zipped_area_length + cor_space + sizeof(STB_INFO);
    executable_barier = S->VirtualAddress+required_size;
    S->Misc.VirtualSize = nth->OptionalHeader.SizeOfImage-S->VirtualAddress;
    if ( required_size > S->Misc.VirtualSize )
    {
        S->SizeOfRawData = required_size;
        //required_size = cxx_alignu(required_size,nth->OptionalHeader.FileAlignment);
        S->Misc.VirtualSize = cxx_alignu(required_size,nth->OptionalHeader.SectionAlignment);
        nth->OptionalHeader.SizeOfImage = S->VirtualAddress+S->Misc.VirtualSize;
    }
    else
    {
        S->SizeOfRawData = S->Misc.VirtualSize;
        S->Misc.VirtualSize = cxx_alignu(S->Misc.VirtualSize,nth->OptionalHeader.SectionAlignment);
    }

    unsigned l = *file_length+starter_gap;
    unsigned zipped_gap = l+cor_space+g_zipped_area_length;
    info = (STB_INFO*)(mapped+zipped_gap+8);
    if ( zh )
    {
        info->zip.offset = l+cor_space;
        info->zip.count = g_zipped_area_length;
    }
    PlaceCore0(mapped,&l,info,ehf);

    if ( zh ) memcpy(mapped+info->zip.offset,g_zipped_area,g_zipped_area_length);

    /*for ( u32_t r = (u32_t)time(0), i = required_size; i < S->Misc.VirtualSize;  )
      {
        enum { USE_BLOCK_SIZE=0x200 };
        unsigned j = S->Misc.VirtualSize-i;
        if ( j > USE_BLOCK_SIZE ) j = USE_BLOCK_SIZE;
        //unsigned o = (teggo::unsigned_random(&r) % (code_bytes_size-USE_BLOCK_SIZE*2))+USE_BLOCK_SIZE;
        //memcpy(mapped+*file_length+i,code_bytes+o,j);
        i += j;
      }*/

    nth->OptionalHeader.BaseOfData = S->VirtualAddress;
    executable_barier -= S->VirtualAddress;
    if ( executable_barier < required_size ) executable_barier = required_size;
    //executable_barier = cxx_alignu(executable_barier,nth->OptionalHeader.FileAlignment);
    for ( int i = 1; i < nth->FileHeader.NumberOfSections; ++i )
        memset(S+i,0,sizeof(IMAGE_SECTION_HEADER));
    nth->FileHeader.NumberOfSections = 1;

    if ( S->SizeOfRawData > /*executable_barier*/ cxx_alignu(executable_barier,nth->OptionalHeader.SectionAlignment) )
    {
        memset(&S[1],0,sizeof(S[1]));
        unsigned e = cxx_alignu(executable_barier,nth->OptionalHeader.SectionAlignment);
        memcpy(S[1].Name,".bss\0\0\0\0",8);
        S->SizeOfRawData = executable_barier;
        S[1].Misc.VirtualSize = S->Misc.VirtualSize - e;
        S->Misc.VirtualSize = e;
        S[1].VirtualAddress = S->VirtualAddress+e;
        nth->OptionalHeader.SizeOfUninitializedData += S[1].Misc.VirtualSize;
        S[1].Characteristics = IMAGE_SCN_CNT_UNINITIALIZED_DATA|IMAGE_SCN_MEM_READ|IMAGE_SCN_MEM_WRITE;
        nth->FileHeader.NumberOfSections += 1;
    }

    nth->OptionalHeader.SizeOfCode += cxx_alignu(S->SizeOfRawData,nth->OptionalHeader.FileAlignment);
    *file_length = S->PointerToRawData+S->SizeOfRawData;
    *file_length = cxx_alignu(*file_length,nth->OptionalHeader.FileAlignment);

    nth->FileHeader.Characteristics =
        IMAGE_FILE_DEBUG_STRIPPED
        |IMAGE_FILE_32BIT_MACHINE
        |IMAGE_FILE_LOCAL_SYMS_STRIPPED
        |IMAGE_FILE_LINE_NUMS_STRIPPED
        |IMAGE_FILE_EXECUTABLE_IMAGE
        //|IMAGE_FILE_RELOCS_STRIPPED
        ;
    nth->FileHeader.PointerToSymbolTable = 0;
    nth->FileHeader.NumberOfSymbols = 0;
    nth->OptionalHeader.CheckSum = 0;

    nth->OptionalHeader.MajorLinkerVersion = _MAJOR_VERSION;
    nth->OptionalHeader.MinorLinkerVersion = _BUILD_NUMBER%100;
    //nth->OptionalHeader.MajorLinkerVersion = 7;
    //nth->OptionalHeader.MinorLinkerVersion = _BUILD_NUMBER%128;

    nth->OptionalHeader.BaseOfData = nth->OptionalHeader.SizeOfImage;
    return true;
}

bool AttachOriginalPe(EhFilter ehf)
{
    DataStreamPtr ds = OpenHandleStream(g_outfile,STREAM_WRITE|STREAM_NONINHERITABLE);
    ds->Seek(0,SEEK_END);
    unsigned offset = ds->Tell();
    ds->Write(g_zipped_area,g_zipped_area_length);
    Unrefe(ds);

    if ( byte_t* mapped = (byte_t*)SysMmapFile(+g_outfile,0,0,true,false,ehf) )
    {
        STB_INFO* info = (STB_INFO*)(mapped+g_stbinfo_offset);
        info->zip.offset = offset;
        info->zip.count = g_zipped_area_length;
        info->zip.crc = Crc32(0,g_zipped_area,g_zipped_area_length);
        SysUnmmapFile(mapped);
    }

    return true;
}

bool RelocateBounded(byte_t* mapped, EhFilter ehf)
{
    IMAGE_NT_HEADERS* nth = NT_HEADERS(mapped);
    IMAGE_SECTION_HEADER* S = IMAGE_FIRST_SECTION(nth);
    unsigned sect_end_offs = ((byte_t*)(S+nth->FileHeader.NumberOfSections+2)-mapped);
    sect_end_offs = (sect_end_offs + 7)&~7;
    IMAGE_DATA_DIRECTORY& d = nth->OptionalHeader.DataDirectory[IMAGE_DIRECTORY_ENTRY_BOUND_IMPORT];
    if ( d.VirtualAddress && sect_end_offs > d.VirtualAddress )
    {
        if ( sect_end_offs + d.Size > S->PointerToRawData )
        { ehf("failed to relocate bound imports table, not enough room"); return false; }
        memmove(mapped+sect_end_offs,mapped+d.VirtualAddress,d.Size);
        byte_t* p = (byte_t*)(S+nth->FileHeader.NumberOfSections);
        memset(p,0,sect_end_offs-(p-mapped));
        Xoln|_S*"(!) bound imports table was relocated from BASE+%xh to BASE+%xh"
        %d.VirtualAddress %sect_end_offs;
        d.VirtualAddress = sect_end_offs;
    }
    return true;
}

bool DeleteBounded(byte_t* mapped, EhFilter ehf)
{
    IMAGE_NT_HEADERS* nth = NT_HEADERS(mapped);
    IMAGE_SECTION_HEADER* S = IMAGE_FIRST_SECTION(nth);
    IMAGE_DATA_DIRECTORY& d = nth->OptionalHeader.DataDirectory[IMAGE_DIRECTORY_ENTRY_BOUND_IMPORT];
    if ( d.VirtualAddress && d.Size )
    {
        memset(mapped+d.VirtualAddress,0,d.Size);
        d.VirtualAddress = 0;
        d.Size = 0;
    }
    return true;
}

void NumerateImports(byte_t* mapped)
{
    IMAGE_NT_HEADERS* nth = NT_HEADERS(mapped);
    IMAGE_SECTION_HEADER* S = IMAGE_FIRST_SECTION(nth);
    IMAGE_SECTION_HEADER* Se = S+nth->FileHeader.NumberOfSections;
    unsigned import_offs = nth->OptionalHeader.DataDirectory[IMAGE_DIRECTORY_ENTRY_IMPORT].VirtualAddress;
    unsigned import_count = nth->OptionalHeader.DataDirectory[IMAGE_DIRECTORY_ENTRY_IMPORT].Size;
    IMAGE_IMPORT_DESCRIPTOR* dS = 0;
    for ( ; S != Se; ++S )
        if ( S->VirtualAddress <= import_offs && S->VirtualAddress + S->SizeOfRawData > import_offs )
            dS = (IMAGE_IMPORT_DESCRIPTOR*)(mapped+S->PointerToRawData+import_offs-S->VirtualAddress);
    for ( int i=0; dS[i].Name; ++i )
    {
        dS[i].TimeDateStamp = i;
    }
}

void UnumerateImports(byte_t* mapped)
{
    IMAGE_NT_HEADERS* nth = NT_HEADERS(mapped);
    IMAGE_SECTION_HEADER* S = IMAGE_FIRST_SECTION(nth);
    IMAGE_SECTION_HEADER* Se = S+nth->FileHeader.NumberOfSections;
    unsigned import_offs = nth->OptionalHeader.DataDirectory[IMAGE_DIRECTORY_ENTRY_IMPORT].VirtualAddress;
    unsigned import_count = nth->OptionalHeader.DataDirectory[IMAGE_DIRECTORY_ENTRY_IMPORT].Size;
    IMAGE_IMPORT_DESCRIPTOR* dS = 0;
    for ( ; S != Se; ++S )
        if ( S->VirtualAddress <= import_offs && S->VirtualAddress + S->SizeOfRawData > import_offs )
            dS = (IMAGE_IMPORT_DESCRIPTOR*)(mapped+S->PointerToRawData+import_offs-S->VirtualAddress);
    if ( dS )
        for ( int i=0; dS[i].Name; ++i )
            dS[i].TimeDateStamp = 0;
}

bool PEoverOpenWfile(pwide_t outfile, unsigned flags, u32_t minsize, u32_t mintls, EhFilter ehf)
{
    byte_t*  mapped = 0;
    unsigned mapped_length = 0;
    unsigned file_length = 0;
    unsigned oep = 0;
    IMAGE_DATA_DIRECTORY cor20 = {0,};

    if ( !(flags&PEWRAP_RELINK) || (flags&PEWRAP_DEMO) ) flags |= PEWRAP_SAVEIAT;
    SysUrandom(g_xcrypt_key,sizeof(g_xcrypt_key));

    bool     retval = false;
    IMAGE_NT_HEADERS* nth;

    if ( !FileExists(outfile) )
    { ehf("file does not exist"); goto e; }

    file_length = LengthOfFile(outfile);
    mapped_length = file_length*2;
    if ( !(mapped = (byte_t*)SysMmapReadonlyFile(outfile,ehf)) )
        goto e;
    nth = NT_HEADERS(mapped);
    if ( mapped_length < nth->OptionalHeader.SizeOfImage )
        mapped_length = nth->OptionalHeader.SizeOfImage;
    SysUnmmapFile(mapped);

    mapped_length += WRAPPING_EXTENSION;
    if ( !(mapped = (byte_t*)SysMmapFile(outfile,mapped_length,0,true,false,ehf)) )
        goto e;
    *(u16_t*)mapped = 0x4d5a;
    g_wfile_flags = flags;
    g_outfile = outfile;
    g_bundles.Clear();


    if ( !(flags&PEWRAP_SAVEIAT) && (flags&PEWRAP_ANTIHACK) && !(flags&PEWRAP_SELFCONTAINS) )
        ReorderImports(mapped,g_squoted,g_xcrypt_key,ehf);

    if ( !DeleteBounded(mapped,ehf) )
        goto e;

    STB_INFO* info = 0;

    if (1)
    {
        nth = NT_HEADERS(mapped);
        nth->OptionalHeader.DataDirectory[IMAGE_DIRECTORY_ENTRY_SECURITY].VirtualAddress = 0;
        nth->OptionalHeader.DataDirectory[IMAGE_DIRECTORY_ENTRY_SECURITY].Size = 0;
        nth->OptionalHeader.DataDirectory[IMAGE_DIRECTORY_ENTRY_BOUND_IMPORT].VirtualAddress = 0;
        nth->OptionalHeader.DataDirectory[IMAGE_DIRECTORY_ENTRY_BOUND_IMPORT].Size = 0;
        nth->OptionalHeader.CheckSum = 0;

        if ( flags & PEWRAP_RELINK )
        {
            if ( !Relink(mapped,&file_length,flags,info,minsize,mintls,ehf) )
                goto e;
        }
    }

    nth = NT_HEADERS(mapped);

    if ( !(flags&PEWRAP_RELINK) )
    {
        NumerateImports(mapped);
        if ( flags & PEWRAP_DOTNET )
        {
            cor20 = nth->OptionalHeader.DataDirectory[IMAGE_DIRECTORY_ENTRY_COM_DESCRIPTOR];
            nth->OptionalHeader.DataDirectory[IMAGE_DIRECTORY_ENTRY_COM_DESCRIPTOR].VirtualAddress = 0;
            nth->OptionalHeader.DataDirectory[IMAGE_DIRECTORY_ENTRY_COM_DESCRIPTOR].Size = 0;
        }
    }

    nth->OptionalHeader.DllCharacteristics &= ~(0x0040|0x0100|0x0080);
    oep = nth->OptionalHeader.AddressOfEntryPoint;
    nth->OptionalHeader.MajorOperatingSystemVersion = 5;
    nth->OptionalHeader.MinorOperatingSystemVersion = 0;
    nth->OptionalHeader.MajorSubsystemVersion = 5;
    nth->OptionalHeader.MinorSubsystemVersion = 0;

    if ( 1 )
        if ( !PlaceChain0(mapped,&file_length,info,minsize,mintls,ehf) )
        { ehf("failed to write starter code"); goto e; }

    if ( !info )
    {
        g_stbinfo_offset = file_length+8;
        g_stbinfo_max    = ((g_stbinfo_offset + sizeof(STB_INFO2))&~4095) + 4095;
        info = (STB_INFO*)(mapped+g_stbinfo_offset);
        file_length += sizeof(STB_INFO2)+8;
    }
    else
        g_stbinfo_offset = (byte_t*)info - mapped;

    *(unsigned*)((IMAGE_DOS_HEADER*)mapped)->e_res = g_stbinfo_offset-8;
    *(unsigned*)(mapped+g_stbinfo_offset-8) = STB_SIGN_FIRST;
    *(unsigned*)(mapped+g_stbinfo_offset-4) = STB_SIGN_SECOND;
    info->cor20.VirtualAddress = cor20.VirtualAddress;
    info->cor20.Size = cor20.Size;
    info->oep = oep;
    info->hwid = QuerySystemHWID();
    info->flags |= STBF_EXTENDED;

    if ( flags & PEWRAP_RELINK )   info->flags |= STBF_EXE_ZIPPED;
    if ( flags & PEWRAP_LOGGING )  info->flags |= TEGGOVBOX_LOG_ACTIVITY;
    if ( flags & PEWRAP_DBGLOG )   info->flags |= TEGGOVBOX_LOG_ACTIVITY_DBG;
    if ( flags & PEWRAP_HASPACKS ) info->flags |= TEGGOVBOX_SEARCH_FOR_EMBEDDINGS;
    if ( flags & PEWRAP_DOTNET )   info->flags |= STBF_EXE_DOTNET;
    if ( flags & PEWRAP_DEMO )     info->flags |= STBF_EXE_DEMO;
    if ( flags & PEWRAP_HWID )     info->flags |= STBF_BIND2HWID;
    if ( flags & PEWRAP_ANTIHACK ) info->flags |= STBF_EXE_ANTIHACK;
    if ( flags & PEWRAP_SAVEIAT )  info->flags |= STBF_SAVE_IAT;
    if ( flags & PEWRAP_STRONGANTIHACK ) info->flags |= STBF_EXE_ARMODIL;
    info->flags |= TEGGOVBOX_VIRTUALIZE_FS;
    if ( flags & PEWRAP_HANDLERGS ) info->flags |= TEGGOVBOX_VIRTUALIZE_REGISTRY;
    if ( flags & PEWRAP_BOXEMBEDED ) info->flags |= TEGGOVBOX_SEARCH_FOR_EMBEDDINGS;
    if ( flags & PEWRAP_NOEXEFILT ) info->flags |= STBF_NOEXEFILT;
    if ( flags & PEWRAP_SELFCONTAINS ) info->flags |= STBF_EMBEDDED_LOADING;
    info->originals_offs = file_length;
    *(unsigned*)(mapped+file_length) = 0; // no restorable bytes (use in protection ???)
    file_length += 4;
    info->tls_index = g_tls_index;
    info->tls_callback = g_tls_callback;
    memcpy(&info->cor20,&cor20,8);
    if ( !(g_wfile_flags&PEWRAP_RELINK) )
        PlaceCore0(mapped,&file_length,info,ehf);
    SysUnmmapFile(mapped); mapped = 0;
    SysTruncateFile(outfile,file_length);
    retval = true;

e:
    if (mapped) SysUnmmapFile(mapped);
    if (!retval && g_zipped_area) { SysUnmmapFile(g_zipped_area); g_zipped_area = 0; }
    if (!retval && g_bundles_area) { SysUnmmapFile(g_bundles_area); g_bundles_area = 0; }
    return retval;
}

bool PEoverCloseWfile(EhFilter ehf)
{
    bool retval = false;

    if ( !AttachBundles(ehf) )
        goto e;

    if ( !AttachSquoted(ehf) )
        goto e;

    if ( !(g_wfile_flags&PEWRAP_RELINK) && !AttachOriginalPe(ehf) )
        goto e;

    if ( byte_t* mapped = (byte_t*)SysMmapFile(g_outfile,g_stbinfo_max,0,true,false,ehf) )
    {
        UnumerateImports(mapped);
        STB_INFO* info = (STB_INFO*)(mapped+g_stbinfo_offset);
        NEWDES_Cipher cipher;
        cipher.SetupEncipher(g_xcrypt_key);
        cipher.DoCipher(info,sizeof(*info)/8);
        memcpy(info->xcrypt_key,g_xcrypt_key,16);
        SysUnmmapFile(mapped);
        retval = true;
    }
e:
    if (g_zipped_area) { SysUnmmapFile(g_zipped_area); g_zipped_area = 0; }
    if (g_bundles_area) { SysUnmmapFile(g_bundles_area); g_bundles_area = 0; }
    return retval;
}

bool ModifyImports(StringA const& module,BUNDLED_IMPORT& bi,EhFilter ehf)
{
    bool modified = false;
    if ( byte_t* mapped = (byte_t*)SysMmapFile(g_outfile,0,0,true,false,ehf) )
    {
        IMAGE_NT_HEADERS* nth = NT_HEADERS(mapped);
        IMAGE_SECTION_HEADER* S = IMAGE_FIRST_SECTION(nth);
        IMAGE_SECTION_HEADER* Se = S+nth->FileHeader.NumberOfSections;
        unsigned import_offs = nth->OptionalHeader.DataDirectory[IMAGE_DIRECTORY_ENTRY_IMPORT].VirtualAddress;
        unsigned import_count = nth->OptionalHeader.DataDirectory[IMAGE_DIRECTORY_ENTRY_IMPORT].Size;
        IMAGE_IMPORT_DESCRIPTOR* dS = 0;
        for ( ; S != Se; ++S )
            if ( S->VirtualAddress <= import_offs && S->VirtualAddress + S->SizeOfRawData > import_offs )
                dS = (IMAGE_IMPORT_DESCRIPTOR*)(mapped+S->PointerToRawData+import_offs-S->VirtualAddress);
        for ( IMAGE_IMPORT_DESCRIPTOR* d = dS; d->Name; )
        {
            pchar_t dn = (pchar_t)(mapped+RVAtoOffs(mapped,d->Name));
            if ( module.EqualI(dn) )
            {
                bi.iidName  = d->Name;
                bi.iidOfth  = d->OriginalFirstThunk;
                bi.iidFth   = d->FirstThunk;
                bi.no       = d->TimeDateStamp;
                IMAGE_IMPORT_DESCRIPTOR dd = *d;
                IMAGE_IMPORT_DESCRIPTOR* dE = d;
                while ( dE->Name ) ++dE;
                if ( d != dE-1 )
                {
                    *d = dE[-1];
                    dE[-1] = dd;
                }
                --dE;
                bi.iidOffs = dE-dS;
                dE->Name = 0;
                dE->OriginalFirstThunk = 0;
                dE->FirstThunk = 0;
                //nth->OptionalHeader.DataDirectory[IMAGE_DIRECTORY_ENTRY_IMPORT].Size = dE-dS;
                modified = true;
                break;
            }
            else
                ++d;
        }
        SysUnmmapFile(mapped);
    }
    return modified;
}

bool PEoverAttachActivator(pwide_t activator,EhFilter ehf)
{
    StringA module = GetBasenameOfPath(activator);
    int l = StrLen(activator);
    STB_INFO* info = 0;
    if ( !(g_wfile_flags & PEWRAP_EMBEDDING) && l >= sizeof(info->act_file_name) )
        return ehf("activator path is too long"),false;
    byte_t*  mapped = 0;
    if ( g_wfile_flags & PEWRAP_EMBEDDING )
    {
        unsigned flags = 0;
        unsigned rel_length = LengthOfFile(activator);
        unsigned crc = 0;
        unsigned offset = 0;
        int pak_length;

        DataStreamPtr ds = OpenHandleStream(activator,STREAM_READ|STREAM_NONINHERITABLE);
        if ( !rel_length || !ds )
        { ehf(_S*"failed to open activator '%s'"%activator); return false; }

        BufferT<byte_t> data(rel_length);
        BufferT<byte_t> cdata(rel_length+4);
        ds->Read(+data,data.Count());
        Unrefe(ds);

        if ( !(g_wfile_flags&PEWRAP_DEMO) && (g_wfile_flags&PEWRAP_ANTIHACK) )
            ReorderImports(+data,g_squoted_act,g_xcrypt_key,ehf);

        if ( (pak_length = zlib_compress(+data,rel_length,+cdata+4,rel_length) ) > 0 )
        {
            cdata.Swap(data);
            flags |= STBF_ACT_ZIPPED;
            pak_length += 4;
            *(unsigned*)+data = rel_length;
        }
        else
            pak_length = rel_length;

        flags |= STBF_ACT_XCRYPED;
        NEWDES_Cipher cipher;
        cipher.SetupEncipher(g_xcrypt_key);
        cipher.DoCipher(+data,pak_length/8);
        crc = Crc32(0,+data,pak_length);

        ds = DataSource->Open(g_outfile,STREAM_WRITE);
        ds->Seek(0,SEEK_END);
        offset = ds->Tell();

        ds->Write(+data,pak_length);
        Unrefe(ds);

        if ( !(mapped = (byte_t*)SysMmapFile(g_outfile,g_stbinfo_max,0,true,false,ehf)) )
            return false;
        info = (STB_INFO*)(mapped+g_stbinfo_offset);
        info->flags |= flags;
        info->act.offset = offset;
        info->act.count = pak_length;
        info->act.crc = crc;
        SysUnmmapFile(mapped);
        Xoln| _S*"  + %-16s [%3d%%] %s" % module %(pak_length*100/rel_length) %activator;
    }
    else
    {
        if ( !(mapped = (byte_t*)SysMmapFile(+g_outfile,g_stbinfo_max,0,true,false,ehf)) )
            return false;
        info = (STB_INFO*)(mapped+g_stbinfo_offset);
        info->flags |= STBF_EXTERNAL_ACT;
        memcpy(info->act_file_name,+StringA(activator),l+1);
        SysUnmmapFile(mapped);
    }
    if ( g_wfile_flags & PEWRAP_EMBEDDING )
    {
        BUNDLED_IMPORT bi = {0};
        u32_t r = GetMillis();
        for ( int i = 0; i < sizeof(bi.name)/4; ++i )
            *(unsigned*)(bi.name+i*4) = teggo::unsigned_random(&r);
        bi.name[sizeof(bi.name)-1] = 0;
        strncpy(bi.name,+module,sizeof(bi.name)-1);
        bi.actfake = 1;
        bi.activator = 1;
        if ( g_wfile_flags & PEWRAP_RELINK )
            g_bundles.Push(bi);
        else
            while ( ModifyImports(module,bi,ehf) )
                g_bundles.Push(bi);
    }
    return true;
}

bool PEoverBundleDLL(pwide_t dllname, bool hideonly, EhFilter ehf)
{
    StringA module = GetBasenameOfPath(dllname);
    BUNDLED_IMPORT bi = {0};

    if ( hideonly )
    {
        bi.actfake = 1;
        if ( !(g_wfile_flags & PEWRAP_RELINK) )
            if ( ModifyImports(module,bi,ehf) )
            {
                g_bundles.Push(bi);
                Xoln| _S*"  - %-16s [****] %s" %module %TrimLength(dllname,50);
                while (ModifyImports(module,bi,ehf))
                    g_bundles.Push(bi);
            }
    }
    else
    {
        unsigned rel_length = LengthOfFile(dllname);
        unsigned crc = 0;
        unsigned offset = 0;
        int pak_length;
        BufferT<byte_t> data(rel_length);
        BufferT<byte_t> cdata(rel_length+4);

        DataStreamPtr ds = OpenHandleStream(dllname,STREAM_READ|STREAM_NONINHERITABLE);
        if ( !ds )
        { ehf(_S*"failed to open module '%s'"%dllname); return false; }

        ds->Read(+data,data.Count());
        Unrefe(ds);

        if ( (pak_length = zlib_compress(+data,rel_length,+cdata+4,rel_length) ) > 0 )
        {
            cdata.Swap(data);
            bi.zipped = 1;
            pak_length += 4;
            *(unsigned*)+data = rel_length;
        }
        else
            pak_length = rel_length;

        bi.xcrypt = 1;
        NEWDES_Cipher cipher;
        cipher.SetupEncipher(g_xcrypt_key);
        cipher.DoCipher(+data,pak_length/8);
        bi.crc = Crc32(0,+data,pak_length);
        ds = OpenHandleStream(g_outfile,STREAM_WRITE|STREAM_NONINHERITABLE);
        ds->Seek(0,SEEK_END);
        bi.offset = ds->Tell();
        ds->Write(+data,pak_length);
        Unrefe(ds);

        {
            u32_t r = GetMillis();
            for ( int i = 0; i < sizeof(bi.name)/4; ++i )
                *(unsigned*)(bi.name+i*4) = teggo::unsigned_random(&r);
            bi.name[sizeof(bi.name)-1] = 0;
            strncpy(bi.name,+module,sizeof(bi.name)-1);
        }

        bi.count = pak_length;
        bi.size  = rel_length;

        //if ( g_wfile_flags & PEWRAP_DLLADDLST )
        //  bi.add2lst = 1;

        if ( !(g_wfile_flags & PEWRAP_RELINK) )
            while ( ModifyImports(module,bi,ehf) )
                g_bundles.Push(bi);
        else
            g_bundles.Push(bi);

        Xoln| _S*"  + %-16s [%3d%%] %s" % module %(pak_length*100/rel_length) %TrimLength(dllname,50);
    }
    return true;
}

bool ListImports(StringParam file, CollectionT<ArrayT<StringW>>& dlls, ArrayT<StringW>& l, EhFilter ehf)
{
    if ( byte_t* mapped = (byte_t*)SysMmapFile(+file,0,0,false,false,EMPTYEF) )
    {
        IMAGE_NT_HEADERS* nth = NT_HEADERS(mapped);
        IMAGE_SECTION_HEADER* S = IMAGE_FIRST_SECTION(nth);
        IMAGE_SECTION_HEADER* Se = S+nth->FileHeader.NumberOfSections;
        unsigned import_offs = nth->OptionalHeader.DataDirectory[IMAGE_DIRECTORY_ENTRY_IMPORT].VirtualAddress;
        unsigned import_count = nth->OptionalHeader.DataDirectory[IMAGE_DIRECTORY_ENTRY_IMPORT].Size;
        IMAGE_IMPORT_DESCRIPTOR* d = 0;
        for ( ; S != Se; ++S )
            if ( S->VirtualAddress <= import_offs && S->VirtualAddress + S->SizeOfRawData > import_offs )
                d = (IMAGE_IMPORT_DESCRIPTOR*)(mapped+S->PointerToRawData+import_offs-S->VirtualAddress);
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

bool IsSystemDLL(pwide_t path)
{
    wchar_t buffer[512];
    if ( GetWindowsDirectoryW(buffer,sizeof(buffer)/2) )
        return teggo::StrLower(path).StartWith(+teggo::StrLower(buffer));
    return false;
}

bool SearchFullPath(pwide_t name, StringW& fullpath)
{
    wchar_t buffer[512] = {0};
    wchar_t* foo;
    if ( int l = SearchPathW(0,name,0,sizeof(buffer)/2,buffer,&foo) )
    {
        fullpath.SetRange(buffer,l);
        fullpath.ToLower();
        return true;
    }
    return false;
}

bool PEoverFilterBounds(pwide_t pefile, ArrayT<StringW>& a, bool autobound, EhFilter ehf, bool silance)
{
    CollectionT<ArrayT<StringW>> dlls;
    ArrayT<StringW> l;
    ListImports(pefile,dlls,l,ehf);

    CollectionT<unsigned> bundles;
    StringW fullpath;

    for ( int i=0; i < a.Count(); ++i )
    {
        bool f = false;
        if ( ArrayT<StringW>* p = dlls.GetLower(+a[i]) )
        {
            if ( SearchFullPath(dlls.KeyOf(p),fullpath) )
                bundles[+fullpath] = 1, f = true;
        }
        else if ( ArrayT<StringW>* p = dlls.GetLower(+(a[i]+L".dll")) )
        {
            if ( SearchFullPath(dlls.KeyOf(p),fullpath) )
                bundles[+fullpath] = 1, f = true;
        }

        if ( !f && !silance )
            Xoln| _S*"(!) bundling request for '%s' was rejected" % a[i];
    }

    if ( autobound )
        for ( CollectionIteratorT<ArrayT<StringW>> i = dlls.Iterate(); i.Next(); )
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
    if ( byte_t* mapped = (byte_t*)SysMmapReadonlyFile(file,ehf) )
    {
        IMAGE_NT_HEADERS* nth = NT_HEADERS(mapped);
        IMAGE_SECTION_HEADER* S = IMAGE_FIRST_SECTION(nth);
        IMAGE_SECTION_HEADER* Se = S+nth->FileHeader.NumberOfSections;
        if ( unsigned cor20_offs = nth->OptionalHeader.DataDirectory[IMAGE_DIRECTORY_ENTRY_COM_DESCRIPTOR].VirtualAddress )
            if ( IMAGE_COR20_HEADER* cor20 = (IMAGE_COR20_HEADER*)(mapped+RVAtoOffs(mapped,cor20_offs)) )
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

bool PeFixCRC(pwide_t filename, EhFilter ehf)
{
    bool succeeded = false;
    unsigned mappedL = 0;
    if ( DataStreamPtr ds = OpenStream(filename,STREAM_READ|STREAM_WRITE) )
    {
        BufferT<byte_t> b(1024*4);
        ds->Seek(0);
        ds->WriteByte(0x4d);
        ds->WriteByte(0x5a);
        ds->Seek(0);
        ds->Read(+b,1024*4);
        IMAGE_NT_HEADERS* nth = NT_HEADERS(+b);
        IMAGE_SECTION_HEADER* S = IMAGE_FIRST_SECTION(nth);
        IMAGE_SECTION_HEADER* Se = S+nth->FileHeader.NumberOfSections - 1;
        while ( !Se->PointerToRawData || !Se->SizeOfRawData ) --Se;
        mappedL = cxx_alignu(Se->PointerToRawData+Se->SizeOfRawData,nth->OptionalHeader.FileAlignment);
    }

    succeeded = true;

    /*
     if ( HMODULE imghlp = LoadLibraryA("imagehlp.dll") )
       {
         if ( byte_t *mapped = (byte_t*)SysMmapFile(filename, mappedL, 0, true, false, ehf) )
           {
             *(u16_t*)mapped = 0x5a4d;
             PIMAGE_NT_HEADERS (__stdcall *fCheckSumMappedFile)(void *,DWORD,DWORD*,DWORD*) = 0;
             unsigned long foo,crc;
             *(void**)&fCheckSumMappedFile = GetProcAddress(imghlp,"CheckSumMappedFile");
             PIMAGE_NT_HEADERS _h = fCheckSumMappedFile(mapped,mappedL,&foo,&crc);
             _h->OptionalHeader.CheckSum = crc;
             succeeded = true;
             SysUnmmapFile(mapped);

             /// insinity chack
             //unsigned long crc2;
             //mapped = (byte_t *)SysMmapReadonlyFile(filename,ehf);
             //fCheckSumMappedFile(mapped,mappedL,&foo,&crc2);
             //printf("%08x,%08x\n",crc,crc2);
             //REQUIRE( _h->OptionalHeader.CheckSum == crc );
             //SysUnmmapFile(mapped);
           }
         FreeLibrary(imghlp);
       }
     else
       {
         ehf("failed to load 'imagehlp.dll' dynamic library");
       }
       */
    return succeeded;
}

bool PeCopyFile(StringParam infile,StringParam outfile,bool preserve_extradata,EhFilter ehf)
{
    bool result = false;
    if ( !DataSource->Exists(infile) )
    { ehf("file does not exist"); return false; }
    u32_t file_length = DataSource->Length(infile);
    if ( byte_t* from = (byte_t*)SysMmapReadonlyFile(infile,ehf) )
    {
        unsigned length = file_length;
        if ( *(u16_t*)from == 0x5a4d && !preserve_extradata )
        {
            IMAGE_NT_HEADERS* nth = NT_HEADERS(from);
            IMAGE_SECTION_HEADER* S = IMAGE_FIRST_SECTION(nth);
            IMAGE_SECTION_HEADER* Se = S+nth->FileHeader.NumberOfSections;
            u32_t a = (Se[-1].SizeOfRawData + nth->OptionalHeader.FileAlignment-1)&~(nth->OptionalHeader.FileAlignment-1);
            length = Se[-1].PointerToRawData + a;
        }
        if ( byte_t* to = (byte_t*)SysMmapFile(outfile, length, 0, true, true, ehf) )
        {
            memcpy(to,from,length);
            result = true;
            SysUnmmapFile(to);
        }
        SysUnmmapFile(from);
    }
    return result;
}

bool PeCopyExtraData(StringParam infile,StringParam outfile,bool encrypt,EhFilter ehf)
{
    bool result = false;
    if ( !DataSource->Exists(infile) )
    { ehf("file does not exist"); return false; }
    if ( !DataSource->Exists(outfile) )
    { ehf("outfile does not exist"); return false; }
    u32_t in_file_length = LengthOfFile(infile);
    u32_t out_file_length = LengthOfFile(outfile);
    //Xoln|_S*"%s:%d,%s:%d" %infile%in_file_length%outfile%out_file_length;
    if ( byte_t* from = (byte_t*)SysMmapFileRnCoW(infile,ehf) )
    {
        if ( *(u16_t*)from == 0x5a4d )
        {
            IMAGE_NT_HEADERS* nth = NT_HEADERS(from);
            IMAGE_SECTION_HEADER* S = IMAGE_FIRST_SECTION(nth);
            IMAGE_SECTION_HEADER* Se = S+nth->FileHeader.NumberOfSections;
            u32_t a = (Se[-1].SizeOfRawData + nth->OptionalHeader.FileAlignment-1)&~(nth->OptionalHeader.FileAlignment-1);
            u32_t length = Se[-1].PointerToRawData + a;
            out_file_length = (out_file_length + nth->OptionalHeader.FileAlignment-1)&~(nth->OptionalHeader.FileAlignment-1);
            u32_t exdata_len = in_file_length-length;
            if ( !exdata_len )
                result = true;
            else if ( DataStreamPtr ds = OpenStream(outfile,STREAM_READ|STREAM_WRITE) )
            {
                if ( encrypt )
                {
                    ds->Seek( 0 );
                    IMAGE_DOS_HEADER dh = {0};
                    ds->Read(&dh,sizeof(IMAGE_DOS_HEADER));
                    ds->Seek(*(unsigned*)dh.e_res+8);
                    STB_INFO info;
                    ds->Read(&info,sizeof(info));
                    NEWDES_Cipher cipher;
                    u32_t count = exdata_len &~7;
                    cipher.SetupEncipher(info.xcrypt_key);
                    cipher.DoCipher(from+length,count/8);
                    cipher.SetupDecipher(info.xcrypt_key);
                    cipher.DoCipher(&info,(sizeof(info)-256)/8);
                    info.extra_offs = out_file_length;
                    info.extra_count = count;
                    cipher.SetupEncipher(info.xcrypt_key);
                    cipher.DoCipher(&info,(sizeof(info)-256)/8);
                    ds->Seek(*(unsigned*)dh.e_res+8);
                    ds->Write(&info,sizeof(info));
                    Xoln|_S*"encrypted %d bytes of extra data"%count;
                }
                ds->Seek( out_file_length );
                ds->Write( from+length, exdata_len );
                Xoln|_S*"added %d bytes of extra data"%exdata_len;
                result = true;
            }
            else
                ehf("failed to reopen target file");
            /*
             if ( byte_t *to = (byte_t*)SysMmapFile(outfile, out_file_length + exdata_len, 0, true, false, ehf) )
               {
                 memcpy(to+out_file_length,from+length,exdata_len);
                 u32_t count = exdata_len &~7;
                 if ( encrypt )
                   {
                     STB_INFO *info = (STB_INFO*)(to+ (*(unsigned*)((IMAGE_DOS_HEADER*)to)->e_res + 8));
                     Xoln|_S*"encrypted %d bytes of extra data"%count;
                     NEWDES_Cipher cipher;
                     cipher.SetupEncipher(info->xcrypt_key);
                     cipher.DoCipher(to+out_file_length,count/8);
                     cipher.SetupDecipher(info->xcrypt_key);
                     cipher.DoCipher(info,(sizeof(*info)-256)/8);
                     info->extra_offs = out_file_length;
                     info->extra_count = count;
                     cipher.SetupEncipher(info->xcrypt_key);
                     cipher.DoCipher(info,(sizeof(*info)-256)/8);
                   }
                 SysUnmmapFile(to);
                 Xoln|_S*"added %d bytes of extra data"%exdata_len;
                 result = true;
               }
            */
        }
        SysUnmmapFile(from);
    }
    else
        ehf("failed to map source");
    return result;
}

bool ReadImageNtHeaders(DataStreamPtr ds,IMAGE_NT_HEADERS* h)
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


bool PeGetParams(DataStreamPtr ds, u32_t* size, u32_t* tls, u32_t* base)
{
    IMAGE_NT_HEADERS nth;
    if ( ReadImageNtHeaders(ds,&nth)
         && (nth.FileHeader.Characteristics&(IMAGE_FILE_DLL|IMAGE_FILE_EXECUTABLE_IMAGE)) == IMAGE_FILE_EXECUTABLE_IMAGE )
    {
        *size = nth.OptionalHeader.SizeOfImage;
        *tls  = 0;
        *base = nth.OptionalHeader.ImageBase;
        return true;
    }
    return false;
}

bool PEoverSetCmdline(pwide_t,EhFilter)
{
    return true;
}

bool PEoverPredefineReg(pwide_t,EhFilter)
{
    return true;
}

bool PEoverPredefineEnviron(pwide_t,EhFilter)
{
    return true;
}


/*****/
/*
struct OldFileContainer : FileContainer
  {
    StringW target;
    bool Open(StringParam original,StringParam target, unsigned flags, unsigned minSize, unsigned minTls, EhFilter ehf)
      {
        target = target;
        if ( !PeCopyFile(original,target,false,ehf) ) { ehf("failed to copy executable"); return false; }
        return PEoverOpenWfile(target, flags, minSize, minTls, ehf);
      }
    bool AttachActivator(StringParam activator, EhFilter ehf)
      {
        return PEoverAttachActivator(+activator,ehf);
      }
    bool SetCmdline(StringParam,EhFilter) { return false; }
    bool PredefineReg(StringParam,EhFilter) { return false; }
    bool PredefineEnviron(StringParam,EhFilter) { return false; }
    bool BundleDLL(StringParam dllname, bool hideonly, EhFilter ehf)
      {
        return PEoverBundleDLL(+dllname,hideonly,ehf);
      }
    bool Finalize(EhFilter ehf)
     {
       return PEoverCloseWfile(ehf);
     }
    bool Close(EhFilter ehf)
     {
       bool PeFixCRC(target, ehf);
     }
  };

FileContainerPtr OpenOldFileContainer(StringParam original,StringParam target, unsigned flags, unsigned minSize, unsigned minTls, EhFilter ehf)
  {
    rcc_ptr<OldFileContainer> sfc = RccPtr(new OldFileContainer());
    if ( sfc->Open(original,target,flags,minSize,minTls,ehf) )
      return RccPtr((FileContainer*)Forget(sfc));
    return FileContainerPtr(0);
  }
*/