
/*

  Copyright (c) 2005-2009, Alexey Sudachen, alexey@sudachen.name

*/

#include "detect_compiler.h"
#if CXX_COMPILER_IS_MSVC_COMPATIBLE
#pragma code_seg(".text$classes")
#endif

#define _TEGGO_LZ77SS_HERE
#include "_specific.h"
//#include "lz77ss.h"
#include "lz77ss.inl"

extern "C" _TEGGO_EXPORTABLE void __teggo_classes_lz77ss_module() {};
