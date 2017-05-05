
/*

  Copyright (c) 2005-2009, Alexey Sudachen, alexey@sudachen.name

*/

#ifndef ___7b92472f_2078_4e76_84cc_b9d9db3f46c3___
#define ___7b92472f_2078_4e76_84cc_b9d9db3f46c3___

#include "saxparser.h"

#if defined _TEGGO_SAXPARS_HERE
# define _TEGGO_SAXPARS_FAKE_INLINE _TEGGO_EXPORTABLE
#else
# define _TEGGO_SAXPARS_FAKE_INLINE CXX_FAKE_INLINE
#endif

namespace teggo {

  _TEGGO_SAXPARS_FAKE_INLINE
    TEGGOt_E CXX_STDCALL SAXparseTag(SAX_TBF &stbf, XMLdocumentBuilder *db)
      {
        static const wchar_t EOD = 0xffffU;
        for(;;)
          {
            if ( *stbf.last == EOD ) return TEGGO_OK;

            while ( *stbf.last != '<' && *stbf.last != EOD ) stbf.GetW();

            if ( *stbf.last == '<' &&
                 (stbf.last[1] == '/' || stbf.last[1] == '!' || iswalpha(stbf.last[1]) ))
              {
                stbf.RemoveExtraWS();
                if ( stbf.Begin() != stbf.End() )
                  db->Body(stbf.Begin(),stbf.End());
                stbf.Trunc();
              }

            stbf.GetW();

            if ( *stbf.last == '!' && stbf.last[1] == '-' && stbf.last[2] == '-' )
              {
                stbf.GetW();
                stbf.GetW();
                stbf.GetW();
                stbf.Trunc();
                for (;;stbf.GetW())
                  {
                    while ( *stbf.last != EOD && *stbf.last != '-' ) stbf.GetW();
                    if ( *stbf.last == '-' && stbf.last[1] == '-' && stbf.last[2] == '>' )
                      {
                        stbf.RemoveExtraWS();
                        db->Comment(stbf.Begin(),stbf.End());
                        break;
                      }
                  }
                stbf.TruncTo('>');
              }
            else if ( *stbf.last == '/' )
              {
                // '</...>'
                stbf.GetW();
                stbf.Trunc();
                while ( isalnum(*stbf.last) ) { stbf.GetW(); }
                db->EndTag(stbf.Begin(),stbf.End());
                stbf.TruncTo('>');
              }
            else if ( *stbf.last == '?' )
              {
                stbf.GetW();
                bool code = (*stbf.last == '-' && stbf.last[1] == '-');
                bool xml =
                          ( *stbf.last == 'x' || *stbf.last == 'X' )
                         && ( stbf.last[1] == 'm' || stbf.last[1] == 'M' )
                         && ( stbf.last[2] == 'l' || stbf.last[2] == 'L' );

                stbf.Trunc();
                for ( ;stbf.GetW(); )
                  if ( *stbf.last == '?' && stbf.last[1] == '>' )
                    {
                      if ( code )
                        {
                          db->BeginTag1(L"<?code?>");
                          db->Param(0,0,stbf.Begin()+2,stbf.End());
                          db->EndTag(0,0);
                        }
                      else
                        {
                        }
                      stbf.Trunc();
                      stbf.GetW_();
                      stbf.GetW_();
                      break;
                    }
                continue;
              }
            else if ( iswalpha(*stbf.last) )
              {
                // '<...>'
                stbf.Trunc();
                while ( iswalnum(stbf.GetW()) ) {}
                db->BeginTag(stbf.Begin(),stbf.End());
                do
                  {
                    while ( iswspace(*stbf.last) ) stbf.GetW();
                    if ( *stbf.last == '/' && stbf.last[1] == '>' )
                      {
                        db->EndTag(0,0);
                        break;
                      }
                    else if ( isalpha(*stbf.last) )
                      {
                        // 'paramname'
                        stbf.Trunc();
                        while ( isalnum(stbf.GetW()) ) {}
                        int pE = stbf.End() - stbf.Begin();
                        int vS = pE;
                        int vE = pE;
                        while ( iswspace(*stbf.last) ) stbf.GetW();
                        if ( *stbf.last == '=' )
                          {
                            // value
                            stbf.GetW(); // skip '='
                            while ( iswspace(*stbf.last) ) stbf.GetW();
                            if ( *stbf.last == '\'' || *stbf.last == '"' )
                              {
                                wchar_t c = *stbf.last;
                                stbf.GetW(); // skip '"'
                                vS = stbf.End() - stbf.Begin();
                                do
                                  {
                                    if ( *stbf.last == '\\' )
                                      stbf.GetW();
                                    stbf.GetW();
                                  }
                                while ( *stbf.last && *stbf.last != c );
                                vE = stbf.End() - stbf.Begin();
                                stbf.GetW(); // skip '"'
                              }
                            else
                              {
                                vS = stbf.End() - stbf.Begin();
                                while ( isalnum(*stbf.last) ) stbf.GetW();
                                vE = stbf.End() - stbf.Begin();
                              }
                          }
                        db->Param(stbf.Begin(),stbf.Begin()+pE,stbf.Begin()+vS,stbf.Begin()+vE);
                      }
                    else if ( *stbf.last == '>' )
                      break;
                    else
                      while ( !iswspace(*stbf.last) && *stbf.last != '/' && *stbf.last != '>' )
                        stbf.GetW();
                  }
                while (*stbf.last != '>');
                stbf.TruncTo('>');
              }
          }
      }

