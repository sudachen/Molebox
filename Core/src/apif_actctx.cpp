
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

struct RTL_CREATEACTCTXW_ARG
{
    const ACTCTXW* ptr;
};

struct InplaceStrm
{
    IStream_Vtbl* vtbl;
    byte_t const* r_ptr;
    byte_t const* r_ptr_E;
    byte_t const* r_ptr_S;
};

HRESULT __stdcall InplaceStrm_QueryInterface( void* This, REFIID riid, void** ppvObject)
{ *ppvObject = This; return S_OK;}
ULONG   __stdcall InplaceStrm_AddRelRef( void* This )
{ return 0; }

HRESULT __stdcall InplaceStrm_Read( void* This, void* pv, ULONG cb, ULONG* pcbRead)
{
    InplaceStrm* self = (InplaceStrm*)This;
    if ( cb > self->r_ptr_E-self->r_ptr ) cb = self->r_ptr_E-self->r_ptr;
    memcpy(pv,self->r_ptr,cb);
    self->r_ptr += cb;
    if ( pcbRead ) *pcbRead = cb;
    return S_OK;
}
HRESULT __stdcall InplaceStrm_Stat( void* This, STATSTG* pstatstg, DWORD grfStatFlag)
{
    InplaceStrm* self = (InplaceStrm*)This;
    memset(pstatstg,0,sizeof(*pstatstg));
    pstatstg->cbSize.LowPart = self->r_ptr_E-self->r_ptr;
    return S_OK;
}

IStream_Vtbl InplaceStrm_Vtbl =
{
    InplaceStrm_QueryInterface,
    InplaceStrm_AddRelRef,
    InplaceStrm_AddRelRef,
    InplaceStrm_Read,
    0, //Write
    0, //Seek
    0, //SetSize
    0, //CopyTo
    0, //Commit
    0, //Revert
    0, //LockRegion
    0, //UnlockRegion
    InplaceStrm_Stat,
    0, //Clone
};

bool _IsWOW64()
{
    static int wow64 = 0;

    if ( !wow64 )
    {
        typedef int (__stdcall *tIsWow64Process)(HANDLE, BOOL*);
        tIsWow64Process fIsWow64Process = 0;
        fIsWow64Process = (tIsWow64Process)GetProcAddress((HMODULE)_KERN32,"IsWow64Process");
        if ( !fIsWow64Process )
            wow64 = -1;
        else
        {
            BOOL b = 0;
            wow64 = (fIsWow64Process((HANDLE)-1,&b) && b)?1:-1;
        }
    }

    return wow64 > 0;
}

struct XpSxSdata
{
    /* +00 */ unsigned flags;
    /* +04 */ unsigned arch_langid;
    /* +08 */ wchar_t const* assmdir;
    /* +0c */ wchar_t const* S60; // 0
    /* +10 */ IStream* stManifest;
    /* +14 */ wchar_t const* manifest_source;
    /* +18 */ unsigned x123;
    /* +1c */ IStream* stConfig;
    /* +20 */ wchar_t const* config_source;
    /* +24 */ unsigned dsntmatter[4];
    /* +34 */ HANDLE   mapping;
    /* +38 */ unsigned insurance[4];
    enum { SXTYPE = 0x50 };
    void SetLocale(pwide_t) {}
    void SetWOW64() { arch_langid |= 0xa; }
    HANDLE Mapping(unsigned) { return mapping; }
};

struct SevenSxSdata
{
    /* +00 */ unsigned flags;
    /* +04 */ unsigned arch; // 0 for intelx86
    /* +08 */ wchar_t const* locale; // L"en_US"
    /* +0c */ wchar_t const* assmdir;
    /* +10 */ wchar_t const* S60; // 0
    /* +14 */ IStream* stManifest;
    /* +18 */ wchar_t const* manifest_source;
    /* +1c */ unsigned x123;
    /* +20 */ IStream* stConfig;
    /* +24 */ wchar_t const* config_source;
    /* +28 */ unsigned dsntmatter0_0;
    /* +2c */ unsigned dsntmatter1_1;
    /* +30 */ unsigned dsntmatter2_0;
    /* +34 */ HANDLE mapping62;
    /* +38 */ HANDLE mapping60;
    /* +3c */ HANDLE mapping61;
    /* +3c */ unsigned insurance[16];
    enum { SXTYPE = 0x60 };
    void SetLocale(pwide_t S) {locale = S;}
    void SetWOW64() { arch |= 0xa; }
    HANDLE Mapping(unsigned osv)
    {
        return osv == 60?mapping60
               :osv == 61?mapping61
               :mapping62;
    }
};

