
/*

  Copyright (c) 2008-2009, Alexey Sudachen, alexey@sudachen.name

*/

#include "myafx.h"
#include "packager.h"
#include "teggovbox.h"
#include "stbinfo.h"
#include "XorS.h"

extern "C" int fpattern_match(const char *pat, const char *fname);
typedef Tuple4<u32_t,StringA,byte_t[16],u32_t> Tu4;

int g_print_register = 0;

struct Tu4Less
  {
    bool operator() ( Tu4 const &a, Tu4 const &b ) const
      {
        return a._1 < b._1;
      }
  };

void Md5S(pchar_t s, byte_t *k)
  {
    MD5_Hash md5;
    md5.Update(s,strlen(s));
    md5.Finalize(k);
  }

bool RunChild(int *child, FILE **childin, FILE **childout, EhFilter ehf)
  {
    StringW modulepath = Hinstance->GetModulePath();
    modulepath.Replace(L"/",L'\\');
    if ( SysPipe(0|_S*L"\"%s\" -T"%modulepath,childin,childout,child,false,false,ehf) )
      {
        int errcode,se,foo=0;
        char buf[128];
        fgets(buf,128,*childout);
        se = sscanf(buf,"%d %*d %*d%n",&errcode,&foo);
        if ( !se || se && errcode != 200 )
          { ehf(_S*"failed to start virtial environmet"); return false; }
        fprintf(*childin,"ivbox %x\n",
#ifdef _DEBUG        
          TEGGOVBOX_LOG_ACTIVITY|TEGGOVBOX_LOG_ACTIVITY_DBG|
#endif          
          TEGGOVBOX_VIRTUALIZE_REGISTRY|TEGGOVBOX_VIRTUALIZE_REGISTRY_FULL);
        fflush(*childin);
        fgets(buf,128,*childout);
        se = sscanf(buf,"%d %*d %*d%n",&errcode,&foo);
        if ( !se || se && errcode != 200 )
          { ehf(_S*"failed to configure virtial environmet"); return false; }
        return true;
      }
    else
      return false;
  }

bool ExitChild(int *child, FILE **childin, FILE **childout, EhFilter ehf)
  {
    if ( child && *child )
      {
        int errcode,se,foo=0;
        char buf[128] = {0};
        fprintf(*childin,"exit\n");
        fflush(*childin);
        fgets(buf,128,*childout);
        //se = sscanf(buf,"%d %*d %*d%n",&errcode,&foo);
        //if ( !se || se && errcode != 200 )
        //  { ehf(_S*"finishing teggovbox error: %s" % (buf+foo)); }
      }
    return SysPipeClose(child,childin,childout,ehf);
    //SysPipeKill(child,childin,childout,ehf);
    //return true;
  }

static const char DEFAULT_PASSWOD[] = "password";
  
template < class Tchr >
  StringT<Tchr> NormalizePath(Tchr const *path, int len = -1)
    {
      if ( len < 0 ) len = StrLen(path);
      BufferT<Tchr> ret(path,path+len);
      for ( int i = 0, iE = ret.Count(); i != iE; ++i )
        if ( ret[i] == '/' ) ret[i] = '\\';
      
      Tchr *p = 0;
      while ( !!(p = (Tchr*)ret.Search(L"\\..\\",4,teggo::BufferT<Tchr>::Untransfomed())) )
        {
          Tchr *pp = p-1;
          while ( pp > ret.Begin() && *pp != '\\')
            --pp;
          if ( pp == ret.Begin() ) break;
          ret.Erase(pp,p-pp+3);
        }
        
      return StringT<Tchr>(+ret,ret.Count());
    }
  
StringW NormalizePath(StringParam par) { return NormalizePath(+par,-1); }
                
StringW Strip(StringParam p, wchar_t c = '"')
  {
    StringW S = +p;
    S.Trim();
    if ( S && S[0] == c ) S.Remove(0,1);
    if ( S && S.Last() == c ) S.Resize(S.Length()-1);
    return S;
  }
    
