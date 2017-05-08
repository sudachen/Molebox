
/*

Copyright © 2010-2011, Alexéy Sudachén, alexey@sudachen.name, Chile

In USA, UK, Japan and other countries allowing software patents:

    This program is free software: you can redistribute it and/or modify
    it under the terms of the GNU General Public License as published by
    the Free Software Foundation, either version 3 of the License, or
    (at your option) any later version.

    This program is distributed in the hope that it will be useful,
    but WITHOUT ANY WARRANTY; without even the implied warranty of
    MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
    GNU General Public License for more details.

    http://www.gnu.org/licenses/

Otherwise:

Permission is hereby granted, free of charge, to any person obtaining a copy
of this software and associated documentation files (the "Software"), to deal
in the Software without restriction, including without limitation the rights
to use, copy, modify, merge, publish, distribute, sublicense, and/or sell
copies of the Software, and to permit persons to whom the Software is
furnished to do so, subject to the following conditions:

The above copyright notice and this permission notice shall be included in
all copies or substantial portions of the Software.

THE SOFTWARE IS PROVIDED "AS IS", WITHOUT WARRANTY OF ANY KIND, EXPRESS OR
IMPLIED, INCLUDING BUT NOT LIMITED TO THE WARRANTIES OF MERCHANTABILITY,
FITNESS FOR A PARTICULAR PURPOSE AND NONINFRINGEMENT. IN NO EVENT SHALL THE
AUTHORS OR COPYRIGHT HOLDERS BE LIABLE FOR ANY CLAIM, DAMAGES OR OTHER
LIABILITY, WHETHER IN AN ACTION OF CONTRACT, TORT OR OTHERWISE, ARISING FROM,
OUT OF OR IN CONNECTION WITH THE SOFTWARE OR THE USE OR OTHER DEALINGS IN
THE SOFTWARE.

Except as contained in this notice, the name of a copyright holder shall not
be used in advertising or otherwise to promote the sale, use or other dealings
in this Software without prior written authorization of the copyright holder.

*/

#ifndef C_once_0ED387CD_668B_44C3_9D91_A6336A2F5F48
#define C_once_0ED387CD_668B_44C3_9D91_A6336A2F5F48

#include "yoyo.hc"
#include "array.hc"

#ifdef _LIBYOYO
#define _YO_STRING_BUILTIN
#endif

#ifdef _YO_STRING_BUILTIN
#define _YO_STRING_EXTERN
#else
#define _YO_STRING_EXTERN extern
#endif

/* caseinsensitive strcmp, returns 0 if equal */
#define Str_Ansi_Equal_Nocase(Cs,Ct) (!strcmp_I(Cs,Ct))
int strcmp_I(char *cs, char *ct)
#ifdef _YO_STRING_BUILTIN
{
    int q = 0;
    if ( cs && ct ) do
        {
            q = Toupper(*cs) - Toupper(*ct++);
        }
        while ( *cs++ && !q );
    return q;
}
#endif
;

#define Str_Unicode_Compare_Nocase(Cs,Ct) wcscmp_I(Cs,Ct)
int wcscmp_I(wchar_t *cs, wchar_t *ct)
#ifdef _YO_STRING_BUILTIN
{
    int q = 0;
    if ( cs && ct ) do
        {
            q = towupper(*cs) - towupper(*ct++);
        }
        while ( *cs++ && !q );
    return q;
}
#endif
;

/* caseinsensitive strncmp, returns 0 if equal */
int strncmp_I(char *cs, char *ct, int l)
#ifdef _YO_STRING_BUILTIN
{
    int q = 0;
    if ( l && cs && ct ) do
        {
            q = Toupper(*cs) - Toupper(*ct++);
        }
        while ( *cs++ && !q && --l );
    return q;
}
#endif
;

int wcsncmp_I(wchar_t *cs, wchar_t *ct, int l)
#ifdef _YO_STRING_BUILTIN
{
    int q = 0;
    if ( l && cs && ct ) do
        {
            q = towupper(*cs) - towupper(*ct++);
        }
        while ( *cs++ && !q && --l );
    return q;
}
#endif
;

int Str_Length(char *S)
#ifdef _YO_STRING_BUILTIN
{
    return S ? strlen(S) : 0;
}
#endif
;

int Str_Is_Empty(char *S)
#ifdef _YO_STRING_BUILTIN
{
    if ( !S ) return 1;
    while ( *S && Isspace(*S) ) ++S;
    return !*S;
}
#endif
;

char Str_Last(char *S)
#ifdef _YO_STRING_BUILTIN
{
    int L = S ? strlen(S) : 0;
    return L ? S[L-1] : 0;
}
#endif
;

#define Str_Copy(Str) Str_Copy_L(Str,-1)
#define Str_Copy_L(Str,Len) __Pool(Str_Copy_Npl(Str,Len))
char *Str_Copy_Npl(char *S,int L)
#ifdef _YO_STRING_BUILTIN
{
    char *p;
    if ( L < 0 ) L = S?strlen(S):0;
    p = Yo_Malloc_Npl(L+1);
    if ( L )
        memcpy(p,S,L);
    p[L] = 0;
    return p;
}
#endif
;

#define Str_Range(Start,End) __Pool(Str_Range_Npl(Start,End))
char *Str_Range_Npl(char *S, char *E)
#ifdef _YO_STRING_BUILTIN
{
    STRICT_REQUIRE(E >= S);
    return Str_Copy_Npl(S,E-S);
}
#endif
;

#define Str_Trim_Copy(Str) Str_Trim_Copy_L(Str,-1)
#define Str_Trim_Copy_L(Str,Len) __Pool(Str_Trim_Copy_Npl(Str,Len))
char *Str_Trim_Copy_Npl(char *S, int L)
#ifdef _YO_STRING_BUILTIN
{
    if ( L < 0 ) L = S?strlen(S):0;
    if ( L && S )
    {
        while ( *S && Isspace(*S) ) { ++S; --L; }
        while ( L && Isspace(S[L-1]) ) --L;
    }
    return Str_Copy_Npl(S,L);
}
#endif
;

#define Str_Unicode_Copy(Str) Str_Unicode_Copy_L(Str,-1)
#define Str_Unicode_Copy_L(Str,Len) Yo_Pool(Str_Unicode_Copy_Npl(Str,Len))
wchar_t *Str_Unicode_Copy_Npl(wchar_t *S,int L)
#ifdef _YO_STRING_BUILTIN
{
    wchar_t *p;
    if ( L < 0 ) L = S?wcslen(S):0;
    p = Yo_Malloc_Npl((L+1)*sizeof(wchar_t));
    if ( L )
        memcpy(p,S,L*sizeof(wchar_t));
    p[L] = 0;
    return p;
}
#endif
;

char *Str_Split_Once_Into(char *S,char *delims,void *arr)
#ifdef _YO_STRING_BUILTIN
{
    if ( !S || !*S ) return 0;

    if ( delims )
    {
        char *d;
        int j = 0;
        for ( ; S[j]; ++j )
            for ( d = delims; *d; ++d )
                if ( S[j] == *d )
                    goto l;
    l:
        Array_Push(arr,Str_Copy_Npl(S,j));
        return S[j] ? S+(j+1) : 0;
    }
    else // split by spaces
    {
        char *p = S, *q;
        while ( *p && Isspace(*p) ) ++p;
        q = p;
        while ( *q && !Isspace(*q) ) ++q;
        Array_Push(arr,Str_Copy_Npl(p,q-p));
        while ( *q && Isspace(*q) ) ++q;
        return *q ? q : 0;
    }
}
#endif
;

