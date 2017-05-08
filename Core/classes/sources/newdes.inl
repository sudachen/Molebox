
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

    CXX_FAKE_INLINE
    void NEWDES_Cipher::Normalize_(byte_t const* kunrav)
    {
        int i,j;
        memset(key_unrav,0,sizeof(key_unrav));
        for ( i = 0; i < 9; ++i )
        {
            for ( j = 0; j < 4; ++j )
                key_unrav[i*8+j] = kunrav[i*7+j];
            if ( i < 8 )
            {
                key_unrav[i*8+j]   = kunrav[i*7+j]; ++j;
                key_unrav[i*8+j+1] = kunrav[i*7+j]; ++j;
                key_unrav[i*8+j+1] = kunrav[i*7+j]; ++j;
            }
        }
    }


    _TEGGO_NEWDES_FAKE_INLINE
    void NEWDES_Cipher::SetupEncipher(void const* key)
    {
        int i,j;
        #if defined _X86_ASSEMBLER
        byte_t kunrav[UNRAV_BYTES];
        #else
        byte_t* kunrav = key_unrav;
        #endif
        for ( j=0; j < UNRAV_BYTES; )
            for ( i=0; i < KEY_BYTES; ++i)
                kunrav[j++] = ((byte_t const*)key)[i];
        #if defined _X86_ASSEMBLER
        Normalize_(kunrav);
        #endif
    }

    _TEGGO_NEWDES_FAKE_INLINE
    void NEWDES_Cipher::SetupDecipher(void const* _key)
    {
        #if defined _X86_ASSEMBLER
        byte_t kunrav[UNRAV_BYTES];
        #else
        byte_t* kunrav = key_unrav;
        #endif
        byte_t* k = kunrav;
        byte_t const* key = (byte_t const*)_key;
        int i = 11, j=0;
        while (1)
        {
            *(k++) = key[i];
            i = ((i+1) % KEY_BYTES );
            *(k++) = key[i];
            i = ((i+1) % KEY_BYTES );
            *(k++) = key[i];
            i = ((i+1) % KEY_BYTES );

            *(k++) = key[i];
            i = (i+9) % 15;
            if (i == 12) break;

            *(k++) = key[i++];
            *(k++) = key[i++];
            *(k++) = key[i];
            i = (i+9) % 15;
        }
        #if defined _X86_ASSEMBLER
        Normalize_(kunrav);
        #endif
    }

    _TEGGO_NEWDES_FAKE_INLINE
    void NEWDES_Cipher::DoCipherBlock(void* _b)
    {
        #if defined _X86_ASSEMBLER
        NEWDES_DoCipher_S(key_unrav,_b,1);
        #else
        static byte_t rotor[] =
        {
            32,137,239,188,102,125,221, 72,212, 68, 81, 37, 86,237,147,149,
            70,229, 17,124,115,207, 33, 20,122,143, 25,215, 51,183,138,142,
            146,211,110,173,  1,228,189, 14,103, 78,162, 36,253,167,116,255,
            158, 45,185, 50, 98,168,250,235, 54,141,195,247,240, 63,148,  2,
            224,169,214,180, 62, 22,117,108, 19,172,161,159,160, 47, 43,171,
            194,175,178, 56,196,112, 23,220, 89, 21,164,130,157,  8, 85,251,
            216, 44, 94,179,226, 38, 90,119, 40,202, 34,206, 35, 69,231,246,
            29,109, 74, 71,176,  6, 60,145, 65, 13, 77,151, 12,127, 95,199,
            57,101,  5,232,150,210,129, 24,181, 10,121,187, 48,193,139,252,
            219, 64, 88,233, 96,128, 80, 53,191,144,218, 11,106,132,155,104,
            91,136, 31, 42,243, 66,126,135, 30, 26, 87,186,182,154,242,123,
            82,166,208, 39,152,190,113,205,114,105,225, 84, 73,163, 99,111,
            204, 61,200,217,170, 15,198, 28,192,254,134,234,222,  7,236,248,
            201, 41,177,156, 92,131, 67,249,245,184,203,  9,241,  0, 27, 46,
            133,174, 75, 18, 93,209,100,120, 76,213, 16, 83,  4,107,140, 52,
            58, 55,  3,244, 97,197,238,227,118, 49, 79,230,223,165,153, 59
        };

        unsigned char* k = key_unrav;
        byte_t* b = (byte_t*)_b;
        for (int count=8; count--;)
        {
            b[4] = b[4] ^ rotor[b[0] ^ *(k++)];
            b[5] = b[5] ^ rotor[b[1] ^ *(k++)];
            b[6] = b[6] ^ rotor[b[2] ^ *(k++)];
            b[7] = b[7] ^ rotor[b[3] ^ *(k++)];

            b[1] = b[1] ^ rotor[b[4] ^ *(k++)];
            b[2] = b[2] ^ rotor[b[4] ^ b[5]];
            b[3] = b[3] ^ rotor[b[6] ^ *(k++)];
            b[0] = b[0] ^ rotor[b[7] ^ *(k++)];
        }
        b[4] = b[4] ^ rotor[b[0] ^ *(k++)];
        b[5] = b[5] ^ rotor[b[1] ^ *(k++)];
        b[6] = b[6] ^ rotor[b[2] ^ *(k++)];
        b[7] = b[7] ^ rotor[b[3] ^ *(k++)];
        #endif
    }

    _TEGGO_NEWDES_FAKE_INLINE
    void NEWDES_Cipher::DoCipher(void* data,u32_t count)
    {
        #if defined _X86_ASSEMBLER
        NEWDES_DoCipher_S(key_unrav,(byte_t*)data,count);
        #else
        for (u32_t i = 0; i < count; ++i)
            DoCipherBlock((byte_t*)data+BLOCK_BYTES*i);
        #endif
    }

    _TEGGO_NEWDES_FAKE_INLINE
    void NEWDES_Cipher::DoCipherCBCI(void* data,u32_t count, uint64_t IVc)
    {
        byte_t q[BLOCK_BYTES] = {0};
        if (IVc) memcpy(q,&IVc,8);
        for (u32_t i = 0; i < count; ++i)
        {
            for ( int j =0; j < BLOCK_BYTES; ++j )
                *((byte_t*)data+BLOCK_BYTES*i+j) ^= *(q+j);
            DoCipherBlock((byte_t*)data+BLOCK_BYTES*i);
            memcpy(q,(byte_t*)data+BLOCK_BYTES*i,BLOCK_BYTES);
        }
    }

    _TEGGO_NEWDES_FAKE_INLINE
    void NEWDES_Cipher::DoCipherCBCO(void* data,u32_t count, uint64_t IVc)
    {
        byte_t q[BLOCK_BYTES] = {0};
        if (IVc) memcpy(q,&IVc,8);
        for (u32_t i = 0; i < count; ++i)
        {
            byte_t w[BLOCK_BYTES];
            memcpy(w,(byte_t*)data+BLOCK_BYTES*i,BLOCK_BYTES);
            DoCipherBlock((byte_t*)data+BLOCK_BYTES*i);
            for ( int j =0; j < BLOCK_BYTES; ++j )
                *((byte_t*)data+BLOCK_BYTES*i+j) ^= *(q+j);
            memcpy(q,w,BLOCK_BYTES);
        }
    }

} // namespace

#endif
#endif // ___101853ab_405b_443a_8d26_152269b14074___
