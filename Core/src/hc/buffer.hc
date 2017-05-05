
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

#ifndef C_once_29A1C0D6_2792_4035_8D0E_9DB1797A4120
#define C_once_29A1C0D6_2792_4035_8D0E_9DB1797A4120

#ifdef _LIBYOYO
#define _YO_BUFFER_BUILTIN
#endif

#include "yoyo.hc"

/* string.hc */
char *Str_Hex_Byte(byte_t val,char pfx,void *out);

typedef struct _YO_BUFFER
  { 
    union {
      char   *at; 
      char   *chars;
      byte_t *bytes;
    };
    int count;
    int capacity;
  } YO_BUFFER;

void Buffer_Clear(YO_BUFFER *bf)
#ifdef _YO_BUFFER_BUILTIN
  {
    free(bf->at);
    bf->at = 0;
    bf->count = 0;
    bf->capacity = 0;
  }
#endif
  ;
  
void *Buffer_Init(int count);
YO_BUFFER *Buffer_Reserve(YO_BUFFER *bf,int capacity)
#ifdef _YO_BUFFER_BUILTIN
  {
    if ( capacity < 0 )
      __Raise(YO_ERROR_OUT_OF_RANGE,0);

    if ( !bf ) bf = Buffer_Init(0);

    if ( bf->capacity < capacity || !bf->at )
      {
        bf->at = __Realloc_Npl(bf->at,capacity+1);
        bf->capacity = capacity;
        STRICT_REQUIRE(bf->count <= bf->capacity );
        bf->at[bf->count] = 0;
      }
      
    return bf;
  }
#endif
  ;

YO_BUFFER *Buffer_Grow_Reserve(YO_BUFFER *bf,int capacity)
#ifdef _YO_BUFFER_BUILTIN
  {
    if ( capacity < 0 )
      __Raise(YO_ERROR_OUT_OF_RANGE,0);

    if ( !bf ) bf = Buffer_Init(0);

    if ( bf->capacity < capacity )
      {
        capacity = Min_Pow2(capacity);
        bf->at = __Realloc_Npl(bf->at,capacity+1);
        bf->capacity = capacity;
      }
      
    return bf;
  }
#endif
  ;

void Buffer_Resize(YO_BUFFER *bf,int count)
#ifdef _YO_BUFFER_BUILTIN
  {
    Buffer_Reserve(bf,count);
    if ( bf->count < count ) memset(bf->at+bf->count,0,count-bf->count);
    bf->count = count;
    if ( bf->at ) bf->at[bf->count] = 0;
  }
#endif
  ;

void Buffer_Grow(YO_BUFFER *bf,int count)
#ifdef _YO_BUFFER_BUILTIN
  {
    Buffer_Grow_Reserve(bf,count);
    if ( bf->count < count ) memset(bf->at+bf->count,0,count-bf->count);
    bf->count = count;
    bf->at[bf->count] = 0;
  }
#endif
  ;
void Buffer_Append(YO_BUFFER *bf,void *S,int len)
#ifdef _YO_BUFFER_BUILTIN
  {
    if ( len < 0 ) /* appending C string */
      len = S?strlen(S):0;

    if ( len && S )
      {
        Buffer_Grow_Reserve(bf,bf->count+len);
        memcpy(bf->at+bf->count,S,len);
        bf->count += len;
        bf->at[bf->count] = 0;
      }
  }
#endif
  ;

void Buffer_Fill_Append(YO_BUFFER *bf,int c,int count)
#ifdef _YO_BUFFER_BUILTIN
  {
    STRICT_REQUIRE( count >= 0 );

    if ( count > 0 )
      {
        Buffer_Grow_Reserve(bf,bf->count+count);
        memset(bf->at+bf->count,c,count);
        bf->count += count;
        bf->at[bf->count] = 0;
      }
  }
#endif
  ;

void Buffer_Print(YO_BUFFER *bf, char *S)
#ifdef _YO_BUFFER_BUILTIN
  {
    Buffer_Append(bf,S,-1);
  }
#endif
  ;

void Buffer_Puts(YO_BUFFER *bf, char *S)
#ifdef _YO_BUFFER_BUILTIN
  {
    Buffer_Append(bf,S,-1);
    Buffer_Fill_Append(bf,'\n',1);
  }
#endif
  ;
  
void Buffer_Set(YO_BUFFER *bf, char *S, int L)
#ifdef _YO_BUFFER_BUILTIN
  {
    Buffer_Resize(bf,0);    
    Buffer_Append(bf,S,L);
  }
#endif
  ;

void Buffer_Printf_Va(YO_BUFFER *bf, char *fmt, va_list va)
#ifdef _YO_BUFFER_BUILTIN
  {
    int q, rq_len;
    
    rq_len = Yo_Detect_Required_Buffer_Size(fmt,va)+1;
    Buffer_Grow_Reserve(bf,bf->count+rq_len);
    
  #ifdef __windoze
    q = vsprintf((char*)bf->at+bf->count,fmt,va);
  #else
    q = vsnprintf((char*)bf->at+bf->count,rq_len,fmt,va);
  #endif
  
    if ( q >= 0 )
      bf->count += q;
    STRICT_REQUIRE(bf->count >= 0 && bf->count <= bf->capacity);
  
    bf->at[bf->count] = 0;
  }
