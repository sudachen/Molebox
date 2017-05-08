
/*

Copyright (c) 2008-2009, Alexey Sudachen, alexey@sudachen.name

*/

#include "_common.h"
#include "apif.h"
#include "splicer.h"
#include "logger.h"
#include "import.h"
#include "vfs.h"
#include "XorS.h"
#include "teggovbox.h"
#include "DbgHelp.h"
#include "xojuman.h"
#include <rsa.h>
#include <psapi.h>

#define DEFGUID(name, l, w1, w2, b1, b2, b3, b4, b5, b6, b7, b8) \
    EXTERN_C const GUID DECLSPEC_SELECTANY name \
        = { l, w1, w2, { b1, b2,  b3,  b4,  b5,  b6,  b7,  b8 } }

//DEFGUID(CLSID_CLRRuntimeHost, 0x90F1A06E, 0x7712, 0x4762, 0x86, 0xB5, 0x7A, 0x5E, 0xBA, 0x6B, 0xDB, 0x02);
DEFGUID(IID_ICorRuntimeHost, 0xcb2f6722, 0xab3a, 0x11d2, 0x9c, 0x40, 0x00, 0xc0, 0x4f, 0xa3, 0x0a, 0x3e);
DEFGUID(CLSID_CorRuntimeHost, 0xcb2f6723, 0xab3a, 0x11d2, 0x9c, 0x40, 0x00, 0xc0, 0x4f, 0xa3, 0x0a, 0x3e);
DEFGUID(IID_ICLRMetaHost, 0xD332DB9E, 0xB9B3, 0x4125, 0x82, 0x07, 0xA1, 0x48, 0x84, 0xF5, 0x32, 0x16);
DEFGUID(CLSID_CLRMetaHost, 0x9280188d, 0xe8e, 0x4867, 0xb3, 0xc, 0x7f, 0xa8, 0x38, 0x84, 0xe8, 0xde);
DEFGUID(IID_ICLRRuntimeInfo, 0xBD39D1D2, 0xBA2F, 0x486a, 0x89, 0xB0, 0xB4, 0xB0, 0xCB, 0x46, 0x68, 0x91);
DEFGUID(CLSID_CLRRuntimeHost, 0x90F1A06E, 0x7712, 0x4762, 0x86, 0xB5, 0x7A, 0x5E, 0xBA, 0x6B, 0xDB, 0x02);
DEFGUID(IID_ICLRRuntimeHost, 0x90F1A06C, 0x7712, 0x4762, 0x86, 0xB5, 0x7A, 0x5E, 0xBA, 0x6B, 0xDB, 0x02);

int g_License_Level = 0;
int g_Inject_Flags = 0;

bool g_Inject_Into_Child = false;
u32_t g_Injector_Master_Id = 0;

void* g_Core_Ptr = 0;
HANDLE g_Core_Handle = 0;
u32_t g_Core_Size = 0;
INT64T g_Core_Offset = 0;

STB_INFO2_FLAGS g_stbi2f;
u32_t g_sid_0;

StringW g_Registery_Mask;

extern void ExecuteActivator(STB_INFO* info);

extern "C" unsigned isNestedProcess = 0;
extern bool exeFilterIsEnabled;
extern "C" int __stdcall _DllMainCRTStartup(void*, unsigned, void*);
void* QueryAPI();
void* QueryAPI1(void* activator_base);
void __stdcall _API_Parse_CommandLine(int* argc, wchar_t** *argv);
void __stdcall _API_Set_CommandLine(wchar_t* cmdl);
void __stdcall _API_CmdLine_SetArgs(wchar_t** cmdl, int count);
extern "C" wchar_t** __cdecl buildargv(wchar_t const* cmdl, int* argc);
extern "C" void __cdecl freeargv(wchar_t** vector);

void SplashScreen();
void InitActFromManifest(void* base, pwide_t mod_path, ulong_t* act_cookie);
void ActDeactivate(ulong_t act_cookie);

extern "C" int Import_Registry(void* text, char error[256]);
extern "C" wchar_t Import_Registry_Appfolder[MAX_PATH + 1];

#ifndef __BUILD_VR_CORE__
void Load_Registry_Files(pwide_t mask)
{
    int before = Set_Registry_Virtualization_Mode(REGISTRY_FULL_VIRTUALIZATION);
    VfsDirlistPtr lst = _VFS::ListEmbeddedFiles(mask);
    wcscpy(Import_Registry_Appfolder, +GetDirectoryOfPath(+VFS->containerName));
    for (int i = 0; i < lst->Count(); ++i)
    {
        u32_t wsr;
        char error[256];
        BufferT<char> text(lst->At(i)->Size() + 2, '\0');
        xlog | _S* _XOr("importing registry file: %s",28,342826031) % lst->At(i)->ComposePath1();
        if (0 == lst->At(i)->Read(0, +text, lst->At(i)->Size(), &wsr))
        {
            if (!Import_Registry(+text, error))
                xlog | _S* _XOr("failed to import registry file: %s",35,336075784) % error;
        }
        else
        {
            xlog | _S* _XOr("failed to read registry file: %s",33,365960272)
            % lst->At(i)->ComposePath1();
        }
    }
    Set_Registry_Virtualization_Mode(before);
}

int ActiveX_Register(int, pwide_t fullname)
{
    xlog | _S* _XOr("ocx_register: %s",17,369040831) % fullname;
    int olderr = SetErrorMode(SEM_NOOPENFILEERRORBOX | SEM_FAILCRITICALERRORS);
    long(__stdcall * DLLregisterServer)(void) = 0;
    if (HMODULE hm = LoadLibraryExW((wchar_t*)fullname, 0, LOAD_WITH_ALTERED_SEARCH_PATH))
    {
        if (!!(*(void**)&DLLregisterServer = GetProcAddress(hm, "DllRegisterServer")))
        {
            long hr = DLLregisterServer();
            if (hr)
                xlog | _S* _XOr("  failed to register with code %d",34,362159510) % hr;
        }
    }
    SetErrorMode(olderr);
    return 1;
}

DWORD __stdcall Register_On_Start(void* q = 0)
{
    if (!q)
    {
        DWORD e;
        void* thr = CreateThread(0, 0, &Register_On_Start, (void*)1, 0, 0);

        if (thr)
        {
            while (GetExitCodeThread(thr, &e) && e != STILL_ACTIVE)
                WaitForSingleObject(thr, 100);
            CloseHandle(thr);
        }
        else
        {
            xlog | "failed to start OCX/ActiveX registrator!";
            __asm int 3;
            ExitProcess(-1);
        }
    }
    else
    {
        xlog | "REGSITER-ON-START";
        int before = Set_Registry_Virtualization_Mode(REGISTRY_FULL_VIRTUALIZATION);
        CoInitializeEx(0, 0);
        VFS->Call_If(SVFS_ACTIVEX, &ActiveX_Register);
        CoUninitialize();
        Set_Registry_Virtualization_Mode(before);
    }

    return 0;
}

#endif

unsigned QuerySystemHWID()
{
    wchar_t p[256] = {0};
    wchar_t n[256] = {0};
    unsigned long serial = 0;
    unsigned long foo;
    u32_t crc = 0;
    int l = GetWindowsDirectoryW(p, 255);
    p[StrFindChar(p, L'\\', 0) + 1] = 0;
    if (GetVolumeInformationW(p, n, 255, &serial, &foo, &foo, n, 255))
        crc = Crc32(0, &serial, 4);
    CPUINFO cpui;
    GetCPUInfo(&cpui);
    crc = Crc32(crc, &cpui, sizeof(cpui));
    return crc;
}

bool isnumber(wchar_t const* q)
{
    if (!q) return false;
    while (*q)
    {
        if (!iswdigit(*q)) return false;
        ++q;
    }
    return true;
}

void ChangeCommandLine(wchar_t const* pattern)
{
    wchar_t fullpath[MAX_PATH + 1] = {0};
    wchar_t** argv;
    int argc;
    ArrayT<StringW> R;
    wchar_t** elms;
    int elmsc;
    _API_Parse_CommandLine(&argc, &argv);
    elms = buildargv(pattern, &elmsc);

    if (elms && elmsc)
    {
        wchar_t* exename = 0;
        GetFullPathNameW(argv[-1], MAX_PATH, fullpath, &exename);
        if (!exename) exename = fullpath;
        for (int i = 0; i < elmsc; ++i)
        {
            StringW A = elms[i];
            if (A.StartsWith(L"<") && A.EndsWith(L">"))
            {
                ArrayT<StringW> L; L.Push(StringW(+A + 1, A.Length() - 2)); //.Split('|');
                if (0);
                else
                {
                    StringW q = L[0];
                    q.Trim().ToLower();
                    if (isnumber(+q))
                    {
                        int i = q.ToLong();
                        if (i > 0 && i <= argc)
                            R.Push(argv[i - 1]);
                    }
                    else if (q == L"basename" || q == L"b" || q == L"$b")
                    {
                        wchar_t const* r = exename + wcslen(exename);
                        for (wchar_t const* q = r; q > exename; --q)
                            if (*q == '.') { r = q; break; }
                        R.Push(StringW(exename, r));
                    }
                    else if (q == L"exename" || q == L"e" || q == L"$e")
                    {
                        R.Push(exename);
                    }
                    else if (q == L"dirname" || q == L"d" || q == L"$d")
                    {
                        R.Push(StringW(fullpath, exename - 1));
                    }
                    else if (q == L"fullpath" || q == L"p" || q == L"$p")
                    {
                        R.Push(fullpath);
                    }
                    else if (q == L"fullname" || q == L"n" || q == L"$n")
                    {
                        wchar_t const* r = fullpath + wcslen(fullpath);
                        for (wchar_t const* q = r; q > exename; --q)
                            if (*q == '.') { r = q; break; }
                        R.Push(StringW(fullpath, r));
                    }
                    else if (q == L"@" || q == L"$@")
                    {
                        for (int i = 0; i < argc; ++i)
                            R.Push(argv[i]);
                    }
                }
            }
            else
                R.Push(elms[i]);
        }

        freeargv(elms);
        BufferT<wchar_t*> a(R.Count());
        for (int i = 0; i < R.Count(); ++i) a[i] = (wchar_t*) + R[i];
        _API_CmdLine_SetArgs(&a[0], a.Count());
    }
}

