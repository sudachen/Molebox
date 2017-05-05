
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

#ifndef C_once_FF657866_8205_4CAE_9D01_65B8583E9D19
#define C_once_FF657866_8205_4CAE_9D01_65B8583E9D19

#ifdef _LIBYOYO
#define _YO_RANDOM_BUILTIN
#endif

#include "yoyo.hc"
#include "sha1.hc"

#ifdef __windoze
# include <wincrypt.h>
# ifdef _MSC_VER
#  pragma comment(lib,"advapi32.lib")
# endif
#else
#endif

#define _YO_DEV_RANDOM "/dev/urandom"

#ifdef _YO_RANDOM_BUILTIN
# define _YO_RANDOM_EXTERN
# define _YO_RANDOM_BUILTIN_CODE(Code) Code
#else
# define _YO_RANDOM_EXTERN extern
# define _YO_RANDOM_BUILTIN_CODE(Code)
#endif

void Soft_Random(byte_t *bits, int count)
#ifdef _YO_RANDOM_BUILTIN
  {
    static uquad_t rnd_ct[4] = {0};
    static byte_t  rnd_bits[20] = {0}; 
    static int rnd_bcont = 0;
    static int initialized = 0;
    __Xchg_Interlock
      {
        if ( !initialized )
          {
            rnd_ct[0] = ((quad_t)getpid() << 48 ) | (quad_t)time(0);
            rnd_ct[1] = 0;
            rnd_ct[2] = 0;
            rnd_ct[3] = (longptr_t)&bits;
            initialized = 1;
          }
          
        while ( count )
          {
            if ( !rnd_bcont )
              {
                rnd_ct[1] = clock();
                rnd_ct[2] = (rnd_ct[2] + ((quad_t)count ^ (longptr_t)bits)) >> 1;
                Sha1_Digest(rnd_ct,sizeof(rnd_ct),rnd_bits);
                ++rnd_ct[3];
                rnd_bcont = sizeof(rnd_bits);
              }
            *bits++ = rnd_bits[--rnd_bcont];
            --count;
          }
      }
  }
#endif
  ;

byte_t *Soft_Random_Bytes(int count)
#ifdef _YO_RANDOM_BUILTIN
  {
    byte_t *b = __Malloc(count);
    Soft_Random(b,count);
    return b;
  }
#endif
  ;
  
void System_Random(void *bits,int count /* of bytes*/ )
#ifdef _YO_RANDOM_BUILTIN
  {
  #ifdef _SOFTRND
    goto simulate;
  #elif !defined __windoze
    int i, fd = open(_YO_DEV_RANDOM,O_RDONLY|O_NONBLOCK);
    if ( fd >= 0 )
      {
        for ( i = 0; i < count; )
          {
            int rd = read(fd,bits+i,count-i);
            if ( rd < 0 )
              {
                if ( rd == EAGAIN )
                  {
                    Soft_Random(bits+i,count-i);
                    break;
                  }
                else
                  {
                    char *err = strerror(errno);
                    close(fd);
                    __Raise_Format(YO_ERROR_IO,
                      (_YO_DEV_RANDOM " does not have required data: %s",err));
                  }
              }
            i += rd;
          }
        close(fd);
        return;
      }
    else
      goto simulate;
  #else
    typedef BOOL (__stdcall *tCryptAcquireContext)(HCRYPTPROV*,LPCTSTR,LPCTSTR,DWORD,DWORD);
    typedef BOOL (__stdcall *tCryptGenRandom)(HCRYPTPROV,DWORD,BYTE*);
    static tCryptAcquireContext fCryptAcquireContext = 0;
    static tCryptGenRandom fCryptGenRandom = 0;
    static HCRYPTPROV cp = 0;
    if ( !fCryptAcquireContext )
      {
        HMODULE hm = LoadLibraryA("advapi32.dll");
        fCryptAcquireContext = (tCryptAcquireContext)GetProcAddress(hm,"CryptAcquireContextA");
        fCryptGenRandom = (tCryptGenRandom)GetProcAddress(hm,"CryptGenRandom");
      }
    if ( !cp && (!fCryptAcquireContext || !fCryptAcquireContext(&cp, 0, 0,PROV_RSA_FULL, CRYPT_VERIFYCONTEXT)) )
      goto simulate;
    if ( !fCryptGenRandom || !fCryptGenRandom(cp,count,(unsigned char*)bits) )
      goto simulate;
    if ( count >= 4 && *(unsigned*)bits == 0 )
      goto simulate;
    return;
  #endif      
  simulate:
    Soft_Random(bits,count);
  }
