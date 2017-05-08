
/*

  Copyright (c) 2005-2009, Alexey Sudachen, alexey@sudachen.name

*/

#if !defined __0103EC7B_15B0_47B9_A506_4B4951D96D61__
#define __0103EC7B_15B0_47B9_A506_4B4951D96D61__

#include "_specific.h"
#include "string.h"
#include "array.h"
#include "format.h"
#include "hinstance.h"
#include "print.h"
#include <fcntl.h>
#include <io.h>
#include <time.h>

#if defined _WIN32 && CXX_COMPILER_IS_MSVC_COMPATIBLE
# pragma comment(lib,"version.lib")
# pragma comment(lib,"winmm.lib")
# pragma comment(lib,"advapi32.lib")
#endif

namespace teggo
{

    _TEGGO_EXPORTABLE StringT<char> CXX_STDCALL GetModuleVersion( pwide_t modname );
    _TEGGO_EXPORTABLE bool CXX_STDCALL GetModuleVersion( pwide_t modname, int* major, int* minor, int* build );
    _TEGGO_EXPORTABLE StringT<char> CXX_STDCALL SysFormatError( u32_t lerror = GetLastError());
    _TEGGO_EXPORTABLE StringT<char> CXX_STDCALL SysFormatErrorOem( u32_t lerror = GetLastError());
    enum MMAP_TYPE { MMAP_RCOW, MMAP_RO, MMAP_RW };
    _TEGGO_EXPORTABLE void* CXX_STDCALL SysMmapFile(StringParam fname, u32_t size = 0, u32_t offset = 0, MMAP_TYPE tp=MMAP_RW,
            EhFilter ehf = PRINTEF);
    _TEGGO_EXPORTABLE void* CXX_STDCALL SysMmapFile(StringParam fname, u32_t size = 0, u32_t offset = 0, bool writable = true,
            bool create_new = false, EhFilter ehf = PRINTEF);
    _TEGGO_EXPORTABLE void* CXX_STDCALL SysMmapFileByHandle(HANDLE handle, u32_t size = 0, u32_t offset = 0, bool writable = true,
            EhFilter ehf = PRINTEF);
    _TEGGO_EXPORTABLE void  CXX_STDCALL SysUnmmapFile(void const*);
    _TEGGO_EXPORTABLE StringT<char> CXX_STDCALL OemRecode(pchar_t);
    _TEGGO_EXPORTABLE bool CXX_STDCALL SysPipe(pwide_t command, FILE** in, FILE** out, int* proc, bool textmode, bool useCmd,
            EhFilter ehf = PRINTEF);
    inline bool SysPipe(pwide_t command, FILE** in, FILE** out, int* proc, bool textmode=true, EhFilter ehf = PRINTEF)
    { return SysPipe(command,in,out,proc,textmode,true,ehf); }
    _TEGGO_EXPORTABLE bool CXX_STDCALL SysPipeClose(int* proc, FILE** in, FILE** out, EhFilter ehf = PRINTEF);
    _TEGGO_EXPORTABLE bool CXX_STDCALL SysPipeKill(int* proc, FILE** in, FILE** out, EhFilter ehf = PRINTEF);
    _TEGGO_EXPORTABLE bool CXX_STDCALL SysPipeExitCode(int proc);

    CXX_FAKE_INLINE void* CXX_STDCALL SysMmapWritableFile(StringParam fname,EhFilter ehf = PRINTEF)
    {return SysMmapFile(fname,0,0,MMAP_RW,ehf);}
    CXX_FAKE_INLINE void* CXX_STDCALL SysMmapReadonlyFile(StringParam fname,EhFilter ehf = PRINTEF)
    {return SysMmapFile(fname,0,0,MMAP_RO,ehf);}
    CXX_FAKE_INLINE void* CXX_STDCALL SysMmapFileRnCoW(StringParam fname,EhFilter ehf = PRINTEF)
    {return SysMmapFile(fname,0,0,MMAP_RCOW,ehf);}
    CXX_FAKE_INLINE void* CXX_STDCALL SysCreateIfNeedAndMmapFile(StringParam fname,u32_t size,EhFilter ehf = PRINTEF)
    {return SysMmapFile(fname,size,0,true,true,ehf);}

