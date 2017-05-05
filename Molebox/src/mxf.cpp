/*

  Copyright (c) 2008-2009, Alexey Sudachen, alexey@sudachen.name

*/

#include "myafx.h"
#include "wx/treectrl.h"
#include "mxf.h"

bool ParseMxfOptLine(StringW const &line, EhFilter ehf, StringW &param, StringW &value)
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

MxfFolder *EnsureFolderExists(CollectionT<MxfFolderPtr> &folders,StringW const &dir)
  {
    if ( !dir ) return +g_root_mxfFolder;    
    if ( MxfFolderPtr *f = folders.Get(+dir)) return +*f;
    MxfFolder *p = EnsureFolderExists(folders,GetDirectoryOfPath(dir));
    StringW foldname = GetBasenameOfPath(dir);
    MxfFolderPtr mxff = RccPtr( new MxfFolder(p,foldname) );
    p->Push(mxff);
    folders[+dir] = mxff;
    return +mxff;
  }

bool ParseMxfFileLine(StringW const &arg, EhFilter ehf, StringW &fspath, StringW &boxpath, bool &hiden)
  {
    fspath = arg;
    bool hidden = false;
    
    if ( fspath.StartWith(L"!") ) 
      {
        fspath.Remove(0,1);
        hidden = true;
      }

    int j = fspath.FindChar(';');
    if ( j > 0 )
      {
        boxpath = +fspath+j+1;
        fspath.Resize(j);
      }
    else
      boxpath = fspath;
    
    //if ( fspath.Length() > 3 && fspath[1] == ':' )
    //  fspath = arg;
    //else
    //  if ( !SearchFullPath(+fspath,fspath) )
    //    fspath = JoinPath(SysGetCurrentDirectory(),fspath);
    //
    
    fspath.Replace(L"/",L'\\');
    return true;
  }