void InitializeStage_Protector(unsigned flags = 0xffffffff)
{
}

void InitializeStage_0(unsigned flags = 0xffffffff, unsigned catalog_base = 0)
{
    xlog | _S* _XOr("commad line: %s",16,364781054) % GetCommandLineW();
    xlog | _XOr("starting at stage 0 ...",24,363404777);
    #if !defined __BUILD_VR_CORE__
    VFS->InitAndLoadInternalCatalog((flags & TEGGOVBOX_SEARCH_FOR_EMBEDDINGS) ? catalog_base : 0);
    #else
    VFS->InitAndLoadInternalCatalog(0);
    #endif
    APIF->SetupNtKrHooks(_NTDLL, _KERN32);
    #if !defined __BUILD_VR_CORE__
    if (flags & TEGGOVBOX_VIRTUALIZE_FS)
    {
        APIF_RegisterFileObjectHooks();
        APIF_RegisterSectionObjectHooks();
        APIF_RegisterActCtxHooks();
        APIF_RegisterCrProcHooks();
        APIF_RegisterCmdlHooks();
    }
    #endif
    if (flags & TEGGOVBOX_VIRTUALIZE_REGISTRY)
        APIF_RegisterRegObjectHooks(!(flags & TEGGOVBOX_VIRTUALIZE_REGISTRY_FULL), !!(flags & TEGGOVBOX_LOG_ACTIVITY_DBG));
}

void InitializeStage_1(unsigned flags = 0xffffffff) // applying default rules
{
    xlog | _XOr("starting at stage 1 ...",24,357440974);
    #if !defined __BUILD_VR_CORE__
    if (flags & TEGGOVBOX_SEARCH_FOR_EMBEDDINGS)
        VFS->MoundDefaults();
    #endif
}

static TEGGOVBOX_FUNCTIONS_TABLE* g_functions_table = 0;

void InitExtraFunctions(unsigned flags)
{
    *(void**)&g_functions_table = VirtualAlloc(0, sizeof(TEGGOVBOX_FUNCTIONS_TABLE), MEM_COMMIT, PAGE_READWRITE);
    TEGGOVBOX_FUNCTIONS_TABLE* ft = g_functions_table;
    ft->WriteVirtualRegistry = &APIF_RegisterWriteToFile;
    ft->SerilizeVirtualRegistryAsText = &APIF_RegisterSerilize;
    ft->LogState = &APIF_LogState;
}

extern "C" void* _PEB();

/*void *find_ntdll(int T)
{
void *handle = 0;
void *l = _DLO();
void *S = FLINK(l);

while ( S != l )
{
if ( T == 0 && ISNTDLL(S) )
{
handle = (void*)DLO_BASE(S);
break;
}
else if ( T == 1 && ISK32DLL(S) )
{
handle = (void*)DLO_BASE(S);
break;
}
S = FLINK(S);
}

return handle;
}*/

void FixupImageName()
{
    if (int l = GetEnvironmentVariableA(_XOr("XOJUMAN-APPNAME",16,358751522), 0, 0))
    {
        //__asm int 3;

        _XPEB* peb = (_XPEB*)_PEB();
        MY_RTL_USER_PROCESS_PARAMETERS* para = peb->ProcessParameters;

        wchar_t* imgpathq = (wchar_t*)HeapAlloc(peb->ProcessHeap, HEAP_ZERO_MEMORY, l * 2 + 2);
        GetEnvironmentVariableW(+StringW(_XOr("XOJUMAN-APPNAME",16,354163996)), imgpathq, l);
        RtlInitUnicodeString(&para->ImagePathName, imgpathq);
        RtlInitUnicodeString(&para->WindowTitle, imgpathq);

        LDR_MODULE* q = (LDR_MODULE*)peb->LoaderData->InMemoryOrderModuleList.Flink;
        while (q && q->BaseAddress != peb->ImageBaseAddress)
            q = (LDR_MODULE*)q->ModuleList.Flink;

        if (q)
        {
            RtlInitUnicodeString(&q->FullDllName, para->ImagePathName.Buffer);
            int k = StrFindRChar(para->ImagePathName.Buffer, (wchar_t)'\\', -1, para->ImagePathName.Length / 2);
            if (k > 0)
                RtlInitUnicodeString(&q->BaseDllName, para->ImagePathName.Buffer + k + 1);
        }
    }
}

static int equal1(wchar_t* a, char* b)
{
    for (; *a && *a == *b; ++a, ++b) ;
    return *a == *b;
}

extern "C" void* _imp__GetProcAddress__8;
extern "C" void* _imp__GetModuleHandleW__4;

void* __stdcall _GetProcAddress(HMODULE mod, char* proc)
{
    return GetProcAddressIndirect(mod, proc, -1);
}

extern "C" void* __stdcall _kernel32_GetModuleHandleW_xcall_6(int,wchar_t*);
void* __stdcall _GetModuleHandleW(wchar_t* modname)
{
    if (equal1(modname, _XOr("KERNEL32.DLL",13,352853256)))
        return _KERN32;
    return _kernel32_GetModuleHandleW_xcall_6(6,modname);
}

void __stdcall _FirstCall(void* ntdll, void* kern32, STB_INFO* info, void* base)
{
    _NTDLL = ntdll;
    _KERN32 = kern32;

    InitHeapAndSplicer(info, base);
    _ntdll_xcall_fullinit();

    _USER32 = GetModuleHandleA(_XOr("user32",7,356457855));
    _ADVA32 = GetModuleHandleA(_XOr("advapi32",9,355212652));
    _imp__GetModuleHandleW__4 = _GetModuleHandleW;
    _imp__GetProcAddress__8 = _GetProcAddress;

    FixupImageName();

    _DllMainCRTStartup(base, DLL_PROCESS_ATTACH, 0);

    ResetMillis();

    //__asm int 3;
    pwide_t modname = 0;

    if (info && (info->flags & STBF_EXTENDED))
    {
        STB_INFO2* info2 = (STB_INFO2*)info;
        modname = info2->modname;
    }

    if (modname)
        VFS->containerName = modname;
    else
        VFS->containerName = Hinstance->GetMainModulePath();

    // fixup image name on winx64
    //
    //APIF_CrProc_SEP = NT_HEADERS(base)->OptionalHeader.AddressOfEntryPoint;
    //SetEnvironmentVariableW(_XOrL("XOJUMAN",0,0),+VFS->containerName);
}

void _Inity(unsigned flags, unsigned catalog_base)
{
    //__asm int 3;
    if (flags & TEGGOVBOX_LOG_ACTIVITY)
        StartLog();
    if (flags & TEGGOVBOX_LOG_ACTIVITY_DBG)
        ifdbg = true;
    InitializeStage_Protector(flags);
    InitializeStage_0(flags, catalog_base);
    InitializeStage_1(flags);
    InitExtraFunctions(flags);
    if (flags & TEGGOVBOX_LOG_ACTIVITY_DBG)
        APIF_LogState();
}

NEWDES_Cipher g_cipher;

void* _ACTIVATOR = 0;
void* LoadEmbeddedLibrary(pchar_t name, unsigned offset, unsigned count, unsigned crc, unsigned zipped, unsigned xcrypt,
                          BufferT<SQUOTED_IMPORT>* squoted = 0);
static BUNDLE_DATA* bundle_data = 0;
void* __stdcall loadlibrary(char* dllname)
{
    if (bundle_data)
    {
        for (int i = 0; i < bundle_data->hdr.number; ++i)
            if (strcmpi(dllname, bundle_data->rec[i].name) == 0)
            {
                if (!bundle_data->rec[i].handle && !bundle_data->rec[i].actfake)
                {
                    BUNDLED_IMPORT* d = bundle_data->rec + i;
                    d->handle = LoadEmbeddedLibrary(d->name, d->offset, d->count, d->crc, d->zipped, d->xcrypt);
                }
                if (!bundle_data->rec[i].activator && (bundle_data->rec[i].actfake || bundle_data->rec[i].system))
                    goto e;
                else if (!bundle_data->rec[i].handle)
                    goto e;

                return bundle_data->rec[i].handle; // 0 if actfake
            }
    }

    if (void* m = LoadLibraryA(dllname))
        return m;
e:
    MessageBox(0, 0 | _S * _XOr("failed to load module %s",25,382868818) % dllname, _XOr("Import Error",13,385227446),
               MB_OK | MB_ICONERROR);
    TerminateProcess((HANDLE) - 1, -1);
    return 0;
}

static bool do_save_iat =
    #if !defined __BUILD_DEMO_CORE__
    0
    #else
    1
    #endif
    ;

