
/*

  Copyright (c) 2005-2009, Alexey Sudachen, alexey@sudachen.name

*/

#if !defined ___945f2106_67c3_4f28_81ba_079b4dfde6c1___
#define ___945f2106_67c3_4f28_81ba_079b4dfde6c1___

#include "_specific.h"
#include "array.h"
#include "refcounted.h"
#include "ptr_rcc.h"
#include "logger.h"

namespace teggo {

  struct CXX_NO_VTABLE DataStream
    {
      virtual u32_t Release() = 0;
      virtual u32_t AddRef() = 0;

      typedef u32_t offset_t;

      virtual int Read(void *buf, int count) = 0;
      _TEGGO_EXPORTABLE int ReadText(char *buf, int count);
      _TEGGO_EXPORTABLE char * ReadLine_(char *buf, int count);
      _TEGGO_EXPORTABLE int UtfReadText(wchar_t *buf, int count);
      _TEGGO_EXPORTABLE wchar_t * UtfReadLine_(wchar_t *buf, int count);
      _TEGGO_EXPORTABLE TEGGOt_E Read(BufferT<byte_t> &buf, int count = -1);
      _TEGGO_EXPORTABLE TEGGOt_E ReadTextEx(BufferT<char> &buf, int count = -1);
      _TEGGO_EXPORTABLE TEGGOt_E ReadTextS(BaseStringT<char> &buf, int count = -1);
      _TEGGO_EXPORTABLE TEGGOt_E ReadLineEx(BufferT<char> &buf);
      _TEGGO_EXPORTABLE TEGGOt_E ReadLineS(BaseStringT<char> &buf);
      _TEGGO_EXPORTABLE StringT<char> ReadLine();
      _TEGGO_EXPORTABLE TEGGOt_E UtfReadTextEx(BufferT<wchar_t> &buf, int count = -1);
      _TEGGO_EXPORTABLE TEGGOt_E UtfReadTextS(BaseStringT<wchar_t> &buf, int count = -1);
      _TEGGO_EXPORTABLE TEGGOt_E UtfReadLineEx(BufferT<wchar_t> &buf);
      _TEGGO_EXPORTABLE TEGGOt_E UtfReadLineS(BaseStringT<wchar_t> &buf);
      _TEGGO_EXPORTABLE StringT<wchar_t> UtfReadLine();
      virtual int Write(void const *buf, int count) = 0;
      _TEGGO_EXPORTABLE int WriteText(char const *buf, int count);
      _TEGGO_EXPORTABLE int UtfWriteText(wchar_t const *buf, int count);
      _TEGGO_EXPORTABLE TEGGOt_E Print(pchar_t s);
      _TEGGO_EXPORTABLE TEGGOt_E Print(FormatT<char> const &f);
      _TEGGO_EXPORTABLE int Printf(pchar_t fmt,...);
      _TEGGO_EXPORTABLE int Vprintf(pchar_t fmt,va_list);
      _TEGGO_EXPORTABLE TEGGOt_E UtfPrint(pwide_t s);
      _TEGGO_EXPORTABLE TEGGOt_E UtfPrint(FormatT<wchar_t> const &f);
      _TEGGO_EXPORTABLE int UtfPrintf(pwide_t fmt,...);
      _TEGGO_EXPORTABLE int UtfVprintf(pwide_t fmt,va_list);

      _TEGGO_EXPORTABLE TEGGOt_E Skip(unsigned cnt);
      _TEGGO_EXPORTABLE TEGGOt_E SkipLine(int cnt, int *skipped = 0);
      _TEGGO_EXPORTABLE TEGGOt_E SkipChar(int cnt, int *skipped = 0);
      _TEGGO_EXPORTABLE TEGGOt_E UtfSkipLine(int cnt, int *skipped = 0);
      _TEGGO_EXPORTABLE TEGGOt_E UtfSkipChar(int cnt, int *skipped = 0);
      _TEGGO_EXPORTABLE TEGGOt_E Ungetc(char c = 0);
      _TEGGO_EXPORTABLE TEGGOt_E UtfReadChar(wchar_t *c);
      _TEGGO_EXPORTABLE TEGGOt_E UtfUngetc(wchar_t c = 0);
      _TEGGO_EXPORTABLE TEGGOt_E MD5(u32_t bytes_count, void *md5);

