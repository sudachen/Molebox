
/*

  Copyright (c) 2005-2009, Alexey Sudachen, alexey@sudachen.name

*/

#if !defined __43439A3D_30F9_448A_A06C_ADF5974F8B15__
#define __43439A3D_30F9_448A_A06C_ADF5974F8B15__

#include "_specific.h"
#include "string.h"

namespace teggo
{

    static int _H_Instance_Fake_module_detector_() { return 0; }

    template <unsigned tFake>
    struct H_Instance_Fake
    {

        //private:
        HMODULE hmodule;
        wchar_t modulename[MAX_PATH];
        wchar_t moduledir[MAX_PATH];
        wchar_t metamodulename[MAX_PATH];
        wchar_t metamoduledir[MAX_PATH];
        wchar_t mainmodulename[MAX_PATH];
        wchar_t mainmoduledir[MAX_PATH];
        wchar_t temporal[MAX_PATH];

    public:
        _TEGGO_EXPORTABLE HMODULE GetHmodule(void const* = &_H_Instance_Fake_module_detector_);
        _TEGGO_EXPORTABLE pwide_t GetModulePath(void const* = &_H_Instance_Fake_module_detector_);
        _TEGGO_EXPORTABLE pwide_t GetModuleDir(void const* = &_H_Instance_Fake_module_detector_);
        _TEGGO_EXPORTABLE pwide_t GetMainModulePath();
        _TEGGO_EXPORTABLE pwide_t GetMainModuleDir();
        _TEGGO_EXPORTABLE pwide_t MetaModuleDir();
        _TEGGO_EXPORTABLE pwide_t MetaModuleName();
        _TEGGO_EXPORTABLE pwide_t GetTemporalDirectory();
        _TEGGO_EXPORTABLE void    AnalyzeMetaModule_();
        _TEGGO_EXPORTABLE void    SetInstanceHandle(HMODULE);
        #ifndef _NO_9x_SUPPORT
        _TEGGO_EXPORTABLE bool IsWi9x();
        #else
        inline bool IsWi9x() { return false; }
        #endif
        //_TEGGO_EXPORTABLE H_Instance_Fake();
    };

    //template<> _TEGGO_EXPORTABLE HMODULE  H_Instance_Fake<0>::GetHmodule();
    //template<> _TEGGO_EXPORTABLE void  H_Instance_Fake<0>::AnalyzeMetaModule_();

    struct  H_Instance_Fake_Instance //: Uncopyable
    {
        virtual H_Instance_Fake<0>* operator->() const
        {
            static H_Instance_Fake<0> instance = {0};
            return &instance;
        }
    };

    #if defined _TEGGOINLINE || defined _TEGGO_INSTANCE_HERE

    #if !defined _TEGGO_INSTANCE_HERE
# define _TEGG_INSTANCE_FAKE_INLINE  CXX_FAKE_INLINE
    #else
# define _TEGG_INSTANCE_FAKE_INLINE  _TEGGO_EXPORTABLE
    #endif

    /*
      template<> _TEGG_INSTANCE_FAKE_INLINE
        H_Instance_Fake<0>::H_Instance_Fake()
          {
            memset(this,0,sizeof(*this));
          }
    */

    template<> _TEGG_INSTANCE_FAKE_INLINE
    void  H_Instance_Fake<0>::SetInstanceHandle(HMODULE h)
    {
        hmodule = h;
    }

    template<> _TEGG_INSTANCE_FAKE_INLINE
    bool  H_Instance_Fake<0>::IsWi9x()
    {
        #if 1
        static int platform_id = 0;
        if ( !platform_id )
        {
            OSVERSIONINFOA ova = {sizeof(OSVERSIONINFOA)};
            GetVersionExA(&ova);
            platform_id = ova.dwPlatformId;
        }
        return platform_id == VER_PLATFORM_WIN32_WINDOWS;
        #else
        return true;
        #endif
    }

    template<> _TEGG_INSTANCE_FAKE_INLINE
    HMODULE  H_Instance_Fake<0>::GetHmodule(void const* marker)
    {
        if ( !marker )
            return GetModuleHandleA(0);

        if ( !hmodule )
        {
            MEMORY_BASIC_INFORMATION meminfo;
            if ( ::VirtualQuery(
                     marker,
                     &meminfo,
                     sizeof(MEMORY_BASIC_INFORMATION)) )
            {
                if ( meminfo.Type == MEM_IMAGE )
                    hmodule = (HMODULE)meminfo.AllocationBase;
            }
            else
                EMERGENCY(L"module handle is unknown");
        }
        return hmodule;
    }

