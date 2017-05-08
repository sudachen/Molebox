
/*

  Copyright (c) 2004, Konstantin Stupnik aka skv (skv@sibinco.ru)
  Copyright (c) 2005-2009, Alexey Sudachen, alexey@sudachen.name.

*/

#if !defined __6FAF6B21_D479_40b1_8A91_BA4B0D4C28E3__
#define __6FAF6B21_D479_40b1_8A91_BA4B0D4C28E3__

#include "_specific.h"
#include "array.h"
#include "mymath.h"
#include "vector.h"

#include "_qsort.h"

namespace teggo
{

    template<class tReal>
    struct LineT
    {
        typedef Vect2T<tReal> fpoint_t;
        fpoint_t b,e;

        LineT()
        {
        }

        LineT(tReal x0,tReal y0,tReal x1,tReal y1)
            :b(x0,y0),e(x1,y1)
        {
        }

        LineT(fpoint_t const& b,fpoint_t const& e)
            :b(b),e(e)
        {
        }

        LineT(fpoint_t const& b,tReal r,tReal a)
            :b(b)
        {
            e = b + fpoint_t(r,0).Rotate(a);
        }

        bool FromString(pwide_t str)
        {
            return swscanf(str,L"%lf,%lf;%lf,%lf",&b.x,&b.y,&e.x,&e.y)==4;
        }

        bool Intersection(const LineT& l,fpoint_t& r) const
        {
            tReal x01,y01,a1,b1,t1;
            tReal x02,y02,a2,b2,t2;

            x01=b.x;
            y01=b.y;
            x02=l.b.x;
            y02=l.b.y;
            a1=e.x-b.x;
            b1=e.y-b.y;
            a2=l.e.x-l.b.x;
            b2=l.e.y-l.b.y;

            if (-a1*b2+a2*b1==0)
                return false;

            t1=(a2*(y02-y01)-b2*(x02-x01))/(a2*b1-a1*b2);

            if (a2!=0)
                t2=(a1*t1+x01-x02)/a2;
            else
                t2=(y01-y02+b1*t1)/b2;

            if (t1>1 || t1<0 || t2>1 || t2<0)
                return false;

            r.x=x01+a1*t1;
            r.y=y01+b1*t1;

            return true;
        }

        bool IsIntersecting(const LineT& l) const
        {
            tReal x01,y01,a1,b1,t1;
            tReal x02,y02,a2,b2,t2;

            x01=b.x;
            y01=b.y;
            x02=l.b.x;
            y02=l.b.y;
            a1=e.x-b.x;
            b1=e.y-b.y;
            a2=l.e.x-l.b.x;
            b2=l.e.y-l.b.y;

            if (-a1*b2+a2*b1==0)
                return false;

            t1=(a2*(y02-y01)-b2*(x02-x01))/(a2*b1-a1*b2);

            if (a2!=0)
                t2=(a1*t1+x01-x02)/a2;
            else
                t2=(y01-y02+b1*t1)/b2;

            if (t1>1 || t1<0 || t2>1 || t2<0)
                return false;

            return true;
        }

        tReal Distance( fpoint_t const& p ) const
        {
            fpoint_t r;
            return Distance(p,r);
        }