      enum
        {
          CAN_READ    = TEGGO_BIT(1),
          CAN_WRITE   = TEGGO_BIT(2),
          CAN_SEEK    = TEGGO_BIT(3)|TEGGO_BIT(4)|TEGGO_BIT(5),
          CAN_REWIND  = TEGGO_BIT(4),
          CAN_UNGETC  = TEGGO_BIT(5),
          NEED_FLUSH  = TEGGO_BIT(10),
          NEED_CLOSE  = TEGGO_BIT(11),
          IS_BUFFERED = TEGGO_BIT(20),
          IS_REGULAR  = TEGGO_BIT(21),
          HAS_SIZE    = TEGGO_BIT(25),
          IS_UNICODE_MEMSTREAM = TEGGO_BIT(26),
        };
      virtual u32_t Features() = 0;

      _TEGGO_EXPORTABLE DataStream();
      _TEGGO_EXPORTABLE virtual ~DataStream();

      virtual offset_t Available() = 0;
      virtual offset_t Tell() = 0;
      virtual TEGGOt_E Seek(i32_t,int opt=0) = 0;
      virtual TEGGOt_E Seek64(i64_t,int opt=0) { return TEGGO_UNSUPPORTED; }
      virtual TEGGOt_E Flush() = 0;
      virtual TEGGOt_E Close() = 0;
      virtual bool IsClosed() = 0;

      _TEGGO_EXPORTABLE virtual TEGGOt_E Rewind();

      virtual StringT<wchar_t> Identifier() = 0;

      virtual TEGGOt_E Error() = 0;
      virtual TEGGOt_E ClearError() = 0;
      inline bool Eof() { return Error() == TEGGO_NODATA; }

      _TEGGO_EXPORTABLE virtual offset_t Size();

      _TEGGO_EXPORTABLE TEGGOt_E Read32be(u32_t*);
      _TEGGO_EXPORTABLE TEGGOt_E Read32le(u32_t*);
      _TEGGO_EXPORTABLE TEGGOt_E Read16be(u16_t*);
      _TEGGO_EXPORTABLE TEGGOt_E Read16le(u16_t*);
      _TEGGO_EXPORTABLE TEGGOt_E Read8(uc8_t*);
      _TEGGO_EXPORTABLE TEGGOt_E ReadUtf(wchar_t*);
      _TEGGO_EXPORTABLE TEGGOt_E ReadIeee32be(float*);
      _TEGGO_EXPORTABLE TEGGOt_E ReadIeee32le(float*);

      inline u32_t Read32be() { u32_t v = 0; return Read32be(&v), v; }
      inline u32_t Read32le() { u32_t v = 0; return Read32le(&v), v; }
      inline u16_t Read16be() { u16_t v = 0; return Read16be(&v), v; }
      inline u16_t Read16le() { u16_t v = 0; return Read16le(&v), v; }
      inline uc8_t Read8()    { uc8_t v = 0; return Read8(&v), v; }
      inline wchar_t ReadUtf() { wchar_t v = 0; return ReadUtf(&v), v; }
      inline float ReadIeee32be() { float v = 0; return ReadIeee32be(&v), v; }
      inline float ReadIeee32le() { float v = 0; return ReadIeee32le(&v), v; }

