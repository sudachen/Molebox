
/*

  Copyright (c) 2005-2009, Alexey Sudachen, alexey@sudachen.name

*/

#ifndef ___6ea1d37e_eace_4986_b99a_fcd335057356___
#define ___6ea1d37e_eace_4986_b99a_fcd335057356___

#if !defined _WITHOUT_TFFCLS
#include "./_specific.h"
#else
#include "./detect_compiler.h"
#define _TEGGO_EXPORTABLE
#endif

namespace teggo {

  struct NEWDES_Cipher // trivial 'aka C' structure
    {
      enum { BLOCK_BYTES = 8 };
      enum { KEY_BYTES   = 15 };
      enum { UNRAV_BYTES = 60 };
      enum { UNRAV_BYTES_1 = 68 };
      enum _CONFIGURE_FLAG { ENCRYPTION, DECRYPTION };

      _TEGGO_EXPORTABLE void SetupEncipher(void const *key);
      _TEGGO_EXPORTABLE void SetupDecipher(void const *key);
      _TEGGO_EXPORTABLE void DoCipher(void *data, u32_t count_of_blocks);
      _TEGGO_EXPORTABLE void DoCipherCBCI(void *data, u32_t count_of_blocks, uint64_t IVc = 0);
      _TEGGO_EXPORTABLE void DoCipherCBCO(void *data, u32_t count_of_blocks, uint64_t IVc = 0);
      _TEGGO_EXPORTABLE void DoCipherBlock(void *b);
      _TEGGO_EXPORTABLE void Normalize_(byte_t const *kunrav);

      union
        {
          unsigned _;
          unsigned char key_unrav[UNRAV_BYTES_1];
        };
    };
}

//#if defined _X86_ASSEMBLER
extern "C" void NEWDES_DoCipher_S(void const *key,void *b,int count);
//#endif

#include "./newdes.inl"

#endif // ___6ea1d37e_eace_4986_b99a_fcd335057356___