void *Str_Split_Once(char *S,char *delims)
#ifdef _YO_STRING_BUILTIN
{
    YO_ARRAY *L = Array_Ptrs();
    if ( S )
    {
        S = Str_Split_Once_Into(S,delims,L);
        if ( S )
            Array_Push(L,Str_Copy_Npl(S,-1));
    }
    return L;
}
#endif
;

void *Str_Split(char *S,char *delims)
#ifdef _YO_STRING_BUILTIN
{
    YO_ARRAY *L = Array_Ptrs();
    while ( S )
        S = Str_Split_Once_Into(S,delims,L);
    return L;
}
#endif
;

int Bits_Pop(ulong_t *r, void *b, int *bits_count, int count)
#ifdef _YO_STRING_BUILTIN
{
    #if 1
    byte_t const *bits = (byte_t const *)b;
    int bC = *bits_count - 1;
    int Q = Yo_MIN(count,bC+1);
    int r_count = Q;

    if ( bC < 0 ) return 0;

    while ( ((bC+1) &7) && Q )
    {
        *r = (*r << 1) | ((bits[bC/8] >> (bC%8))&1);
        --bC; --Q;
    }

    while ( bC >= 0 && Q )
        if ( Q > 7 )
        {
            *r = ( *r << 8 ) | bits[bC/8];
            Q -= 8; bC -= 8;
        }
        else
        {
            *r = (*r << Q) | bits[bC/8] >> (8-Q);
            bC -= Q; Q = 0;
        }

    *bits_count = bC + 1;
    return r_count;
    #else
    int r_count = 0;
    byte_t *bits = (byte_t *)b;
    while ( count && *bits_count )
    {
        int q = *bits_count-1;
        *r = (*r << 1) | ((bits[q/8] >> (q%8))&1);
        --*bits_count;
        --count;
        ++r_count;
    }
    return r_count;
    #endif
}
#endif
;

void Bits_Push(ulong_t bits, void *b, int *bits_count, int count)
#ifdef _YO_STRING_BUILTIN
{
    while ( count-- )
    {
        int q = *bits_count;
        byte_t *d = ((byte_t *)b+q/8);
        *d = (byte_t)(((bits&1) << (q%8)) | (*d&~(1<<(q%8))));
        ++*bits_count;
        bits >>= 1;
    }
}
#endif
;

char *Str_Xbit_Encode(void *data, int count /*of bits*/, int BC, char *bit_table, char *out )
#ifdef _YO_STRING_BUILTIN
{
    char *Q = out;
    ulong_t q = 0;
    if ( count%BC )
    {
        Bits_Pop(&q,data,&count,count%BC);
        *Q++ = bit_table[q];
    }
    while ( count )
    {
        q = 0;
        Bits_Pop(&q,data,&count,BC);
        *Q++ = bit_table[q];
    }
    return out;
}
#endif
;

_YO_STRING_EXTERN char Str_5bit_Encoding_Table[] /* 32 */
#ifdef _YO_STRING_BUILTIN
    = "0123456789abcdefgjkmnpqrstuvwxyz"
#endif
      ;

_YO_STRING_EXTERN char Str_5bit_Encoding_Table_Upper[] /* 32 */
#ifdef _YO_STRING_BUILTIN
    = "0123456789ABCDEFGJKMNPQRSTUVWXYZ"
#endif
      ;

_YO_STRING_EXTERN char Str_6bit_Encoding_Table[] /* 64 */
#ifdef _YO_STRING_BUILTIN
    = "0123456789abcdefghijklmnopqrstuvwxyzABCDEFGHIJKLMNOPQRSTUVWXYZ_-"
#endif
      ;

#define Str_5bit_Encode(S,L) Str_Qbit_Encode(S,L,Str_5bit_Encoding_Table,5,0)
#define Str_5bit_Encode_Upper(S,L) Str_Qbit_Encode(S,L,Str_5bit_Encoding_Table_Upper,5,0)
#define Str_6bit_Encode(S,L) Str_Qbit_Encode(S,L,Str_6bit_Encoding_Table,6,0)
#define Str_5bit_Encode_(S,L,Out) Str_Qbit_Encode(S,L,Str_5bit_Encoding_Table,5,Out)
#define Str_5bit_Encode_Upper_(S,L,Out) Str_Qbit_Encode(S,L,Str_5bit_Encoding_Table_Upper,5,Out)
#define Str_6bit_Encode_(S,L,Out) Str_Qbit_Encode(S,L,Str_6bit_Encoding_Table,6,Out)

char *Str_Qbit_Encode(void *data,int len, char *tbl, int btl, char *out)
#ifdef _YO_STRING_BUILTIN
{
    if ( data && len )
    {
        int rq_len = (len*8+btl-1)/btl;
        if ( !out )
            out = Yo_Malloc(rq_len+1);
        memset(out,0,rq_len+1);
        return Str_Xbit_Encode(data,len*8,btl,tbl,out);
    }
    return 0;
}
#endif
;

void *Str_Xbit_Decode(char *inS, int len, int BC, byte_t *bit_table, void *out)
#ifdef _YO_STRING_BUILTIN
{
    int count = 0;
    byte_t *S = ((byte_t *)inS+len)-1, *E = (byte_t *)inS-1;
    while ( S != E )
    {
        byte_t bits = bit_table[*S--];
        if ( bits == 255 )
            __Raise(YO_ERROR_CORRUPTED,
                    __Format(__yoTa("bad symbol '%c' in encoded sequence",0),S[1]));
        Bits_Push(bits,out,&count,BC);
    }
    return out;
}
#endif
;

_YO_STRING_EXTERN byte_t Str_5bit_Decoding_Table[] /* 32 */
#ifdef _YO_STRING_BUILTIN
=
{
    255,255,255,255,255,255,255,255,255,255,255,255,255,255,255,255,
    255,255,255,255,255,255,255,255,255,255,255,255,255,255,255,255,
    255,255,255,255,255,255,255,255,255,255,255,255,255,255,255,255,
    0,  1,  2,  3,  4,  5,  6,  7,  8,  9,255,255,255,255,255,255,
    255, 10, 11, 12, 13, 14, 15, 16,255,255, 17, 18,255, 19, 20,255,
    21, 22, 23, 24, 25, 26, 27, 28, 29, 30, 31,255,255,255,255,255,
    255, 10, 11, 12, 13, 14, 15, 16,255,255, 17, 18,255, 19, 20,255,
    21, 22, 23, 24, 25, 26, 27, 28, 29, 30, 31,255,255,255,255,255,
    255,255,255,255,255,255,255,255,255,255,255,255,255,255,255,255,
    255,255,255,255,255,255,255,255,255,255,255,255,255,255,255,255,
    255,255,255,255,255,255,255,255,255,255,255,255,255,255,255,255,
    255,255,255,255,255,255,255,255,255,255,255,255,255,255,255,255,
    255,255,255,255,255,255,255,255,255,255,255,255,255,255,255,255,
    255,255,255,255,255,255,255,255,255,255,255,255,255,255,255,255,
    255,255,255,255,255,255,255,255,255,255,255,255,255,255,255,255,
    255,255,255,255,255,255,255,255,255,255,255,255,255,255,255,255,
}
#endif
;

