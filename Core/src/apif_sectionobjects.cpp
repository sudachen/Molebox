
/*

  Copyright (c) 2008-2009, Alexey Sudachen, alexey@sudachen.name

*/

#include "_common.h"
#include "apif.h"
#include "import.h"
#include "splicer.h"
#include "logger.h"
#include "hoarray.h"
#include "vfs.h"
#include "XorS.h"
#include "xnt.h"

struct NT_CLOSE_ARGS
{
    HANDLE               FileHandle;
};

struct NT_CREATESECT_ARGS
{
    HANDLE*               SectionHandle;
    ULONG                DesiredAccess;
    OBJECT_ATTRIBUTES*    ObjectAttributes;
    LARGE_INTEGER*        MaximumSize;
    ULONG                PageAttributes;
    ULONG                SectionAttributes;
    HANDLE               FileHandle;
};

struct NT_MAPSECTION_ARGS
{
    HANDLE               SectionHandle;
    HANDLE               ProcessHandle;
    VOID**                 BaseAddress;
    ULONG                ZeroBits;
    ULONG                CommitSize;
    LARGE_INTEGER*        SectionOffset;
    ULONG*                ViewSize;
    SECTION_INHERIT      InheritDisposition;
    ULONG                AllocationType;
    ULONG                Protect;
};

struct NT_UNMAPSECTION_ARGS
{
    HANDLE               ProcessHandle;
    VOID*                 BaseAddress;
};

struct NT_QUESECTION_ARGS
{
    HANDLE               SectionHandle;
    SECTION_INFORMATION_CLASS InformationClass;
    VOID*                 InformationBuffer;
    ULONG                InformationBufferSize;
    ULONG*                ResultLength;
};

IMAGE_NT_HEADERS* GET_NT_HEADERS(void* p)
{
    return (IMAGE_NT_HEADERS*)((byte_t*)p + ((IMAGE_DOS_HEADER*)p)->e_lfanew);
}

extern "C" void Regster_Clear();

struct SectionObject : Refcounted
{
    FioProxyPtr proxy_;
    HANDLE section_;
    u32_t size_;
    u32_t access_;
    u32_t type_; // SEC_IMAGE or SEC_FILE
    u32_t preferred_base_;
    byte_t*  mapped_;

    BufferT<Tuple2<void*,u32_t>> ranges_;

    bool IsValid() { return proxy_->IsValid(); }
    pchar_t PackageName() { return proxy_->PackageName(); }
    pwide_t ComposePath(BaseStringT<wchar_t>& b = (BaseStringT<wchar_t>&)StringT<wchar_t,128>())
    { return proxy_->ComposePath(b); }
    pwide_t ComposePath1(BaseStringT<wchar_t>& b = (BaseStringT<wchar_t>&)StringT<wchar_t,128>())
    { return proxy_->ComposePath1(b); }

    HANDLE Section() { return section_; }
    u32_t  Size()    { return size_; }
    u32_t  Access()  { return access_; }
    u32_t  Type()    { return type_; }

    FioProxy* Proxy() { return +proxy_; }

    SectionObject(FioProxyPtr p, u32_t size, u32_t access, u32_t type)
        : proxy_(p), section_(0), size_(size), access_(access), type_(type), mapped_(0)
    {
        if ( type == SEC_IMAGE )
            size_ = 0,
            access_ = 0;
    }

    ~SectionObject()
    {
        if ( section_ )
            NtClose(section_);
    }

    bool Create()
    {
        u32_t desired_access;
        LARGE_INTEGER max_size;
        u32_t page_attributes;
        u32_t section_attributes;

        if ( type_ == SEC_IMAGE )
        {
            IMAGE_NT_HEADERS nthdr = {0};
            if ( proxy_->ReadImageNtHeaders(&nthdr) )
            {
                XLOG|_S*"<APIF/VFS> failed to read PE HEADERS file '%s\\%s'"
                %proxy_->PackageName()
                %proxy_->ComposePath1();
                return false;
            }
            size_              = nthdr.OptionalHeader.SizeOfImage;
            preferred_base_    = nthdr.OptionalHeader.ImageBase;
            desired_access     = SECTION_MAP_READ | SECTION_MAP_EXECUTE | SECTION_MAP_WRITE;
            page_attributes    = PAGE_EXECUTE_READWRITE;
            section_attributes = SEC_RESERVE;
        }
        else
        {
            if ( size_ == 0 )  size_ = proxy_->Size();
            preferred_base_    = 0;
            desired_access     = SECTION_MAP_READ | SECTION_MAP_EXECUTE | SECTION_MAP_WRITE;
            section_attributes = SEC_RESERVE;
            page_attributes    = PAGE_EXECUTE_READWRITE;
        }

        max_size.QuadPart = size_;

        NTSTATUS st = NtCreateSection(&section_,desired_access,0,&max_size,page_attributes,section_attributes,0);

        if ( st )
        {
            //MessageBoxA(0,"!","!",0);
            XLOG|_S*"<APIF/VFS> failed to create section object for file '%s\\%s', status %08x"
            %proxy_->PackageName()
            %proxy_->ComposePath1()
            %st;
            return false;
        }

        return true;
    }

