
/*template < class T >
  struct SinchronizedT
    {
      T t_;
      struct U
        {
          T *t_;
          Tlocker &l_;
          T *operator->() { return t_; }
          U(T *t, Tlocker &l) : t_(t), l_(l) { l_.Lock(); }
          ~U() { l_.Unlock(); }
        };
      Tlocker l_;
      SinchronizedT() {};
      U operator ->() { return U(&t_,l_); }
    };*/


