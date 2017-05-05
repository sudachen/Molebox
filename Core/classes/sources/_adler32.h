
/*

  Copyright (c) 2005-2009, Alexey Sudachen, alexey@sudachen.name

*/

#if !defined ___a33f5e10_fd2f_4ac4_93c4_25e70e1220cc___
#define ___a33f5e10_fd2f_4ac4_93c4_25e70e1220cc___

#if defined _WITHOUT_TFFCLS
#include "./detect_compiler.h"
#endif

#define _ADLER32_DO1(buf,i)  {s1 += buf[i]; s2 += s1;}
#define _ADLER32_DO2(buf,i)  _ADLER32_DO1(buf,i); _ADLER32_DO1(buf,i+1);
#define _ADLER32_DO4(buf,i)  _ADLER32_DO2(buf,i); _ADLER32_DO2(buf,i+2);
#define _ADLER32_DO8(buf,i)  _ADLER32_DO4(buf,i); _ADLER32_DO4(buf,i+4);
#define _ADLER32_DO16(buf)   _ADLER32_DO8(buf,0); _ADLER32_DO8(buf,8);

namespace teggo {

u32_t CXX_STDCALL Adler32(u32_t adler,const byte_t *buf,u32_t len);

#if defined _TEGGOINLINE || defined _TEGGO_ADLER_HERE || defined _ADLER32_LOCAL

CXX_EXTERNC u32_t adler32(u32_t adler,const byte_t *buf,u32_t len) { return Adler32(adler,buf,len); }

# if defined _TEGGOINLINE || defined _ADLER32_LOCAL
CXX_FAKE_INLINE
# endif

u32_t CXX_STDCALL Adler32(u32_t adler,const byte_t *buf,u32_t len)
  {
    enum
      {
        BASE = 65521L, /* largest prime smaller than 65536 */
        /* NMAX is the largest n such that 255n(n+1)/2 + (n+1)(BASE-1) <= 2^32-1 */
        NMAX = 5552
      };

    u32_t s1 = adler & 0xffff;
    u32_t s2 = (adler >> 16) & 0xffff;
    int k;

    if (buf == 0) return 1L;

    while (len > 0)
      {
        k = len < NMAX ? len : NMAX;
        len -= k;
        while (k >= 16)
          {
            _ADLER32_DO16(buf);
            buf += 16;
            k -= 16;
          }
        if (k != 0)
          do
            {
              s1 += *buf++;
              s2 += s1;
            }
          while (--k);
        s1 %= BASE;
        s2 %= BASE;
      }
    return (s2 << 16) | s1;
  }
#endif

#undef _ADLER32_DO1
#undef _ADLER32_DO2
#undef _ADLER32_DO4
#undef _ADLER32_DO8
#undef _ADLER32_DO16

} //namespace

#endif // ___a33f5e10_fd2f_4ac4_93c4_25e70e1220cc___
