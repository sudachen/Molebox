
/*

  Copyright (c) 2008-2009, Alexey Sudachen, alexey@sudachen.name

*/

#include "_common.h"
#include "logger.h"
#include "vfs.h"
#include "XorS.h"
#include "apif.h"

extern "C" int w_fpattern_match(const wchar_t* pat, const wchar_t* fname);
extern "C" int Import_Registry(void* text,char error[256]);

bool exeFilterIsEnabled = true;

StringW _VFS::containerName;

namespace
{
    svfs::Catalog* catalog_ = 0;
    BufferT<svfs::Package*> pkgs_;
    Tlocker l_;
    StringW root_path_;
    StringW rootS_path_;
    StringW exe_basename_dot_;
    StringW exe_basename_dotS_;
    bool defaults_mounted = false;
    rcc_ptr<HandleDataStream> selfname_ds_;
    LARGE_INTEGER selfname_times_;
    byte_t selfname_sign_[16] = {0};
    u32_t self_size_ = 0;
    u32_t self_extra_data_ = 0;
    u32_t self_extra_count_ = 0;
    HANDLE appfolder_handle_ = 0;

    void _NormalizePath(pwide_t path, BaseStringT<wchar_t> /*out*/& normpath)
    {
        UNICODE_STRING us = {0, 0, 0};
        RtlDosPathNameToNtPathName_U(path, &us, 0, 0);
        normpath.Set(us.Buffer);
        //XLOG|_S*"dos2nt: '%s' -> '%s'" %path %normpath;
        RtlFreeUnicodeString(&us);
    }
}

CRITICAL_SECTION* _VFS::_GetLock() { return l_; }

int _VFS::GetPkgsCount() { return pkgs_.Count(); }
svfs::Package* _VFS::GetPackage(int no) { return pkgs_[no]; }

struct FakeFioProxy: FioProxy
{
    SPACK_FILE_INFO data_;
    StringW name_;
    LARGE_INTEGER mtime_;
    LARGE_INTEGER ctime_;

    FakeFioProxy(pwide_t name, u32_t size, u32_t flags) : FioProxy(&data_)
    {
        memset(&data_, 0, sizeof(data_));
        name_ = name;
        data_.flags = flags;
        data_.name.S = +name_;
        data_.name.length = name_.Length();
        data_.size = size;
        ctime_ = mtime_ = selfname_times_;
    }
    FakeFioProxy(WIN32_FIND_DATAW const* wdta) : FioProxy(&data_)
    {
        memset(&data_, 0, sizeof(data_));
        name_ = wdta->cFileName;
        data_.name.S = +name_;
        data_.name.length = name_.Length();
        data_.size = wdta->nFileSizeLow;
        data_.flags = 0;
        if (wdta->dwFileAttributes & FILE_ATTRIBUTE_DIRECTORY)
            data_.flags |= SVFS_DIRECTORY;
        mtime_ = (LARGE_INTEGER&)wdta->ftLastWriteTime;
        ctime_ = (LARGE_INTEGER&)wdta->ftCreationTime;
    }
    virtual ~FakeFioProxy() {}

    virtual LARGE_INTEGER GetTimes()
    {
        return mtime_;
    }

    virtual LARGE_INTEGER GetMtime()
    {
        return GetTimes();
    }

    virtual LARGE_INTEGER GetCtime()
    {
        return ctime_;
    }
};


struct FIOPROXYNAMELESS
{
    bool operator()(FioProxyPtr const& a, FioProxyPtr const& b)
    {
        return StrSafeLessI(a->p->name.S, b->p->name.S);
    }
};

void _VFS::SelfExtraData(u32_t offs, u32_t count)
{
    self_extra_data_ = offs;
    self_extra_count_ = count;
}

