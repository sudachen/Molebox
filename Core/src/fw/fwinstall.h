
typedef unsigned char byte_t;
typedef long longptr_t;

int _DLO(void);
int ISNTDLL(int S);
int ISK32DLL(int S);
int LZSS_Decomress(byte_t* in_b, int in_b_len, char* out_b, int out_b_len);
void Do_Cipher(byte_t* unrav, void* b, int count);
void Setup_Decipher(byte_t* unrav, void* K);
unsigned adler32(unsigned adler, byte_t* buf, unsigned len);
int _Core_Offset();
int _Core_Size();
void* _Self_Ptr();
void* _Self_Key();
void* _Query_DT64();

void* _S_Table(int);
#define _S(Mag,Text) _S_Table(Mag)

typedef void* (__stdcall* VA_t)(void*, int, int, int);
typedef void  (__stdcall* VF_t)(void*, int, int);
typedef void* (__stdcall* CFM_t)(void*, void*, int, int, int, void*);
typedef void* (__stdcall* MVOF_t)(void*, int, int, int, int);
typedef void* (__stdcall* MVOFX_t)(void*, int, int, int, int, void*);
typedef void  (__stdcall* UVOF_t)(void*);
typedef void  (__stdcall* CH_t)(void*);
typedef void* (__stdcall* GPA_t)(void*, char*);
typedef void* (__stdcall* Ldrl_t)(char*);
typedef void* (__stdcall* GMH_t)(char*);
typedef int   (__stdcall* GEV_t)(char*, void*, int);
typedef	void *(__stdcall *CF_t)(void*,unsigned,unsigned,void*,unsigned,unsigned,void*);
typedef int   (__stdcall *SFP_t)(void*,unsigned,unsigned*,unsigned);
typedef int   (__stdcall *RF_t)(void*,void*,unsigned,unsigned*,void*);

#define FILE_MAP_WRITE            0x0002
#define FILE_MAP_READ             0x0004
#define FILE_MAP_EXECUTE          0x0008

typedef struct _FUNCS
{
	void*   eh;
	wchar_t* modname;
	int		injected;
	VA_t    Virtual_Alloc;
	VF_t    Virtual_Free;
	CFM_t   Create_File_Mapping;
	MVOF_t  Map_View_Of_File;
	UVOF_t  Unmap_View_Of_File;
	CH_t    Close_Handle;
	GPA_t   Get_Proc_Address;
	Ldrl_t  Load_Library;
	GMH_t   Get_Module_Handle;
	GEV_t   Get_Env_Value;
	MVOFX_t Map_View_Of_File_Ex;
	CF_t	Create_File;
	SFP_t	Set_File_Pointer;
	RF_t	Read_File;
} FUNCS;

void memcopy(byte_t* to, byte_t* from, int count);
void* S_Decrypt(char* b, byte_t* S);
void Relocate_Core(CORE0_HEADER* core);
void Import_Core(CORE0_HEADER* core, FUNCS* f);
void* Find_Dll(int T);
void* Find_Func(void* handle, char* name);
void* Find_GPA(void* ntdll, void* kernl);
int LZSS_Decompress(byte_t* in_b, int in_b_len, char* out_b, int out_b_len);
void Displace_Support_Info(void* core, FUNCS* f);
