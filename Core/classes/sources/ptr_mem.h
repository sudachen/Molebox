
/*

  Copyright (c) 2005-2009, Alexey Sudachen, alexey@sudachen.name

*/

#if !defined __612F9733_B06D_4e78_A2CE_F50A7B1EC0F5__
#define __612F9733_B06D_4e78_A2CE_F50A7B1EC0F5__

#include "_specific.h"

namespace teggo
{
  template < class tTx, class tMem >
    struct Auto_Ptr
      {
        mutable tTx *ptr_;

        tTx* forget_() const
          {
            tTx *tmp = ptr_;
            ptr_=0;
            return tmp;
          }

        CXX_NO_INLINE
        void reset_(tTx *p)
          {
            if ( ptr_==p ) return;
            if (ptr_) tMem::Delete(ptr_);
            ptr_ = p;
          }

        explicit Auto_Ptr(tTx *p=0)
          : ptr_(p)
          {
          }

        Auto_Ptr(const Auto_Ptr& p)
          {
            ptr_ = p.forget_();
          }

        ~Auto_Ptr()
          {
            if (ptr_)
               tMem::Delete(ptr_);
          }

        tTx* operator->() const
          {
            return ptr_;
          }

        tTx* operator+(int i) const
          {
            return ptr_+i;
          }

        tTx& operator*() const
          {
            return *ptr_;
          }

        tTx& operator[](int i) const
          {
            return ptr_[i];
          }

        bool operator!() const
          {
            return !ptr_;
          }

        operator bool() const
          {
            return !!ptr_;
          }

        tTx*& operator +() const
          {
            return ptr_;
          }

        // for compatibility
        tTx*& operator~() const
          {
            return operator+();
          }

        const Auto_Ptr& operator = (const Auto_Ptr &p)
          {
            reset_(p.forget_());
            return *this;
          }

      private:
        const Auto_Ptr& operator = (tTx *);
      };

  template < class tTx, class P > inline void CXX_STDCALL Unrefe(Auto_Ptr<tTx,P> &p) { p.reset_(0); }
  template < class tTx, class P > inline tTx* CXX_STDCALL Forget(Auto_Ptr<tTx,P> &p) { return p.forget_(); }
  template < class tTx, class P > inline void CXX_STDCALL Reset(Auto_Ptr<tTx,P> &p,tTx *pp) { p.reset_(pp); }

  struct Delete_One
    {
      template <class tTx> static void CXX_STDCALL Delete(tTx *p) { delete p; }
      template <class tTx> void operator ()(tTx *&p) const { delete p; p = 0; }
    };

  template <class tTx>
    struct mem_ptr : Auto_Ptr<tTx,Delete_One>
      {
        mem_ptr(tTx *p = 0) : Auto_Ptr<tTx,Delete_One> (p) {}
      };

  template <class tTx> inline
    mem_ptr<tTx> CXX_STDCALL MemPtr(tTx *p)
      {
        return mem_ptr<tTx>(p);
      }

  struct Delete_Array
    {
      template <class tTx> static void CXX_STDCALL Delete(tTx *p) { if ( p != 0 ) delete[] p; }
      template <class tTx> void operator ()(tTx *&p) const { Delete(p); p = 0; }
    };

  template <class tTx>
    struct mem_arr : Auto_Ptr<tTx,Delete_Array>
      {
        mem_arr(tTx *p = 0) : Auto_Ptr<tTx,Delete_Array> (p) {}
      };

  template <class tTx> inline
    mem_arr<tTx> CXX_STDCALL MemArr(tTx *p)
      {
        return mem_arr<tTx>(p);
      }

  template <class tTx> inline
    void CXX_STDCALL safe_delete( tTx *&p )
      {
        if ( p )
          {
            delete p;
            p = 0;
          }
      }

} // namespace

#endif // __612F9733_B06D_4e78_A2CE_F50A7B1EC0F5__