    inline StringT<char> CXX_STDCALL GetModuleVersion( pchar_t modname )
    {
        return GetModuleVersion(+StringT<wchar_t>(modname));
    }

    inline bool CXX_STDCALL GetModuleVersion( pchar_t modname, int* major, int* minor, int* build )
    {
        return GetModuleVersion(+StringT<wchar_t>(modname),major,minor,build);
    }

    _TEGGO_EXPORTABLE void  CXX_STDCALL ProcessYield();
    _TEGGO_EXPORTABLE u32_t CXX_STDCALL GetMillis(void);
    _TEGGO_EXPORTABLE void  CXX_STDCALL ResetMillis(u32_t millis = 0);
    _TEGGO_EXPORTABLE void  CXX_STDCALL SysUrandom(void* p,int len);
    _TEGGO_EXPORTABLE bool  CXX_STDCALL SysExec(StringParam command, bool useCmd, EhFilter ehf = PRINTEF);

    inline  bool  CXX_STDCALL SysExec(StringParam command,EhFilter ehf = PRINTEF)
    {
        return SysExec(command,false,ehf);
    }

    _TEGGO_EXPORTABLE StringW CXX_STDCALL SysGetCurrentDirectory();
    _TEGGO_EXPORTABLE StringW CXX_STDCALL SysSetCurrentDirectory(StringParam dir);
    _TEGGO_EXPORTABLE void CXX_STDCALL SysBuildArgv(char const* input, int& argc, char**& argv);
    _TEGGO_EXPORTABLE void CXX_STDCALL SysBuildArgv(wchar_t const* input, int& argc, wchar_t**& argv);
    _TEGGO_EXPORTABLE StringW CXX_STDCALL SysGetLocalUserAppFolder();
    _TEGGO_EXPORTABLE StringW CXX_STDCALL SysGetUserAppFolder();

    #if defined _TEGGOINLINE || defined _TEGGO_SYSUTIL_HERE

    #if !defined _TEGGO_SYSUTIL_HERE
# define _TEGGO_SYSUTIL_FAKE_INLINE  CXX_FAKE_INLINE
    #else
# define _TEGGO_SYSUTIL_FAKE_INLINE
    #endif

    extern "C" HRESULT CXX_STDCALL SHGetFolderPathW(HWND hwndOwner,int nFolder,HANDLE hToken,DWORD dwFlags,LPWSTR pszPath);
    extern "C" HRESULT CXX_STDCALL SHGetFolderPathW(HWND hwndOwner,int nFolder,HANDLE hToken,DWORD dwFlags,LPWSTR pszPath);

    _TEGGO_SYSUTIL_FAKE_INLINE
    StringT<char> CXX_STDCALL GetModuleVersion( pwide_t modname )
    {
        unsigned long foo = 0;
        unsigned size = GetFileVersionInfoSizeW((wchar_t*)modname,&foo);
        if ( size != 0 )
        {
            BufferT<byte_t> buffer(size);
            if ( GetFileVersionInfoW((wchar_t*)modname,0,size,&buffer[0]) )
            {
                VS_FIXEDFILEINFO* vs = 0;
                unsigned len = 0;
                if ( VerQueryValueW(&buffer[0],L"\\",(void**)&vs,&len)
                     && (0xfeef04bd == vs->dwSignature) )
                {
                    return StringT<char>(
                               _S*"%d.%d.%d.%d"
                               %(vs->dwFileVersionMS >> 16)
                               %(vs->dwFileVersionMS & 0xffff)
                               %(vs->dwFileVersionLS >> 16)
                               %(vs->dwFileVersionLS & 0xffff));
                }
            }
        }
        return StringT<char>("x.x.x.x");
    }