_YO_STRING_EXTERN byte_t Str_6bit_Decoding_Table[] /* 64 */
#ifdef _YO_STRING_BUILTIN
=
{
    255,255,255,255,255,255,255,255,255,255,255,255,255,255,255,255,
    255,255,255,255,255,255,255,255,255,255,255,255,255,255,255,255,
    255,255,255,255,255,255,255,255,255,255,255,255,255,63, 255,255,
    0  ,1  ,2  ,3  ,4  ,5  ,6  ,7  ,8  ,9  ,255,255,255,255,255,255,
    255,36 ,37 ,38 ,39 ,40 ,41 ,42 ,43 ,44 ,45 ,46 ,47 ,48 ,49 ,50 ,
    51 ,52 ,53 ,54 ,55 ,56 ,57 ,58 ,59 ,60 ,61 ,255,255,255,255,62 ,
    255,10 ,11 ,12 ,13 ,14 ,15 ,16 ,17 ,18 ,19 ,20 ,21 ,22 ,23 ,24 ,
    25 ,26 ,27 ,28 ,29 ,30 ,31 ,32 ,33 ,34 ,35 ,255,255,255,255,255,
    255,255,255,255,255,255,255,255,255,255,255,255,255,255,255,255,
    255,255,255,255,255,255,255,255,255,255,255,255,255,255,255,255,
    255,255,255,255,255,255,255,255,255,255,255,255,255,255,255,255,
    255,255,255,255,255,255,255,255,255,255,255,255,255,255,255,255,
    255,255,255,255,255,255,255,255,255,255,255,255,255,255,255,255,
    255,255,255,255,255,255,255,255,255,255,255,255,255,255,255,255,
    255,255,255,255,255,255,255,255,255,255,255,255,255,255,255,255,
    255,255,255,255,255,255,255,255,255,255,255,255,255,255,255,255,
}
#endif
;

#define Str_5bit_Decode(S,L) Str_Qbit_Decode(S,L,Str_5bit_Decoding_Table,5)
#define Str_6bit_Decode(S,L) Str_Qbit_Decode(S,L,Str_6bit_Decoding_Table,6)

void *Str_Qbit_Decode(char *S,int *len,byte_t *tbl,int btl)
#ifdef _YO_STRING_BUILTIN
{
    int S_len = S ? strlen(S): 0;
    int rq_len = S_len ? (S_len*btl+7)/8 : 0;

    if ( rq_len )
    {
        void *out = Yo_Malloc(rq_len);
        memset(out,0,rq_len);
        Str_Xbit_Decode(S,S_len,btl,tbl,out);
        if ( len ) *len = rq_len;
        return out;
    }

    return 0;
}
#endif
;

char *Str_Hex_Byte(byte_t val,char pfx,void *out)
#ifdef _YO_STRING_BUILTIN
{
    static char symbols[] =
    { '0','1','2','3','4','5','6','7','8','9','a','b','c','d','e','f' };
    char *q = out;
    switch ( pfx&0x7f )
    {
        case 'x': *q++='0'; *q++='x'; break;
        case '\\': *q++='\\'; *q++='x'; break;
        case '%': *q++='%'; break;
        default: break;
    }
    *q++ = symbols[(val>>4)];
    *q++ = symbols[val&0x0f];
    if ( !pfx&0x80 )
        *q = 0;
    return out;
}
#endif
;

#define Str_Hex_Encode(S,L) Str_Hex_Encode_(S,L,0)
char *Str_Hex_Encode_(void *data, int len, char *out)
#ifdef _YO_STRING_BUILTIN
{
    if ( data && len )
    {
        int i;
        int rq_len = len*2;
        if ( !out )
            out = Yo_Malloc(rq_len+1);
        memset(out,0,rq_len+1);
        for ( i = 0; i < len; ++i )
            Str_Hex_Byte(((byte_t *)data)[i],0,out+i*2);
        return out;
    }
    return 0;
}
#endif
;

#define STR_UNHEX_HALF_OCTET(c,r,i) \
    if ( *(c) >= '0' && *(c) <= '9' ) \
        r |= (*(c)-'0') << (i); \
    else if ( *(c) >= 'a' && *(c) <= 'f' ) \
        r |= (*(c)-'a'+10) << (i); \
    else if ( *(c) >= 'A' && *(c) <= 'F' ) \
        r |= (*(c)-'A'+10) << (i); \
     
byte_t Str_Unhex_Byte(char *S,int pfx,int *cnt)
#ifdef _YO_STRING_BUILTIN
{
    int i;
    byte_t r = 0;
    byte_t *c = (byte_t *)S;
    if ( pfx )
    {
        if ( *c == '0' && c[1] == 'x' ) c+=2;
        else if ( *c == '\\' && c[1] == 'x' ) c+=2;
        else if ( *c == '%' ) ++c;
    }
    for ( i=4; i >= 0; i-=4, ++c )
    {
        STR_UNHEX_HALF_OCTET(c,r,i);
    }
    if ( cnt ) *cnt = c-(byte_t *)S;
    return r;
}
#endif
;

#define Str_Hex_Decode(S,L) Str_Hex_Decode_(S,L,0)
void *Str_Hex_Decode_(char *S,int *len,byte_t *out)
#ifdef _YO_STRING_BUILTIN
{
    int S_len = S ? strlen(S): 0;
    int rq_len = S_len ? S_len/2 : 0;

    if ( rq_len )
    {
        int i;
        if ( !out )
            out = Yo_Malloc(rq_len+1);
        for ( i = 0; i < rq_len; ++i )
            out[i] = Str_Unhex_Byte(S+i*2,0,0);
        out[rq_len] = 0;
        if ( len ) *len = rq_len;
        return out;
    }

    return 0;
}
#endif
;

int Str_Urldecode_Char(char **S)
#ifdef _YO_STRING_BUILTIN
{
    int r = 0;
    if ( *S )
    {
        if ( **S == '+' )
        {
            r = ' ';
            ++*S;
        }
        else if ( **S == '%' && Isxdigit((*S)[1]) && Isxdigit((*S)[2]) )
        {
            STR_UNHEX_HALF_OCTET((*S)+1,r,4);
            STR_UNHEX_HALF_OCTET((*S)+2,r,0);
            *S += 3;
        }
        else
            r = *(*S)++;
    }
    return r;
}
#endif
;

byte_t *Str_Base64_Decode(char *S, int *l)
#ifdef _YO_STRING_BUILTIN
{
    return 0;
}
#endif
;

char *Str_Base64_Encode(void *data, int l)
#ifdef _YO_STRING_BUILTIN
{
    return 0;
}
#endif
;

void Quad_To_Hex16(uquad_t val,char *out)
#ifdef _YO_STRING_BUILTIN
{
    int i;
    for ( i = 0; i < 8; ++i )
        Str_Hex_Byte((byte_t)(val>>(i*8)),0x80,out+i*2);
}
#endif
;

void Unsigned_To_Hex8(uint_t val,char *out)
#ifdef _YO_STRING_BUILTIN
{
    int i;
    for ( i = 0; i < 4; ++i )
        Str_Hex_Byte((byte_t)(val>>(i*8)),0x80,out+i*2);
}
#endif
;

void Unsigned_To_Hex4(uint_t val,char *out)
#ifdef _YO_STRING_BUILTIN
{
    int i;
    for ( i = 0; i < 2; ++i )
        Str_Hex_Byte((byte_t)(val>>(i*8)),0x80,out+i*2);
}
#endif
;

#define Unsigned_To_Hex2(Val,Out) Str_Hex_Byte((byte_t)(Val),0x80,Out)

uquad_t Hex16_To_Quad(char *S)
#ifdef _YO_STRING_BUILTIN
{
    uint_t ret = 0;
    int i;
    for ( i = 0; i < 8; ++i )
        ret |= ( (uint_t)Str_Unhex_Byte(S+i*2,0,0) << (i*8) );
    return ret;
}
#endif
;

