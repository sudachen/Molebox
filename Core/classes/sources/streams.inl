
/*

  Copyright (c) 2005-2009, Alexey Sudachen, alexey@sudachen.name

*/

#if !defined ___1f0b46f2_1025_41d0_9967_2a3bcb1f266c___
#define ___1f0b46f2_1025_41d0_9967_2a3bcb1f266c___

#include "streams.h"
#include <sys/stat.h>
#include <stdio.h>
#include <io.h>
#include "md5.h"
#include "sysutil.h"

#if defined _TEGGO_STREAMS_HERE
# define _TEGGO_STREAMS_FAKE_INLINE _TEGGO_EXPORTABLE
#else
# define _TEGGO_STREAMS_FAKE_INLINE CXX_FAKE_INLINE
#endif

namespace teggo {

  _TEGGO_STREAMS_FAKE_INLINE
  DataStream::DataStream() {}

  _TEGGO_STREAMS_FAKE_INLINE
  DataStream::~DataStream() {}

  _TEGGO_STREAMS_FAKE_INLINE
  TEGGOt_E DataStream::Rewind()
    {
      if ( Features() & CAN_SEEK )
        return Seek(0,0);
      else
        return TEGGO_UNSUPPORTED;
    }

  _TEGGO_STREAMS_FAKE_INLINE
  TEGGOt_E DataStream::Read( BufferT<byte_t> &buf, int count)
    {
      if ( count < 0 )
        count = this->Available();
      buf.Resize(count);
      if ( !count )
        return Error();
      int q = Read( &buf[0], count );
      if ( q > 0 )
        buf.Resize(q);
      else
        buf.Clear();
      return q ? TEGGO_OK : Error();
    }

  _TEGGO_STREAMS_FAKE_INLINE
  int DataStream::ReadText(char *buf, int count)
    {
      REQUIRE( count >= 0 );
      TextBuffer bf_(*this);
      int i = 0;
      for ( i = 0; i < count; ++i )
        {
          int c = bf_.GetC();
          if ( c > 0 )
            {
              if ( buf ) buf[i] = c;
            }
          else
            break;
        }
      return i;
    }

  _TEGGO_STREAMS_FAKE_INLINE
  TEGGOt_E DataStream::UtfReadTextEx( BufferT<wchar_t> &buf, int count )
    {
      buf.Resize(0);
      UtfTextBuffer bf_(*this);
      int c,i=0;
      while  ( (count < 0 || i != count )  && (c = bf_.GetC()) > 0 )
        buf.Append(c), ++i;
      return TEGGO_OK;
    }

  _TEGGO_STREAMS_FAKE_INLINE
  int DataStream::UtfReadText(wchar_t *buf, int count)
    {
      REQUIRE( count >= 0 );
      UtfTextBuffer bf_(*this);
      int i = 0;
      for ( i = 0; i < count; ++i )
        {
          int c = bf_.GetC();
          if ( c > 0 )
            {
              if ( buf ) buf[i] = c;
            }
          else
            break;
        }
      return i;
    }

  _TEGGO_STREAMS_FAKE_INLINE
  int _DataStream_ReadLine( DataStream &ds, GenericWriterT<char> &out )
    {
      bool was_eof = ds.Eof();
      TextBuffer bf_(ds);
      for ( ;; )
        {
          int c = bf_.GetC();
          if ( c > 0 )
            {
              out.Push((char)c);
              if ( c == '\n' ) return 1;
            }
          else
            return ( ds.Eof() && (!was_eof || out.Count()) ) ? 1 : 0;
        }
      return 1;
    }

  _TEGGO_STREAMS_FAKE_INLINE
  char * DataStream::ReadLine_( char * buf, int count )
    {
      if (buf) memset(buf,0,count);
      GenericWriterT<char> gw( buf, buf, buf+count );
      if ( _DataStream_ReadLine( *this, gw ) )
        return buf;
      else
        return 0;
    }

  _TEGGO_STREAMS_FAKE_INLINE
  TEGGOt_E DataStream::ReadLineEx( BufferT<char> &out )
    {
      out.Resize(0);
      BufferTwriterT<char> gw( out );
      if (  _DataStream_ReadLine( *this, gw ) )
        return TEGGO_OK;
      else
        return Error();
    }

  _TEGGO_STREAMS_FAKE_INLINE
  TEGGOt_E DataStream::ReadLineS( BaseStringT<char> &out )
    {
      out.Resize(0);
      StringTwriterT<char> gw( out );
      if (  _DataStream_ReadLine( *this, gw ) )
        return TEGGO_OK;
      else
        return Error();
    }

  _TEGGO_STREAMS_FAKE_INLINE
  StringT<char> DataStream::ReadLine()
    {
      StringT<char> out;
      StringTwriterT<char> gw( out );
      if (  _DataStream_ReadLine( *this, gw ) )
        return out;
      else
        return StringT<char>();
    }

  _TEGGO_STREAMS_FAKE_INLINE
  int _DataStream_UtfReadLine( DataStream &ds, GenericWriterT<wchar_t> &out )
    {
      UtfTextBuffer bf_(ds);
      for ( ;; )
        {
          int c = bf_.GetC();
          if ( c > 0 )
            {
              out.Push((wchar_t)c);
              if ( c == '\n' ) return 1;
            }
          else
            return ( ds.Eof() && out.Count() ) ? 1 : 0;
        }
      return 1;
    }

  _TEGGO_STREAMS_FAKE_INLINE
  wchar_t * DataStream::UtfReadLine_( wchar_t * buf, int count )
    {
      if (buf) memset(buf,0,count*sizeof(wchar_t));
      GenericWriterT<wchar_t> gw( buf, buf, buf+count );
      if (  _DataStream_UtfReadLine( *this, gw ) )
        return buf;
      else
        return 0;
    }

  _TEGGO_STREAMS_FAKE_INLINE
  TEGGOt_E DataStream::UtfReadLineEx( BufferT<wchar_t> &out )
    {
      out.Resize(0);
      BufferTwriterT<wchar_t> gw( out );
      if (  _DataStream_UtfReadLine( *this, gw ) )
        return TEGGO_OK;
      else
        return Error();
    }

  _TEGGO_STREAMS_FAKE_INLINE
  TEGGOt_E DataStream::UtfReadLineS( BaseStringT<wchar_t> &out )
    {
      out.Resize(0);
      StringTwriterT<wchar_t> gw( out );
      if (  _DataStream_UtfReadLine( *this, gw ) )
        return TEGGO_OK;
      else
        return Error();
    }

  _TEGGO_STREAMS_FAKE_INLINE
  StringT<wchar_t> DataStream::UtfReadLine()
    {
      StringT<wchar_t> out;
      StringTwriterT<wchar_t> gw( out );
      if (  _DataStream_UtfReadLine( *this, gw ) )
        return out;
      else
        return StringT<wchar_t>();
    }
  