    u32_t MapView(u32_t offset, void** p, unsigned long* viewsize)
    {
        NTSTATUS st;
        bool not_at_base = false;
        u32_t size = 0;

        if ( !proxy_->IsValid() )
        {
            return STATUS_INVALID_HANDLE;
        }

        bool first_map = false;

        if ( !mapped_ || !*p )
        {
            first_map = true;

            if ( type_ == SEC_IMAGE )
            {
                mapped_ = (byte_t*)preferred_base_,
                size    = size_;
                if ( viewsize ) *viewsize = size;
            }
            else
            {
                mapped_ = (byte_t*)*p;// - offset;
                if ( viewsize ) size = *viewsize;
                if ( !size || size+offset > size_ ) size = size_-offset;
                if ( viewsize ) *viewsize = size;
            }

        ntmap:
            st = NtMapViewOfSection(
                     section_,
                     INVALID_HANDLE_VALUE,
                     (void**)&mapped_,
                     0,
                     0,
                     0,
                     viewsize,
                     ViewUnmap,
                     0,
                     PAGE_EXECUTE_READWRITE
                 );

            if ( st == STATUS_CONFLICTING_ADDRESSES )
            { mapped_ = 0; not_at_base = true; goto ntmap; }

            if ( !NT_SUCCESS(st) )
            {
                XLOG|_S*"<APIF/VFS> failed to allocate swap area for file '%s\\%s', status %08x"
                %proxy_->PackageName()
                %proxy_->ComposePath1()
                %st;
                mapped_ = 0;
                return st;
            }
            st = NtAllocateVirtualMemory(
                     INVALID_HANDLE_VALUE,
                     (void**)&mapped_,
                     0,
                     viewsize,
                     MEM_COMMIT,
                     //PAGE_READ|PAGE_WRITE|PAGE_EXECUTE
                     PAGE_EXECUTE_READWRITE);

            if ( !NT_SUCCESS(st) )
            {
                XLOG|_S*"<APIF/VFS> failed to commit memory for file '%s\\%s', status %08x"
                %proxy_->PackageName()
                %proxy_->ComposePath1()
                %st;
                return st;
            }
        }

        if ( type_ == SEC_IMAGE && first_map )
        {
            IMAGE_NT_HEADERS* nt;
            u32_t wasread = 0;
            if ( 0 != proxy_->Read(0,mapped_,8*1024,&wasread) )
            {
                XLOG|_S*"<APIF/VFS> failed to read from file '%s\\%s'"
                %proxy_->PackageName()
                %proxy_->ComposePath1();
                return STATUS_INVALID_HANDLE;
            }
            nt = GET_NT_HEADERS(mapped_); // if we have created this section, so it's PE image
            IMAGE_SECTION_HEADER* sec = IMAGE_FIRST_SECTION(nt);
            for ( int i = 0; i<nt->FileHeader.NumberOfSections; ++i )
            {
                if ( sec[i].PointerToRawData && sec[i].SizeOfRawData )
                    if ( 0 != proxy_->Read(
                             sec[i].PointerToRawData,
                             mapped_+sec[i].VirtualAddress,
                             sec[i].SizeOfRawData,
                             &wasread) )
                    {
                        XLOG|_S*"<APIF/VFS> failed to read section %d from file '%s\\%s'"
                        %i
                        %proxy_->PackageName()
                        %proxy_->ComposePath1();
                        return STATUS_INVALID_HANDLE;
                    }
            }
        }
        else if ( type_ != SEC_IMAGE && first_map )
        {
            //MessageBoxA(0,"","",0);
            u32_t wasread = 0;
            XLOG|_S*"reading %d bytes"%size;
            if ( 0 != proxy_->Read(offset,mapped_,size,&wasread) )
            {
                XLOG|_S*"<APIF/VFS> failed to read from file '%s\\%s'"
                %proxy_->PackageName()
                %proxy_->ComposePath1();
                return STATUS_INVALID_HANDLE;
            }
        }
        *p = mapped_;
        return not_at_base?STATUS_IMAGE_NOT_AT_BASE:0;
    }
};

