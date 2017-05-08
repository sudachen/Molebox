
/*

Copyright (c) 2008-2009, Alexey Sudachen, alexey@sudachen.name

*/

#include "myafx.h"
#include "peover.h"

extern "C" time_t _BUILD_TIME_EXACT;
extern int g_print_register;

extern "C" unsigned char core0S[];
extern "C" unsigned core0S_size;

extern int DoCommandStream(EhFilter ehf);
extern unsigned PeBoxPackContent(StringParam mainstorage, unsigned, Xdocument xdoc, EhFilter ehf);
bool SearchFullPath(pwide_t name, StringW& fullpath, StringW const&);

#define _XOr(l,x,y) l

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
    if ( Maj == 5 && Min == 0 ) osname = _XOr("Windows 2000",13,2228270);
    else if ( Maj == 5 && Min == 1 ) osname = _XOr("Windows XP",11,5308465);
    else if ( Maj == 5 && Min == 2 ) osname = _XOr("Windows Server 2003",20,6225987);
    else if ( Maj == 6 && Min == 0 )
        if ( osinfo.wProductType == VER_NT_WORKSTATION ) osname = _XOr("Windows Vista",14,7864408);
        else osname = _XOr("Windows Server 2008",20,7012471);
    else if ( Maj == 6 && Min == 1 )
        if ( osinfo.wProductType == VER_NT_WORKSTATION ) osname = _XOr("Windows 7",10,25233281);
        else osname = _XOr("Windows Server 2008 R2",23,26019733);
    else
        osname = _S*_XOr("Unknwon Windows %d.%d",22,27854761)%Maj%Min;
    osname += _S*_XOr(" build %d",10,29558735)%osinfo.dwBuildNumber;
    if ( osinfo.szCSDVersion[0] )
        osname += _S*" (%s)"%osinfo.szCSDVersion;
    return osname;
}

StringW Unavailable(pchar_t p) { return _S*L"option %s is limited by license" %p; }

struct secure_call_t
{
    virtual void registered() {}
};

void Asterisk()
{
    Xo|" ";
}

bool ParseMxfOptLine(StringW const& line, EhFilter ehf, StringW& param, StringW& value)
{
    int i = line.FindChar(L'=');
    if ( i > 0 )
    {
        param.SetRange(+line,i);
        param.Trim();
        value = +line+i+1;
        value.Trim();
        return true;
    }
    return false;
}

struct EhOut : ErrorFilter
{
    virtual bool Ascii(const char* message, int e) const
    {
        if ( e )
            Xoln|_S*"error: %s\n" % message;
        else
            Xoln|_S*"warning: %s\n" % message;
        return !e;
    }
};

CargsPtrW cargs;