uint_t Hex8_To_Unsigned(char *S)
#ifdef _YO_STRING_BUILTIN
{
    uint_t ret = 0;
    int i;
    for ( i = 0; i < 4; ++i )
        ret |= ( (uint_t)Str_Unhex_Byte(S+i*2,0,0) << (i*8) );
    return ret;
}
#endif
;

uint_t Hex4_To_Unsigned(char *S)
#ifdef _YO_STRING_BUILTIN
{
    uint_t ret = 0;
    int i;
    for ( i = 0; i < 2; ++i )
        ret |= ( (uint_t)Str_Unhex_Byte(S+i*2,0,0) << (i*8) );
    return ret;
}
#endif
;

#define Hex2_To_Unsigned(S) ( (uint_t)Str_Unhex_Byte(S,0,0) )

_YO_STRING_EXTERN char Utf8_Char_Length[]
#ifdef _YO_STRING_BUILTIN
=
{
    /* Map UTF-8 encoded prefix byte to sequence length.  zero means
       illegal prefix.  see RFC 2279 for details */
    1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1,
    1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1,
    1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1,
    1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1,
    1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1,
    1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1,
    1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1,
    1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1,
    0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0,
    0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0,
    0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0,
    0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0,
    2, 2, 2, 2, 2, 2, 2, 2, 2, 2, 2, 2, 2, 2, 2, 2,
    2, 2, 2, 2, 2, 2, 2, 2, 2, 2, 2, 2, 2, 2, 2, 2,
    3, 3, 3, 3, 3, 3, 3, 3, 3, 3, 3, 3, 3, 3, 3, 3,
    4, 4, 4, 4, 4, 4, 4, 4, 5, 5, 5, 5, 6, 6, 0, 0
}
#endif
;

wchar_t Utf8_Char_Decode(void *S,int *cnt)
#ifdef _YO_STRING_BUILTIN
{
    byte_t *text = S;
    int c = -1;
    int c0 = *text++;
    if ( cnt ) ++*cnt;
    if (c0 < 0x80)
        c = (wchar_t)c0;
    else
    {
        int c1 = 0;
        int c2 = 0;
        int c3 = 0;
        int l = Utf8_Char_Length[c0];
        switch ( l )
        {
            case 2:
                if ( (c1 = *text) > 0 )
                    c = ((c0 & 0x1f) << 6) + (c1 & 0x3f);
                if ( cnt ) ++*cnt;
                break;
            case 3:
                if ( (c1 = *text) > 0 && (c2 = text[1]) > 0 )
                    c = ((c0 & 0x0f) << 12) + ((c1 & 0x3f) << 6) + (c2 & 0x3f);
                if ( cnt ) *cnt += 2;
                break;
            case 4: // hm, UCS4 ????
                if ( (c1 = *text) > 0 && (c2 = text[1]) > 0 && (c3 = text[2]) > 0 )
                    c = ((c0 & 0x7) << 18) + ((c1 & 0x3f) << 12) + ((c2 & 0x3f) << 6) + (c3 & 0x3f);
                if ( cnt ) *cnt += 3;
                break;
            default:
                break;
        }
    }
    return c;
}
#endif
;

int Utf8_Wide_Length(wchar_t c)
#ifdef _YO_STRING_BUILTIN
{
    if ( c < 0x80 )
        return 1;
    else if ( c < 0x0800 )
        return 2;
    else
        return 3;
    return 0;
}
#endif
;

char *Utf8_Wide_Encode(void *_bf,wchar_t c,int *cnt)
#ifdef _YO_STRING_BUILTIN
{
    char *bf = _bf;
    int l = 0;
    if ( c < 0x80 )
    {
        *bf++ = (char)c;
        l = 1;
    }
    else if ( c < 0x0800 )
    {
        *bf++ = (char)(0xc0 | (c >> 6));
        *bf++ = (char)(0x80 | (c & 0x3f));
        l = 2;
    }
    else
    {
        *bf++ = (char)(0xe0 | (c >> 12));
        *bf++ = (char)(0x80 | ((c >> 6) & 0x3f));
        *bf++ = (char)(0x80 | (c & 0x3f));
        l = 3;
    }
    if ( cnt ) *cnt += l;
    return bf;
}
#endif
;

wchar_t Utf8_Get_Wide(char **S)
#ifdef _YO_STRING_BUILTIN
{
    wchar_t out = 0;
    if ( S && *S )
    {
        int cnt = 0;
        out = Utf8_Char_Decode(*S,&cnt);
        while ( **S && cnt-- ) ++*S;
    }
    return out;
}
#endif
;

char *Utf8_Skip(char *S,int l)
#ifdef _YO_STRING_BUILTIN
{
    if ( S )
        while ( *S && l-- )
        {
            int q = Utf8_Char_Length[(unsigned)*S];
            if ( q ) while ( q-- && *S ) ++S;
            else ++S;
        }
    return S;
}
#endif
;

wchar_t *Str_Utf8_To_Unicode_Convert(char *S, wchar_t *out, int maxL)
#ifdef _YO_STRING_BUILTIN
{
    int i = 0;
    if ( S )
    {
        for (; *S && i < maxL; ) { out[i++] = Utf8_Get_Wide(&S); }
    }
    if ( i < maxL ) out[i] = 0;
    return out;
}
#endif
;

#define Str_Utf8_To_Unicode(S) Yo_Pool(Str_Utf8_To_Unicode_Npl(S))
wchar_t *Str_Utf8_To_Unicode_Npl(char *S)
#ifdef _YO_STRING_BUILTIN
{
    wchar_t *out = 0;
    if ( S )
    {
        int n = 0;
        char *Q = S;
        while ( *Q ) { Utf8_Get_Wide(&Q); ++n; }
        out = Yo_Malloc_Npl((n+1)*sizeof(wchar_t));
        for ( n = 0; *S; ) { out[n++] = Utf8_Get_Wide(&S); }
        out[n] = 0;
    }
    return out;
}
#endif
;

char *Str_Unicode_To_Utf8_Convert(wchar_t *S, char *out, int maxL)
#ifdef _YO_STRING_BUILTIN
{
    int i = 0;
    if ( S )
    {
        for (; *S && i + Utf8_Wide_Length(*S) < maxL; )
        { Utf8_Wide_Encode(out+i,*S++,&i); }
    }
    if ( i < maxL ) out[i] = 0;
    return out;
}
#endif
;

#define Str_Unicode_To_Utf8(S) Yo_Pool(Str_Unicode_To_Utf8_Npl(S))
char *Str_Unicode_To_Utf8_Npl(wchar_t *S)
#ifdef _YO_STRING_BUILTIN
{
    char *out = 0;
    if ( S )
    {
        int n = 0;
        wchar_t *Q = S;
        while ( *Q )
            n += Utf8_Wide_Length(*Q++);
        out = Yo_Malloc_Npl(n+1);
        for ( n = 0; *S; )
            Utf8_Wide_Encode(out+n,*S++,&n);
        out[n] = 0;
    }
    return out;
}
#endif
;

/** specially for windoze **/
#define Str_Locale_To_Unicode(S) Yo_Pool(Str_Locale_To_Unicode_Npl(S));
wchar_t *Str_Locale_To_Unicode_Npl(char *S)
#ifdef _YO_STRING_BUILTIN
{
    #ifdef __windoze
    int L = strlen(S);
    wchar_t *ret = Yo_Malloc_Npl((L+1)*sizeof(wchar_t));
    MultiByteToWideChar(CP_ACP,0,S,-1,ret,L);
    ret[L] = 0;
    return ret;
    #else
    return Str_Utf8_To_Unicode_Npl(S);
    #endif
}
#endif
;

