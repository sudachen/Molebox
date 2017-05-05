
/*

  Copyright (c) 2005-2009, Alexey Sudachen, alexey@sudachen.name

*/

#ifndef ___45599678_f9b0_450f_8106_c63bc9cf7fc8___
#define ___45599678_f9b0_450f_8106_c63bc9cf7fc8___

#include "xdom.h"
#include "tuple.h"

#if defined _TEGGO_XDOM_HERE
# define _TEGGO_XDOM_FAKE_INLINE _TEGGO_EXPORTABLE
#else
# define _TEGGO_XDOM_FAKE_INLINE CXX_FAKE_INLINE
#endif

namespace teggo {

  _TEGGO_XDOM_FAKE_INLINE
    Xdocument_::Xdocument_( SymbolTable *symtable )
      : literal_( Refe(symtable) )
      {
        if ( !literal_ )
          literal_ = RccPtr(new SymbolTable());

        freezed_ = false;
        root_    = 0;
        literal_->GetSymbol(L"<?comment?>");
        literal_->GetSymbol(L"<?cdata?>");
        literal_->GetSymbol(L"<?body?>");
        literal_->GetSymbol(L"");
        pwide_t rootname = literal_->GetSymbol(L"<?root?>");
        root_    = new Xnode(rootname,0,this,&root_);
      }

  _TEGGO_XDOM_FAKE_INLINE
    Xdocument_::~Xdocument_()
      {
        root_->Erase();
      }

  _TEGGO_XDOM_FAKE_INLINE
    pwide_t Xdocument_::LookupLiteral(pwide_t l)
      {
        return literal_->FindSymbol(l);
      }

  static void __Notify( Xnode *x, Xnode *p, u32_t op, pwide_t a)
    {
      if ( XnodeObserver *xo = (XnodeObserver*)x->Pointer(TEGGO_GUIDOF(XnodeObserver)) )
        while ( xo )
          {
            XnodeObserver * xo_ = xo;
            xo = xo->next;
            xo_->OnModify(p,op,a);
          }
    }

  _TEGGO_XDOM_FAKE_INLINE
    void Xnode::_Notify( Xnode *p, u32_t op, pwide_t a)
      {
        __Notify(this,p,op,a);
        if ( parent_ )
          {
            if ( op == XnodeObserver::ON_ERASE )
              __Notify(parent_,p,op,a);
            __Notify(doc_->root_,p,op,a);
          }
        else
          REQUIRE( doc_->root_ == this );
      }

  _TEGGO_XDOM_FAKE_INLINE
    void Xnode::LinkObserver(rcc_ptr<XnodeObserver> const &xo)
      {
        XnodeObserver **p = (XnodeObserver**)this->Ppointer(TEGGO_GUIDOF(XnodeObserver));
        xo->LinkIn(p);
        Refe(xo);
      }

  _TEGGO_XDOM_FAKE_INLINE
    void Xnode::Erase()
      {
        REQUIRE( doc_ != 0 );

        _Notify(this,XnodeObserver::ON_ERASE,name_);

        while ( subnode_ )
          subnode_->Erase();

        REQUIRE( &subnode_ == psubnode_ );

        if ( next_ )
          next_->pnext_ = pnext_;
        else if (parent_)
          {
            REQUIRE ( parent_->psubnode_ == &next_ );
            parent_->psubnode_ = pnext_;
          }

        if ( pnext_ )
          *pnext_ = next_;

        parent_ = 0;
        delete this;
      }

  _TEGGO_XDOM_FAKE_INLINE
    Xnode *Xnode::Insert(pwide_t tag)
      {
        tag = doc_->literal_->GetSymbol(tag);
        return Insert_(tag);
      }

  _TEGGO_XDOM_FAKE_INLINE
    Xnode *Xnode::Insert(pwide_t tag,pwide_t pS, pwide_t pE)
      {
        tag = doc_->literal_->GetSymbol(tag);
        Xnode *n = Insert_(tag);
        n->SetParam(doc_->literal_->GetSymbol(L""),pS,pE);
        return n;
      }