  _TEGGO_SAXPARS_FAKE_INLINE
    TEGGOt_E CXX_STDCALL SAXparse(DataStream *ds, XMLdocumentBuilder *db)
      {
        SAX_TBF tbf(*ds);
        tbf.Fill();
        db->BeginDocument();
        TEGGOt_E e = SAXparseTag(tbf,db);
        db->EndDocument();
        return e;
      }

  _TEGGO_SAXPARS_FAKE_INLINE
    TEGGOt_E CXX_STDCALL SAXparse(StringParam source, XMLdocumentBuilder *db)
      {
        if ( rcc_ptr<DataStream> ds = DataSource->Open(source,FILEOPEN_READ) )
          return SAXparse(+ds,db);
        return TEGGO_NOFILE;
      }

  _TEGGO_SAXPARS_FAKE_INLINE
    XMLdocumentWriter::XMLdocumentWriter(DataStream *ds) : ds_(Refe(ds)), state_(0)
      {
      }

  _TEGGO_SAXPARS_FAKE_INLINE
    XMLdocumentWriter::~XMLdocumentWriter()
      {
      }

  CXX_FAKE_INLINE
    void XMLdocumentWriter::_Indent()
      {
        ds_->WriteText("\n",1);
        for ( int i = 0; i < tab_; ++i )
          ds_->WriteText("    ",4);
      }

  _TEGGO_SAXPARS_FAKE_INLINE
    TEGGOt_E XMLdocumentWriter::BeginTag(pwide_t pS, pwide_t pE)
      {
        if ( state_ )
          ds_->UtfPrint(L">");
        state_ = 1;
        _Indent();
        ds_->UtfPrint(L"<");
        ds_->UtfWriteText(pS,pE-pS);
        ++tab_;
        return TEGGO_OK;
      }

  _TEGGO_SAXPARS_FAKE_INLINE
    TEGGOt_E XMLdocumentWriter::EndTag(pwide_t pS, pwide_t pE)
      {
        --tab_;
        REQUIRE( tab_ >= 0 );
        if ( state_ )
          {
            ds_->UtfPrint(L" />"), state_ = 0;
          }
        else
          {
            _Indent();
            ds_->UtfPrint(L"</");
            ds_->UtfWriteText(pS,pE-pS);
            ds_->UtfPrint(L">");
          }
        return TEGGO_OK;
      }

  _TEGGO_SAXPARS_FAKE_INLINE
    TEGGOt_E XMLdocumentWriter::Body(pwide_t pS, pwide_t pE)
      {
        if ( state_ )
          ds_->UtfPrint(L">"), state_ = 0;
        _Indent();
        ds_->UtfWriteText(pS,pE-pS);
        return TEGGO_OK;
      }

  _TEGGO_SAXPARS_FAKE_INLINE
    TEGGOt_E XMLdocumentWriter::Param(pwide_t namS, pwide_t namE, pwide_t valS, pwide_t valE)
      {
        STRICT_REQUIRE( state_ == 1 );
        ds_->UtfPrint(L" ");
        ds_->UtfWriteText(namS,namE-namS);
        if ( valE != valS )
          {
            ds_->UtfPrint(L"=\"");
            ds_->UtfWriteText(valS,valE-valS);
            ds_->UtfPrint(L"\"");
          }
        return TEGGO_OK;
      }

  _TEGGO_SAXPARS_FAKE_INLINE
    TEGGOt_E XMLdocumentWriter::Comment(pwide_t pS, pwide_t pE)
      {
        if ( state_ )
          ds_->UtfPrint(L">"), state_ = 0;

        _Indent();
        ds_->UtfPrint(L"<!--");
        ds_->UtfWriteText(pS,pE-pS);
        ds_->UtfPrint(L"-->");
        return TEGGO_OK;
      }

  _TEGGO_SAXPARS_FAKE_INLINE
    TEGGOt_E XMLdocumentWriter::CDATA(pwide_t pS, pwide_t pE)
      {
        return TEGGO_OK;
      }

  _TEGGO_SAXPARS_FAKE_INLINE
    TEGGOt_E XMLdocumentWriter::BeginDocument()
      {
        ds_->UtfPrint(L"<?xml version=\"1.0\" encoding=\"utf-8\" ?>");
        state_ = 0;
        tab_ = 0;
        return TEGGO_OK;
      }

  _TEGGO_SAXPARS_FAKE_INLINE
    TEGGOt_E XMLdocumentWriter::EndDocument()
      {
        if ( state_ )
          ds_->UtfPrint(L">"), state_ = 0;
        ds_->UtfPrint(L"\n");
        return TEGGO_OK;
      }

  _TEGGO_SAXPARS_FAKE_INLINE
    rcc_ptr<XMLdocumentWriter> CXX_STDCALL OpenXMLdocWriter(StringParam fname)
      {
        if ( rcc_ptr<DataStream> ds = DataSource->Open(fname,FILEOPEN_WRITE|FILEOPEN_CREATE) )
          return rcc_ptr<XMLdocumentWriter>(new XMLdocumentWriter(+ds));
        else
          return rcc_ptr<XMLdocumentWriter>(0);
      }

} // namespace teggo

#endif // ___7b92472f_2078_4e76_84cc_b9d9db3f46c3___
