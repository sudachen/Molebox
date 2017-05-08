
/*

  Copyright (c) 2008-2009, Alexey Sudachen, alexey@sudachen.name

*/

#include "myafx.h"
#include "wx/aboutdlg.h"
#include "wx/sizer.h"
#include "wx/html/htmlwin.h"

struct AboutDialog : wxDialog
{
    AboutDialog(wxWindow* parent) : wxDialog(parent,wxID_ANY,L"About",wxDefaultPosition,wxDefaultSize,wxCAPTION)
    {
    }

    void CreateControls()
    {
        wxBoxSizer* topszr = new wxBoxSizer(wxVERTICAL);
        wxPanel* pan = new wxPanel(this);
        wxHtmlWindow* html = new wxHtmlWindow(pan,wxID_ANY,wxDefaultPosition,wxSize(520,480),wxHW_SCROLLBAR_NEVER|wxSUNKEN_BORDER);

        static const wchar_t about_html[] =
            L"<HTML><BODY><CENTER>\n"
            L"<br>\n"
            //L"<img src=\"logo.png\" border=0><br>\n"
            L"<br><font size=+1><b>MoleBox Virtualization Solution</font><br>GPL revision</b><br>\n"
            L"<br><font size=-1>follow molebox on github goo.gl/UY1PHP</font><br>\n"
            L"<font size=-1>Version %1</font><br>\n"
            L"</CENTER></BODY></HTML>\n";

        BufferT<wchar_t> b(about_html,about_html+sizeof(about_html));

        static StringW version_tmpl = "%1";
        static StringW version_val  = _S*L"%d.%d" %_MAJOR_VERSION %_BUILD_NUMBER;

        b.Replace(+version_tmpl,version_tmpl.Length(),+version_val,version_val.Length(),towlower);
        html->SetPage(+b);
        html->SetSize( html->GetInternalRepresentation()->GetWidth(),
                       html->GetInternalRepresentation()->GetHeight());
        html->SetBorders(0);
        pan->SetSize(html->GetSize());
        topszr->Add(pan, 0, wxALL, 10);
        wxSizer* buttons = CreateButtonSizer(wxOK);
        //buttons->Add( new wxButton(this,ctID_EXTEND,L"Renew subscription") );
        topszr->Add( buttons, 0, wxALIGN_CENTER_HORIZONTAL|wxBOTTOM, 10 );
        SetSizer(topszr);
        topszr->Fit(this);
        wxSize sz = GetSize();
        wxSize psz = GetParent()->GetSize();
        wxPoint xy = GetParent()->GetPosition();
        SetPosition(wxPoint(xy.x+(psz.x-sz.x)/2,xy.y+(psz.y-sz.y)/2));
        //SetClientSize(topszr->ComputeFittingClientSize(this));
    }

    DECLARE_EVENT_TABLE()
};

BEGIN_EVENT_TABLE( AboutDialog, wxDialog )
    //EVT_BUTTON( wxID_OK, ConfigDialog::OnOK )
    //EVT_BUTTON( ctID_HELP, ActivateDialog::OnHelp )
END_EVENT_TABLE()

void ShowAboutInfo(wxWindow* parent)
{
    AboutDialog dialog(parent);
    dialog.CreateControls();
    dialog.ShowModal();
}
