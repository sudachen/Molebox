
/*

  Copyright (c) 2005-2009, Alexey Sudachen, alexey@sudachen.name

*/

namespace teggo
{
  template < unsigned tUseExceptions >
    struct Teggo_Internal_NPCpS_
      {
        template < class tTx >
          static void CXX_STDCALL MoveRight_( tTx *begin, tTx *&end, unsigned number )
            {
              tTx *from = end -1;
              end = end + number;
              tTx *to   = end -1;

              CXX_TRY
                {
                  for ( ; from != begin-1 ; --from, --to )
                    {
                      teggo_new_and_move_data(to,from);
                      from->~tTx();
                    }
                }
              CXX_CATCH_ALL
                {
                  --to;
                  for ( ; to != end; )
                    {
                      to->~tTx();
                    }
                  from->~tTx();
                  end = from;
                  CXX_RETHROW;
                }
            }

        template < class tTx >
          static void CXX_STDCALL MoveRight( tTx *begin, tTx *&end, unsigned number, tTx const *from )
            {
              if ( begin != end )
                {
                  tTx *to = begin;
                  MoveRight_( begin, end, number );
                  CXX_TRY
                    {
                      for ( tTx const *where = to+number ; to != where; ++to )
                        new (to) tTx(*(from++)); // filling
                    }
                  CXX_CATCH_ALL
                    {
                      for ( tTx *where = begin + number; where != end; ++where )
                        where->~tTx();

                      end = to;

                      CXX_RETHROW;
                    }
                }
              else
                {
                  for ( tTx *where = end+number ; end != where; ++end )
                    new (end) tTx( *(from++) ); // filling
                }
            }

        template < class tTx >
          static void CXX_STDCALL MoveRight( tTx *begin, tTx *&end, unsigned number, tTx const& filler )
            {
              if ( begin != end )
                {
                  tTx *to = begin;
                  MoveRight_( begin, end, number );
                  CXX_TRY
                    {
                      for ( tTx *where = to+number ; to != where; ++to )
                        new (to) tTx( filler );
                    }
                  CXX_CATCH_ALL
                    {
                      for ( tTx *where = begin + number; where != end; ++where )
                        where->~tTx();

                      end = to;

                      CXX_RETHROW;
                    }
                }
              else
                {
                  while ( number-- )
                    {
                      new (end) tTx( filler );
                      ++end;
                    }
                }
            }

        template < class tTx >
          static void CXX_STDCALL MoveLeft( tTx *begin, tTx *&end, unsigned number )
            {
              tTx *nend = end - number;

              if ( begin != end )
                {
                  tTx *from   = begin + number;
                  CXX_TRY
                    {
                      for ( ; from != end ; ++from, ++begin )
                        {
                          begin->~tTx();
                          //new (begin) tTx (*from);
                          teggo_new_and_move_data(begin,from);
                        }
                    }
                  CXX_CATCH_ALL
                    {
                      for ( ; from != end; ++from )
                        from->~tTx();
                      end = begin;
                      CXX_RETHROW;
                    }
                }

              for ( ; end != nend ; --end )
                (end-1)->~tTx();
            }
      };

  typedef Teggo_Internal_NPCpS_<CXX_USES_EXCEPTIONS> NonPodCopyStrategy;

  struct PodCopyStrategy
    {
      template < class tTx >
        static void CXX_STDCALL MoveRight( tTx *begin, tTx *&end, unsigned number, tTx const *from )
          {
            if ( begin != end )
              memmove( begin + number, begin, (end-begin) * sizeof ( tTx ) );
            memcpy( begin, from, number * sizeof ( tTx ) );
            end += number;
          }

      template < class tTx >
        static void CXX_STDCALL MoveRight( tTx *begin, tTx *&end, unsigned number, tTx const& filler )
          {
            if ( begin != end )
              memmove( begin + number, begin, (end-begin) * sizeof ( tTx ) );
            end += number;
            while( number-- )
              memcpy( begin++, &filler, sizeof(tTx) );
          }

      template < class tTx >
        static void CXX_STDCALL MoveLeft( tTx *begin, tTx *&end, unsigned number )
          {
            if ( begin != end )
              memmove( begin, begin+number, ((end-begin)-number) * sizeof(tTx));
            end -= number;
          }
    };

} // namespace
