
/*

  Copyright (c) 2008-2009, Alexey Sudachen, alexey@sudachen.name

*/

#include "myafx.h"
#include "mxf.h"
#include "wx/sizer.h"

struct MyErrorBoxDlg : wxDialog
  {
    wxWindow *MakeSubControls(wxWindow *parent,pwide_t msg)
      {
        wxPanel *p = new wxPanel(parent);
        wxBoxSizer *s = new wxBoxSizer(wxHORIZONTAL);
        //s->Add( new wxTextCtrl(p,wxID_ANY,msg,
        //                               wxDefaultPosition,wxDefaultSize,
        //                               wxTE_LEFT|wxTE_MULTILINE|wxTE_READONLY|wxTE_NO_VSCROLL),
        s->Add( new wxStaticText(p,wxID_ANY,msg,
                                      wxDefaultPosition,wxDefaultSize
                                      ),
              1, wxGROW|wxALL, 5);
        p->SetSizer(s);
        return p;
      }
    
    MyErrorBoxDlg(wxWindow *parent,StringParam title,StringParam message)
      :wxDialog(parent,wxID_ANY,+title,wxDefaultPosition,wxDefaultSize,wxCAPTION)
      {
        wxBoxSizer *topszr = new wxBoxSizer(wxVERTICAL);
        topszr->Add(MakeSubControls(this,+message), 1, wxGROW|wxALL, 10);
        wxSizer *buttons = CreateButtonSizer(wxOK);
        topszr->Add( buttons, 0, wxALIGN_CENTER_HORIZONTAL|wxBOTTOM, 10 );
        SetSizer(topszr);
        topszr->Fit(this);

        wxSize sz  = GetSize();
        wxSize psz = GetParent()->GetSize();
        wxPoint xy = GetParent()->GetPosition();
        SetPosition(wxPoint(xy.x+(psz.x-sz.x)/2,xy.y+(psz.y-sz.y)/2));
     }
  };
  
struct MyMsgBoxDlg : wxDialog
  {
    wxWindow *MakeSubControls(wxWindow *parent,pwide_t msg)
      {
        wxPanel *p = new wxPanel(parent);
        wxBoxSizer *s = new wxBoxSizer(wxHORIZONTAL);
        //s->Add( new wxTextCtrl(p,wxID_ANY,msg,
        //                               wxDefaultPosition,wxDefaultSize,
        //                               wxTE_LEFT|wxTE_MULTILINE|wxTE_READONLY|wxTE_NO_VSCROLL),
        s->Add( new wxStaticText(p,wxID_ANY,msg,
                                      wxDefaultPosition,wxDefaultSize
                                      ),
              1, wxGROW|wxALL, 5);
        p->SetSizer(s);
        return p;
      }
    
    MyMsgBoxDlg(wxWindow *parent,StringParam title,StringParam message)
      :wxDialog(parent,wxID_ANY,+title,wxDefaultPosition,wxDefaultSize,wxCAPTION)
      {
        wxBoxSizer *topszr = new wxBoxSizer(wxVERTICAL);
        topszr->Add(MakeSubControls(this,+message), 1, wxGROW|wxALL, 10);
        wxSizer *buttons = CreateButtonSizer(wxOK);
        topszr->Add( buttons, 0, wxALIGN_CENTER_HORIZONTAL|wxBOTTOM, 10 );
        SetSizer(topszr);
        topszr->Fit(this);

        wxSize sz  = GetSize();
        wxSize psz = GetParent()->GetSize();
        wxPoint xy = GetParent()->GetPosition();
        SetPosition(wxPoint(xy.x+(psz.x-sz.x)/2,xy.y+(psz.y-sz.y)/2));
     }
  };

  
void DisplayError(wxWindow *parent,StringParam message,StringParam title)
  {
    MyErrorBoxDlg dlg(parent,+title,+message);
    dlg.ShowModal();
  }

void DisplayMessage(wxWindow *parent,StringParam message,StringParam title)
  {
    MyMsgBoxDlg dlg(parent,+title,+message);
    dlg.ShowModal();
  }
  
struct AreYouSureDlg : wxDialog
  {
    wxWindow *MakeSubControls(wxWindow *parent,pwide_t msg)
      {
        wxPanel *p = new wxPanel(parent);
        wxBoxSizer *s = new wxBoxSizer(wxHORIZONTAL);
        //s->Add( new wxTextCtrl(p,wxID_ANY,msg,
        //                               wxDefaultPosition,wxDefaultSize,
        //                               wxTE_LEFT|wxTE_MULTILINE|wxTE_READONLY|wxTE_NO_VSCROLL),
        s->Add( new wxStaticText(p,wxID_ANY,msg,
                                      wxDefaultPosition,wxDefaultSize
                                      ),
              1, wxGROW|wxALL, 5);
        p->SetSizer(s);
        return p;
      }
    
    AreYouSureDlg(wxWindow *parent,StringParam title,StringParam message)
      :wxDialog(parent,wxID_ANY,+title,wxDefaultPosition,wxDefaultSize,wxCAPTION)
      {
        wxBoxSizer *topszr = new wxBoxSizer(wxVERTICAL);
        topszr->Add(MakeSubControls(this,+message), 1, wxGROW|wxALL, 10);
        wxSizer *buttons = CreateButtonSizer(wxYES|wxNO);
        topszr->Add( buttons, 0, wxALIGN_CENTER_HORIZONTAL|wxBOTTOM, 10 );
        SetSizer(topszr);
        topszr->Fit(this);

        wxSize sz  = GetSize();
        wxSize psz = GetParent()->GetSize();
        wxPoint xy = GetParent()->GetPosition();
        SetPosition(wxPoint(xy.x+(psz.x-sz.x)/2,xy.y+(psz.y-sz.y)/2));
     }
     
    void OnYes(wxCommandEvent &e)
      {
        EndModal(wxID_YES);
      }
      
    void OnNo(wxCommandEvent &e)
      {
        EndModal(wxID_NO);
      }

    DECLARE_EVENT_TABLE()
  };
  
BEGIN_EVENT_TABLE(AreYouSureDlg, wxDialog)
    EVT_BUTTON(wxID_YES,  AreYouSureDlg::OnYes)
    EVT_BUTTON(wxID_NO,  AreYouSureDlg::OnNo)
END_EVENT_TABLE()
  
bool AreYouSure(wxWindow *parent,StringParam message,StringParam title)
  {
    AreYouSureDlg dlg(parent,+title,+message);
    return ( dlg.ShowModal() == wxID_YES );
  }