    _TEGGO_SYSUTIL_FAKE_INLINE
    bool CXX_STDCALL GetModuleVersion( pwide_t modname, int* major, int* minor, int* build )
    {
        StringT<char> v = GetModuleVersion( modname );
        if ( v )
            return sscanf(+v,"%d.%d.%*d.%d",major,minor,build) == 3;
        return false;
    }

    _TEGGO_SYSUTIL_FAKE_INLINE
    StringT<char> CXX_STDCALL SysFormatError( u32_t lerror )
    {
        BufferT<char> buff(1024,0);
        u32_t len =
            FormatMessageA(
                FORMAT_MESSAGE_FROM_SYSTEM |
                FORMAT_MESSAGE_IGNORE_INSERTS,
                NULL,
                lerror,
                MAKELANGID(LANG_NEUTRAL, SUBLANG_DEFAULT), // Default language
                &buff[0],
                buff.Count()-1,
                NULL
            );
        char* pE = buff.Begin()+len;
        pE = teggo::remove(buff.Begin(),pE,'\n');
        pE = teggo::remove(buff.Begin(),pE,'\r');
        *pE = 0;
        return StringT<char>(+buff);
    }

    _TEGGO_SYSUTIL_FAKE_INLINE
    StringT<char> CXX_STDCALL SysFormatErrorOem( u32_t lerror )
    {
        StringT<char> se = SysFormatError( lerror );
        BufferT<char> oem;
        oem.Resize(se.Length()+1);
        CharToOemA(+se,+oem);
        return StringT<char>(+oem);
    }

    _TEGGO_SYSUTIL_FAKE_INLINE
    void CXX_STDCALL ProcessYield()
    {
        typedef BOOL (__stdcall *SwitchToThread_t)(void);
        static SwitchToThread_t SwitchToThread_f  =
            (SwitchToThread_t)GetProcAddress(GetModuleHandleW(L"kernel32.dll"),"SwitchToThread");
        if ( SwitchToThread_f )
            SwitchToThread_f();
        else
            Sleep(1);
    }

    template <unsigned tFake>
    struct Millis_Static_
    {
        static u32_t millis;
        static u32_t millis_pf;
        static u32_t millis_pf_interval;
    };
    template <unsigned tFake> u32_t Millis_Static_<tFake>::millis = 0;
    template <unsigned tFake> u32_t Millis_Static_<tFake>::millis_pf = 0;
    template <unsigned tFake> u32_t Millis_Static_<tFake>::millis_pf_interval = 0;

    _TEGGO_SYSUTIL_FAKE_INLINE
    u32_t CXX_STDCALL __get_millis_pf()
    {
        static __int64 perffreq = 0;
        static BOOL _fake = QueryPerformanceFrequency((LARGE_INTEGER*)&perffreq);
        if ( perffreq > 1000 )
        {
            __int64 val;
            QueryPerformanceCounter((LARGE_INTEGER*)&val);
            return u32_t(val / (perffreq/1000));
        }
        else
            return timeGetTime();
    }

    _TEGGO_SYSUTIL_FAKE_INLINE
    u32_t CXX_STDCALL GetMillis(void)
    {
        typedef Millis_Static_<0> g;
        static u32_t last_ms = timeGetTime();
        u32_t ms  = timeGetTime();
        u32_t mpf = __get_millis_pf();
        if ( ms - last_ms > (mpf-g::millis_pf+20) )
        {
            g::millis_pf = mpf;
            g::millis_pf_interval = ms - g::millis;
        }
        last_ms = ms;
        return mpf-g::millis_pf + g::millis_pf_interval;
    }

    _TEGGO_SYSUTIL_FAKE_INLINE
    void CXX_STDCALL ResetMillis(u32_t millis)
    {
        typedef Millis_Static_<0> g;
        g::millis_pf = __get_millis_pf();
        g::millis    = timeGetTime();
        if ( !millis )
            g::millis_pf_interval = 0;
        else
            g::millis_pf_interval = g::millis - millis;
    }