/** specially for windoze **/
#define Str_Locale_To_Utf8(S) Yo_Pool(Str_Locale_To_Utf8_Npl(S));
char *Str_Locale_To_Utf8_Npl(char *S)
#ifdef _YO_STRING_BUILTIN
{
    #ifdef __windoze
    wchar_t *tmp = Str_Locale_To_Unicode_Npl(S);
    char *ret = Str_Unicode_To_Utf8_Npl(tmp);
    free(tmp);
    return ret;
    #else
    return Str_Copy_Npl(S,-1);
    #endif
}
#endif
;

#define Str_Concat(A,B) Yo_Pool(Str_Concat_Npl(A,B))
char *Str_Concat_Npl(char *a, char *b)
#ifdef _YO_STRING_BUILTIN
{
    int a_len = a?strlen(a):0;
    int b_len = b?strlen(b):0;
    char *out = Yo_Malloc_Npl(a_len+b_len+1);
    if ( a_len )
        memcpy(out,a,a_len);
    if ( b_len )
        memcpy(out+a_len,b,b_len);
    out[a_len+b_len] = 0;
    return out;
}
#endif
;

#define Str_Join_Q(Sep,Sx) ((char*)__Pool(Str_Join_Q_Npl(Sep,Sx)))
char *Str_Join_Q_Npl(char sep, char **Sx)
#ifdef _YO_STRING_BUILTIN
{
    int i;
    int len = 0;
    char *q = 0, *out = 0;
    for ( i = 0; !!(q = Sx[i]); ++i )
        len += strlen(q)+(sep?1:0);
    if ( len )
    {
        char *Q = out = Yo_Malloc_Npl(len+(sep?0:1));
        for ( i = 0; !!(q = Sx[i]); ++i )
        {
            int l = strlen(q);
            if ( sep && Q != out )
                *Q++ = sep;
            memcpy(Q,q,l);
            Q += l;
        }
        *Q = 0;
    }
    else
    {
        out = Yo_Malloc_Npl(1);
        *out = 0;
    }
    return out;
}
#endif
;

char *Str_Join_Va_Npl(char sep, va_list va)
#ifdef _YO_STRING_BUILTIN
{
    int len = 0;
    char *q = 0, *out = 0;
    va_list va2;
    #ifdef __GNUC__
    va_copy(va2,va);
    #else
    va2 = va;
    #endif
    while ( !!(q = va_arg(va2,char *)) )
        len += strlen(q)+(sep?1:0);
    if ( len )
    {
        char *Q = out = Yo_Malloc_Npl(len+(sep?0:1));
        while ( !!(q = va_arg(va,char *)) )
        {
            int l = strlen(q);
            if ( sep && Q != out )
                *Q++ = sep;
            memcpy(Q,q,l);
            Q += l;
        }
        *Q = 0;
    }
    else
    {
        out = Yo_Malloc_Npl(1);
        *out = 0;
    }
    return out;
}
#endif
;

char *Str_Join_Npl_(char sep, ...)
#ifdef _YO_STRING_BUILTIN
{
    char *out;
    va_list va;
    va_start(va,sep);
    out = Str_Join_Va_Npl(sep,va);
    va_end(va);
    return out;
}
#endif
;

#define Str_Join_Npl_2(Sep,S1,S2) Str_Join_Npl_(Sep,S1,S2,(char*)0)
#define Str_Join_Npl_3(Sep,S1,S2,S3) Str_Join_Npl_(Sep,S1,S2,S3,(char*)0)
#define Str_Join_Npl_4(Sep,S1,S2,S3,S4) Str_Join_Npl_(Sep,S1,S2,S3,S4,(char*)0)
#define Str_Join_Npl_5(Sep,S1,S2,S3,S4,S5) Str_Join_Npl_(Sep,S1,S2,S3,S4,S5,(char*)0)
#define Str_Join_Npl_6(Sep,S1,S2,S3,S4,S5,S6) Str_Join_Npl_(Sep,S1,S2,S3,S4,S5,S6,(char*)0)

char *Str_Join_(char sep, ...)
#ifdef _YO_STRING_BUILTIN
{
    char *out;
    va_list va;
    va_start(va,sep);
    out = Yo_Pool(Str_Join_Va_Npl(sep,va));
    va_end(va);
    return out;
}
#endif
;

#define Str_Join_2(Sep,S1,S2) Str_Join_(Sep,S1,S2,(char*)0)
#define Str_Join_3(Sep,S1,S2,S3) Str_Join_(Sep,S1,S2,S3,(char*)0)
#define Str_Join_4(Sep,S1,S2,S3,S4) Str_Join_(Sep,S1,S2,S3,S4,(char*)0)
#define Str_Join_5(Sep,S1,S2,S3,S4,S5) Str_Join_(Sep,S1,S2,S3,S4,S5,(char*)0)
#define Str_Join_6(Sep,S1,S2,S3,S4,S5,S6) Str_Join_(Sep,S1,S2,S3,S4,S5,S6,(char*)0)

#define Str_Unicode_Concat(A,B) ((wchar_t*)__Pool(Str_Unicode_Concat_Npl(A,B)))
wchar_t *Str_Unicode_Concat_Npl(wchar_t *a, wchar_t *b)
#ifdef _YO_STRING_BUILTIN
{
    int a_len = a?wcslen(a):0;
    int b_len = b?wcslen(b):0;
    wchar_t *out = Yo_Malloc_Npl((a_len+b_len+1)*sizeof(wchar_t));
    if ( a_len )
        memcpy(out,a,a_len*sizeof(wchar_t));
    if ( b_len )
        memcpy(out+a_len,b,b_len*sizeof(wchar_t));
    out[a_len+b_len] = 0;
    return out;
}
#endif
;

wchar_t *Str_Unicode_Join_Va_Npl(int sep, va_list va)
#ifdef _YO_STRING_BUILTIN
{
    int len = 0;
    wchar_t *q, *out = 0;
    va_list va2;
    #ifdef __GNUC__
    va_copy(va2,va);
    #else
    va2 = va;
    #endif
    while ( !!(q = va_arg(va2,wchar_t *)) )
        len += (wcslen(q)+(sep?1:0))*sizeof(wchar_t);
    if ( len )
    {
        wchar_t *Q = out = Yo_Malloc_Npl( (len+(sep?0:1))*sizeof(wchar_t) );
        while ( !!(q = va_arg(va,wchar_t *)) )
        {
            int l = wcslen(q);
            if ( sep && Q != out )
                *Q++ = sep;
            memcpy(Q,q,l*sizeof(wchar_t));
            Q += l;
        }
        *Q = 0;
    }
    else
    {
        out = Yo_Malloc_Npl(1*sizeof(wchar_t));
        *out = 0;
    }
    return out;
}
#endif
;

wchar_t *Str_Unicode_Join_Npl_(int sep, ...)
#ifdef _YO_STRING_BUILTIN
{
    wchar_t *out;
    va_list va;
    va_start(va,sep);
    out = Str_Unicode_Join_Va_Npl(sep,va);
    va_end(va);
    return out;
}
#endif
;

#define Str_Unicode_Join_Npl_2(Sep,S1,S2) Str_Unicode_Join_Npl_(Sep,S1,S2,(char*)0)
#define Str_Unicode_Join_Npl_3(Sep,S1,S2,S3) Str_Unicode_Join_Npl_(Sep,S1,S2,S3,(char*)0)
#define Str_Unicode_Join_Npl_4(Sep,S1,S2,S3,S4) Str_Unicode_Join_Npl_(Sep,S1,S2,S3,S4,(char*)0)
#define Str_Unicode_Join_Npl_5(Sep,S1,S2,S3,S4,S5) Str_Unicode_Join_Npl_(Sep,S1,S2,S3,S4,S5,(char*)0)
#define Str_Unicode_Join_Npl_6(Sep,S1,S2,S3,S4,S5,S6) Str_Unicode_Join_Npl_(Sep,S1,S2,S3,S4,S5,S6,(char*)0)

