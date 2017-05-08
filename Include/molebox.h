
/*

  Copyright (C) 2014, Alexey Sudachen, https://goo.gl/lJNXya.

*/

#ifndef C_once_73505ECE_CF68_40AB_A9CC_5D139B77F710
#define C_once_73505ECE_CF68_40AB_A9CC_5D139B77F710

#include <stdint.h>

#ifdef __cplusplus
extern "C" {
#endif

#ifdef _MOLEBOX_BUILD_DLL
#define EXPORTABLE __declspec(dllexport)
#elif !defined _MOLEBOX_STATIC
#define EXPORTABLE __declspec(dllimport)
#else
#define EXPORTABLE
#endif

enum
{
    MOLEBOX_CORE_FILES              = 1 << 2,
    MOLEBOX_CORE_REGISTRY           = 1 << 1,
    MOLEBOX_CORE_REGISTRY_FULL      = (1 << 6) | MOLEBOX_CORE_REGISTRY,
    MOLEBOX_CORE_PROCESS            = 1 << 3,
    MOLEBOX_CORE_LOGGING            = 1,
    MOLEBOX_CORE_DEBUG_LOG          = (1 << 5) | MOLEBOX_CORE_LOGGING,
    MOLEBOX_CORE_LOOK_FOR_PACKAGE   = 1 << 4,
    MOLEBOX_CORE_DEFAULT            = 0 | MOLEBOX_CORE_FILES | MOLEBOX_CORE_REGISTRY | MOLEBOX_CORE_PROCESS,
    MOLEBOX_CORE_INJECT             = 1 << 7,
};

enum
{
    MOLEBOX_CORE_ERROR_FAILED_TO_LOAD = -1,
    MOLEBOX_CORE_ERROR_ALREADY_LOADED = -2,
    MOLEBOX_CORE_ERROR_IS_NOT_LOADED  = -3,
    MOLEBOX_CORE_ERROR_FILE_NOT_FOUND = -4,
    MOLEBOX_CORE_ERROR_RSRC_NOT_FOUND = -5,
};

enum _MOLEBOX_VREGISTRY_OPTS
{
    DONT_VIRTUALIZE_REGISTRY          = 0,
    PARTIAL_VIRTUALIZE_REGISTRY       = 1,
    FULLY_VIRTUALIZE_REGISTRY         = 2,
    VIRTUAL_VIRTUALIZE_REGISTRY       = 3,
};

#define MOLEBOX_FOUR_BYTES(a,b,c,d) \
    ((uint32_t)(a)<<24) | ((uint32_t)(b)<<16) | \
    ((uint32_t)(c)<<8)  | ((uint32_t)(d))

typedef struct MOLEBOX_API
{
    int buildno;
    void* activator_base_address;
    void (__stdcall* CmdLine_Set)(const wchar_t* cmdl);
    void (__stdcall* CmdLine_Parse)(/*out*/ int* argc, /*out*/ wchar_t** *argv);
    void (__stdcall* Expmod)(void* a, void* e, void* n, /*out*/ void* R, int bitcount);
    void (__stdcall* Digest)(void* data, unsigned len, /*out*/ void* digest);
    uint32_t (__stdcall* PcId)();
    void (__stdcall* CmdLine_SetArgs)(const wchar_t** args, size_t count);
    void (__stdcall* CmdLine_InsertArgs)(int after, const wchar_t** args, size_t count);
    int (__stdcall* Mount)(const wchar_t* mask, char* passwd, int opts);
    int (__stdcall* Mount_At)(const wchar_t* path, char* passwd, int opts, int64_t offs);
    void (__stdcall* _r0)(); /*int (__stdcall* Unmount)(const wchar_t* mask, int opts);*/
    int (__stdcall* Exec)(const wchar_t* progname, int opts);
    int (__stdcall* Set_Registry_Virtualization)(int opts);
    int (__stdcall* Get_Registry_Virtualization)();
} MOLEBOX_API;

EXPORTABLE MOLEBOX_API* Molebox_Query_API();
EXPORTABLE const void* Molebox_Get_Core(uint32_t* core_size);
EXPORTABLE int Molebox_Init(int opts, void* key, int64_t offs);
EXPORTABLE int Molebox_Init_RsRc(int opts, int rcid, char* passwd, void* key, int64_t offs);
EXPORTABLE int Molebox_BuildNo();
EXPORTABLE int Molebox_PcId();
EXPORTABLE int Molebox_CmdLine_Set(const wchar_t* cmdl);
EXPORTABLE int Molebox_CmdLine_Parse(int* argc, wchar_t** * argv);
EXPORTABLE int Molebox_CmdLine_SetArgs(const wchar_t** args, size_t count);
EXPORTABLE int Molebox_CmdLine_InsertArgs(int after, const wchar_t** args, size_t count);
EXPORTABLE int Molebox_Mount(const wchar_t* mask, char* passwd, int opts);
EXPORTABLE int Molebox_Mount_At(const wchar_t* path, char* passwd, int opts, int64_t offs);
EXPORTABLE int Molebox_Mount_RsRc(HANDLE, int rcid, char* passwd, int opts);
EXPORTABLE int Molebox_Unmount(const wchar_t* mask, int opts);
EXPORTABLE int Molebox_Exec(const wchar_t* progname, int opts);

#ifdef __cplusplus
} /*extern "C"*/
#endif

#endif /* C_once_73505ECE_CF68_40AB_A9CC_5D139B77F710 */
