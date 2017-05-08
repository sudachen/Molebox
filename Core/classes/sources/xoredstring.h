
/*

  Copyright (c) 2005-2009, Alexey Sudachen, alexey@sudachen.name

*/

#ifndef ___4d259329_5ee0_475c_9566_f52f55850619___
#define ___4d259329_5ee0_475c_9566_f52f55850619___

namespace teggo
{

    char* XoredStringA_Xor(unsigned char const* data, unsigned n, char* val)
    {
        int x = data[0];
        for ( unsigned i = 1; i < n; ++i )
        {
            val[i-1] = data[i] ^ x;
            x = ((x << 1) | (x >> 7))  & 0x0ff;
        }
        val[n-1] = 0;
        return val;
    }

    template <unsigned tN>
    struct XoredStringA_
    {
        mutable char value[tN];
    };


    template <unsigned tN>
    struct XoredStringA
    {
        unsigned char text[tN];
        inline char* Xor(XoredStringA_<tN> const& a = XoredStringA_<tN>())
        {
            return XoredStringA_Xor(text,tN,a.value);
        }
    };

} // namespace

#endif // ___4d259329_5ee0_475c_9566_f52f55850619___
