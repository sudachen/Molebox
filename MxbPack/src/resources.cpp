
/*

  Copyright (c) 2008-2009, Alexey Sudachen, alexey@sudachen.name

*/


#include "myafx.h"
#include <time.h>
#include "peover.h"
#include "teggovbox.h"

static const char predefined_manifest[] =
    ""
    ;

void FilterManifest(void const* text, int len, StringA& S)
{
    S = StringA((char const*)text,len);
    int a = 0;

repeat:
    if ( (a = S.Search(a,"<dependency>",-1,false)) >= 0 )
    {
        char e[] = "</dependency>";
        int b = S.Search(a,e,-1,false);
        if ( b > a )
            S.Remove(a,b-a+sizeof(e));
        else
            a += 12;
        goto repeat;
    }
}

struct ICOHEADER
{
    WORD wReserved;  // Always 0
    WORD wResID;     // Always 1
    WORD wNumImages; // Number of icon images/directory entries
};

struct ICOENTRY
{
    BYTE  bWidth;
    BYTE  bHeight;
    BYTE  bColors;
    BYTE  bReserved;
    WORD  wPlanes;
    WORD  wBitCount;
    WORD  dwBytesInImage_Lo;
    WORD  dwBytesInImage_Hi;
    WORD  wID;
};

bool FindRSC(
    unsigned rsc,
    int id,
    byte_t* rcdata,
    IMAGE_RESOURCE_DATA_ENTRY** entry,
    WORD* found_id = 0)
{
    // main (RT variants)
    IMAGE_RESOURCE_DIRECTORY* m = (IMAGE_RESOURCE_DIRECTORY*)rcdata;
    IMAGE_RESOURCE_DIRECTORY_ENTRY* mR = (IMAGE_RESOURCE_DIRECTORY_ENTRY*)((byte_t*)m + sizeof(*m));
    IMAGE_RESOURCE_DIRECTORY_ENTRY* mRE = mR+m->NumberOfIdEntries+m->NumberOfNamedEntries;
    for ( ; mR != mRE; ++mR ) if ( mR->NameOffset == rsc && mR->DataIsDirectory)
        {
            // rt roup (ID variants)
            IMAGE_RESOURCE_DIRECTORY* r = (IMAGE_RESOURCE_DIRECTORY*)(rcdata + mR->OffsetToDirectory);
            IMAGE_RESOURCE_DIRECTORY_ENTRY* rR= (IMAGE_RESOURCE_DIRECTORY_ENTRY*)((byte_t*)r + sizeof(*r));
            IMAGE_RESOURCE_DIRECTORY_ENTRY* rRE = rR+r->NumberOfIdEntries+r->NumberOfNamedEntries;
            for ( ; rR != rRE; ++rR ) if ( id < 0 || (rR->Id == id && rR->DataIsDirectory))
                {
                    // id group (LANG variants)
                    IMAGE_RESOURCE_DIRECTORY_ENTRY* e = (IMAGE_RESOURCE_DIRECTORY_ENTRY*)
                                                        (rcdata + rR->OffsetToDirectory + sizeof(IMAGE_RESOURCE_DIRECTORY));
                    if ( !e->DataIsDirectory )
                        *entry = (IMAGE_RESOURCE_DATA_ENTRY*)(rcdata+e->OffsetToData);

                    if ( found_id ) *found_id = rR->Id;

                    return !!*entry;
                }
        }
    return false;
}

unsigned WriteFFinfoString(ArrayDataStream& ads, pwide_t name, pwide_t value)
{
    unsigned S = ads.Tell();
    if ( int i = S&3 ) for ( ; 4-i; ++i) ads.Write8(0);
    unsigned foo = 0;
    unsigned nl = wcslen(name) * 2 +2;
    unsigned vl = wcslen(value) * 2 +2;
    unsigned sl = 3*2 + nl;
    unsigned nl_align;
    foo = (sl+3)& ~3;
    nl_align = foo - sl;
    sl = foo + vl;
    foo = (sl+3)&~3;
    ads.Write16le(sl);
    ads.Write16le(vl/2-1);
    ads.Write16le(1);
    ads.Write(name,nl);
    ads.Write("\0\0\0\0",nl_align);
    ads.Write(value,vl);
    return S;
}

