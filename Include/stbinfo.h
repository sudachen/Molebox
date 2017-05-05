
#ifndef __938F65D3_9ACB_4AFC_9846_70C943799CB7__
#define __938F65D3_9ACB_4AFC_9846_70C943799CB7__

//enum { STB_SIGN_FIRST = 0x454C4F4D, STB_SIGN_SECOND = 0x34584f42 };
enum { STB_SIGN_FIRST = 0x554A4F58, STB_SIGN_SECOND = 0x4A4E414D };

typedef struct _OCC
{
	unsigned offset;
	unsigned count;
	unsigned crc;
} OCC;

typedef struct _STB_INFO
{
	unsigned oep;
	unsigned flags;
	unsigned originals_offs;
	unsigned tls_index;
	unsigned tls_callback;
	struct _OCC act;
	struct _OCC core;
	struct _OCC zip;
	struct _OCC bundle;
	struct _OCC squoted;
	struct _OCC squoted_act;
	struct
	{
		unsigned VirtualAddress;
		unsigned Size;
	} cor20;
	unsigned extra_offs;
	unsigned extra_count;
	unsigned hwid;
	union
	{
		char act_file_name[256];
		wchar_t cmdline[128];
	};
	unsigned char xcrypt_key[16];
} STB_INFO;

typedef struct _STB_INFO2_FLAGS
{
	unsigned extended: 1;
	unsigned minidump_full: 1;
	unsigned insinity: 1;
	unsigned r: 29;
} STB_INFO2_FLAGS;

typedef struct _STB_INFO2
{
	STB_INFO i;
	unsigned char origin_md5[16];
	unsigned origin_crc32;
	unsigned mxbpack_build;
	unsigned coreemb_build;
	unsigned _1;
	unsigned hostip;
	unsigned licid;
	unsigned actxid;
	struct _STB_INFO2_FLAGS f;
	struct _OCC streams;
	unsigned catalog;
	unsigned liclevel;
	struct _OCC regmask;
	unsigned char r[3 * 16 - 3 * 8];
	wchar_t* modname;
	unsigned padding;
} STB_INFO2;

enum
{
    STBF_EXTERNAL_CORE = 1L << 31,
    STBF_EXTERNAL_ACT  = 1L << 30,
    STBF_REGISTER_OCX  = 1L << 29,
    STBF_CORE_ZIPPED   = 1L << 28,
    STBF_CORE_XCRYPED  = 1L << 27,
    STBF_ACT_ZIPPED    = 1L << 26,
    STBF_ACT_XCRYPED   = 1L << 25,
    STBF_EXE_ZIPPED    = 1L << 24,
    STBF_EXE_BUNDLED   = 1L << 23,
    STBF_EXE_DOTNET    = 1L << 22,
    STBF_EXE_DEMO      = 1L << 21,
    STBF_EXE_ANTIHACK  = 1L << 20,
    STBF_EXE_ARMODIL   = 1L << 19,
    STBF_SAVE_IAT      = 1L << 18,
    STBF_CMDLINE       = 1L << 17,
    STBF_BIND2HWID     = 1L << 16,
    STBF_EXTENDED      = 1L << 15,
    STBF_BOXED_ACT     = 1L << 14,
    STBF_EMBEDDED_LOADING = 1L << 13,
    STBF_CHILD         = 1L << 12, /*free*/
};

enum { CORE0_SIGN_FIRST = 0x45455453, CORE0_SIGN_SECOND = 0x584F424C };

typedef struct _CORE0_HEADER
{
	unsigned sign[2];
	unsigned build_number;
	unsigned imports;
	unsigned imports_size;
	unsigned relocs;
	unsigned relocs_size;
	unsigned image_base;
	unsigned flags;
	unsigned inity;
	unsigned install;
	unsigned sdkinit;
	unsigned inject;
} CORE0_HEADER;

typedef struct _ZIPPED_SECTION
{
	char     name[8];
	unsigned access;
	unsigned rva;
	unsigned vasize;
	unsigned rawsize;
	unsigned offset;
	unsigned count;
	unsigned crc;
	unsigned zipped: 1;
	unsigned xcrypt: 1;
} ZIPPED_SECTION;

typedef struct _ZIPPED_HEADER
{
	unsigned number;
	unsigned file_alignment;
	unsigned section_alignment;
	IMAGE_DATA_DIRECTORY orgdir[16];
	struct _ZIPPED_SECTION section[1];
} ZIPPED_HEADER;

typedef struct _BUNDLED_IMPORT
{
	char     name[64];   /* xored */
	unsigned count;
	unsigned offset;
	unsigned crc;
	unsigned size;
	unsigned address;   /* if not system & not zero - palcing address */
	/* if system - address of IMPORT_RECORD */
	unsigned iidName;
	unsigned iidOffs;
	unsigned iidFth;
	unsigned iidOfth;
	void*    handle;

	unsigned system: 1; /* virtual bundling, use system dll */
	unsigned zipped: 1;
	unsigned xcrypt: 1;
	unsigned add2lst: 1; /* add to peb.ldr & if addres != 0 place to address */
	unsigned actfake: 1;
	unsigned activator: 1; /* the import record for activator */
	unsigned no: 8;
} BUNDLED_IMPORT;

typedef struct _BUNDLE_HEADER
{
	unsigned sign[2];
	unsigned number;
	unsigned r;
} BUNDLE_HEADER;

typedef struct _BUNDLE_DATA
{
	struct _BUNDLE_HEADER  hdr;
	struct _BUNDLED_IMPORT rec[];
} BUNDLE_DATA;

typedef struct _SQUOTED_IMPORT
{
	unsigned rva;
	unsigned thunk;
} SQUOTED_IMPORT;

typedef struct _SQUOTED_HEADER
{
	unsigned sign[2];
	unsigned number;
	unsigned r;
} SQUOTED_HEADER;

typedef struct _SQUOTED_DATA
{
	struct _SQUOTED_HEADER hdr;
	struct _SQUOTED_IMPORT rec[];
} SQUOTED_DATA;

typedef struct _STREAMS_HEADER
{
	unsigned sign[2];
	unsigned number;
	unsigned r;
} STREAMS_HEADER;

typedef struct _STREAMS_RECORD
{
	unsigned id;
	unsigned offs;
	unsigned count;
	unsigned rsize;
	unsigned zipped: 1;
	unsigned xcrypted: 1;
} STREAMS_RECORD;

typedef struct _STREAMS_DATA
{
	struct _STREAMS_HEADER hdr;
	struct _STREAMS_RECORD rec[];
} STREAMS_DATA;

#endif  /* __938F65D3_9ACB_4AFC_9846_70C943799CB7__ */
