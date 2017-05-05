
/*

  Copyright (c) 2005-2009, Alexey Sudachen, alexey@sudachen.name

*/

#if defined _TEGGO_ZLIB_HERE || defined _TEGGOINLINE

#include <string.h>
#include <stdlib.h>

#define ZLIB_STATIC static

#ifdef __cplusplus

#if defined _TEGGOINLINE
# define _ZLIB_FAKE_INLINE ZLIB_STATIC CXX_FAKE_INLINE
# define _ZLIB_EXPORTABLE CXX_FAKE_INLINE
#else
# define _ZLIB_FAKE_INLINE ZLIB_STATIC
# define _ZLIB_EXPORTABLE extern "C" _TEGGO_EXPORTABLE
#endif

#else /* raw C */

#if defined _TEGGOINLINE
# define _ZLIB_FAKE_INLINE ZLIB_STATIC
# define _ZLIB_EXPORTABLE ZLIB_STATIC
#else
# define _ZLIB_FAKE_INLINE ZLIB_STATIC
# define _ZLIB_EXPORTABLE _TEGGO_EXPORTABLE
#endif

#endif

CXX_EXTERNC int compress(void *dest, u32_t *destLen, void const *source, u32_t sourceLen, int level);
CXX_EXTERNC int uncompress(void *dest, u32_t *destLen, void const *source, u32_t sourceLen);

_ZLIB_EXPORTABLE int zlib_compress_l(void const *in_b, int in_b_len, void *out_b, int out_b_len, int level)
  {
    u32_t l = out_b_len;
    int r = compress(out_b,&l,in_b,in_b_len,level);
    switch ( r )
      {
        case 0:
          return l;
        default:
          return -1;
      }
  }

_ZLIB_EXPORTABLE int zlib_compress(void const *in_b, int in_b_len, void *out_b, int out_b_len)
  {
    return zlib_compress_l(in_b,in_b_len,out_b,out_b_len,6);
  }

_ZLIB_EXPORTABLE int zlib_decompress(void const *in_b, int in_b_len, void *out_b, int out_b_len)
  {
    u32_t l = out_b_len;
    int r = uncompress(out_b,&l,in_b,in_b_len);
    switch ( r )
      {
        case 0:
          return l;
        default:
          return -1;
      }
  }

/*#else no here*/
#endif