void* __stdcall getproc(void* module, char* procname)
{
    void* proc = GetProcAddressIndirect(module, procname, -1);
    if (!proc)
    {
        char modname[256] = {0};
        GetModuleBaseNameA((HANDLE) - 1, (HMODULE)module, modname, 255);
        MessageBox(0, 0 | _S * _XOr("module %s doesn't have symbol %s",33,384244391) % modname % procname,
                   _XOr("Import Error",13,378346125), MB_OK | MB_ICONERROR);
        TerminateProcess((HANDLE) - 1, -1);
    }
    if (module && (0
                   || module == _KERN32
                   //|| module == _USER32
                   //|| module == _ADVA32
                   || module == _ACTIVATOR
                  ) && !do_save_iat)
        proc = WrapWithJxThunk(proc);
    return proc;
}


StringW Query_UCS_Text(OCC* occ)
{
    if (occ->count)
    {
        APIF->AcquireFilterLock();
        HANDLE f = CreateFileW(+VFS->containerName, GENERIC_READ, FILE_SHARE_READ, 0, OPEN_EXISTING, 0, 0);
        APIF->ReleaseFilterLock();
        if (f && f != INVALID_HANDLE_VALUE)
        {
            BufferT<wchar_t> data(occ->count);
            long foo = 0;
            SetFilePointer(f, occ->offset, &foo, 0);
            AuxReadFile(f, +data, occ->count);
            g_cipher.DoCipherCBCO(+data, occ->count / 8);
            CloseHandle(f);
            return StringW(+data);
        }
    }
    return L"";
}

#if !defined __BUILD_DEMO_CORE__

void ReorderImports(byte_t* mapped)
{
    IMAGE_NT_HEADERS* nth = NT_HEADERS(mapped);
    IMAGE_SECTION_HEADER* S = IMAGE_FIRST_SECTION(nth);
    IMAGE_SECTION_HEADER* Se = S + nth->FileHeader.NumberOfSections;
    unsigned import_offs = nth->OptionalHeader.DataDirectory[IMAGE_DIRECTORY_ENTRY_IMPORT].VirtualAddress;
    unsigned import_count = nth->OptionalHeader.DataDirectory[IMAGE_DIRECTORY_ENTRY_IMPORT].Size;
    IMAGE_IMPORT_DESCRIPTOR* d = (IMAGE_IMPORT_DESCRIPTOR*)(mapped + import_offs);
    for (; d->Name; ++d)
    {
        StringA dn = StringA((pchar_t)(mapped + d->Name)).ToLower();
        if (dn.StartWith(_XOr("user32",7,381623039)) || dn.StartWith(_XOr("gdi32",6,379788003)))
            continue;
        int n = 0;
        u32_t* p = (u32_t*)(mapped + d->FirstThunk);
        while (p[n]) ++n;
        if (n > 1)
        {
            BufferT<u32_t> reord(n);
            for (int i = 0; i < n; ++i)
                reord[i] = i;
            u32_t sid = g_sid_0;
            for (int i = 0; i < n; ++i)
            {
                u32_t q = teggo::unsigned_random(&sid) % n;
                if (i != q)
                {
                    u32_t t = reord[i]; reord[i] = reord[q]; reord[q] = t;
                }
            }
            for (int i = 0; i + 1 < n; i += 2)
            {
                cxx_swap(p[reord[i]], p[reord[i + 1]]);
            }
        }
    }
}

bool LoadSquotedData(STB_INFO* info, bool act, BufferT<SQUOTED_IMPORT>& data)
{
    SQUOTED_HEADER hdr;
    //unsigned count = act ? info->squoted_act.count ? info->squoted.count;
    unsigned offset  = act ? info->squoted_act.offset : info->squoted.offset;
    unsigned crc   = act ? info->squoted_act.crc : info->squoted.crc;
    if (!offset) return false;

    APIF->AcquireFilterLock();
    HANDLE f = CreateFileW(+VFS->containerName, GENERIC_READ, FILE_SHARE_READ, 0, OPEN_EXISTING, 0, 0);
    APIF->ReleaseFilterLock();
    if (f && f != INVALID_HANDLE_VALUE)
    {
        long foo = 0;
        SetFilePointer(f, offset, &foo, 0);
        AuxReadFile(f, &hdr, sizeof(hdr));
        if (hdr.number)
        {
            data.Resize(hdr.number);
            AuxReadFile(f, +data, hdr.number * sizeof(SQUOTED_IMPORT));
            g_cipher.DoCipherCBCO(+data, hdr.number * sizeof(SQUOTED_IMPORT) / 8);
        }
        CloseHandle(f);
    }
    return true;
}

void UnsquoteSquoted(byte_t* base, BufferT<SQUOTED_IMPORT>& sq)
{
    for (int i = 0; i < sq.Count(); ++i)
    {
        unsigned** q = (unsigned**)(base + sq[i].rva);
        unsigned* t = (unsigned*)(base + sq[i].thunk);
        unsigned* j = (unsigned*)AllocExecutableBlock(4);
        *j = *t;
        *q = j;
    }
}

void UnsqouteExecutable(byte_t* base, STB_INFO* info)
{
    //__asm int 3;
    BufferT<SQUOTED_IMPORT> sq;
    if (LoadSquotedData(info, false, sq))
        UnsquoteSquoted(base, sq);
}

#endif __BUILD_DEMO_CORE__

void* LoadEmbeddedLibrary(
    pchar_t name,
    unsigned offset, unsigned count, unsigned crc, unsigned zipped, unsigned xcrypt,
    BufferT<SQUOTED_IMPORT>* squoted)
{
    byte_t* base   = 0;
    byte_t* b = (byte_t*)AuxAllocExec(count);
    APIF->AcquireFilterLock();
    HANDLE f = CreateFileW(+VFS->containerName, GENERIC_READ, FILE_SHARE_READ, 0, OPEN_EXISTING, 0, 0);
    APIF->ReleaseFilterLock();
    if (f && f != INVALID_HANDLE_VALUE)
    {
        long foo = 0;
        SetFilePointer(f, offset, &foo, 0);
        AuxReadFile(f, b, count);
        if (Crc32(0, b, count) != crc)
            __asm int 3
            ;
        if (xcrypt)
            g_cipher.DoCipherCBCO(b, count / 8);
        if (zipped)
        {
            unsigned size = *(unsigned*)b;
            byte_t* p = (byte_t*)AuxAllocExec(size);
            zlib_decompress(b + 4, count - 4, p, size);
            AuxFree(b);
            b = p;
        }
        CloseHandle(f);
    }

    IMAGE_NT_HEADERS const* nth = NT_HEADERS(b);
    int dos_sz = (byte_t*)nth - b;
    base = (byte_t*)AuxAllocExec(nth->OptionalHeader.SizeOfImage);
    memcopy(base, b,
            dos_sz
            + sizeof(IMAGE_NT_HEADERS) - sizeof(IMAGE_OPTIONAL_HEADER)
            + sizeof(IMAGE_SECTION_HEADER)*nth->FileHeader.NumberOfSections
            + nth->FileHeader.SizeOfOptionalHeader
           );
    IMAGE_SECTION_HEADER const* S = (IMAGE_SECTION_HEADER*)((char*)nth + (24 + nth->FileHeader.SizeOfOptionalHeader));
    for (int i = 0; i < nth->FileHeader.NumberOfSections; ++i)
    {
        IMAGE_SECTION_HEADER const* const Si = S + i;
        memcopy(base + Si->VirtualAddress, b + Si->PointerToRawData, Si->SizeOfRawData);
    }
    Relocate(base, nth);

    ulong_t act_cookie = 0;
    InitActFromManifest((byte_t*)base, +StringW(name), &act_cookie);
    Import(base, nth, loadlibrary);

    #if !defined __BUILD_DEMO_CORE__
    if (squoted)
    {
        ReorderImports((byte_t*)base);
        UnsquoteSquoted((byte_t*)base, *squoted);
    }
    #endif

    if (nth->OptionalHeader.AddressOfEntryPoint)
    {
        void* ep = base + nth->OptionalHeader.AddressOfEntryPoint;
        ((int(__stdcall*)(void*, int, void*))ep)(base, DLL_PROCESS_ATTACH, 0);
    }
    AuxFree(b);
    ActDeactivate(act_cookie);
    return base;
}

bool LoadBundledDlls(STB_INFO* info, void* base)
{
    bundle_data = (BUNDLE_DATA*)VirtualAlloc(0, info->bundle.count, MEM_COMMIT, PAGE_READWRITE);
    APIF->AcquireFilterLock();
    HANDLE f = CreateFileW(+VFS->containerName, GENERIC_READ, FILE_SHARE_READ, 0, OPEN_EXISTING, 0, 0);
    APIF->ReleaseFilterLock();
    if (f && f != INVALID_HANDLE_VALUE)
    {
        long foo = 0;
        SetFilePointer(f, info->bundle.offset, &foo, 0);
        AuxReadFile(f, bundle_data, info->bundle.count);
        g_cipher.DoCipherCBCO(bundle_data->rec, (bundle_data->hdr.number * sizeof(bundle_data->rec[0])) / 8);
        CloseHandle(f);
    }
    for (int i = 0; i < bundle_data->hdr.number; ++i)
        if (!bundle_data->rec[i].handle && !bundle_data->rec[i].actfake)
        {
            BUNDLED_IMPORT* d = bundle_data->rec + i;
            d->handle = LoadEmbeddedLibrary(d->name, d->offset, d->count, d->crc, d->zipped, d->xcrypt);
        }
    return true;
}

