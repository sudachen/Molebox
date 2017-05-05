
#ifndef ___1e1e0e4e_b624_4a9c_9aea_48994ce31c97___
#define ___1e1e0e4e_b624_4a9c_9aea_48994ce31c97___

enum
  {
    RSA_DEFAULT_VALUE_BITS  = 512,
    RSA_DEFAULT_VALUE_BYTES = RSA_DEFAULT_VALUE_BITS/8,
    RSA_DEFAULT_VALUE_WORDS = RSA_DEFAULT_VALUE_BITS/(8*4),
    RSA_DEFAULT_PRINT_LEN   = RSA_DEFAULT_VALUE_BITS/3+1,
  };
typedef u32_t RSA_VALUE_TYPE[RSA_DEFAULT_VALUE_WORDS];
typedef u32_t RSA_MULRES_TYPE[2*RSA_DEFAULT_VALUE_WORDS];

extern "C" void   _lv_mul4(u32_t a, u32_t *b, u32_t bl, u32_t *r, u32_t l);
extern "C" u32_t  _lv_add4(u32_t a, u32_t *r, u32_t rl);
extern "C" u32_t  _lv_shl(u32_t a,u32_t *r, u32_t rl);
extern "C" u32_t  _lv_less( u32_t *a, u32_t al, u32_t *b, u32_t bl );
extern "C" u32_t  _lv_less0( u32_t *a, u32_t *b, u32_t l );
extern "C" void   _lv_sshl1( u32_t *a, u32_t al, u32_t *r, u32_t rl );
extern "C" u32_t  _lv_sshl( u32_t n, u32_t *a, u32_t al, u32_t *r, u32_t rl );
extern "C" u32_t  _lv_maxbit( u32_t *a, u32_t al );
extern "C" void   _lv_sub(u32_t *m, u32_t ml, u32_t *r, u32_t rl);
extern "C" u32_t  _lv_add(u32_t *m, u32_t ml, u32_t *r, u32_t rl);
extern "C" void   _lv_mul(u32_t *a,u32_t *b,u32_t *r, u32_t l);
extern "C" void   _lv_mul0(u32_t *a,u32_t *b,u32_t *r, u32_t l);
extern "C" void   long_mul_mod( u32_t *a, u32_t *b, u32_t *m, u32_t *r, u32_t RSA_VALUE_BITS = RSA_DEFAULT_VALUE_BITS );
extern "C" void   long_exp_mod( u32_t *p, u32_t *e, u32_t *m, u32_t *r, u32_t RSA_VALUE_BITS = RSA_DEFAULT_VALUE_BITS );
extern "C" int    long_print( char *text, int len, u32_t const *a, u32_t RSA_VALUE_BITS = RSA_DEFAULT_VALUE_BITS );
extern "C" void   long_scan( char const *text, u32_t *r, u32_t RSA_VALUE_BITS = RSA_DEFAULT_VALUE_BITS );
extern "C" u32_t  long_div2( u16_t a, u32_t *r, u32_t RSA_VALUE_BITS = RSA_DEFAULT_VALUE_BITS );
extern "C" void   long_mul4( u32_t a, u32_t *r, u32_t RSA_VALUE_BITS = RSA_DEFAULT_VALUE_BITS );
extern "C" void   long_add4( u32_t a, u32_t *r, u32_t RSA_VALUE_BITS = RSA_DEFAULT_VALUE_BITS );

#define RSA_mulmod long_mul_mod
#define RSA_expmod long_exp_mod
#define RSA_lprint long_print
#define RSA_lscan  long_scan
#define RSA_div2   long_div2
#define RSA_mul4   long_mul4
#define RSA_add4   long_add4

#endif /*___1e1e0e4e_b624_4a9c_9aea_48994ce31c97___*/
