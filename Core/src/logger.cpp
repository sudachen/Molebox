
/*

  Copyright (c) 2008-2009, Alexey Sudachen, alexey@sudachen.name

*/

#include "_common.h"
#include "logger.h"
#include "xnt.h"
#include "XorS.h"
#include "apif.h"

#include <stdio.h>
#include <time.h>

#include "vfs.h"

#if _DEBUG
# define _LXL_GEBUGCHANNEL_VALUE true
#else
# define _LXL_GEBUGCHANNEL_VALUE true
#endif

typedef HANDLE LOG_HANDLE;

bool iflog = false;
bool ifdbg = false;

namespace logcontrol
  {
    LOG_HANDLE handle = 0;
    int dbglevel = 100;
    bool reopen = false;
  }

bool IsLogLevelGeq(int lo) { return lo <= logcontrol::dbglevel; }

StringA QueryCpuInformation()
  {
    CPUINFO cpui;
    GetCPUInfo(&cpui);
    return FormatCPUInfo(&cpui);
  }
  
StringA QueryOsInformation()
  {
    OSVERSIONINFOEX osinfo;
    osinfo.dwOSVersionInfoSize = sizeof(OSVERSIONINFOEX);
    GetVersionEx((OSVERSIONINFO*)&osinfo);
    StringA osname;
    u32_t Maj = osinfo.dwMajorVersion, Min = osinfo.dwMinorVersion;
    if ( Maj == 5 && Min == 0 ) osname = _XOr("Windows 2000",13,305476175);
    else if ( Maj == 5 && Min == 1 ) osname = _XOr("Windows XP",11,307769976);
    else if ( Maj == 5 && Min == 2 ) osname = _XOr("Windows Server 2003",20,308818536);
    else if ( Maj == 6 && Min == 0 )
      if ( osinfo.wProductType == VER_NT_WORKSTATION ) osname = _XOr("Windows Vista",14,309738898);
      else osname = _XOr("Windows Server 2008",20,312098238);
    else if ( Maj == 6 && Min == 1 )
      if ( osinfo.wProductType == VER_NT_WORKSTATION ) osname = _XOr("Windows 7",10,312491424);
      else osname = _XOr("Windows Server 2008 R2",23,314326484);
    else
      osname = _S*_XOr("Unknwon Windows %d.%d",22,316227067)%Maj%Min;
    osname += _S*_XOr(" build %d",10,316947740)%osinfo.dwBuildNumber;
    if ( osinfo.szCSDVersion[0] )
      osname += _S*" (%s)"%osinfo.szCSDVersion;
    return osname;
  }

