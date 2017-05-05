
/*

  Copyright (c) 2008-2009, Alexey Sudachen, alexey@sudachen.name

*/


#include <classes.h>
#define NT_HEADERS(x) ((IMAGE_NT_HEADERS*)((unsigned char*)x + ((IMAGE_DOS_HEADER*)x)->e_lfanew))

#include <winternl.h>

#if defined _DEMOVERSION_
  static const bool ISDEMOVERSION = true;
#else
  static const bool ISDEMOVERSION = false;
#endif

#if defined _BUNDLER_
  static const bool ISMOLEBUNDLER = true;
#else
  static const bool ISMOLEBUNDLER = false;
#endif

#if defined _PROTECTOR_
  static const bool ISMOLEPROTECTOR = true;
#else
  static const bool ISMOLEPROTECTOR = false;
#endif

void usage();
bool ProcessSpack(StringParam mainstorage, Xdocument config, EhFilter ehf);
int  GenerateIAT(byte_t *section,unsigned section_rva,unsigned section_size,IMAGE_DATA_DIRECTORY *dict, unsigned reserve, EhFilter ehf);
bool GetRequiredResources(byte_t *mapped, IMAGE_NT_HEADERS *nth, BufferT<byte_t> &data);
bool RscFixup(byte_t *rsrc,unsigned rsrc_rva );
bool TakeOriginalResources(byte_t *mapped, IMAGE_NT_HEADERS *nth, BufferT<byte_t> &data);

extern "C" int _MAJOR_VERSION;
//extern "C" int _MINOR_VERSION;
extern "C" int _BUILD_NUMBER;     
extern "C" time_t _BUILD_TIME;

template<class tChr>
  StringT<tChr> TrimLength(tChr const *t, unsigned trim_len)
    {
      if ( trim_len < 3 ) trim_len = 3;
      unsigned l = StrLen(t);
      if ( l > trim_len )
        {
          StringT<tChr> s("...");
          s += t+(l-(trim_len-3));
          return s;
        }
      else
        return StringT<tChr>(t);
    }
  
extern teggo::XFILE Xo;
extern teggo::XFILE Xoln;
extern void SwitchXoToUtf8(bool = true);