      _TEGGO_EXPORTABLE TEGGOt_E Write32be(u32_t);
      _TEGGO_EXPORTABLE TEGGOt_E Write32le(u32_t);
      _TEGGO_EXPORTABLE TEGGOt_E Write16be(u16_t);
      _TEGGO_EXPORTABLE TEGGOt_E Write16le(u16_t);
      _TEGGO_EXPORTABLE TEGGOt_E Write8(uc8_t);  
      inline TEGGOt_E WriteByte(unsigned b) { return Write8((byte_t)b); }
      _TEGGO_EXPORTABLE TEGGOt_E WriteUtf(wchar_t);
      _TEGGO_EXPORTABLE TEGGOt_E WriteIeee32be(float);
      _TEGGO_EXPORTABLE TEGGOt_E WriteIeee32le(float);
      //_TEGGO_EXPORTABLE offset_t Size();
      
      virtual void *GetHandle(int kind) { return 0; }
    };

  struct CXX_NO_VTABLE BaseDataStream_ : DataStream
    {
      typedef DataStream::offset_t offset_t;
      TEGGOt_E error_;
      _TEGGO_EXPORTABLE int Read( void *buf, int count );
      _TEGGO_EXPORTABLE int Write( void const *buf, int count );
      _TEGGO_EXPORTABLE u32_t Features();

      _TEGGO_EXPORTABLE BaseDataStream_();
      _TEGGO_EXPORTABLE ~BaseDataStream_();

      _TEGGO_EXPORTABLE offset_t Available();
      _TEGGO_EXPORTABLE offset_t Tell();
      _TEGGO_EXPORTABLE TEGGOt_E Seek(i32_t, int opt);
      _TEGGO_EXPORTABLE TEGGOt_E Flush();
      _TEGGO_EXPORTABLE TEGGOt_E Close();
      _TEGGO_EXPORTABLE TEGGOt_E Error();
      _TEGGO_EXPORTABLE TEGGOt_E ClearError();
      _TEGGO_EXPORTABLE bool IsClosed();
    };

  struct NullDataStream : RefcountedT<DataStream>
    {
      typedef DataStream::offset_t offset_t;
      _TEGGO_EXPORTABLE NullDataStream();
      _TEGGO_EXPORTABLE virtual ~NullDataStream();
      _TEGGO_EXPORTABLE virtual int Read( void *buf, int count );
      _TEGGO_EXPORTABLE virtual int Write( void const *buf, int count );
      _TEGGO_EXPORTABLE virtual u32_t Features();
      _TEGGO_EXPORTABLE virtual offset_t Available();
      _TEGGO_EXPORTABLE virtual offset_t Tell();
      _TEGGO_EXPORTABLE virtual TEGGOt_E Seek(i32_t, int opt);
      _TEGGO_EXPORTABLE virtual TEGGOt_E Flush();
      _TEGGO_EXPORTABLE virtual TEGGOt_E Close();
      _TEGGO_EXPORTABLE virtual TEGGOt_E Error();
      _TEGGO_EXPORTABLE virtual TEGGOt_E ClearError();
      _TEGGO_EXPORTABLE virtual StringT<wchar_t> Identifier();
      _TEGGO_EXPORTABLE virtual bool IsClosed();
    };

  struct LogDataStream : RefcountedT<DataStream>
    {
      typedef DataStream::offset_t offset_t;
      _T_logger_out_ lout;

      _TEGGO_EXPORTABLE LogDataStream(_T_logger_out_ lo);
      _TEGGO_EXPORTABLE virtual ~LogDataStream();
      _TEGGO_EXPORTABLE virtual int Read( void *buf, int count );
      _TEGGO_EXPORTABLE virtual int Write( void const *buf, int count );
      _TEGGO_EXPORTABLE virtual u32_t Features();
      _TEGGO_EXPORTABLE virtual offset_t Available();
      _TEGGO_EXPORTABLE virtual offset_t Tell();
      _TEGGO_EXPORTABLE virtual TEGGOt_E Seek(i32_t, int opt);
      _TEGGO_EXPORTABLE virtual TEGGOt_E Flush();
      _TEGGO_EXPORTABLE virtual TEGGOt_E Close();
      _TEGGO_EXPORTABLE virtual TEGGOt_E Error();
      _TEGGO_EXPORTABLE virtual TEGGOt_E ClearError();
      _TEGGO_EXPORTABLE virtual StringT<wchar_t> Identifier();
      _TEGGO_EXPORTABLE virtual bool IsClosed();
    };

