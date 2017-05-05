
namespace teggo {
  
  struct fuslog
    {
      enum { FUS_UNKNOWN = 0, FUS_TRUE, FUS_FALSE };
      byte_t val_;
      fuslog() : val_(FUS_UNKNOWN) {}
      fuslog(fuslog const &x) : val_(x.val_) {}
      fuslog(bool b) {}
      bool False() { return val_ == FUS_FALSE; }
      bool True() { return val_ == FUS_TRUE; }
      void operator |= (bool b) { if ( b ) val_ = FUS_TRUE; }
      void operator &= (bool b) { if ( !b ) val_ = FUS_FALSE; }
    };

  // if ( false || fusval ) -> false if fusval is not true
  // if ( true  || fusval ) -> true
  // if ( false && fusval ) -> false 
  // if ( true  && fusval ) -> true if fusval is not false

  inline bool operator || ( bool b, fuslog f ) { if ( !b && f.val_ != fuslog::FUS_TRUE ) return false; return true; }  
  inline bool operator && ( bool b, fuslog f ) { if ( b && f.val_ != fuslog::FUS_FALSE ) return true; return false; }  
  inline bool operator || ( fuslog f, bool b ) { return b || f; }
  inline bool operator && ( fuslog f, bool b ) { return b && f; }

} // namespace
