
/*

  Copyright (c) 2008-2009, Alexey Sudachen, alexey@sudachen.name

*/

#include "_common.h"

#define _XOrL(a,b,c) L##a

#include "svfs.h"
#include "logger.h"
#include "XorS.h"
#include <libhash/SHA1.h>

extern "C" int fpattern_match(const char* pat, const char* fname);

namespace svfs
{

	FioProxy* Proxy(SPACK_FILE_INFO* fio)
	{
		if (!fio)
			return 0;
		if (fio->proxy)
			return (FioProxy*)fio->proxy;
		if (!fio->pkg)
			return 0;

		FioProxy* p = ((Package*)fio->pkg)->_Proxy(fio);
		fio->proxy = p;
		return p;
	}

	pchar_t FioProxy::PackageName()
	{
		if (Package* pkg = (Package*)p->pkg)
			return +pkg->name_;
		else
			return "<virtual>";
	}

	pwide_t _FioProxy_ComposePath(BaseStringT<wchar_t>& b, SPACK_FILE_INFO* p)
	{
		if (p->dir && p->dir->dir)
		{
			_FioProxy_ComposePath(b, p->dir);
			b.Append(L"\\");
		}
		if (p->name.S)
			b.Append(p->name.S);
		else if (p->dir)
			b.Append(_XOrL("<hidden>", 0, 0));

		return +b;
	}

	pwide_t FioProxy::ComposePath(BaseStringT<wchar_t>& b)
	{
		return _FioProxy_ComposePath(b, p);
	}

	pwide_t FioProxy::ComposePath1(BaseStringT<wchar_t>& b)
	{
		_FioProxy_ComposePath(b, p);
		if (p->dir && (p->flags & SVFS_DIRECTORY))
			b.Append(L"\\");
		return +b;
	}

	unsigned Catalog::PkgsCount()
	{
		return pkgcount_;
	}

	unsigned Catalog::MntsCount()
	{
		return mntcount_;
	}

	pchar_t  Catalog::PkgName(int i)
	{
		if (i > 0 && i <= pkgcount_)
		{
			return pkglist_[i - 1].name;
		}
		return 0;
	}

	pchar_t  Catalog::MntMask(int i)
	{
		if (i > 0 && i <= mntcount_)
		{
			return mntlist_[i - 1].mask;
		}
		return 0;
	}

	bool Catalog::IsIntrMount(int i)
	{
		if (i > 0 && i <= mntcount_)
		{
			return mntlist_[i - 1].flags & 1;
		}
		return 0;
	}

	cbyte_t* Catalog::MountPwdsign(int i)
	{
		if (i > 0 && i <= mntcount_)
		{
			return mntlist_[i - 1].pwd;
		}
		return 0;
	}

	int Catalog::PkgMatch(pchar_t mask, int i)
	{
		++i;
		while (i > 0 && i <= pkgcount_)
		{
			if (fpattern_match(mask, +pkglist_[i - 1].name))
				return i;
			++i;
		}
		return 0;
	}

	Catalog::Catalog()
	{
	}

	Catalog::~Catalog()
	{
	}

    inline bool Catalog_ReadCrcCount(DataStream* ds, u32_t* crc, u32_t* count, Cipher& cipher)
	{
		ArrayDataStream ads;
		BufferT<byte_t> q(8);
		ds->Read(+q, 8);
		cipher.DoCipher(+q, 1);
		ads.SwapBuffer(q);
		ads.Read32le(count);
		ads.Read32le(crc);
		if (!*count || *count > 64 * 1024) return false;
		return true;
	}

