
#include "../classes.h"
#include "./rsa.h"

extern "C" void long_exp_mod( u32_t *p, u32_t *e, u32_t *m, u32_t *r, u32_t RSA_VALUE_BITS )
  {
/*
    r = 1
    t = p
    while e:
        if e & 1:
            r = (r * t) % mod
        e = e >> 1
        t = (t * t) % mod
    return r
*/
    u32_t const RSA_VALUE_BYTES = RSA_VALUE_BITS/8;

    u32_t *tR = (u32_t*)alloca(RSA_VALUE_BYTES);
    memcpy(tR,p,RSA_VALUE_BYTES);

    u32_t *aR = (u32_t*)alloca(RSA_VALUE_BYTES);
    memset(aR,0,RSA_VALUE_BYTES);
    aR[0] = 1;

    u32_t eMaxBit = _lv_maxbit(e,RSA_VALUE_BYTES);

    for ( int i = 0; i <= eMaxBit; ++i )
      {
        if ( e[i/32] & (1 << (i%32)) )
          RSA_mulmod( aR, tR, m, aR, RSA_VALUE_BITS );
        RSA_mulmod( tR, tR, m, tR, RSA_VALUE_BITS );
      }

    memcpy(r,aR,RSA_VALUE_BYTES);
  }

extern "C" void long_mul_mod( u32_t *a, u32_t *b, u32_t *m, u32_t *r, u32_t RSA_VALUE_BITS )
  {
    u32_t const RSA_VALUE_BYTES = RSA_VALUE_BITS/8;

    u32_t *rR = (u32_t*)alloca(RSA_VALUE_BYTES*2);
    u32_t *aR = (u32_t*)alloca(RSA_VALUE_BYTES*2);

    _lv_mul(a,b,aR,RSA_VALUE_BYTES);

    /* mod */

    u32_t aMaxBit = _lv_maxbit(aR,RSA_VALUE_BYTES*2) + 1;
    u32_t mMaxBit = _lv_maxbit(m,RSA_VALUE_BYTES) + 1;

    //printf("mMaxBit %d, aMaxBit %d\n",mMaxBit,aMaxBit);
    if ( mMaxBit > aMaxBit )
      {
        memcpy(r,aR,RSA_VALUE_BYTES);
      }
    else
      {
        memset(rR,0,RSA_VALUE_BYTES*2);
        //__asm int 3;

        if ( int i = RSA_VALUE_BYTES*2*8-aMaxBit )
          _lv_sshl(i,aR,RSA_VALUE_BYTES*2,rR,RSA_VALUE_BYTES*2);
        _lv_sshl(mMaxBit,aR,RSA_VALUE_BYTES*2,rR,RSA_VALUE_BYTES*2);
        for( u32_t i = aMaxBit - mMaxBit;;)
          {
            while ( !_lv_less(rR,RSA_VALUE_BYTES+4,m,RSA_VALUE_BYTES) )
              _lv_sub(m,RSA_VALUE_BYTES,rR,RSA_VALUE_BYTES+4);
            if ( i-- )
              _lv_sshl(1,aR,RSA_VALUE_BYTES*2,rR,RSA_VALUE_BYTES*2);
            else
              break;
          }
        memcpy(r,rR,RSA_VALUE_BYTES);
      }
  }

static u32_t _lv_greater0( u32_t *a, u32_t l )
  {
    do
      if ( *a++ ) return 1;
    while ( l -= 4 );
    return 0;
  }

extern "C" u32_t  _lv_less( u32_t *a, u32_t al, u32_t *b, u32_t bl )
  {
    int l = al;
    if ( al < bl )
      { if ( _lv_greater0(b+al/4,bl-al) ) return 1; }
    else if ( al > bl )
      {
        if ( _lv_greater0(a+bl/4,al-bl) ) return 0;
        l = bl;
      }
    return _lv_less0(a,b,l);
  }

extern "C" void _lv_mul(u32_t *a,u32_t *b,u32_t *r, u32_t l)
  {
    u32_t const RSA_VALUE_WORDS = l/4;
    memset(r,0,l*2);
    int i = RSA_VALUE_WORDS-1;
    while ( i >= 0 && !a[i] ) --i;
    for ( int j = 0; j <= i; ++j )
      _lv_mul4(a[j],b,l,r+j,l*2-j*4);
  }

extern "C" u32_t _lv_add( u32_t *a, u32_t const al, u32_t *r, u32_t const rl )
  {
    u32_t cf = 0;
    for ( int i = 0; i < al/4 && i < rl/4; ++i )
      cf = _lv_add4(a[i],r+i,rl-i*4);
    return cf;
  }

extern "C" u32_t _lv_shl( u32_t n, u32_t *r, u32_t const rl )
  {
    u32_t shift = 0;
    for ( int i = 0; i < rl/4; ++i )
      {
        u32_t x = r[i];;
        r[i] = ( x << n ) | shift;
        shift = x >> 32-n;
      }
    return shift;
  }

