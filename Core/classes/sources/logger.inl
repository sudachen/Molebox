
/*

  Copyright (c) 2005-2009, Alexey Sudachen, alexey@sudachen.name

*/

#if !defined ___e4d630eb_a08f_4ab7_945c_275375d02f63___
#define ___e4d630eb_a08f_4ab7_945c_275375d02f63___

#include "logger.h"
#include "hinstance.h"

#if defined _TEGGOINLINE || defined _TEGGO_LOGGER_HERE
#include <stdio.h>
#include <time.h>
#endif

#if _DEBUG
# define _TEGGO_GEBUGCHANNEL_VALUE true
#else
# define _TEGGO_GEBUGCHANNEL_VALUE true
#endif

namespace teggo
{

    #if defined _TEGGOINLINE
    template <class tFake> int logcontrol_<tFake>::dbglevel = Ldbg;
    template <class tFake> bool logcontrol_<tFake>::reopen = false;
    template <class tFake> LOG_HANDLE logcontrol_<tFake>::handle = 0;
# define _TEGGO_LOGGER_FAKE_INLINE CXX_FAKE_INLINE
    #elif defined _TEGGO_LOGGER_HERE
    _TEGGO_EXPORTABLE int logcontrol::dbglevel = Ldbg;
    _TEGGO_EXPORTABLE bool logcontrol::reopen = false;
    _TEGGO_EXPORTABLE LOG_HANDLE logcontrol::handle = 0;
# define _TEGGO_LOGGER_FAKE_INLINE _TEGGO_EXPORTABLE
    #endif

    _TEGGO_LOGGER_FAKE_INLINE
    int CXX_STDCALL OpenLog(pwide_t logname,pwide_t suffix)
    {
        if ( logcontrol::handle ) return 0;
        StringT<wchar_t> my_log_name;

        if ( !logname )
        {
            my_log_name << _S*"%?/%?" %Hinstance->MetaModuleDir() %Hinstance->MetaModuleName();
        }
        else
            my_log_name = logname;

        if ( suffix )
            my_log_name += suffix;
        else if (!logname)
            my_log_name += "-log.txt";

        //logcontrol::handle = fopen( +StringA(my_log_name), +StringA(logcontrol::reopen?L"ab":L"w+b"));
        //logcontrol::handle = _wfopen( +my_log_name, logcontrol::reopen?L"ab":L"w+b");
        logcontrol::handle = ::CreateFileA(
                                 +StringA(my_log_name),
                                 GENERIC_WRITE, FILE_SHARE_READ, 0,
                                 logcontrol::reopen?OPEN_ALWAYS:CREATE_ALWAYS, 0, 0);

        if ( logcontrol::handle == INVALID_HANDLE_VALUE )
        {
            int i = my_log_name.FindRChar('/');
            if ( i > 0 ) my_log_name.Remove(0,i);
            my_log_name.Insert(0,Hinstance->GetTemporalDirectory());
            logcontrol::handle = ::CreateFileA(
                                     +StringA(my_log_name),
                                     GENERIC_WRITE, FILE_SHARE_READ, 0,
                                     logcontrol::reopen?OPEN_ALWAYS:CREATE_ALWAYS, 0, 0);
        }

        if ( !logcontrol::handle || logcontrol::handle != INVALID_HANDLE_VALUE ) return 0;
        if ( !logcontrol::reopen )
        {
            time_t t = time(0);
            StringT<char> ob = _S*"[%s]\r\n" %StringT<char,64>(ctime(&t)).Chomp();
            unsigned long w;
            ::WriteFile(logcontrol::handle,+ob,ob.Length(),&w,0);
            //fprintf(logcontrol::handle,"[%s]\r\n",+timeS.Chomp());
            //fflush(logcontrol::handle);
        }
        logcontrol::reopen = true;
        return 1;
    }

    _TEGGO_LOGGER_FAKE_INLINE
    void CXX_STDCALL CloseLog()
    {
        if ( logcontrol::handle )
        {
            //fclose( logcontrol::handle );
            ::CloseHandle(logcontrol::handle);
            logcontrol::handle = 0;
        }
    }

    _TEGGO_LOGGER_FAKE_INLINE
    bool CXX_STDCALL LogIsOpen()
    {
        return logcontrol::handle != 0;
    }

    _TEGGO_LOGGER_FAKE_INLINE
    void CXX_STDCALL Logout_( pchar_t text, int n, int lo )
    {
        if ( lo > logcontrol::dbglevel ) return;
        void* o = logcontrol::handle;
        #ifndef _TEGGO_DISABLE_DBGOUTPUT
        OutputDebugStringA(text);
        #endif
        if ( o || ( lo == Lerr && OpenLog() ) )
        {
            if ( n < 0 ) n = strlen(text);
            //fwrite(text,1,n,logcontrol::handle);
            //fflush(logcontrol::handle);
            unsigned long w;
            ::WriteFile(logcontrol::handle,text,n,&w,0);
            if ( !o ) CloseLog();
        }
    }

    _TEGGO_LOGGER_FAKE_INLINE
    void CXX_STDCALL LogoutN_( BaseStringT<char>& _, int lo )
    {
        if ( !_.Length() || *(_.End()-1) != '\n' ) _.Append("\n");
        _.RestoreCRLF();
        Logout_(+_,_.Length(),lo);
    }

    _TEGGO_LOGGER_FAKE_INLINE
    void CXX_STDCALL LogoutBufferCRLF_( pchar_t buff, int count, int lo )
    {
        StringT<char> _(buff,buff+count);
        _.RestoreCRLF();
        Logout_(+_,_.Length(),lo);
    }

    _TEGGO_LOGGER_FAKE_INLINE
    void CXX_STDCALL LogoutN_( BaseStringT<wchar_t>& _, int lo )
    {
        StringT<char> __(_);
        LogoutN_(__,lo);
    }

    _TEGGO_LOGGER_FAKE_INLINE
    void CXX_STDCALL Logout( pwide_t text, int lo )
    {
        if ( lo > logcontrol::dbglevel ) return;
        StringT<char,128> _(text);
        LogoutN_(_,lo);
    }

    _TEGGO_LOGGER_FAKE_INLINE
    void CXX_STDCALL Logout( pchar_t text, int lo )
    {
        if ( lo > logcontrol::dbglevel ) return;
        StringT<char,128> _(text);
        LogoutN_(_,lo);
    }

    template <class tTc>
    void CXX_STDCALL operator_out__ ( FormatT<tTc> const& fmt, _T_logger_out_ lo)
    {
        if ( lo > logcontrol::dbglevel ) return;
        StringT<tTc> _;
        fmt.Write(_);
        LogoutN_(_,lo);
    }

} // namsepace

#undef _TEGGO_GEBUGCHANNEL_VALUE

_TEGGO_LOGGER_FAKE_INLINE
void CXX_STDCALL operator << ( _T_logger_out_ lo, teggo::FormatT<char> const& fmt)
{teggo::operator_out__(fmt,lo); }

_TEGGO_LOGGER_FAKE_INLINE
void CXX_STDCALL operator << ( _T_logger_out_ lo, teggo::FormatT<wchar_t> const& fmt)
{ teggo::operator_out__(fmt,lo); }

#undef _TEGGO_LOGGER_FAKE_INLINE

#endif // ___e4d630eb_a08f_4ab7_945c_275375d02f63___