  enum DSt_ATTRIBUTES
    {
      DS_NOEXISTS = 0,
      DS_REGULAR = 1,
      DS_DIRECTORY = 2
    };

  struct CXX_NO_VTABLE DataSourceFilter : Refcounted
    {
      typedef DataStream::offset_t offset_t;
      int prior;
      DataSourceFilter *next;
      virtual rcc_ptr<DataStream> Open( pwide_t identifier, unsigned flgs ) = 0;
      virtual int   Exists( pwide_t identifier ) = 0;
      virtual time_t MtimeOf( pwide_t identifier ) = 0;
      virtual time_t CtimeOf( pwide_t identifier ) = 0;
      virtual offset_t Length( pwide_t identifier ) = 0;
      DataSourceFilter(int prior = 0) : prior(prior), next(0) {}
      _TEGGO_EXPORTABLE virtual ~DataSourceFilter();
    };

  template < unsigned tFake >
    struct DataSource_
      {

        typedef DataStream::offset_t offset_t;
        static DataSourceFilter *first;
        static bool fs_search;

        struct Gate
          {
            DataSource_ *operator ->() const { return 0; }
          };

        _TEGGO_EXPORTABLE static void CXX_STDCALL RegisterSourceFilter( rcc_ptr<DataSourceFilter> sf );
        _TEGGO_EXPORTABLE static rcc_ptr<DataStream> CXX_STDCALL Open( StringParam identifier, unsigned flgs = 0 );
        _TEGGO_EXPORTABLE static int   CXX_STDCALL Exists( StringParam identifier );
        _TEGGO_EXPORTABLE static offset_t CXX_STDCALL Length( StringParam identifier );
        _TEGGO_EXPORTABLE static void *CXX_STDCALL Mmap(StringParam identifier, u32_t size = 0, u32_t offset = 0, bool writable = false, bool create_new = false);
        _TEGGO_EXPORTABLE static void  CXX_STDCALL Unmmap(void const*);
        //_TEGGO_EXPORTABLE static u32_t CXX_STDCALL LastMtimeOf( StringParam identifier ); // compatbility
        _TEGGO_EXPORTABLE static time_t CXX_STDCALL MtimeOf( StringParam identifier );
        _TEGGO_EXPORTABLE static time_t CXX_STDCALL CtimeOf( StringParam identifier );
        _TEGGO_EXPORTABLE static TEGGOt_E CXX_STDCALL ReadAllBytesFrom(StringParam identifier, BufferT<byte_t> &dta );
        _TEGGO_EXPORTABLE static void CXX_STDCALL EnableFsSearch(bool enable);
        _TEGGO_EXPORTABLE static void CXX_STDCALL UnregisterAll();

      };

  static const DataSource_<0>::Gate DataSource = DataSource_<0>::Gate();

  _TEGGO_EXPORTABLE bool CXX_STDCALL NextDataStreamIdentifier(
    pwide_t idlst,pwide_t *i, BaseStringT<wchar_t> &id, int *opt = 0);

  _TEGGO_EXPORTABLE rcc_ptr<DataStream> CXX_STDCALL OpenStream(StringParam fname , unsigned flags );
  inline rcc_ptr<DataStream> OpenFile(StringParam fname , unsigned flags ) { return OpenStream(fname,flags); }
  _TEGGO_EXPORTABLE int   CXX_STDCALL FileExists(StringParam fname );
  //_TEGGO_EXPORTABLE u32_t CXX_STDCALL LastMtimeOfFile(StringParam identifier ); // compatibility
  _TEGGO_EXPORTABLE time_t CXX_STDCALL MtimeOfFile(StringParam identifier );
  _TEGGO_EXPORTABLE time_t CXX_STDCALL CtimeOfFile(StringParam identifier );
  _TEGGO_EXPORTABLE DataStream::offset_t CXX_STDCALL LengthOfFile(StringParam identifier );
  _TEGGO_EXPORTABLE unsigned CXX_STDCALL ParseFopenOpt(StringParam opt);