  _TEGGO_STREAMS_FAKE_INLINE
  int DataStream::WriteText(char const *buf, int count)
    {
      REQUIRE( count >= 0 );
      char bf[128];
      int nwrote = 0;
      int k = 0;
      while ( k != count )
        {
          int i = 0;
          for ( ; i < sizeof(bf)-1 && k < count; ++i )
            {
              if ( buf[k] == '\n' )
                if ( i+1 < sizeof(bf)-1 )
                  bf[i++] = '\r', bf[i] = '\n', ++k;
                else
                  break;
              else
                bf[i] = buf[k++];
            }
          bf[i] = 0;
          int waswrote = Write( bf, i );
          if ( nwrote > 0 ) nwrote += waswrote;
          if ( waswrote != i ) break;
        }
      return nwrote;
    }

  _TEGGO_STREAMS_FAKE_INLINE
  int DataStream::UtfWriteText(wchar_t const *buf, int count)
    {
      char bf[128];
      int j = 0;
      int nwrote = 0;
      for ( int i = 0; i < count ; ++i )
        {
          if ( j >= sizeof(bf) - 4 )
            {
              nwrote += WriteText(bf,j);
              j = 0;
            }
          UtfEncode(bf+j,buf[i],&j);
        }
      if ( j )
        nwrote += WriteText(bf,j);
      return nwrote;
    }

  _TEGGO_STREAMS_FAKE_INLINE
  TEGGOt_E DataStream::Print(char const *s)
    {
      return WriteText(s,strlen(s)), Error();
    }

  _TEGGO_STREAMS_FAKE_INLINE
  TEGGOt_E DataStream::UtfPrint(wchar_t const *s)
    {
      return UtfWriteText(s,wcslen(s)), Error();
    }

  _TEGGO_STREAMS_FAKE_INLINE 
  int DataStream::UtfPrintf(pwide_t fmt,...)
    {
      va_list va;
      va_start(va,fmt);
      int retval = UtfVprintf(fmt,va);
      va_end(va);
      return retval;
    }

    _TEGGO_STREAMS_FAKE_INLINE 
  int DataStream::UtfVprintf(pwide_t fmt,va_list va)
    {
      BufferT<wchar_t> s;
      BufferTwriterT<wchar_t> S(s);
      _XsprintArglistVa<wchar_t> xa(va);
      int i = VsprintF(S,fmt,xa);
      UtfWriteText(+s,s.Count()-1);
      return i;
    }
    
    _TEGGO_STREAMS_FAKE_INLINE
  int DataStream::Printf(pchar_t fmt,...)
    {
      va_list va;
      va_start(va,fmt);
      int retval = Vprintf(fmt,va);
      va_end(va);
      return retval;
    }

  _TEGGO_STREAMS_FAKE_INLINE
  int DataStream::Vprintf(pchar_t fmt,va_list va)
    {
      BufferT<char> s;
      BufferTwriterT<char> S(s);
      _XsprintArglistVa<char> xa(va);
      int i = VsprintF(S,fmt,xa);
      WriteText(+s,s.Count()-1);
      return i;
    }

  _TEGGO_STREAMS_FAKE_INLINE
  TEGGOt_E DataStream::Skip(offset_t count)
    {
      // FIXME count - unsigned long value, but seek argument - i32_t
      if ( Features() & CAN_SEEK )
        return Seek(count,SEEK_CUR);
      else
        {
          byte_t bf[128];
          offset_t n = 0;
          while ( n < count )
            {
              if ( int nread = Read(bf,teggo_min<int>(sizeof(bf),count)) )
                { if (nread > 0) n += nread; else break; }
              else break;
            }
          if ( n != count )
            return Error();
          else
            return TEGGO_OK;
        }
    }

  _TEGGO_STREAMS_FAKE_INLINE
  TEGGOt_E DataStream::MD5(u32_t count,void *md5)
    {
      byte_t bf[256];
      MD5_Hash o_md5;
      int n = 0;
      int acount = teggo_abs(count);
      while ( n < acount )
        {
          if ( int nread = Read(bf,teggo_min<int>(sizeof(bf),acount)) )
            {
              n += nread;
              o_md5.Update(bf,nread);
            }
          else
            break;
        }
      o_md5.Finalize(md5);
      if ( count > 0 && n != acount )
        return Error();
      else
        return TEGGO_OK;
    }

  template < class tTc >
    TEGGOt_E _DataStream_SkipChar(DataStream *ds, int (DataStream::* rf)(tTc *,int), int count, int *skipped)
      {
        int n = 0;
        int acount = teggo_abs(count);
        while ( n < acount )
          {
            if ( int nread = (ds->*rf)(0,acount) )
              n += nread;
            else
              break;
          }
        if ( skipped ) *skipped = n;
        if ( count > 0 && n != acount )
          return ds->Error();
        else
          return TEGGO_OK;
      }

  _TEGGO_STREAMS_FAKE_INLINE
  TEGGOt_E DataStream::SkipChar(int count, int *skipped)
    {
      return _DataStream_SkipChar(this,&DataStream::ReadText,count,skipped);
    }

  _TEGGO_STREAMS_FAKE_INLINE
  TEGGOt_E DataStream::UtfSkipChar(int count, int *skipped)
    {
      return _DataStream_SkipChar(this,&DataStream::UtfReadText,count,skipped);
    }

  template < class tTc >
    TEGGOt_E _DataStream_SkipLine(DataStream *ds, tTc *(DataStream::* rl)(tTc *,int), int count, int *skipped)
      {
        ds->ClearError();
        if ( skipped ) *skipped = 0;
        for ( int i = 0; i < count; ++i, (skipped) ? ++*skipped : 0 )
          {
            (ds->*rl)(0,INT_MAX);
            if ( TEGGOt_E e = ds->Error() ) return e;
          }
        return TEGGO_OK;
      }

  _TEGGO_STREAMS_FAKE_INLINE
  TEGGOt_E DataStream::SkipLine(int count, int *skipped)
    {
      return _DataStream_SkipLine(this,&DataStream::ReadLine_,count,skipped);
    }

  _TEGGO_STREAMS_FAKE_INLINE
  TEGGOt_E DataStream::UtfSkipLine(int count, int *skipped)
    {
      return _DataStream_SkipLine(this,&DataStream::UtfReadLine_,count,skipped);
    }

  _TEGGO_STREAMS_FAKE_INLINE
  TEGGOt_E DataStream::ReadIeee32be(float *fu)
    {
      return Read32be((u32_t *)fu);
    }

  _TEGGO_STREAMS_FAKE_INLINE
  TEGGOt_E DataStream::Read32be(u32_t *u)
    {
      byte_t b[4];
      if ( Read(b,-4) == 4 )
        {
          *u = ((u32_t)b[0]) << 24;
          *u |= ((u32_t)b[1]) << 16;
          *u |= ((u32_t)b[2]) << 8;
          *u |= ((u32_t)b[3]);
          return TEGGO_OK;
        }
      else
        return Error();
    }