bool RestoreImports(void* base)
{
    if (bundle_data)
    {
        unsigned long foo = 0, bar = 0;
        IMAGE_NT_HEADERS* nth = NT_HEADERS(base);
        IMAGE_IMPORT_DESCRIPTOR* S = (IMAGE_IMPORT_DESCRIPTOR*)((byte_t*)base +
                                     nth->OptionalHeader.DataDirectory[IMAGE_DIRECTORY_ENTRY_IMPORT].VirtualAddress);
        unsigned sz = nth->OptionalHeader.DataDirectory[IMAGE_DIRECTORY_ENTRY_IMPORT].Size + sizeof(IMAGE_IMPORT_DESCRIPTOR) *
                      (bundle_data->hdr.number + 1);
        VirtualProtect(S, sz, PAGE_READWRITE, &foo);
        //__asm int 3;
        int j = 0;
        while (S[j].Name) ++j;
        for (int i = 0; i < bundle_data->hdr.number; ++i)
            if (bundle_data->rec[i].iidName)
            {
                IMAGE_IMPORT_DESCRIPTOR* import = S + bundle_data->rec[i].iidOffs;
                import->Name = bundle_data->rec[i].iidName;
                import->OriginalFirstThunk = bundle_data->rec[i].iidOfth;
                import->FirstThunk = bundle_data->rec[i].iidFth;
                unsigned* p = (unsigned*)((byte_t*)base + import->FirstThunk);
                int i = 0;
                while (p[i]) ++i;
                VirtualProtect(p, i * 4, PAGE_READWRITE, &bar);
                Import1(import, (byte_t*)base, loadlibrary);
                VirtualProtect(p, i * 4, bar, &bar);
            }
        //for ( int i = 0; i < bundle_data->hdr.number; ++i ) S[j+i] = S[i];
        if (nth->OptionalHeader.DataDirectory[IMAGE_DIRECTORY_ENTRY_IMPORT].Size > (j + bundle_data->hdr.number)*sizeof(
                IMAGE_IMPORT_DESCRIPTOR))
            memset(&S[j + bundle_data->hdr.number], 0, sizeof(IMAGE_IMPORT_DESCRIPTOR));
        VirtualProtect(S, sz, foo, &foo);
        //VirtualProtect(&nth->OptionalHeader.DataDirectory[IMAGE_DIRECTORY_ENTRY_IMPORT].Size,4,PAGE_READWRITE,&foo);
        //nth->OptionalHeader.DataDirectory[IMAGE_DIRECTORY_ENTRY_IMPORT].Size = sz;//-sizeof(IMAGE_IMPORT_DESCRIPTOR);
        //VirtualProtect(&nth->OptionalHeader.DataDirectory[IMAGE_DIRECTORY_ENTRY_IMPORT].Size,4,foo,&foo);
    }
    return true;
}

void RestoreExecutable(STB_INFO* info, void* base)
{
    //__asm int 3;
    HANDLE f = 0;
    ZIPPED_HEADER* zh = 0;
    IMAGE_NT_HEADERS* nth = NT_HEADERS(base);
    u32_t tls_index = 0;
    //__asm int 3;
    if (nth->OptionalHeader.DataDirectory[IMAGE_DIRECTORY_ENTRY_TLS].VirtualAddress)
    {
        IMAGE_TLS_DIRECTORY* tls = (IMAGE_TLS_DIRECTORY*)((byte_t*)base +
                                   nth->OptionalHeader.DataDirectory[IMAGE_DIRECTORY_ENTRY_TLS].VirtualAddress);
        tls_index = *(unsigned*)tls->AddressOfIndex;
    }

    if (info->zip.offset)
    {
        unsigned long foo;
        unsigned cnt  = info->zip.offset + info->zip.count;

        APIF->AcquireFilterLock();
        f = CreateFileW(+VFS->containerName, GENERIC_READ, FILE_SHARE_READ, 0, OPEN_EXISTING, 0, 0);
        APIF->ReleaseFilterLock();

        //APIF->AcquireFilterLock();
        //byte_t *_mapped = (byte_t*)SysMmapFile(Hinstance->GetMainModulePath(),(cnt+4095)&~4095,0,false,false,EMPTYEF);
        //APIF->ReleaseFilterLock();

        if (!f || f == INVALID_HANDLE_VALUE)
            __asm int 3
            ;

        zh = (ZIPPED_HEADER*)VirtualAlloc(0, info->zip.count, MEM_COMMIT, PAGE_READWRITE);
        SetFilePointer(f, info->zip.offset, 0, 0);
        //__asm int 3;
        AuxReadFile(f, zh, info->zip.count);
        g_cipher.DoCipherCBCO(zh, info->zip.count / 8);
        if (Crc32(0, zh, info->zip.count) != info->zip.crc)
            __asm int 3

            IMAGE_SECTION_HEADER* S = IMAGE_FIRST_SECTION(nth);
        if (!VirtualProtect(base, nth->OptionalHeader.SizeOfImage, PAGE_EXECUTE_READWRITE, &foo))
            VirtualProtect(base, nth->OptionalHeader.SizeOfImage, PAGE_READWRITE, &foo); // old windows versions

        nth->FileHeader.NumberOfSections = zh->number;
        for (int i = 0; i < 16; ++i)
            nth->OptionalHeader.DataDirectory[i] = zh->orgdir[i];

        for (int i = 0, p2rd = nth->OptionalHeader.FileAlignment; i < zh->number; ++i)
        {
            memset(S + i, 0, sizeof(IMAGE_SECTION_HEADER));
            memcpy(S[i].Name, zh->section[i].name, 8);
            //g_cipher.DoCipher(S[i].Name,1);
            S[i].Characteristics = zh->section[i].access;
            S[i].VirtualAddress = zh->section[i].rva;
            S[i].SizeOfRawData = zh->section[i].rawsize;
            S[i].Misc.VirtualSize = zh->section[i].vasize;
            S[i].PointerToRawData = p2rd; p2rd += S[i].SizeOfRawData;
            ZIPPED_SECTION* Z = zh->section + i;
            unsigned vsz = cxx_max(cxx_max(S[i].SizeOfRawData, S[i].Misc.VirtualSize), nth->OptionalHeader.SectionAlignment);
            memset((byte_t*)base + S[i].VirtualAddress, 0, cxx_alignu(vsz, nth->OptionalHeader.SectionAlignment));
            if (Z->count)
            {
                byte_t* p = (byte_t*)VirtualAlloc(0, Z->count, MEM_COMMIT, PAGE_READWRITE);
                SetFilePointer(f, info->zip.offset + Z->offset, 0, 0);
                //__asm int 3;
                AuxReadFile(f, p, Z->count);
                //memcpy(p,mapped+Z->offset,Z->count);
                //for ( u32_t r=crcX, i=0; i+4 < Z->count; i+=4 )
                //  *(u32_t*)(p+i) ^= teggo::unsigned_random(&r);
                if (Z->xcrypt)
                    g_cipher.DoCipherCBCO(p, Z->count / 8);
                byte_t* va = (byte_t*)base + Z->rva;
                if (Z->zipped)
                    zlib_decompress(p, Z->count, va, Z->rawsize);
                else
                    memcpy(va, p, Z->rawsize);
                //VirtualProtect(va,Z->rawsize,foo,&foo);
                VirtualFree(p, 0, MEM_RELEASE);
                if (Crc32(0, va, Z->rawsize) != Z->crc)
                    __asm int 3
                    ;
            }
        }

        nth->OptionalHeader.FileAlignment = zh->file_alignment;
        nth->OptionalHeader.SectionAlignment = zh->section_alignment;
        nth->OptionalHeader.SizeOfImage = nth->OptionalHeader.SizeOfImage & ~(nth->OptionalHeader.SectionAlignment - 1);
        nth->OptionalHeader.SizeOfHeaders = S->PointerToRawData;
        //SysUnmmapFile(_mapped);
        if (zh) VirtualFree(zh, 0, MEM_RELEASE);
        if (f) CloseHandle(f);
    }

    if (nth->OptionalHeader.DataDirectory[IMAGE_DIRECTORY_ENTRY_TLS].VirtualAddress)
    {
        IMAGE_TLS_DIRECTORY* tls = (IMAGE_TLS_DIRECTORY*)((byte_t*)base +
                                   nth->OptionalHeader.DataDirectory[IMAGE_DIRECTORY_ENTRY_TLS].VirtualAddress);
        //if ( tls->AddressOfCallBacks && info->tls_callback )
        //  *(unsigned*)tls->AddressOfCallBacks = info->tls_callback;
        *(unsigned*)tls->AddressOfIndex = tls_index;
    }

    getproc_t g = 0;
    #if !defined __BUILD_DEMO_CORE__
    g = getproc;
    #endif
    u32_t oldmode = SetErrorMode(0);
    if (nth->OptionalHeader.DataDirectory[IMAGE_DIRECTORY_ENTRY_BASERELOC].VirtualAddress)
        Relocate((byte_t*)base, nth);

    ulong_t act_cookie = 0;
    InitActFromManifest((byte_t*)base, Hinstance->GetMainModulePath(), &act_cookie);
    Import((byte_t*)base, nth, loadlibrary, g);
    //ActDeactivate(act_cookie);

    SetErrorMode(oldmode);
    #if !defined __BUILD_DEMO_CORE__
    if ((info->flags & STBF_EXE_ANTIHACK) && !(info->flags & (STBF_EXE_DEMO | STBF_SAVE_IAT)))
    {
        ReorderImports((byte_t*)base);
        UnsqouteExecutable((byte_t*)base, info);
    }
    #endif
}

