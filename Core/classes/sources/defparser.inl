
/*

  Copyright (c) 2005-2009, Alexey Sudachen, alexey@sudachen.name

*/

#ifndef ___803e9fb1_2535_4c15_b9d0_2507576a46cd___
#define ___803e9fb1_2535_4c15_b9d0_2507576a46cd___

#include "defparser.h"
#include "slist.h"
#include "tuple.h"

#if defined _TEGGO_DEFPARS_HERE
# define _TEGGO_DEFPARS_FAKE_INLINE _TEGGO_EXPORTABLE
#else
# define _TEGGO_DEFPARS_FAKE_INLINE CXX_FAKE_INLINE
#endif

namespace teggo {

  struct DEF_TBF : SAX_TBF
    {
      DEF_TBF(DataStream &ds) : SAX_TBF(ds)
        {
        }
    };

  _TEGGO_DEFPARS_FAKE_INLINE
    TEGGOt_E CXX_STDCALL DEFparseTag(DEF_TBF &stbf, XMLdocumentBuilder *db)
      {
        static wchar_t const EOD = 0xffff;

        SlistT<StringW> tags;
        StringW tag;
        bool tag_opened = false;

        for(;;)
          {
            stbf.TruncSkipWS();
            if ( *stbf.last == EOD )
              return TEGGO_OK;

            if ( iswalnum(*stbf.last) || *stbf.last == '_' || *stbf.last == '-' || *stbf.last == '.')
              {
                if ( tag && tag_opened )
                  db->EndTag1(+tag);
                tag_opened = false;
                while ( iswalnum(*stbf.last) || *stbf.last == '_' || *stbf.last == '-' || *stbf.last == '.')
                  { stbf.GetW(); }
                tag = StringW(stbf.Begin(),stbf.End());
              }
            else if ( *stbf.last == '/' && stbf.last[1] == '/' )
              {
                while ( *stbf.last != EOD && *stbf.last != '\n' )
                  stbf.GetW_();
                stbf.GetW_();
              }
            else if ( *stbf.last == '/' && stbf.last[1] == '*' )
              {
                while ( *stbf.last != EOD && !(*stbf.last == '*' && stbf.last[1] == '/') )
                  stbf.GetW_();
                stbf.GetW_();
                stbf.GetW_();
              }
            else if ( *stbf.last == '(' && tag && !tag_opened )
              { // content
                tag_opened = true;
                db->BeginTag1(+tag);
                stbf.TruncTo('(');
                while ( *stbf.last != EOD  && *stbf.last != ')' ) stbf.GetW();
                db->Body(stbf.Begin(),stbf.End());
                stbf.TruncTo(')');
              }

            else if ( *stbf.last == '{' && tag )
              { // params & children
                if ( !tag_opened )
                  {
                    tag_opened = false;
                    db->BeginTag1(+tag);
                  }
                tags.Push(tag);
                tag = "";
                stbf.TruncTo('{');
              }

            else if ( *stbf.last == '}' && !tags.Empty() )
              {
                if ( tag && tag_opened )
                  db->EndTag1(+tag);
                tag_opened = false;
                db->EndTag1(+tags.Top());
                tags.Pop();
                tag = "";
                stbf.TruncTo('}');
              }

            else if ( *stbf.last == '=' && tag )
              {
                StringW pname;

                stbf.GetW_();
                stbf.TruncSkipWS();

                if ( tag_opened )
                  pname = L"value";
                else
                  pname = tag;

                if ( *stbf.last == '"' || *stbf.last == '\'' || *stbf.last == '`' )
                  {
                    wchar_t c = *stbf.last;
                    stbf.GetW_();
                    if ( c == '`' ) c = '\'';
                    while ( *stbf.last != EOD && *stbf.last != c ) stbf.GetW();
                    db->Param(pname.Begin(),pname.End(),stbf.Begin(),stbf.End());
                    stbf.GetW_();
                  }
                else
                  {
                    while ( *stbf.last != EOD && !iswspace(*stbf.last) && *stbf.last!='}' )
                      stbf.GetW();
                    db->Param(pname.Begin(),pname.End(),stbf.Begin(),stbf.End());
                  }
              }

            else // error
              {
                Lerr << _S*"{ERROR} DEFparse failed to parse '%s' at line %d"
                  %stbf.tbf_.ds_.Identifier()
                  %stbf.tbf_.count_;
                return TEGGO_FAIL;
              }
          }
      }

  _TEGGO_DEFPARS_FAKE_INLINE
    TEGGOt_E CXX_STDCALL DEFparse(DataStream *ds, XMLdocumentBuilder *db)
      {
        DEF_TBF tbf(*ds);
        tbf.Fill();
        db->BeginDocument();
        TEGGOt_E e = DEFparseTag(tbf,db);
        db->EndDocument();
        return e;
      }

  _TEGGO_DEFPARS_FAKE_INLINE
    TEGGOt_E CXX_STDCALL DEFparse(StringParam source, XMLdocumentBuilder *db)
      {
        if ( rcc_ptr<DataStream> ds = DataSource->Open(source,FILEOPEN_READ) )
          return DEFparse(+ds,db);
        return TEGGO_NOFILE;
      }

