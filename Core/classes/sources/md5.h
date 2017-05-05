
#if !defined __60EDC02C_AAD6_415D_B287_0490666C10E9__
#define __60EDC02C_AAD6_415D_B287_0490666C10E9__

#if !defined _WITHOUT_TFFCLS
#include "./_specific.h"
#else
#include "./detect_compiler.h"
#endif

#include <libhash/MD5.h>

namespace teggo
{
  class MD5_Hash {
  public:
    MD5_Hash();
    void Update(void const *input, unsigned long input_length);
    void Finalize(void *outDigest);

  private:
    MD5_CONTEXT context;

  };

  inline MD5_Hash::MD5_Hash()
  {
    MD5_Start(&context);
  }

  CXX_FAKE_INLINE
  void MD5_Hash::Update(void const *_input, unsigned long input_length)
  {
    MD5_Update(&context,_input,input_length);
  }

  // AlgMD5 finalization. Writing the message digest and zeroizing the context.
  CXX_FAKE_INLINE
  void MD5_Hash::Finalize (void *outDigest)
  {
    MD5_Finish(&context,outDigest);
  }

  CXX_FAKE_INLINE 
  void Md5sign(void const *data,unsigned len,void *md5sign)
    {
      MD5_Hash md5;
      md5.Update(data,len);
      md5.Finalize(md5sign);
    }

} // namespace

#endif // __60EDC02C_AAD6_415D_B287_0490666C10E9__
