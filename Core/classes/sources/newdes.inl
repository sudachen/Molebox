
/*

  Copyright (c) 2005-2009, Alexey Sudachen, alexey@sudachen.name

*/

#ifndef ___101853ab_405b_443a_8d26_152269b14074___
#define ___101853ab_405b_443a_8d26_152269b14074___

#include "./newdes.h"

#if defined _TEGGOINLINE || defined _WITHOUT_TFFCLS || defined _TEGGO_NEWDES_HERE

namespace teggo
{

    #if defined _TEGGO_NEWDES_HERE
# define _TEGGO_NEWDES_FAKE_INLINE _TEGGO_EXPORTABLE
    #else
# define _TEGGO_NEWDES_FAKE_INLINE CXX_FAKE_INLINE
    #endif

    _TEGGO_NEWDES_FAKE_INLINE
    void NEWDES_Cipher::SetupEncipher(void const* key)
    {
	NDES_Init_Encipher(&ctx,key);
    }

    _TEGGO_NEWDES_FAKE_INLINE
    void NEWDES_Cipher::SetupDecipher(void const* key)
    {
	NDES_Init_Decipher(&ctx,key);
    }

    _TEGGO_NEWDES_FAKE_INLINE
    void NEWDES_Cipher::DoCipherBlock(void* b)
    {
	NDES_Cipher_8(&ctx,b);
    }

    _TEGGO_NEWDES_FAKE_INLINE
    void NEWDES_Cipher::DoCipher(void* data,u32_t count)
    {
	NDES_Cipher(&ctx,data,count);
    }

    _TEGGO_NEWDES_FAKE_INLINE
    void NEWDES_Cipher::DoCipherCBCI(void* data,u32_t count, uint64_t IVc)
    {
        byte_t q[NDES_BLOCK_BYTES] = {0};
        if (IVc) memcpy(q,&IVc,8);
        for (u32_t i = 0; i < count; ++i)
        {
            for ( int j =0; j < NDES_BLOCK_BYTES; ++j )
                *((byte_t*)data+NDES_BLOCK_BYTES*i+j) ^= *(q+j);
            DoCipherBlock((byte_t*)data+NDES_BLOCK_BYTES*i);
            memcpy(q,(byte_t*)data+NDES_BLOCK_BYTES*i,NDES_BLOCK_BYTES);
        }
    }

    _TEGGO_NEWDES_FAKE_INLINE
    void NEWDES_Cipher::DoCipherCBCO(void* data,u32_t count, uint64_t IVc)
    {
        byte_t q[NDES_BLOCK_BYTES] = {0};
        if (IVc) memcpy(q,&IVc,8);
        for (u32_t i = 0; i < count; ++i)
        {
            byte_t w[NDES_BLOCK_BYTES];
            memcpy(w,(byte_t*)data+NDES_BLOCK_BYTES*i,NDES_BLOCK_BYTES);
            DoCipherBlock((byte_t*)data+NDES_BLOCK_BYTES*i);
            for ( int j =0; j < NDES_BLOCK_BYTES; ++j )
                *((byte_t*)data+NDES_BLOCK_BYTES*i+j) ^= *(q+j);
            memcpy(q,w,NDES_BLOCK_BYTES);
        }
    }

} // namespace

#endif
#endif // ___101853ab_405b_443a_8d26_152269b14074___
