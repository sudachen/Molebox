
/*

  Copyright (c) 2008-2009, Alexey Sudachen, alexey@sudachen.name

*/

enum _Lx_logger_out_
{
    xlog    = -1,
    xdbg    = 3,
};

extern bool iflog;
#define XLOG !iflog||xlog
//#define XLOG xlog
extern bool ifdbg;
#define XDBG !ifdbg||xlog

int  StartLog();

void LogoutS(BaseStringT<char>&);
void LogoutS(pwide_t text, int level);
void LogoutS(pchar_t text, int level);
void LogoutS_(pchar_t text, int count, int level);
void LogoutFormatted(FormatT<char> const& fmt, _Lx_logger_out_ lo);
void LogoutString(pwide_t text, int lo);
void LogoutString(pchar_t text, int lo);

template <class tTc>
inline void operator << (_Lx_logger_out_ lo, tTc const* text)
{ LogoutString(text, lo); }

template <class tTc>
inline bool operator | (_Lx_logger_out_ lo, tTc const* text)
{ LogoutString(text, lo); return false; }

template <class tTc>
inline void operator << (_Lx_logger_out_ lo, BaseStringT<tTc> const& text)
{ LogoutString(+text, lo); }

template <class tTc>
inline bool operator | (_Lx_logger_out_ lo, BaseStringT<tTc> const& text)
{ LogoutString(+text, lo); return false; }

inline void operator << (_Lx_logger_out_ lo, FormatT<char> const& fmt)
{ LogoutFormatted(fmt, lo); }

inline bool operator | (_Lx_logger_out_ lo, FormatT<char> const& fmt)
{ LogoutFormatted(fmt, lo); return false; }
