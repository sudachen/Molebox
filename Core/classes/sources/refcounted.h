
/*

  Copyright (c) 2005-2009, Alexey Sudachen, alexey@sudachen.name

*/

#if !defined __50DFA54F_0876_4F8C_9226_0BD2D899B7D0__
#define __50DFA54F_0876_4F8C_9226_0BD2D899B7D0__

#include "_specific.h"
#include "guid.h"

namespace teggo
{

  struct CXX_NO_VTABLE Ireferred
    {
      virtual u32_t Release() = 0;
      virtual u32_t AddRef() = 0;
      virtual void *QueryInterface(guid_t const &guid) { return 0; }
      virtual ~Ireferred() {}
      //_TEGGO_EXPORTABLE virtual void Finalize();
//       template <class tTx>
//         tTx *Cast(Ireferred *p, tTx *_=0)
//           { return (tTx*)QueryInterface(TEGGO_GUIDOF(tTx)); }
    };
    
  template < class tTbase >
    struct RefcountedT : tTbase
      {
        u32_t Release()
          {
            if ( long refcount = InterlockedDecrement(&refcount_) )
              return refcount;
            else
              return (delete this), 0;
          }

        u32_t AddRef()
          {
            return InterlockedIncrement(&refcount_);
          }

        RefcountedT() : refcount_(1) {}
        u32_t _Refcount() { return refcount_; }

      protected:
        virtual ~RefcountedT() {}

      private:
        RefcountedT( const RefcountedT& );
        RefcountedT& operator =( const RefcountedT& );
        long refcount_;
      };

  typedef RefcountedT<Ireferred> Refcounted;

} // namespace

#endif // __50DFA54F_0876_4F8C_9226_0BD2D899B7D0__
