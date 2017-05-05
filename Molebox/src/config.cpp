
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

struct MyTedit : wxTextCtrl
  {
    MyTedit(wxWindow *parent, int id, int chars_length,unsigned flags=0)
      {
        Create(parent,id,wxEmptyString,wxDefaultPosition,wxSize(chars_length*6,-1),flags);
      }
  };

struct ConfigDialog : wxDialog
  {
    wxTextCtrl *target_;
    wxTextCtrl *source_;
    wxTextCtrl *password_;
    wxTextCtrl *ext_mask_;
    wxTextCtrl *ext_pwd_;
    wxTextCtrl *cmdline_;
    wxTextCtrl *act_name_;
    wxTextCtrl *reg_;
    wxTextCtrl *signature_;
    wxTextCtrl *token_;
    wxTextCtrl *regmask_;
    wxCheckBox *is_executable_ ; 
    wxCheckBox *do_logging_ ; 
    wxCheckBox *use_vreg_ ; 
    wxCheckBox *do_saveiat_ ; 
    wxCheckBox *do_saversc_ ; 
    wxCheckBox *inject_ ; 
    wxCheckBox *extradata_ ; 
    wxCheckBox *build_vreg_ ; 
    wxCheckBox *relink_exec_ ; 
    wxCheckBox *anticrack_ ; 
    wxCheckBox *fullreg_ ; 
    //wxCheckBox *compress_content_ ; 
    //wxCheckBox *encrypt_content_ ; 
    wxCheckBox *hide_all_ ; 
    wxButton   *source_browse_;
    wxButton   *reg_browse_;
    wxButton   *acquire_token_;
    wxButton   *act_browse_;
    wxCheckBox *autobundle_;
    wxCheckBox *cmdl_chk_;
    wxCheckBox *reg_chk_;
    wxCheckBox *act_chk_;
    wxCheckBox *regmask_chk_;
    wxChoice   *conflict_;
    wxChoice   *autoreg_;
    
    StringW last_source_;
  
    ConfigDialog(wxWindow *parent) 
      : wxDialog(parent,wxID_ANY,Tr(1,"Processing Configuration"),wxDefaultPosition,wxDefaultSize,wxCAPTION)
      {
        autobundle_ = 0;
        signature_ = 0;
        token_ = 0;
        acquire_token_ = 0;
      }

    wxPanel *MakeServiceControls(wxWindow *parent)
      {
        wxPanel *pan = new wxPanel(parent);
        wxBoxSizer *topszr = new wxStaticBoxSizer(new wxStaticBox(pan,wxID_ANY,L"Packaging token"),wxVERTICAL);
        wxBoxSizer *r0 = new wxBoxSizer(wxHORIZONTAL);
        r0->Add(new wxStaticText(pan,wxID_ANY,L"Signature",wxDefaultPosition,wxSize(80,-1)), 0, wxRIGHT|wxALIGN_CENTER_VERTICAL, 3);
        r0->Add(signature_ = new MyTedit(pan,ctID_INPUTTEXT8,60,wxTE_READONLY), 0, wxALIGN_CENTER_VERTICAL, 0 );
        topszr->Add(r0, 0, wxLEFT|wxRIGHT|wxGROW|wxALIGN_LEFT, 5 );
        wxBoxSizer *r1 = new wxBoxSizer(wxHORIZONTAL);
        r1->Add(new wxStaticText(pan,wxID_ANY,L"Token",wxDefaultPosition,wxSize(80,-1)), 0, wxRIGHT|wxALIGN_CENTER_VERTICAL, 3);
        r1->Add(token_ = new MyTedit(pan,ctID_INPUTTEXT9,60), 0, wxALIGN_CENTER_VERTICAL, 0 );
        r1->Add(acquire_token_ = new wxButton(pan,ctID_ACQUIRE_TOKEN,L"Get One"), 0, wxLEFT|wxALIGN_CENTER_VERTICAL, 5 );
        topszr->Add(r1, 0, wxLEFT|wxRIGHT|wxBOTTOM|wxTOP|wxGROW|wxALIGN_LEFT, 5 );
        pan->SetSizer(topszr);
        return pan;
      }

    wxPanel *MakeTargetControls(wxWindow *parent)
      {
        wxPanel *pan = new wxPanel(parent);
        wxBoxSizer *topszr = new wxStaticBoxSizer(new wxStaticBox(pan,wxID_ANY,Tr(2,"Package processing parameters")),wxVERTICAL);
        wxBoxSizer *r0 = new wxBoxSizer(wxHORIZONTAL);
        r0->Add(new wxStaticText(pan,wxID_ANY,Tr(3,"Target"),wxDefaultPosition,wxSize(90,-1)), 0, wxRIGHT|wxALIGN_CENTER_VERTICAL, 3);
        //r0->Add(target_ = new MyTedit(pan,ctID_INPUTTEXT1,60), 0, wxALIGN_CENTER_VERTICAL, 0 );
        r0->Add(target_ = new wxTextCtrl(pan,ctID_INPUTTEXT1,wxEmptyString,wxDefaultPosition,wxDefaultSize,wxTE_LEFT), 1, wxALIGN_CENTER_VERTICAL, 3 );
        r0->Add(new wxButton(pan,ctID_BROWSE_TARGET,Tr(4,"BROWSE")), 0, wxLEFT|wxALIGN_CENTER_VERTICAL, 5 );
        topszr->Add(r0, 0, wxALL|wxGROW|wxALIGN_LEFT, 5 );
        wxSizer *r1 = new wxBoxSizer(wxHORIZONTAL);
        r1->Add(new wxStaticText(pan,wxID_ANY,Tr(5,"Protect package with password"),wxDefaultPosition,/*wxSize(120,-1)*/wxDefaultSize), 0, wxRIGHT|wxALIGN_CENTER_VERTICAL, 5);
        r1->Add(password_ = new MyTedit(pan,ctID_INPUTTEXT2,16), 0, wxRIGHT|wxALIGN_CENTER_VERTICAL, 15);
        if ( wxBoxSizer *rx = new wxBoxSizer(wxHORIZONTAL) )
          {
            wxArrayString strings;     
            strings.Add(Tr(6,"USE EMBEDDED COPY"));
            strings.Add(Tr(7,"USE EXTERNAL COPY"));
            strings.Add(Tr(8,"ACCESS ERROR"));
            //strings.Add(L"DELETE EXTERNAL");   
            rx->Add(new wxStaticText(pan,wxID_ANY,Tr(9,"If file named like embedded exists on disk: "),wxDefaultPosition,wxDefaultSize), 0, wxRIGHT|wxALIGN_CENTER_VERTICAL, 5);
            rx->Add(conflict_ = new wxChoice(pan,ctID_CONFLICT,wxDefaultPosition,/*wxSize(90,-1)*/wxDefaultSize,strings), 0, wxRIGHT|wxALIGN_CENTER_VERTICAL, 0);
            conflict_->SetSelection(0);
            r1->Add(rx, 0, wxRIGHT|wxALIGN_CENTER_VERTICAL, 10 );
            //rx->Show(false);
          }
        topszr->Add(r1, 1, wxLEFT|wxRIGHT|wxBOTTOM|wxGROW|wxALIGN_LEFT, 5 );
        topszr->Add(new wxStaticLine(pan,wxID_ANY), 0, wxALL|wxLEFT|wxRIGHT|wxBOTTOM|wxGROW, 5 );
        r1 = new wxGridSizer(1,3,0,0);
        //r1 = new wxBoxSizer(wxHORIZONTAL);
        r1->Add(build_vreg_ = new wxCheckBox(pan,ctID_CONTENT_BUILDREG,Tr(10,"Build virtual regestry")), 0, wxRIGHT|wxALIGN_CENTER_VERTICAL, 10);
        //r1->Add(encrypt_content_ = new wxCheckBox(pan,ctID_CONTENT_ENCRYPT,Tr(11,"Encrypt"));//, 0, wxRIGHT|wxALIGN_CENTER_VERTICAL, 10);
        //r1->Add(compress_content_ = new wxCheckBox(pan,ctID_CONTENT_ZIP,Tr(12,"Compress"));//, 0, wxRIGHT|wxALIGN_CENTER_VERTICAL, 10);
        r1->Add(hide_all_ = new wxCheckBox(pan,ctID_CONTENT_ZIP,Tr(13,"Hide all files")), 0, wxRIGHT|wxALIGN_CENTER_VERTICAL, 10);
        topszr->Add(r1, 1, wxLEFT|wxRIGHT|wxBOTTOM|wxALIGN_LEFT|wxGROW, 5 );
        pan->SetSizer(topszr);
        return pan;
      }
      
    wxPanel *MakeExecutableControls(wxWindow *parent)
      {
        wxPanel *pan = new wxPanel(parent);
        wxBoxSizer *topszr = new wxStaticBoxSizer(new wxStaticBox(pan,wxID_ANY,Tr(14,"Executable processing parameters")),wxVERTICAL);
        //wxSizer *r00 = new wxGridSizer(1,3,0,0);
        topszr->Add(is_executable_ = new wxCheckBox(pan,ctID_HASCODE,Tr(15,"Merge source executable && package into one EXE file")), 
          0, wxLEFT|wxRIGHT|wxBOTTOM|wxALIGN_LEFT|wxALIGN_CENTER_VERTICAL, 5);
        wxBoxSizer *r0 = new wxBoxSizer(wxHORIZONTAL);
        r0->Add(new wxStaticText(pan,wxID_ANY,Tr(16,"Source"),wxDefaultPosition,wxSize(90,-1)), 0, wxRIGHT|wxALIGN_CENTER_VERTICAL, 3);
        r0->Add(source_ = new wxTextCtrl(pan,ctID_INPUTTEXT3,wxEmptyString,wxDefaultPosition,wxDefaultSize,wxTE_LEFT), 1, wxALIGN_CENTER_VERTICAL, 0 );
        r0->Add(source_browse_ = new wxButton(pan,ctID_BROWSE_SOURCE,Tr(17,"BROWSE")), 0, wxLEFT|wxALIGN_CENTER_VERTICAL, 5 );
        topszr->Add(r0, 0, wxLEFT|wxRIGHT|wxBOTTOM|wxGROW|wxALIGN_LEFT, 5 );
          
        wxSizer *r1 = new wxBoxSizer(wxHORIZONTAL);
        r1->Add(new wxStaticText(pan,wxID_ANY,Tr(18,"Mount external packages by mask"),wxDefaultPosition,wxSize(90,-1)), 0, wxRIGHT|wxALIGN_CENTER_VERTICAL, 5);
        r1->Add(ext_mask_ = new MyTedit(pan,ctID_INPUTTEXT4,16), 0, wxRIGHT|wxALIGN_CENTER_VERTICAL, 15);
        r1->Add(new wxStaticText(pan,wxID_ANY,Tr(19,"with password"),wxDefaultPosition,wxDefaultSize), 0, wxRIGHT|wxALIGN_CENTER_VERTICAL, 5);
        r1->Add(ext_pwd_ = new MyTedit(pan,ctID_INPUTTEXT5,16), 0, wxRIGHT|wxALIGN_CENTER_VERTICAL, 10);
        topszr->Add(r1, 0, wxLEFT|wxRIGHT|wxBOTTOM|wxGROW|wxALIGN_LEFT, 5 );
        topszr->Add(new wxStaticLine(pan,wxID_ANY), 0, wxLEFT|wxRIGHT|wxGROW|wxBOTTOM|wxTOP, 5 );
        //r1 = new wxBoxSizer(wxHORIZONTAL);
        r1 = new wxGridSizer(3,3,10,0);
        //r1->Add(new wxStaticText(pan,wxID_ANY,L"Password",wxDefaultPosition,wxDefaultSize), 0, wxRIGHT|wxALIGN_CENTER_VERTICAL, 3);
        //r1->Add(password_ = new wxTextCtrl(pan,ctID_INPUTTEXT2,wxEmptyString,wxDefaultPosition,wxDefaultSize), 1, wxRIGHT|wxALIGN_CENTER_VERTICAL, 10);
        r1->Add(relink_exec_ = new wxCheckBox(pan,ctID_RELINK,Tr(20,"Relink")), 0, wxRIGHT|wxALIGN_CENTER_VERTICAL, 10);
        //r1->Add(anticrack_ = new wxCheckBox(pan,ctID_ANTICRACK,Tr(21,"Strong Anti-Crack protection")), 0, wxRIGHT|wxALIGN_CENTER_VERTICAL, 10);
        r1->Add(do_logging_ = new wxCheckBox(pan,ctID_LOGGING,Tr(22,"Enable logging")), 0, wxRIGHT|wxALIGN_CENTER_VERTICAL, 10);
        r1->Add(use_vreg_ = new wxCheckBox(pan,ctID_EXE_USEREG,Tr(23,"Use virtual regestry")), 0, wxRIGHT|wxALIGN_CENTER_VERTICAL, 10);
        r1->Add(anticrack_ = new wxCheckBox(pan,ctID_ANTICRACK,Tr(24,"Enable Anti-Crack features")), 0, wxRIGHT|wxALIGN_CENTER_VERTICAL, 10);
        r1->Add(do_saveiat_ = new wxCheckBox(pan,ctID_ANY,Tr(25,"Do not touch IAT")), 0, wxRIGHT|wxALIGN_CENTER_VERTICAL, 10);
        r1->Add(do_saversc_ = new wxCheckBox(pan,ctID_ANY,Tr(26,"Do not touch RSC")), 0, wxRIGHT|wxALIGN_CENTER_VERTICAL, 10);
        r1->Add(inject_ = new wxCheckBox(pan,ctID_ANY,Tr(27,"Inherit virtual environment")), 0, wxRIGHT|wxALIGN_CENTER_VERTICAL, 10);
        r1->Add(extradata_ = new wxCheckBox(pan,ctID_ANY,Tr(28,"Do not encrypt extra")), 0, wxRIGHT|wxALIGN_CENTER_VERTICAL, 10);
        r1->Add(fullreg_ = new wxCheckBox(pan,ctID_ANY,Tr(29,"Full registry virtualization")), 0, wxRIGHT|wxALIGN_CENTER_VERTICAL, 10);
        //r1->Add(autobundle_ = new wxCheckBox(pan,ctID_EXE_AUTOBUNDLE,Tr(30,"Auto bundle DLL dependies")), 0, wxRIGHT|wxALIGN_CENTER_VERTICAL, 10);
        topszr->Add(r1, 0, wxLEFT|wxRIGHT|wxBOTTOM|wxTOP|wxALIGN_LEFT, 10 );

        topszr->Add(new wxStaticLine(pan,wxID_ANY), 0, wxLEFT|wxRIGHT|wxGROW|wxBOTTOM|wxTOP, 5 );

        if ( wxBoxSizer *rx = new wxBoxSizer(wxHORIZONTAL) )
          {
            rx->Add(cmdl_chk_ = new wxCheckBox(pan,ctID_CMDL,Tr(31,"Cmdline"),wxDefaultPosition,wxSize(90,-1)), 0, wxRIGHT|wxALIGN_CENTER_VERTICAL, 3);
            rx->Add(cmdline_ = new wxTextCtrl(pan,ctID_COMMANDLINE,wxEmptyString,wxDefaultPosition,wxDefaultSize,wxTE_LEFT), 1, wxALIGN_CENTER_VERTICAL, 0 );
            topszr->Add(rx, 0, wxALL|wxGROW|wxALIGN_LEFT, 5 );
            //rx->Show(false);
          }
          
        if ( wxBoxSizer *rx = new wxBoxSizer(wxHORIZONTAL) )
          {
            rx->Add(act_chk_ = new wxCheckBox(pan,ctID_ACT,Tr(32,"Activator"),wxDefaultPosition,wxSize(90,-1)), 0, wxRIGHT|wxALIGN_CENTER_VERTICAL, 3);
            rx->Add(act_name_ = new wxTextCtrl(pan,ctID_ACTIVATOR,wxEmptyString,wxDefaultPosition,wxDefaultSize,wxTE_LEFT), 1, wxALIGN_CENTER_VERTICAL, 0 );
            rx->Add(act_browse_ = new wxButton(pan,ctID_BROWSE_ACT,Tr(33,"BROWSE")), 0, wxLEFT|wxALIGN_CENTER_VERTICAL, 5 );
            topszr->Add(rx, 0, wxALL|wxGROW|wxALIGN_LEFT, 5 );
          }
          
        if ( wxBoxSizer *rx = new wxBoxSizer(wxHORIZONTAL) )
          {
            rx->Add(regmask_chk_ = new wxCheckBox(pan,ctID_ACT,Tr(34,"Regmask"),wxDefaultPosition,wxSize(90,-1)), 0, wxRIGHT|wxALIGN_CENTER_VERTICAL, 3);
            rx->Add(regmask_ = new wxTextCtrl(pan,ctID_ACTIVATOR,wxEmptyString,wxDefaultPosition,wxDefaultSize,wxTE_LEFT), 1, wxRIGHT|wxALIGN_CENTER_VERTICAL, 5 );
            wxArrayString strings;     
            strings.Add(Tr(35,"DO NOT REGISTER"));
            strings.Add(Tr(36,"REGISTER ON START"));
            strings.Add(Tr(37,"REGISTER WHEN PACK"));
            //strings.Add(L"DELETE EXTERNAL");   
            rx->Add(new wxStaticText(pan,wxID_ANY,Tr(38,"Autoregister OCX/ActiveX: "),wxDefaultPosition,wxDefaultSize), 0, wxLEFT|wxRIGHT|wxALIGN_CENTER_VERTICAL, 5);
            rx->Add(autoreg_ = new wxChoice(pan,ctID_CONFLICT,wxDefaultPosition,/*wxSize(90,-1)*/wxDefaultSize,strings), 0, wxRIGHT|wxALIGN_CENTER_VERTICAL, 0);
            autoreg_->SetSelection(2);
            
            topszr->Add(rx, 0, wxALL|wxGROW|wxALIGN_LEFT, 5 );
          }

        pan->SetSizer(topszr);
        return pan;
      }

    wxPanel *MakeSubControls(wxWindow *parent)
      {
        wxBoxSizer *topszr = new wxBoxSizer(wxVERTICAL);
        wxPanel *pan = new wxPanel(parent);
        topszr->Add( MakeExecutableControls(pan), 0, wxGROW|wxALL , 5 );
        topszr->Add( MakeTargetControls(pan), 0, wxGROW|wxALL , 5 );
        //if ( LicenseType() == LICENSE_NONE || LicenseType() == LICENSE_DEMO )
        //  topszr->Add( MakeServiceControls(pan), 0, wxGROW|wxALL , 5 );
        pan->SetSizer(topszr);
        return pan;
      }
      
    void CreateControls()
      {
        wxBoxSizer *topszr = new wxBoxSizer(wxVERTICAL);
        topszr->Add(MakeSubControls(this), 1, wxGROW|wxALL, 10);
        wxSizer *buttons = CreateButtonSizer(wxOK|wxCANCEL);
        buttons->Add( new wxButton(this,ctID_HELP,L"Help") );
        topszr->Add( buttons, 0, wxALIGN_CENTER_HORIZONTAL|wxBOTTOM, 10 );
        SetSizer(topszr);
        topszr->Fit(this);
        wxSize sz = GetSize();
        wxSize psz = GetParent()->GetSize();
        wxPoint xy = GetParent()->GetPosition();
        SetPosition(wxPoint(xy.x+(psz.x-sz.x)/2,xy.y+(psz.y-sz.y)/2));
        // fetching values
        if ( g_mxf_pkg_password ) password_->SetValue(+g_mxf_pkg_password);
        if ( g_mxf_ext_mask ) ext_mask_->SetValue(+g_mxf_ext_mask);
        if ( g_mxf_ext_password ) ext_pwd_->SetValue(+g_mxf_ext_password);
        if ( g_mxf_is_executable ) is_executable_->SetValue(wxCHK_CHECKED); 
        if ( g_mxf_do_logging ) do_logging_->SetValue(wxCHK_CHECKED); 
        if ( g_mxf_use_vreg ) use_vreg_->SetValue(wxCHK_CHECKED);
        if ( g_mxf_do_saveiat ) do_saveiat_->SetValue(wxCHK_CHECKED);
        if ( g_mxf_do_saversc ) do_saversc_->SetValue(wxCHK_CHECKED);
        if ( g_mxf_inject ) inject_->SetValue(wxCHK_CHECKED);
        if ( g_mxf_extradata ) extradata_->SetValue(wxCHK_CHECKED);
        if ( g_mxf_build_vreg ) build_vreg_->SetValue(wxCHK_CHECKED); 
        if ( g_mxf_relink_exec ) relink_exec_->SetValue(wxCHK_CHECKED); 
        if ( g_mxf_use_anticrack ) anticrack_->SetValue(wxCHK_CHECKED);
        //if ( g_mxf_compress ) compress_content_->SetValue(wxCHK_CHECKED); 
        if ( g_mxf_autobundle && autobundle_ ) autobundle_->SetValue(wxCHK_CHECKED); 
        //if ( g_mxf_encrypt ) encrypt_content_->SetValue(wxCHK_CHECKED); 
        if ( g_mxf_target ) target_->SetValue(+g_mxf_target); 
        if ( g_mxf_source ) source_->SetValue(+g_mxf_source); 
        if ( g_mxf_cmdline ) cmdline_->SetValue(+g_mxf_cmdline);
        if ( g_mxf_use_cmdline ) cmdl_chk_->SetValue(wxCHK_CHECKED);
        if ( g_mxf_activator ) act_name_->SetValue(+g_mxf_activator);
        if ( g_mxf_use_activator ) act_chk_->SetValue(wxCHK_CHECKED);
        if ( g_mxf_hideall ) hide_all_->SetValue(wxCHK_CHECKED);
        if ( g_mxf_fullreg ) fullreg_->SetValue(wxCHK_CHECKED);
        if ( token_ && g_mxf_token ) token_->SetValue(+g_mxf_token);
        if ( g_mxf_conflict && g_mxf_conflict >= 0 && g_mxf_conflict < 3 ) 
          conflict_->SetSelection(g_mxf_conflict); 
        if ( g_mxf_use_regmask ) regmask_chk_->SetValue(wxCHK_CHECKED);
        if ( g_mxf_regmask ) regmask_->SetValue(+g_mxf_regmask);
        if ( g_mxf_autoreg >= 0 && g_mxf_autoreg < 3 )
          autoreg_->SetSelection(g_mxf_autoreg);
        VerifyAndFix();
      }
    
    void VerifyAndFix()
      {
        //is_executable_->SetValue(wxCHK_CHECKED);
        bool is_executable = is_executable_->GetValue()==wxCHK_CHECKED;
        is_executable_->Enable(true);
        use_vreg_->Enable(is_executable);
        source_->Enable(is_executable);
        source_browse_->Enable(is_executable);
        cmdl_chk_->Enable(is_executable);
        cmdline_->Enable(is_executable && cmdl_chk_->GetValue()==wxCHK_CHECKED);
        act_chk_->Enable(is_executable);
        act_name_->Enable(is_executable && act_chk_->GetValue()==wxCHK_CHECKED);
        act_browse_->Enable(is_executable && act_chk_->GetValue()==wxCHK_CHECKED);
        ext_mask_->Enable(is_executable);
        ext_pwd_->Enable(is_executable);
        password_->Enable(true);
        //encrypt_content_->Enable(true);
        do_logging_->Enable(is_executable);
        build_vreg_->Enable(!is_executable);
        if ( is_executable ) build_vreg_->SetValue(use_vreg_->GetValue());
        if ( autobundle_ ) autobundle_->Enable(is_executable);
        
        regmask_chk_->Enable(is_executable && use_vreg_->GetValue()==wxCHK_CHECKED);
        autoreg_->Enable(is_executable && use_vreg_->GetValue()==wxCHK_CHECKED);
        regmask_->Enable(is_executable  && regmask_chk_->GetValue()==wxCHK_CHECKED && use_vreg_->GetValue()==wxCHK_CHECKED);
        relink_exec_->Enable(is_executable);
        anticrack_->Enable(is_executable /*&& relink_exec_->GetValue()==wxCHK_CHECKED*/);
        do_saveiat_->Enable(is_executable /*&& relink_exec_->GetValue()==wxCHK_CHECKED*/);        
        do_saversc_->Enable(is_executable /*&& relink_exec_->GetValue()==wxCHK_CHECKED*/);        
        inject_->Enable(is_executable /*&& relink_exec_->GetValue()==wxCHK_CHECKED*/);        
        fullreg_->Enable(is_executable /*&& relink_exec_->GetValue()==wxCHK_CHECKED*/);        
        extradata_->Enable(is_executable && relink_exec_->GetValue()==wxCHK_CHECKED );        
        //compress_content_->Enable(encrypt_content_->GetValue()!=wxCHK_CHECKED);
        //if ( encrypt_content_->GetValue()==wxCHK_CHECKED ) compress_content_->SetValue(wxCHK_CHECKED);
        if ( relink_exec_->GetValue()!=wxCHK_CHECKED ) anticrack_->SetValue(wxCHK_UNCHECKED); 
        
        if ( token_ ) 
          {
            token_->Enable(is_executable);
          }
          
        /*if ( signature_ )
          {
            signature_->Enable(is_executable);
            if ( last_source_ != source_->GetValue().c_str() )
              {
                last_source_ = source_->GetValue().c_str();
                signature_->SetValue(+CalculateSourceSignature(+last_source_));
              }
            else if ( !last_source_.Length() )
              {
                signature_->SetValue(L"");
              }
          }*/
        if ( acquire_token_ )
          {
            acquire_token_->Enable(is_executable);
          }
      }
    
    void OnStateUpdate(wxCommandEvent &event)
      {
        VerifyAndFix();
      }
    
    void OnOK(wxCommandEvent &event)
      {
        // pushing values
        g_mxf_is_executable = is_executable_->GetValue()==wxCHK_CHECKED; 
        //g_mxf_compress = compress_content_->GetValue()==wxCHK_CHECKED; 
        //g_mxf_encrypt = encrypt_content_->GetValue()==wxCHK_CHECKED; 
        g_mxf_pkg_password = password_->GetValue().c_str();
        g_mxf_ext_mask = ext_mask_->GetValue().c_str();
        g_mxf_ext_password = ext_pwd_->GetValue().c_str();
        g_mxf_do_logging = do_logging_->GetValue()==wxCHK_CHECKED; 
        g_mxf_use_vreg = use_vreg_->GetValue()==wxCHK_CHECKED;
        g_mxf_do_saveiat = do_saveiat_->GetValue()==wxCHK_CHECKED;
        g_mxf_do_saversc = do_saversc_->GetValue()==wxCHK_CHECKED;
        g_mxf_build_vreg = build_vreg_->GetValue()==wxCHK_CHECKED; 
        if (autobundle_) g_mxf_autobundle = autobundle_->GetValue()==wxCHK_CHECKED; 
        g_mxf_relink_exec = relink_exec_->GetValue()==wxCHK_CHECKED; 
        g_mxf_use_anticrack = anticrack_->GetValue()==wxCHK_CHECKED;
        g_mxf_source = source_->GetValue().c_str();
        g_mxf_target = target_->GetValue().c_str();
        if ( g_mxf_optscrc != MxfCalcuateOptsCRC() )
          { 
            g_mxf_changed = true;
            g_mxf_optscrc = MxfCalcuateOptsCRC();
          }
        g_mxf_cmdline = cmdline_->GetValue().c_str();
        g_mxf_use_cmdline = cmdl_chk_->GetValue()==wxCHK_CHECKED;
        g_mxf_activator = act_name_->GetValue().c_str();
        g_mxf_use_activator = act_chk_->GetValue()==wxCHK_CHECKED;
        g_mxf_hideall = hide_all_->GetValue()==wxCHK_CHECKED;
        g_mxf_conflict = conflict_->GetSelection();
        g_mxf_inject = inject_->GetValue()==wxCHK_CHECKED;
        g_mxf_extradata = extradata_->GetValue()==wxCHK_CHECKED;
        g_mxf_fullreg = fullreg_->GetValue()==wxCHK_CHECKED;
        g_mxf_use_regmask = regmask_chk_->GetValue()==wxCHK_CHECKED;
        g_mxf_regmask = regmask_->GetValue().c_str();
        g_mxf_autoreg = autoreg_->GetSelection();
        EndModal(wxID_OK);
      }

    StringW GetFileNameViaOpenDialog(int target)
      {
        wxString caption    = target?Tr(39,"Select Packing Target"):Tr(40,"Select Executable Source");
        wxString wildcard   = target?Tr(41,"Executable Target (*.exe)|*.exe|External Package (*.dat)|*.dat|Another File Name (*.*)|*.*")
                                    :Tr(42,"Executable Source (*.exe)|*.exe");
        switch ( target )
          {
            case 0:
              caption    = Tr(43,"Select Executable Source");
              wildcard   = Tr(44,"Executable Source (*.exe)|*.exe");
              break;
            case 1:
              caption    = Tr(45,"Select Packing Target");
              wildcard   = Tr(46,"Executable Target (*.exe)|*.exe|External Package (*.dat)|*.dat|Another File Name (*.*)|*.*");
              break;
            case 2:
              caption    = Tr(47,"Select Activator");
              wildcard   = Tr(48,"Acivator DLL (*.dll)|*.dll|Another File Name (*.*)|*.*");
              break;
          }
          
        wxString defaultDir = wxEmptyString;//g_last_operated_directory;
        wxString defaultFilename = wxEmptyString;
        wxFileDialog dialog(this, caption, defaultDir, defaultFilename,wildcard, wxFD_OPEN/*|wxFILE_MUST_EXIT*/);
        wxSize sz  = dialog.GetSize();
        wxSize psz = this->GetSize();
        wxPoint xy = this->GetPosition();
        dialog.SetPosition(wxPoint(xy.x+(psz.x-sz.x)/2,xy.y+(psz.y-sz.y)/2));

        if (dialog.ShowModal() == wxID_OK)
          {
            StringW f = dialog.GetPath().c_str();
            f.Trim();
            return f;
          }
          
        return L"";
      }
      
    void OnBrowseTarget(wxCommandEvent &event)
      {
        if ( StringW n = GetFileNameViaOpenDialog(1) )
          target_->SetValue(+n);
      }
      
    void OnBrowseSource(wxCommandEvent &event)
      {
        if ( StringW n = GetFileNameViaOpenDialog(0) )
          {
            source_->SetValue(+n);  
            VerifyAndFix();
          }
      }
      
    void OnBrowseAct(wxCommandEvent &event)
      {
        if ( StringW n = GetFileNameViaOpenDialog(2) )
          {
            act_name_->SetValue(+n);  
            VerifyAndFix();
          }
      }

    void OnHelp(wxCommandEvent &e)
      {
        ShowHelpSection(L"using_package_options.htm");
      }

    DECLARE_EVENT_TABLE()
  };