  _TEGGO_EXPORTABLE rcc_ptr<DataStream> CXX_STDCALL CreateDataStreamFromFILE( FILE* fd, StringParam fname, int dupit, int opt );
  _TEGGO_EXPORTABLE rcc_ptr<DataStream> CXX_STDCALL CreateDataStreamFromHANDLE( HANDLE fd, StringParam fname, int dupit, int opt );

  enum { RTRY_COUNT = 10, RTRY_TIMEOUT = 300 };

  inline rcc_ptr<DataStream> CXX_STDCALL OpenStreamRtry(StringParam fname, unsigned flags, int count = RTRY_COUNT, int timeout = RTRY_TIMEOUT )
    {
      for ( int i = 0; i < count; ++i )
        {
          if ( rcc_ptr<DataStream> ds = OpenStream( fname, flags ) )
            return ds;
          else
            Sleep(timeout);
        }

      return rcc_ptr<DataStream>(0);
    }

  typedef int   (__cdecl *_fdstream_fread_t)(void *, u32_t, u32_t, void *);
  typedef int   (__cdecl *_fdstream_fwrite_t)(void const*, u32_t, u32_t, void *);
  typedef int   (__cdecl *_fdstream_fseek_t)(void *, i32_t, int);
  typedef i32_t (__cdecl *_fdstream_ftell_t)(void *);
  typedef int   (__cdecl *_fdstream_fflush_t)(void *);
  typedef int   (__cdecl *_fdstream_fclose_t)(void *);
  typedef int   (__cdecl *_fdstream_feof_t)(void *);

  struct _fdstream_api_t
    {
      _fdstream_fread_t  f_read;
      _fdstream_fwrite_t f_write;
      _fdstream_fseek_t  f_seek;
      _fdstream_ftell_t  f_tell;
      _fdstream_fflush_t f_flush;
      _fdstream_fclose_t f_close;
      _fdstream_feof_t   f_eof;
    };

  struct CXX_NO_VTABLE BaseFileDataStream : RefcountedT<BaseDataStream_>
    {
      void *f_;
      StringT<wchar_t> fullpath_;
      _fdstream_api_t const *api_;

      bool canread_  : 1;
      bool canwrite_ : 1;
      bool canseek_  : 1;

      _TEGGO_EXPORTABLE void _Init(void *fd, pwide_t fname, int opt);
      _TEGGO_EXPORTABLE BaseFileDataStream(_fdstream_api_t const *api);
      _TEGGO_EXPORTABLE virtual ~BaseFileDataStream();

      _TEGGO_EXPORTABLE virtual int Read( void *buf, int count );
      _TEGGO_EXPORTABLE virtual int Write( void const *buf, int count );
      _TEGGO_EXPORTABLE virtual u32_t Features();
      _TEGGO_EXPORTABLE virtual offset_t Available();
      _TEGGO_EXPORTABLE virtual offset_t Tell();
      _TEGGO_EXPORTABLE virtual TEGGOt_E Seek(i32_t pos,int opt = 0);
      _TEGGO_EXPORTABLE virtual TEGGOt_E Flush();
      _TEGGO_EXPORTABLE virtual TEGGOt_E Close();
      _TEGGO_EXPORTABLE virtual bool IsClosed();

      _TEGGO_EXPORTABLE virtual StringT<wchar_t> Identifier();
    };

