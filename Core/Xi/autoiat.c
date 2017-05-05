
#include <C+.hc>

static void *_KERN32 = 0;
static void *_NTDLL = 0;

static int chrcmp(unsigned register val,byte_t *p)
{          
	if (!val) return 1;                                                   
	if ( *p == (byte_t)val || *p == (byte_t)(val>>8) ) 
		return chrcmp(val>>16,p + 2);
	return 0;
}

static void *In_DLLS(void *q)
{             
	wchar_t *p = *(wchar_t**)((byte_t*)q+40);  
	wchar_t *s = p;                           
	while ( *p++ ) { if (p[-1] == '\\') s = p; }
	return s;
}

typedef struct _XPEB_LDR_DATA 
{
	ULONG                   Length;
	BOOLEAN                 Initialized;
	PVOID                   SsHandle;
	LIST_ENTRY              InLoadOrderModuleList;
	LIST_ENTRY              InMemoryOrderModuleList;
	LIST_ENTRY              InInitializationOrderModuleList;
} XPEB_LDR_DATA;

typedef struct _XLDR_MODULE
{
	LIST_ENTRY              InLoadOrderModuleList;
	LIST_ENTRY              InMemoryOrderModuleList;
	LIST_ENTRY              InInitializationOrderModuleList;
	PVOID                   BaseAddress;
	PVOID                   EntryPoint;
	ULONG                   SizeOfImage;
	/*UNICODE_STRING*/ void* FullDllName;
	/*UNICODE_STRING*/ void* BaseDllName;
	ULONG                   Flags;
	SHORT                   LoadCount; 
	SHORT                   TlsIndex;
	LIST_ENTRY              HashTableEntry;
	ULONG                   TimeDateStamp;
} XLDR_MODULE;

XPEB_LDR_DATA *_XLDR()
{
	XPEB_LDR_DATA *Ldr;
	__asm mov eax, fs:[0x18] 
	__asm mov eax, [eax+0x30] // PEB
	__asm mov eax, [eax+0x0c] // Ldr
	__asm mov Ldr, eax
	return Ldr;
}

static int Is_NTDLL(void *q)
{               
	byte_t *p = In_DLLS(q);
	return chrcmp(0x54744e6e,p) && chrcmp(0x4c6c4464,p+4);
}

static int Is_K32DLL(void *q)
{
	byte_t *p = In_DLLS(q);
	return chrcmp(0x45654b6b,p) && chrcmp(0x6e4e5272,p+4)
		&& chrcmp(0x4c6c4565,p+8) && chrcmp(0x32323333,p+12);
}

static void Find_Kr_n_Nt()
{
	XLDR_MODULE *l = (XLDR_MODULE *)_XLDR()->InLoadOrderModuleList.Flink;  
	XLDR_MODULE *S = (XLDR_MODULE *)l->InLoadOrderModuleList.Flink; // skip executable image

	while ( S != l && (!_KERN32 || !_NTDLL) )
	{
		if ( Is_NTDLL(S) )
			_NTDLL = S->BaseAddress;
		else if ( Is_K32DLL(S) )
			_KERN32 = S->BaseAddress;
		S = (XLDR_MODULE *)S->InLoadOrderModuleList.Flink;
	}
}

static void *Get_Predefined_Module(char *modname)
{
	struct _predefined { void **hm; char *modname; };
	static struct _predefined predefined[] =
	{
		{&_KERN32,/*"kernel32"*/"\x3e\x30\x27\x3b\x30\x39\x66\x67"},
		{&_NTDLL,/*"ntdll"*/"\x3b\x21\x31\x39\x39"}
	};

	if ( !_KERN32 || !_NTDLL )
		Find_Kr_n_Nt();

	__Gogo
	{
		char *x = modname;
		int i = 0;
		for ( ; i < 2; ++i )
		{
			char *k = predefined[i].modname;
			while ( *k )
				if ( *x
					&& (( *x <= 'Z' && *x >= 'A' && *k == ((*x-'A'+'a')^'\x55') )
					|| *k == (*x^'\x55')) )
					++k,++x;
				else
					break;
			if ( !*k )
				return *predefined[i].hm;
		}
	}

	return 0;
}

static int Gpa_Xstrncmp(char *a, char *b, int l)
{
	for ( ;l && *a && *a == *b; --l, ++a, ++b ) (void)0;
	return l ? (int)*(byte_t*)a-(int)*(byte_t*)b : 0;
}