  _TEGGO_STREAMS_FAKE_INLINE
  TEGGOt_E DataStream::ReadIeee32le(float *fu)
    {
      return Read32be((u32_t *)fu);
    }

  _TEGGO_STREAMS_FAKE_INLINE
  TEGGOt_E DataStream::Read32le(u32_t *u)
    {
      byte_t b[4];
      if ( Read(b,-4) == 4 )
        {
          *u = ((u32_t)b[0]);
          *u |= ((u32_t)b[1]) << 8;
          *u |= ((u32_t)b[2]) << 16;
          *u |= ((u32_t)b[3]) << 24;
          return TEGGO_OK;
        }
      else
        return Error();
    }

  _TEGGO_STREAMS_FAKE_INLINE
  TEGGOt_E DataStream::Read16be(u16_t *u)
    {
      byte_t b[2];
      if ( Read(b,-2) == 2 )
        {
          *u = ((u16_t)b[0]) << 8;
          *u |= ((u16_t)b[1]);
          return TEGGO_OK;
        }
      else
        return Error();
    }

  _TEGGO_STREAMS_FAKE_INLINE
  TEGGOt_E DataStream::Read16le(u16_t *u)
    {
      byte_t b[2];
      if ( Read(b,-2) == 2 )
        {
          *u = ((u16_t)b[0]);
          *u |= ((u16_t)b[1]) << 8;
          return TEGGO_OK;
        }
      else
        return Error();
    }

  _TEGGO_STREAMS_FAKE_INLINE
  TEGGOt_E DataStream::Read8(uc8_t *u)
    {
      if ( Read(u,-1) == 1 )
        {
          return TEGGO_OK;
        }
      else
        return Error();
    }


  _TEGGO_STREAMS_FAKE_INLINE
  TEGGOt_E DataStream::WriteIeee32be(float fu)
    {
      return Write32be((u32_t const&)fu);
    }

  _TEGGO_STREAMS_FAKE_INLINE
  TEGGOt_E DataStream::Write32be(u32_t u)
    {
      byte_t b[4];
      b[0] = byte_t(u >> 24);
      b[1] = byte_t(u >> 16);
      b[2] = byte_t(u >> 8);
      b[3] = byte_t(u);
      return Write( b, 4 ) == 4 ? TEGGO_OK : Error();
    }

  _TEGGO_STREAMS_FAKE_INLINE
  TEGGOt_E DataStream::WriteIeee32le(float fu)
    {
      return Write32le((u32_t const&)fu);
    }

  _TEGGO_STREAMS_FAKE_INLINE
  TEGGOt_E DataStream::Write32le(u32_t u)
    {
      byte_t b[4];
      b[3] = byte_t(u >> 24);
      b[2] = byte_t(u >> 16);
      b[1] = byte_t(u >> 8);
      b[0] = byte_t(u);
      return Write( b, 4 ) == 4 ? TEGGO_OK : Error();
    }

  _TEGGO_STREAMS_FAKE_INLINE
  TEGGOt_E DataStream::Write16be(u16_t u)
    {
      byte_t b[2];
      b[0] = byte_t(u >> 8);
      b[1] = byte_t(u);
      return Write( b, 2 ) == 2 ? TEGGO_OK : Error();
    }

  _TEGGO_STREAMS_FAKE_INLINE
  TEGGOt_E DataStream::Write16le(u16_t u)
    {
      byte_t b[2];
      b[1] = byte_t(u >> 8);
      b[0] = byte_t(u);
      return Write( b, 2 ) == 2 ? TEGGO_OK : Error();
    }

  _TEGGO_STREAMS_FAKE_INLINE
  TEGGOt_E DataStream::Write8(uc8_t u)
    {
      return Write( &u, 1 ) == 1 ? TEGGO_OK : Error();
    }

  _TEGGO_STREAMS_FAKE_INLINE
  TEGGOt_E DataStream::Ungetc(char c)
    {
      if ( Features() & CAN_SEEK )
        return Seek(-1,SEEK_CUR);
      else
        return TEGGO_UNSUPPORTED;
    }

  _TEGGO_STREAMS_FAKE_INLINE
  DataStream::offset_t DataStream::Size()
    {
      offset_t sz = 0;
      if ( Features() & CAN_SEEK )
        {
          offset_t p = Tell();
          Seek(0,0);
          sz = Available();
          Seek(p,0);
        }
      else
        sz = Available();
      return sz;
    }

  _TEGGO_STREAMS_FAKE_INLINE
  int BaseDataStream_::Read( void *buf, int count )
      {
        error_ = TEGGO_UNSUPPORTED;
        return -1;
      }

  _TEGGO_STREAMS_FAKE_INLINE
  int BaseDataStream_::Write( void const *buf, int count )
    {
      error_ = TEGGO_UNSUPPORTED;
      return -1;
    }

  _TEGGO_STREAMS_FAKE_INLINE
  u32_t BaseDataStream_::Features()
    {
      return 0;
    }

  _TEGGO_STREAMS_FAKE_INLINE
  BaseDataStream_::BaseDataStream_()
    : error_(TEGGO_OK) {}

  _TEGGO_STREAMS_FAKE_INLINE
  BaseDataStream_::~BaseDataStream_()
    {}

  _TEGGO_STREAMS_FAKE_INLINE
  BaseDataStream_::offset_t BaseDataStream_::Available()
    {
      error_ = TEGGO_UNSUPPORTED;
      return 0;
    }

  _TEGGO_STREAMS_FAKE_INLINE
  BaseDataStream_::offset_t BaseDataStream_::Tell()
    {
      error_ = TEGGO_UNSUPPORTED;
      return 0;
    }

  _TEGGO_STREAMS_FAKE_INLINE
  TEGGOt_E BaseDataStream_::Seek(i32_t, int opt)
    {
      return error_ = TEGGO_UNSUPPORTED;
    }

  _TEGGO_STREAMS_FAKE_INLINE
  TEGGOt_E BaseDataStream_::Flush()
    {
      return error_ = TEGGO_UNSUPPORTED;
    }

  _TEGGO_STREAMS_FAKE_INLINE
  TEGGOt_E BaseDataStream_::Close()
    {
      return error_ = TEGGO_UNSUPPORTED;
    }

  _TEGGO_STREAMS_FAKE_INLINE
  TEGGOt_E BaseDataStream_::Error()
    {
      return error_;
    }

  _TEGGO_STREAMS_FAKE_INLINE
  TEGGOt_E BaseDataStream_::ClearError()
    {
      TEGGOt_E e= error_;
      error_ = TEGGO_OK;
      return e;
    }

  _TEGGO_STREAMS_FAKE_INLINE
  bool BaseDataStream_::IsClosed()
    {
      return error_ = TEGGO_UNSUPPORTED, true;
    }

