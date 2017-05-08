
/*

  Copyright (c) 2005-2009, Alexey Sudachen, alexey@sudachen.name

*/

#if !defined __4813E86F_F133_4B7B_9D29_D90F81504277__
#define __4813E86F_F133_4B7B_9D29_D90F81504277__

namespace teggo
{
    enum QSORT_CONSTANTS { QSORT_THRESH  = 5  };

    template <class tIx, class tLess>
    void CXX_STDCALL Isort(tIx iS, tIx iE, tLess less)
    {
        STRICT_REQUIRE( iE >= iS );
        if (iE == iS) return;

        tIx minval = iS;
        for ( tIx i = iS; ++i != iE; )
        {
            for ( tIx j = i; j != iE; ++j )
                if ( less( *j, *minval) )
                    minval = j;
            if ( minval != i-1 )
                teggo_swap(*minval,*(i-1));
            minval = i;
        }

        {
            for ( tIx i = iS+1; i != iE; ++i )
                REQUIRE ( !less(*i,*(i-1)) );
        }
    }

    template <class tIx, class tLess>
    void CXX_STDCALL Qst_(tIx iS, tIx iE, tLess less)
    {
        STRICT_REQUIRE( iE > iS );

        tIx mid = iS+(iE-iS)/2;
        STRICT_REQUIRE( mid >= iS && mid < iE );

        tIx i = iS;
        tIx j = iE-1;

        for (;;)
        {
            while ( i < mid && !less(*mid,*i) ) ++i;
            while ( j > mid && !less(*j,*mid) ) --j;
            if ( i == mid || j == mid )
            {
                if ( j-i > 1 )
                {
                    tIx k = i + (j-i)/2;
                    teggo_swap(*k,*mid);
                    mid = k;
                }
                else
                {
                    if ( i != j )
                    {
                        tIx ii = ( i == mid ) ? j : i;
                        if ( ii < mid && less(*mid,*ii) || mid < ii && less(*ii,*mid) )
                        {
                            teggo_swap(*mid,*ii);
                            teggo_swap(mid,ii);
                        }
                    }
                    break;
                }
            }
            else
                teggo_swap(*j--,*i++);
        }

        {
            for ( tIx i = iS; i != mid; ++i )
                REQUIRE ( !less(*mid,*i));
        }
        {
            for ( tIx i = mid+1; i != iE; ++i )
                REQUIRE ( !less(*i,*mid));
        }

        if ( mid - iS > QSORT_THRESH )
            Qst_(iS,mid,less);
        else
            Isort(iS,mid,less);

        if ( iE - mid > QSORT_THRESH )
            Qst_(mid,iE,less);
        else
            Isort(mid,iE,less);

        {
            for ( tIx i = iS+1; i != iE; ++i )
                REQUIRE ( !less(*i,*(i-1)) );
        }
    }

    template <class tIx, class tLess>
    void CXX_STDCALL Qsort(tIx iS, tIx iE, tLess less)
    {
        STRICT_REQUIRE( iE >= iS );
        if ( iE - iS >= QSORT_THRESH )
            Qst_(iS, iE, less);
        else
            Isort(iS,iE,less);
    }

    template <class tIx>
    void CXX_STDCALL Qsort(tIx iS, tIx iE )
    {
        Qsort(iS,iE, Less());
    }
}

#endif //__4813E86F_F133_4B7B_9D29_D90F81504277__
