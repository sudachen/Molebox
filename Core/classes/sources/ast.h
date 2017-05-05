
/*

  Copyright (c) 2005-2009, Alexey Sudachen, alexey@sudachen.name

*/

#if !defined __C26B3732_CD91_4E1E_9D39_D1590F635188__
#define __C26B3732_CD91_4E1E_9D39_D1590F635188__

#include "_specific.h"
#include "_crc32.h"

namespace teggo
{
  template < class tTa, unsigned tHtsize = 101, unsigned tPoolpage = 1024 >
    struct AST_TEMPLATE
      {

        struct Node
          {
            tTa _;
            const unsigned chlcnt;
            Node( unsigned n ) : chlcnt(n)
              {
                for ( int i = 0; i < n; ++i ) children_[i] = 0;
                memset(&_,0,sizeof(tTa));
              }
            Node*& Child(unsigned i)
              {
                REQUIRE ( i < chlcnt ) ;
                return children_[i];
              }
          private:
            Node *children_[1];
          };

        Node* Alloc(unsigned chlcnt)
          {
            ++nod_count_;
            if ( chlcnt > 0 )
              return new( Sbrk(sizeof(Node)+(chlcnt-1)*sizeof(Node*)) ) Node(chlcnt);
            else
              return new( Sbrk(sizeof(Node)-sizeof(Node*)) ) Node(0);
          }

        struct LitRec
          {
            unsigned hashcode;
            unsigned length;
            LitRec*  next;
            pwide_t  literal;
          };

        CXX_NO_INLINE
        pwide_t InsertLiteral(pwide_t pS, unsigned ln,unsigned hashcode,bool is_static)
          {
            LitRec* r = (LitRec*)Sbrk(
              sizeof(LitRec) + (is_static ? 0 : ((ln+1)*sizeof(wchar_t))) );

            if ( is_static )
              r->literal = pS;
            else
              {
                r->literal = (pwide_t)((byte_t*)r+sizeof(LitRec));
                memcpy( (void*)r->literal, pS, (ln+1)*sizeof(wchar_t) );
                ((wchar_t*)r->literal)[ln] = 0;
              }
            r->hashcode = hashcode;
            r->length = ln;
            LitRec** rr = &htable_[hashcode % tHtsize];
            r->next = *rr;
            *rr = r;
            ++lit_count_;
            return r->literal;
          }

        pwide_t StaticLiteral( pwide_t l )
          {
            unsigned hashcode = 0;
            unsigned ln = wcslen(l);
            if ( LitRec* lit = FindLiteral_( l, l+ln, &hashcode ) )
              return 0;
            else
              return InsertLiteral( l, ln, hashcode, true );
          };

        pwide_t Literal( pchar_t pS, pchar_t pE )
          {
            StringT<wchar_t,128> temp(pS,pE);
            return Literal( temp.Str(), temp.Str()+temp.Length() );
          }

        pwide_t LiteralI( pchar_t pS, pchar_t pE )
          {
            StringT<wchar_t,128> temp(pS,pE);
            temp.ToLower();
            return Literal( temp.Str(), temp.Str()+temp.Length() );
          }

        template <class T>
          pwide_t Literal( BaseStringT<T> const& t)
            {
              return Literal( t.Str(), t.Str()+t.Length() );
            }

        pwide_t Literal( pwide_t pS, pwide_t pE )
          {
            unsigned hashcode = 0;
            if ( LitRec* lit = FindLiteral_( pS, pE, &hashcode ) )
              return lit->literal;
            else
              return InsertLiteral( pS, pE-pS, hashcode, false );
          }

        CXX_NO_INLINE
        LitRec* FindLiteral_( pwide_t pS, pwide_t pE, unsigned *hashcode )
          {
            *hashcode = Crc32( 0, (const byte_t*)pS, (pE-pS)*sizeof(wchar_t) );
            LitRec* r = htable_[*hashcode%tHtsize];
            for ( ; r; r = r->next )
              if ( r->hashcode == *hashcode && r->length == pE-pS )
                if ( 0 == memcmp( pS, r->literal, (pE-pS)*sizeof(wchar_t) ) )
                  return r;
            return 0;
          }

        pwide_t FindLiteral( pwide_t pS )
          {
            return FindLiteral( pS, pS + wcslen(pS) );
          }

        pwide_t FindLiteral( pwide_t pS, pwide_t pE )
          {
            unsigned hashcode;
            if ( LitRec *r = FindLiteral_( pS,pE, &hashcode ) )
              return r->literal;
            else
              return 0;
          }

        void* pool_;
        char* sbrk_;
        char* sbrkE_;
        LitRec** htable_;

        enum { POOL_PAGE_SIZE = tPoolpage };

        void AllocatePage_()
          {
            char* pool = new char[POOL_PAGE_SIZE];
            *(void**)pool = pool_;
            pool_ = pool;
            sbrk_ = pool+16;
            sbrkE_= pool+POOL_PAGE_SIZE;
            mem_size_ += POOL_PAGE_SIZE;
          }

        CXX_NO_INLINE
        void* Sbrk( unsigned cnt )
          {
            if ( sbrkE_ - sbrk_ < cnt )
              AllocatePage_();

            void* foo = sbrk_;
            sbrk_ += cnt;
            return foo;
          }

        ~AST_TEMPLATE()
          {
            while ( pool_ )
              {
                void* foo = *(void**)pool_;
                delete[] (char*)pool_;
                pool_ = foo;
              }
            delete[] htable_;
          }

        AST_TEMPLATE()
          {
            pool_ = 0;
            sbrk_ = 0;
            sbrkE_ = 0;
            htable_ = new LitRec*[tHtsize];
            memset( htable_, 0, sizeof(LitRec*)*tHtsize );
            mem_size_ = nod_count_ = lit_count_ = 0;
          }

        unsigned mem_size_,nod_count_,lit_count_;

        unsigned GetMemUsed()  { return mem_size_; }
        unsigned GetMemAlct()  { return mem_size_-(sbrkE_-sbrk_); }
        unsigned GetNodCount() { return nod_count_; }
        unsigned GetLitCount() { return lit_count_; }

        CXX_NO_INLINE
        pwide_t FirstLiteral(void **pi)
          {
            for ( int i = 0; i < tHtsize; ++i )
              if ( htable_[i] )
                {
                  *pi = htable_[i];
                  return htable_[i]->literal;
                }
            *pi = 0;
            return 0;
          }

        CXX_NO_INLINE
        pwide_t NextLiteral(void **pi)
          {
            if ( *pi )
            {
              LitRec* lr = (LitRec*)*pi;
              if ( lr->next )
                {
                  *pi = lr->next;
                  return lr->next->literal;
                }
              else
                {
                  unsigned i = lr->hashcode % tHtsize;
                  while ( ++i < tHtsize && !htable_[i]  ) {}
                  if ( i < tHtsize )
                    {
                      *pi = htable_[i];
                      return htable_[i]->literal;
                    }
                  else
                    *pi = 0;
                }
            }

            return 0;
          }

      private:
        AST_TEMPLATE& operator=(AST_TEMPLATE const&);
        AST_TEMPLATE(AST_TEMPLATE const&);

      };

} //namespace

#endif // __C26B3732_CD91_4E1E_9D39_D1590F635188__
