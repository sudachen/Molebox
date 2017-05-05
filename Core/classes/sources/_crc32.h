
#if !defined __DC6F6981_BBF5_4871_B1AF_5404567FD536__
#define __DC6F6981_BBF5_4871_B1AF_5404567FD536__

#include <libhash/CRC8.h>
#include <libhash/CRC16.h>
#include <libhash/CRC32.h>

namespace teggo {

inline uint32_t Crc32(uint32_t crc, const void* buf, size_t len)
{
    return Crc_32(crc, buf, len);
}

inline uint16_t Crc16(uint16_t crc, const uint8_t* buf, size_t len)
{
    return Crc_16(crc, buf, len);
}

inline uint8_t Crc8(uint8_t crc, const uint8_t* buf, size_t len)
{
    return Crc_8(crc, buf, len);
}

#if defined _TEGGOINLINE || defined _TEGGO_CRC32_HERE

CXX_EXTERNC unsigned long crc32(unsigned long crc, const unsigned char* buf, unsigned int len)
  {
    return Crc32(crc,buf,len);
  }

#endif // defined _TEGGOINLINE || defined _TEGGO_CRC32_HERE

} // namespace

#endif // __DC6F6981_BBF5_4871_B1AF_5404567FD536__