bool GetRequiredResources(byte_t* mapped, IMAGE_NT_HEADERS* nth, BufferT<byte_t>& data)
{
    void* self_base = GetModuleHandleA(0);
    unsigned rsc_va = nth->OptionalHeader.DataDirectory[IMAGE_DIRECTORY_ENTRY_RESOURCE].VirtualAddress;
    unsigned rsc_ln = nth->OptionalHeader.DataDirectory[IMAGE_DIRECTORY_ENTRY_RESOURCE].Size;
    IMAGE_SECTION_HEADER* S = IMAGE_FIRST_SECTION(nth);
    IMAGE_SECTION_HEADER* Se = S+nth->FileHeader.NumberOfSections;
    do
        if ( S->VirtualAddress <= rsc_va && S->VirtualAddress+S->SizeOfRawData > rsc_va )
            break;
    while ( ++S != Se );
    if ( S == Se ) S = 0;
    //S = 0;

    IMAGE_NT_HEADERS* nth1 = NT_HEADERS(self_base);
    unsigned rsc_va1 = nth1->OptionalHeader.DataDirectory[IMAGE_DIRECTORY_ENTRY_RESOURCE].VirtualAddress;
    unsigned rsc_ln1 = nth1->OptionalHeader.DataDirectory[IMAGE_DIRECTORY_ENTRY_RESOURCE].Size;
    IMAGE_SECTION_HEADER* S1 = IMAGE_FIRST_SECTION(nth1);
    IMAGE_SECTION_HEADER* S1e = S+nth1->FileHeader.NumberOfSections;
    do
        if ( S1->VirtualAddress <= rsc_va1 && S1->VirtualAddress+S1->SizeOfRawData > rsc_va1 )
            break;
    while ( ++S1 != S1e );
    if ( S1 == S1e ) return 0;

    byte_t* rcdata = S ? mapped + (rsc_va - S->VirtualAddress + S->PointerToRawData) : 0;
    byte_t* rcdata1 = (byte_t*)self_base + rsc_va1;
    IMAGE_RESOURCE_DATA_ENTRY* icog_entry = 0;
    IMAGE_RESOURCE_DATA_ENTRY* info_entry = 0;
    IMAGE_RESOURCE_DATA_ENTRY* mfs_entry = 0;

    enum {MAX_ICONS=15};

    struct RESOURCES
    {
        struct
        {
            IMAGE_RESOURCE_DIRECTORY h;
            IMAGE_RESOURCE_DIRECTORY_ENTRY r[4];
        } main;
        struct
        {
            IMAGE_RESOURCE_DIRECTORY h;
            IMAGE_RESOURCE_DIRECTORY_ENTRY r; // rt group
            IMAGE_RESOURCE_DIRECTORY i;
            IMAGE_RESOURCE_DIRECTORY_ENTRY e; // lang group of id
            IMAGE_RESOURCE_DATA_ENTRY d;
        } igrp;
        struct
        {
            IMAGE_RESOURCE_DIRECTORY h;
            IMAGE_RESOURCE_DIRECTORY_ENTRY r; // rt group
            IMAGE_RESOURCE_DIRECTORY i;
            IMAGE_RESOURCE_DIRECTORY_ENTRY e; // lang group of id
            IMAGE_RESOURCE_DATA_ENTRY d;
        } ver;
        struct
        {
            IMAGE_RESOURCE_DIRECTORY h;
            IMAGE_RESOURCE_DIRECTORY_ENTRY r; // rt group
            IMAGE_RESOURCE_DIRECTORY i;
            IMAGE_RESOURCE_DIRECTORY_ENTRY e; // lang group of id
            IMAGE_RESOURCE_DATA_ENTRY d;
        } mfs;
        struct
        {
            IMAGE_RESOURCE_DIRECTORY h;
            IMAGE_RESOURCE_DIRECTORY_ENTRY r[MAX_ICONS]; // rt group
            struct
            {
                IMAGE_RESOURCE_DIRECTORY i;
                IMAGE_RESOURCE_DIRECTORY_ENTRY e; // lang group of id
            } o[MAX_ICONS];
            IMAGE_RESOURCE_DATA_ENTRY d[MAX_ICONS];
        } ico;
    };

    RESOURCES rsc = {0};
    ArrayDataStream ads;
    ads.Skip(sizeof(rsc));
    IMAGE_RESOURCE_DIRECTORY* d = S? (IMAGE_RESOURCE_DIRECTORY*)rcdata : (IMAGE_RESOURCE_DIRECTORY*)rcdata1;
    rsc.main.h.TimeDateStamp         = d->TimeDateStamp;
    rsc.main.h.MajorVersion          = d->MajorVersion;
    rsc.main.h.MinorVersion          = d->MinorVersion;
    rsc.main.h.NumberOfIdEntries     = 4;

    enum { Vrt = 2, Grt = 1, Ico = 0, Mfs = 3 };
    rsc.main.r[Ico].Id = 0x03; //RT_ICON
    rsc.main.r[Ico].OffsetToDirectory = (byte_t*)&rsc.ico.h - (byte_t*)&rsc.main.h;
    rsc.main.r[Ico].DataIsDirectory = 1;
    rsc.main.r[Grt].Id = 0x0e; //RT_GROUP_ICON
    rsc.main.r[Grt].OffsetToDirectory = (byte_t*)&rsc.igrp.h - (byte_t*)&rsc.main.h;
    rsc.main.r[Grt].DataIsDirectory = 1;
    rsc.main.r[Vrt].Id = 0x10; //RT_VERSION
    rsc.main.r[Vrt].OffsetToDirectory = (byte_t*)&rsc.ver.h - (byte_t*)&rsc.main.h;
    rsc.main.r[Vrt].DataIsDirectory = 1;
    rsc.main.r[Mfs].Id = 0x18; //RT_MANIFEST
    rsc.main.r[Mfs].OffsetToDirectory = (byte_t*)&rsc.mfs.h - (byte_t*)&rsc.main.h;
    rsc.main.r[Mfs].DataIsDirectory = 1;

    // RT_GROUP_ICON && RT_ICON
    rsc.igrp.h = rsc.main.h;
    rsc.igrp.h.NumberOfIdEntries = 1;
    rsc.igrp.r.Id = 121;
    rsc.igrp.r.OffsetToDirectory = (byte_t*)&rsc.igrp.i - (byte_t*)&rsc.main.h;
    rsc.igrp.r.DataIsDirectory = 1;
    rsc.igrp.i  = rsc.igrp.h;
    rsc.igrp.e.Id = 0;
    rsc.igrp.e.OffsetToData = (byte_t*)&rsc.igrp.d - (byte_t*)&rsc.main.h;

    if (S) FindRSC(0x0e,-1,rcdata,&icog_entry); // RT_GROUP_ICON
    ICOENTRY* ie = 0;
    ICOHEADER ihx = {0};
    ICOHEADER* ih = &ihx;
    rsc.ico.h = rsc.main.h;

    if ( icog_entry )
    {
        rsc.igrp.d = *icog_entry;
        rsc.igrp.d.OffsetToData = ads.Tell();
        ihx = *(ICOHEADER*)(rcdata+icog_entry->OffsetToData-rsc_va);
        unsigned foo = ih->wNumImages;
        if ( ih->wNumImages > MAX_ICONS )  ih->wNumImages = MAX_ICONS;
        rsc.ico.h.NumberOfIdEntries = ih->wNumImages;
        //ih->wNumImages = 1;
        rsc.igrp.d.Size = sizeof(ICOHEADER)+sizeof(ICOENTRY)*ih->wNumImages;
        ads.Write(rcdata+icog_entry->OffsetToData-rsc_va,rsc.igrp.d.Size);
        ih->wNumImages = foo;
        ie  = (ICOENTRY*)(rcdata+icog_entry->OffsetToData-rsc_va+sizeof(ICOHEADER));
    }
    else
    {
        //IMAGE_RESOURCE_DATA_ENTRY *g = 0;
        //FindRSC(0x0e,200,rcdata1,&g);
        //rsc.igrp.d = *g;
        //rsc.igrp.d.OffsetToData = ads.Tell();
        //ads.Write(rcdata1+g->OffsetToData-rsc_va1,g->Size);
        //ihx = *(ICOHEADER*)(rcdata1+g->OffsetToData-rsc_va1);
        //rsc.ico.h.NumberOfIdEntries = ih->wNumImages;
        //ie  = (ICOENTRY *)(rcdata1+g->OffsetToData-rsc_va1+sizeof(ICOHEADER));
    }


    for ( int in = 0; in < ih->wNumImages && in < MAX_ICONS; ++in )
    {
        IMAGE_RESOURCE_DATA_ENTRY* e = 0;

        rsc.ico.r[in].Id = ie[in].wID;
        rsc.ico.r[in].OffsetToDirectory = (byte_t*)&rsc.ico.o[in] - (byte_t*)&rsc.main.h;
        rsc.ico.r[in].DataIsDirectory = 1;
        rsc.ico.o[in].i = rsc.ico.h;
        rsc.ico.o[in].i.NumberOfIdEntries = 1;
        rsc.ico.o[in].e.Id = 0;
        rsc.ico.o[in].e.OffsetToData = (byte_t*)&rsc.ico.d[in] - (byte_t*)&rsc.main.h;
        unsigned offset_to_data = ads.Tell();

        //__asm int 3;

        ICOENTRY* iein = ie+in;
        if ( icog_entry )
        {
            FindRSC(0x03,iein->wID,rcdata,&e);
            ads.Write(rcdata+e->OffsetToData-rsc_va,e->Size);
        }
        else
        {
            FindRSC(0x03,iein->wID,rcdata1,&e);
            ads.Write(rcdata1+e->OffsetToData-rsc_va1,e->Size);
        }

        rsc.ico.d[in] = *e;
        rsc.ico.d[in].OffsetToData = offset_to_data;
    }

    // RT_VERSION
    if (S) FindRSC(0x10,-1,rcdata,&info_entry); // RT_VERSION
    rsc.ver.h = rsc.main.h;
    rsc.ver.h.NumberOfIdEntries =1;
    rsc.ver.r.Id = 1;
    rsc.ver.r.OffsetToDirectory = (byte_t*)&rsc.ver.i - (byte_t*)&rsc.main.h;
    rsc.ver.r.DataIsDirectory = 1;
    rsc.ver.i  = rsc.ver.h;
    rsc.ver.e.Id = 0;
    rsc.ver.e.OffsetToData = (byte_t*)&rsc.ver.d - (byte_t*)&rsc.main.h;

    if ( unsigned q = ads.Tell()%4 )
        for ( int i = 0; i < 4-q; ++i )
            ads.Write8(0);

    if ( info_entry )
    {
        rsc.ver.d = *info_entry;
        rsc.ver.d.OffsetToData = ads.Tell();
        ads.Write(rcdata+info_entry->OffsetToData-rsc_va,info_entry->Size);
    }
    else
    {
        char char_1[] = "aouei";
        char char_2[] = "qwrtypsdfghjklzxcvbnm";
        wchar_t* char_3[] = {L" Ltd.",L" LLC.",L" Inc."};

        StringW company;

        if ( 1 )
        {
            srand(time(0));

            for ( int q = 0; q < 2; ++q )
            {
                bool start_company_name = true;
                for ( int i = 0, j = 2+rand()%3; i < j; ++i )
                {
                    char t = char_2[rand()*(sizeof(char_2)-1)/RAND_MAX];
                    if (start_company_name)
                    {
                        if ( q ) company.Append(' ');
                        t = toupper(t);
                        start_company_name = false;
                    }
                    company.Append(t);
                    company.Append(char_1[rand()*(sizeof(char_1)-1)/RAND_MAX]);
                    if ( rand() % 1000 < 500 )
                        company.Append(char_2[rand()*(sizeof(char_2)-1)/RAND_MAX]);
                }
                if ( rand() % 1000 < 500 )
                    break;
            }

            company.Append(char_3[(rand()*3)/RAND_MAX]);
        }

        int major_rev = 1;
        int build_rev = 1;
        VS_FIXEDFILEINFO ffi = {0};
        ffi.dwSignature = 0xfeef04bdL;
        ffi.dwStrucVersion = 0x0010000L;
        ffi.dwFileOS = 0x00040004L;
        ffi.dwFileType = 0x00000001L;
        ffi.dwProductVersionMS=major_rev<<16;
        ffi.dwProductVersionLS=build_rev;
        ffi.dwFileVersionMS=major_rev<<16;
        ffi.dwFileVersionLS=build_rev;
        ffi.dwFileFlagsMask=0x3f;
        unsigned foo = 0;
        rsc.ver.d.OffsetToData = ads.Tell();
        ads.Write16le(0); // length
        ads.Write16le(0x34); // fixed info length
        ads.Write16le(0); // string information
        ads.Write(L"VS_VERSION_INFO\0",17*2); // aligned
        REQUIRE(sizeof(ffi) == 0x34);
        ads.Write(&ffi,0x34);
        // aligned, padding skipped
        unsigned string_fi_offs = ads.Tell();
        ads.Write16le(0); // length
        ads.Write16le(0); // reserved
        ads.Write16le(1); // string information
        ads.Write(L"StringFileInfo\0",15*2); //aligned
        unsigned string_fi_table = ads.Tell();
        ads.Write16le(0); // length
        ads.Write16le(0); // reserved
        ads.Write16le(1); // string information
        ads.Write(L"000004b0\0",9*2); //aligned
        WriteFFinfoString(ads,L"CompanyName",+company);
        //WriteFFinfoString(ads,L"FileDescription",L"Protected Application");
        WriteFFinfoString(ads,L"FileVersion",0|_S*L"%d, 0, 0, %d"%major_rev%build_rev);
        SYSTEMTIME stime = {0}; GetSystemTime(&stime);
        WriteFFinfoString(ads,L"LegalCopyright",0|_S*L"Copyright (c) %d, %s"%stime.wYear%company);
        WriteFFinfoString(ads,L"ProductVersion",0|_S*L"%d, 0, 0, %d"%major_rev%build_rev);
        //WriteFFinfoString(ads,L"Comments",0|_S*L"Is protected with Teggo MoleBox %d.%d" %_MAJOR_VERSION %_BUILD_NUMBER);
        foo = ads.Tell();
        ads.Seek(string_fi_table);
        ads.Write16le(foo-string_fi_table);
        ads.Seek(string_fi_offs);
        ads.Write16le(foo-string_fi_offs);
        ads.Seek(foo);
        if ( int i = foo&3 ) for (; 4-i; ++i) ads.Write8(0);
        unsigned var_fi_offs = ads.Tell();
        ads.Write16le(0); // length
        ads.Write16le(0); // reserved
        ads.Write16le(1); // string information
        ads.Write(L"VarFileInfo\0",13*2); //aligned
        unsigned var_fi_table = ads.Tell();
        ads.Write16le(0); // length
        ads.Write16le(4); // data length
        ads.Write16le(0); // binary information
        ads.Write(L"Translation\0",13*2); //aligned
        ads.Write32le(0x04b00000);
        foo = ads.Tell();
        ads.Seek(var_fi_table);
        ads.Write16le(foo-var_fi_table);
        ads.Seek(var_fi_offs);
        ads.Write16le(foo-var_fi_offs);
        ads.Seek(rsc.ver.d.OffsetToData);
        rsc.ver.d.Size = foo-rsc.ver.d.OffsetToData;
        ads.Write16le(rsc.ver.d.Size);
        ads.Seek(foo);
    }

    rsc.mfs.r.Id = 1;
    if (S) FindRSC(0x18,-1,rcdata,&mfs_entry,&rsc.mfs.r.Id); // RT_MANIFEST
    rsc.mfs.h = rsc.main.h;
    rsc.mfs.h.NumberOfIdEntries = 1;
    rsc.mfs.r.OffsetToDirectory = (byte_t*)&rsc.mfs.i - (byte_t*)&rsc.main.h;
    rsc.mfs.r.DataIsDirectory = 1;
    rsc.mfs.i  = rsc.mfs.h;
    rsc.mfs.e.Id = 0;
    rsc.mfs.e.OffsetToData = (byte_t*)&rsc.mfs.d - (byte_t*)&rsc.main.h;
    rsc.mfs.d.OffsetToData = ads.Tell();

    if ( mfs_entry )
    {
        //rsc.mfs.d = *mfs_entry;
        StringA filtered;
        FilterManifest(rcdata+mfs_entry->OffsetToData-rsc_va,mfs_entry->Size,filtered);
        rsc.mfs.d.Size = filtered.Length();
        ads.Write(+filtered,filtered.Length());
        //ads.Write(rcdata+mfs_entry->OffsetToData-rsc_va,mfs_entry->Size);
        /*ads.Write("\0\0\0\0",4);
        BufferT<byte_t> packed(mfs_entry->Size+128);
        int sz = zlib_compress(rcdata+mfs_entry->OffsetToData-rsc_va,mfs_entry->Size,+packed,packed.Count());
        ads.Write32le(sz);
        ads.Write32le(mfs_entry->Size);
        //Md5Encrypt32(mfs_entry->Size,+packed,packed.Count());

        if (sz > 0)
          {
            byte_t digest[16] = {0};
            Md5sign(&rsc.mfs.d.Size,sizeof(rsc.mfs.d.Size),&digest);
            NEWDES_Cipher cipher;
            cipher.SetupEncipher(digest);
            cipher.DoCipherCBCI(+packed,sz/8);
            ads.Write(+packed,sz);
          }

        ulong_t r_size = ads.Tell() - rsc.mfs.d.OffsetToData;
        if ( r_size < mfs_entry->Size )
          for ( int i = 0, j = mfs_entry->Size - r_size; i < j; ++i )
            ads.Write8(0);
        */
    }
    else
    {
        rsc.mfs.d.Size = sizeof(predefined_manifest);
        ads.Write(predefined_manifest,sizeof(predefined_manifest));
        //ads.Write32le(0);
    }

    ads.Seek(0);
    ads.Write(&rsc,sizeof(rsc));
    ads.SwapBuffer(data);
    return true;
}