  _TEGGO_STREAMS_FAKE_INLINE NullDataStream::NullDataStream() {}
  _TEGGO_STREAMS_FAKE_INLINE NullDataStream::~NullDataStream() {}
  _TEGGO_STREAMS_FAKE_INLINE int NullDataStream::Read( void *buf, int count )
    {
      if ( count > 0 )
        memset(buf,0,count);
      else
        count = 0;
      return count;
    }
  _TEGGO_STREAMS_FAKE_INLINE int NullDataStream::Write( void const *buf, int count )
    {
      return count;
    }
  _TEGGO_STREAMS_FAKE_INLINE u32_t NullDataStream::Features()
    {
      return DataStream::CAN_WRITE
            |DataStream::CAN_READ
            |DataStream::CAN_SEEK
            |DataStream::CAN_UNGETC
            |DataStream::CAN_REWIND;
    }
  _TEGGO_STREAMS_FAKE_INLINE NullDataStream::offset_t NullDataStream::Available() { return 0; }
  _TEGGO_STREAMS_FAKE_INLINE NullDataStream::offset_t NullDataStream::Tell() { return 0; }
  _TEGGO_STREAMS_FAKE_INLINE TEGGOt_E NullDataStream::Seek(i32_t, int opt) { return TEGGO_OK; }
  _TEGGO_STREAMS_FAKE_INLINE TEGGOt_E NullDataStream::Flush() { return TEGGO_OK; }
  _TEGGO_STREAMS_FAKE_INLINE TEGGOt_E NullDataStream::Close() { return TEGGO_OK; }
  _TEGGO_STREAMS_FAKE_INLINE TEGGOt_E NullDataStream::Error() { return TEGGO_OK; }
  _TEGGO_STREAMS_FAKE_INLINE TEGGOt_E NullDataStream::ClearError() { return TEGGO_OK; }
  _TEGGO_STREAMS_FAKE_INLINE StringT<wchar_t> NullDataStream::Identifier() { return StringT<wchar_t>("<null>"); }
  _TEGGO_STREAMS_FAKE_INLINE bool NullDataStream::IsClosed() { return false; }

#ifdef _TEGGO_STREAMS_LOGWRITE_HERE

  _TEGGO_STREAMS_FAKE_INLINE LogDataStream::LogDataStream(_T_logger_out_ lo) : lout(lo) {}
  _TEGGO_STREAMS_FAKE_INLINE LogDataStream::~LogDataStream() {}
  _TEGGO_STREAMS_FAKE_INLINE int LogDataStream::Read( void *buf, int count ) { return TEGGO_UNSUPPORTED; }
  _TEGGO_STREAMS_FAKE_INLINE int LogDataStream::Write( void const *buf, int count )
    {
      Logout_((const char*)buf,count,lout);
      return count;
    }
  _TEGGO_STREAMS_FAKE_INLINE u32_t LogDataStream::Features()  { return DataStream::CAN_WRITE; }
  _TEGGO_STREAMS_FAKE_INLINE LogDataStream::offset_t LogDataStream::Available() { return 0; }
  _TEGGO_STREAMS_FAKE_INLINE LogDataStream::offset_t LogDataStream::Tell() { return 0; }
  _TEGGO_STREAMS_FAKE_INLINE TEGGOt_E LogDataStream::Seek(i32_t, int opt) { return TEGGO_OK; }
  _TEGGO_STREAMS_FAKE_INLINE TEGGOt_E LogDataStream::Flush() { return TEGGO_OK; }
  _TEGGO_STREAMS_FAKE_INLINE TEGGOt_E LogDataStream::Close() { return TEGGO_OK; }
  _TEGGO_STREAMS_FAKE_INLINE TEGGOt_E LogDataStream::Error() { return TEGGO_OK; }
  _TEGGO_STREAMS_FAKE_INLINE TEGGOt_E LogDataStream::ClearError() { return TEGGO_OK; }
  _TEGGO_STREAMS_FAKE_INLINE StringT<wchar_t> LogDataStream::Identifier() { return StringT<wchar_t>(_S*L"log(%d)" %lout); }
  _TEGGO_STREAMS_FAKE_INLINE bool LogDataStream::IsClosed() { return false; }

#endif

  CXX_FAKE_INLINE
  static int __cdecl _f_eof(FILE* f) { return feof(f); }

  static _fdstream_api_t const _FILE_fdstream_api =
    {
      (_fdstream_fread_t)fread,
      (_fdstream_fwrite_t)fwrite,
      (_fdstream_fseek_t)fseek,
      (_fdstream_ftell_t)ftell,
      (_fdstream_fflush_t)fflush,
      (_fdstream_fclose_t)fclose,
      (_fdstream_feof_t)_f_eof
    };

  _TEGGO_STREAMS_FAKE_INLINE
  StringT<wchar_t> BaseFileDataStream::Identifier()
    {
      return fullpath_;
    }

  _TEGGO_STREAMS_FAKE_INLINE
  u32_t BaseFileDataStream::Features()
    {
      return DataStream::CAN_WRITE
            | (canread_  ? DataStream::CAN_READ  : 0 )
            | (canwrite_ ? DataStream::CAN_WRITE : 0 )
            | (canseek_  ? DataStream::CAN_SEEK  : 0 )
            //|DataStream::CAN_UNGETC
            //|DataStream::CAN_REWIND
            |DataStream::IS_REGULAR
            |DataStream::NEED_CLOSE
            |DataStream::NEED_FLUSH
            ;
    }

  inline pwide_t DFS_MakeOpt(int flags)
    {
      static wchar_t opt_[8] = {0};
      wchar_t *opt = &opt_[0];

      if ( flags & FILEOPEN_CREATE )
        *opt++ = 'w';
      else if ( flags & FILEOPEN_APPEND )
        *opt++ = 'a';
      else
        *opt++ = 'r';

      if ( flags & FILEOPEN_WRITE && !(flags & FILEOPEN_APPEND) )
        *opt++ = '+';

      *opt++ = 'b';
      *opt = 0;
      return opt_;
    }

  _TEGGO_STREAMS_FAKE_INLINE
  BaseFileDataStream::BaseFileDataStream(_fdstream_api_t const *api)
    : f_(0), api_(api)
    {
      canread_ = canwrite_ = canseek_ = false;
    }

  _TEGGO_STREAMS_FAKE_INLINE
  void BaseFileDataStream::_Init(void *fd, pwide_t fname, int opt)
    {
      canread_ = canwrite_ = canseek_ = false;
      f_ = fd;
      if ( opt & (FILEOPEN_APPEND|FILEOPEN_WRITE|FILEOPEN_CREATE) )
        canwrite_ = true;
      if ( !(opt & FILEOPEN_APPEND) )
        {
          canread_ = true;
          canseek_ = true;
        }
      if ( opt & STREAM_CHARBYCHAR ) 
        canseek_ = false;
      fullpath_ = fname;
      error_ = TEGGO_OK;
      return;
    }