#endif
  ;

ulong_t Random_Bits(int no)
#ifdef _YO_RANDOM_BUILTIN
  {
    static byte_t bits[128] = {0};
    static int bits_count = 0;
    ulong_t r = 0;
    
    STRICT_REQUIRE( no > 0 && no <= sizeof(ulong_t)*8 );
    
    __Xchg_Interlock
      while ( no )
        {
          if ( !bits_count )
            {
              System_Random(bits,sizeof(bits));
              bits_count = sizeof(bits)*8;
            }
          no -= Bits_Pop(&r,bits,&bits_count,no);
        }

    return r;
  }
#endif
  ;

#if 0
ulong_t Get_Random(unsigned min, unsigned max)
#ifdef _YO_RANDOM_BUILTIN
  {
    ulong_t r;
    int k = sizeof(r)*8/2;
    STRICT_REQUIRE(max > min);
    r = ((Random_Bits(k)*(max-min))>>k) + min;
    STRICT_REQUIRE(r >= min && r < max);
    return r;
  }
#endif
  ;
#endif

uint_t Get_Random(uint_t min, uint_t max)
#ifdef _YO_RANDOM_BUILTIN
  {
    uint_t r;
    uint_t dif = max - min;
    STRICT_REQUIRE(max > min);
    r = (uint_t)(((uquad_t)Random_Bits(32)*dif) >> 32) + min;
    STRICT_REQUIRE(r >= min && r < max);
    return r;
  }
#endif
  ;

/*
Tiny Mersenne Twister only 127 bit internal state
Mutsuo Saito (Hiroshima University)
Makoto Matsumoto (University of Tokyo)
Copyright (C) 2011 Mutsuo Saito, Makoto Matsumoto,
Hiroshima University and The University of Tokyo.
All rights reserved.
The 3-clause BSD License is applied to this software
*/

enum
  {
    YO_FASTRND_TINYMT32_MEXP = 127,
    YO_FASTRND_TINYMT32_SH0  = 1,
    YO_FASTRND_TINYMT32_SH1  = 10,
    YO_FASTRND_TINYMT32_SH8  = 8,
    YO_FASTRND_TINYMT32_MASK = 0x7fffffff,
  };
  
typedef struct _YO_FASTRND
  {
    uint_t status[4];
    uint_t mat1;
    uint_t mat2;
    uint_t tmat;
  } YO_FASTRND;

void Fast_Random_Next_State(YO_FASTRND *random)
#ifdef _YO_RANDOM_BUILTIN
  {
    uint_t x;
    uint_t y;
    
    y = random->status[3];
    x = (random->status[0] & YO_FASTRND_TINYMT32_MASK)
        ^ random->status[1]
        ^ random->status[2];
    x ^= (x << YO_FASTRND_TINYMT32_SH0);
    y ^= (y >> YO_FASTRND_TINYMT32_SH0) ^ x;
    random->status[0] = random->status[1];
    random->status[1] = random->status[2];
    random->status[2] = x ^ (y << YO_FASTRND_TINYMT32_SH1);
    random->status[3] = y;
    random->status[1] ^= -((int)(y & 1)) & random->mat1;
    random->status[2] ^= -((int)(y & 1)) & random->mat2;
  }
#endif
  ;

uint_t Fast_Random_Temper(YO_FASTRND *random)
#ifdef _YO_RANDOM_BUILTIN
  {
    uint_t t0, t1;
    t0 = random->status[3];
    t1 = random->status[0] + (random->status[2] >> YO_FASTRND_TINYMT32_SH8);
    t0 ^= t1;
    t0 ^= -((int)(t1 & 1)) & random->tmat;
    return t0;
  }
