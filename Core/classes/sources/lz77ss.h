
/*

  Copyright (c) 2005-2009, Alexey Sudachen, alexey@sudachen.name

*/

#ifndef ___4d88f09e_0e1b_433b_9d95_d7984107b2bd___
#define ___4d88f09e_0e1b_433b_9d95_d7984107b2bd___

#include "_specific.h"

#if defined _TEGGOINLINE
#include "./lz77ss.inl"
#else
extern "C" _TEGGO_EXPORTABLE int lz77ss_compress(void *in_b, int in_b_len, void *out_b, int out_b_len);
extern "C" _TEGGO_EXPORTABLE int lz77ss_decompress(void *in_b, int in_b_len, void *out_b, int out_b_len);
#endif

#endif /*___4d88f09e_0e1b_433b_9d95_d7984107b2bd___*/
