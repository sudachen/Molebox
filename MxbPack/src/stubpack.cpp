
/*

Copyright (c) 2008-2009, Alexey Sudachen, alexey@sudachen.name

*/

#include "myafx.h"
#include <time.h>
#include "stbinfo.h"
#include "teggovbox.h"
#include "peover.h"
#include <rsa.h>
#include <sys/types.h>
#include <sys/stat.h>

//extern "C" unsigned char core0S_demo[];
//extern "C" unsigned core0S_demo_size;
extern "C" unsigned char core0S[];
extern "C" unsigned core0S_size;

struct StubbedFileContainer : FileContainer
{
    bool Open(StringParam original,StringParam target, StringParam stub, unsigned flags, unsigned minSize, unsigned minTls,
              EhFilter ehf);
    bool AttachActivator(StringParam activator, EhFilter ehf);
    bool SetCmdline(StringParam,EhFilter);
    bool SetRegmask(StringParam,EhFilter);
    bool PredefineReg(StringParam,EhFilter);
    bool PredefineEnviron(StringParam,EhFilter);
    bool BundleDLL(StringParam dllname, bool hideonly, EhFilter ehf);
    bool Close(EhFilter);
    bool Finalize(EhFilter);
    STB_INFO2* StbInfo2() { return info2; }

    IMAGE_NT_HEADERS orignHdrs;
    IMAGE_NT_HEADERS stubHdrs;

    StringW target;
    unsigned wrapflags;
    unsigned file_length;

    enum { WRAPPING_EXTENSION = 1*1024*1024 };
    STB_INFO2* info2;
    STB_INFO*  info;
    unsigned   info_offs;
    unsigned   info_max;
    STB_INFO2  info_dat;

    BufferT<BUNDLED_IMPORT> bundles;
    BufferT<SQUOTED_IMPORT> squoted;
    BufferT<SQUOTED_IMPORT> squoted_act;

    //unsigned Core0sLength() { return wrapflags&PEWRAP_DEMO?core0S_demo_size:core0S_size; }
    //byte_t const *Core0S() { return wrapflags&PEWRAP_DEMO?core0S_demo:core0S; }
    unsigned Core0sLength() { return core0S_size; }
    byte_t const* Core0S() { return core0S; }

    //unsigned Core0sLength() { return core0S_size; }
    //byte_t const *Core0S() { return core0S; }

    bool AddTlsSection(byte_t* mapped, byte_t* orign, unsigned minTls, EhFilter ehf);
    bool Relocate(byte_t* mapped, byte_t* orign, EhFilter ehf);
    bool ExtendBss(byte_t* mapped, byte_t* orign, unsigned minSize, EhFilter ehf);
    bool AddRscSection(byte_t* mapped, byte_t* orign, EhFilter ehf);
    bool RecalculateSizes(byte_t* mapped, EhFilter ehf);
    bool AddOrignSections(byte_t* mapped, byte_t* orign, EhFilter ehf);
    bool AddCore(byte_t* mapped, EhFilter ehf);
    bool CopyExtraData(StringParam infile, bool encrypt, EhFilter ehf);
};

FileContainerPtr OpenStubbedFileContainer(StringParam original,StringParam target, StringParam stub, unsigned flags,
        unsigned minSize, unsigned minTls, EhFilter ehf)
{
    rcc_ptr<StubbedFileContainer> sfc = RccPtr(new StubbedFileContainer());
    if ( sfc->Open(original,target,stub,flags,minSize,minTls,ehf) )
        return RccPtr((FileContainer*)Forget(sfc));
    return FileContainerPtr(0);
}

