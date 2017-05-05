
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

#ifndef C_once_A1549AAE_F443_4982_AFD4_F5BE02C14DE6
#define C_once_A1549AAE_F443_4982_AFD4_F5BE02C14DE6

#ifdef _LIBYOYO
#define _YO_CRC_BUILTIN
#endif

#include "yoyo.hc"
#include "string.hc"

#ifdef _YO_CRC_BUILTIN
# define _YO_CRC_BUILTIN_CODE(Code) Code
# define _YO_CRC_EXTERN 
#else
# define _YO_CRC_BUILTIN_CODE(Code)
# define _YO_CRC_EXTERN extern 
#endif

_YO_CRC_EXTERN char Oj_Digest_Size_OjCID[] _YO_CRC_BUILTIN_CODE( = "#digest-size/i"); 
int Oj_Digest_Size(void *hash) _YO_CRC_BUILTIN_CODE(
  { return Yo_Find_Constant_Of(hash,Oj_Digest_Size_OjCID,YO_RAISE_ERROR,0); });
  
_YO_CRC_EXTERN char Oj_Digest_Update_OjMID[] _YO_CRC_BUILTIN_CODE( = "digest-update/@*i"); 
void Oj_Digest_Update(void *hash,void *buf,int L) _YO_CRC_BUILTIN_CODE(
  { ((void(*)(void*,void*,int))Yo_Find_Method_Of(&hash,Oj_Digest_Update_OjMID,YO_RAISE_ERROR))
        (hash,buf,L); });

_YO_CRC_EXTERN char Oj_Digest_Start_OjMID[] _YO_CRC_BUILTIN_CODE( = "digest-start/@"); 
void Oj_Digest_Start(void *hash) _YO_CRC_BUILTIN_CODE(
  { ((void(*)(void*))Yo_Find_Method_Of(&hash,Oj_Digest_Start_OjMID,YO_RAISE_ERROR))
        (hash); });

_YO_CRC_EXTERN char Oj_Digest_Finish_OjMID[] _YO_CRC_BUILTIN_CODE( = "digest-finish/@*"); 
void *Oj_Digest_Finish(void *hash, void *out) _YO_CRC_BUILTIN_CODE(
  { return ((void*(*)(void*,void*))Yo_Find_Method_Of(&hash,Oj_Digest_Finish_OjMID,YO_RAISE_ERROR))
        (hash,out); });

char *Oj_Digest_Hex_Finish(void *hash)
#ifdef _YO_CRC_BUILTIN
  { 
    int dgst_size = Oj_Digest_Size(hash);
    byte_t *dgst = alloca(dgst_size);
    return Str_Hex_Encode(Oj_Digest_Finish(hash,dgst),dgst_size);
  }
#endif
  ;
  
uint_t    Crc_32(uint_t crc, void *, int len);
ushort_t  Crc_16(ushort_t crc, void *, int len);
byte_t    Crc_8(byte_t crc, void *, int len);
byte_t    Crc_8_Of_Cstr(char *S);
uint_t    Adler_32(uint_t adler, void *, int len);

#define Digest_Crc_32(Data,Len) Crc_32(0,Data,Len)
#define Digest_Crc_16(Data,Len) Crc_16(0,Data,Len)
#define Digest_Crc_8(Data,Len)  Crc_8(0,Data,Len)
#define Crc32_Digest(Data,Len,Buf)  Unsigned_To_Four(Crc_32(0,Data,Len),(Buf))
#define Crc16_Digest(Data,Len,Buf)  Unsigned_To_Two(Crc_16(0,Data,Len),(Buf))
#define Crc8_Digest(Data,Len,Buf)   (*(byte_t*)(Buf) = Crc_8(0,Data,Len))

byte_t Crc_8_32(byte_t start, uint_t val) 
#ifdef _YO_CRC_BUILTIN
  { 
    byte_t q[] = { (byte_t)(val>>24), (byte_t)(val>>16), (byte_t)(val>>8), (byte_t)(val) }; 
    return Crc_8(start,q,4); 
  }
#endif
  ;
  