#endif
  ;

_YO_RANDOM_EXTERN char Oj_Random_32_OjMID[] _YO_RANDOM_BUILTIN_CODE( = "random_32/@"); 
uint_t Oj_Random_32(void *random) _YO_RANDOM_BUILTIN_CODE(
  { return 
      ((uint_t(*)(void*))Yo_Find_Method_Of(&random,Oj_Random_32_OjMID,YO_RAISE_ERROR))
        (random); });

_YO_RANDOM_EXTERN char Oj_Random_Flt_OjMID[] _YO_RANDOM_BUILTIN_CODE( = "random_flt/@"); 
float Oj_Random_Flt(void *random) _YO_RANDOM_BUILTIN_CODE(
  { return 
      ((float(*)(void*))Yo_Find_Method_Of(&random,Oj_Random_Flt_OjMID,YO_RAISE_ERROR))
        (random); });

_YO_RANDOM_EXTERN char Oj_Random_OjMID[] _YO_RANDOM_BUILTIN_CODE( = "random/@ii"); 
int Oj_Random(void *random,int min_val,int max_val) _YO_RANDOM_BUILTIN_CODE(
  { return 
      ((int(*)(void*,int,int))Yo_Find_Method_Of(&random,Oj_Random_OjMID,YO_RAISE_ERROR))
        (random,min_val,max_val); });

uint_t Fast_Random_Next_32(YO_FASTRND *random)
#ifdef _YO_RANDOM_BUILTIN
  {
    Fast_Random_Next_State(random);
    return Fast_Random_Temper(random);
  }
#endif
  ;

float Fast_Random_Next_Flt(YO_FASTRND *random)
#ifdef _YO_RANDOM_BUILTIN
  {
    Fast_Random_Next_State(random);
    return Fast_Random_Temper(random) * (1.0f / 4294967296.0f);
  }
#endif
  ;

int Fast_Random(YO_FASTRND *random,int min_val, int max_val)
#ifdef _YO_RANDOM_BUILTIN
  {
    quad_t q;
    Fast_Random_Next_State(random);
    q = Fast_Random_Temper(random);
    return (int)((q * max_val)>>32) + min_val;
  }
#endif
  ;

void Fast_Random_Period_Certification(YO_FASTRND *random) 
#ifdef _YO_RANDOM_BUILTIN
  {
    if ( !(random->status[0] & YO_FASTRND_TINYMT32_MASK)
      && !random->status[1] 
      && !random->status[2]
      && !random->status[3]) 
      {
        random->status[0] = 'T';
        random->status[1] = 'I';
        random->status[2] = 'N';
        random->status[3] = 'Y';
      }
  }
#endif
  ;
  
YO_FASTRND *Fast_Random_Init_Static(YO_FASTRND *random,uint_t seed)
#ifdef _YO_RANDOM_BUILTIN
  {
    int i;
    random->status[0] = seed;
    random->status[1] = random->mat1;
    random->status[2] = random->mat2;
    random->status[3] = random->tmat;
    for (i = 1; i < 8; i++) 
      {
        random->status[i & 3] ^= i + 1812433253U
          * (random->status[(i - 1) & 3]
          ^ (random->status[(i - 1) & 3] >> 30));
      }
    Fast_Random_Period_Certification(random);
    for (i = 0; i < 8; i++)
      Fast_Random_Next_State(random);
    return random;
  }
#endif
  ;

YO_FASTRND *Fast_Random_Init(uint_t seed)
#ifdef _YO_RANDOM_BUILTIN
  {
    static YO_FUNCTABLE funcs[] = 
      { {0},
        {Oj_Random_32_OjMID,  Fast_Random_Next_32},
        {Oj_Random_Flt_OjMID, Fast_Random_Next_Flt},
        {Oj_Random_OjMID,     Fast_Random},
        {0}};
    YO_FASTRND *random = __Object(sizeof(YO_FASTRND),funcs);
    return Fast_Random_Init_Static(random,seed);
  }
#endif
  ;

#endif /* C_once_FF657866_8205_4CAE_9D01_65B8583E9D19 */