int wmain(int argc, wchar_t const** argv)
{
    EhFilter ehf = EhOut();
    cargs = ParseCargs(argc,argv,"V?hHno:m:P:p:r:Rde:a:BqQlLTOUg:G:D:c:C:012FfS:s89tk:",
                       "verbose,help,version,nologo,usereg,inject,regfull,ncr,company:,regmask:,image:,tls:,autoreg:,sdk:",ehf);
    //GetModuleVersion(Hinstance->GetModulePath(),&_MAJOR_VERSION,&_MINOR_VERSION,&_BUILD_NUMBER);
    BufferT<byte_t>  pwd;
    int foo;
    StringW exe_basename_dot;

    if ( cargs && cargs->HasOpts("T") )
        exit(DoCommandStream(ehf));

    if ( !cargs || (cargs && cargs->HasNoOpts("VnRr?","help,version,nologo")) )
    {
        DateTime dt = DateTime::FromPOSIXtime(_BUILD_TIME_EXACT);
        Xoln| _S*"  Molebox Virtualization Solution %d.%d (%d %s %d)"
        %_MAJOR_VERSION %_BUILD_NUMBER
        %dt.Day() %dt.Smon() %dt.Year();
        Xoln| "  https://www.github.com/sudachen/molebox";
        Xoln| "-  -  -  -  -  -  -  -  -  -  -  -  -  -  -  -  -  -  -  -  -  -  -  - ";
    }

    if (cargs && cargs->args.Count() && cargs->HasNoOpts("V?hrR","help,version"))
    {
        FileContainerPtr fcp;
        StringW inputfile;
        StringW outputfile;
        StringW activator;
        fuslog use_activator;
        //StringW vendor_filter;
        //StringW filter_manifest;
        ArrayT<StringW> barray;
        CollectionT<bool> hidden_files;
        CollectionT<int>  conflict_opt;
        CollectionT<StringW> files; /*packed  -> original mapping*/
        CollectionT<bool> executables;
        Xdocument xdoc = Xdocument_New();
        Xnode* package = xdoc->GetRoot()->Insert(L"package");
        StringW search_in_dir;// = SysGetCurrentDirectory();
        StringW pkg_password;
        StringW ext_password;
        StringW ext_mask;
        StringW regmask;
        StringW autoreg_value;

        bool process_executable = false;
        bool do_logging = false;
        bool virtual_registry = false;
        bool relink_exec = false;
        bool use_anticrack = false;
        //bool pkg_compress = false;
        //bool pkg_encrypt = false;
        bool bandle_nonsys_dlls = false;
        bool do_saveiat = false;
        bool do_saversc = false;
        bool do_selfcont = false;
        bool use_compatible = false;
        bool hide_all = false;
        int conflict_rule = 0;
        bool donot_encrypt_extra = false;
        bool inject = false;
        bool regfull = false;
        bool use_regmask = false;
        bool nocrc = false;

        StringW cmdline;
        fuslog use_cmdl;
        StringW envr;
        fuslog use_envr;
        StringW regfile;
        fuslog use_regfile;
        StringW stub;

        if ( cargs->HasOpts("U") )
        {
            setvbuf( stdout, NULL, _IONBF, 0 );
            SwitchXoToUtf8();
            Xoln|_S*"mxbpack version %d.%d" % _MAJOR_VERSION % _BUILD_NUMBER;
            Xoln|_S*"cmdline:%s" %GetCommandLineW();
            Xoln|_S*"system: %s" %QueryOsInformation();
            Xoln|_S*"cpu: %s" %QueryCpuInformation();
            Xoln|"";
            Xoln|"~~~~~";
        }

        u32_t flags = 0;
        unsigned clrver[3] = {0};

        if ( cargs->HasOpts("g") )
        {
            search_in_dir = cargs->opts["-g"];
            SetCurrentDirectoryW(+search_in_dir);
        }

        if ( cargs->HasOpts("H") )
            hide_all = true;

        if ( cargs->HasOpts("C") )
            conflict_rule = StrAsLong(+cargs->opts["-C"],10);

        ArrayT<StringW> args;
        for ( int i = 1; i < cargs->args.Count(); ++i)
        {
            if ( cargs->args[i].StartWith(L"@") )
            {
                DataStreamPtr ds;
                if ( cargs->args[i] == L"@=" )
                    ds = CreateDataStreamFromFILE(stdin,"@=",true,STREAM_READ|STREAM_CHARBYCHAR);
                else
                {
                    ds = OpenStream(+cargs->args[i]+1,STREAM_READ);
                    if (!search_in_dir)
                        search_in_dir = AtCurrentDir(GetDirectoryOfPath(+cargs->args[i]+1));
                }
                if ( ds )
                {
                    bool first_line = true;
                    bool is_opts = false;

                    while ( !ds->Eof() )
                    {
                        StringW fname = ds->UtfReadLine();
                        if ( first_line )
                        {
                            first_line = false;
                            if ( fname && fname[0] == 0xfeff )
                                continue;
                        }
                        fname.Trim();
                        if ( fname == L"<end>" ) break;
                        if ( fname.StartWith(L";") ) continue;
                        if ( fname == L"<options>" )
                            is_opts = true;
                        else if ( fname == L"<files>" )
                            is_opts = false;
                        else if ( fname )
                            if ( !is_opts )
                            {
                                args.Push(fname);
                            }
                            else
                            {
                                StringW param, value;
                                if ( !ParseMxfOptLine(fname,ehf,param,value) )
                                    return false;
                                if ( param.EqualI(L"target") )
                                {
                                    //SearchFullPath(+value,value,search_in_dir);
                                    //if ( search_in_dir && !IsFullPath(value) )
                                    //  outputfile = JoinPath(search_in_dir,value);
                                    //else
                                    outputfile = value;
                                }
                                else if ( param.EqualI(L"source") )
                                {
                                    SearchFullPath(+value,value,search_in_dir);
                                    inputfile = value;
                                }
                                else if ( param.EqualI(L"pkgpwd") )
                                    pkg_password = value;
                                else if ( param.EqualI(L"extpwd") )
                                    ext_password = value;
                                else if ( param.EqualI(L"extmask") )
                                    ext_mask = value;
                                else if ( param.EqualI(L"isexecutable") )
                                    process_executable = ToBool(+value);
                                else if ( param.EqualI(L"dologging") )
                                    do_logging = ToBool(+value);
                                else if ( param.EqualI(L"buildvreg") )
                                    virtual_registry = ToBool(+value);
                                else if ( param.EqualI(L"relink") )
                                    relink_exec = ToBool(+value);
                                else if ( param.EqualI(L"compatible") )
                                    use_compatible = ToBool(+value);
                                else if ( param.EqualI(L"savextra") )
                                    donot_encrypt_extra = ToBool(+value);
                                else if ( param.EqualI(L"autobundle") )
                                    bandle_nonsys_dlls = ToBool(+value);
                                else if ( param.EqualI(L"saveiat") )
                                    do_saveiat = ToBool(+value);
                                else if ( param.EqualI(L"saversc") )
                                    do_saversc = ToBool(+value);
                                else if ( param.EqualI(L"activator") )
                                    activator = +value;
                                else if ( param.EqualI(L"useact") )
                                    use_activator = ToBool(+value);
                                else if ( param.EqualI(L"usecmdl") )
                                    use_cmdl = ToBool(+value);
                                else if ( param.EqualI(L"useenvr") )
                                    use_envr = ToBool(+value);
                                else if ( param.EqualI(L"useregfile") )
                                    use_regfile = ToBool(+value);
                                else if ( param.EqualI(L"regfile") )
                                    regfile = value;
                                else if ( param.EqualI(L"envr") )
                                    envr = value;
                                else if ( param.EqualI(L"cmdline") )
                                    cmdline = value;
                                else if ( param.EqualI(L"hideall") )
                                    hide_all = ToBool(+value);
                                else if ( param.EqualI(L"inject") )
                                    inject = ToBool(+value);
                                else if ( param.EqualI(L"regfull") )
                                    regfull = ToBool(+value);
                                else if ( param.EqualI(L"conflict") )
                                    conflict_rule = ToLong(+value);
                                else if ( param.EqualI(L"regmask") )
                                    regmask = value;
                                else if ( param.EqualI(L"use_regmask") )
                                    use_regmask = ToBool(+value);
                                else if ( param.EqualI(L"autoreg") )
                                {
                                    int autoreg_i = ToLong(+value);
                                    if ( autoreg_i == 0 ) autoreg_value = L"none";
                                    else if ( autoreg_i == 1 ) autoreg_value = L"onstart";
                                    else if ( autoreg_i == 2 ) autoreg_value = L"onpack";
                                    else
                                    {
                                        ehf(_S*"invalid autoreg value '%s'"%value); goto e;
                                    }
                                }
                                else if ( param.EqualI(L"ncr") )
                                {
                                    nocrc = ToBool(+value);
                                }
                            }
                    }
                }
                else
                {
                    ehf(_S*"failed to open file '%s'"%(+cargs->args[i]+1)); goto e;
                }
            }
            else
                args.Push(cargs->args[i]);
        }

        if ( !search_in_dir ) search_in_dir = SysGetCurrentDirectory();
        if ( cargs->args[0] != L"~~" ) inputfile = cargs->args[0];
        else if ( !process_executable ) inputfile = "";

        if ( cargs->HasOpts("o") )
        {
            outputfile = cargs->opts["-o"];
            if ( 0 > outputfile.FindRChar('.') )
            {
                int i = inputfile.FindRChar('.');
                if ( i > 0 )
                    outputfile += inputfile.Str()+i;
            }
        }
        else if ( !outputfile )
        {
            outputfile = cargs->args[0];
            int i = outputfile.FindRChar('.');
            if ( i > 0 )
                outputfile.SetLength_(i);
            outputfile += L"$.EXE";
        }

        if ( inputfile == L"~~" )
            inputfile = "";
        else if ( inputfile && inputfile.FindRChar('.') < 0 )
            inputfile += L".EXE";

        if ( inputfile && !AfxIsExecutable(inputfile) )
        { ehf(_S*"file '%s' is not PE image"%+inputfile); goto e; }

        memset(&flags,0,sizeof(flags));

        if ( cargs->HasOpts("ea") )
        {
            int f = 0;
            use_activator |= true;
            if ( cargs->HasOpts("e") )
                flags |= PEWRAP_EMBEDDING,
                         activator = cargs->opts["-e"],
                         ++f;
            if ( cargs->HasOpts("a") )
                activator = cargs->opts["-a"],
                ++f;
            if ( f > 1 )
            { ehf("use either -e or -a, but not both"); goto e; }
        }
        else if ( activator && use_activator )
            flags |= PEWRAP_EMBEDDING;
        else
            activator = "";

        if ( activator && !IsFullPath(activator) )
            SearchFullPath(+activator,activator,search_in_dir);

        if ( cargs->HasOpts("t") )
            g_print_register = 1;

        if ( cargs->HasOpts("S") )
        {
            flags |= PEWRAP_STUBBED;
            stub = cargs->opts["-S"];
        }

        if ( stub && !IsFullPath(stub) )
            SearchFullPath(+stub,stub,search_in_dir);

        if ( cargs->HasOpts("d") )
            flags |= PEWRAP_DEBUGCORE;

        if ( cargs->HasOpts("89") )
            flags |= PEWRAP_INSINITY;

        if ( cargs->HasOpts("9") )
            flags |= PEWRAP_MINIDUMPFULL;

        if ( cargs->HasOpts(0,"ncr") || nocrc )
            flags |= PEWRAP_NO_CRC0;

        if (1)
        {
            flags |= PEWRAP_STUBBED;

            if ( cargs->HasOpts("s") || do_selfcont || use_compatible )
            {
                flags |= PEWRAP_SELFCONTAINS;
            }
            else if ( cargs->HasOpts("z") || relink_exec )
            {
            Z:
                flags |= PEWRAP_RELINK;
            }
            else goto Z;
        }

        //if ( cargs->HasOpts("p") )
        //    pwd = cargs->opts["-p"];
        //if ( cargs->HasOpts("P") )
        //    pwd = cargs->opts["-P"];
        //if ( cargs->HasOpts("m") )
        //    mask = cargs->opts["-m"];

        if ( cargs->HasOpts("c") )
        {
            cmdline = cargs->opts["-c"];
            use_cmdl = true;
        }

        if ( cargs->HasOpts("l") || do_logging )
            flags |= PEWRAP_LOGGING;
        if ( cargs->HasOpts("q") || do_saveiat )
            flags |= PEWRAP_SAVEIAT;
        if ( cargs->HasOpts("Q") || do_saversc )
            flags |= PEWRAP_SAVERSC;
        if ( cargs->HasOpts("L") )
            flags |= PEWRAP_LOGGING|PEWRAP_DBGLOG;

        if ( cargs->HasOpts("F") || donot_encrypt_extra || (flags&PEWRAP_SELFCONTAINS) )
        {
            donot_encrypt_extra = true;
        }

        if ( cargs->HasOpts(0,"inject") || inject )
        {
            flags |= PEWRAP_INJECT;
        }

        if ( cargs->HasOpts(0,"regmask") )
        {
            regmask = cargs->opts["--regmask"];
            use_regmask = true;
            //printf("regmask: %S\n",+regmask);
        }

        if ( regmask && use_regmask )
        {
            flags |= PEWRAP_HANDLERGS;
        }

        if ( cargs->HasOpts(0,"regfull") || regfull )
        {
            flags |= PEWRAP_HANDLERGS|PEWRAP_HANDLERGS_FULL;
        }

        if ( !autoreg_value && cargs->HasOpts(0,"autoreg") )
        {
            autoreg_value = cargs->opts["--autoreg"];
            if ( !StrSafeEqualI(+autoreg_value,L"none")
                 && !StrSafeEqualI(+autoreg_value,L"onstart")
                 && !StrSafeEqualI(+autoreg_value,L"onpack"))
            {
                ehf(_S*"Invalid autoreg argument: should be none, onstart or onpack");
                goto e;
            }
        }

        if ( !autoreg_value )
        {
            autoreg_value = "onpack";
        }

        if ( StrSafeEqualI(+autoreg_value,L"onstart") )
        {
            flags |= PEWRAP_REGISTER_OCX;
        }

        if ( cargs->HasOpts("O","usereg") || virtual_registry
             || ( autoreg_value && !StrSafeEqualI(+autoreg_value,L"none" )) )
            flags |= PEWRAP_HANDLERGS;

        flags |= PEWRAP_BOXEMBEDED;


        if ( inputfile && PeIsDotNet(+inputfile,clrver,ehf) )
        {
            Xoln| _S*"found CLR metadata, treating as .NET %d.%d.%d executable"
            %clrver[0] %clrver[1] %clrver[2];
            if ( clrver[0] == 1 )
            {
                ehf(_S*"Opps, .NET < 2.x is not supported yet!");
                goto e;
            }
            flags |= PEWRAP_DOTNET;
            flags = (flags & ~PEWRAP_RELINK) | PEWRAP_SELFCONTAINS;

            Xoln| _S*"executable splicing was disabled";
        }

        if ( (cargs->HasOpts("B") || bandle_nonsys_dlls) && inputfile )
            if ( !PEoverFilterBounds(+inputfile,barray,true,ehf,true))
                goto e;

        //__asm int 3;
        if ( inputfile )
            if ( activator )
                Xoln| _S*"%s -> %s%s%s"
                % inputfile
                % outputfile
                %(flags&PEWRAP_EMBEDDING?"/":" + ")
                %(flags&PEWRAP_EMBEDDING?+GetBasenameOfPath(+activator):+activator);
            else
                Xoln| _S*"%s -> %s"
                % inputfile
                % outputfile;

        if ( inputfile )
        {
            exe_basename_dot = GetBasenameOfPath(inputfile);
            if ( 0 < (foo = exe_basename_dot.FindRChar('.')) )
                exe_basename_dot.Resize(foo+1);
        }

        u32_t minsize = 0;
        u32_t mintls = 0;
        u32_t imgbase = 0;

        if ( cargs->HasOpts(0,"tls") )
        {
            mintls = ToLong(+cargs->opts["--tls"]);
        }

        if ( cargs->HasOpts(0,"image") )
        {
            minsize = ToLong(+cargs->opts["--image"]);
        }

        if ( inputfile )
        {
            if ( inputfile )
                if ( DataStreamPtr ids = DataSource->Open(inputfile,STREAM_READ|STREAM_WRITE) )
                {
                    bool r;
                    PeGetParams(ids,&minsize,&mintls,&imgbase,&r);
                }
        }

        for ( int i = 0; i < args.Count(); ++i)
        {
            int j;
            StringW fspath = args[i];

            if ( fspath.StartWith(L"!") ) fspath.Remove(0,1);
            j = fspath.FindChar(';');
            if ( j > 0 ) fspath.Resize(j);

            if ( fspath.Length() > 3 && fspath[1] == ':' )
                ;//fspath = args[i];
            else if ( !SearchFullPath(+fspath,fspath,search_in_dir) )
                fspath = JoinPath(SysGetCurrentDirectory(),fspath);

            fspath.Replace(L"/",L'\\');
            if ( DataStreamPtr ds = DataSource->Open(fspath,STREAM_READ) )
            {
                u32_t tls, size, base;
                bool relocatable;
                if ( PeGetParams(ds,&size,&tls,&base,&relocatable) )
                    if ( imgbase == base || relocatable )
                    {
                        minsize = cxx_max(minsize,size);
                        mintls = cxx_max(mintls,tls);
                        executables[fspath] = true;
                        Xoln|_S*"'%s' is executable from the box" %GetBasenameOfPath(fspath);
                    }
                    else
                        Xoln|_S*"executable '%s' can't be executed from the box" %GetBasenameOfPath(fspath);
            }
        }

        if ( inputfile && (flags&PEWRAP_SELFCONTAINS) )
        {
            StringW boxname = L"<>.EXE";//+GetBasenameOfPath(inputfile);
            files[+boxname] = inputfile;
            hidden_files[+boxname] = true;
            executables[inputfile] = true;
        }

        if ( cargs->args.Count() || barray.Count() )
        {
            StringW act = activator?GetBasenameOfPath(+activator):"";
            act.ToLower();

            if ( StrSafeEqualI(+autoreg_value,L"onstart") )
            {
                Xoln| "activex/ocx components will register on start";
            }

            // barray possible contains imported dlls marked for bounding
            for ( int i = 0; i < barray.Count(); ++i ) // dlls are placed at package root
            {
                StringW bval = GetBasenameOfPath(barray[i]);
                if ( act && (act == bval) )
                    // activator is special dll and is placed out of box, in executable directly
                    continue;
                files[+bval] = barray[i];
            }

            if ( pkg_password ) package->SetParam(L"password",pkg_password);
            package->SetParam(L"embedded",L"yes");
            if ( inputfile )
                package->SetContent("default");
            else
                package->SetContent(GetBasenameOfPath(outputfile));

            package->SetParam(L"compressed",L"yes");
            package->SetParam(L"encrypted",L"yes");

            if ( cargs->HasOpts("O") || virtual_registry
                 || StrSafeEqualI(+autoreg_value,L"onstart")
                 || StrSafeEqualI(+autoreg_value,L"onpack"))
                package->SetParam(L"treatAllAsOCX",L"yes");

            if ( autoreg_value )
                package->SetParam(L"autoreg",autoreg_value);

            for ( int i = 0; i < args.Count(); ++i)
            {
                int j;
                bool hidden = hide_all;
                int  conflict = conflict_rule;
                StringW fspath = args[i];

                if ( fspath.StartWith(L"!") )
                {
                    fspath.Remove(0,1);
                    hidden = true;
                }
                else if ( fspath.StartWith(L"#") )
                {
                    fspath.Remove(0,1);
                    hidden = false;
                }
                else if ( fspath.StartWith(L"*") )
                {
                    fspath.Remove(0,1);
                }

                if ( fspath.StartWith(L"~0") )
                {
                    fspath.Remove(0,2);
                    conflict = 0;
                }
                else if ( fspath.StartWith(L"~1") )
                {
                    fspath.Remove(0,2);
                    conflict = 1;
                }
                else if ( fspath.StartWith(L"~2") )
                {
                    fspath.Remove(0,2);
                    conflict = 2;
                }
                else if ( fspath.StartWith(L"~*") )
                {
                    fspath.Remove(0,2);
                }

                StringW boxpath;
                j = fspath.FindChar(';');
                if ( j > 0 )
                {
                    boxpath = +fspath+j+1;
                    fspath.Resize(j);
                }
                else
                    boxpath = fspath;

                if ( fspath.Length() > 3 && fspath[1] == ':' )
                    ;//fspath = args[i];
                else if ( !SearchFullPath(+fspath,fspath,search_in_dir) )
                    fspath = JoinPath(SysGetCurrentDirectory(),fspath);

                fspath.Replace(L"/",L'\\');

                if ( act && (act == GetBasenameOfPath(fspath).ToLower()) )
                {
                    flags |= PEWRAP_BOXED_ACT;
                    activator = boxpath;
                    printf("activator was set to %S\n",+activator);
                }

                if ( !files.Get(+boxpath) )
                {
                    if ( !DataSource->Exists(fspath) )
                        if ( !SearchFullPath(+fspath,fspath,search_in_dir) )
                        {
                            //__asm int 3;
                            ehf(_S *"could not find file '%s'" % fspath); goto e;
                        }
                    files[+boxpath] = fspath;
                }
                if ( hidden ) hidden_files[+boxpath] = true;
                if ( conflict ) conflict_opt[+boxpath] = conflict;
            }
        }

        if ( inputfile )
        {
            if ( flags & PEWRAP_STUBBED )
                if ( stub )
                {
                    StringW xstub = FindStub(stub);
                    if ( !xstub )
                    {
                        ehf(_S*"custom exe-template '%s' does not exist"%stub);
                        goto e;
                    }
                    stub = xstub;
                    Xoln|_S*"using custom exe-template '%s'"%stub;
                }
                else
                {
                    if ( stub )
                        Xoln|_S*"using private exe-template '%s'" % GetBasenameOfPath(stub);
                }

            fcp = OpenStubbedFileContainer(inputfile,outputfile,stub,flags,minsize,mintls,ehf);

            if ( !fcp ) goto e;

            if ( activator )
                if ( !fcp->AttachActivator(+activator,ehf) )
                    goto e;

            if ( cmdline && use_cmdl )
                if ( !fcp->SetCmdline(cmdline,ehf) )
                    goto e;

            if ( regmask && use_regmask )
                if ( !fcp->SetRegmask(regmask,ehf) )
                    goto e;

            if ( !(flags&PEWRAP_RELINK) && (flags&PEWRAP_DOTNET) )
            {
                StringW fullpath;
                if ( SearchFullPath(L"mscoree.dll",fullpath,search_in_dir) )
                    fcp->BundleDLL(+fullpath,true,ehf); // fake bound, hide imports only
            }
        }

        for ( CollectionT<StringW,wchar_t>::Iterator i = files.Iterate(); i.Next(); )
        {
            Xnode* file = package->Insert(L"file");
            file->SetContent(i.Value());
            file->SetParam(L"rename",i.Key());
            if ( hidden_files.Get(i.Key()) ) file->SetParam(L"hidden",L"yes");
            if ( int* q = conflict_opt.Get(i.Key()) )
            {
                if ( *q == 1 )
                    file->SetParam(L"conflict",L"external");
                else if ( *q == 2 )
                    file->SetParam(L"conflict",L"noaccess");
                else
                    goto conflict_embedded;
            }
            else
            conflict_embedded:
                file->SetParam(L"conflict",L"embedded");

            if ( inputfile )
                if ( !fcp->BundleDLL(+i.Value(),true,ehf) ) // hide imports
                    goto e;
            if ( exe_basename_dot && StrStartWithI(i.Key(), +exe_basename_dot ) )
            {
                file = package->Insert(L"file");
                file->SetContent(i.Value());
                file->SetParam(L"rename",StringW(L"<>.").Append(i.Key()+exe_basename_dot.Length()));
                file->SetParam(L"hidden",L"yes");
                file->SetParam(L"silent",L"yes");
            }
            if ( executables.Get(i.Value()) )
            {
                //Xoln| _S*"%s is executable from the box"%i.Key();
                file->SetParam(L"executable",L"yes");
            }
        }

        if ( package )
        {
            Xnode* mount = xdoc->GetRoot()->Insert(L"mount");
            mount->Insert(L"internal");
            if ( ext_mask )
            {
                Xnode* n = mount->Insert(L"external");
                n->SetContent(ext_mask);
                if (ext_password)
                    n->SetParam(L"value",ext_password);
            }
        }

        if ( inputfile )
        {
            if ( !fcp->Finalize(ehf) )
                goto e;
        }

        if ( unsigned catalog = PeBoxPackContent(+outputfile,!!inputfile,xdoc,ehf) )
        { if (!!inputfile && fcp) fcp->StbInfo2()->catalog = catalog; }
        else
            goto e;

        if ( inputfile && !(flags&PEWRAP_SELFCONTAINS) )
            if ( !fcp->CopyExtraData(inputfile,!donot_encrypt_extra,ehf) )
                goto e;

        if ( inputfile )
            if ( !fcp->Close(ehf) )
                goto e;

        return 0;
    e:
        Xoln|"failed.";
        DeleteFileW(+outputfile);
        return -1;
    }

    else if (cargs && cargs->HasOpts("V","version"))
    {
        Xoln| _S*"%d.%d" % _MAJOR_VERSION % _BUILD_NUMBER;
        return 0;
    }

    else
    {
        Xoln| _S*"using: %s [options] ~~ @<mxbconf.mxb>" % Hinstance->MetaModuleName();
        Xoln| _S*"using: %s [options] ~~ -o external.dat [[!/#/*][~0/1/2/*]file[;pack_as]]" % Hinstance->MetaModuleName();
        Xoln| _S*"using: %s [options] <pefile.exe> [-o out.exe] [[!/#/*][~0/1/2/*]file[;pack_as]]" % Hinstance->MetaModuleName();
        Xoln| "";
        Xoln|"  -V               ~  print program version and exit";
        Xoln|"  -?               ~  print option's help and exit";
        Xoln|"  -o <outputfile>  ~  new name for package or wrapped file";

        if ( cargs && cargs->HasOpts("?","help") )
        {
            //Xoln|"  -L               ~  add bundled dlls into system loaded dlls list";
            Xoln|"  -B               ~  blindly auto-bundle all non-system dependencies";
            Xoln|"  -l               ~  enable virtualization logging";
            Xoln|"  -O               ~  build/use virtual registry";
            Xoln|"  -H               ~  hide all files";
            Xoln|"  -C <0/1/2>       ~  if file named like embedded exists on a disk";
            Xoln|"                      0=>use embedded, 1=>use external, 2=>access error";
            //Xoln|"  -j <filename>    ~  use virtual registry from file <filename>";
            //Xoln|"  -J <filename>    ~  build virtual registry and exit if -j is not specified";
            //Xoln|"                      -O is equal to -j <tempfile> -J <tempfile>
            Xoln|"  -D <envar>=<val> ~  define environment variable";
            //Xoln|"  -G <filename>    ~  load predefined registry keys from a REG file";
            Xoln|"  -e/-a <dllname>  ~  '-e' embed activator dll or '-a' use external one";
            //Xoln|"  -M <msg>         ~  show incompatibility message if OS is earlier Windows2000";
            //Xoln|"                      use -M @mesgfile if message is contained in file mesgfile";
            Xoln|"  -c <cmdl-args>   ~  override commandline arguments";
            //Xoln|"                      ! -c -a -M are incompatible, use only one of them!";
            Xoln|"  -F               ~  do not encrypt executable extradata";
            //Xoln|"  -0               ~  don't change original part of executable";
            //Xoln|"                      ! -q -Q -S -z -s -1 -2 -M -c are not available in -0 mode";
            Xoln|"  -z     *default* ~  slice executable and store inaccessible via VFS";
            Xoln|"  -s               ~  opposite to -z, store original EXE as-is in default package";
            //Xoln|"                      ! -0 -z -s are incompatible, use only one of them!";
            Xoln|"  -S <container>   ~  use defined container instead original executable";
            //Xoln|"  -1               ~  enable anti-cracking features";
            //Xoln|"  -2               ~  like -1 and use some undocumented windows API/behavior";
            //Xoln|"                      ! -0 -1 -2 are incompatible, use only one of them!";
            Xoln|"  -q               ~  don't touch import table (required for some theme hacks)";
            Xoln|"  -Q               ~  don't touch RSRC section";
            //Xoln|"  -w               ~  prevent injection (automaticly used in -2 mode)";
            //Xoln|"  -u               ~  use anti-debugging tricks (automaticly used in -2 mode)";
            //Xoln|"  -j               ~  use jump masking (automaticly used in -2 mode)";
            //Xoln|"  -l               ~  look for protector API markers (automaticly used in -2 mode)";
            Xoln|"  -p <password>    ~  current package password";
            Xoln|"  -P <password>    ~  externally mounted packages password";
            Xoln|"  -m <filemask>    ~  externally mounted packages mask";
            Xoln|"  --usereg         ~  virtualize registry but not build it";
            Xoln|"  --regfull        ~  virtualize registry totally";
            Xoln|"  --inject         ~  inject virtual environment to child external process";
            Xoln|"  --regmask *.reg  ~  file mask to import registery files on start";
            Xoln|"  --company \"name\" ~  company name to add into version resourcses";
            Xoln|"  --autoreg <opt>  ~ none, onstart, onpack. The onpack selected by default";
            Xoln|"  --tls <size>     ~ minimal size of tls area in (bytes)";
            Xoln|"  --image <size>   ~ minimal size of executable image mapping (kilobytes)";
            //Xoln|"  --rqstcert       ~  generate a build certificate request";
            //Xoln|"  -k <key>         ~  'token' license key";
            //Xoln| "Use ~~ instead <pefile.exe> to generate pure nonexecutable package";
        }
        return -1;
    }
}

bool SearchFullPath(pwide_t name, StringW& fullpath, StringW const& search_in)
{
    StringW p = JoinPath(search_in,name);
    if ( DataSource->Exists(+p) )
    {
        fullpath = p;
        fullpath.ToLower();
        return true;
    }
    wchar_t buffer[512] = {0};
    wchar_t* foo;
    if ( int l = SearchPathW(0,name,0,sizeof(buffer)/sizeof(wchar_t),buffer,&foo) )
    {
        fullpath.SetRange(buffer,l);
        fullpath.ToLower();
        return true;
    }
    return false;
}

