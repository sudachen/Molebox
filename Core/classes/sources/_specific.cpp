
/*

  Copyright (c) 2005-2009, Alexey Sudachen, alexey@sudachen.name

*/


#define _Nomemory _Nomemory__

#include "detect_compiler.h"
#if CXX_COMPILER_IS_MSVC_COMPATIBLE
#pragma code_seg(".text$classes")
//#pragma data_seg(".data$classes")
#endif

#define _TEGGO_SPECIFIC_HERE
#include "_specific.h"
#include "_specific.inl"

#undef _Nomemory

namespace std
{
    void _Nomemory(void) {}
}