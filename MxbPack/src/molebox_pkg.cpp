
/*

  Copyright (C) 2014, Alexey Sudachen, DesaNova Ltda.
  http://www.desanova.cl

*/

#include "myafx.h"
#include "mxbpack.h"

#define __VFS_PACKER__
#include "../Core/Svfs/spackstru.h"
#include "XorS.h"

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
	FileInfo* dir;
	ArrayT<Ptr> ls;
	BufferT<FileBlock> blocks;
	unsigned flags;
	unsigned size;

	u32_t spack_offs;
	byte_t sign[16];
	FileInfo* copy;

	FileInfo() : dir(0), flags(0), size(0), copy(0) {};
	BufferT<FileBlock>& Blocks() { return copy ? copy->Blocks() : blocks; }
};

typedef FileInfo::Ptr FileInfoPtr;

struct MOLEBOX_PKG : RefcountedT<Packager>
{
	FileInfoPtr root_;
	unsigned package_base_;
	byte_t ndeskey_[16];

	DataStreamPtr ds_;
	unsigned stream_ptr_;
	unsigned stream_size_;

	FileInfo* _Lookup(StringParam name, EhFilter ehf);

	CollectionT<FileInfo*> sources_;
	FileInfoPtr current_;
	BufferT<byte_t> acc_;

	StringW error_;

	unsigned opt_encrypt_;
	unsigned opt_compress_;
	unsigned opt_hide_;
	unsigned opt_conflict_;

	MXBPAK_ERROR Error(MXBPAK_ERROR err, const char* errval)
	{
		error_ = errval;
		return err;
	}

	MXBPAK_ERROR Start(DataStream* ds, void* pwd, size_t pwd_len, unsigned opt);
	MXBPAK_ERROR Fill(byte_t* data, u32_t length);
	MXBPAK_ERROR Flush();
	MXBPAK_ERROR Close();
	MXBPAK_ERROR Finish();


	MOLEBOX_PKG();
	virtual ~MOLEBOX_PKG();
};

MOLEBOX_PKG::MOLEBOX_PKG()
{
	acc_.Reserve(SVFS_FILE_BLOCK_SIZE);
}

MOLEBOX_PKG::~MOLEBOX_PKG()
{
}

MXBPAK_ERROR Molebox_Pkg_Start(
    PWCHART_CONST container_name,
    const char* passwd, int passwd_len, unsigned opt,
    MOLEBOX_PKG /* out */ **pkg)
{
	rcc_ptr<MOLEBOX_PKG> mpk = RccPtr(new MOLEBOX_PKG());
	DataStream ds;

	*pkg = 0;
	unsigned rw_opt = opt & _MXBPAK_PKG_OPT_REWRITE_MASK;
	if ( !rw_opt ) rw_opt = MXBPAK_PKG_OPT_REWRITE;

	if (rw_opt == MXBPAK_PKG_OPT_REWRITE || !DataSource->Exists(container_name))
	{
		ds = DataSource->Open(container_name, STREAM_OVERWRITE | STREAM_CREATE_PATH);
		if (!ds) return MXBPAK_ERROR_FILE_ACCESS_DNI;
	}
	else
	{
		ds = DataSource->Open(container_name, STREAM_READWRITE );
		if (!ds) return MXBPAK_ERROR_FILE_DSNT_EXIST;
	}

	int err = mpk->Start(ds, passwd, passwd_len, opt);
	if (!err)
		*pkg = Refe(mpk);
	return err;
}