        tReal Distance( fpoint_t const& p, fpoint_t& res) const
        {
            if ( b.x == e.x )
            {
                if (IsBetween(p.y,b.y,e.y))
                {
                    res=fpoint_t(b.x,p.y);
                    return fabs(p.x-b.x);
                }

                tReal r1=(b-p).Length();
                tReal r2=(e-p).Length();

                if (r1<r2)
                {
                    res=b;
                    return r1;
                }
                else
                {
                    res=e;
                    return r2;
                }
            }
            if ( b.y==e.y )
            {

                if (IsBetween(p.x,b.x,e.x))
                {
                    res=fpoint_t(p.x,b.y);
                    return fabs(p.y-b.y);
                }

                tReal r1=(b-p).Length();
                tReal r2=(e-p).Length();

                if (r1<r2)
                {
                    res=b;
                    return r1;
                }
                else
                {
                    res=e;
                    return r2;
                }
            }

            tReal u=e.x-b.x;
            tReal v=e.y-b.y;
            tReal t=(p.x-b.x)/u-(p.y-b.y)/v;

            LineT l;
            l.b=p;

            if (t<0)
            {
                l.e=p+fpoint_t(e.y-b.y,b.x-e.x);
            }
            else
            {
                l.e=p+fpoint_t(b.y-e.y,e.x-b.x);
            }

            fpoint_t r;

            if (Intersection(l,r))
            {
                res=r;
                return (p-r).Length();
            }

            tReal r1=(b-p).Length();
            tReal r2=(e-p).Length();

            if (r1<r2)
            {
                res=b;
                return r1;
            }
            else
            {
                res=e;
                return r2;
            }
        }

        // result is y coordinate
        // x MUST be in between b.x and e.x
        bool VSlice(tReal x,tReal& outY)
        {
            if ( is_fequal(e.x,b.x) )
                return false;

            tReal t=(x-b.x)/(e.x-b.x);

            if ( t<=0 || t>=1 )
                return false;

            outY=b.y+(e.y-b.y)*t;
            return true;
        }

        bool HSlice(tReal y,tReal& outX)
        {
            if ( is_fequal(e.y,b.y) )
                return false;

            tReal t=(y-b.y)/(e.y-b.y);

            if ( t<=0 || t>=1 )
                return false;

            outX=b.x+(e.x-b.x)*t;
            return true;
        }
    };

    template <class tReal>
    struct TriangleT
    {
        typedef Vect2T<tReal> fpoint_t;
        typedef LineT<tReal>  line_t;

        fpoint_t p0,p1,p2;

        TriangleT()
        {
        }

        TriangleT(fpoint_t const& q0,fpoint_t const& q1,fpoint_t const& q2)
        {
            tReal a=q1.x*q2.y-q2.x*q1.y-q0.x*q2.y+q0.y*q2.x+q0.x*q1.y-q0.y*q1.x;
            if (a>0) p0=q0, p1=q1, p2=q2;
            else    p0=q0, p1=q2, p2=q1;
        }

        bool IsACW()
        {
            return (p1.x-p0.x)*(p2.y-p0.y)-(p1.y-p0.y)*(p2.x-p0.x)>0;
        }

        bool FromString(pwide_t str)
        {
            if (swscanf(str,L"%lf,%lf;%lf,%lf;%lf,%lf",&p0.x,&p0.y,&p1.x,&p1.y,&p2.x,&p2.y)!=6)
                return false;

            if (!IsACW())
            {
                fpoint_t t=p2; p2=p1; p1=t;
            }

            return true;
        }

        bool IsInside(fpoint_t const& p) const
        {
            tReal ax,ay,bx,by,cx,cy,apx,apy,bpx,bpy,cpx,cpy;
            tReal cap,bcp,abp;

            ax=p2.x-p1.x;
            ay=p2.y-p1.y;
            bx=p0.x-p2.x;
            by=p0.y-p2.y;
            cx=p1.x-p0.x;
            cy=p1.y-p0.y;
            apx=p.x-p0.x;
            apy=p.y-p0.y;
            bpx=p.x-p1.x;
            bpy=p.y-p1.y;
            cpx=p.x-p2.x;
            cpy=p.y-p2.y;

            abp = ax*bpy - ay*bpx;
            cap = cx*apy - cy*apx;
            bcp = bx*cpy - by*cpx;

            return ((abp >= 0.0L) && (bcp >= 0.0L) && (cap >= 0.0L));
        }

        bool IsIntersecting(const line_t& l)const
        {
            return IsInside(l.b) || IsInside(l.e) || line_t(p0,p1).IsIntersecting(l) ||
                   line_t(p1,p2).IsIntersecting(l) || line_t(p2,p0).IsIntersecting(l);
        }
    };