typedef rcc_ptr<SectionObject> SectionObjectPtr;

inline void Delete(SectionObject* p) { p->Release(); }
typedef HandledObjectsArray<SectionObject,u32_t> HOA;
typedef FlatmapT<void*, SectionObjectPtr> SUBJECT;

extern FioProxy* GetProxyOfApifFileHandle(HANDLE);

struct SectionObjectFilter : APIfilter
{

    HOA handles_;
    SUBJECT subject_;
    Tlocker l_;

    SectionObjectFilter() {};

    APIF_STATUS DoCloseObject(void* _a, unsigned* result)
    {
        NT_CLOSE_ARGS* args = (NT_CLOSE_ARGS*)_a;
        //XDBG|_S*_XOr(".apif.CloseObject %08x",23,299176021) % args->FileHandle;
        if ( HOA::R o = handles_.Get(args->FileHandle) )
        {
            if (o)
            {
                __lockon__(VFS)
                if ( o->IsValid() )
                    XLOG|_S* _XOr("<APIF/VFS> close section (%08x) '%s\\%s'",40,298455114)
                    %args->FileHandle
                    %o->PackageName()
                    %o->ComposePath();
                handles_.Close(args->FileHandle);
                *result = 0;
            }
            else
            { *result = STATUS_INVALID_HANDLE; }
            return APIF_RETURN;
        }
        else
            return APIF_CONTINUE;
    }


    APIF_STATUS DoCreateSection(void* _a, unsigned* result)
    {
        NT_CREATESECT_ARGS* args = (NT_CREATESECT_ARGS*)_a;
        XDBG|_S* _XOr(".apif.CreateSection file %08x, size %08x, page %08x, type %08x",63,295178648)
        %args->FileHandle
        %(args->MaximumSize?args->MaximumSize->LowPart:0)
        %args->PageAttributes
        %args->SectionAttributes;

        //if ( !args->FileHandle && !args->ObjectAttributes->ObjectName )
        //  __asm int 3
        //  ;

        if ( args->FileHandle )
            __lockon__(VFS)
            if ( FioProxy* p = GetProxyOfApifFileHandle(args->FileHandle) )
            {
                if ( p->IsValid() )
                {
                    //__asm int 3;
                    if ( HANDLE h = p->QueryMySelf() )
                    {
                        args->FileHandle = h;
                        return APIF_CONTINUE;
                    }
                    if ( p->IsExecutable() && (args->SectionAttributes&SEC_IMAGE) )
                    {
                        *result = NtCreateSection(
                                      args->SectionHandle,args->DesiredAccess,args->ObjectAttributes,
                                      args->MaximumSize,args->PageAttributes,args->SectionAttributes,
                                      VFS->SelfHandle());
                        if ( !result )
                        {
                            // write info about real executable
                        }
                        return APIF_RETURN;
                    }
                    u32_t size=0,access=0,type;
                    if ( args->SectionAttributes&SEC_IMAGE ) type = SEC_IMAGE;
                    else type = SEC_FILE;
                    SectionObjectPtr so = RccPtr(new SectionObject(RccRefe(p),size,access,type));
                    if ( so->Create() )
                        *args->SectionHandle = handles_.Put(Refe(+so)),
                               *result = 0;
                    else
                        *args->SectionHandle = 0,
                               *result = STATUS_INVALID_HANDLE;
                    XLOG|_S* _XOr("<APIF/VFS> create section for '%s\\%s', attr %08x, size %08x -> %08x,%08x",73,291115482)
                    %p->PackageName()
                    %p->ComposePath()
                    %args->SectionAttributes
                    %size
                    %*args->SectionHandle
                    %*result;
                }
                else
                    *result = STATUS_INVALID_HANDLE;
                return APIF_RETURN;
            }
        return APIF_CONTINUE;
    }

