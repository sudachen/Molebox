
/*

Copyright (c) 2010, Alexey Sudachen, alexey@sudachen.name

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
#include "xojuman.h"

#include "../loader.lso.BIN.c"

extern void* Query_Core(int* size);
extern void* Query_Core_Handle(int* size);

extern bool g_Inject_Into_Child;
extern u32_t g_Injector_Master_Id;
extern int g_Inject_Flags;
extern StringW g_Registery_Mask;

template <class tC> struct STI { typedef LPSTARTUPINFOA T; };
template<> struct STI<wchar_t> { typedef LPSTARTUPINFOW T; };

#define CRPROCARGS(tC) \
    const tC *lpApplicationName,\
    tC *lpCommandLine,\
    LPSECURITY_ATTRIBUTES lpProcessAttributes,\
    LPSECURITY_ATTRIBUTES lpThreadAttributes,\
    BOOL bInheritHandles,\
    DWORD dwCreationFlags,\
    LPVOID lpEnvironment,\
    const tC *lpCurrentDirectory,\
    STI<tC>::T lpStartupInfo,\
    LPPROCESS_INFORMATION lpProcessInformation

typedef HANDLE (__stdcall* tCreateProcessA)(CRPROCARGS(char));
typedef HANDLE (__stdcall* tCreateProcessW)(CRPROCARGS(wchar_t));

template <class tC> struct tCRPROC_ARGS
{
    const tC*              lpApplicationName;
    tC*                    lpCommandLine;
    LPSECURITY_ATTRIBUTES lpProcessAttributes;
    LPSECURITY_ATTRIBUTES lpThreadAttributes;
    BOOL                  bInheritHandles;
    DWORD                 dwCreationFlags;
    LPVOID                lpEnvironment;
    const tC*              lpCurrentDirectory;
    typename STI<tC>::T   lpStartupInfo;
    LPPROCESS_INFORMATION lpProcessInformation;
};

typedef tCRPROC_ARGS<char>    CRPROCA_ARGS;
typedef tCRPROC_ARGS<wchar_t> CRPROCW_ARGS;

struct WINEXEC_ARGS
{
    LPCSTR  lpCmdLine;
    UINT    uCmdShow;
};

struct CrProcFilter : APIfilter
{
    FioProxyPtr FindEmbeddedModule(StringW& appname, StringW& cmdline, StringW& out_appname)
    {
        //XLOG|_S*"appname: no found!";
        //return FioProxyPtr(0);

        if ( appname )
        {
            out_appname = appname;
        l:
            if ( VFS->containerName.EqualI(+out_appname) )
            {
                XLOG|_S*"appname: '%s'" %out_appname;
                if ( FioProxyPtr p = VFS->GetProxy(+out_appname) )
                {
                    XLOG|_S*"found %s" % (p->IsExecutable()?"executable":"nonexecutable");
                    //if ( p->IsExecutable() )
                    return p;
                }
            }
        }
        else
        {
            static wchar_t** (__stdcall *fCommandLineToArgvW)(wchar_t*,int*) = 0;
            int argv = 0;
            if ( !fCommandLineToArgvW )
            {
                HMODULE shell32 = LoadLibrary(_XOr("shell32.dll",12,46073239));
                *(void**)&fCommandLineToArgvW = GetProcAddressIndirect(shell32,_XOr("CommandLineToArgvW",19,42534281),0);
                if ( !fCommandLineToArgvW )
                    __asm int 3;
            }
            XLOG|_S*"cmdline: '%s'" %cmdline;
            if ( wchar_t** argc = fCommandLineToArgvW((wchar_t*)+cmdline,&argv) )
            {
                out_appname = argc[0];
                goto l;
            }
        }

        return FioProxyPtr(0);
    }

    #if 0
    bool CreateInternallExecutable(FioProxyPtr p, CRPROCW_ARGS* a, StringW& appname, unsigned* result)
    {
        XLOG|_S* _XOr("<APIF/CRP> img: '%s\\%s', cmdl: '%s'",36,48367010)
        //%(a->lpApplicationName?a->lpApplicationName:L"-")
        %p->PackageName()
        %p->ComposePath()
        %(a->lpCommandLine?a->lpCommandLine:L"-");

        //__asm int 3;

        BufferT<wchar_t> appnameS;
        BufferT<wchar_t> environment;
        //PROCESS_FIXUP fixup;
        IMAGE_NT_HEADERS nth = {0};

        p->ReadImageNtHeaders(&nth);
        //fixup.SubSystem = nth.OptionalHeader.Subsystem;

        {
            wchar_t* fp = 0;
            unsigned l = GetFullPathNameW(+appname,0,0,0);
            appnameS.Resize(l+1);
            GetFullPathNameW(+appname,l,+appnameS,&fp);
            appnameS.Resize(StrLen(+appnameS));
            //appnameS[0] = 0x8000|wchar_t(appnameS.Count()*2-2);
        }

        //appnameS.Last() = ' ';
        //appnameS.Insert(appnameS.End(),a->lpCommandLine,StrLen(a->lpCommandLine)+1);

        {
            if ( a->lpEnvironment )
            {
                if ( a->dwCreationFlags & CREATE_UNICODE_ENVIRONMENT )
                {
                    pwide_t p,pS;
                    p = pS = (pwide_t)a->lpEnvironment;
                    while (*p || p[1] ) ++p; p+=2;
                    if ( pS[0] != 0 )
                        environment.Insert(0,pS,p);
                }
                else
                {
                    pchar_t p,pS;
                    p = pS = (pchar_t)a->lpEnvironment;
                    while (*p || p[1] ) ++p; p+=2;
                    if ( pS[0] != 0 )
                    {
                        environment.Resize(p-pS,0);
                        memcpy(+environment,pS,environment.Count());
                        teggo::InplaceCharToWide(+environment,environment.Count(),false);
                    }
                }
            }

            if ( !environment.Count() )
            {
                pwide_t pS, p;
                p = pS = (pwide_t)NtCurrentPeb()->ProcessParameters->Environment;
                while (*p || p[1] ) ++p; p+=2;
                environment.Insert(0,pS,p);
            }

            XOJUMAN_STARTUP_INFO st_info = {0};
            st_info.subsystem = nth.OptionalHeader.Subsystem;
            wcsncpy(st_info.container,+VFS->containerName,256);
            wcsncpy(st_info.appname,+appnameS,256);
            wcsncpy(st_info.cmdline,a->lpCommandLine?a->lpCommandLine:L"",256);

            StringW cmdl = _S*L"%s=%s" %+_XOr("XOJUMAN-CMDLINE",16,46728649) %(a->lpCommandLine?a->lpCommandLine:L"");
            environment.Insert(0,+cmdl,cmdl.Length()+1);
            StringW relnam = _S*L"%s=%s" %+_XOr("XOJUMAN-APPNAME",16,52889087) %+appnameS;
            environment.Insert(0,+relnam,relnam.Length()+1);
            StringW subsys = _S*L"%s=%d" %+_XOr("XOJUMAN-SUBSYS",15,53675283) %nth.OptionalHeader.Subsystem;
            environment.Insert(0,+subsys,subsys.Length()+1);
            StringW enval = _S*L"%s=%08x%08x" %+_XOr("XOJUMAN-STINFO",15,51119364) %GetCurrentProcessId() %&st_info;
            environment.Insert(0,+enval,enval.Length()+1);
            StringW enval = _S*L"%s=%s" %+_XOr("XOJUMAN",8,51971385) %VFS->containerName;
            environment.Insert(0,+enval,enval.Length()+1);
            environment.Push(0);
            environment.Push(0);

            //TlsSetValue(proc_fixup,&fixup);

            //__asm int 3;
            *result = (unsigned)((tCreateProcessW)apif_realcall[APIF_KCRPROCW])(
                          +VFS->containerName,
                          a->lpCommandLine,
                          //+appnameS,
                          a->lpProcessAttributes,
                          a->lpThreadAttributes,
                          a->bInheritHandles,
                          a->dwCreationFlags|CREATE_UNICODE_ENVIRONMENT,
                          +environment,
                          a->lpCurrentDirectory,
                          a->lpStartupInfo,
                          a->lpProcessInformation
                      );

            return !!*result;
        }
    }
    #endif

    void InheritPackages(XOJUMAN_STARTUP_INFO* sti)
    {
        int max_count = VFS->GetPkgsCount();
        for ( int i = 0; i < max_count; ++i )
        {
            if ( sti->pkg_count == XOJUMAN_MAX_PACKAGE_COUNT )
                break;
            svfs::Package* pkg = VFS->GetPackage(i);
            HANDLE ph = pkg->ds_->GetHandle('HNDL');
            if ( ph )
            {
                sti->pkg[sti->pkg_count].handle = ph;
                sti->pkg[sti->pkg_count].offs = pkg->offset_;
                sti->pkg[sti->pkg_count].opts = 0;
                memcpy(sti->pkg[sti->pkg_count].pwd,pkg->pwd_,16);
                strncpy(sti->pkg[sti->pkg_count].name,+pkg->name_,XOJUMAN_MAX_PACKAGE_NAME_LEN);
                ++sti->pkg_count;
            }
        }
    }

    void DupHandle(HANDLE* h, HANDLE pH)
    {
        HANDLE src = *h;
        DuplicateHandle(GetCurrentProcess(),src,pH,h,0,0,DUPLICATE_SAME_ACCESS);
    }

    bool InjectExecutable(FioProxyPtr p, CRPROCW_ARGS* a, StringW& appname, unsigned* result)
    {
        if ( p )
        {
            XLOG|_S* _XOr("<APIF/CRP> internal: '%s\\%s', cmdl: '%s'",41,57410852)
            //%(a->lpApplicationName?a->lpApplicationName:L"-")
            %p->PackageName()
            %p->ComposePath()
            %(a->lpCommandLine?a->lpCommandLine:L"-");
        }
        else
        {
            XLOG|_S* _XOr("<APIF/CRP> inject: '%s'",24,56034639)
            %(a->lpCommandLine?a->lpCommandLine:L"-");
        }

        BufferT<wchar_t> appnameS;

        if ( 1 )
        {
            wchar_t* fp = 0;
            unsigned l = GetFullPathNameW(+appname,0,0,0);
            appnameS.Resize(l+1);
            GetFullPathNameW(+appname,l,+appnameS,&fp);
            appnameS.Resize(StrLen(+appnameS));
        }

        IMAGE_NT_HEADERS pnth = {0};

        if ( 1 )
        {
            XOJUMAN_STARTUP_INFO st_info = {0};

            if ( p )
            {
                p->ReadImageNtHeaders(&pnth);
                st_info.subsystem = pnth.OptionalHeader.Subsystem;
            }

            wcsncpy(st_info.container,+VFS->containerName,XOJUMAN_MAX_PATH_LEN);
            wcsncpy(st_info.cmdline,a->lpCommandLine?a->lpCommandLine:L"",XOJUMAN_MAX_PATH_LEN);
            wcsncpy(st_info.regmask,+g_Registery_Mask,XOJUMAN_MAX_PATH_LEN);

            if ( p )
                wcsncpy(st_info.appname,+appnameS,XOJUMAN_MAX_PATH_LEN);

            st_info.master_id = g_Injector_Master_Id;
            st_info.flags = g_Inject_Flags;

            //__asm int 3;
            *result = (unsigned)((tCreateProcessW)apif_realcall[APIF_KCRPROCW])(
                          (p ? +VFS->containerName : +appnameS),
                          a->lpCommandLine,
                          a->lpProcessAttributes,
                          a->lpThreadAttributes,
                          a->bInheritHandles,
                          a->dwCreationFlags|CREATE_SUSPENDED,
                          a->lpEnvironment,
                          a->lpCurrentDirectory,
                          a->lpStartupInfo,
                          a->lpProcessInformation
                      );

            if ( *result )
            {
                //__asm int 3;
                // Inject Core
                CONTEXT ctx = {0};
                HANDLE hP = a->lpProcessInformation->hProcess;
                DWORD foo;

                byte_t xpeb_first[80];
                XPEB* xpeb = (XPEB*)&xpeb_first;
                IMAGE_DOS_HEADER dos = {0};
                IMAGE_NT_HEADERS nth = {0};
                //MEMORY_BASIC_INFORMATION mbi = {0};
                //VirtualQueryEx(hP,0,&mbi,sizeof(mbi));
                PROCESS_BASIC_INFORMATION pbi;
                NtQueryInformationProcess(hP,ProcessBasicInformation,&pbi,sizeof(pbi),&foo);
                ReadProcessMemory(hP,(char*)pbi.PebBaseAddress,&xpeb_first,sizeof(xpeb_first),0);
                ReadProcessMemory(hP,(char*)xpeb->ImageBaseAddress,&dos,sizeof(dos),0);
                ReadProcessMemory(hP,(char*)xpeb->ImageBaseAddress+dos.e_lfanew,&nth,sizeof(nth),0);
                ReadProcessMemory(hP,(char*)xpeb->ImageBaseAddress+nth.OptionalHeader.AddressOfEntryPoint,st_info.orign,sizeof(st_info.orign),
                                  0);

                if ( p && nth.OptionalHeader.SizeOfImage < pnth.OptionalHeader.SizeOfImage )
                {
                    void* a = (char*)xpeb->ImageBaseAddress+nth.OptionalHeader.SizeOfImage;
                    a = (void*)(((unsigned)a + 64*1024-1) & ~(64*1024-1));
                    int sz = pnth.OptionalHeader.SizeOfImage-nth.OptionalHeader.SizeOfImage;
                    st_info.reserved_space = VirtualAllocEx(hP,a,sz,MEM_RESERVE,PAGE_NOACCESS);
                    //int lerr = GetLastError();
                    //__asm int 3;
                }

                void* extmem = VirtualAllocEx(hP,0,loader_BIN_size+sizeof(st_info),MEM_COMMIT,PAGE_EXECUTE_READWRITE);
                if ( !extmem ) extmem = VirtualAllocEx(hP,0,loader_BIN_size+sizeof(st_info),MEM_COMMIT,PAGE_READWRITE);

                WriteProcessMemory(hP,extmem,loader_BIN,loader_BIN_size,0);

                InheritPackages(&st_info);
                st_info.core_handle = Query_Core_Handle(&st_info.core_size);
                DupHandle(&st_info.core_handle,hP);

                for ( int i = 0; i < st_info.pkg_count; ++i )
                    DupHandle(&st_info.pkg[i].handle,hP);

                WriteProcessMemory(hP,(char*)extmem+loader_BIN_size,&st_info,sizeof(st_info),0);

                byte_t jmp[] = "\xcc"\
                               "\x68\x00\x00\x00\x00"\
                               "\x68\x00\x00\x00\x00"\
                               "\xc3";

                *(void**)(jmp+2) = (char*)extmem+loader_BIN_size;
                *(void**)(jmp+7) = (char*)extmem+5;
                VirtualProtectEx(hP,(char*)xpeb->ImageBaseAddress+nth.OptionalHeader.AddressOfEntryPoint,12,PAGE_EXECUTE_READWRITE,&foo);
                WriteProcessMemory(hP,(char*)xpeb->ImageBaseAddress+nth.OptionalHeader.AddressOfEntryPoint,jmp+1,11,0);

                if ( !(a->dwCreationFlags&CREATE_SUSPENDED) )
                    ResumeThread(a->lpProcessInformation->hThread);
            }
        }

        return !!*result;
    }

    APIF_STATUS DoCreateProcessA(void* args, unsigned* result)
    {
        //__asm int 3;
        *result = 0;
        CRPROCA_ARGS* a = (CRPROCA_ARGS*)args;
        XDBG|_S* _XOr("apif.CreateProcA %s::%s",24,61539691)
        %(a->lpApplicationName?a->lpApplicationName:"-")
        %(a->lpCommandLine?a->lpCommandLine:"-");
        StringW appname;
        FioProxyPtr p = FindEmbeddedModule(StringW(a->lpApplicationName),StringW(a->lpCommandLine),appname);
        if ( p || g_Inject_Into_Child )
        {
            CRPROCW_ARGS b = {0};
            BufferT<byte_t> stiB(a->lpStartupInfo->cb);
            STARTUPINFOW* sti = (STARTUPINFOW*)+stiB;
            memcpy(&b,a,sizeof(*a));
            memcpy(+stiB,a->lpStartupInfo,a->lpStartupInfo->cb);
            sti->cb = sizeof(STARTUPINFOW);
            StringW applicationName = a->lpApplicationName;
            StringW commandLine = a->lpCommandLine;
            StringW title = a->lpStartupInfo->lpTitle;
            StringW desktop = a->lpStartupInfo->lpDesktop;
            b.lpApplicationName = a->lpApplicationName?(wchar_t*)+applicationName:0;
            b.lpCommandLine = a->lpCommandLine?(wchar_t*)+commandLine:0;
            b.lpStartupInfo = sti;
            sti->lpDesktop = sti->lpDesktop?(wchar_t*)+desktop:0;
            sti->lpTitle   = sti->lpTitle?(wchar_t*)+title:0;
            if ( !InjectExecutable(p,&b,appname,result) ) goto l;
        }
    else l:
            *result = (unsigned)((tCreateProcessA)apif_realcall[APIF_KCRPROCA])(
                          a->lpApplicationName,
                          a->lpCommandLine,
                          a->lpProcessAttributes,
                          a->lpThreadAttributes,
                          a->bInheritHandles,
                          a->dwCreationFlags,
                          a->lpEnvironment,
                          a->lpCurrentDirectory,
                          a->lpStartupInfo,
                          a->lpProcessInformation
                      );
        XDBG|_S* _XOr("apif.CreateProcA.Result = %08x",31,59311241) % *result;
        return APIF_RETURN;
    }

    APIF_STATUS DoWinExec(void* args, unsigned* result)
    {
        //__asm int 3;
        *result = 0;
        WINEXEC_ARGS* a = (WINEXEC_ARGS*)args;
        XDBG|_S* _XOr("apif.WinExec %s",16,66389165) %a->lpCmdLine;
        StringW appname;
        FioProxyPtr p = FindEmbeddedModule(StringW(),StringW(a->lpCmdLine),appname);
        if ( p || g_Inject_Into_Child )
        {
            CRPROCW_ARGS b = {0};
            STARTUPINFOW sti = {sizeof(STARTUPINFOW),0};
            PROCESS_INFORMATION pti = {0};
            sti.wShowWindow = a->uCmdShow;
            StringW commandLine = a->lpCmdLine;
            b.lpStartupInfo = &sti;
            b.lpProcessInformation = &pti;
            b.lpCommandLine = (wchar_t*)+commandLine;
            if ( !InjectExecutable(p,&b,appname,result) ) goto l;
            return APIF_RETURN;
        }
    l:
        return APIF_CONTINUE;
    }

    APIF_STATUS DoCreateProcessW(void* args, unsigned* result)
    {
        //__asm int 3;
        *result = 0;
        CRPROCW_ARGS* a = (CRPROCW_ARGS*)args;
        XDBG|_S* _XOr("apif.CreateProcW %s::%s",24,63374559)
        %(a->lpApplicationName?a->lpApplicationName:L"-")
        %(a->lpCommandLine?a->lpCommandLine:L"-");
        StringW appname;
        FioProxyPtr p = FindEmbeddedModule(StringW(a->lpApplicationName),StringW(a->lpCommandLine),appname);
        if ( (p || g_Inject_Into_Child) && InjectExecutable(p,a,appname,result) ) ;
        else
        {
            //__asm int 3;
            *result = (unsigned)((tCreateProcessW)apif_realcall[APIF_KCRPROCW])(
                          a->lpApplicationName,
                          a->lpCommandLine,
                          a->lpProcessAttributes,
                          a->lpThreadAttributes,
                          a->bInheritHandles,
                          a->dwCreationFlags,
                          a->lpEnvironment,
                          a->lpCurrentDirectory,
                          a->lpStartupInfo,
                          a->lpProcessInformation
                      );
        }
        XDBG|_S* _XOr("apif.CreateProcW.Result = %08x",31,69469436) % *result;
        return APIF_RETURN;
    }

    #if 0
    APIF_STATUS DoFixUpCrProc(void* args, unsigned* result)
    {
        struct X_API_MSG
        {
            PORT_MESSAGE h;
            PCSR_CAPTURE_HEADER CaptureBuffer;
            CSR_API_NUMBER ApiNumber;
            ULONG ReturnValue;
            ULONG Reserved;
            HANDLE hProcess;
            HANDLE hThread;
        };
        struct ARGS
        {
            X_API_MSG* m;
            void*      _1;
            unsigned  api_id;
            unsigned  length;
        };
        ARGS* a = (ARGS*)args;
        XDBG|_S*"X:%08x,%08x,%08x,%08x" %a->m %a->_1 %a->api_id %a->length;
        PROCESS_FIXUP* l = (PROCESS_FIXUP*)TlsGetValue(proc_fixup);
        if ( a->api_id == 0x10000 && l )
        {
            NTSTATUS st = 0;
            X_API_MSG* m = (X_API_MSG*)a->m;
            PROCESS_BASIC_INFORMATION processInfo;
            RTL_USER_PROCESS_PARAMETERS Par = {0};
            PRTL_USER_PROCESS_PARAMETERS ParInProc = 0;
            PRTL_USER_PROCESS_PARAMETERS pPar = 0;

            //__asm int 3;
            TlsSetValue(proc_fixup,0);

            st = NtQueryInformationProcess(m->hProcess,ProcessBasicInformation,&processInfo,sizeof(processInfo),0);
            XPEB* pPeb = (XPEB*)processInfo.PebBaseAddress;
            st = NtReadVirtualMemory(m->hProcess,&pPeb->ProcessParameters,&ParInProc,sizeof(ParInProc),0);
            st = NtReadVirtualMemory(m->hProcess,ParInProc,&Par,sizeof(Par),0);

            Par.ImagePathName.Buffer = Par.CommandLine.Buffer+1;
            //Par.ImagePathName.Buffer = Par.CommandLine.Buffer;
            Par.ImagePathName.Length = l->ImagePathLength-2;
            Par.ImagePathName.MaximumLength = l->ImagePathLength;
            (char*&)Par.CommandLine.Buffer += Par.ImagePathName.MaximumLength;
            Par.CommandLine.Length -= Par.ImagePathName.MaximumLength;
            Par.CommandLine.MaximumLength -= Par.ImagePathName.MaximumLength;
            //Par.ImagePathName.Buffer[Par.ImagePathName.Length/2] = 0;
            if ( 1/*(unsigned)Par.ImagePathName.Buffer < Par.MaximumLength*/ )
            {
                BufferT<byte_t> b(Par.MaximumLength);
                st = NtReadVirtualMemory(m->hProcess,ParInProc,+b,Par.MaximumLength,0);
                //__asm int 3;
                memcpy(+b,&Par,sizeof(Par));
                if ( (unsigned)Par.ImagePathName.Buffer < Par.MaximumLength )
                {
                    wchar_t* S = (wchar_t*)(+b+(unsigned)Par.ImagePathName.Buffer);
                    S[Par.ImagePathName.Length/2] = 0;
                }
                else if ( (unsigned)Par.ImagePathName.Buffer - (unsigned)ParInProc  < Par.MaximumLength )
                {
                    unsigned q = (unsigned)Par.ImagePathName.Buffer - (unsigned)ParInProc;
                    wchar_t* S = (wchar_t*)(+b+q);
                    S[Par.ImagePathName.Length/2] = 0;
                }
                st = NtWriteVirtualMemory(m->hProcess,ParInProc,+b,Par.MaximumLength,0);
            }
            else
                st = NtWriteVirtualMemory(m->hProcess,ParInProc,&Par,sizeof(Par),0);
            st = NtWriteVirtualMemory(m->hProcess,&pPeb->ImageSubSystem,&l->SubSystem,sizeof( pPeb->ImageSubSystem ),0);
            return APIF_ORIGINAL;
        }
        return APIF_CONTINUE;
    }
    #endif

    virtual APIF_STATUS DoCall(int apif_id, void* args, unsigned* result)
    {
        switch (apif_id)
        {
            case APIF_KCRPROCA:
                return DoCreateProcessA(args,result);
            case APIF_KCRPROCW:
                return DoCreateProcessW(args,result);
            case APIF_KWINEXEC:
                return DoWinExec(args,result);
                //case APIF_CSRCALL:
                //  return DoFixUpCrProc(args,result);
        }
        return APIF_CONTINUE;
    }
};

CrProcFilter apif_crprocfilt = CrProcFilter();

void APIF_RegisterCrProcHooks()
{
    APIF->Push(&apif_crprocfilt);
};
