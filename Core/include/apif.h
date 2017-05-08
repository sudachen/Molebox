
/*

  Copyright (c) 2008-2009, Alexey Sudachen, alexey@sudachen.name

*/

enum APIF_STATUS { APIF_RETURN, APIF_ORIGINAL, APIF_CONTINUE };

typedef struct _API_FILTER
{
    APIF_STATUS (*do_call)(void* flt, int apif_id, void* args, unsigned* result);
    void (*log_state)(void* flt);
} API_FILTER;

enum APIF_ID
{
    APIF_NONE         = 0,
    APIF_CREATEFILE   = 1,
    APIF_OPENFILE     = 2,
    APIF_READFILE     = 3,
    APIF_WRITEFILE    = 4,
    APIF_CLOSE        = 5,
    APIF_QUEFILINFO   = 6,
    APIF_SETFILINFO   = 7,
    APIF_QUEVOLINFO   = 8,
    APIF_QUEDIRFILE   = 9,
    APIF_CREATESECT   = 10,
    APIF_QUESECTINFO  = 11,
    APIF_MAPSECT      = 12,
    APIF_UNMAPSECT    = 13,
    APIF_QUEFATTR     = 14,
    APIF_CREATEHKEY   = 15,
    APIF_OPENHKEY     = 16,
    APIF_SETVALHKEY   = 17,
    APIF_QUEKEY       = 18,
    APIF_QUEVALKEY    = 19,
    APIF_ENUKEY       = 20,
    APIF_ENUVALKEY    = 21,
    APIF_OPENHKEYEX   = 22,
    APIF_NOTIFMULTKEY = 23,
    APIF_NOTIFKEY     = 24,
    APIF_QUEFATTRFULL = 25,
    APIF_DELKEY       = 26,
    APIF_DUPLICATE    = 27,
    APIF_QUEPROCINFO  = 28,
    APIF_CREATECTX    = 29,
    APIF_ADDREFCTX    = 30,
    APIF_RELREFCTX    = 31,
    APIF_ACTIVECTX    = 32,
    APIF_DEACTECTX    = 33,
    APIF_FREETHCTX    = 34,
    APIF_GETACTCTX    = 35,
    APIF_ISACTCTX     = 36,
    APIF_QUEACTCTX    = 37,
    APIF_FINDCTXCSS   = 38,
    APIF_KRCREATECTX  = 39,
    APIF_CSRCALL      = 40,
    APIF_LOCKFILE     = 41,
    APIF_UNLOCKFILE   = 42,
    APIF_GETENVAR     = 43,
    APIF_KCRPROCA     = 44,
    APIF_KCRPROCW     = 45,
    APIF_QUESEQ       = 46,
    APIF_NOTICHNGFL   = 47,
    APIF_KWINEXEC     = 48,
    APIF_CMDLINEA     = 49,
    APIF_CMDLINEW     = 50,
    APIF_DELKEYVAL    = 51,
    APIF_COUNT
};

#ifdef __cplusplus
extern "C" {
#endif

extern void* apif_realcall[APIF_COUNT];
extern void Regster_API_Filter(API_FILTER* flt);

#ifdef __cplusplus
}
#endif

#ifdef __cplusplus

struct APIfilter
{
    virtual APIF_STATUS DoCall(int apif_id, void* args, unsigned* result) = 0;
    virtual void LogState() {}
};

struct APIfilter_C : APIfilter
{
    API_FILTER* filter;
    APIfilter_C(API_FILTER* flt) : filter(flt) {}

    virtual APIF_STATUS DoCall(int apif_id, void* args, unsigned* result)
    {
        return filter->do_call(filter,apif_id,args,result);
    }

    virtual void LogState()
    {
        if (filter->log_state) filter->log_state(filter);
    }
};

struct APIF_
{
    static BufferT<APIfilter*> filters_;
    static void Remove(APIfilter* f);
    static void Push(APIfilter* f);
    static void SetupNtKrHooks(void* ntdll,void* krdll);
    static void UnsetHooks();
    #if !defined PEOVER_MINICORE
    static void AcquireFilterLock();
    static void ReleaseFilterLock();
    #else
    static void AcquireFilterLock() {}
    static void ReleaseFilterLock() {}
    #endif
};

//extern "C" APIF_STATUS __stdcall APIF_DoCall(int apif_id, void *args, unsigned *result);

struct APIFGATE { APIF_* operator->() const { return 0; } };
static const APIFGATE APIF = APIFGATE();

extern void APIF_RegisterFileObjectHooks();
extern void APIF_RegisterSectionObjectHooks();
extern void APIF_RegisterRegObjectHooks(bool=true,bool=false);
extern void APIF_RegisterActCtxHooks();
extern void APIF_RegisterCrProcHooks();
extern void APIF_RegisterCmdlHooks();
//extern unsigned APIF_CrProc_SEP;

extern long __stdcall APIF_RegisterWriteToFile(unsigned flags,wchar_t const* filename,wchar_t const* filter);
extern long __stdcall APIF_RegisterSerilize(unsigned flags,wchar_t const* filter,char** p,int* len);
extern long __stdcall APIF_LogState();

struct COREAPI
{
    unsigned major_version;
    unsigned build_number;
    unsigned (__stdcall* EnableExeFilter)(unsigned);
    unsigned (__stdcall* EnableFsFilter)(unsigned);
    unsigned (__stdcall* EnableRegFilter)(unsigned);
    unsigned (__stdcall* Mount)(wchar_t const* pakagemask, char const* passwd, unsigned atAppfold);
    unsigned (__stdcall* Unmount)(wchar_t const* pakagemask);
};

enum
{
    REGISTRY_DISABLE_VIRTUALIZATION = 0,
    REGISTRY_PARTIAL_VIRTUALIZATION = 1,
    REGISTRY_FULL_VIRTUALIZATION    = 2,
    REGISTRY_VIRTUAL_VIRTUALIZATION = 3,
};

int Get_Registry_Virtualization_Mode();
int Set_Registry_Virtualization_Mode(int mode);

#endif
