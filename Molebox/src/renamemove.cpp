
/*

  Copyright (c) 2008-2009, Alexey Sudachen, alexey@sudachen.name

*/

#include "myafx.h"
#include "mxf.h"
#include "wx/sizer.h"
#include "wx/busyinfo.h"
#include "wx/msgdlg.h"
#include "wx/statbox.h"
#include "wx/statline.h"
#include "wx/choice.h"
#include "wx/listctrl.h"

struct RenameMoveDlg : wxDialog
  {
    StringW &newfold_;
    StringW &name_;
    wxChoice *parfold_;
    wxTextCtrl *namectrl_;
    BufferT<MxfFolder*> objs_;
    bool operate_folder_;
    bool enable_rename_;
    
    RenameMoveDlg(wxWindow *parent, bool operate_folder, bool enable_rename, StringW &newfold, StringW &name)
    : wxDialog(parent,wxID_ANY,
        (operate_folder?Tr(154,"Move/Rename Folder"):Tr(155,"Move/Rename File")),
        wxDefaultPosition,wxDefaultSize,wxCAPTION),
      newfold_(newfold),
      name_(name),
      operate_folder_(operate_folder),
      enable_rename_(enable_rename)
      {
        namectrl_ = 0;
        wxBoxSizer *topszr = new wxBoxSizer(wxVERTICAL);
        topszr->Add(MakeSubControls(this), 1, wxGROW|wxALL, 10);
        wxSizer *buttons = CreateButtonSizer(wxOK|wxCANCEL);
        topszr->Add( buttons, 0, wxALIGN_CENTER_HORIZONTAL|wxBOTTOM, 10 );
        SetSizer(topszr);
        topszr->Fit(this);

        wxSize sz  = GetSize();
        wxSize psz = GetParent()->GetSize();
        wxPoint xy = GetParent()->GetPosition();
        SetPosition(wxPoint(xy.x+(psz.x-sz.x)/2,xy.y+(psz.y-sz.y)/2));
      }

    void MxfIterate(wxArrayString &strs, BufferT<MxfFolder*> &objs, int &id, MxfFolderPtr &r )
      {
        int i = strs.Count();
        if ( operate_folder_ )
          { if ( +r == g_curr_mxfFolder->GetParent() ) id = i; }
        else
          { if ( r == g_curr_mxfFolder ) id = i; }
        StringW path = r->GetPkgPath();
        if ( !path ) path = L"<VFSROOT>";
        strs.Add(+path);
        objs.Push(+r);
        for ( MxfFolderPtr *p = r->Begin(); p!=r->End(); ++p )
          MxfIterate(strs,objs,id,*p);
      };
      
    wxPanel *MakeSubControls(wxWindow *parent)
      {
        wxBoxSizer *topszr = new wxBoxSizer(wxVERTICAL);
        wxPanel *pan = new wxPanel(parent);
        
        int cur_fold_id = 0;
        wxArrayString strings;        
        MxfIterate(strings,objs_,cur_fold_id,g_root_mxfFolder);
        parfold_ = new wxChoice(pan, ctID_INPUTTEXT2, wxDefaultPosition, wxDefaultSize, strings);
        parfold_->SetSelection(cur_fold_id);
        
        if ( operate_folder_ )
          {
            topszr->Add( new wxStaticText(pan,wxID_ANY,
                                          (enable_rename_?Tr(156,"Move/Rename folder:"):Tr(157,"Move folder:")))
                                          , 0, wxBOTTOM|wxTOP , 5 );
            topszr->Add( new wxStaticText(pan,wxID_ANY,+name_,
                                          wxDefaultPosition, wxDefaultSize, wxSTATIC_BORDER),
                        0, wxBOTTOM|wxGROW , 5 );
          }
        else if ( name_ )
          {
            topszr->Add( new wxStaticText(pan,wxID_ANY,
                                          (enable_rename_?Tr(158,"Move/Rename file:"):Tr(159,"Move file:")))
                                          , 0, wxBOTTOM|wxTOP , 5 );
            topszr->Add( new wxStaticText(pan,wxID_ANY,+name_,
                                          wxDefaultPosition, wxDefaultSize, wxSTATIC_BORDER),
                        0, wxBOTTOM|wxGROW , 5 );
          }
          
        if ( enable_rename_ )
          {
            topszr->Add( new wxStaticText(pan,wxID_ANY,Tr(160,"With new name:")), 0, wxBOTTOM|wxTOP , 5 );
            topszr->Add( namectrl_ = new wxTextCtrl(pan,wxID_ANY,+name_,wxDefaultPosition, wxSize(240, -1)), 0, wxBOTTOM , 5 );
          }

        if ( !operate_folder_ && !name_ )
          topszr->Add( new wxStaticText(pan,wxID_ANY,Tr(161,"Move selected files to subfolder:")), 0, wxBOTTOM|wxTOP , 5 );
        else if ( enable_rename_ )        
          topszr->Add( new wxStaticText(pan,wxID_ANY,Tr(162,"To/At subfolder:")), 0, wxBOTTOM|wxTOP , 5 );
        else
          topszr->Add( new wxStaticText(pan,wxID_ANY,Tr(163,"To subfolder:")), 0, wxBOTTOM|wxTOP , 5 );
        topszr->Add( parfold_, 0, wxGROW|wxBOTTOM , 5 );
        pan->SetSizer(topszr);
        return pan;
      }

    void OnOK(wxCommandEvent &)
      {
        MxfFolder *p = objs_[parfold_->GetCurrentSelection()];
        newfold_ = p->GetPkgPath();
        if ( namectrl_ )
          { name_ = namectrl_->GetValue().c_str().AsWChar(); name_.Trim(); }
        else
          name_ = "";
        EndModal(wxID_OK);
      }
      
    DECLARE_EVENT_TABLE()
  };
  
BEGIN_EVENT_TABLE( RenameMoveDlg, wxDialog )
    EVT_BUTTON( wxID_OK, RenameMoveDlg::OnOK )
END_EVENT_TABLE()

bool RenameMove(
  wxWindow *parent, 
  StringW &targ_subfolder, 
  StringW &inout_name, 
  bool operate_folder, bool enable_rename)
  {
    RenameMoveDlg dlg(parent,operate_folder,enable_rename,targ_subfolder,inout_name);
    return dlg.ShowModal() == wxID_OK;
  }