    APIF_STATUS DoMapSection(void* _a, unsigned* result)
    {
        NT_MAPSECTION_ARGS* args = (NT_MAPSECTION_ARGS*)_a;
        //__asm int 3
        ;
        XDBG|_S* _XOr(".apif.MapSection %08x, base %08x, offset %08x, size %08x, protect %08x",71,285806857)
        %args->SectionHandle
        %*args->BaseAddress
        %(args->SectionOffset?args->SectionOffset->LowPart:0)
        %(args->ViewSize?*args->ViewSize:0)
        %args->Protect;
        if ( HOA::R o = handles_.Get(args->SectionHandle) )
        {
            if (o)
                __lockon__(VFS)
            {
                *result = o->MapView(
                              (args->SectionOffset?args->SectionOffset->LowPart:0),
                              args->BaseAddress,
                              args->ViewSize);
                XLOG|_S* 
		   _XOr("<APIF/VFS> mmap section (%08x) '%s\\%s' at %08x - %08x -> %08x",62,317857458) 
				%args->SectionHandle %o->PackageName() %o->ComposePath() 
				%*args->BaseAddress 
				%(args->ViewSize?*args->ViewSize:o->Size())
                %*result;
                if ( NT_SUCCESS(*result) )
                    subject_.Put(*args->BaseAddress,RccRefe(+o));
            }
            else
                *result = STATUS_INVALID_HANDLE;
            return APIF_RETURN;
        }
        else
        {
            //if ( (args->Protect&0x0ff) == PAGE_EXECUTE || (args->Protect&0x0ff) == PAGE_EXECUTE_WRITECOPY )
            if ( 1)
            {
                //XLOG|_S*"alloc type %d" %args->AllocationType;
                //XLOG|_S*"protect matched";
                *result = NtMapViewOfSection(args->SectionHandle,
                                             args->ProcessHandle,
                                             args->BaseAddress,
                                             args->ZeroBits,
                                             args->CommitSize,
                                             args->SectionOffset,
                                             args->ViewSize,
                                             args->InheritDisposition,
                                             args->AllocationType,
                                             args->Protect);
                if ( SUCCEEDED(*result) )
                {
                    //XLOG|_S*"succeeded";
                    //__asm int 3;
                    byte_t* b = (byte_t*)*args->BaseAddress;
                    MEMORY_BASIC_INFORMATION memi = {0};
                    long res = NtQueryVirtualMemory((void*)-1,b,0,&memi,sizeof(memi),0);

                    if ( res || memi.RegionSize < 4000 || memi.State != MEM_COMMIT )
                        return APIF_RETURN;

                    IMAGE_DOS_HEADER* dos = (IMAGE_DOS_HEADER*)b;
                    if ( dos->e_magic == IMAGE_DOS_SIGNATURE && dos->e_lfanew < 4000 )
                    {
                        IMAGE_NT_HEADERS* nth = (IMAGE_NT_HEADERS*)(b+dos->e_lfanew);
                        if ( nth->Signature == IMAGE_NT_SIGNATURE )
                        {
                            static unsigned sh4v[] = { 0x88524348, 0xfdb28eaf, 0x5dfa64fb, 0x9fbbbbc2 };

                            byte_t md5_sign[16] = {0};
                            MD5_Hash md5;
                            IMAGE_SECTION_HEADER* sec = IMAGE_FIRST_SECTION(nth);
                            if ( (byte_t*)sec - b > 0 && (byte_t*)sec - b < 4000 )
                            {
                                byte_t c = 0;
                                //XLOG|_S*"section name %8s" %(char*)sec->Name;

                                c = toupper(sec->Name[0]);
                                md5.Update(&c,1);
                                c = toupper(sec->Name[1]);
                                md5.Update(&c,1);
                                c = toupper(sec->Name[2]);
                                md5.Update(&c,1);
                                c = toupper(sec->Name[3]);
                                md5.Update(&c,1);
                                c = toupper(sec->Name[4]);
                                md5.Update(&c,1);
                                c = toupper(sec->Name[5]);
                                md5.Update(&c,1);
                                c = toupper(sec->Name[6]);
                                md5.Update(&c,1);
                                md5.Finalize(md5_sign);

                                /*XLOG|_S*"md5 %08x %08x %08x %08x"
                                  %((unsigned*)md5_sign)[0]
                                  %((unsigned*)md5_sign)[1]
                                  %((unsigned*)md5_sign)[2]
                                  %((unsigned*)md5_sign)[3];

                                XLOG|_S*"shv4 %08x %08x %08x %08x"
                                  %((unsigned*)sh4v)[0]
                                  %((unsigned*)sh4v)[1]
                                  %((unsigned*)sh4v)[2]
                                  %((unsigned*)sh4v)[3]; */

                                if ( memcmp(md5_sign,sh4v,16) == 0 )
                                {
                                    //XLOG|"INJECTR!!! ALERT! ALERT! ALERT!";
                                    //ExitProcess(-1);
                                    Regster_Clear();
                                }
                            }
                        }
                    }
                }
                return APIF_RETURN;
            }
            else
                return APIF_CONTINUE;
        }
    }