	bool Catalog::Open(StringParam fname, unsigned base, EhFilter ehf)
	{
		byte_t catalog_key[16];
		Md5S(_XOr("g46dgsfet567etwh501bhsd-=352",29,462371461), catalog_key);
        Cipher cipher;
		cipher.SetupDecipher(catalog_key);
		HandleDataStream* hds;

		ds_ = DataStreamPtr(hds = new HandleDataStream(fname, STREAM_READ));
		if (ds_->Error())
			return ehf(_S*"failed to open storage file '%s'" % +fname), false;
		::GetFileTime((HANDLE)hds->f_, 0, 0, &lmodftime_);

		u32_t crc, count;
		ds_->Seek(base);
		unsigned cOfss = ds_->Tell();
		if (!Catalog_ReadCrcCount(+ds_, &crc, &count, cipher))
		{
			ds_->Seek(-8, SEEK_END);
			cOfss = ds_->Tell();
			if (!Catalog_ReadCrcCount(+ds_, &crc, &count, cipher))
				return ehf("there is no spack catalog"), false;
			ds_->Seek(-count - 8, SEEK_END);
		}
		else
			ds_->Seek(base - count);

		XDBG | _S*"Catalog offset %08x" % cOfss;

		BufferT<byte_t> b(count);
		ds_->Read(+b, count);

		cipher.SetupDecipher(catalog_key);
        cipher.DoCipherCBCO(+b, b.Count() / 8);

		if (crc != Crc32(0, +b, b.Count()))
			return ehf("crc of catalog data is invalid"), false;

		data_space_.Swap(b);
		pkglist_ = (PACKAGE_LIST_RECORD*)(+data_space_ + ((u32_t*) + data_space_)[0]);
		pkgcount_ = ((u32_t*) + data_space_)[1];
		mntlist_ = (MOUNT_LIST_RECORD*)(+data_space_ + ((u32_t*) + data_space_)[2]);
		mntcount_ = ((u32_t*) + data_space_)[3];

		for (int i = 0; i < pkgcount_; ++i)
			pkglist_[i].name += (u32_t) + data_space_;
		for (int i = 0; i < mntcount_; ++i)
			mntlist_[i].mask += (u32_t) + data_space_;

		return true;
	}

	CatalogPtr OpenCatalog(StringParam fname, unsigned base, EhFilter ehf)
	{
		CatalogPtr ctlg = RccPtr(new Catalog());
		if (ctlg->Open(fname, base, ehf))
			return ctlg;
		else
			return CatalogPtr(0);
	}

	PackagePtr Catalog::Mount(int i, pchar_t pwd, EhFilter ehf)
	{
		byte_t sign[16];
		Md5S(pwd, sign);
		return MountS(i, sign, ehf);
	}

	PackagePtr Catalog::MountS(int i, cbyte_t* pwdsign, EhFilter ehf)
	{
		if (i > 0 && i <= pkgcount_)
		{
			PackagePtr pkg = RccPtr(new Package());
			if (pkg->Open(+ds_, pkglist_[i - 1].offset, pwdsign, pkglist_[i - 1].name, ehf))
			{
				pkg->lmodftime_ = lmodftime_;
				return pkg;
			}
		}
		return PackagePtr(0);
	}

	void Package::_Configure(SPACK_FILE_INFO* n, SPACK_FILE_INFO* dir)
	{
		n->dir   = dir;
		n->pkg   = this;
		n->proxy = 0;

		if (n->name.S && (!(n->flags & SVFS_HIDDEN) || (n->flags & SVFS_DIRECTORY)))
			n->name.S = (wchar_t*)(+data_space_ + (int)n->name.S);
		else
			memset(&n->name, 0, sizeof(n->name));

		if (n->flags & SVFS_DIRECTORY)
			for (int i = 0; i < n->count; ++i)
			{
				n->ls[i] = (SPACK_FILE_INFO*)(+data_space_ + (int)n->ls[i]);
				_Configure(n->ls[i], n);
			}
		else
			;
	}