VfsDirlistPtr _VFS::ListEmbeddedFiles(pwide_t mask)
{
    StringW _;
    FioProxyPtr dirfio;
    VfsDirlistPtr ls = RccPtr(new VfsDirlist());
    if (FioProxyPtr fp = _VFS::GetProxy(mask, &_, true))
    {
        ls->Push(fp);
        return ls;
    }
    _ = GetDirectoryOfPath(mask);
    if (_)
        dirfio = _VFS::GetProxyInternal(_);
    else
        dirfio = RccRefe(
                     pkgs_.Count()
                     ? svfs::Proxy(pkgs_.Last()->GetRoot())
                     : 0);
    if (dirfio)
    {
        SPACK_FILE_INFO* f = dirfio->Get();
        svfs::Package* pkg = dirfio->PackageObj();
        if (pkg) for (;;)
            {
                if (f && (f->flags & SVFS_DIRECTORY))
                    for (int i = 0; i < f->count; ++i)
                        if (f->ls[i]->name.S  /*&& !(f->ls[i]->flags&SVFS_HIDDEN)*/)
                            if (!mask || w_fpattern_match(mask, f->ls[i]->name.S))
                                ls->BinsertOne(RccRefe(svfs::Proxy(f->ls[i])), FIOPROXYNAMELESS(), false);
                pkg = pkg->Next();
                if (pkg) f = pkg->_ElementS(dirfio->Sign());
                else break;
            }
    }

    return ls;
}

VfsDirlistPtr _VFS::ListDir(FioProxy* fio, pwide_t mask)
{
    //__asm int 3
    ;

    VfsDirlistPtr ls = RccPtr(new VfsDirlist());
    bool wildcarded = false;

    __lockon__(l_)
    if (fio->IsValid() && fio->IsDirectory())
    {
        //XLOG|_S*"find file first, mask: '%s'" %mask;

        SPACK_FILE_INFO* f = fio->Get();
        byte_t masksign[16] = {0};

        if (mask && !mask[0]) mask = 0;
        if (mask)
            for (pwide_t mp = mask; *mp; ++mp)
                switch (*mp)
                case '*': case '?': case '<': case '>':
    { wildcarded = true; break; }
        else
            wildcarded = true;

        if (!wildcarded)
        {
            StringW path;
            fio->ComposePath1(path); // dirictory always has public path
            path += mask;
            path.ToUpper();
            if (FioProxyPtr fp = GetProxyInternal(path))
                ls->Push(fp);
        }
        else
        {
            //__asm int 3
            ;
            svfs::Package* pkg = fio->PackageObj();
            if (pkg) for (;;)
                {
                    if (f && (f->flags & SVFS_DIRECTORY))
                        for (int i = 0; i < f->count; ++i)
                            if (f->ls[i]->name.S  /*&& !(f->ls[i]->flags&SVFS_HIDDEN)*/)
                                if (!mask || w_fpattern_match(mask, f->ls[i]->name.S))
                                    ls->BinsertOne(RccRefe(svfs::Proxy(f->ls[i])), FIOPROXYNAMELESS(), false);
                    pkg = pkg->Next();
                    if (pkg) f = pkg->_ElementS(fio->Sign());
                    else break;
                }
        }

        if (mask || wildcarded)
        {

            StringW fpath = root_path_;
            fpath.Append(fio->ComposePath1());
            int fpath_l = fpath.Length();

            if (mask)
                for (pwide_t mp = mask; *mp; ++mp)
                    switch (*mp)
                    {
                        case '<': fpath.Append(L'*'); break;
                        case '>': fpath.Append(L'?'); break;
                        case '"': fpath.Append(L'.'); break;
                        default:  fpath.Append(*mp);
                    }
            else
                fpath.Append('*');

            WIN32_FIND_DATAW wdta = {0};
            fpath[1] = '\\';
            HANDLE h = FindFirstFileW(+fpath, &wdta);
            //XLOG|_S*"find file first: '%s' (%08x)" %+fpath %h;

            if (h != INVALID_HANDLE_VALUE)
            {
                do
                    if (memcmp(wdta.cFileName, L".", 4) && memcmp(wdta.cFileName, L"..", 6))
                    {
                        FioProxyPtr exfp = RccPtr((FioProxy*)new FakeFioProxy(&wdta));
                        if (FioProxyPtr* fp = ls->Bfind(exfp, FIOPROXYNAMELESS()))
                        {
                            if (!(*fp)->IsHidden() || (*fp)->Conflict()
                                || (exfp->IsDirectory() && (*fp)->IsDirectory()))
                                *fp = exfp;
                        }
                        else
                            ls->BinsertOne(exfp, FIOPROXYNAMELESS(), false);
                    }
                while (FindNextFileW(h, &wdta));
                FindClose(h);
            }

            if (w_fpattern_match(+fpath + fpath_l, L".."))
                ls->Insert(ls->Begin(), RccPtr((FioProxy*)new FakeFioProxy(L"..", 0, SVFS_DIRECTORY)));
            if (w_fpattern_match(+fpath + fpath_l, L"."))
                ls->Insert(ls->Begin(), RccPtr((FioProxy*)new FakeFioProxy(L".", 0, SVFS_DIRECTORY)));
        }
    }
e:
    if (wildcarded)
        for (int i = 0; i < ls->Count();)
            if (ls->At(i)->IsHidden())
                ls->Erase(ls->Begin() + i);
            else
                ++i;

    return ls;
}