  _TEGGO_XDOM_FAKE_INLINE
    Xnode *Xnode::GetNode(pwide_t tag)
      {
        pwide_t mytag = TranslateName_(tag);
        if ( mytag ) return FindTag_(mytag);
        return 0;
      }

  _TEGGO_XDOM_FAKE_INLINE
    pwide_t Xnode::GetContent()
      {
        pwide_t tag = doc_->literal_->GetSymbol(L"<?body?>");
        if ( Xnode *n = FindTag_(tag) )
          return n->GetParam(doc_->literal_->GetSymbol(L""));
        else
          return 0;
      }

  _TEGGO_XDOM_FAKE_INLINE
    void Xnode::SetContent(pwide_t value)
      {
        pwide_t tag = doc_->literal_->GetSymbol(L"<?body?>");
        pwide_t vE = value+wcslen(value);
        Xnode *nn = Insert(tag,value,vE);
        Xnode **n = &subnode_;

        while ( *n != nn )
          {
            if ( (*n)->name_ == tag )
              {
                (*n)->Erase();
              }
            else
              n = &(*n)->next_;
          }
      }

  _TEGGO_XDOM_FAKE_INLINE
    pwide_t Xnode::GetParam(pwide_t attr)
      {
        if ( pwide_t myattr = TranslateName_(attr) )
          {
            Xparam* attrval = FindAttr_(myattr);
            if ( attrval )
              return attrval->GetValue();
          }
        return 0;
      }

  _TEGGO_XDOM_FAKE_INLINE
    pwide_t Xnode::GetParam(StringParam param_name)
      {
        if ( pwide_t myattr = TranslateName_(+param_name) )
          {
            Xparam* attrval = FindAttr_(myattr);
            if ( attrval )
              return attrval->GetValue();
          }
        return 0;
      }

  _TEGGO_XDOM_FAKE_INLINE
    void Xnode::SetParam(pwide_t attr,pwide_t pS, pwide_t pE)
      {
        if ( pwide_t myattr = doc_->literal_->GetSymbol(attr) )
          {
            Xparam *attrval = FindAttr_(myattr);
            if ( !attrval )
              par_ = new Xparam(myattr,par_,pS,pE);
            else
              attrval->SetValue(pS,pE);
          }
        _Notify(this,XnodeObserver::ON_SETPARAM,attr);
      }

  _TEGGO_XDOM_FAKE_INLINE
    void Xnode::SetParam(pwide_t attr,pwide_t value)
      {
        pwide_t pE = value + wcslen(value);
        SetParam(attr,value,pE);
      }

  _TEGGO_XDOM_FAKE_INLINE
    void Xnode::SetParam(StringParam attr,StringParam value)
      {
        SetParam(+attr,+value);
      }

  _TEGGO_XDOM_FAKE_INLINE
    long Xnode::GetCountNodes(pwide_t tag)
      {
        if ( tag )
          {
            tag = TranslateName_(tag);
            if ( !tag ) return 0;
          }
        long count = 0;
        Xnode *node = subnode_;
        while ( node )
          {
            if ( !tag || node->name_ == tag ) ++count;
            node = node->next_;
          }
        return count;
      }

  _TEGGO_XDOM_FAKE_INLINE
    Xiterator Xnode::Iterate(pwide_t tag)
      {
        if ( tag )
          {
            tag = TranslateName_(tag);
            if ( !tag ) return Xiterator(0,0);
          }
        return Xiterator(this,tag);
      }

  _TEGGO_XDOM_FAKE_INLINE
    pwide_t Xnode::TranslateName_(pwide_t tag)
      {
        return doc_->literal_->FindSymbol(tag);
      }

  _TEGGO_XDOM_FAKE_INLINE
    bool Xnode::IsTag(pwide_t tag)
      {
        return wcsicmp(tag,this->name_) == 0;
      }