	bool Package::Open(DataStream* ds, u32_t offset, cbyte_t* pwdsign, pchar_t name, EhFilter ehf)
	{
		int fixup = 0;
		byte_t sign[8] = {0};
		SPACK_HEADER hdr;
        char b8[8];

        name_ = name;

        memcpy(b8,_XOr("STELPACK",9,464403114),8);
        cipher_.SetupEncipher(pwdsign);
        cipher_.DoCipher(b8,1);
        cipher_.DoCipher(b8,1);
        cipher_.SetupDecipher(pwdsign);
        memcpy(pwd_,pwdsign,16);

        int passno = 0;

		ds->Seek(offset);
		ds->Skip(4);
		ds->Read(sign, 8);
		cipher_.DoCipher(sign, 1);

        if ( 0 == memcmp(sign, _XOr("QUICKBOX",9,465058520), 8) )
		{
			ds->Read(&fixup, 4);
			if (fixup >= 4) fixup -= 4;
			fixup += offset;
		}

	repeat:

		memset(&hdr, 0, sizeof(hdr));

		ds->Seek(offset);
		ds->Read(&hdr, sizeof(hdr));
		cipher_.DoCipher(hdr.signature, 1);

		ds->Seek(offset);
		ds->Read(&hdr, sizeof(hdr));
        cipher_.DoCipherCBCO(&hdr, sizeof(hdr) / 8);


        if ( memcmp(hdr.signature,b8,8)  )
          if ( passno < 1 )
            {
              ds->Seek(offset);
              ds->Read(&offset,4);
              offset += fixup;
              ++passno;
              goto repeat;
            }
          else
            return ehf("bad password"), false;

        BufferT<byte_t> b(hdr.tree_size);
		ds->Seek(offset - b.Count());
		ds->Read(+b, b.Count());
        cipher_.DoCipherCBCO(+b, b.Count() / 8);

		if (hdr.crc != Crc32(0, +b, b.Count()))
			return ehf("failed do CRC integrity check"), false;

		data_space_.Swap(b);
		root_ = (SPACK_FILE_INFO*)(+data_space_ + hdr.root_roffs);

		stream_ptr_ = hdr.stream_ptr;
		stream_size_ = hdr.stream_size;

		//__asm int 3
		;

		_Configure(root_, 0);

		map_ = (SPACK_FILE_INFO**)(+data_space_ + hdr.map_roffs);
		map_count_ = hdr.map_count;

		for (int i = 0; i < map_count_; ++i)
			map_[i] = (SPACK_FILE_INFO*)(+data_space_ + (int)map_[i]);

		ds_ = RccRefe(ds);
		base_ = offset - hdr.base_boffs;
		offset_ = offset;

		if ( stream_ptr_ != 0 && stream_size_ == 0 )
		{
			byte_t b[8];
			ds_->Seek(stream_ptr_+base_);
			ds_->Read(b,8);
			if ( memcmp(b,"STRM",4) != 0 || *(uint32_t*)(b+4) > 999 )
				;
			else
			{
				byte_t sha1[20],sha1a[20];
				ds_->Read(sha1,20);
				strms_.Resize(*(uint32_t*)(b+4));
				ds_->Read(&strms_[0],strms_.Count()*sizeof(strms_[0]));
				SHA1_CONTEXT ctx;
				SHA1_Start(&ctx);
				SHA1_Update(&ctx,&strms_[0],strms_.Count()*sizeof(strms_[0]));
				SHA1_Finish(&ctx,sha1a);
				if ( memcmp(sha1a,sha1,20) != 0 )
				{
					xlog | _S* _XOr("streams catatalog corrupted",28,464861903);
					strms_.Clear();
				}
			}
		}
		return true;
	}

	Package::Package() : next_(0) {}
	Package::~Package() {}

	SPACK_FILE_INFO** sign_search(SPACK_FILE_INFO** iS, u32_t len, cbyte_t* sign)
	{
		u32_t half;
		SPACK_FILE_INFO** middle;

		while (len > 0)
		{
			half = len >> 1;
			middle = iS + half;
			if (memcmp((*middle)->key.sign, sign, 16) < 0)
			{
				iS = middle;
				++iS;
				len = len - half - 1;
			}
			else
				len = half;
		}
		return iS;
	}