bool ReadMxfConfig(StringParam mxfconf, EhFilter ehf)
  {
    CollectionT<MxfFolderPtr> folders_;
    CollectionT<bool> files; // pkg -> disk
    if ( DataStreamPtr ds = DataSource->Open(mxfconf,STREAM_READ) )
      {
        g_mxf_is_executable = false;
        enum { STATE_NONE, STATE_OPTIONS, STATE_FILES };
        int state = STATE_NONE;
        StringW line, boxpath, fspath;
        bool hidden;
        bool lbind = true;
        ds->UtfReadLineS(line); 
        if ( line.Length() > 1 && line[0] == 0xfeff ) 
          line.Remove(0,1);
        if ( !line.StartWith(L";MOLEBOX CONFIG SCRIPT") )
          { ehf("The file isn't a molebox script"); return false; }
        while( !ds->Error() )
          {
            ds->UtfReadLineS(line); 
            line.Trim();
            if ( line == L"<end>" ) break;
            if ( line == L"<files>" )
              state = STATE_FILES;
            else if ( line == L"<options>" )
              state = STATE_OPTIONS;
            else if ( line && line[0] != ';' && line[0] != '<' )
              {
                if ( state == STATE_FILES )
                  {
                    if ( !ParseMxfFileLine(line,ehf,fspath,boxpath,hidden) )
                      return false;
                    if ( !files.Get(+boxpath) )
                      {
                        StringW pkgdir = GetDirectoryOfPath(+boxpath);
                        MxfFolder *mxff = EnsureFolderExists(folders_,+pkgdir);
                        MxfFilePtr f = RccPtr( new MxfFile(mxff,GetBasenameOfPath(+boxpath),fspath,lbind) );
                        mxff->Push(f);
                        files[+boxpath] = f;
                      }
                  }
                else if ( state == STATE_OPTIONS )
                  {
                    StringW param, value;                    
                    if ( !ParseMxfOptLine(line,ehf,param,value) )
                      return false;
                    if ( param.EqualI(L"target") )
                      g_mxf_target = value;
                    else if ( param.EqualI(L"source") )
                      g_mxf_source = value;
                    else if ( param.EqualI(L"pkgpwd") )
                      g_mxf_pkg_password = value;
                    else if ( param.EqualI(L"extpwd") )
                      g_mxf_ext_password = value;
                    else if ( param.EqualI(L"extmask") )
                      g_mxf_ext_mask = value;
                    else if ( param.EqualI(L"isexecutable") )
                      g_mxf_is_executable = ToBool(+value);
                    else if ( param.EqualI(L"dologging") )
                      g_mxf_do_logging = ToBool(+value);
                    else if ( param.EqualI(L"usevreg") )
                      g_mxf_use_vreg = ToBool(+value);
                    else if ( param.EqualI(L"buildvreg") )
                      g_mxf_build_vreg = ToBool(+value);
                    else if ( param.EqualI(L"relink") )  
                      g_mxf_relink_exec = ToBool(+value);
                    else if ( param.EqualI(L"compatible") )
                      g_mxf_relink_exec = !ToBool(+value);
                    else if ( param.EqualI(L"anticrack") )
                      g_mxf_use_anticrack = ToBool(+value);
                    else if ( param.EqualI(L"compress") )
                      g_mxf_compress = ToBool(+value);
                    else if ( param.EqualI(L"encrypt") )
                      g_mxf_encrypt = ToBool(+value);
                    else if ( param.EqualI(L"autobundle") )
                      g_mxf_autobundle = ToBool(+value);
                    else if ( param.EqualI(L"saveiat") )
                      g_mxf_do_saveiat = ToBool(+value);
                    else if ( param.EqualI(L"saversc") )
                      g_mxf_do_saversc = ToBool(+value);
                    else if ( param.EqualI(L"usecmdl") )
                      g_mxf_use_cmdline = ToBool(+value);
                    else if ( param.EqualI(L"useregfile") )
                      g_mxf_use_regfile = ToBool(+value);
                    else if ( param.EqualI(L"useenviron") )
                      g_mxf_use_environ = ToBool(+value);
                    else if ( param.EqualI(L"useact") )
                      g_mxf_use_activator = ToBool(+value);
                    else if ( param.EqualI(L"cmdline") )
                      g_mxf_cmdline = value;
                    else if ( param.EqualI(L"regfile") )
                      g_mxf_regfile = value;
                    else if ( param.EqualI(L"environ") )
                      g_mxf_environ = value;
                    else if ( param.EqualI(L"activator") )
                      g_mxf_activator = value;
                    else if ( param.EqualI(L"hideall") )
                      g_mxf_hideall = ToBool(+value);
                    else if ( param.EqualI(L"inject") )
                      g_mxf_inject = ToBool(+value);
                    else if ( param.EqualI(L"savextra") )
                      g_mxf_extradata = ToBool(+value);
                    else if ( param.EqualI(L"fullreg") )
                      g_mxf_fullreg = ToBool(+value);
                    else if ( param.EqualI(L"use_regmask") )
                      g_mxf_use_regmask = ToBool(+value);
                    else if ( param.EqualI(L"regmask") )
                      g_mxf_regmask = value;
                    else if ( param.EqualI(L"ncr") )
                      g_mxf_ncr = ToBool(+value);
                    else if ( param.EqualI(L"autoreg") )
                      {
                        g_mxf_autoreg = ToLong(+value);
                        if ( g_mxf_autoreg <0 || g_mxf_autoreg > 2 ) g_mxf_autoreg = 2;
                      }
                    else if ( param.EqualI(L"optional") )
                      g_mxf_optional = value;
                    else if ( param.EqualI(L"author") )
                      g_mxf_author = value;
                    else if ( param.EqualI(L"tls_size") )
                      g_mxf_TLS_size = ToLong(+value);
                    else if ( param.EqualI(L"image_size") )
                      g_mxf_IMG_size = ToLong(+value);
                    else if ( param.EqualI(L"conflict") )
                      {
                        g_mxf_conflict = ToLong(+value);
                        if ( g_mxf_conflict < 0 ||g_mxf_conflict > 2 ) g_mxf_conflict = 0;
                      }
                    else
                      // skip invalid param
                      ;
                  }
              }
          }
        g_mxf_optscrc = MxfCalcuateOptsCRC();
        g_mxf_changed = false;
        return true;
      }
    else
      return ehf("failed to open MolBox Config Script file");
  }

bool MxfFolder::WriteFilesRecursive(DataStream *ds, EhFilter ehf)
  {
    //__asm int 3;
    for ( int i = 0; i < files_.Count(); ++i )
      ds->UtfPrint(0|_S*L"%s;%s\n" %files_[i]->GetDiskPath() %files_[i]->GetPkgPath() );
    for ( int i = 0; i < subfolds_.Count(); ++i )
      subfolds_[i]->WriteFilesRecursive(ds,ehf);
    return true;
  }
  