  _TEGGO_XDOM_FAKE_INLINE
    Xnode *Xnode::FindTag_(pwide_t tag)
      {
        Xnode *node = subnode_;
        while ( node )
          {
            if ( node->name_ == tag ) return node;
            node = node->next_;
          }
        return 0;
      }

  _TEGGO_XDOM_FAKE_INLINE
    Xparam *Xnode::FindAttr_(pwide_t name)
      {
        Xparam *attr = par_;
        while ( attr )
          {
            if ( attr->name == name ) return attr;
            attr = attr->next;
          }
        return 0;
      }

  _TEGGO_XDOM_FAKE_INLINE
    Xnode *Xnode::Insert_(pwide_t tag)
      {
        Xnode *node = new Xnode(tag,this,doc_,psubnode_);
        psubnode_ = &node->next_;
        _Notify(node,XnodeObserver::ON_INSERT,tag);
        return node;
      }

  _TEGGO_XDOM_FAKE_INLINE
    XnodeLinkedSupport_::XnodeLinkedSupport_(SymbolTable *st,guid_t const &_guid )
      : guid(_guid), mhb(st), cookie(0), next(0), void_pointer(0)
      {
      }

  _TEGGO_XDOM_FAKE_INLINE
    XnodeLinkedSupport_::~XnodeLinkedSupport_()
      {
        Unrefe(referred);
      }

  _TEGGO_XDOM_FAKE_INLINE
    XnodeLinkedSupport_ *& CXX_STDCALL XnodeLinkedSupport_::Find(
      XnodeLinkedSupport_ *&_l,guid_t const &guid)
      {
        XnodeLinkedSupport_ **l = &_l;
        for ( ; *l; l = &(*l)->next )
          if ( !memcmp(&(*l)->guid,&guid,sizeof(guid)) )
            break;
        return *l;
      }

  _TEGGO_XDOM_FAKE_INLINE
    XnodeLinkedSupport_ * CXX_STDCALL XnodeLinkedSupport_::Ensure(
      XnodeLinkedSupport_ *&_l,guid_t const &guid,SymbolTable *st)
      {
        XnodeLinkedSupport_ *&l = Find(_l,guid);
        if ( !l )
          l = new XnodeLinkedSupport_(st,guid);
        return l;
      }

  _TEGGO_XDOM_FAKE_INLINE
    Xnode::Xnode(pwide_t name,Xnode *parent,Xdocument_ *doc,Xnode **pnext)
      {
        name_   = name;
        parent_ = parent;
        doc_    = doc;
        xlsinfo_ = 0;
        weaklink_ = 0;

        if ( pnext )
          {
            REQUIRE( *pnext == 0 );
            *pnext = this;
          }
        pnext_ = pnext;

        par_ = 0;
        subnode_ = 0;
        psubnode_ = &subnode_;
        next_ = 0;
      }

  _TEGGO_XDOM_FAKE_INLINE
    Xnode::~Xnode()
      {

        REQUIRE( subnode_ == 0 );
        REQUIRE( parent_ == 0 );

        while ( par_ )
          {
            Xparam *a = par_;
            par_ = par_->next;
            delete a;
          }

        while ( xlsinfo_ )
          {
            XnodeLinkedSupport_ *l = xlsinfo_;
            xlsinfo_ = xlsinfo_->next;
            delete l;
          }

        for ( XnodeObserver **p = (XnodeObserver**)this->Ppointer(TEGGO_GUIDOF(XnodeObserver)); *p; )
          {
            XnodeObserver *x = *p;
            x->LinkOut();
            Unrefe(x);
          }

        WeakLink<Xnode*>::UnlinkAll(&weaklink_);

        doc_ = 0;
      }

  _TEGGO_XDOM_FAKE_INLINE
    rcc_ptr<> Xnode::Referred(guid_t const &guid)
      {
        if ( XnodeLinkedSupport_ *l = XnodeLinkedSupport_::Find(xlsinfo_,guid) )
          return l->referred;
        else
          return rcc_ptr<>(0);
      }

