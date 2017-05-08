
/*

  Copyright (c) 2005-2009, Alexey Sudachen, alexey@sudachen.name

*/

#if !defined __ED377106_626B_434d_8DDD_F6089E43BA37__
#define __ED377106_626B_434d_8DDD_F6089E43BA37__

#include "_specific.h"

namespace teggo
{

    static const float  F_PI  = float(3.14159265358979323846264338327950288419716939937510582);
    static const double LF_PI = double(3.14159265358979323846264338327950288419716939937510582);
    static const float  F_EPSILON = float(1e-6);
    static const double LF_EPSILON = double(1e-6);

    template<class real> inline
    bool CXX_STDCALL in_epsilon(real val) { return fabs(val) < real(1e-6); }
    template<class real> inline
    bool CXX_STDCALL is_fequal(real a,real b) { return fabs(a-b) < real(1e-6); }

    template <class real>
    int CXX_STDCALL SolveQuadratic(real A,real B,real C,real (&solution)[2])
    {
        real D = B*B - 4*A*C;

        if ( D <  0 )
            return 0;

        if ( D == 0 )
            return solution[0] = -B/(2*A), 1;
        else
        {
            real sqrtD = sqrt(D);
            solution[0] = (-B + sqrtD)/(2*A);
            solution[1] = (-B - sqrtD)/(2*A);
            return 2;
        }
    }

} // namespace

#endif // __ED377106_626B_434d_8DDD_F6089E43BA37__