extern NEWDES_Cipher g_cipher;
struct SelfFioProxy: FioProxy
{
    SPACK_FILE_INFO data_;
    StringW name_;

    SelfFioProxy() : FioProxy(&data_)
    {
        memset(&data_, 0, sizeof(data_));
        name_ = GetBasenameOfPath(Hinstance->GetMainModulePath());//"self.exe";
        data_.flags = 0;
        data_.name.S = +name_;
        data_.name.length = name_.Length();
        data_.size = self_size_;
    }

    virtual HANDLE QueryMySelf()
    {
        return (HANDLE)selfname_ds_->f_;
    }

    virtual STATUS Read(u32_t offs, void* buff, u32_t count, u32_t* wasread)
    {
        if (offs >= data_.size)
            return END_OF_FIO;
        if (count + offs > data_.size)
            count = p->size - offs;
        selfname_ds_->Seek(offs);
        selfname_ds_->Read(buff, count);

        if (offs + count > self_extra_data_ && offs < self_extra_data_ + self_extra_count_)
        {
            u32_t leftS, countS;
            byte_t fooL[8]; bool applay_L = false;
            byte_t fooR[8]; bool applay_R = false;
            if (offs > self_extra_data_)
            {
                leftS = 8 - ((offs - self_extra_data_) % 8);
                if (leftS)
                {
                    applay_L = true;
                    selfname_ds_->Seek(offs + leftS - 8);
                    selfname_ds_->Read(fooL, 8);
                }
            }
            else
                leftS = self_extra_data_ - offs;

            if (leftS < count)
                if (offs + count >= self_extra_data_ + self_extra_count_)
                    countS = (self_extra_data_ + self_extra_count_) - (offs + leftS);
                else
                {
                    countS = count - leftS;
                    applay_R = countS % 8 != 0;
                    countS &= ~7;
                    if (applay_R)
                    {
                        selfname_ds_->Seek(offs + leftS + countS);
                        selfname_ds_->Read(fooR, 8);
                    }
                }

            if (leftS >= count || countS < 8)
                countS = 0;
            else
                g_cipher.DoCipher((byte_t*)buff + leftS, countS / 8);

            if (applay_L)
            {
                g_cipher.DoCipher(fooL, 1);
                memcpy(buff, fooL + 8 - leftS, cxx_min<unsigned>(leftS, count));
            }
            if (applay_R)
            {
                g_cipher.DoCipher(fooR, 1);
                memcpy((byte_t*)buff + leftS + countS, fooR, cxx_min<unsigned>(8, count - (leftS + countS)));
            }
        }
        *wasread = count;
        return FIO_SUCCESS;
    }

    virtual LARGE_INTEGER GetTimes()
    {
        return selfname_times_;
    }

    virtual ~SelfFioProxy() {}
};

HANDLE _VFS::SelfHandle()
{
    return (HANDLE)selfname_ds_->f_;
}

FioProxyPtr _VFS::GetProxyInternal_(BaseStringT<wchar_t> const& path)
{
    __lockon__(l_)
    if (!path)
        return RccRefe(
                   pkgs_.Count()
                   ? svfs::Proxy(pkgs_.Last()->GetRoot())
                   : 0);
    else
    {
        byte_t sign[16];
        svfs::SignPath(+path, sign);
        if (!memcmp(selfname_sign_, sign, 16) && exeFilterIsEnabled)
            return FioProxyPtr(new SelfFioProxy());
        for (int i = pkgs_.Count(); i--;)
            if (FioProxy* fp = pkgs_[i]->ElementS(sign))
                return RccRefe(fp);
    }

    return FioProxyPtr(0);
}