  _TEGGO_XDOM_FAKE_INLINE
    rcc_ptr<> Xnode::Referred(guid_t const &guid, rcc_ptr<> const &r)
      {
        rcc_ptr<> rval( r );
        XnodeLinkedSupport_ *l = XnodeLinkedSupport_::Ensure(xlsinfo_,guid,+doc_->literal_);
        rval.Swap(l->referred);
        return rval;
      }

  _TEGGO_XDOM_FAKE_INLINE
    u32_t Xnode::Cookie(guid_t const &guid)
      {
        if ( XnodeLinkedSupport_ *l = XnodeLinkedSupport_::Find(xlsinfo_,guid) )
          return l->cookie;
        else
          return 0;
      }

  _TEGGO_XDOM_FAKE_INLINE
    u32_t Xnode::Cookie(guid_t const &guid, u32_t cookie)
      {
        XnodeLinkedSupport_ *l = XnodeLinkedSupport_::Ensure(xlsinfo_,guid,+doc_->literal_);
        teggo_swap(cookie,l->cookie);
        return cookie;
      }


  _TEGGO_XDOM_FAKE_INLINE
    void *Xnode::Pointer(guid_t const &guid)
      {
        if ( XnodeLinkedSupport_ *l = XnodeLinkedSupport_::Find(xlsinfo_,guid) )
          return l->void_pointer;
        else
          return 0;
      }

  _TEGGO_XDOM_FAKE_INLINE
    void *Xnode::Pointer(guid_t const &guid, void *p)
      {
        XnodeLinkedSupport_ *l = XnodeLinkedSupport_::Ensure(xlsinfo_,guid,+doc_->literal_);
        teggo_swap(p,l->void_pointer);
        return p;
      }

  _TEGGO_XDOM_FAKE_INLINE
    void **Xnode::Ppointer(guid_t const &guid)
      {
        XnodeLinkedSupport_ *l = XnodeLinkedSupport_::Ensure(xlsinfo_,guid,+doc_->literal_);
        return &l->void_pointer;
      }

  _TEGGO_XDOM_FAKE_INLINE
    MessageHub &Xnode::MesgHub(guid_t const &guid)
      {
        return XnodeLinkedSupport_::Ensure(xlsinfo_,guid,+doc_->literal_)->mhb;
      }

  _TEGGO_XDOM_FAKE_INLINE
    MessageHub *Xnode::FindMesgHub(guid_t const &guid)
      {
        if ( XnodeLinkedSupport_ *l = XnodeLinkedSupport_::Find(xlsinfo_,guid) )
          return &l->mhb;
        else
          return 0;
      }


  _TEGGO_XDOM_FAKE_INLINE
    Xnode *Xnode::InsertCopyOf(Xnode *n, bool requce)
      {
        Xnode *cpy = Insert( n->GetName() );
        for ( Xparam *a = n->par_; a; a = a->next )
          cpy->SetParam( a->name, +a->value );
        if ( requce )
          {
            Xiterator i = n->Iterate();
            while ( i.Next() )
              cpy->InsertCopyOf( i.Get(), true );
          }
        else
          {
            Xiterator i = n->Iterate(L"<?body?>");
            while ( i.Next() )
              cpy->InsertCopyOf( i.Get(), false );
          }
        return cpy;
      }

  _TEGGO_XDOM_FAKE_INLINE
    void Xnode::Link( WeakLink<Xnode*> *l )
      {
        l->LinkIn( &weaklink_, this );
      }


  _TEGGO_XDOM_FAKE_INLINE
    Xnode *Xnode::GetNext(pwide_t tag)
      {
        Xnode *node = this->next_;
        if ( tag )
          tag = TranslateName_(tag);
        while ( node )
          {
            if ( !tag || node->name_ == tag ) return node;
            node = node->next_;
          }
        return 0;
      }

