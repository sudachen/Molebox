
/*

  Copyright (c) 2005-2009, Alexey Sudachen, alexey@sudachen.name

*/

#ifndef ___79e62a42_d149_40f0_b6b5_5c54d3da19b8___
#define ___79e62a42_d149_40f0_b6b5_5c54d3da19b8___

#include "_specific.h"
#include "_crc32.h"
#include "string.h"
#include "collection.h"
#include "ptr_rcc.h"
#include "format.h"
#include "print.h"

namespace teggo
{

    template <class Tchr>
    struct Cargs : Refcounted
    {
        typedef StringT<Tchr> strtype;
        CollectionT<strtype,char> opts;
        ArrayT<strtype> args;
        strtype progname;
        strtype fullpath;

        struct oI
        {
            char const* s_;
            StringA n_;
            bool rqa_;
            virtual bool NextPart() = 0;
            bool Next()
            {
                if ( !NextPart() )
                    return false;
                else if (*s_ == ':')
                    rqa_ = true, ++s_;
                else
                    rqa_ = false;
                return true;
            }
            char const* Name() {return +n_;}
            bool ReqArg() {return rqa_;}
            oI(char const* s) : s_(s) {}
        };

        struct SoI : oI
        {
            SoI(char const* s) : oI(s) {}
            virtual bool NextPart()
            {
                if ( !s_ ) return false;
                while ( *s_ == ',' || *s_ == ':' || *s_ == '-' ) ++s_;
                char const* S = s_;
                if ( *s_ )
                {
                    ++s_;
                    n_ = "-";
                    n_.AppendRange(S,s_-S);
                    return true;
                }
                else
                    return false;
            }
        };

        struct LoI : oI
        {
            LoI(char const* s) : oI(s) {}
            virtual bool NextPart()
            {
                if ( !s_ ) return false;
                while ( *s_ == ',' || *s_ == ':' || *s_ == '-' ) ++s_;
                char const* S = s_;
                while ( *s_ && *s_ != ':' && *s_ != ',' ) ++s_;
                if ( s_ != S )
                {
                    n_ = "--";
                    n_.AppendRange(S,s_-S);
                    return true;
                }
                else
                    return false;
            }
        };

        bool HasOpts(char const* opts, char const* lopts=0)
        {
            for ( SoI i = SoI(opts); i.Next(); )
                if ( this->opts.Get(i.Name()) )
                    return true;
            for ( LoI i = LoI(lopts); i.Next(); )
                if ( this->opts.Get(i.Name()) )
                    return true;
            return false;
        }

        bool HasNoOpts(char const* opts, char const* lopts=0)
        {
            for ( SoI i = SoI(opts); i.Next(); )
                if ( this->opts.Get(i.Name()) )
                    return false;
            for ( LoI i = LoI(lopts); i.Next(); )
                if ( this->opts.Get(i.Name()) )
                    return false;
            return true;
        }

        bool ParseCargs(int argc, Tchr const** argv, char const* sopts=0, char const* lopts = 0, EhFilter eh = EMPTYEF)
        {
            CollectionT<bool,char> c;
            for ( SoI i = SoI(sopts); i.Next(); ) c.Put(i.Name(),i.ReqArg());
            for ( LoI i = LoI(lopts); i.Next(); ) c.Put(i.Name(),i.ReqArg());
            for ( int i = 1; i < argc; ++i )
            {
                if ( *argv[i] && *argv[i] == '-' )
                {
                    Tchr const* s = argv[i]+1;
                    if (  *s == '-' )
                    {
                        if ( bool* rqa = c.Get(+StringA(argv[i])) )
                        {
                            if ( *rqa )
                                if ( i+1 < argc)
                                    this->opts.Put(+StringA(argv[i]),argv[i+1]), ++i;
                                else
                                {
                                    if ( !eh("there is should be an option's argument") )
                                        return false;
                                }
                            else
                                this->opts.Rec(+StringA(argv[i]));
                        }
                        else if ( !eh(_S*"invalid option '%s'" %argv[i]) )
                            return false;
                    }
                    else
                        while ( *s )
                        {
                            char o[3] = "-\0";
                            o[1] = *s++;
                            if ( bool* rqa = c.Get(o) )
                            {
                                if ( *rqa )
                                {
                                    if ( *s )
                                        this->opts.Put(o,s);
                                    else if ( i+1 < argc)
                                        this->opts.Put(o,argv[i+1]), ++i;
                                    else if ( !eh("there is should be an option's argument") )
                                        return false;
                                    break;
                                }
                                else
                                    this->opts.Rec(o);
                            }
                            else if ( !eh(_S*"invalid option '%s'" %argv[i]) )
                                return false;
                        }
                }
                else
                    this->args.Append(argv[i]);
            }

            /** FIXIT !!! **/
            this->progname = argv[0];
            this->fullpath = argv[0];

            return true;
        }
    };

    typedef Cargs<char> CargsA;
    typedef Cargs<wchar_t> CargsW;
    typedef rcc_ptr<CargsA> CargsPtrA;
    typedef rcc_ptr<CargsW> CargsPtrW;

    template <class Tchr>
    rcc_ptr<Cargs<Tchr>> ParseCargs(int argc, Tchr const** argv, char const* opts=0, char const* lopts = 0, EhFilter eh = EMPTYEF)
    {
        rcc_ptr<Cargs<Tchr>> cargs(new Cargs<Tchr>());
        if ( cargs->ParseCargs(argc,argv,opts,lopts,eh) )
            return cargs;
        return rcc_ptr<Cargs<Tchr>>(0);
    }

    template <class Tchr>
    rcc_ptr<Cargs<Tchr>> ParseCmdL(Tchr const* cmdl, char const* opts=0, char const* lopts = 0, EhFilter eh = EMPTYEF)
    {
        rcc_ptr<Cargs<Tchr>> cargs(new Cargs<Tchr>());
        int argc = 0; Tchr** argv = 0;
        SysBuildArgv(cmdl,argc,argv);
        if ( argc && argv )
            if ( cargs->ParseCargs(argc,(Tchr const**)argv,opts,lopts,eh) )
                return cargs;
        return rcc_ptr<Cargs<Tchr>>(0);
    }
} // namespace

#endif /*___79e62a42_d149_40f0_b6b5_5c54d3da19b8___*/