wchar_t *Str_Unicode_Join_(int sep, ...)
#ifdef _YO_STRING_BUILTIN
{
    wchar_t *out;
    va_list va;
    va_start(va,sep);
    out = __Pool(Str_Unicode_Join_Va_Npl(sep,va));
    va_end(va);
    return out;
}
#endif
;

#define Str_Unicode_Join_2(Sep,S1,S2) Str_Unicode_Join_(Sep,S1,S2,(char*)0)
#define Str_Unicode_Join_3(Sep,S1,S2,S3) Str_Unicode_Join_(Sep,S1,S2,S3,(char*)0)
#define Str_Unicode_Join_4(Sep,S1,S2,S3,S4) Str_Unicode_Join_(Sep,S1,S2,S3,S4,(char*)0)
#define Str_Unicode_Join_5(Sep,S1,S2,S3,S4,S5) Str_Unicode_Join_(Sep,S1,S2,S3,S4,S5,(char*)0)
#define Str_Unicode_Join_6(Sep,S1,S2,S3,S4,S5,S6) Str_Unicode_Join_(Sep,S1,S2,S3,S4,S5,S6,(char*)0)

#define Str_From_Int(Value) Str_From_Int_Base(Value,10)
char *Str_From_Int_Base(long value, int base)
#ifdef _YO_STRING_BUILTIN
{
    char syms[70] = {0};
    int l = 0;
    switch ( base )
    {
        case 16:
            l = sprintf(syms,"0x%lx",value); break;
        case 8:
            l = sprintf(syms,"%0lo",value); break;
        case 10:
        default:
            l = sprintf(syms,"%ld",value); break;
    }
    return Str_Copy_L(syms,l);
}
#endif
;

#define Str_From_Flt(Value) Str_From_Flt_Perc(Value,3);
char *Str_From_Flt_Perc(double value, int perc)
#ifdef _YO_STRING_BUILTIN
{
    int l;
    char syms[70] = {0};
    char fmt[] = "%._f";
    if ( perc )
        fmt[2] = (perc%9)+'0';
    l = sprintf(syms,fmt,value);
    return Str_Copy_L(syms,l);
}
#endif
;

char *Str_From_Bool(int b)
#ifdef _YO_STRING_BUILTIN
{
    if ( b )
        return Str_Copy_L("#true",5);
    else
        return Str_Copy_L("#false",6);
}
#endif
;

int Str_To_Bool_Dflt(char *S,int dflt)
#ifdef _YO_STRING_BUILTIN
{
    if ( S && *S == '#' ) ++S;
    if ( !S || !*S || !strcmp_I(S,"no") || !strcmp_I(S,"off")
         || !strcmp_I(S,"false") || !strcmp_I(S,"1") )
        return 0;
    if ( !strcmp_I(S,"yes") || !strcmp_I(S,"on") || !strcmp_I(S,"true")
         || !strcmp_I(S,"0") )
        return 1;
    return dflt;
}
#endif
;

int Str_To_Bool(char *S)
#ifdef _YO_STRING_BUILTIN
{
    int q = Str_To_Bool_Dflt(S,3);
    if ( q == 3 )
        __Raise(YO_ERROR_ILLFORMED,__Format("invalid logical value '%s'",S));
    return q;
}
#endif
;

long Str_To_Int(char *S)
#ifdef _YO_STRING_BUILTIN
{
    long l = 0;

    if (!S)
        __Raise(YO_ERROR_NULL_PTR,0);
    else
    {
        char *ep = 0;
        l = strtol(S,&ep,0);
        if ( !*S || *ep )
            __Raise(YO_ERROR_ILLFORMED,__Format("invalid integer value '%s'",S));
    }

    return l;
}
#endif
;

long Str_To_Int_Dflt(char *S, long dflt)
#ifdef _YO_STRING_BUILTIN
{
    long l;
    if (!S)
        l = dflt;
    else
    {
        char *ep = 0;
        l = strtol(S,&ep,0);
        if ( !*S || *ep )
            l = dflt;
    }
    return l;
}
#endif
;

double Str_To_Flt(char *S)
#ifdef _YO_STRING_BUILTIN
{
    double l = 0;

    if (!S)
        __Raise(YO_ERROR_NULL_PTR,0);
    else
    {
        char *ep = 0;
        l = strtod(S,&ep);
        if ( !*S || *ep )
            __Raise(YO_ERROR_ILLFORMED,__Format("invalid float value '%s'",S));
    }

    return l;
}
#endif
;

double Str_To_Flt_Dflt(char *S, double dflt)
#ifdef _YO_STRING_BUILTIN
{
    double l = 0;

    if (!S)
        l = dflt;
    else
    {
        char *ep = 0;
        l = strtod(S,&ep);
        if ( !*S || *ep )
            l = dflt;
    }

    return l;
}
#endif
;

#define Str_Equal_Nocase(Cs,Ct) Str_Equal_Nocase_Len(Cs,Ct,INT_MAX)
int Str_Equal_Nocase_Len(char *S, char *T, int L)
#ifdef _YO_STRING_BUILTIN
{
    wchar_t c;
    char *Se = S+L;
    char *Te = T+L;

    if (L <= 0) return 0;

    do
    {
        if ( (c = Utf8_Get_Wide(&S)) != Utf8_Get_Wide(&T) )
            return 0;
    }
    while ( c && S < Se && T < Te );
    return 1;
}
#endif
;

enum
{
    YO_BOM_DOESNT_PRESENT = 0,
    YO_BOM_UTF16_LE = 1,
    YO_BOM_UTF16_BE = 2,
    YO_BOM_UTF8 = 3,
};

int Str_Find_BOM(void *S)
#ifdef _YO_STRING_BUILTIN
{
    if ( *(byte_t *)S == 0x0ff && ((byte_t *)S)[1] == 0x0fe )
        return YO_BOM_UTF16_LE;
    if ( *(byte_t *)S == 0x0fe && ((byte_t *)S)[1] == 0x0ff )
        return YO_BOM_UTF16_BE;
    if ( *(byte_t *)S == 0x0ef && ((byte_t *)S)[1] == 0x0bb && ((byte_t *)S)[1] == 0x0bf )
        return YO_BOM_UTF8;
    return YO_BOM_DOESNT_PRESENT;
}
#endif
;

int Str_Starts_With(char *S, char *patt)
#ifdef _YO_STRING_BUILTIN
{
    if ( !patt || !S ) return 0;

    while ( *patt )
        if ( *S++ != *patt++ )
            return 0;
    return 1;
}
#endif
;

int Str_Ends_With(char *S, char *patt)
#ifdef _YO_STRING_BUILTIN
{
    if ( patt && S )
    {
        int S_L = strlen(S);
        int patt_L = strlen(patt);
        if ( patt_L < S_L )
        {
            S += S_L-patt_L;
            while ( *patt )
                if ( *S++ != *patt++ )
                    return 0;
            return 1;
        }
    }
    return 0;
}
#endif
;

int Str_Unicode_Starts_With(wchar_t *S, wchar_t *patt)
#ifdef _YO_STRING_BUILTIN
{
    if ( !patt || !S ) return 0;

    while ( *patt )
        if ( *S++ != *patt++ )
            return 0;
    return 1;
}
#endif
;