  _TEGGO_XDOM_FAKE_INLINE
    Xnode *Xiterator::FindNextWithTag(Xnode *node,pwide_t tag)
      {
        while ( node )
          {
            if ( !tag || node->name_ == tag ) return node;
            node = node->next_;
          }
        return 0;
      }

  _TEGGO_XDOM_FAKE_INLINE
    bool Xiterator::Next()
      {
        curr_ = next_;
        if ( next_ ) next_ = FindNextWithTag(next_->next_,tag_);
        return !!curr_;
      }

  _TEGGO_XDOM_FAKE_INLINE
    bool Xiterator::NextFiltered()
      {
        while ( Next() )
          if ( memcmp(curr_->name_,L"<?",sizeof(wchar_t)*2) )
            return true;
        return false;
      }

  _TEGGO_XDOM_FAKE_INLINE
    bool Xiterator::Reset()
      {
        if ( base_ )
          next_ = FindNextWithTag(base_->subnode_,tag_);
        else
          next_ = 0;
        curr_ = 0;
        return !!next_;
      }

  _TEGGO_XDOM_FAKE_INLINE
    XdocumentBuilder::XdocumentBuilder()
      {
      }

  _TEGGO_XDOM_FAKE_INLINE
    XdocumentBuilder::~XdocumentBuilder()
      {
      }

  _TEGGO_XDOM_FAKE_INLINE
    TEGGOt_E XdocumentBuilder::Source(StringParam)
      {
        return TEGGO_OK;
      }

  _TEGGO_XDOM_FAKE_INLINE
    TEGGOt_E XdocumentBuilder::BeginTag(pwide_t pS, pwide_t pE)
      {
        REQUIRE( tagstk_.Count() );
        StringT<wchar_t,32> _(pS,pE);
        //Lout << _S*"<%?>"%_;
        tagstk_.Push(tagstk_.Last()->Insert(+_));
        return TEGGO_OK;
      }

  _TEGGO_XDOM_FAKE_INLINE
    TEGGOt_E XdocumentBuilder::EndCurTag()
      {
        REQUIRE( tagstk_.Count() );
        return EndTag1(tagstk_.Last()->GetName());
      }

  _TEGGO_XDOM_FAKE_INLINE
    TEGGOt_E XdocumentBuilder::EndTag(pwide_t pS, pwide_t pE)
      {
        REQUIRE( tagstk_.Count() );
        if ( pS != 0 && pE != 0 )
          {
            StringT<wchar_t,32> _(pS,pE);
            //Lout << _S*"</%?>"%_;
            Xnode *n = tagstk_.Last();
            bool here = false;
            while ( tagstk_.Count() > 1 && !here )
              {
                here = n->IsTag(+_);
                tagstk_.Pop();
              }
          }
        else
          tagstk_.Pop();
        return TEGGO_OK;
      }

  _TEGGO_XDOM_FAKE_INLINE
    TEGGOt_E XdocumentBuilder::Body(pwide_t pS, pwide_t pE)
      {
        REQUIRE( tagstk_.Count() );
        tagstk_.Last()->Insert(L"<?body?>",pS,pE);
        return TEGGO_OK;
      }

  _TEGGO_XDOM_FAKE_INLINE
    TEGGOt_E XdocumentBuilder::Param(pwide_t namS, pwide_t namE, pwide_t valS, pwide_t valE)
      {
        REQUIRE( tagstk_.Count() );
        StringT<wchar_t,32> _(namS,namE);
        tagstk_.Last()->SetParam(+_,valS,valE);
        return TEGGO_OK;
      }

  _TEGGO_XDOM_FAKE_INLINE
    TEGGOt_E XdocumentBuilder::Comment(pwide_t pS, pwide_t pE)
      {
        REQUIRE( tagstk_.Count() );
        tagstk_.Last()->Insert(L"<?comment?>",pS,pE);
        return TEGGO_OK;
      }