ushort_t Crc_16_32(ushort_t start, uint_t val) 
#ifdef _YO_CRC_BUILTIN
  { 
    byte_t q[] = { (byte_t)(val>>24), (byte_t)(val>>16), (byte_t)(val>>8), (byte_t)(val) }; 
    return Crc_16(start,q,4); 
  }
#endif
  ;

uint_t Crc_32_32(uint_t start, uint_t val) 
#ifdef _YO_CRC_BUILTIN
  { 
    byte_t q[] = { (byte_t)(val>>24), (byte_t)(val>>16), (byte_t)(val>>8), (byte_t)(val) }; 
    return Crc_32(start,q,4); 
  }
#endif
  ;
  
byte_t Crc_8_16(byte_t start, ushort_t val) 
#ifdef _YO_CRC_BUILTIN
  { 
    byte_t q[] = { (byte_t)(val>>8), (byte_t)(val) }; 
    return Crc_8(start,q,2); 
  }
#endif
  ;

ushort_t Crc_16_16(ushort_t start, ushort_t val) 
#ifdef _YO_CRC_BUILTIN
  { 
    byte_t q[] = { (byte_t)(val>>8), (byte_t)(val) }; 
    return Crc_16(start,q,2); 
  }
#endif
  ;
  
uint_t Crc_32_16(uint_t start, ushort_t val) 
#ifdef _YO_CRC_BUILTIN
  { 
    byte_t q[] = { (byte_t)(val>>8), (byte_t)(val) }; 
    return Crc_32(start,q,2); 
  }
#endif
  ;
  
byte_t Crc_8_64(byte_t start, uquad_t val) 
#ifdef _YO_CRC_BUILTIN
  { 
    byte_t C = Crc_8_32(start,(uint_t)(val>>32)); 
    return Crc_8_32(C,(uint_t)val); 
  }
#endif
  ;

ushort_t Crc_16_64(ushort_t start, uquad_t val) 
#ifdef _YO_CRC_BUILTIN
  { 
    ushort_t C = Crc_16_32(start,(uint_t)(val>>32)); 
    return Crc_16_32(C,(uint_t)val); 
  }
#endif
  ;
  
uint_t Crc_32_64(uint_t start, uquad_t val) 
#ifdef _YO_CRC_BUILTIN
  { 
    uint_t C = Crc_32_32(start,(uint_t)(val>>32)); 
    return Crc_32_32(C,(uint_t)val); 
  }
#endif
  ;