bool RestoreExeFromFile(STB_INFO* info, void* base, pwide_t filename)
{
    //__asm int 3;
    u32_t tls_index = 0;
    IMAGE_NT_HEADERS* nth = NT_HEADERS(base); // if we have created this section, so it's PE image
    if (nth->OptionalHeader.DataDirectory[IMAGE_DIRECTORY_ENTRY_TLS].VirtualAddress)
    {
        IMAGE_TLS_DIRECTORY* tls = (IMAGE_TLS_DIRECTORY*)((byte_t*)base +
                                   nth->OptionalHeader.DataDirectory[IMAGE_DIRECTORY_ENTRY_TLS].VirtualAddress);
        tls_index = *(unsigned*)tls->AddressOfIndex;
    }

    FioProxyPtr fp = VFS->GetProxy(filename, 0);
    if (!fp) return false;

    unsigned long foo;
    VirtualProtect(base, 4096, PAGE_READWRITE, &foo);
    u32_t wasread = 0;
    if (0 != fp->Read(0, base, 4 * 1024, &wasread))
    {
        XLOG | _S*"<EXELOAD> failed to read from file '%s\\%s'"
        % fp->PackageName() % fp->ComposePath1();
        return false;
    }

    nth = NT_HEADERS(base); // if we have created this section, so it's PE image
    IMAGE_SECTION_HEADER* sec = IMAGE_FIRST_SECTION(nth);
    for (int i = 0; i < nth->FileHeader.NumberOfSections; ++i)
    {
        IMAGE_SECTION_HEADER* S = sec + i;
        unsigned vsz = cxx_max(cxx_max(S->SizeOfRawData, S->Misc.VirtualSize), nth->OptionalHeader.SectionAlignment);
        unsigned section_len = cxx_alignu(vsz, nth->OptionalHeader.SectionAlignment);
        if (!VirtualProtect((byte_t*)base + S->VirtualAddress, section_len, PAGE_EXECUTE_READWRITE, &foo))
            VirtualProtect((byte_t*)base + S->VirtualAddress, section_len, PAGE_READWRITE, &foo); // old windows versions
        memset((byte_t*)base + S->VirtualAddress, 0, section_len);
        if (S->PointerToRawData && S->SizeOfRawData)
            if (0 != fp->Read(
                    S->PointerToRawData,
                    (byte_t*)base + sec[i].VirtualAddress,
                    S->SizeOfRawData,
                    &wasread))
            {
                XLOG | _S*"<EXELOAD> failed to read section %d from file '%s\\%s'"
                % i % fp->PackageName() % fp->ComposePath1();
                return false;
            }
    }
    if (nth->OptionalHeader.DataDirectory[IMAGE_DIRECTORY_ENTRY_TLS].VirtualAddress)
    {
        IMAGE_TLS_DIRECTORY* tls = (IMAGE_TLS_DIRECTORY*)((byte_t*)base +
                                   nth->OptionalHeader.DataDirectory[IMAGE_DIRECTORY_ENTRY_TLS].VirtualAddress);
        //if ( tls->AddressOfCallBacks && info->tls_callback )
        //  *(unsigned*)tls->AddressOfCallBacks = info->tls_callback;
        *(unsigned*)tls->AddressOfIndex = tls_index;
    }

    getproc_t g = 0;
    #if !defined __BUILD_DEMO_CORE__
    g = getproc;
    #endif
    u32_t oldmode = SetErrorMode(0);
    if (nth->OptionalHeader.DataDirectory[IMAGE_DIRECTORY_ENTRY_BASERELOC].VirtualAddress)
        Relocate((byte_t*)base, nth);

    ulong_t act_cookie = 0;
    InitActFromManifest((byte_t*)base, filename, &act_cookie);
    Import((byte_t*)base, nth, loadlibrary, g);
    //ActDeactivate(act_cookie);

    SetErrorMode(oldmode);
    return true;
}

void ExecuteActivator(STB_INFO* info)
{
    void* base = 0;
    if (info->flags & STBF_EXTERNAL_ACT)
    {
        base = LoadLibraryA(info->act_file_name);
    }
    else if (info->act.offset)
    {
        BufferT<SQUOTED_IMPORT>* squoted = 0;
        #if !defined __BUILD_DEMO_CORE__
        BufferT<SQUOTED_IMPORT> sq;
        if (LoadSquotedData(info, true, sq))
            squoted = &sq;
        #endif
        base = LoadEmbeddedLibrary(0,
                                   info->act.offset,
                                   info->act.count,
                                   info->act.crc,
                                   info->flags & STBF_ACT_ZIPPED,
                                   info->flags & STBF_ACT_XCRYPED,
                                   squoted);
        if (bundle_data)
            for (int i = 0; i < bundle_data->hdr.number; ++i)
                if (bundle_data->rec[i].activator)
                {
                    bundle_data->rec[i].handle = base;
                    _ACTIVATOR = base;
                    break;
                }
    }
    else
        return;

    //if ( base )
    //  {
    //    int (__stdcall *f)(void *base) = 0;
    //    *(void**)&f = GetProcAddressIndirect(base,_XOr("_Activate@4",12,374741718),0);
    //    if ( f && f(base) ) return;
    //  }

    if (base)
    {
        void* api = QueryAPI1(base);

        int (__stdcall * f)(void* base) = 0;
        typedef int (__cdecl * fC)(void*);
        *(void**)&f = GetProcAddressIndirect(base, _XOr("_Activate@4",12,373693126), 0);
        if (!f) *(void**)&f = GetProcAddressIndirect(base, _XOr("Activate@4",11,376445488), 0);
        if (!f) *(void**)&f = GetProcAddressIndirect(base, _XOr("Activate",9,375855655), 0);

        bool ok = 1;
        ulong_t act_cookie = 0;
        InitActFromManifest((byte_t*)base, 0, &act_cookie);

        if (f)
        {
            ok = !!f(api);
        }
        else
        {
            *(void**)&f = GetProcAddressIndirect(base, _XOr("_activate",10,376052266), 0);
            ok = (f && ((fC)f)(api));
        }

        ActDeactivate(act_cookie);

        if (ok) return ;
    }

    TerminateProcess((HANDLE) - 1, -1);
}

void ActDeactivate(ulong_t cookie)
{
    //printf("%08x\n",cookie);
    if (!cookie) return;
    static BOOL (__stdcall * fDeactivateActCtx)(DWORD, ulong_t) = 0;
    if (!fDeactivateActCtx)
    {
        HMODULE hm = GetModuleHandle(_XOr("kernel32.dll",13,370743833));
        *(void**)&fDeactivateActCtx = GetProcAddress(hm, _XOr("DeactivateActCtx",17,369760778));
    }
    fDeactivateActCtx(DEACTIVATE_ACTCTX_FLAG_FORCE_EARLY_DEACTIVATION, cookie);
}

void InitActFromManifest(void* base, pwide_t mod_path, ulong_t* cookie)
{
    int MANIFEST_ID = 0;

    static BOOL (__stdcall * fActivateActCtx)(HANDLE, unsigned long*) = 0;
    static HANDLE(__stdcall * fCreateActCtxW)(ACTCTXW*) = 0;

    if (!fCreateActCtxW)
    {
        HMODULE hm = GetModuleHandle(_XOr("kernel32.dll",13,371268193));
        *(void**)&fActivateActCtx = GetProcAddress(hm, _XOr("ActivateActCtx",15,400300636));
        *(void**)&fCreateActCtxW = GetProcAddress(hm, _XOr("CreateActCtxW",14,399448655));
    }

    IMAGE_NT_HEADERS* nth = NT_HEADERS(base);

    if (nth->OptionalHeader.DataDirectory[IMAGE_DIRECTORY_ENTRY_RESOURCE].VirtualAddress)
    {
        char* rcdata = (char*)base + nth->OptionalHeader.DataDirectory[IMAGE_DIRECTORY_ENTRY_RESOURCE].VirtualAddress;
        IMAGE_RESOURCE_DIRECTORY* d = (IMAGE_RESOURCE_DIRECTORY*)((char*)base +
                                      nth->OptionalHeader.DataDirectory[IMAGE_DIRECTORY_ENTRY_RESOURCE].VirtualAddress);
        IMAGE_RESOURCE_DIRECTORY_ENTRY* e = (IMAGE_RESOURCE_DIRECTORY_ENTRY*)(d + 1);

        for (int i = 0; i < d->NumberOfIdEntries + d->NumberOfNamedEntries; ++i)
            if (e[i].Id == 0x18)
            {
                // resource id group
                IMAGE_RESOURCE_DIRECTORY_ENTRY* r = (IMAGE_RESOURCE_DIRECTORY_ENTRY*)
                                                    (rcdata + (e[i].OffsetToDirectory + sizeof(IMAGE_RESOURCE_DIRECTORY)));
                MANIFEST_ID = r->Id;
                break;
            }
    }

    if (fCreateActCtxW)
    {
        ACTCTXW actCtx = {0};
        actCtx.cbSize = sizeof(ACTCTXW);
        actCtx.lpResourceName = MAKEINTRESOURCEW(MANIFEST_ID);
        actCtx.wLangId  = 0;
        actCtx.dwFlags  =
            ACTCTX_FLAG_RESOURCE_NAME_VALID | ACTCTX_FLAG_HMODULE_VALID | ACTCTX_FLAG_LANGID_VALID
            | ACTCTX_FLAG_SET_PROCESS_DEFAULT;
        actCtx.lpSource = mod_path;
        actCtx.hModule  = (HMODULE)base;
        HANDLE h = fCreateActCtxW(&actCtx);
        bool fA = 0;
        if (h != INVALID_HANDLE_VALUE)
            fA = fActivateActCtx(h, cookie);
        if (!fA)
            xlog | _XOr("failed to activate default manifest",36,402332603);
    }
}