struct ActCtxFilter : APIfilter
{
    //HOA handles_;
    //SUBJECT subject_;
    //Tlocker l_;

    wchar_t langs_[128];
    unsigned osversion_;
    BOOL (__stdcall* sxsGenerateActivationContext)(void*);
    //HRESULT (__stdcall *rtlCreateActivationContext)(unsigned flags, void *conext, unsigned extrabytes, void * notification, void * notificationctx, HANDLE *result);
    HRESULT (__stdcall* rtlCreateActivationContext)(unsigned, void*, unsigned, void*, void*, HANDLE*);
    ActCtxFilter() :
        sxsGenerateActivationContext(0),
        rtlCreateActivationContext(0)
    {
        *(void**)&rtlCreateActivationContext = GetProcAddressIndirect(_NTDLL,
		_XOr("RtlCreateActivationContext",27,9831310),0);
        if ( rtlCreateActivationContext )
        {
            HMODULE hm = LoadLibrary(_XOr("sxs.dll",8,16122798));
            if ( hm )
                *(void**)&sxsGenerateActivationContext = GetProcAddressIndirect(hm,
		_XOr("SxsGenerateActivationContext",29,16319449),0);
        }
        if ( !sxsGenerateActivationContext )
            *(void**)&sxsGenerateActivationContext = (void*)-1;
        OSVERSIONINFO osvi = {sizeof(OSVERSIONINFO),0};
        GetVersionEx(&osvi);
        osversion_ = osvi.dwMajorVersion*10 + osvi.dwMinorVersion;
        if ( osversion_ >= 60 ) // VISTA
        {
            unsigned long p1,p2=128;
            if ( !GetThreadPreferredUILanguages(/*MUI_LANGUAGE_NAME*/8,&p1,langs_,&p2) )
                memcpy(langs_,L"en-US\0",7*2);
        }
    };

    void FindAppName(StringW* appname, wchar_t const* qAppname)
    {
        if ( !qAppname )
            *appname = Hinstance->GetMainModulePath();
        else
            *appname = qAppname;
    }

    void FindAssmDirectory(
        StringW* assmdir, pwide_t appname, pwide_t source, pwide_t qAssmdir)
    {
        if (qAssmdir)
            *assmdir = qAssmdir;
        else if ( source )
            *assmdir = GetDirectoryOfPath(source);
        else if ( appname )
            *assmdir = GetDirectoryOfPath(appname);
        //else
        //  *assmdir = GetSystemLibrariesDirectory();
    }

    bool GetManifestFromFile(pwide_t fname, StringA* manifest, StringW* manifest_source)
    {
        if ( unsigned len = LengthOfFile(fname) )
            if ( void* data = SysMmapReadonlyFile(fname,EMPTYEF) )
            {
                manifest->SetRange((char*)data,len);
                *manifest_source = fname;
                SysUnmmapFile(data);
                return true;
            }
        return false;
    }
    bool GetManifestFromPE(HMODULE hmodule, pwide_t source, pwide_t rcid, unsigned langid, StringA* manifest,
                           StringW* manifest_source)
    {
        bool retval = false;
        bool unload = false;
        if ( !hmodule || hmodule == (HMODULE)-1 )
        {
            //if ( !hmodule )
            //  {
            hmodule = LoadLibraryExW((LPWSTR)source,0,LOAD_LIBRARY_AS_DATAFILE);
            unload = true;
            //  }
        }
        if ( hmodule )
        {
            if ( HRSRC rc = FindResourceExW( hmodule, (LPWSTR)RT_MANIFEST, rcid, langid?langid:MAKELANGID(LANG_NEUTRAL, SUBLANG_NEUTRAL)) )
                if ( HGLOBAL hg = LoadResource( hmodule, rc ) )
                {
                    unsigned len = SizeofResource(hmodule,rc);
                    void* data = (void*)hg;//LockResource(hg);
                    manifest->SetRange((char*)data,len);
                    *manifest_source = source;
                    //FreeResource(hg);
                    retval = true;
                }
        }
        if ( unload ) FreeLibrary(hmodule);
        return retval;
    }

    void GetManifest(
        StringA* manifest, StringW* manifest_source,
        pwide_t appname, pwide_t assmdir, pwide_t qSource,
        HMODULE hmodule, unsigned langid, pwide_t rcid)
    {
        //__asm int 3;
        bool unloadmod;
        StringW source = qSource?qSource:appname;
        if ( !source.EndsWithI(_XOrL(".Manifest",0,0)) )
        {
            if ( rcid == (LPCWSTR)-1 )
            {if ( GetManifestFromFile(0|_S*_XOrL("%s.Manifest",0,0)%source,manifest,manifest_source) ) return;}
            else if ( rcid != 0 )
            {
                StringW rcname = (unsigned)rcid&0xffff0000 ? rcid : (0|_S*L"%d"%rcid);
                if ( GetManifestFromFile(0|_S*_XOrL("%s.%s.Manifest",0,0)%source%rcname,manifest,manifest_source) ) return;
            }
            if ( GetManifestFromFile(0|_S*_XOrL("%s.Manifest",0,0)%source,manifest,manifest_source) )
                return;
            if ( GetManifestFromPE(hmodule,+source,rcid,langid,manifest,manifest_source) )
                return;
        }
        else
            GetManifestFromFile(+source,manifest,manifest_source);
    }

