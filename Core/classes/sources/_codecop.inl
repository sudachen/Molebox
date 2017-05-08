
/*

  Copyright (c) 2005-2009, Alexey Sudachen, alexey@sudachen.name

*/


#if !defined ___41F3F0C3_8C9B_4A61_8846_83C5692309C0___
#define ___41F3F0C3_8C9B_4A61_8846_83C5692309C0___

#include "_specific.h"

#if defined _TEGGO_CODECOP_HERE
# define _TEGGO_CODECOP_FAKE_INLINE _TEGGO_EXPORTABLE
#else
# define _TEGGO_CODECOP_FAKE_INLINE CXX_FAKE_INLINE
#endif

//extern "C" void __stdcall DebugBreak(void);

#if defined _DEBUG
//extern "C" int __stdcall IsDebuggerPresent(void);
_TEGGO_CODECOP_FAKE_INLINE
void CXX_STDCALL _Teggo_Debug_Code_Here_()
{
    if ( IsDebuggerPresent() )
        DebugBreak();
}
#endif

namespace teggo
{

    _TEGGO_CODECOP_FAKE_INLINE
    void CXX_STDCALL CodePolice__::Message(pwide_t msg,pwide_t prefix, int lineno, pchar_t fname)
    {
        __asm int 3;
        #   if SYSTEM_IS_WINDOWS
        HANDLE errh = GetStdHandle(STD_ERROR_HANDLE);
        if ( errh && errh != INVALID_HANDLE_VALUE  )
        #   endif
            fprintf(stderr,"%ls: %ls, at '%s':%d \n",prefix,msg,fname,lineno);
        #   if SYSTEM_IS_WINDOWS
        else
        {
            int (__stdcall *MessageBoxW_fn)(HWND,pwide_t,pwide_t,int);
            HMODULE usr32dll = LoadLibraryA("user32.dll");
            *((FARPROC*)&MessageBoxW_fn)
                = GetProcAddress(usr32dll,"MessageBoxW");
            MessageBoxW_fn(0,msg,prefix,0);
            FreeLibrary(usr32dll);
        }
        #   endif
    }

    _TEGGO_CODECOP_FAKE_INLINE
    void CXX_STDCALL CodePolice__::Terminator(pwide_t msg, int lineno, pchar_t fname)
    {
        char mem[96] = {0};
        Message(msg,L"CodPolice::Termination",lineno,fname);
        _Teggo_Debug_Code_Here_();
        abort();
    }

    _TEGGO_CODECOP_FAKE_INLINE
    void CXX_STDCALL CodePolice__::Notifier(pwide_t msg, int lineno, pchar_t fname)
    {
        static pwide_t prefix = L"CodPolice::Notification";
        Message(msg,L"CodPolice::Notification",lineno,fname);
    }


    template <> _TEGGO_CODECOP_FAKE_INLINE
    void CodePolice_<0>::Analyse( ILLEGAL_SITUATION sit, pwide_t descr, int lineno, pchar_t fname )
    {
        if ( sit == SIT_VIOLATION || sit == SIT_ASSERTION || sit == SIT_EMERGENCY )
        {
            if ( terminate_ )
                (*terminate_)(descr,lineno,fname);
        }
        else if ( sit == SIT_WARNING )
        {
            if ( notify_ )
                (*notify_)(descr,lineno,fname);
        }
    }

    #if defined _TEGGO_CODECOP_HERE
    //template <>
    _TEGGO_EXPORTABLE_DATA
    CodePolice_<0> CodePolice_Office<0>::police_
    =
    {
        &CodePolice__::Notifier,
        &CodePolice__::Terminator
    };
    #endif

} // namespace teggo

#undef _TEGGO_CODECOP_FAKE_INLINE

#endif // ___41F3F0C3_8C9B_4A61_8846_83C5692309C0___