    _TEGGO_SYSUTIL_FAKE_INLINE
    void* CXX_STDCALL SysMmapFileByHandle2(HANDLE handle, u32_t size, u32_t offset, bool writable, bool cow, EhFilter ehf)
    {
        byte_t* result = 0;
        HANDLE file_mapping = 0;
        if ( handle != INVALID_HANDLE_VALUE )
            file_mapping = CreateFileMappingA(
                               handle?handle:INVALID_HANDLE_VALUE,0,
                               (writable?PAGE_READWRITE:(cow?PAGE_WRITECOPY:PAGE_READONLY)),
                               0,offset+size,0);

        if ( file_mapping )
        {
            if ( !(result = (byte_t*)MapViewOfFile(
                                file_mapping,
                                (writable?FILE_MAP_WRITE:(cow?FILE_MAP_COPY:FILE_MAP_READ)),
                                0,offset,size )))
                ehf(_S*"could not map file, %s"%+teggo::SysFormatError());
        }
        else
            ehf(_S*"could not create file mapping, %s"%+teggo::SysFormatError())
            ;

        if ( file_mapping ) CloseHandle( file_mapping );
        return result;
    }

    _TEGGO_SYSUTIL_FAKE_INLINE
    void* CXX_STDCALL SysMmapFileByHandle(HANDLE handle, u32_t size, u32_t offset, bool writable, EhFilter ehf)
    {
        void* p = SysMmapFileByHandle2(handle, size, offset, writable, false, ehf);
        return p;
    }

    _TEGGO_SYSUTIL_FAKE_INLINE
    void* CXX_STDCALL SysMmapFile2(StringParam fname, u32_t size, u32_t offset, bool writable, bool create_new, bool cow,
                                   EhFilter ehf)
    {
        HANDLE file_handle = INVALID_HANDLE_VALUE;
        if ( Hinstance->IsWi9x() )
            file_handle = CreateFileA(
                              +StringA(+fname),
                              GENERIC_READ|(writable?GENERIC_WRITE:0),
                              FILE_SHARE_READ/*|(writable?FILE_SHARE_WRITE:0)*/,
                              0,
                              (create_new?CREATE_ALWAYS:OPEN_EXISTING),
                              (writable?FILE_FLAG_WRITE_THROUGH:0),
                              0);
        else
            file_handle = CreateFileW(
                              +fname,
                              GENERIC_READ|(writable?GENERIC_WRITE:0),
                              FILE_SHARE_READ/*|(writable?FILE_SHARE_WRITE:0)*/,
                              0,
                              (create_new?CREATE_ALWAYS:OPEN_EXISTING),
                              (writable?FILE_FLAG_WRITE_THROUGH:0),
                              0);

        if ( file_handle != INVALID_HANDLE_VALUE )
        {
            SetHandleInformation(file_handle,HANDLE_FLAG_INHERIT,0);
            void* p = SysMmapFileByHandle2(file_handle, size, offset, writable, cow, ehf);
            if ( !offset && cow )
            {
                DWORD foo = 0;
                if ( !size )
                    size = GetFileSize(file_handle,&foo);
                VirtualProtect(p,size,PAGE_READWRITE,&foo);
            }
            CloseHandle(file_handle);
            return p;
        }
        else
            ehf(+SysFormatErrorOem());
        return 0;

    }

    _TEGGO_SYSUTIL_FAKE_INLINE
    void* CXX_STDCALL SysMmapFile(StringParam fname, u32_t size, u32_t offset, bool writable, bool create_new, EhFilter ehf)
    {
        return SysMmapFile2(fname, size, offset, writable, create_new, false, ehf);
    }

    _TEGGO_SYSUTIL_FAKE_INLINE
    void* CXX_STDCALL SysMmapFile(StringParam fname, u32_t size, u32_t offset, MMAP_TYPE tp, EhFilter ehf)
    {
        if ( tp == MMAP_RO )
            return SysMmapFile2(fname, size, offset, false, false, false, ehf);
        if ( tp == MMAP_RW )
            return SysMmapFile2(fname, size, offset, true, false, false, ehf);
        if ( tp == MMAP_RCOW )
            return SysMmapFile2(fname, size, offset, false, false, true, ehf);
        return 0;
    }

