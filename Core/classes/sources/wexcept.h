
/*

  Copyright (c) 2005-2009, Alexey Sudachen, alexey@sudachen.name

*/

#if !defined __E48D916E_9149_4F44_9FA5_10FA6930CFD5__
#define __E48D916E_9149_4F44_9FA5_10FA6930CFD5__

#include "_specific.h"
#include "format.h"
#include "string.h"

namespace teggo
{
    struct WideException
    {
        WideException(pwide_t what=0) : what_(what) {}
        WideException(FormatT<wchar_t> const& fwhat) { fwhat.Write(what_); }
        virtual pwide_t What() const { return what_?what_.Str():L"unknown wexception"; }
        virtual ~WideException() {}

        widestring what_;
    };
}

#endif // __E48D916E_9149_4F44_9FA5_10FA6930CFD5__
