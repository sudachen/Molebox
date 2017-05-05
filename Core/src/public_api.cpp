
#include "_common.h"
#include "vfs.h"
#include "apif.h"
#include "XorS.h"
#include "rsa.h"
#include "logger.h"

extern int g_License_Level;
extern "C" void* _PEB();
unsigned QuerySystemHWID();
extern "C" wchar_t** __cdecl buildargv(wchar_t const* cmdl, int* argc);
extern "C" void __cdecl freeargv(wchar_t** vector);

enum _MOLEBOX_PKG_OPTS
{
    MOLEBOX_CORE_PKG_NO_INHERIT   = 1,
    MOLEBOX_CORE_PKG_HIDE_ALL     = 2,
    MOLEBOX_CORE_PKG_AT_APPFOLD   = 4,
    MOLEBOX_CORE_PKG_UNICODE_PWD  = 8,
};

enum
{
    DONT_VIRTUALIZE_REGISTRY      = 0,
    PARTIAL_VIRTUALIZE_REGISTRY   = 1,
    FULLY_VIRTUALIZE_REGISTRY     = 2,
    VIRTUAL_VIRTUALIZE_REGISTRY   = 3,
};

struct MOLE_API
{
	int buildno;
	void* actbase;

	void (__stdcall* Set_CommandLine)(wchar_t* cmdl);
	void (__stdcall* Parse_CommandLine)(int* argc, wchar_t** *argv);

	void (__stdcall* Expmod)(void* a, void* e, void* n, void* R, int bitcount);
	void (__stdcall* Md5)(void* data, unsigned len, void* digest);
	unsigned(__stdcall* PcId)();

	void (__stdcall* CmdLine_SetArgs)(wchar_t** args, int count);
	void (__stdcall* CmdLine_InsertArgs)(int after, wchar_t** args, int count);

	int (__stdcall* Mount)(wchar_t* pkg, char* pwd, int opt);
	int (__stdcall* Mount_At)(wchar_t* pkg, char* pwd, int opt, i64_t offs);
	int (__stdcall* Unmount)(wchar_t* pkg, int opt);
	int (__stdcall* Exec)(wchar_t* progname, int opts);
	int (__stdcall* Registry_Virtualization_Set)(int opt);
	int (__stdcall* Registry_Virtualization_Get)();
};

int __stdcall _API_Registry_Virtualization_Set(int opt)
{
	return Set_Registry_Virtualization_Mode(opt);
}

int __stdcall _API_Registry_Virtualization_Get()
{
	return Get_Registry_Virtualization_Mode();
}

void __stdcall _API_Set_CommandLine(wchar_t* cmdl)
{
	//__asm int 3;
	static wchar_t* buffer = 0;
	if (buffer) free(buffer);
	int cmdl_len = wcslen(cmdl);
	buffer = (wchar_t*)malloc((cmdl_len + 1) * 2);
	memcpy(buffer, cmdl, ((cmdl_len + 1) * 2));

	_XPEB* peb = (_XPEB*)_PEB();
	MY_RTL_USER_PROCESS_PARAMETERS* para = peb->ProcessParameters;

	RtlInitUnicodeString(&para->CommandLine, buffer);
}

void __stdcall _API_Parse_CommandLine(int* argc, wchar_t** *argv)
{
	static wchar_t** s_argv = 0;
	if (s_argv)
	{ freeargv(s_argv); s_argv = 0; }

	_XPEB* peb = (_XPEB*)_PEB();
	MY_RTL_USER_PROCESS_PARAMETERS* para = peb->ProcessParameters;

	s_argv = buildargv(para->CommandLine.Buffer, argc);
	*argv = s_argv + 1;
	--*argc;
}

void Append(BufferT<wchar_t>& b, wchar_t* a)
{
	if (!a) return;
	int l = 0;
	bool quote = false;
	for (wchar_t* q = a; *q; ++q)
	{
		quote |= (*q == ' ' || *q == '"');
		++l;
	}
	if (quote) b.Push('"');
	for (wchar_t* q = a; *q; ++q)
	{
		b.Push(*q);
		if (*q == '"')
		{
			b.Push('"');
			if (q[1]) b.Push('"');
		}
	}
	if (quote && b.Last() != '"') b.Push('"');
}

void __stdcall _API_CmdLine_SetArgs(wchar_t** cmdl, int count)
{
	int argc = 0;
	wchar_t** argv = 0;
	_API_Parse_CommandLine(&argc, &argv);
	BufferT<wchar_t> b;
	//for ( int i = 0; i < argc; ++i )
	//  XLOG|_S*"%d = %s"%i%argv[i];
	Append(b, argv[-1]);
	for (int i = 0; i < count; ++i)
	{
		b.Push(' ');
		Append(b, cmdl[i]);
	}
	b.Push(0);
	_API_Set_CommandLine(+b);
	//XLOG|+b;
}

