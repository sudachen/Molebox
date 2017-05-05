
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

struct ProcDlgIntercom: Refcounted
  {
    ArrayT<StringW> processing_log_;
    int finished_;
    bool kill_;
    Tlocker lock_;

    ProcDlgIntercom()
      {
        kill_ = false;
        finished_ = 0;
      }
    
    void AppendText(pwide_t text)
      {
        __lockon__(lock_)
          {
            processing_log_.Append(StringW(text));
          }
      }

    unsigned Count()
      {
        return processing_log_.Count();
      }
      
    void Clear()
      {
        processing_log_.Clear();
      }

    StringW const &Get(int i)
      {
        return processing_log_[i];
      }

    int Finished()
      {
        return finished_;
      }
      
    void Finish(int val=1)
      {
        finished_ = val;
      }
      
    void KillChild()
      {
        kill_ = true;
      }
  };

struct ProcessDlg: wxDialog
  {

    wxTimer timer_;
    wxButton *break_button_;
    wxButton *go_test_button_;
    wxTextCtrl *text_;
        
    rcc_ptr<ProcDlgIntercom> intercom;
        
    ProcessDlg(wxWindow *parent) 
      : wxDialog(parent,wxID_ANY,Tr(141,"Packing..."),wxDefaultPosition,wxDefaultSize,wxCAPTION),
        timer_(this,tmID_DEFAULT)
      {
      }

    wxWindow *MakeSubControls(wxWindow *parent)
      {
        wxPanel *pan = new wxPanel(parent);
        wxBoxSizer *topszr = new wxBoxSizer(wxVERTICAL);
        topszr->Add(text_ = new wxTextCtrl(pan,wxID_ANY,wxEmptyString,
                                       wxDefaultPosition,wxDefaultSize,
                                       wxTE_RICH2|wxTE_LEFT|wxTE_MULTILINE|wxTE_READONLY|wxHSCROLL), 
                            1, wxGROW|wxALL, 5);
        pan->SetSizer(topszr);
        return pan;
      }
      
    void CreateControls()
      {
        wxBoxSizer *topszr = new wxBoxSizer(wxVERTICAL);
        topszr->Add(MakeSubControls(this), 1, wxGROW|wxALL, 10);
        wxSizer *buttons = new wxBoxSizer(wxHORIZONTAL);//CreateButtonSizer(wxOK|wxCANCEL);
        buttons->Add( go_test_button_ = new wxButton(this,ctID_GOTEST,Tr(142,"Go Test")), 0, wxRIGHT, 10 );
        buttons->Add( break_button_ = new wxButton(this,ctID_BREAK,Tr(143,"Break")), 0, 0, 0 );
        topszr->Add( buttons, 0, wxALIGN_CENTER_HORIZONTAL|wxBOTTOM, 10 );
        go_test_button_->Enable(false);
        SetSizer(topszr);
        SetSize(wxSize(700,380));
        wxSize sz  = GetSize();
        wxSize psz = GetParent()->GetSize();
        wxPoint xy = GetParent()->GetPosition();
        SetPosition(wxPoint(xy.x+(psz.x-sz.x)/2,xy.y+(psz.y-sz.y)/2));
        
        intercom = RccPtr(new ProcDlgIntercom);
      }

    void OnBreak_()
      {
        intercom->KillChild();
      }
      
    void OnBreak(wxCommandEvent &event)
      {
        OnBreak_();
        EndModal(0);
      }
      
    void OnGoTest(wxCommandEvent &event)
      {
        if ( intercom->Finished() != -1 && g_mxf_is_executable )
          {
            StringEfW ehf;
            StringW target = MxfGetFullTarget();
            StringW fold = GetDirectoryOfPath(target);
            StringW cwd = SysGetCurrentDirectory();
            SysSetCurrentDirectory(fold);
            
            if ( !SysExec(_S*L"\"%s\""%target,ehf) )
              DisplayError(this,_S*Tr(144,"Could not run packed executable\n\n-Reason-\n%s")%ehf.Str(),L"Error");
              
            SysSetCurrentDirectory(cwd);
          }
      }

    void OnTimer(wxTimerEvent &event)
      {        
        __lockon__(intercom->lock_)
          {
            if ( intercom->Count() )
              {
                for ( int i = 0; i < intercom->Count() ; ++i ) 
                  {
                    text_->AppendText(+intercom->Get(i));
                    text_->AppendText(L"\n");
                  }
                intercom->Clear();
              }
            if ( intercom->Finished() )
              {
                break_button_->SetLabel(Tr(145,"Close"));
                if ( intercom->Finished() != -1 && g_mxf_is_executable )
                  go_test_button_->Enable(true);
              }
          }
      }
      
      
    static void Coroutine(ProcDlgIntercom *intercom)
      {
        int child = 0; 
        FILE *childin = 0; 
        FILE *childout = 0;
        rcc_ptr<ProcDlgIntercom> i = RccRefe(intercom);
        StringEfW ehf;
        StringW appname = AtAppFolder("mxbpack.exe").Replace(L"/",'\\');
        //StringW source = MxfGetFullSource();//g_mxf_source;
        //StringW target = MxfGetFullTarget();//g_mxf_target;
        //if ( !source || !g_mxf_is_executable ) source = L"~~";
        //if ( target ) target = _S*"-o \"%s\""%target;
        StringW rootfolder = MxfGetRootFolder();
        /*if ( g_current_mxf_file_name )
          rootfolder = GetDirectoryOfPath(g_current_mxf_file_name);
        if ( !rootfolder && g_mxf_source )
          rootfolder = GetDirectoryOfPath(g_mxf_source);*/
        if ( !rootfolder )
          rootfolder = SysGetCurrentDirectory();
#if defined _DEBUG
        if ( SysPipe(0|_S*L"\"%s\" -nU -g \"%s\" ~~ @="%appname%rootfolder,
        //if ( SysPipe(0|_S*L"\"%s\" -nU -d \"%s\" -g \"%s\" %s @="%InAppFold("mxbpack.exe")%source%rootfolder%target,
#else      
        if ( SysPipe(0|_S*L"\"%s\" -nU -g \"%s\" ~~ @="%appname%rootfolder,
        //if ( SysPipe(0|_S*L"\"%s\" -nU \"%s\" -g \"%s\" %s @="%InAppFold("mxbpack.exe")%source%rootfolder%target,
#endif        
                     &childin,&childout,&child,true,false,ehf) )
        //if ( SysPipe(0|_S*L"%s "%appname,&childin,&childout,&child,true,false,ehf) )
          {
            FileDataStream pin(childin,"",true,STREAM_WRITE);
            FileDataStream pot(childout,"",true,STREAM_READ|STREAM_CHARBYCHAR);
            WriteMxfConfig(&pin,ehf);
            //pin.Print("<end>\n");
            pin.Flush();
            StringW l;
            //__asm int 3;
            while ( !pot.Error() )
              {
                if ( TEGGO_OK == pot.UtfReadLineS(l) )
                  i->AppendText(+l.RightTrim());
                if ( intercom->kill_ )
                  {
                    SysPipeKill(&child,&childin,&childout,ehf);
                    i->AppendText(Tr(146,"terminated"));
                    i->Finish(-1);
                    goto e;
                  }
              }              
            i->AppendText(Tr(147,"finished"));
            i->Finish( SysPipeExitCode(child) == 0 ? 1 : -1 );
            SysPipeClose(&child,&childin,&childout,ehf);
          e:;
          }
        else
          {
            i->AppendText(Tr(148,"failed to start mxbpack.exe"));
            i->AppendText(ehf.Str());
            i->Finish(-1);
          }
      }

    bool DoDialog()
      {
        CreateControls();
        timer_.Start(100);
        intercom->AppendText(Tr(149,"Starting packaging process ..."));
        StartCoroutine(&Coroutine,+intercom);
        return ShowModal() == wxID_OK;
      }
      
    DECLARE_EVENT_TABLE()
  };

BEGIN_EVENT_TABLE( ProcessDlg, wxDialog )
    EVT_BUTTON( ctID_GOTEST, ProcessDlg::OnGoTest )
    EVT_BUTTON( ctID_BREAK,  ProcessDlg::OnBreak )
    EVT_TIMER(  tmID_DEFAULT, ProcessDlg::OnTimer)
END_EVENT_TABLE()

bool DoPackingProcess(wxWindow *parent)
  {
    ProcessDlg dialog(parent);
    return dialog.DoDialog();
  }