MXBPAK_ERROR MOLEBOX_PKG::Start(DataStream* ds, void* pwd, size_t pwd_len, unsigned opt)
{
	ds_ = RccRefe(ds);
	ds_->Seek(0,SEEK_END);

	root_ = RccPtr(new FileInfo());
	root_->flags = FT_DIRECTORY | F_HIDDEN;
	root_->name = "";
	package_base_ = ds_->Tell();
	stream_ptr_ = 0;
	stream_size_ = 0;
	sources_.Clear();

	if (pwd)
	{
		unsigned pwd_opt = opt & _MXBPAK_PKG_OPT_PASSWD_MASK;
		if (!pwd_opt) pwd_opt = MXBPAK_PKG_OPT_TEXT_PASSWD;

		if (pwd_opt == MXBPAK_PKG_OPT_TEXT_PASSWD)
		{
			MD5_Hash md5;
			md5.Update(pwd, strlen(pwd));
			md5.Finalize(ndeskey_);
		}
		else if (pwd_opt == MXBPAK_PKG_OPT_BINARY_PASSWD)
		{
			if (pwd_len == 0 || pwd_len > 512)
				return Error(MXBPAK_ERROR_INVALID_ARGUMENT, "bad password len");
			MD5_Hash md5;
			md5.Update(pwd, pwd_len);
			md5.Finalize(ndeskey_);
		}
		else if (pwd_opt == MXBPAK_PKG_OPT_HEXCHR_PASSWD)
		{
			return Error(MXBPAK_ERROR_UNSUPPORTED, "hex encoded password is unsupported yet");
		}
	}

	NEWDES_Cipher cipher;
	cipher.SetupEncipher(ndeskey_);
	char pkg_first_sign[8] = { 'Q', 'U', 'I', 'C', 'K', 'B', 'O', 'X' };
	cipher.DoCipher(pkg_first_sign, 1);
	ds_->Write(pkg_first_sign, 8);
	ds_->Write32le(package_base_);

	opt_encrypt_ = opt & _MXBPAK_PKG_OPT_ENCRYPT_MASK;
	if (! opt_encrypt_) opt_encrypt_ = pwd ? MXBPAK_PKG_OPT_ENCRYPT : MXBPAK_PKG_OPT_DNT_ENCRYPT;
	opt_compress_ = opt & _MXBPAK_PKG_OPT_COMPRESS_MASK;
	if (! opt_compress_) opt_compress_ = MXBPAK_PKG_OPT_COMPRESS;
	opt_hide_ = opt & _MXBPAK_PKG_OPT_HIDE_MASK;
	if (! opt_hide_) opt_hide_ = MXBPAK_PKG_OPT_HIDE;
	opt_conflict_ = opt & _MXBPAK_PKG_OPT_CONFLICT_MASK;
	if (! opt_conflict_) opt_conflict_ = MXBPAK_PKG_OPT_CONFLICT_INTERNAL;

	return true;
}

MXBPAK_ERROR Molebox_Pkg_Write(MOLEBOX_PKG* pkg, void* data, size_t length)
{
	return pkg->Fill(data, length);
}

MXBPAK_ERROR MOLEBOX_PKG::Fill(byte_t* data, u32_t length)
{
	if (!current_) return Error(MXBPAK_ERROR_NO_FILE);

	while (length)
	{
		u32_t to_append = min(SVFS_FILE_BLOCK_SIZE - acc_.Count(), length);
		acc_.Insert(acc_.End(), data, data + to_append);
		length -= to_append;
		data += to_append;
		if (acc_.Count() == SVFS_FILE_BLOCK_SIZE)
		{
			if (int err == Flush())
				return err;
		}
	}

	return MXBPAK_SUCCESS;
}

