
/*

  Copyright (C) 2008, Alexey Sudachen, DesaNova Ltda.
  http://www.desanova.com

*/

#include "myafx.h"
#include "packager.h"

#define __VFS_PACKER__
#include "../../Core/include/spackstru.h"
#include "XorS.h"

typedef BLOWFISH_Cipher Cipher;

inline unsigned MASK_FILE_OPTS(unsigned a) { return a; }

enum { FT_FILE = 0, FT_DIRECTORY = 0x80000000 };

struct FileBlock
  {
    u16_t packed_size;
    u16_t real_size;
    u16_t flags;
    u16_t crc;
    u32_t offset;

    FileBlock(u32_t o, u16_t a, u16_t b, u16_t c, u16_t crc = 0) :
      packed_size(a), real_size(b), flags(c), crc(crc), offset(o)
      {}
  };

struct FileInfo : Refcounted
  {
    typedef rcc_ptr<FileInfo> Ptr;
    StringW name;
    FileInfo *dir;
    ArrayT<Ptr> ls;
    BufferT<FileBlock> blocks;
    unsigned flags;
    unsigned size;

    u32_t spack_offs;
    byte_t sign[16];
    FileInfo *copy;

    FileInfo() : dir(0), flags(0), size(0), copy(0) {};
    BufferT<FileBlock> &Blocks() { return copy ? copy->Blocks() : blocks; }
  };

typedef FileInfo::Ptr FileInfoPtr;

struct _Packager : RefcountedT<Packager>
  {

    FileInfoPtr root_;
    unsigned package_base_;
    byte_t crypokey_[16];

    DataStreamPtr ds_;
    unsigned stream_ptr_;
    unsigned stream_size_;
    
    virtual bool Start(DataStream *ds, pchar_t pwd, EhFilter ehf);
    virtual bool File(StringParam srcname, StringParam dstname, unsigned flags, EhFilter ehf, unsigned *persent);
    virtual bool Stream(byte_t *data, unsigned count, bool xzip, EhFilter ehf);
    virtual bool Finish(u32_t /*out*/ *offset,EhFilter ehf);
    virtual ~_Packager();

    bool _WriteContent(StringParam name,FileInfoPtr &f, EhFilter ehf);
    FileInfo *_Lookup(StringParam name, EhFilter ehf);
    
    CollectionT<FileInfo*> sources_;
  };

_Packager::~_Packager()
  {
  }

bool _Packager::Start(DataStream *ds, pchar_t pwd, EhFilter ehf)
  {
    ds_ = RccRefe(ds);

    root_ = RccPtr( new FileInfo() );
    root_->flags = FT_DIRECTORY|F_HIDDEN;
    root_->name = "";
    package_base_ = ds_->Tell();
    stream_ptr_ = 0;
    stream_size_ = 0;
    sources_.Clear();
    
    if ( pwd )
      {
        MD5_Hash md5;
        md5.Update(pwd,strlen(pwd));
        md5.Finalize(crypokey_);
      }


    Cipher cipher;
    cipher.SetupEncipher(crypokey_);
    char pkg_first_sign[8] = { 'Q','U','I','C','K','B','O','X' };
    cipher.DoCipher(pkg_first_sign,1);
    ds_->Write(pkg_first_sign,8);
    ds_->Write32le(package_base_);
    
    return true;
  }

bool _Packager::_WriteContent(StringParam fname,FileInfoPtr &f, EhFilter ehf)
  {
    BufferT<byte_t> z;
    BufferT<byte_t> _m(SVFS_FILE_BLOCK_SIZE);
    byte_t *m = &_m[0];
    Cipher cipher;
    cipher.SetupEncipher(crypokey_);

    f->size = 0;
    DataStreamPtr ids = DataSource->Open(fname,STREAM_READ);

    if ( !ids )
      return ehf(_S*"failed to add file '%s', no file" %fname), false;

    if ( f->flags & F_COMPRESSED ) z.Resize(SVFS_FILE_BLOCK_SIZE);

    while (!ids->Eof())
      {
        unsigned wasread = ids->Read(m,SVFS_FILE_BLOCK_SIZE);
        unsigned flags = 0;
        if ( wasread > 0 )
          {
            unsigned count = wasread;
            f->size += wasread;

            unsigned crc = Crc16(0,m,wasread);

            if ( f->flags & F_COMPRESSED )
              {
                int l = zlib_compress_l(m,count,+z,SVFS_FILE_BLOCK_SIZE,9);
                if ( l > 0 && l < count )
                  {
                    flags|= F_COMPRESSED;
                    count = l;
                    _m.Swap(z);
                    m = &_m[0];
                  }
              }

            uint64_t off = ds_->Tell()-package_base_;

            if ( f->flags & F_ENCRYPTED )
              {
                flags|= F_ENCRYPTED;
                count = (count+7) & ~7;
                cipher.DoCipherCBCI(m,count/8,off);
              }

            f->blocks.Append(FileBlock(off,count,wasread,flags,crc));
            ds_->Write(m,count);
          }
      }
    return true;
  }