void RestoreDotNetDirectory(STB_INFO* info, void* base)
{
    IMAGE_NT_HEADERS* nth = NT_HEADERS(base);
    unsigned long foo = 0;
    VirtualProtect(nth->OptionalHeader.DataDirectory, sizeof(nth->OptionalHeader.DataDirectory), PAGE_READWRITE, &foo);
    nth->OptionalHeader.DataDirectory[14].VirtualAddress = info->cor20.VirtualAddress;
    nth->OptionalHeader.DataDirectory[14].Size           = info->cor20.Size;
}

StringW Cor20Version(void* base)
{
    IMAGE_NT_HEADERS* nth = NT_HEADERS(base);
    unsigned metadat_addr = nth->OptionalHeader.DataDirectory[14].VirtualAddress;
    if (metadat_addr)
    {
        IMAGE_COR20_HEADER* cor20 = (IMAGE_COR20_HEADER*)((byte_t*)base + metadat_addr);
        return StringW((char*)base + cor20->MetaData.VirtualAddress + 16);
    }
    return StringW();
}

struct xICLRMetaHost : public IUnknown
{
    virtual HRESULT STDMETHODCALLTYPE GetRuntime(
        /* [in] */ LPCWSTR pwzVersion,
        /* [in] */ REFIID riid,
        /* [retval][iid_is][out] */ LPVOID* ppRuntime) = 0;
    virtual HRESULT _1() = 0;
    virtual HRESULT STDMETHODCALLTYPE EnumerateInstalledRuntimes(
        /* [retval][out] */ IEnumUnknown** ppEnumerator) = 0;
    virtual HRESULT _3() = 0;
    virtual HRESULT _4() = 0;
    virtual HRESULT STDMETHODCALLTYPE QueryLegacyV2RuntimeBinding(
        /* [in] */ REFIID riid,
        /* [retval][iid_is][out] */ LPVOID* ppUnk) = 0;
};

struct xICLRRuntimeInfo : public IUnknown
{
    virtual HRESULT STDMETHODCALLTYPE GetVersionString(
        /* [size_is][out] */
        LPWSTR pwzBuffer,
        /* [out][in] */ DWORD* pcchBuffer) = 0;
    virtual HRESULT _2() = 0;
    virtual HRESULT _3() = 0;
    virtual HRESULT _4() = 0;
    virtual HRESULT _5() = 0;
    virtual HRESULT _6() = 0;
    virtual HRESULT STDMETHODCALLTYPE GetInterface(
        /* [in] */ REFCLSID rclsid,
        /* [in] */ REFIID riid,
        /* [retval][iid_is][out] */ LPVOID* ppUnk) = 0;
    virtual HRESULT _8() = 0;
    virtual HRESULT _9() = 0;
    virtual HRESULT _a() = 0;
    virtual HRESULT STDMETHODCALLTYPE BindAsLegacyV2Runtime(void) = 0;
};

struct xICLRRuntimeHost : public IUnknown
{
    virtual HRESULT STDMETHODCALLTYPE Start(void) = 0;
    virtual HRESULT STDMETHODCALLTYPE Stop(void) = 0;
    virtual HRESULT _1() = 0;
    virtual HRESULT _2() = 0;
    virtual HRESULT _3() = 0;
    virtual HRESULT _4() = 0;
    virtual HRESULT _5() = 0;
    virtual HRESULT STDMETHODCALLTYPE ExecuteApplication(
        /* [in] */ LPCWSTR pwzAppFullName,
        /* [in] */ DWORD dwManifestPaths,
        /* [in] */ LPCWSTR* ppwzManifestPaths,
        /* [in] */ DWORD dwActivationData,
        /* [in] */ LPCWSTR* ppwzActivationData,
        /* [out] */ int* pReturnValue) = 0;
};

static int (__stdcall* f_CorExeMain)() = 0;

void ExecuteCLR()
{
    int e = f_CorExeMain();
    ExitProcess(e);
}

void CLRfailed()
{
    TerminateProcess((HANDLE) - 1, -1);
}

void RewriteSnvf(void* snvf)
{
    unsigned long foo = 0;
    VirtualProtect(snvf, 16, PAGE_READWRITE, &foo);
    memcpy(snvf, "\xb8\x01\x00\x00\x00\xc2\x0c\x00", 8);
    VirtualProtect(snvf, 16, foo, &foo);
}

void* InitializeDotNet(STB_INFO* info, void* base)
{
    static HRESULT(__stdcall * fCoInitializeEE)(unsigned) = 0;
    static HRESULT(__stdcall * f_CorValidateImage)(void*, pwide_t) = 0;
    static HRESULT(__stdcall * fCorBindToRuntime)(pwide_t ver, pwide_t flavor, REFCLSID rclsid, REFIID riid,
            void* ppv) = 0;
    static HRESULT(__stdcall * fCLRCreateInstance)(REFCLSID clsid, REFIID riid, void* ppv) = 0;

    //__asm int 3;

    StringW cor_ver = Cor20Version(base);
    if (!cor_ver)
        if (info)
            return &CLRfailed;
        else
            return 0;

    void* mscoree = 0;

    if (!fCoInitializeEE)
        if (mscoree = GetModuleHandle(_XOr("mscoree.dll",12,395058060)))
        {
            *(void**)&fCoInitializeEE = GetProcAddressIndirect(mscoree, _XOr("CoInitializeEE",15,398269437), -1);
            *(void**)&f_CorValidateImage = GetProcAddressIndirect(mscoree, _XOr("_CorValidateImage",18,397286382), -1);
            *(void**)&fCorBindToRuntime = GetProcAddressIndirect(mscoree, _XOr("CorBindToRuntime",17,390405061), -1);
            //*(void**)&fCLRCreateInstance = GetProcAddressIndirect(mscoree,_XOr("CLRCreateInstance",18,394009404),-1);
        }
    if (fCoInitializeEE)
    {
        IUnknown* pv = 0;
        HRESULT hr = fCorBindToRuntime(+cor_ver, 0, CLSID_CLRRuntimeHost, IID_ICLRRuntimeHost, &pv);
        if (FAILED(hr))
            return CLRfailed;

        void* mscorwks = GetModuleHandle(_XOr("mscorwks",9,392829738));
        void* clr = GetModuleHandle(_XOr("clr",4,387455768));

        void* snvf1 = GetProcAddressIndirect(mscoree, _XOr("StrongNameSignatureVerification",32,385948417), -1);
        void* snvf2 = GetProcAddressIndirect(mscorwks, _XOr("StrongNameSignatureVerification",32,388766572), -1);
        void* snvf4 = GetProcAddressIndirect(clr, _XOr("StrongNameSignatureVerification",32,415767368), -1);

        if (snvf1)
            RewriteSnvf(snvf1);
        if (snvf2)
            RewriteSnvf(snvf2);
        if (snvf4)
            RewriteSnvf(snvf4);

        if (info)
        {
            IMAGE_NT_HEADERS* nth = NT_HEADERS(base);
            nth->OptionalHeader.AddressOfEntryPoint = info->oep;
        }

        if (mscorwks)
            *(void**)&f_CorExeMain = GetProcAddressIndirect(mscorwks, _XOr("_CorExeMain",12,412489878), -1);
        if (!f_CorExeMain && clr)
            *(void**)&f_CorExeMain = GetProcAddressIndirect(clr, _XOr("_CorExeMain",12,411375749), -1);
        if (f_CorExeMain)
            return &ExecuteCLR;
    }

    return &CLRfailed;
}

void* RestoreNinitDnetNmanifest(STB_INFO* info, void* base)
{
    if (RestoreExeFromFile(info, base, Hinstance->GetMainModulePath()))
    {
        //InitActFromManifest(base,Hinstance->GetMainModulePath());
        if (void* p = InitializeDotNet(0, base))
            return p;
    }
    else
    {
        __asm int 1;
        TerminateProcess((HANDLE) - 1, -1);
    }
    return 0;
}

void CheckFirstPage()
{
}

LONG CALLBACK AvHandler(PEXCEPTION_POINTERS ex)
{
    if (ex->ExceptionRecord->ExceptionCode == EXCEPTION_ACCESS_VIOLATION)
    {
        byte_t* p = (byte_t*)ex->ContextRecord->Eip;
        if (!memcmp(p, "\xc7\x44\x24\x04", 4) && *(p + 8) == 0xe9)
        {
            ex->ContextRecord->ContextFlags = CONTEXT_ALL;
            *(unsigned*)(ex->ContextRecord->Esp + 4) = *(unsigned*)(p + 4);
            ex->ContextRecord->Eip = *(unsigned*)(p + 9) + ex->ContextRecord->Eip + 13;
            return EXCEPTION_CONTINUE_EXECUTION;
        }
    }
    if (g_stbi2f.insinity &&
        (ex->ExceptionRecord->ExceptionCode == EXCEPTION_ACCESS_VIOLATION
         || ex->ExceptionRecord->ExceptionCode == EXCEPTION_BREAKPOINT))
    {
        // fault
        int minidump_ready = 0;
        wchar_t d[1024 * 4];
        wcscat(d, Hinstance->GetMainModulePath());
        wcscat(d, L".dmp");
        APIF->AcquireFilterLock(); // no release!!!!
        HANDLE f = CreateFileW(d, GENERIC_WRITE, 0, 0, CREATE_ALWAYS, 0, 0);
        if (f && f != INVALID_HANDLE_VALUE)
        {
            MINIDUMP_EXCEPTION_INFORMATION mei;
            mei.ThreadId = GetCurrentThreadId();
            mei.ExceptionPointers = ex;
            mei.ClientPointers = 1;
            BOOL (__stdcall * fMiniDumpWriteDump)(HANDLE hProcess, DWORD ProcessId, HANDLE hFile,
                                                  MINIDUMP_TYPE DumpType,
                                                  PMINIDUMP_EXCEPTION_INFORMATION ExceptionParam,
                                                  PMINIDUMP_USER_STREAM_INFORMATION UserStreamParam,
                                                  PMINIDUMP_CALLBACK_INFORMATION CallbackParam) = 0;
            if (HMODULE dbghlp = LoadLibraryA("dbghelp.dll"))
                if (*(void**)&fMiniDumpWriteDump = GetProcAddressIndirect(dbghlp, "MiniDumpWriteDump", -1))
                    minidump_ready = fMiniDumpWriteDump(
                                         (HANDLE) - 1, GetCurrentProcessId(), f,
                                         (g_stbi2f.minidump_full ? MiniDumpWithFullMemory : MiniDumpNormal),
                                         &mei, 0, 0);
            CloseHandle(f);
        }
        sprintf((char*)d, "exception %08x at %08x", ex->ExceptionRecord->ExceptionCode, ex->ContextRecord->Eip);
        if (minidump_ready)
            MessageBoxA(0, (char*)d, "minidump ready", 0);
        else
            MessageBoxA(0, (char*)d, "failed to write minidump", 0);
        TerminateProcess((HANDLE) - 1, -1);
    }
    return EXCEPTION_CONTINUE_SEARCH;
}