  struct MemDataStream : BaseFileDataStream
    {
      bool is_unicode_mem_stream_ : 1;
      struct MEMDATA { byte_t *pS,*pI,*pE; };
      MEMDATA memdesc_;
      _TEGGO_EXPORTABLE MemDataStream(void *data, unsigned len, int features);
      _TEGGO_EXPORTABLE MemDataStream(void const *data, unsigned len, int features);
      _TEGGO_EXPORTABLE ~MemDataStream();
      _TEGGO_EXPORTABLE virtual u32_t Features();
    };

  struct FdDataStream : BaseFileDataStream
    {
      _TEGGO_EXPORTABLE FdDataStream(FILE *fd, StringParam fname, int dupit, int opt);
      _TEGGO_EXPORTABLE FdDataStream(StringParam fname, unsigned flags);
      _TEGGO_EXPORTABLE ~FdDataStream();
    };

  struct HandleDataStream : BaseFileDataStream
    {
      _TEGGO_EXPORTABLE HandleDataStream(HANDLE fh, StringParam fname, int dupit, int opt);
      _TEGGO_EXPORTABLE HandleDataStream(StringParam fname, unsigned flags);
      _TEGGO_EXPORTABLE ~HandleDataStream();
      void *GetHandle(int kind) { if ( kind == 'HNDL' ) return f_; return 0; }
    };

  inline rcc_ptr<DataStream> OpenHandleStream(StringParam fname, unsigned flags ) 
    { 
      rcc_ptr<DataStream> ds(new HandleDataStream(fname,flags));
      return ( !ds->Error() ) ? ds : rcc_ptr<DataStream>(0); 
    }

  inline rcc_ptr<DataStream> OpenHandleStream(HANDLE hndl, StringParam name, unsigned flags ) 
    { 
      rcc_ptr<DataStream> ds(new HandleDataStream(hndl,name,0,flags));
      return ( !ds->Error() ) ? ds : rcc_ptr<DataStream>(0); 
    }

  inline rcc_ptr<DataStream> CXX_STDCALL OpenHandleStreamRtry(StringParam fname , unsigned flags, int count = RTRY_COUNT, int timeout = RTRY_TIMEOUT )
    {
      for ( int i = 0; i < count; ++i )
        {
          if ( rcc_ptr<DataStream> ds = OpenHandleStream( fname, flags ) )
            return ds;
          else
            Sleep(timeout);
        }
        
      return rcc_ptr<DataStream>(0);
    }

  struct ArrayDataStream : BaseFileDataStream
    {
      BufferT<byte_t> buffer_;
      unsigned p_;
      _TEGGO_EXPORTABLE ArrayDataStream(unsigned reserved=0);
      _TEGGO_EXPORTABLE ~ArrayDataStream();
      _TEGGO_EXPORTABLE void SwapBuffer(BufferT<byte_t> &buffer);
    };

  typedef FdDataStream FileDataStream;
  typedef rcc_ptr<DataStream> DataStreamPtr;

} // namespace

enum FILEOPEN_FLAG_VALUES
  {
    FILEOPEN_READ = 1,
    STREAM_READ = FILEOPEN_READ,
    FILEOPEN_WRITE = 2,
    STREAM_WRITE = FILEOPEN_WRITE,
    FILEOPEN_APPEND = 4,
    STREAM_APPEND = FILEOPEN_APPEND,
    FILEOPEN_CREATE = 16,
    STREAM_CREATE = FILEOPEN_CREATE,
    FILEOPEN_UNICODE_MEMSTREAM = 32,
    FILEOPEN_RW = FILEOPEN_WRITE | FILEOPEN_READ,
    STREAM_READWRITE = FILEOPEN_RW,
    STREAM_CHARBYCHAR = 64,
    STREAM_NONINHERITABLE = 128,
    STREAM_EXECUTABLE = 256,
    STREAM_NOTSEEK_BEFORE = 512,
    STREAM_NOSHARE_READ = 1024,
    STREAM_SHARE_WRITE = 2048,
  };

#include "streams_u2.inl"

#if defined _TEGGOINLINE
#include "streams.inl"
#endif

#endif // ___945f2106_67c3_4f28_81ba_079b4dfde6c1___