extern "C" u32_t  _lv_sshl32(u32_t n, u32_t *a, u32_t al, u32_t *r, u32_t rl )
  {
    u32_t shift = 0;
    for ( int i = 0; i < al/4; ++i )
      {
        u32_t x = a[i];
        a[i] = ( x << n ) | shift;
        shift = x >> (32-n);
      }
    for ( int i = 0; i < rl/4; ++i )
      {
        u32_t x = r[i];
        r[i] = ( x << n ) | shift;
        shift = x >> (32-n);
      }
    return shift;
  }

extern "C" u32_t  _lv_sshl(u32_t n, u32_t *a, u32_t al, u32_t *r, u32_t rl )
  {
    u32_t rval;
    if ( n % 32 )
        rval = _lv_sshl32( n%32, a, al,r, rl ), n-=n%32;
    n/=32;
    if ( n )
      {
        rl/=4;
        al/=4;
        rval = *(a+al-n);
        memmove(r+n,r,(rl-n)*4);
        memcpy(r,a+al-n,n*4);
        memmove(a+n,a,(al-n)*4);
        memset(a,0,n*4);
      }
    return rval;
  }

extern "C" void long_scan( char const *text, u32_t *r, u32_t RSA_VALUE_BITS )
  {
    u32_t const RSA_VALUE_BYTES = RSA_VALUE_BITS/8;
    u32_t const RSA_VALUE_WORDS = RSA_VALUE_BITS/(8*4);
    u32_t const RSA_VALUE_HALFS = RSA_VALUE_BITS/(8*2);
    u32_t *b = (u32_t*)alloca(RSA_VALUE_BYTES);
    memset(r,0,RSA_VALUE_BYTES);
    for ( ;*text; ++text )
      {
        u32_t a = *text-'0';
        // *10 = (*4 + *1) * 2
        memcpy(b,r,RSA_VALUE_BYTES);
        _lv_shl(2,r,RSA_VALUE_BYTES); // *4
        _lv_add(b,RSA_VALUE_BYTES,r,RSA_VALUE_BYTES); // + (*1)
        _lv_shl(1,r,RSA_VALUE_BYTES); // *2
        // +mod
        _lv_add4(a,r,RSA_VALUE_BYTES);
      }
  }

extern "C" int long_print( char *text, int len, u32_t const *a, u32_t RSA_VALUE_BITS )
  {
    u32_t const RSA_VALUE_BYTES = RSA_VALUE_BITS/8;
    u32_t const RSA_VALUE_WORDS = RSA_VALUE_BITS/(8*4);
    u32_t const RSA_VALUE_HALFS = RSA_VALUE_BITS/(8*2);

    memset(text,0,len);
    char *pI = text, *pE = text+len-1;
    u16_t *b = (u16_t *)alloca(RSA_VALUE_BYTES);
    memcpy(b,a,RSA_VALUE_BYTES);

    for ( int f = 1; f && pI != pE; )
      {
        u32_t i = RSA_VALUE_HALFS;
        u32_t mod = 0;
        f = 0;
        while ( i-- )
          {
            u32_t x = (mod << 16) | b[i];
            mod = x%10;
            f |= (b[i]=x/10);
          }
        *pI++ = mod + '0';
      }

    while ( pI > text+1 && *(pI-1) == '0' ) --pI; *pI = 0;
    for ( char *l = text, *r = pI-1; l < r ; ++l, --r ) cxx_swap(*l,*r);

    return pI-text;
  }

extern "C" u32_t _lv_div2( u16_t a, u32_t *r, u32_t rl )
  {
    u16_t *b = (u16_t*)r;
    rl /= 2;
    u32_t x = 0;
    while ( rl-- )
      {
        x = (x << 16) | b[rl];
        b[rl] = x/a;
        x = x%a;
      }
    return x;
  }

extern "C" u32_t long_div2( u16_t a, u32_t *r, u32_t RSA_VALUE_BITS )
  {
    u32_t const RSA_VALUE_BYTES = RSA_VALUE_BITS/8;

    return _lv_div2(a,r,RSA_VALUE_BYTES);
  }

extern "C" void long_mul4( u32_t a, u32_t *r, u32_t RSA_VALUE_BITS )
  {
    u32_t const RSA_VALUE_BYTES = RSA_VALUE_BITS/8;
    u32_t *temp = (u32_t*)alloca(RSA_VALUE_BYTES+4);
    memset(temp,0,RSA_VALUE_BYTES+4);
    _lv_mul4(a,r,RSA_VALUE_BYTES,temp,RSA_VALUE_BYTES+4);
    memcpy(r,temp,RSA_VALUE_BYTES);
  }

extern "C" void long_add4( u32_t a, u32_t *r, u32_t RSA_VALUE_BITS )
  {
    u32_t const RSA_VALUE_BYTES = RSA_VALUE_BITS/8;
    _lv_add4(a,r,RSA_VALUE_BYTES);
  }