bool StubbedFileContainer::AddTlsSection(byte_t* mapped, byte_t* orign, unsigned minTls, EhFilter ehf)
{
    IMAGE_NT_HEADERS* nth = NT_HEADERS(mapped);
    IMAGE_NT_HEADERS* oNth = NT_HEADERS(orign);
    unsigned tls_offs = 0;
    unsigned oTls_index = 0;
    unsigned oTls_zCnt = 0;
    byte_t*   oTls_data = 0;
    unsigned oTls_dataCnt = 0;

    IMAGE_SECTION_HEADER* oS = IMAGE_FIRST_SECTION(oNth);
    IMAGE_SECTION_HEADER* oSe = oS+oNth->FileHeader.NumberOfSections;
    IMAGE_SECTION_HEADER* S  = IMAGE_FIRST_SECTION(nth);
    IMAGE_SECTION_HEADER* Se = S+nth->FileHeader.NumberOfSections;
    IMAGE_SECTION_HEADER* Stls = Se-1;

    if ( unsigned va = oNth->OptionalHeader.DataDirectory[IMAGE_DIRECTORY_ENTRY_TLS].VirtualAddress )
    {
        unsigned oTls_offs = 0;
        unsigned tls_offs = 0;
        for ( int i = 0; i < oNth->FileHeader.NumberOfSections; ++i )
            if ( oS[i].VirtualAddress <= va
                 && va < oS[i].VirtualAddress +
                 cxx_max(oNth->OptionalHeader.SectionAlignment,
                         cxx_align(oS[i].SizeOfRawData,oNth->OptionalHeader.SectionAlignment)) )
            {
                oTls_offs = oS[i].PointerToRawData+va-oS[i].VirtualAddress;
                break;
            }

        unsigned xva = nth->OptionalHeader.DataDirectory[IMAGE_DIRECTORY_ENTRY_TLS].VirtualAddress;
        for ( int i = 0; i < nth->FileHeader.NumberOfSections; ++i )
            if ( S[i].VirtualAddress <= xva
                 && xva < S[i].VirtualAddress +
                 cxx_max(nth->OptionalHeader.SectionAlignment,
                         cxx_align(S[i].SizeOfRawData,nth->OptionalHeader.SectionAlignment)) )
            {
                tls_offs = S[i].PointerToRawData+xva-S[i].VirtualAddress;
                break;
            }

        IMAGE_TLS_DIRECTORY* oTls = (IMAGE_TLS_DIRECTORY*)(orign+oTls_offs);
        IMAGE_TLS_DIRECTORY* tls = (IMAGE_TLS_DIRECTORY*)(mapped+tls_offs);
        info->tls_index      = oTls->AddressOfIndex;
        info->tls_callback   = oTls->AddressOfCallBacks;

        if ( memcmp(Stls->Name,".tls\0\0\0",8) )
        { ehf(_S*"last section of container is not .tls (%s)"%StringA((char*)Stls->Name,8)); return false; }

        unsigned oTlsSize = oTls->EndAddressOfRawData-oTls->StartAddressOfRawData+4;
        if ( oTlsSize > minTls ) minTls = oTlsSize;

        tls->StartAddressOfRawData = Stls->VirtualAddress + nth->OptionalHeader.ImageBase;
        tls->EndAddressOfRawData = tls->StartAddressOfRawData + cxx_alignu(minTls,4);
        tls->AddressOfCallBacks = tls->EndAddressOfRawData;

        if ( tls->StartAddressOfRawData != nth->OptionalHeader.ImageBase+Stls->VirtualAddress )
        {
            ehf(_S*".tls section has is not aligned with tls start address (%08x != %08x)"
                %tls->StartAddressOfRawData %(nth->OptionalHeader.ImageBase+Stls->VirtualAddress));
            return false;
        }

        Stls->Misc.VirtualSize = minTls;
        if ( Stls->SizeOfRawData < minTls )
            Stls->SizeOfRawData = cxx_alignu(minTls,nth->OptionalHeader.FileAlignment);
        memset(mapped+Stls->PointerToRawData,0,Stls->SizeOfRawData);
        file_length = Stls->PointerToRawData+Stls->SizeOfRawData;
    }
    else
    {
        //printf("has not tls\n");
        IMAGE_SECTION_HEADER* Sx = (S+nth->FileHeader.NumberOfSections-1);
        IMAGE_SECTION_HEADER* Sxx = Sx-1;
        //printf("last section %8s\n",Sxx->Name);
        if ( minTls && 0 == memcmp(Sxx->Name,".bss\0\0\0\0",8) )
        {
            printf("generating dummy tls\n");
            //nth->FileHeader.NumberOfSections -= 1;
            //memset(S+nth->FileHeader.NumberOfSections,0,sizeof(IMAGE_SECTION_HEADER));
            if ( memcmp(Sx->Name,".tls\0\0\0",8) )
            { ehf(_S*"last section of container is not .tls (%s)"%StringA((char*)Stls->Name,8)); return false; }
            //nth->OptionalHeader.DataDirectory[IMAGE_DIRECTORY_ENTRY_TLS].VirtualAddress
            //= Sxx->VirtualAddress+cxx_alignu(cxx_max(Sxx->SizeOfRawData,Sxx->Misc.VirtualSize),nth->OptionalHeader.SectionAlignment);
            //nth->OptionalHeader.DataDirectory[IMAGE_DIRECTORY_ENTRY_TLS].Size = 0;
            //IMAGE_SECTION_HEADER *Sx = (S+nth->FileHeader.NumberOfSections-1);
            //while ( !Sx->PointerToRawData ) --Sx;
            //file_length = cxx_alignu(Sx->PointerToRawData+Sx->SizeOfRawData,nth->OptionalHeader.FileAlignment);
            unsigned xva = nth->OptionalHeader.DataDirectory[IMAGE_DIRECTORY_ENTRY_TLS].VirtualAddress;
            unsigned tls_offs = 0;
            for ( int i = 0; i < nth->FileHeader.NumberOfSections; ++i )
                if ( S[i].VirtualAddress <= xva
                     && xva < S[i].VirtualAddress +
                     cxx_max(nth->OptionalHeader.SectionAlignment,
                             cxx_align(S[i].SizeOfRawData,nth->OptionalHeader.SectionAlignment)) )
                {
                    tls_offs = S[i].PointerToRawData+xva-S[i].VirtualAddress;
                    break;
                }
            unsigned tlsS = cxx_alignu(((GetMillis()>>4)%128)+minTls+8,4);
            IMAGE_TLS_DIRECTORY* tls = (IMAGE_TLS_DIRECTORY*)(mapped+tls_offs);
            Sx->VirtualAddress = Sxx->VirtualAddress+cxx_alignu(cxx_max(Sxx->SizeOfRawData,Sxx->Misc.VirtualSize),
                                 nth->OptionalHeader.SectionAlignment);
            tls->StartAddressOfRawData = Sx->VirtualAddress + nth->OptionalHeader.ImageBase;
            tls->EndAddressOfRawData = tls->StartAddressOfRawData + tlsS;
            //tls->AddressOfCallBacks  = tls->EndAddressOfRawData;
            info->tls_index = Sx->VirtualAddress+nth->OptionalHeader.SectionAlignment-8;
            info->tls_callback = 0;
            Sx->SizeOfRawData = cxx_alignu(tlsS,nth->OptionalHeader.FileAlignment);
            Sx->Misc.VirtualSize = tlsS+1;
            file_length = cxx_alignu(Sx->PointerToRawData+Sx->SizeOfRawData,nth->OptionalHeader.FileAlignment);
        }
        else
        {
            nth->FileHeader.NumberOfSections -= 1;
            memset(S+nth->FileHeader.NumberOfSections,0,sizeof(IMAGE_SECTION_HEADER));
            IMAGE_SECTION_HEADER* Sx = (S+nth->FileHeader.NumberOfSections-1);
            nth->OptionalHeader.DataDirectory[IMAGE_DIRECTORY_ENTRY_TLS].VirtualAddress = 0;
            nth->OptionalHeader.DataDirectory[IMAGE_DIRECTORY_ENTRY_TLS].Size = 0;
            while ( !Sx->PointerToRawData ) --Sx;
            file_length = cxx_alignu(Sx->PointerToRawData+Sx->SizeOfRawData,nth->OptionalHeader.FileAlignment);
        }
    }

    return true;

}

