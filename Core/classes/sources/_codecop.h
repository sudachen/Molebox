
/*

  Copyright (c) 2005-2009, Alexey Sudachen, alexey@sudachen.name

*/

namespace teggo
{

    enum ILLEGAL_SITUATION
    {
        SIT_VIOLATION,
        SIT_ASSERTION,
        SIT_EMERGENCY,
        SIT_WARNING,
        SIT_DEBUGIT,
        SIT_FAKE_SITUATION
    };

    typedef void (CXX_STDCALL* CodePoliceNotification)(pwide_t /*description*/, int /*lineno*/, pchar_t /*fname*/);
    typedef void (CXX_STDCALL* CodePoliceTerminator)(pwide_t /*description*/, int /*lineno*/, pchar_t /*fname*/);

    struct CodePolice__
    {
        _TEGGO_EXPORTABLE static void CXX_STDCALL Message(pwide_t msg,pwide_t prefix, int lineno, pchar_t fname);
        _TEGGO_EXPORTABLE static void CXX_STDCALL Terminator(pwide_t msg, int lineno, pchar_t fname);
        _TEGGO_EXPORTABLE static void CXX_STDCALL Notifier(pwide_t msg, int lineno, pchar_t fname);
    };

    template <unsigned tNo>
    struct CodePolice_
    {
        CodePoliceNotification notify_;
        CodePoliceTerminator   terminate_;
        _TEGGO_EXPORTABLE void Analyse( ILLEGAL_SITUATION sit, pwide_t descr, int lineno, pchar_t fname );
    };

    template <> void CodePolice_<0>::Analyse( ILLEGAL_SITUATION sit, pwide_t descr, int lineno, pchar_t fname );

    template <unsigned tNo>
    struct CodePolice_Office
    {
        static _TEGGO_EXPORTABLE_DATA CodePolice_<tNo> police_;
        CodePolice_<tNo>* operator->()
        {
            return &police_;
        }
    };

    #if defined _TEGGOINLINE
    template <unsigned tNo>
    CodePolice_<tNo> CodePolice_Office<tNo>::police_
    =
    {
        &CodePolice__::Notifier,
        &CodePolice__::Terminator
    };
    #endif

    static CodePolice_Office<0> CodePolice;

    inline void CXX_STDCALL violation_check(
        bool occured, pwide_t description, int lineno, pchar_t fname )
    {
        if ( occured )
            CodePolice->Analyse( SIT_VIOLATION, description, lineno, fname );
    }

    inline void CXX_STDCALL assertion_check(
        bool satisfied, pwide_t description, int lineno, pchar_t fname )
    {
        if ( !satisfied )
            CodePolice->Analyse( SIT_ASSERTION, description, lineno, fname );
    }

    inline void CXX_STDCALL emergency(
        pwide_t description, int lineno, pchar_t fname )
    {
        CodePolice->Analyse( SIT_EMERGENCY, description, lineno, fname );
    }

    inline void CXX_STDCALL warning(
        pwide_t msg, int lineno, pchar_t fname )
    {
        CodePolice->Analyse( SIT_WARNING, msg, lineno, fname );
    }

} // namespace

#if defined _TEGGOSTRIP__FILE__
# define _CodeCopStripFile(x) "<file>"
#else
# define _CodeCopStripFile(x) x
#endif

#if CXX_COMPILER_IS_MSVC
# if defined _TEGGOSTRIP__EXPR__
#  define _CodeLexpr(x) L"<expr>"
# else
#  define _CodeLexpr(x) L#x
# endif
#else
# if defined _TEGGOSTRIP__EXPR__
#  define _CodeLexpr(x) #"<expr>"
# else
#  define _CodeLexpr(x) #x
# endif
#endif
// неотключаемые проверки
#define VIOLATION(occured,desc) ::teggo::violation_check( !!(occured), (desc), __LINE__, _CodeCopStripFile(__FILE__) )
#define REQUIRE(expr) ::teggo::assertion_check( !!(expr), L"ASSERTION:" _CodeLexpr(expr), __LINE__, _CodeCopStripFile(__FILE__) )
#define EMERGENCY(desc) ::teggo::emergency( desc, __LINE__, _CodeCopStripFile(__FILE__) )
#define WARNING(msg) ::teggo::warning( msg, __LINE__, _CodeCopStripFile(__FILE__) )
#define UNREACHABLE() EMERGENCY(L"unreachable code")

// отключаемые проверки
#if defined _DBGCODECOP
# define DBG_VIOLATION(occured,desc) ::teggo::violation_check( !!(occured), (desc), __LINE__, _CodeCopStripFile(__FILE__) )
# define DBG_REQUIRE(expr) ::teggo::assertion_check( !!(expr), L"DBG_ASSERTION:" _CodeLexpr(expr), __LINE__, _CodeCopStripFile(__FILE__) )
# define DBG_WARNING(msg) ::teggo::warning( msg, __LINE__, _CodeCopStripFile(__FILE__) )
#else
# define DBG_VIOLATION(occured,desc)
# define DBG_REQUIRE(expr)
# define DBG_WARNING(msg)
#endif

// жёсткие проверки
#if defined _STRICTCODECOP
# define STRICT_VIOLATION(occured,desc) ::teggo::violation_check( !!(occured), (desc), __LINE__, _CodeCopStripFile(__FILE__) )
# define STRICT_REQUIRE(expr) ::teggo::assertion_check( !!(expr), L"STRICT_ASSERTION:" _CodeLexpr(expr), __LINE__, _CodeCopStripFile(__FILE__) )
# define STRICT_ENSURE(expr) ::teggo::assertion_check( !!(expr), L"STRICT_ASSERTION:" _CodeLexpr(expr), __LINE__, _CodeCopStripFile(__FILE__) )
# define STRICT_WARNING(msg) ::teggo::warning( msg, __LINE__, _CodeCopStripFile(__FILE__) )
#else
# define STRICT_VIOLATION(occured,desc) ((void)0)
# define STRICT_REQUIRE(expr) ((void)0)
# define STRICT_ENSURE(expr) ((void)(expr))
# define STRICT_WARNING(msg) ((void)0)
#endif

#if !defined _SAFE  /* check internal contract */
# if defined _STRICTCODECOP || defined _DBGCODECOP
#   define _SAFE 3
# else
#   define _SAFE 1
# endif
#endif

#define _SAFE3 ( _SAFE >= 3 ) /* check internal structures integrity */
#define _SAFE5 ( _SAFE >= 5 ) /* do insane check */

#if defined _DEBUG
_TEGGO_EXPORTABLE void CXX_STDCALL _Teggo_Debug_Code_Here_();
#else
inline void _Teggo_Debug_Code_Here_() {}
#endif

#define _SAFE_DEBUG_IF(expr) ((expr)?(_Teggo_Debug_Code_Here_(),true):false)
#define _SAFE_VIOLATION(expr) (!(expr)?(_Teggo_Debug_Code_Here_(),true):false)

#if defined _TEGGOINLINE
# include "_codecop.inl"
#endif