    template <class tReal>
    struct PolygonT
    {
        typedef Vect2T<tReal>     fpoint_t;
        typedef LineT<tReal>      line_t;
        typedef TriangleT<tReal>  triangle_t;

        PolygonT()
        {
        }

        explicit PolygonT(int n):points_(n)
        {
        }

        PolygonT(PolygonT const& src)
            :points_(src.points_)
        {
        }

        void AddPoint(fpoint_t const& p)
        {
            points_.Push(p);
        }

        void AddPoint(tReal x,tReal y)
        {
            points_.Push(fpoint_t(x,y));
        }

        void Clear()
        {
            points_.Clear();
        }

        bool IsInside(fpoint_t const& p) const
        {
            const tReal PI = F_PI; // alias
            tReal a=0;

            for ( int i=0; i<points_.Count(); ++i )
            {
                tReal b = ACWAngleDiff(
                              (points_[i]-p).GetAngle(),
                              (points_[(i+1)%points_.Count()]-p).GetAngle());

                if ( b > PI )
                    b-=2*PI;

                a+=b;
            }

            return !IsZero(a);
        }

        tReal CalcArea() const
        {
            return fabs(CalcAreaInternal());
        }

        bool IsACW() const
        {
            return CalcAreaInternal()>0;
        }

        struct EdgePoint
        {
            enum
            {
                deadLLBegin=1,
                deadLLEnd,
                deadLRBegin,
                deadLREnd,
                deadRRBegin,
                deadRREnd,
                deadRLBegin,
                deadRLEnd,
                deadSingleLeft,
                deadSingleRight
            };

            fpoint_t p;
            int   edge;
            int   used;
            int   vclass;
        };

        struct EPYSort
        {
            bool operator()(EdgePoint const& a,EdgePoint const& b) const
            {
                return a.p.y > b.p.y;
            }
        };

        struct EPXSort
        {
            bool operator()(EdgePoint const& a,EdgePoint const& b) const
            {
                return a.p.x > b.p.x;
            }
        };

