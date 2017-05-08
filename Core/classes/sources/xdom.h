
/*

  Copyright (c) 2005-2009, Alexey Sudachen, alexey@sudachen.name

*/

#ifndef ___a1ae834d_04f5_4c5a_a2f1_e628d30736e0___
#define ___a1ae834d_04f5_4c5a_a2f1_e628d30736e0___

#include "_specific.h"
#include "string.h"
#include "flatset.h"
#include "saxparser.h"
#include "messages.h"
#include "symboltable.h"
#include "weaklink.h"

namespace teggo
{

    struct XdataMemObject
    {
        enum { RECSIZE = 32 + 8 };
        static void* operator new(size_t sz)
        {
            REQUIRE(sz <= RECSIZE);
            return new byte_t[RECSIZE];
        }
        static void operator delete(void* p)
        {
            if ( p )
                delete[] (byte_t*)p;
        }
    };

    struct Xparam : XdataMemObject
    {
        pwide_t name;
        Xparam* next;
        StringT<wchar_t,11> value;

        Xparam(pwide_t name,Xparam* next,pwide_t value)
        {
            this->name = name;
            this->next = next;
            this->value = value;
        }

        Xparam(pwide_t name,Xparam* next,pwide_t pS, pwide_t pE)
        {
            this->name = name;
            this->next = next;
            this->value.SetRange(pS,pE);
        }

        void SetValue(pwide_t value)  { this->value = value; }
        pwide_t GetValue() const      { return this->value.Str();  }
        void SetValue(pwide_t pS, pwide_t pE)  { this->value.SetRange(pS,pE); }

    private:

        Xparam( Xparam const&);
        Xparam& operator=( Xparam const&);

    };

    struct Xnode;
    struct Xiterator;

    typedef GuidT<0x30cdf0ef,0x6aca,0x4347,0xb0,0x3e,0x85,0xc9,0x67,0x73,0x57,0x64> XDOUCMENT_CONTROL_UID;
    enum { XDOCEVT_PARAM, XDOCEVT_INSERT, XDOCEVT_ERASE, XDOCEVT_DISPOSE };

    struct Xdocument_ : RefcountedT<XdataMemObject>
    {
        rcc_ptr<SymbolTable> literal_;
        Xnode* root_;
        StringT<wchar_t> source_;
        struct
        {
            bool freezed_ : 1;
        };

        _TEGGO_EXPORTABLE Xdocument_( SymbolTable* = 0 );
        _TEGGO_EXPORTABLE ~Xdocument_();
        _TEGGO_EXPORTABLE pwide_t LookupLiteral(pwide_t);

        SymbolTable* GetSymbolTable() { return +literal_; }
        Xnode* GetRoot() { return +root_; }
        pwide_t GetSource() { return +source_; }
        void SetSource(pwide_t src) { source_ = src; }

    private:
        Xdocument_( Xdocument_ const&);
        Xdocument_& operator =( Xdocument_ const&);
    };

//   struct XsupportInfo : XdataMemObject
//     {
//       MessageHub signotify;
//       MessageHub sigaction;
//       void *cookie;
//       rcc_ptr<Ireferred> referred_;
//       _TEGGO_EXPORTABLE XsupportInfo( SymbolTable *st );
//       _TEGGO_EXPORTABLE ~XsupportInfo();
//     };

    struct XnodeLinkedSupport_ : XdataMemObject
    {
        guid_t guid;
        MessageHub mhb;
        rcc_ptr<> referred;
        u32_t cookie;
        XnodeLinkedSupport_* next;
        void* void_pointer;

        _TEGGO_EXPORTABLE XnodeLinkedSupport_(SymbolTable* st, guid_t const&);
        _TEGGO_EXPORTABLE ~XnodeLinkedSupport_();
        _TEGGO_EXPORTABLE static XnodeLinkedSupport_*& CXX_STDCALL Find(
            XnodeLinkedSupport_*& l,guid_t const& guid);
        _TEGGO_EXPORTABLE static XnodeLinkedSupport_* CXX_STDCALL Ensure(
            XnodeLinkedSupport_*& l,guid_t const& guid,SymbolTable* st);
    };

    struct XnodeObserver : InternList<XnodeObserver,Refcounted>
    {
        typedef GuidT<0x25d1dad2,0xe33f,0x4060,0x9f,0x58,0x7a,0x1a,0x36,0x12,0x70,0xb1> Guid;
        enum OPERATION_ { ON_DISPOSE, ON_ERASE, ON_PARAM, ON_INSERT, ON_SETPARAM };

