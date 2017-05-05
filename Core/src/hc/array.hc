
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

#ifndef C_once_44A7F9A5_269A_48D5_AABB_F08291F9087B
#define C_once_44A7F9A5_269A_48D5_AABB_F08291F9087B

#ifdef _LIBYOYO
#define _YO_ARRAY_BUILTIN
#endif

#include "yoyo.hc"

#ifdef _YO_ARRAY_BUILTIN
# define _YO_ARRAY_BUILTIN_CODE(Code) Code
# define _YO_ARRAY_EXTERN 
#else
# define _YO_ARRAY_BUILTIN_CODE(Code)
# define _YO_ARRAY_EXTERN extern 
#endif

_YO_ARRAY_EXTERN char Oj_Sort_OjMID[] _YO_ARRAY_BUILTIN_CODE ( = ">>>/@" );
_YO_ARRAY_EXTERN char Oj_Push_OjMID[] _YO_ARRAY_BUILTIN_CODE ( = "$+/@*" );
_YO_ARRAY_EXTERN char Oj_Pop_OjMID[] _YO_ARRAY_BUILTIN_CODE ( = "$-/@" );
_YO_ARRAY_EXTERN char Oj_Pop_Npl_OjMID[] _YO_ARRAY_BUILTIN_CODE ( = "`$-/@" );
_YO_ARRAY_EXTERN char Oj_Sorted_Insert_OjMID[] _YO_ARRAY_BUILTIN_CODE ( = ">1>/@*" );
_YO_ARRAY_EXTERN char Oj_Sorted_Find_OjMID[] _YO_ARRAY_BUILTIN_CODE ( = ">?>/@*" );
_YO_ARRAY_EXTERN char Oj_Push_Front_OjMID[] _YO_ARRAY_BUILTIN_CODE ( = "+$/@*" );
_YO_ARRAY_EXTERN char Oj_Pop_Front_OjMID[] _YO_ARRAY_BUILTIN_CODE ( = "-$/@" );
_YO_ARRAY_EXTERN char Oj_Pop_Front_Npl_OjMID[] _YO_ARRAY_BUILTIN_CODE ( = "`-$/@" );

void Oj_Sort(void *self)
#ifdef _YO_ARRAY_BUILTIN
  {
    void (*sort)(void *) = Yo_Find_Method_Of(&self,Oj_Sort_OjMID,YO_RAISE_ERROR);
    sort(self);
  }
#endif
  ;

void Oj_Sorted_Insert(void *self, void *val)
#ifdef _YO_ARRAY_BUILTIN
  {
    void (*insert)(void *, void *) = Yo_Find_Method_Of(&self,Oj_Sorted_Insert_OjMID,YO_RAISE_ERROR);
    insert(self,val);
  }
#endif
  ;

void *Oj_Sorted_Find(void *self, void *val)
#ifdef _YO_ARRAY_BUILTIN
  {
    void *(*find)(void *, void *) = Yo_Find_Method_Of(&self,Oj_Sorted_Find_OjMID,YO_RAISE_ERROR);
    return find(self,val);
  }
#endif
  ;

void Oj_Push(void *self, void *val)
#ifdef _YO_ARRAY_BUILTIN
  {
    void (*push)(void *, void *) = Yo_Find_Method_Of(&self,Oj_Push_OjMID,YO_RAISE_ERROR);
    push(self,val);
  }
#endif
  ;

void *Oj_Pop(void *self)
#ifdef _YO_ARRAY_BUILTIN
  {
    void *(*pop)(void *) = Yo_Find_Method_Of(&self,Oj_Pop_OjMID,YO_RAISE_ERROR);
    return pop(self);
  }
#endif
  ;

void *Oj_Pop_Npl(void *self)
#ifdef _YO_ARRAY_BUILTIN
  {
    void *(*pop)(void *) = Yo_Find_Method_Of(&self,Oj_Pop_Npl_OjMID,YO_RAISE_ERROR);
    return pop(self);
  }
#endif
  ;

void Oj_Push_Front(void *self, void *val)
#ifdef _YO_ARRAY_BUILTIN
  {
    void (*push)(void *, void *) = Yo_Find_Method_Of(&self,Oj_Push_Front_OjMID,YO_RAISE_ERROR);
    push(self,val);
  }