        template <class Sorter, class CmpOp>
        void Slice(
            tReal fpoint_t::*A,
            tReal fpoint_t::*B,
            bool (line_t::*LineSlice)(tReal,tReal&),
            tReal v,
            BufferT<PolygonT>& top,
            BufferT<PolygonT>& bottom,
            Sorter _1,
            CmpOp _2) const
        {
            const int points_count = points_.Count();

            if ( points_count < 3 )
                return;

            BufferT<EdgePoint> edgPts(points_count);
            BufferT<int> edg2ptMap(points_count,-1);
            BufferT<int> dead(points_count,0);

            //последняя точка лежит на секущей прямой
            bool lastWasDead=false;

            //индекс первой мёртвой точки в серии
            int firstDead=-1;
            int from=0;

            while ( is_fequal(points_[from].*A,v) )
                if ( ++from == points_count )
                    return;

            if ( ++from == points_count )
                return;

            bool onLeft = CmpOp()(points_[from-1].*A,v);

            {
                int i=0,idx=from,prev=(idx-1+points_count)%points_count;

                for ( ; i<points_count; i++,prev=idx++ )
                {
                    if ( idx >= points_count )
                        idx-=points_count;

                    if ( is_fequal(points_[idx].*A,v) )
                    {
                        dead[idx]=true;

                        if ( !lastWasDead )
                            firstDead=idx;

                        lastWasDead=true;
                        continue;
                    }

                    if ( onLeft != CmpOp()(points_[idx].*A,v) )
                    {
                        if ( firstDead != -1 && firstDead != prev )
                        {
                            EdgePoint ep;
                            ep.p.*A=v;
                            ep.edge=firstDead;
                            ep.used=0;

                            if ( onLeft )
                                ep.vclass = EdgePoint::deadLRBegin;
                            else
                                ep.vclass = EdgePoint::deadRLBegin;

                            ep.p.*B=points_[firstDead].*B;
                            edgPts.Push(ep);

                            ep.edge=prev;
                            ep.used=0;

                            if (onLeft)
                                ep.vclass=EdgePoint::deadLREnd;
                            else
                                ep.vclass=EdgePoint::deadRLEnd;

                            ep.p.*B=points_[prev].*B;
                            edgPts.Push(ep);

                            firstDead=-1;
                        }
                        else
                        {

                            EdgePoint ep;
                            ep.p.*A=v;
                            ep.edge=prev;
                            ep.vclass=0;
                            ep.used=0;

                            if (lastWasDead)
                                ep.p.*B=points_[prev].*B;
                            else
                                (line_t(points_[prev],points_[idx]).*LineSlice)(v,ep.p.*B);

                            edgPts.Push(ep);
                        }

                        onLeft = CmpOp()(points_[idx].*A,v);
                        lastWasDead = false;
                    }

                    if (firstDead!=-1 && firstDead!=prev)
                    {
                        if ( onLeft )
                        {
                            if ( points_[firstDead].*B>points_[idx].*B )
                            {
                                EdgePoint ep;
                                ep.p.*A=v;
                                ep.edge=firstDead;
                                ep.vclass=EdgePoint::deadLLBegin;
                                ep.used=0;
                                ep.p.*B=points_[firstDead].*B;
                                edgPts.Push(ep);
                                ep.vclass=EdgePoint::deadLLEnd;
                                ep.edge=prev;
                                ep.p.*B=points_[idx].*B;
                                edgPts.Push(ep);
                            }
                            else
                            {
                                dead[firstDead]=false;
                                dead[prev]=false;
                            }
                        }
                        else
                        {
                            if ( points_[firstDead].*B<points_[idx].*B )
                            {
                                EdgePoint ep;
                                ep.p.*A=v;
                                ep.edge=firstDead;
                                ep.vclass=EdgePoint::deadRRBegin;
                                ep.used=0;
                                ep.p.*B=points_[firstDead].*B;
                                edgPts.Push(ep);
                                ep.vclass=EdgePoint::deadRREnd;
                                ep.edge=prev;
                                ep.p.*B=points_[idx].*B;
                                edgPts.Push(ep);
                            }
                            else
                            {
                                dead[firstDead]=false;
                                dead[prev]=false;
                            }
                        }
                        lastWasDead=false;
                    }

                    if ( lastWasDead )
                    {
                        EdgePoint ep;
                        ep.p.*A=v;
                        ep.edge=prev;
                        ep.vclass=onLeft?EdgePoint::deadSingleLeft:EdgePoint::deadSingleRight;
                        ep.used=0;
                        ep.p.*B=points_[prev].*B;
                        edgPts.Push(ep);
                    }
                    firstDead=-1;
                    lastWasDead=false;
                }
            }

            int ecnt=0;

            {
                for ( unsigned i=0; i < edgPts.Count(); ++i)
                {
                    if (edgPts[i].vclass!=EdgePoint::deadSingleLeft &&
                        edgPts[i].vclass!=EdgePoint::deadSingleRight)
                    {
                        ecnt++;
                        break;
                    }
                }
            }

            if ( ecnt == 0 )
            {
                (onLeft?top:bottom).Push(*this);
                return;
            }

            Qsort(edgPts.Begin(),edgPts.End(),Sorter());

            PolygonT t;
            int ep;

            ecnt=0;


            {
                for ( unsigned i=0; i<edgPts.Count(); ++i )
                {
                    if (edgPts[i].vclass==EdgePoint::deadSingleLeft ||
                        edgPts[i].vclass==EdgePoint::deadSingleRight)
                    {
                        if ( ecnt & 1 )
                        {
                            EdgePoint p=edgPts[i];

                            if ( edgPts[i].vclass == EdgePoint::deadSingleLeft )
                            {
                                edgPts[i].vclass=EdgePoint::deadLLBegin;
                                p.vclass=EdgePoint::deadLLEnd;
                            }
                            else
                            {
                                edgPts[i].vclass=EdgePoint::deadRRBegin;
                                p.vclass=EdgePoint::deadRREnd;
                            }

                            edgPts.Insert(edgPts.Begin()+i+1,p);
                            i--;
                            continue;
                        }
                        else
                        {
                            dead[edgPts[i].edge]=false;
                            edgPts.Erase(edgPts.Begin()+i);
                            i--;
                            continue;
                        }
                    }

                    if (edgPts[i].vclass==EdgePoint::deadLREnd ||
                        edgPts[i].vclass==EdgePoint::deadRLBegin ||
                        edgPts[i].vclass==EdgePoint::deadRRBegin ||
                        edgPts[i].vclass==EdgePoint::deadRREnd
                       )
                        edgPts[i].used++;
                    else
                        ecnt++;
                }
            }

            {
                for (unsigned i=0; i<edgPts.Count(); i++)
                    edg2ptMap[edgPts[i].edge] = i;
            }

            for (;;)
            {
                ep=-1;

                for ( unsigned i=0; i<edgPts.Count(); i++ )
                {
                    if ( edgPts[i].used == 0 )
                    {
                        ep=i;
                        break;
                    }
                }

                if ( ep == -1 )
                    break;

                t.Clear();

                for (;;)
                {
                    if ( ep<0 || edgPts[ep].used )
                        break;

                    t.AddPoint(edgPts[ep].p);
                    edgPts[ep].used++;

                    int idx = edgPts[ep].edge+1;
                    if ( idx >= points_count )
                        idx=0;

                    while ( edg2ptMap[idx] == -1 )
                    {
                        if ( !dead[idx] )
                            t.AddPoint(points_[idx]);
                        idx++;
                        if ( idx >= points_count )
                            idx=0;
                    }

                    if ( !dead[idx] )
                        t.AddPoint(points_[idx]);

                    ep = edg2ptMap[idx];

                    if ( ep>0 && edgPts[ep].used && edgPts[ep].edge == edgPts[ep-1].edge )
                    {
                        // little hack
                        ep--;
                    }

                    if ( edgPts[ep].used == 0 )
                    {
                        t.AddPoint(edgPts[ep].p);
                        edgPts[ep].used++;
                    }

                    while ( ep>=0 && edgPts[ep].used )
                        ep--;
                }
                //top.Push(t);
                top.Push(PolygonT());
                top[-1].points_.Swap(t.points_);
            }

            {
                for ( unsigned i=0; i<edgPts.Count(); i++ )
                {
                    if (edgPts[i].vclass==EdgePoint::deadRLEnd ||
                        edgPts[i].vclass==EdgePoint::deadLRBegin ||
                        edgPts[i].vclass==EdgePoint::deadLLBegin ||
                        edgPts[i].vclass==EdgePoint::deadLLEnd
                       )
                        edgPts[i].used++;
                }
            }

            for (;;)
            {
                ep=-1;

                for ( int i = edgPts.Count()-1 ; i >= 0 ; i-- )
                {
                    if ( edgPts[i].used == 1 )
                    {
                        ep=i;
                        break;
                    }
                }

                if ( ep == -1 )
                    break;

                t.Clear();

                for (;;)
                {
                    if ( unsigned(ep) >= edgPts.Count() || edgPts[ep].used == 2 )
                        break;

                    t.AddPoint(edgPts[ep].p);
                    edgPts[ep].used++;
                    int idx=edgPts[ep].edge+1;

                    if ( idx >= points_count )
                        idx=0;

                    while ( edg2ptMap[idx] == -1 )
                    {
                        if ( !dead[idx] )
                            t.AddPoint(points_[idx]);

                        idx++;
                        if ( idx >= points_count )
                            idx=0;
                    }

                    if ( !dead[idx] )
                        t.AddPoint(points_[idx]);

                    ep=edg2ptMap[idx];

                    if ( ep < edgPts.Count()-1 && edgPts[ep].used == 2
                         && edgPts[ep].edge == edgPts[ep+1].edge )
                    {
                        ep++;
                    }

                    if ( edgPts[ep].used == 1 )
                    {
                        t.AddPoint(edgPts[ep].p);
                        edgPts[ep].used++;
                    }

                    while ( ep < edgPts.Count() && edgPts[ep].used == 2 )
                        ep++;
                }
                //bottom.Push(t);
                bottom.Push(PolygonT());
                bottom[-1].points_.Swap(t.points_);
            }

        }

