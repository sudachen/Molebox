
/*

  Copyright (c) 2008-2009, Alexey Sudachen, alexey@sudachen.name

*/

#include "myafx.h"

enum
  {
    ERR_IVBOX_FAILED_TO_INITIALIZE = 410,
    ERR_REGSVR_FAILED_TO_LOAD,
    ERR_REGSVR_NO_RS_ENTRY,
    ERR_REGSVR_NAME_IS_REQUIRED,
    ERR_REGPRN_FAILED_TO_SERILIZE,
    ERR_REGCLR_FAILED_TO_CLEAR,
    ERR_LOGSTA_FAILED_TO_LOG,
    ERR_REGSVR_NO_TYPELIB
  };

extern bool InstallVBOX(unsigned flags);
extern bool GetVboxRegistryText(char **p,int *len);
extern bool ClearVboxRegistry();
extern bool LogVboxState();

bool ManualRegister(pwide_t server);

int GetArg(pchar_t S,BaseStringT<char> &out)
  {
    int i = StrFindChar(S,'\"');
    if ( i > 0 )
      {
        int j = StrFindChar(S+i+1,'\"');
        if ( j > 0 )
          {
            out.SetRange(S+i+1,j);
            return i+j+1;
          }
      }
    return 0;
  }

int DoCommandStream(EhFilter ehf)
  {
    fprintf(stdout,"200 0 0 STARTED\n"); fflush(stdout);
    BufferT<char> command(4*1024);
    for (;;)
      {
        u32_t foo, bar;
        memset(+command,0,command.Count());
        if ( !fgets(+command,command.Count(),stdin) ) break;
        if ( sscanf(+command,"ivbox %x",&foo) == 1 )
          {
            if ( InstallVBOX(foo) )
              fprintf(stdout,"200 0 0 VBOX INITIALIZED\n");
            else
              fprintf(stdout,"%d 0 0 VBOX FAILED TO INITIALIZE\n",ERR_IVBOX_FAILED_TO_INITIALIZE);
            fflush(stdout);
          }
        else if ( StrStartWith(+command,"exit") )
          {
            fprintf(stdout,"200 0 0 FINISHED\n"); fflush(stdout);
            return 0;
          }
        else if ( StrStartWith(+command,"regsvr") )
          {
            StringA serverA;
            GetArg(+command,serverA);
            if ( serverA )
              {
                StringW server = WideStringFromUtf8(+serverA);
                StringW cwd = SysGetCurrentDirectory();
                if ( StringW srvfold = GetDirectoryOfPath(+server) ) 
                  SysSetCurrentDirectory(srvfold);
                //wchar_t shname[256];
                //GetShortPathNameW(+server,shname,sizeof(shname)/2);
                wchar_t sName[256] = {0};
                GetLongPathNameW(+server,sName,sizeof(sName)/2);
                if ( HMODULE hdll = LoadLibraryW(sName) )
                  {
                    CoInitialize(0);
                    long (__stdcall *DLLregisterServer)(void) = 0;
                    *(void**)&DLLregisterServer = GetProcAddress(hdll,"DllRegisterServer");
                    if ( DLLregisterServer )
                      {  
                        ClearVboxRegistry(); // kill parasites!
                        long st = DLLregisterServer();
                        if ( st )
                          fprintf(stdout,"%d 0 0 FAILED TO CALL REGISTERSERVER %08x\n",ERR_REGSVR_FAILED_TO_LOAD,st);
                        else
                          fprintf(stdout,"200 0 0 SUCCESSFUL REGISTERED\n");
                      }
                    else
                      fprintf(stdout,"%d 0 0 FAILED TO GET REGISTERSERVER ENTRY\n",ERR_REGSVR_NO_RS_ENTRY);
                    fflush(stdout);
                    CoUninitialize();
                    FreeLibrary(hdll);
                  }
                else
                  { fprintf(stdout,"%d 0 0 FAILED TO LOAD SERVER\n",ERR_REGSVR_FAILED_TO_LOAD); fflush(stdout); }
                SysSetCurrentDirectory(cwd);
              }
            else
              { fprintf(stdout,"%d 0 0 SERVER NAME IS REQUIRED\n",ERR_REGSVR_NAME_IS_REQUIRED); fflush(stdout); }
          }
        else if ( StrStartWith(+command,"altreg") )
          {
            StringA serverA;
            GetArg(+command,serverA);
            if ( serverA )
              {
                //ClearVboxRegistry();
                StringW server = WideStringFromUtf8(+serverA);
                //wchar_t shname[256];
                //GetShortPathNameW(+server,shname,sizeof(shname)/2);
                wchar_t sName[256] = {0};
                GetLongPathNameW(+server,sName,sizeof(sName)/2);
                if ( ManualRegister(sName) )
                  fprintf(stdout,"200 0 0 SUCCESSFUL REGISTERED\n");
                else
                  fprintf(stdout,"%d 0 0 FAILED TO FIND TYPELIBRARY\n",ERR_REGSVR_NO_TYPELIB);
                fflush(stdout);
              }
            else
              { fprintf(stdout,"%d 0 0 SERVER NAME IS REQUIRED\n",ERR_REGSVR_NAME_IS_REQUIRED); fflush(stdout); }
          }
        else if ( StrStartWith(+command,"regprn") )
          {
            char *p;
            int  len;
            if ( GetVboxRegistryText(&p,&len) )
              {
                fprintf(stdout,"200 0 %d REGISTRY\n",len);
                fwrite(p,len,1,stdout);
              }
            else
              fprintf(stdout,"%d 0 0 FAILED TO SERILIZE REGISTRY\n",ERR_REGPRN_FAILED_TO_SERILIZE);
            fflush(stdout);
          }
        else if ( StrStartWith(+command,"regclr") )
          {
            if ( ClearVboxRegistry() )
              fprintf(stdout,"200 0 0 REGISTRY WAS CLEARED\n");
            else
              fprintf(stdout,"%d 0 0 FAILED TO CLEAR REGISTRY\n",ERR_REGCLR_FAILED_TO_CLEAR);
            fflush(stdout);
          }
        else if ( StrStartWith(+command,"logsta") )
          {
            if ( LogVboxState() )
              fprintf(stdout,"200 0 0 STATE HAS LOGGED\n");
            else
              fprintf(stdout,"%d 0 0 FAILED TO LOG STATE\n",ERR_LOGSTA_FAILED_TO_LOG);
            fflush(stdout);
          }
      }

    return -1; // hmm, it's unreachable code
  }