FioProxyPtr _VFS::GetProxyInternal(BaseStringT<wchar_t> const& path)
{
    FioProxyPtr proxy = _VFS::GetProxyInternal_(path);

    if (!proxy)
        if (path.StartWith(+exe_basename_dot_))
        {
            proxy = _VFS::GetProxyInternal_(
                        StringW(path).
                        Remove(0, exe_basename_dot_.Length()).
                        Insert(0, L"<>."));
        }
        else if (exe_basename_dotS_ && path.StartWith(+exe_basename_dotS_))
        {
            proxy = _VFS::GetProxyInternal_(
                        StringW(path).
                        Remove(0, exe_basename_dotS_.Length()).
                        Insert(0, L"<>."));
        }

    return proxy;
}

FioProxyPtr _VFS::GetProxy(pwide_t filename, BaseStringT<wchar_t>* path, bool atApf)
{
    StringW _path;
    if (!path) path = &_path;

    if (!StrStartWith(filename, L"\\??\\") && !atApf)
        _NormalizePath(filename, *path);
    else
        path->Set(filename);

    path->ToUpper();

    if (!atApf && path->Length() == root_path_.Length() - 1)
        path->Append(L"\\");

    if (path->StartWith(+root_path_))
    {
        path->Remove(0, root_path_.Length()); // + '\'
        atApf = true;
    }
    else if (path->StartWith(+rootS_path_))
    {
        path->Remove(0, rootS_path_.Length()); // + '\'
        atApf = true;
    }

    FioProxyPtr proxy;

    if (atApf)
    {
        while (path->Chop(L'\\')) {}
        proxy = _VFS::GetProxyInternal(*path);
    }

    return proxy;
}

void _VFS::InitAndLoadInternalCatalog(unsigned search_for_embeddings)
{
    //__asm int 3;
    StringW containerN = VFS->containerName;
    StringW containerD = GetDirectoryOfPath(containerN);
    StringW peimageN   = Hinstance->GetMainModulePath();

    StringEfA sefa;
    StringW path;
    BufferT<wchar_t> buffer(1024);
    int foo;

    //XLOG|_S*"embedded catalog base: %d (%08x)"%search_for_embeddings%search_for_embeddings;

    if (search_for_embeddings)
        catalog_ = Refe(+svfs::OpenCatalog(containerN, search_for_embeddings, sefa));
    selfname_ds_ = RccPtr(new HandleDataStream(containerN, STREAM_READ | STREAM_EXECUTABLE));

    //__asm int 3;
    GetShortPathNameW(+containerN, +buffer, 1024);
    exe_basename_dotS_ = GetBasenameOfPath(+buffer);
    if (0 < (foo = exe_basename_dotS_.FindRChar('.')))
        exe_basename_dotS_.Resize(foo + 1);
    GetLongPathNameW(+containerN, +buffer, 1024);
    exe_basename_dot_ = GetBasenameOfPath(+buffer);
    if (0 < (foo = exe_basename_dot_.FindRChar('.')))
        exe_basename_dot_.Resize(foo + 1);
    if (exe_basename_dot_ == exe_basename_dotS_)
        exe_basename_dotS_ = "";
    else
        exe_basename_dotS_.ToUpper();
    exe_basename_dot_.ToUpper();

    GetLongPathNameW(+containerN, +buffer, 1024);
    _NormalizePath(+buffer, path);
    if (!GetLongPathNameW(+containerD, +buffer, 1024))
        wcsncpy(+buffer, +containerD, 1024);
    _NormalizePath(+buffer, root_path_);
    root_path_.ToUpper();
    if (!GetShortPathNameW(+containerD, +buffer, 1024))
        wcsncpy(+buffer, +containerD, 1024);
    _NormalizePath(+buffer, rootS_path_);
    rootS_path_.ToUpper();

    XLOG | +root_path_;
    XLOG | +rootS_path_;

    if (root_path_.Last() != '\\') root_path_.Append('\\');
    if (rootS_path_.Last() != '\\') rootS_path_.Append('\\');
    XLOG | _S* _XOr("<VFS> embedded root '%s'",25,448424618) % root_path_;

    path.Remove(0, root_path_.Length());
    while (path.Chop(L'\\')) {}

    self_size_ = selfname_ds_->Available();
    ::GetFileTime((HANDLE)selfname_ds_->f_, 0, 0, (FILETIME*)&selfname_times_);
    svfs::SignPath(+path, selfname_sign_);

    appfolder_handle_ = CreateFileW(+containerD, GENERIC_READ | FILE_LIST_DIRECTORY,
                                    FILE_SHARE_READ | FILE_SHARE_DELETE | FILE_SHARE_WRITE, 0, OPEN_EXISTING, FILE_FLAG_BACKUP_SEMANTICS, 0);
    if (!appfolder_handle_ || appfolder_handle_ == INVALID_HANDLE_VALUE)
        appfolder_handle_ = CreateFileW(+containerD, GENERIC_READ | FILE_LIST_DIRECTORY, FILE_SHARE_READ | FILE_SHARE_DELETE, 0,
                                        OPEN_EXISTING, FILE_FLAG_BACKUP_SEMANTICS, 0);
    if (!appfolder_handle_ || appfolder_handle_ == INVALID_HANDLE_VALUE)
        appfolder_handle_ = CreateFileW(+containerD, GENERIC_READ | FILE_LIST_DIRECTORY, FILE_SHARE_READ, 0, OPEN_EXISTING,
                                        FILE_FLAG_BACKUP_SEMANTICS, 0);
    if (!appfolder_handle_ || appfolder_handle_ == INVALID_HANDLE_VALUE)
        XLOG | _XOr("<VFS/ERR> failed to open appfolder",35,442067657);

    if (!catalog_ && search_for_embeddings)
        XLOG | _S* _XOr("<VFS> failed to open embedded packages catalog: %s",51,443705874) % +sefa;
}