    void GetConfig(
        StringA* manifest, StringW* manifest_source,
        pwide_t appname, pwide_t assmdir, pwide_t qSource,
        HMODULE hmodule, unsigned wLangId, pwide_t rcid)
    {
        bool unloadmod;
        StringW source = qSource?qSource:appname;
        if ( !source.EndsWithI(_XOrL(".Config",0,0)) )
        {
            if ( rcid == (LPCWSTR)-1 )
            {if ( GetManifestFromFile(0|_S*_XOrL("%s.Config",0,0)%source,manifest,manifest_source) ) return;}
            else if (rcid != 0)
            {
                StringW rcname = (unsigned)rcid&0xffff0000 ? rcid : (0|_S*L"%d"%rcid);
                if ( GetManifestFromFile(0|_S*_XOrL("%s.%s.Config",0,0)%source%rcname,manifest,manifest_source) ) return;
            }
            if ( GetManifestFromFile(0|_S*_XOrL("%s.Config",0,0)%source,manifest,manifest_source) )
                return;
        }
        else
            GetManifestFromFile(+source,manifest,manifest_source);
    }

    template <class T>
    bool CreateSxSActCtxT(
        T& sxsd,
        pwide_t appname,pwide_t assmdir,
        StringA const& manifest, pwide_t manifest_source,
        StringA const& config, pwide_t config_source,
        unsigned* result
    )
    {
        enum { SXTYPE = typename T::SXTYPE };
        InplaceStrm strm_Manifest = { &InplaceStrm_Vtbl, 0 };
        InplaceStrm strm_Config = { &InplaceStrm_Vtbl, 0 };
        sxsd.assmdir = assmdir;
        sxsd.S60 = appname;
        sxsd.x123 = 1;
        sxsd.SetLocale(langs_);

        if (_IsWOW64())
        {
            sxsd.SetWOW64();
        }

        if ( manifest )
        {
            strm_Manifest.r_ptr_S = strm_Manifest.r_ptr = (byte_t*)+manifest;
            strm_Manifest.r_ptr_E = strm_Manifest.r_ptr_S + manifest.Length();
            sxsd.stManifest = (IStream*)&strm_Manifest;
            sxsd.manifest_source = +manifest_source;
        }
        if ( config )
        {
            strm_Config.r_ptr_S = strm_Config.r_ptr = (byte_t*)+config;
            strm_Config.r_ptr_E = strm_Config.r_ptr_S + config.Length();
            sxsd.stConfig = (IStream*)&strm_Config;
            sxsd.config_source = +config_source;
        }
        if ( sxsGenerateActivationContext(&sxsd) )
            if ( sxsd.Mapping(osversion_) )
            {
                void* actctx = MapViewOfFile(sxsd.Mapping(osversion_),FILE_MAP_ALL_ACCESS,0,0,0);
                if ( HRESULT r = rtlCreateActivationContext(0,actctx,0,0,0,(HANDLE*)result) )
                {
                    XLOG | _S* _XOr("<APIF/CTX> RtlCrateActivationContext failed with error %08x",60,14287866)%r;
                    UnmapViewOfFile(actctx);
                }
                else
                {
                    XLOG | _S* _XOr("<APIF/CTX> activation context successful created -> %08x",57,18416441)%*result;
                    return true;
                }
            }

        XLOG | _S* _XOr("<APIF/CTX> SxsGenerateActivationContext failed",47,22872949);
        return false;
    }

