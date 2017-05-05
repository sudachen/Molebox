
/*

  Copyright (c) 2008-2009, Alexey Sudachen, alexey@sudachen.name

*/

EXTERN_C void* _NTDLL;
EXTERN_C void* _KERN32;
EXTERN_C void* _USER32;
EXTERN_C void* _ADVA32;
EXTERN_C void* (__stdcall* _get_module_hook)(char* modname);
EXTERN_C void* (__stdcall* _load_module_hook)(char* modname);
EXTERN_C void* get_predefined_module(char* modname);
EXTERN_C void* GetProcAddressIndirect(void* module, char* procname, int hint);

/// - light imports - ///

enum
{
    IMPORT_ERR_MODULE_NO_EXISTS = 1,
};

enum
{
    LI_USE_FULL_THUNK     = 1,
    LI_DONT_SPLICE        = 2,
    LI_WRAP_DEPENDEND     = 4,
    LI_UNPROTECT_SECTION  = 8,
    LI_HOOK_API           = 16,
};

typedef void* (__stdcall* loadlibrary_t)(char* dllname);
typedef void* (__stdcall* getproc_t)(void* module, char* procname);
EXTERN_C void Import(unsigned char* base, IMAGE_NT_HEADERS const* nth, loadlibrary_t l CXX_OPTIONAL_0,
                     getproc_t g CXX_OPTIONAL_0);
EXTERN_C void Import1(IMAGE_IMPORT_DESCRIPTOR* import, unsigned char* base, loadlibrary_t l CXX_OPTIONAL_0,
                      getproc_t g CXX_OPTIONAL_0);
EXTERN_C void Relocate(unsigned char* base, IMAGE_NT_HEADERS const* nth);