bool StubbedFileContainer::Relocate(byte_t* mapped, byte_t* orign, EhFilter ehf)
{
    IMAGE_NT_HEADERS* nth = NT_HEADERS(mapped);
    if ( orignHdrs.OptionalHeader.ImageBase != nth->OptionalHeader.ImageBase )
    {
        unsigned offs = orignHdrs.OptionalHeader.ImageBase - nth->OptionalHeader.ImageBase;
        byte_t* reloc  = mapped+RVAtoOffs(mapped,nth->OptionalHeader.DataDirectory[IMAGE_DIRECTORY_ENTRY_BASERELOC].VirtualAddress);
        byte_t* relocE = reloc+nth->OptionalHeader.DataDirectory[IMAGE_DIRECTORY_ENTRY_BASERELOC].Size;
        while ( reloc != relocE )
        {
            unsigned bsize; int rva; unsigned char const* e;
            rva = RVAtoOffs(mapped,*(int*)reloc); reloc += 4;
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
                        *(unsigned*)(mapped+(rva+fixup)) += offs;
                    }
                    case 0:
                        ;
                }
                reloc+=2;
            }
        }
    }
    nth->FileHeader.NumberOfSections -= 1; // delete .reloc section
    IMAGE_SECTION_HEADER* S  = IMAGE_FIRST_SECTION(nth)+nth->FileHeader.NumberOfSections;
    memset(S,0,sizeof(*S));
    S--;
    if ( memcmp(S->Name,".rsrc\0\0",8) == 0 )
    {
        memset(S,0,sizeof(*S));
        S--;
        nth->FileHeader.NumberOfSections -= 1; // delete .rsrc section
    }
    nth->OptionalHeader.DataDirectory[IMAGE_DIRECTORY_ENTRY_BASERELOC].VirtualAddress = 0;
    nth->OptionalHeader.DataDirectory[IMAGE_DIRECTORY_ENTRY_BASERELOC].Size = 0;
    nth->FileHeader.PointerToSymbolTable = 0;
    nth->FileHeader.NumberOfSymbols = 0;
    nth->OptionalHeader.ImageBase = orignHdrs.OptionalHeader.ImageBase;
    return true;
}

bool StubbedFileContainer::ExtendBss(byte_t* mapped, byte_t* orign, unsigned minSize, EhFilter ehf)
{
    IMAGE_NT_HEADERS* oNth = NT_HEADERS(orign);
    if ( oNth->OptionalHeader.SizeOfImage > minSize ) minSize = oNth->OptionalHeader.SizeOfImage;
    IMAGE_NT_HEADERS* nth = NT_HEADERS(mapped);
    IMAGE_SECTION_HEADER* S  = IMAGE_FIRST_SECTION(nth);
    IMAGE_SECTION_HEADER* Se = S+nth->FileHeader.NumberOfSections;
    IMAGE_SECTION_HEADER* Sbss = Se-2;
    if ( !memcmp(Sbss->Name,".data\0\0\0",8) )
    {
        Sbss->Misc.VirtualSize = cxx_alignu(Sbss->Misc.VirtualSize+minSize,nth->OptionalHeader.SectionAlignment);
        Se[-1].VirtualAddress = Sbss->VirtualAddress + Sbss->Misc.VirtualSize;
        Sbss->Misc.VirtualSize -= time(0)%(nth->OptionalHeader.SectionAlignment-100)+51;
    }
    else if ( !memcmp(Sbss->Name,".bss\0\0\0",8) )
    {
        Sbss->Misc.VirtualSize = cxx_alignu(Sbss->Misc.VirtualSize+minSize,nth->OptionalHeader.SectionAlignment);
        Se[-1].VirtualAddress = Sbss->VirtualAddress + Sbss->Misc.VirtualSize;
        Sbss->Misc.VirtualSize -= time(0)%(nth->OptionalHeader.SectionAlignment-100)+51;
    }
    else
    { ehf(_S*"Se[-2] is not .bss section, is '%8s'"%(char*)Sbss->Name); return false; }
    return true;
}