  _TEGGO_STREAMS_FAKE_INLINE
  FdDataStream::FdDataStream(FILE *fd, StringParam fname, int dupit, int opt)
    : BaseFileDataStream(&_FILE_fdstream_api)
    {
      int ff = _dup(_fileno(fd));
      if ( ff >= 0 )
        {
          FILE *f = dupit ? _wfdopen(ff,(wchar_t*)DFS_MakeOpt(opt)) : fd;
          if ( f )
            {
              _Init(f,+fname,opt);
              return;
            }
        }
      error_ = TEGGO_FAIL;
    }

  _TEGGO_STREAMS_FAKE_INLINE
  FdDataStream::FdDataStream(StringParam fname, unsigned flags)
    : BaseFileDataStream(&_FILE_fdstream_api)
    {
      canread_ = canwrite_ = canseek_ = false;
      BufferT<wchar_t> pathbuf(MAX_PATH);
      int n = GetFullPathNameW(+fname,MAX_PATH,&pathbuf[0],0);
      if ( n && n <= MAX_PATH )
        {
          fullpath_.SetRange(&pathbuf[0],n);
          FILE *f = _wfopen( fullpath_.Str(), (wchar_t*)DFS_MakeOpt(flags) );

          if ( f )
            {
              _Init(f,+fname,flags);
              return;
            }
        }

      error_ = TEGGO_NOFILE;
    }

  _TEGGO_STREAMS_FAKE_INLINE
  FdDataStream::~FdDataStream()
    {
      Close();
    }
    
  _TEGGO_STREAMS_FAKE_INLINE
  int __cdecl _mem_stream_fake()
    {
      return 0;
    }

  _TEGGO_STREAMS_FAKE_INLINE
  int __cdecl _mem_stream_read(
    void *buf, u32_t size, u32_t count, MemDataStream::MEMDATA *mem)
    {
      int l = mem->pE-mem->pI;
      l = cxx_min<int>(l,size*count);
      l = (l/size)*size;
      if ( l != 0 )
        {
          memcpy(buf,mem->pI,l);
          mem->pI+=l;
        }
      return l;
    }

  _TEGGO_STREAMS_FAKE_INLINE
  int __cdecl _mem_stream_write(void *, u32_t, u32_t, void *)
    {
      return -1;
    }

  _TEGGO_STREAMS_FAKE_INLINE
  int __cdecl _mem_stream_fseek(
    MemDataStream::MEMDATA *mem, i32_t offs, int orign)
    {
      if ( orign == SEEK_SET )
        {
          if ( mem->pS + offs > mem->pE )
            return -1;
          else
            mem->pI = mem->pS + offs;
          return 0;
        }
      else if ( orign == SEEK_CUR )
        {
          if ( mem->pI + offs > mem->pE )
            return -1;
          else
            mem->pI += offs;
          return 0;
        }
      else if ( orign == SEEK_END )
        {
          if ( mem->pE + offs > mem->pE )
            return -1;
          else
            mem->pI = mem->pE+offs;
          return 0;
        }
      return -1;
    }

  _TEGGO_STREAMS_FAKE_INLINE
  i32_t __cdecl _mem_stream_ftell(MemDataStream::MEMDATA *mem)
    {
      return mem->pI-mem->pS;
    }

  _TEGGO_STREAMS_FAKE_INLINE
  int __cdecl _mem_stream_feof(MemDataStream::MEMDATA *mem)
    {
      return !(mem->pS-mem->pI);
    }

  static _fdstream_api_t const _MEM_fdstream_api =
    {
      (_fdstream_fread_t)_mem_stream_read,
      (_fdstream_fwrite_t)_mem_stream_write,
      (_fdstream_fseek_t)_mem_stream_fseek,
      (_fdstream_ftell_t)_mem_stream_ftell,
      (_fdstream_fflush_t)_mem_stream_fake,
      (_fdstream_fclose_t)_mem_stream_fake,
      (_fdstream_feof_t)_mem_stream_feof
    };

  _TEGGO_STREAMS_FAKE_INLINE
  MemDataStream::MemDataStream(void *data, unsigned len, int features)
    :BaseFileDataStream(&_MEM_fdstream_api)
    {
      memdesc_.pS = (byte_t*)data;
      memdesc_.pE = memdesc_.pS + len;
      memdesc_.pI = memdesc_.pS;
      is_unicode_mem_stream_ = !!(features & FILEOPEN_UNICODE_MEMSTREAM);
      _Init(&memdesc_,L"?memory?",features);
    }

  _TEGGO_STREAMS_FAKE_INLINE
  MemDataStream::MemDataStream(void const *data, unsigned len, int features)
    :BaseFileDataStream(&_MEM_fdstream_api)
    {
      memdesc_.pS = (byte_t*)data;
      memdesc_.pE = memdesc_.pS + len;
      memdesc_.pI = memdesc_.pS;
      is_unicode_mem_stream_ = !!(features & FILEOPEN_UNICODE_MEMSTREAM);
      _Init(&memdesc_,L"?memory?",features&~FILEOPEN_WRITE);
    }

  _TEGGO_STREAMS_FAKE_INLINE
  MemDataStream::~MemDataStream()
    {
      Close();
    }

  _TEGGO_STREAMS_FAKE_INLINE
  u32_t MemDataStream::Features()
    {
      return BaseFileDataStream::Features() |
        ( is_unicode_mem_stream_ ? IS_UNICODE_MEMSTREAM : 0 );
    }


  CXX_FAKE_INLINE
  int __cdecl _hf_stream_read(
    void *buf, u32_t size, u32_t count, HANDLE fh)
      {
        unsigned l = size*count;
        for ( int j = 0; j < l; )
          {
            unsigned long wr = 0;
            if ( !ReadFile(fh,(char*)buf+j,l-j,&wr,0) )
              return -1;
            else
              j+=wr;
            if ( wr == 0 )
              return (j)/size;
          }
        return size*count;
      }

  CXX_FAKE_INLINE
  int __cdecl _hf_stream_write(
    void *buf, u32_t size, u32_t count, HANDLE fh)
      {
        size = size*count;
        for ( int j = 0; j < size; )
          {
            unsigned long wr = 0;
            if ( !WriteFile(fh,(char*)buf+j,size-j,&wr,0) )
              {
                return -1;
              }
            else
              j+=wr;
            if ( wr == 0 )
              return (j)/size;
          }
        return count;
      }

  CXX_FAKE_INLINE
  int __cdecl _hf_stream_fseek(
    HANDLE fh, i32_t offs, int orign)
    {
      switch(orign)
        {
          case SEEK_SET: orign = FILE_BEGIN; break;
          case SEEK_CUR: orign = FILE_CURRENT; break;
          case SEEK_END: orign = FILE_END; break;
        }
      if ( INVALID_SET_FILE_POINTER == SetFilePointer(fh,offs,0,orign) )
        return -1;
      return 0;
    }

  CXX_FAKE_INLINE
  i32_t __cdecl _hf_stream_ftell(HANDLE fh)
    {
      return SetFilePointer(fh,0,0,FILE_CURRENT);
    }

  CXX_FAKE_INLINE
  int __cdecl _hf_stream_feof(HANDLE fh)
    {
      return SetFilePointer(fh,0,0,FILE_CURRENT) == GetFileSize(fh,0);
    }

