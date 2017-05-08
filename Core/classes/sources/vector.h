
/*

  Copyright (c) 2005-2009, Alexey Sudachen, alexey@sudachen.name

*/

#if !defined __F5DA1352_F667_4327_853D_65F0B2E43C4B__
#define __F5DA1352_F667_4327_853D_65F0B2E43C4B__

#include "_specific.h"
#include "mymath.h"

namespace teggo
{
    template <class real>
    struct Vect2T
    {
        union
        {
            real x_[2];
            struct
            {
                real x;
                real y;
            };
        };

        Vect2T() { x_[0] = real(), x_[1] = real(); }
        Vect2T(real x0,real x1) { x_[0] = x0, x_[1] = x1; }
        real operator() (int i) const { return x_[i]; }
        real& operator() (int i) { return x_[i]; }
        Vect2T& operator +=(real value) { x_[0] += value; x_[1] += value; return *this;}
        Vect2T& operator -=(real value) { x_[0] -= value; x_[1] -= value; return *this; }
        Vect2T& operator *=(real value) { x_[0] *= value; x_[1] *= value; return *this; }
        Vect2T& operator /=(real value) { x_[0] /= value; x_[1] /= value; return *this; }
        Vect2T& operator += (const Vect2T& v) { x_[0] += v.x_[0]; x_[1] += v.x_[1]; return *this; }
        Vect2T& operator -= (const Vect2T& v) { x_[0] -= v.x_[0]; x_[1] -= v.x_[1]; return *this; }
        real Length() const { return sqrt(x_[0] * x_[0] + x_[1] * x_[1]); }
        Vect2T Normalize() const { return (Vect2T(*this) /= Length()); }
    };

    typedef Vect2T<float> fvect2_t;
    typedef const Vect2T<float> fvect2cr_t;
    typedef Vect2T<double> dvect2_t;
    typedef const Vect2T<double> dvect2cr_t;

    template <class real> inline
    real CXX_STDCALL operator ^ (const Vect2T<real>& v0,const Vect2T<real>& v1)
    {
        return v0.x_[0] * v1.x_[1] - v0.x_[1] * v1.x_[0];
    }

    template <class real> inline
    real CXX_STDCALL operator * (const Vect2T<real>& v0,const Vect2T<real>& v1)
    {
        return v0.x_[0] * v1.x_[0] + v0.x_[1] * v1.x_[1];
    }

    template <class real> inline
    Vect2T<real> CXX_STDCALL operator + (const Vect2T<real>& v0,const Vect2T<real>& v1)
    {
        return Vect2T<real>(v0.x_[0] + v1.x_[0],v0.x_[1] + v1.x_[1]);
    }

    template <class real> inline
    Vect2T<real> CXX_STDCALL operator - (const Vect2T<real>& v0,const Vect2T<real>& v1)
    {
        return Vect2T<real>(v0.x_[0] - v1.x_[0],v0.x_[1] - v1.x_[1]);
    }

    template <class real> inline
    Vect2T<real> CXX_STDCALL operator * (const Vect2T<real>& v0,real value)
    {
        return Vect2T<real>(v0.x_[0] * value, v0.x_[1] * value);
    }

    template <class real> inline
    Vect2T<real> CXX_STDCALL operator / (const Vect2T<real>& v0,real value)
    {
        return Vect2T<real>(v0.x_[0] / value, v0.x_[1] / value);
    }

    template <class real> inline
    Vect2T<real> CXX_STDCALL operator + (const Vect2T<real>& v0,real value)
    {
        return Vect2T<real>(v0.x_[0] + value,v0.x_[1] + value);
    }

    template <class real> inline
    Vect2T<real> CXX_STDCALL operator - (const Vect2T<real>& v0,real value)
    {
        return Vect2T<real>(v0.x_[0] - value,v0.x_[1] - value);
    }

    template <class real> inline
    real CXX_STDCALL Distance(const Vect2T<real>& a, const Vect2T<real> b)
    {
        const real X = b(0)-a(0);
        const real Y = b(1)-a(1);
        return sqrt(X*X+Y*Y);
    }

    template <class real>
    bool CXX_STDCALL IntersectSegments(
        const Vect2T<real> a0,const Vect2T<real> a1,
        const Vect2T<real> b0,const Vect2T<real> b1,
        Vect2T<real>* res)
    {
        const real X  = a0(0) - b0(0);
        const real Y  = a0(1) - b0(1);
        const real A  = a1(0) - a0(0);
        const real B  = a1(1) - a0(1);
        const real A_ = b1(0) - b0(0);
        const real B_ = b1(1) - b0(1);
        const real D  = A*B_  - B*A_;
        if ( in_epsilon(D) ) return false;
        const real t  = ( Y*A_ - X*B_ ) / D;
        const real t_ = ( Y*A  - X*B  ) / D;
        if ( t < 0 || t > 1 ) return false;
        if ( t_ < 0 || t_ > 1 ) return false;
        if ( res )
        {
            (*res)(0) = t*A + a0(0);
            (*res)(1) = t*B + a0(1);
        }
        return true;
    }

    template <class real>
    bool CXX_STDCALL IntersectRectAndSegment(
        const Vect2T<real>(&rect)[4],
        const Vect2T<real> a0,const Vect2T<real> b0,
        Vect2T<real>* res)
    {
        Vect2T<real> p[4];
        unsigned count = 0;

        if ( IntersectSegments(a0,b0,rect[0],rect[1],&p[count]) ) ++count;
        if ( IntersectSegments(a0,b0,rect[1],rect[2],&p[count]) ) ++count;
        if ( IntersectSegments(a0,b0,rect[2],rect[3],&p[count]) ) ++count;
        if ( IntersectSegments(a0,b0,rect[3],rect[0],&p[count]) ) ++count;
        if ( !count ) return false;

        if ( count > 1 )
        {
            real dist = Distance(a0,p[0]);
            real d;
            for ( unsigned i = 1; i < count; ++i )
                if ( (d = Distance(a0,p[count])) < dist )
                    p[0] = p[count], dist = d;
        }
        *res = p[0];
        return true;
    }

    /* --------------------------------------------------------------------------------------------- */
    //template <class real>
    //bool ContainsPoint(const Vect2T<real>* poly,long count,const Vect2T<real> a)
    //{
    //   unsigned val = 0;
    //   if ( count < 3 ) return false;
    //   for ( int i = 1; i < count; ++i )
    //   {
    //      const Vect2T<real> ap = poly[i-1] - a,
    //                          pp = poly[i] - poly[i-1];
    //      real p = pp(0) * ap(1) - ap(0) * pp(1);
    //      if ( p < 0 )      val |= 0x1;
    //      else if ( p > 0 ) val |= 0x2;
    //      if ( val == 0x3 ) return false;
    //   }
    //   return true;
    //}

    typedef Vect2T<int> point_t;
    typedef Vect2T<float> fpoint_t;

} // namespace

#endif // __F5DA1352_F667_4327_853D_65F0B2E43C4B__