#endif
  ;

void Buffer_Printf(YO_BUFFER *bf, char *fmt, ...)
#ifdef _YO_BUFFER_BUILTIN
  {
    va_list va;
    va_start(va,fmt);
    Buffer_Printf_Va(bf,fmt,va);
    va_end(va);
  }
#endif
  ;

void Buffer_Hex_Append(YO_BUFFER *bf, void *S, int len)
#ifdef _YO_BUFFER_BUILTIN
  {
    if ( len < 0 ) /* appending C string */
      len = S?strlen(S):0;

    if ( len && S )
      {
        int i;
        Buffer_Grow_Reserve(bf,bf->count+len*2);
        for ( i = 0; i < len; ++i )
          Str_Hex_Byte( ((byte_t*)S)[i], 0, bf->at+bf->count+i*2 );

        bf->count += len*2;
        bf->at[bf->count] = 0;
      }
  }
#endif
  ;

void Buffer_Esc_Append(YO_BUFFER *bf, void *S, int len)
#ifdef _YO_BUFFER_BUILTIN
  {
    byte_t *q = S;
    byte_t *p = q;
    byte_t *E;
    
    if ( len < 0 ) 
      len = S?strlen(S):0;
    
    E = p + len;
    
    while ( p != E )
      {
        do 
          { 
            if ( *p < 30 || *p > 127 ||*p == '\\' ||  *p == '"' || *p == '\'' ) 
              break; 
            ++p; 
          } 
        while ( p != E );
        
        if ( q != p )
          Buffer_Append(bf,q,p-q);
        
        if ( p != E )
          {
            char *t; 
            Buffer_Fill_Append(bf,0,4);
            STRICT_REQUIRE( bf->count >= 5 );
            t = bf->chars + bf->count - 4;
            t[0] = '\\';
            t[1] = ((*p>>6)%8) + '0';
            t[2] = ((*p>>3)%8) + '0';            
            t[3] = (*p%8) + '0';          
          }
          
        q = ++p;
      }
  }
#endif
  ;
  
void Buffer_Quote_Append(YO_BUFFER *bf, void *S, int len, int brk)
#ifdef _YO_BUFFER_BUILTIN
  {
    byte_t *q = S;
    byte_t *p = q;
    byte_t *E;
    
    if ( len < 0 ) 
      len = S?strlen(S):0;
    
    E = p + len;
    
    while ( p != E )
      {
        do 
          { 
            if ( *p < 30 || *p == '\\' 
              || (brk ? *p == brk : ( *p == '"' || *p == '\'' )) ) 
              break; 
            ++p; 
          } 
        while ( p != E );
        
        if ( q != p )
          Buffer_Append(bf,q,p-q);
        
        if ( p != E )
          {
            if ( *p == '\n' ) Buffer_Append(bf,"\\n",2);
            else if ( *p == '\t' ) Buffer_Append(bf,"\\t",2);
            else if ( *p == '\r' ) Buffer_Append(bf,"\\r",2);
            else if ( *p == '\\' ) Buffer_Append(bf,"\\\\",2);
            else if ( brk && *p == brk )  
              { 
                Buffer_Fill_Append(bf,'\\',1);
                Buffer_Fill_Append(bf,brk,1);
              }
            else if ( !brk && *p == '"' ) 
              Buffer_Append(bf,"\\\"",2);
            else if ( !brk && *p == '\'' ) 
              Buffer_Append(bf,"\\'",2);
            else if ( *p < 30 ) 
              {
                Buffer_Append(bf,"\\x",2);
                Buffer_Hex_Append(bf,p,1);
              }
          
            ++p;
          }
          
        q = p;
      }
  }
#endif
  ;

void Buffer_Html_Quote_Append(YO_BUFFER *bf, void *S, int len)
#ifdef _YO_BUFFER_BUILTIN
  {
    byte_t *q = S;
    byte_t *p = q;
    byte_t *E;
    
    if ( len < 0 ) 
      len = S?strlen(S):0;
    
    E = p + len;
    
    while ( p != E )
      {
        do 
          { 
            if ( *p == '<' || *p == '>'  || *p == '&') 
              break; 
            ++p; 
          } 
        while ( p != E );
        
        if ( q != p )
          Buffer_Append(bf,q,p-q);
        
        if ( p != E )
          {
            if ( *p == '<' ) Buffer_Append(bf,"&lt;",4);
            else if ( *p == '>' ) Buffer_Append(bf,"&gt;",4);
            else if ( *p == '&' ) Buffer_Append(bf,"&amp;",5);
            ++p;
          }
          
        q = p;
      }
  }
