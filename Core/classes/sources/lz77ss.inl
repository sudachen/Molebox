
/*

  Copyright (c) 2005-2009, Alexey Sudachen, alexey@sudachen.name

*/

#if defined _TEGGO_LZ77SS_HERE || defined _TEGGOINLINE

#include <string.h>
#include <stdlib.h>

#define LZ77SS_STATIC static

#ifdef __cplusplus

#if defined _TEGGOINLINE
# define _LZ77SS_FAKE_INLINE LZ77SS_STATIC CXX_FAKE_INLINE
# define _LZ77SS_EXPORTABLE CXX_FAKE_INLINE
#else
# define _LZ77SS_FAKE_INLINE LZ77SS_STATIC
# define _LZ77SS_EXPORTABLE extern "C" _TEGGO_EXPORTABLE
#endif

#else /* raw C */

#if defined _TEGGOINLINE
# define _LZ77SS_FAKE_INLINE LZ77SS_STATIC
# define _LZ77SS_EXPORTABLE LZ77SS_STATIC
#else
# define _LZ77SS_FAKE_INLINE LZ77SS_STATIC
# define _LZ77SS_EXPORTABLE _TEGGO_EXPORTABLE
#endif

#endif

enum {
  LZSS_MIN_LEN = 4,
  LZSS_MAX_LEN = 15,
  LZSS_TABLE_MOD = 4095, /*4095, */
  LZSS_TABLE_LEN = LZSS_TABLE_MOD+1,
};

#ifdef _CHEKED_BUILD
#include <assert.h>
#define LZ77SS_ASSERT(x) LZ77SS_ASSERT
#else
#define LZ77SS_ASSERT(x)
#endif

_LZ77SS_FAKE_INLINE int lzss_search_index(char **table_L, char**iE, char* val, int strong, int *p_l, int maxl)
  {
    char **iS = table_L;
    int len = iE - iS;
    int half;
    char ** middle;
    int  l = 0, ll = 0;
    if (p_l) *p_l = 0;

    while (len > 0)
      {
        half = len >> 1;
        middle = iS + half;
        if ( memcmp(*middle,val,maxl) < 0 )
          {
            iS = middle;
            ++iS;
            len = len - half - 1;
          }
        else
          {
            len = half;
          }
      }

    if ( iS != iE )
      {
        char **p = iS-1;
        for ( l = 0; l < maxl && (*iS)[l] == val[l]; ) ++l;
        if ( p >= table_L && strong == 2 )
          {
            for ( ll = 0; ll < maxl && (*p)[ll] == val[ll]; ) ++ll;
            if ( ll > l ) iS = p, l = ll;
          }
      }
    else
      l = 0;

    if ( strong == 1 )
      {
        char **k = iS;
        LZ77SS_ASSERT ( iS != iE );

        if ( *k != val )
          for ( ; k >= table_L ; --k ) if (*k == val || **k != *val) break;
        if ( *k != val )
          for ( k = iS ; k != iE ; ++k ) if (*k == val || **k != *val) break;
        if ( *k == val )
          iS = k;
      }

    LZ77SS_ASSERT ( strong!=1 || *iS == val );
    LZ77SS_ASSERT ( !l || memcmp(val,*iS,l) == 0 );
    if ( p_l ) *p_l = l;
    return iS-table_L;
  }

_LZ77SS_FAKE_INLINE void lzss_replace_string(char **table, char *in_b, int r_i)
  {
    int l, o_idx, idx;
    if (r_i >= LZSS_TABLE_MOD) // replace
      {
        char *p = in_b+(r_i-LZSS_TABLE_MOD);
        o_idx = lzss_search_index(table,table+LZSS_TABLE_MOD,p,1,0,LZSS_MAX_LEN);

        LZ77SS_ASSERT ( o_idx >=0 && o_idx < LZSS_TABLE_MOD );
        if ( !(l = memcmp(p,in_b+ r_i,LZSS_MAX_LEN)) )
          {
            table[o_idx] = in_b+r_i;
            return;
          }
      }
    else // append
      {
        o_idx = r_i;
        l = 1;
      }

    if ( l < 0  )  // old string less then new
      { // working with right part
        if ( o_idx == LZSS_TABLE_MOD-1 )
          {
            table[o_idx] = in_b+r_i;
          }
        else
          {
            char **table_L = table + (o_idx + 1);
            idx = lzss_search_index(table_L,table+LZSS_TABLE_MOD,in_b+r_i,0,&l,LZSS_MAX_LEN);
            memmove(table_L-1,table_L,idx*sizeof(char*));
            table_L[idx-1] = in_b+r_i;
          }
      }
    else  // old string great then new
      { // working with left part
        char **table_R = table + o_idx;
        idx = lzss_search_index(table,table_R,in_b+r_i,0,&l,LZSS_MAX_LEN);
        memmove(table+idx+1,table+idx,((table_R-table)-idx)*sizeof(char*));
        table[idx] = in_b+r_i;
      }
  }