int Str_Unicode_Starts_With_Nocase(wchar_t *S, wchar_t *patt)
#ifdef _YO_STRING_BUILTIN
{
    if ( !patt || !S ) return 0;

    while ( *patt )
        if ( !*S || towupper(*S++) != towupper(*patt++) )
            return 0;

    return 1;
}
#endif
;

void Str_Cat(char **inout, char *S, int L)
#ifdef _YO_STRING_BUILTIN
{

    int count = *inout?strlen(*inout):0;
    if ( L < 0 ) L = S?strlen(S):0;

    __Elm_Append(inout,count,S,L,1,0);
}
#endif
;

void Str_Unicode_Cat(wchar_t **inout, wchar_t *S, int L)
#ifdef _YO_STRING_BUILTIN
{
    int count = *inout?wcslen(*inout):0;
    if ( L < 0 ) L = S?wcslen(S):0;

    __Elm_Append(inout,count,S,L,sizeof(wchar_t),0);
}
#endif
;

wchar_t *Str_Unicode_Cr_To_CfLr_Inplace(wchar_t **S_ptr)
#ifdef _YO_STRING_BUILTIN
{
    int capacity = 0;
    wchar_t *S = *S_ptr;
    int i, L = wcslen(S);
    for ( i = 0; i < L; ++i )
    {
        if ( S[i] == '\n' && ( !i || S[i-1] != '\r' ) )
        {
            L += __Elm_Insert(&S,i,L,L"\r",1,sizeof(wchar_t),&capacity);
            ++i;
        }
    }
    *S_ptr = S;
    return S;
}
#endif
;

#define Str_Unicode_Search(S,Patt) Str_Unicode_Search_(S,-1,Patt,-1,0)
#define Str_Unicode_Search_Nocase(S,Patt) Str_Unicode_Search_(S,-1,Patt,-1,1)
int Str_Unicode_Search_( wchar_t *S, int L, wchar_t *patt, int pattL, int nocase )
#ifdef _YO_STRING_BUILTIN
{
    wchar_t *p, *pE;

    if ( L < 0 ) L = S?wcslen(S):0;
    if ( pattL < 0 ) pattL = patt?wcslen(patt):0;

    if ( L < pattL ) return -1;

    for ( p = S, pE = S+L-pattL+1; p < pE; ++p )
        if ( *p == *patt )
        {
            int i = 0;
            for ( ; i < pattL; ++i )
                if ( !nocase )
                { if ( patt[i] != p[i] ) break; }
                else
                { if ( towupper(patt[i]) != towupper(p[i]) ) break; }
            if (i == pattL)
                return p-S;
        }

    return -1;
}
#endif
;

/* #define Str_Search_Nocase(S,Patt) Str_Search_Nocase_(S,-1,Patt,-1,1) */
#define Str_Search(S,Patt) Str_Search_(S,-1,Patt,-1)
int Str_Search_( char *S, int L, char *patt, int pattL )
#ifdef _YO_STRING_BUILTIN
{
    char *p, *pE;

    if ( L < 0 ) L = S?strlen(S):0;
    if ( pattL < 0 ) pattL = patt?strlen(patt):0;

    if ( L < pattL ) return -1;

    for ( p = S, pE = S+L-pattL+1; p < pE; ++p )
        if ( *p == *patt )
        {
            int i = 0;
            for ( ; i < pattL; ++i )
            { if ( patt[i] != p[i] ) break; }
            if (i == pattL)
                return p-S;
        }

    return -1;
}
#endif
;

#define Str_Replace_Npl(S,Patt,Val) Str_Replace_Npl_(S,-1,Patt,-1,Val,-1)
#define Str_Replace(S,Patt,Val) __Pool(Str_Replace_Npl_(S,-1,Patt,-1,Val,-1))
char *Str_Replace_Npl_(char *S, int L, char *patt, int pattL, char *val, int valL)
#ifdef _YO_STRING_BUILTIN
{
    int i;
    char *R = 0;
    int R_count = 0;
    int R_capacity = 0;

    if ( pattL < 0 ) pattL = patt?strlen(patt):0;
    if ( valL < 0 ) valL = val?strlen(val):0;

    __Elm_Resize_Npl(&R, 32, 1, &R_capacity);

    if ( pattL )
        while ( 0 <= (i = Str_Search_(S,L,patt,pattL)) )
        {
            if ( i )
                R_count += __Elm_Append_Npl(&R,R_count,S,i,1,&R_capacity);
            if ( valL )
                R_count += __Elm_Append_Npl(&R,R_count,val,valL,1,&R_capacity);
            L -= i+pattL; S += i+pattL;
        }

    if ( L )
        __Elm_Append_Npl(&R,R_count,S,L,1,&R_capacity);

    return R;
}
#endif
;

#define Str_Unicode_Replace_Npl(S,Patt,Val) Str_Unicode_Replace_Npl_(S,-1,Patt,-1,Val,-1,0)
#define Str_Unicode_Replace_Nocase_Npl(S,Patt,Val) __Pool(Str_Unicode_Replace_Npl_(S,-1,Patt,-1,Val,-1,1))
#define Str_Unicode_Replace(S,Patt,Val) Str_Unicode_Replace_Npl_(S,-1,Patt,-1,Val,-1,0)
#define Str_Unicode_Replace_Nocase(S,Patt,Val) __Pool(Str_Unicode_Replace_Npl_(S,-1,Patt,-1,Val,-1,1))
wchar_t *Str_Unicode_Replace_Npl_(wchar_t *S, int L, wchar_t *patt, int pattL, wchar_t *val, int valL, int nocase)
#ifdef _YO_STRING_BUILTIN
{
    int i;
    wchar_t *R = 0;
    int R_count = 0;
    int R_capacity = 0;

    if ( pattL < 0 ) pattL = patt?wcslen(patt):0;
    if ( valL < 0 ) valL = val?wcslen(val):0;

    if ( pattL )
        while ( 0 <= (i = Str_Unicode_Search_(S,L,patt,pattL,nocase)) )
        {
            if ( i )
                R_count += __Elm_Append_Npl(&R,R_count,S,i,sizeof(wchar_t),&R_capacity);
            if ( valL )
                R_count += __Elm_Append_Npl(&R,R_count,val,valL,sizeof(wchar_t),&R_capacity);
            S += i+pattL; L -= i+pattL;
        }

    if ( L )
        __Elm_Append_Npl(&R,R_count,S,L,sizeof(wchar_t),&R_capacity);

    return R;
}
#endif
;

#define Str_Match(S,Patt) Str_Submatch(S,Patt,0)
#define Str_Match_Nocase(S,Patt) Str_Submatch(S,Patt,1)
int Str_Submatch(char *S, char *patt, int nocase);

int Str_Submatch_Nocase_(char *S, char *patt)
#ifdef _YO_STRING_BUILTIN
{
    char *SS = S;

    if ( *S )
    {
        do
        {
            int l = Utf8_Char_Length[(byte_t)*S];
            if ( l == 1 )
                ++S;
            else
            {
                if ( *(S+l) )
                    if ( Str_Submatch_Nocase_(S+l,patt) )
                        return 1;
                break;
            }
        }
        while ( *S );

        for ( ; S != SS; --S )
            if ( Str_Submatch(S,patt,1) )
                return 1;
    }

    return 0;
}
#endif
;