bool StubbedFileContainer::AddRscSection(byte_t* mapped, byte_t* orign, EhFilter ehf)
{
    IMAGE_NT_HEADERS* nth = NT_HEADERS(mapped);
    IMAGE_NT_HEADERS* oNth = NT_HEADERS(orign);
    IMAGE_SECTION_HEADER* Ss = IMAGE_FIRST_SECTION(nth);
    IMAGE_SECTION_HEADER* Se = Ss+nth->FileHeader.NumberOfSections;
    IMAGE_SECTION_HEADER* S = Se;
    IMAGE_SECTION_HEADER* Sx = S-1;
    IMAGE_SECTION_HEADER* Sxx = Sx-1;
    BufferT<byte_t> rsc;

    //if ( 0 && oNth->OptionalHeader.DataDirectory[IMAGE_DIRECTORY_ENTRY_RESOURCE].VirtualAddress )
    if ( 1 )
    {
        if ( wrapflags&PEWRAP_SAVERSC )
        {
            TakeOriginalResources(orign,oNth,rsc);
        }
        else
        {
            GetRequiredResources(orign,oNth,rsc);
        }

        if ( rsc.Count() )
        {
            if ( Sx->PointerToRawData )
                file_length = Sx->PointerToRawData + cxx_alignu(Sx->SizeOfRawData,nth->OptionalHeader.FileAlignment);
            else
                file_length = Sxx->PointerToRawData + cxx_alignu(Sxx->SizeOfRawData,nth->OptionalHeader.FileAlignment);
            file_length = cxx_alignu(file_length,nth->OptionalHeader.FileAlignment);

            //__asm int 3;
            nth->FileHeader.NumberOfSections += 1;
            memcpy(S->Name,".rsrc\0\0",8);
            S->PointerToRawData = file_length;
            S->Misc.VirtualSize = rsc.Count();
            S->SizeOfRawData = cxx_alignu(rsc.Count(),nth->OptionalHeader.FileAlignment);
            S->VirtualAddress = Sx->VirtualAddress
                                + cxx_alignu( cxx_max(Sx->SizeOfRawData,Sx->Misc.VirtualSize),nth->OptionalHeader.SectionAlignment);
            S->Characteristics = IMAGE_SCN_MEM_READ|IMAGE_SCN_CNT_INITIALIZED_DATA;;
            memcpy(mapped+file_length,+rsc,rsc.Count());
            RscFixup(mapped+file_length,S->VirtualAddress);
            file_length = cxx_alignu(file_length + rsc.Count(),nth->OptionalHeader.FileAlignment);
            nth->OptionalHeader.DataDirectory[IMAGE_DIRECTORY_ENTRY_RESOURCE].VirtualAddress = S->VirtualAddress;
            nth->OptionalHeader.DataDirectory[IMAGE_DIRECTORY_ENTRY_RESOURCE].Size = rsc.Count();
        }
    }
    return true;
}

bool StubbedFileContainer::RecalculateSizes(byte_t* mapped, EhFilter ehf)
{
    IMAGE_NT_HEADERS* nth = NT_HEADERS(mapped);
    IMAGE_SECTION_HEADER* S = IMAGE_FIRST_SECTION(nth);
    IMAGE_SECTION_HEADER* Se = S+nth->FileHeader.NumberOfSections;
    nth->OptionalHeader.SizeOfImage = Se[-1].VirtualAddress +
                                      cxx_alignu( cxx_max(Se[-1].SizeOfRawData,Se[-1].Misc.VirtualSize),nth->OptionalHeader.SectionAlignment);
    nth->OptionalHeader.SizeOfCode = 0;
    nth->OptionalHeader.SizeOfInitializedData = 0;
    nth->OptionalHeader.SizeOfUninitializedData = 0;
    for ( IMAGE_SECTION_HEADER* s = S; s != Se; ++s )
    {
        unsigned x = cxx_alignu( cxx_max(s->SizeOfRawData,s->Misc.VirtualSize),nth->OptionalHeader.FileAlignment);

        if ( s->Characteristics & IMAGE_SCN_CNT_UNINITIALIZED_DATA )
            nth->OptionalHeader.SizeOfUninitializedData += x;
        else if ( s->Characteristics & IMAGE_SCN_CNT_CODE )
            nth->OptionalHeader.SizeOfCode += x;
        else if ( s->Characteristics & IMAGE_SCN_CNT_INITIALIZED_DATA )
            nth->OptionalHeader.SizeOfInitializedData += x;
    }
    IMAGE_SECTION_HEADER* SS = Se-1;
    while ( !SS->PointerToRawData ) --SS;
    file_length = SS->PointerToRawData + cxx_alignu(SS->SizeOfRawData,nth->OptionalHeader.FileAlignment);
    return true;
}