bool _Packager::Stream(byte_t *data, unsigned count, bool xzip, EhFilter ehf)
  {
    if ( count > 0 )
      {
        unsigned foo = count;
        BufferT<byte_t> compressed(count);
        int l = zlib_compress_l(data,count,+compressed,compressed.Count(),9);
        if ( l > 0 && xzip )
          compressed.Resize(l);
        else
          {
            foo = xzip?-1:-2;
            compressed.Clear();
            compressed.Insert(compressed.End(),data,data+count);
          }
        if ( xzip )
          {
            Cipher cipher;
            cipher.SetupEncipher(crypokey_);
            cipher.DoCipherCBCI(+compressed,compressed.Count()/8);
          }
        compressed.Insert(compressed.Begin(),(byte_t*)&foo,4);
        stream_ptr_ = ds_->Tell();
        stream_size_ = compressed.Count();
        ds_->Write(+compressed,compressed.Count());
      }
    return true;
  }

FileInfo *_Packager::_Lookup(StringParam name, EhFilter ehf)
  {
    ArrayT<StringW> l = SplitPath(GetDirectoryOfPath(name));
    FileInfo *dir = +root_;
    for ( int i = 0; i < l.Count(); ++i )
      {
        if ( dir->flags & FT_DIRECTORY )
          {
            FileInfo *new_dir = 0;
            for ( int j = 0; j < dir->ls.Count(); ++j )
              {
                if ( dir->ls[j]->name == l[i] )
                  new_dir = +dir->ls[j];
              }
            if ( !new_dir )
              {
                FileInfoPtr n = RccPtr(new FileInfo());
                n->flags = FT_DIRECTORY;
                n->name  = l[i];
                n->dir   = dir;
                dir->ls.Append(n);
                new_dir = +n;
              }
            dir = new_dir;
          }
        else
          return ehf(_S*"failed to add file in directory '%s', is not directory" %name), 0;
      }

    return dir;
  }

bool _Packager::File(StringParam srcname, StringParam dstname, unsigned flags, EhFilter ehf, unsigned *persent)
  {
    FileInfoPtr finfo = RccPtr( new FileInfo() );
    StringW fname = dstname;
    StringW diskfname = srcname;

    finfo->flags = MASK_FILE_OPTS(flags);

    if ( !(finfo->dir  = _Lookup(fname,ehf)) ) return false;

    finfo->name = GetBasenameOfPath(fname);

    if ( FileInfo** fi = sources_.Get(+diskfname) )
      {
        finfo->size = (*fi)->size;
        finfo->copy = *fi;
      }
    else
      {
        if ( !_WriteContent(diskfname,finfo,ehf) ) 
          return false;
        sources_.Put(+diskfname,+finfo);
      }
      
    finfo->dir->ls.Append(finfo);
        
    if ( persent )
      {
        unsigned packed = 0;
        for ( int i = 0; i < finfo->Blocks().Count(); ++i ) packed += finfo->Blocks()[i].packed_size;
        if ( finfo->size )
          *persent = packed*100/finfo->size;
        else
          *persent = 100;
      }
    
    return true;
  }

void print_tab(int tab)
  {
    while ( tab-- )
      xout| ".";
  }

void print_tree( FileInfo *finfo, int tab = 0 )
  {
    if ( finfo->flags & FT_DIRECTORY )
      {
        print_tab(tab);
        xoutln| _S*"DIR: %s" % finfo->name;
        for ( int i = 0; i < finfo->ls.Count(); ++i )
          print_tree( +finfo->ls[i], tab+2 );
      }
    else
      {
        print_tab(tab);
        xoutln| _S*"FILE: %s" % finfo->name;
        for ( int i = 0; i < finfo->Blocks().Count(); ++i )
          print_tab(tab+2), xoutln| _S*"+ %d/%d" % finfo->Blocks()[i].packed_size %finfo->Blocks()[i].real_size;
      }
  }

void _GetFullName( FileInfo *finfo, StringW &w)
  {
    if ( finfo->dir->dir ) // not in root folder
      {
        _GetFullName(finfo->dir,w);
        w += L"\\";
      }
    w += finfo->name;
  }

bool _WriteNames( FileInfo *finfo, SPACK_FILE_INFO *sfilinfo, DataStream *xds, EhFilter ehf )
  {
    StringW fullname; _GetFullName(finfo,fullname); fullname.ToUpper();
    sfilinfo->key.crc = Crc32(0,(byte_t*)+fullname,fullname.Length()*2);
    MD5_Hash md5;
    md5.Update(+fullname,fullname.Length()*2);
    md5.Finalize(sfilinfo->key.sign);
    memcpy(finfo->sign,sfilinfo->key.sign,16);
    if ( !(finfo->flags&Packager::F_HIDDEN) || (finfo->flags&FT_DIRECTORY) )
      {
        sfilinfo->name.S = (pwide_t)xds->Tell();
        sfilinfo->name.length = finfo->name.Length();
        xds->Write(+finfo->name,finfo->name.Length()*2+2);
      }
    return true;
  }