extern void REGISTER_LoadRegData(byte_t*, unsigned);

void _VFS::_AfterMountPackage(svfs::PackagePtr& pkg, pchar_t pkgname, pchar_t intern, StringEfA& sefa)
{
    if (pkg)
    {
        BufferT<byte_t> register_data;
        if (!pkg->GetStream(register_data, sefa))
        {
            size_t count = pkg->StreamCount();
            for (size_t i = 0; i < count; ++i)
                if (pkg->StreamKind(i) == 1)
                {
                    register_data.Clear();
                    if (pkg->GetStreamData(i, register_data, sefa))
                    {
                        char error[256];
                        if (!Import_Registry(+register_data, error))
                            xlog | _S* _XOr("failed to register regfile from stream %d: %s ",47,440363559) % i % error;
                    }
                }
        }
        else
            REGISTER_LoadRegData(+register_data, register_data.Count());
        if (pkgs_.Count()) pkg->LinkNext(+pkgs_.Last());
        pkgs_.Append(Forget(pkg));
        XLOG | _S* _XOr("<VFS> package '%s' has been mounted %s",39,468216440)
        % pkgname
        % intern;
    }
    else
        XLOG | _S* _XOr("<VFS> failed to mount %s package '%s': %s",42,461464961)
        % intern
        % pkgname
        % +sefa;
}

unsigned _VFS::Mount(pwide_t msk, pchar_t pwd, bool atAppfold)
{
    byte_t sign[16];
    svfs::Md5S((pwd && pwd[0]) ? pwd : _XOr("password",9,464938414), sign);
    return MountS(msk, sign, atAppfold);
}

unsigned _VFS::Mount_At(pwide_t source, pchar_t pwd, i64_t offs)
{
    byte_t sign[16];
    svfs::Md5S((pwd && pwd[0]) ? pwd : _XOr("password",9,457926107), sign);
    return MountS_At(+source, +StringA(GetBasenameOfPath(source)), sign, offs);
}

