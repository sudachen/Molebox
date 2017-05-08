
/*

  Copyright (c) 2008-2009, Alexey Sudachen, alexey@sudachen.name

*/


#include <classes.h>
#include <wx/wx.h>

//extern pwide_t Translate(int i,pwide_t text);
//#define Tr(i,t) Translate(i,L##t)
extern pwide_t MessageValue(int i);
#define Tr(i,t) MessageValue(i)

struct MessageBoxEhFiler : teggo::ErrorFilter
{
    virtual bool Ascii(const char* message, int e) const
    {
        MessageBoxA(0,message,e?"error":"warning",0);
        return !e;
    }
};

extern int _MAJOR_VERSION;
extern int _MINOR_VERSION;
extern int _BUILD_NUMBER;
extern "C" time_t _BUILD_TIME;

enum
{
    tmID_DEFAULT = 1000,
    myID_PKGCONFIGURE = wxID_HIGHEST,
    myID_PKGDOPACK,
    myID_NEWSUBFOLDER,
    myID_ADDFOLDER,
    myID_DELFOLDER,
    myID_ADDFILES,
    myID_DELFILES,
    myID_EDIT,
    myID_SAVEnEXIT,
    myID_SPLITTER1,
    myID_SPLITTER2,
    myID_SPLITTER3,
    myID_SPLITTER4,
    ctID_FOLDERSTREE,
    myID_ACTIVATE,
    ctID_INPUTTEXT1,
    ctID_INPUTTEXT2,
    ctID_INPUTTEXT3,
    ctID_INPUTTEXT4,
    ctID_INPUTTEXT5,
    ctID_INPUTTEXT6,
    ctID_INPUTTEXT7,
    ctID_INPUTTEXT8,
    ctID_INPUTTEXT9,
    ctID_ACTIVATOR,
    ctID_ACTIVATE,
    ctID_WEBACTIVATE,
    ctID_ORDER,
    ctID_BROWSE_TARGET,
    ctID_BROWSE_SOURCE,
    ctID_BROWSE_ACT,
    ctID_CONTENT_ENCRYPT,
    ctID_CONTENT_ZIP,
    ctID_RELINK,
    ctID_ACT,
    ctID_ANTICRACK,
    ctID_LOGGING,
    ctID_EXE_USEREG,
    ctID_CONTENT_BUILDREG,
    ctID_HASCODE,
    ctID_GOTEST,
    ctID_BREAK,
    ctID_EXE_AUTOBUNDLE,
    myID_FOLDRENAME,
    myID_FILERENAME,
    ctID_ENV,
    ctID_ENVIRONMENT,
    ctID_BROWSE_ENV,
    ctID_REG,
    ctID_STATICREG,
    ctID_BROWSE_REG,
    ctID_CMDL,
    ctID_COMMANDLINE,
    ctID_ANY,
    ctID_ACQUIRE_TOKEN,
    ctID_HELP,
    ctID_CONFLICT,
    ctID_EXTEND,
};

void ShowAboutInfo(wxWindow* parent);
void ShowExpiredInfo(wxWindow* parent);
void ShowActivateDialog(wxWindow* parent);
bool ShowConfigDialog(wxWindow* parent);
bool DoPackingProcess(wxWindow* parent);
void DisplayError(wxWindow* parent,StringParam message,StringParam title);
void DisplayMessage(wxWindow* parent,StringParam message,StringParam title);
bool AreYouSure(wxWindow* parent,StringParam message,StringParam title);
bool QueryNewSubfolder(wxWindow* parent, StringW& subfolder);
bool QueryAddFiles(wxWindow* parent, ArrayT<StringW> const& names, StringW& folder);
bool RenameMove(wxWindow* parent, StringW& targ_subfolder, StringW& inout_name, bool operate_folder, bool enable_rename);

void DirList(StringParam rfold, StringParam patt, ArrayT<StringW>& names, u32_t flgs);
enum
{
    DIRLIST_FILESONLY = 1,
    DIRLIST_DIRESONLY = 2,
    DIRLIST_RECURSIVE = 4,
    DIRLIST_FULLPATH  = 8,
};

extern void ShowHelpSection(StringParam name);