    bool CreateSxSActCtx(
        pwide_t appname,pwide_t assmdir,
        StringA const& manifest, pwide_t manifest_source,
        StringA const& config, pwide_t config_source,
        unsigned* result
    )
    {
        int r2 = 0;//GetSystemMetrics(/*SM_SERVERR2*/89);
        XDBG|_S*"R2 == %d" % r2;
        switch ( osversion_ )
        {
            case 51: // XP
            { XpSxSdata d = {0}; return CreateSxSActCtxT(d,appname,assmdir,manifest,manifest_source,config,config_source,result); }
            case 52: // Server 2003 R2/.NET
                if ( r2 )
                { SevenSxSdata d = {0}; return CreateSxSActCtxT(d,appname,assmdir,manifest,manifest_source,config,config_source,result); }
                else
                { XpSxSdata d = {0}; return CreateSxSActCtxT(d,appname,assmdir,manifest,manifest_source,config,config_source,result); }
            case 60: // Vista
            { SevenSxSdata d = {0}; return CreateSxSActCtxT(d,appname,assmdir,manifest,manifest_source,config,config_source,result); }
            case 61: // Seven
            { SevenSxSdata d = {0}; return CreateSxSActCtxT(d,appname,assmdir,manifest,manifest_source,config,config_source,result); }
            case 62: // Windows 8
            {
                SevenSxSdata d = {0};
                d.dsntmatter1_1 = 1;
                return CreateSxSActCtxT(d,appname,assmdir,manifest,manifest_source,config,config_source,result);
            }
            default:
                return false;
        }
    }

    APIF_STATUS DoCreateActCtxW(void* _a, unsigned* result)
    {

        RTL_CREATEACTCTXW_ARG* args = (RTL_CREATEACTCTXW_ARG*)_a;
        if ( !args->ptr) return APIF_CONTINUE;

        XDBG|_S* _XOr(".apif.CreatActCtxW ptr: %08x",29,26149511) % args->ptr;
        XDBG|_S* _XOr(".apif.CreatActCtxW flags: %08x, proc: %08x, resid: %08x, langid: %08x, hmod: %08x",82,32113322)
        % args->ptr->dwFlags
        % args->ptr->wProcessorArchitecture
        % args->ptr->lpResourceName
        % args->ptr->wLangId
        % args->ptr->hModule;
        //XDBG|_S*_XOr(".apif.CreatActCtxW strs: \n      '%s',\n      '%s',\n      '%s'",61,33554944)
        //  % args->ptr->lpSource
        //  % args->ptr->lpAssemblyDirectory
        //  % args->ptr->lpApplicationName;

        if ( (void*)sxsGenerateActivationContext != (void*)-1 )
        {
            ACTCTXW const* actctx = args->ptr;
            StringA config;
            StringW config_source;
            StringA manifest;
            StringW manifest_source;
            StringW appname;
            StringW assmdir;
            FindAppName(&appname, (actctx->dwFlags & ACTCTX_FLAG_APPLICATION_NAME_VALID? actctx->lpApplicationName : 0 ));
            FindAssmDirectory(&assmdir,
                              +appname,
                              actctx->lpSource,
                              (actctx->dwFlags & ACTCTX_FLAG_ASSEMBLY_DIRECTORY_VALID? actctx->lpAssemblyDirectory : 0));
            GetManifest(
                &manifest,
                &manifest_source,
                +appname,
                +assmdir,
                actctx->lpSource,
                (actctx->dwFlags & ACTCTX_FLAG_HMODULE_VALID? actctx->hModule : (HMODULE)-1),
                (actctx->dwFlags & ACTCTX_FLAG_LANGID_VALID? actctx->wLangId : 0),
                (actctx->dwFlags & ACTCTX_FLAG_RESOURCE_NAME_VALID? actctx->lpResourceName : (LPCWSTR)-1));
            GetConfig(
                &config,
                &config_source,
                +appname,
                +assmdir,
                actctx->lpSource,
                (actctx->dwFlags & ACTCTX_FLAG_HMODULE_VALID? actctx->hModule : (HMODULE)-1),
                (actctx->dwFlags & ACTCTX_FLAG_LANGID_VALID? actctx->wLangId : 0),
                (actctx->dwFlags & ACTCTX_FLAG_RESOURCE_NAME_VALID? actctx->lpResourceName : (LPCWSTR)-1));

            if (manifest || config)
            {
                if ( CreateSxSActCtx(+appname,+assmdir,manifest,+manifest_source,config,+config_source,result) )
                    return APIF_RETURN;
            }
            else
                XLOG | _S* _XOr("<APIF/CTX> failed to found manifest for %s/%d/%d",49,37945923)
                %actctx->lpSource %(unsigned)actctx->lpResourceName %actctx->wLangId;
            *result = 0;
            return APIF_RETURN;
        }
        return APIF_CONTINUE;
    }

    virtual APIF_STATUS DoCall(int apif_id, void* args, unsigned* result)
    {
        switch (apif_id)
        {
            case APIF_KRCREATECTX:
                return DoCreateActCtxW(args,result);
        }
        return APIF_CONTINUE;
    }
};

ActCtxFilter apif_actctxfilt = ActCtxFilter();

void APIF_RegisterActCtxHooks()
{
    APIF->Push(&apif_actctxfilt);
};

