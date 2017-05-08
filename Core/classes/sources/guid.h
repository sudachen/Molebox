
/*

  Copyright (c) 2005-2009, Alexey Sudachen, alexey@sudachen.name

*/

#if !defined __7CEC2042_1664_4CD3_AA3D_48DB4768C18E__
#define __7CEC2042_1664_4CD3_AA3D_48DB4768C18E__


#include "_specific.h"

#if !defined CXX_NO_GUID_SUPPORT

typedef GUID guid_t;

template <class tTx>
struct teggo_GuidOf
{
    typedef typename tTx::Guid Guid;
};

template <class tTx> inline
guid_t const* CXX_STDCALL teggo_guidof( tTx const* /*fake*/ = 0)
{
    typedef typename teggo_GuidOf<tTx>::Guid Guid;
    return &Guid::value;
}

namespace teggo
{

    template <
        unsigned tLx,
        unsigned tWx1, unsigned tWx2,
        unsigned tBx1, unsigned tBx2, unsigned tBx3, unsigned tBx4,
        unsigned tBx5, unsigned tBx6, unsigned tBx7, unsigned tBx8 >
    struct GuidT
    {
        static guid_t const value;
    };

    template <
        unsigned tLx,
        unsigned tWx1, unsigned tWx2,
        unsigned tBx1, unsigned tBx2, unsigned tBx3, unsigned tBx4,
        unsigned tBx5, unsigned tBx6, unsigned tBx7, unsigned tBx8 >
    guid_t const GuidT <tLx,tWx1,tWx2,tBx1,tBx2,tBx3,tBx4,tBx5,tBx6,tBx7,tBx8>
    ::value = { tLx, tWx1, tWx2, { tBx1,tBx2,tBx3,tBx4,tBx5,tBx6,tBx7,tBx8 } };

    struct GUIDless
    {
        bool operator() ( guid_t const& a, guid_t const& b ) const
        {
            return memcmp(&a,&b,sizeof(guid_t)) < 0;
        }
    };

    inline int CXX_STDCALL GUIDcmpf( void const* a, void const* b )
    {
        return memcmp(a,b,sizeof(guid_t));
    }
}

#define TEGGO_DECLARE_GUID(x,l,w1,w2,b1,b2,b3,b4,b5,b6,b7,b8) \
    typedef teggo::GuidT<l,w1,w2,b1,b2,b3,b4,b5,b6,b7,b8> x;

#define TEGGO_DECLARE_GUIDOF(x,l,w1,w2,b1,b2,b3,b4,b5,b6,b7,b8) \
    template <> \
    struct teggo_GuidOf<x> { typedef teggo::GuidT<l,w1,w2,b1,b2,b3,b4,b5,b6,b7,b8> Guid; };

#define TEGGO_DECLARE_GUIDOF_(x,l,w1,w2,b1,b2,b3,b4,b5,b6,b7,b8) \
    struct x; \
    TEGGO_DECLARE_GUIDOF(x,l,w1,w2,b1,b2,b3,b4,b5,b6,b7,b8)

#define TEGGO_GUIDOF_(x) teggo_GuidOf<x>::Guid::value
#define TEGGO_GUIDOF(x) (*teggo_guidof((x*)0))
#endif // CXX_NO_GUID_SUPPORT

#endif // __7CEC2042_1664_4CD3_AA3D_48DB4768C18E__