#endif
  ;

void *Oj_Pop_Front_Npl(void *self)
#ifdef _YO_ARRAY_BUILTIN
  {
    void *(*pop)(void *) = Yo_Find_Method_Of(&self,Oj_Pop_Front_Npl_OjMID,YO_RAISE_ERROR);
    return pop(self);
  }
#endif
  ;

void *Oj_Pop_Front(void *self)
#ifdef _YO_ARRAY_BUILTIN
  {
    void *(*pop)(void *) = Yo_Find_Method_Of(&self,Oj_Pop_Front_OjMID,YO_RAISE_ERROR);
    return pop(self);
  }
#endif
  ;

void *Lower_Boundary(void **S, int S_len, void *compare, void *val, int *found)
#ifdef _YO_ARRAY_BUILTIN
  {
    typedef int (*Tcmp)(void *,void*);
    int cmp_r = 0;
    void **iS = S;
    void **middle = iS;
    int half;
    int len = S_len;

    if ( len  )
      {
        while (len > 0)
          {
            half = len >> 1;
            middle = iS + half;
            if ( (cmp_r = ((Tcmp)compare)(*middle,val)) < 0 )
              {
                iS = middle;
                ++iS;
                len = len - half - 1;
              }
            else
              len = half;
          }
          
        if ( middle != iS && iS < S+S_len )
          {
            cmp_r = ((Tcmp)compare)(*iS,val);
          }
      }
      
    *found = !cmp_r;
    return iS;
  }
#endif
  ;

typedef struct _YO_ARRAY
  {
    void **at;
    int count;
    int capacity;
  } YO_ARRAY;

void Array_Del(YO_ARRAY *a,int pos,int n)
#ifdef _YO_ARRAY_BUILTIN
  {
    int i;
    void *self = a;
    void (*destruct)(void *) = Yo_Find_Method_Of(&self,Oj_Destruct_Element_OjMID,0);

    if ( pos < 0 ) pos = a->count + pos;
    if ( n < 0 || pos + n > a->count ) n = a->count-pos;
    if ( pos < 0 || pos+n > a->count ) 
      Yo_Raise(YO_ERROR_OUT_OF_RANGE,0,__Yo_FILE__,__LINE__);

    if ( destruct )
      for ( i = 0; i < n; ++i )
        destruct((a->at)[i+pos]);
    
    if ( pos != a->count-n )
      memmove(a->at+pos,a->at+(pos+n),(a->count-(pos+n))*sizeof(void*));
    a->count -= n;
  }
#endif
  ;

void *Array_Take_Npl(YO_ARRAY *a,int pos)
#ifdef _YO_ARRAY_BUILTIN
  {
    void *Q = 0;
    
    if ( pos < 0 ) pos = a->count + pos;
    if ( pos < 0 || pos >= a->count ) 
      Yo_Raise(YO_ERROR_OUT_OF_RANGE,0,__Yo_FILE__,__LINE__);

    Q = (a->at)[pos];
    
    if ( pos != a->count-1 )
      memmove(a->at+pos,a->at+(pos+1),(a->count-(pos+1))*sizeof(void*));
    a->count -= 1;
    
    return Q;
  }
#endif
  ;

void *Array_Take(YO_ARRAY *a,int pos)
#ifdef _YO_ARRAY_BUILTIN
  {
    void *self = a;
    void (*destruct)(void *) = Yo_Find_Method_Of(&self,Oj_Destruct_Element_OjMID,YO_RAISE_ERROR);
    void *Q = Array_Take_Npl(a,pos);
    
    if ( Q )
      Yo_Pool_Ptr(Q,destruct);
    
    return Q;
  }
#endif
  ;

void Array_Grow(YO_ARRAY *a,int require)
#ifdef _YO_ARRAY_BUILTIN
  {
    int capacity = Min_Pow2(require*sizeof(void*));
    if ( !a->at || a->capacity < capacity )
      {
        a->at = Yo_Realloc_Npl(a->at,capacity);
        a->capacity = capacity;
      }
  }
#endif
  ;
  