BEGIN_EVENT_TABLE( ConfigDialog, wxDialog )
    EVT_BUTTON( wxID_OK, ConfigDialog::OnOK )
    EVT_CHECKBOX( ctID_HASCODE, ConfigDialog::OnStateUpdate )
    EVT_CHECKBOX( ctID_EXE_USEREG, ConfigDialog::OnStateUpdate )
    EVT_CHECKBOX( ctID_RELINK, ConfigDialog::OnStateUpdate )
    EVT_CHECKBOX( ctID_CONTENT_ENCRYPT, ConfigDialog::OnStateUpdate )
    EVT_CHECKBOX( ctID_REG, ConfigDialog::OnStateUpdate )
    EVT_CHECKBOX( ctID_CMDL, ConfigDialog::OnStateUpdate )
    EVT_CHECKBOX( ctID_ACT, ConfigDialog::OnStateUpdate )
    EVT_CHECKBOX( ctID_ENV, ConfigDialog::OnStateUpdate )
    EVT_BUTTON( ctID_BROWSE_TARGET, ConfigDialog::OnBrowseTarget )
    EVT_BUTTON( ctID_BROWSE_SOURCE, ConfigDialog::OnBrowseSource )
    EVT_BUTTON( ctID_BROWSE_ACT, ConfigDialog::OnBrowseAct )
    EVT_BUTTON( ctID_HELP, ConfigDialog::OnHelp )
END_EVENT_TABLE()

bool ShowConfigDialog(wxWindow *parent)
  {
    ConfigDialog dialog(parent);
    dialog.CreateControls();
    parent->Raise();
    dialog.Raise();
    return dialog.ShowModal() == wxID_OK;
  }
  
