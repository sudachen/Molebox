

  const pchar_t FormatGUID(const GUID& guid,BaseStringT<tTchr>& s)
    {
      s << _S*
          BaseStringT<tTchr>::SelectTstr(
            "%08x-%04x-%04x-%02x%02x-%02x%02x%02x%02x%02x%02x",
            L"%08x-%04x-%04x-%02x%02x-%02x%02x%02x%02x%02x%02x"),
          guid.Data1,guid.Data2,guid.Data3,
          guid.Data4[0],guid.Data4[1],guid.Data4[2],guid.Data4[3],
          guid.Data4[4],guid.Data4[5],guid.Data4[6],guid.Data4[7]);
    }
  #endif // MOLESTUDIO_NO_GUID_SUPPORT

  #if !defined MOLESTUDIO_NO_GUID_SUPPORT
  template<class tTchr>  CXX_NO_INLINE
    const tTchr* GUIDToRegString(const GUID& guid,BaseStringT<tTchr>& s)
      {
        return
          s.SetF(
            BaseStringT<tTchr>::SelectTstr(
              "{%08x-%04x-%04x-%02x%02x-%02x%02x%02x%02x%02x%02x}",
              L"{%08x-%04x-%04x-%02x%02x-%02x%02x%02x%02x%02x%02x}"),
            guid.Data1,guid.Data2,guid.Data3,
            guid.Data4[0],guid.Data4[1],guid.Data4[2],guid.Data4[3],
            guid.Data4[4],guid.Data4[5],guid.Data4[6],guid.Data4[7]);
      }
  #endif

  #if !defined MOLESTUDIO_NO_GUID_SUPPORT

  inline widestring  GUIDToWideString(const GUID& guid)
    {
      widestring ws;
      GUIDToString(guid,ws);
      return ws;
    }

  inline widestring  GUIDToWideRegString(const GUID& guid)
    {
      widestring ws;
      GUIDToRegString(guid,ws);
      return ws;
    }

  inline asciistring GUIDToAsciiString(const GUID& guid)
    {
      asciistring as;
      GUIDToString(guid,as);
      return as;
    }

  inline asciistring GUIDToAsciiRegString(const GUID& guid)
    {
      asciistring as;
      GUIDToRegString(guid,as);
      return as;
    }
  
  #if !defined CXX_NO_GUID_SUPPORT
    CXX_NO_INLINE
    static StringT GenGUID()
      {
        GUID guid;
        CoCreateGuid(&guid);
        StringT s;
        GUIDToString(guid,s);
        return s;
      }
  #endif

  };


  #endif // MOLESTUDIO_NO_GUID_SUPPORT

