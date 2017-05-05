
/*

  Copyright (c) 2005-2009, Alexey Sudachen, alexey@sudachen.name

*/

namespace teggo {

  _TEGGO_EXPORTABLE u32_t Shash(char const *t,int len = -1);

#if defined _TEGGO_SPECIFIC_HERE || defined _TEGGOINLINE

#if defined _TEGGO_SPECIFIC_HERE
_TEGGO_EXPORTABLE
#else
CXX_FAKE_INLINE
#endif

  u32_t Shash(char const *t,int len)
    {
      byte_t const *p = (byte_t const*)t;
      u32_t h = 0;
      if ( len < 0 )
        for ( ;*p; ++p )
          h = ( h << 1 ) | *p;
      else
        while ( len-- > 0 )
          h = ( h << 1 ) | *p++;
      return h;
    }

#endif

} // namespace