void SetAvHandler(STB_INFO* info)
{
    PVOID(__stdcall * fAddVectoredExceptionHandler)(ULONG FirstHandler, PVECTORED_EXCEPTION_HANDLER VectoredHandler) = 0;
    *(void**)&fAddVectoredExceptionHandler = GetProcAddressIndirect(_KERN32, _XOr("AddVectoredExceptionHandler", 28,
            453190913), -1);
    if (fAddVectoredExceptionHandler)
    {
        fAddVectoredExceptionHandler(1, AvHandler);
    }
}

void ReConfigureDEP()
{
    BOOL (__stdcall * fSetProcessDEPPolicy)(DWORD) = 0;
    *(void**)&fSetProcessDEPPolicy = GetProcAddressIndirect(_KERN32, _XOr("SetProcessDEPPolicy",20,414587126), -1);
    if (fSetProcessDEPPolicy)
    {
        if (!fSetProcessDEPPolicy(0))
            fSetProcessDEPPolicy(2);
    }
    else
    {
        u32_t exflags = 0x72;
        NtSetInformationProcess((HANDLE) - 1, 0x22, &exflags, 4);
    }
}

CRPROC_INFO* GetNestedProcessInfo(unsigned* isNestedProcess, STB_INFO* info)
{
    *isNestedProcess = 0;
    return 0;
}

extern "C" /*__declspec(dllexport)*/
void* __stdcall Inity(void* ntdll, void* kern32, STB_INFO* info, void* base)
{
    //__asm int 3;

    unsigned catalog_base = 0;
    g_Inject_Flags = info->flags & (0x0ff | STBF_REGISTER_OCX | STBF_SAVE_IAT);
    g_Inject_Into_Child = info->flags & TEGGOVBOX_CORE_INJECT;

    if (info->flags & STBF_EXTENDED)
    {
        g_stbi2f = ((STB_INFO2*)info)->f;
        catalog_base = ((STB_INFO2*)info)->catalog;
        g_License_Level = ((STB_INFO2*)info)->liclevel;
    }
    else
    {
        g_License_Level = 100;
        memset(&g_stbi2f, 0, sizeof(g_stbi2f));
    }
    //__asm int 3;

    do_save_iat = info->flags & STBF_SAVE_IAT;

    _FirstCall(ntdll, kern32, info, base);

    ReConfigureDEP(); // trying to disable DEP

    if (!(info->flags & STBF_EXTERNAL_CORE))
    {
        g_Core_Size   = info->core.count;
        g_Core_Offset = info->core.offset;
    }

    CheckFirstPage();
    SetAvHandler(info); // setting AV handler

    _Inity(info->flags, catalog_base);
    g_sid_0 = *(u32_t*)info->xcrypt_key;
    g_cipher.SetupDecipher(info->xcrypt_key);

    //if ( info->flags & STBF_EXE_ZIPPED )

    // if (!(info->flags & STBF_EMBEDDED_LOADING))
    //  InitActFromManifest(base,Hinstance->GetMainModulePath());

    if (info->flags & STBF_EXTENDED)
    {
        g_Registery_Mask = Query_UCS_Text(&((STB_INFO2*)info)->regmask);
        Load_Registry_Files(+g_Registery_Mask);
    }

    if (!info->extra_count || info->flags & STBF_EMBEDDED_LOADING)
        exeFilterIsEnabled = false;
    else
        VFS->SelfExtraData(info->extra_offs, info->extra_count);

    if (info->flags & STBF_CMDLINE)
        ChangeCommandLine(info->cmdline);

    if (info->flags & STBF_REGISTER_OCX)
        Register_On_Start();

    if (info->bundle.count) LoadBundledDlls(info, base);
    ExecuteActivator(info);

    SplashScreen();

    if (info->flags & STBF_EMBEDDED_LOADING)
    {
        if (void* p = RestoreNinitDnetNmanifest(info, base))
            return p;
    }
    else
    {
        if (info->flags & STBF_EXE_ZIPPED)
        {
            RestoreExecutable(info, base);
            if (info->flags & STBF_EXE_DOTNET)
                return InitializeDotNet(info, base);
        }
        else
        {
            RestoreImports(base);
            if (info->flags & STBF_EXE_DOTNET)
            {
                RestoreDotNetDirectory(info, base);
                return InitializeDotNet(info, base);
            }
        }
    }

    unsigned oep = (info->flags & STBF_EMBEDDED_LOADING) ? NT_HEADERS(base)->OptionalHeader.AddressOfEntryPoint : info->oep;
    return (char*)base + oep;
}

int __stdcall DllMain(void*, unsigned reason, void*)
{
    if (reason == DLL_PROCESS_DETACH)
    {
    }
    return 1;
}

extern "C" __declspec(dllexport) TEGGOVBOX_FUNCTIONS_TABLE* __stdcall Install(unsigned flags, void* ntdll, void* kern32,
        void* base)
{
    _FirstCall(ntdll, kern32, 0, base);
    _Inity(flags, 0);
    return g_functions_table;
}

void* Mmap_Address_Space(void* prefer, u32_t img_size, bool relocatable)
{
    HANDLE h = CreateFileMappingA((void*) - 1, 0, PAGE_EXECUTE_READWRITE, 0, img_size, 0);
    if (!h) h = CreateFileMappingA((void*) - 1, 0, PAGE_READWRITE, 0, img_size, 0);
    void* ptr = MapViewOfFileEx(h, FILE_MAP_WRITE | FILE_MAP_READ | FILE_MAP_EXECUTE, 0, 0, 0, prefer);
    if (!ptr && relocatable)
    {
        ptr = MapViewOfFile(h, FILE_MAP_WRITE | FILE_MAP_READ | FILE_MAP_EXECUTE, 0, 0, 0);
    }
    CloseHandle(h);
    return ptr;
}

void* Mmap_Exe_Address_Space(IMAGE_NT_HEADERS* nth, bool* relocated)
{
    bool relocatable = !!nth->OptionalHeader.DataDirectory[IMAGE_DIRECTORY_ENTRY_BASERELOC].VirtualAddress;
    void* p = Mmap_Address_Space((void*)nth->OptionalHeader.ImageBase, nth->OptionalHeader.SizeOfImage, relocatable);
    if (p != (void*)nth->OptionalHeader.ImageBase)
    {
        XLOG | _S* _XOr("<EXELOAD> relocated %08x -> %08x",33,413669608)
        % nth->OptionalHeader.ImageBase
        % p;
        *relocated = 1;
    }
    if (!p)
        XLOG | _XOr("<EXELOAD> failed to allocate address space",43,410458167);
    return p;
}

