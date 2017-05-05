struct DataSubstream : RefcountedT<BaseDataStream_>
  {
    typedef DataStream::offset_t offset_t;
    _TEGGO_EXPORTABLE DataSubstream(DataStream *container,offset_t offset,offset_t count,unsigned flags);
    _TEGGO_EXPORTABLE ~DataSubstream();
    _TEGGO_EXPORTABLE int Write( void const *buf, int count );
    _TEGGO_EXPORTABLE int Read( void *buf, int count );
    _TEGGO_EXPORTABLE u32_t Features();
    _TEGGO_EXPORTABLE offset_t Available();
    _TEGGO_EXPORTABLE TEGGOt_E Flush();
    _TEGGO_EXPORTABLE TEGGOt_E Close();
    _TEGGO_EXPORTABLE bool IsClosed();
    _TEGGO_EXPORTABLE virtual StringT<wchar_t> Identifier();

    DataStreamPtr ds_;
    offset_t start_;
    offset_t count_;
    offset_t pos_;
    bool seekBefore;
  };
  
_TEGGO_STREAMS_FAKE_INLINE
DataSubstream::DataSubstream(DataStream *container,offset_t offset,offset_t count,unsigned flags)
  {
    if ( container )
      { 
        ds_ = RccRefe(container);
        seekBefore = (container->Features() & CAN_SEEK) && !(flags&STREAM_NOTSEEK_BEFORE);
        start_ = offset;
        count_ = count;
        pos_ = 0;
      }
  }
  
_TEGGO_STREAMS_FAKE_INLINE
DataSubstream::~DataSubstream()
  {
    Close();
  }
  
_TEGGO_STREAMS_FAKE_INLINE
int DataSubstream::Write( void const *buf, int count )
  {
    if ( ds_ )
      {
        if ( count_ >0 && pos_ + count > count_ ) count = count_ - pos_;
        if ( count > 0 )
          {
            if ( seekBefore ) ds_->Seek(start_+pos_);
            int q = ds_->Write(buf,count);
            if ( q > 0 ) pos_ += q;
            REQUIRE( count_ < 0 || pos_ <= count_ );
            return q;
          } 
        else
          return 0;
      }
    return -1;
  }
  
_TEGGO_STREAMS_FAKE_INLINE
int DataSubstream::Read( void *buf, int count )
  {
    return -1;
  }
  
_TEGGO_STREAMS_FAKE_INLINE
u32_t DataSubstream::Features()
  {
    if ( ds_ )
      {
        unsigned f = ds_->Features() & (CAN_SEEK|CAN_WRITE|CAN_READ);
        return f;
      }
      return 0; 
  }
  
_TEGGO_STREAMS_FAKE_INLINE
DataSubstream::offset_t DataSubstream::Available()
  {
    if ( ds_ )
      {
        offset_t a = ds_->Available();
        if ( a < start_ ) a = 0; else a-=start_;
        if ( count_ < 0 ) return a;
        else
          return cxx_min(count_,a);
      }
    return 0;
  }
  
_TEGGO_STREAMS_FAKE_INLINE
TEGGOt_E DataSubstream::Flush()
  {
    if ( ds_ )
      return ds_->Flush();
    return TEGGO_CLOSED;
  }
  
_TEGGO_STREAMS_FAKE_INLINE
TEGGOt_E DataSubstream::Close()
  {
    Unrefe(ds_);
    return TEGGO_OK;
  }
  
_TEGGO_STREAMS_FAKE_INLINE
bool DataSubstream::IsClosed()
  {
    return !ds_;
  }
  
_TEGGO_STREAMS_FAKE_INLINE
StringW DataSubstream::Identifier()
  {
    if ( ds_ )
      return StringW(_S*L"substream:%s"%ds_->Identifier());
    return "";
  }

