
/*

Copyright (c) 2008-2009, Alexey Sudachen, alexey@sudachen.name

*/

#include "_common.h"
#include "apif.h"
#include "splicer.h"
#include "logger.h"
#include "import.h"
#include "XorS.h"

namespace
{

    int _read_e_file(HANDLE f,void* ptr, unsigned len)
    {
        unsigned long foo = 0;
        do
        {
            if ( ReadFile(f,ptr,len,&foo,0) )
                *(char**)&ptr += foo, len-=foo;
            else
                return 0;
            //else
            //  __asm__(("int3"));
        }
        while (len);
        return 1;
    }

    void read_e_file(HANDLE f, void* ptr, unsigned len)
    {
        if ( !_read_e_file(f,ptr,len) )
            __asm int 3
            ;
    }

    int seek_e_file(HANDLE f, unsigned offs)
    {
        long foo = 0;
        return SetFilePointer(f,offs,&foo,0);
    }

    void* valloc(int sz)
    {
        void* m = 0;
        if ( m = VirtualAlloc(0,sz,MEM_COMMIT,PAGE_EXECUTE_READWRITE) )
            return m;
        if ( m = VirtualAlloc(0,sz,MEM_COMMIT,PAGE_READWRITE) )
            return m;
        __asm int 3
        ;
        return 0;
    }

    void Relocate(unsigned char* base, IMAGE_NT_HEADERS const* nth)
    {
        unsigned offs = (unsigned)base - nth->OptionalHeader.ImageBase;
        const unsigned char* reloc  = base+nth->OptionalHeader.DataDirectory[IMAGE_DIRECTORY_ENTRY_BASERELOC].VirtualAddress;
        const unsigned char* relocE = reloc+nth->OptionalHeader.DataDirectory[IMAGE_DIRECTORY_ENTRY_BASERELOC].Size;
        while ( reloc != relocE )
        {
            int rva = *(int*)reloc; reloc += 4;
            unsigned bsize = *(unsigned*)reloc-8; reloc += 4;
            unsigned char const* e = reloc+bsize;
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

    void* LoadDll(HANDLE f,STB_INFO* info)
    {
        unsigned foo, size = info->act.count, i;
        byte_t* buf, *unpacked, *base;

        seek_e_file(f,info->act.offset);
        buf = (byte_t*)valloc(size);
        read_e_file(f,buf,size);

        if ( Crc32(0,buf,size) != info->act.crc )
            __asm int 3
            ;

        if ( info->flags & STBF_ACT_XCRYPED )
        {
            NEWDES_Cipher cipher;
            cipher.SetupDecipher(info->xcrypt_key);
            cipher.DoCipherCBCO(buf,size/8);
        }

        if ( info->flags & STBF_ACT_ZIPPED )
        {
            unsigned l = *(unsigned*)buf;
            unpacked = (byte_t*)valloc(l);
            zlib_decompress(buf+4,size-4,unpacked,l);
        }
        else
            unpacked = buf;

        IMAGE_NT_HEADERS const* nth = NT_HEADERS(unpacked);
        base = (byte_t*)valloc(nth->OptionalHeader.SizeOfImage);
        memcopy(base,unpacked,
                sizeof(IMAGE_NT_HEADERS)-sizeof(IMAGE_OPTIONAL_HEADER)
                +sizeof(IMAGE_SECTION_HEADER)*nth->FileHeader.NumberOfSections
                +nth->FileHeader.SizeOfOptionalHeader
               );
        IMAGE_SECTION_HEADER const* S = (IMAGE_SECTION_HEADER*)((char*)nth + (24 + nth->FileHeader.SizeOfOptionalHeader));
        for ( i = 0; i < nth->FileHeader.NumberOfSections; ++i )
        {
            IMAGE_SECTION_HEADER const* const Si = S+i;
            memcopy(base+Si->VirtualAddress,unpacked+Si->PointerToRawData,Si->SizeOfRawData);
        }
        Relocate(base,nth);
        Import(base,nth);
        if ( nth->OptionalHeader.AddressOfEntryPoint )
        {
            void* ep = base + nth->OptionalHeader.AddressOfEntryPoint;
            ((int(__stdcall*)(void*,int,void*))ep)(base,DLL_PROCESS_ATTACH,0);
        }
        VirtualFree(buf,0,MEM_RELEASE);
        if (unpacked != buf) VirtualFree(unpacked,0,MEM_RELEASE);
        return base;
    }

} // namespace

void ExecuteActivator(STB_INFO* info)
{
    void* base = 0;

    if ( info->flags & STBF_EXTERNAL_ACT )
    {
        if ( info->flags & STBF_BOXED_ACT )
            base = LoadLibraryW(+StringW(Hinstance->GetMainModulePath())+L"\\"+info->act_file_name);
        else
            base = LoadLibraryA(info->act_file_name);
    }
    else if ( info->act.offset )
    {
        HANDLE f = CreateFileW(Hinstance->GetMainModulePath(),GENERIC_READ,FILE_SHARE_READ,0,OPEN_EXISTING,0,0);
        if ( f && f != INVALID_HANDLE_VALUE )
        {
            base = LoadDll(f,info);
            CloseHandle(f);
        }
        else
            __asm int 3
            ;
    }
    else
        return;

    if ( base )
    {
        int (__stdcall *f)(void* base) = 0;
        typedef int (__cdecl *fC)(void*);
        *(void**)&f = GetProcAddressIndirect(base,_XOr("_Activate@4",12,1376285),0);
        if ( !f ) *(void**)&f = GetProcAddressIndirect(base,_XOr("Activate@4",11,6422638),0);
        if ( !f ) *(void**)&f = GetProcAddressIndirect(base,_XOr("Activate",9,7340144),0);
        if ( f && f(base) ) return;
        else *(void**)&f = GetProcAddressIndirect(base,_XOr("_activate",10,8126532),0);
        if ( f && (fC)(base) ) return;
    }

    TerminateProcess((HANDLE)-1,-1);
}
