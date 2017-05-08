
/*

  Copyright (c) 2005-2009, Alexey Sudachen, alexey@sudachen.name

*/

#ifndef ___ccec6e05_9363_4b36_9628_f75bb5368d07___
#define ___ccec6e05_9363_4b36_9628_f75bb5368d07___

#include "_specific.h"

#if defined _TEGGOINLINE
#include "./zlib.inl"
#else
CXX_EXTERNC _TEGGO_EXPORTABLE int zlib_compress(void* in_b, int in_b_len, void* out_b, int out_b_len);
CXX_EXTERNC _TEGGO_EXPORTABLE int zlib_compress_l(void* in_b, int in_b_len, void* out_b, int out_b_len, int level);
CXX_EXTERNC _TEGGO_EXPORTABLE int zlib_decompress(void* in_b, int in_b_len, void* out_b, int out_b_len);
#endif

#endif /*___ccec6e05_9363_4b36_9628_f75bb5368d07___*/