        struct HCmpOp
        {
            bool operator()(tReal const& a,tReal const& v) const
            {
                return a >= v;
            }
        };

        struct VCmpOp
        {
            bool operator()(tReal const& a,tReal const& v) const
            {
                return a <= v;
            }
        };

        void HSlice(tReal yv, BufferT<PolygonT>& top,BufferT<PolygonT>& bottom) const
        {
            Slice(&fpoint_t::y,&fpoint_t::x,&line_t::HSlice,yv,top,bottom,EPXSort(),HCmpOp());
        }

        void VSlice(tReal xv, BufferT<PolygonT>& left, BufferT<PolygonT>& right) const
        {
            Slice(&fpoint_t::x,&fpoint_t::y,&line_t::VSlice,xv,left,right,EPYSort(),VCmpOp());
        }

        bool Triangulate( BufferT<triangle_t>& result )
        {
            int n = points_.Count();
            if ( n < 3 ) return false;

            for ( int q=n-1; q>=0; q-- )
            {
                result.Resize(0);
                BufferT<int> V(n);

                {for ( int v=0; v < n; v++ ) V[v] = v; }

                int nv = n;

                /*  remove nv-2 Vertices, creating 1 triangle every time */
                int count = nv+1;   /* error detection */

                {
                    for ( int v=q; nv>2; )
                    {
                        /* if we loop, it is probably a non-simple polygon */
                        if ( 0 >= (count--) )
                        {
                            //** Triangulate: ERROR - probable bad polygon!
                            break;
                        }

                        /* three consecutive vertices in current polygon, <u,v,w> */
                        int u = v  ;
                        if (nv <= u) u = 0;     /* previous */
                        v = u+1;
                        if (nv <= v) v = 0;     /* new v    */
                        int w = v+1;
                        if (nv <= w) w = 0;     /* next     */

                        if ( Snip(u,v,w,nv,V) )
                        {
                            result.Push(triangle_t(points_[V[u]],points_[V[v]],points_[V[w]]));

                            /* remove v from remaining polygon */

                            V.Erase(V.Begin()+v);
                            nv--;
                            v-=(n-q);
                            while ( v < 0 ) v += nv;
                            /* resest error detection counter */
                            count = nv+1;
                        }
                    }
                }

                if ( nv < 3 ) return true;
            }
            return false;
        }

        bool Snip(int u,int v,int w,int n,const BufferT<int>& V)
        {
            int p;
            triangle_t t;

            t.p0=points_[V[u]];
            t.p1=points_[V[v]];
            t.p2=points_[V[w]];

            if ( !t.IsACW() ) return false;

            for (p=0; p<n; p++)
            {
                if ( (p == u) || (p == v) || (p == w) )
                    continue;
                if ( t.IsInside(points_[V[p]]) )
                    return false;
            }

            return true;
        }

        tReal CalcAreaInternal() const
        {
            int n = points_.Count();
            if ( n < 3 ) return 0;

            tReal area=0.0L;

            for ( int i=n-1,j=0; j<n; i=j++ )
            {
                area+=points_[i].x*points_[j].y-points_[j].x*points_[i].y;
            }

            return area*0.5;
        }

        void Swap(PolygonT& poly)
        {
            points_.Swap(poly.points_);
        }

        unsigned Count() const
        {
            return points_.Count();
        }

        BufferT<fpoint_t> points_;
    };

} // namespace

#endif