bool WriteMxfConfig(DataStream *ds, EhFilter ehf)
  {
    ds->UtfPrint(L"\xfeff;MOLEBOX CONFIG SCRIPT 1.0\n");
    ds->Print("<options>\n");
    if (g_mxf_target)
      ds->UtfPrint(_S*L"target=%s\n"%+g_mxf_target);
    if ( g_mxf_source )
      ds->UtfPrint(_S*L"source=%s\n"%+g_mxf_source);
    if ( g_mxf_pkg_password )
      ds->UtfPrint(_S*L"pkgpwd=%s\n"%+g_mxf_pkg_password);
    if ( g_mxf_ext_password )
      ds->UtfPrint(_S*L"extpwd=%s\n"%+g_mxf_ext_password);
    if ( g_mxf_ext_mask )
      ds->UtfPrint(_S*L"extmask=%s\n"%+g_mxf_ext_mask);
    if ( g_mxf_is_executable )
      ds->UtfPrint(L"isexecutable=yes\n"); 
    if ( g_mxf_do_logging )
      ds->UtfPrint(L"dologging=yes\n");
    if ( g_mxf_use_vreg )
      ds->UtfPrint(L"usevreg=yes\n");
    else
      ds->UtfPrint(L"usevreg=no\n");
    
    if ( g_mxf_build_vreg )
      ds->UtfPrint(L"buildvreg=yes\n");
    if ( g_mxf_relink_exec )     
      {
        ds->UtfPrint(L"relink=yes\n");   
        ds->UtfPrint(L"compatible=no\n");
      }
    else
      {
        ds->UtfPrint(L"relink=no\n");
        ds->UtfPrint(L"compatible=yes\n");
      }   
    if ( g_mxf_use_anticrack )
      ds->UtfPrint(L"anticrack=yes\n");
    if ( g_mxf_compress )
      ds->UtfPrint(L"compress=yes\n");
    if ( g_mxf_encrypt )
      ds->UtfPrint(L"encrypt=yes\n");
    if ( g_mxf_autobundle )
      ds->UtfPrint(L"autobundle=yes\n");
    if ( g_mxf_do_saveiat )
      ds->UtfPrint(L"saveiat=yes\n");
    if ( g_mxf_do_saversc )
      ds->UtfPrint(L"saversc=yes\n");
    if ( g_mxf_cmdline )
      ds->UtfPrint(_S*L"cmdline=%s\n"%+g_mxf_cmdline);
    if ( g_mxf_use_cmdline )
      ds->UtfPrint(L"usecmdl=yes\n");
    if ( g_mxf_regfile )
      ds->UtfPrint(_S*L"regfile=%s\n"%+g_mxf_regfile);
    if ( g_mxf_use_regfile )
      ds->UtfPrint(L"useregfile=yes\n");
    if ( g_mxf_environ )
      ds->UtfPrint(_S*L"environ=%s\n"%+g_mxf_environ);
    if ( g_mxf_use_environ )
      ds->UtfPrint(L"useenviron=yes\n");
    if ( g_mxf_hideall )
      ds->UtfPrint(L"hideall=yes\n");
    if ( g_mxf_use_activator )
      ds->UtfPrint(L"useact=yes\n");
    if ( g_mxf_inject )
      ds->UtfPrint(L"inject=yes\n");
    if ( g_mxf_extradata )
      ds->UtfPrint(L"savextra=yes\n");
    if ( g_mxf_fullreg )
      ds->UtfPrint(L"fullreg=yes\n");
    if ( g_mxf_conflict )
      ds->UtfPrint(_S*L"conflict=%d\n"%g_mxf_conflict);
    if ( g_mxf_activator )
      ds->UtfPrint(_S*L"activator=%s\n"%+g_mxf_activator);
    if ( g_mxf_use_regmask )
      ds->UtfPrint(_S*L"use_regmask=yes\n"%g_mxf_use_regmask);
    if ( g_mxf_regmask )
      ds->UtfPrint(_S*L"regmask=%s\n"%g_mxf_regmask);
    
    ds->UtfPrint(_S*L"autoreg=%d\n"%g_mxf_autoreg);
    
    if ( g_mxf_TLS_size )
      ds->UtfPrint(_S*L"tls_size=%d\n"%g_mxf_TLS_size);
    if ( g_mxf_IMG_size )
      ds->UtfPrint(_S*L"image_size=%d\n"%g_mxf_IMG_size);
    if ( g_mxf_author )
      ds->UtfPrint(_S*L"author=%s\n"%g_mxf_author);
    if ( g_mxf_optional )
      ds->UtfPrint(_S*L"optional=%s\n"%g_mxf_optional);
    if ( g_mxf_ncr )
      ds->UtfPrint(_S*L"ncr=yes\n");
    ds->Print("<files>\n");
    if ( !g_root_mxfFolder->WriteFilesRecursive(ds,ehf) ) return false;
    ds->Print("<end>\n");
    return true;
  }

