
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

struct QueryNewSubfolderDlg : wxDialog
{
    StringW& ret_;
    wxTextCtrl* subfold_;
    wxChoice*   parfold_;
    BufferT<MxfFolder*> objs_;

    QueryNewSubfolderDlg(wxWindow* parent,StringW& ret)
        : wxDialog(parent,wxID_ANY,Tr(138,"New Subfolder"),wxDefaultPosition,wxDefaultSize,wxCAPTION),
          ret_(ret)
    {

        wxBoxSizer* topszr = new wxBoxSizer(wxVERTICAL);
        topszr->Add(MakeSubControls(this), 1, wxGROW|wxALL, 10);
        wxSizer* buttons = CreateButtonSizer(wxOK|wxCANCEL);
        topszr->Add( buttons, 0, wxALIGN_CENTER_HORIZONTAL|wxBOTTOM, 10 );
        SetSizer(topszr);
        topszr->Fit(this);

        wxSize sz  = GetSize();
        wxSize psz = GetParent()->GetSize();
        wxPoint xy = GetParent()->GetPosition();
        SetPosition(wxPoint(xy.x+(psz.x-sz.x)/2,xy.y+(psz.y-sz.y)/2));
    }

    void MxfIterate(wxArrayString& strs, BufferT<MxfFolder*>& objs, int& id, MxfFolderPtr& r )
    {
        int i = strs.Count();
        if ( r == g_curr_mxfFolder )
            id = i;
        StringW path = r->GetPkgPath();
        if ( !path ) path = L"<VFSROOT>";
        strs.Add(+path);
        objs.Push(+r);
        for ( MxfFolderPtr* p = r->Begin(); p!=r->End(); ++p )
            MxfIterate(strs,objs,id,*p);
    };

    wxPanel* MakeSubControls(wxWindow* parent)
    {
        wxBoxSizer* topszr = new wxBoxSizer(wxVERTICAL);
        wxPanel* pan = new wxPanel(parent);

        int cur_fold_id = 0;
        wxArrayString strings;
        MxfIterate(strings,objs_,cur_fold_id,g_root_mxfFolder);

        subfold_ = new wxTextCtrl(pan, ctID_INPUTTEXT1, wxEmptyString, wxDefaultPosition, wxSize(60*6,-1));
        parfold_ = new wxChoice(pan, ctID_INPUTTEXT2, wxDefaultPosition, wxDefaultSize, strings);
        parfold_->SetSelection(cur_fold_id);

        topszr->Add( new wxStaticText(pan,wxID_ANY,Tr(139,"Enter name of new Subfolder here")), 0, wxBOTTOM|wxTOP , 5 );
        topszr->Add( subfold_, 1, wxGROW|wxBOTTOM , 5 );
        topszr->Add( new wxStaticText(pan,wxID_ANY,Tr(140,"Select the Parent folder")), 0, wxBOTTOM , 5 );
        topszr->Add( parfold_, 1, wxGROW|wxBOTTOM , 5 );
        pan->SetSizer(topszr);
        return pan;
    }

    void OnOK(wxCommandEvent&)
    {
        ret_ = "";
        MxfFolder* p = objs_[parfold_->GetCurrentSelection()];
        p->ResolvePkgPath(ret_,+StringW(subfold_->GetValue().c_str().AsWChar()).Trim());
        EndModal(wxID_OK);
    }

    DECLARE_EVENT_TABLE()
};

BEGIN_EVENT_TABLE( QueryNewSubfolderDlg, wxDialog )
    EVT_BUTTON( wxID_OK, QueryNewSubfolderDlg::OnOK )
END_EVENT_TABLE()

bool QueryNewSubfolder(wxWindow* parent, StringW& subfolder)
{
    QueryNewSubfolderDlg dlg(parent,subfolder);
    return dlg.ShowModal() == wxID_OK;
}