void __stdcall _API_CmdLine_InsertArgs(int after, wchar_t** cmdl, int count)
{
	int argc = 0;
	wchar_t** argv = 0;
	_API_Parse_CommandLine(&argc, &argv);
	BufferT<wchar_t> b;
	//for ( int i = 0; i < argc; ++i )
	//  XLOG|_S*"%d = %s"%i%argv[i];

	if (after < 0 || after >= argc) after = argc;
	--after;

	for (int i = -1; i < argc; ++i)
	{
		if (i >= 0) b.Push(' ');
		Append(b, argv[i]);
		if (i == after)
			for (int j = 0; j < count; ++j)
			{
				b.Push(' ');
				Append(b, cmdl[j]);
			}
	}
	b.Push(0);

	_API_Set_CommandLine(+b);
	//XLOG|+b;
}

void __stdcall _API_Expmod(void* a, void* e, void* n, void* R, int bitcount)
{
	RSA_expmod((u32_t*)a, (u32_t*)e, (u32_t*)n, (u32_t*)R, bitcount);
}

void __stdcall _API_Md5(void* data, unsigned len, void* digest)
{
	MD5_Hash md5;
	md5.Update(data, len);
	md5.Finalize(digest);
}

unsigned __stdcall _API_PcId()
{
	return QuerySystemHWID();
}

#define _DECODE_PWD \
	if ( pwd && (opt & MOLEBOX_CORE_PKG_UNICODE_PWD) ) \
	{ \
		wchar_t *u_pwd = (wchar_t*)pwd; \
		int i, L = wcslen(u_pwd); \
		pwd = (char*)_alloca(L+1); \
		for ( i = 0; i < L; ++i ) pwd[i] = (char)u_pwd[i]; \
		pwd[i] = 0; \
	}

int __stdcall _API_Mount(wchar_t* pkg, char* pwd, int opt)
{
	int r;
	_DECODE_PWD
	APIF->AcquireFilterLock();
	r =  _VFS::Mount(pkg, pwd, (opt & MOLEBOX_CORE_PKG_AT_APPFOLD));
	APIF->ReleaseFilterLock();
	return r;
}

int __stdcall _API_Mount_At(wchar_t* pkg, char* pwd, int opt, i64_t offs)
{
	int r;
	_DECODE_PWD
	APIF->AcquireFilterLock();
	r = _VFS::Mount_At(pkg, pwd, offs);
	APIF->ReleaseFilterLock();
	return r;
}

int __stdcall _API_Unmount(wchar_t* pkg, int opt)
{
	return 0;
}

enum _MOLEBOX_EXEC_OPTS
{
    MOLEBOX_EXEC_KILL_CURRENT     = 1,
    MOLEBOX_EXEC_RANDOM_BASE      = 2,
    /*MOLEBOX_EXEC_RETURN           = 4,*/
    /*MOLEBOX_EXEC_DONT_CHANGE_PEB  = 8,*/
};

extern "C" void _Internal_Exec(void*);
void* Load_Exe_From_File(pwide_t filename, bool reset_peb_base, bool kill_current);
void Set_Process_Name(wchar_t* process_name);

int __stdcall _API_Exec(wchar_t* progname, int opts)
{
	wchar_t* fullname = (wchar_t*)VirtualAlloc(0, 4096, MEM_COMMIT, PAGE_READWRITE);
	GetFullPathNameW(progname, 4096 / sizeof(wchar_t), fullname, 0);
	Set_Process_Name(fullname);
	if (void* img_base = Load_Exe_From_File(fullname,
	                                        1,
	                                        !!(opts & MOLEBOX_EXEC_KILL_CURRENT)))
	{
		IMAGE_NT_HEADERS* nth = NT_HEADERS(img_base);
		_Internal_Exec((char*)img_base + nth->OptionalHeader.AddressOfEntryPoint);
	}
	else
	{
		__asm mov ecx, __LINE__;
		__asm int 3;
	}

	TerminateProcess((HANDLE) - 1, -1);
	return 0; /*unreachable*/
}

void* QueryAPI()
{
	static MOLE_API api = {0};

	if (!api.buildno)
	{
		api.buildno = _BUILD_NUMBER;

		api.Set_CommandLine = _API_Set_CommandLine;
		api.Parse_CommandLine = _API_Parse_CommandLine;
		api.Expmod = _API_Expmod;
		api.Md5 = _API_Md5;
		api.PcId = _API_PcId;
		api.CmdLine_SetArgs = _API_CmdLine_SetArgs;
		api.CmdLine_InsertArgs = _API_CmdLine_InsertArgs;
		api.Registry_Virtualization_Set = _API_Registry_Virtualization_Set;
		api.Registry_Virtualization_Get = _API_Registry_Virtualization_Get;

		if (g_License_Level >= 50)    // DEMO LICENSE
		{
			api.Mount = _API_Mount;
			api.Mount_At = _API_Mount_At;
			api.Unmount = _API_Unmount;
			api.Exec = _API_Exec;
		}
	}

	return &api;
};

void* QueryAPI1(void* activator_base)
{
	MOLE_API* a = (MOLE_API*)QueryAPI();
	a->actbase = activator_base;
	return a;
}