        virtual void OnModify(Xnode* n, u32_t op, pwide_t a = 0) = 0;
    };

    struct Xnode : XdataMemObject
    {

        typedef GuidT<0x25bd462a,0x86af,0x49e0,0xb0, 0x58, 0xe7, 0x80, 0xce, 0x88, 0x6a, 0x1b> Guid;

        _TEGGO_EXPORTABLE void LinkObserver(rcc_ptr<XnodeObserver> const&);

        _TEGGO_EXPORTABLE rcc_ptr<> Referred(guid_t const&);
        _TEGGO_EXPORTABLE rcc_ptr<> Referred(guid_t const&, rcc_ptr<> const&);
        _TEGGO_EXPORTABLE u32_t Cookie(guid_t const&);
        _TEGGO_EXPORTABLE u32_t Cookie(guid_t const&, u32_t);
        _TEGGO_EXPORTABLE MessageHub& MesgHub(guid_t const&);
        _TEGGO_EXPORTABLE MessageHub* FindMesgHub(guid_t const&);
        _TEGGO_EXPORTABLE void** Ppointer(guid_t const&);
        _TEGGO_EXPORTABLE void* Pointer(guid_t const&);
        _TEGGO_EXPORTABLE void* Pointer(guid_t const&, void*);
//       _TEGGO_EXPORTABLE void *Pointer(guid_t const &, int no);
//       _TEGGO_EXPORTABLE void *Pointer(guid_t const &, int no, void*);

        template <class tTx>
        rcc_ptr<tTx> SetReferred(tTx* p)
        { return rcc_cast<tTx>(Referred(TEGGO_GUIDOF(tTx),RccRefe<Ireferred>(p))); }

        _TEGGO_EXPORTABLE void  Erase();
        _TEGGO_EXPORTABLE Xnode* Clone();
        _TEGGO_EXPORTABLE Xnode* Insert(pwide_t tag);
        _TEGGO_EXPORTABLE Xnode* Insert(pwide_t tag,pwide_t pS,pwide_t pE);
        _TEGGO_EXPORTABLE Xnode* InsertCopyOf(Xnode*,bool = false);
        _TEGGO_EXPORTABLE Xnode* GetNode(pwide_t tag);
        _TEGGO_EXPORTABLE pwide_t GetParam(pwide_t par);
        _TEGGO_EXPORTABLE pwide_t GetParam(StringParam);
        _TEGGO_EXPORTABLE void SetParam(StringParam par,StringParam value);
        _TEGGO_EXPORTABLE void SetParam(pwide_t par,pwide_t value);
        _TEGGO_EXPORTABLE void SetParam(pwide_t par,pwide_t pS, pwide_t pE);
        _TEGGO_EXPORTABLE long GetCountNodes(pwide_t tag);
        _TEGGO_EXPORTABLE Xiterator   Iterate(pwide_t tag=0);
        _TEGGO_EXPORTABLE bool IsTag(pwide_t tag);

        rcc_ptr<Xdocument_> GetDocument() { return rcc_ptr<Xdocument_>(doc_,true); }
        Xnode* GetRoot()   { return doc_->GetRoot(); }
        Xnode* GetParent() { return parent_; }
        pwide_t GetName()  { return name_; }
        bool HasChildren() { return subnode_ != 0; }
        Xparam const* GetFirstParam() { return par_; }
        Xnode* GetSubNode() { return subnode_; }


        _TEGGO_EXPORTABLE void Link( WeakLink<Xnode*>* l );
        _TEGGO_EXPORTABLE pwide_t GetContent();
        _TEGGO_EXPORTABLE void SetContent(pwide_t);
        inline void SetContent(StringParam s) { SetContent(+s); }

        _TEGGO_EXPORTABLE Xnode* GetNext(pwide_t tag);
        _TEGGO_EXPORTABLE pwide_t LookupLiteral(pwide_t l) { return doc_->LookupLiteral(l); }

        bool Empty() { return !subnode_ && !par_; }

    protected:
        friend struct Xdocument_;
        friend struct Xiterator;

        pwide_t    name_;
        Xdocument_* doc_;
        Xparam*    par_;
        Xnode*     subnode_;
        Xnode*     next_;
        Xnode**    pnext_;
        Xnode*     parent_;
        Xnode**    psubnode_;