    _TEGGO_SYSUTIL_FAKE_INLINE
    void CXX_STDCALL SysUnmmapFile(void const* p)
    {
        FlushViewOfFile(p,0);
        UnmapViewOfFile((void*)p);
    }

    _TEGGO_SYSUTIL_FAKE_INLINE
    StringT<char> CXX_STDCALL OemRecode(pchar_t s)
    {
        BufferT<char> oem;
        oem.Resize(strlen(s)+1);
        CharToOemA(+s,+oem);
        return StringT<char>(+oem);
    }

    //_TEGGO_SYSUTIL_FAKE_INLINE
    template <class tT> StringT<tT> _GetExecutable(tT const* S)
    {
        StringT<tT> e = S;
        e.LeftTrim();
        if ( e && e[0] == '"' )
        {
            int i = e.FindChar('"',1);
            if ( i > 0 )
            { e.Resize(i); e.Remove(0,1); }
        }
        else
        {
            int i = e.FindChar(L' ');
            if ( i > 0 )
            { e.Resize(i); }
        }
        return e;
    }

    _TEGGO_SYSUTIL_FAKE_INLINE
    bool CXX_STDCALL SysPipe(pwide_t command, FILE** in, FILE** out, int* proc, bool textmode, bool useCmd, EhFilter ehf)
    {
        SECURITY_ATTRIBUTES saa;
        saa.nLength = sizeof(SECURITY_ATTRIBUTES);
        saa.bInheritHandle = TRUE;
        saa.lpSecurityDescriptor = NULL;
        int foo;
        bool retval = false;
        HANDLE inrd=0, inwr=0, _inwr = 0, outrd=0, outwr=0, _outrd = 0;
        HANDLE curproc = GetCurrentProcess();
        PROCESS_INFORMATION piProcInfo = {0};
        STARTUPINFOW siStartInfo = {0};

        if (!CreatePipe(&inrd, &_inwr, &saa, 0))
        { ehf("failed to create input pipe"); goto e; }
        if (!CreatePipe(&_outrd, &outwr, &saa, 0))
        { ehf("failed to create output pipe"); goto e; }
        DuplicateHandle(curproc, _inwr, curproc, &inwr, 0, FALSE, DUPLICATE_SAME_ACCESS);
        CloseHandle(_inwr); _inwr = 0;
        DuplicateHandle(curproc, _outrd, curproc, &outrd, 0, FALSE, DUPLICATE_SAME_ACCESS);
        CloseHandle(_outrd); _outrd = 0;

        siStartInfo.cb = sizeof(STARTUPINFOW);
        siStartInfo.dwFlags    = STARTF_USESTDHANDLES;
        siStartInfo.hStdInput  = inrd;
        siStartInfo.hStdOutput = outwr;
        siStartInfo.hStdError  = outwr;
        siStartInfo.wShowWindow = SW_HIDE;

        if ( useCmd )
        {
            BufferT<wchar_t> comspec(256,L'\0');
            if ( !GetEnvironmentVariableW(L"COMSPEC", +comspec, 256) )
            { ehf("failed to fetch COMSPEC value"); goto e; }

            if (!CreateProcessW(NULL,
                                (wchar_t*)(WCS|_S*L"\"%s\" /c %s"%+comspec%command),
                                NULL,
                                NULL,
                                TRUE,
                                CREATE_NO_WINDOW,
                                NULL,
                                NULL,
                                &siStartInfo,
                                &piProcInfo) )
            { ehf(_S*"failed to crate child process, %s"%SysFormatError()); goto e; }
        }
        else
        {
            StringW executable = _GetExecutable(command);
            if (!CreateProcessW(
                    +executable,
                    (wchar_t*)command,
                    NULL,
                    NULL,
                    TRUE,
                    CREATE_NO_WINDOW,
                    NULL,
                    NULL,
                    &siStartInfo,
                    &piProcInfo) )
            { ehf(_S*"failed to crate child process, %s"%SysFormatError()); goto e; }
        }

        CloseHandle(piProcInfo.hThread);
        *proc = (int)piProcInfo.hProcess;

        foo   = _open_osfhandle((int)inwr, _O_WRONLY | (textmode?_O_TEXT:_O_BINARY));
        *in   = _fdopen(foo, (textmode?"w":"wb"));
        foo   = _open_osfhandle((int)outrd, _O_RDONLY | (textmode?_O_TEXT:_O_BINARY));
        *out  = _fdopen(foo, (textmode?"r":"rb"));

        retval = true;
    e:

        if ( !retval )
        {
            if ( inwr ) CloseHandle(inwr);
            if ( outrd ) CloseHandle(outrd);
        }

        if ( inrd ) CloseHandle(inrd);
        if ( _inwr ) CloseHandle(_inwr);
        if ( outwr ) CloseHandle(outwr);
        if ( _outrd ) CloseHandle(_outrd);

        return retval;
    }