_LZ77SS_FAKE_INLINE short lzss_update_table(char **table, char *in_b, int _in_i, int out_l)
  {
    int code = 0, idx = 0, s_i, l=0;
    char **table_R;
    int r_i = _in_i-(LZSS_MAX_LEN-1);  // old (will replaced) string
    int t_i = _in_i;                   // encoded string
    int maxl = (out_l >= LZSS_MAX_LEN ? LZSS_MAX_LEN : out_l);

    if ( r_i > LZSS_TABLE_MOD )
      table_R = table + LZSS_TABLE_MOD;
    else
      {
        table_R = table + r_i;
        if ( out_l < LZSS_MAX_LEN ) table_R -= (LZSS_MAX_LEN - out_l);
      }

    // encoding
    idx = lzss_search_index(table,table_R,in_b+t_i,2,&l,maxl);
    s_i = t_i - (table[idx]-in_b);
    if ( l >= LZSS_MIN_LEN )
      {
        LZ77SS_ASSERT ( s_i <= LZSS_TABLE_MOD+LZSS_MAX_LEN && s_i >= LZSS_MAX_LEN );
        code = (((s_i-LZSS_MAX_LEN)%LZSS_TABLE_MOD)<<4)|l;
      }

    if ( out_l-- >= LZSS_MAX_LEN )
      lzss_replace_string(table,in_b,r_i++);

    // adding new strings
    if ( l >= LZSS_MIN_LEN )
      while ( --l && out_l-- >= LZSS_MAX_LEN )
        lzss_replace_string(table,in_b,r_i++);

    return code;
  }

_LZ77SS_EXPORTABLE int lz77ss_compress(char *in_b, int in_b_len, unsigned char *out_b, int out_b_len)
  {
    char **table = 0;
    int  out_i = 4, in_i = 0;
    unsigned char *cnt_p = 0;

    if ( in_b_len < 2*LZSS_MAX_LEN ) return 0;

    out_b[0] = in_b_len&0x0ff;
    out_b[1] = (in_b_len>>8)&0x0ff;
    out_b[2] = (in_b_len>>16)&0x0ff;
    out_b[3] = (in_b_len>>24)&0x0ff;

    table = (char**)malloc(LZSS_TABLE_LEN*sizeof(char**));
    memset(table,0,LZSS_TABLE_LEN*sizeof(char**));
    table[0] = in_b;

    out_b[out_i++] = 0;
    cnt_p = out_b+out_i;
    out_b[out_i++] = LZSS_MAX_LEN-1;
    memcpy(out_b+out_i,in_b,LZSS_MAX_LEN);
    out_i += LZSS_MAX_LEN;
    in_i  += LZSS_MAX_LEN;

    while ( in_i <  in_b_len && out_i+1 < out_b_len )
      {
        unsigned short code = lzss_update_table(table,in_b,in_i,in_b_len-in_i);
        if ( !code )
          {
            if ( !cnt_p || (!cnt_p[-1] && *cnt_p == 255) )
              {
                out_b[out_i++] = 0x80;
                *(cnt_p = out_b + out_i++) = in_b[in_i++];
              }
            else
              {
                if ( cnt_p[-1] == 0x80 )
                  {
                    out_b[out_i++] = *cnt_p;
                    *cnt_p = cnt_p[-1] = 0;
                  }
                ++*cnt_p;
                out_b[out_i++] = in_b[in_i++];
              }
          }
        else
          {
            int l = code&0x0f;
            cnt_p = 0;
            out_b[out_i++] = code&0x0ff;
            out_b[out_i++] = (code>>8)&0x0ff;
            in_i += l;
          }
      }
    if ( in_i != in_b_len ) out_i = -out_i;
    free(table);
    return out_i;
  }

_LZ77SS_EXPORTABLE int lz77ss_decompress(unsigned char *in_b, int in_b_len, char *out_b, int out_b_len)
  {
    int in_i = 0;
    int out_i = 0;
    while ( in_i < in_b_len && out_i < out_b_len )
      {
        if ( in_b[in_i] == 0x80 )
          {// one char
            out_b[out_i++] = in_b[++in_i];
            ++in_i;
          }
        else if ( !in_b[in_i] )
          {// several chars
            int l = (int)in_b[++in_i]+1;
            ++in_i;
            while ( l-- )
              {
                out_b[out_i++] = in_b[in_i++];
              }
          }
        else
          {// code
            unsigned short code = (short)in_b[in_i]|((short)in_b[in_i+1] << 8);
            int l = code & 0x0f;
            int off = code >> 4;
            memcpy(out_b+out_i,out_b+out_i-off-LZSS_MAX_LEN,l);
            out_i += l;
            in_i += 2;
          }
      }

    return out_i;
  }

/*#else no here*/
#endif