void PrintRegisterPart(pwide_t &S,int tab,pchar_t before)
  {  
    //wprintf(L"%d:%s\n",tab,S);
    while ( *S && *S != '"' ) ++S;
    pwide_t SS = S; ++S;   
    while ( *S && *S != '"' ) ++S;
    ++S;
    StringA n = Strip(StringW(SS,S));
    StringA key;       
    StringA values;
    if ( before )
      { key = before; key += L"\\"; }
    key += n;
    while ( *S ) 
      {
        while ( *S == ' ' ) ++S;
        if ( *S == '(' )    
          { ++S; PrintRegisterPart(S,tab+2,+key); continue; }
        if ( *S == ')' ) 
          {
            if ( values )
              { 
                printf("[%s]",+key);
                printf("%s\n",+values);
              }     
            ++S; return; 
          }
        SS = S;    
        while ( *S && *S != ')' && *S != '(' && *S != '=' ) ++S;
        if ( *S == '=' ) 
          {
            while ( *S && *S != '"' ) ++S;
            ++S;
            while ( *S && *S != ')' && *S != '(' && *S != '"' ) ++S;
            ++S;
          }
          StringA SSS(SS,S); SSS.Trim();
          if (SSS)
            {
              values += "\n";
              values.AppendRange(SS,S);
            } 
         
      }
  }
 
void PrintRegisterFormated(pwide_t S)
  {                     
    while ( *S )
      {
        while ( *S && *S != '(') ++S;
        ++S;
        PrintRegisterPart(S,2,0);
      }
  }
  