bool StubbedFileContainer::AddOrignSections(byte_t* mapped, byte_t* orign, EhFilter ehf)
{
    IMAGE_NT_HEADERS* nth = NT_HEADERS(orign);
    IMAGE_NT_HEADERS* nth2 = NT_HEADERS(mapped);
    NEWDES_Cipher cipher;
    cipher.SetupEncipher(info->xcrypt_key);
    byte_t* area = mapped+file_length;
    unsigned zhl = sizeof(ZIPPED_HEADER)+(nth->FileHeader.NumberOfSections-1)*sizeof(ZIPPED_SECTION);
    unsigned area_offs = zhl;
    IMAGE_SECTION_HEADER* S = IMAGE_FIRST_SECTION(nth);
    IMAGE_SECTION_HEADER* Se = S+nth->FileHeader.NumberOfSections;

    for ( int i = 0; i < nth->FileHeader.NumberOfSections; ++i ) zhl += S[i].SizeOfRawData;
    ZIPPED_HEADER* zh = (ZIPPED_HEADER*)(area);
    zh->file_alignment = nth->OptionalHeader.FileAlignment;
    zh->section_alignment = nth->OptionalHeader.SectionAlignment;

    for ( int i = 0; i < 16; ++i )
    {
        if ( i == IMAGE_DIRECTORY_ENTRY_RESOURCE && (wrapflags&PEWRAP_SAVERSC) )
            zh->orgdir[i] = nth2->OptionalHeader.DataDirectory[i];
        else
            zh->orgdir[i] = nth->OptionalHeader.DataDirectory[i];
    }

    zh->number = nth->FileHeader.NumberOfSections;
    for ( int i = 0; i < nth->FileHeader.NumberOfSections; ++i )
    {
        memset(zh->section+i,0,sizeof(zh->section[i]));
        memcpy(zh->section[i].name,S[i].Name,8);

        //cipher.DoCipher(zh->section[i].name,1);
        zh->section[i].access = S[i].Characteristics;
        zh->section[i].rva    = S[i].VirtualAddress;
        zh->section[i].vasize = S[i].Misc.VirtualSize;
        zh->section[i].rawsize= S[i].SizeOfRawData;

        if ( !strnicmp((char*)S[i].Name,".rsrc",5) && (wrapflags&PEWRAP_SAVERSC) )
        {
            Xoln | _S*"section %s skipped" % (char*)S[i].Name;
            continue;
        }

        zh->section[i].offset = area_offs;
        zh->section[i].crc    = Crc32(0,orign+S[i].PointerToRawData,S[i].SizeOfRawData);

        if ( S[i].SizeOfRawData )
        {
            int l = zlib_compress_l(
                        orign+S[i].PointerToRawData,
                        S[i].SizeOfRawData,
                        area+area_offs,
                        zhl-area_offs,
                        9);
            if ( l > 0 && l < S[i].SizeOfRawData )
            {
                zh->section[i].zipped = 1;
                zh->section[i].count = l;
            }
            else
            {
                memcpy(area+area_offs,orign+S[i].PointerToRawData,S[i].SizeOfRawData);
                zh->section[i].count = S[i].SizeOfRawData;
            }
            cipher.DoCipherCBCI(area+area_offs,zh->section[i].count/8);
            zh->section[i].xcrypt = 1;
        }

        area_offs += zh->section[i].count;
    }

    info->zip.offset = file_length;
    info->zip.count  = sizeof(ZIPPED_HEADER)+(nth->FileHeader.NumberOfSections-1)*sizeof(ZIPPED_SECTION);//area_offs;
    file_length += area_offs;
    info->zip.crc = Crc32(0,zh,info->zip.count);
    cipher.DoCipherCBCI(zh,info->zip.count/8);
    return true;
}

bool StubbedFileContainer::AddCore(byte_t* mapped ,EhFilter ehf)
{
    if ( wrapflags & PEWRAP_DEBUGCORE )
    {
        info->flags |= STBF_EXTERNAL_CORE;
    }
    if ( 1 )
    {
        info->core.offset = file_length;
        info->core.count  = Core0sLength();
        info->flags |= STBF_CORE_ZIPPED|STBF_CORE_XCRYPED;
        memcpy(mapped+file_length,Core0S(),info->core.count);
        NEWDES_Cipher cipher;
        cipher.SetupEncipher(info->xcrypt_key);
        cipher.DoCipherCBCI(mapped+file_length,info->core.count/8);
        info->core.crc = Crc32(0,mapped+file_length,info->core.count);
        file_length += info->core.count;
    }
    return true;
}