void RscFixup_(byte_t* rsrc,unsigned rsrc_rva,IMAGE_RESOURCE_DIRECTORY* d )
{
    IMAGE_RESOURCE_DIRECTORY_ENTRY* r = (IMAGE_RESOURCE_DIRECTORY_ENTRY*)((byte_t*)d + sizeof(*d));
    IMAGE_RESOURCE_DIRECTORY_ENTRY* rE = r+d->NumberOfIdEntries+d->NumberOfNamedEntries;
    for ( ; r != rE; ++r )
        if ( r->DataIsDirectory )
            RscFixup_(rsrc,rsrc_rva,(IMAGE_RESOURCE_DIRECTORY*)(rsrc+r->OffsetToDirectory));
        else
        {
            IMAGE_RESOURCE_DATA_ENTRY* e = (IMAGE_RESOURCE_DATA_ENTRY*)(rsrc+r->OffsetToData);
            //printf("%08x,%08x\n",r->NameOffset,e->OffsetToData);
            e->OffsetToData += rsrc_rva;
        }
}

bool RscFixup(byte_t* rsrc,unsigned rsrc_rva )
{
    IMAGE_RESOURCE_DIRECTORY* d = (IMAGE_RESOURCE_DIRECTORY*)rsrc;
    RscFixup_(rsrc,rsrc_rva,d);
    return true;
}

bool TakeOriginalResources(byte_t* mapped, IMAGE_NT_HEADERS* nth, BufferT<byte_t>& data)
{
    unsigned rsc_va = nth->OptionalHeader.DataDirectory[IMAGE_DIRECTORY_ENTRY_RESOURCE].VirtualAddress;
    unsigned rsc_ln = nth->OptionalHeader.DataDirectory[IMAGE_DIRECTORY_ENTRY_RESOURCE].Size;
    IMAGE_SECTION_HEADER* S = IMAGE_FIRST_SECTION(nth);
    IMAGE_SECTION_HEADER* Se = S+nth->FileHeader.NumberOfSections;

    do
        if ( S->VirtualAddress <= rsc_va && S->VirtualAddress+S->SizeOfRawData > rsc_va )
            break;
    while ( ++S != Se );

    if ( S == Se ) S = 0;

    if ( S )
    {
        int q = S->SizeOfRawData;
        if ( q > S->Misc.VirtualSize && S->Misc.VirtualSize && q-S->Misc.VirtualSize < 0x1000)
        {
            q = S->Misc.VirtualSize;
        }
        data.Insert(0,mapped+S->PointerToRawData,q);
        RscFixup(+data,-S->VirtualAddress);
    }

    return true;
}