int StartLog()
  {
    if ( logcontrol::handle ) return 0;
    StringT<wchar_t> my_log_name;
    
    my_log_name = JoinPath(GetDirectoryOfPath(VFS->containerName),GetBasenameOfPath(Hinstance->GetMainModulePath()));
    //Hinstance->GetMainModulePath();
    my_log_name.Replace(L"/",L'\\');
    my_log_name += _XOr("-up.txt",8,317799699);
    
    APIF->AcquireFilterLock();
    
    logcontrol::handle = CreateFileW(
      +my_log_name,
      GENERIC_WRITE, FILE_SHARE_READ, 0,
      logcontrol::reopen?OPEN_ALWAYS:CREATE_ALWAYS, 0, 0);
    
    if ( logcontrol::handle == INVALID_HANDLE_VALUE )
      {
        logcontrol::handle = CreateFileW(
          (0|_S*L"%s.%d"%my_log_name%GetCurrentProcessId()),
          GENERIC_WRITE, FILE_SHARE_READ, 0,
          logcontrol::reopen?OPEN_ALWAYS:CREATE_ALWAYS, 0, 0);
        if ( logcontrol::handle == INVALID_HANDLE_VALUE )
          {
            int i = my_log_name.FindRChar('\\');
            if ( i > 0 ) my_log_name.Remove(0,i);
            my_log_name.Insert(0,Hinstance->GetTemporalDirectory());
            logcontrol::handle = CreateFileW(
              +my_log_name,
              GENERIC_WRITE, FILE_SHARE_READ, 0,
              logcontrol::reopen?OPEN_ALWAYS:CREATE_ALWAYS, 0, 0);
          }
      }
      
    if ( !logcontrol::handle || logcontrol::handle == INVALID_HANDLE_VALUE ) return 0;
    if ( !logcontrol::reopen )
      {
        time_t t = time(0);
        StringT<char> ob = _S*_XOr("~\r\n~ %s\r\n",10,353189127) %StringT<char,64>(ctime(&t)).Chomp();
        //ob += _S*_XOr("~ V-Core %d.%d (C) 2002-2010, Alexey Sudachen, DesaNova Ltda.\r\n~ %s %s\r\n~\r\n\r\n",78,354303290)
        ob += _S*_XOr("~ V-Core %d.%d\r\n~ %s %s\r\n~\r\n\r\n",31,359480679)
          %_MAJOR_VERSION
          %_BUILD_NUMBER
          %QueryOsInformation()
          %QueryCpuInformation();
        //ob += _XOr("~ Copyright (C) 2002-2009, Alexey Sudachen, http://alexey.sudachen.name\r\n~\r\n",77,360856710);
        //ob += _S*_XOr("~ Running on %s\r\n~ %s\r\n",24,366820599) %QueryOsInformation()%QueryCpuInformation();
        //ob += _S*_XOr("~ %s %s",8,369048597) %QueryOsInformation()%QueryCpuInformation();
        //ob += _XOr("~\r\n",4,336083976);
        unsigned long w;
        WriteFile(logcontrol::handle,+ob,ob.Length(),&w,0);
      }
      
    APIF->ReleaseFilterLock();
    
    logcontrol::reopen = true;
    iflog = true;
    return 1;
  }

static int logging_step_idx = TlsAlloc();

void LogoutS( teggo::BaseStringT<char> &bs )
  {
    int logging_step = (int)TlsGetValue(logging_step_idx);
    int diff = 0;
    static char const steps[] = "  .    .    .    .    .    .    .    .    .    .    .    .    .    .    .    .  ";
    static char const steps2[] = "  ...  ...  ...  ...  ...  ...  ...  ...  ...  ...  ...  ...  ...  ...  ...  ...";
    if ( bs.StartWith("{CAL}") )
      diff = 1;
    else if ( bs.StartWith("{RET}") )
      diff = -1;
    if ( !bs.Length() || *(bs.End()-1) != '\n' ) bs.Append("\n");
    if ( logging_step > 0)
      bs.Insert(0,(diff<0?steps2:steps),5);
    if ( logging_step > 1 )
      bs.Insert(0,steps,cxx_min<unsigned>(((logging_step-1)*5),sizeof(steps)));
    bs.Insert(0,0|_S*"%5dms|%04x| " %teggo::GetMillis() %GetCurrentThreadId());
    bs.RestoreCRLF();
    LogoutS_(+bs,bs.Length(),Lout);
    logging_step += diff;
    TlsSetValue(logging_step_idx,(void*)logging_step);
  }

void LogoutS_( pchar_t text, int n, int lo )
  {
    void *o = logcontrol::handle;
#ifndef _DISABLE_DBGOUTPUT
    OutputDebugStringA(text);
#endif
    if ( o )
      {
        if ( n < 0 ) n = strlen(text);
        unsigned long w;
        APIF->AcquireFilterLock();
        WriteFile(logcontrol::handle,text,n,&w,0);
        APIF->ReleaseFilterLock();
      }
  }

void LogoutString( pwide_t text, int lo )
  {
    if ( lo > logcontrol::dbglevel ) return;
    StringT<char,128> _(text);
    LogoutS((BaseStringT<char>&)_);
  }

void LogoutString( pchar_t text, int lo )
  {
    if ( lo > logcontrol::dbglevel ) return;
    StringT<char,128> _(text);
    LogoutS(_);
  }

void LogoutFormatted( FormatT<char> const& fmt, _Lx_logger_out_ lo)
  {
    if ( lo > logcontrol::dbglevel ) return;
    StringT<char,128> _;
    fmt.Write(_);
    LogoutS(_);
  }

#undef _LXL_GEBUGCHANNEL_VALUE
