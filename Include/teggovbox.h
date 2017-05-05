
/*

  Copyright (c) 2008-2009, Alexey Sudachen, alexey@sudachen.name

*/

#ifndef ___86f249bb_663f_4d00_a538_e2d00cda08aa___
#define ___86f249bb_663f_4d00_a538_e2d00cda08aa___

enum _TEGGOVBOX_CONSTANTS
{
    //    TEGGOVBOX_TEXTFORMAT       = 0x100000,
    //    TEGGOVBOX_BINARYFORMAT     = 0x200000,

    TEGGOVBOX_LOG_ACTIVITY             = 1L,
    TEGGOVBOX_VIRTUALIZE_REGISTRY      = 1L << 1,
    TEGGOVBOX_VIRTUALIZE_FS            = 1L << 2,
    //TEGGOVBOX_VIRTUALIZE_FS_FOR_WRITE  = (1L<<3) | (1L<<2),
    TEGGOVBOX_SEARCH_FOR_EMBEDDINGS    = 1L << 4,
    TEGGOVBOX_LOG_ACTIVITY_DBG         = 1L << 5,
    TEGGOVBOX_VIRTUALIZE_REGISTRY_FULL = 1L << 6,
    TEGGOVBOX_CORE_PROCESS             = 1 << 3,
    TEGGOVBOX_CORE_INJECT              = 1 << 7,
    //TEGGOVBOX_LOG_REGISTRY_LOADING     = 1L<<7,

    //    TEGGOVBOX_LAST_ENTRIES_ONLY       = 0x1000,
    //    TEGGOVBOX_TEXT_FORMAT             = 0,
    //    TEGGOVBOX_BINARY_FORMAT           = 1,
    TEGGOVBOX_ONELINE_FORMAT          = 2,
};

typedef struct _TEGGOVBOX_FUNCTIONS_TABLE
{
	long(__stdcall* WriteVirtualRegistry)(unsigned flags, wchar_t const* filename, wchar_t const* filter);
	long(__stdcall* SerilizeVirtualRegistryAsText)(unsigned flags, wchar_t const* filter, char** p, int* len);
	long(__stdcall* DeserilizeVirtualRegistryFromText)(unsigned flags, wchar_t const* filter);
	//long (__stdcall *MaskVirtualRegistryValues)(unsigned flags,wchar_t const *pattern,wchar_t const *mask);
	//long (__stdcall *MarkVirtualRegistry)();
	long(__stdcall* LogState)();
} TEGGOVBOX_FUNCTIONS_TABLE;

#define TEGGOVBOX_INSTALL_ROUTINE "_Install@8"
typedef TEGGOVBOX_FUNCTIONS_TABLE* (__stdcall* TEGGOVBOX_INSTALL_FUNCTION)(unsigned flags, void*);

#endif /*___86f249bb_663f_4d00_a538_e2d00cda08aa___*/