	SPACK_FILE_INFO* Package::_ElementS(cbyte_t* sign)
	{
		if (!((u32_t*)sign)[0] && !(((u32_t*)sign)[1] | ((u32_t*)sign)[2] | ((u32_t*)sign)[3]))
			return +root_;
		SPACK_FILE_INFO** p = sign_search(map_, map_count_, sign);
		if (p != map_ + map_count_ && !memcmp((*p)->key.sign, sign, 16))
			return *p;
		return 0;
	}

	SPACK_FILE_INFO* Package::_Element(pwide_t path)
	{
		byte_t sign[16];
		SignPath(path, sign);
		return _ElementS(sign);
	}

	FioProxy* Package::_Proxy(SPACK_FILE_INFO* fio)
	{
		FioProxyPtr p = RccPtr(new FioProxy(fio));
		fios_.Append(p);
		return +p;
	}

	byte_t* g_temporal_block = (byte_t*)VirtualAlloc(0, SVFS_FILE_BLOCK_SIZE, MEM_COMMIT, PAGE_READWRITE);
	struct BUFFERED_BLOCK
	{
		SPACK_FILE_INFO* p;
		int no;
		u32_t stamp;
		byte_t buffer[SVFS_FILE_BLOCK_SIZE];
	};

	enum { COUNT_OF_BUFFERS = 16 };
	BUFFERED_BLOCK* g_buffers = (BUFFERED_BLOCK*)VirtualAlloc(0, sizeof(BUFFERED_BLOCK)*COUNT_OF_BUFFERS, MEM_COMMIT,
	                            PAGE_READWRITE);

	BUFFERED_BLOCK* _FindCachedBlock(SPACK_FILE_INFO* p, int block_no, bool* allocated)
	{
		static int stamp = 0;
		BUFFERED_BLOCK* c = 0;
		for (int i = 0; i < COUNT_OF_BUFFERS; ++i)
		{
			if (g_buffers[i].p == p && g_buffers[i].no == block_no)
			{
				g_buffers[i].stamp = ++stamp;
				return g_buffers + i;
			}
			if (!c || c->stamp > g_buffers[i].stamp) c = g_buffers + i;
		}
		c->p = p;
		c->no = block_no;
		c->stamp = ++stamp;
		*allocated = true;
		return c;
	}

	bool _BufferedReadFio(SPACK_FILE_INFO* p, int block_no, byte_t* buf, unsigned offs, unsigned count)
	{
		bool allocated = false;
		BUFFERED_BLOCK* _cached_block = _FindCachedBlock(p, block_no, &allocated);
		byte_t* cached_block = _cached_block->buffer;
		if (allocated)
		{
			Package* pkg = ((Package*)p->pkg);
			DataStream* ds = +pkg->ds_;
			ds->Seek(pkg->base_ + p->block[block_no].offset);

			byte_t* b = cached_block;
			if (p->block[block_no].flags & SVFS_COMPRESSED)
				b = g_temporal_block;

            uint64_t off = p->block[block_no].offset;
			ds->Read(b, p->block[block_no].packed_size);

			if (p->block[block_no].flags & SVFS_ENCRYPTED)
                pkg->cipher_.DoCipherCBCO(b, p->block[block_no].packed_size / 8, off);

			if (p->block[block_no].flags & SVFS_COMPRESSED)
				zlib_decompress(b, p->block[block_no].packed_size, cached_block, p->block[block_no].real_size);

			if (p->block[block_no].crc != Crc16(0, cached_block, p->block[block_no].real_size))
				goto e;
		}
		memcpy(buf, cached_block + offs, count);
		return true;

	e:
		_cached_block->p = 0;
		_cached_block->stamp = 0;
		return false;
	}

