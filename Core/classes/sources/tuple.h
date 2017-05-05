
/*

  Copyright (c) 2005-2009, Alexey Sudachen, alexey@sudachen.name

*/

#ifndef ___7ddf9ffd_bc93_44ea_99c1_ac70e15bdf58___
#define ___7ddf9ffd_bc93_44ea_99c1_ac70e15bdf58___

#include "_specific.h"

namespace teggo {
  template < class tA, class tB >
    struct Tuple2
      {
        tA _0;
        tB _1;
        Tuple2( tA a, tB b ) : _0(a), _1(b) {}
        Tuple2() {}
      };

  template < class tA, class tB >
    Tuple2<tA,tB> tuple2(tA a,tB b) { return Tuple2<tA,tB>(a,b); }

  template < class tA, class tB, class tC >
    struct Tuple3
      {
        tA _0;
        tB _1;
        tC _2;
        Tuple3( tA a, tB b, tC c ) : _0(a), _1(b), _2(c) {}
        Tuple3() {}
      };

  template < class tA, class tB, class tC >
    Tuple3<tA,tB,tC> tuple3(tA a,tB b,tC c) { return Tuple3<tA,tB,tC>(a,b,c); }

  template < class tA, class tB, class tC, class tD >
    struct Tuple4
      {
        tA _0;
        tB _1;
        tC _2;
        tD _3;
        Tuple4( tA a, tB b, tC c, tD d ) : _0(a), _1(b), _2(c), _3(d) {}
        Tuple4() {}
      };

  template < class tA, class tB, class tC, class tD >
    Tuple4<tA,tB,tC,tD> tuple4(tA a,tB b,tC c,tD d) { return Tuple4<tA,tB,tC,tD>(a,b,c,d); }

} // namespace teggo

#endif  // ___7ddf9ffd_bc93_44ea_99c1_ac70e15bdf58___