void Array_Insert(YO_ARRAY *a,int pos,void *p)
#ifdef _YO_ARRAY_BUILTIN
  {
    if ( pos < 0 ) pos = a->count + pos + 1;
    if ( pos < 0 || pos > a->count ) 
      {
        void *self = a;
        void (*destruct)(void *) = Yo_Find_Method_Of(&self,Oj_Destruct_Element_OjMID,0);
        if ( destruct ) destruct(p);
        Yo_Raise(YO_ERROR_OUT_OF_RANGE,0,__Yo_FILE__,__LINE__);
      }
    
    Array_Grow(a,a->count+1);
    if ( pos < a->count )
      memmove(a->at+pos+1,a->at+pos,(a->count-pos)*sizeof(void*));
    a->at[pos] = p;
    ++a->count;
  }
#endif
  ;

void Array_Fill(YO_ARRAY *a,int pos,void *p, int count)
#ifdef _YO_ARRAY_BUILTIN
  {
    int i;
    
    if ( !count ) return;
    else if ( count < 0 || pos < 0 ) 
      Yo_Raise(YO_ERROR_INVALID_PARAM,0,__Yo_FILE__,__LINE__);
    
    if ( !a->at || a->count-pos < count )
      {
        Array_Grow(a,a->count-pos+count);
        memset(a->at+pos,0,sizeof(void*)*count);
        a->count = a->count-pos+count;
      }
      
    for ( i = 0; i < count; ++i ) 
      {
        if ( a->at[pos+i] )
          {
            void *self = a;
            void (*destruct)(void *) = Yo_Find_Method_Of(&self,Oj_Destruct_Element_OjMID,0);
            if ( destruct ) destruct(a->at[pos+i]);
          }
        a->at[pos+i] = p;
      }
  }
#endif
  ;

void Array_Set(YO_ARRAY *a,int pos,void *val)
#ifdef _YO_ARRAY_BUILTIN
  {
    void *self = a;
    void (*destruct)(void *) = Yo_Find_Method_Of(&self,Oj_Destruct_Element_OjMID,0);

    if ( pos < 0 ) pos = a->count + pos;
    if ( pos < 0 || pos >= a->count ) 
      {
        if ( destruct ) destruct(val);
        __Raise(YO_ERROR_OUT_OF_RANGE,0);
      }
      
    if ( destruct )
      destruct((a->at)[pos]);

    a->at[pos] = val;
  }
#endif
  ;

int Array_Sorted_Lower_Boundary(YO_ARRAY *a,void *val,int *found,int except)
#ifdef _YO_ARRAY_BUILTIN
  {
    void *self = a;
    int (*compare)(void *, void *);
    
    if ( !a->count ) 
      {
        *found = 0;
        return 0;
      }
    
    if ( 0 != (compare = Yo_Find_Method_Of(&self,Oj_Compare_Elements_OjMID,0)) )
      {
        void **p = Lower_Boundary(a->at,a->count,compare,val,found);
        return p - a->at;
      }
    else if (except)
      __Raise(YO_ERROR_UNSORTABLE,__yoTa("array is unsortable",0));
    else
      return -1;
      
    return 0;
  }
#endif
  ;

void Array_Sorted_Insert(YO_ARRAY *a,void *p)
#ifdef _YO_ARRAY_BUILTIN
  {
    if ( !a->count )
      Array_Insert(a,-1,p);
    else
      {
        int found = 0;
        int pos ;
        
        pos = Array_Sorted_Lower_Boundary(a,p,&found,0);
        
        if ( pos < 0 )
          {
            void *self = a;
            void (*destructor)(void *) = Yo_Find_Method_Of(&self,Oj_Destruct_Element_OjMID,0);
            if ( destructor )
              destructor(p);
            __Raise(YO_ERROR_UNSORTABLE,__yoTa("array is unsortable",0));
          }
          
        if ( !found )
          Array_Insert(a,pos,p);
        else
          Array_Set(a,pos,p);
      }
  }
#endif
  ;

void *Array_Binary_Find(YO_ARRAY *a, void *p)
#ifdef _YO_ARRAY_BUILTIN
  {
    int found = 0;
    int pos = Array_Sorted_Lower_Boundary(a,p,&found,1);
    return found ? a->at[pos] : 0;
  }
#endif
  ;

#ifdef _YO_ARRAY_BUILTIN
int Array_Sort_Qsort_Compare(int (*compare)(void *a,void *b), void **a, void **b)
  {
    return compare(*a,*b);
  }
