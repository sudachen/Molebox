
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

#include <libhash/NDES.h>

namespace teggo
{

    struct NEWDES_Cipher // trivial 'aka C' structure
    {
	NDES_CONTEXT ctx;

        _TEGGO_EXPORTABLE void SetupEncipher(void const* key);
        _TEGGO_EXPORTABLE void SetupDecipher(void const* key);
        _TEGGO_EXPORTABLE void DoCipher(void* data, u32_t count_of_blocks);
        _TEGGO_EXPORTABLE void DoCipherCBCI(void* data, u32_t count_of_blocks, uint64_t IVc = 0);
        _TEGGO_EXPORTABLE void DoCipherCBCO(void* data, u32_t count_of_blocks, uint64_t IVc = 0);
        _TEGGO_EXPORTABLE void DoCipherBlock(void* b);
    };
}

#include "./newdes.inl"

#endif // ___6ea1d37e_eace_4986_b99a_fcd335057356___