    _TEGGO_SYSUTIL_FAKE_INLINE
    bool CXX_STDCALL SysPipeClose(int* proc, FILE** in, FILE** out, EhFilter ehf)
    {
        while ( *proc )
        {
            unsigned long ecode;
            if ( !GetExitCodeProcess((HANDLE)*proc,&ecode) ) break;
            if ( ecode == STILL_ACTIVE )
                WaitForSingleObject((HANDLE)*proc,100);
            else
                *proc = 0;
        }
        if ( in && *in ) { fclose(*in); *in = 0; }
        if ( out && *out ) { fclose(*out); *out = 0; }
        //if ( proc && *proc ) { CloseHandle((HANDLE)*proc); *proc = 0; }
        return true;
    }

    _TEGGO_SYSUTIL_FAKE_INLINE
    bool CXX_STDCALL SysPipeKill(int* proc, FILE** in, FILE** out, EhFilter ehf)
    {
        if ( proc && *proc )
        {
            TerminateProcess((HANDLE)*proc,-1); *proc = 0;
            //CloseHandle((HANDLE)*proc); *proc = 0;
        }
        if ( in && *in ) { fclose(*in); *in = 0; }
        if ( out && *out ) { fclose(*out); *out = 0; }
        return true;
    }

    _TEGGO_SYSUTIL_FAKE_INLINE
    bool CXX_STDCALL SysPipeExitCode(int proc)
    {
        unsigned long ecode;
        if ( proc )
            if ( GetExitCodeProcess((HANDLE)proc,&ecode) )
                if ( ecode != STILL_ACTIVE )
                    return ecode;
        return 0;
    }

    _TEGGO_SYSUTIL_FAKE_INLINE
    void CXX_STDCALL SysUrandom(void* p,int len)
    {
        typedef BOOL (WINAPI *tCryptAcquireContext)(HCRYPTPROV*,LPCTSTR,LPCTSTR,DWORD,DWORD);
        typedef BOOL (WINAPI *tCryptGenRandom)(HCRYPTPROV,DWORD,BYTE*);
        static tCryptAcquireContext fCryptAcquireContext = 0;
        static tCryptGenRandom fCryptGenRandom = 0;
        static HCRYPTPROV cp = 0;
        if ( !fCryptAcquireContext )
        {
            HMODULE hm = LoadLibraryA("advapi32.dll");
            fCryptAcquireContext = (tCryptAcquireContext)GetProcAddress(hm,"CryptAcquireContextA");
            fCryptGenRandom = (tCryptGenRandom)GetProcAddress(hm,"CryptGenRandom");
        }
        if ( !cp && !fCryptAcquireContext(&cp, 0, 0,PROV_RSA_FULL, CRYPT_VERIFYCONTEXT) )
            goto simulate;
        if ( !fCryptGenRandom(cp,len,(unsigned char*)p) )
            goto simulate;
        if ( len >= 4 && *(unsigned*)p == 0 )
            goto simulate;
        return;
    simulate:
        //__asm int 3;
        for ( int i = 0; i < len; i+=4 )
        {
            u32_t sid = (u32_t)time(0);
            u32_t foo = unsigned_random(&sid);
            memcpy((char*)p+i,&foo,cxx_min<unsigned>(len-i,4));
        }
    };