MXBPAK_ERROR MOLEBOX_PKG::Flush()
{
	if (!current_) return Error(MXBPAK_ERROR_NO_FILE);

	BufferT<byte_t> z;
	byte_t* m = &acc_[0];
	NEWDES_Cipher cipher;
	cipher.SetupEncipher(ndeskey_);

	//f->size = 0;

	if (current_->flags & F_COMPRESSED) z.Resize(SVFS_FILE_BLOCK_SIZE);
	unsigned flags = 0;
	unsigned count = acc_.Count();
	current_->size += count;
	unsigned crc = Crc16(0, m, count);

	if (current_->flags & F_COMPRESSED)
	{
		int l = zlib_compress_l(m, count, +z, SVFS_FILE_BLOCK_SIZE, 9);
		if (l > 0 && l < count)
		{
			flags |= F_COMPRESSED;
			count = l;
			m = &z[0];
		}
	}

	if (current_->flags & F_ENCRYPTED)
	{
		flags |= F_ENCRYPTED;
		count = (count + 7) & ~7;
		cipher.DoCipherCBCI(m, count / 8);
	}

	current_->blocks.Append(FileBlock(ds_->Tell() - package_base_, count, acc_.Count(), flags, crc));
	ds_->Write(m, count);

	acc_.Resize(0);
	return MXBPAK_SUCCESS;
}

MXBPAK_ERROR Molebox_Pkg_Stream(
    MOLEBOX_PKG* pkg, int kind, unsigned opt,
    void* data, size_t length)
{
	pkg->Close();
	return pkg->Stream(data, length, opt);
}

MXBPAK_ERROR MOLEBOX_PKG::Stream(byte_t* data, unsigned count, unsigned opt)
{
	unsigned opt_compress = opt & _MXBPAK_FILE_OPT_COMPRESS_MASK;
	if (!opt_compress)
		opt_compress = (opt_compress_ == MXBPAK_PKG_OPT_COMPRESS)
		               ? MXBPAK_FILE_OPT_COMPRESS : MXBPAK_FILE_OPT_DNT_COMPRESS;

	bool xzip = opt_compress == MXBPAK_FILE_OPT_COMPRESS;

	if (count > 0)
	{
		unsigned foo = count;
		BufferT<byte_t> compressed(count);
		int l = zlib_compress_l(data, count, +compressed, compressed.Count(), 9);
		if (l > 0 && xzip)
			compressed.Resize(l);
		else
		{
			foo = xzip ? -1 : -2;
			compressed.Clear();
			compressed.Insert(compressed.End(), data, data + count);
		}
		if (xzip)
		{
			NEWDES_Cipher cipher;
			cipher.SetupEncipher(ndeskey_);
			cipher.DoCipherCBCI(+compressed, compressed.Count() / 8);
		}
		compressed.Insert(compressed.Begin(), (byte_t*)&foo, 4);
		stream_ptr_ = ds_->Tell();
		stream_size_ = compressed.Count();
		ds_->Write(+compressed, compressed.Count());
	}

	return MXBPAK_SUCCESS;
}

FileInfo* MOLEBOX_PKG::_Lookup(StringParam name)
{
	ArrayT<StringW> l = SplitPath(GetDirectoryOfPath(name));
	FileInfo* dir = +root_;

	for (int i = 0; i < l.Count(); ++i)
	{
		if (dir->flags & FT_DIRECTORY)
		{
			FileInfo* new_dir = 0;
			for (int j = 0; j < dir->ls.Count(); ++j)
			{
				if (dir->ls[j]->name == l[i])
					new_dir = +dir->ls[j];
			}
			if (!new_dir)
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
			return Error(MXBPAK_ERROR_INTERNAL, 0 | _S*"failed to add file in directory '%s', is not directory" % name));
		}

	return dir;
}

MXBPAK_ERROR Molebox_Pkg_File(MOLEBOX_PKG* pkg, PWCHART_CONST internal_name, unsigned opt)
{
	return pkg->File(internal_name,opt);
}

MXBPAK_ERROR MOLEBOX_PKG::File(StringParam internal_name, unsigned opt)
{
	Close();

	FileInfoPtr finfo = RccPtr(new FileInfo());
	StringW fname = internal_name;
	
	finfo->flags = MaskOpts(opt);

	if (!(finfo->dir  = _Lookup(fname))) 
		return Error(MXBPAK_ERROR_FILE_EXISTS,"file already exists in package");

	finfo->name = GetBasenameOfPath(fname);
	current_ = finfo;
}