    APIF_STATUS DoQuerySection(void* _a, unsigned* result)
    {
        NT_QUESECTION_ARGS* args = (NT_QUESECTION_ARGS*)_a;
        //XDBG|_S*_XOr(".apif.MapSection %08x, base %08x, protect %08x",47,314449662) %;
        //__asm int 3
        ;
        if ( HOA::R o = handles_.Get(args->SectionHandle) )
        {
            XLOG|_S* _XOr("<APIF/VFS> query section (%08x) '%s\\%s', Ic = %d",49,306912971)
            %args->SectionHandle %o->PackageName() %o->ComposePath() %args->InformationClass;
            if (o)
                __lockon__(VFS) if ( o->IsValid() )
                {
                    if ( args->InformationClass == SectionImageInformation )
                    {
                        SECTION_IMAGE_INFORMATION* info = (SECTION_IMAGE_INFORMATION*)args->InformationBuffer;
                        memset(info,0,args->InformationBufferSize);
                        IMAGE_NT_HEADERS h = {0};
                        o->Proxy()->ReadImageNtHeaders(&h);
                        info->TransferAddress = (void*)(h.OptionalHeader.ImageBase+h.OptionalHeader.AddressOfEntryPoint);
                        info->MaximumStackSize = h.OptionalHeader.SizeOfStackReserve;
                        info->CommittedStackSize = h.OptionalHeader.SizeOfStackCommit;
                        info->SubSystemType = h.OptionalHeader.Subsystem;
                        info->SubSystemMajorVersion = h.OptionalHeader.MajorSubsystemVersion;
                        info->SubSystemMinorVersion = h.OptionalHeader.MinorSubsystemVersion;
                        info->ImageCharacteristics = h.FileHeader.Characteristics;
                        info->DllCharacteristics = h.OptionalHeader.DllCharacteristics;
                        info->Machine = h.FileHeader.Machine;
                        info->ImageContainsCode = 1;
                        info->Reserved[0] = 0x1b000;
                        *result = 0;
                    }
                    else if ( args->InformationClass == SectionBasicInformation )
                    {
                        SECTION_BASIC_INFORMATION* info = (SECTION_BASIC_INFORMATION*)args->InformationBuffer;
                        memset(info,0,args->InformationBufferSize);
                        *result = 0;
                    }
                    else
                        *result = STATUS_INVALID_HANDLE;
                }
                else
                    *result = STATUS_INVALID_HANDLE;
            return APIF_RETURN;
        }
        else
            return APIF_CONTINUE;
    }

    APIF_STATUS DoUnmapSection(void* _a, unsigned* result)
    {
        NT_UNMAPSECTION_ARGS* args = (NT_UNMAPSECTION_ARGS*)_a;
        __lockon__(VFS)
        if ( SectionObjectPtr* o = subject_.Get(args->BaseAddress) )
        {
            XLOG|_S* _XOr("<APIF/VFS> unmmap section '%s\\%s' at %08x",42,301997568)
            %(*o)->Proxy()->PackageName() %(*o)->Proxy()->ComposePath()
            %args->BaseAddress
            ;
            subject_.Erase(args->BaseAddress);
        }

        return APIF_CONTINUE;
    }

    virtual APIF_STATUS DoCall(int apif_id, void* args, unsigned* result)
    {
        switch (apif_id)
        {
            case APIF_CLOSE:
                return DoCloseObject(args,result);
            case APIF_CREATESECT:
                return DoCreateSection(args,result);
            case APIF_MAPSECT:
                return DoMapSection(args,result);
            case APIF_UNMAPSECT:
                return DoUnmapSection(args,result);
            case APIF_QUESECTINFO:
                return DoQuerySection(args,result);
        }
        return APIF_CONTINUE;
    }

    SectionObjectFilter(SectionObjectFilter const&);
};

SectionObjectFilter apif_sobjfilt = SectionObjectFilter();

void APIF_RegisterSectionObjectHooks()
{
    APIF->Push(&apif_sobjfilt);
};