void* Load_Exe_From_File(pwide_t filename, bool reset_peb_base, bool kill_current)
{
    bool relocated = 0;
    u32_t wasread = 0;
    FioProxyPtr fp = VFS->GetProxy(filename, 0);

    if (!fp)
    {
        XLOG | _S* _XOr("<EXELOAD> failed to open internal file '%s'",44,403118087) % filename;
        __asm mov ecx, __LINE__;
        __asm int 3;
    }

    IMAGE_NT_HEADERS l_nth = {0};
    if (0 != fp->ReadImageNtHeaders(&l_nth))
    {
        XLOG | _S* _XOr("<EXELOAD> failed to read from file '%s\\%s'",43,433395797)
        % fp->PackageName() % fp->ComposePath1();
        __asm mov ecx, __LINE__;
        __asm int 3;
    }

    if (kill_current)
    {
        _XPEB* peb = (_XPEB*)_PEB();
        if (!UnmapViewOfFile(peb->ImageBaseAddress))
        {
            XLOG | _S* _XOr("<EXELOAD> failed to unmap original PE",38,435886523);
            __asm mov ecx, __LINE__;
            __asm int 3;
        }
    }

    void* base = Mmap_Exe_Address_Space(&l_nth, &relocated);

    if (0 != fp->Read(0, base, 4 * 1024, &wasread))
    {
        XLOG | _S* _XOr("<EXELOAD> failed to read from file '%s\\%s'",43,428743054)
        % fp->PackageName() % fp->ComposePath1();
        __asm mov ecx, __LINE__;
        __asm int 3;
    }

    IMAGE_NT_HEADERS* nth = NT_HEADERS(base); // if we have created this section, so it's PE image
    IMAGE_SECTION_HEADER* sec = IMAGE_FIRST_SECTION(nth);

    for (int i = 0; i < nth->FileHeader.NumberOfSections; ++i)
    {
        IMAGE_SECTION_HEADER* S = sec + i;
        unsigned vsz = cxx_max(cxx_max(S->SizeOfRawData, S->Misc.VirtualSize), nth->OptionalHeader.SectionAlignment);
        memset((byte_t*)base + S->VirtualAddress, 0, cxx_alignu(vsz, nth->OptionalHeader.SectionAlignment));
        if (S->PointerToRawData && S->SizeOfRawData)
            if (0 != fp->Read(
                    S->PointerToRawData,
                    (byte_t*)base + sec[i].VirtualAddress,
                    S->SizeOfRawData,
                    &wasread))
            {
                XLOG | _S* _XOr("<EXELOAD> failed to read section %d from file '%s\\%s'",54,425466332)
                % i % fp->PackageName() % fp->ComposePath1();
                __asm mov ecx, __LINE__;
                __asm int 3;
            }
    }

    u32_t oldmode = SetErrorMode(0);
    if (relocated && nth->OptionalHeader.DataDirectory[IMAGE_DIRECTORY_ENTRY_BASERELOC].VirtualAddress)
        Relocate((byte_t*)base, nth);

    if (reset_peb_base)
    {
        _XPEB* peb = (_XPEB*)_PEB();
        LDR_MODULE* q = (LDR_MODULE*)peb->LoaderData->InMemoryOrderModuleList.Flink;
        while (q && q->BaseAddress != peb->ImageBaseAddress)
            q = (LDR_MODULE*)q->ModuleList.Flink;
        q->BaseAddress = base;
        peb->ImageBaseAddress = base;
    }

    ulong_t act_cookie = 0;
    InitActFromManifest((byte_t*)base, filename, &act_cookie);
    Import((byte_t*)base, nth, loadlibrary, getproc);
    SetErrorMode(oldmode);

    /*if ( nth->OptionalHeader.DataDirectory[IMAGE_DIRECTORY_ENTRY_TLS].VirtualAddress )
    {
    __asm int 3;
    unsigned tls_index = TlsAlloc();
    IMAGE_TLS_DIRECTORY *tls = (IMAGE_TLS_DIRECTORY*)((byte_t*)base+nth->OptionalHeader.DataDirectory[IMAGE_DIRECTORY_ENTRY_TLS].VirtualAddress);
    *(unsigned*)tls->AddressOfIndex = tls_index;
    unsigned tls_size = tls->EndAddressOfRawData - tls->StartAddressOfRawData + tls->SizeOfZeroFill;
    void *Q = VirtualAlloc(0,tls_size,MEM_COMMIT,PAGE_READWRITE);
    TlsSetValue(tls_index,Q);
    }*/

    return base;
}

void Open_Inherited(XOJUMAN_STARTUP_INFO* sti)
{
    g_Injector_Master_Id = sti->master_id;
    g_Core_Handle = (HANDLE)sti->core_handle;
    g_Core_Size = sti->core_size;

    for (int i = 0; i < sti->pkg_count; ++i)
    {
        XOJUMAN_INHERIT_PACKAGE* pkg = &sti->pkg[i];
        VFS->MountS_At((HANDLE)pkg->handle, pkg->name, pkg->pwd, pkg->offs);
        memset(pkg, 0, sizeof(*pkg));
    }
}

void Set_Process_Name(wchar_t* process_name)
{
    _XPEB* peb = (_XPEB*)_PEB();
    MY_RTL_USER_PROCESS_PARAMETERS* para = peb->ProcessParameters;

    RtlInitUnicodeString(&para->ImagePathName, process_name);
    //RtlInitUnicodeString(&para->WindowTitle,processname);

    LDR_MODULE* q = (LDR_MODULE*)peb->LoaderData->InMemoryOrderModuleList.Flink;
    while (q && q->BaseAddress != peb->ImageBaseAddress)
        q = (LDR_MODULE*)q->ModuleList.Flink;

    if (q)
    {
        RtlInitUnicodeString(&q->FullDllName, para->ImagePathName.Buffer);
        int k = StrFindRChar(para->ImagePathName.Buffer, (wchar_t)'\\', -1, para->ImagePathName.Length / 2);
        if (k > 0)
            RtlInitUnicodeString(&q->BaseDllName, para->ImagePathName.Buffer + k + 1);
    }
}

#ifndef __BUILD_VR_CORE__

extern "C" __declspec(dllexport) void* __stdcall SDK_Inject(unsigned flags, void* ntdll, void* kern32, void* base,
        XOJUMAN_STARTUP_INFO* sti)
{
    //__asm int 3;

    _NTDLL = ntdll;
    _KERN32 = kern32;
    InitHeapAndSplicer(0, base);
    _ntdll_xcall_fullinit();

    _USER32 = GetModuleHandleA(_XOr("user32",7,420944151));
    _ADVA32 = GetModuleHandleA(_XOr("advapi32",9,421075225));
    _imp__GetModuleHandleW__4 = _GetModuleHandleW;
    _imp__GetProcAddress__8 = _GetProcAddress;

    if (sti->appname[0])
    {
        Set_Process_Name(sti->appname);
    }

    _DllMainCRTStartup(base, DLL_PROCESS_ATTACH, 0);

    g_License_Level = 100;
    g_Inject_Flags = flags;
    g_Inject_Into_Child = flags & TEGGOVBOX_CORE_INJECT;

    #if !defined __BUILD_DEMO_CORE__
    do_save_iat = flags & STBF_SAVE_IAT;
    #endif

    ResetMillis();

    if (sti->container[0])
        VFS->containerName = sti->container;
    else
        VFS->containerName = Hinstance->GetMainModulePath();

    ReConfigureDEP(); // trying to disable DEP
    CheckFirstPage();
    //SetAvHandler(0); // setting AV handler
    _Inity(flags, 0);

    XLOG | _XOr("working in Injectin mode",25,419961096);

    Open_Inherited(sti);
    exeFilterIsEnabled = false;

    DWORD foo = 0;
    g_Registery_Mask = sti->regmask;
    Load_Registry_Files(+g_Registery_Mask);

    if (g_Inject_Flags & STBF_REGISTER_OCX)
        Register_On_Start();

    if (sti->appname[0])
    {
        if (sti->reserved_space)
            VirtualFree(sti->reserved_space, 0, MEM_RELEASE);
        if (void* img_base = Load_Exe_From_File(sti->appname, 1, 1))
        {
            IMAGE_NT_HEADERS* nth = NT_HEADERS(img_base);
            return (char*)img_base + nth->OptionalHeader.AddressOfEntryPoint;
        }
        else
        {
            __asm mov ecx, __LINE__;
            __asm int 3;
        }
        TerminateProcess((HANDLE) - 1, -1);
    }
    else
    {
        _XPEB* peb = (_XPEB*)_PEB();
        IMAGE_NT_HEADERS* nth = NT_HEADERS(peb->ImageBaseAddress);
        // restory entry point
        memcpy((char*)peb->ImageBaseAddress + nth->OptionalHeader.AddressOfEntryPoint,
               sti->orign,
               sizeof(sti->orign));
        return (char*)peb->ImageBaseAddress + nth->OptionalHeader.AddressOfEntryPoint;
    }
}

extern "C" __declspec(dllexport) void* __stdcall SDK_Init(unsigned flags, void* ntdll, void* kern32, void* base,
        void* key, void* core, int core_size)
{
    //__asm int 3;
    _FirstCall(ntdll, kern32, 0, base);
    //ReConfigureDEP(); // trying to disable DEP
    //SetAvHandler(0); // setting AV handler
    #if defined __BUILD_DEMO_CORE__
    ShowSDKDemoBanner();
    #endif
    g_License_Level = 100;
    _Inity(flags, 0);
    XLOG | _XOr("working in SDK mode",20,421730659);
    g_Core_Handle = core;
    g_Inject_Flags = flags;
    g_Inject_Into_Child = flags & TEGGOVBOX_CORE_INJECT;
    if (core) g_Core_Size = core_size;
    return QueryAPI();
}

#endif

HANDLE Query_Core_Handle(int* corSize)
{
    if (!g_Core_Handle && g_Core_Size)
    {
        APIF->AcquireFilterLock();
        HANDLE f = CreateFileW(+VFS->containerName, GENERIC_READ, FILE_SHARE_READ, 0, OPEN_EXISTING, 0, 0);
        APIF->ReleaseFilterLock();
        g_Core_Handle = CreateFileMappingA(INVALID_HANDLE_VALUE, 0, PAGE_READWRITE, 0, g_Core_Size, 0);
        g_Core_Ptr    = MapViewOfFile(g_Core_Handle, FILE_MAP_WRITE | FILE_MAP_READ, 0, 0, 0);
        SetFilePointer(f, g_Core_Offset, 0, 0);
        AuxReadFile(f, g_Core_Ptr, g_Core_Size);
        g_cipher.DoCipherCBCO(g_Core_Ptr, g_Core_Size / 8);
        CloseHandle(f);
    }
    if (corSize) *corSize = g_Core_Size;
    return g_Core_Handle;
}

void* Query_Core(int* corSize)
{
    if (!g_Core_Ptr)
    {
        Query_Core_Handle(0);
        if (g_Core_Handle)
            g_Core_Ptr = MapViewOfFile(g_Core_Handle, FILE_MAP_WRITE | FILE_MAP_READ, 0, 0, 0);
    }
    *corSize = g_Core_Size;
    return g_Core_Ptr;
}

