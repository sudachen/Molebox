
/*

  Copyright (c) 2005-2009, Alexey Sudachen, alexey@sudachen.name

*/

#if !defined __FE8D5BD0_D76C_4886_96D9_98B851B5336C__
#define __FE8D5BD0_D76C_4886_96D9_98B851B5336C__

#include "_specific.h"
#include "string.h"

namespace teggo
{

    template<class tTchr>
    struct SplitterT
    {
        typedef tTchr const* Tstr;
        bool Reset();
        Tstr Next();
        Tstr Get() const;
        Tstr Get(long no) const;
        u32_t Count() const;
        SplitterT();
        SplitterT(Tstr text,tTchr delim);
        ~SplitterT();
        SplitterT(const SplitterT&);
        SplitterT& operator=(const SplitterT&);
        operator bool() const { return count_ != 0; }
        bool operator!() const { return count_ == 0; }
    private:
        tTchr* text_;
        tTchr** ptrs_;
        long  count_;
        long  curr_;
    };

    typedef SplitterT<wchar_t> widesplitter;
    typedef SplitterT<char>    asciisplitter;

    template<class tTchr>
    bool SplitterT<tTchr>::Reset()
    {
        curr_ = -1;
        return count_ > 0;
    }

    template<class tTchr>
    const tTchr* SplitterT<tTchr>::Next()
    {
        return ( curr_+1 < count_ ? ( ++curr_, ptrs_[curr_] ) : 0 );
    }

    template<class tTchr>
    const tTchr* SplitterT<tTchr>::Get() const
    {
        return ( curr_ >=0 && curr_ < count_  ? ptrs_[curr_] : 0 );
    }

    template<class tTchr>
    const tTchr* SplitterT<tTchr>::Get(long no) const
    {
        return ( no >= 0 && no < count_ ? ptrs_[no] : 0 );
    }

    template<class tTchr>
    u32_t SplitterT<tTchr>::Count() const
    {
        return count_;
    }

    template<class tTchr>
    SplitterT<tTchr>::SplitterT() : text_(0), count_(0), curr_(-1), ptrs_(0)
    {
    }

    template<class tTchr> CXX_NO_INLINE
    SplitterT<tTchr>::SplitterT(Tstr text,tTchr delim) : text_(0), count_(0), curr_(-1), ptrs_(0)
    {
        if ( text )
        {
            long textlen = StrLen(text);
            if ( textlen )
            {
                Tstr p  = text;
                Tstr pE = p+textlen;

                for ( ; p != pE ; ++p )
                    if ( *p == delim )
                        ++count_;
                ++count_;

                long buffsize = (textlen+1)*sizeof(tTchr)+count_*sizeof(tTchr**)+1*sizeof(long);
                byte_t* buff = (byte_t*)BaseStringT<tTchr>::widestring_malloc(buffsize);
                *(long*)buff = buffsize;
                ptrs_ = (tTchr**)(buff+sizeof(long));
                text_ = (tTchr*)(buff+sizeof(long)+sizeof(tTchr**)*count_);
                memcpy(text_,text,sizeof(tTchr)*(textlen+1));
                long id = 1;
                ptrs_[0] = text_;

                if ( count_ > 1 )
                    for ( tTchr* p = text_, *pE = text_+textlen ; p != pE ; ++p )
                        if ( *p == delim ) ptrs_[id++] = p+1, *p = 0;
            }
        }
    }

    template<class tTchr> CXX_NO_INLINE
    SplitterT<tTchr>::~SplitterT()
    {
        if ( ptrs_ )
            BaseStringT<tTchr>::widestring_free((byte_t*)ptrs_-sizeof(long));
        ptrs_ = 0,text_ = 0, count_ = 0;
    }

    template<class tTchr>
    SplitterT<tTchr>::SplitterT(const SplitterT& a)
        : text_(0), count_(0), curr_(-1), ptrs_(0)
    {
        operator=(a);
    }

    template<class tTchr> CXX_NO_INLINE
    SplitterT<tTchr>& SplitterT<tTchr>::operator=(const SplitterT& a)
    {
        if ( ptrs_ )
            BaseStringT<tTchr>::widestring_free((byte_t*)ptrs_-sizeof(long));

        ptrs_ = 0,text_ = 0, count_ = 0;

        if ( a.ptrs_ )
        {
            long bufsize = *(long*)((byte_t*)a.ptrs_-sizeof(long));
            byte_t* buff = (byte_t*)BaseStringT<tTchr>::widestring_malloc(bufsize);
            memcpy(buff,((byte_t*)a.ptrs_-sizeof(long)),bufsize);
            ptrs_ = (tTchr**)((byte_t*)buff+sizeof(long));
        }

        text_ = (tTchr*)((byte_t*)ptrs_ + ((byte_t*)a.text_ - (byte_t*)a.ptrs_));
        curr_ = -1;
        count_ = a.count_;

        for ( int i = 0; i < count_; ++i )
            ptrs_[i] = text_ + ( ptrs_[i] - a.text_ );

        return *this;
    }

} // namespace

#endif // __FE8D5BD0_D76C_4886_96D9_98B851B5336C__