int (*Array_Sort_Qsort_Compare_Static_Compare)(void *a,void *b) = 0;
int Array_Sort_Qsort_Compare_Static(void **a, void **b)
  {
    return Array_Sort_Qsort_Compare_Static_Compare(*a,*b);
  }
#endif

void Array_Sort(YO_ARRAY *a)
#ifdef _YO_ARRAY_BUILTIN
  {
    void *self = a;
    void *compare;
    if ( a->at && a->count )
      {
        if ( 0 != (compare = Yo_Find_Method_Of(&self,Oj_Compare_Elements_OjMID,0)) )
          {
          #if !defined __windoze && !defined __NetBSD__ && !defined __linux__ && !defined __CYGWIN__
            qsort_r(a->at,a->count,sizeof(void*),compare,(void*)Array_Sort_Qsort_Compare);
          #elif defined __windoze && _MSC_VER > 1400 
            qsort_s(a->at,a->count,sizeof(void*),compare,(void*)Array_Sort_Qsort_Compare);
          #else /* use global variable */
            __Xchg_Interlock
              {
                Array_Sort_Qsort_Compare_Static_Compare = compare;
                qsort(a->at,a->count,sizeof(void*),(void*)Array_Sort_Qsort_Compare_Static);
              }
          #endif
          }
        else
          __Raise(YO_ERROR_UNSORTABLE,__yoTa("array is unsortable",0));
      }
  }
#endif
  ;

#define Array_COUNT(Arr)          ((int)((YO_ARRAY *)(Arr))->count+0)
#define Array_BEGIN(Arr)          (((YO_ARRAY *)(Arr))->at)
#define Array_END(Arr)            (Array_BEGIN(Arr)+Array_COUNT(Arr))
#define Array_AT(Arr,Idx)         ((((YO_ARRAY *)(Arr))->at)[Idx])
#define Array_Push(Arr,Val)       Array_Insert(Arr,-1,Val)
#define Array_Pop(Arr)            Array_Take(Arr,-1)
#define Array_Pop_Npl(Arr)        Array_Take_Npl(Arr,-1)
#define Array_Push_Front(Arr,Val) Array_Insert(Arr,0,Val)
#define Array_Pop_Front(Arr)      Array_Take(Arr,0)
#define Array_Pop_Front_Npl(Arr)  Array_Take_Npl(Arr,0)

void Array_Push_Oj(YO_ARRAY *a, void *val)
#ifdef _YO_ARRAY_BUILTIN
  {
    Array_Push(a,val);
  }
#endif
  ;

void *Array_Pop_Oj(YO_ARRAY *a)
#ifdef _YO_ARRAY_BUILTIN
  {
    return Array_Pop(a);
  }
#endif
  ;

void *Array_Pop_Npl_Oj(YO_ARRAY *a)
#ifdef _YO_ARRAY_BUILTIN
  {
    return Array_Pop_Npl(a);
  }
#endif
  ;

void Array_Push_Front_Oj(YO_ARRAY *a, void *val)
#ifdef _YO_ARRAY_BUILTIN
  {
    Array_Push_Front(a,val);
  }
#endif
  ;

void *Array_Pop_Front_Oj(YO_ARRAY *a)
#ifdef _YO_ARRAY_BUILTIN
  {
    return Array_Pop_Front(a);
  }
#endif
  ;

void *Array_Pop_Front_Npl_Oj(YO_ARRAY *a)
#ifdef _YO_ARRAY_BUILTIN
  {
    return Array_Pop_Front_Npl(a);
  }
#endif
  ;

int Array_Count(YO_ARRAY *a)
#ifdef _YO_ARRAY_BUILTIN
  {
    if ( a )
      return Array_COUNT(a);
    return 0;
  }
#endif
  ;
  
void *Array_Begin(YO_ARRAY *a)
#ifdef _YO_ARRAY_BUILTIN
  {
    if ( a )
      return Array_BEGIN(a);
    return 0;
  }
#endif
  ;

void *Array_End(YO_ARRAY *a)
#ifdef _YO_ARRAY_BUILTIN
  {
    if ( a )
      return Array_END(a);
    return 0;
  }
