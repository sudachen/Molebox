
/*

  Copyright (C) 2008, Alexey Sudachen, DesaNova Ltda.
  http://www.desanova.com

*/

struct Packager : Ireferred
  {

    enum
      {
        F_HIDDEN     = 1L,
        F_COMPRESSED = 1L << 1,
        F_ENCRYPTED  = 1L << 2,
        F_ACTIVEX    = 1L << 3,
        F_EXECUTABLE = 1L << 4,
        F_EXTERNAL   = 1L << 5,
        F_NOACCESS   = 1L << 6,
      };

    virtual bool Start(DataStream *ds, pchar_t pwd, EhFilter ehf) = 0;
    virtual bool File(StringParam srcname, StringParam dstname, unsigned flags, EhFilter ehf, unsigned *persent) = 0;
    virtual bool Stream(byte_t *data, unsigned count,bool xzip,EhFilter ehf) = 0;
    virtual bool Finish(u32_t /*out*/ *offset,EhFilter ehf) = 0;

    virtual ~Packager() {}
    static rcc_ptr<Packager> Create();

  };
