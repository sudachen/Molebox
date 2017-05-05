
/*

  Copyright (c) 2005-2009, Alexey Sudachen, alexey@sudachen.name

*/

#ifndef ___4c244a21_2eff_49f3_a429_0b734f7e9cd1___
#define ___4c244a21_2eff_49f3_a429_0b734f7e9cd1___

#if !defined _WITHOUT_TFFCLS
#include "./_specific.h"
#else
#include "./detect_compiler.h"
#define _TEGGO_EXPORTABLE
#endif

#include <libhash/BF.h>

namespace teggo
{

    struct BLOWFISH_Cipher
    {
        enum { BLOCK_BYTES = 8 };
        enum { KEY_BYTES   = 15 };
        enum DIRECTION { ENCRYPTION, DECRYPTION };

        BF_CONTEXT self;
        DIRECTION direction;

        void SetupEncipher(void const* key, size_t count = KEY_BYTES)
        {
            Blowfish_Init(&self,key,count);
            direction = ENCRYPTION;
        }
        void SetupDecipher(void const* key, size_t count = KEY_BYTES)
        {
            Blowfish_Init(&self,key,count);
            direction = DECRYPTION;
        }

        void DoCipherBlock(void* b)
        {
            if ( direction = ENCRYPTION )
                Blowfish_Encrypt8(&self,b);
            else
                Blowfish_Decrypt8(&self,b);
        }

        void DoCipher(void* data, u32_t count_of_blocks)
        {
            for (u32_t i = 0; i < count_of_blocks; ++i)
                DoCipherBlock((byte_t*)data+BLOCK_BYTES*i);
        }

        void DoCipherCBCI(void* data, u32_t count_of_blocks, uint64_t IVc = 0)
        {
            byte_t q[BLOCK_BYTES] = {0};
            if (IVc) memcpy(q,&IVc,8);
            for (u32_t i = 0; i < count_of_blocks; ++i)
            {
                for ( int j =0; j < BLOCK_BYTES; ++j )
                    *((byte_t*)data+BLOCK_BYTES*i+j) ^= *(q+j);
                DoCipherBlock((byte_t*)data+BLOCK_BYTES*i);
                memcpy(q,(byte_t*)data+BLOCK_BYTES*i,BLOCK_BYTES);
            }

        }

        void DoCipherCBCO(void* data, u32_t count_of_blocks, uint64_t IVc = 0)
        {
            byte_t q[BLOCK_BYTES] = {0};
            if (IVc) memcpy(q,&IVc,8);
            for (u32_t i = 0; i < count_of_blocks; ++i)
            {
                byte_t w[BLOCK_BYTES];
                memcpy(w,(byte_t*)data+BLOCK_BYTES*i,BLOCK_BYTES);
                DoCipherBlock((byte_t*)data+BLOCK_BYTES*i);
                for ( int j =0; j < BLOCK_BYTES; ++j )
                    *((byte_t*)data+BLOCK_BYTES*i+j) ^= *(q+j);
                memcpy(q,w,BLOCK_BYTES);
            }
        }
    };
}

#endif // ___4c244a21_2eff_49f3_a429_0b734f7e9cd1___