#ifdef _YO_CRC_BUILTIN

  #define CRC32_MCRO_DO1(buf)  crc = crc_table[((int)crc ^ (*buf++)) & 0xff] ^ (crc >> 8);
  #define CRC32_MCRO_DO2(buf)  CRC32_MCRO_DO1(buf); CRC32_MCRO_DO1(buf);
  #define CRC32_MCRO_DO4(buf)  CRC32_MCRO_DO2(buf); CRC32_MCRO_DO2(buf);
  #define CRC32_MCRO_DO8(buf)  CRC32_MCRO_DO4(buf); CRC32_MCRO_DO4(buf);

  uint_t Crc_32(uint_t crc, void *_buf, int len)
    {
      byte_t *buf = _buf;
      static uint_t crc_table[256] =
        {
          0x00000000, 0x77073096, 0xee0e612c, 0x990951ba, 0x076dc419,
          0x706af48f, 0xe963a535, 0x9e6495a3, 0x0edb8832, 0x79dcb8a4,
          0xe0d5e91e, 0x97d2d988, 0x09b64c2b, 0x7eb17cbd, 0xe7b82d07,
          0x90bf1d91, 0x1db71064, 0x6ab020f2, 0xf3b97148, 0x84be41de,
          0x1adad47d, 0x6ddde4eb, 0xf4d4b551, 0x83d385c7, 0x136c9856,
          0x646ba8c0, 0xfd62f97a, 0x8a65c9ec, 0x14015c4f, 0x63066cd9,
          0xfa0f3d63, 0x8d080df5, 0x3b6e20c8, 0x4c69105e, 0xd56041e4,
          0xa2677172, 0x3c03e4d1, 0x4b04d447, 0xd20d85fd, 0xa50ab56b,
          0x35b5a8fa, 0x42b2986c, 0xdbbbc9d6, 0xacbcf940, 0x32d86ce3,
          0x45df5c75, 0xdcd60dcf, 0xabd13d59, 0x26d930ac, 0x51de003a,
          0xc8d75180, 0xbfd06116, 0x21b4f4b5, 0x56b3c423, 0xcfba9599,
          0xb8bda50f, 0x2802b89e, 0x5f058808, 0xc60cd9b2, 0xb10be924,
          0x2f6f7c87, 0x58684c11, 0xc1611dab, 0xb6662d3d, 0x76dc4190,
          0x01db7106, 0x98d220bc, 0xefd5102a, 0x71b18589, 0x06b6b51f,
          0x9fbfe4a5, 0xe8b8d433, 0x7807c9a2, 0x0f00f934, 0x9609a88e,
          0xe10e9818, 0x7f6a0dbb, 0x086d3d2d, 0x91646c97, 0xe6635c01,
          0x6b6b51f4, 0x1c6c6162, 0x856530d8, 0xf262004e, 0x6c0695ed,
          0x1b01a57b, 0x8208f4c1, 0xf50fc457, 0x65b0d9c6, 0x12b7e950,
          0x8bbeb8ea, 0xfcb9887c, 0x62dd1ddf, 0x15da2d49, 0x8cd37cf3,
          0xfbd44c65, 0x4db26158, 0x3ab551ce, 0xa3bc0074, 0xd4bb30e2,
          0x4adfa541, 0x3dd895d7, 0xa4d1c46d, 0xd3d6f4fb, 0x4369e96a,
          0x346ed9fc, 0xad678846, 0xda60b8d0, 0x44042d73, 0x33031de5,
          0xaa0a4c5f, 0xdd0d7cc9, 0x5005713c, 0x270241aa, 0xbe0b1010,
          0xc90c2086, 0x5768b525, 0x206f85b3, 0xb966d409, 0xce61e49f,
          0x5edef90e, 0x29d9c998, 0xb0d09822, 0xc7d7a8b4, 0x59b33d17,
          0x2eb40d81, 0xb7bd5c3b, 0xc0ba6cad, 0xedb88320, 0x9abfb3b6,
          0x03b6e20c, 0x74b1d29a, 0xead54739, 0x9dd277af, 0x04db2615,
          0x73dc1683, 0xe3630b12, 0x94643b84, 0x0d6d6a3e, 0x7a6a5aa8,
          0xe40ecf0b, 0x9309ff9d, 0x0a00ae27, 0x7d079eb1, 0xf00f9344,
          0x8708a3d2, 0x1e01f268, 0x6906c2fe, 0xf762575d, 0x806567cb,
          0x196c3671, 0x6e6b06e7, 0xfed41b76, 0x89d32be0, 0x10da7a5a,
          0x67dd4acc, 0xf9b9df6f, 0x8ebeeff9, 0x17b7be43, 0x60b08ed5,
          0xd6d6a3e8, 0xa1d1937e, 0x38d8c2c4, 0x4fdff252, 0xd1bb67f1,
          0xa6bc5767, 0x3fb506dd, 0x48b2364b, 0xd80d2bda, 0xaf0a1b4c,
          0x36034af6, 0x41047a60, 0xdf60efc3, 0xa867df55, 0x316e8eef,
          0x4669be79, 0xcb61b38c, 0xbc66831a, 0x256fd2a0, 0x5268e236,
          0xcc0c7795, 0xbb0b4703, 0x220216b9, 0x5505262f, 0xc5ba3bbe,
          0xb2bd0b28, 0x2bb45a92, 0x5cb36a04, 0xc2d7ffa7, 0xb5d0cf31,
          0x2cd99e8b, 0x5bdeae1d, 0x9b64c2b0, 0xec63f226, 0x756aa39c,
          0x026d930a, 0x9c0906a9, 0xeb0e363f, 0x72076785, 0x05005713,
          0x95bf4a82, 0xe2b87a14, 0x7bb12bae, 0x0cb61b38, 0x92d28e9b,
          0xe5d5be0d, 0x7cdcefb7, 0x0bdbdf21, 0x86d3d2d4, 0xf1d4e242,
          0x68ddb3f8, 0x1fda836e, 0x81be16cd, 0xf6b9265b, 0x6fb077e1,
          0x18b74777, 0x88085ae6, 0xff0f6a70, 0x66063bca, 0x11010b5c,
          0x8f659eff, 0xf862ae69, 0x616bffd3, 0x166ccf45, 0xa00ae278,
          0xd70dd2ee, 0x4e048354, 0x3903b3c2, 0xa7672661, 0xd06016f7,
          0x4969474d, 0x3e6e77db, 0xaed16a4a, 0xd9d65adc, 0x40df0b66,
          0x37d83bf0, 0xa9bcae53, 0xdebb9ec5, 0x47b2cf7f, 0x30b5ffe9,
          0xbdbdf21c, 0xcabac28a, 0x53b39330, 0x24b4a3a6, 0xbad03605,
          0xcdd70693, 0x54de5729, 0x23d967bf, 0xb3667a2e, 0xc4614ab8,
          0x5d681b02, 0x2a6f2b94, 0xb40bbe37, 0xc30c8ea1, 0x5a05df1b,
          0x2d02ef8d
        };
      if (buf == 0) return 0;
      crc = crc ^ 0x0ffffffff;
      while (len >= 8)
        {
          CRC32_MCRO_DO8(buf);
          len -= 8;
        }
      if (len)
        do {CRC32_MCRO_DO1(buf);} while (--len);
      return crc ^ 0x0ffffffff;
    }

  #undef CRC32_MCRO_DO1
  #undef CRC32_MCRO_DO2
  #undef CRC32_MCRO_DO4
  #undef CRC32_MCRO_DO8


  ushort_t Crc_16(ushort_t crc, void *_buf, int len)
    {
      byte_t *buf = _buf;
      static ushort_t crc_table[256] =
        {
          0x0000, 0xC0C1, 0xC181, 0x0140, 0xC301, 0x03C0, 0x0280, 0xC241,
          0xC601, 0x06C0, 0x0780, 0xC741, 0x0500, 0xC5C1, 0xC481, 0x0440,
          0xCC01, 0x0CC0, 0x0D80, 0xCD41, 0x0F00, 0xCFC1, 0xCE81, 0x0E40,
          0x0A00, 0xCAC1, 0xCB81, 0x0B40, 0xC901, 0x09C0, 0x0880, 0xC841,
          0xD801, 0x18C0, 0x1980, 0xD941, 0x1B00, 0xDBC1, 0xDA81, 0x1A40,
          0x1E00, 0xDEC1, 0xDF81, 0x1F40, 0xDD01, 0x1DC0, 0x1C80, 0xDC41,
          0x1400, 0xD4C1, 0xD581, 0x1540, 0xD701, 0x17C0, 0x1680, 0xD641,
          0xD201, 0x12C0, 0x1380, 0xD341, 0x1100, 0xD1C1, 0xD081, 0x1040,
          0xF001, 0x30C0, 0x3180, 0xF141, 0x3300, 0xF3C1, 0xF281, 0x3240,
          0x3600, 0xF6C1, 0xF781, 0x3740, 0xF501, 0x35C0, 0x3480, 0xF441,
          0x3C00, 0xFCC1, 0xFD81, 0x3D40, 0xFF01, 0x3FC0, 0x3E80, 0xFE41,
          0xFA01, 0x3AC0, 0x3B80, 0xFB41, 0x3900, 0xF9C1, 0xF881, 0x3840,
          0x2800, 0xE8C1, 0xE981, 0x2940, 0xEB01, 0x2BC0, 0x2A80, 0xEA41,
          0xEE01, 0x2EC0, 0x2F80, 0xEF41, 0x2D00, 0xEDC1, 0xEC81, 0x2C40,
          0xE401, 0x24C0, 0x2580, 0xE541, 0x2700, 0xE7C1, 0xE681, 0x2640,
          0x2200, 0xE2C1, 0xE381, 0x2340, 0xE101, 0x21C0, 0x2080, 0xE041,
          0xA001, 0x60C0, 0x6180, 0xA141, 0x6300, 0xA3C1, 0xA281, 0x6240,
          0x6600, 0xA6C1, 0xA781, 0x6740, 0xA501, 0x65C0, 0x6480, 0xA441,
          0x6C00, 0xACC1, 0xAD81, 0x6D40, 0xAF01, 0x6FC0, 0x6E80, 0xAE41,
          0xAA01, 0x6AC0, 0x6B80, 0xAB41, 0x6900, 0xA9C1, 0xA881, 0x6840,
          0x7800, 0xB8C1, 0xB981, 0x7940, 0xBB01, 0x7BC0, 0x7A80, 0xBA41,
          0xBE01, 0x7EC0, 0x7F80, 0xBF41, 0x7D00, 0xBDC1, 0xBC81, 0x7C40,
          0xB401, 0x74C0, 0x7580, 0xB541, 0x7700, 0xB7C1, 0xB681, 0x7640,
          0x7200, 0xB2C1, 0xB381, 0x7340, 0xB101, 0x71C0, 0x7080, 0xB041,
          0x5000, 0x90C1, 0x9181, 0x5140, 0x9301, 0x53C0, 0x5280, 0x9241,
          0x9601, 0x56C0, 0x5780, 0x9741, 0x5500, 0x95C1, 0x9481, 0x5440,
          0x9C01, 0x5CC0, 0x5D80, 0x9D41, 0x5F00, 0x9FC1, 0x9E81, 0x5E40,
          0x5A00, 0x9AC1, 0x9B81, 0x5B40, 0x9901, 0x59C0, 0x5880, 0x9841,
          0x8801, 0x48C0, 0x4980, 0x8941, 0x4B00, 0x8BC1, 0x8A81, 0x4A40,
          0x4E00, 0x8EC1, 0x8F81, 0x4F40, 0x8D01, 0x4DC0, 0x4C80, 0x8C41,
          0x4400, 0x84C1, 0x8581, 0x4540, 0x8701, 0x47C0, 0x4680, 0x8641,
          0x8201, 0x42C0, 0x4380, 0x8341, 0x4100, 0x81C1, 0x8081, 0x4040
        };

      while (len--)
          crc = (crc >> 8) ^ crc_table[(crc & 0xFF) ^ *buf++];

      return crc;
    }

  static byte_t Crc_8_Table[256] =
    {
      0x00, 0x31, 0x62, 0x53, 0xC4, 0xF5, 0xA6, 0x97,
      0xB9, 0x88, 0xDB, 0xEA, 0x7D, 0x4C, 0x1F, 0x2E,
      0x43, 0x72, 0x21, 0x10, 0x87, 0xB6, 0xE5, 0xD4,
      0xFA, 0xCB, 0x98, 0xA9, 0x3E, 0x0F, 0x5C, 0x6D,
      0x86, 0xB7, 0xE4, 0xD5, 0x42, 0x73, 0x20, 0x11,
      0x3F, 0x0E, 0x5D, 0x6C, 0xFB, 0xCA, 0x99, 0xA8,
      0xC5, 0xF4, 0xA7, 0x96, 0x01, 0x30, 0x63, 0x52,
      0x7C, 0x4D, 0x1E, 0x2F, 0xB8, 0x89, 0xDA, 0xEB,
      0x3D, 0x0C, 0x5F, 0x6E, 0xF9, 0xC8, 0x9B, 0xAA,
      0x84, 0xB5, 0xE6, 0xD7, 0x40, 0x71, 0x22, 0x13,
      0x7E, 0x4F, 0x1C, 0x2D, 0xBA, 0x8B, 0xD8, 0xE9,
      0xC7, 0xF6, 0xA5, 0x94, 0x03, 0x32, 0x61, 0x50,
      0xBB, 0x8A, 0xD9, 0xE8, 0x7F, 0x4E, 0x1D, 0x2C,
      0x02, 0x33, 0x60, 0x51, 0xC6, 0xF7, 0xA4, 0x95,
      0xF8, 0xC9, 0x9A, 0xAB, 0x3C, 0x0D, 0x5E, 0x6F,
      0x41, 0x70, 0x23, 0x12, 0x85, 0xB4, 0xE7, 0xD6,
      0x7A, 0x4B, 0x18, 0x29, 0xBE, 0x8F, 0xDC, 0xED,
      0xC3, 0xF2, 0xA1, 0x90, 0x07, 0x36, 0x65, 0x54,
      0x39, 0x08, 0x5B, 0x6A, 0xFD, 0xCC, 0x9F, 0xAE,
      0x80, 0xB1, 0xE2, 0xD3, 0x44, 0x75, 0x26, 0x17,
      0xFC, 0xCD, 0x9E, 0xAF, 0x38, 0x09, 0x5A, 0x6B,
      0x45, 0x74, 0x27, 0x16, 0x81, 0xB0, 0xE3, 0xD2,
      0xBF, 0x8E, 0xDD, 0xEC, 0x7B, 0x4A, 0x19, 0x28,
      0x06, 0x37, 0x64, 0x55, 0xC2, 0xF3, 0xA0, 0x91,
      0x47, 0x76, 0x25, 0x14, 0x83, 0xB2, 0xE1, 0xD0,
      0xFE, 0xCF, 0x9C, 0xAD, 0x3A, 0x0B, 0x58, 0x69,
      0x04, 0x35, 0x66, 0x57, 0xC0, 0xF1, 0xA2, 0x93,
      0xBD, 0x8C, 0xDF, 0xEE, 0x79, 0x48, 0x1B, 0x2A,
      0xC1, 0xF0, 0xA3, 0x92, 0x05, 0x34, 0x67, 0x56,
      0x78, 0x49, 0x1A, 0x2B, 0xBC, 0x8D, 0xDE, 0xEF,
      0x82, 0xB3, 0xE0, 0xD1, 0x46, 0x77, 0x24, 0x15,
      0x3B, 0x0A, 0x59, 0x68, 0xFF, 0xCE, 0x9D, 0xAC
    };


  byte_t Crc_8(byte_t crc, void *_buf, int len)
    {
      byte_t *buf = _buf;
      while (len--)
        crc = Crc_8_Table[crc ^ *buf++];

      return crc;
    }

  byte_t Crc_8_Of_Cstr(char *S)
    {
      byte_t crc = 0;
      byte_t *buf = (byte_t*)S;
      while (*buf)
        crc = Crc_8_Table[crc ^ *buf++];

      return crc;
    }