#ifdef _YO_STRING_BUILTIN
int Str_Submatch(char *S, char *patt, int nocase)
{
    if ( S && patt )
    {
        while ( *S && *patt )
        {
            char *SS = S;
            int c = nocase ? Utf8_Get_Wide(&SS) : *SS++;
            int pc = nocase ? Utf8_Get_Wide(&patt) : *patt++;

            switch ( pc )
            {
                case '?': S = SS; break;
                case '*':
                    if ( !*patt )
                        return 1;
                    if ( nocase )
                        return Str_Submatch_Nocase_(SS,patt);
                    else
                    {
                        while ( *SS ) ++SS;
                        while ( S != --SS )
                            if ( Str_Submatch(SS,patt,0) )
                                return 1;
                    }
                    return 0;
                //case '[':
                default:
                    if ( c != pc ) return 0;
                    S = SS;
            }
        }
        return !*S && *S==*patt;
    }
    return 0;
}
#endif

char *Str_Fetch_Substr(char *S, char *prefx, char *skip, char *stopat)
#ifdef _YO_STRING_BUILTIN
{
    int j = 0;
    char *qoo;
    char *Q = strstr(S,prefx);
    if ( Q )
    {
        Q += strlen(prefx);
        if ( skip )
        l:
            for ( qoo = skip; *Q && *qoo; ++qoo ) if ( *qoo == *Q ) { ++Q; goto l; }
        for ( ; Q[j]; ++j )
            if ( stopat )
                for ( qoo = stopat; *qoo; ++qoo )
                    if ( *qoo == Q[j] )
                        goto ret;
    }
ret:
    if ( Q && j ) return Str_Copy_L(Q,j);
    return 0;
}
#endif
;

#define Str_Reverse(S,L) __Pool(Str_Reverse_Npl(S,L))
char *Str_Reverse_Npl(char *S, int L)
#ifdef _YO_STRING_BUILTIN
{
    int i;
    char *ret;
    if ( L < 0 ) L = S?strlen(S):0;
    ret = __Malloc_Npl(L+1);
    for ( i = 0; i < L; ++i )
        ret[i] = S[(L-i)-1];
    ret[L] = 0;
    return ret;
}
#endif
;

wchar_t *Str_Unicode_Transform_Npl(wchar_t *S, int L, wchar_t (*transform)(wchar_t))
#ifdef _YO_STRING_BUILTIN
{
    int i;
    wchar_t *ret;
    if ( L < 0 ) L = S?wcslen(S):0;
    ret = __Malloc_Npl((L+1)*sizeof(wchar_t));
    for ( i = 0; i < L; ++i )
        ret[i] = transform(S[i]);
    ret[i] = 0;
    return ret;
}
#endif
;

#define Str_Unicode_Upper(S,L) ((wchar_t*)__Pool(Str_Unicode_Upper_Npl(S,L)))
#define Str_Unicode_Upper_Npl(S,L) Str_Unicode_Transform_Npl(S,L,(void*)towupper)
#define Str_Unicode_Lower(S,L) ((wchar_t*)__Pool(Str_Unicode_Lower_Npl(S,L)))
#define Str_Unicode_Lower_Npl(S,L) Str_Unicode_Transform_Npl(S,L,(void*)towlower)

char *Str_Ansi_Transform_Npl(char *S, int L, char (*transform)(char))
#ifdef _YO_STRING_BUILTIN
{
    int i;
    char *ret;
    if ( L < 0 ) L = S?strlen(S):0;
    ret = __Malloc_Npl(L+1);
    for ( i = 0; i < L; ++i )
        ret[i] = transform(S[i]);
    ret[i] = 0;
    return ret;
}
#endif
;

#define Str_Ansi_Upper(S,L) ((char*)__Pool(Str_Ansi_Upper_Npl(S,L)))
#define Str_Ansi_Upper_Npl(S,L) Str_Ansi_Transform_Npl(S,L,(void*)toupper)
#define Str_Ansi_Lower(S,L) ((char*)__Pool(Str_Ansi_Lower_Npl(S,L)))
#define Str_Ansi_Lower_Npl(S,L) Str_Ansi_Transform_Npl(S,L,(void*)tolower)

char *Str_Utf8_Transform_Npl(char *S, int L, wchar_t (*transform)(wchar_t) )
#ifdef _YO_STRING_BUILTIN
{
    char *E, *R = 0;
    int R_capacity = 1;
    int R_count = 0;

    if ( S )
    {
        if ( L < 0 ) L = strlen(S);
        R_capacity = L+1;
        for ( E = S+L; S < E; )
        {
            int wc_L = 0;
            byte_t b[8];
            wchar_t wc = Utf8_Get_Wide(&S);
            wc = transform(wc);
            Utf8_Wide_Encode(b,wc,&wc_L);
            R_count += __Elm_Append_Npl(&R,R_count,b,wc_L,1,&R_capacity);
        }
    }

    __Elm_Append_Npl(&R,R_count,"\0",1,1,&R_capacity);
    return R;
}
#endif
;

#define Str_Utf8_Upper(S)   Str_Utf8_Upper_L(S,-1)
#define Str_Utf8_Upper_L(S,L) ((char*)__Pool(Str_Utf8_Upper_Npl(S,L)))
#define Str_Utf8_Upper_Npl(S,L) Str_Utf8_Transform_Npl(S,L,(void*)towupper)
#define Str_Utf8_Lower(S)     Str_Utf8_Lower_L(S,-1)
#define Str_Utf8_Lower_L(S,L) ((char*)__Pool(Str_Utf8_Lower_Npl(S,L)))
#define Str_Utf8_Lower_Npl(S,L) Str_Utf8_Transform_Npl(S,L,(void*)towlower)

char *Str_Safe_Quote(char *S)
#ifdef _YO_STRING_BUILTIN
{
    int S_len = S? strlen(S):0;
    int R_count = 0;
    int R_capacity = S_len+1;
    char *R = 0;

    if ( S )
        for ( ; *S; ++S )
        {
            if ( 0 ) ;
            else if ( *(signed char *)S < 30
                      || *S == '|' || *S == '&' || *S == ';'
                      || *S == '<' || *S == '>' || *S == '['
                      || *S == ']' || *S == '{' || *S == '}'
                      || *S == '"' || *S == '\'' ||*S == '\\'
                      || *S == '#' || *S == '$' //|| *S == '%'
                      || *S == '?' || *S == '=' || *S == '`' )
            {
                char b[3] = {'#',0,0};
                Unsigned_To_Hex2(*S,b+1);
                R_count += __Elm_Append_Npl(&R,R_count,b,3,1,&R_capacity);
            }
            else R_count += __Elm_Append_Npl(&R,R_count,S,1,1,&R_capacity);
        }

    return __Pool(R);
}
#endif
;

char *Str_Escape(char *S)
#ifdef _YO_STRING_BUILTIN
{
    int S_len = S? strlen(S):0;
    int R_count = 0;
    int R_capacity = S_len+1;
    char *R = 0;

    if ( S )
        for ( ; *S; ++S )
        {
            if ( 0 ) ;
            else if ( *(signed char *)S < 30
                      || *S == '"' || *S == '\'' ||*S == '\\' )
            {
                char b[5] = {0,};
                b[0] = '\\';
                b[1] = ((*S>>6)%8) + '0';
                b[2] = ((*S>>3)%8) + '0';
                b[3] = (*S%8) + '0';
                //Str_Hex_Byte(*S,'\\',b);
                R_count += __Elm_Append_Npl(&R,R_count,b,4,1,&R_capacity);
            }
            else R_count += __Elm_Append_Npl(&R,R_count,S,1,1,&R_capacity);
        }

    return __Pool(R);
}
#endif
;

#endif /* C_once_0ED387CD_668B_44C3_9D91_A6336A2F5F48 */