unsigned _VFS::MountS_At(DataStreamPtr ds, pchar_t pkgname, cbyte_t* pwdsign, i64_t offs)
{
    StringEfA sefa;
    int count = 0;
    svfs::PackagePtr pkg;
    if (ds)
    {
        pkg = RccPtr(new svfs::Package());
        if (!pkg->Open(+ds, offs, pwdsign, pkgname, sefa))
        {
            Unrefe(pkg);
            XLOG | _S* _XOr("%s: %s",7,459236807) % pkgname % +sefa;
        }
    }
    else
        XLOG | _S* _XOr("%s: '%s does not exist'",24,458450419) % pkgname % ds->Identifier();

    if (pkg)
    {
        ++count;
        _AfterMountPackage(pkg, pkgname, _XOr("externally",11,460416273), sefa);
    }

    return count;
}

unsigned _VFS::MountS_At(pwide_t source, pchar_t pkgname, cbyte_t* pwdsign, i64_t offs)
{
    return MountS_At(OpenHandleStream(source, STREAM_READ | STREAM_NONINHERITABLE),
                     pkgname,
                     pwdsign,
                     offs);
}

unsigned _VFS::MountS_At(HANDLE hndl, pchar_t pkgname, cbyte_t* pwdsign, i64_t offs)
{
    return MountS_At(OpenHandleStream(hndl, pkgname, STREAM_READ | STREAM_NONINHERITABLE),
                     pkgname,
                     pwdsign,
                     offs);
}

unsigned _VFS::MountS(pwide_t mask, cbyte_t* pwdsign, bool atAppfold)
{
    StringEfA sefa;
    APIF->AcquireFilterLock();

    unsigned count = 0;
    StringW pkgmask = mask;
    XLOG | _S* _XOr("mounting packages by mask %s ...",33,453010688) % pkgmask;
    if (pkgmask)
    {
        if (atAppfold)
        {
            pkgmask.Insert(0, L"\\");
            pkgmask.Insert(0, +GetDirectoryOfPath(VFS->containerName));
        }
        pkgmask.Replace(L"/", L'\\');
        StringW findAt = GetDirectoryOfPath(pkgmask);
        WIN32_FIND_DATAW fdtw;
        HANDLE hfnd = FindFirstFileW(+pkgmask, &fdtw);
        if (hfnd && hfnd != INVALID_HANDLE_VALUE)
        {
            do if (!(fdtw.dwFileAttributes & FILE_ATTRIBUTE_DIRECTORY))
                {
                    StringA pkgname = fdtw.cFileName;
                    StringW path = (findAt ? (findAt + L"\\") : StringW()) + fdtw.cFileName;
                    count += MountS_At(+path, +pkgname, pwdsign, 0);
                }
            while (FindNextFileW(hfnd, &fdtw));
            FindClose(hfnd);
        }
    }

    APIF->ReleaseFilterLock();
    return count;
}

void _VFS::MoundDefaults()
{
    StringEfA sefa;
    APIF->AcquireFilterLock();

    if (catalog_ && !defaults_mounted)
        for (int i = 1, iE = catalog_->MntsCount(); i <= iE ; ++i)
        {
            if (catalog_->IsIntrMount(i))
            {
                svfs::PackagePtr pkg;
                StringA pkgname = catalog_->MntMask(i);
                pkg = catalog_->MountS(i, catalog_->MountPwdsign(i), sefa);
                if (pkg)
                    _AfterMountPackage(pkg, +pkgname, _XOr("internally",11,457073958), sefa);
            }
            else
                MountS(+StringW(catalog_->MntMask(i)), catalog_->MountPwdsign(i), true);
        }

    APIF->ReleaseFilterLock();
    defaults_mounted = true;
}

HANDLE _VFS::AppFolderHandle()
{
    return appfolder_handle_;
}

void _VFS::Call_If(int mask, int (*do_It)(int, pwide_t))
{
    for (int i = pkgs_.Count(); i--;)
    {
        SPACK_FILE_INFO** map = pkgs_[i]->map_;
        int count = pkgs_[i]->map_count_;
        for (int j = 0; j < count; ++j)
        {
            if (map[j]->flags & mask)
                if (map[j]->name.S)
                {
                    StringW b;
                    if (!do_It(map[i]->flags, svfs::_FioProxy_ComposePath(b, map[j])))
                        goto lb_exit;
                }
        }
    }
lb_exit:
    ;
}