        XnodeLinkedSupport_* xlsinfo_;
        WeakLink<Xnode*>* weaklink_;

        _TEGGO_EXPORTABLE pwide_t   TranslateName_(pwide_t tag);
        _TEGGO_EXPORTABLE Xnode*    FindTag_(pwide_t tag);
        _TEGGO_EXPORTABLE Xparam*   FindAttr_(pwide_t name);
        _TEGGO_EXPORTABLE Xnode*    Insert_(pwide_t tag);

        _TEGGO_EXPORTABLE Xnode(pwide_t name,Xnode* parent,Xdocument_ *doc,Xnode** pnext);
        _TEGGO_EXPORTABLE ~Xnode();

    private:

        _TEGGO_EXPORTABLE void _Notify( Xnode* p, u32_t op, pwide_t a);

        Xnode( Xnode const&);
        Xnode& operator=( Xnode const&);
    };

    struct Xiterator
    {
        _TEGGO_EXPORTABLE bool Next();
        _TEGGO_EXPORTABLE bool NextFiltered();
        _TEGGO_EXPORTABLE bool Reset();

        Xnode* Get() { return curr_; }
        Xnode* operator->() { return curr_; }
        Xnode& operator*() { return *curr_; }
        Xnode* operator+() { return curr_; }

        Xiterator()
        {
            tag_    = 0;
            base_   = 0;
            next_   = 0;
            curr_   = 0;
        }

        Xiterator(Xnode* base,pwide_t tag=0)
        {
            tag_  = tag;
            base_ = base;
            Reset();
        }

    private:
        Xnode* base_;
        Xnode* curr_;
        Xnode* next_;
        pwide_t  tag_;
        _TEGGO_EXPORTABLE static Xnode* CXX_STDCALL FindNextWithTag(Xnode* node,pwide_t tag);
    };

    typedef rcc_ptr<Xdocument_> Xdocument;
    _TEGGO_EXPORTABLE Xdocument CXX_STDCALL XopenXmlDocument(StringParam);
    _TEGGO_EXPORTABLE Xdocument CXX_STDCALL XopenXmlDocument(DataStream&);

    inline Xdocument Xdocument_New(SymbolTable* st = 0) { return Xdocument( new Xdocument_(st) ); }

    struct XdocumentBuilder : RefcountedT<XMLdocumentBuilder>
    {
        Xdocument document_;
        BufferT<Xnode*> tagstk_;

        _TEGGO_EXPORTABLE XdocumentBuilder();
        _TEGGO_EXPORTABLE virtual ~XdocumentBuilder();
        _TEGGO_EXPORTABLE virtual TEGGOt_E BeginTag(pwide_t pS, pwide_t pE);
        _TEGGO_EXPORTABLE virtual TEGGOt_E EndTag(pwide_t pS, pwide_t pE);
        _TEGGO_EXPORTABLE virtual TEGGOt_E Body(pwide_t pS, pwide_t pE);
        _TEGGO_EXPORTABLE virtual TEGGOt_E Param(pwide_t namS, pwide_t namE, pwide_t valS, pwide_t valE);
        _TEGGO_EXPORTABLE virtual TEGGOt_E Comment(pwide_t pS, pwide_t pE);
        _TEGGO_EXPORTABLE virtual TEGGOt_E CDATA(pwide_t pS, pwide_t pE);
        _TEGGO_EXPORTABLE virtual TEGGOt_E BeginDocument();
        _TEGGO_EXPORTABLE virtual TEGGOt_E EndDocument();
        _TEGGO_EXPORTABLE virtual TEGGOt_E Source(StringParam);
        _TEGGO_EXPORTABLE virtual TEGGOt_E EndCurTag();
        _TEGGO_EXPORTABLE Xdocument ProduceDocument();

    };

    _TEGGO_EXPORTABLE TEGGOt_E CXX_STDCALL XDocReproduce(Xdocument doc, XMLdocumentBuilder* db);

} // namsepace teggo

template <class tTx>
teggo::rcc_ptr<tTx> XnodeReferred( teggo::Xnode* n, tTx* _ =0 )
{
    return rcc_cast<tTx>(n->Referred(TEGGO_GUIDOF(tTx)));
}

#if defined _TEGGOINLINE
#include "xdom.inl"
#endif

#endif //___a1ae834d_04f5_4c5a_a2f1_e628d30736e0___