  CXX_FAKE_INLINE
  int __cdecl _hf_stream_flush(HANDLE fh)
    {
      return FlushFileBuffers(fh);
    }

  CXX_FAKE_INLINE
  void __cdecl _hf_stream_fclose(HANDLE fh)
    {
      CloseHandle(fh);
    }

  static _fdstream_api_t const _HF_fdstream_api =
    {
      (_fdstream_fread_t)_hf_stream_read,
      (_fdstream_fwrite_t)_hf_stream_write,
      (_fdstream_fseek_t)_hf_stream_fseek,
      (_fdstream_ftell_t)_hf_stream_ftell,
      (_fdstream_fflush_t)_hf_stream_flush,
      (_fdstream_fclose_t)_hf_stream_fclose,
      (_fdstream_feof_t)_hf_stream_feof
    };

  _TEGGO_STREAMS_FAKE_INLINE
  HandleDataStream::HandleDataStream(HANDLE fh, StringParam fname, int dupit, int opt)
    : BaseFileDataStream(&_HF_fdstream_api)
    {
      HANDLE ff = INVALID_HANDLE_VALUE;
      if (dupit)
        DuplicateHandle(GetCurrentProcess(),fh,GetCurrentProcess(),&ff,0,FALSE,DUPLICATE_SAME_ACCESS);
      else
        ff = fh;

      if ( ff != INVALID_HANDLE_VALUE )
        {
          _Init(ff,+fname,opt);
          return;
        }

      error_ = TEGGO_FAIL;
    }

  _TEGGO_STREAMS_FAKE_INLINE
  HandleDataStream::HandleDataStream(StringParam fname, unsigned flags)
    : BaseFileDataStream(&_HF_fdstream_api)
    {
      enum { MAGA_MAX_PATH = 4096 };
      canread_ = canwrite_ = canseek_ = false;
      BufferT<wchar_t> pathbuf(MAGA_MAX_PATH);
      int n = GetFullPathNameW(+fname,MAGA_MAX_PATH,&pathbuf[0],0);
      if ( n && n <= MAGA_MAX_PATH )
        {

          fullpath_.SetRange(&pathbuf[0],n);

          u32_t accs  =
             (flags&(STREAM_CREATE|STREAM_WRITE|STREAM_APPEND)?(GENERIC_WRITE|GENERIC_READ):GENERIC_READ)
            |((flags&STREAM_EXECUTABLE)?GENERIC_EXECUTE:0);

          u32_t dispo = (flags&STREAM_CREATE)?CREATE_ALWAYS:OPEN_EXISTING;
          HANDLE ff = CreateFileW( fullpath_.Str(), accs, FILE_SHARE_READ, 0, dispo, 0, 0 );
          
          if ( ff && ff != INVALID_HANDLE_VALUE )
            {
              if ( flags & STREAM_NONINHERITABLE )
                SetHandleInformation(ff,HANDLE_FLAG_INHERIT,0);
              _Init(ff,+fname,flags);
              return;
            }
        }

      error_ = TEGGO_NOFILE;
    }

  _TEGGO_STREAMS_FAKE_INLINE
  HandleDataStream::~HandleDataStream()
    {
      Close();
    }

    _TEGGO_STREAMS_FAKE_INLINE
  BaseFileDataStream::~BaseFileDataStream()
    {
      //BaseFileDataStream::Close();
    }

  _TEGGO_STREAMS_FAKE_INLINE
  bool BaseFileDataStream::IsClosed()
    {
      return !f_;
    }

  _TEGGO_STREAMS_FAKE_INLINE
  TEGGOt_E BaseFileDataStream::Flush()
    {
      if ( f_ )
        {
          if ( canwrite_ )
            {
              api_->f_flush(f_);
              return TEGGO_OK;
            }
          else
            return error_ = TEGGO_UNSUPPORTED;
        }
       else
        return error_ = TEGGO_NOFILE;
    }

  _TEGGO_STREAMS_FAKE_INLINE
  TEGGOt_E BaseFileDataStream::Close()
    {
      if ( f_ )
        {
          api_->f_close(f_);
          f_ = 0;
          return TEGGO_OK;
        }
       else
        return error_ = TEGGO_NOFILE;
    }

  _TEGGO_STREAMS_FAKE_INLINE
  TEGGOt_E BaseFileDataStream::Seek(i32_t pos, int opt)
    {
      if ( f_ )
        {
          if ( canseek_ )
            {
              if ( 0 != api_->f_seek(f_, pos, opt) )
                return error_ = TEGGO_FAIL;
              else
                return TEGGO_OK;
            }
          else return error_ = TEGGO_UNSUPPORTED;
        }
       else return error_ = TEGGO_NOFILE;
    }

  _TEGGO_STREAMS_FAKE_INLINE
  BaseFileDataStream::offset_t BaseFileDataStream::Tell()
    {
      if ( f_ )
        {
          if ( canseek_ )
            {
              offset_t q = (offset_t)api_->f_tell(f_);
              return q;
            }
          else
            return error_ = TEGGO_UNSUPPORTED, 0;
        }
       else
        return error_ = TEGGO_NOFILE, 0;
    }

  _TEGGO_STREAMS_FAKE_INLINE
  BaseFileDataStream::offset_t BaseFileDataStream::Available()
    {
      if ( canread_ )
        {
          offset_t pS = api_->f_tell(f_); api_->f_seek(f_,0,SEEK_END);
          offset_t pE = api_->f_tell(f_); api_->f_seek(f_,pS,SEEK_SET);
          return pE-pS;
        }
      else
        return 0;
    }

  _TEGGO_STREAMS_FAKE_INLINE
  int BaseFileDataStream::Write( void const *buf, int count )
    {
      if ( !f_ )
        return error_ = TEGGO_NOFILE, 0;
      if ( !canwrite_ )
        return error_ = TEGGO_NOACCESS, 0;
      int k = api_->f_write(buf,1,count,f_);
      if ( k < 0 )
        return error_ = TEGGO_FAIL, 0;
      return k;
    }

  _TEGGO_STREAMS_FAKE_INLINE
  int BaseFileDataStream::Read( void *buf, int count )
    {
      if ( !f_ )
        return error_ = TEGGO_NOFILE, 0;
      if ( !canread_ )
        return error_ = TEGGO_NOACCESS, 0;

      int k = 0;

      for (int u = teggo_abs(count); k != u;)
        {
          int j = api_->f_read((byte_t*)buf+k,1,u-k,f_);
          if ( j > 0 )
            k +=j;
          else
            {
              if ( !k ) k = j;
              break;
            }
        }

      if ( k <= 0 )
        error_ = api_->f_eof(f_) ? TEGGO_NODATA : TEGGO_FAIL;
//       else if ( k < count && api_->f_eof(f_) )
//         error_ = TEGGO_NODATA;

      if ( k < 0 ) return 0;

      if ( count < 0 && k != teggo_abs(count) )
        {
          if ( canseek_ )
            api_->f_seek(f_,-k,SEEK_CUR);
          error_ = TEGGO_NODATA;
        }

      return k;
    }

#if defined _TEGGOINLINE
  template < unsigned tFake > DataSourceFilter *DataSource_<tFake>::first = 0;
  template < unsigned tFake > bool DataSource_<tFake>::fs_search = true;
#elif defined _TEGGO_STREAMS_HERE
  DataSourceFilter *DataSource_<0>::first = 0;
  bool DataSource_<0>::fs_search = true;
#endif