    template<> _TEGG_INSTANCE_FAKE_INLINE
    pwide_t  H_Instance_Fake<0>::GetModulePath(void const* marker)
    {
        if ( !modulename[0] )
        {
            unsigned count = 0;
            if ( IsWi9x() )
            {
                count = ::GetModuleFileNameA(GetHmodule(marker),(char*)modulename,MAX_PATH);
                InplaceCharToWide(modulename);
            }
            else
                count = ::GetModuleFileNameW(GetHmodule(marker),&modulename[0],MAX_PATH);
            if ( count)
                replace(&modulename[0],&modulename[0]+count,'\\','/');
            else
                EMERGENCY(L"module path is empty");
        }
        return modulename;
    }

    template<> _TEGG_INSTANCE_FAKE_INLINE
    pwide_t  H_Instance_Fake<0>::GetMainModulePath()
    {
        if ( !modulename[0] )
        {
            unsigned count = 0;
            if ( IsWi9x() )
            {
                count = ::GetModuleFileNameA(GetHmodule(0),(char*)modulename,MAX_PATH);
                InplaceCharToWide(modulename);
            }
            else
                count = ::GetModuleFileNameW(GetHmodule(0),&modulename[0],MAX_PATH);
            if ( count)
                replace(&modulename[0],&modulename[0]+count,'\\','/');
            else
                EMERGENCY(L"module path is empty");
        }
        return modulename;
    }

    template<> _TEGG_INSTANCE_FAKE_INLINE
    pwide_t  H_Instance_Fake<0>::GetModuleDir(void const* marker)
    {
        if ( !moduledir[0] )
        {
            GetModulePath(marker);
            memcpy(moduledir,modulename,sizeof(modulename));
            if ( wchar_t* p = wcsrchr(moduledir,'/') )
                *p = 0;
            else
                EMERGENCY(L"module directory is empty");
        }
        return moduledir;
    }

    template<> _TEGG_INSTANCE_FAKE_INLINE
    pwide_t  H_Instance_Fake<0>::GetMainModuleDir()
    {
        if ( !mainmoduledir[0] )
        {
            GetMainModulePath();
            memcpy(moduledir,modulename,sizeof(modulename));
            if ( wchar_t* p = wcsrchr(moduledir,'/') )
                *p = 0;
            else
                EMERGENCY(L"module directory is empty");
        }
        return moduledir;
    }

    template<> _TEGG_INSTANCE_FAKE_INLINE
    pwide_t  H_Instance_Fake<0>::MetaModuleName()
    {
        if ( !metamodulename[0] ) AnalyzeMetaModule_();
        return metamodulename;
    }

    template<> _TEGG_INSTANCE_FAKE_INLINE
    pwide_t  H_Instance_Fake<0>::GetTemporalDirectory()
    {
        if ( !temporal[0] )
        {
            unsigned count = 0;
            if ( IsWi9x() )
            {
                count = ::GetTempPathA(MAX_PATH,(char*)temporal);
                InplaceCharToWide(temporal);
            }
            else
                count = ::GetTempPathW(MAX_PATH,temporal);
            replace(temporal,temporal+count,'\\','/');
        }
        return temporal;
    }

    template<> _TEGG_INSTANCE_FAKE_INLINE
    pwide_t  H_Instance_Fake<0>::MetaModuleDir()
    {
        if ( !metamoduledir[0] ) AnalyzeMetaModule_();
        return metamoduledir;
    }