#endif
  ;

void *Array_At(YO_ARRAY *a,int pos)
#ifdef _YO_ARRAY_BUILTIN
  {
    if ( a )
      {
        if ( pos < 0 ) pos = a->count + pos;
        if ( pos < 0 || pos >= a->count ) 
          Yo_Raise(YO_ERROR_OUT_OF_RANGE,0,__Yo_FILE__,__LINE__);
        return Array_AT(a,pos);
      }
    return 0;
  }
#endif
  ;

void Array_Destruct(YO_ARRAY *a)
#ifdef _YO_ARRAY_BUILTIN
  {
    int i = 0;
    void *self = a;
    void (*destructor)(void *) = Yo_Find_Method_Of(&self,Oj_Destruct_Element_OjMID,0);
    if ( destructor )
      for ( i = 0; i < a->count; ++i )
        if ( a->at[i] ) 
          destructor(a->at[i]);
    if ( a->at )
      free(a->at);
    Yo_Object_Destruct(a);
  }
#endif
  ;

void Array_Clear(YO_ARRAY *a)
#ifdef _YO_ARRAY_BUILTIN
  {
    int i = 0;
    void *self = a;
    void (*destructor)(void *) = Yo_Find_Method_Of(&self,Oj_Destruct_Element_OjMID,0);
    if ( destructor )
      for ( i = 0; i < a->count; ++i )
        destructor(a->at[i]);
    if ( a->at )
      memset(a->at,0,a->count*sizeof(a->at[0]));
    a->count = 0;
  }
#endif
  ;

#define Array_Init() Array_Void()
void *Array_Void(void)
#ifdef _YO_ARRAY_BUILTIN
  {
    static YO_FUNCTABLE funcs[] = 
      { {0},
        {Oj_Destruct_OjMID,         Array_Destruct},
        {Oj_Sort_OjMID,             Array_Sort},
        {Oj_Sorted_Insert_OjMID,    Array_Sorted_Insert},
        {Oj_Sorted_Find_OjMID,      Array_Binary_Find},
        {Oj_Push_OjMID,             Array_Push_Oj},
        {Oj_Push_Front_OjMID,       Array_Push_Front_Oj},
        {Oj_Count_OjMID,            Array_Count},
        {Oj_Pop_Npl_OjMID,          Array_Pop_Npl_Oj},
        {Oj_Pop_Front_Npl_OjMID,    Array_Pop_Front_Npl_Oj},
        {0}};
    YO_ARRAY *arr = Yo_Object(sizeof(YO_ARRAY),funcs);
    return arr;
  }
#endif
  ;

void *Array_Refs(void)
#ifdef _YO_ARRAY_BUILTIN
  {
    static YO_FUNCTABLE funcs[] = 
      { {0},
        {Oj_Destruct_OjMID,         Array_Destruct},
        {Oj_Destruct_Element_OjMID, Yo_Unrefe},
        {Oj_Sort_OjMID,             Array_Sort},
        {Oj_Sorted_Insert_OjMID,    Array_Sorted_Insert},
        {Oj_Sorted_Find_OjMID,      Array_Binary_Find},
        {Oj_Push_OjMID,             Array_Push_Oj},
        {Oj_Pop_OjMID,              Array_Pop_Oj},
        {Oj_Push_Front_OjMID,       Array_Push_Front_Oj},
        {Oj_Pop_Front_OjMID,        Array_Pop_Front_Oj},
        {Oj_Count_OjMID,            Array_Count},
        {Oj_Pop_Npl_OjMID,          Array_Pop_Npl_Oj},
        {Oj_Pop_Front_Npl_OjMID,    Array_Pop_Front_Npl_Oj},
        {0}};
    YO_ARRAY *arr = Yo_Object(sizeof(YO_ARRAY),funcs);
    return arr;
  }
#endif
  ;

void *Array_Refs_Copy(void *refs, int count)
#ifdef _YO_ARRAY_BUILTIN
  {
    YO_ARRAY *arr = Array_Refs();
    int i;
    Array_Fill(arr,0,0,count);
    for ( i = 0; i < count; ++i )
      arr->at[i] = __Refe(((void**)refs)[i]);
    return arr;
  }
#endif
  ;
  