MXBPAK_ERROR MOLEBOX_PKG::Present(unsigned *persent)
{
	if ( !current_ ) return Error(MXBPAK_ERROR_NO_FILE);
	if ( !persent ) return Error(MXBPAK_ERROR_INVALID_ARGUMENT, "pointer to parsent value should be not NULL");
	unsigned packed = 0;
	for (int i = 0; i < current_->Blocks().Count(); ++i) packed += current_->Blocks()[i].packed_size;
	if (current_->size)
		*persent = packed * 100 / current_->size;
	else
		*persent = 100;
}

MXBPAK_ERROR MOLEBOX_PKG::Close()
{
	if ( current_ )
	{
		Flush();
		current_->dir->ls.Append(current_);
		current_ = RccPtr(0);
	}
}

void print_tab(int tab)
{
	while (tab--)
		xout | ".";
}

void print_tree(FileInfo* finfo, int tab = 0)
{
	if (finfo->flags & FT_DIRECTORY)
	{
		print_tab(tab);
		xoutln | _S*"DIR: %s" % finfo->name;
		for (int i = 0; i < finfo->ls.Count(); ++i)
			print_tree(+finfo->ls[i], tab + 2);
	}
	else
	{
		print_tab(tab);
		xoutln | _S*"FILE: %s" % finfo->name;
		for (int i = 0; i < finfo->Blocks().Count(); ++i)
			print_tab(tab + 2), xoutln | _S*"+ %d/%d" % finfo->Blocks()[i].packed_size % finfo->Blocks()[i].real_size;
	}
}

void _GetFullName(FileInfo* finfo, StringW& w)
{
	if (finfo->dir->dir)   // not in root folder
	{
		_GetFullName(finfo->dir, w);
		w += L"\\";
	}
	w += finfo->name;
}

bool _WriteNames(FileInfo* finfo, SPACK_FILE_INFO* sfilinfo, DataStream* xds, EhFilter ehf)
{
	StringW fullname; _GetFullName(finfo, fullname); fullname.ToUpper();
	sfilinfo->key.crc = Crc32(0, (byte_t*) + fullname, fullname.Length() * 2);
	MD5_Hash md5;
	md5.Update(+fullname, fullname.Length() * 2);
	md5.Finalize(sfilinfo->key.sign);
	memcpy(finfo->sign, sfilinfo->key.sign, 16);
	if (!(finfo->flags & Packager::F_HIDDEN) || (finfo->flags & FT_DIRECTORY))
	{
		sfilinfo->name.S = (pwide_t)xds->Tell();
		sfilinfo->name.length = finfo->name.Length();
		xds->Write(+finfo->name, finfo->name.Length() * 2 + 2);
	}
	return true;
}

bool _WriteFile(FileInfo* finfo, DataStream* xds, EhFilter ehf)
{
	SPACK_FILE_INFO sfilinfo = {0};
	sfilinfo.flags = finfo->flags;
	sfilinfo.count = finfo->Blocks().Count();
	sfilinfo.size  = finfo->size;
	_WriteNames(finfo, &sfilinfo, xds, ehf);
	finfo->spack_offs = xds->Tell();
	xds->Write(&sfilinfo, sizeof(sfilinfo));
	for (int i = 0; i < finfo->Blocks().Count(); ++ i)
		//SPACK_BLOCK is equal to FileBlock
		xds->Write(&finfo->Blocks()[i], sizeof(SPACK_FILE_BLOCK));
	return true;
}

bool _WriteDir(FileInfo* finfo, DataStream* xds, EhFilter ehf)
{
	for (int i = 0; i < finfo->ls.Count(); ++i)
		if (finfo->ls[i]->flags & FT_DIRECTORY)
		{ if (!_WriteDir(+finfo->ls[i], xds, ehf)) return false; }
		else
		{ if (!_WriteFile(+finfo->ls[i], xds, ehf)) return false; }
	SPACK_FILE_INFO sfilinfo = {0};
	sfilinfo.flags = finfo->flags;
	sfilinfo.count = finfo->ls.Count();
	if (finfo->dir)   // is not root
		_WriteNames(finfo, &sfilinfo, xds, ehf);
	finfo->spack_offs = xds->Tell();
	xds->Write(&sfilinfo, sizeof(sfilinfo));
	for (int i = 0; i < finfo->ls.Count(); ++i)
		xds->Write32le(finfo->ls[i]->spack_offs);
	xds->Write32le(0xdeadbeaf);
	return true;
}