    _TEGGO_SYSUTIL_FAKE_INLINE
    StringW CXX_STDCALL SysGetCurrentDirectory()
    {
        BufferT<wchar_t> buffer(256);
        if ( int l = ::GetCurrentDirectoryW(buffer.Count()-1,+buffer) )
            return StringW(+buffer,l);
        return StringW();
    }

    _TEGGO_SYSUTIL_FAKE_INLINE
    StringW CXX_STDCALL SysSetCurrentDirectory(StringParam dir)
    {
        StringW old = SysGetCurrentDirectory();
        ::SetCurrentDirectoryW(+dir);
        return old;
    }

    _TEGGO_SYSUTIL_FAKE_INLINE
    bool CXX_STDCALL SysExec(StringParam command, bool useCmd, EhFilter ehf)
    {
        SECURITY_ATTRIBUTES saa;
        saa.nLength = sizeof(SECURITY_ATTRIBUTES);
        saa.bInheritHandle = TRUE;
        saa.lpSecurityDescriptor = NULL;
        int foo;
        bool retval = false;
        HANDLE curproc = GetCurrentProcess();
        PROCESS_INFORMATION piProcInfo = {0};
        STARTUPINFOW siStartInfo = {0};
        siStartInfo.wShowWindow = SW_SHOWDEFAULT;

        if ( useCmd )
        {
            siStartInfo.wShowWindow = SW_HIDE;
            BufferT<wchar_t> comspec(256,L'\0');
            if ( !GetEnvironmentVariableW(L"COMSPEC", +comspec, 256) )
            { ehf("failed to fetch COMSPEC value"); goto e; }

            if (!CreateProcessW(NULL,
                                (wchar_t*)(WCS|_S*L"\"%s\" /c %s"%+comspec%+command),
                                NULL,
                                NULL,
                                TRUE,
                                0,
                                NULL,
                                NULL,
                                &siStartInfo,
                                &piProcInfo) )
            { ehf("failed to crate child process"); goto e; }
        }
        else
        {
            StringW executable = _GetExecutable(+command);
            if (!CreateProcessW(
                    +executable,
                    (wchar_t*)+command,
                    NULL,
                    NULL,
                    TRUE,
                    0,
                    NULL,
                    NULL,
                    &siStartInfo,
                    &piProcInfo) )
            { ehf("failed to crate child process"); goto e; }
        }

        CloseHandle(piProcInfo.hThread);
        CloseHandle(piProcInfo.hProcess);
        retval = true;
    e:
        return retval;
    }

    //_TEGGO_SYSUTIL_FAKE_INLINE
    template <class tChr> void _freeargv(tChr** vector)
    {
        tChr** scan;
        if (vector != NULL)
        {
            for (scan = vector; *scan != NULL; scan++) free (*scan);
            free (vector);
        }
    }

    _TEGGO_SYSUTIL_FAKE_INLINE char* _Tstrdup(char const* dat) { return strdup(dat); }
    _TEGGO_SYSUTIL_FAKE_INLINE wchar_t* _Tstrdup(wchar_t const* dat) { return wcsdup(dat); }

