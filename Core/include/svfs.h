
/*

  Copyright (c) 2008-2009, Alexey Sudachen, alexey@sudachen.name

*/

#include "./spackstru.h"

namespace svfs
{

    typedef NEWDES_Cipher Cipher;

	struct Package;

	struct FioProxy: Refcounted
	{
		enum STATUS
		{
		    FIO_SUCCESS  = 0,
		    END_OF_FIO   = 1,
		    FIO_FAILED   = 2,
		    FIO_NOTPE    = 3,
		};

		SPACK_FILE_INFO* p;
		SPACK_FILE_INFO* Get() { return p; }

		void Unlink() { if (p) { p->proxy = 0; p = 0; }}
		FioProxy(SPACK_FILE_INFO* p) : p(p) {}
		virtual ~FioProxy() { Unlink(); }
		pchar_t PackageName();
		u32_t Size() { return p->size; }
		int IsDirectory() { return (p->flags & SVFS_DIRECTORY) ? 1 : 0; }
		int IsHidden() { return (p->flags & SVFS_HIDDEN) ? 1 : 0; }
		int Conflict() { return (p->flags & SVFS_EXTERNAL) ? 1 : (p->flags & SVFS_NOACCESS) ? 2 : 0; }
		int IsRoot() { return IsDirectory() && (p->dir ? 0 : 1); }
		int IsValid() { return p ? 1 : 0; }
		int IsExecutable() { return p->flags & SVFS_EXECUTABLE; }
		pwide_t PublicName() { return p->name.S; }
		Package* PackageObj() { return (Package*)p->pkg; }
		virtual LARGE_INTEGER GetTimes();
		virtual LARGE_INTEGER GetMtime();
		virtual LARGE_INTEGER GetCtime();
		virtual STATUS Read(u32_t offs, void* buff, u32_t count, u32_t* wasread);
		virtual HANDLE QueryMySelf() { return 0; }
		cbyte_t* Sign() { return p->key.sign; }
		pwide_t ComposePath(BaseStringT<wchar_t>& b = (BaseStringT<wchar_t>&)StringT<wchar_t, 128>());
		pwide_t ComposePath1(BaseStringT<wchar_t>& b = (BaseStringT<wchar_t>&)StringT<wchar_t, 128>());
		STATUS ReadImageNtHeaders(IMAGE_NT_HEADERS*);
	};

	typedef rcc_ptr<FioProxy> FioProxyPtr;

	FioProxy* Proxy(SPACK_FILE_INFO*);

	struct Package: Refcounted
	{
		SPACK_FILE_INFO const* Root() { return +root_; }
		FioProxy* Element(pwide_t path) { return Proxy(_Element(path)); }
		FioProxy* ElementS(cbyte_t* sign) { return Proxy(_ElementS(sign)); }
		SPACK_FILE_INFO* _Element(pwide_t path);
		SPACK_FILE_INFO* _ElementS(cbyte_t* sign);

		DataStreamPtr ds_;
		BufferT<byte_t> data_space_;
		SPACK_FILE_INFO* root_;
		SlistT<FioProxyPtr> fios_;
		SPACK_FILE_INFO** map_;
		u32_t map_count_;
		StringA name_;
		u32_t base_;
		u32_t offset_;
        Cipher cipher_;
		Package* next_;
		byte_t pwd_[16];

		u32_t stream_ptr_;
		u32_t stream_size_;

		FILETIME lmodftime_;

		bool Open(DataStream* ds, u32_t offs, cbyte_t* pwdsign, pchar_t name, EhFilter ehf);
		void _Configure(SPACK_FILE_INFO* n, SPACK_FILE_INFO* dir);
		FioProxy* _Proxy(SPACK_FILE_INFO* fio);

		SPACK_FILE_INFO* GetRoot() { return root_; }
		Package* Next() { return next_; }
		void LinkNext(Package* n) { next_ = n; }
		bool GetStream(BufferT<byte_t>&, EhFilter ehf);
		LARGE_INTEGER GetFtime() { return *(LARGE_INTEGER*)&lmodftime_; }
		void DoCipher_(void* p, int count);

		struct StreamInfo
		{
			uint32_t kind;
			uint32_t offset;
			uint32_t size;
		};
		BufferT<StreamInfo> strms_;

		bool GetStreamData(uint32_t no, BufferT<byte_t>&, EhFilter ehf);
		size_t StreamCount() { return strms_.Count(); }
		int    StreamKind(size_t no) { if (no >= strms_.Count()) return 0; else return strms_[no].kind; }

		Package();
		~Package();
	};

	typedef rcc_ptr<Package> PackagePtr;

	struct Catalog: Refcounted
	{
		struct PACKAGE_LIST_RECORD
		{
			u32_t offset;
			pchar_t name;
		};

		struct MOUNT_LIST_RECORD
		{
			u32_t flags;
			pchar_t mask;
			byte_t pwd[16];
		};

		unsigned PkgsCount();
		unsigned MntsCount();
		pchar_t  PkgName(int i);
		pchar_t  MntMask(int i);
		int      PkgMatch(pchar_t mask, int i);
		bool     IsIntrMount(int i);
		cbyte_t* MountPwdsign(int i);

		PackagePtr Mount(int i, pchar_t pwd, EhFilter ehf);
		PackagePtr MountS(int i, cbyte_t* pwdsign, EhFilter ehf);

		DataStreamPtr ds_;
		BufferT<byte_t> data_space_;
		PACKAGE_LIST_RECORD* pkglist_;
		u32_t pkgcount_;
		MOUNT_LIST_RECORD* mntlist_;
		u32_t mntcount_;
		FILETIME lmodftime_;

		Catalog();
		~Catalog();

		bool Open(StringParam, unsigned base, EhFilter ehf);
	};

	typedef rcc_ptr<Catalog> CatalogPtr;

	CatalogPtr OpenCatalog(StringParam fname, unsigned base, EhFilter ehf);
	//void SignPath(pwide_t s, byte_t *k);

	CXX_FAKE_INLINE void Md5S(pchar_t s, byte_t* k)
	{
		MD5_Hash md5;
		md5.Update(s, strlen(s));
		md5.Finalize(k);
	}

	CXX_FAKE_INLINE void SignPath(pwide_t s, byte_t* k)
	{
		StringW a(s); a.ToUpper();

		MD5_Hash md5;
		md5.Update(+a, a.Length() * 2);
		md5.Finalize(k);
	}

	pwide_t _FioProxy_ComposePath(BaseStringT<wchar_t>& b, SPACK_FILE_INFO* p);
}

