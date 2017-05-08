
/*

  Copyright (c) 2008-2009, Alexey Sudachen, alexey@sudachen.name

*/

#include "stbinfo.h"

enum
{
    PEWRAP_EMBEDDING     = 1L << 8,
    PEWRAP_X0n           = 1L << 9,
    PEWRAP_DEBUGCORE     = 1L << 10,
    PEWRAP_LOGGING       = 1L << 11,
    PEWRAP_HASPACKS      = 1L << 12,
    PEWRAP_SAVERSC       = 1L << 13,
    PEWRAP_RELINK        = 1L << 14,
    PEWRAP_REGISTER_OCX  = 1L << 15,
    PEWRAP_DOTNET        = 1L << 16,
    PEWRAP_DEMO          = 0, //1L << 17,
    PEWRAP_X2n           = 1L << 18,
    PEWRAP_ANTIHACK      = 1L << 19,
    PEWRAP_SAVEIAT       = 1L << 20,
    PEWRAP_BOXEMBEDED    = 1L << 21,
    PEWRAP_DBGLOG        = 1L << 22,
    PEWRAP_HANDLERGS     = 1L << 23,
    PEWRAP_HANDLERGS_FULL= 1L << 24,
    PEWRAP_STRONGANTIHACK  = 1L << 25,
    PEWRAP_HWID          = 1L << 26,
    PEWRAP_BOXED_ACT     = 1L << 27,
    PEWRAP_SELFCONTAINS  = 1L << 28,
    PEWRAP_STUBBED       = 1L << 29,
    PEWRAP_INSINITY      = 1L << 30,
    PEWRAP_MINIDUMPFULL  = 1L << 31,
    PEWRAP_L1            = 1L,
    PEWRAP_L2            = 2L,
    PEWRAP_NO_CRC0       = 4L,
    PEWRAP_INJECT        = 1L<<7,
};

bool PEoverOpenWfile(pwide_t outfile, unsigned flags, pwide_t stub, u32_t minsize, u32_t mintls, EhFilter ehf);
bool PEoverCloseWfile(EhFilter ehf);
bool PEoverAttachActivator(pwide_t  activator,EhFilter ehf);
bool PEoverBundleDLL(pwide_t dllname, bool hideonly, EhFilter ehf);
bool PEoverFilterBounds(pwide_t  pefile, ArrayT<StringW>& a, bool autobound, EhFilter ehf, bool silance=false);
bool PeIsDotNet(pwide_t  file, unsigned ver[3], EhFilter ehf);
bool SearchFullPath(pwide_t  name, StringA& fullpath);
bool PeFixCRC(pwide_t  filename, EhFilter ehf);
bool PeCopyFile(StringParam infile,StringParam outfile,bool preserve_extradata,EhFilter ehf);
bool PeCopyExtraData(StringParam infile,StringParam outfile,bool encrypt,EhFilter ehf);
bool PeGetParams(DataStreamPtr ds, u32_t* size, u32_t* tls, u32_t* base, bool* reloc);
bool ReadImageNtHeaders(DataStreamPtr ds,IMAGE_NT_HEADERS* h);

bool PEoverSetCmdline(pwide_t,EhFilter);
bool PEoverPredefineReg(pwide_t,EhFilter);
bool PEoverPredefineEnviron(pwide_t,EhFilter);

unsigned QuerySystemHWID();
unsigned RVAtoOffs(byte_t* mapped,unsigned rva);
void SysTruncateFile(StringParam fname,unsigned len);
int InitXdataSection(byte_t* mapped,IMAGE_SECTION_HEADER* S,IMAGE_NT_HEADERS* nth,unsigned* file_length,EhFilter ehf);
bool InitTlsDataSection(byte_t* mapped,IMAGE_SECTION_HEADER* S,IMAGE_NT_HEADERS* nth,unsigned* file_length,unsigned reserved,
                        unsigned inited, EhFilter ehf);
bool InitRSRCSection(byte_t* mapped,IMAGE_SECTION_HEADER* S,IMAGE_NT_HEADERS* nth,unsigned* file_length,BufferT<byte_t>& rsc,
                     EhFilter ehf);
unsigned FindFunction(byte_t* mapped,IMAGE_NT_HEADERS* nth,IMAGE_SECTION_HEADER* S,char* fn);
void ReorderImports(byte_t* mapped, BufferT<SQUOTED_IMPORT>& squoted, byte_t* xkey, EhFilter ehf);

struct FileContainer : Refcounted
{
    virtual bool CopyExtraData(StringParam infile, bool encrypt, EhFilter ehf) = 0;
    virtual bool AttachActivator(StringParam, EhFilter ehf) = 0;
    virtual bool SetCmdline(StringParam,EhFilter) = 0;
    virtual bool SetRegmask(StringParam,EhFilter) = 0;
    virtual bool PredefineReg(StringParam,EhFilter) = 0;
    virtual bool PredefineEnviron(StringParam,EhFilter) = 0;
    virtual bool BundleDLL(StringParam dllname, bool hideonly, EhFilter ehf) = 0;
    virtual bool Close(EhFilter) = 0;
    virtual bool Finalize(EhFilter) = 0;
    virtual STB_INFO2* StbInfo2() = 0;
    virtual ~FileContainer() {}
};

typedef rcc_ptr<FileContainer> FileContainerPtr;
FileContainerPtr OpenStubbedFileContainer(StringParam original,StringParam target, StringParam stub, unsigned flags,
        unsigned minSize, unsigned minTls, EhFilter ehf);
FileContainerPtr OpenOldFileContainer(StringParam original,StringParam target,unsigned flags, unsigned minSize, unsigned minTls,
                                      EhFilter ehf);

struct LessBundle
{
    bool operator ()(BUNDLED_IMPORT const& a,BUNDLED_IMPORT const& b) const { return a.no < b.no; }
};

bool AfxIsExecutable(StringParam name);
bool AfxAttachBundles(StringParam target, BufferT<BUNDLED_IMPORT>& bundles, byte_t* xcrypt_key, OCC& info_bundle, EhFilter ehf);
bool AfxBundleDLL(StringParam dllname, StringA& module, StringParam target, BUNDLED_IMPORT& bi, byte_t* xcrypt_key,
                  EhFilter ehf);
bool AfxAttachSquoted(StringParam target, BufferT<SQUOTED_IMPORT>& squoted, byte_t* xcrypt_key, OCC& info_squoted,
                      EhFilter ehf);
StringW FindStub(StringParam s);
StringW CreatePersonalStubName();
DataStreamPtr CopyStub(StringParam from, StringParam to, void*, EhFilter ehf);