bool StubbedFileContainer::Open(StringParam original,StringParam target, StringParam stub, unsigned flags, unsigned minSize,
                                unsigned minTls, EhFilter ehf)
{
    byte_t*  mapped = 0;
    byte_t*  orignMapped = 0;
    unsigned mapped_length = 0;

    if ( !(flags&PEWRAP_RELINK) || (flags&PEWRAP_DEMO) ) flags |= PEWRAP_SAVEIAT;

    bool retval = false;
    info2 = &info_dat;
    info  = &info2->i;
    info_offs = 0;
    memset ( &info_dat,0,sizeof(info_dat) );
    file_length = 0;
    SysUrandom(info->xcrypt_key,sizeof(info->xcrypt_key));

    IMAGE_NT_HEADERS* nth = 0;

    /******/

    DataStreamPtr targetDs;

    if ( stub.Length() && !FileExists(stub) )
    { ehf(_S*"container '%s' does not exist" %stub); goto e; }

    if ( !CreateDirectoriesIfNeed(target) )
    { ehf("failed to create required directories"); goto e; }

    if ( !(targetDs = CopyStub(stub,target,0,ehf)) )
    { ehf("failed to copy container"); goto e; }
    targetDs->Flush();

    if ( FileExists(target) ) // insighnity check!
    {
        targetDs->Seek(0);
        if ( ReadImageNtHeaders(targetDs,&stubHdrs) );
        else
        { ehf("failed to read PE data from container"); goto e; }
    }
    else
    { ehf("failed to open container file, does not exist"); goto e;}

    //targetDs->Seek(0);
    //targetDs->Write16le(0x0000); // kill MZ signature
    targetDs->Flush();
    targetDs->Close();

    if ( DataStreamPtr ds = OpenHandleStreamRtry(original,STREAM_READ) )
        if ( ReadImageNtHeaders(ds,&orignHdrs) );
        else
        { ehf("failed to read PE data from original EXE"); goto e; }
    else
    { ehf("failed to open original EXE file"); goto e;}

    // waiting for anivirus/watcher software which can locking target file
    Sleep(300);

    this->target = target;
    file_length = LengthOfFile(target);
    mapped_length = file_length*2;

    if ( !(orignMapped = (byte_t*)SysMmapFileRnCoW(original,ehf)) )
        goto e;

    if ( !(flags&PEWRAP_SAVEIAT) && (flags&PEWRAP_ANTIHACK) && !(flags&PEWRAP_SELFCONTAINS) )
        ReorderImports(orignMapped,squoted,info->xcrypt_key,ehf);

    if ( mapped_length < orignHdrs.OptionalHeader.SizeOfImage )
        mapped_length = orignHdrs.OptionalHeader.SizeOfImage;

    mapped_length += WRAPPING_EXTENSION;
    if ( !(mapped = (byte_t*)SysMmapFile(target,mapped_length,0,true,false,ehf)) )
        goto e;

    //IMAGE_DOS_HEADER *hdos = (IMAGE_DOS_HEADER *)mapped;
    //hdos->e_csum = 0;

    nth = NT_HEADERS(mapped);
    nth->OptionalHeader.CheckSum = 0;
    *(u16_t*)mapped = 0x4a5a; // temporary it's not an EXEcutable
    wrapflags = flags;

    //nth->OptionalHeader.DllCharacteristics &= ~(0x0040|0x0100|0x0080);
    //nth->OptionalHeader.MajorOperatingSystemVersion = 5;
    //nth->OptionalHeader.MinorOperatingSystemVersion = 0;
    //nth->OptionalHeader.MajorSubsystemVersion = 5;
    //nth->OptionalHeader.MinorSubsystemVersion = 0;

    nth->OptionalHeader.SizeOfStackReserve = orignHdrs.OptionalHeader.SizeOfStackReserve;
    nth->OptionalHeader.SizeOfStackCommit  = orignHdrs.OptionalHeader.SizeOfStackCommit;
    nth->OptionalHeader.SizeOfHeapReserve  = orignHdrs.OptionalHeader.SizeOfHeapReserve;
    nth->OptionalHeader.SizeOfHeapCommit   = orignHdrs.OptionalHeader.SizeOfHeapCommit;
    nth->OptionalHeader.Subsystem          = orignHdrs.OptionalHeader.Subsystem;
    nth->FileHeader.Characteristics |= 0x001;
    nth->FileHeader.Characteristics |= orignHdrs.FileHeader.Characteristics & 0x020; /*IMAGE_FILE_LARGE_ADDRESS_AWARE*/;

    nth->OptionalHeader.DataDirectory[IMAGE_DIRECTORY_ENTRY_DEBUG].Size = 0;
    nth->OptionalHeader.DataDirectory[IMAGE_DIRECTORY_ENTRY_DEBUG].VirtualAddress = 0;

    Relocate(mapped,orignMapped,ehf);
    ExtendBss(mapped,orignMapped,minSize,ehf);
    AddTlsSection(mapped,orignMapped,minTls,ehf);
    AddRscSection(mapped,orignMapped,ehf);
    RecalculateSizes(mapped,ehf);

    file_length = cxx_alignu(file_length,nth->OptionalHeader.FileAlignment);
    info_offs = file_length+8;
    info_max = ((info_offs + sizeof(info_dat))&~4095) + 4095;
    file_length += sizeof(info_dat)+8;
    info->originals_offs = file_length;
    info2->padding = GetMillis();
    //*(unsigned*)(mapped+file_length) = 0; // no restorable bytes (use in protection ???)
    //file_length += 4;
    u32_t mask = GetMillis();
    unsigned b = STB_SIGN_SECOND ^ mask;
    unsigned a = STB_SIGN_FIRST ^ mask;
    REQUIRE( (b ^ STB_SIGN_SECOND) ^ a == STB_SIGN_FIRST );
    *(unsigned*)(mapped+info_offs-8) = a;
    *(unsigned*)(mapped+info_offs-4) = b;

    AddCore(mapped,ehf);

    if ( !(flags & PEWRAP_SELFCONTAINS) )
        AddOrignSections(mapped,orignMapped,ehf);

    info->oep = orignHdrs.OptionalHeader.AddressOfEntryPoint;
    info->hwid = QuerySystemHWID();
    info->flags |= STBF_EXTENDED;
    info->flags |= TEGGOVBOX_VIRTUALIZE_FS;
    info2->mxbpack_build = _BUILD_NUMBER;
    info2->licid = 0;
    info2->liclevel = 99;

    if ( flags & PEWRAP_RELINK )        info->flags |= STBF_EXE_ZIPPED;
    if ( flags & PEWRAP_LOGGING )       info->flags |= TEGGOVBOX_LOG_ACTIVITY;
    if ( flags & PEWRAP_DBGLOG )        info->flags |= TEGGOVBOX_LOG_ACTIVITY_DBG;
    if ( flags & PEWRAP_HASPACKS )      info->flags |= TEGGOVBOX_SEARCH_FOR_EMBEDDINGS;
    if ( flags & PEWRAP_DOTNET )        info->flags |= STBF_EXE_DOTNET;
    if ( flags & PEWRAP_DEMO )          info->flags |= STBF_EXE_DEMO;
    if ( flags & PEWRAP_HWID )          info->flags |= STBF_BIND2HWID;
    if ( flags & PEWRAP_ANTIHACK )      info->flags |= STBF_EXE_ANTIHACK;
    if ( flags & PEWRAP_SAVEIAT )       info->flags |= STBF_SAVE_IAT;
    if ( flags & PEWRAP_STRONGANTIHACK )info->flags |= STBF_EXE_ARMODIL;
    if ( flags & PEWRAP_HANDLERGS )     info->flags |= TEGGOVBOX_VIRTUALIZE_REGISTRY;
    if ( flags & PEWRAP_HANDLERGS_FULL )info->flags |= TEGGOVBOX_VIRTUALIZE_REGISTRY_FULL;
    if ( flags & PEWRAP_BOXEMBEDED )    info->flags |= TEGGOVBOX_SEARCH_FOR_EMBEDDINGS;
    if ( flags & PEWRAP_SELFCONTAINS )  info->flags |= STBF_EMBEDDED_LOADING;
    if ( flags & PEWRAP_INJECT )        info->flags |= TEGGOVBOX_CORE_INJECT;
    if ( flags & PEWRAP_REGISTER_OCX )  info->flags |= STBF_REGISTER_OCX;

    if ( flags & PEWRAP_INSINITY )
    {
        info2->f.insinity = 1;
        Xoln | "insinity mode is ON!";
    }
    if ( (flags & PEWRAP_INSINITY) && (flags & PEWRAP_MINIDUMPFULL) )
    {
        info2->f.minidump_full = 1;
        Xoln | "full minidump mode is ON!";
    }


    SysUnmmapFile(mapped); mapped = 0;
    SysTruncateFile(target,file_length);
    retval = true;
e:
    if (mapped) SysUnmmapFile(mapped);
    if (orignMapped) SysUnmmapFile(orignMapped);
    return retval;
}