  _TEGGO_STREAMS_FAKE_INLINE
  bool CXX_STDCALL NextDataStreamIdentifier(
    pwide_t idlst, pwide_t *i, BaseStringT<wchar_t> &id, int *opt)
    {
      if ( *i )
        {
        mk_id:
          if( **i != 0 )
            {
              id.SetRange(idlst,ChrFind(idlst,'|'));
              pwide_t pc = ChrFind(*i+1,'&','|');
              pwide_t p  = pc;
              if ( *p == '&' ) p = ChrFind(p,'|');
              id.AppendRange(*i+1,pc);
              if ( opt )
                *opt = ( p != pc ) ? ParseFopenOpt(pc+1) : 0;
              *i = p;
              return true;
            }
          else
            return false;
        }
      else
        {
          *i = ChrFind(idlst,'|');
          if ( **i )
            goto mk_id;
          else
            {
              id.Set(idlst);
              if ( opt ) *opt = 0;
              return true;
            }
        }
    }

  _TEGGO_STREAMS_FAKE_INLINE
  rcc_ptr<DataStream> CXX_STDCALL OpenStream( StringParam fname , unsigned flags )
    {
      unsigned f_len = fname.Length();
      rcc_ptr<DataStream> ds = RccPtr(
        f_len < MAX_PATH ? (DataStream*) new FileDataStream(fname,flags)
                         : (DataStream*) new HandleDataStream(fname,flags) );
      return ( !ds->Error() ) ? ds : rcc_ptr<DataStream>(0);
    }

  _TEGGO_STREAMS_FAKE_INLINE
  rcc_ptr<DataStream> CXX_STDCALL CreateDataStreamFromFILE( FILE* fd, StringParam fname, int dupit, int opt )
    {
      rcc_ptr<DataStream> ds = RccPtr( (DataStream*) new FdDataStream(fd,+fname,dupit,opt) );
      return ( !ds->Error() ) ? ds : rcc_ptr<DataStream>(0);
    }

  _TEGGO_STREAMS_FAKE_INLINE
  rcc_ptr<DataStream> CXX_STDCALL CreateDataStreamFromHANDLE( HANDLE fd, StringParam fname, int dupit, int opt )
    {
      rcc_ptr<DataStream> ds = RccPtr( (DataStream*) new HandleDataStream(fd,+fname,dupit,opt) );
      return ( !ds->Error() ) ? ds : rcc_ptr<DataStream>(0);
    }

  _TEGGO_STREAMS_FAKE_INLINE
  int CXX_STDCALL FileExists( StringParam fname )
    {
      struct _stat s = {0};
      if ( 0 == _wstat(fname,&s) )
        {
          return (s.st_mode&_S_IFDIR) ? DS_DIRECTORY
                :(s.st_mode&_S_IFREG) ? DS_REGULAR
                :DS_NOEXISTS;
        }
      return DS_NOEXISTS;
    }

  _TEGGO_STREAMS_FAKE_INLINE
  time_t CXX_STDCALL MtimeOfFile( StringParam fname )
    {
      struct _stat s = {0};
      if ( 0 == _wstat(fname,&s) )
        return u32_t(s.st_mtime);
      return u32_t(0);
    }

  _TEGGO_STREAMS_FAKE_INLINE
  time_t CXX_STDCALL CtimeOfFile( StringParam fname )
    {
      struct _stat s = {0};
      if ( 0 == _wstat(fname,&s) )
        return u32_t(s.st_ctime);
      return u32_t(0);
    }
    
  _TEGGO_STREAMS_FAKE_INLINE
  DataStream::offset_t CXX_STDCALL LengthOfFile( StringParam fname )
    {
      struct _stat s = {0};
      if ( 0 == _wstat(fname,&s) )
        return DataStream::offset_t(s.st_size);
      return DataStream::offset_t(0);
    }

  _TEGGO_STREAMS_FAKE_INLINE
  rcc_ptr<DataStream> CXX_STDCALL DataSource_<0>::Open( StringParam identifier, unsigned flgs )
    {
      for ( DataSourceFilter *f = first; f; f= f->next )
        if ( rcc_ptr<DataStream> ds = f->Open(identifier,flgs) )
          return ds;

      if ( fs_search )
        {
          StringT<wchar_t,128> id;
          int opt;
          for ( pwide_t i = 0; NextDataStreamIdentifier( identifier, &i, id, &opt ); )
            if ( (FileExists( +id ) & DS_REGULAR) || (!opt && (flgs&FILEOPEN_CREATE)) )
              return OpenStream( +id, opt ? opt : flgs );
        }

      return rcc_ptr<DataStream>(0);
    }

  _TEGGO_STREAMS_FAKE_INLINE
  int CXX_STDCALL DataSource_<0>::Exists( StringParam identifier )
    {
      for ( DataSourceFilter *f = first; f; f= f->next )
        if ( int e = f->Exists(identifier) )
          return e;

      if ( fs_search )
        {
          StringT<wchar_t,128> id;
          for ( pwide_t i = 0; NextDataStreamIdentifier( identifier, &i, id ); )
            if ( int e = FileExists( +id ) )
              return e;
        }

      return DS_NOEXISTS;
    }

  _TEGGO_STREAMS_FAKE_INLINE
  time_t CXX_STDCALL DataSource_<0>::MtimeOf( StringParam identifier )
    {
      for ( DataSourceFilter *f = first; f; f= f->next )
        if ( u32_t mt = f->MtimeOf(identifier) )
          return mt;

      if ( fs_search )
        {
          StringT<wchar_t,128> id;
          for ( pwide_t i = 0; NextDataStreamIdentifier( identifier, &i, id ); )
            if ( u32_t mt = MtimeOfFile( +id ) )
              return mt;
        }

      return 0;
    }

  _TEGGO_STREAMS_FAKE_INLINE
  time_t CXX_STDCALL DataSource_<0>::CtimeOf( StringParam identifier )
    {
      for ( DataSourceFilter *f = first; f; f= f->next )
        if ( u32_t mt = f->CtimeOf(identifier) )
          return mt;

      if ( fs_search )
        {
          StringT<wchar_t,128> id;
          for ( pwide_t i = 0; NextDataStreamIdentifier( identifier, &i, id ); )
            if ( u32_t mt = CtimeOfFile( +id ) )
              return mt;
        }

      return 0;
    }
    
