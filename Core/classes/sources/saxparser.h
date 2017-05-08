
/*

  Copyright (c) 2005-2009, Alexey Sudachen, alexey@sudachen.name

*/

#ifndef ___5d72cb29_7852_411b_bfc9_c8a6d5970ebb___
#define ___5d72cb29_7852_411b_bfc9_c8a6d5970ebb___

#include "_specific.h"
#include "string.h"
#include "streams.h"
#include "refcounted.h"

namespace teggo
{

    struct SAX_TBF
    {
        TextBuffer tbf_;
        BufferT<wchar_t> buffer_;
        int iS_, iI_;
        wchar_t* last;
        struct
        {
            bool utf8 : 1;
        } f;

        SAX_TBF(DataStream& ds) : tbf_(ds)
        {
            f.utf8 = false;
            iS_ = 0;
            iI_ = 0;
            last = (wchar_t*)-1;
            buffer_.Reserve(256);
        }

        wchar_t* Begin() { return &buffer_[iS_]; }
        wchar_t* End() { return &buffer_[iI_]; }

        int Fill()
        {
            buffer_.Push(TextBuffer_LocaleGetW(tbf_));
            buffer_.Push(TextBuffer_LocaleGetW(tbf_));
            buffer_.Push(TextBuffer_LocaleGetW(tbf_));
            iS_ = 0;
            iI_ = 0;
            last = &buffer_[0];
            return *last;
        }

        CXX_NO_INLINE
        void Trunc(...)
        {
//           if ( iI_ > 128 && iS_ )
//             {
//               memmove(+buffer_,+buffer_+iI_,(buffer_.Count()-iI_)*sizeof(wchar_t));
//               iI_ = 0;
//             }
//           iS_ = iI_;
//           last = &buffer_[iI_];
//           buffer_.Resize(iI_+3);
            if ( iI_ != 0 )
                buffer_[0] = buffer_[iI_],
                             buffer_[1] = buffer_[iI_+1],
                                          buffer_[2] = buffer_[iI_+2];
            iS_ = iI_ = 0;
            last = &buffer_[iI_];
            buffer_.Resize(3);
        }

        wchar_t GetW_()
        {
            last[0] = last[1];
            last[1] = last[2];
            last[2] = ( f.utf8 )
                      ? TextBuffer_UtfGetW(tbf_)
                      : TextBuffer_LocaleGetW(tbf_);
            return *last;
        }

        CXX_NO_INLINE
        void TruncTo(wchar_t c,...)
        {
            Trunc();
            while ( *last != (wchar_t)-1 && *last != c ) GetW_();
            GetW_();
        }

        CXX_NO_INLINE
        void TruncSkipWS(...)
        {
            Trunc();
            while ( *last != (wchar_t)-1 && iswspace(*last) ) GetW_();
        }

        CXX_NO_INLINE
        wchar_t GetW(...)
        {
            int c = ( f.utf8 )
                    ? TextBuffer_UtfGetW(tbf_)
                    : TextBuffer_LocaleGetW(tbf_);
            //if ( c < 0 ) c = 0;
            ++iI_;
            buffer_.Push(c);
            last = &buffer_[iI_];
            return *last;
        }

        CXX_NO_INLINE
        void RemoveExtraWS(...)
        {
            for ( ; iS_ != iI_ && iswspace(buffer_[iS_]); ++iS_ )
            {}
            if ( iS_ == iI_ ) return;
            int i = iS_;
            for ( ; i != iI_ && !iswspace(buffer_[i]); ++i )
            {}
            ++i;
            int j = i;
            for ( ; i < iI_; ++i )
            {
                if ( buffer_[i] == '\n' || buffer_[i] == '\t' ) buffer_[i] = ' ';
                if ( iswspace(buffer_[i]) && iswspace(buffer_[i-1]) ) ;
                else
                    buffer_[j++] = buffer_[i];
            }

            if ( iswspace(buffer_[j-1]) ) --j;

            if ( j < iI_ )
            {
                int n = iI_ - j;
                for ( int i = iI_, iE = buffer_.Count(); i < iE; ++i )
                    buffer_[j++] = buffer_[i];
                buffer_.Resize(j);
                iI_ -= n;
                last = &buffer_[iI_];
            }
            else
                STRICT_REQUIRE( j == iI_ );
        }
    };