bool WriteMxfConfig(StringParam mxfconf, EhFilter ehf)
  {
    if ( DataStreamPtr ds = DataSource->Open(mxfconf,STREAM_CREATE|STREAM_WRITE) )
      {
        return WriteMxfConfig(+ds,ehf);
      }
    else
      return ehf("failed to create file");
  }

StringW MxfFile::GetPkgPath() 
  {
    StringW buf;
    return folder_->ResolvePkgPath(buf,+pkgname_);
  }

void MxfFile::Remove()
  {
    if ( folder_ ) 
      folder_->Remove(this);
  }

StringW g_current_mxf_file_name;
StringW g_mxf_pkg_password;;
StringW g_mxf_ext_mask;
StringW g_mxf_ext_password;
StringW g_mxf_target;
StringW g_mxf_source;
bool g_mxf_is_executable = true;
bool g_mxf_do_logging = false;
bool g_mxf_use_vreg = false;
bool g_mxf_build_vreg = false;
bool g_mxf_relink_exec = true;
bool g_mxf_use_anticrack = false;
bool g_mxf_compress = false;
bool g_mxf_encrypt = false;
bool g_mxf_hideall = false;
bool g_mxf_autobundle = false;
bool g_mxf_changed = false;
bool g_mxf_do_saveiat = false;
bool g_mxf_do_saversc = false;
bool g_mxf_inject = false;
bool g_mxf_extradata = false;
bool g_mxf_fullreg = false;
u32_t g_mxf_optscrc = 0;
StringW g_mxf_cmdline;
bool g_mxf_use_cmdline = false;
StringW g_mxf_regfile;
bool g_mxf_use_regfile = false;
StringW g_mxf_environ;
bool g_mxf_use_environ = false;
StringW g_mxf_token;
StringW g_mxf_activator;
bool g_mxf_use_activator = false;
int g_mxf_conflict = 0;
bool g_mxf_use_regmask = false;
StringW g_mxf_regmask;
int g_mxf_autoreg = 2;
StringW g_mxf_optional;
StringW g_mxf_author;
int g_mxf_TLS_size = 0;
int g_mxf_IMG_size = 0;
bool g_mxf_ncr = false;

void ClearMxfOpts()
  {
    g_mxf_pkg_password = "";
    g_mxf_ext_mask = "";
    g_mxf_ext_password = "";
    g_mxf_target = "";
    g_mxf_source = "";
    g_mxf_is_executable = true;
    g_mxf_do_logging = false;
    g_mxf_use_vreg = true;
    g_mxf_build_vreg = false;
    g_mxf_relink_exec = true;
    g_mxf_use_anticrack = false;
    g_mxf_compress = false;
    g_mxf_encrypt = false;
    g_mxf_autobundle = false;
    g_mxf_changed = false;
    g_mxf_optscrc = MxfCalcuateOptsCRC();
    g_mxf_cmdline = "";
    g_mxf_use_cmdline = false;
    g_mxf_regfile = "";
    g_mxf_use_regfile = false;
    g_mxf_environ = "";
    g_mxf_use_environ = false;
    g_mxf_do_saveiat = false;
    g_mxf_do_saversc = false;
    g_mxf_hideall = false;
    g_mxf_use_activator = false;
    g_mxf_activator = "";
    g_mxf_conflict = 0;
    g_mxf_inject = false;
    g_mxf_extradata = false;
    g_mxf_fullreg = false;
    g_mxf_use_regmask = false;
    g_mxf_regmask = "*.reg";
    g_mxf_autoreg = 2;
    g_mxf_author = "";
    g_mxf_optional = "";
    g_mxf_TLS_size = 0;
    g_mxf_IMG_size = 0;
    g_mxf_ncr = false;
  }