	FioProxy::STATUS FioProxy::Read(u32_t offs, void* buff, u32_t count, u32_t* wasread)
	{
		if (offs >= p->size)
			return END_OF_FIO;

		if (count + offs > p->size)
			count = p->size - offs;

		for (int i = 0; i < count;)
		{
			int block_no = (offs + i) / SVFS_FILE_BLOCK_SIZE;
			int bi = (offs + i) % SVFS_FILE_BLOCK_SIZE;
			int j = cxx_min<int>(SVFS_FILE_BLOCK_SIZE - bi, count - i);
			if (!_BufferedReadFio(p, block_no, (byte_t*)buff + i, bi, j))
				return FIO_FAILED;
			*wasread = (i += j);
		}

		return FIO_SUCCESS;
	}

	FioProxy::STATUS FioProxy::ReadImageNtHeaders(IMAGE_NT_HEADERS* h)
	{
		IMAGE_DOS_HEADER dos = {0};

		u32_t wasread = 0;

		if (STATUS st = Read(0, &dos, sizeof(dos), &wasread))
			return st;
		if (wasread < sizeof(dos) || dos.e_magic != IMAGE_DOS_SIGNATURE)
			return FIO_NOTPE;
		if (STATUS st = Read(dos.e_lfanew, h, sizeof(IMAGE_NT_HEADERS), &wasread))
			return st;
		if (wasread < sizeof(IMAGE_NT_HEADERS) || h->Signature != IMAGE_NT_SIGNATURE)
			return FIO_NOTPE;

		return FIO_SUCCESS;
	}

	bool Package::GetStreamData(size_t no, BufferT<byte_t>& data, EhFilter ehf)
	{
		BufferT<byte_t> tmp;
		data.Clear();
		if ( no >= strms_.Count() ) return false;
		ds_->Seek(base_ + strms_[no].offset);
		tmp.Resize(strms_[no].size);
		ds_->Read(+tmp,strms_[no].size);
		if (*(int*) + tmp != -2)
		{
            cipher_.DoCipherCBCO(+tmp + 4, (tmp.Count() - 4) / 8);
		}
		if (*(int*) + tmp > 0)
		{
			unsigned cnt = *(unsigned*) + tmp;
			data.Resize(cnt);
			if (zlib_decompress(+tmp + 4, tmp.Count() - 4, +data, data.Count()) <= 0) return false;
		}
		else
			data.Insert(data.End(), tmp.Begin() + 4, tmp.End());
		return true;
	}

	bool Package::GetStream(BufferT<byte_t>& data, EhFilter ehf)
	{
		data.Clear();
		if ( strms_.Count() ) return true;
		if (stream_ptr_ && stream_size_)
		{
			BufferT<byte_t> tmp;
			tmp.Resize(stream_size_);
			ds_->Seek(stream_ptr_);
			ds_->Read(+tmp, stream_size_);
			if (*(int*) + tmp != -2)
			{
                cipher_.DoCipherCBCO(+tmp + 4, (tmp.Count() - 4) / 8);
			}
			if (*(int*) + tmp > 0)
			{
				unsigned cnt = *(unsigned*) + tmp;
				data.Resize(cnt);
				if (zlib_decompress(+tmp + 4, tmp.Count() - 4, +data, data.Count()) <= 0) return false;
			}
			else
				data.Insert(data.End(), tmp.Begin() + 4, tmp.End());
		}
		return true;
	}

	LARGE_INTEGER FioProxy::GetTimes()
	{
		if (Package* p = PackageObj())
			return p->GetFtime();
		else
		{
			LARGE_INTEGER l; memset(&l, 0, sizeof(l)); return l;
		}
	}

	LARGE_INTEGER FioProxy::GetMtime()
	{
		return GetTimes();
	}

	LARGE_INTEGER FioProxy::GetCtime()
	{
		return GetTimes();
	}
} // namespace