#define _ADLER32_DO1(buf,i)  {s1 += buf[i]; s2 += s1;}
#define _ADLER32_DO2(buf,i)  _ADLER32_DO1(buf,i); _ADLER32_DO1(buf,i+1);
#define _ADLER32_DO4(buf,i)  _ADLER32_DO2(buf,i); _ADLER32_DO2(buf,i+2);
#define _ADLER32_DO8(buf,i)  _ADLER32_DO4(buf,i); _ADLER32_DO4(buf,i+4);
#define _ADLER32_DO16(buf)   _ADLER32_DO8(buf,0); _ADLER32_DO8(buf,8);

uint_t Adler_32(uint_t adler,void *_buf,int len)
  {
    enum
      {
        BASE = 65521, /* largest prime smaller than 65536 */
        /* NMAX is the largest n such that 255n(n+1)/2 + (n+1)(BASE-1) <= 2^32-1 */
        NMAX = 5552
      };

    byte_t *buf = _buf;
    uint_t s1 = adler & 0xffff;
    uint_t s2 = (adler >> 16) & 0xffff;
    int k;

    if (buf == 0) return 1;

    while (len > 0)
      {
        k = len < NMAX ? len : NMAX;
        len -= k;
        while (k >= 16)
          {
            _ADLER32_DO16(buf);
            buf += 16;
            k -= 16;
          }
        if (k != 0)
          do
            {
              s1 += *buf++;
              s2 += s1;
            }
          while (--k);
        s1 %= BASE;
        s2 %= BASE;
      }
      
    return (s2 << 16) | s1;
  }

#undef _ADLER32_DO1
#undef _ADLER32_DO2
#undef _ADLER32_DO4
#undef _ADLER32_DO8
#undef _ADLER32_DO16

#endif /* _YO_CRC_BUILTIN */

#endif /* C_once_A1549AAE_F443_4982_AFD4_F5BE02C14DE6 */