bool _UpdateHiddensOnly(FileInfo* finfo)
{
	bool hiddens_only = true;
	for (int i = 0; i < finfo->ls.Count(); ++i)
		if (finfo->ls[i]->flags & FT_DIRECTORY)
		{
			if (!_UpdateHiddensOnly(+finfo->ls[i]))
			{ hiddens_only = false; }
		}
		else
		{
			if (!(finfo->ls[i]->flags & Packager::F_HIDDEN))
			{ hiddens_only = false; }
		}
	if (hiddens_only)
	{
		finfo->flags |= Packager::F_HIDDEN;
	}
	return hiddens_only;
}

void _AppendInfos(FileInfo* i, BufferT<FileInfo*>& b)
{
	if (i->dir)
		b.Append(i);
	if (i->flags & FT_DIRECTORY)
		for (int j = 0; j < i->ls.Count(); ++j)
			_AppendInfos(+i->ls[j], b);
}

struct FiLess
{
	bool operator()(FileInfo* a, FileInfo* b)
	{
		return memcmp(a->sign, b->sign, 16) < 0;
	}
};

u32_t _WriteMap(FileInfo* root, DataStream* xds)
{
	BufferT<FileInfo*> infos;
	_AppendInfos(+root, infos);
	Qsort(infos.Begin(), infos.End(), FiLess());
	for (int i = 0; i < infos.Count(); ++i)
		xds->Write32le(infos[i]->spack_offs);
	return infos.Count();
}

MXBPAK_ERROR int Molebox_Pkg_Finish(
	MOLEBOX_PKG* pkg,
	INT64T /* out */ *started_at, INT64T /* out */ *catalog_at,
	void /* out */ *sha20byte);

MXBPAK_ERROR MOLEBOX_PKG::Finish(u32_t /*out*/ *offset)
{
	//print_tree(+root_);

	NEWDES_Cipher cipher;
	SPACK_HEADER shdr;
	ArrayDataStream ads;
	BufferT<byte_t> b;

	ads.Write32le(123456789);

	shdr.stream_ptr = stream_ptr_;
	shdr.stream_size = stream_size_;

	cipher.SetupEncipher(ndeskey_);

	_UpdateHiddensOnly(+root_);
	root_->flags = root_->flags&~Packager::F_HIDDEN;
	if (!_WriteDir(+root_, &ads, ehf)) return false;
	shdr.map_roffs = ads.Tell();
	shdr.map_count = _WriteMap(+root_, &ads);

	ads.SwapBuffer(b);
	while (b.Count() % 8) b.Append(0);

	shdr.crc = Crc32(0, +b, b.Count());

	cipher.DoCipherCBCI(+b, b.Count() / 8);

	DataStream::offset_t start = ds_->Tell();
	ds_->Write(+b, b.Count());

	shdr.tree_boffs = ds_->Tell() - b.Count(); // + crc
	shdr.tree_size  = b.Count();
	shdr.root_roffs = root_->spack_offs;
	shdr.base_boffs = ds_->Tell() - package_base_;
	memcpy(shdr.signature, _XOr("STELPACK", 9, 14484317), 8);
	cipher.DoCipher(shdr.signature, 1);
	cipher.DoCipher(shdr.signature, 1);
	cipher.DoCipherCBCI(&shdr, sizeof(shdr) / 8);

	*offset = ds_->Tell();
	ds_->Write(&shdr, sizeof(shdr));
	ds_->Flush();

	return true;
}