extern wchar_t startdir[];

StringW MxfGetFullSource()
  {
    if ( IsFullPath(g_mxf_source) )
      return g_mxf_source;
    if ( StringW cfg_fold = MxfGetCfgFolder() )
      return JoinPath(cfg_fold,g_mxf_source);
    return JoinPath(startdir,g_mxf_source);
  }
  
StringW MxfGetFullTarget()
  {
    if ( IsFullPath(g_mxf_target) )
      return g_mxf_target;
    if ( StringW cfg_fold = MxfGetCfgFolder() )
      return JoinPath(cfg_fold,g_mxf_target);
    return AtSameFolder(MxfGetFullSource(),g_mxf_target);
  }
  
StringW MxfGetRootFolder()
  {
    if ( StringW cfg_fold = MxfGetCfgFolder() )
      return cfg_fold;
    return GetDirectoryOfPath(MxfGetFullSource());
  }

StringW MxfGetCfgFolder()
  {
    //__asm int 3;
    if ( g_current_mxf_file_name )
      return GetDirectoryOfPath(g_current_mxf_file_name);
    return StringW();
  }

MxfFolderPtr MxfEnsureSubfolder(StringParam folder, EhFilter ehf)
  {
    StringW f = folder;   
    f.Replace(L"\\",L'/');
    MxfFolderPtr p = g_root_mxfFolder;
    
    while ( f )
      {
        StringW left =  f.TillAndTrim(L'/',true);
        if ( MxfFolderPtr x = p->Find(left) )
          p = x;
        else
          {
            x = RccPtr(new MxfFolder(+p,left));
            p->Push(x);
            p = x;
          }
      }      
    return p;
  }
  
bool MxfNewSubfolder(StringParam folder, EhFilter ehf)
  {
    //__asm int 3;
    MxfEnsureSubfolder(folder,ehf);
    return true;
  }

bool MxfAddFileIntoFolder(StringParam folder, StringParam name, StringParam path, EhFilter ehf)
  {
    MxfFolderPtr p = MxfEnsureSubfolder(folder,ehf);
    MxfFilePtr f = RccPtr( new MxfFile(+p,name,path,true) );
    p->Push(f);
    return true;
  }

u32_t MxfCalcuateOptsCRC()
  {
    u32_t bits = 0;
    bits = (bits<<1)|(g_mxf_is_executable?1:0);
    bits = (bits<<1)|(g_mxf_do_logging?1:0);
    bits = (bits<<1)|(g_mxf_use_vreg?1:0);
    bits = (bits<<1)|(g_mxf_build_vreg?1:0);
    bits = (bits<<1)|(g_mxf_relink_exec?1:0);
    bits = (bits<<1)|(g_mxf_use_anticrack?1:0);
    bits = (bits<<1)|(g_mxf_compress?1:0);
    bits = (bits<<1)|(g_mxf_encrypt?1:0);
    bits = (bits<<1)|(g_mxf_autobundle?1:0);
    bits = (bits<<1)|(g_mxf_autoreg?1:0);
    bits = (bits<<1)|(g_mxf_autoreg>1?1:0);
    u32_t crc = Crc32(0,&bits,4);
    crc = Crc32(crc,+g_mxf_pkg_password,g_mxf_pkg_password.Length());
    crc = Crc32(crc,+g_mxf_ext_mask,g_mxf_ext_mask.Length());
    crc = Crc32(crc,+g_mxf_ext_password,g_mxf_ext_password.Length());
    crc = Crc32(crc,+g_mxf_target,g_mxf_target.Length());
    crc = Crc32(crc,+g_mxf_source,g_mxf_source.Length());
    crc = Crc32(crc,+g_mxf_regmask,g_mxf_regmask.Length());
    crc = Crc32(crc,+g_mxf_activator,g_mxf_activator.Length());
    crc = Crc32(crc,+g_mxf_cmdline,g_mxf_cmdline.Length());
    return crc;
  }