bool StubbedFileContainer::SetRegmask(StringParam regmask, EhFilter ehf)
{
    int L = (regmask.Length()*2+2+7) & ~7;
    BufferT<byte_t> data(L,0);
    memcpy(+data,+regmask,regmask.Length()*2);
    NEWDES_Cipher cipher;
    cipher.SetupEncipher(info->xcrypt_key);
    cipher.DoCipherCBCI(+data,L/8);
    u32_t crc = Crc32(0,+data,L);

    DataStreamPtr ds = OpenStreamRtry(target,STREAM_WRITE);
    ds->Seek(0,SEEK_END);
    u32_t offset = ds->Tell();

    ds->Write(+data,L);
    Unrefe(ds);

    info2->regmask.offset = offset;
    info2->regmask.count  = L;
    info2->regmask.crc = crc;

    Xoln| _S*"regmask '%s' will be used to import registry files" %regmask;
    return true;
}

bool StubbedFileContainer::AttachActivator(StringParam activator, EhFilter ehf)
{
    StringA module = GetBasenameOfPath(activator);
    int l = StrLen(activator);
    if ( !(wrapflags & PEWRAP_EMBEDDING) && l >= sizeof(info->act_file_name) )
        return ehf("activator path is too long"),false;

    if ( (wrapflags & PEWRAP_EMBEDDING) && !(wrapflags & PEWRAP_BOXED_ACT) )
    {
        unsigned flags = 0;
        unsigned rel_length = LengthOfFile(activator);
        unsigned crc = 0;
        unsigned offset = 0;
        int pak_length;

        DataStreamPtr ds = OpenHandleStreamRtry(activator,STREAM_READ|STREAM_NONINHERITABLE);
        if ( !rel_length || !ds )
        { ehf(_S*"failed to open activator '%s'"%activator); return false; }

        BufferT<byte_t> data(rel_length);
        BufferT<byte_t> cdata(rel_length+4);
        ds->Read(+data,data.Count());
        Unrefe(ds);

        if ( !(wrapflags&PEWRAP_DEMO) && (wrapflags&PEWRAP_ANTIHACK) )
            ReorderImports(+data,squoted_act,info->xcrypt_key,ehf);

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
        cipher.SetupEncipher(info->xcrypt_key);
        cipher.DoCipherCBCI(+data,pak_length/8);
        crc = Crc32(0,+data,pak_length);

        ds = OpenStreamRtry(target,STREAM_WRITE);
        ds->Seek(0,SEEK_END);
        offset = ds->Tell();

        ds->Write(+data,pak_length);
        Unrefe(ds);

        info->flags |= flags;
        info->act.offset = offset;
        info->act.count = pak_length;
        info->act.crc = crc;
        Xoln| _S*"  + %-16s [%3d%%] %s" % module %(pak_length*100/rel_length) %activator;
    }
    else
    {
        info->flags |= STBF_EXTERNAL_ACT;
        memcpy(info->act_file_name,+StringA(activator),l+1);
        if ( wrapflags & PEWRAP_BOXED_ACT )
            info->flags |= STBF_BOXED_ACT;
    }

    if ( (wrapflags & PEWRAP_EMBEDDING) && !(wrapflags & PEWRAP_BOXED_ACT) )
    {
        BUNDLED_IMPORT bi = {0};
        u32_t r = GetMillis();
        for ( int i = 0; i < sizeof(bi.name)/4; ++i )
            *(unsigned*)(bi.name+i*4) = teggo::unsigned_random(&r);
        bi.name[sizeof(bi.name)-1] = 0;
        strncpy(bi.name,+module,sizeof(bi.name)-1);
        bi.actfake = 1;
        bi.activator = 1;
        bundles.Push(bi);
    }

    return true;
}

bool StubbedFileContainer::SetCmdline(StringParam S,EhFilter ehf)
{
    if ( info->flags & STBF_EXTERNAL_ACT )
    {
        ehf("commandline overriding incompatible with external activator");
        return false;
    }

    if ( S.Length() > sizeof(info->cmdline)/sizeof(wchar_t) )
    {
        ehf("commandline to long");
        return false;
    }

    info->flags |= STBF_CMDLINE;
    wcscpy(info->cmdline,+S);
    return true;
}

bool StubbedFileContainer::PredefineReg(StringParam S,EhFilter ehf)
{
    return true;
}

bool StubbedFileContainer::PredefineEnviron(StringParam S,EhFilter ehf)
{
    return true;
}

bool StubbedFileContainer::BundleDLL(StringParam dllname, bool hideonly, EhFilter ehf)
{
    if ( hideonly ) return true;

    StringA module = GetBasenameOfPath(dllname);
    BUNDLED_IMPORT bi = {0};

    if ( !AfxBundleDLL(dllname,module,target,bi,info->xcrypt_key,ehf) )
        return false;

    bundles.Push(bi);
    Xoln| _S*"  + %-16s [%3d%%] %s" % module %(bi.count*100/bi.size) %TrimLength(+dllname,50);

    return true;
}