unsigned PeBoxPackContent(StringParam mainstorage, unsigned writecatalog, Xdocument xdoc, EhFilter ehf)
  {
    char pipebuf[128];
    int errcode,se,foo=0;
    int child = 0;
    FILE *childin, *childout;
  
    BufferT<byte_t> register_data;
    rcc_ptr<Packager> pkgr = Packager::Create();

    if ( !CreateDirectoriesIfNeed(mainstorage) ) { ehf("failed to create required directories"); return false; }
    unsigned create = writecatalog?0:STREAM_CREATE;
    DataStreamPtr ds = DataStreamPtr(new HandleDataStream(mainstorage,create|STREAM_READWRITE|STREAM_NONINHERITABLE));
    ds->Seek(0,SEEK_END);
    if ( ds->Error() )
      { ehf("failed to access to container file"); return false; }

    //Xdocument xdoc = XopenDefDocument(config);
    if ( !xdoc )
      return ehf("failed to open config file"), false;
    Xnode *r = xdoc->GetRoot();

    ArrayT<Tu4> embedded_pkgs;

    for ( Xiterator i = r->Iterate(L"package"); i.Next(); )
      {
        StringW packname = i->GetContent();
        bool embedded = ToBool(i->GetParam(L"embedded"));
        bool all_is_activex = ToBool(i->GetParam(L"treatallasocx"));
        bool register_onpack  = StrSafeEqualI( i->GetParam(L"autoreg"), L"onpack" );
        
        //Xoln| _S*"processing %s package '%s'" %(embedded?"embedded":"")  %packname; 
        Xoln| _S*"processing package '%s'" %packname; 
        StringA pwd = i->GetParam(L"password");

        if ( !pwd ) pwd = DEFAULT_PASSWOD;

        unsigned pkg_flags = 0;
        if ( ToBool(i->GetParam(L"encrypted")) ) pkg_flags |= Packager::F_ENCRYPTED;
        if ( ToBool(i->GetParam(L"compressed"))) pkg_flags |= Packager::F_COMPRESSED;
        if ( ToBool(i->GetParam(L"hidden")))     pkg_flags |= Packager::F_HIDDEN;

        DataStreamPtr xds;

        if ( embedded )
          {
            xds = ds;
            xds->Seek(0,SEEK_END);
          }
        else
          xds = //DataSource->Open(JoinPath(GetDirectoryOfPath(mainstorage),packname),STREAM_READWRITE|STREAM_CREATE);
                DataStreamPtr(new HandleDataStream(JoinPath(GetDirectoryOfPath(mainstorage),packname)
                              ,STREAM_READWRITE|STREAM_CREATE|STREAM_NONINHERITABLE));

        u32_t dir_ptr_offs = xds->Tell();        
        xds->Write32le(0);

        if ( !pkgr->Start(+xds,+pwd,ehf) ) return false;

        for ( Xiterator j = i->Iterate(L"file"); j.Next(); )
          {
            unsigned flags = pkg_flags;
            if ( pwide_t fc = j->GetParam(L"encrypted"))
              flags = flags&~Packager::F_ENCRYPTED | (ToBool(fc) ? Packager::F_ENCRYPTED : 0);
            if ( pwide_t fc = j->GetParam(L"compressed"))
              flags = flags&~Packager::F_COMPRESSED | (ToBool(fc) ? Packager::F_COMPRESSED : 0);
            if ( pwide_t fc = j->GetParam(L"hidden"))
              flags = flags&~Packager::F_HIDDEN | (ToBool(fc) ? Packager::F_HIDDEN : 0);
            if ( pwide_t fc = j->GetParam(L"executable"))
              flags = flags&~Packager::F_EXECUTABLE | (ToBool(fc) ? Packager::F_EXECUTABLE : 0);
            if ( pwide_t fc = j->GetParam(L"conflict") )
              {
                if ( StrSafeEqualI( fc, L"embedded" ) )
                  flags = flags&~(Packager::F_EXTERNAL|Packager::F_NOACCESS);
                else if ( StrSafeEqualI( fc, L"external") )
                  flags = flags&~(Packager::F_EXTERNAL|Packager::F_NOACCESS) | Packager::F_EXTERNAL;
                else if ( StrSafeEqualI( fc, L"noaccess") )
                  flags = flags&~(Packager::F_EXTERNAL|Packager::F_NOACCESS) | Packager::F_NOACCESS;
              }

            StringW inname  = j->GetContent();
            StringW outname = j->GetParam(L"rename");
            if ( !outname ) outname = inname;
            inname.Replace(L"/",L'\\');
            inname.ReplaceS(L"\\\\",L"\\",1);
            outname.Replace(L"/",L'\\');
            outname.ReplaceS(L"\\\\",L"\\");
            
            unsigned persent = 0;
            inname = NormalizePath(inname);
            
            //Xoln|+inname;
            //Xoln|+outname;
            
            pwide_t ax = j->GetParam(L"activex");
            if ( (ax && ToBool(ax)) || (all_is_activex && !ax) )
              {
                SetErrorMode(SEM_NOOPENFILEERRORBOX|SEM_FAILCRITICALERRORS);
                long (__stdcall *DLLregisterServer)(void) = 0;
                if ( HMODULE hm = LoadLibraryExW(+inname,0,DONT_RESOLVE_DLL_REFERENCES) )
                  {
                    if ( !!(*(void**)&DLLregisterServer = GetProcAddress(hm,"DllRegisterServer")) )
                      {
                        flags |= Packager::F_ACTIVEX;
                        FreeLibrary(hm); hm = 0;

                        if ( register_onpack )
                          {
                            if (child)
                              ExitChild(&child, &childin, &childout, ehf);
                            //if ( !child )
                            if ( !RunChild(&child,&childin,&childout,ehf) )
                              return false;
                            bool rstat = true;
                            StringW basename = GetBasenameOfPath(inname);
                            if ( basename.StartsWithI(L"flash") && basename.EndsWithI(L".ocx") )
                              {
                                //
                                // normall registration does not write wnything to regestery else secutiry information!!!
                                // So we use here manual rgs registration with virtual registration for security reason.
                                //
                                Xoln|"    recognized as Flash OCX, using registration hack";
                                fprintf(childin,"regsvr \"%s\"\n",+Utf8String(inname));
                                fflush(childin);
                                fgets(pipebuf,128,childout);
                                goto l_alt;
                              }
                            fprintf(childin,"regsvr \"%s\"\n",+Utf8String(inname));
                            //Xoln | +Utf8String(inname);
                            fflush(childin);
                            pipebuf[0] = 0; foo = 0;
                            fgets(pipebuf,128,childout);
                            errcode = 666;
                            se = sscanf(pipebuf,"%d %*d %*d%n",&errcode,&foo);
                            if ( !se || se && errcode != 200 )
                              {
                                rstat = false;
                                Xoln|"    failed to process standard OCX registration, trying alternative";
                            l_alt:
                                fprintf(childin,"altreg \"%s\"\n",+Utf8String(inname));
                                fflush(childin);
                                pipebuf[0] = 0; foo = 0;
                                fgets(pipebuf,128,childout);
                                errcode = 666;
                                se = sscanf(pipebuf,"%d %*d %*d%n",&errcode,&foo);
                                if ( !se || se && errcode != 200 )
                                  Xoln|_S*"    is not OCX yet or does not registered properly";
                                else
                                  rstat = true;
                              }

                            if ( rstat )
                              Xoln|_S*"    COM server registered successfully";
                          
                            int chars = 0;
                            fprintf(childin,"regprn\n");
                            fflush(childin);
                            pipebuf[0] = 0; foo = 0;
                            fgets(pipebuf,128,childout);
                            se = sscanf(pipebuf,"%d %*d %d%n",&errcode,&chars,&foo);
                            if ( !se || se && errcode != 200 )
                              { ehf("virtual register fetching error"); return false; }

                            BufferT<byte_t> rgs(chars);
                            for ( int i = 0; i != chars; )
                              {
                                int k = fread(+rgs+i,1,chars-i,childout);
                                if ( k > 0 )
                                  i += k;
                                else
                                  { ehf("failed to read from child"); return false; }
                              }

                            ArrayDataStream ads;
                            ads.SwapBuffer(rgs);

                            //__asm int 3;
                            BufferT<wchar_t> text(256,L'\0');
                            GetShortPathNameW(+inname,+text,255);
                            StringW innameS = +text;
                            GetLongPathNameW(+inname,+text,255);
                            inname = +text;
                            StringW innameN;
                            //printf("%S\n",+text);
                            if ( +text[1] == ':' )
                              {
                                wchar_t Q[3] = { text[0],':',0};
                                wchar_t N[MAX_PATH+1] = {0};
                                DWORD foo = MAX_PATH;
                                if ( 0 == WNetGetConnectionW(Q,N,&foo) )
                                  {
                                    innameN = N;
                                    innameN += +text+2;
                                    //printf("innameN: %S\n",+innameN);
                                  }
                              }
                            text.Clear();
                            ads.UtfReadTextEx(text);
                            text.Push(L'\0');
                            StringW S = _S*L"{%%MAINFOLDER%%}\\%s"%outname;
                            S.EscMask();
                            inname.EscMask();
                            innameS.EscMask();
                            innameN.EscMask();
                            //printf("innameN/Esc: %S\n",+innameN);
                            //Xoln|+inname;
                            //Xoln|+innameS;
                            //for ( int i = 0; i < text.Count(); ++i ) text[i] = towlower(text[i]);
                            if ( innameN )
                              text.Replace(+innameN,innameN.Length(),+S,S.Length(),towlower);

                            text.Replace(+inname,inname.Length(),+S,S.Length(),towlower);
                            text.Replace(+innameS,innameS.Length(),+S,S.Length(),towlower);
                              
                            //wprintf(L"%s",+text);
                            rgs.Clear(); // hmm,.... way be not?
                            ads.SwapBuffer(rgs);
                            text.Last() = '\n';
                            ads.UtfWriteText(+text,text.Count()); // any part ended with '\n' 
                            ads.SwapBuffer(rgs);
                            register_data.Insert(register_data.End(),rgs.Begin(),rgs.End());
                            if ( g_print_register )
                              PrintRegisterFormated(+text);
                            //fprintf(childin,"regclr\n");
                            //fflush(childin);
                            //pipebuf[0] = 0; foo = 0;
                            //fgets(pipebuf,128,childout);
                            //se = sscanf(pipebuf,"%d %*d %*d%n",&errcode,&foo);
                            //if ( !se || se && errcode != 200 )
                            //  { ehf(_S*"register cleannig error: %s" % (pipebuf+foo)); return false; }
                          }
                        if (child)
                          ExitChild(&child, &childin, &childout, ehf);
                        if ( hm ) FreeLibrary(hm);
                      }
                  }
              }
            if ( !pkgr->File(inname,outname,flags,ehf,&persent) ) return false;
            if (!ToBool(j->GetParam(L"silent"))) 
              Xoln| _S*"%s%s [%3d%%] %s -> %s" 
                %((flags&Packager::F_HIDDEN)?"H":" ")
                %(
                  (flags&Packager::F_EXTERNAL)?"e":
                    (flags&Packager::F_NOACCESS)?"n":
                      " "
                 )
                %persent 
                %TrimLength(+inname,40) 
                %outname;

          }

        u32_t pkgoffs = 0;

        if ( register_data.Count() )
          if ( !pkgr->Stream(+register_data,register_data.Count(),
                pkg_flags&(Packager::F_COMPRESSED|Packager::F_ENCRYPTED),ehf) ) 
                  return false;
        if ( !pkgr->Finish(&pkgoffs,ehf) ) 
          return false;
          
        u32_t endof_pkg = xds->Tell();
        xds->Seek(dir_ptr_offs);
        xds->Write32le(pkgoffs);
        xds->Seek(endof_pkg);
        
        if ( !embedded ) xds->Close();
        else
          {
            Tu4 t;
            t._0 = pkgoffs;
            t._1 = packname;
            Md5S(+pwd,&t._2[0]);
            embedded_pkgs.Append(t);
          }
      }

    Qsort(embedded_pkgs.Begin(),embedded_pkgs.End(),Tu4Less());

    typedef Tuple4<u32_t,StringA,byte_t[16],u32_t> Mnt4;
    ArrayT<Mnt4> mount_pkgs;

     if ( writecatalog ) for ( Xiterator i = r->Iterate(L"mount"); i.Next(); )
      {
        for ( Xiterator j = i->Iterate(); j.Next(); )
          {
            if ( StrSafeEqual(j->GetName(),L"internal") )
              {
                StringA mask = j->GetContent();
                if ( !mask ) 
                  mask = "*";
                else
                  Xoln| _S*"automount internally by mask '%s'" % mask;

                for ( int i = 0; i < embedded_pkgs.Count(); ++i )
                  if (fpattern_match(+mask,+embedded_pkgs[i]._1))
                    {
                      Mnt4 m;
                      m._0 = 1;
                      m._1 = embedded_pkgs[i]._1;
                      memcpy(m._2,embedded_pkgs[i]._2,16);
                      mount_pkgs.Append(m);
                      Xoln| _S*"package '%s' is automounted internally" %embedded_pkgs[i]._1;
                    }
              }
            else if ( StrSafeEqual(j->GetName(),L"external") )
              {
                pwide_t mask = j->GetContent();
                if ( !mask ) mask = L"*";
                pwide_t pwd = j->GetParam(L"value");
                Xoln| _S*"mount externally by mask '%s'%s%s%s" % mask
                  % ( pwd ? " with password '" : " without password")
                  % ( pwd ? pwd : L"")
                  % ( pwd ? "'" : "");
                Mnt4 m;
                m._0 = 0;
                m._1 = mask;
                if ( pwd ) Md5S(+StringA(pwd),&m._2[0]); else Md5S(DEFAULT_PASSWOD,&m._2[0]);
                mount_pkgs.Append(m);
              }
          }
      }

    if ( ds && writecatalog )
      {
        ArrayDataStream ads;
        ads.Skip(6*4);
        // ptr to packages table
        // count fo packages
        // ptr to mount table
        // count of mounts
        // ptr to register
        // count of register bytes

        for ( int i = 0; i < embedded_pkgs.Count(); ++i )
          {
            embedded_pkgs[i]._3 = ads.Tell();
            byte_t l = embedded_pkgs[i]._1.Length();
            BufferT<byte_t> b(l+2);
            b[0]  = l;
            memcpy(&b[1],+embedded_pkgs[i]._1,l+1);
            ads.Write(+b,b.Count());
          }

        for ( int i = 0; i < mount_pkgs.Count(); ++i )
          {
            mount_pkgs[i]._3 = ads.Tell();
            byte_t l = mount_pkgs[i]._1.Length();
            BufferT<byte_t> b(l+2);
            b[0]  = l;
            memcpy(&b[1],+mount_pkgs[i]._1,l+1);
            ads.Write(+b,b.Count());
          }

        u32_t ptr_to_package_table = ads.Tell();

        for ( int i = 0; i < embedded_pkgs.Count(); ++i )
          {
            ads.Write32le(embedded_pkgs[i]._0); // offset in file
            ads.Write32le(embedded_pkgs[i]._3+1); // ptr to name
          }

        u32_t ptr_to_mount_table = ads.Tell();

        for ( int i = 0; i < mount_pkgs.Count(); ++i )
          {
            ads.Write32le(mount_pkgs[i]._0); // internal/external
            ads.Write32le(mount_pkgs[i]._3+1); // mount mask
            ads.Write(mount_pkgs[i]._2,16);  // password
          }

        ads.Seek(0);
        ads.Write32le(ptr_to_package_table);
        ads.Write32le(embedded_pkgs.Count());
        ads.Write32le(ptr_to_mount_table);
        ads.Write32le(mount_pkgs.Count());

        BufferT<byte_t> b;
        ads.SwapBuffer(b);
        while ( b.Count() % 8 ) b.Append(0);
        u32_t crc = Crc32(0,+b,b.Count());

        byte_t catalog_key[16];
        Md5S(_XOr("g46dgsfet567etwh501bhsd-=352",29,13566795),catalog_key);
        //Md5S("g46dgsfet567etwh501bhsd-=352",catalog_key);
        NEWDES_Cipher cipher;
        cipher.SetupEncipher(catalog_key);
        cipher.DoCipherCBCI(+b,b.Count()/8);
        ds->Write(+b,b.Count());
        
        u32_t catalog = ds->Tell();
        if ( !catalog )
          { ehf("zero catalog offset"); return false; }
        
        ads.Write32le(b.Count());
        ads.Write32le(crc);
        ads.SwapBuffer(b);
        cipher.DoCipher(+b,b.Count()/8);
        ds->Write(+b,b.Count());
           
        // and now, write catalog offs to DOS header =)
        //Xoln|_S*"catalog offset: %d(%08x)" %catalog %catalog;
        
        return catalog;
      }

    if ( ds )
      ds->Close();
      
    return 1;
  }