  _TEGGO_DEFPARS_FAKE_INLINE
    TEGGOt_E CXX_STDCALL XwriteDefDocument_Print(DataStream *ds, bool use_utf8, pwide_t text)
      {
        if ( use_utf8 )
          return ds->UtfPrint(text);
        else
          return ds->Print(+StringA(text));
      }

  _TEGGO_DEFPARS_FAKE_INLINE
    void CXX_STDCALL XwriteDefDocument_Indent(DataStream *ds, int tabs)
      {
        for ( int i =0; i < tabs; ++i )
          ds->Print("  ");
      }

  _TEGGO_DEFPARS_FAKE_INLINE
    void CXX_STDCALL XwriteDefDocument_Params(DataStream *ds, bool use_utf8, int tabs, Xparam const *par)
      {
        for ( ; par ; par = par->next )
          {
            XwriteDefDocument_Indent(ds,tabs);
            XwriteDefDocument_Print(ds,use_utf8,par->name);
            ds->Print(" = \"");
            XwriteDefDocument_Print(ds,use_utf8,+par->value);
            ds->Print("\"\n");
          }
      }

  _TEGGO_DEFPARS_FAKE_INLINE
    TEGGOt_E CXX_STDCALL XwriteDefDocument(Xnode *root_node, DataStream *ds, bool use_utf8)
      {
        int tabs = 0;
        pwide_t body_tag = root_node->GetDocument()->LookupLiteral(L"<?body?>");
        pwide_t comment_tag = root_node->GetDocument()->LookupLiteral(L"<?comment?>");

        if ( Xparam const *par = root_node->GetFirstParam() )
          XwriteDefDocument_Params(ds,use_utf8,0,par);

        BufferT< Tuple2<Xnode *,Xiterator> > nstk;
        nstk.Push(tuple2(root_node,root_node->Iterate()));

        while ( !nstk.Empty() )
          if ( nstk->_1.Next() )
            {
              Xnode *n = nstk->_1.Get();
              pwide_t tag = n->GetName();
              if ( tag == body_tag )
                ;// skip
              else if ( tag == comment_tag )
                {
                  XwriteDefDocument_Indent(ds,tabs);
                  ds->Print("/*");
                  if ( pwide_t val = n->GetParam(L"") )
                    XwriteDefDocument_Print(ds,use_utf8,val);
                  ds->Print("*/\n");
                }
              else
                {
                  XwriteDefDocument_Indent(ds,tabs);
                  XwriteDefDocument_Print(ds,use_utf8,tag);
                  if ( pwide_t content = n->GetContent() )
                    {
                      ds->Print("(");
                      XwriteDefDocument_Print(ds,use_utf8,content);
                      ds->Print(")");
                    }
                  ds->Print("{");
                  ++tabs;
                  bool need_ln = true;
                  if ( Xparam const *par = n->GetFirstParam() )
                    {
                      ds->Print("\n");
                      need_ln = false;
                      XwriteDefDocument_Params(ds,use_utf8,tabs,par);
                    }
                  if ( n->HasChildren() )
                    {
                      if ( need_ln ) ds->Print("\n");
                      nstk.Push(tuple2(n,n->Iterate()));
                    }
                  else
                    {
                      --tabs;
                      XwriteDefDocument_Indent(ds,tabs);
                      ds->Print("}\n");
                    }
                }
            }
          else
            {
              if ( nstk.Count() > 1 )
                {
                  REQUIRE( tabs > 0 );
                  --tabs;
                  XwriteDefDocument_Indent(ds,tabs);
                  ds->Print("}\n");
                }
              nstk.Pop();
            }

        return TEGGO_OK;
      }

  _TEGGO_DEFPARS_FAKE_INLINE
    TEGGOt_E  CXX_STDCALL XwriteDefDocument(Xnode *root_node, StringParam fname,bool use_utf8)
      {
          if ( rcc_ptr<DataStream> ds = DataSource->Open(fname,FILEOPEN_WRITE|FILEOPEN_CREATE) )
            return XwriteDefDocument(root_node, +ds,use_utf8);
          else
            return TEGGO_NOFILE;
      }

  _TEGGO_DEFPARS_FAKE_INLINE
    Xdocument CXX_STDCALL XopenDefDocument(StringParam docname)
      {
        teggo::XdocumentBuilder db;
        if ( TEGGOt_E e = DEFparse(docname,&db) )
          {
            StringW err = _S*L"[XDOCUMENT&DEF ERROR] %?, docname '%?'" %TeggoErrorText(e) %+docname;
            Lerr << err;
            _E.Fail(err);
            return Xdocument(0);
          }
        return db.ProduceDocument();
      }

  _TEGGO_DEFPARS_FAKE_INLINE
    Xdocument CXX_STDCALL XopenDefDocument(DataStream &ds)
      {
        teggo::XdocumentBuilder db;
        if ( TEGGOt_E e = DEFparse(&ds,&db) )
          {
            StringW err = _S*L"[XDOCUMENT&DEF ERROR] %?, docname '%?'" %TeggoErrorText(e) %ds.Identifier();
            Lerr << err;
            _E.Fail(err);
            return Xdocument(0);
          }
        return db.ProduceDocument();
      }

} // namespace teggo

#endif // ___803e9fb1_2535_4c15_b9d0_2507576a46cd___