//void RegWriteTypelib(GUID const &guid,pchar_t server)
//  {
//  }
  
void RegWriteCoClass(u32_t typekind,GUID const &guid, GUID const &tlib, StringW const &Sn, pwide_t server)
  {
    StringT<char,46> clsid_s;
    StringT<char,46> tlib_s;

    StringT<char,8+46> keyroot;

    HRESULT hr = S_OK;

    teggo::GUIDToRegString(guid,clsid_s);
    teggo::GUIDToRegString(tlib,tlib_s);
    keyroot.Set(_S*"CLSID\\%s" %clsid_s);

    teggo::ComRegWriteStr(+keyroot,0,0,+StringA(Sn));
    teggo::ComRegWriteStr(+keyroot,"InprocServer32",0,+StringA(server));
    teggo::ComRegWriteStr(+keyroot,"TypeLib",0,+tlib_s);
    teggo::ComRegWriteStr(+StringA(Sn),"CLSID",0, +clsid_s);
  }

extern bool RegisterRcScript(pchar_t text,unsigned len,pwide_t modname);
 
namespace 
  {
    pwide_t g_modulename = 0;
  
    BOOL CALLBACK fEnumResNameProc(HMODULE hModule,LPCWSTR lpszType, LPWSTR lpszName,LONG f)
      {
        //__asm int 3;
        if ( HRSRC hr = FindResourceW(hModule,lpszName,lpszType) )
          if ( HGLOBAL hg = LoadResource(hModule,hr) )
            {
              unsigned sz = SizeofResource(hModule,hr);
              if ( RegisterRcScript((pchar_t)hg,sz,g_modulename) )
                *(bool*)f = true;
            }
        return TRUE;
      }

    bool ManualRegisterThroughRegScript(pwide_t server)
      {
        bool succeeded = false;
        g_modulename = server;
        if ( HMODULE hm = LoadLibraryExW(server,0,LOAD_LIBRARY_AS_DATAFILE) )
          {
            EnumResourceNamesW(hm,L"REGISTRY",fEnumResNameProc,(LONG)&succeeded);
            FreeLibrary(hm);
          }
        return succeeded;
      }
  }
  
bool ManualRegister(pwide_t server)
  {
    rcc_ptr< ITypeLib > tlib;
    StringW tlib_prefix;
    GUID tlibguid;
    bool is_typelib = false;
    
    bool co_registered =  ManualRegisterThroughRegScript(server);
    
    if ( FAILED( LoadTypeLibEx( +server, REGKIND_NONE, &+tlib ) ) )
      return co_registered;
    
    ::RegisterTypeLib(+tlib,(BSTR)server,(BSTR)L"");
    if ( co_registered ) return co_registered;
    
    if ( DataStreamPtr ds = OpenStream(server,STREAM_READ) )
      {
        byte_t b[5] = { 0 };
        ds->Read(b,4);
        if ( 0 == memcmp(b,"MSFT",4) )
          is_typelib = true;
      }

    {
      TLIBATTR *tatr = 0;
      //__asm int 3;
      if ( SUCCEEDED( tlib->GetLibAttr(&tatr) ) )
        {
          BSTR tlib_name = 0;
          tlib->GetDocumentation(-1,&tlib_name,0,0,0);
          tlib_prefix = +tlib_name;
          if ( tlib_name )
            SysFreeString( tlib_name );
          tlibguid = tatr->guid;
          tlib->ReleaseTLibAttr(tatr);
        }
    }

    if ( tlib_prefix.Length() > 4 && tlib_prefix.EndsWith(L"Lib") )
    tlib_prefix.Resize(tlib_prefix.Length()-3);
      
    if ( !is_typelib && !co_registered )
      {
        //__asm int 3;
        u32_t typesCount = tlib->GetTypeInfoCount();
        for ( u32_t typeIdx = 0; typeIdx < typesCount; ++typeIdx )
          {
            rcc_ptr < ITypeInfo > tinfo;
            if ( SUCCEEDED( tlib->GetTypeInfo( typeIdx, &+tinfo ) ) )
              {
                TYPEATTR *tatr = 0;
                if ( SUCCEEDED( tinfo->GetTypeAttr(&tatr) ) )
                  {
                    if ( 
                        tatr->typekind == TKIND_COCLASS 
                      || ( tatr->typekind == TKIND_DISPATCH) && !(tatr->wTypeFlags&TYPEFLAG_FDUAL) )
                      {
                        BSTR coclass_name = 0;
                        StringW nS;
                        tlib->GetDocumentation(typeIdx,&coclass_name,0,0,0);
                        if ( coclass_name )
                          { 
                            nS = _S*L"%?.%?" %tlib_prefix %coclass_name;
                            SysFreeString( coclass_name );
                          }
                          
                        RegWriteCoClass(tatr->typekind,tatr->guid,tlibguid,nS,server);
                      }
                    tinfo->ReleaseTypeAttr(tatr);
                  }
              }
          }
      }
      
    return true;    
  }