  _TEGGO_STREAMS_FAKE_INLINE
  DataStream::offset_t CXX_STDCALL DataSource_<0>::Length( StringParam identifier )
    {
      for ( DataSourceFilter *f = first; f; f= f->next )
        if ( f->Exists(identifier) & DS_REGULAR )
          return f->Length(identifier);

      if ( fs_search )
        {
          StringT<wchar_t,128> id;
          for ( pwide_t i = 0; NextDataStreamIdentifier( identifier, &i, id ); )
            if ( FileExists(+id) )
              return LengthOfFile( +id );
        }

      return DataStream::offset_t(-1);
    }

  _TEGGO_STREAMS_FAKE_INLINE
  void *CXX_STDCALL DataSource_<0>::Mmap(StringParam identifier, u32_t size, u32_t offset, bool writable, bool create_new)
    {
      return SysMmapFile(identifier,size,offset,writable,create_new);
    }

  _TEGGO_STREAMS_FAKE_INLINE
  void  CXX_STDCALL DataSource_<0>::Unmmap(void const *p)
    {
      SysUnmmapFile(p);
    }

  _TEGGO_STREAMS_FAKE_INLINE
  TEGGOt_E  CXX_STDCALL DataSource_<0>::ReadAllBytesFrom(StringParam identifier, BufferT<byte_t> &dta )
    {
      if ( rcc_ptr<DataStream> ds = Open( identifier ) )
        return ds->Read(dta,-1), ds->Error();
      else
        return TEGGO_NOFILE;
    }

  _TEGGO_STREAMS_FAKE_INLINE
  void CXX_STDCALL DataSource_<0>::EnableFsSearch(bool enable)
    {
      fs_search = enable;
    }

  _TEGGO_STREAMS_FAKE_INLINE
  void CXX_STDCALL DataSource_<0>::UnregisterAll()
    {
      while ( first )
        {
          DataSourceFilter *dsf = first;
          first = first->next;
          dsf->Release();
        }
    }

  _TEGGO_STREAMS_FAKE_INLINE
  void CXX_STDCALL DataSource_<0>::RegisterSourceFilter( rcc_ptr<DataSourceFilter> sf )
    {

      REQUIRE( sf->next == 0 );
      DataSourceFilter **dsf = &first;

      for(;;)
        {
          if ( !*dsf || (*dsf)->prior < sf->prior )
            {
              sf->next = *dsf;
              *dsf = Refe(sf);
              break;
            }
          else
            dsf = &(*dsf)->next;
        }

    }

  _TEGGO_STREAMS_FAKE_INLINE
  DataSourceFilter::~DataSourceFilter() {}

  _TEGGO_STREAMS_FAKE_INLINE
  unsigned CXX_STDCALL ParseFopenOpt(StringParam optarg)
    {
      unsigned o = 0;
      pwide_t opt = +optarg;
      for ( ; *opt; ++opt )
        {
          if ( *opt == L'w' )
            o |= FILEOPEN_CREATE;
          else if ( *opt == L'r' )
            o |= FILEOPEN_READ;
          else if ( *opt == '+' )
            o |= FILEOPEN_WRITE;
          else if ( *opt == 't' )
            ; // skip
          else if ( *opt == 'b' )
            o |= 0;
          else if ( *opt == 'a' )
            o |= FILEOPEN_APPEND;
          else
            break;
        }
      return o;
    }


  CXX_FAKE_INLINE
  int __cdecl _array_stream_fake()
    {
      return 0;
    }

  CXX_FAKE_INLINE
  int __cdecl _array_stream_read(
    void *buf, u32_t size, u32_t count, ArrayDataStream *ads)
    {
      int l = ads->buffer_.Count()-ads->p_;
      l = cxx_min<int>(l,size*count);
      l = (l/size)*size;
      if ( l != 0 )
        {
          memcpy(buf,+ads->buffer_+ads->p_,l);
          ads->p_+=l;
        }
      return l;
    }

  CXX_FAKE_INLINE
  int __cdecl _array_stream_write(
    void *buf, u32_t size, u32_t count, ArrayDataStream *ads)
    {
      int l = size*count;
      if ( ads->p_ + l > ads->buffer_.Count() )
        ads->buffer_.Resize( ads->p_ + l );
      memcpy(+ads->buffer_+ads->p_,buf,l);
      ads->p_+=l;
      return l;
    }

  CXX_FAKE_INLINE
  int __cdecl _array_stream_fseek(
    ArrayDataStream *ads, i32_t offs, int orign)
    {
      if ( orign == SEEK_SET )
        {
          ads->p_ = offs;
          return 0;
        }
      else if ( orign == SEEK_CUR )
        {
          ads->p_ += offs;
          return 0;
        }
      else if ( orign == SEEK_END )
        {
          if ( (int)ads->buffer_.Count() + offs < 0 )
            return -1;
          else
            ads->p_ = (int)ads->buffer_.Count()+offs;
          return 0;
        }
      return -1;
    }

  CXX_FAKE_INLINE
  i32_t __cdecl _array_stream_ftell(ArrayDataStream *ads)
    {
      return ads->p_;
    }

  CXX_FAKE_INLINE
  int __cdecl _array_stream_feof(ArrayDataStream *ads)
    {
      return ads->p_ >= ads->buffer_.Count();
    }


  static _fdstream_api_t const _ARRAY_fdstream_api =
    {
      (_fdstream_fread_t)_array_stream_read,
      (_fdstream_fwrite_t)_array_stream_write,
      (_fdstream_fseek_t)_array_stream_fseek,
      (_fdstream_ftell_t)_array_stream_ftell,
      (_fdstream_fflush_t)_array_stream_fake,
      (_fdstream_fclose_t)_array_stream_fake,
      (_fdstream_feof_t)_array_stream_feof
    };

  _TEGGO_STREAMS_FAKE_INLINE
  ArrayDataStream::ArrayDataStream(unsigned reserved)
    :BaseFileDataStream(&_ARRAY_fdstream_api)
    {
      if (reserved) buffer_.Reserve(reserved);
      p_ = 0;
      _Init(this,L"?array?",FILEOPEN_WRITE);
    }

  _TEGGO_STREAMS_FAKE_INLINE
  ArrayDataStream::~ArrayDataStream()
    {
    }

  _TEGGO_STREAMS_FAKE_INLINE
  void ArrayDataStream::SwapBuffer(BufferT<byte_t> &b)
    {
      Seek(0);
      buffer_.Swap(b);
    }

    
  _TEGGO_STREAMS_FAKE_INLINE
  TEGGOt_E DataStream::UtfPrint(FormatT<wchar_t> const &f) 
    { 
      return UtfPrint(f.Str()); 
    }
    
  _TEGGO_STREAMS_FAKE_INLINE
  TEGGOt_E DataStream::Print(FormatT<char> const &f) 
    { 
      return Print(f.Str()); 
    }
    
} // namespace

#endif // ___1f0b46f2_1025_41d0_9967_2a3bcb1f266c___