  _TEGGO_XDOM_FAKE_INLINE
    TEGGOt_E XdocumentBuilder::CDATA(pwide_t pS, pwide_t pE)
      {
        REQUIRE( tagstk_.Count() );
        tagstk_.Last()->Insert(L"<?cdata?>",pS,pE);
        return TEGGO_OK;
      }

  _TEGGO_XDOM_FAKE_INLINE
    TEGGOt_E XdocumentBuilder::BeginDocument()
      {
        document_ = Xdocument(new Xdocument_());
        tagstk_.Clear();
        tagstk_.Push (document_->GetRoot());
        return TEGGO_OK;
      }

  _TEGGO_XDOM_FAKE_INLINE
    TEGGOt_E XdocumentBuilder::EndDocument()
      {
        REQUIRE( tagstk_.Count() );
        tagstk_.Clear();
        return TEGGO_OK;
      }

  _TEGGO_XDOM_FAKE_INLINE
    Xdocument XdocumentBuilder::ProduceDocument()
      {
        Xdocument doc = document_;
        Unrefe(document_);
        tagstk_.Clear();
        return doc;
      }

  _TEGGO_XDOM_FAKE_INLINE
    TEGGOt_E CXX_STDCALL XDocReproduce(Xdocument doc, XMLdocumentBuilder *db)
      {
        if ( doc )
          {
            pwide_t body_tag = doc->LookupLiteral(L"<?body?>");
            pwide_t comment_tag = doc->LookupLiteral(L"<?comment?>");

            BufferT< Tuple2<Xnode *,Xiterator> > nstk;
            db->BeginDocument();
            nstk.Push(tuple2(doc->GetRoot(),doc->GetRoot()->Iterate()));

            while ( !nstk.Empty() )
              if ( nstk->_1.Next() )
                {
                  Xnode *n = nstk->_1.Get();
                  pwide_t tag = n->GetName();
                  if ( tag == body_tag )
                    {
                      if ( pwide_t val = n->GetParam(L"") )
                        db->Body1( val );
                    }
                  else if ( tag == comment_tag )
                    {
                      if ( pwide_t val = n->GetParam(L"") )
                        db->Comment1( val );
                    }
                  else
                    {
                      db->BeginTag1(tag);
                      Xparam const *par = n->GetFirstParam();
                      for ( ; par ; par = par->next )
                        db->Param2(par->name,+par->value);
                      if ( n->HasChildren() )
                        {
                          //db->StartTagContent();
                          nstk.Push(tuple2(n,n->Iterate()));
                        }
                      else
                        db->EndTag(0,0);
                    }
                }
              else
                {
                  if ( nstk.Count() > 1 )
                    db->EndTag1(nstk->_0->GetName());
                  nstk.Pop();
                }

            db->EndDocument();
          }
        else
          return TEGGO_NODATA;

        return TEGGO_OK;
      }

  _TEGGO_XDOM_FAKE_INLINE
    Xdocument CXX_STDCALL XopenXmlDocument(StringParam docname)
      {
        teggo::XdocumentBuilder db;
        if ( TEGGOt_E e = teggo::SAXparse(docname,&db) )
          {
            StringW err = _S*L"[XDOCUMENT&SAX ERROR] %?, docname '%?'" %TeggoErrorText(e) %+docname;
            Lerr << err;
            _E.Fail(err);
            return Xdocument(0);
          }
        return db.ProduceDocument();
      }

  _TEGGO_XDOM_FAKE_INLINE
    Xdocument CXX_STDCALL XopenXmlDocument(DataStream &ds)
      {
        teggo::XdocumentBuilder db;
        if ( TEGGOt_E e = teggo::SAXparse(&ds,&db) )
          {
            StringW err = _S*L"[XDOCUMENT&SAX ERROR] %?, docname '%?'" %TeggoErrorText(e) %ds.Identifier();
            Lerr << err;
            _E.Fail(err);
            return Xdocument(0);
          }
        return db.ProduceDocument();
      }

} // namespace teggo

#endif // ___45599678_f9b0_450f_8106_c63bc9cf7fc8___