bool StubbedFileContainer::CopyExtraData(StringParam infile, bool encrypt, EhFilter ehf)
{
    bool result = false;
    if ( !DataSource->Exists(infile) )
    { ehf("file does not exist"); return false; }
    u32_t in_file_length = LengthOfFile(infile);
    if ( DataStreamPtr outds = OpenStreamRtry(target,STREAM_WRITE) )
    {
        outds->Seek(0,SEEK_END);
        u32_t out_file_length = outds->Tell();
        out_file_length = (out_file_length + (0x200-1))&~(0x200-1);
        outds->Seek(out_file_length);
        //Xoln|_S*"%s:%d,%s:%d" %infile%in_file_length%outfile%out_file_length;
        if ( byte_t* from = (byte_t*)SysMmapFileRnCoW(infile,ehf) )
        {
            //Xoln|_S*"%04x" %*(u16_t*)from;
            if ( *(u16_t*)from == 0x5a4d || *(u16_t*)from == 0x4a5a )
            {
                IMAGE_NT_HEADERS* nth = NT_HEADERS(from);
                IMAGE_SECTION_HEADER* S = IMAGE_FIRST_SECTION(nth);
                IMAGE_SECTION_HEADER* Se = S+nth->FileHeader.NumberOfSections;
                u32_t a = (Se[-1].SizeOfRawData + nth->OptionalHeader.FileAlignment-1)&~(nth->OptionalHeader.FileAlignment-1);
                u32_t length = Se[-1].PointerToRawData + a;
                u32_t exdata_len = in_file_length-length;
                if ( !exdata_len )
                    result = true;
                else
                {
                    if ( encrypt )
                    {
                        u32_t count = exdata_len &~7;
                        NEWDES_Cipher cipher;
                        cipher.SetupEncipher(info->xcrypt_key);
                        cipher.DoCipher(from+length,count/8);
                        info->extra_offs = outds->Tell();
                        info->extra_count = count;
                        Xoln|_S*"encrypted %d bytes of extra data"%count;
                    }
                    outds->Write( from+length, exdata_len );
                    Xoln|_S*"added %d bytes of extra data"%exdata_len;
                    result = true;
                    outds->Close();
                }
            }
            else
                ehf("opss?!");
            SysUnmmapFile(from);
        }
        else
            ehf("failed to map source");
    }
    else
        ehf("failed to re-open target");
    return result;
}

bool StubbedFileContainer::Finalize(EhFilter ehf)
{
    AfxAttachBundles(target, bundles, info->xcrypt_key, info->bundle, ehf);
    AfxAttachSquoted(target, squoted, info->xcrypt_key, info->squoted, ehf);
    AfxAttachSquoted(target, squoted_act, info->xcrypt_key, info->squoted_act, ehf);
    return true;
}

unsigned CalcFileChecksum(StringParam name,EhFilter ehf)
{
    unsigned sum = 0;
    HANDLE f  =  CreateFileW( +name, GENERIC_READ, FILE_SHARE_READ, 0, OPEN_EXISTING, 0, 0 );
    if ( f )
    {
        byte_t bf[1024*64];
        unsigned flen = GetFileSize(f,0);
        unsigned i = 0;
        while ( i < flen )
        {
            DWORD q = 0;
            int left = flen-i;
            int toread = cxx_min<size_t>(left,sizeof(bf));
            memset(bf,0,sizeof(bf));

            if ( !ReadFile(f,bf,toread,&q,0) )
            {
                ehf(_S*"Failed to calculate checksum. file error");
                return 0;
            }

            if ( q > 0 )
            {
                unsigned short* p = (unsigned short*)bf;
                if ( !i  ) *p = 0x5a4d;
                for ( int j = 0; j < (q+1)/2; ++j )
                {
                    sum += *p++;
                    sum = (sum&0x0ffff) + (sum >> 16);
                }
            }
            i += q;
        }
        CloseHandle(f);
        sum = ((sum&0x0ffff) + (sum >> 16))&0x0ffff;
        //printf("file len: %08x(%d), sum: %08x(%d)\n",flen,flen,sum,sum);
        return  sum + flen;
    }
    else
    {
        char* err = strerror(errno);
        ehf(_S*"Failed to calculate checksum. file error %s"%err);
        return 0;
    }
}

bool StubbedFileContainer::Close(EhFilter ehf)
{
    if ( byte_t* mapped = (byte_t*)SysMmapFile(target,info_max,0,true,false,ehf) )
    {
        //UnumerateImports(mapped);
        STB_INFO* xinfo = (STB_INFO*)(mapped+info_offs);
        NEWDES_Cipher cipher;
        memcpy(xinfo,&info_dat,sizeof(info_dat));
        cipher.SetupEncipher(info->xcrypt_key);
        cipher.DoCipherCBCI(xinfo,(sizeof(STB_INFO)-16)/8);
        cipher.DoCipherCBCI(xinfo+1,(sizeof(info_dat)-sizeof(STB_INFO))/8);
        memcpy(xinfo->xcrypt_key,info->xcrypt_key,sizeof(info->xcrypt_key));
        IMAGE_NT_HEADERS* nth = NT_HEADERS(mapped);
        //nth->OptionalHeader.CheckSum = sum;
        SysUnmmapFile(mapped);
    }
    unsigned sum = CalcFileChecksum(target,ehf);
    if ( byte_t* mapped = (byte_t*)SysMmapFile(target,info_max,0,true,false,ehf) )
    {
        IMAGE_NT_HEADERS* nth = NT_HEADERS(mapped);
        nth->OptionalHeader.CheckSum = sum;
        *(u16_t*)mapped = 0x5a4d;
        SysUnmmapFile(mapped);
    }
    //printf("sum: %08x(%d)\n",sum,sum);
    return true;
}
