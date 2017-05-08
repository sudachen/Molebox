
/*

  Copyright (c) 2005-2009, Alexey Sudachen, alexey@sudachen.name

*/

#ifndef ___87975bda_a67d_4053_9d8f_a1501f601d38___
#define ___87975bda_a67d_4053_9d8f_a1501f601d38___

#include "streams.h"

#if defined _TEGGO_STREAMS_HERE
# define _TEGGO_STREAMS_FAKE_INLINE _TEGGO_EXPORTABLE
#else
# define _TEGGO_STREAMS_FAKE_INLINE CXX_FAKE_INLINE
#endif

namespace teggo
{

    struct TextBuffer
    {
        DataStream& ds_;
        byte_t bS_[128];
        byte_t* bp_;
        int count_;
        int clen_;
        bool const part_;

//       int Get_()
//         {
//           return --count_,*bp_++;
//         }

        _TEGGO_EXPORTABLE TextBuffer(DataStream& ds,bool charByChar=false);
        _TEGGO_EXPORTABLE ~TextBuffer();
        _TEGGO_EXPORTABLE int Pump();
        _TEGGO_EXPORTABLE int GetC();
    };

    #if defined _TEGGO_STREAMS_HERE || defined _TEGGOINLINE
    _TEGGO_STREAMS_FAKE_INLINE
    int TextBuffer::Pump()
    {
        int nread;
        int n = part_?sizeof(bS_)-count_:1;
        if ( count_ ) memmove(bS_,bp_,count_);
        bp_ = bS_;
        nread = ds_.Read(bp_+count_,n);
        count_ += nread;
        //Lout << _S*"pump %d bytes" % nread;
        return count_;
    }

    _TEGGO_STREAMS_FAKE_INLINE
    int TextBuffer::GetC()
    {
        int clen;
        //int cc;

        wchar_t c = 0;

    repeat:

        clen = 0;

        do
        {
            if ( !count_ )
                if ( !Pump() )
                    return -1;
            ((byte_t*)&c)[clen] = *bp_++; --count_;
        }
        while ( ++clen <= clen_ );

        if ( c == '\r' )
            goto repeat;

        return (int)c;
    }

    _TEGGO_STREAMS_FAKE_INLINE
    TextBuffer::TextBuffer(DataStream& ds,bool charByChar)
        : ds_(ds), bp_(bS_), count_(0), clen_(0),
          part_(((ds.Features()&DataStream::CAN_SEEK)==DataStream::CAN_SEEK)&& !charByChar)
    {
        if ( ds_.Features() & DataStream::IS_UNICODE_MEMSTREAM )
            clen_ = sizeof(wchar_t) - 1;
    }

    _TEGGO_STREAMS_FAKE_INLINE
    TextBuffer::~TextBuffer()
    {
        if ( count_ && part_ )
            ds_.Seek(-count_,SEEK_CUR);
    }
    #endif

    #if !defined _TEGGO_STREAMS_HERE && !defined _TEGGOINLINE
    _TEGGO_EXPORTABLE int CXX_STDCALL TextBuffer_LocaleGetW(TextBuffer& tbf);
    _TEGGO_EXPORTABLE int CXX_STDCALL TextBuffer_UtfGetW(TextBuffer& tbf);
    #else
    _TEGGO_STREAMS_FAKE_INLINE
    int CXX_STDCALL TextBuffer_LocaleGetW(TextBuffer& tbf)
    {
        int c = tbf.GetC();
        if ( tbf.clen_ )
            return c;
        if ( c > 0 )
            return (u16_t)ucs2_btowc(c);
        return c;
    }

    _TEGGO_STREAMS_FAKE_INLINE
    int CXX_STDCALL TextBuffer_UtfGetW(TextBuffer& tbf)
    {
        int c = -1;
        int c0 = tbf.GetC();
        if ( c0 > 0 )
        {
            if (c0 < 0x80) return c0;
            int c1 = 0;
            int c2 = 0;
            int c3 = 0;
            int l = UtfGetCodeLength(c0);
            switch ( l )
            {
                case 2:
                    if ( (c1 = tbf.GetC()) > 0 )
                        c = ((c0 & 0x1f) << 6) + (c1 & 0x3f);
                    break;
                case 3:
                    if ( (c1 = tbf.GetC()) > 0
                         && (c2 = tbf.GetC()) > 0 )
                        c = ((c0 & 0x0f) << 12) + ((c1 & 0x3f) << 6) + (c2 & 0x3f);
                    break;
                case 4: // hm, UCS4 ????
                    if ( (c1 = tbf.GetC()) > 0
                         && (c2 = tbf.GetC()) > 0
                         && (c3 = tbf.GetC()) > 0 )
                        c = ((c0 & 0x7) << 18) + ((c0 & 0x3f) << 12) +
                            ((c2 & 0x3f) << 6) + (c3 & 0x3f);
                    break;
                default:
                    break;
            }
        }
        return c;
    }
    #endif

    struct UtfTextBuffer
    {
        TextBuffer tbf;
        UtfTextBuffer(DataStream& ds,bool charByChar=false) : tbf(ds,charByChar) {}
        int GetC() { return TextBuffer_UtfGetW(tbf); }
    };
}
#endif // ___87975bda_a67d_4053_9d8f_a1501f601d38___
