/*

  Copyright (c) 2008-2009, Alexey Sudachen, alexey@sudachen.name

*/

#include "wx/treectrl.h"
struct MxfFolder;
typedef rcc_ptr<MxfFolder> MxfFolderPtr;
extern bool g_mxf_changed;

struct MxfFile : Refcounted
{
    MxfFolder* folder_;
    StringW pkgname_;
    StringW diskpath_;
    bool selected_;
    bool latebinding_;

    MxfFolder* GetFolder() { return folder_; }
    StringW const& GetName() { return pkgname_; }
    StringW GetPkgPath();
    StringW GetDiskPath() { return diskpath_; }
    StringW GetDiskFullPath();

    MxfFile( MxfFolder* fold, StringParam pkgname, StringParam diskpath, bool lbind )
        : folder_(fold), pkgname_(+pkgname), diskpath_(+diskpath), selected_(false), latebinding_(lbind)
    {
    }

    void Remove();
};
typedef rcc_ptr<MxfFile> MxfFilePtr;

struct MxfFolder : Refcounted
{

    struct MxfFileLess
    {
        bool operator ()(MxfFilePtr const& a, MxfFilePtr const& b) const
        {
            return StrCompareI(+a->pkgname_,+b->pkgname_) < 0;
        }
    };

    struct MxfFolderLess
    {
        bool operator ()(MxfFolderPtr const& a, MxfFolderPtr const& b) const
        {
            return StrCompareI(+a->name_,+b->name_) < 0;
        }
    };

    MxfFolder* parent_;
    ArrayT<MxfFilePtr> files_;
    ArrayT<MxfFolderPtr> subfolds_;
    StringW name_;
    wxTreeItemId tid_;

    MxfFolderPtr* Begin() { return subfolds_.Begin(); }
    MxfFolderPtr* End()   { return subfolds_.End(); }

    MxfFilePtr File(unsigned i) { return files_[i]; }
    unsigned SubCount() { return subfolds_.Count(); }
    unsigned FilesCount() { return files_.Count(); }
    void SetTid(wxTreeItemId tid) { tid_ = tid; }
    wxTreeItemId GetTid() { return tid_; }
    MxfFolder* GetParent() { return parent_; }
    StringW const& GetName() { return name_; }
    StringW GetPkgPath()
    {
        if ( parent_ )
        {
            StringW buf;
            return parent_->ResolvePkgPath(buf,+name_);
        }
        else
            return L"";
    }
    StringW& ResolvePkgPath(StringW& buf, pwide_t lastpart)
    {
        if ( parent_ )
        {
            parent_->ResolvePkgPath(buf,+name_);
            buf.Append(L'/');
        }
        buf.Append(+lastpart);
        return buf;
    }

    void Push(MxfFolderPtr& fold)
    {
        //subfolds_.Push(fold);
        subfolds_.BinsertOne(fold,MxfFolderLess(),false);
        fold->parent_ = this;
        g_mxf_changed = true;
    }

    void Push(MxfFilePtr& file)
    {
        //files_.Push(file);
        files_.BinsertOne(file,MxfFileLess(),false);
        file->folder_ = this;
        g_mxf_changed = true;
    }

    void Remove(MxfFile* file)
    {
        for ( MxfFilePtr* p = files_.Begin(); p != files_.End(); ++p )
            if ( +*p == file )
            {
                g_mxf_changed = true;
                files_.Erase(p);
                break;
            }
    }

    void Remove()
    {
        tid_ = 0;
        if ( parent_ )
            parent_->Remove(this);
        else
        {
            g_mxf_changed = true;
            files_.Clear();
            subfolds_.Clear();
        }
    }

    void Remove(MxfFolder* fold)
    {
        for ( MxfFolderPtr* p = subfolds_.Begin(); p != subfolds_.End(); ++p )
            if ( +*p == fold )
            {
                g_mxf_changed = true;
                subfolds_.Erase(p); break;
            }
    }

    MxfFolderPtr Find(StringParam name)
    {
        for ( MxfFolderPtr* i = Begin(); i != End(); ++i )
            if ( (*i)->name_ == +name )
                return *i;
        return MxfFolderPtr(0);
    }

    void UnselectAll()
    {
        for ( int i = 0; i < files_.Count(); ++i )
            files_[i]->selected_ = false;
    }

    MxfFolder( MxfFolder* fold, StringParam name )
        : parent_(fold), name_(+name), tid_(0)
    {
    }

    bool WriteFilesRecursive(DataStream* ds, EhFilter ehf);
};

extern MxfFolderPtr g_root_mxfFolder;
extern MxfFolderPtr g_curr_mxfFolder;
extern StringW g_mxf_pkg_password;;
extern StringW g_mxf_ext_mask;
extern StringW g_mxf_ext_password;
extern StringW g_mxf_target;
extern StringW g_mxf_source;
extern bool g_mxf_is_executable;
extern bool g_mxf_do_logging;
extern bool g_mxf_use_vreg;
extern bool g_mxf_build_vreg;
extern bool g_mxf_relink_exec;
extern bool g_mxf_use_anticrack;
extern bool g_mxf_compress;
extern bool g_mxf_encrypt;
extern bool g_mxf_autobundle;
extern bool g_mxf_do_saveiat;
extern bool g_mxf_do_saversc;
extern bool g_mxf_inject;
extern bool g_mxf_extradata;
extern bool g_mxf_fullreg;
extern StringW g_current_mxf_file_name;
extern u32_t g_mxf_optscrc;
extern StringW g_mxf_cmdline;
extern bool g_mxf_use_cmdline;
extern StringW g_mxf_regfile;
extern bool g_mxf_use_regfile;
extern StringW g_mxf_environ;
extern bool g_mxf_use_environ;
extern StringW g_mxf_token;
extern bool g_mxf_hideall;
extern StringW g_mxf_activator;
extern bool g_mxf_use_activator;
extern int g_mxf_conflict;
extern bool g_mxf_use_regmask;
extern StringW g_mxf_regmask;
extern int g_mxf_autoreg;
extern StringW g_mxf_optional;
extern StringW g_mxf_author;
extern int g_mxf_TLS_size;
extern int g_mxf_IMG_size;
extern bool g_mxf_ncr;

bool ReadMxfConfig(StringParam mxfconf, EhFilter ehf);
bool WriteMxfConfig(StringParam mxfconf, EhFilter ehf);
bool WriteMxfConfig(DataStream* ds, EhFilter ehf);
void ClearMxfOpts();
u32_t MxfCalcuateOptsCRC();
StringW MxfGetFullSource();
StringW MxfGetFullTarget();
StringW MxfGetRootFolder();
StringW MxfGetCfgFolder();

bool MxfNewSubfolder(StringParam folder, EhFilter ehf);
bool MxfAddFileIntoFolder(StringParam folder, StringParam name, StringParam path, EhFilter ehf);
MxfFolderPtr MxfEnsureSubfolder(StringParam folder, EhFilter ehf);