void *Array_Ptrs(void)
#ifdef _YO_ARRAY_BUILTIN
  {
    static YO_FUNCTABLE funcs[] = 
      { {0},
        {Oj_Destruct_OjMID,         Array_Destruct},
        {Oj_Destruct_Element_OjMID, free},
        {Oj_Sort_OjMID,             Array_Sort},
        {Oj_Sorted_Insert_OjMID,    Array_Sorted_Insert},
        {Oj_Sorted_Find_OjMID,      Array_Binary_Find},
        {Oj_Push_OjMID,             Array_Push_Oj},
        {Oj_Pop_OjMID,              Array_Pop_Oj},
        {Oj_Push_Front_OjMID,       Array_Push_Front_Oj},
        {Oj_Pop_Front_OjMID,        Array_Pop_Front_Oj},
        {Oj_Count_OjMID,            Array_Count},
        {Oj_Pop_Npl_OjMID,          Array_Pop_Npl_Oj},
        {Oj_Pop_Front_Npl_OjMID,    Array_Pop_Front_Npl_Oj},
        {0}};
    YO_ARRAY *arr = Yo_Object(sizeof(YO_ARRAY),funcs);
    return arr;
  }
#endif
  ;

void *Array_Pchars(void)
#ifdef _YO_ARRAY_BUILTIN
  {
    static YO_FUNCTABLE funcs[] = 
      { {0},
        {Oj_Destruct_OjMID,         Array_Destruct},
        {Oj_Destruct_Element_OjMID, free},
        {Oj_Compare_Elements_OjMID, strcmp},
        {Oj_Sort_OjMID,             Array_Sort},
        {Oj_Sorted_Insert_OjMID,    Array_Sorted_Insert},
        {Oj_Sorted_Find_OjMID,      Array_Binary_Find},
        {Oj_Push_OjMID,             Array_Push_Oj},
        {Oj_Pop_OjMID,              Array_Pop_Oj},
        {Oj_Push_Front_OjMID,       Array_Push_Front_Oj},
        {Oj_Pop_Front_OjMID,        Array_Pop_Front_Oj},
        {Oj_Count_OjMID,            Array_Count},
        {Oj_Pop_Npl_OjMID,          Array_Pop_Npl_Oj},
        {Oj_Pop_Front_Npl_OjMID,    Array_Pop_Front_Npl_Oj},
        {0}};
    YO_ARRAY *arr = Yo_Object(sizeof(YO_ARRAY),funcs);
    return arr;
  }
#endif
  ;

#ifdef _YO_ARRAY_BUILTIN
int __wcscmp(void *a, void *b) { return wcscmp(a,b); }
#endif

void *Array_Pwide(void)
#ifdef _YO_ARRAY_BUILTIN
  {
    static YO_FUNCTABLE funcs[] = 
      { {0},
        {Oj_Destruct_OjMID,         Array_Destruct},
        {Oj_Destruct_Element_OjMID, free},
        {Oj_Compare_Elements_OjMID, __wcscmp},
        {Oj_Sort_OjMID,             Array_Sort},
        {Oj_Sorted_Insert_OjMID,    Array_Sorted_Insert},
        {Oj_Sorted_Find_OjMID,      Array_Binary_Find},
        {Oj_Push_OjMID,             Array_Push_Oj},
        {Oj_Pop_OjMID,              Array_Pop_Oj},
        {Oj_Push_Front_OjMID,       Array_Push_Front_Oj},
        {Oj_Pop_Front_OjMID,        Array_Pop_Front_Oj},
        {Oj_Count_OjMID,            Array_Count},
        {Oj_Pop_Npl_OjMID,          Array_Pop_Npl_Oj},
        {Oj_Pop_Front_Npl_OjMID,    Array_Pop_Front_Npl_Oj},
        {0}};
    YO_ARRAY *arr = Yo_Object(sizeof(YO_ARRAY),funcs);
    return arr;
  }
#endif
  ;

void *Array_Take_Data_Npl(YO_ARRAY *a)
#ifdef _YO_ARRAY_BUILTIN
  {
    void *p = a->at;
    a->at = 0;
    a->capacity = a->count = 0;
    return p;
  }
#endif
  ;

#endif /* C_once_44A7F9A5_269A_48D5_AABB_F08291F9087B */

