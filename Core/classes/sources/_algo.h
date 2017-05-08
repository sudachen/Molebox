
/*

  Copyright (c) 2005-2009, Alexey Sudachen, alexey@sudachen.name

*/

namespace teggo
{


    /*

       Рандомизаторы

    */

    typedef u32_t randizer_t;

    inline u32_t CXX_STDCALL unsigned_random( randizer_t* store )
    {
        *store = 1664525U * *store + 1013904223U;
        return *store;
    }

    inline u32_t CXX_STDCALL modulate_random( randizer_t* store, int mod )
    {
        return unsigned_random(store) % mod;
    }

    inline float CXX_STDCALL float_random( randizer_t* store )
    {
        u32_t f = 0x3f800000U|(0x007fffffU&unsigned_random(store));
        return *(float*)&f - 1.f;
    }

    template <class tIx> inline
    void CXX_STDCALL random_shuffle( tIx iS,tIx iE, randizer_t* store )
    {
        if (iS == iE) return;
        for (tIx i = iS + 1; i != iE; ++i)
            teggo_swap(*i, *(iS + modulate_random(store,(i - iS) + 1)));
    }

    /*

       Компораторы

    */

    struct Less
    {
        template <class tTx, class tTy>
        bool operator() ( tTx const& a, tTy const& b ) const { return a < b; }
    };

    struct Greater
    {
        template <class tTx, class tTy>
        bool operator() ( tTx const& a, tTy const& b ) const { return a > b; }
    };

    struct Equal
    {
        template <class tTx, class tTy>
        bool operator() ( tTx const& a, tTy const& b ) const { return a == b; }
    };

    template <class tTcmp> struct Not
    {
        tTcmp Comparer;
        Not(tTcmp cmp = tTcmp()) : Comparer(cmp) {}

        template <class tTx, class tTy>
        bool operator() ( tTx const& a, tTy const& b ) const { return !Comparer(a,b); }
        template <class tTx>
        bool operator() ( tTx const& a ) const { return !Comparer(a); }
    };

    /*

      Бинарный поиск

    */

    template <class tIx, class tTx, class tTcmp> CXX_NO_INLINE
    tIx CXX_STDCALL binary_lower(tIx iS, tIx iE, tTx const& val, tTcmp cmp)
    {
        u32_t len = iE - iS;
        u32_t half;
        tIx middle;

        while (len > 0)
        {
            half = len >> 1;
            middle = iS + half;
            if ( cmp(*middle,val) )
            {
                iS = middle;
                ++iS;
                len = len - half - 1;
            }
            else
                len = half;
        }
        return iS;
    }

    template <class tIx, class tTx> inline
    tIx CXX_STDCALL binary_lower(tIx iS, tIx iE, tTx const& val)
    { return binary_lower(iS,iE,val,Less()); }

    template <class tIx, class tTx, class tTcmp> CXX_NO_INLINE
    tIx CXX_STDCALL binary_upper(tIx iS, tIx iE, tTx const& val, tTcmp cmp)
    {
        u32_t len = iE - iS;
        u32_t half;
        tIx middle;

        while (len > 0)
        {
            half = len >> 1;
            middle = iS + half;
            if ( cmp(val,*middle))
                len = half;
            else
            {
                iS = middle;
                ++iS;
                len = len - half - 1;
            }
        }
        return iS;
    }

    template <class tIx, class tTx> inline
    tIx CXX_STDCALL binary_upper(tIx iS, tIx iE, tTx const& val)
    { return binary_upper(iS,iE,val,Less()); }

    template <class tIx, class tTx, class tTcmp>
    tIx CXX_STDCALL binary_find(tIx iS, tIx iE, tTx const& val, tTcmp cmp)
    {
        iS = binary_lower(iS,iE,val,cmp);
        return ( iS != iE  &&  !cmp(val,*iS) ) ? iS : 0;
    }

    template <class tIx, class tTx> inline
    tIx CXX_STDCALL binary_find(tIx iS, tIx iE, tTx const& val)
    { return binary_find(iS,iE,val,Less()); }


    /*

      Линейный поиск

    */


    template <class tIx, class tTx, class tTcmp> inline
    tIx CXX_STDCALL linear_find(tIx iS, tIx iE, tTx const& val, tTcmp cmp)
    {
        while (iS != iE && !cmp(*iS,val)) ++iS;
        return iS;
    }

    template <class tIx, class tTx> inline
    tIx CXX_STDCALL linear_find(tIx iS, tIx iE, tTx const& val)
    { return linear_find(iS,iE,val,Equal()); }

