
/*

  Copyright (c) 2005-2009, Alexey Sudachen, alexey@sudachen.name

*/

#if !defined __8A26ECE9_C113_4110_9252_CACCAD8E56AB__
#define __8A26ECE9_C113_4110_9252_CACCAD8E56AB__

#include "_specific.h"

namespace teggo
{

    template <class tTx, unsigned tCount, bool tCheckit = true>
    struct StackT
    {
        byte_t buffer_[sizeof(tTx)*tCount];
        unsigned p_;
        StackT() : p_(0) {}

        ~StackT()
        {
            while ( !Empty() ) Pop();
        }

        StackT& Push( const tTx& val )
        {
            if ( tCheckit )
                VIOLATION( ( p_ >= sizeof(buffer_) ),
                           L"stack overflow, size limit reached" );
            new (&buffer_[p_]) tTx(val);
            p_ += sizeof(tTx);
            return *this;
        }

        void Pop()
        {
            if ( tCheckit )
                VIOLATION( ( p_ == 0 ),
                           L"stack underflow" );
            p_ -= sizeof(tTx);
            ((tTx*)&buffer_[p_])->~tTx();
        }

        tTx& Top()
        {
            return *(tTx*)&buffer_[p_ - sizeof(tTx)];
        }

        tTx& operator *()
        {
            return Top();
        }

        tTx* operator ->()
        {
            return &Top();
        }

        bool Empty()
        {
            return p_ == 0;
        }

        unsigned Size()
        {
            return p_ / sizeof(tTx);
        }

        tTx& operator [](unsigned i)
        {
            return *(tTx*)&buffer_[sizeof(tTx)*i];
        }
    };

} // namespace

#endif