    template<> _TEGG_INSTANCE_FAKE_INLINE
    void  H_Instance_Fake<0>::AnalyzeMetaModule_()
    {
        enum {CMDLSZ=1024};
        wchar_t cmdl[CMDLSZ];
        unsigned cmdl_size = 0;
        if ( IsWi9x() )
        {
            pchar_t cmdlineA = GetCommandLineA();
            cmdl_size = teggo_min<unsigned>(strlen(cmdlineA),CMDLSZ-1);
            StrCopyN(cmdl,cmdlineA,cmdl_size);
        }
        else
        {
            wchar_t* cmdline = GetCommandLineW();
            cmdl_size = teggo_min<unsigned>(wcslen(cmdline),CMDLSZ-1);
            copy_n(cmdline,cmdl,cmdl_size);
        }

        wchar_t* const cmdlE = cmdl+cmdl_size;
        *cmdlE = 0;
        replace(cmdl,cmdlE,'\\','/');
        transform(cmdl,cmdlE,cmdl,towlower);

        wchar_t _pC = '"';

        wchar_t* p =
            ( *cmdl == '"' ) ? linear_find(cmdl+1,cmdlE,'"')
            : (( *cmdl == '\'' ) ? _pC = '\'', linear_find(cmdl+1,cmdlE,'\'')
               : linear_find(cmdl+1,cmdlE,' '));

        wchar_t* pp  = p;

        while ( pp != cmdl && *pp != '/' ) --pp;
        if ( *pp == '/' || *pp == _pC ) ++pp;

        if ( !wcsnicmp(pp,L"wscript.exe",p-pp)
             || !wcsnicmp(pp,L"cscript.exe",p-pp)
             || !wcsnicmp(pp,L"python.exe",p-pp)
             || !wcsnicmp(pp,L"pythonw.exe",p-pp)
             || !wcsnicmp(pp,L"python_d.exe",p-pp)
             || !wcsnicmp(pp,L"pythonw_d.exe",p-pp)
           )
        {
            wchar_t* pp0 = &*linear_find(p+1,cmdlE,' ',Not<Equal>());
            if ( pp0 != cmdlE )
            {
                wchar_t* pp1;
                if ( *pp0 == '"' )
                {
                    pp1 = linear_find(pp0+1,cmdlE,_pC);
                    pp0++;
                }
                else
                    pp1 = linear_find(pp0+1,cmdlE,' ');

                wchar_t* pp2 = pp1;
                while ( pp2 != pp0 && *pp2 != '/' ) --pp2;

                copy(pp2,pp1,metamodulename); // нуль там уже есть

                if ( pp2 == pp0 )
                {
                    // рабочий каталог текущая директория
                    if ( IsWi9x() )
                    {
                        GetCurrentDirectoryA(MAX_PATH,(char*)metamoduledir);
                        InplaceCharToWide(metamoduledir);
                    }
                    else
                        GetCurrentDirectoryW(MAX_PATH,metamoduledir);
                }
                else
                {
                    *pp2 = 0;
                    if ( IsWi9x() )
                    {
                        GetFullPathNameA(+StringA(pp0),MAX_PATH,(char*)metamoduledir,0);
                        InplaceCharToWide(metamoduledir);
                    }
                    else
                        GetFullPathNameW(pp0,MAX_PATH,metamoduledir,0);
                }
            }
            if ( ! *metamodulename )
                wcscpy(metamodulename,L"uscript");
        }
        else if ( !wcsnicmp(L"regsvr32.exe",pp,p-pp) )
        {
            metamodulename[0]=0;
            metamoduledir[0]=0;
        }
        else if (
            !wcsnicmp(L"vb6.exe",pp,p-pp)
            || !wcsnicmp(L"vb5.exe",pp,p-pp) )
        {
            wcsncpy(metamodulename,pp,p-pp);
            if ( IsWi9x() )
            {
                GetCurrentDirectoryA(MAX_PATH,(char*)metamoduledir);
                InplaceCharToWide(metamoduledir);
            }
            else
                GetCurrentDirectoryW(MAX_PATH,metamoduledir);
        }
        else
        {
            wchar_t* _pX = p;
            while (_pX != pp && *_pX != '/') --_pX;
            if ( *_pX == '/' ) ++_pX;
            wcsncpy(metamodulename,_pX,p-_pX);
            if ( _pX != pp )
            {
                *_pX = 0;
                if ( IsWi9x() )
                {
                    GetFullPathNameA(+StringA(pp),MAX_PATH,(char*)metamoduledir,0);
                    InplaceCharToWide(metamoduledir);
                }
                else
                    GetFullPathNameW(pp,MAX_PATH,metamoduledir,0);
            }
            else
            {
                if ( IsWi9x() )
                {
                    GetCurrentDirectoryA(MAX_PATH,(char*)metamoduledir);
                    InplaceCharToWide(metamoduledir);
                }
                else
                    GetCurrentDirectoryW(MAX_PATH,metamoduledir);
            }
        }
        size_t mpath_len = wcslen(metamoduledir);
        teggo::replace(metamoduledir,metamoduledir+mpath_len,'\\','/');
        if ( metamoduledir[mpath_len-1] == '/' )
            metamoduledir[mpath_len-1] = 0;
    }

    #endif //TEGGO_IMPLEMENTS_HERE

    typedef H_Instance_Fake_Instance const HinstanceType;
    static HinstanceType Hinstance = HinstanceType();

}

#endif // __43439A3D_30F9_448A_A06C_ADF5974F8B15__
