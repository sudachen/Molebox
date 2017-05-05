
#include <stdio.h>

int _atexit( void (*func)(void) )
  { 
    //printf("%08x\n",func);
    return 0;
  }

/*
int onexit( void (*func)(void) )
  {
    return 0;
  }

void abort ( void )
  {
  }
  

void assert (int expression)
  {
  }
  
char *strncpy(char *dest, const char *source, size_t count)
{
  char *start = dest;

  while (count && (*dest++ = *source++)) count--;
  if (count) while (--count) *dest++ = '\0';
  return start;
}

int strncmp(const char *s1, const char *s2, size_t count)
{
  if (!count) return 0;

  while (--count && *s1 && *s1 == *s2)
  {
    s1++;
    s2++;
  }

  return *(unsigned char *) s1 - *(unsigned char *) s2;
}

void *memset(void *p, int c, size_t n)
{
  char *pb = (char *) p;
  char *pbend = pb + n;
  while (pb != pbend) *pb++ = c;
  return p;
}

int memcmp(const void *dst, const void *src, size_t n)
{
  if (!n) return 0;

  while (--n && *(char *) dst == *(char *) src)
  {
    dst = (char *) dst + 1;
    src = (char *) src + 1;
  }

  return *((unsigned char *) dst) - *((unsigned char *) src);
}

void *memcpy(void *dst, const void *src, size_t n)
{
  void *ret = dst;

  while (n--)
  {
    *(char *)dst = *(char *)src;
    dst = (char *) dst + 1;
    src = (char *) src + 1;
  }

  return ret;
}

char *strcpy(char *dst, const char *src)
{
  char *cp = dst;
  while (*cp++ = *src++);
  return dst;
}

int strlen(const char *s)
{
  const char *eos = s;
  while (*eos++);
  return (int) (eos - s - 1);
}

int strcmp(const char *s1, const char *s2)
{
  int ret = 0;
  while (!(ret = *(unsigned char *) s1 - *(unsigned char *) s2) && *s2) ++s1, ++s2;

  if (ret < 0)
    ret = -1;
  else if (ret > 0)
    ret = 1 ;

  return ret;
}
   
typedef unsigned short wchar_t;
wchar_t* wcscpy(wchar_t *__restrict__ dest, const wchar_t *__restrict__ src) 
{
  wchar_t* orig=dest;
  for (; (*dest=*src); ++src,++dest) ;
  return orig;
}

extern __stdcall void *HeapAlloc(void *heap,unsigned flags,unsigned size);
extern __stdcall void HeapFree(void *heap,unsigned flags,void *p);
extern __stdcall void *GetProcessHeap();
        
void *malloc(unsigned l)
  { 
    return HeapAlloc(GetProcessHeap(),0,l);
  }
  
void free(void *p)
  {
    HeapFree(GetProcessHeap(),0,p);
  }
*/