    template <class tIx, class tTpred> inline
    tIx CXX_STDCALL linear_find_if(tIx iS, tIx iE, tTpred pred)
    {
        while (iS != iE && !pred(*iS)) ++iS;
        return iS;
    }

    template <class tIx, class tIy> CXX_NO_INLINE
    tIx CXX_STDCALL search( tIx iS, tIx iE, tIy pS, tIy pE )
    {
        if (iS == iE || pS == pE) return iS;

        tIy tmp(pS); ++tmp;
        if (tmp == pE)
            return linear_find(iS, iE, *pS);

        tIy p1 = pS, p;
        tIx current = iS;

        ++p1;

        while (iS != iE)
        {
            iS = linear_find(iS, iE, *pS);
            if (iS == iE)
                return iE;

            p = p1;
            current = iS;
            if (++current == iE)
                return iE;

            while (*current == *p)
            {
                if (++p == pE)
                    return iS;
                if (++current == iE)
                    return iE;
            }

            ++iS;
        }

        return iS;
    }

    /*

      Трансформации

    */

    template <class tIx, class tOx, class tTx, class tTcmp> inline
    tOx CXX_STDCALL remove__(tIx iS, tIx iE, tOx iO, tTx const& val, tTcmp cmp)
    {
        for ( ; iS != iE; ++iS)
            if (!cmp(*iS,val))
                *iO++ = *iS;
        return iO;
    }

    template <class tIx, class tTx, class tTcmp> inline
    tIx CXX_STDCALL remove(tIx iS, tIx iE, tTx const& val, tTcmp cmp)
    {
        iS = linear_find(iS, iE, val,cmp);
        tIx i = iS;
        return i == iE ? iS : remove__(++i, iE, iS, val,cmp);
    }

    template <class tIx, class tTx> inline
    tIx CXX_STDCALL remove(tIx iS, tIx iE, tTx const& val)
    { return remove(iS,iE,val,Equal()); }


    template <class tIx, class tTx, class tTcmp> inline
    void CXX_STDCALL replace(tIx iS, tIx iE, tTx const& val, tTx const& nval, tTcmp cmp )
    {
        for ( ; iS != iE; ++iS)
            if ( cmp(*iS,val) )
                *iS = nval;
    }

    template <class tIx, class tTx> inline
    void CXX_STDCALL replace(tIx iS, tIx iE, tTx const& val, tTx const& nval)
    { replace(iS,iE,val,nval,Equal()); }

    template <class tIx, class tOx, class tTop> inline
    tOx CXX_STDCALL transform(tIx iS, tIx iE, tOx iO, tTop op)
    {
        for ( ; iS != iE; ++iS, ++iO) *iO = op(*iS);
        return iO;
    }

    template <class tIx, class tOx> inline
    tOx CXX_STDCALL copy(tIx iS, tIx iE, tOx iO)
    {
        for ( ; iS != iE; ++iS, ++iO ) *iO = *iS;
        return iO;
    }

    template <class tIx, class tOx> inline
    tOx CXX_STDCALL copy_n(tIx iS, tOx iO, unsigned count)
    {
        for ( tIx iE = iS + count; iS != iE; ++iS, ++iO ) *iO = *iS;
        return iO;
    }

    template <class tIx, class tTx> inline
    tIx CXX_STDCALL fill(tIx iS, tIx iE, tTx v)
    {
        for ( ; iS != iE; ++iS ) *iS = v;
        return iS;
    }

    /*

      Пребор элементов

    */

    template <class tIx, class tTx, class tTcmp> inline
    u32_t CXX_STDCALL enumerate( tIx iS, tIx iE, tTx const& val, tTcmp cmp )
    {
        u32_t n = 0;
        for ( ; iS != iE; ++iS )
            if ( cmp(*iS,val) )
                ++n;
        return n;
    }

    template <class tIx, class tTx> inline
    u32_t CXX_STDCALL enumerate( tIx iS, tIx iE, tTx const& val )
    { return enumerate(iS,iE,val,Equal()); }

    template <class tIx, class tTpred> inline
    u32_t CXX_STDCALL enumerate_if( tIx iS, tIx iE, tTpred pred )
    {
        u32_t n = 0;
        for ( ; iS != iE; ++iS )
            if ( pred(*iS) )
                ++n;
        return n;
    }

    template <class tTx> inline
    int CXX_STDCALL count_bits( tTx t )
    {
        int cnt = 0;
        while ( t )
        {
            ++cnt;
            t &= t-1;
        }
        return cnt;
    }

    template <class tTx> inline
    tTx CXX_STDCALL align2_to( tTx t, tTx mod )
    {
        return (t+mod-1) & ~(mod-1);
    }

} // namespace