#endif
  ;

void Buffer_Insert(YO_BUFFER *bf,int pos,void *S,int len)
#ifdef _YO_BUFFER_BUILTIN
  {
    if ( len < 0 ) /* appending C string */
      len = S?strlen(S):0;

    if ( pos < 0 ) pos = bf->count + pos + 1;
    if ( pos < 0 || pos > bf->count ) 
      __Raise(YO_ERROR_OUT_OF_RANGE,0);
    
    Buffer_Grow_Reserve(bf,bf->count+len);
    if ( pos < bf->count )
      memmove(bf->at+pos+len,bf->at+pos,bf->count-pos);
    memcpy(bf->at+pos,S,len);
    bf->count += len;
    bf->at[bf->count] = 0;
  }
#endif
  ;


#define Buffer_Take_Data(Bf) __Pool(Buffer_Take_Data_Npl(Bf))
void *Buffer_Take_Data_Npl(YO_BUFFER *bf)
#ifdef _YO_BUFFER_BUILTIN
  {
    void *R = bf->at;
    bf->count = 0;
    bf->at = 0;
    bf->capacity = 0;
    return R;
  }
#endif
  ;
  
void *Buffer_Take_Data_Non(YO_BUFFER *bf)
#ifdef _YO_BUFFER_BUILTIN
  {
    void *n = Buffer_Take_Data(bf);
    if ( !n ) 
      { 
        n = Yo_Malloc(1);
        *((char*)n) = 0;
      }
    return n;
  }
#endif
  ;
  
#define Buffer_COUNT(Bf)    ((int)((YO_BUFFER *)(Bf))->count)
#define Buffer_CAPACITY(Bf) ((int)((YO_BUFFER *)(Bf))->capacity)
#define Buffer_BEGIN(Bf)    (((YO_BUFFER *)(Bf))->at)
#define Buffer_END(Bf)      (Buffer_BEGIN(Bf)+Buffer_COUNT(Bf))

int Buffer_Count(YO_BUFFER *bf)
#ifdef _YO_BUFFER_BUILTIN
  {
    if ( bf )
      return Buffer_COUNT(bf);
    return 0;
  }
#endif
  ;
  
int Buffer_Capacity(YO_BUFFER *bf)
#ifdef _YO_BUFFER_BUILTIN
  {
    if ( bf )
      return Buffer_CAPACITY(bf);
    return 0;
  }
#endif
  ;
 void *Buffer_Begin(YO_BUFFER *bf)
#ifdef _YO_BUFFER_BUILTIN
  {
    if ( bf )
      return Buffer_BEGIN(bf);
    return 0;
  }
#endif
  ;

void *Buffer_End(YO_BUFFER *bf)
#ifdef _YO_BUFFER_BUILTIN
  {
    if ( bf )
      return Buffer_END(bf);
    return 0;
  }
#endif
  ;

void Buffer_Destruct(YO_BUFFER *bf)
#ifdef _YO_BUFFER_BUILTIN
  {
    if ( bf->at )
      free(bf->at);
    __Destruct(bf);
  }
#endif
  ;

void *Buffer_Init(int count)
#ifdef _YO_BUFFER_BUILTIN
  {
    YO_BUFFER *bf = Yo_Object_Dtor(sizeof(YO_BUFFER),Buffer_Destruct);
    if ( count )
      Buffer_Resize(bf,count);
    return bf;
  }
#endif
  ;
  
void *Buffer_Acquire(char *S)
#ifdef _YO_BUFFER_BUILTIN
  {
    YO_BUFFER *bf = Yo_Object_Dtor(sizeof(YO_BUFFER),Buffer_Destruct);
    int L = strlen(S);
    bf->at = S;
    bf->count = L;
    bf->capacity = L;
    return bf;
  }
#endif
  ;

void *Buffer_Zero(int count)
#ifdef _YO_BUFFER_BUILTIN
  {
    YO_BUFFER *bf = Buffer_Init(count);
    if ( bf->count ) 
      memset(bf->at,0,bf->count);
    return bf;
  }
#endif
  ;

void *Buffer_Copy(void *S, int count)
#ifdef _YO_BUFFER_BUILTIN
  {
    YO_BUFFER *bf;
    if ( count < 0 ) count = S?strlen(S):0;
    bf = Buffer_Init(count);
    if ( count )
      memcpy(bf->at,S,count);
    return bf;
  }
#endif
  ;

void Buffer_Swap(YO_BUFFER *a, YO_BUFFER *b)
#ifdef _YO_BUFFER_BUILTIN
  {
    YO_BUFFER tmp;
    memcpy(&tmp,a,sizeof(YO_BUFFER));
    memcpy(a,b,sizeof(YO_BUFFER));
    memcpy(b,&tmp,sizeof(YO_BUFFER));
  }
#endif
  ;

#endif /* C_once_29A1C0D6_2792_4035_8D0E_9DB1797A4120 */