bool _WriteFile( FileInfo *finfo, DataStream *xds, EhFilter ehf )
  {
    SPACK_FILE_INFO sfilinfo = {0};
    sfilinfo.flags = finfo->flags;
    sfilinfo.count = finfo->Blocks().Count();
    sfilinfo.size  = finfo->size;
    _WriteNames(finfo,&sfilinfo,xds,ehf);
    finfo->spack_offs = xds->Tell();
    xds->Write(&sfilinfo,sizeof(sfilinfo));
    for ( int i = 0; i < finfo->Blocks().Count(); ++ i )
      //SPACK_BLOCK is equal to FileBlock
      xds->Write(&finfo->Blocks()[i],sizeof(SPACK_FILE_BLOCK));
    return true;
  }

bool _WriteDir( FileInfo *finfo, DataStream *xds, EhFilter ehf )
  {
    for ( int i = 0; i < finfo->ls.Count(); ++i )
      if ( finfo->ls[i]->flags & FT_DIRECTORY )
        { if ( !_WriteDir( +finfo->ls[i], xds, ehf )) return false; }
      else
        { if ( !_WriteFile( +finfo->ls[i], xds, ehf )) return false; }
    SPACK_FILE_INFO sfilinfo = {0};
    sfilinfo.flags = finfo->flags;
    sfilinfo.count = finfo->ls.Count();
    if ( finfo->dir ) // is not root
      _WriteNames(finfo,&sfilinfo,xds,ehf);
    finfo->spack_offs = xds->Tell();
    xds->Write(&sfilinfo,sizeof(sfilinfo));
    for ( int i = 0; i < finfo->ls.Count(); ++i )
      xds->Write32le(finfo->ls[i]->spack_offs);
    xds->Write32le(0xdeadbeaf);
    return true;
  }

bool _UpdateHiddensOnly( FileInfo *finfo )
  {
    bool hiddens_only = true;
    for ( int i = 0; i < finfo->ls.Count(); ++i )
      if ( finfo->ls[i]->flags & FT_DIRECTORY )
        { 
          if ( !_UpdateHiddensOnly( +finfo->ls[i] ) ) 
          { hiddens_only = false; }
        }
      else
        { 
          if ( !(finfo->ls[i]->flags&Packager::F_HIDDEN) ) 
            { hiddens_only = false; }
        }
    if ( hiddens_only )
      {
        finfo->flags |= Packager::F_HIDDEN;
      }
    return hiddens_only;
  }

void _AppendInfos( FileInfo *i, BufferT<FileInfo*> &b)
  {
    if ( i->dir )
      b.Append(i);
    if ( i->flags & FT_DIRECTORY )
      for ( int j = 0; j < i->ls.Count(); ++j )
        _AppendInfos(+i->ls[j],b);
  }

struct FiLess {
  bool operator() ( FileInfo *a, FileInfo *b )
    {
      return memcmp(a->sign,b->sign,16) < 0;
    }
};

u32_t _WriteMap( FileInfo *root, DataStream *xds )
  {
    BufferT<FileInfo*> infos;
    _AppendInfos(+root,infos);
    Qsort(infos.Begin(),infos.End(),FiLess());
    for ( int i = 0; i < infos.Count(); ++i )
      xds->Write32le(infos[i]->spack_offs);
    return infos.Count();
  }

bool _Packager::Finish(u32_t /*out*/ *offset,EhFilter ehf)
  {
    Cipher cipher;
    SPACK_HEADER shdr;
    ArrayDataStream ads;
    BufferT<byte_t> b;

    ads.Write32le(123456789);

    shdr.stream_ptr = stream_ptr_;
    shdr.stream_size = stream_size_;
    
    cipher.SetupEncipher(crypokey_);

    _UpdateHiddensOnly(+root_);
    root_->flags = root_->flags&~Packager::F_HIDDEN;
    if ( !_WriteDir(+root_,&ads,ehf) ) return false;
    shdr.map_roffs = ads.Tell();
    shdr.map_count = _WriteMap(+root_,&ads);

    ads.SwapBuffer(b);
    while ( b.Count() % 8 ) b.Append(0);

    shdr.crc = Crc32(0,+b,b.Count());

    cipher.DoCipherCBCI(+b,b.Count()/8);

    DataStream::offset_t start = ds_->Tell();
    ds_->Write(+b,b.Count());

    shdr.tree_boffs = ds_->Tell()-b.Count(); // + crc
    shdr.tree_size  = b.Count();
    shdr.root_roffs = root_->spack_offs;
    shdr.base_boffs = ds_->Tell()-package_base_;
    memcpy(shdr.signature,_XOr("STELPACK",9,14484317),8);
    cipher.DoCipher(shdr.signature,1);
    cipher.DoCipher(shdr.signature,1);
    cipher.DoCipherCBCI(&shdr,sizeof(shdr)/8);

    *offset = ds_->Tell();
    ds_->Write(&shdr,sizeof(shdr));
    ds_->Flush();

    return true;
  }

rcc_ptr<Packager> Packager::Create()
  {
    return rcc_ptr<Packager>((Packager*) new _Packager());
  }