    //_TEGGO_SYSUTIL_FAKE_INLINE
    template <class tChr> void _SysBuildArgv(tChr const* input, int& argc, tChr**& argv)
    {
#define ISBLANK(a) ((a) == ' ' || (a) == '\t')
        enum {EOS=0,INITIAL_MAXARGC=128};
        tChr* arg;
        tChr* copybuf;
        int squote = 0;
        int dquote = 0;
        int bsquote = 0;
        int maxargc = 0;
        tChr** nargv;

        argv = 0;
        argc = 0;

        if (input != NULL)
        {
            copybuf = (tChr*) _alloca ( (StrLen(input) + 1)*sizeof(tChr) );
            do
            {
                while ( ISBLANK(*input) )
                {
                    input++;
                }
                if ((maxargc == 0) || (argc >= (maxargc - 1)))
                {
                    if (argv == NULL)
                    {
                        maxargc = INITIAL_MAXARGC;
                        nargv = (tChr**) malloc (maxargc * sizeof (tChr*));
                    }
                    else
                    {
                        maxargc *= 2;
                        nargv = (tChr**) realloc (argv, maxargc * sizeof (tChr*));
                    }
                    if (nargv == NULL)
                    {
                        if (argv != NULL)
                        {
                            _freeargv (argv);
                            argv = NULL;
                        }
                        break;
                    }
                    argv = nargv;
                    argv[argc] = NULL;
                }
                arg = copybuf;
                while (*input != EOS)
                {
                    if (ISBLANK (*input) && !squote && !dquote && !bsquote)
                    {
                        break;
                    }
                    else
                    {
                        if (bsquote)
                        {
                            bsquote = 0;
                            *arg++ = *input;
                        }
                        else if (squote)
                        {
                            if (*input == '\'') squote = 0;
                            else *arg++ = *input;
                        }
                        else if (dquote)
                        {
                            if (*input == '"') dquote = 0;
                            else *arg++ = *input;
                        }
                        else
                        {
                            if (*input == '\'') squote = 1;
                            else if (*input == '"') dquote = 1;
                            else *arg++ = *input;
                        }
                        input++;
                    }
                }
                *arg = EOS;
                argv[argc] = _Tstrdup (copybuf);
                if (argv[argc] == NULL)
                {
                    _freeargv (argv);
                    argv = NULL;
                    break;
                }
                argc++;
                argv[argc] = NULL;

                while (ISBLANK (*input)) ++input;
            }
            while (*input != EOS);
        }
#undef ISBLANK
    }

    _TEGGO_SYSUTIL_FAKE_INLINE
    void CXX_STDCALL SysBuildArgv(char const* input, int& argc, char**& argv) {_SysBuildArgv(input,argc,argv);}

    _TEGGO_SYSUTIL_FAKE_INLINE
    void CXX_STDCALL SysBuildArgv(wchar_t const* input, int& argc, wchar_t**& argv) {_SysBuildArgv(input,argc,argv);}

    #if defined _WIN32  && !defined _TEGGO_WITHOUT_SHL
    _TEGGO_SYSUTIL_FAKE_INLINE
    StringW CXX_STDCALL SysGetLocalUserAppFolder()
    {
        enum { APPDATA = 0x001a, LOCAL_APPDATA=0x001c, FLAG_CREATE=0x8000, TYPE_CURRENT = 0 };
        wchar_t b[MAX_PATH] = {0};
        if ( SUCCEEDED(SHGetFolderPathW(0,LOCAL_APPDATA|FLAG_CREATE,0,TYPE_CURRENT,b)) )
            return StringW(b);
        else
            return Hinstance->GetModuleDir();
    }

    _TEGGO_SYSUTIL_FAKE_INLINE
    StringW CXX_STDCALL SysGetUserAppFolder()
    {
        enum { APPDATA = 0x001a, LOCAL_APPDATA=0x001c, FLAG_CREATE=0x8000, TYPE_CURRENT = 0 };
        wchar_t b[MAX_PATH] = {0};
        if ( SUCCEEDED(SHGetFolderPathW(0,APPDATA|FLAG_CREATE,0,TYPE_CURRENT,b)) )
            return StringW(b);
        else
            return Hinstance->GetModuleDir();
    }
    #endif /* _WIN32 */

    #endif

} // namespace

#endif // __0103EC7B_15B0_47B9_A506_4B4951D96D61__
