
/*

  Copyright (c) 2005-2009, Alexey Sudachen, alexey@sudachen.name

*/

#if !defined __E3C93A52_A2B1_455B_9552_6F835A4C52EB__
#define __E3C93A52_A2B1_455B_9552_6F835A4C52EB__

#include "_specific.h"
#include "vector.h"

namespace teggo
{

    template <class tTx>
    struct Size2T
    {
        tTx width;
        tTx height;
        Size2T() {}
        Size2T(tTx w, tTx h) : width(w), height(h) {}
    };
    typedef Size2T<int> size2_t;
    typedef Size2T<float> fsize2_t;

    template<class tTx>
    struct RectT
    {
        tTx left;
        tTx top;
        tTx right;
        tTx bottom;
        RectT() {}
        RectT(tTx l,tTx t,tTx r,tTx b) : left(l),top(t),right(r),bottom(b) {}
        RectT(Vect2T<tTx> const& p,Size2T<tTx> const& s) : left(p.x),top(p.y),right(p.x+s.width),bottom(p.y+s.height) {}

        static RectT mkbysize(tTx x, tTx y, tTx width, tTx height)
        {
            return RectT(x,y,x+width,y+height);
        }

        RectT Shift(tTx dx,tTx dy) const
        {
            return RectT(left+dx,top+dy,right+dx,bottom+dy);
        }

        RectT& ShiftSelf(tTx dx,tTx dy)
        {
            left    +=dx;
            top     +=dy;
            right   +=dx;
            bottom  +=dy;
            return *this;
        }

        RectT Move(tTx x,tTx y) const
        {
            return RectT(x,y,x+right-left,y+bottom-top);
        }

        RectT& MoveSelf(tTx x,tTx y)
        {
            right = x+right-left;
            bottom = y+bottom-top;
            left = x;
            top = y;
            return *this;
        }

        RectT& ClipSelf(tTx x, tTx y)
        {
            if ( teggo_max(right-left,0) > x ) right = teggo_max(left+x,0);
            if ( teggo_max(bottom-top,0) > y ) bottom = teggo_max(top+y,0);
            return *this;
        }

        RectT Intersection(const RectT& r) const
        {
            tTx x_right   = teggo_min(r.right,right);
            tTx x_bottom  = teggo_min(r.bottom,bottom);
            tTx x_left    = teggo_max(r.left,left);
            tTx x_top     = teggo_max(r.top,top);
            if ( x_right  < x_left ) return RectT(0,0,0,0);
            if ( x_bottom < x_top  ) return RectT(0,0,0,0);
            return RectT(x_left,x_top,x_right,x_bottom);
        }

        RectT& IntersectionSelf(const RectT& r)
        {
            right   = /*std::*/teggo_min(r.right,right);
            bottom  = /*std::*/teggo_min(r.bottom,bottom);
            left    = /*std::*/teggo_max(r.left,left);
            top     = /*std::*/teggo_max(r.top,top);
            if ( right  < left ) left = right = 0;
            if ( bottom < top  ) bottom = top = 0;
            return *this;
        }

        bool Intersect(const RectT& r) const
        {
            tTx x_right   = /*std::*/teggo_min(r.right,right);
            tTx x_left    = /*std::*/teggo_max(r.left,left);
            if ( x_right  < x_left ) return false;
            tTx x_bottom  = /*std::*/teggo_min(r.bottom,bottom);
            tTx x_top     = /*std::*/teggo_max(r.top,top);
            if ( x_bottom < x_top  ) return false;
            return true;
        }

        bool Contains(const RectT& r) const
        {
            return  r.left >= left && r.top >= top &&
                    r.right <= right && r.bottom <= bottom &&
                    r.left <= r.right && r.top <= r.bottom;
        }

        bool Contains(tTx x, tTx y ) const
        {
            return  x >= left && y >= top &&
                    x <= right && y <= bottom;
        }

        bool operator ==(const RectT& r) const
        {
            return  memcmp(this,&r,sizeof(RectT)) == 0;
        }

        bool IsZero() const
        {
            return left == right || top == bottom;
        }

        tTx Width() const
        {
            return right-left;
        }

        tTx Height() const
        {
            return bottom-top;
        }

        RectT& GrowSelf(float d)
        {
            left -= d;
            top -= d;
            right += d;
            bottom += d;
            return *this;
        }

        RectT Grow(float d) const
        {
            return RectT(left-d,top-d,right+d,bottom+d);
        }

        Vect2T<tTx> Position() const
        {
            return Vect2T<tTx>(left,top);
        }

        Size2T<tTx> Size() const
        {
            return Size2T<tTx>( Width(), Height() );
        }
    };

    typedef RectT<int>   rect_t;
    typedef RectT<float> frect_t;

} // namespace

#endif // __E3C93A52_A2B1_455B_9552_6F835A4C52EB__