static void *Gpa_Function_Bsearch(byte_t *base,IMAGE_EXPORT_DIRECTORY *dir,char *name,int hint)
{
	int k = 0;
	int L = 0;
	int R = 0;

	if ( hint >= 0 && hint < dir->NumberOfNames )
		if ( 0 == strcmp(name,(char*)base+((unsigned int*)(base+dir->AddressOfNames))[hint]) )
		{
			k = hint; goto l_hinted;
		}

		L = 0;
		R = dir->NumberOfNames-1;
		while ( L<=R )
		{
			byte_t *r_name;
			int cmp;
			k = (L+R)/2;
			r_name = base+((unsigned int*)(base+dir->AddressOfNames))[k];
			if ( hint < 0 )
				cmp = Gpa_Xstrncmp(name,(char*)r_name,127);
			else
				cmp = strncmp(name,(char*)r_name,127);
			if ( !cmp )
			{
				unsigned fndx;
				void *proc;
l_hinted:
				fndx = ((unsigned short*)(base+dir->AddressOfNameOrdinals))[k];
				proc =
					(base + ((unsigned int*)(base+dir->AddressOfFunctions))[fndx/*-dir->Base*/]);
				return  proc;
			}
			else
				if ( cmp > 0 ) L = k+1;
				else R = k-1;
		}
		return 0;
}

static void *Get_Proc_Address_Indirect(void *module,char *procname,int hint)
{
repeat:
	if ( (unsigned int)module > 64*1024 && (unsigned int)module < 0xffff0000 && !((unsigned int)module&3) )
	{
		byte_t *base = (byte_t*)module;
		IMAGE_NT_HEADERS const *_nth = (IMAGE_NT_HEADERS const*)(base+((IMAGE_DOS_HEADER const*)base)->e_lfanew);
		IMAGE_DATA_DIRECTORY expo = _nth->OptionalHeader.DataDirectory[IMAGE_DIRECTORY_ENTRY_EXPORT];
		IMAGE_EXPORT_DIRECTORY *edir = (IMAGE_EXPORT_DIRECTORY*)(base+expo.VirtualAddress);
		byte_t *p = 0;

		if ( !expo.Size || !expo.VirtualAddress )
			return 0;

		if ( ((unsigned int)procname >> 16) == 0 )
			p = (base +
			((unsigned int*)(base+edir->AddressOfFunctions))
			[(unsigned int)procname-edir->Base]);
		else
			p = (byte_t *)Gpa_Function_Bsearch(base,edir,procname,hint);

		if ( base+expo.VirtualAddress < p && p < base+expo.VirtualAddress+expo.Size )
		{
			char modname[128] = {0};
			char *m = modname;
			while ( *p && *p != '!' && *p != '.' ) *m++ = *p++;
			procname = (char *)(p+1);

			if ( !(module = Get_Predefined_Module(modname)) )
				if ( !(module = GetModuleHandleA(modname) ) )
					module = LoadLibraryA(modname);

			if ( module )
				goto repeat;
			else
				return 0;
		}
		else
			return p;
	}
	return 0;
}

void *__stdcall AutoIatSpliceProcAddress(char *modname, void **modhandle, unsigned char *xxored, unsigned xno)
{
	byte_t *xored = xxored;
	char AutoIatSpliceProcAddress_b[128] = {0};
	void *f = 0;
	byte_t *bp = (byte_t*)AutoIatSpliceProcAddress_b;
	ulong_t sid = xno;
	int no = 0;

	sid += 0xabecaffe; // 0xfabecaffe; ???
	do
	{
		sid = 1664525*sid + 1013904223;
		*bp = (sid >> ( no++ % 8 )) ^ *xored++;
	}
	while ( *bp++ && no < 127 );

	if ( !*modhandle )
		if ( !(*modhandle = Get_Predefined_Module(modname)) )
			if ( !(*modhandle = LoadLibraryA(modname)) )
			{
				__asm mov ecx,__LINE__;
				__asm int 3;
				;
			}

	if ( !(f = Get_Proc_Address_Indirect(*modhandle,AutoIatSpliceProcAddress_b,0)) )
	{
		__asm mov ecx,__LINE__;
		__asm int 3;
	}

	memset(AutoIatSpliceProcAddress_b,0xcc,sizeof(AutoIatSpliceProcAddress_b));
	return f;
}
