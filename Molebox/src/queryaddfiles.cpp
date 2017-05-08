
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

struct QueryAddFilesDlg : wxDialog
{
    StringW& ret_;
    wxChoice*   parfold_;
    BufferT<MxfFolder*> objs_;

    QueryAddFilesDlg(wxWindow* parent, ArrayT<StringW> const& names, StringW& ret)
        : wxDialog(parent,wxID_ANY,Tr(150,"Add Files"),wxDefaultPosition,wxDefaultSize,wxCAPTION),
          ret_(ret)
    {

        wxBoxSizer* topszr = new wxBoxSizer(wxVERTICAL);
        topszr->Add(MakeSubControls(this,names), 1, wxGROW|wxALL, 10);
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

    wxPanel* MakeSubControls(wxWindow* parent,ArrayT<StringW> const& names)
    {
        wxBoxSizer* topszr = new wxBoxSizer(wxVERTICAL);
        wxPanel* pan = new wxPanel(parent);

        int cur_fold_id = 0;
        wxArrayString strings;
        MxfIterate(strings,objs_,cur_fold_id,g_root_mxfFolder);

        //subfold_ = new wxTextCtrl(pan, ctID_INPUTTEXT1, wxEmptyString, wxDefaultPosition, wxSize(60*6,-1));
        wxListCtrl* files = new wxListCtrl(pan,wxID_ANY,wxDefaultPosition,wxSize(230,8*10),wxLC_REPORT|wxSUNKEN_BORDER);
        files->InsertColumn(0, Tr(151,"File name"), wxLIST_FORMAT_LEFT, 200);
        for ( int i = 0; i < names.Count(); ++i )
        {
            wxString wxs(+names[i]);
            files->InsertItem(i, wxs, 0);
        }
        parfold_ = new wxChoice(pan, ctID_INPUTTEXT2, wxDefaultPosition, wxDefaultSize, strings);
        parfold_->SetSelection(cur_fold_id);

        topszr->Add( new wxStaticText(pan,wxID_ANY,Tr(152,"The list of files")), 0, wxBOTTOM|wxTOP , 5 );
        topszr->Add( files, 1, wxGROW|wxBOTTOM , 5 );
        topszr->Add( new wxStaticText(pan,wxID_ANY,Tr(153,"The folder where files will be added")), 0, wxBOTTOM , 5 );
        topszr->Add( parfold_, 0, wxGROW|wxBOTTOM , 5 );
        pan->SetSizer(topszr);
        return pan;
    }

    void OnOK(wxCommandEvent&)
    {
        MxfFolder* p = objs_[parfold_->GetCurrentSelection()];
        ret_ = p->GetPkgPath();
        EndModal(wxID_OK);
    }

    DECLARE_EVENT_TABLE()
};

BEGIN_EVENT_TABLE( QueryAddFilesDlg, wxDialog )
    EVT_BUTTON( wxID_OK, QueryAddFilesDlg::OnOK )
END_EVENT_TABLE()

bool QueryAddFiles(wxWindow* parent, ArrayT<StringW> const& names, StringW& subfolder)
{
    QueryAddFilesDlg dlg(parent,names,subfolder);
    return dlg.ShowModal() == wxID_OK;
}
