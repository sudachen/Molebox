
/*

  Copyright (c) 2008-2009, Alexey Sudachen, alexey@sudachen.name

*/

#include "svfs.h"

using svfs::FioProxy;
using svfs::FioProxyPtr;

typedef RefcountedT<ArrayT<FioProxyPtr>> VfsDirlist;
typedef rcc_ptr<VfsDirlist> VfsDirlistPtr;

struct _VFS
{
    static FioProxyPtr GetProxy(pwide_t filename, BaseStringT<wchar_t>* = 0, bool = false);
    static FioProxyPtr GetProxyInternal(BaseStringT<wchar_t> const& path);
    static FioProxyPtr GetProxyInternal_(BaseStringT<wchar_t> const& path);
    static void InitAndLoadInternalCatalog(unsigned search_for_embeddings);
    static void _AfterMountPackage(svfs::PackagePtr& pkg, pchar_t pkgname, pchar_t intern, StringEfA& sefa);
    static void MoundDefaults();
    static VfsDirlistPtr ListDir(FioProxy* fio, pwide_t mask);
    static VfsDirlistPtr ListEmbeddedFiles(pwide_t mask);
    static CRITICAL_SECTION* _GetLock();
    static void SelfExtraData(u32_t offs, u32_t count);
    static HANDLE AppFolderHandle();
    static HANDLE SelfHandle();
    static unsigned Mount(pwide_t msk, pchar_t pwd, bool atAppfold);
    static unsigned MountS(pwide_t mask, cbyte_t* pwdsign, bool atAppfold);
    static unsigned Mount_At(pwide_t source, pchar_t pwd, i64_t offs);
    static unsigned MountS_At(pwide_t, pchar_t pkgname, cbyte_t* pwdsign, i64_t offs);
    static unsigned MountS_At(HANDLE, pchar_t pkgname, cbyte_t* pwdsign, i64_t offs);
    static unsigned MountS_At(DataStreamPtr, pchar_t pkgname, cbyte_t* pwdsign, i64_t offs);
    static StringW containerName;
    static int GetPkgsCount();
    static svfs::Package* GetPackage(int no);
    static void Call_If(int mask, int (*)(int, pwide_t));
};

struct
{
    _VFS* operator ->() const { return 0; }
    operator CRITICAL_SECTION* () const { return (*this)->_GetLock(); }
} const VFS = {};