    struct XMLdocumentBuilder : Ireferred
    {
        XMLdocumentBuilder() {}
        virtual ~XMLdocumentBuilder() {}
        virtual TEGGOt_E BeginTag(pwide_t pS, pwide_t pE) = 0;
        virtual TEGGOt_E EndTag(pwide_t pS, pwide_t pE) = 0;
        virtual TEGGOt_E Body(pwide_t pS, pwide_t pE) = 0;
        virtual TEGGOt_E Param(pwide_t namS, pwide_t namE, pwide_t valS, pwide_t valE) = 0;
        virtual TEGGOt_E Comment(pwide_t pS, pwide_t pE) = 0;
        virtual TEGGOt_E CDATA(pwide_t pS, pwide_t pE) = 0;
        virtual TEGGOt_E BeginDocument() = 0;
        virtual TEGGOt_E EndDocument() = 0;
        TEGGOt_E BeginTag1(pwide_t p) { return BeginTag(p,p+wcslen(p)); }
        TEGGOt_E EndTag1(pwide_t p) { return EndTag(p,p+wcslen(p)); }
        TEGGOt_E Body1(pwide_t p) { return Body(p,p+wcslen(p)); }
        TEGGOt_E Param2(pwide_t nam, pwide_t val)
        { return Param(nam,nam+wcslen(nam),val,val+wcslen(val)); }
        TEGGOt_E Comment1(pwide_t p) { return Comment(p,p+wcslen(p)); }
        TEGGOt_E CDATA1(pwide_t p) { return CDATA(p,p+wcslen(p)); }
    };

    _TEGGO_EXPORTABLE TEGGOt_E CXX_STDCALL SAXparse(DataStream* ds, XMLdocumentBuilder* db);
    _TEGGO_EXPORTABLE TEGGOt_E CXX_STDCALL SAXparse(StringParam source, XMLdocumentBuilder* db);

    struct XMLdocumentWriter : RefcountedT<XMLdocumentBuilder>
    {
        rcc_ptr<DataStream> ds_;
        int state_;
        int tab_;
        _TEGGO_EXPORTABLE XMLdocumentWriter(DataStream* ds);
        _TEGGO_EXPORTABLE virtual ~XMLdocumentWriter();
        _TEGGO_EXPORTABLE virtual TEGGOt_E BeginTag(pwide_t pS, pwide_t pE);
        _TEGGO_EXPORTABLE virtual TEGGOt_E EndTag(pwide_t pS, pwide_t pE);
        _TEGGO_EXPORTABLE virtual TEGGOt_E Body(pwide_t pS, pwide_t pE);
        _TEGGO_EXPORTABLE virtual TEGGOt_E Param(pwide_t namS, pwide_t namE, pwide_t valS, pwide_t valE);
        _TEGGO_EXPORTABLE virtual TEGGOt_E Comment(pwide_t pS, pwide_t pE);
        _TEGGO_EXPORTABLE virtual TEGGOt_E CDATA(pwide_t pS, pwide_t pE);
        _TEGGO_EXPORTABLE virtual TEGGOt_E BeginDocument();
        _TEGGO_EXPORTABLE virtual TEGGOt_E EndDocument();
        void _Indent();
    };

    _TEGGO_EXPORTABLE rcc_ptr<XMLdocumentWriter> CXX_STDCALL OpenXMLdocWriter(StringParam fname);

} // namespace teggo

#if defined _TEGGOINLINE
#include "saxparser.inl"
#endif

#endif //___5d72cb29_7852_411b_bfc9_c8a6d5970ebb___
